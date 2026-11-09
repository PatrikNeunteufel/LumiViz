/**
 * @file Rohrblatt.hpp
 * @brief Klasse 8 Rohrblatt und Rohr: ein Ton eines Holzblasinstruments mit Rohrblatt — Rohr modal (Moden der
 *        Eingangsimpedanz je Griff, `gemeinsam/Rohr` `RohrKern`, Kopplung mit Durchgriff), Rohrblatt mit einem
 *        Freiheitsgrad mit Anschlag an der Bahn und Zungenstoss, Vokaltrakt in Reihe fuer den Bend, Legato als
 *        Ueberblenden der Moden (`klassen/Klasse 8 - Rohrblatt und Rohr/modell-entwurf.md`; Entscheide Patrik
 *        2026-10-29: F-1 … F-4 a ueber FRAGEN_KONSOLIDIERT T2, T3a, T3c, T5, T7, T8)
 *
 * **Rohrblatt** (Entwurf §2 Punkt 2, Werte Chatziioannou et al. 2019 Tab. I — nur Bericht): m·ÿ + m·g·ẏ + k·y =
 * −S_r·Δp + F_Bahn + F_Zunge, y zur Bahn hin positiv (der Druck im Mund schliesst), Spalt h = H − y, Fluss nach
 * Bernoulli U = w·max(h, 0)·√(2|Δp|/ρ)·sgn Δp; an der Bahn (y > H − y_lay … hier ab h < 0) die Kontaktkraft nach
 * Hunt–Crossley k_lay·(−h)²·(1 + r_lay·ẏ) (der Exponent 2 aus der Einheit „N/m2“ der Tabelle gedeutet — offen). Mit
 * p = p_vorher + D·U geschlossen geloest wie die Lippe der Klasse 7. S_r folgt aus dem Schliessdruck p_M = k·H/S_r
 * (Vorgabe offen).
 *
 * **Griff** (Abweichung vom Entwurf, begruendet in `offen.md`): die Tonloecher sind **nicht** einzeln gebaut
 * (Keefe-Glieder; die Tabelle von Debut et al. ist nur als umformatierter Bericht da, „vor Uebernahme am Original
 * pruefen“) — je Note eine **wirksame Laenge** der Bohrung mit offenem Ende, durch das Modell gestimmt (`stimme`):
 * die Spitze des Registers trifft die Note. Das zweite Register (Klarinette: die Duodezime, Saxophon: die Oktave) mit
 * dem Registerloch, das die erste Mode schwaecht (`registerDaempfung`, offen).
 *
 * **Zungenstoss** (T3c a: „Der Zungenstoss ist physikalisch (Zunge am Rohrblatt)“): die Zunge drueckt das Blatt bis
 * zum Einsatz zu und gibt es in `zungeS` linear frei (Bahn stueckweise linear wie bei Chatziioannou 2019; Steife
 * k_tg nach Hunt–Crossley — nur Bericht). **Flatterzunge:** dieselbe Zunge periodisch (Rate als Wert der Art).
 * **Atemgeraeusch** (T3c a): Rauschen mal Spalt, **nicht physikalisch** gekennzeichnet, Pegel offen.
 * **Bend** (T3a a: Pitch als Ziel ueber den Vokaltrakt): eine Resonanz des Trakts unter der Spielfrequenz zieht den
 * Ton nach unten; Grenze als Ausstattung (`bendGrenzeCent`), nach oben 0.
 *
 * Reines C++, deterministisch (das Rauschen mit festem Startwert).
 */
#pragma once

#include "gemeinsam/Kurve.hpp"
#include "gemeinsam/Rohr.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ic::rohrblatt
{

/// Die Bohrung als Radius ueber der Laenge (Punkte x [m], r [m], linear dazwischen), ab der Spitze des Blatts.
struct Profil
{
    std::vector<std::pair<double, double>> punkte;
    double mundstueckM3 = 0.0;  ///< Ersatzvolumen des Mundstuecks (Saxophon: der fehlende Kegel)
};

struct RohrblattSpiel
{
    std::string name;
    int noteMin = 50;
    int noteMax = 89;
    bool spielbar = true;
    std::string grund;
    Profil profil;
    int registerAb = 71;              ///< ab dieser Note das zweite Register
    int registerMode = 2;             ///< die Mode, die im zweiten Register spielt (Klarinette 2 = Duodezime, Saxophon 2 = Oktave)
    double registerDaempfung = 0.1;   ///< so viel bleibt von der ersten Mode mit Registerloch (offen)
    int moden = 16;                   ///< so viele Moden der Impedanz je Griff
    /// die Grenzfrequenz des Gitters der offenen Tonloecher hinter der wirksamen Laenge (`Bohrung::gitterGrenzeHz`):
    /// ohne sie sprang das Modell auf hohe Moden (Befund IC Session 4)
    double gitterGrenzeHz = 1500.0;
    // --- Rohrblatt: Masse und Ruhespalt nach Chatziioannou et al. 2019 Tab. I (nur Bericht); Resonanz und Daempfung mit
    // der Lippe nach Silva et al. 2007 Fig. 1: „ω_r = 2π×1500“, „q_r = 0.4“ (nur Bericht) — mit Chatziioannous k = 1765 N/m
    // und g = 3000 1/s (Resonanz 3,7 kHz, Guete 8) quietschte das Modell bei 1,7 kHz (Befund IC Session 4) ---
    double masseKg = 3.25e-6;
    double steife = 3.25e-6 * (2.0 * 3.141592653589793 * 1500.0) * (2.0 * 3.141592653589793 * 1500.0);  ///< N/m: m·ω_r²
    double daempfung = 0.4 * 2.0 * 3.141592653589793 * 1500.0;  ///< g = q_r·ω_r, 1/s
    double ruheSpaltM = 0.41e-3;      ///< H (y_eq)
    double breiteM = 0.012;           ///< w (offen)
    double schliessdruckPa = 8000.0;  ///< p_M = k·H/S_r (offen)
    double bahnSteife = 1.97e6;       ///< k_lay (N/m²)
    double bahnDaempfung = 2.0;       ///< r_lay (s/m)
    // --- Zunge ---
    double zungeSteife = 3.16e4;
    double zungeWegM = 0.51e-3;       ///< max(y_tg)
    double zungeS = 0.01;             ///< so lange gibt die Zunge frei (offen)
    // --- Druck (Expression als Weg ueber die Schwelle, wie Klasse 7) ---
    double druckMaxPa = 5400.0;       ///< Klarinette 2,0–5,4 kPa (Fuks & Sundberg, nur Bericht)
    double weg = 2.5;                 ///< ff = min(P_max, weg·P_s), pp = 1,15·P_s (offen)
    // --- Atem, Vokaltrakt ---
    double atemPegel = 1e-5;          ///< Rauschen je Pa Druck, mal Spalt/H (offen, nicht physikalisch; zu setzen am Rest)
    double bendGrenzeCent = 300.0;    ///< Ausstattung (Klarinette, Saxophon: bis etwa eine grosse Terz abwaerts — nur Bericht)
    double vokalSpitzePa = 4e7;       ///< Spitze der Bend-Resonanz (Chen: Trakt „vergleichbar“ der Bohrung 40–90 MPa·s/m³)
    // --- je Note gestimmt (Rechnung) ---
    Kurve laengeJeNote;               ///< wirksame Laenge, m (log)
    Kurve schwelleJeNote;             ///< Pa (log)
    Kurve obergrenzeJeNote;           ///< Pa (log): darueber springt das Modell auf eine andere Mode; leer = keine
    double ausgangBezug = 1.0;
    std::string kennzeichen = "ungemessen";
};

struct KettenNote
{
    double abS = 0.0;
    int note = 60;
    double hz = 261.63;
};

enum class Ansatz
{
    Zunge,         ///< Zungenstoss (Vorgabe)
    Atem,          ///< ohne Zunge (Druck steigt in `anblasS`)
    Flatterzunge,
};

struct RohrblattAnschlag
{
    std::vector<KettenNote> kette;
    double expression = 0.5;
    double haltenS = 1.0;
    Ansatz ansatz = Ansatz::Zunge;
    double flatterHz = 25.0;
    double vibratoCent = 0.0;   ///< ueber den Druck (T3a a: Rohrblatt — Druck oder Kiefer)
    double vibratoHz = 5.0;
    double pitchCent = 0.0;     ///< nur abwaerts wirksam (Vokaltrakt), gekappt an `bendGrenzeCent`
    int rate = 48000;
};

struct RohrblattMessung
{
    double spielHz = 0.0;
    bool schwingt = false;
    double schwankung = 0.0;    ///< Spitze–Spitze des Mundstueckdrucks / Blasdruck
    double anstiegDbJeS = 0.0;  ///< groesste Steigung der Huelle (10-ms-Effektivwert) im Einsatz
};

[[nodiscard]] Bohrung bohrungBis(const RohrblattSpiel& s, double laengeM);
/// Die Moden der Impedanz eines Griffs (wirksame Laenge, Register).
[[nodiscard]] std::vector<ImpedanzMode> grifModen(const RohrblattSpiel& s, double laengeM, bool zweitesRegister);

void rechneRohrblattTon(const RohrblattSpiel& s, const RohrblattAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                        RohrblattMessung* messung = nullptr);

/// Die wirksame Laenge und die Schwelle je Note durch das Modell hindurch (je Note ein Stuetzpunkt).
void stimme(RohrblattSpiel& s);

/// `gestimmt`: die Stimmung aus `Stimmung.inc` (erzeugt von `ic_stimme_rohrblatt`) oder durch `stimme`; falsch: ungestimmt.
[[nodiscard]] RohrblattSpiel baueVorgabeKlarinette(bool gestimmt = true);
[[nodiscard]] RohrblattSpiel baueVorgabeSopransaxophon(bool gestimmt = true);
[[nodiscard]] RohrblattSpiel baueVorgabeTenorsaxophon(bool gestimmt = true);
[[nodiscard]] RohrblattSpiel baueVorgabeAltsaxophon(bool gestimmt = true);
[[nodiscard]] RohrblattSpiel baueVorgabeOboe(bool gestimmt = true);
[[nodiscard]] RohrblattSpiel baueVorgabeFagott();

}  // namespace ic::rohrblatt
