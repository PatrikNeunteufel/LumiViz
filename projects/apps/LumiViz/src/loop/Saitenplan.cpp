/**
 * @file Saitenplan.cpp
 * @brief Loop: Saitenwahl und Abdaempfen beim Planen (Stufe 23b)
 */

#include "loop/Saitenplan.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <limits>
#include <map>
#include <tuple>
#include <utility>

namespace lumi::loop
{

namespace
{

/// So viele Moeglichkeiten prueft die Suche je Anschlag hoechstens; danach gilt
/// die beste bis dahin. Bei sechs Noten auf sechs Saiten sind es 720 — die
/// Grenze greift erst bei sehr vielen Saiten und Noten zugleich.
constexpr long kVersucheMax = 200000;

/// Was eine Moeglichkeit kostet — kleiner ist besser, Stelle fuer Stelle: nicht
/// greifbar, Flageoletts (Stufe 24b), Abstand der Buende, Weg der Hand, Summe
/// der Buende. Ohne Noten mit *Harmonic* ist die zweite Stelle immer 0 — die
/// Wahl ist dieselbe wie bis Stufe 23.
using Kosten = std::tuple<int, int, int, int, int>;

/// Was ein Flageolett kostet (S-W7 a): ein natuerliches sein n, ein kuenstliches
/// mehr als jedes natuerliche.
int flageolettKosten(const Flageolett& f)
{
    return f.kuenstlich ? kFlageolettTeiltonMax + 1 : f.teilton;
}

bool istFlageolett(const Note& note)
{
    return (note.artikulationen & kHarmonic) != 0;
}

/// Ist `note` auf `saite` spielbar — gegriffen, oder bei *Harmonic* als Flageolett?
bool spielbarAuf(const Saitensatz& saiten, int saite, const Note& note)
{
    return istFlageolett(note) ? flageolettAuf(saiten, saite, note.hoehe).teilton > 0
                               : greifbarAuf(saiten, saite, note.hoehe);
}

/// Der Griff einer spielbaren Note auf `saite`.
Griff griffAuf(const Saitensatz& saiten, int saite, const Note& note)
{
    if (istFlageolett(note))
    {
        const Flageolett f = flageolettAuf(saiten, saite, note.hoehe);
        return Griff{saite, f.bund, true, f.teilton};
    }
    return Griff{saite, bundAuf(saiten, saite, note.hoehe), true};
}

/// Die naechstbeste Saite einer Note, die nirgends frei greifbar ist: die mit
/// dem kleinsten Weg bis in den greifbaren Bereich, bei Gleichstand die erste.
int naechstbesteSaite(const Saitensatz& saiten, int hoehe)
{
    int beste = 0;
    int besterWeg = std::numeric_limits<int>::max();
    for (int s = 0; s < saiten.anzahl(); ++s)
    {
        const int bund = bundAuf(saiten, s, hoehe);
        const int weg = bund < 0 ? -bund : bund > kBuendeMax ? bund - kBuendeMax : 0;
        if (weg < besterWeg)
        {
            besterWeg = weg;
            beste = s;
        }
    }
    return beste;
}

/// Die vorige Note einer Note mit *Legato* oder *Slide* (Stufe 25b, S-L1 a): die
/// letzte, die vor ihrem Anschlag (`a`, der erste Platz ihres Anschlags) auf einer
/// Saite anschlug, auf der beide spielbar sind — bei einer gesetzten Saite nur auf
/// dieser. Unter mehreren die spaeteste, dann die naechste in der Tonhoehe, dann
/// die tiefere Saite (von Claude gesetzt). Eine Pause dazwischen trennt nicht: die
/// Saite klingt nach. −1 = keine.
int vorigeNote(const std::vector<Note>& noten, const std::vector<Griff>& griffe, const Saitensatz& saiten,
               std::size_t i, std::size_t a)
{
    const Note& note = noten[i];
    const bool gesetzt = note.saite >= 0 && note.saite < saiten.anzahl();
    int beste = -1;
    for (std::size_t j = 0; j < a; ++j)
    {
        const Griff& g = griffe[j];
        if (!g.greifbar || g.saite < 0) continue;
        if (gesetzt && g.saite != note.saite) continue;
        if (!spielbarAuf(saiten, g.saite, note)) continue;
        // die letzte auf ihrer Saite vor dem Anschlag?
        bool spaeter = false;
        for (std::size_t k = j + 1; k < a && !spaeter; ++k)
            spaeter = griffe[k].greifbar && griffe[k].saite == g.saite;
        if (spaeter) continue;
        if (beste < 0)
        {
            beste = static_cast<int>(j);
            continue;
        }
        const Note& bisher = noten[static_cast<std::size_t>(beste)];
        const Note& kandidat = noten[j];
        const int abstandBisher = std::abs(bisher.hoehe - note.hoehe);
        const int abstandKandidat = std::abs(kandidat.hoehe - note.hoehe);
        if (std::tie(kandidat.schritt, abstandBisher, griffe[static_cast<std::size_t>(beste)].saite) >
            std::tie(bisher.schritt, abstandKandidat, g.saite))
            beste = static_cast<int>(j);
    }
    return beste;
}

}  // namespace

std::vector<Griff> greifeNoten(const Muster& muster, const Saitensatz& saiten)
{
    const std::vector<Note>& noten = muster.noten;
    std::vector<Griff> griffe(noten.size());
    if (!saiten.istSaiteninstrument()) return griffe;
    const int anzahl = saiten.anzahl();
    const bool hand = muster.saitenwahl == Saitenwahl::Hand;
    int vorigeLage = -1;  // die Lage des vorigen Anschlags; −1 = noch keine

    for (std::size_t a = 0; a < noten.size();)
    {
        // ein Anschlag: die Noten, die am selben Schritt beginnen (gerichtet: nach Schritt sortiert)
        std::size_t b = a;
        while (b < noten.size() && noten[b].schritt == noten[a].schritt) ++b;
        const int schritt = noten[a].schritt;

        // belegt: Saiten von Noten, die noch gehalten werden
        std::vector<bool> belegt(static_cast<std::size_t>(anzahl), false);
        for (std::size_t i = 0; i < a; ++i)
            if (griffe[i].greifbar && noten[i].schritt + noten[i].laenge > schritt)
                belegt[static_cast<std::size_t>(griffe[i].saite)] = true;

        // eine Note mit greifbarer Saite behaelt sie (S-M2 a); zwei auf derselben: die erste
        std::vector<std::size_t> offen;
        std::vector<bool> imAnschlag(static_cast<std::size_t>(anzahl), false);
        std::vector<int> festeBuende;
        std::vector<bool> fehlt(b - a, false);
        for (std::size_t i = a; i < b; ++i)
        {
            const Note& note = noten[i];
            // Stufe 25b (S-L1 a): Legato oder Slide — auf die Saite der vorigen Note; die ist von ihr
            // belegt, aber sie klingt in dieser weiter
            if ((note.artikulationen & kVerbunden) != 0)
            {
                const int vorige = vorigeNote(noten, griffe, saiten, i, a);
                if (vorige >= 0)
                {
                    const int s = griffe[static_cast<std::size_t>(vorige)].saite;
                    if (!imAnschlag[static_cast<std::size_t>(s)])
                    {
                        imAnschlag[static_cast<std::size_t>(s)] = true;
                        griffe[i] = griffAuf(saiten, s, note);
                        griffe[i].vorige = vorige;
                        festeBuende.push_back(griffe[i].bund);
                        continue;
                    }
                }
                fehlt[i - a] = true;
            }
            if (spielbarAuf(saiten, note.saite, note) && !imAnschlag[static_cast<std::size_t>(note.saite)])
            {
                imAnschlag[static_cast<std::size_t>(note.saite)] = true;
                griffe[i] = griffAuf(saiten, note.saite, note);
                festeBuende.push_back(griffe[i].bund);
            }
            else
                offen.push_back(i);
        }

        // die uebrigen: alle Moeglichkeiten auf freien Saiten, die beste nach `Kosten`
        std::vector<int> wahl(offen.size(), -1);
        std::vector<int> beste(offen.size(), -1);
        Kosten besteKosten{std::numeric_limits<int>::max(), 0, 0, 0, 0};
        long versuche = 0;
        const auto bewerte = [&]() {
            int nicht = 0;
            int flageoletts = 0;
            int tief = std::numeric_limits<int>::max();
            int hoch = -1;
            int summe = 0;
            const auto zaehle = [&](int bund) {
                summe += bund;
                if (bund > 0)
                {
                    tief = std::min(tief, bund);
                    hoch = std::max(hoch, bund);
                }
            };
            for (const int bund : festeBuende) zaehle(bund);
            for (std::size_t k = 0; k < offen.size(); ++k)
            {
                if (wahl[k] < 0)
                {
                    ++nicht;
                    continue;
                }
                const Note& note = noten[offen[k]];
                if (istFlageolett(note))
                {
                    const Flageolett f = flageolettAuf(saiten, wahl[k], note.hoehe);
                    flageoletts += flageolettKosten(f);
                    zaehle(f.bund);  // ein natuerliches greift nicht
                }
                else
                    zaehle(bundAuf(saiten, wahl[k], note.hoehe));
            }
            const int abstand = hoch >= 0 ? hoch - tief : 0;
            const int weg = hand && vorigeLage >= 0 && hoch >= 0 ? std::abs(tief - vorigeLage) : 0;
            return Kosten{nicht, flageoletts, abstand, weg, summe};
        };
        const std::function<void(std::size_t)> suche = [&](std::size_t k) {
            if (versuche >= kVersucheMax) return;
            if (k == offen.size())
            {
                ++versuche;
                const Kosten kosten = bewerte();
                if (kosten < besteKosten)
                {
                    besteKosten = kosten;
                    beste = wahl;
                }
                return;
            }
            const Note& note = noten[offen[k]];
            for (int s = 0; s < anzahl; ++s)
            {
                const auto platz = static_cast<std::size_t>(s);
                if (belegt[platz] || imAnschlag[platz] || !spielbarAuf(saiten, s, note)) continue;
                imAnschlag[platz] = true;
                wahl[k] = s;
                suche(k + 1);
                imAnschlag[platz] = false;
            }
            wahl[k] = -1;  // auch ohne Saite weiter: eine Note zu viel verhindert die anderen nicht
            suche(k + 1);
        };
        suche(0);

        int lage = std::numeric_limits<int>::max();
        for (std::size_t k = 0; k < offen.size(); ++k)
        {
            Griff& griff = griffe[offen[k]];
            const int hoehe = noten[offen[k]].hoehe;
            if (beste[k] >= 0)
                griff = griffAuf(saiten, beste[k], noten[offen[k]]);
            else
            {
                const int s = naechstbesteSaite(saiten, hoehe);
                griff = Griff{s, bundAuf(saiten, s, hoehe), false};
            }
        }
        for (std::size_t i = a; i < b; ++i)
        {
            if (griffe[i].greifbar && griffe[i].bund > 0) lage = std::min(lage, griffe[i].bund);
            griffe[i].verbindungFehlt = fehlt[i - a];
        }
        if (lage != std::numeric_limits<int>::max()) vorigeLage = lage;  // nur leere Saiten: die Hand bleibt
        a = b;
    }
    return griffe;
}

std::vector<Anschlagplan> schlageAn(const Muster& muster, const Saitensatz& saiten, const std::vector<Griff>& griffe,
                                    int schlaegeJeTakt, int schritteJeSchlag)
{
    const std::vector<Note>& noten = muster.noten;
    std::vector<Anschlagplan> plan(noten.size());
    if (!saiten.istSaiteninstrument() || griffe.size() != noten.size()) return plan;
    for (std::size_t a = 0; a < noten.size();)
    {
        // ein Akkord: die Noten, die am selben Schritt anschlagen (gerichtet: nach Schritt sortiert)
        std::size_t b = a;
        while (b < noten.size() && noten[b].schritt == noten[a].schritt) ++b;
        const int schritt = noten[a].schritt;
        const Anschlag* abweichung = anschlagAn(muster, schritt);
        const Anschlagart art = abweichung != nullptr ? abweichung->art : Anschlagart::Hand;
        const double spreadMs =
            abweichung != nullptr && abweichung->spreadMs >= 0.0 ? abweichung->spreadMs : muster.strum.spreadMs;
        Richtung richtung = Richtung::Ab;  // auch bei Together
        if (art == Anschlagart::Auf)
            richtung = Richtung::Auf;
        else if (art == Anschlagart::Hand)
            richtung = richtungAnStelle(muster.strum,
                                        handStelleVon(muster.strum.raster, schritt, schlaegeJeTakt, schritteJeSchlag));

        int erste = std::numeric_limits<int>::max();
        int letzte = -1;
        for (std::size_t i = a; i < b; ++i)
            if (griffe[i].saite >= 0)
            {
                erste = std::min(erste, griffe[i].saite);
                letzte = std::max(letzte, griffe[i].saite);
            }
        for (std::size_t i = a; i < b; ++i)
        {
            plan[i].richtung = richtung;
            const int saite = griffe[i].saite;
            if (art == Anschlagart::Zugleich || saite < 0) continue;
            // Down: die erste Saite am Instrument zuerst, Up: die letzte — je Saite des Wegs bis hierher
            // um Spread spaeter (S-W12 a)
            const int weg = richtung == Richtung::Ab ? saite - erste : letzte - saite;
            plan[i].versatzMs = spreadMs * weg;
        }
        a = b;
    }
    return plan;
}

std::vector<ClipToene> toeneImKanal(const Loop& loop, std::size_t kanalNr, const SaitenVon& saitenVon)
{
    std::vector<ClipToene> toene;
    if (kanalNr >= loop.kanaele.size()) return toene;
    const Kanal& kanal = loop.kanaele[kanalNr];
    toene.resize(kanal.clips.size());
    if (!saitenVon || loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return toene;

    // dieselbe Stelle wie `plane`, in Millisekunden
    const double schrittMs = 60000.0 / loop.bpm / loop.schritteJeSchlag;
    const double swingMs = loop.swing * 0.5 * schrittMs;
    const int jeTakt = loop.schritteJeTakt();
    const double durchlaufMs = loop.durchlaufMs();

    struct Gegriffen
    {
        Saitensatz saiten;
        std::vector<Griff> griffe;
        std::vector<Anschlagplan> anschlag;
    };
    std::map<const Muster*, Gegriffen> gegriffen;
    struct Vorkommen
    {
        std::size_t clip;
        std::size_t runde;
        std::size_t note;
        double startMs;
        const Muster* muster;
        int saite;
    };
    std::vector<Vorkommen> vorkommen;
    // je Muster und Saite die Anschlaege im Durchlauf (S-M3 a: derselbe Kanal, dasselbe Muster)
    std::map<std::pair<const Muster*, int>, std::vector<double>> anschlaege;

    for (std::size_t c = 0; c < kanal.clips.size(); ++c)
    {
        const LoopClip& clip = kanal.clips[c];
        const Muster* muster = loop.findeMuster(clip.muster);
        if (muster == nullptr || muster->art != MusterArt::Instrument) continue;
        auto ort = gegriffen.find(muster);
        if (ort == gegriffen.end())
        {
            Gegriffen neu;
            neu.saiten = saitenVon(muster->instrument);
            richteSaiten(neu.saiten);
            neu.griffe = greifeNoten(*muster, neu.saiten);
            neu.anschlag = schlageAn(*muster, neu.saiten, neu.griffe, loop.schlaegeJeTakt, loop.schritteJeSchlag);
            ort = gegriffen.emplace(muster, std::move(neu)).first;
        }
        const Gegriffen& g = ort->second;
        if (!g.saiten.istSaiteninstrument()) continue;

        const auto runden = static_cast<std::size_t>(std::max(1, clip.wiederholungen));
        toene[c].assign(runden, std::vector<GeplanterTon>(muster->noten.size()));
        for (std::size_t r = 0; r < runden; ++r)
        {
            const std::int64_t schrittDavor =
                static_cast<std::int64_t>(clip.startTakt + static_cast<int>(r) * muster->takte) * jeTakt;
            for (std::size_t i = 0; i < muster->noten.size(); ++i)
            {
                const Note& note = muster->noten[i];
                const Griff& griff = g.griffe[i];
                GeplanterTon& ton = toene[c][r][i];
                ton.instrument = muster->instrument;
                ton.hoehe = note.hoehe;
                ton.dauerMs = loop.notenMs(note);
                ton.saite = griff.saite;
                // Stufe 24b: Richtung und Versatz des Akkords, die Artikulationen, das Flageolett
                ton.richtung = g.anschlag[i].richtung;
                ton.versatzMs = g.anschlag[i].versatzMs;
                ton.artikulationen = note.artikulationen & (kPalmMute | kTapping);
                ton.teilton = griff.greifbar ? griff.teilton : 1;
                if (!griff.greifbar) continue;  // eine nicht greifbare Note daempft nicht und wird nicht gedaempft
                const bool versetzt = (note.schritt % loop.schritteJeSchlag) % 2 == 1;
                const double startMs = static_cast<double>(schrittDavor + note.schritt) * schrittMs +
                                       (versetzt ? swingMs : 0.0) + ton.versatzMs;
                vorkommen.push_back({c, r, i, startMs, muster, griff.saite});
                anschlaege[{muster, griff.saite}].push_back(startMs);
            }
        }
    }
    for (auto& [schluessel, stellen] : anschlaege) std::sort(stellen.begin(), stellen.end());

    // das Ende: wo die naechste Note auf derselben Saite anschlaegt — nach dem
    // letzten die erste im naechsten Durchlauf
    for (const Vorkommen& v : vorkommen)
    {
        if (durchlaufMs <= 0.0) break;
        const std::vector<double>& stellen = anschlaege[{v.muster, v.saite}];
        const auto danach = std::upper_bound(stellen.begin(), stellen.end(), v.startMs);
        const double naechste = danach != stellen.end() ? *danach : stellen.front() + durchlaufMs;
        GeplanterTon& ton = toene[v.clip][v.runde][v.note];
        ton.endeMs = std::max(1, static_cast<int>(std::lround(naechste - v.startMs)));
        // Let ring (S-W6 a): gehalten, bis die naechste Note auf derselben Saite sie abdaempft
        if ((v.muster->noten[v.note].artikulationen & kLetRing) != 0) ton.dauerMs = std::max(ton.dauerMs, ton.endeMs);
    }
    return toene;
}

std::vector<Kette> kettenImMuster(const Muster& muster, const std::vector<Griff>& griffe)
{
    std::vector<Kette> ketten;
    const std::size_t n = muster.noten.size();
    if (griffe.size() != n) return ketten;
    // je Note die Kette, zu der sie gehoert: die vorige liegt im Muster immer davor
    std::vector<std::size_t> platz(n, 0);
    for (std::size_t i = 0; i < n; ++i)
    {
        const int vorige = griffe[i].vorige;
        if (vorige >= 0 && static_cast<std::size_t>(vorige) < i)
        {
            platz[i] = platz[static_cast<std::size_t>(vorige)];
            ketten[platz[i]].glieder.push_back(i);
        }
        else
        {
            platz[i] = ketten.size();
            ketten.push_back(Kette{i, {}});
        }
    }
    return ketten;
}

void setzeKette(TonAngabe& ton, const Loop& loop, const Muster& muster, const Kette& kette)
{
    ton.verlauf.clear();
    ton.biegung.clear();
    if (kette.kopf >= muster.noten.size() || loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return;
    // dieselben Stellen wie `toeneImKanal`: Schritte mal Schrittlaenge, mit Swing
    const double schrittMs = 60000.0 / loop.bpm / loop.schritteJeSchlag;
    const double swingMs = loop.swing * 0.5 * schrittMs;
    const auto startMs = [&](const Note& note) {
        const bool versetzt = (note.schritt % loop.schritteJeSchlag) % 2 == 1;
        return note.schritt * schrittMs + (versetzt ? swingMs : 0.0);
    };
    const Note& kopf = muster.noten[kette.kopf];
    const double nullMs = startMs(kopf);

    std::vector<std::size_t> alle{kette.kopf};
    alle.insert(alle.end(), kette.glieder.begin(), kette.glieder.end());
    double endeMs = static_cast<double>(loop.notenMs(kopf));
    for (const std::size_t g : kette.glieder)
    {
        const Note& note = muster.noten[g];
        const double ab = startMs(note) - nullMs;
        ton.verlauf.push_back(TonWechsel{static_cast<int>(std::lround(ab)), note.hoehe, (note.artikulationen & kSlide) != 0});
        endeMs = std::max(endeMs, ab + loop.notenMs(note));
    }
    ton.dauerMs = std::max(ton.dauerMs, static_cast<int>(std::lround(endeMs)));

    // die Biegung: je Note ihr Bend auf ihrer Zeit; dazwischen haelt sie, bis die naechste Note
    // ohne Bend sie springend loest (S-L2 a)
    const auto setze = [&ton](int ms, int viertel) {
        if (!ton.biegung.empty() && ton.biegung.back() == TonBiegung{ms, viertel}) return;
        ton.biegung.push_back(TonBiegung{ms, viertel});
    };
    for (const std::size_t i : alle)
    {
        const Note& note = muster.noten[i];
        const int ab = static_cast<int>(std::lround(startMs(note) - nullMs));
        const int laenge = loop.notenMs(note);
        if (!ton.biegung.empty() && ton.biegung.back().ms < ab) setze(ab, ton.biegung.back().viertel);
        if (note.bend.empty())
        {
            if (!ton.biegung.empty() && ton.biegung.back().viertel != 0) setze(ab, 0);
            continue;
        }
        if (ton.biegung.empty() && ab > 0)
        {
            setze(0, 0);
            setze(ab, 0);
        }
        for (const BendPunkt& p : note.bend)
            setze(ab + static_cast<int>(std::lround(p.zeit * laenge)), static_cast<int>(std::lround(p.halbtoene / kBendRaster)));
    }
}

}  // namespace lumi::loop
