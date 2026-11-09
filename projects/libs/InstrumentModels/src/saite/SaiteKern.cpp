/**
 * @file SaiteKern.cpp
 * @brief InstrumentModels, Kern `struckString`: Tafel, Werte ↔ Spiel, Adapter, Vorlagen (34i-3; Herleitung im Kopf von
 *        `SaiteKern.hpp`)
 */
#include "InstrumentModels/saite/SaiteKern.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <utility>

namespace lumi::kern::saite
{

namespace
{

constexpr std::string_view kNameNoteMin = "noteMin";
constexpr std::string_view kNameNoteMax = "noteMax";
constexpr std::string_view kAbnehmerStellen = "pickupPositions";
constexpr std::string_view kStelle = "position";

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

/// Ein Wert des Spiels in der Tafel (auch eine Kurve oder die Liste der Tonabnehmer): Angabe, Lesen, Setzen.
struct Feld
{
    ParameterAngabe angabe;
    std::function<Wert(const SaitenSpiel&)> lies;
    std::function<void(SaitenSpiel&, const Wert&)> setze;
};

Feld zahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
          double SaitenSpiel::* feld)
{
    return Feld{angabe(name, titel, gruppe, WertArt::Zahl, einheit, von, bis), [feld](const SaitenSpiel& s) { return Wert{s.*feld}; },
                [feld](SaitenSpiel& s, const Wert& w) {
                    if (const auto* d = std::get_if<double>(&w)) s.*feld = *d;
                }};
}

Feld ganzzahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
              int SaitenSpiel::* feld)
{
    return Feld{angabe(name, titel, gruppe, WertArt::Ganzzahl, einheit, von, bis),
                [feld](const SaitenSpiel& s) { return Wert{static_cast<std::int64_t>(s.*feld)}; },
                [feld](SaitenSpiel& s, const Wert& w) {
                    if (const auto* g = std::get_if<std::int64_t>(&w)) s.*feld = static_cast<int>(*g);
                }};
}

Feld schalter(std::string_view name, std::string_view titel, std::string_view gruppe, bool SaitenSpiel::* feld)
{
    return Feld{angabe(name, titel, gruppe, WertArt::Schalter), [feld](const SaitenSpiel& s) { return Wert{s.*feld}; },
                [feld](SaitenSpiel& s, const Wert& w) {
                    if (const auto* b = std::get_if<bool>(&w)) s.*feld = *b;
                }};
}

template <typename E>
Feld wahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::vector<std::pair<std::string, E>> wahlen,
          E SaitenSpiel::* feld)
{
    ParameterAngabe a = angabe(name, titel, gruppe, WertArt::Wahl);
    for (const auto& [n, e] : wahlen) a.wahlen.push_back(n);
    return Feld{std::move(a),
                [feld, wahlen](const SaitenSpiel& s) {
                    for (const auto& [n, e] : wahlen)
                        if (e == s.*feld) return Wert{n};
                    return Wert{wahlen.front().first};
                },
                [feld, wahlen](SaitenSpiel& s, const Wert& w) {
                    if (const auto* t = std::get_if<std::string>(&w))
                        for (const auto& [n, e] : wahlen)
                            if (n == *t) s.*feld = e;
                }};
}

/// Eine Kurve, die Grenzen aus `richteSaitenSpiel`.
Feld kurve(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, Skala skala, double von,
           double bis, Kurve SaitenSpiel::* feld)
{
    ParameterAngabe a = angabe(name, titel, gruppe, WertArt::Kurve, einheit, von, bis);
    a.skala = skala;
    return Feld{std::move(a), [feld](const SaitenSpiel& s) { return Wert{s.*feld}; },
                [feld](SaitenSpiel& s, const Wert& w) {
                    if (const auto* k = std::get_if<Kurve>(&w)) s.*feld = *k;
                }};
}

Feld schlaegelZahl(std::string_view name, std::string_view titel, std::string_view einheit, double von, double bis, double Stossanreger::* feld)
{
    return Feld{angabe(name, titel, "Excitation", WertArt::Zahl, einheit, von, bis),
                [feld](const SaitenSpiel& s) { return Wert{s.schlaegel.*feld}; },
                [feld](SaitenSpiel& s, const Wert& w) {
                    if (const auto* d = std::get_if<double>(&w)) s.schlaegel.*feld = *d;
                }};
}

/// Alle Werte der Tafel ausser dem Bezug, in ihrer Reihenfolge (Gruppen wie `SaitenSpiel`).
const std::vector<Feld>& felder()
{
    using L = Skala;
    static const std::vector<Feld> f = [] {
        std::vector<Feld> v;
        v.push_back(ganzzahl(kNameNoteMin, "Lowest key", "Keys", "MIDI", kSaitenUmfangMin, kSaitenUmfangMax, &SaitenSpiel::noteMin));
        v.push_back(ganzzahl(kNameNoteMax, "Highest key", "Keys", "MIDI", kSaitenUmfangMin, kSaitenUmfangMax, &SaitenSpiel::noteMax));
        v.push_back(kurve("stringLength", "String length", "String", "m", L::Logarithmisch, 0.02, 8.0, &SaitenSpiel::laengeM));
        v.push_back(kurve("stringMass", "String mass", "String", "g/m", L::Logarithmisch, 0.01, 500.0, &SaitenSpiel::masseGm));
        v.push_back(kurve("inharmonicity", "Inharmonicity", "String", "", L::Logarithmisch, 1e-7, 0.1, &SaitenSpiel::inharmonizitaet));
        v.push_back(kurve("decay", "Decay", "Decay", "s", L::Logarithmisch, 0.01, 120.0, &SaitenSpiel::t60S));
        v.push_back(kurve("lateDecay", "Late decay", "Decay", "s", L::Logarithmisch, 0.01, 120.0, &SaitenSpiel::t60SpaetS));
        v.push_back(kurve("highDecay", "High decay", "Decay", "", L::Logarithmisch, 0.001, 1.0, &SaitenSpiel::abklingHoch));
        v.push_back(kurve("strings", "Strings", "Course", "", L::Linear, 1.0, 5.0, &SaitenSpiel::chor));
        v.push_back(kurve("detune", "Detune", "Course", "cent", L::Linear, 0.0, 20.0, &SaitenSpiel::verstimmungCent));
        v.push_back(kurve("polarizationAngle", "Polarization angle", "Course", "deg", L::Linear, 0.0, 90.0, &SaitenSpiel::polarisationWinkelGrad));
        v.push_back(kurve("polarizationDetune", "Polarization detune", "Course", "cent", L::Linear, 0.01, 20.0, &SaitenSpiel::polarisationCent));
        v.push_back(kurve("horizontalRatio", "Horizontal ratio", "Course", "", L::Linear, 0.0, 1.0, &SaitenSpiel::waagrechtVerhaeltnis));
        v.push_back(zahl("middleBridgeSplit", "Middle bridge split", "Course", "", 0.0, 0.95, &SaitenSpiel::stegTeilung));
        v.push_back(zahl("middleBridgeCoupling", "Middle bridge coupling", "Course", "", 0.0, 10.0, &SaitenSpiel::mittelstegKopplung));
        v.push_back(schalter("sympathetic", "Other courses resonate", "Course", &SaitenSpiel::mitschwingen));
        v.push_back(zahl("sympatheticCoupling", "Resonance coupling", "Course", "", 0.0, 10.0, &SaitenSpiel::mitschwingKopplung));
        v.push_back(wahl<Anregung>("excitation", "Excitation", "Excitation", {{"mallet", Anregung::Schlaegel}, {"tangent", Anregung::Tangente}},
                                   &SaitenSpiel::anregung));
        v.push_back(kurve("strikePosition", "Strike position", "Excitation", "", L::Linear, 0.0, 1.0, &SaitenSpiel::anschlagStelle));
        v.push_back(schlaegelZahl("malletMass", "Mallet mass", "kg", 1e-4, 10.0, &Stossanreger::masseKg));
        v.push_back(schlaegelZahl("malletStiffness", "Mallet stiffness", "N/mm^p", 1e-6, 1e9, &Stossanreger::q0));
        v.push_back(schlaegelZahl("malletExponent", "Mallet exponent", "", 1.0, 6.0, &Stossanreger::p));
        v.push_back(schlaegelZahl("malletHysteresis", "Mallet hysteresis", "", 0.0, 0.999, &Stossanreger::eps));
        v.push_back(schlaegelZahl("malletRelaxation", "Mallet relaxation", "s", 0.0, 1.0, &Stossanreger::tauS));
        v.push_back(Feld{angabe("malletHuntCrossley", "Hunt–Crossley contact", "Excitation", WertArt::Schalter),
                         [](const SaitenSpiel& s) { return Wert{s.schlaegel.huntCrossley}; },
                         [](SaitenSpiel& s, const Wert& w) {
                             if (const auto* b = std::get_if<bool>(&w)) s.schlaegel.huntCrossley = *b;
                         }});
        v.push_back(schlaegelZahl("malletDampingHc", "Hunt–Crossley damping", "s/mm", 0.0, 1.0, &Stossanreger::lambdaHc));
        v.push_back(zahl("velocityMax", "Speed at full strength", "Excitation", "m/s", 0.01, 20.0, &SaitenSpiel::tempoMax));
        v.push_back(ganzzahl("oversampling", "Oversampling of the contact", "Excitation", "", 1, 256, &SaitenSpiel::ueberabtastung));
        v.push_back(zahl("tangentTravel", "Tangent travel", "Tangent", "mm", 0.01, 10.0, &SaitenSpiel::tangenteWegMm));
        v.push_back(zahl("wholePerSpeaking", "Whole string per speaking part", "Tangent", "", 1.0, 4.0, &SaitenSpiel::ganzJeKlingend));
        v.push_back(kurve("yarnDecay", "Decay under the yarn", "Tangent", "s", L::Logarithmisch, 0.01, 10.0, &SaitenSpiel::garnT60S));
        v.push_back(zahl("keyPressureLimit", "Key pressure limit", "Tangent", "cent", 0.0, 100.0, &SaitenSpiel::tastendruckGrenzeCent));
        v.push_back(schalter("damper", "Damper", "Damper", &SaitenSpiel::daempfer));
        v.push_back(kurve("damperDecay", "Damper", "Damper", "s", L::Logarithmisch, 0.01, 10.0, &SaitenSpiel::daempferT60S));
        v.push_back(kurve("sliderDecay", "Damper slider", "Damper", "s", L::Logarithmisch, 0.01, 10.0, &SaitenSpiel::schieberT60S));
        v.push_back(wahl<Abnahme>("pickup", "Pickup", "Pickups", {{"bridge", Abnahme::Steg}, {"magnetic", Abnahme::Magnetisch}},
                                  &SaitenSpiel::abnahme));
        {
            ParameterAngabe l = angabe(kAbnehmerStellen, "Pickup positions", "Pickups", WertArt::Liste);
            ParameterAngabe e = angabe(kStelle, "Position", "", WertArt::Zahl, "x/L", 0.0, 1.0);
            e.vorgabe = 0.1;
            l.felder.push_back(std::move(e));
            l.eintraegeMin = 0;
            l.eintraegeMax = 2;
            v.push_back(Feld{std::move(l),
                             [](const SaitenSpiel& s) {
                                 Liste li;
                                 for (const double x : s.abnehmerStellen)
                                 {
                                     Werte w;
                                     w.setze(std::string(kStelle), x);
                                     li.eintraege.push_back(std::move(w));
                                 }
                                 return Wert{std::move(li)};
                             },
                             [](SaitenSpiel& s, const Wert& w) {
                                 if (const Liste* li = std::get_if<Liste>(&w))
                                 {
                                     s.abnehmerStellen.clear();
                                     for (const Werte& e : li->eintraege)
                                         if (const Wert* x = e.finde(kStelle))
                                             if (const auto* d = std::get_if<double>(x)) s.abnehmerStellen.push_back(*d);
                                 }
                             }});
        }
        v.push_back(schalter("amplifier", "Amplifier", "Amp", &SaitenSpiel::verstaerker));
        return v;
    }();
    return f;
}

/// Belegkraft und Herkunft der Werte ohne Kurve (tafelweit; die Kurven tragen ihre aus der ersten Vorlage); die Quellen
/// stehen je Vorlage (QM1 a, `quellenDerSaite` unten).
void belege(ParameterAngabe& a)
{
    const auto setze = [&](Belegkraft b, std::string h) {
        a.belegkraft = b;
        a.herkunft = std::move(h);
    };
    const std::string& n = a.name;
    if (n == kNameNoteMin || n == kNameNoteMax)
        setze(Belegkraft::Offen, "clavinet F1-E6, 60 keys (open); dulcimer G2-D6, cimbalom D2-E6 (IC, open)");
    else if (n == "velocityMax")
        setze(Belegkraft::Quelle, "clavinet: key speed \"1 to 4 m/s\" (Gabrielli et al. 2013 p. 11); dulcimer and cimbalom 4 m/s (open)");
    else if (n == "wholePerSpeaking")
        setze(Belegkraft::Quelle, "clavinet: \"the pitch decrease after release is three semitones\" (Gabrielli et al. 2013 p. 6): 2^(3/12)");
    else if (n == "amplifier")
        setze(Belegkraft::Quelle, "shelves -3 dB at 130 Hz, +3 dB at 4 kHz (Gabrielli et al. 2013 p. 6), read as RBJ shelf S = 1 (open)");
    else if (n == kAbnehmerStellen)
        setze(Belegkraft::Offen, "clavinet 0.10 and 0.30 of the speaking length (after Remaggi et al. 2012: 4 cm and 6.5-18.5 cm, report only)");
    else if (n == "keyPressureLimit")
        setze(Belegkraft::Offen, "0 until a measurement (IC K1 F-4-1 a)");
    else if (n.rfind("mallet", 0) == 0)
        setze(Belegkraft::Offen, "dulcimer: 8-12 g (Peterson 2005, report only), stiffness of the Orff mallet (Rucz & Ulveczki 2021, "
                                 "transferred); cimbalom: felt after Stulov at MIDI 40 (transferred); clavinet without mallet");
    else if (n == "middleBridgeSplit")
        setze(Belegkraft::Offen, "dulcimer and cimbalom 3 : 2 (Peterson 1994, report only)");
    else
        setze(Belegkraft::Offen, "IC SaitenVorgaben.cpp for each instrument (switch, choice or setting; open)");
}

/// QM1 a: die Quellen eines Werts ohne Kurve in einer Vorlage, sonst die eigene Vorgabe.
std::vector<std::string> quellenIn(const Vorlage& v, std::string_view name)
{
    for (const auto& [n, q] : v.quellen)
        if (n == name) return q;
    return {"lumiviz"};
}

/// QM1 a: die Quellen der Werte ohne Kurve je Instrument nach `Vorgaben.cpp` (die Kurven setzt `mitQuellen` dort).
/// - Clavinet: Tastengeschwindigkeit, drei Halbtoene nach dem Loslassen und Verstaerker nach Gabrielli et al. 2013; die
///   Lagen der Tonabnehmer nach Remaggi et al. 2012 (nur Bericht).
/// - Hackbrett: Masse des Schlaegels nach Peterson 2005, Steife und Exponent vom Orff-Schlaegel nach Rucz & Ulveczki 2021.
/// - Cimbalom: der Filz nach Stulov & Kartofelev 2009 (an MIDI 40 uebertragen), die Masse nach Peterson 2005.
QuellenJeWert quellenDerSaite(std::string_view name)
{
    QuellenJeWert q;
    const auto dazu = [&](std::string_view wert, std::vector<std::string> schluessel) { q.emplace_back(std::string(wert), std::move(schluessel)); };
    if (name == "clavinet_modal")
    {
        for (const std::string_view w : {"velocityMax", "wholePerSpeaking", "amplifier"}) dazu(w, {"gabrielli2013"});
        dazu(kAbnehmerStellen, {"remaggi2012"});
    }
    else if (name == "hammered_dulcimer")
    {
        dazu("malletMass", {"peterson2005"});
        dazu("malletStiffness", {"rucz2021"});
        dazu("malletExponent", {"rucz2021"});
    }
    else if (name == "cimbalom")
    {
        dazu("malletMass", {"peterson2005"});
        for (const std::string_view w : {"malletStiffness", "malletExponent", "malletHysteresis", "malletRelaxation"}) dazu(w, {"stulov2009"});
    }
    return q;
}

Parametertafel baueTafel(const Vorlage& vorlage)
{
    const SaitenSpiel& vorgabe = vorlage.spiel;
    Parametertafel t;
    t.kern = std::string(kKernName);
    t.titel = "Struck string";
    t.rechenFassung = 1;
    t.noteMin = kSaitenUmfangMin;
    t.noteMax = kSaitenUmfangMax;
    t.gruppen = {"Keys", "String", "Decay", "Course", "Excitation", "Tangent", "Damper", "Pickups", "Amp", "Output"};
    t.umfangVon = std::string(kNameNoteMin);
    t.umfangBis = std::string(kNameNoteMax);
    for (const Feld& f : felder())
    {
        ParameterAngabe a = f.angabe;
        a.vorgabe = f.lies(vorgabe);
        if (a.art == WertArt::Kurve)
        {
            const Kurve& k = std::get<Kurve>(a.vorgabe);
            a.belegkraft = k.belegkraft;
            a.herkunft = k.herkunft;
            a.quellen = k.quellen;
        }
        else
        {
            belege(a);
            a.quellen = quellenIn(vorlage, a.name);
        }
        a.klangbestimmend = a.name != kNameNoteMin && a.name != kNameNoteMax;
        t.werte.push_back(std::move(a));
    }
    ParameterAngabe bezug = angabe(kBezug, "Output reference", "Output", WertArt::Zahl, "", 1e-12, 1e12);
    bezug.vorgabe = 1.0;
    bezug.belegkraft = Belegkraft::Rechnung;
    bezug.herkunft = "normiere: peak 0.5 at the middle key of the instrument, level 0 (IK2-10 a)";
    bezug.klangbestimmend = false;
    t.werte.push_back(std::move(bezug));
    t.bezugWert = std::string(kBezug);
    t.bezugRunden = 1;
    return t;
}

double spurAmBeginn(const Tonauftrag& auftrag, std::string_view name, Frames beginn, double ohne)
{
    const Spur* s = findeSpur(auftrag, name);
    return s != nullptr ? spurWert(*s, beginn, ohne) : ohne;
}

}  // namespace

const std::vector<Vorlage>& vorlagen()
{
    static const std::vector<Vorlage> v = [] {
        std::vector<Vorlage> l = {
            {"clavinet_modal", "Clavinet modal", baueVorgabeClavinet(), true, {}},  // KL2 a: bis zu den Hoerproben neben `clavinet`
            {"hammered_dulcimer", "Hammered dulcimer", baueVorgabeHackbrett(), false, {}},  // nicht `dulcimer` (das LumiViz-Instrument)
            {"cimbalom", "Cimbalom", baueVorgabeCimbalom(), false, {}},
        };
        for (Vorlage& x : l) x.quellen = quellenDerSaite(x.name);
        return l;
    }();
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
        t.auskunft.spuren = {std::string(kSpurSchieber), std::string(kSpurTastendruck), std::string(kSpurAbnehmer)};
        t.auskunft.innere = {"contact", "release"};
        t.auskunft.vorbereitungTeuer = false;
        t.auskunft.koerperAnDerSchicht = false;
        t.erzeuge = [](const Werte& w) -> std::shared_ptr<const Tonkern> { return std::make_shared<const SaiteKern>(spielAus(w)); };
        return t;
    }();
    return typ;
}

Werte werteAus(const SaitenSpiel& spiel)
{
    Werte w;
    for (const Feld& f : felder()) w.felder.emplace_back(f.angabe.name, f.lies(spiel));
    w.felder.emplace_back(std::string(kBezug), spiel.ausgangBezug);
    return w;
}

SaitenSpiel spielAus(const Werte& werte)
{
    SaitenSpiel s;
    for (const Feld& f : felder())
        if (const Wert* w = werte.finde(f.angabe.name)) f.setze(s, *w);
    if (const Wert* w = werte.finde(kBezug))
        if (const auto* d = std::get_if<double>(w)) s.ausgangBezug = *d;
    // die Regel L11 und die Grenzen der Zahlen, die die Tafel nicht kennt (wie IC)
    richteSaitenSpiel(s, s);
    return s;
}

AbnehmerWahl abnehmerWahlAus(double wert)
{
    const int i = std::clamp(static_cast<int>(std::lround(wert)), 0, 3);
    constexpr AbnehmerWahl kWahlen[4] = {AbnehmerWahl::Beide, AbnehmerWahl::Steg, AbnehmerWahl::Mitte, AbnehmerWahl::Gegenphase};
    return kWahlen[i];
}

SaitenAnschlag anschlagAus(const SaitenSpiel& spiel, const Tonauftrag& auftrag)
{
    (void)spiel;
    SaitenAnschlag a;
    a.rate = static_cast<int>(auftrag.rate);
    a.stufe = auftrag.stufe;
    if (auftrag.noten.empty()) return a;
    const KettenNote& n = auftrag.noten.front();
    a.note = n.note;
    a.hz = n.hz;
    a.halten = n.ende - n.beginn;
    a.gedaempft = true;  // Sustain steht in der Planung (die Note haelt laenger)
    a.schieber = spurAmBeginn(auftrag, kSpurSchieber, n.beginn, 0.0);
    a.tastendruckCent = spurAmBeginn(auftrag, kSpurTastendruck, n.beginn, 0.0);
    a.abnehmerWahl = abnehmerWahlAus(spurAmBeginn(auftrag, kSpurAbnehmer, n.beginn, 0.0));
    a.mitschwingen = false;  // nur je Clip (Rechenweg Clip, 31e/34k)
    return a;
}

SaiteKern::SaiteKern(SaitenSpiel spiel) : m_spiel(std::move(spiel)) {}

Frames SaiteKern::nachklang(const Tonauftrag& auftrag, Frames hoechstens) const
{
    if (auftrag.noten.empty()) return std::min<Frames>(hoechstens, 1);
    const SaitenAnschlag a = anschlagAus(m_spiel, auftrag);
    return std::clamp<Frames>(saitenNachklang(m_spiel, a, hoechstens), 1, std::max<Frames>(1, hoechstens));
}

void SaiteKern::rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere) const
{
    if (auftrag.noten.empty())
    {
        ton.assign(static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames)), 0.0f);
        return;
    }
    const auto will = [&](std::string_view name) {
        return innere != nullptr && std::find(innere->gewuenscht.begin(), innere->gewuenscht.end(), name) != innere->gewuenscht.end();
    };
    const bool messen = will("contact") || will("release");
    SaitenMessung messung;
    rechneSaitenTon(m_spiel, anschlagAus(m_spiel, auftrag), ton, auftrag.frames, messen ? &messung : nullptr);
    if (will("contact")) innere->ergebnis.push_back(InnereGroesse{"contact", "s, N", {messung.kontaktS, messung.kraftSpitzeN}});
    if (will("release")) innere->ergebnis.push_back(InnereGroesse{"release", "Hz", {messung.hzNachLoslassen}});
}

}  // namespace lumi::kern::saite
