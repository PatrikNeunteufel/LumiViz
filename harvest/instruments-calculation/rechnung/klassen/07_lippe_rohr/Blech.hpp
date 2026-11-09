/**
 * @file Blech.hpp
 * @brief Klasse 7 Lippe und Rohr: ein Ton eines Blechblasinstruments — Lippe als nach aussen schlagendes Ventil mit
 *        einem Freiheitsgrad, Kessel, Zylinder als veraenderliche und druckabhaengige Verzoegerung (Zug, Schmettern),
 *        Rest der Bohrung als Reflexion (`gemeinsam/Rohrschleife`), Vokaltrakt in Reihe, Growl als zweites Ventil
 *        (`klassen/Klasse 7 - Lippe und Rohr/modell-entwurf.md`; Entscheide Patrik 2026-10-29: F-1 … F-7 a ueber
 *        FRAGEN_KONSOLIDIERT T2, T3a, T3b, T3c, T5, T7, T8)
 *
 * **Lippe** (Entwurf §2 Punkt 2): ÿ + (ω_L/Q)·ẏ + ω_L²·y = Δp/µ (Masse je Flaeche µ, Form wie Petiot et al. 2023),
 * Δp = p_Mund − p_Kessel, nach aussen schlagend (Δp > 0 oeffnet); Spalt h = max(0, H0 + y); Fluss nach Bernoulli
 * U = w·h·√(2|Δp|/ρ)·sgn Δp. Mit Vokaltrakt in Reihe (T5 a, normal null) ist p_Mund = P − Z_V·U — die Gleichung in U
 * wird geschlossen geloest (quadratisch, wie bei Guillemain et al. 2005 — nur Bericht).
 *
 * **Je Note gestimmt** (Entwurf §4: „Lippenresonanz je Note gesucht, so dass die Spielfrequenz den Ton trifft“): das
 * Register aus den Spitzen der Eingangsimpedanz, dann — mit Zug — die Zuglaenge, sonst die Lippe, durch das Modell
 * hindurch auf 3 Cent (`stimme`), an drei Staerken (der Weg durch Druck **und** Ansatz, T3a a: der Ansatz haelt die
 * Tonhoehe). Ergebnis als Kurven ueber den Tonumfang (`lippeJeNote`, `zugJeNote`, Belegkraft „Rechnung“).
 *
 * **Kette verbundener Noten** (T2 a, F-R4 a): ein Ton; an jeder Note wechseln Zuglaenge (in `zugBewegungS`) und
 * Lippe, der Anreger laeuft durch. **Vibrato** (T3a a, Blech: die Lippe) moduliert ω_L; **Pitch** (Bend) verstimmt die
 * Lippe und — mit Zug — den Zug. **Fall, Doit** (T3c a): die Lippe gleitet am Ende um `tiefe` Halbtoene in `laenge` s
 * — das Register springt von selbst. **Growl** (T3c a): ein zweites Ventil vor der Lippe (Stimmlippen mit
 * vorgegebener Bewegung, Oeffnung (1 + sin)/2 bei der gesungenen Tonhoehe, Fluss nach Bernoulli in den Mundraum als
 * Nachgiebigkeit) — die Kombinationstoene entstehen aus den zwei Ventilen.
 *
 * **Daempfer** (T3b a): je Daempfer ein vorab gerechneter Satz — hier ein Helmholtz-Resonator am Schallstueck
 * (Frequenz, Guete, Restflaeche der Abstrahlung) als Abschluss der Bohrung; die Umschaltstelle braucht eine Pause
 * (kein Uebergang). Werte offen (Entwurf §4: „Straight ~77 Hz bei der Posaune, Cup −20 dB um 1 kHz“ — nur Bericht).
 *
 * Reines C++, deterministisch.
 */
#pragma once

#include "gemeinsam/Kurve.hpp"
#include "gemeinsam/Rohr.hpp"
#include "gemeinsam/Rohrschleife.hpp"

#include <memory>

#include <cstdint>
#include <string>
#include <vector>

namespace ic::blech
{

/// Ein Daempfer als Satz (T3b a): offen bzw. ein Helmholtz-Resonator am Schallstueck.
struct Daempfer
{
    std::string name = "offen";
    double hz = 0.0;            ///< Resonanz des Daempfers (0 = kein Daempfer)
    double q = 5.0;
    double restFlaeche = 1.0;   ///< Anteil der Flaeche des Schallstuecks, der noch abstrahlt
    /// Wah-Wah (T3b a): die Restflaeche, wenn die Hand den Daempfer schliesst (0 = kein Wah-Wah an diesem Daempfer)
    double restZu = 0.0;
};

struct BlechSpiel
{
    std::string name;
    int noteMin = 40;
    int noteMax = 77;
    bool spielbar = true;           ///< falsch: keine Bohrung mit Quelle (T8 a, Kipppunkt b — vorgelegt)
    std::string grund;              ///< warum nicht spielbar
    // --- Bohrung ---
    double zylinderRadiusM = 0.007;
    double zylinderGrundM = 1.596;  ///< Laenge des Zylinders in der ersten Lage bzw. ohne Ventil
    double zugMaxM = 0.0;           ///< so viel laenger hoechstens (Posaune: zweimal der Aussenzug)
    std::vector<double> ventilM;    ///< Zusatzlaengen je Ventil (im Zylinder)
    Bohrung rest;                   ///< hinter dem Zylinder bis zum Schallstueck
    double kesselM3 = 11.6e-6;
    // --- Lippe ---
    double lippeMasseJeFlaeche = 1.3;  ///< µ, kg/m²
    double lippeGuete = 5.0;
    double lippeBreiteM = 0.01;
    double lippeSpaltM = 2.9e-4;       ///< H0
    // --- Druck (Expression 0 … 1: P = P_min·(P_max/P_min)^e) ---
    double druckMinPa = 500.0;
    double druckMaxPa = 6000.0;
    double anblasS = 0.005;         ///< Anstieg des Drucks (Zungenstoss: die Zunge gibt den Druck frei — offen)
    double absetzenS = 0.05;
    double zugBewegungS = 0.05;
    // --- Schmettern ---
    double beta = 1.2;              ///< (γ + 1)/2; 0 = linear
    // --- je Note gestimmt (Rechnung, `stimme`) ---
    Kurve lippeJeNote;              ///< f_L/f_Note bei mf (linear)
    Kurve lippeJeNoteLeise;         ///< … bei pp
    Kurve lippeJeNoteLaut;          ///< … bei ff
    Kurve zugJeNote;                ///< Zusatzlaenge des Zylinders, m (linear)
    /// Die Anblasschwelle je Note, Pa (log): der kleinste Druck, bei dem der Kessel nach 0,4 s mit mindestens 30 % des
    /// Blasdrucks schwingt — der Druck ist P = 1,3·P_s·(min(P_max, 6·P_s)/(1,3·P_s))^e (T3a a: Expression als Weg ueber den
    /// Tonumfang; pp spricht an jeder Note an; 1,3 und 6 offen). Leer: P = P_min·(P_max/P_min)^e.
    Kurve schwelleJeNote;
    // --- Vokaltrakt (T5 a, normal null) ---
    double vokalGewicht = 0.0;      ///< 0 = ohne; 1 = Fletcher et al. 2006 II Tab. I (nur Bericht)
    double vokalHz1 = 1500.0, vokalHz2 = 2500.0, vokalQ = 10.0, vokalSpitzePa = 2e6;  ///< Pa·s/m³ an der Spitze (offen)
    // --- Growl ---
    double growlFlaecheM2 = 2e-5;   ///< groesste Oeffnung der Stimmlippen (offen)
    double mundM3 = 5e-5;           ///< Mundraum zwischen beiden Ventilen (offen)
    std::vector<Daempfer> daempfer;
    /// Die Filter der Schleife je Daempfer (Index 0 = offen, dann je Daempfer), vorab gerechnet (`bereite`) — die
    /// Verluste des Zylinders bei der Grundlaenge plus halbem Zug (die Aenderung mit dem Zug ist klein, offen.md).
    std::shared_ptr<const std::vector<SchleifenFilter>> filter;
    /// … und je Daempfer mit Wah-Wah der Satz bei geschlossener Hand (Index wie `filter`; leer, wo keiner)
    std::shared_ptr<const std::vector<SchleifenFilter>> filterZu;
    double ausgangBezug = 1.0;
    std::string kennzeichen = "ungemessen";
};

/// Eine Note einer Kette (T2 a: eine Kette verbundener Noten ist ein Ton).
struct KettenNote
{
    double abS = 0.0;
    int note = 60;
    double hz = 261.63;
};

/// Die Art einer Note am Ende (T3c a).
enum class Endart
{
    Normal,
    Fall,
    Doit,
};

struct BlechAnschlag
{
    std::vector<KettenNote> kette;  ///< mindestens eine Note
    double expression = 0.5;        ///< 0 … 1 (T3a a: Weg durch Druck und Ansatz)
    double haltenS = 1.0;           ///< bis zum Absetzen
    double vibratoCent = 0.0;
    double vibratoHz = 5.0;
    double pitchCent = 0.0;         ///< Bend der ganzen Kette
    Endart ende = Endart::Normal;
    double endeTiefe = 5.0;         ///< Halbtoene
    double endeLaengeS = 0.25;
    bool growl = false;
    double growlIntervall = -12.0;  ///< gesungen relativ zum Ton, Halbtoene
    double vokal = 0.0;             ///< Spur „Vokal“ 0 … 1 (T3c a; nur mit Vokaltrakt)
    int daempfer = -1;              ///< Index in `BlechSpiel::daempfer`, −1 = offen
    /// Wah-Wah (T3b a): die Oeffnung der Hand am Daempfer ueber die Zeit der Note — Stuetzpunkte (Zeit s, Oeffnung 0 = zu
    /// … 1 = offen), linear dazwischen, davor und danach der Rand; leer = offen. Nur an einem Daempfer mit `restZu`.
    std::vector<std::pair<double, double>> wah;
    bool kesselAusgeben = false;    ///< statt des abgestrahlten Drucks den Druck im Kessel (Mundstueck) ausgeben (Messung)
    int rate = 48000;
};

struct BlechMessung
{
    double spielHz = 0.0;  ///< die Tonhoehe der letzten Note im gehaltenen Teil (YIN)
    bool schwingt = false;
    /// Spitze–Spitze des Kesseldrucks im letzten Fuenftel des gehaltenen Teils, bezogen auf den Blasdruck — die
    /// Schwelle (`stimme`) verlangt 0,3: darunter steht nur ein kleiner Rest, kein Ton
    double kesselSchwankung = 0.0;
};

/// Einen Ton rechnen (`frames` Werte Mono, in Einheiten von `ausgangBezug`); ohne `spielbar` Stille.
void rechneBlechTon(const BlechSpiel& s, const BlechAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                    BlechMessung* messung = nullptr);

/// Die gesamte Laenge der Bohrung bei Zusatzlaenge `zusatz` (fuer Tests).
[[nodiscard]] Bohrung ganzeBohrung(const BlechSpiel& s, double zusatzM);

/// Die Spitzen der Eingangsimpedanz mit Kessel bei Zusatzlaenge `zusatzM` bis `bisHz` (die Register).
[[nodiscard]] std::vector<double> registerSpitzen(const BlechSpiel& s, double zusatzM, double bisHz);

/// Die Kurven `lippeJeNote…` und `zugJeNote` durch das Modell hindurch setzen (Stuetzpunkte jede `schritt`-te Note;
/// mit Zug muss es jede sein — die Zuglage springt zwischen den Lagen).
void stimme(BlechSpiel& s, int schritt = 1);

/// Die Filter vorab rechnen (nach jeder Aenderung an Bohrung oder Daempfern).
void bereite(BlechSpiel& s);

/// Grenzen (danach `bereite`).
void richteBlechSpiel(BlechSpiel& s);

/// Die Vorgaben. `gestimmt`: die Stimmung aus `Stimmung.inc` (erzeugt von `ic_stimme_blech`) oder, fehlt sie, durch
/// `stimme` (langsam); falsch: ungestimmt (fuer das Werkzeug).
[[nodiscard]] BlechSpiel baueVorgabePosaune(bool gestimmt = true);
[[nodiscard]] BlechSpiel baueVorgabeAlphorn(bool gestimmt = true);
[[nodiscard]] BlechSpiel baueVorgabeTrompete(bool gestimmt = true);
[[nodiscard]] BlechSpiel baueVorgabeTuba();
[[nodiscard]] BlechSpiel baueVorgabeHorn();
[[nodiscard]] BlechSpiel baueVorgabeDidgeridoo(bool gestimmt = true);

}  // namespace ic::blech
