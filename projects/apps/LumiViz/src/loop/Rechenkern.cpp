/**
 * @file Rechenkern.cpp
 * @brief Loop: aus einem Loop und einer Stelle werden Abtastwerte
 */

#include "loop/Rechenkern.hpp"

#include "loop/HammerSaite.hpp"  // anschlagRest (Stufe 31b)
#include "loop/Pedalplan.hpp"
#include "loop/Saitenplan.hpp"
#include "loop/Stueckweise.hpp"
#include "loop/WaveKlang.hpp"

#include <QHash>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <utility>

namespace lumi::loop
{

namespace
{

/// Die Stimme eines Wave-Musters in `stimmeVon`: keine Spur hat diese Nummer.
constexpr std::size_t kWaveStimme = std::numeric_limits<std::size_t>::max();

/// Die Kurve einer Spur ueber die Wiederholungen eines Clips (Stufe 28c-1): je
/// Durchlauf am Anfang der erste Wert, die Punkte, bis zur Naht der letzte.
Stuecke ueberDieWiederholungen(const std::vector<SpurPunkt>& spur, double clipSchritt, double schritte, int mal,
                               double schrittFrames)
{
    Stuecke stuecke;
    if (spur.empty()) return stuecke;
    const auto frame = [&](double schritt) { return std::llround((clipSchritt + schritt) * schrittFrames); };
    for (int r = 0; r < mal; ++r)
    {
        const double davor = r * schritte;
        stuecke.emplace_back(frame(davor), spur.front().wert);
        for (const SpurPunkt& p : spur) stuecke.emplace_back(frame(davor + p.stelle), p.wert);
        stuecke.emplace_back(frame(davor + schritte), spur.back().wert);
    }
    return stuecke;
}

/// Die Kanaele, deren Steuersignal die Bausteine im Kanal `kanal` bekommen (Stufe 15l): nur
/// wirksame — der Clip nicht eingefroren (seine Effekte stecken in der Wellenform), seine
/// Effekte wirken, der Baustein ist an —, ohne Kreis (`steuerKanal`).
std::vector<int> genannteKanaele(const Loop& loop, std::size_t kanal, const Eingefroren& eingefroren)
{
    std::vector<int> ziele;
    for (const LoopClip& clip : loop.kanaele[kanal].clips)
    {
        if (!clip.effekteWirken() || eingefroren.contains({static_cast<int>(kanal), clip.startTakt})) continue;
        for (const ClipBaustein& baustein : clip.effekte.reihe)
            if (bausteinAn(baustein))
                if (const int ziel = steuerKanal(loop, static_cast<int>(kanal), baustein); ziel >= 0) ziele.push_back(ziel);
    }
    return ziele;
}

/// Die Abgriffe (Stufe 15l, WV5 a): ausgehend von den eingeschalteten Kanaelen jeder Kanal,
/// den ein wirksamer Baustein nennt, auch mittelbar — je einer, jeder hinter denen, deren
/// Signal er selbst braucht (ohne Kreis ist das immer moeglich). `abgriffVon` bekommt je
/// Kanal den Index oder -1.
std::vector<Abgriff> planeAbgriffe(const Loop& loop, const Eingefroren& eingefroren, std::vector<int>& abgriffVon)
{
    std::vector<Abgriff> abgriffe;
    abgriffVon.assign(loop.kanaele.size(), -1);
    std::vector<bool> besucht(loop.kanaele.size(), false);
    const auto besuche = [&](const auto& selbst, std::size_t kanal) -> void {
        for (const int ziel : genannteKanaele(loop, kanal, eingefroren))
        {
            const auto z = static_cast<std::size_t>(ziel);
            if (besucht[z]) continue;
            besucht[z] = true;
            selbst(selbst, z);  // zuerst, was der genannte Kanal selbst braucht
            abgriffVon[z] = static_cast<int>(abgriffe.size());
            abgriffe.push_back(Abgriff{ziel, 0.0f, {}});
        }
    };
    for (std::size_t k = 0; k < loop.kanaele.size(); ++k)
        if (loop.kanaele[k].an) besuche(besuche, k);
    return abgriffe;
}

}  // namespace

double Ausdruck::wert(std::int64_t frame) const
{
    const std::int64_t x = std::min(frame, ende);
    StueckLeser m(muster);
    StueckLeser c(clip);
    return m.bei(x) * c.bei(x);
}

Ablauf plane(const Loop& loop, int rate, const InstrumentVon& saiten, const Eingefroren& eingefroren)
{
    Ablauf ablauf;
    ablauf.rate = rate;
    ablauf.klangNamen = loop.klangNamen(saiten, eingefroren);
    ablauf.bpm = loop.bpm;
    if (rate <= 0 || loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return ablauf;

    const double schrittFrames = rate * 60.0 / loop.bpm / loop.schritteJeSchlag;
    ablauf.schrittFrames = schrittFrames;
    // Stufe 15a (RS1-1 a): der Swing je Muster — der des Loops mit dem Offset des Musters
    const auto swingFramesVon = [&](const Muster& m) { return swingVon(loop, m) * 0.5 * schrittFrames; };
    const int jeTakt = loop.schritteJeTakt();
    QHash<QString, int> klangVon;
    for (int i = 0; i < ablauf.klangNamen.size(); ++i) klangVon.insert(ablauf.klangNamen[i], i);
    // Stufe 15l (WV5 a): die Kanaele, die ein Baustein als Steuersignal nennt
    std::vector<int> abgriffVon;
    ablauf.abgriffe = planeAbgriffe(loop, eingefroren, abgriffVon);

    for (std::size_t kanalNr = 0; kanalNr < loop.kanaele.size(); ++kanalNr)
    {
        const Kanal& kanal = loop.kanaele[kanalNr];
        // ein genannter Kanal klingt auch ausgeschaltet — nur als Steuersignal (vor dem Schalter)
        const int abgriff = abgriffVon[kanalNr];
        if (!kanal.an && abgriff < 0) continue;
        // der Fader (Stufe 27e-1) und die Lautstaerke des Loops (Stufe 30, WX5 a): ohne
        // Effekte am Schlag, mit Effekten am Bus — nach den Effekten, wie an einem Mischpult.
        // In einem genannten Kanal traegt beides der Abgriff, nach dem Steuersignal (15l).
        const double fader = kanal.gainFaktor();
        const auto ausgang = static_cast<float>(kanal.an ? fader * loop.laut : 0.0);
        if (abgriff >= 0) ablauf.abgriffe[static_cast<std::size_t>(abgriff)].faktor = ausgang;
        const double faderOhneBus = abgriff < 0 ? fader : 1.0;
        const double loopOhneBus = abgriff < 0 ? loop.laut : 1.0;
        // je Kanal: (Muster, Spur) -> Stimme
        std::map<std::pair<const Muster*, std::size_t>, std::size_t> stimmeVon;
        // je Kanal: (Instrument-Muster, Ton) -> Stimme. Jeder Ton — Tonhoehe
        // und Dauer — ist eine eigene Stimme: ein Akkord klingt zusammen, und
        // nur derselbe Ton schneidet sich selbst ab.
        std::map<std::pair<const Muster*, int>, std::size_t> tonStimmeVon;
        // die Toene der Saiteninstrumente je Vorkommen (Stufe 23b); leer ohne `saiten`
        const std::vector<ClipToene> saitenToene = toeneImKanal(loop, kanalNr, saiten);
        for (std::size_t clipNr = 0; clipNr < kanal.clips.size(); ++clipNr)
        {
            const LoopClip& clip = kanal.clips[clipNr];
            const ClipToene& toeneDesClips = saitenToene[clipNr];
            const Muster* muster = loop.findeMuster(clip.muster);
            if (muster == nullptr) continue;
            const double swingFrames = swingFramesVon(*muster);
            // eingefroren (Stufe 30f): die Wellenform als ein Klang ab dem Anfang des Clips, mit
            // Fader und Lautstaerke des Loops (F3 a) — Effekte und Expression stecken in ihr
            if (const auto ort = eingefroren.find({static_cast<int>(kanalNr), clip.startTakt}); ort != eingefroren.end())
            {
                Stimme stimme;
                stimme.klang = klangVon.value(frozenKlang(ort->second), -1);
                stimme.schneidet = false;  // FV1 a
                Schlag schlag;
                schlag.frame = std::llround(static_cast<double>(clip.startTakt) * jeTakt * schrittFrames);
                schlag.links = static_cast<float>(faderOhneBus * loopOhneBus);
                schlag.rechts = schlag.links;
                schlag.abgriff = abgriff;
                stimme.schlaege.push_back(schlag);
                ablauf.stimmen.push_back(std::move(stimme));
                continue;
            }
            // ein Clip mit Effekten: seine Schlaege gehen in einen eigenen Bus —
            // nicht, wenn sein Schalter aus ist (E1 a): dann klingt er trocken
            int bus = -1;
            if (clip.effekteWirken())
            {
                bus = static_cast<int>(ablauf.busse.size());
                Bus neu{static_cast<int>(kanalNr), clip.startTakt, clip.effekte,
                        abgriff < 0 ? static_cast<float>(fader * loop.laut) : ausgang};
                // Stufe 15l: je Baustein das Steuersignal, das er bekommt
                neu.abgriff = abgriff;
                for (const ClipBaustein& baustein : clip.effekte.reihe)
                {
                    const int ziel = bausteinAn(baustein) ? steuerKanal(loop, static_cast<int>(kanalNr), baustein) : -1;
                    neu.steuer.push_back(ziel >= 0 ? abgriffVon[static_cast<std::size_t>(ziel)] : -1);
                }
                if (std::all_of(neu.steuer.begin(), neu.steuer.end(), [](int s) { return s < 0; })) neu.steuer.clear();
                if (abgriff >= 0) ablauf.abgriffe[static_cast<std::size_t>(abgriff)].busse.push_back(bus);
                ablauf.busse.push_back(std::move(neu));
            }
            const double faderAmSchlag = bus < 0 ? faderOhneBus : 1.0;
            const double loopAmSchlag = bus < 0 ? loopOhneBus : 1.0;
            const int abgriffAmSchlag = bus < 0 ? abgriff : -1;
            // Expression (Stufe 28c-1): die Spur des Musters und die Kurve am Clip — an einem
            // Drumkit nur die Kurve; ein Instrument mit Saiten traegt sie im Ton (28c-2b, SK12 a);
            // ein Wave-Muster (Stufe 15c) wie ein Instrument ohne Saiten
            int ausdruck = -1;
            if (toeneDesClips.empty())
            {
                static const std::vector<SpurPunkt> kKeine;
                const std::vector<SpurPunkt>& spur = muster->art == MusterArt::Instrument || muster->art == MusterArt::Wave
                                                         ? muster->zeitspuren.von(Steuerspur::Expression)
                                                         : kKeine;
                if (!spur.empty() || !clip.expression.empty())
                {
                    const double schritte = static_cast<double>(muster->takte) * jeTakt;
                    const double clipSchritt = static_cast<double>(clip.startTakt) * jeTakt;
                    const int mal = std::max(1, clip.wiederholungen);
                    Ausdruck a;
                    a.muster = ueberDieWiederholungen(spur, clipSchritt, schritte, mal, schrittFrames);
                    for (const SpurPunkt& p : clip.expression)
                        a.clip.emplace_back(std::llround((clip.startTakt + p.stelle) * jeTakt * schrittFrames), p.wert);
                    a.ende = std::llround((clipSchritt + mal * schritte) * schrittFrames);
                    ausdruck = static_cast<int>(ablauf.ausdruecke.size());
                    ablauf.ausdruecke.push_back(std::move(a));
                }
            }
            if (muster->art == MusterArt::Wave)
            {
                // Stufe 15c (WV8 a): je Wiederholung ein Schlag am Anfang des Musters — ohne Swing,
                // ohne Panorama; die naechste Wiederholung schneidet die vorige ab (sie endet dort)
                if (muster->wave.datei.isEmpty()) continue;
                const int klang = klangVon.value(waveKlangName(waveKlangVon(loop, *muster)), -1);
                const auto [ort, neu] = stimmeVon.try_emplace({muster, kWaveStimme}, ablauf.stimmen.size());
                if (neu)
                {
                    Stimme stimme;
                    stimme.klang = klang;
                    ablauf.stimmen.push_back(std::move(stimme));
                }
                const auto laut = static_cast<float>(loopAmSchlag * faderAmSchlag);
                for (int runde = 0; runde < clip.wiederholungen; ++runde)
                {
                    Schlag schlag;
                    schlag.frame = std::llround(static_cast<double>(clip.startTakt + runde * muster->takte) * jeTakt *
                                                schrittFrames);
                    schlag.links = laut;
                    schlag.rechts = laut;
                    schlag.bus = bus;
                    schlag.ausdruck = ausdruck;
                    schlag.abgriff = abgriffAmSchlag;
                    ablauf.stimmen[ort->second].schlaege.push_back(schlag);
                }
                continue;
            }
            if (muster->art == MusterArt::Instrument)
            {
                if (muster->stumm) continue;
                const double lautMuster = muster->laut * loopAmSchlag * faderAmSchlag;
                // Stufe 31b (HK2 a): an einer Hammer-Saite traegt der Ton die Stufe, der Rest ist Pegel
                const bool hammer = saiten && saiten(muster->instrument).hammer;
                for (int runde = 0; runde < clip.wiederholungen; ++runde)
                {
                    const std::int64_t schrittDavor =
                        static_cast<std::int64_t>(clip.startTakt + runde * muster->takte) * jeTakt;
                    for (std::size_t noteNr = 0; noteNr < muster->noten.size(); ++noteNr)
                    {
                        const Note& note = muster->noten[noteNr];
                        const GeplanterTon* geplant =
                            toeneDesClips.empty() ? nullptr : &toeneDesClips[static_cast<std::size_t>(runde)][noteNr];
                        const bool versetzt = (note.schritt % loop.schritteJeSchlag) % 2 == 1;
                        // Strum (Stufe 24b): eine Saite des Akkords schlaegt so viel spaeter an
                        const double strumFrames = geplant == nullptr ? 0.0 : geplant->versatzMs * rate / 1000.0;
                        // 31d-9 (N1 a): der Versatz des Anfangs in Bruchteilen eines Schritts; ohne ihn + 0
                        const double stelle = static_cast<double>(schrittDavor + note.schritt) * schrittFrames +
                                              (versetzt ? swingFrames : 0.0) + strumFrames +
                                              note.versatzAnfang * schrittFrames;
                        const double laut =
                            (hammer ? anschlagRest(note.laut, stufeDerNote(note, true)) : note.laut) * lautMuster;
                        const auto schlage = [&](const QString& name) {
                            const int klang = klangVon.value(name, -1);
                            const auto [ort, neu] = tonStimmeVon.try_emplace({muster, klang}, ablauf.stimmen.size());
                            if (neu)
                            {
                                Stimme stimme;
                                stimme.klang = klang;
                                ablauf.stimmen.push_back(std::move(stimme));
                            }
                            Schlag schlag;
                            schlag.frame = std::llround(stelle);
                            schlag.links = static_cast<float>(laut * std::min(1.0, 1.0 - muster->pan));
                            schlag.rechts = static_cast<float>(laut * std::min(1.0, 1.0 + muster->pan));
                            schlag.bus = bus;
                            schlag.ausdruck = ausdruck;
                            schlag.abgriff = abgriffAmSchlag;
                            ablauf.stimmen[ort->second].schlaege.push_back(schlag);
                        };
                        // ohne Saiten: so lange, wie die Pedale halten (Stufe 28b), mit Pitch und
                        // Vibrato (28c-1b) — ohne Spuren derselbe Name wie bis Stufe 27
                        schlage(geplant == nullptr ? tonKlang(tonImClip(loop, *muster, clip.wiederholungen, runde, noteNr, hammer))
                                                   : tonKlang(*geplant));
                        // Stufe 25c (S-L12 a): beim Kopf einer Kette die Saite ueber die ganze Kette
                        if (geplant != nullptr)
                            for (const TonAngabe& zusatz : geplant->zusatz) schlage(tonKlang(zusatz));
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
                            stimme.klang = static_cast<int>(ablauf.klangNamen.indexOf(spur.spielt()));
                            ablauf.stimmen.push_back(std::move(stimme));
                        }
                        const bool versetzt = (i % loop.schritteJeSchlag) % 2 == 1;
                        const double stelle = static_cast<double>(schrittDavor + i) * schrittFrames +
                                              (versetzt ? swingFrames : 0.0);
                        const double laut = wert * spur.laut * loopAmSchlag * faderAmSchlag;
                        Schlag schlag;
                        schlag.frame = std::llround(stelle);
                        schlag.links = static_cast<float>(laut * std::min(1.0, 1.0 - spur.pan));
                        schlag.rechts = static_cast<float>(laut * std::min(1.0, 1.0 + spur.pan));
                        schlag.bus = bus;
                        schlag.ausdruck = ausdruck;
                        schlag.abgriff = abgriffAmSchlag;
                        ablauf.stimmen[ort->second].schlaege.push_back(schlag);
                    }
                }
            }
        }
    }
    ablauf.durchlaufFrames = std::llround(static_cast<double>(loop.durchlaufSchritte()) * schrittFrames);
    return ablauf;
}

void rechne(const Ablauf& ablauf, const Klaenge& klaenge, std::int64_t von, int frames, float* aus,
            float* const* busAus, float* const* abgriffAus)
{
    if (frames <= 0) return;
    std::fill(aus, aus + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
    if (busAus != nullptr)
        for (std::size_t b = 0; b < ablauf.busse.size(); ++b)
            std::fill(busAus[b], busAus[b] + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
    if (abgriffAus != nullptr)
        for (std::size_t a = 0; a < ablauf.abgriffe.size(); ++a)
            std::fill(abgriffAus[a], abgriffAus[a] + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
    const std::int64_t durchlauf = ablauf.durchlaufFrames;
    const std::int64_t bis = von + frames;
    if (durchlauf <= 0 || bis <= 0) return;
    const std::int64_t ab = std::max<std::int64_t>(0, von);

    for (const Stimme& stimme : ablauf.stimmen)
    {
        const std::vector<Schlag>& schlaege = stimme.schlaege;
        const auto anzahl = static_cast<std::int64_t>(schlaege.size());
        if (anzahl == 0 || stimme.klang < 0 || static_cast<std::size_t>(stimme.klang) >= klaenge.size()) continue;
        const Klang* gefunden = klaenge[static_cast<std::size_t>(stimme.klang)].get();
        if (gefunden == nullptr) continue;
        const Klang& klangDerStimme = *gefunden;
        const std::int64_t laenge = klangDerStimme.frames();
        if (laenge == 0) continue;
        const float* klang = klangDerStimme.daten.data();

        // Die Schlaege der Stimme ueber alle Durchlaeufe, durchgezaehlt ab 0.
        const auto beginn = [&](std::int64_t g) {
            return (g / anzahl) * durchlauf + schlaege[static_cast<std::size_t>(g % anzahl)].frame;
        };
        // Der letzte Schlag, der bei `ab` schon begonnen hat — und der davor:
        // er kann noch in seiner Schnitt-Blende klingen. Schneidet die Stimme sich
        // nicht (FV1 a), so weit zurueck, wie ihr Klang reicht.
        const std::int64_t runde = ab / durchlauf;
        std::int64_t g = 0;
        if (stimme.schneidet)
        {
            const auto danach = std::upper_bound(schlaege.begin(), schlaege.end(), ab - runde * durchlauf,
                                                 [](std::int64_t stelle, const Schlag& h) { return stelle < h.frame; });
            g = runde * anzahl + (danach - schlaege.begin()) - 2;
        }
        else
            g = (runde - laenge / durchlauf - 1) * anzahl;
        g = std::max<std::int64_t>(0, g);

        for (;; ++g)
        {
            const std::int64_t start = beginn(g);
            if (start >= bis) break;
            const std::int64_t naechster =
                stimme.schneidet ? beginn(g + 1) : std::numeric_limits<std::int64_t>::max();
            const std::int64_t ende =
                stimme.schneidet ? std::min(start + laenge, naechster + kSchnittFrames) : start + laenge;
            const Schlag& schlag = schlaege[static_cast<std::size_t>(g % anzahl)];
            // auch die Schnitt-Blende eines Schlags bleibt in seinem Bus (oder Abgriff, 15l)
            float* ziel = aus;
            // ohne Busse geht ein Schlag mit Effekten unbearbeitet nach `aus` — dann mit dem Fader;
            // ebenso ohne Abgriffe ein Schlag eines genannten Kanals
            float fader = 1.0f;
            if (schlag.bus >= 0)
            {
                if (busAus != nullptr)
                    ziel = busAus[schlag.bus];
                else
                    fader = ablauf.busse[static_cast<std::size_t>(schlag.bus)].faktor;
            }
            else if (schlag.abgriff >= 0)
            {
                if (abgriffAus != nullptr)
                    ziel = abgriffAus[schlag.abgriff];
                else
                    fader = ablauf.abgriffe[static_cast<std::size_t>(schlag.abgriff)].faktor;
            }
            if (schlag.ausdruck < 0)
            {
                for (std::int64_t f = std::max(ab, start); f < std::min(bis, ende); ++f)
                {
                    const float blende =
                        f < naechster ? 1.0f : 1.0f - static_cast<float>(f - naechster) / kSchnittFrames;
                    const std::int64_t imKlang = f - start;
                    const std::int64_t imBlock = f - von;
                    ziel[2 * imBlock] += klang[2 * imKlang] * schlag.links * blende * fader;
                    ziel[2 * imBlock + 1] += klang[2 * imKlang + 1] * schlag.rechts * blende * fader;
                }
                continue;
            }
            // Expression (Stufe 28c-1, SK9 a): der Pegel e² an der Stelle des Abtastwerts im
            // Durchlauf — hinter dem Ende des Clips der Wert dort
            const Ausdruck& ausdruck = ablauf.ausdruecke[static_cast<std::size_t>(schlag.ausdruck)];
            StueckLeser muster(ausdruck.muster);
            StueckLeser clip(ausdruck.clip);
            for (std::int64_t f = std::max(ab, start); f < std::min(bis, ende); ++f)
            {
                const float blende =
                    f < naechster ? 1.0f : 1.0f - static_cast<float>(f - naechster) / kSchnittFrames;
                const std::int64_t imKlang = f - start;
                const std::int64_t imBlock = f - von;
                const std::int64_t x = std::min(schlag.frame + imKlang, ausdruck.ende);
                const double e = muster.bei(x) * clip.bei(x);
                const auto pegel = static_cast<float>(e * e);
                ziel[2 * imBlock] += klang[2 * imKlang] * schlag.links * blende * fader * pegel;
                ziel[2 * imBlock + 1] += klang[2 * imKlang + 1] * schlag.rechts * blende * fader * pegel;
            }
        }
    }
}

}  // namespace lumi::loop
