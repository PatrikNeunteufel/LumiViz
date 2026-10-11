/**
 * @file Auswahl.cpp
 * @brief Loop: mehrere Schritte oder Noten eines Musters zugleich
 */

#include "loop/Auswahl.hpp"

#include <algorithm>

namespace lumi::loop
{

namespace
{

/// Der Wert des Schritts zum Aendern; nullptr = diese Stelle gibt es nicht.
double* schrittIn(Muster& muster, SchrittOrt ort)
{
    if (ort.spur < 0 || ort.spur >= static_cast<int>(muster.zeilen.size())) return nullptr;
    std::vector<double>& zeile = muster.zeilen[static_cast<std::size_t>(ort.spur)];
    if (ort.schritt < 0 || ort.schritt >= static_cast<int>(zeile.size())) return nullptr;
    return &zeile[static_cast<std::size_t>(ort.schritt)];
}

const double* schrittIn(const Muster& muster, SchrittOrt ort)
{
    return schrittIn(const_cast<Muster&>(muster), ort);
}

/// Jeden Ort einmal, in der Reihenfolge des ersten Auftretens.
template <typename Ort>
std::vector<Ort> einmal(const std::vector<Ort>& orte)
{
    std::vector<Ort> ohneDoppelte;
    for (const Ort& ort : orte)
        if (std::find(ohneDoppelte.begin(), ohneDoppelte.end(), ort) == ohneDoppelte.end()) ohneDoppelte.push_back(ort);
    return ohneDoppelte;
}

/// Wie `Loop::richte` die Noten ordnet: nach Schritt, dann Tonhoehe.
void ordneNoten(Muster& muster)
{
    std::stable_sort(muster.noten.begin(), muster.noten.end(), [](const Note& a, const Note& b) {
        return a.schritt != b.schritt ? a.schritt < b.schritt : a.hoehe < b.hoehe;
    });
}

}  // namespace

// --- Schritte ----------------------------------------------------------------

SchrittOrt anfangVon(const std::vector<SchrittOrt>& orte)
{
    if (orte.empty()) return {};
    SchrittOrt anfang = orte.front();
    for (const SchrittOrt& ort : orte)
    {
        anfang.spur = std::min(anfang.spur, ort.spur);
        anfang.schritt = std::min(anfang.schritt, ort.schritt);
    }
    return anfang;
}

SchrittKopie kopiereSchritte(const Muster& muster, const std::vector<SchrittOrt>& orte)
{
    std::vector<SchrittOrt> gueltig;
    for (const SchrittOrt& ort : einmal(orte))
        if (schrittIn(muster, ort) != nullptr) gueltig.push_back(ort);
    SchrittKopie kopie;
    if (gueltig.empty()) return kopie;
    const SchrittOrt anfang = anfangVon(gueltig);
    for (const SchrittOrt& ort : gueltig)
    {
        kopie.teile.push_back({ort.spur - anfang.spur, ort.schritt - anfang.schritt, *schrittIn(muster, ort)});
        kopie.spuren = std::max(kopie.spuren, ort.spur - anfang.spur + 1);
        kopie.schritte = std::max(kopie.schritte, ort.schritt - anfang.schritt + 1);
    }
    return kopie;
}

int fuegeSchritteEin(Muster& muster, const SchrittKopie& kopie, SchrittOrt ziel, bool ersetzen,
                     std::vector<SchrittOrt>* eingefuegt)
{
    int ohnePlatz = 0;
    if (eingefuegt != nullptr) eingefuegt->clear();
    for (const SchrittKopie::Teil& teil : kopie.teile)
    {
        const SchrittOrt ort{ziel.spur + teil.spur, ziel.schritt + teil.schritt};
        double* wert = schrittIn(muster, ort);
        if (wert == nullptr)
        {
            if (teil.wert > 0.0) ++ohnePlatz;
            continue;
        }
        if (ersetzen)
            *wert = teil.wert;
        else if (teil.wert > 0.0 && *wert <= 0.0)
            *wert = teil.wert;
        if (eingefuegt != nullptr) eingefuegt->push_back(ort);
    }
    return ohnePlatz;
}

bool leereSchritte(Muster& muster, const std::vector<SchrittOrt>& orte)
{
    bool geaendert = false;
    for (const SchrittOrt& ort : orte)
        if (double* wert = schrittIn(muster, ort); wert != nullptr && *wert > 0.0)
        {
            *wert = 0.0;
            geaendert = true;
        }
    return geaendert;
}

bool versetzeSchritte(Muster& muster, std::vector<SchrittOrt>& orte, int spurVersatz, int schrittVersatz)
{
    if (spurVersatz == 0 && schrittVersatz == 0) return false;
    const std::vector<SchrittOrt> quellen = einmal(orte);
    if (quellen.empty()) return false;
    // erst pruefen: jeder muss es geben, und jeder muss am Ziel Platz im Muster haben
    for (const SchrittOrt& ort : quellen)
        if (schrittIn(muster, ort) == nullptr ||
            schrittIn(muster, {ort.spur + spurVersatz, ort.schritt + schrittVersatz}) == nullptr)
            return false;
    // herausnehmen, dann schreiben — so stoert es nicht, wenn Quelle und Ziel sich ueberdecken
    std::vector<double> werte;
    werte.reserve(quellen.size());
    for (const SchrittOrt& ort : quellen)
    {
        double* wert = schrittIn(muster, ort);
        werte.push_back(*wert);
        *wert = 0.0;
    }
    std::vector<SchrittOrt> ziele = quellen;
    for (std::size_t i = 0; i < ziele.size(); ++i)
    {
        ziele[i].spur += spurVersatz;
        ziele[i].schritt += schrittVersatz;
        *schrittIn(muster, ziele[i]) = werte[i];
    }
    orte = std::move(ziele);
    return true;
}

bool dreheSchritte(Muster& muster, std::vector<SchrittOrt>& orte, int schrittVersatz, int schritteImMuster)
{
    if (schritteImMuster <= 0) return false;
    const int versatz = ((schrittVersatz % schritteImMuster) + schritteImMuster) % schritteImMuster;
    if (versatz == 0) return false;
    const std::vector<SchrittOrt> quellen = einmal(orte);
    if (quellen.empty()) return false;
    for (const SchrittOrt& ort : quellen)
        if (schrittIn(muster, ort) == nullptr || ort.schritt >= schritteImMuster) return false;
    std::vector<double> werte;
    for (const SchrittOrt& ort : quellen)
    {
        double* wert = schrittIn(muster, ort);
        werte.push_back(*wert);
        *wert = 0.0;
    }
    std::vector<SchrittOrt> ziele = quellen;
    for (std::size_t i = 0; i < ziele.size(); ++i)
    {
        ziele[i].schritt = (ziele[i].schritt + versatz) % schritteImMuster;
        if (double* wert = schrittIn(muster, ziele[i])) *wert = werte[i];
    }
    orte = std::move(ziele);
    return true;
}

// --- Noten -------------------------------------------------------------------

int findeNote(const Muster& muster, NotenOrt ort)
{
    for (std::size_t i = 0; i < muster.noten.size(); ++i)
        if (muster.noten[i].schritt == ort.schritt && muster.noten[i].hoehe == ort.hoehe) return static_cast<int>(i);
    return -1;
}

NotenKopie kopiereNoten(const Muster& muster, const std::vector<NotenOrt>& orte)
{
    NotenKopie kopie;
    for (const NotenOrt& ort : einmal(orte))
        if (const int platz = findeNote(muster, ort); platz >= 0)
            kopie.noten.push_back(muster.noten[static_cast<std::size_t>(platz)]);
    if (kopie.noten.empty()) return kopie;
    int erster = kopie.noten.front().schritt;
    for (const Note& note : kopie.noten) erster = std::min(erster, note.schritt);
    for (Note& note : kopie.noten)
    {
        note.schritt -= erster;
        kopie.schritte = std::max(kopie.schritte, note.schritt + note.laenge);
    }
    std::stable_sort(kopie.noten.begin(), kopie.noten.end(), [](const Note& a, const Note& b) {
        return a.schritt != b.schritt ? a.schritt < b.schritt : a.hoehe < b.hoehe;
    });
    return kopie;
}

NotenRest fuegeNotenEin(Muster& muster, const NotenKopie& kopie, int schritt, int schritteImMuster,
                        std::vector<NotenOrt>* eingefuegt)
{
    NotenRest rest;
    if (eingefuegt != nullptr) eingefuegt->clear();
    for (Note note : kopie.noten)
    {
        note.schritt += schritt;
        if (note.schritt < 0 || note.schritt >= schritteImMuster)
        {
            ++rest.weg;
            continue;
        }
        if (note.schritt + note.laenge > schritteImMuster)
        {
            note.laenge = schritteImMuster - note.schritt;
            ++rest.gekuerzt;
        }
        // je Schritt und Tonhoehe gibt es eine Note: die neue ersetzt die alte
        const NotenOrt ort{note.schritt, note.hoehe};
        if (const int platz = findeNote(muster, ort); platz >= 0)
            muster.noten[static_cast<std::size_t>(platz)] = note;
        else
            muster.noten.push_back(note);
        if (eingefuegt != nullptr) eingefuegt->push_back(ort);
    }
    ordneNoten(muster);
    return rest;
}

bool loescheNoten(Muster& muster, const std::vector<NotenOrt>& orte)
{
    const auto vorher = muster.noten.size();
    muster.noten.erase(std::remove_if(muster.noten.begin(), muster.noten.end(),
                                      [&orte](const Note& note) {
                                          return std::find(orte.begin(), orte.end(), NotenOrt{note.schritt, note.hoehe}) !=
                                                 orte.end();
                                      }),
                       muster.noten.end());
    return muster.noten.size() != vorher;
}

bool versetzeNoten(Muster& muster, std::vector<NotenOrt>& orte, int schrittVersatz, int hoeheVersatz,
                   int schritteImMuster)
{
    if (schrittVersatz == 0 && hoeheVersatz == 0) return false;
    const std::vector<NotenOrt> quellen = einmal(orte);
    if (quellen.empty()) return false;
    // erst pruefen: keine darf ueber einen Rand geraten
    std::vector<Note> bewegt;
    for (const NotenOrt& ort : quellen)
    {
        const int platz = findeNote(muster, ort);
        if (platz < 0) return false;
        Note note = muster.noten[static_cast<std::size_t>(platz)];
        note.schritt += schrittVersatz;
        note.hoehe += hoeheVersatz;
        if (note.schritt < 0 || note.schritt + note.laenge > schritteImMuster || note.hoehe < kHoeheMin ||
            note.hoehe > kHoeheMax)
            return false;
        bewegt.push_back(note);
    }
    (void)loescheNoten(muster, quellen);
    std::vector<NotenOrt> ziele;
    for (const Note& note : bewegt)
    {
        const NotenOrt ort{note.schritt, note.hoehe};
        if (const int platz = findeNote(muster, ort); platz >= 0)
            muster.noten[static_cast<std::size_t>(platz)] = note;  // eine nicht markierte dort wird ersetzt
        else
            muster.noten.push_back(note);
        ziele.push_back(ort);
    }
    ordneNoten(muster);
    orte = std::move(ziele);
    return true;
}

bool dreheNoten(Muster& muster, std::vector<NotenOrt>& orte, int schrittVersatz, int schritteImMuster, int* gekuerzt)
{
    if (gekuerzt != nullptr) *gekuerzt = 0;
    if (schritteImMuster <= 0) return false;
    const int versatz = ((schrittVersatz % schritteImMuster) + schritteImMuster) % schritteImMuster;
    if (versatz == 0) return false;
    const std::vector<NotenOrt> quellen = einmal(orte);
    if (quellen.empty()) return false;
    std::vector<Note> bewegt;
    for (const NotenOrt& ort : quellen)
    {
        const int platz = findeNote(muster, ort);
        if (platz < 0) return false;
        Note note = muster.noten[static_cast<std::size_t>(platz)];
        note.schritt = (note.schritt + versatz) % schritteImMuster;
        if (note.schritt + note.laenge > schritteImMuster)
        {
            note.laenge = schritteImMuster - note.schritt;
            if (gekuerzt != nullptr) ++*gekuerzt;
        }
        bewegt.push_back(note);
    }
    (void)loescheNoten(muster, quellen);
    std::vector<NotenOrt> ziele;
    for (const Note& note : bewegt)
    {
        const NotenOrt ort{note.schritt, note.hoehe};
        if (const int platz = findeNote(muster, ort); platz >= 0)
            muster.noten[static_cast<std::size_t>(platz)] = note;
        else
            muster.noten.push_back(note);
        ziele.push_back(ort);
    }
    ordneNoten(muster);
    orte = std::move(ziele);
    return true;
}

}  // namespace lumi::loop
