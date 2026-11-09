/**
 * @file StabKern.cpp
 * @brief InstrumentModels, Kern `bar`: Tafel, Werte ↔ Spiel, Adapter, Vorlagen (34g-3; Herleitung im Kopf von
 *        `StabKern.hpp`)
 */
#include "InstrumentModels/stab/StabKern.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <utility>

namespace lumi::kern::stab
{

namespace
{

constexpr int kUmfangMin = kStabUmfangMin;  // A0
constexpr int kUmfangMax = kStabUmfangMax;  // C8

constexpr std::string_view kNameNoteMin = "noteMin";
constexpr std::string_view kNameNoteMax = "noteMax";
constexpr std::string_view kPartnerZielDb = "partnerGainDb";
constexpr std::string_view kPartnerZielNote = "partnerGainNote";
constexpr std::string_view kBogenAb = "bowedFrom";
constexpr std::string_view kFesteVerhaeltnisse = "fixedRatios";
constexpr std::string_view kVerhaeltnis = "ratio";

/// Eine Kurve des Spiels in der Tafel.
struct KurvenFeld
{
    std::string_view name, titel, gruppe, einheit;
    Skala skala;
    double von, bis;
    Kurve StabSpiel::* feld;
};

/// Die Kurven in der Reihenfolge der Tafel, die Grenzen aus IC `richteStabSpiel` (`modell.md`); der Pegel des Partners
/// mit 1e-6 statt 0 unten (logarithmisch; ein Instrument ohne akustischen Partner traegt 1, ohne Wirkung).
const std::vector<KurvenFeld>& kurvenFelder()
{
    static const std::vector<KurvenFeld> f = {
        {"fundamentalRatio", "Fundamental per note", "Bar", "", Skala::Linear, 0.05, 2.0, &StabSpiel::grundJeNote},
        {"ratio2", "Ratio of mode 2", "Bar", "", Skala::Linear, 1.5, 20.0, &StabSpiel::verhaeltnis2},
        {"ratio3", "Ratio of mode 3", "Bar", "", Skala::Linear, 2.0, 60.0, &StabSpiel::verhaeltnis3},
        {"t60", "Decay T60 of mode 1", "Bar", "s", Skala::Logarithmisch, 0.01, 200.0, &StabSpiel::t60S},
        {"dampingExponent", "Damping exponent", "Bar", "", Skala::Linear, 0.0, 3.0, &StabSpiel::daempfungExponent},
        {"mass", "Mass of the bar", "Bar", "kg", Skala::Logarithmisch, 1e-4, 10.0, &StabSpiel::masseKg},
        {"torsionRatio", "Ratio of torsion 1", "Bar", "", Skala::Linear, 0.0, 20.0, &StabSpiel::torsionVerhaeltnis},
        {"torsionT60", "Decay T60 of torsion", "Bar", "s", Skala::Logarithmisch, 0.01, 200.0, &StabSpiel::torsionT60S},
        {"radiationExponent", "Radiation exponent", "Bar", "", Skala::Linear, -2.0, 2.0, &StabSpiel::abstrahlExponent},
        {"partnerRatio", "Partner ratio", "Partner", "", Skala::Linear, 0.8, 1.25, &StabSpiel::partnerVerhaeltnis},
        {"partnerDamping", "Partner damping ratio", "Partner", "", Skala::Logarithmisch, 1e-5, 0.2, &StabSpiel::partnerXi},
        {"couplingPerCritical", "Coupling per critical", "Partner", "", Skala::Linear, 0.0, 10.0, &StabSpiel::kopplungJeKritisch},
        {"strikePosition", "Strike position", "Mallet", "x/L", Skala::Linear, 0.0, 1.0, &StabSpiel::anschlagStelle},
        {"offAxis", "Off the axis", "Mallet", "e/b", Skala::Linear, -0.5, 0.5, &StabSpiel::querVersatz},
        {"damperT60", "Decay T60 under the damper", "Damper", "s", Skala::Logarithmisch, 0.01, 10.0, &StabSpiel::daempferT60S},
        {"pickupPosition", "Pickup position", "Pickup", "x/L", Skala::Linear, 0.0, 1.0, &StabSpiel::abnehmerStelle},
    };
    return f;
}

/// Eine Zahl, Ganzzahl, ein Schalter oder eine Wahl des Spiels in der Tafel.
struct Feld
{
    ParameterAngabe angabe;
    std::function<Wert(const StabSpiel&)> lies;
    std::function<void(StabSpiel&, const Wert&)> setze;
};

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

Feld zahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
          double StabSpiel::* feld)
{
    return Feld{angabe(name, titel, gruppe, WertArt::Zahl, einheit, von, bis), [feld](const StabSpiel& s) { return Wert{s.*feld}; },
                [feld](StabSpiel& s, const Wert& w) {
                    if (const auto* d = std::get_if<double>(&w)) s.*feld = *d;
                }};
}

Feld schlaegelZahl(std::string_view name, std::string_view titel, std::string_view einheit, double von, double bis, double Stossanreger::* feld)
{
    return Feld{angabe(name, titel, "Mallet", WertArt::Zahl, einheit, von, bis), [feld](const StabSpiel& s) { return Wert{s.schlaegel.*feld}; },
                [feld](StabSpiel& s, const Wert& w) {
                    if (const auto* d = std::get_if<double>(&w)) s.schlaegel.*feld = *d;
                }};
}

Feld ganzzahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von, double bis,
              int StabSpiel::* feld)
{
    return Feld{angabe(name, titel, gruppe, WertArt::Ganzzahl, einheit, von, bis),
                [feld](const StabSpiel& s) { return Wert{static_cast<std::int64_t>(s.*feld)}; },
                [feld](StabSpiel& s, const Wert& w) {
                    if (const auto* g = std::get_if<std::int64_t>(&w)) s.*feld = static_cast<int>(*g);
                }};
}

Feld schalter(std::string_view name, std::string_view titel, std::string_view gruppe, bool StabSpiel::* feld)
{
    return Feld{angabe(name, titel, gruppe, WertArt::Schalter), [feld](const StabSpiel& s) { return Wert{s.*feld}; },
                [feld](StabSpiel& s, const Wert& w) {
                    if (const auto* b = std::get_if<bool>(&w)) s.*feld = *b;
                }};
}

template <typename E>
Feld wahl(std::string_view name, std::string_view titel, std::string_view gruppe, std::vector<std::pair<std::string, E>> wahlen, E StabSpiel::* feld)
{
    ParameterAngabe a = angabe(name, titel, gruppe, WertArt::Wahl);
    for (const auto& [n, e] : wahlen) a.wahlen.push_back(n);
    return Feld{std::move(a),
                [feld, wahlen](const StabSpiel& s) {
                    for (const auto& [n, e] : wahlen)
                        if (e == s.*feld) return Wert{n};
                    return Wert{wahlen.front().first};
                },
                [feld, wahlen](StabSpiel& s, const Wert& w) {
                    if (const auto* t = std::get_if<std::string>(&w))
                        for (const auto& [n, e] : wahlen)
                            if (n == *t) s.*feld = e;
                }};
}

/// Die Felder ausser den Kurven, je mit ihrer Gruppe — die Tafel ordnet sie mit den Kurven nach Gruppen.
const std::vector<Feld>& felder()
{
    static const std::vector<Feld> f = [] {
        std::vector<Feld> v;
        v.push_back(ganzzahl(kNameNoteMin, "Lowest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &StabSpiel::noteMin));
        v.push_back(ganzzahl(kNameNoteMax, "Highest key", "Keys", "MIDI", kUmfangMin, kUmfangMax, &StabSpiel::noteMax));
        v.push_back(schalter("cantilever", "Clamped at one end", "Bar", &StabSpiel::kragbalken));
        v.push_back(zahl("quadraticCoupling", "Quadratic coupling", "Bar", "", 0.0, 10.0, &StabSpiel::quadratischeKopplung));
        v.push_back(wahl<PartnerArt>("partner", "Partner", "Partner",
                                     {{"none", PartnerArt::Keiner}, {"closedTube", PartnerArt::RohrGeschlossen}, {"singleMode", PartnerArt::EineMode}},
                                     &StabSpiel::partner));
        v.push_back(zahl(kPartnerZielDb, "Partner gain target", "Partner", "dB", 0.0, 40.0, &StabSpiel::partnerZielDb));
        v.push_back(ganzzahl(kPartnerZielNote, "Partner gain at key", "Partner", "MIDI", kUmfangMin, kUmfangMax, &StabSpiel::partnerZielNote));
        v.push_back(wahl<Anregung>("excitation", "Excitation", "Mallet", {{"strike", Anregung::Stoss}, {"pluck", Anregung::Zupfen}}, &StabSpiel::anregung));
        v.push_back(schlaegelZahl("malletMass", "Mallet mass", "kg", 1e-4, 10.0, &Stossanreger::masseKg));
        v.push_back(schlaegelZahl("malletStiffness", "Mallet stiffness", "N/mm^p", 1e-3, 1e9, &Stossanreger::q0));
        v.push_back(schlaegelZahl("malletExponent", "Mallet exponent", "", 1.0, 6.0, &Stossanreger::p));
        v.push_back(schlaegelZahl("malletHysteresis", "Mallet hysteresis", "", 0.0, 0.999, &Stossanreger::eps));
        v.push_back(schlaegelZahl("malletRelaxation", "Mallet relaxation", "s", 0.0, 1.0, &Stossanreger::tauS));
        v.push_back(Feld{angabe("malletHuntCrossley", "Hunt–Crossley contact", "Mallet", WertArt::Schalter),
                         [](const StabSpiel& s) { return Wert{s.schlaegel.huntCrossley}; },
                         [](StabSpiel& s, const Wert& w) {
                             if (const auto* b = std::get_if<bool>(&w)) s.schlaegel.huntCrossley = *b;
                         }});
        v.push_back(schlaegelZahl("malletDampingHc", "Hunt–Crossley damping", "s/mm", 0.0, 1.0, &Stossanreger::lambdaHc));
        v.push_back(zahl("velocityMax", "Speed at full strength", "Mallet", "m/s", 0.01, 20.0, &StabSpiel::tempoMax));
        v.push_back(zahl("pluckForce", "Pluck force at full strength", "Mallet", "N", 1e-3, 100.0, &StabSpiel::zupfKraftN));  // KA1 a (S183)
        v.push_back(ganzzahl("oversampling", "Oversampling of the contact", "Mallet", "", 1, 256, &StabSpiel::ueberabtastung));
        v.push_back(wahl<DaempferArt>("damper", "Damper", "Damper",
                                      {{"none", DaempferArt::Keiner}, {"bar", DaempferArt::Leiste}, {"single", DaempferArt::Einzeln}},
                                      &StabSpiel::daempfer));
        v.push_back(schalter("motor", "Motor", "Motor", &StabSpiel::motor));
        v.push_back(zahl("closedOpening", "Opening when closed", "Motor", "", 0.0, 1.0, &StabSpiel::restOeffnung));
        v.push_back(zahl("spinUp", "Spin-up time", "Motor", "s", 0.01, 10.0, &StabSpiel::hochlaufS));
        v.push_back(wahl<Abnahme>("pickup", "Pickup", "Pickup",
                                  {{"acoustic", Abnahme::Akustisch},
                                   {"magnetic", Abnahme::Magnetisch},
                                   {"electrostatic", Abnahme::Elektrostatisch},
                                   {"slot", Abnahme::Schlitz},    // WU1 a (S183): der Wurlitzer
                                   {"bridge", Abnahme::Steg}},    // KA1 a (S183): die Kalimba
                                  &StabSpiel::abnahme));
        v.push_back(zahl("pickupDistance", "Pickup distance", "Pickup", "m", 1e-5, 0.1, &StabSpiel::abnehmerAbstandM));
        v.push_back(zahl("pickupOffset", "Pickup offset", "Pickup", "m", -0.1, 0.1, &StabSpiel::abnehmerVersatzM));
        v.push_back(schalter("bow", "Bowed", "Bow", &StabSpiel::bogen));
        v.push_back(ganzzahl(kBogenAb, "Bowed from key", "Bow", "MIDI", kUmfangMin, kUmfangMax + 1, &StabSpiel::bogenAb));
        v.push_back(zahl("bowPosition", "Bow position", "Bow", "x/L", 0.0, 1.0, &StabSpiel::bogenStelle));
        v.push_back(zahl("bowStaticFriction", "Static friction", "Bow", "", 0.0, 5.0, &StabSpiel::bogenMuS));
        v.push_back(zahl("bowVelocityPp", "Bow speed pp", "Bow", "m/s", 1e-3, 5.0, &StabSpiel::bogenVbPp));
        v.push_back(zahl("bowVelocityFf", "Bow speed ff", "Bow", "m/s", 1e-3, 5.0, &StabSpiel::bogenVbFf));
        v.push_back(zahl("bowAttack", "Bow attack", "Bow", "s", 1e-3, 1.0, &StabSpiel::bogenAnsatzS));
        return v;
    }();
    return f;
}

ParameterAngabe gestimmteAngabe(std::string_view name, std::string_view titel, std::string_view gruppe, std::string_view einheit, double von,
                                double bis)
{
    ParameterAngabe a = angabe(name, titel, gruppe, WertArt::Kurve, einheit, von, bis);
    a.skala = Skala::Logarithmisch;
    a.vorgabe = Kurve{};
    a.gestimmt = true;
    a.belegkraft = Belegkraft::Rechnung;
    return a;
}

/// Belegkraft und Herkunft der Zahlen (der Vorgabe der Tafel, der Marimba; der Text nennt die anderen Instrumente mit);
/// die Quellen stehen je Vorlage (QM1 a, `quellenDesStabs` unten), die Tafel traegt die der ersten Vorlage.
void belege(ParameterAngabe& a)
{
    const auto setze = [&](Belegkraft b, std::string h) {
        a.belegkraft = b;
        a.herkunft = std::move(h);
    };
    const std::string& n = a.name;
    if (n == kNameNoteMin || n == kNameNoteMax)
        setze(Belegkraft::Quelle, "the range of each instrument as in IC Vorgaben.cpp (Iowa MIS, CCRMA, Freesound pages — report only; facts, no data used)");
    else if (n == kPartnerZielDb)
        setze(Belegkraft::Quelle, "\"by as much as 15 dB\" (Rucz & Ulveczki 2021 p. 7, xylophone); celesta: transferred (open)");
    else if (n == kPartnerZielNote)
        setze(Belegkraft::Rechnung, "the key the target holds at: F4 as in Rucz (tube instruments), C6 for the celesta box (IC)");
    else if (n.rfind("mallet", 0) == 0)
        setze(Belegkraft::Quelle, "default (marimba): mallet 25 g, K = 5e7 N/m^1.5, Hertz p = 1.5 (Rucz & Ulveczki 2021 Tab. 1 p. 5); "
                                  "celesta: Stulov 2009 eq. 9; tine piano: Hunt–Crossley after Soenderbo 2024 Tab. 3.2; reed piano: small hard "
                                  "maple-and-felt hammer, Hunt–Crossley 3 g, p 1.5, 300 N/mm^1.5 (estimate, S183 WU2 a, open until M8)");
    else if (n == "pluckForce")
        setze(Belegkraft::Offen, "kalimba: 3 N at full strength, the thumb's force (open; S183 KA1 a) — the displacement follows from the "
                                 "tine's stiffness, so the force at the mount is even over the range");
    else if (n == "pickupDistance" || n == "pickupOffset")
        setze(Belegkraft::Offen, "tine piano: 1 mm after Falaize & Helie 2015 (report only), offset 0.5 mm voicing (open); reed piano slot: "
                                 "bell width 1 mm (plate thickness, open) and working point 0.5 mm — the 200/200A service manual voices loud "
                                 "notes by bending the pickup end 1/32-1/16 in (S183 WU1 a)");
    else if (n == kBogenAb)
        setze(Belegkraft::Rechnung, "IC test of the upper register from E-flat 4 (63); the low register is IC class C (KS3 a)");
    else if (n.rfind("bow", 0) == 0)
        setze(Belegkraft::Offen, "transferred from class 3 (Galluzzo friction, Chudy bow speed — report only), open");
    else if (n == "quadraticCoupling")
        setze(Belegkraft::Offen, "0 until a measurement (IC T8 a; steelpan, Deluxe)");
    else
        setze(Belegkraft::Offen, "IC Vorgaben.cpp for each instrument (switch, choice or setting; open)");
}

/// QM1 a: die Quellen eines Werts ohne Kurve in einer Vorlage, sonst die eigene Vorgabe.
std::vector<std::string> quellenIn(const Vorlage& v, std::string_view name)
{
    for (const auto& [n, q] : v.quellen)
        if (n == name) return q;
    return {"lumiviz"};
}

/// QM1 a (S180): die Quellen der Werte ohne Kurve je Stabspiel nach `Vorgaben.cpp` (die Kurven setzt `mitQuellen` dort).
/// - Schlaegel: Rucz & Ulveczki (Masse, Steife, Exponent nach Hertz) an den Staeben mit `ruczSchlaegel`; Celesta der
///   Filzhammer nach Stulov & Kartofelev 2009 Gl. 9 (Masse, Steife, Exponent, Hysterese, Relaxation); Tine electric piano
///   Hunt–Crossley nach Sønderbo 2024 Tab. 3.2 (Masse, Steife, Exponent, Schalter, Daempfung) und v bis 4 m/s; Reed electric
///   piano seit S183 (WU2 a) ein kleiner harter Hammer als Einschaetzung (`lumiviz`); die Kalimba wird gezupft (Kraft
///   `pluckForce`, KA1 a, `lumiviz`).
/// - Ziel des Partners: „by as much as 15 dB“ (Rucz) mit Rohr (Marimba, Xylophon, Vibraphon) und am Kasten der Celesta
///   (uebertragen); die Taste F4 wie Rucz an den Rohren, C6 der Celesta ist IC.
/// - Tonabnehmer des Tine electric piano: 1 mm Abstand nach Falaize & Hélie 2015.
/// - Feste Verhaeltnisse der Steelpan: Thomas, Monteil & Touzé 2014.
QuellenJeWert quellenDesStabs(std::string_view name)
{
    QuellenJeWert q;
    const auto dazu = [&](std::string_view wert, std::vector<std::string> schluessel) { q.emplace_back(std::string(wert), std::move(schluessel)); };
    const auto alle = [&](std::initializer_list<std::string_view> werte, const std::vector<std::string>& schluessel) {
        for (const std::string_view w : werte) dazu(w, schluessel);
    };
    const bool rohr = name == "marimba" || name == "xylophone" || name == "vibraphone";
    if (rohr || name == "glockenspiel" || name == "tubular_bells" || name == "steelpan")
        alle({"malletMass", "malletStiffness", "malletExponent"}, {"rucz2021"});
    if (rohr)
    {
        dazu(kPartnerZielDb, {"rucz2021"});
        dazu(kPartnerZielNote, {"rucz2021"});
    }
    if (name == "celesta")
    {
        alle({"malletMass", "malletStiffness", "malletExponent", "malletHysteresis", "malletRelaxation"}, {"stulov2009"});
        dazu(kPartnerZielDb, {"rucz2021"});
    }
    if (name == "reed_electric_piano")
    {
        // WU2 a (S183): der Hammer ist eine Einschaetzung zum Bauteil (Manual: Ahorn mit Filz), nicht mehr Stulov — `lumiviz`
        // WU1 a (S183): der Schlitz nach Pfeifle 2017 Fig. 2, der Arbeitspunkt nach dem Service Manual (Plattenende aufbiegen)
        dazu("pickupDistance", {"pfeifle2017", "wurlitzer-manual-200"});
        dazu("pickupOffset", {"wurlitzer-manual-200"});
    }
    if (name == "tine_electric_piano")
    {
        alle({"malletMass", "malletStiffness", "malletExponent", "malletHuntCrossley", "malletDampingHc", "velocityMax"}, {"soenderbo2024"});
        dazu("pickupDistance", {"falaize2015"});
    }
    if (name == "steelpan") dazu(kFesteVerhaeltnisse, {"thomas2014"});
    return q;
}

Parametertafel baueTafel(const Vorlage& vorlage)
{
    const StabSpiel& vorgabe = vorlage.spiel;
    Parametertafel t;
    t.kern = std::string(kKernName);
    t.titel = "Bar and plate";
    t.rechenFassung = 2;  // S183 (LP2 b): Zupfen mit Kraft, Abnahmen Schlitz und Steg — die Eingaben-Abdruecke aendern sich
    t.noteMin = kUmfangMin;
    t.noteMax = kUmfangMax;
    t.gruppen = {"Keys", "Bar", "Partner", "Mallet", "Damper", "Motor", "Pickup", "Bow", "Output"};
    t.umfangVon = std::string(kNameNoteMin);
    t.umfangBis = std::string(kNameNoteMax);
    const Werte v = werteAus(vorgabe);
    for (const std::string& gruppe : t.gruppen)
    {
        for (const Feld& f : felder())
            if (f.angabe.gruppe == gruppe)
            {
                ParameterAngabe a = f.angabe;
                a.vorgabe = f.lies(vorgabe);
                belege(a);
                a.quellen = quellenIn(vorlage, a.name);
                a.klangbestimmend = a.name != kNameNoteMin && a.name != kNameNoteMax && a.name != kPartnerZielNote;
                t.werte.push_back(std::move(a));
            }
        for (const KurvenFeld& k : kurvenFelder())
            if (k.gruppe == gruppe)
            {
                ParameterAngabe a = angabe(k.name, k.titel, k.gruppe, WertArt::Kurve, k.einheit, k.von, k.bis);
                a.skala = k.skala;
                a.vorgabe = vorgabe.*k.feld;
                a.belegkraft = (vorgabe.*k.feld).belegkraft;
                a.herkunft = (vorgabe.*k.feld).herkunft;
                a.quellen = (vorgabe.*k.feld).quellen;
                t.werte.push_back(std::move(a));
            }
        if (gruppe == "Bar")
        {
            ParameterAngabe l = angabe(kFesteVerhaeltnisse, "Fixed mode ratios", "Bar", WertArt::Liste);
            l.felder.push_back(angabe(kVerhaeltnis, "Ratio", "", WertArt::Zahl, "", 0.1, 100.0));
            l.felder.back().vorgabe = 1.0;
            l.eintraegeMin = 0;
            l.eintraegeMax = static_cast<std::size_t>(kBiegeModenMax);
            l.vorgabe = *v.finde(kFesteVerhaeltnisse);
            l.herkunft = "empty = the curves; the steelpan's tone field (Thomas, Monteil & Touze 2014, report only)";
            l.quellen = quellenIn(vorlage, l.name);
            t.werte.push_back(std::move(l));
        }
        if (gruppe == "Partner")
        {
            // KS2 a: der Pegel des Partners, gestimmt durch das Modell auf das Ziel
            ParameterAngabe g = gestimmteAngabe(kPartnerPegel, "Partner gain", "Partner", "", 1e-6, 1e6);
            g.herkunft = "tuned through the model: the partner raises the level by the target at its key";
            g.quellen = {"rucz2021"};
            t.werte.push_back(std::move(g));
        }
        if (gruppe == "Bow")
        {
            // KS3 a: die Fenster der Bogenkraft je Stufe, nach dem Pegel des Partners gestimmt (sie haengen an ihm)
            static constexpr std::string_view kStufe[3] = {"pp", "mf", "ff"};
            for (int i = 0; i < 3; ++i)
            {
                t.werte.push_back(gestimmteAngabe(kBogenUnten[i], std::string("Bow force low ") + std::string(kStufe[i]), "Bow", "N", 1e-3, 1e3));
                t.werte.push_back(gestimmteAngabe(kBogenOben[i], std::string("Bow force high ") + std::string(kStufe[i]), "Bow", "N", 1e-3, 1e3));
            }
        }
    }
    ParameterAngabe bezug = angabe(kBezug, "Output reference", "Output", WertArt::Zahl, "", 1e-12, 1e12);
    bezug.vorgabe = 1.0;
    bezug.belegkraft = Belegkraft::Rechnung;
    bezug.herkunft = "normiere: peak 0.5 at the middle key of the instrument, level 0 (IK2-10 a)";
    bezug.klangbestimmend = false;
    t.werte.push_back(std::move(bezug));
    t.bezugWert = std::string(kBezug);
    return t;
}

bool getreten(const Tonauftrag& auftrag, std::string_view spurName, Frames beginn)
{
    const Spur* s = findeSpur(auftrag, spurName);
    return s != nullptr && spurWert(*s, beginn, 0.0) >= 0.5;
}

}  // namespace

const std::vector<Vorlage>& vorlagen()
{
    static const std::vector<Vorlage> v = [] {
        std::vector<Vorlage> l = {
            {"marimba", "Marimba", baueVorgabeMarimba(), true, {}},
            {"xylophone", "Xylophone", baueVorgabeXylophon(), true, {}},
            {"vibraphone", "Vibraphone", baueVorgabeVibraphon(), true, {}},
            {"glockenspiel", "Glockenspiel", baueVorgabeGlockenspiel(), true, {}},
            {"tubular_bells", "Tubular bells", baueVorgabeRoehrenglocken(), true, {}},
            {"celesta", "Celesta", baueVorgabeCelesta(), true, {}},
            {"kalimba", "Kalimba", baueVorgabeKalimba(), true, {}},
            {"tine_electric_piano", "Tine electric piano", baueVorgabeRhodes(), true, {}},
            {"reed_electric_piano", "Reed electric piano", baueVorgabeWurlitzer(), true, {}},
            {"steelpan", "Steelpan", baueVorgabeSteeldrum(), false, {}},
        };
        for (Vorlage& x : l) x.quellen = quellenDesStabs(x.name);
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
        t.auskunft.spuren = {std::string(spur::kExpression), std::string(kSpurMotor), std::string(kSpurGestrichen), std::string(kSpurSchlaegelOrt),
                             std::string(kSpurDaempferAndruck)};
        t.auskunft.innere = {"contact"};
        t.auskunft.vorbereitungTeuer = false;
        t.erzeuge = [](const Werte& w) -> std::shared_ptr<const Tonkern> { return std::make_shared<const StabKern>(spielAus(w)); };
        return t;
    }();
    return typ;
}

Werte werteAus(const StabSpiel& spiel)
{
    Werte w;
    for (const Feld& f : felder()) w.felder.emplace_back(f.angabe.name, f.lies(spiel));
    for (const KurvenFeld& k : kurvenFelder()) w.felder.emplace_back(std::string(k.name), spiel.*k.feld);
    Liste l;
    for (const double r : spiel.festeVerhaeltnisse)
    {
        Werte e;
        e.setze(std::string(kVerhaeltnis), r);
        l.eintraege.push_back(std::move(e));
    }
    w.felder.emplace_back(std::string(kFesteVerhaeltnisse), std::move(l));
    w.felder.emplace_back(std::string(kPartnerPegel), spiel.partnerAbstrahl);
    for (int i = 0; i < 3; ++i)
    {
        w.felder.emplace_back(std::string(kBogenUnten[i]), spiel.bogenKraftUnten[static_cast<std::size_t>(i)]);
        w.felder.emplace_back(std::string(kBogenOben[i]), spiel.bogenKraftOben[static_cast<std::size_t>(i)]);
    }
    w.felder.emplace_back(std::string(kBezug), spiel.ausgangBezug);
    return w;
}

StabSpiel spielAus(const Werte& werte)
{
    StabSpiel s;
    for (const Feld& f : felder())
        if (const Wert* w = werte.finde(f.angabe.name)) f.setze(s, *w);
    for (const KurvenFeld& k : kurvenFelder())
        if (const Wert* w = werte.finde(k.name))
            if (const Kurve* kurve = std::get_if<Kurve>(w)) s.*k.feld = *kurve;
    if (const Wert* w = werte.finde(kFesteVerhaeltnisse))
        if (const Liste* l = std::get_if<Liste>(w))
            for (const Werte& e : l->eintraege)
                if (const Wert* r = e.finde(kVerhaeltnis))
                    if (const auto* d = std::get_if<double>(r)) s.festeVerhaeltnisse.push_back(*d);
    if (const Wert* w = werte.finde(kPartnerPegel))
        if (const Kurve* k = std::get_if<Kurve>(w)) s.partnerAbstrahl = *k;
    for (int i = 0; i < 3; ++i)
    {
        if (const Wert* w = werte.finde(kBogenUnten[i]))
            if (const Kurve* k = std::get_if<Kurve>(w)) s.bogenKraftUnten[static_cast<std::size_t>(i)] = *k;
        if (const Wert* w = werte.finde(kBogenOben[i]))
            if (const Kurve* k = std::get_if<Kurve>(w)) s.bogenKraftOben[static_cast<std::size_t>(i)] = *k;
    }
    if (const Wert* w = werte.finde(kBezug))
        if (const auto* d = std::get_if<double>(w)) s.ausgangBezug = *d;
    // L11: die Entartung der Kopplung meiden (die Tafel richtet die Grenzen, diese Regel nicht)
    richteStabSpiel(s, s);
    return s;
}

StabAnschlag anschlagAus(const StabSpiel& spiel, const Tonauftrag& auftrag)
{
    StabAnschlag a;
    a.rate = static_cast<int>(auftrag.rate);
    a.stufe = auftrag.stufe;
    if (auftrag.noten.empty()) return a;
    const KettenNote& n = auftrag.noten.front();
    a.note = n.note;
    a.hz = n.hz;
    const Frames halten = n.ende - n.beginn;
    a.daempferAb = halten;
    if (const Spur* p = findeSpur(auftrag, kSpurDaempferAndruck)) a.andruck = std::clamp(spurWert(*p, n.beginn, 1.0), 0.0, 1.0);
    if (const Spur* o = findeSpur(auftrag, kSpurSchlaegelOrt)) a.stelleOffset = spurWert(*o, n.beginn, 0.0);
    if (const Spur* m = findeSpur(auftrag, kSpurMotor))
        for (const SpurPunkt& p : m->punkte)
            a.motor.push_back(MotorStelle{static_cast<double>(p.frame - n.beginn) / auftrag.rate, std::max(0.0, p.wert)});
    a.motorPhaseStart = 0.0;
    a.gestrichen = spiel.bogen && getreten(auftrag, kSpurGestrichen, n.beginn) && n.note >= spiel.bogenAb;
    if (a.gestrichen)
    {
        a.haltenS = static_cast<double>(halten) / auftrag.rate;
        if (const Spur* e = findeSpur(auftrag, spur::kExpression)) a.expression = std::clamp(spurWert(*e, n.beginn, 0.5), 0.0, 1.0);
    }
    return a;
}

StabKern::StabKern(StabSpiel spiel) : m_spiel(std::move(spiel)) {}

Frames StabKern::nachklang(const Tonauftrag& auftrag, Frames hoechstens) const
{
    if (auftrag.noten.empty()) return std::min<Frames>(hoechstens, 1);
    const StabAnschlag a = anschlagAus(m_spiel, auftrag);
    const KettenNote& n = auftrag.noten.front();
    const Frames halten = n.ende - n.beginn;
    // vom Anschlag an gerechnet (ohne Daempfer); nach dem Ende der Note der Rest
    Frames rest = std::max<Frames>(1, stabNachklang(m_spiel, a, hoechstens + halten) - halten);
    if (m_spiel.daempfer != DaempferArt::Keiner && a.andruck > 0.0)
    {
        // unter dem Daempfer klingt jede Mode hoechstens mit seiner Abklingzeit (er verkuerzt, nie verlaengert er)
        const int note = std::clamp(a.note, m_spiel.noteMin, m_spiel.noteMax);
        const double sigma = sigmaAusT60(wertAn(m_spiel.daempferT60S, note, Skala::Logarithmisch)) * a.andruck;
        if (sigma > 0.0) rest = std::min<Frames>(rest, static_cast<Frames>(std::ceil(kLn1000 / sigma * auftrag.rate)));
    }
    return std::clamp<Frames>(rest, 1, std::max<Frames>(1, hoechstens));
}

void StabKern::rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere) const
{
    if (auftrag.noten.empty())
    {
        ton.assign(static_cast<std::size_t>(std::max<Frames>(0, auftrag.frames)), 0.0f);
        return;
    }
    StabMessung messung;
    const bool messen = innere != nullptr && std::find(innere->gewuenscht.begin(), innere->gewuenscht.end(), "contact") != innere->gewuenscht.end();
    rechneStabTon(m_spiel, anschlagAus(m_spiel, auftrag), ton, auftrag.frames, messen ? &messung : nullptr);
    if (messen) innere->ergebnis.push_back(InnereGroesse{"contact", "s, N", {messung.kontaktS, messung.kraftSpitzeN}});
}

}  // namespace lumi::kern::stab
