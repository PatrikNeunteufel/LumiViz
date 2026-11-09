/**
 * @file Trommel.cpp
 * @brief Klasse 5: der Schlag auf eine Trommel (Herleitung im Kopf von `Trommel.hpp`)
 */
#include "klassen/05_membran/Trommel.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic::trommel
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr int kBlock = 32;  // die Energie fuer das Gleiten alle 32 Takte (Marogna et al.: ×32 unterabgetastet)

struct Zufall
{
    std::uint64_t s = 0x9E3779B97F4A7C15ull;
    double naechste()
    {
        s ^= s >> 12;
        s ^= s << 25;
        s ^= s >> 27;
        return static_cast<double>((s * 0x2545F4914F6CDD1Dull) >> 11) / 9007199254740992.0 - 0.5;
    }
};

/// Ein Bandpass zweiter Ordnung (RBJ, Guete q).
struct Band
{
    double b0 = 0, b2 = 0, a1 = 0, a2 = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    void setze(double f, double q, double fs)
    {
        const double w = 2.0 * kPi * f / fs, al = std::sin(w) / (2.0 * q), a0 = 1.0 + al;
        b0 = al / a0;
        b2 = -al / a0;
        a1 = -2.0 * std::cos(w) / a0;
        a2 = (1.0 - al) / a0;
    }
    double schritt(double x)
    {
        const double y = b0 * x + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }
};

}  // namespace

void bereite(TrommelSpiel& s)
{
    s.moden = std::make_shared<const std::vector<FellMode>>(fellModen(s.felle, s.kessel, s.bisHz));
    s.referenzHz = 0.0;
    for (const FellMode& m : *s.moden)
        if (m.m == 1 && m.n == 1)
        {
            s.referenzHz = m.hz;
            break;
        }
}

void rechneTrommelTon(const TrommelSpiel& s, const TrommelAnschlag& a, std::vector<float>& mono, std::int64_t frames, TrommelMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0 || !s.moden || s.moden->empty()) return;
    const double fs = a.rate;
    const double e = std::clamp(a.staerke, 0.0, 1.0);
    const double tempo = s.tempoPp * std::pow(s.tempoFf / s.tempoPp, e);
    // die Pauke: alle Moden mit dem Verhaeltnis der Note zur (1,1) der Vorgabe (die Luftlast nicht neu geloest — offen)
    double faktor = 1.0;
    if (s.stimmbar && s.referenzHz > 0.0 && a.note > 0)
        faktor = 440.0 * std::exp2((std::clamp(a.note, s.note, s.noteMax) - 69) / 12.0) / s.referenzHz;
    double rel = s.schlagstelleRel;
    if (a.schlag == Schlag::Mitte) rel = 0.0;
    if (a.schlag == Schlag::Rand || a.schlag == Schlag::Randschlag) rel = 0.85;
    const double daempfung = a.schlag == Schlag::Gedaempft ? 6.0 : 1.0;
    std::vector<Mode> moden;
    std::vector<double> masse, omega0, resoMitte;
    for (const FellMode& fm : *s.moden)
    {
        const double phi = formAn(fm, rel, 0);
        Mode m = modeAus(fm.hz * faktor, fm.t60S / daempfung, fm.masseKg, phi, phi, fm.abstrahlung, Ausgang::Beschleunigung);
        // (−i)^m: die Phase des Fernfelds je Knotendurchmesser
        const Komplex dreh = std::pow(Komplex(0.0, -1.0), fm.m);
        m.aus *= dreh;
        moden.push_back(m);
        masse.push_back(fm.masseKg);
        omega0.push_back(2.0 * kPi * fm.hz * faktor);
        resoMitte.push_back(fm.felle > 1 ? formAn(fm, 0.0, 1) : 0.0);
    }
    ModalBank bank(moden, fs);
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    // die Schlaege: einer, oder ein Wirbel (abwechselnde Haende ±10 %, offen), je Schlag `prellen` Aufschlaege
    struct Treffer
    {
        std::int64_t ab;
        double tempo;
    };
    std::vector<Treffer> treffer;
    const bool reifenAllein = a.schlag == Schlag::Reifen;
    if (a.wirbelS > 0.0)
    {
        const double abstand = 1.0 / s.wirbelHz;
        int k = 0;
        for (double t = 0.0; t < a.wirbelS; t += abstand, ++k)
            for (int p = 0; p < std::max(1, s.prellen); ++p)
                treffer.push_back({static_cast<std::int64_t>(std::llround((t + p * abstand / (s.prellen + 1)) * fs)), tempo * (k % 2 ? 0.9 : 1.0) * std::pow(0.7, p)});
    }
    else
        for (int p = 0; p < std::max(1, s.prellen); ++p)
            treffer.push_back({static_cast<std::int64_t>(std::llround(p * 0.012 * fs)), tempo * std::pow(0.7, p)});
    // die Energie des Fells fuer das Gleiten: Σ ½·M·ω²·|z|²
    const double r = s.felle.front().radiusM, t0 = s.felle.front().spannungNm * faktor * faktor;
    const double kGleit = s.youngH / (2.0 * kPi * r * r * (1.0 - s.nu * s.nu) * t0);
    Zufall z;
    Band teppichBand, hochBand, klickBand;
    teppichBand.setze(4500.0, 0.6, fs);
    hochBand.setze(std::min(0.45 * fs, s.bisHz * 1.4), 0.8, fs);
    klickBand.setze(2500.0, 1.5, fs);
    double hochHuelle = 0.0, klickHuelle = 0.0, resoVor = 0.0, teppichHuelle = 0.0;
    std::vector<Komplex> pol(moden.size());
    double erstesGleiten = 0.0, spaetesGleiten = 0.0;
    std::size_t naechster = 0;
    for (std::int64_t n = 0; n < frames;)
    {
        if (naechster < treffer.size() && n >= treffer[naechster].ab)
        {
            std::vector<double> stoss;
            StossMessung sm;
            if (!reifenAllein)
                sm = stosse(bank, s.schlaegel, treffer[naechster].tempo, stoss, 8, 0.03, 0.001);
            else
            {
                // Cross-Stick: das Fell nur schwach (der Stock liegt auf, offen)
                sm = stosse(bank, s.schlaegel, 0.15 * treffer[naechster].tempo, stoss, 8, 0.03, 0.001);
            }
            for (std::size_t i = 0; i < stoss.size() && n + static_cast<std::int64_t>(i) < frames; ++i) summe[static_cast<std::size_t>(n) + i] += stoss[i];
            if (naechster == 0 && messung != nullptr)
            {
                messung->kontaktS = sm.kontaktS;
                messung->kraftSpitzeN = sm.kraftSpitzeN;
            }
            // die Pegel der Rauschanteile relativ zum normierten Ausgang (`ausgangBezug`): ff-Schlag = 1
            hochHuelle += s.hochPegel * s.ausgangBezug * sm.kraftSpitzeN / 100.0;
            if (a.schlag == Schlag::Randschlag || a.schlag == Schlag::Reifen)
                klickHuelle += s.klickPegel * s.ausgangBezug * treffer[naechster].tempo / s.tempoFf;
            n += std::max<std::int64_t>(1, sm.frames);
            ++naechster;
            continue;
        }
        const std::int64_t ende = std::min<std::int64_t>(frames, std::min<std::int64_t>(n + kBlock, naechster < treffer.size() ? treffer[naechster].ab : frames));
        // das Gleiten: die Energie jetzt, die Pole fuer den Block
        double energie = 0.0;
        auto& bm = bank.moden();
        for (std::size_t i = 0; i < bm.size(); ++i) energie += 0.5 * masse[i] * omega0[i] * omega0[i] * std::norm(bm[i].zustand);
        const double gleit = std::sqrt(1.0 + kGleit * energie / t0);  // √(1 + T_qs/T₀)
        const double t = static_cast<double>(n) / fs;
        const double gliss = std::exp2(a.glissandoCent / 1200.0 * std::min(1.0, t / std::max(1e-3, static_cast<double>(frames) / fs)));
        for (std::size_t i = 0; i < bm.size(); ++i)
            pol[i] = std::exp(Komplex(bm[i].lambda.real(), bm[i].lambda.imag() * gleit * gliss) / fs);
        if (messung != nullptr && !bm.empty())
        {
            if (t >= 0.01 && erstesGleiten == 0.0) erstesGleiten = gleit;
            if (t >= 1.0 && spaetesGleiten == 0.0) spaetesGleiten = gleit;
        }
        for (; n < ende; ++n)
        {
            double y = 0.0, reso = 0.0;
            // ausgeschrieben in reellen Zahlen (clang ruft fuer Komplex·Komplex ohne fast-math die NaN-sichere
            // Bibliothek — viermal langsamer; fuer endliche Werte dieselbe Formel)
            for (std::size_t i = 0; i < bm.size(); ++i)
            {
                const double zr = bm[i].zustand.real(), zi = bm[i].zustand.imag();
                const double pr = pol[i].real(), pi = pol[i].imag();
                const double nr = zr * pr - zi * pi, ni = zr * pi + zi * pr;
                bm[i].zustand = Komplex(nr, ni);
                y += bm[i].aus.real() * ni + bm[i].aus.imag() * nr;
                reso += resoMitte[i] * ni;
            }
            // der Teppich: das Resonanzfell entfernt sich von den Draehten → sie schlagen an (Rauschen, offen)
            if (s.teppich && !a.ohneTeppich)
            {
                const double v = (reso - resoVor) * fs;
                teppichHuelle = std::max(teppichHuelle * 0.999, std::max(0.0, -v));
                y += s.teppichPegel * s.ausgangBezug * teppichHuelle / tempo * teppichBand.schritt(z.naechste());
            }
            resoVor = reso;
            if (hochHuelle > 0.0)
            {
                y += hochHuelle * hochBand.schritt(z.naechste());
                hochHuelle *= std::exp(-1.0 / (0.03 * fs));  // 30 ms (offen)
            }
            if (klickHuelle > 0.0)
            {
                y += klickHuelle * klickBand.schritt(z.naechste());
                klickHuelle *= std::exp(-1.0 / (0.004 * fs));  // 4 ms (offen)
            }
            summe[static_cast<std::size_t>(n)] += y;
        }
    }
    if (messung != nullptr && erstesGleiten > 0.0) messung->gleitenCent = 1200.0 * std::log2(erstesGleiten / std::max(1.0, spaetesGleiten));
    const auto blende = std::min<std::int64_t>(frames, std::max<std::int64_t>(1, std::llround(0.005 * fs)));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double w = summe[static_cast<std::size_t>(i)] / s.ausgangBezug;
        const std::int64_t rest = frames - 1 - i;
        if (rest < blende) w *= static_cast<double>(rest) / static_cast<double>(blende);
        mono[static_cast<std::size_t>(i)] = static_cast<float>(w);
    }
}

}  // namespace ic::trommel
