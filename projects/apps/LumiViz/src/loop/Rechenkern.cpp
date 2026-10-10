/**
 * @file Rechenkern.cpp
 * @brief Loop: aus einem Loop und einer Stelle werden Abtastwerte (Stufe 3)
 */

#include "loop/Rechenkern.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

Ablauf plane(const Loop& loop, int rate)
{
    Ablauf ablauf;
    ablauf.rate = rate;
    ablauf.spuren.resize(loop.spuren.size());
    if (rate <= 0 || loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return ablauf;

    const double schrittFrames = rate * 60.0 / loop.bpm / loop.schritteJeSchlag;
    ablauf.schrittFrames = schrittFrames;
    const double swingFrames = loop.swing * 0.5 * schrittFrames;
    std::int64_t schrittDavor = 0;  // Schritte der Muster, die in der Abfolge schon liegen
    for (const Muster* muster : loop.ablaufMuster())
    {
        const int schritte = (muster != nullptr ? muster->takte : 1) * loop.schritteJeTakt();
        const std::size_t zeilen = muster != nullptr ? std::min(muster->zeilen.size(), loop.spuren.size()) : 0;
        for (std::size_t s = 0; s < zeilen; ++s)
        {
            const Spur& spur = loop.spuren[s];
            if (spur.stumm) continue;
            const std::vector<double>& zeile = muster->zeilen[s];
            const int anzahl = std::min(schritte, static_cast<int>(zeile.size()));
            for (int i = 0; i < anzahl; ++i)
            {
                const double wert = zeile[static_cast<std::size_t>(i)];
                if (wert <= 0.0) continue;
                const bool versetzt = (i % loop.schritteJeSchlag) % 2 == 1;
                const double stelle =
                    static_cast<double>(schrittDavor + i) * schrittFrames + (versetzt ? swingFrames : 0.0);
                const double laut = wert * spur.laut * loop.laut;
                Schlag schlag;
                schlag.frame = std::llround(stelle);
                schlag.links = static_cast<float>(laut * std::min(1.0, 1.0 - spur.pan));
                schlag.rechts = static_cast<float>(laut * std::min(1.0, 1.0 + spur.pan));
                ablauf.spuren[s].push_back(schlag);
            }
        }
        schrittDavor += schritte;
    }
    ablauf.durchlaufFrames = std::llround(static_cast<double>(schrittDavor) * schrittFrames);
    return ablauf;
}

void rechne(const Ablauf& ablauf, const std::vector<Klang>& klaenge, std::int64_t von, int frames, float* aus)
{
    if (frames <= 0) return;
    std::fill(aus, aus + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
    const std::int64_t durchlauf = ablauf.durchlaufFrames;
    const std::int64_t bis = von + frames;
    if (durchlauf <= 0 || bis <= 0) return;
    const std::int64_t ab = std::max<std::int64_t>(0, von);

    for (std::size_t s = 0; s < ablauf.spuren.size() && s < klaenge.size(); ++s)
    {
        const std::vector<Schlag>& schlaege = ablauf.spuren[s];
        const auto anzahl = static_cast<std::int64_t>(schlaege.size());
        const std::int64_t laenge = klaenge[s].frames();
        if (anzahl == 0 || laenge == 0) continue;
        const float* klang = klaenge[s].daten.data();

        // Die Schlaege der Spur ueber alle Durchlaeufe, durchgezaehlt ab 0.
        const auto beginn = [&](std::int64_t g) {
            return (g / anzahl) * durchlauf + schlaege[static_cast<std::size_t>(g % anzahl)].frame;
        };
        // Der letzte Schlag, der bei `ab` schon begonnen hat — und der davor:
        // er kann noch in seiner Schnitt-Blende klingen.
        const std::int64_t runde = ab / durchlauf;
        const auto danach = std::upper_bound(schlaege.begin(), schlaege.end(), ab - runde * durchlauf,
                                             [](std::int64_t stelle, const Schlag& h) { return stelle < h.frame; });
        std::int64_t g = runde * anzahl + (danach - schlaege.begin()) - 2;
        g = std::max<std::int64_t>(0, g);

        for (;; ++g)
        {
            const std::int64_t start = beginn(g);
            if (start >= bis) break;
            const std::int64_t naechster = beginn(g + 1);
            const std::int64_t ende = std::min(start + laenge, naechster + kSchnittFrames);
            const Schlag& schlag = schlaege[static_cast<std::size_t>(g % anzahl)];
            for (std::int64_t f = std::max(ab, start); f < std::min(bis, ende); ++f)
            {
                const float blende =
                    f < naechster ? 1.0f : 1.0f - static_cast<float>(f - naechster) / kSchnittFrames;
                const std::int64_t imKlang = f - start;
                const std::int64_t imBlock = f - von;
                aus[2 * imBlock] += klang[2 * imKlang] * schlag.links * blende;
                aus[2 * imBlock + 1] += klang[2 * imKlang + 1] * schlag.rechts * blende;
            }
        }
    }
}

}  // namespace lumi::loop
