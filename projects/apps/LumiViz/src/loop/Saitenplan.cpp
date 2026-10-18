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

/// Was eine Moeglichkeit kostet — kleiner ist besser, Stelle fuer Stelle.
using Kosten = std::tuple<int, int, int, int>;  // nicht greifbar, Abstand der Buende, Weg der Hand, Summe der Buende

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
        for (std::size_t i = a; i < b; ++i)
        {
            const Note& note = noten[i];
            if (greifbarAuf(saiten, note.saite, note.hoehe) && !imAnschlag[static_cast<std::size_t>(note.saite)])
            {
                imAnschlag[static_cast<std::size_t>(note.saite)] = true;
                griffe[i] = Griff{note.saite, bundAuf(saiten, note.saite, note.hoehe), true};
                festeBuende.push_back(griffe[i].bund);
            }
            else
                offen.push_back(i);
        }

        // die uebrigen: alle Moeglichkeiten auf freien Saiten, die beste nach `Kosten`
        std::vector<int> wahl(offen.size(), -1);
        std::vector<int> beste(offen.size(), -1);
        Kosten besteKosten{std::numeric_limits<int>::max(), 0, 0, 0};
        long versuche = 0;
        const auto bewerte = [&]() {
            int nicht = 0;
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
                zaehle(bundAuf(saiten, wahl[k], noten[offen[k]].hoehe));
            }
            const int abstand = hoch >= 0 ? hoch - tief : 0;
            const int weg = hand && vorigeLage >= 0 && hoch >= 0 ? std::abs(tief - vorigeLage) : 0;
            return Kosten{nicht, abstand, weg, summe};
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
            const int hoehe = noten[offen[k]].hoehe;
            for (int s = 0; s < anzahl; ++s)
            {
                const auto platz = static_cast<std::size_t>(s);
                if (belegt[platz] || imAnschlag[platz] || !greifbarAuf(saiten, s, hoehe)) continue;
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
                griff = Griff{beste[k], bundAuf(saiten, beste[k], hoehe), true};
            else
            {
                const int s = naechstbesteSaite(saiten, hoehe);
                griff = Griff{s, bundAuf(saiten, s, hoehe), false};
            }
        }
        for (std::size_t i = a; i < b; ++i)
            if (griffe[i].greifbar && griffe[i].bund > 0) lage = std::min(lage, griffe[i].bund);
        if (lage != std::numeric_limits<int>::max()) vorigeLage = lage;  // nur leere Saiten: die Hand bleibt
        a = b;
    }
    return griffe;
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
            ort = gegriffen.emplace(muster, std::move(neu)).first;
        }
        const Gegriffen& g = ort->second;
        if (!g.saiten.istSaiteninstrument()) continue;

        const auto runden = static_cast<std::size_t>(std::max(1, clip.wiederholungen));
        toene[c].assign(runden, std::vector<TonAngabe>(muster->noten.size()));
        for (std::size_t r = 0; r < runden; ++r)
        {
            const std::int64_t schrittDavor =
                static_cast<std::int64_t>(clip.startTakt + static_cast<int>(r) * muster->takte) * jeTakt;
            for (std::size_t i = 0; i < muster->noten.size(); ++i)
            {
                const Note& note = muster->noten[i];
                const Griff& griff = g.griffe[i];
                toene[c][r][i] = TonAngabe{muster->instrument, note.hoehe, loop.notenMs(note), griff.saite, 0};
                if (!griff.greifbar) continue;  // eine nicht greifbare Note daempft nicht und wird nicht gedaempft
                const bool versetzt = (note.schritt % loop.schritteJeSchlag) % 2 == 1;
                const double startMs =
                    static_cast<double>(schrittDavor + note.schritt) * schrittMs + (versetzt ? swingMs : 0.0);
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
        toene[v.clip][v.runde][v.note].endeMs = std::max(1, static_cast<int>(std::lround(naechste - v.startMs)));
    }
    return toene;
}

}  // namespace lumi::loop
