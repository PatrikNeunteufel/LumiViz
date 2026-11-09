/**
 * @file Stab.hpp
 * @brief InstrumentModels, Klasse 4 Platte und Stab: ein Ton eines Stabspiels — Stab modal (Biege- und Torsionsmoden),
 *        Schlaegel im Zeitbereich, Partner der Moden (Resonatorrohr, Tonebar, Holzkasten) als gekoppelte Schwinger,
 *        Motor des Vibraphons als zeitabhaengige Kopplung, Daempfer (Leiste oder einzeln), Abnahme akustisch oder ueber
 *        einen Tonabnehmer (magnetisch, elektrostatisch); dazu die Zunge der Kalimba (Zupfen) und der Bogen am Stab
 *
 * Uebernommen aus IC `rechnung/klassen/04_platte_stab/Stab.hpp`/`.cpp` (`bb7e6d6`; IC Entwurf
 * `klassen/Klasse 4 - Platte und Stab/modell-entwurf.md`, Entscheide Patrik 2026-10-29: F-1 a … F-6 a, F-7 c) — die
 * Rechnung wortgleich, geaendert nur (Instrumente_IC_Konzept.md 34g-3, Uebernahme-Journal):
 * - Namensraum `lumi::kern::stab`, Bausteine aus `InstrumentCore` (`Kurve`, `Modal`) und `InstrumentModels/gemeinsam`
 *   (`Balken`, `Reibung`).
 * - **Kein Messcode im Kern** (IK2-2 a): IC mass beim gestrichenen Ton die Tonhoehe selbst (`messung::tonhoehe`,
 *   `StabMessung::spielHz`); hier gibt der Kern den gehaltenen Teil ab (`StabMessung::gehalten`), gemessen wird im
 *   Stimmer (`InstrumentMeasure/StabStimmen`).
 * - `stimmeBogen` (Fenster der Bogenkraft am Modell) und das Setzen des Pegels des Partners (`setzePartnerAbstrahl`)
 *   ziehen als Stimmer in die Messung (IK2-11 a, KS2 a, KS3 a); `normiere` ist `kern::normiereNachTafel` (IK2-10 a);
 *   `stufeAus` ist `kern::anschlagStufe` (IK2-2 a, „je einmal“). `Stimmung.inc` faellt weg — die Fenster sind Daten.
 *
 * **Kern** (Entwurf §2): je Mode q̈ + 2σq̇ + ω²q = φ(x_s)·F/m; der Schlaegel eine Masse mit F = Q0·δ^p (Hertz,
 * Rucz & Ulveczki 2021 Gl. 6) oder Hunt–Crossley (Filz/Neopren), achtfach bis 200-fach feiner waehrend des Kontakts
 * (`InstrumentCore/Modal` `stosse`). Die Formen φ_n(x) der Biegemoden sind die des gleichfoermigen frei–freien Balkens
 * (`gemeinsam/Balken`) — **Abweichung vom Entwurf:** der Unterschnitt aendert die Formen; ohne Profil mit Quelle
 * bleiben sie gleichfoermig (IC offen.md). Die Frequenzen stehen als Kurven der Verhaeltnisse f_n/f_1 (Literatur).
 * Torsionsmoden (Entwurf §2 Punkt 1: „zieht Energie ab“) mit Form φ_t = √24·(e/b)·cos(kπx/L) fuer den starren
 * Querschnitt (Rechnung: Auslenkung θ·y, J = m·b²/12, normiert auf die modale Masse m), angeregt nur ausserhalb der
 * Laengsachse (Versatz e).
 *
 * **Partner** (Entwurf §3, T4 a fuer Kasten und Tonebar): eine Mode des Stabs und eine Mode des Partners mit fast
 * gleicher Frequenz als gekoppeltes Paar in komplexer Form (die langsamen Huellen, Naeherung rotierender Wellen):
 *   ż_s = λ_s z_s − iκ z_p,  ż_p = λ_p z_p − iκ z_s,
 * Eigenwerte λ = (λ_s + λ_p)/2 ± √(((λ_s − λ_p)/2)² − κ²). Je Takt die exakte 2 × 2-Exponentialmatrix — bei
 * zeitabhaengigem κ (Motor) je Takt neu. Die Kopplung κ steht als Vielfaches der kritischen κ_c = |σ_p − σ_s|/2 (ab dort
 * tragen beide Moden das Mittel der Daempfungen — Rucz: T60 des Grundtons von „close to 10 s“ auf 0,7 s, mit
 * ξ_a = 8,8·10⁻³ und ξ_m = 2,57·10⁻⁴ ist (ξ_a + ξ_m)/2 genau T60 = 0,71 s; Test). Bei κ = κ_c fallen die Eigenwerte
 * zusammen (entartet, L11) — die Grenze schliesst 0,95 … 1,05 aus. Das Rohr der Stabspiele ist unten geschlossen:
 * Partnermoden bei (2m − 1)·f_r (Rechnung, ideal mit fester Endkorrektur); gekoppelt wird jede Stabmode an die naechste
 * Partnermode, wenn sie innerhalb `kPaarFenster` liegt (Schwelle, an der das Modell seine Struktur wechselt — L4,
 * gesetzt, nicht gesucht).
 *
 * **Motor** (T2 a, Entscheid F-2 a): die Drehzahl ist eine Passage-Spur (Umschaltstellen mit Drehzahl, aus = 0); die
 * Phase der Klappen laeuft ueber den Ton durch (Startwinkel aus der Zeit der Note), Hoch- und Auslauf mit `hochlaufS`;
 * die Oeffnung w(θ) = r + (1 − r)·(1 + cos 2θ)/2 (zwei Perioden je Umdrehung, Restoeffnung r) wirkt auf Kopplung und
 * Abstrahlung des Rohrs. **Offen:** Tiefe, Perioden je Umdrehung, Haltewinkel (Entwurf §3).
 *
 * **Daempfer** (T3b a, F-3 a): *Daempferleiste* (Vibraphon, Roehrenglocken: das Pedal dampft alle Staebe, das
 * Notenende nicht; halbes Pedal als Andruck nach SK6 a) oder *Einzeldaempfer* (Celesta, Rhodes, Wurlitzer: wie die
 * Hammer-Saite, das Notenende dampft). Der Aufrufer gibt den Takt, ab dem gedaempft wird, und den Andruck.
 *
 * **S183 (LP2 b — Lautheit ueber den Umfang, Herleitung `docs/modellierung/anbindung-ic/2026-11-09_S183_vorgaben_je_taste.md`):**
 * die Vorgaben von Wurlitzer und Kalimba je Taste statt fest (WU1 a: Masse der Zunge aus Laenge, Breite, Dicke und Lot;
 * KA1 a: Laenge und Masse der Zunge aus Chapman 2006), dazu zwei Aenderungen am Kern: das Zupfen gibt eine **Kraft**
 * (`zupfKraftN`) statt eines Wegs, und zwei Abnahmen — **Schlitz** (der Tonabnehmer des Wurlitzer als Ausschnitt der Platte,
 * der grosse Wege saettigt) und **Steg** (die Kraft an der Einspannung: das Brett der Kalimba strahlt). Rechenfassung 2
 * der Tafel; die IC-Abdruecke von Kalimba und Wurlitzer sind mit Begruendung neu (Journal J60 ff.).
 *
 * Reines C++ ohne Qt, deterministisch.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"
#include "InstrumentCore/Modal.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace lumi::kern::stab
{

/// Partnermoden nur fuer Stabmoden innerhalb dieses Anteils (|f_s − f_p| / f_s) — Schwelle (L4), gesetzt.
inline constexpr double kPaarFenster = 0.06;
/// Hoechstens so viele Biegemoden.
inline constexpr int kBiegeModenMax = 24;
/// Teiltoene nur bis zu diesem Anteil der Abtastrate (wie der Fluegel).
inline constexpr double kTeiltonGrenze = 0.45;

/// Der Partner einer Stabmode: Resonatorrohr (unten geschlossen, ungerade Moden), Tonebar oder Kasten (eine Mode).
enum class PartnerArt
{
    Keiner,
    RohrGeschlossen,
    EineMode,
};

enum class DaempferArt
{
    Keiner,
    Leiste,
    Einzeln,
};

enum class Abnahme
{
    Akustisch,        ///< Σ g_n·q̈_n plus Partner
    Magnetisch,       ///< Rhodes: dΦ(y)/dt, Φ ∝ 1/√((y − y0)² + d²) (Polmodell — offen)
    Elektrostatisch,  ///< Plattenkondensator: u0·dC(y)/dt, C ∝ 1/(d − y) — bis S183 der Wurlitzer; klemmt bei grossem Weg (offen)
    Schlitz,          ///< Wurlitzer (WU1 a, S183): die Zunge schwingt durch einen Ausschnitt der geladenen Platte (Pfeifle 2017
                      ///< Fig. 2, „vibrate freely between the symmetric cutouts“); C(y) ∝ 1/(1 + ((y − y0)/T)²) mit T =
                      ///< `abnehmerAbstandM` (Breite der Glocke, die Plattendicke) und y0 = `abnehmerVersatzM` (Arbeitspunkt —
                      ///< das Service Manual stimmt zu laute Toene, indem es das Plattenende 1/32″ … 1/16″ aufbiegt). Grosse
                      ///< Wege geben je Durchgang einen Puls fester Flaeche: die Grundwelle saettigt, der Bass wird reich an
                      ///< Obertoenen, nicht lauter
    Steg,             ///< Kalimba (KA1 a, S183): die Kraft an der Einspannung, m·Σ g_n·q̈_n plus Partner — das Brett strahlt,
                      ///< nicht die Zunge
};

enum class Anregung
{
    Stoss,   ///< Schlaegel, Hammer
    Zupfen,  ///< Kalimba: die Zunge wird mit der Kraft `zupfKraftN` an der Stelle ausgelenkt und losgelassen (KA1 a, S183:
             ///< Kraft, nicht Weg — der Daumen gibt eine Kraft, der Weg folgt aus der Steife der Zunge)
};

/// Der moegliche Umfang aller Stabspiele (die Tafel des Kerns `bar`, KS1 a): A0 … C8. Auf ihn richten sich die Kurven
/// (KL4-1 a); der Umfang eines Instruments (`noteMin`/`noteMax`) verschiebt keine Stuetzpunkte.
inline constexpr int kStabUmfangMin = 21;
inline constexpr int kStabUmfangMax = 108;

/// Die Groessen eines Stabspiels als Kurven ueber den Tonumfang (MIDI); Skala je Groesse beim Feld.
struct StabSpiel
{
    std::string name;
    int noteMin = 48;
    int noteMax = 96;
    /// Tonhoehe der Note → Frequenz der ersten Biegemode: f_1 = f_Note · `grundJeNote` (1 bei Staeben; bei den
    /// Roehrenglocken liegt der Schlagton unter Mode 4, linear)
    Kurve grundJeNote;
    /// Verhaeltnisse f_n/f_1 der Biegemoden 2 und 3 (linear); darueber der Abstand des gleichfoermigen Balkens
    Kurve verhaeltnis2;
    Kurve verhaeltnis3;
    /// Statt der Kurven eine feste Liste der Verhaeltnisse (Tonfeld der Steeldrum), leer = die Kurven
    std::vector<double> festeVerhaeltnisse;
    /// Abklingzeit der ersten Biegemode ohne Partner, s (log)
    Kurve t60S;
    /// Exponent κ der Daempfung ueber die Frequenz: ξ_n = ξ_1·(f_n/f_1)^κ (linear; 0 = fester Verlustfaktor)
    Kurve daempfungExponent;
    /// Masse des Stabs, kg (log) — modale Masse jeder Mode in den Formen des gleichfoermigen Balkens
    Kurve masseKg;
    /// Erste Torsionsmode im Verhaeltnis zu f_1 (linear, 0 = keine); weitere k·f_t1
    Kurve torsionVerhaeltnis;
    /// Abklingzeit der Torsionsmoden, s (log)
    Kurve torsionT60S;
    /// Lagerung: frei–frei (Stab) oder Kragbalken (Zunge: Rhodes-Tine, Wurlitzer, Kalimba)
    bool kragbalken = false;
    /// Abstrahlgewicht der Biegemoden ueber die Mode (Mode 1 = 1): g_n = n^`abstrahlExponent` (linear)
    Kurve abstrahlExponent;
    /// --- Partner ---
    PartnerArt partner = PartnerArt::Keiner;
    Kurve partnerVerhaeltnis;   ///< f_r/f_1 (linear)
    Kurve partnerXi;            ///< Daempfungsgrad ξ der Partnermoden (log)
    Kurve kopplungJeKritisch;   ///< κ/κ_c (linear)
    Kurve partnerAbstrahl;      ///< Abstrahlgewicht des Partners gegen den Stab (log) — gestimmt (KS2 a)
    /// das Ziel des gestimmten `partnerAbstrahl` (KS2 a; in IC `setzePartnerAbstrahl(s, note, dB)`): an
    /// `partnerZielNote` ist der Ton mit Partner um `partnerZielDb` lauter als ohne (Effektivwert der ersten 0,5 s bei
    /// Stufe −6). Der Ton liest beides nicht, nur der Stimmer
    double partnerZielDb = 15.0;
    int partnerZielNote = 65;
    /// --- Schlaegel ---
    Anregung anregung = Anregung::Stoss;
    Stossanreger schlaegel;
    Kurve anschlagStelle;       ///< x_s/L (linear)
    Kurve querVersatz;          ///< e/b, der Versatz von der Laengsachse (linear)
    double tempoMax = 5.0;      ///< m/s bei Stufe 0 (Stoss)
    double zupfKraftN = 3.0;    ///< N bei Stufe 0 (Zupfen, KA1 a): q_n(0) = F·φ_n/(m·ω_n²), der Weg δ = F/k folgt aus der Steife
    int ueberabtastung = 64;
    /// --- Daempfer --- (Entscheid nach FRAGEN_KONSOLIDIERT T3b a)
    DaempferArt daempfer = DaempferArt::Keiner;
    Kurve daempferT60S;         ///< Abklingzeit unter dem Daempfer bei vollem Andruck (log)
    /// --- Motor (nur mit Rohr) --- (Entscheid nach FRAGEN_KONSOLIDIERT T2 a)
    bool motor = false;
    double restOeffnung = 0.0;  ///< r: die Oeffnung, wenn die Klappe zu ist
    double hochlaufS = 0.5;     ///< so lange braucht der Motor von 0 auf die neue Drehzahl
    /// --- Abnahme ---
    Abnahme abnahme = Abnahme::Akustisch;
    Kurve abnehmerStelle;       ///< x/L am Abnehmer (Spitze der Zunge: 1)
    double abnehmerAbstandM = 1e-3;  ///< d: Abstand (magnetisch, elektrostatisch) bzw. Breite T der Glocke (Schlitz)
    double abnehmerVersatzM = 0.0;   ///< y0: Versatz der Ruhelage (magnetisch) bzw. Arbeitspunkt (Schlitz)
    double ausgangBezug = 1.0;  ///< der Ausgang, der als 1 ausgegeben wird
    /// --- geometrische Nichtlinearitaet (Steeldrum, Deluxe): quadratische Kopplung der Moden im Verhaeltnis 1 : 2
    /// (Thomas, Monteil & Touzé 2014, S. 2: „quadratischen Termen für 1:2:2 und 1:2:4“ — nur Bericht): Potential
    /// V = c·m·ω_j²·q_i²·q_j/L0 mit L0 = 1 mm; c = 0 schaltet sie aus (Grenze 0 bis zu einer Messung, T8 a) ---
    double quadratischeKopplung = 0.0;
    /// --- Bogen × Stab (FRAGEN_KONSOLIDIERT T1 a, K4 F-4 a): die Reibung der Klasse 3 (`gemeinsam/Reibung`) an der
    /// Stelle `bogenStelle` (Ende des Stabs), Expression als Weg wie dort: v_B geometrisch von `bogenVbPp` bis
    /// `bogenVbFf`; F_B in der geometrischen Mitte des Fensters, das der Stimmer je Note bei pp, mf und ff am Modell
    /// sucht (stueckweise geometrisch dazwischen; fehlt ein Fenster, gilt das naechste der Note) ---
    bool bogen = false;
    double bogenStelle = 0.0;      ///< x/L (0 = Ende des Stabs)
    double bogenMuS = 1.2;         ///< Haftreibung (Galluzzo, Haar auf Darm — uebertragen, offen)
    double bogenVbPp = 0.08;       ///< m/s (wie Klasse 3, Chudy — uebertragen, offen)
    double bogenVbFf = 0.4;
    double bogenAnsatzS = 0.05;    ///< Anstieg und Absetzen der Kraft
    std::array<Kurve, 3> bogenKraftUnten, bogenKraftOben;  ///< N bei pp, mf (v_B = √(v_pp·v_ff)), ff (log) — gestimmt
    /// gestrichen erst ab dieser Note, darunter der Anschlag (KS3 a: IC Einteilung C schliesst die tiefe Lage aus, der
    /// IC-Test prueft die obere Lage ab E♭4 = 63). Der Ton liest es nicht, der Adapter
    int bogenAb = 63;
};

/// Eine Umschaltstelle der Motor-Spur: ab `zeitS` (Zeit im Ton; negativ = vor dem Anschlag) die Drehzahl in U/s.
struct MotorStelle
{
    double zeitS = 0.0;
    double drehzahl = 0.0;
};

/// Was ein Ton braucht.
struct StabAnschlag
{
    int note = 60;
    double hz = 261.63;
    int stufe = 0;                       ///< Anschlagstufe ≤ 0 (dB der Geschwindigkeit, wie HA3 a)
    std::int64_t daempferAb = -1;        ///< ab diesem Takt greift der Daempfer (−1 = nie)
    double andruck = 1.0;                ///< 0 … 1 (halbes Pedal, SK6 a)
    std::vector<MotorStelle> motor;      ///< Passage-Spur der Drehzahl (leer = Motor aus, Klappen offen)
    double motorPhaseStart = 0.0;        ///< Winkel der Klappen beim Anschlag, rad (aus der Zeit der Note)
    /// Abweichung der Anschlagstelle von der Vorgabe (Passage-Spur Schlaegel-Ort; Entscheid nach FRAGEN_KONSOLIDIERT T3b a)
    double stelleOffset = 0.0;
    /// --- gestrichen (nur mit `StabSpiel::bogen`) ---
    bool gestrichen = false;
    double expression = 0.5;             ///< 0 … 1
    double haltenS = 1.0;                ///< so lange liegt der Bogen auf
    double bogenKraftN = 0.0;            ///< > 0: die Kraft unmittelbar (fuer den Stimmer), sonst aus dem Fenster
    double bogenVbMS = 0.0;              ///< > 0: v_B unmittelbar, sonst aus der Expression
    int rate = 48000;
};

/// Was beim Anschlag geschah.
struct StabMessung
{
    double kontaktS = 0.0;
    double kraftSpitzeN = 0.0;
    int kontakte = 0;
    double auslenkungSpitzeM = 0.0;  ///< groesste Auslenkung am Abnehmer (nur mit Tonabnehmer)
    // --- gestrichen ---
    /// der gehaltene Teil des Ausgangs (zweite Haelfte des Haltens, vor dem Bezug und der Blende), wenn er laenger als
    /// 4800 Takte ist — die Tonhoehe misst der Stimmer (in IC: `spielHz`, `schwingt`, im Kern gemessen)
    std::vector<double> gehalten;
    double haftAnteil = 0.0;         ///< Anteil der Takte mit Haften am Bogen (zweite Haelfte des Haltens)
};

/// Eine Mode, wie der Ton sie rechnet (fuer Tests und die Messung am Modell).
struct StabMode
{
    double hz = 0.0;
    double t60S = 0.0;
    double form = 0.0;      ///< φ an der Anschlagstelle
    double abnehmer = 0.0;  ///< φ am Abnehmer
    double abstrahl = 0.0;
    bool torsion = false;
    int partner = -1;       ///< Index der Partnermode oder −1
};

struct PartnerMode
{
    double hz = 0.0;
    double t60S = 0.0;
    double kopplung = 0.0;  ///< κ in 1/s
    double abstrahl = 0.0;
};

struct StabModen
{
    std::vector<StabMode> stab;
    std::vector<PartnerMode> partner;
};

/// Die Moden an einer Note, aus den Kurven (dieselbe Rechnung wie im Ton).
[[nodiscard]] StabModen stabModen(const StabSpiel& s, const StabAnschlag& a);

/// So lange klingt der Ton, bis −60 dB der schwaechsten Mode (ohne Daempfer), hoechstens `maxFrames`.
[[nodiscard]] std::int64_t stabNachklang(const StabSpiel& s, const StabAnschlag& a, std::int64_t maxFrames);

/// Einen Ton rechnen: `frames` Werte Mono, in Einheiten von `ausgangBezug`.
void rechneStabTon(const StabSpiel& s, const StabAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                   StabMessung* messung = nullptr);

/// Jede Kurve in ihre Grenzen (Grenzen in IC `modell.md`); eine leere bekommt die Vorgabe des Instruments. Die Tafel
/// richtet dieselben Grenzen (`kernTyp`); hier bleibt dazu die Regel L11 (κ/κ_c nicht in 0,95 … 1,05).
void richteStabSpiel(StabSpiel& s, const StabSpiel& vorgabe);

// --- die Instrumente der Klasse (Vorgaben, `Vorgaben.cpp`) ---------------------------------------------------
// Ohne die gestimmten Werte (Pegel des Partners, Fenster des Bogens) und ohne Bezug: die rechnet der Stimmer
// (`InstrumentMeasure/StabStimmen`, KS2 a, KS3 a), den Bezug `kern::normiereNachTafel` (IK2-10 a).
[[nodiscard]] StabSpiel baueVorgabeMarimba();
[[nodiscard]] StabSpiel baueVorgabeXylophon();
[[nodiscard]] StabSpiel baueVorgabeVibraphon();
[[nodiscard]] StabSpiel baueVorgabeGlockenspiel();
[[nodiscard]] StabSpiel baueVorgabeRoehrenglocken();
[[nodiscard]] StabSpiel baueVorgabeCelesta();
[[nodiscard]] StabSpiel baueVorgabeKalimba();
[[nodiscard]] StabSpiel baueVorgabeRhodes();
[[nodiscard]] StabSpiel baueVorgabeWurlitzer();
/// Deluxe in IC, nicht ausgeliefert (KS4 a): die Tonfelder sind Platzhalter (Balken statt Schale).
[[nodiscard]] StabSpiel baueVorgabeSteeldrum();
/// Alle zehn, in der Reihenfolge von IC `alleStabSpiele`.
[[nodiscard]] std::vector<StabSpiel> alleStabSpiele();

}  // namespace lumi::kern::stab
