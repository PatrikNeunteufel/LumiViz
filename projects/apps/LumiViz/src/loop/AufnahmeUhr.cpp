/**
 * @file AufnahmeUhr.cpp
 * @brief Loop: eine Aufnahme an der Wiedergabe (Stufe 15k-1, S169)
 */

#include "loop/AufnahmeUhr.hpp"

#include <QSettings>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace lumi::loop
{

namespace
{

constexpr int kMindestensGefunden = 3;
const QString kKlickName = QStringLiteral("Click (recording)");

std::int64_t median(std::vector<std::int64_t> werte)
{
    const auto mitte = werte.begin() + static_cast<std::ptrdiff_t>(werte.size() / 2);
    std::nth_element(werte.begin(), mitte, werte.end());
    return *mitte;
}

/// Ein Name fuer die Latenz in den Einstellungen: ohne `/` und `\` (sie trennten Gruppen).
QString schluesselTeil(QString name)
{
    name.replace(QLatin1Char('/'), QLatin1Char('_'));
    name.replace(QLatin1Char('\\'), QLatin1Char('_'));
    return name.isEmpty() ? QStringLiteral("default") : name;
}

QString latenzSchluessel(const QString& eingang, const QString& ausgang)
{
    return QStringLiteral("loop/latency/") + schluesselTeil(eingang) + QLatin1Char('|') + schluesselTeil(ausgang);
}

}  // namespace

std::optional<std::int64_t> uhrVersatz(const std::vector<IAudioEngine::RecordClockMark>& marken)
{
    // nur Bloecke, waehrend derer die Uhr lief — vor dem Start der Wiedergabe steht sie still
    std::vector<std::int64_t> abstaende;
    abstaende.reserve(marken.size());
    for (std::size_t i = 1; i < marken.size(); ++i)
        if (marken[i].clock > marken[i - 1].clock) abstaende.push_back(marken[i].clock - marken[i].recorded);
    if (abstaende.empty()) return std::nullopt;
    return median(std::move(abstaende));
}

std::vector<std::int64_t> klickStellen(int rate)
{
    std::vector<std::int64_t> stellen;
    for (int i = 0; i < kLatenzKlicks; ++i)
        stellen.push_back(std::llround((kKlickVorlaufS + i * kKlickAbstandS) * rate));
    return stellen;
}

std::int64_t messungFrames(int rate)
{
    return std::llround((kKlickVorlaufS + (kLatenzKlicks - 1) * kKlickAbstandS + kLatenzHoechstensS + 0.1) * rate);
}

float klickWert(std::int64_t i, int rate)
{
    const auto laenge = static_cast<std::int64_t>(kKlickS * rate);
    if (i < 0 || i >= laenge || rate <= 0) return 0.0f;
    const double t = static_cast<double>(i) / rate;
    // ein Ton von 1 kHz, der in 10 ms abklingt — kurz und scharf, gut wiederzufinden
    return static_cast<float>(0.8 * std::sin(2.0 * std::numbers::pi * 1000.0 * t) * (1.0 - t / kKlickS));
}

KlickGeber::KlickGeber(int rate) : m_rate(rate), m_klicks(klickStellen(rate)) {}

void KlickGeber::render(float* out, int frames)
{
    const std::int64_t von = m_stelle.load();
    for (int f = 0; f < frames; ++f)
    {
        float wert = 0.0f;
        for (const std::int64_t k : m_klicks) wert += klickWert(von + f - k, m_rate);
        out[2 * f] = wert;
        out[2 * f + 1] = wert;
    }
    m_stelle.store(von + frames);
}

Latenzmessung messeLatenz(const std::vector<float>& stereo, std::int64_t versatz,
                          const std::vector<std::int64_t>& klicks, int rate)
{
    Latenzmessung m;
    m.erwartet = static_cast<int>(klicks.size());
    const auto n = static_cast<std::int64_t>(stereo.size() / 2);
    if (n == 0 || rate <= 0 || klicks.empty()) return m;
    const auto betrag = [&stereo](std::int64_t k) {
        return std::max(std::abs(stereo[static_cast<std::size_t>(2 * k)]), std::abs(stereo[static_cast<std::size_t>(2 * k + 1)]));
    };
    // das Rauschen: die Spitze vor dem ersten Klick (dort spielt noch nichts)
    float rauschen = 0.0f;
    for (std::int64_t k = 0; k < std::min(n, klicks.front() - versatz); ++k) rauschen = std::max(rauschen, betrag(k));
    const auto fenster = static_cast<std::int64_t>(kLatenzHoechstensS * rate);
    std::vector<std::int64_t> latenzen;
    for (const std::int64_t klick : klicks)
    {
        // gesucht wird ab der Stelle, an der der Klick ohne Latenz ankaeme
        const std::int64_t von = std::max<std::int64_t>(0, klick - versatz);
        const std::int64_t bis = std::min(n, von + fenster);
        if (bis <= von) continue;
        float spitze = 0.0f;
        for (std::int64_t k = von; k < bis; ++k) spitze = std::max(spitze, betrag(k));
        if (spitze < 0.01f || spitze < 4.0f * rauschen) continue;
        for (std::int64_t k = von; k < bis; ++k)
            if (betrag(k) >= 0.5f * spitze)
            {
                latenzen.push_back(versatz + k - klick);
                break;
            }
    }
    m.gefunden = static_cast<int>(latenzen.size());
    if (m.gefunden < kMindestensGefunden) return m;
    const auto [kleinste, groesste] = std::minmax_element(latenzen.begin(), latenzen.end());
    m.streuungMs = static_cast<double>(*groesste - *kleinste) * 1000.0 / rate;
    m.ms = static_cast<double>(median(latenzen)) * 1000.0 / rate;
    return m;
}

std::optional<double> gemerkteLatenzMs(const QString& eingang, const QString& ausgang)
{
    const QVariant wert = QSettings().value(latenzSchluessel(eingang, ausgang));
    if (!wert.isValid()) return std::nullopt;
    bool ok = false;
    const double ms = wert.toDouble(&ok);
    return ok ? std::optional<double>(ms) : std::nullopt;
}

void merkeLatenzMs(const QString& eingang, const QString& ausgang, double ms)
{
    QSettings().setValue(latenzSchluessel(eingang, ausgang), ms);
}

std::vector<float> schneideTake(const std::vector<float>& stereo, std::int64_t versatz, std::int64_t latenz,
                                std::int64_t vonUhr, std::int64_t bisUhr)
{
    if (bisUhr <= vonUhr) return {};
    const auto n = static_cast<std::int64_t>(stereo.size() / 2);
    // Abtastwert k gehoert zur gespielten Stelle k + versatz − latenz
    const std::int64_t erster = vonUhr - versatz + latenz;
    std::vector<float> take(static_cast<std::size_t>(2 * (bisUhr - vonUhr)), 0.0f);
    for (std::int64_t i = 0; i < bisUhr - vonUhr; ++i)
    {
        const std::int64_t k = erster + i;
        if (k < 0 || k >= n) continue;
        take[static_cast<std::size_t>(2 * i)] = stereo[static_cast<std::size_t>(2 * k)];
        take[static_cast<std::size_t>(2 * i + 1)] = stereo[static_cast<std::size_t>(2 * k + 1)];
    }
    return take;
}

Aufnahmefassung aufnahmeFassung(const Loop& loop, const WaveAufnahmePlan& plan)
{
    Aufnahmefassung f;
    f.loop = loop;
    const int start = std::max(0, plan.startTakt);
    const int einzaehlen = std::clamp(plan.einzaehlen, 0, 2);
    // reicht das Einzaehlen vor Takt 0, ruecken alle Clips nach hinten
    const int verschoben = std::max(0, einzaehlen - start);
    for (Kanal& kanal : f.loop.kanaele)
        for (LoopClip& clip : kanal.clips) clip.startTakt += verschoben;
    f.aufnahmeTakt = start + verschoben;
    f.abspielTakt = f.aufnahmeTakt - einzaehlen;

    const int durchlauf = f.loop.durchlaufTakte();
    const int aufnahme = plan.klick ? (plan.takte > 0 ? plan.takte : std::max(1, durchlauf - f.aufnahmeTakt)) : 0;
    const int klickTakte = einzaehlen + aufnahme;
    if (klickTakte <= 0) return f;
    // der Klick: auf jedem Schlag, die Eins lauter
    Muster klick;
    klick.name = kKlickName;
    for (int n = 2; f.loop.findeMuster(klick.name) != nullptr; ++n) klick.name = kKlickName + QStringLiteral(" %1").arg(n);
    klick.takte = 1;
    klick.spuren = {{QStringLiteral("Click"), QStringLiteral("kit:hat_closed"), 1.0, 0.0, false}};
    std::vector<double> zeile(static_cast<std::size_t>(f.loop.schritteJeTakt()), 0.0);
    for (int schlag = 0; schlag < f.loop.schlaegeJeTakt; ++schlag)
        zeile[static_cast<std::size_t>(schlag * f.loop.schritteJeSchlag)] = schlag == 0 ? 1.0 : 0.6;
    klick.zeilen = {zeile};
    f.loop.muster.push_back(klick);
    Kanal kanal;
    kanal.name = klick.name;
    kanal.clips = {LoopClip(klick.name, f.abspielTakt, klickTakte)};
    f.loop.kanaele.push_back(std::move(kanal));
    f.loop.richte();
    return f;
}

double taktFrames(const Loop& loop, int rate)
{
    if (loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return 0.0;
    const double schrittFrames = rate * 60.0 / loop.bpm / loop.schritteJeSchlag;
    return schrittFrames * loop.schritteJeTakt();
}

}  // namespace lumi::loop
