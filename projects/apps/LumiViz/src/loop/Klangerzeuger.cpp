/**
 * @file Klangerzeuger.cpp
 * @brief Loop: die lebende Quelle im Mixer (Stufe 5)
 */

#include "loop/Klangerzeuger.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace lumi::loop
{

namespace
{

const Klaenge kKeineKlaenge;

/// Ein Bus, in den so lange nichts kam, gilt als verklungen.
constexpr std::int64_t kLangeStill = std::int64_t{1} << 60;

/// Im Ton-Thread: das neue Programm uebernimmt je Clip, den es im alten
/// schon gab (derselbe Kanal, derselbe Takt), was dessen Effekte gehoert
/// haben. Holt keinen Speicher.
void uebernimmGedaechtnis(const Programm* alt, const Programm& neu)
{
    if (alt == nullptr || alt->effekte == nullptr || neu.effekte == nullptr) return;
    if (alt->ablauf.rate != neu.ablauf.rate) return;
    EffektZustand& von = *alt->effekte;
    EffektZustand& nach = *neu.effekte;
    for (std::size_t n = 0; n < neu.ablauf.busse.size(); ++n)
    {
        const Bus& bus = neu.ablauf.busse[n];
        for (std::size_t a = 0; a < alt->ablauf.busse.size(); ++a)
        {
            const Bus& vorher = alt->ablauf.busse[a];
            if (vorher.kanal != bus.kanal || vorher.startTakt != bus.startTakt) continue;
            nach.ketten[n].uebernimm(von.ketten[a]);
            nach.stille[n] = von.stille[a];
            break;
        }
    }
}

}  // namespace

std::shared_ptr<const Programm> baueProgramm(const Loop& loop, int rate,
                                             std::shared_ptr<const Klaenge> klaenge, const InstrumentVon& saiten,
                                             const Eingefroren& eingefroren)
{
    auto programm = std::make_shared<Programm>();
    programm->ablauf = plane(loop, rate, saiten, eingefroren);
    programm->klaenge = std::move(klaenge);
    const std::size_t busse = programm->ablauf.busse.size();
    const std::size_t abgriffe = programm->ablauf.abgriffe.size();
    if (busse > 0 || abgriffe > 0)
    {
        // alles, was die Effekte an Speicher brauchen, entsteht hier — nicht
        // im Ton-Thread
        const auto block = 2 * static_cast<std::size_t>(kEffektBlock);
        auto zustand = std::make_shared<EffektZustand>();
        zustand->ketten.resize(busse);
        zustand->nachklang.resize(busse);
        zustand->stille.assign(busse, kLangeStill);
        zustand->puffer.assign(busse * block, 0.0f);
        zustand->zeiger.resize(busse);
        // Stufe 15l: die Bloecke der Abgriffe und je Baustein sein Steuersignal
        zustand->abgriffPuffer.assign(abgriffe * block, 0.0f);
        zustand->abgriffZeiger.resize(abgriffe);
        for (std::size_t a = 0; a < abgriffe; ++a) zustand->abgriffZeiger[a] = zustand->abgriffPuffer.data() + a * block;
        zustand->steuer.resize(busse);
        for (std::size_t b = 0; b < busse; ++b)
        {
            const Bus& bus = programm->ablauf.busse[b];
            zustand->ketten[b].richteEin(bus.effekte, rate, loop.bpm);
            zustand->nachklang[b] = std::llround(nachklangSekunden(bus.effekte, loop.bpm) * rate);
            zustand->zeiger[b] = zustand->puffer.data() + b * block;
            for (const int a : bus.steuer)
                zustand->steuer[b].push_back(a >= 0 ? zustand->abgriffZeiger[static_cast<std::size_t>(a)] : nullptr);
        }
        programm->effekte = std::move(zustand);
    }
    return programm;
}

void Klangerzeuger::setzeProgramm(std::shared_ptr<const Programm> programm)
{
    // was der Ton-Thread abgegeben hat, wird hier frei — nicht bei ihm
    if (m_abgegebenVoll.load(std::memory_order_acquire))
    {
        m_abgegeben.reset();
        m_abgegebenVoll.store(false, std::memory_order_release);
    }
    const std::shared_ptr<const Programm> alt = m_programm.load();
    if (alt != nullptr && programm != nullptr && alt->ablauf.schrittFrames > 0.0 &&
        programm->ablauf.schrittFrames > 0.0 && alt->ablauf.schrittFrames != programm->ablauf.schrittFrames)
    {
        // dieselbe Stelle in Schritten, in der neuen Schrittlaenge. Rechnet
        // der Ton-Thread gerade einen Block, zaehlt er danach nicht weiter
        // (`render` nimmt seine Stelle nur, wenn sie noch gilt).
        const double schritte = static_cast<double>(m_stelle.load()) / alt->ablauf.schrittFrames;
        m_stelle.store(std::llround(schritte * programm->ablauf.schrittFrames));
    }
    m_programm.store(std::move(programm));
    m_davor = alt;
}

std::int64_t Klangerzeuger::stelleImDurchlauf() const
{
    const std::shared_ptr<const Programm> p = m_programm.load();
    if (p == nullptr || p->ablauf.durchlaufFrames <= 0) return 0;
    return m_stelle.load() % p->ablauf.durchlaufFrames;
}

void Klangerzeuger::render(float* out, int frames)
{
    if (frames <= 0) return;
    const std::shared_ptr<const Programm> p = m_programm.load();
    if (p != m_imTon)
    {
        // ein neues Programm: das Gedaechtnis der Effekte geht mit, das alte
        // Programm geht zurueck an den, der es freigeben darf
        std::shared_ptr<const Programm> alt = std::move(m_imTon);
        m_imTon = p;
        if (p != nullptr) uebernimmGedaechtnis(alt.get(), *p);
        if (alt != nullptr && !m_abgegebenVoll.load(std::memory_order_acquire))
        {
            m_abgegeben = std::move(alt);
            m_abgegebenVoll.store(true, std::memory_order_release);
        }
    }
    if (p == nullptr)
    {
        std::fill(out, out + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
        return;
    }
    std::int64_t von = m_stelle.load();
    const Klaenge& klaenge = p->klaenge != nullptr ? *p->klaenge : kKeineKlaenge;
    EffektZustand* const effekte = p->effekte.get();
    if (effekte == nullptr)
        rechne(p->ablauf, klaenge, von, frames, out);
    else
    {
        // ein weiter Sprung: Echo und Hall vergessen, was sie gehoert haben (G1 b)
        const std::int64_t sprung = static_cast<std::int64_t>(p->ablauf.rate) * kSprungMs / 1000;
        if (m_erwartet >= 0 && std::abs(von - m_erwartet) > sprung)
        {
            for (EffektKette& kette : effekte->ketten) kette.leere();
            std::fill(effekte->stille.begin(), effekte->stille.end(), kLangeStill);
        }
        const std::size_t busse = effekte->ketten.size();
        for (int ab = 0; ab < frames; ab += kEffektBlock)
        {
            const int n = std::min(kEffektBlock, frames - ab);
            const auto werte = 2 * static_cast<std::ptrdiff_t>(n);
            float* const ziel = out + 2 * static_cast<std::ptrdiff_t>(ab);
            rechne(p->ablauf, klaenge, von + ab, n, ziel, effekte->zeiger.data(), effekte->abgriffZeiger.data());
            // die Effekte eines Busses; nullptr = verklungen, nichts dazuzumischen
            const auto rechneBus = [&](std::size_t b) -> const float* {
                float* const block = effekte->zeiger[b];
                const std::vector<const float*>& steuer = effekte->steuer[b];
                const bool ton =
                    std::any_of(block, block + werte, [](float wert) { return wert != 0.0f; });
                if (ton)
                    effekte->stille[b] = 0;
                // mit Steuersignal nie verklungen: seine Huelle folgt dem Signal weiter (15l)
                else if (effekte->stille[b] > effekte->nachklang[b] && steuer.empty())
                    return nullptr;
                else
                    effekte->stille[b] += n;
                effekte->ketten[b].rechne(block, n, steuer.empty() ? nullptr : steuer.data());
                return block;
            };
            // Stufe 15l (WV5 a): zuerst die genannten Kanaele, in ihrer Folge — ihre Busse in den
            // Abgriff, der Abgriff mal Fader und Loop in die Summe
            for (std::size_t a = 0; a < p->ablauf.abgriffe.size(); ++a)
            {
                const Abgriff& abgriff = p->ablauf.abgriffe[a];
                float* const summe = effekte->abgriffZeiger[a];
                for (const int b : abgriff.busse)
                    if (const float* block = rechneBus(static_cast<std::size_t>(b)))
                        for (std::ptrdiff_t i = 0; i < werte; ++i) summe[i] += block[i];
                if (abgriff.faktor != 0.0f)
                    for (std::ptrdiff_t i = 0; i < werte; ++i) ziel[i] += summe[i] * abgriff.faktor;
            }
            for (std::size_t b = 0; b < busse; ++b)
            {
                if (p->ablauf.busse[b].abgriff >= 0) continue;  // schon im Abgriff
                const float* block = rechneBus(b);
                if (block == nullptr) continue;
                // der Fader des Kanals nach den Effekten (Stufe 27e-1); bei 0 dB genau 1
                const float fader = p->ablauf.busse[b].faktor;
                for (std::ptrdiff_t i = 0; i < werte; ++i) ziel[i] += block[i] * fader;
            }
        }
    }
    m_erwartet = von + frames;
    // die Spitze des Blocks fuer die Anzeige im Editor (L6 a)
    float spitze = 0.0f;
    for (std::ptrdiff_t i = 0; i < 2 * static_cast<std::ptrdiff_t>(frames); ++i)
        spitze = std::max(spitze, std::abs(out[i]));
    float bisher = m_spitze.load();
    while (spitze > bisher && !m_spitze.compare_exchange_weak(bisher, spitze)) {}
    // weiterzaehlen — ausser jemand hat die Stelle inzwischen gesetzt
    m_stelle.compare_exchange_strong(von, von + frames);
}

}  // namespace lumi::loop
