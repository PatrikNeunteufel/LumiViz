/**
 * @file Rechenkern.cpp
 * @brief Loop: aus einem Loop und einer Stelle werden Abtastwerte
 */

#include "loop/Rechenkern.hpp"

#include <QHash>

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

namespace lumi::loop
{

Ablauf plane(const Loop& loop, int rate)
{
    Ablauf ablauf;
    ablauf.rate = rate;
    ablauf.klangNamen = loop.klangNamen();
    ablauf.bpm = loop.bpm;
    if (rate <= 0 || loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return ablauf;

    const double schrittFrames = rate * 60.0 / loop.bpm / loop.schritteJeSchlag;
    ablauf.schrittFrames = schrittFrames;
    const double swingFrames = loop.swing * 0.5 * schrittFrames;
    const int jeTakt = loop.schritteJeTakt();
    QHash<QString, int> klangVon;
    for (int i = 0; i < ablauf.klangNamen.size(); ++i) klangVon.insert(ablauf.klangNamen[i], i);

    for (std::size_t kanalNr = 0; kanalNr < loop.kanaele.size(); ++kanalNr)
    {
        const Kanal& kanal = loop.kanaele[kanalNr];
        if (!kanal.an) continue;
        // je Kanal: (Muster, Spur) -> Stimme
        std::map<std::pair<const Muster*, std::size_t>, std::size_t> stimmeVon;
        // je Kanal: (Instrument-Muster, Ton) -> Stimme. Jeder Ton — Tonhoehe
        // und Dauer — ist eine eigene Stimme: ein Akkord klingt zusammen, und
        // nur derselbe Ton schneidet sich selbst ab.
        std::map<std::pair<const Muster*, int>, std::size_t> tonStimmeVon;
        for (const LoopClip& clip : kanal.clips)
        {
            const Muster* muster = loop.findeMuster(clip.muster);
            if (muster == nullptr) continue;
            // ein Clip mit Effekten: seine Schlaege gehen in einen eigenen Bus
            int bus = -1;
            if (clip.effekte.einerAn())
            {
                bus = static_cast<int>(ablauf.busse.size());
                ablauf.busse.push_back({static_cast<int>(kanalNr), clip.startTakt, clip.effekte});
            }
            if (muster->art == MusterArt::Instrument)
            {
                if (muster->stumm) continue;
                const double lautMuster = muster->laut * loop.laut;
                for (int runde = 0; runde < clip.wiederholungen; ++runde)
                {
                    const std::int64_t schrittDavor =
                        static_cast<std::int64_t>(clip.startTakt + runde * muster->takte) * jeTakt;
                    for (const Note& note : muster->noten)
                    {
                        const int klang = klangVon.value(
                            tonKlang(muster->instrument, note.hoehe, loop.notenMs(note)), -1);
                        const auto [ort, neu] = tonStimmeVon.try_emplace({muster, klang}, ablauf.stimmen.size());
                        if (neu)
                        {
                            Stimme stimme;
                            stimme.klang = klang;
                            ablauf.stimmen.push_back(std::move(stimme));
                        }
                        const bool versetzt = (note.schritt % loop.schritteJeSchlag) % 2 == 1;
                        const double stelle = static_cast<double>(schrittDavor + note.schritt) * schrittFrames +
                                              (versetzt ? swingFrames : 0.0);
                        const double laut = note.laut * lautMuster;
                        Schlag schlag;
                        schlag.frame = std::llround(stelle);
                        schlag.links = static_cast<float>(laut * std::min(1.0, 1.0 - muster->pan));
                        schlag.rechts = static_cast<float>(laut * std::min(1.0, 1.0 + muster->pan));
                        schlag.bus = bus;
                        ablauf.stimmen[ort->second].schlaege.push_back(schlag);
                    }
                }
                continue;
            }
            const int schritte = muster->takte * jeTakt;
            const std::size_t zeilen = std::min(muster->zeilen.size(), muster->spuren.size());
            for (int runde = 0; runde < clip.wiederholungen; ++runde)
            {
                const std::int64_t schrittDavor =
                    static_cast<std::int64_t>(clip.startTakt + runde * muster->takte) * jeTakt;
                for (std::size_t s = 0; s < zeilen; ++s)
                {
                    const Spur& spur = muster->spuren[s];
                    if (spur.stumm) continue;
                    const std::vector<double>& zeile = muster->zeilen[s];
                    const int anzahl = std::min(schritte, static_cast<int>(zeile.size()));
                    for (int i = 0; i < anzahl; ++i)
                    {
                        const double wert = zeile[static_cast<std::size_t>(i)];
                        if (wert <= 0.0) continue;
                        const auto [ort, neu] = stimmeVon.try_emplace({muster, s}, ablauf.stimmen.size());
                        if (neu)
                        {
                            Stimme stimme;
                            stimme.klang = static_cast<int>(ablauf.klangNamen.indexOf(spur.klang));
                            ablauf.stimmen.push_back(std::move(stimme));
                        }
                        const bool versetzt = (i % loop.schritteJeSchlag) % 2 == 1;
                        const double stelle = static_cast<double>(schrittDavor + i) * schrittFrames +
                                              (versetzt ? swingFrames : 0.0);
                        const double laut = wert * spur.laut * loop.laut;
                        Schlag schlag;
                        schlag.frame = std::llround(stelle);
                        schlag.links = static_cast<float>(laut * std::min(1.0, 1.0 - spur.pan));
                        schlag.rechts = static_cast<float>(laut * std::min(1.0, 1.0 + spur.pan));
                        schlag.bus = bus;
                        ablauf.stimmen[ort->second].schlaege.push_back(schlag);
                    }
                }
            }
        }
    }
    ablauf.durchlaufFrames = std::llround(static_cast<double>(loop.durchlaufSchritte()) * schrittFrames);
    return ablauf;
}

void rechne(const Ablauf& ablauf, const std::vector<Klang>& klaenge, std::int64_t von, int frames, float* aus,
            float* const* busAus)
{
    if (frames <= 0) return;
    std::fill(aus, aus + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
    if (busAus != nullptr)
        for (std::size_t b = 0; b < ablauf.busse.size(); ++b)
            std::fill(busAus[b], busAus[b] + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
    const std::int64_t durchlauf = ablauf.durchlaufFrames;
    const std::int64_t bis = von + frames;
    if (durchlauf <= 0 || bis <= 0) return;
    const std::int64_t ab = std::max<std::int64_t>(0, von);

    for (const Stimme& stimme : ablauf.stimmen)
    {
        const std::vector<Schlag>& schlaege = stimme.schlaege;
        const auto anzahl = static_cast<std::int64_t>(schlaege.size());
        if (anzahl == 0 || stimme.klang < 0 || static_cast<std::size_t>(stimme.klang) >= klaenge.size()) continue;
        const Klang& klangDerStimme = klaenge[static_cast<std::size_t>(stimme.klang)];
        const std::int64_t laenge = klangDerStimme.frames();
        if (laenge == 0) continue;
        const float* klang = klangDerStimme.daten.data();

        // Die Schlaege der Stimme ueber alle Durchlaeufe, durchgezaehlt ab 0.
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
            // auch die Schnitt-Blende eines Schlags bleibt in seinem Bus
            float* const ziel = busAus != nullptr && schlag.bus >= 0 ? busAus[schlag.bus] : aus;
            for (std::int64_t f = std::max(ab, start); f < std::min(bis, ende); ++f)
            {
                const float blende =
                    f < naechster ? 1.0f : 1.0f - static_cast<float>(f - naechster) / kSchnittFrames;
                const std::int64_t imKlang = f - start;
                const std::int64_t imBlock = f - von;
                ziel[2 * imBlock] += klang[2 * imKlang] * schlag.links * blende;
                ziel[2 * imBlock + 1] += klang[2 * imKlang + 1] * schlag.rechts * blende;
            }
        }
    }
}

}  // namespace lumi::loop
