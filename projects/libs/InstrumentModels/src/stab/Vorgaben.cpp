/**
 * @file Vorgaben.cpp
 * @brief InstrumentModels, Klasse 4: die Vorgaben der zehn Instrumente (`baueVorgabe…()`), mit Herkunft je Groesse, und
 *        die Grenzen (`richteStabSpiel`)
 *
 * Uebernommen aus IC `rechnung/klassen/04_platte_stab/Vorgaben.cpp` (`bb7e6d6`), die Werte wortgleich; geaendert
 * (Instrumente_IC_Konzept.md 34g-3, Uebernahme-Journal): der Pegel des Partners wird nicht hier durch das Modell
 * gesucht, sondern ist ein gestimmter Wert mit Ziel (`partnerZielNote`, `partnerZielDb`; KS2 a, Stimmer in
 * `InstrumentMeasure/StabStimmen`); `normiere` ist `kern::normiereNachTafel` (IK2-10 a); die Fenster des Bogens stimmt
 * der Stimmer (KS3 a, statt `Stimmung.inc`); das Kennzeichen „ungemessen“ folgt aus der Herkunft (IK2-9). Die Quellen
 * je Wert (WZ1 a) setzt `mitQuellen` am Ende jeder Vorgabe.
 *
 * **Herkunft** (AUFTRAG_BAU §4 Punkt 2): *Quelle* = selbst nachgelesen (IC Session 1 im Volltext, so im Entwurf
 * vermerkt; Rucz & Ulveczki 2021 lag im Cache der Sitzung, nicht im Repo); *nur Bericht* = aus einem Agentenbericht
 * unter `recherche/` mit Wortlaut, an der Quelle nicht geprueft — Belegkraft „offen“; *Rechnung* = abgeleitet, die
 * Rechnung steht hier; *offen* = Vorgabe mit Grund, zu setzen an der Messgroesse aus `messplan.md`. Die Tabelle
 * aller Groessen steht in `klassen/Klasse 4 - Platte und Stab/modell.md`, das Offene in `offen.md`.
 *
 * Reihenfolge nach F-1 a (T8 a): Kern an Marimba, Xylophon, Vibraphon, Glockenspiel → Roehrenglocken, Celesta,
 * Kalimba → Rhodes, Wurlitzer mit Tonabnehmer → Steeldrum als Deluxe; nichts zurueckgestellt, ungemessen
 * gekennzeichnet.
 */
#include "InstrumentModels/stab/Stab.hpp"

#include "InstrumentModels/gemeinsam/Balken.hpp"
#include "InstrumentModels/hammer/HammerSaite.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>
#include <utility>

namespace lumi::kern::stab
{

namespace
{

constexpr double kPi = std::numbers::pi;

/// Rucz & Ulveczki 2021, Tab. 1 (S. 5), Orff-Xylophon F4 aus Palisander: Stab 348,8 Hz, ξ_m,1 = 2,57·10⁻⁴, 96,7 g;
/// Resonator 349,1 Hz, ξ_a,1 = 8,8·10⁻³; Schlaegel 25 g, K_M = 5·10⁷ N/m^1,5 (Hertz, Gl. 6), Kontakt 0,6 ms bei
/// 1 m/s (nachgelesen IC Session 1, `modell-entwurf.md` §3, §5).
constexpr double kRuczStabHz = 348.8;
constexpr double kRuczXiStab = 2.57e-4;
constexpr double kRuczMasseKg = 0.0967;
constexpr double kRuczRohrHz = 349.1;
constexpr double kRuczXiRohr = 8.8e-3;
constexpr double kRuczSchlaegelKg = 0.025;
constexpr double kRuczSchlaegelK = 5e7;  // N/m^1,5
/// „by as much as 15 dB“ (Rucz S. 7): so viel hebt ein gestimmter Resonator den Pegel.
constexpr double kRuczResonatorDb = 15.0;
const std::string kRucz = "Rucz & Ulveczki 2021, ICSV27, Tab. 1 S. 5 (nachgelesen IC Session 1)";

/// Die Schluessel der Quellen je Kurve (WZ1 a) aus der Herkunft, die IC je Groesse fuehrt: eine Tafel von Namen im Text
/// zu Schluesseln des Registers (`asset/loop/instrument_quellen.json`); eine Kurve ohne genannte Quelle (Rechnung,
/// Einschaetzung, offen) traegt `lumiviz`. Der Waechter `*Instrument Quellen*` prueft jeden Schluessel.
void mitQuellen(Kurve& k)
{
    static const std::pair<const char*, const char*> kTafel[] = {
        {"Rucz", "rucz2021"},           {"Chapman", "chapman2006"},       {"Falaize", "falaize2015"},
        {"Soenderbo", "soenderbo2024"}, {"Pfeifle", "pfeifle2017"},       {"Thomas, Monteil", "thomas2014"},
        {"CCRMA", "ccrma"},
        // S183 (LP2 b): die Zungen je Taste aus Haendler- und Forenberichten und dem Service Manual (alle nur Bericht)
        {"Vintage Vibe", "vintagevibe-wurlitzer-reeds"}, {"DocWurly", "epforum-wurlitzer-reeds"}, {"Paleophone", "paleophone-wurlitzer-reeds"},
        {"Service Manual", "wurlitzer-manual-200"},     {"cmuse", "cmuse-kalimba"},
    };
    // nicht in der Tafel: La Favre (Torsion „ohne Zahl“) und das Patent (Daempfer „unklar“) — die Werte dort sind
    // Einschaetzungen, nicht aus der Quelle; ebenso Iowa (nur als Ziel einer spaeteren Messung genannt)
    k.quellen.clear();
    for (const auto& [name, schluessel] : kTafel)
        if (k.herkunft.find(name) != std::string::npos) k.quellen.emplace_back(schluessel);
    if (k.quellen.empty()) k.quellen.emplace_back("lumiviz");
}

void mitQuellen(StabSpiel& s)
{
    for (Kurve* k : {&s.grundJeNote, &s.verhaeltnis2, &s.verhaeltnis3, &s.t60S, &s.daempfungExponent, &s.masseKg, &s.torsionVerhaeltnis,
                     &s.torsionT60S, &s.abstrahlExponent, &s.partnerVerhaeltnis, &s.partnerXi, &s.kopplungJeKritisch, &s.partnerAbstrahl,
                     &s.anschlagStelle, &s.querVersatz, &s.daempferT60S, &s.abnehmerStelle})
        mitQuellen(*k);
    for (Kurve& k : s.bogenKraftUnten) mitQuellen(k);
    for (Kurve& k : s.bogenKraftOben) mitQuellen(k);
}

Kurve konst(double w, Belegkraft b, const std::string& h, const StabSpiel& s)
{
    return kurveKonstant(w, b, h, s.noteMin, s.noteMax);
}

double hzVon(int note)
{
    return 440.0 * std::exp2((note - 69) / 12.0);
}

/// T60 bei festem Daempfungsgrad ξ ueber den Umfang: T60 = ln1000/(ξ·2πf); Punkte an Rand und Mitte (in ln T60
/// linear in der Note — exakt fuer f = 440·2^((n−69)/12)).
Kurve t60BeiXi(double xi, const StabSpiel& s, Belegkraft b, const std::string& h)
{
    Kurve k;
    for (const int n : {s.noteMin, (s.noteMin + s.noteMax) / 2, s.noteMax}) k.punkte.push_back({n, kLn1000 / (xi * 2.0 * kPi * hzVon(n))});
    k.belegkraft = b;
    k.herkunft = h;
    return k;
}

Stossanreger ruczSchlaegel()
{
    Stossanreger a;
    a.masseKg = kRuczSchlaegelKg;
    a.q0 = kRuczSchlaegelK * std::pow(1e-3, 1.5);  // N/m^1,5 → N/mm^1,5
    a.p = 1.5;
    return a;
}

// --- S183 (LP2 b): die Zungen von Wurlitzer und Kalimba je Taste — Herleitung mit Quellen und Proben in
// `docs/modellierung/anbindung-ic/2026-11-09_S183_vorgaben_je_taste.md` ---------------------------------------------

constexpr double kStahlE = 200e9;     // Pa, Federstahl (Lehrbuch)
constexpr double kStahlRho = 7850.0;  // kg/m³
constexpr double kZoll = 0.0254;
/// Modale Masse der ersten Mode des Kragbalkens je Gesamtmasse bei Form 1 an der Spitze — so normiert der Kern seine Formen.
constexpr double kKragModalAnteil = 0.2427;
/// f₁ = 0,1615·t/L²·√(E/ρ) des gleichfoermigen Kragbalkens ((β₁L)²/(2π·√12), β₁L = 1,8751).
constexpr double kKragFrequenzFaktor = 0.1615;

/// Die Zunge des Wurlitzer Nr. n = MIDI − 32 (WU1 a): Gesamtlaenge 3″ − n/20″ bis #20 (2″), ab #21 je 1/44″ weniger bis
/// #64 (1″) — DocWurly (EP-Forum 2017) und Paleophone gleichlautend (nur Bericht); Breite je Gruppe .155/.129/.120/.110/.097″
/// (Vintage Vibe, Case Study 200A, nur Bericht); Dicke der Zunge .020″ bis #20, .026″ ab #21 (Vintage Vibe 200A-Text, nur
/// Bericht; das EP-Forum misst am 140B .024–.026″ bis #42 und .022″ darueber); Stahl „Swedish Sandvik“ (Service Manual S. 2).
/// Einspannung c aus Zunge #64 ohne Lot (Annahme); Steife an der Spitze k = E·w·t³/(4·L_frei³); das Lot aus f₀:
/// m_Lot = k/(2πf₀)² − 0,2427·m_Zunge, nicht unter 0 (die Zunge klingt in der Grundmode, Pfeifle 2017 S. DAFX-21); die Masse,
/// die der Kern als gleichfoermigen Stab erwartet: m = m_Zunge + m_Lot/0,2427 — 3,2 g (A1) … 0,2 g (C7), heute 2 g fest.
double wurlitzerMasseKg(int n)
{
    const auto gesamt = [](int k) { return (k <= 20 ? 3.0 - k / 20.0 : 2.0 - (k - 20) / 44.0) * kZoll; };
    const auto breite = [](int k) { return (k <= 14 ? 0.155 : k <= 20 ? 0.129 : k <= 42 ? 0.120 : k <= 50 ? 0.110 : 0.097) * kZoll; };
    const auto dicke = [](int k) { return (k <= 20 ? 0.020 : 0.026) * kZoll; };
    const double wurzel = std::sqrt(kStahlE / kStahlRho);
    const double lFrei64 = std::sqrt(kKragFrequenzFaktor * dicke(64) * wurzel / hzVon(96));
    const double c = gesamt(64) - lFrei64;
    const double l = gesamt(n) - c;
    const double w = breite(n);
    const double t = dicke(n);
    const double k = kStahlE * w * t * t * t / (4.0 * l * l * l);
    const double mZunge = kStahlRho * w * t * l;
    const double omega = 2.0 * kPi * hzVon(32 + n);
    const double mLot = std::max(0.0, k / (omega * omega) - kKragModalAnteil * mZunge);
    return mZunge + mLot / kKragModalAnteil;
}

Kurve wurlitzerMasse()
{
    Kurve k;
    for (const int n : {1, 7, 14, 15, 20, 21, 27, 33, 39, 42, 43, 50, 51, 57, 64}) k.punkte.push_back({32 + n, wurlitzerMasseKg(n)});
    k.belegkraft = Belegkraft::Offen;
    k.herkunft = "Rechnung je Zunge (wurlitzerMasseKg) aus Laenge (DocWurly, Paleophone), Breite und Dicke (Vintage Vibe 200A) — nur "
                 "Bericht — und dem Lot aus f0 (Grundmode nach Pfeifle 2017; Stahl nach dem Service Manual): 3,2 g (A1) … 0,2 g (C7); "
                 "S183 WU1 a, Herleitung 2026-11-09_S183_vorgaben_je_taste.md §2";
    return k;
}

/// Die Zunge der Kalimba (KA1 a): freie Laenge a(f) = A/√f mit A = 91,8 cm·√Hz aus Chapman 2006 Tab. 1 S. 27 (a·√f₀ = 89 … 94
/// fuer die fuenf Zungen D3 … D4, nachgelesen S183; Kragbalken f ∝ 1/a²), Federstahl 1,2 × 5 mm („around 1.2 millimeters is
/// typical“, cmuse, nur Bericht — am gleichfoermigen Kragbalken auf 10–20 % an Chapmans f₀, den Rest nimmt der Abschnitt
/// hinter dem Steg): m = ρ·w·t·a — 3,6 g (D3) … 1,3 g (D6), heute 3 g fest.
constexpr double kChapmanAWurzelF = 0.918;  // m·√Hz
constexpr double kKalimbaDickeM = 1.2e-3;
constexpr double kKalimbaBreiteM = 5.0e-3;

Kurve kalimbaMasse(const StabSpiel& s)
{
    Kurve k;
    for (const int n : {s.noteMin, 62, 74, s.noteMax})
        k.punkte.push_back({n, kStahlRho * kKalimbaBreiteM * kKalimbaDickeM * kChapmanAWurzelF / std::sqrt(hzVon(n))});
    k.belegkraft = Belegkraft::Offen;
    k.herkunft = "Rechnung: m = ρ·w·t·a mit a(f) = 91,8 cm·√Hz/√f aus Chapman 2006 Tab. 1 (nachgelesen S183), Federstahl 1,2 × 5 mm "
                 "(cmuse, nur Bericht): 3,6 g (D3) … 1,3 g (D6); S183 KA1 a, Herleitung 2026-11-09_S183_vorgaben_je_taste.md §3";
    return k;
}

/// Gemeinsame Vorgaben eines Stabs aus Holz mit Unterschnitt (Marimba, Xylophon).
StabSpiel holzStab(const std::string& name, int von, int bis, double r2, double r3, const std::string& stimmung)
{
    StabSpiel s;
    s.name = name;
    s.noteMin = von;
    s.noteMax = bis;
    s.grundJeNote = konst(1.0, Belegkraft::Rechnung, "der Stab klingt auf seiner ersten Biegemode", s);
    s.verhaeltnis2 = konst(r2, Belegkraft::Quelle, stimmung, s);
    s.verhaeltnis3 = konst(r3, Belegkraft::Quelle, stimmung, s);
    s.t60S = t60BeiXi(kRuczXiStab, s, Belegkraft::Offen,
                      "ξ_1 = 2,57e-4 aus " + kRucz + " (F4, Palisander); ueber den Umfang offen: fester ξ (T60 ∝ 1/f); "
                      "zu setzen an Iowa (M2)");
    s.daempfungExponent = konst(0.0, Belegkraft::Offen,
                                "offen: fester Verlustfaktor; Rucz S. 7: hoehere Teiltoene „significantly greater“ — erwartet "
                                "κ > 0; zu setzen an Iowa (M2)", s);
    s.masseKg = konst(kRuczMasseKg, Belegkraft::Offen, "96,7 g aus " + kRucz + " (F4); ueber den Umfang offen (M4)", s);
    s.torsionVerhaeltnis = konst(3.0, Belegkraft::Offen,
                                 "offen, Platzhalter ohne Wirkung bei Versatz 0 (La Favre misst die erste Torsion — nur Bericht, "
                                 "ohne Zahl); zu setzen am Modalfit (M1)", s);
    s.torsionT60S = s.t60S;
    s.torsionT60S.herkunft = "offen: wie die erste Biegemode";
    s.abstrahlExponent = konst(0.0, Belegkraft::Offen, "offen: alle Moden gleich gewichtet (Ausgang ist q̈, die Hoehen schon ∝ ω²); M3", s);
    s.partner = PartnerArt::RohrGeschlossen;
    s.partnerVerhaeltnis = konst(kRuczRohrHz / kRuczStabHz, Belegkraft::Quelle, "Resonator 349,1 Hz gegen Stab 348,8 Hz, " + kRucz, s);
    s.partnerXi = konst(kRuczXiRohr, Belegkraft::Quelle, "ξ_a,1 = 8,8e-3, " + kRucz + "; ueber den Umfang offen (M5)", s);
    s.kopplungJeKritisch = konst(1.2, Belegkraft::Offen,
                                 "offen: ueber der kritischen Kopplung (Rucz: T60 des Grundtons 0,7 s = Mittel der Daempfungen, das "
                                 "verlangt κ ≥ κ_c); 1,2 meidet die Entartung bei 1 (L11) und haelt die Schwebung unter 2 Hz; M5", s);
    s.partnerAbstrahl = konst(1.0, Belegkraft::Rechnung, "gesetzt: +15 dB Pegel mit Rohr an F4 („by as much as 15 dB“, Rucz S. 7)", s);
    s.schlaegel = ruczSchlaegel();
    s.anschlagStelle = konst(0.5, Belegkraft::Offen, "offen: Mitte (messplan M3: „Mitte üblich“, Einschaetzung); Spur Schlaegel-Ort (T3b a)", s);
    s.querVersatz = konst(0.0, Belegkraft::Offen, "offen: auf der Laengsachse (dann ohne Torsion); M1", s);
    s.tempoMax = 5.0;
    s.daempferT60S = konst(0.2, Belegkraft::Offen, "ohne Daempfer", s);
    s.abnehmerStelle = konst(0.5, Belegkraft::Offen, "akustisch — ohne Bedeutung", s);
    return s;
}

}  // namespace

StabSpiel baueVorgabeMarimba()
{
    // Umfang C2–C7 wie Iowa MIS (nur Bericht, Instrumentseite)
    StabSpiel s = holzStab("Marimba", 36, 96, 4.0, 10.0, "Unterschnitt „1 : 4 : 10“, Rucz & Ulveczki 2021 S. 1 (nachgelesen); CCRMA nennt 9,2 (nur Bericht)");
    // IC-Kennzeichen "ungemessen" (IK2-9: folgt hier aus der Herkunft)
    s.partnerZielNote = 65;  // KS2 a: der Stimmer setzt den Pegel durch das Modell
    s.partnerZielDb = kRuczResonatorDb;
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

StabSpiel baueVorgabeXylophon()
{
    // Umfang F4–C8 wie Iowa MIS (nur Bericht); Rucz rechnete ein Orff-Xylophon — die Werte gelten hier unmittelbar
    StabSpiel s = holzStab("Xylophon", 65, 108, 3.0, 9.0, "„1 : 3 : 9“, Rucz & Ulveczki 2021 S. 1 (nachgelesen)");
    s.schlaegel = ruczSchlaegel();
    s.partnerZielNote = 65;  // KS2 a: der Stimmer setzt den Pegel durch das Modell
    s.partnerZielDb = kRuczResonatorDb;
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

StabSpiel baueVorgabeVibraphon()
{
    // Umfang C3–F6 wie Iowa MIS (nur Bericht)
    StabSpiel s = holzStab("Vibraphon", 48, 89, 4.0, 4.0 * 5.4039 / 2.7565, "1 : 4 (CCRMA, nur Bericht)");
    s.verhaeltnis3.belegkraft = Belegkraft::Offen;
    s.verhaeltnis3.herkunft = "offen: ueber der zweiten Mode der Abstand des gleichfoermigen Balkens (4·5,404/2,757)";
    s.t60S = t60BeiXi(1e-4, s, Belegkraft::Offen,
                      "offen: ξ = 1e-4 (Aluminium daempft schwaecher als Palisander mit 2,57e-4, Einschaetzung); M2 an Iowa „Sustain“");
    s.masseKg = konst(kRuczMasseKg, Belegkraft::Offen, "offen: wie der Palisanderstab bei Rucz (keine Masse eines Vibraphonstabs mit Quelle)", s);
    s.partnerXi.belegkraft = Belegkraft::Offen;
    s.partnerXi.herkunft = "ξ_a aus " + kRucz + " uebertragen (dort Xylophon); M5";
    s.daempfer = DaempferArt::Leiste;
    s.daempferT60S = konst(0.3, Belegkraft::Offen,
                           "offen: Filzleiste (Patent: „halbe Amplitude in 1,5 s“ — nur Bericht, unklar ob gedaempft); M7 an Iowa „Dampen“", s);
    s.motor = true;
    s.restOeffnung = 0.0;
    s.hochlaufS = 0.5;
    // gestrichen (T1 a, K4 F-4 a): der Bogen am Ende des Stabs; Reibung, Haftreibung und Weg v_B von Klasse 3
    // uebertragen (offen), die Fenster der Kraft am Modell (der Stimmer, KS3 a)
    s.bogen = true;
    s.bogenStelle = 0.0;
    s.partnerZielNote = 65;  // KS2 a: der Stimmer setzt den Pegel durch das Modell
    s.partnerZielDb = kRuczResonatorDb;
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

StabSpiel baueVorgabeGlockenspiel()
{
    // Umfang G5–C8 (CCRMA, nur Bericht)
    StabSpiel s = holzStab("Glockenspiel", 79, 108, 2.7565, 5.4039, "gleichfoermiger Stahlstab ohne Unterschnitt: frei–frei 1 : 2,757 : 5,404 (Rechnung, Balken)");
    s.verhaeltnis2.belegkraft = s.verhaeltnis3.belegkraft = Belegkraft::Rechnung;
    s.t60S = t60BeiXi(1e-4, s, Belegkraft::Offen, "offen: ξ = 1e-4 (Stahl, Einschaetzung); M2 an Iowa „Bells“");
    s.daempfungExponent = konst(1.0, Belegkraft::Offen, "offen: CCRMA „All but the lowest transverse mode die out quickly“ (nur Bericht) → κ = 1", s);
    s.masseKg = konst(0.05, Belegkraft::Offen, "offen: 50 g (Einschaetzung, kleiner Stahlstab); M4", s);
    s.partner = PartnerArt::Keiner;
    s.schlaegel = ruczSchlaegel();
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

StabSpiel baueVorgabeRoehrenglocken()
{
    // Umfang C3–F4 wie Freesound sgossner (nur Bericht)
    StabSpiel s = holzStab("Roehrenglocken", 48, 65, 2.7565, 5.4039, "frei–freies Rohr (Biegung wie ein Balken): 1 : 2,757 : 5,404 (Rechnung)");
    s.verhaeltnis2.belegkraft = s.verhaeltnis3.belegkraft = Belegkraft::Rechnung;
    const std::vector<double> v = freiFreiVerhaeltnisse(4);
    s.grundJeNote = konst(2.0 / v[3], Belegkraft::Offen,
                          "gehoert wird eine Oktave unter Mode 4 (CCRMA, nur Bericht): f_1 = 2·f_Note/(f_4/f_1), f_4/f_1 = 8,933 (Rechnung)", s);
    s.t60S = t60BeiXi(5e-5, s, Belegkraft::Offen, "offen: ξ = 5e-5 (Messingrohr klingt lang, Einschaetzung); M2");
    s.masseKg = konst(1.0, Belegkraft::Offen, "offen: 1 kg (Einschaetzung, Messingrohr um 1,5 m)", s);
    s.anschlagStelle = konst(0.02, Belegkraft::Offen, "offen: am oberen Rand (Stopfen, CCRMA nur Bericht)", s);
    s.partner = PartnerArt::Keiner;
    s.daempfer = DaempferArt::Leiste;
    s.daempferT60S = konst(0.5, Belegkraft::Offen, "offen: Pedal-Daempfer (Einschaetzung)", s);
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

StabSpiel baueVorgabeCelesta()
{
    // Umfang C4–C8 offen (keine Fachquelle, Entwurf §4)
    StabSpiel s = holzStab("Celesta", 60, 108, 2.7565, 5.4039, "offen: gleichfoermiger Stahlstab (keine Fachquelle)");
    s.verhaeltnis2.belegkraft = s.verhaeltnis3.belegkraft = Belegkraft::Offen;
    s.t60S = t60BeiXi(1e-4, s, Belegkraft::Offen, "offen: ξ = 1e-4 (Stahl, Einschaetzung)");
    s.masseKg = konst(0.03, Belegkraft::Offen, "offen: 30 g (Einschaetzung)", s);
    s.partner = PartnerArt::EineMode;  // der Holzkasten je Ton als Luftraum mit einer Mode (T4 a)
    s.partnerVerhaeltnis = konst(1.0, Belegkraft::Offen, "offen: der Kasten auf den Grundton gestimmt (Entwurf §4: „wie Kalimba oder Rohr“)", s);
    s.partnerXi = konst(kRuczXiRohr, Belegkraft::Offen, "offen: wie das Rohr bei Rucz", s);
    // der Hammer: Filz nach Stulov & Kartofelev 2009 Gl. 9 (ueber das Vorbild), an der Taste — uebertragen vom Fluegel;
    // ε und Q0 seit HS-3 a (S187) nach Kartofelev & Stulov 2014, der Kontakt 32-fach (τ0 ≈ 1,8 µs, wie der Fluegel)
    s.schlaegel.masseKg = hammer::stulovHammerMasseG(88 - 20) * 1e-3;
    s.schlaegel.q0 = hammer::stulovHammerSteife(88 - 20);
    s.schlaegel.p = hammer::stulovHammerExponent(88 - 20);
    s.schlaegel.eps = hammer::stulovHammerHysterese(88 - 20);
    s.schlaegel.tauS = hammer::stulovHammerRelaxationUs(88 - 20) * 1e-6;
    s.ueberabtastung = 32;
    s.daempfer = DaempferArt::Einzeln;
    s.daempferT60S = konst(0.2, Belegkraft::Offen, "offen: wie der Daempfer des Fluegels im Diskant (Vorbild 0,2 s)", s);
    s.partnerZielNote = 84;  // KS2 a: der Stimmer setzt den Pegel durch das Modell
    s.partnerZielDb = kRuczResonatorDb;
    // das Ziel +15 dB ist vom Rohr bei Rucz uebertragen (offen) — in IC stand das am gesetzten Pegel
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

StabSpiel baueVorgabeKalimba()
{
    StabSpiel s;
    s.name = "Kalimba";
    s.noteMin = 50;  // D3 … offen (Chapman misst D3 und D4)
    s.noteMax = 86;
    s.kragbalken = true;
    s.anregung = Anregung::Zupfen;
    s.grundJeNote = konst(1.0, Belegkraft::Rechnung, "die Zunge klingt auf ihrer ersten Mode", s);
    // Chapmans Notennamen sind Klaviernoten: sein „D3“ ist 293,7 Hz = MIDI 62, „D4“ 587,3 Hz = MIDI 74 (bis S182 an 50/62)
    s.verhaeltnis2 = kurveAusPunkten({{62, 5.86}, {74, 5.30}}, Belegkraft::Quelle,
                                     "Chapman 2006, Tab. 1 S. 27: D3 (293,7 Hz) 5,86, D4 (587,3 Hz) 5,30 (nachgelesen S183)");
    s.verhaeltnis3 = kurveAusPunkten({{62, 5.86 * 2.8}, {74, 5.30 * 2.8}}, Belegkraft::Offen,
                                     "offen: Abstand des Kragbalkens (β3/β2)² = (7,855/4,694)² = 2,80 (Rechnung)");
    s.t60S = konst(2.0, Belegkraft::Offen, "offen: 2 s (Chapman: „two exponentially damped sinusoids“ ohne Zahl)", s);
    s.daempfungExponent = konst(0.0, Belegkraft::Offen, "offen", s);
    s.masseKg = kalimbaMasse(s);  // KA1 a (S183): je Taste aus Chapmans Laengen, statt 3 g fest
    s.torsionVerhaeltnis = konst(0.0, Belegkraft::Offen, "ohne Torsion", s);
    s.torsionT60S = s.t60S;
    s.abstrahlExponent = konst(0.0, Belegkraft::Offen, "offen", s);
    s.anschlagStelle = konst(1.0, Belegkraft::Offen, "an der freien Spitze gezupft", s);
    s.querVersatz = konst(0.0, Belegkraft::Offen, "auf der Achse", s);
    s.tempoMax = 5.0;     // ohne Bedeutung beim Zupfen (KA1 a: die Kraft)
    s.zupfKraftN = 3.0;   // offen: ein kraeftiger Daumen (Einschaetzung); der Bezug kuerzt den Betrag heraus, es zaehlt die Form δ ∝ a³
    s.daempferT60S = konst(0.2, Belegkraft::Offen, "ohne Daempfer", s);
    s.abnehmerStelle = konst(1.0, Belegkraft::Rechnung, "ohne Bedeutung: die Kraft an der Einspannung (KA1 a)", s);
    // KA1 a (S183): das Brett strahlt, getrieben von der Kraft an der Einspannung — nicht die Beschleunigung der Zunge; bei
    // fester Zupfkraft ist diese Kraft ueber den Umfang eben (Probe S183: 1,3 LU statt 37,4)
    s.abnahme = Abnahme::Steg;
    s.partner = PartnerArt::Keiner;  // Kasten offen (Masse und Volumen ohne Quelle): Vorgabe die Brett-Kalimba
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

namespace
{

/// Die Rhodes-Tine nach Falaize & Hélie 2015 (S. 5, nur Bericht, Wortlaut im Bericht): Stahl r = 2 mm,
/// ρ = 7750 kg/m³, E = 180 GPa, 440 Hz bei l = 7,83 cm; die Laenge je Note aus f ∝ 1/l² (Kragbalken, Rechnung).
double tineLaenge(double hz)
{
    return 0.0783 * std::sqrt(440.0 / hz);
}

}  // namespace

StabSpiel baueVorgabeRhodes()
{
    StabSpiel s;
    s.name = "Rhodes";
    s.noteMin = 28;  // Mark I 73: E1–E7 (offen)
    s.noteMax = 100;
    s.kragbalken = true;
    s.grundJeNote = konst(1.0, Belegkraft::Rechnung, "die Tine klingt auf ihrer ersten Mode", s);
    const std::vector<double> beta = kragBetaL(3);
    s.verhaeltnis2 = konst(std::pow(beta[1] / beta[0], 2), Belegkraft::Offen,
                           "offen: Kragbalken (β2/β1)² = 6,267 (Rechnung); die Stimmfeder senkt es (Pfeifle 2017, nur Bericht)", s);
    s.verhaeltnis3 = konst(std::pow(beta[2] / beta[0], 2), Belegkraft::Offen, "offen: Kragbalken (β3/β1)² = 17,55 (Rechnung)", s);
    // Daempfung nach Soenderbo 2024 Tab. 3.1 S. 9 (nur Bericht): σ = σ0 + σ1·β², σ0 = 1e-4 1/s, σ1 = 0,005 m²/s, mit der
    // Laenge der Tine je Note (Rechnung)
    s.t60S.belegkraft = Belegkraft::Offen;
    s.t60S.herkunft = "Soenderbo 2024 Tab. 3.1 (nur Bericht): σ = 1e-4 + 0,005·(1,8751/l)², l nach Falaize & Hélie (Rechnung)";
    for (const int n : {s.noteMin, 64, s.noteMax})
    {
        const double l = tineLaenge(hzVon(n));
        const double sigma = 1e-4 + 0.005 * std::pow(beta[0] / l, 2);
        s.t60S.punkte.push_back({n, kLn1000 / sigma});
    }
    s.daempfungExponent = konst(0.0, Belegkraft::Rechnung, "σ1·β² ∝ f: fester ξ (Rechnung aus Soenderbo)", s);
    s.masseKg.belegkraft = Belegkraft::Rechnung;
    s.masseKg.herkunft = "ρ·π·r²·l mit den Werten von Falaize & Hélie (nur Bericht), l je Note (Rechnung)";
    for (const int n : {s.noteMin, 64, s.noteMax}) s.masseKg.punkte.push_back({n, 7750.0 * kPi * 0.002 * 0.002 * tineLaenge(hzVon(n))});
    s.torsionVerhaeltnis = konst(0.0, Belegkraft::Offen, "ohne Torsion (runde Tine)", s);
    s.torsionT60S = s.t60S;
    s.abstrahlExponent = konst(0.0, Belegkraft::Offen, "ohne Bedeutung (Tonabnehmer)", s);
    s.partner = PartnerArt::EineMode;  // die Tonebar
    s.partnerVerhaeltnis = konst(1.0, Belegkraft::Offen, "offen: Tonebar auf den Grundton gestimmt (Pfeifle 2017: gekoppelt, nur Bericht)", s);
    s.partnerXi = konst(1e-3, Belegkraft::Offen, "offen: ξ = 1e-3 (Einschaetzung, Messingleiste)", s);
    s.kopplungJeKritisch = konst(1.2, Belegkraft::Offen, "offen: wie der Resonator", s);
    s.partnerAbstrahl = konst(0.0, Belegkraft::Rechnung, "die Tonebar wird nicht abgenommen", s);
    // der Hammer: Neopren nach Hunt–Crossley, Soenderbo 2024 Tab. 3.2 S. 14 (nur Bericht): M = 0,011 kg, α = 2,8,
    // k = 1,5e11 N/m^2,8, λ = 9e10 (λ/k = 0,6 s/m); v bis 4 m/s
    s.schlaegel.masseKg = 0.011;
    s.schlaegel.p = 2.8;
    s.schlaegel.q0 = 1.5e11 * std::pow(1e-3, 2.8);
    s.schlaegel.huntCrossley = true;
    s.schlaegel.lambdaHc = 0.6 * 1e-3;  // s/mm
    s.tempoMax = 4.0;
    s.ueberabtastung = 32;
    s.anschlagStelle = konst(2.35 / 7.83, Belegkraft::Offen, "Falaize & Hélie 2015 S. 5: Hammer bei z_h = 2,35 cm von 7,83 cm (nur Bericht)", s);
    s.querVersatz = konst(0.0, Belegkraft::Offen, "auf der Achse", s);
    s.daempfer = DaempferArt::Einzeln;
    s.daempferT60S = konst(0.15, Belegkraft::Offen, "offen: Filzdaempfer (Einschaetzung)", s);
    s.abnahme = Abnahme::Magnetisch;
    s.abnehmerStelle = konst(1.0, Belegkraft::Rechnung, "an der Spitze der Tine", s);
    s.abnehmerAbstandM = 1e-3;  // Falaize & Hélie: „Tonabnehmer 1 mm Abstand“ (nur Bericht)
    s.abnehmerVersatzM = 0.5e-3;  // offen: Voicing — bei 0 fehlte der Grundton (Φ gerade in y)
    // IC-Kennzeichen "ungemessen (Aufnahme nur NC, F-7 c)" (IK2-9: folgt hier aus der Herkunft)
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

StabSpiel baueVorgabeWurlitzer()
{
    StabSpiel s = baueVorgabeRhodes();
    s.name = "Wurlitzer";
    s.noteMin = 33;  // 64 Tasten A1–C7 (offen)
    s.noteMax = 96;
    s.verhaeltnis2.herkunft = "offen: Kragbalken 6,267 (Rechnung); das Lot an der Spitze hebt das Verhaeltnis (Pfeifle 2017: Zunge mit Lot, "
                              "ohne Zahl)";
    s.t60S = t60BeiXi(5e-4, s, Belegkraft::Offen, "offen: ξ = 5e-4 (Einschaetzung)");
    s.masseKg = wurlitzerMasse();  // WU1 a (S183): je Zunge aus den Massen, statt 2 g fest
    s.partner = PartnerArt::Keiner;
    // Hammer (WU2 a, S183): dreilagiger Ahorn mit duennem Filz (Service Manual S. 2), klein und hart — ohne Mass und Steife mit
    // Quelle, Einschaetzung zum Bauteil nach Hunt–Crossley: 3 g, Hertz-Exponent 1,5, 300 N/mm^1,5, Daempfung 0,3 ms/mm (offen bis
    // M8). Bis S182 der Fluegelhammer an C4 (Stulov, 8,3 g, p 4,3), in WU1 a der am Fluegel-Diskant (n = 100, 4,7 g, p 5,2): beide
    // zu schwer und zu weich fuer eine Zunge von 0,2 … 3 g — der Diskant blieb 14 … 21 dB unter der Mitte (Probe S183: 3 g geht,
    // 4 g nicht; ein Filz mit p = 5,2 gibt der leichten Zunge kaum Impuls)
    s.schlaegel = Stossanreger{};
    s.schlaegel.masseKg = 0.003;
    s.schlaegel.q0 = 300.0;
    s.schlaegel.p = 1.5;
    s.schlaegel.huntCrossley = true;
    s.schlaegel.lambdaHc = 0.3e-3;  // s/mm
    s.ueberabtastung = 16;
    // offen (Einschaetzung, zu setzen an einer Aufnahme M8): Blow distance 1 5/64″ (Service Manual S. 17), die Zunge laeuft im
    // Bass 5 … 8 mm weit — der Schlitz saettigt das, er klemmt nicht (WU1 a)
    s.tempoMax = 1.5;
    s.anschlagStelle = konst(0.3, Belegkraft::Offen, "offen", s);
    // WU1 a, WU2 a (S183): der Tonabnehmer ist ein Ausschnitt der geladenen Platte (Pfeifle 2017 Fig. 2): Breite der Glocke
    // T = 1,5 mm (Plattendicke, Einschaetzung), Arbeitspunkt y0 = 0,5·T (das Service Manual S. 43 biegt bei zu lauten Toenen das
    // Plattenende 1/32″ … 1/16″ auf); Probe S183 mit dem Hammer oben: Spanne 10,8 LU ueber den Umfang statt 34 mit dem
    // Plattenkondensator (T verschiebt den Pegel, kaum die Spanne: 1 mm 11,9, 2 mm 11,0)
    s.abnahme = Abnahme::Schlitz;
    s.abnehmerAbstandM = 1.5e-3;
    s.abnehmerVersatzM = 0.75e-3;
    // IC-Kennzeichen "ungemessen" (IK2-9: folgt hier aus der Herkunft)
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

StabSpiel baueVorgabeSteeldrum()
{
    StabSpiel s = holzStab("Steeldrum", 60, 88, 2.0, 4.0, "Thomas, Monteil & Touzé 2014: G3 „1:2:2:4:4:6:8“ (nur Bericht, Wortlaut im Bericht)");
    s.festeVerhaeltnisse = {1.0, 2.0, 2.0, 4.0, 4.0, 6.0, 8.0};
    s.verhaeltnis2.belegkraft = s.verhaeltnis3.belegkraft = Belegkraft::Offen;
    s.t60S = konst(1.5, Belegkraft::Offen, "offen: 1,5 s (Einschaetzung)", s);
    s.masseKg = konst(0.05, Belegkraft::Offen, "offen: modale Masse eines Tonfelds (Einschaetzung)", s);
    s.partner = PartnerArt::Keiner;
    s.anschlagStelle = konst(0.3, Belegkraft::Offen, "offen: die Formen sind Platzhalter (Balken statt Schale)", s);
    s.quadratischeKopplung = 0.0;  // Entscheid nach FRAGEN_KONSOLIDIERT T8 a: ungemessen, Grenze 0 bis zu einer Messung; Deluxe
    // IC-Kennzeichen "Deluxe, ungemessen (Aufnahme nur NC, F-7 c)" (IK2-9: folgt hier aus der Herkunft)
    mitQuellen(s);  // der Bezug: kern::normiereNachTafel (IK2-10 a)
    return s;
}

std::vector<StabSpiel> alleStabSpiele()
{
    return {baueVorgabeMarimba(), baueVorgabeXylophon(), baueVorgabeVibraphon(), baueVorgabeGlockenspiel(), baueVorgabeRoehrenglocken(),
            baueVorgabeCelesta(), baueVorgabeKalimba(), baueVorgabeRhodes(), baueVorgabeWurlitzer(), baueVorgabeSteeldrum()};
}

void richteStabSpiel(StabSpiel& s, const StabSpiel& v)
{
    // KL4-1 a (S181): auf den Umfang der Tafel, nicht den des Instruments (IC richtet auf `s.noteMin`/`s.noteMax`;
    // die Abdruecke aus IC bleiben Bit fuer Bit, weil keine Vorgabe dort einen Stuetzpunkt im Klang verschiebt)
    const auto r = [&](Kurve& k, double von, double bis, const Kurve& e) { richteKurve(k, von, bis, e, kStabUmfangMin, kStabUmfangMax); };
    r(s.grundJeNote, 0.05, 2.0, v.grundJeNote);
    r(s.verhaeltnis2, 1.5, 20.0, v.verhaeltnis2);
    r(s.verhaeltnis3, 2.0, 60.0, v.verhaeltnis3);
    r(s.t60S, 0.01, 200.0, v.t60S);
    r(s.daempfungExponent, 0.0, 3.0, v.daempfungExponent);
    r(s.masseKg, 1e-4, 10.0, v.masseKg);
    r(s.torsionVerhaeltnis, 0.0, 20.0, v.torsionVerhaeltnis);
    r(s.torsionT60S, 0.01, 200.0, v.torsionT60S);
    r(s.abstrahlExponent, -2.0, 2.0, v.abstrahlExponent);
    r(s.partnerVerhaeltnis, 0.8, 1.25, v.partnerVerhaeltnis);
    r(s.partnerXi, 1e-5, 0.2, v.partnerXi);
    r(s.kopplungJeKritisch, 0.0, 10.0, v.kopplungJeKritisch);
    // die Entartung bei κ = κ_c ausschliessen (L11): 0,95 … 1,05 auf den naechsten Rand
    for (auto* liste : {&s.kopplungJeKritisch.punkte, &s.kopplungJeKritisch.abweichungen})
        for (NotenWert& p : *liste)
            if (p.wert > 0.95 && p.wert < 1.05) p.wert = p.wert < 1.0 ? 0.95 : 1.05;
    r(s.partnerAbstrahl, 0.0, 1e6, v.partnerAbstrahl);
    r(s.anschlagStelle, 0.0, 1.0, v.anschlagStelle);
    r(s.querVersatz, -0.5, 0.5, v.querVersatz);
    r(s.daempferT60S, 0.01, 10.0, v.daempferT60S);
    r(s.abnehmerStelle, 0.0, 1.0, v.abnehmerStelle);
    s.restOeffnung = std::clamp(s.restOeffnung, 0.0, 1.0);
    s.hochlaufS = std::clamp(s.hochlaufS, 0.01, 10.0);
    s.tempoMax = std::clamp(s.tempoMax, 0.01, 20.0);
    s.zupfKraftN = std::clamp(s.zupfKraftN, 1e-3, 100.0);
    s.ueberabtastung = std::clamp(s.ueberabtastung, 1, 256);
    s.quadratischeKopplung = std::clamp(s.quadratischeKopplung, 0.0, 10.0);
}

}  // namespace lumi::kern::stab
