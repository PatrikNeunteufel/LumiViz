/**
 * @file ZungeStimmen.cpp
 * @brief Messen am Kern `freeReed` (Herleitung im Kopf von `ZungeStimmen.hpp`)
 */
#include "InstrumentMeasure/ZungeStimmen.hpp"

#include "InstrumentCore/Pool.hpp"
#include "InstrumentMeasure/Bausteine.hpp"
#include "InstrumentModels/zunge/ZungeKern.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <utility>

namespace lumi::messung
{

using kern::zunge::Chor;
using kern::zunge::ZungenAnschlag;
using kern::zunge::ZungenMessung;
using kern::zunge::ZungenSpiel;

namespace
{

/// wie IC `Zunge.cpp` (dieselben Rechenschritte, damit die Stimmung Bit fuer Bit gleich bleibt)
double hzVon(double note)
{
    return 440.0 * std::exp2((note - 69.0) / 12.0);
}

bool enthaelt(const std::vector<std::string>& namen, std::string_view name)
{
    return std::find(namen.begin(), namen.end(), name) != namen.end();
}

kern::Kurve gestimmt(std::string herkunft)
{
    kern::Kurve k;
    k.belegkraft = kern::Belegkraft::Rechnung;
    k.herkunft = std::move(herkunft);
    k.quellen = {"lumiviz"};
    return k;
}

}  // namespace

ZungenTon messeZungenTon(std::span<const double> ton, double haltenS, int rate, double amplitudeRel)
{
    // IC Zunge.cpp:101–122 (die Zeitpunkte wie dort: `halten` in Frames, gerundet)
    ZungenTon m;
    m.amplitudeRel = amplitudeRel;
    const auto frames = static_cast<std::int64_t>(ton.size());
    const auto haltenF = static_cast<std::size_t>(std::min<std::int64_t>(frames, std::llround(haltenS * rate)));
    const auto ab = static_cast<std::size_t>(0.5 * haltenS * rate);
    if (haltenF > ab + 4800)
    {
        const std::vector<double> stueck(ton.begin() + static_cast<std::ptrdiff_t>(ab), ton.begin() + static_cast<std::ptrdiff_t>(haltenF));
        const Tonhoehe th = tonhoehe(stueck, rate, 30.0, 3000.0);
        std::vector<double> hz;
        for (const double x : th.hz)
            if (x > 0.0) hz.push_back(x);
        if (hz.size() > th.hz.size() / 2)
        {
            std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
            m.spielHz = hz[hz.size() / 2];
            m.schwingt = true;
        }
    }
    const std::vector<double> alles(ton.begin(), ton.begin() + static_cast<std::ptrdiff_t>(haltenF));
    m.einschwingS = einschwingen(alles, rate);
    return m;
}

ZungenTon messeEineZunge(const ZungenSpiel& spiel, double f0, double hz, const ZungenAnschlag& a, std::int64_t frames, std::vector<double>* ton)
{
    std::vector<double> t;
    ZungenMessung k;
    kern::zunge::rechneEineZunge(spiel, f0, hz, a, t, frames, &k);
    const ZungenTon m = messeZungenTon(t, a.haltenS, a.rate, k.amplitudeRel);
    if (ton != nullptr) *ton = std::move(t);
    return m;
}

ZungenTon messeErsteZunge(const ZungenSpiel& spiel, const ZungenAnschlag& a, std::int64_t frames)
{
    const bool bass = spiel.bassAb > 0 && a.note < spiel.bassAb;
    const std::vector<Chor>& choere = bass ? spiel.bassChoere : spiel.choere;
    if (choere.empty() || frames <= 0 || a.rate <= 0) return ZungenTon{};
    const kern::zunge::ZungeDesChors z = kern::zunge::zungeDesChors(spiel, choere.front(), a.note);
    return messeEineZunge(spiel, z.f0, z.hz, a, frames);
}

ZungenStimmung zungenStimmung(const ZungenSpiel& s)
{
    // IC Zunge.cpp `stimme` (Zeilen 163–244), die Messung ueber `messeEineZunge`
    const std::size_t anzahl = static_cast<std::size_t>(std::max(0, s.noteMax - s.noteMin + 1));
    std::vector<double> f0(anzahl, 0.0), unten(anzahl, 0.0), oben(anzahl, 0.0);
    ZungenSpiel t = s;
    t.druckUntenJeNote = {};
    t.druckObenJeNote = {};
    kern::parallelJe(anzahl, [&](std::size_t i) {
        const int note = s.noteMin + static_cast<int>(i);
        const double f = hzVon(note);
        const auto spielt = [&](double druck, double x) {
            ZungenAnschlag a;
            a.note = note;
            a.haltenS = 0.5;
            a.druckPa = druck;
            return messeEineZunge(t, x, f, a, static_cast<std::int64_t>(0.5 * a.rate));
        };
        // das Fenster: Raster 2^(1/4) von 100 Pa bis 6 kPa, die Zunge spricht an (Amplitude ≥ 2 %, Tonhoehe ±100 Cent
        // an f₀), der laengste Lauf ohne die Raender
        double x = 1.02 * f;
        const double schritt = std::exp2(0.25);
        std::pair<double, double> best{0.0, 0.0};
        int bestLauf = 0, lauf = 0;
        double anfang = 0.0;
        for (double p = 100.0; p <= 6000.0; p *= schritt)
        {
            const ZungenTon m = spielt(p, x);
            if (m.schwingt && m.amplitudeRel >= 0.02 && std::abs(1200.0 * std::log2(m.spielHz / x)) < 100.0)
            {
                if (lauf++ == 0) anfang = p;
                if (lauf > bestLauf)
                {
                    bestLauf = lauf;
                    best = {anfang, p};
                }
            }
            else
                lauf = 0;
        }
        if (best.second >= best.first * schritt * schritt * schritt * 0.999)
        {
            best.first *= schritt;
            best.second /= schritt;
        }
        if (best.first > 0.0)
        {
            const double mitte = std::sqrt(best.first * best.second);
            for (int runde = 0; runde < 6; ++runde)
            {
                const ZungenTon m = spielt(mitte, x);
                if (!m.schwingt) break;
                const double fehler = 1200.0 * std::log2(m.spielHz / f);
                if (std::abs(fehler) < 1.0 || std::abs(fehler) > 600.0) break;
                x *= f / m.spielHz;
            }
        }
        f0[i] = x;
        unten[i] = best.first;
        oben[i] = best.second;
    });
    ZungenStimmung st{gestimmt("tuned through the model: f0 of the reed so that it plays the key in the middle of its window (stimme)"),
                      gestimmt("tuned through the model: the lower edge of the pressure at which the reed speaks (amplitude >= 2 %, "
                               "without the edge; stimme)"),
                      gestimmt("tuned through the model: the upper edge of this window (stimme)")};
    for (std::size_t i = 0; i < anzahl; ++i)
    {
        const int note = s.noteMin + static_cast<int>(i);
        st.eigenfrequenz.punkte.push_back({note, f0[i]});
        if (unten[i] > 0.0)
        {
            st.druckUnten.punkte.push_back({note, unten[i]});
            st.druckOben.punkte.push_back({note, oben[i]});
        }
    }
    return st;
}

Stimmer zungeStimmer()
{
    using namespace kern::zunge;
    return Stimmer{std::string(kKernName), [](const kern::Fabrik&, const kern::Werte& werte, const std::vector<std::string>& namen) {
                       GestimmteWerte aus;
                       if (!enthaelt(namen, kEigenfrequenz) && !enthaelt(namen, kDruckUnten) && !enthaelt(namen, kDruckOben)) return aus;
                       // die drei entstehen zusammen (f₀ in der Mitte des Fensters)
                       const ZungenStimmung st = zungenStimmung(spielAus(werte));
                       if (enthaelt(namen, kEigenfrequenz)) aus.emplace_back(std::string(kEigenfrequenz), st.eigenfrequenz);
                       if (enthaelt(namen, kDruckUnten)) aus.emplace_back(std::string(kDruckUnten), st.druckUnten);
                       if (enthaelt(namen, kDruckOben)) aus.emplace_back(std::string(kDruckOben), st.druckOben);
                       return aus;
                   }};
}

}  // namespace lumi::messung
