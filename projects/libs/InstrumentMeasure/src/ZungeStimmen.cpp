/**
 * @file ZungeStimmen.cpp
 * @brief Messen am Kern `freeReed` (Herleitung im Kopf von `ZungeStimmen.hpp`)
 */
#include "InstrumentMeasure/ZungeStimmen.hpp"

#include "InstrumentMeasure/Bausteine.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

namespace lumi::messung
{

using kern::zunge::Chor;
using kern::zunge::ZungenAnschlag;
using kern::zunge::ZungenMessung;
using kern::zunge::ZungenSpiel;

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

}  // namespace lumi::messung
