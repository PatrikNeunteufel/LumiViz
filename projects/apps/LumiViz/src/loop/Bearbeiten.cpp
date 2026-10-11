/**
 * @file Bearbeiten.cpp
 * @brief Loop: das Arrangement bearbeiten
 */

#include "loop/Bearbeiten.hpp"

#include <algorithm>
#include <cstdlib>
#include <vector>

namespace lumi::loop
{

namespace
{

bool gibtKanal(const Loop& loop, int kanal)
{
    return kanal >= 0 && kanal < static_cast<int>(loop.kanaele.size());
}

bool gibtClip(const Loop& loop, int kanal, int clip)
{
    return gibtKanal(loop, kanal) &&
           clip >= 0 && clip < static_cast<int>(loop.kanaele[static_cast<std::size_t>(kanal)].clips.size());
}

/// Die Clips des Kanals nach Start sortieren; gibt den Platz des Clips zurueck,
/// der bei `startTakt` beginnt.
int ordne(Kanal& kanal, int startTakt)
{
    std::stable_sort(kanal.clips.begin(), kanal.clips.end(),
                     [](const LoopClip& a, const LoopClip& b) { return a.startTakt < b.startTakt; });
    for (std::size_t i = 0; i < kanal.clips.size(); ++i)
        if (kanal.clips[i].startTakt == startTakt) return static_cast<int>(i);
    return -1;
}

/// Basen entfernen, die kein Clip mehr spielt.
void raeumeBasen(Loop& loop)
{
    loop.muster.erase(std::remove_if(loop.muster.begin(), loop.muster.end(),
                                     [&loop](const Muster& m) { return basisNutzer(loop, m.name) == 0; }),
                      loop.muster.end());
}

/// Der erste Name aus `vorsatz` und einer Zahl ab 1, den `vergeben` nicht kennt.
template <typename Vergeben>
QString freierName(const QString& vorsatz, Vergeben vergeben)
{
    for (int nr = 1;; ++nr)
    {
        const QString name = QStringLiteral("%1 %2").arg(vorsatz).arg(nr);
        if (!vergeben(name)) return name;
    }
}

}  // namespace

bool istFrei(const Loop& loop, int kanal, int startTakt, int takte, int ausser)
{
    if (!gibtKanal(loop, kanal) || startTakt < 0 || takte <= 0) return false;
    const auto& clips = loop.kanaele[static_cast<std::size_t>(kanal)].clips;
    for (int c = 0; c < static_cast<int>(clips.size()); ++c)
    {
        if (c == ausser) continue;
        const LoopClip& anderer = clips[static_cast<std::size_t>(c)];
        if (startTakt < anderer.startTakt + loop.clipTakte(anderer) && anderer.startTakt < startTakt + takte)
            return false;
    }
    return true;
}

int basisNutzer(const Loop& loop, const QString& muster)
{
    int zahl = 0;
    for (const Kanal& kanal : loop.kanaele)
        for (const LoopClip& clip : kanal.clips)
            if (clip.muster == muster) ++zahl;
    return zahl;
}

QString folgeBasis(const Loop& alt, const QString& name, const Loop& neu)
{
    if (neu.findeMuster(name) != nullptr) return name;
    const Muster* davor = alt.findeMuster(name);
    if (davor == nullptr) return {};
    QString folge;
    for (const Muster& m : neu.muster)
    {
        if (alt.findeMuster(m.name) != nullptr) continue;
        Muster gleich = m;
        gleich.name = davor->name;
        if (gleich == *davor) folge = m.name;
    }
    return folge;
}

int neuerClip(Loop& loop, int kanal, int startTakt, MusterArt art)
{
    if (!istFrei(loop, kanal, startTakt, kTakteMin)) return -1;
    Muster basis;
    basis.name = freierName(QStringLiteral("Pattern"),
                            [&loop](const QString& name) { return loop.findeMuster(name) != nullptr; });
    basis.art = art;
    if (art == MusterArt::Instrument) basis.instrument = instrumentVorgabe();
    basis.takte = kTakteMin;
    loop.muster.push_back(basis);
    LoopClip clip;
    clip.muster = basis.name;
    clip.startTakt = startTakt;
    Kanal& ziel = loop.kanaele[static_cast<std::size_t>(kanal)];
    ziel.clips.push_back(clip);
    return ordne(ziel, startTakt);
}

int versetzeClip(Loop& loop, int kanal, int clip, int zielKanal, int startTakt)
{
    if (!gibtClip(loop, kanal, clip) || !gibtKanal(loop, zielKanal)) return -1;
    const LoopClip bewegt = loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    const int takte = loop.clipTakte(bewegt);
    const int wunsch = std::max(0, startTakt);
    const int ausser = zielKanal == kanal ? clip : -1;

    // der Wunsch selbst — und die Kanten der Clips im Zielkanal, davor und dahinter
    std::vector<int> stellen = {wunsch};
    const auto& andere = loop.kanaele[static_cast<std::size_t>(zielKanal)].clips;
    for (int c = 0; c < static_cast<int>(andere.size()); ++c)
    {
        if (c == ausser) continue;
        const LoopClip& anderer = andere[static_cast<std::size_t>(c)];
        stellen.push_back(anderer.startTakt + loop.clipTakte(anderer));
        stellen.push_back(anderer.startTakt - takte);
    }
    int beste = -1;
    for (const int stelle : stellen)
    {
        if (!istFrei(loop, zielKanal, stelle, takte, ausser)) continue;
        if (beste < 0 || std::abs(stelle - wunsch) < std::abs(beste - wunsch)) beste = stelle;
    }
    if (beste < 0) return -1;

    auto& quelle = loop.kanaele[static_cast<std::size_t>(kanal)].clips;
    quelle.erase(quelle.begin() + clip);
    LoopClip neu = bewegt;
    neu.startTakt = beste;
    Kanal& ziel = loop.kanaele[static_cast<std::size_t>(zielKanal)];
    ziel.clips.push_back(neu);
    return ordne(ziel, beste);
}

int setzeEnde(Loop& loop, int kanal, int clip, int endeTakt)
{
    if (!gibtClip(loop, kanal, clip)) return -1;
    auto& clips = loop.kanaele[static_cast<std::size_t>(kanal)].clips;
    LoopClip& ziel = clips[static_cast<std::size_t>(clip)];
    const Muster* basis = loop.findeMuster(ziel.muster);
    if (basis == nullptr) return -1;
    const int takte = basis->takte;
    // so oft, dass das Ende moeglichst nah an der Maus liegt
    int mal = (endeTakt - ziel.startTakt + takte / 2) / takte;
    if (clip + 1 < static_cast<int>(clips.size()))
        mal = std::min(mal, (clips[static_cast<std::size_t>(clip) + 1].startTakt - ziel.startTakt) / takte);
    ziel.wiederholungen = std::max(1, mal);
    return clip;
}

int setzeAnfang(Loop& loop, int kanal, int clip, int startTakt)
{
    if (!gibtClip(loop, kanal, clip)) return -1;
    auto& clips = loop.kanaele[static_cast<std::size_t>(kanal)].clips;
    LoopClip& ziel = clips[static_cast<std::size_t>(clip)];
    const Muster* basis = loop.findeMuster(ziel.muster);
    if (basis == nullptr) return -1;
    const int takte = basis->takte;
    const int ende = ziel.startTakt + loop.clipTakte(ziel);
    const int davor = clip > 0 ? clips[static_cast<std::size_t>(clip) - 1].startTakt +
                                     loop.clipTakte(clips[static_cast<std::size_t>(clip) - 1])
                               : 0;
    int mal = (ende - startTakt + takte / 2) / takte;
    mal = std::clamp(mal, 1, std::max(1, (ende - davor) / takte));
    ziel.wiederholungen = mal;
    ziel.startTakt = ende - mal * takte;
    return clip;
}

int dupliziere(Loop& loop, int kanal, int clip)
{
    if (!gibtClip(loop, kanal, clip)) return -1;
    Kanal& ziel = loop.kanaele[static_cast<std::size_t>(kanal)];
    const LoopClip vorlage = ziel.clips[static_cast<std::size_t>(clip)];
    const int takte = loop.clipTakte(vorlage);
    // hinter der Vorlage — oder hinter dem ersten spaeteren Clip, nach dem sie passt
    int stelle = -1;
    for (int c = clip; c < static_cast<int>(ziel.clips.size()) && stelle < 0; ++c)
    {
        const LoopClip& nach = ziel.clips[static_cast<std::size_t>(c)];
        const int kante = nach.startTakt + loop.clipTakte(nach);
        if (istFrei(loop, kanal, kante, takte)) stelle = kante;
    }
    if (stelle < 0) return -1;
    LoopClip kopie = vorlage;
    kopie.startTakt = stelle;
    ziel.clips.push_back(kopie);
    return ordne(ziel, stelle);
}

bool loescheClip(Loop& loop, int kanal, int clip)
{
    if (!gibtClip(loop, kanal, clip)) return false;
    auto& clips = loop.kanaele[static_cast<std::size_t>(kanal)].clips;
    clips.erase(clips.begin() + clip);
    raeumeBasen(loop);
    return true;
}

int findeClip(const Loop& loop, ClipOrt ort)
{
    if (!gibtKanal(loop, ort.kanal)) return -1;
    const auto& clips = loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips;
    for (std::size_t c = 0; c < clips.size(); ++c)
        if (clips[c].startTakt == ort.startTakt) return static_cast<int>(c);
    return -1;
}

namespace
{

/// Die genannten Clips aus dem Loop nehmen (jeden einmal); false = einen gibt
/// es nicht. `orte` und `clips` laufen danach gleich.
bool nimmHeraus(Loop& loop, std::vector<ClipOrt>& orte, std::vector<LoopClip>& clips)
{
    std::vector<ClipOrt> einmal;
    for (const ClipOrt& ort : orte)
        if (std::find(einmal.begin(), einmal.end(), ort) == einmal.end()) einmal.push_back(ort);
    clips.clear();
    for (const ClipOrt& ort : einmal)
    {
        const int platz = findeClip(loop, ort);
        if (platz < 0) return false;
        auto& liste = loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips;
        clips.push_back(liste[static_cast<std::size_t>(platz)]);
        liste.erase(liste.begin() + platz);
    }
    orte = std::move(einmal);
    return true;
}

/// Die Clips an ihre Orte legen; false = an einem ist kein Platz.
bool legeHinein(Loop& loop, const std::vector<ClipOrt>& orte, const std::vector<LoopClip>& clips)
{
    for (std::size_t i = 0; i < orte.size(); ++i)
    {
        if (!istFrei(loop, orte[i].kanal, orte[i].startTakt, loop.clipTakte(clips[i]))) return false;
        LoopClip clip = clips[i];
        clip.startTakt = orte[i].startTakt;
        Kanal& ziel = loop.kanaele[static_cast<std::size_t>(orte[i].kanal)];
        ziel.clips.push_back(clip);
        (void)ordne(ziel, clip.startTakt);
    }
    return true;
}

}  // namespace

bool versetzeClips(Loop& loop, std::vector<ClipOrt>& orte, int kanalVersatz, int taktVersatz)
{
    if (orte.empty()) return false;
    Loop probe = loop;
    std::vector<ClipOrt> ziele = orte;
    std::vector<LoopClip> clips;
    if (!nimmHeraus(probe, ziele, clips)) return false;
    // der Versatz, so weit er fuer alle geht
    int ersterTakt = ziele.front().startTakt;
    int ersterKanal = ziele.front().kanal;
    int letzterKanal = ziele.front().kanal;
    for (const ClipOrt& ort : ziele)
    {
        ersterTakt = std::min(ersterTakt, ort.startTakt);
        ersterKanal = std::min(ersterKanal, ort.kanal);
        letzterKanal = std::max(letzterKanal, ort.kanal);
    }
    const int kanaele = static_cast<int>(loop.kanaele.size());
    taktVersatz = std::max(taktVersatz, -ersterTakt);
    kanalVersatz = std::clamp(kanalVersatz, -ersterKanal, kanaele - 1 - letzterKanal);
    if (taktVersatz == 0 && kanalVersatz == 0) return false;
    for (ClipOrt& ort : ziele)
    {
        ort.kanal += kanalVersatz;
        ort.startTakt += taktVersatz;
    }
    if (!legeHinein(probe, ziele, clips)) return false;
    loop = std::move(probe);
    orte = std::move(ziele);
    return true;
}

bool dupliziereClips(Loop& loop, std::vector<ClipOrt>& orte)
{
    if (orte.empty()) return false;
    Loop ohne = loop;
    std::vector<ClipOrt> vorlagen = orte;
    std::vector<LoopClip> clips;
    if (!nimmHeraus(ohne, vorlagen, clips)) return false;
    int anfang = vorlagen.front().startTakt;
    int ende = 0;
    for (std::size_t i = 0; i < vorlagen.size(); ++i)
    {
        anfang = std::min(anfang, vorlagen[i].startTakt);
        ende = std::max(ende, vorlagen[i].startTakt + loop.clipTakte(clips[i]));
    }
    // hinter dem Ende des ganzen Loops ist immer Platz: dort endet die Suche
    const int weitester = std::max(ende, loop.durchlaufTakte()) - anfang;
    for (int versatz = ende - anfang; versatz <= weitester; ++versatz)
    {
        Loop probe = loop;
        std::vector<ClipOrt> kopien = vorlagen;
        for (ClipOrt& ort : kopien) ort.startTakt += versatz;
        if (!legeHinein(probe, kopien, clips)) continue;
        loop = std::move(probe);
        orte = std::move(kopien);
        return true;
    }
    return false;
}

bool loescheClips(Loop& loop, const std::vector<ClipOrt>& orte)
{
    if (orte.empty()) return false;
    Loop probe = loop;
    std::vector<ClipOrt> weg = orte;
    std::vector<LoopClip> clips;
    if (!nimmHeraus(probe, weg, clips)) return false;
    raeumeBasen(probe);
    loop = std::move(probe);
    return true;
}

bool benenneBasisUm(Loop& loop, const QString& alt, const QString& neu)
{
    const QString name = neu.trimmed();
    if (name.isEmpty() || name == alt || loop.findeMuster(name) != nullptr) return false;
    const auto basis =
        std::find_if(loop.muster.begin(), loop.muster.end(), [&alt](const Muster& m) { return m.name == alt; });
    if (basis == loop.muster.end()) return false;
    basis->name = name;
    for (Kanal& kanal : loop.kanaele)
        for (LoopClip& clip : kanal.clips)
            if (clip.muster == alt) clip.muster = name;
    return true;
}

Muster* musterIn(Loop& loop, const QString& name)
{
    const auto it =
        std::find_if(loop.muster.begin(), loop.muster.end(), [&name](const Muster& m) { return m.name == name; });
    return it == loop.muster.end() ? nullptr : &*it;
}

QString neueBasisAus(Loop& loop, int kanal, int clip)
{
    if (!gibtClip(loop, kanal, clip)) return {};
    LoopClip& ziel = loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    const Muster* basis = loop.findeMuster(ziel.muster);
    if (basis == nullptr) return {};
    Muster kopie = *basis;
    kopie.name = QString();
    for (int nr = 2; kopie.name.isEmpty(); ++nr)
    {
        const QString name = QStringLiteral("%1 %2").arg(basis->name).arg(nr);
        if (loop.findeMuster(name) == nullptr) kopie.name = name;
    }
    const QString alt = ziel.muster;
    ziel.muster = kopie.name;
    loop.muster.push_back(kopie);  // `basis` zeigt ab hier nicht mehr sicher auf das Original
    const QString neu = loop.muster.back().name;
    if (basisNutzer(loop, alt) == 0) raeumeBasen(loop);  // der Clip war der einzige: das Original geht
    return neu;
}

bool setzeMusterTakte(Loop& loop, const QString& muster, int takte)
{
    Muster* ziel = musterIn(loop, muster);
    if (ziel == nullptr) return false;
    ziel->takte = std::clamp(takte, kTakteMin, kTakteMax);
    // Zeilen auf die neue Laenge, und Clips, die jetzt ineinander reichten, ruecken nach hinten
    loop.richte();
    return true;
}

bool setzeRaster(Loop& loop, int schlaegeJeTakt, int schritteJeSchlag)
{
    const int schlaege = std::clamp(schlaegeJeTakt, 1, kRasterMax);
    const int schritte = std::clamp(schritteJeSchlag, 1, kRasterMax);
    const int alteSchlaege = loop.schlaegeJeTakt;
    const int alteSchritte = loop.schritteJeSchlag;
    if (schlaege == alteSchlaege && schritte == alteSchritte) return false;
    for (Muster& m : loop.muster)
    {
        // Noten: der Einsatz bleibt an seiner Stelle im Takt, wo das neue Raster
        // sie kennt — sonst geht die Note; die Laenge bleibt dieselbe Zeit, auf
        // ganze Schritte gerundet
        std::vector<Note> noten;
        for (const Note& note : m.noten)
        {
            const int takt = note.schritt / (alteSchlaege * alteSchritte);
            const int imTakt = note.schritt % (alteSchlaege * alteSchritte);
            const int schlag = imTakt / alteSchritte;
            const int teil = imTakt % alteSchritte;
            if (schlag >= schlaege || (teil * schritte) % alteSchritte != 0) continue;
            Note neu = note;
            neu.schritt = (takt * schlaege + schlag) * schritte + teil * schritte / alteSchritte;
            neu.laenge = std::max(1, (note.laenge * schritte + alteSchritte / 2) / alteSchritte);
            noten.push_back(neu);
        }
        m.noten = std::move(noten);
        for (std::vector<double>& zeile : m.zeilen)
        {
            std::vector<double> neu(static_cast<std::size_t>(m.takte * schlaege * schritte), 0.0);
            for (int takt = 0; takt < m.takte; ++takt)
                for (int schlag = 0; schlag < std::min(schlaege, alteSchlaege); ++schlag)
                    for (int teil = 0; teil < schritte; ++teil)
                    {
                        // derselbe Bruchteil des Schlags — nur, wenn das alte Raster ihn kannte
                        if ((teil * alteSchritte) % schritte != 0) continue;
                        const auto alt = static_cast<std::size_t>((takt * alteSchlaege + schlag) * alteSchritte +
                                                                  teil * alteSchritte / schritte);
                        if (alt < zeile.size())
                            neu[static_cast<std::size_t>((takt * schlaege + schlag) * schritte + teil)] = zeile[alt];
                    }
            zeile = std::move(neu);
        }
    }
    loop.schlaegeJeTakt = schlaege;
    loop.schritteJeSchlag = schritte;
    loop.richte();  // eine Note, die jetzt ueber das Ende ihres Musters reichte, wird kuerzer
    return true;
}

int spurHinzu(Loop& loop, const QString& muster, const QStringList& kitKlaenge)
{
    Muster* ziel = musterIn(loop, muster);
    if (ziel == nullptr) return -1;
    Spur spur;
    spur.name = freierName(QStringLiteral("Track"), [ziel](const QString& name) {
        return std::any_of(ziel->spuren.begin(), ziel->spuren.end(), [&name](const Spur& s) { return s.name == name; });
    });
    for (const QString& klang : kitKlaenge)
    {
        const bool vergeben = std::any_of(ziel->spuren.begin(), ziel->spuren.end(),
                                          [&klang](const Spur& s) { return s.klang == klang; });
        if (!vergeben)
        {
            spur.klang = klang;
            break;
        }
    }
    ziel->spuren.push_back(spur);
    ziel->zeilen.resize(ziel->spuren.size());
    ziel->zeilen.back().assign(static_cast<std::size_t>(ziel->takte * loop.schritteJeTakt()), 0.0);
    return static_cast<int>(ziel->spuren.size()) - 1;
}

bool spurLoeschen(Loop& loop, const QString& muster, int spur)
{
    Muster* ziel = musterIn(loop, muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return false;
    ziel->spuren.erase(ziel->spuren.begin() + spur);
    if (spur < static_cast<int>(ziel->zeilen.size())) ziel->zeilen.erase(ziel->zeilen.begin() + spur);
    return true;
}

int kanalHinzu(Loop& loop)
{
    Kanal kanal;
    kanal.name = freierName(QStringLiteral("Channel"), [&loop](const QString& name) {
        return std::any_of(loop.kanaele.begin(), loop.kanaele.end(),
                           [&name](const Kanal& k) { return k.name == name; });
    });
    loop.kanaele.push_back(kanal);
    return static_cast<int>(loop.kanaele.size()) - 1;
}

bool kanalLoeschen(Loop& loop, int kanal)
{
    if (!gibtKanal(loop, kanal)) return false;
    loop.kanaele.erase(loop.kanaele.begin() + kanal);
    raeumeBasen(loop);
    return true;
}

bool kanalUmbenennen(Loop& loop, int kanal, const QString& name)
{
    if (!gibtKanal(loop, kanal) || name.trimmed().isEmpty()) return false;
    loop.kanaele[static_cast<std::size_t>(kanal)].name = name.trimmed();
    return true;
}

bool kanalSchalten(Loop& loop, int kanal, bool an)
{
    if (!gibtKanal(loop, kanal)) return false;
    loop.kanaele[static_cast<std::size_t>(kanal)].an = an;
    return true;
}

}  // namespace lumi::loop
