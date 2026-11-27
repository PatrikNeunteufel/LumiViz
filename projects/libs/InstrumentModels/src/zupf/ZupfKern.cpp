/**
 * @file ZupfKern.cpp
 * @brief InstrumentModels, Kern `pluckedString` Rechenfassung 2: Tafel, Werte ↔ Spiel, Adapter, Vorlagen (34k-7d-4 (c);
 *        Herleitung im Kopf von `ZupfKern.hpp`). Die Fassung aus IC (Rechenfassung 1) stand hier bis S224 (ZB1 a).
 */
#include "InstrumentModels/zupf/ZupfKern.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <numbers>
#include <utility>

namespace lumi::kern::zupf
{

namespace
{

constexpr int kUmfangMin = 21;   // A0
constexpr int kUmfangMax = 108;  // C8

constexpr std::string_view kNameNoteMin = "noteMin";
constexpr std::string_view kNameNoteMax = "noteMax";
constexpr std::string_view kSatz = "stringSet";
constexpr std::string_view kKraftWahl = "pluckBy";
constexpr std::string_view kWahlAuslenkung = "displacement";
constexpr std::string_view kWahlKraft = "force";

/// Die Gruppen der Tafel in ihrer Reihenfolge (`werteAus` schreibt in derselben).
constexpr std::string_view kGruppen[] = {"Keys", "Strings", "Pluck", "Tunable", "Output"};

ParameterAngabe angabe(std::string_view name, std::string_view titel, std::string_view gruppe, WertArt art, std::string_view einheit = {},
                       double von = 0.0, double bis = 1.0)
{
    ParameterAngabe a;
    a.name = std::string(name);
    a.titel = std::string(titel);
    a.gruppe = std::string(gruppe);
    a.art = art;
    a.darstellung = darstellungZu(art);
    a.einheit = std::string(einheit);
    a.von = von;
    a.bis = bis;
    a.belegkraft = Belegkraft::Offen;
    a.quellen = {"lumiviz"};
    return a;
}

/// Ein Wert eines Spiels (oder eines Eintrags einer Liste) in der Tafel: Angabe, Lesen, Setzen.
template <typename S>
struct Feld
{
    ParameterAngabe angabe;
    std::function<Wert(const S&)> lies;
    std::function<void(S&, const Wert&)> setze;
};

template <typename S>
Feld<S> zahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
             double S::* feld)
{
    return Feld<S>{angabe(name, titel, gruppe, WertArt::Zahl, einheit, von, bis), [feld](const S& s) { return Wert{s.*feld}; },
                   [feld](S& s, const Wert& w) {
                       if (const auto* d = std::get_if<double>(&w)) s.*feld = *d;
                   }};
}

template <typename S>
Feld<S> ganzzahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
                 int S::* feld)
{
    return Feld<S>{angabe(name, titel, gruppe, WertArt::Ganzzahl, einheit, von, bis),
                   [feld](const S& s) { return Wert{static_cast<std::int64_t>(s.*feld)}; },
                   [feld](S& s, const Wert& w) {
                       if (const auto* g = std::get_if<std::int64_t>(&w)) s.*feld = static_cast<int>(*g);
                   }};
}

Feld<ZupfSpiel> kurve(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von,
                      double bis, Skala skala, Kurve ZupfSpiel::* feld)
{
    Feld<ZupfSpiel> f{angabe(name, titel, gruppe, WertArt::Kurve, einheit, von, bis), [feld](const ZupfSpiel& s) { return Wert{s.*feld}; },
                      [feld](ZupfSpiel& s, const Wert& w) {
                          if (const auto* k = std::get_if<Kurve>(&w)) s.*feld = *k;
                      }};
    f.angabe.skala = skala;
    return f;
}

/// Eine Saite des Satzes als Eintrag der Liste `stringSet`.
struct Satzsaite
{
    int chor = 0;
    int leer = 40;
    int oktave = 0;
    double laengenFaktor = 1.0;
    double zug = 70.0, masse = 5e-4, biegesteife = 0.0, verlustBiegung = 0.0, verlustInnen = 0.0, verlustLuft = 0.0, durchmesser = 0.0,
           dichte = 0.0;
};

const std::vector<Feld<Satzsaite>>& satzFelder()
{
    static const std::vector<Feld<Satzsaite>> f = {
        ganzzahl<Satzsaite>("course", "Course (from the lowest, as the app counts strings)", "", "", 0, 31, &Satzsaite::chor),
        ganzzahl<Satzsaite>("open", "Open string", "", "MIDI", kUmfangMin, kUmfangMax, &Satzsaite::leer),
        ganzzahl<Satzsaite>("octave", "Above the course", "", "semitones", 0, 24, &Satzsaite::oktave),
        zahl<Satzsaite>("lengthFactor", "Length factor (unison detuning)", "", "", 0.9, 1.1, &Satzsaite::laengenFaktor),
        zahl<Satzsaite>("tension", "Tension", "", "N", 0.1, 5000.0, &Satzsaite::zug),
        zahl<Satzsaite>("mass", "Mass per length", "", "kg/m", 1e-7, 1.0, &Satzsaite::masse),
        zahl<Satzsaite>("bendingStiffness", "Bending stiffness EI", "", "N m²", 0.0, 1.0, &Satzsaite::biegesteife),
        zahl<Satzsaite>("lossBending", "Loss factor of bending η_B", "", "", 0.0, 1.0, &Satzsaite::verlustBiegung),
        zahl<Satzsaite>("lossInternal", "Internal loss η_F (Valette)", "", "", 0.0, 1.0, &Satzsaite::verlustInnen),
        zahl<Satzsaite>("lossAir", "Air loss η_A (Valette)", "", "1/s", 0.0, 100.0, &Satzsaite::verlustLuft),
        zahl<Satzsaite>("diameter", "Diameter (monofilament; 0 = Valette)", "", "m", 0.0, 0.01, &Satzsaite::durchmesser),
        zahl<Satzsaite>("density", "Density (monofilament)", "", "kg/m³", 0.0, 30000.0, &Satzsaite::dichte),
    };
    return f;
}

Liste satzAlsListe(const Saitensatz& satz)
{
    Liste l;
    for (std::size_t c = 0; c < satz.choere.size(); ++c)
        for (const Chorsaite& cs : satz.choere[c].saiten)
        {
            const Satzsaite x{static_cast<int>(c), satz.choere[c].leer, cs.oktave, cs.laengenFaktor, cs.werte.zug, cs.werte.masse,
                              cs.werte.biegesteife, cs.werte.verlustBiegung, cs.werte.verlustInnen, cs.werte.verlustLuft,
                              cs.werte.durchmesser, cs.werte.dichte};
            Werte e;
            for (const Feld<Satzsaite>& f : satzFelder()) e.felder.emplace_back(f.angabe.name, f.lies(x));
            l.eintraege.push_back(std::move(e));
        }
    return l;
}

/// Die Choere aus der Liste (nach `course`, je Chor die Saiten in der Reihenfolge der Liste); Mensur und Buende setzt der
/// Rufer.
std::vector<Saitenchor> choereAus(const Liste& l)
{
    std::map<int, Saitenchor> choere;
    for (const Werte& e : l.eintraege)
    {
        Satzsaite x;
        for (const Feld<Satzsaite>& f : satzFelder())
            if (const Wert* w = e.finde(f.angabe.name)) f.setze(x, *w);
        Saitenchor& c = choere[x.chor];
        c.leer = x.leer;
        Chorsaite cs;
        cs.werte = {x.zug, x.masse, x.biegesteife, x.verlustBiegung, x.verlustInnen, x.verlustLuft, x.durchmesser, x.dichte};
        cs.oktave = x.oktave;
        cs.laengenFaktor = x.laengenFaktor;
        c.saiten.push_back(cs);
    }
    std::vector<Saitenchor> v;
    for (auto& [_, c] : choere) v.push_back(std::move(c));
    return v;
}

/// Alle Werte des Spiels in der Reihenfolge der Tafel (Gruppe fuer Gruppe), ausser dem Bezug.
const std::vector<Feld<ZupfSpiel>>& felder()
{
    static const std::vector<Feld<ZupfSpiel>> f = [] {
        std::vector<Feld<ZupfSpiel>> v = {
            ganzzahl<ZupfSpiel>(kNameNoteMin, "Lowest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &ZupfSpiel::noteMin),
            ganzzahl<ZupfSpiel>(kNameNoteMax, "Highest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &ZupfSpiel::noteMax),
            kurve("stringLength", "String length per key", "Strings", "m", 0.01, 10.0, Skala::Linear, &ZupfSpiel::laenge),
            kurve("stringTension", "String tension per key", "Strings", "N", 0.1, 5000.0, Skala::Linear, &ZupfSpiel::zug),
            kurve("stringMass", "Mass per length per key", "Strings", "kg/m", 1e-7, 1.0, Skala::Linear, &ZupfSpiel::masse),
            kurve("bendingStiffness", "Bending stiffness EI per key", "Strings", "N m²", 0.0, 1.0, Skala::Linear, &ZupfSpiel::biegesteife),
            kurve("lossBending", "Loss factor of bending η_B per key", "Strings", "", 0.0, 1.0, Skala::Linear, &ZupfSpiel::verlustBiegung),
            kurve("lossInternal", "Internal loss η_F per key (Valette)", "Strings", "", 0.0, 1.0, Skala::Linear, &ZupfSpiel::verlustInnen),
            kurve("lossAir", "Air loss η_A per key (Valette)", "Strings", "1/s", 0.0, 100.0, Skala::Linear, &ZupfSpiel::verlustLuft),
            kurve("stringDiameter", "Diameter per key (monofilament; 0 = Valette)", "Strings", "m", 0.0, 0.01, Skala::Linear,
                  &ZupfSpiel::durchmesser),
            kurve("stringDensity", "Density per key (monofilament)", "Strings", "kg/m³", 0.0, 30000.0, Skala::Linear, &ZupfSpiel::dichte),
        };
        Feld<ZupfSpiel> satz{angabe(kSatz, "String set", "Strings", WertArt::Liste),
                             [](const ZupfSpiel& s) { return Wert{satzAlsListe(s.satz)}; },
                             [](ZupfSpiel& s, const Wert& w) {
                                 if (const Liste* l = std::get_if<Liste>(&w)) s.satz.choere = choereAus(*l);
                             }};
        for (const Feld<Satzsaite>& e : satzFelder())
        {
            ParameterAngabe a = e.angabe;
            a.vorgabe = e.lies(Satzsaite{});
            satz.angabe.felder.push_back(std::move(a));
        }
        satz.angabe.eintraegeMin = 0;
        satz.angabe.eintraegeMax = 64;
        v.push_back(std::move(satz));
        v.push_back(Feld<ZupfSpiel>{angabe("scaleLength", "Scale length", "Strings", WertArt::Zahl, "m", 0.0, 2.0),
                                    [](const ZupfSpiel& s) { return Wert{s.satz.mensur}; },
                                    [](ZupfSpiel& s, const Wert& w) {
                                        if (const auto* d = std::get_if<double>(&w)) s.satz.mensur = *d;
                                    }});
        v.push_back(Feld<ZupfSpiel>{angabe("frets", "Frets", "Strings", WertArt::Ganzzahl, "", 0, 36),
                                    [](const ZupfSpiel& s) { return Wert{static_cast<std::int64_t>(s.satz.buende)}; },
                                    [](ZupfSpiel& s, const Wert& w) {
                                        if (const auto* g = std::get_if<std::int64_t>(&w)) s.satz.buende = static_cast<int>(*g);
                                    }});
        v.push_back(kurve("pluckPoint", "Plucking point per key (from the bridge)", "Pluck", "m", 0.001, 5.0, Skala::Linear, &ZupfSpiel::zupfort));
        Feld<ZupfSpiel> wahl{angabe(kKraftWahl, "Pluck by", "Pluck", WertArt::Wahl),
                             [](const ZupfSpiel& s) { return Wert{std::string(s.kraftGegeben ? kWahlKraft : kWahlAuslenkung)}; },
                             [](ZupfSpiel& s, const Wert& w) {
                                 if (const auto* t = std::get_if<std::string>(&w)) s.kraftGegeben = *t == kWahlKraft;
                             }};
        wahl.angabe.wahlen = {std::string(kWahlAuslenkung), std::string(kWahlKraft)};
        v.push_back(std::move(wahl));
        v.push_back(zahl<ZupfSpiel>("pluckDisplacement", "Displacement at release (level 0)", "Pluck", "m", 1e-6, 0.05, &ZupfSpiel::auslenkung));
        v.push_back(kurve("pluckForce", "Force at release per key (level 0)", "Pluck", "N", 1e-4, 1000.0, Skala::Logarithmisch, &ZupfSpiel::kraft));
        v.push_back(zahl<ZupfSpiel>("levelPerStep", "Displacement or force per level", "Pluck", "dB/dB", 0.0, 2.0, &ZupfSpiel::pegelJeStufe));
        v.push_back(zahl<ZupfSpiel>("tiltPerStep", "Tilt per level around 1 kHz", "Pluck", "dB/oct/dB", -1.0, 1.0, &ZupfSpiel::neigungJeStufe));
        v.push_back(zahl<ZupfSpiel>("pluckAngle", "Angle at release", "Pluck", "°", 0.0, 90.0, &ZupfSpiel::winkelGrad));
        v.push_back(zahl<ZupfSpiel>("fingerWidth", "Width of finger or plectrum", "Pluck", "m", 0.0, 0.05, &ZupfSpiel::breite));
        v.push_back(zahl<ZupfSpiel>("releaseSpeed", "Speed at release (level 0)", "Pluck", "m/s", 0.0, 20.0, &ZupfSpiel::tempo));
        v.push_back(zahl<ZupfSpiel>("dampingTime", "Damping at the end of the note (−60 dB)", "Pluck", "s", 1e-3, 10.0, &ZupfSpiel::daempfenS));
        Feld<ZupfSpiel> umlauf{umlaufTafelAngabe(),
                               [](const ZupfSpiel& s) { return Wert{umlaufTafelListe(s.umlauf)}; },
                               [](ZupfSpiel& s, const Wert& w) {
                                   if (const Liste* l = std::get_if<Liste>(&w)) s.umlauf = umlaufTafelEintraege(*l);
                               }};
        umlauf.angabe.gruppe = "Tunable";
        v.push_back(std::move(umlauf));
        return v;
    }();
    return f;
}

/// Belegkraft und Herkunft der Vorgaben, die keine Kurve sind (tafelweit, der Text nennt die fuenf Instrumente).
void belege(ParameterAngabe& a)
{
    const auto setze = [&](Belegkraft b, std::string h) {
        a.belegkraft = b;
        a.herkunft = std::move(h);
    };
    const std::string& n = a.name;
    if (n == kNameNoteMin || n == kNameNoteMax)
        setze(Belegkraft::Quelle, "harpsichord FF–f3 (Taskin 1769), concert harp the keys with a bridge point 25–104 (KQ22 a), guitar, lute, "
                                  "oud from the lowest open string to the highest fret of the highest");
    else if (n == kSatz || n == "scaleLength" || n == "frets")
        setze(Belegkraft::Quelle, "guitar: Woodhouse et al. 2012 Tab. I set (a) nylon, scale 650 mm, 19 frets; lute: 8 courses in G after "
                                  "Woodhouse & Lynch-Aird 2019 Tab. II, 625 mm, 12 frets, unison ±2 cent; oud: D'Addario EJ95A, 600 mm "
                                  "(assumption), 12 frets, unison ±1.5 cent (GZ1-1 a, KQ4 a, KQ5 a); empty for harp and harpsichord");
    else if (n == kKraftWahl)
        setze(Belegkraft::Quelle, "the harpsichord's plectrum releases at a force, not a displacement (GZ13 a, Perng 2011); the others by "
                                  "displacement");
    else if (n == "pluckDisplacement")
        setze(Belegkraft::Quelle, "guitar 4 mm at 16 cm (Pavlidou 1997 after Scherrer 2013 Tab. 2.1, GZ6 a); lute, oud 2 mm (transferred); "
                                  "harp 4 mm (Chadefaux et al. 2012)");
    else if (n == "levelPerStep")
        setze(Belegkraft::Gemessen, "guitar 1 (GZ6 a, Iowa MIS); harp 1 (ZD1 a, ZD1-1 a: VCSL f against mf); lute, oud 1 (transferred, "
                                    "ZD2 a); harpsichord 1/18 — 1 dB less down to pp (GZ10 a)");
    else if (n == "tiltPerStep")
        setze(Belegkraft::Gemessen, "guitar 0.16 dB per octave per dB level around 1 kHz (GZ6 a, fitted to Iowa MIS); lute, oud 0.16 "
                                    "(transferred, ZD2 a); harp 0 (ZD1-1 a: VCSL, not separable from 0, k −0.15 … +0.06); harpsichord 0");
    else if (n == "pluckAngle")
        setze(Belegkraft::Quelle, "harpsichord 20° (Chadefaux et al. 2013 Tab. 1: 20 ± 1° fast); harp 25° (Chadefaux et al. 2012); guitar, "
                                  "lute, oud 30° (rest/free stroke after Scherrer & Depalle)");
    else if (n == "fingerWidth")
        setze(Belegkraft::Quelle, "7.5 mm (Woodhouse 2012) for guitar, lute, oud, harp; harpsichord 3 mm (plectrum, assumption)");
    else if (n == "releaseSpeed")
        setze(Belegkraft::Quelle, "harp 2 m/s at release (Chadefaux et al. 2012), as a distributed impulse over the triangle; the others 0");
    else if (n == "dampingTime")
        setze(Belegkraft::Offen, "a damper at the nut brings the string down by 60 dB in this time (ZN1 a): harpsichord 60 ms, harp "
                                 "150 ms, the others 80 ms (the values of the former IC model, open)");
    else
        setze(Belegkraft::Offen, "open");
}

/// QM1 a: die Quellen eines Werts ohne Kurve in einer Vorlage, sonst die eigene Vorgabe.
std::vector<std::string> quellenIn(const Vorlage& v, std::string_view name)
{
    for (const auto& [n, q] : v.quellen)
        if (n == name) return q;
    return {"lumiviz"};
}

Parametertafel baueTafel(const Vorlage& vorlage)
{
    const ZupfSpiel& vorgabe = vorlage.spiel;
    Parametertafel t;
    t.kern = std::string(kKernName);
    t.titel = "Plucked string";
    t.rechenFassung = kRechenFassung;
    t.noteMin = kUmfangMin;
    t.noteMax = kUmfangMax;
    for (const std::string_view g : kGruppen) t.gruppen.emplace_back(g);
    t.umfangVon = std::string(kNameNoteMin);
    t.umfangBis = std::string(kNameNoteMax);
    std::vector<std::string> saitenNamen;  // woraus die Tafel des Umlauf-Filters gestimmt wird: die Saiten
    for (const std::string& gruppe : t.gruppen)
        for (const Feld<ZupfSpiel>& f : felder())
            if (f.angabe.gruppe == gruppe)
            {
                ParameterAngabe a = f.angabe;
                if (a.name == kUmlaufTafel)
                {
                    a.gestimmt = true;
                    a.abhaengigVon = saitenNamen;
                }
                else
                {
                    const Wert wert = f.lies(vorgabe);
                    a.vorgabe = wert;
                    if (a.art != WertArt::Kurve) belege(a);
                    else if (const Kurve* k = std::get_if<Kurve>(&wert))
                    {
                        a.belegkraft = k->belegkraft;
                        a.herkunft = k->herkunft;
                    }
                    a.quellen = quellenIn(vorlage, a.name);
                    a.klangbestimmend = a.name != kNameNoteMin && a.name != kNameNoteMax;
                    if (a.gruppe == "Strings") saitenNamen.push_back(a.name);
                }
                t.werte.push_back(std::move(a));
            }
    ParameterAngabe bezug = angabe(kBezug, "Output reference", "Output", WertArt::Zahl, "", 1e-12, 1e12);
    bezug.vorgabe = 1.0;
    bezug.belegkraft = Belegkraft::Rechnung;
    bezug.herkunft = "normiere: peak 0.5 of the force at a rigid bridge at the middle key of the instrument, level 0, held 1 s "
                     "(IK2-10 a)";
    bezug.klangbestimmend = false;
    t.werte.push_back(std::move(bezug));
    t.bezugWert = std::string(kBezug);
    return t;
}

/// Ein starrer Steg mit einem Stegpunkt (ohne Koerper im Auftrag).
const Steg& starrerSteg(int taste, Steg& ablage)
{
    ablage = Steg{};
    ablage.punkte.push_back({taste, {}, 0.0});
    return ablage;
}

}  // namespace

const std::vector<Vorlage>& vorlagen()
{
    static const std::vector<Vorlage> v = {
        {"harpsichord", "Harpsichord", baueVorgabeCembalo(), {}, false},
        {"concert_harp", "Concert harp", baueVorgabeHarfe(), {}, false},
        {"classical_guitar", "Classical guitar", baueVorgabeGitarre(), {}, false},
        {"lute", "Lute", baueVorgabeLaute(), {}, false},
        {"oud", "Oud", baueVorgabeOud(), {}, false},
    };
    return v;
}

Werte werteVon(const Vorlage& vorlage)
{
    Werte w = werteAus(vorlage.spiel);
    for (const ParameterAngabe& a : kernTyp().tafel.werte)
        if (a.art != WertArt::Kurve) w.setzeQuellen(a.name, quellenIn(vorlage, a.name));
    return w;
}

const KernTyp& kernTyp()
{
    static const KernTyp typ = [] {
        KernTyp t;
        t.tafel = baueTafel(vorlagen().front());
        t.auskunft.rechenweg = Rechenweg::Note;
        t.auskunft.stufeImNamen = true;
        t.auskunft.spuren = {};
        t.auskunft.innere = {};
        t.auskunft.vorbereitungTeuer = false;
        t.auskunft.koerperAnDerSchicht = false;
        t.auskunft.stegImAuftrag = true;  // GZ5 a, GZ7 a
        t.erzeuge = [](const Werte& w) -> std::shared_ptr<const Tonkern> { return std::make_shared<const ZupfKern>(spielAus(w)); };
        return t;
    }();
    return typ;
}

Werte werteAus(const ZupfSpiel& spiel)
{
    Werte w;
    for (const std::string_view gruppe : kGruppen)
        for (const Feld<ZupfSpiel>& f : felder())
            if (f.angabe.gruppe == gruppe) w.felder.emplace_back(f.angabe.name, f.lies(spiel));
    w.felder.emplace_back(std::string(kBezug), spiel.ausgangBezug);
    return w;
}

ZupfSpiel spielAus(const Werte& werte)
{
    ZupfSpiel s;
    for (const Feld<ZupfSpiel>& f : felder())
        if (const Wert* w = werte.finde(f.angabe.name)) f.setze(s, *w);
    if (const Wert* w = werte.finde(kBezug))
        if (const auto* d = std::get_if<double>(w)) s.ausgangBezug = *d;
    return s;
}

ZupfKern::ZupfKern(ZupfSpiel spiel) : m_spiel(std::move(spiel)) {}

Frames ZupfKern::nachklang(const Tonauftrag& auftrag, Frames hoechstens) const
{
    if (auftrag.noten.empty()) return std::min<Frames>(hoechstens, 1);
    double moden = 0.0;  // die langsamste Mode des Stegs bis −60 dB
    if (auftrag.steg)
        for (const Steg::Mode& m : auftrag.steg->moden)
            if (m.hz > 0.0 && m.verlust > 0.0) moden = std::max(moden, std::log(1000.0) / (std::numbers::pi * m.hz * m.verlust));
    const double s = kVorlauf + m_spiel.daempfenS + moden + 0.02;
    return std::clamp<Frames>(static_cast<Frames>(std::ceil(s * auftrag.rate)), 1, std::max<Frames>(1, hoechstens));
}

void ZupfKern::rechneAmSteg(const Tonauftrag& auftrag, std::vector<Stegkraft>& kraefte, Innere* innere) const
{
    (void)innere;  // der Kern gibt keine inneren Groessen ab
    kraefte.clear();
    Stegkraft k;
    const std::size_t frames = static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames));
    k.kraft.assign(frames, 0.0f);
    if (auftrag.noten.empty())
    {
        kraefte.push_back(std::move(k));
        return;
    }
    const KettenNote& n = auftrag.noten.front();
    const ZupfNote note = zupfNote(m_spiel, n.note, n.saite, auftrag.stufe, n.ende - n.beginn, auftrag.rate);
    k.taste = note.stegTaste;
    if (!note.fehler.empty() || n.beginn >= auftrag.frames)
    {
        kraefte.push_back(std::move(k));
        return;
    }
    Steg starr;
    const Steg* steg = auftrag.steg.get();
    const Steg::Punkt* punkt = steg != nullptr ? steg->punktZu(note.stegTaste) : nullptr;
    if (punkt == nullptr)
    {
        steg = &starrerSteg(note.stegTaste, starr);
        punkt = &steg->punkte.front();
    }
    Schleifenauftrag a;
    a.saiten = note.saiten;
    a.zupf = note.zupf;
    a.steg = steg;
    a.punkt = punkt;
    a.frames = static_cast<std::size_t>(auftrag.frames - n.beginn);
    a.vorlauf = kVorlauf;
    const Schleifenkraft kraft = rechneSchleife(a);
    const double faktor = m_spiel.ausgangBezug > 0.0 ? 1.0 / m_spiel.ausgangBezug : 1.0;
    for (std::size_t i = 0; i < kraft.senkrecht.size(); ++i)
        k.kraft[static_cast<std::size_t>(n.beginn) + i] = static_cast<float>(kraft.senkrecht[i] * faktor);
    kraefte.push_back(std::move(k));
}

void ZupfKern::rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere) const
{
    std::vector<Stegkraft> k;
    rechneAmSteg(auftrag, k, innere);
    ton = std::move(k.front().kraft);
}

}  // namespace lumi::kern::zupf
