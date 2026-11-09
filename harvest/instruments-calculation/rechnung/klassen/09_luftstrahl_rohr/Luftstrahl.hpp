/**
 * @file Luftstrahl.hpp
 * @brief Klasse 9 Luftstrahl und Rohr: ein Ton einer Floete oder Labialpfeife — Strahl als Verzoegerung mit
 *        Saettigung am Labium (jet-drive), Resonator modal ueber die Admittanz am offenen Mund (`RohrKern`), Verluste
 *        am Mund, Pfad „Schneidenton“ im Einschwingen (halbphysikalisch), Tremulant und Schweller fuer die Orgel
 *        (`klassen/Klasse 9 - Luftstrahl und Rohr/modell-entwurf.md`; Entscheide Patrik 2026-10-29: F-1 … F-5 a ueber
 *        FRAGEN_KONSOLIDIERT T2, T3a, T5, T7, T8)
 *
 * **Strahl** (Entwurf §2 Punkt 2): Strahlgeschwindigkeit U_j = √(2P/ρ); die Querauslenkung am Labium folgt der
 * Schallschnelle am Mund mit der Laufzeit τ = W/c_p, c_p = 0,4·U_j, verstaerkt e^(α_i·W) (Terrien et al., nur Bericht):
 * η(t) = (h/U_j)·v(t − τ)·e^(α_i W); der Fluss ins Rohr nach Fletcher & Douglas 1980 S. 768 (nachgelesen):
 * Q = b·H·U_j·(1 + tanh((η − y0)/b)), b = 2h/5 (nur Bericht) — „even harmonics are all identically zero when the lip
 * is symmetrically placed“ (y0 = 0). Die Druckquelle am Mund Δp = (ρ·δ_d/S_m)·dQ/dt mit δ_d = (4/π)·√(2hW) (Verge
 * 1997 — nur aus dem Gedaechtnis, offen); dazu die Verluste der Abloesung −ρ/(2α_vc²)·v|v|, α_vc = 0,6 (nur Bericht).
 *
 * **Resonator:** die Moden der Admittanz Y = 1/Z am offenen Mund (Spitzen von |Y|); der Fluss U = Y·Δp im RohrKern
 * (Eingang Druck, Ausgang Fluss). Ausgang: die Abstrahlung am Mund und am Ende als Monopol ρ/(4π)·dU/dt (offen: das
 * Ende abgeschwaecht durch die Uebertragung, hier die Summe beider mit gleichem Gewicht — Vorgabe).
 *
 * **Register und Tonhoehe ueber dem Druck** entstehen im Modell (mehr Druck → kuerzere Laufzeit → hoeher, Fletcher &
 * Douglas S. 767, nachgelesen); der **Ansatz der Querfloete** ist keine eigene Spur (T3a a): W, y0 aus Note und
 * Expression (Kurven, offen). **Schneidenton** (T5 a): ein Oszillator bei f_e = 0,3·U_j/W (Strouhal, offen) mit
 * Huelle im Einschwingen und geformtes Rauschen, als halbphysikalisch gekennzeichnet.
 *
 * Reines C++, deterministisch (Rauschen mit festem Startwert).
 */
#pragma once

#include "gemeinsam/Kurve.hpp"
#include "gemeinsam/Rohr.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ic::luftstrahl
{

struct StrahlSpiel
{
    std::string name;
    int noteMin = 72;
    int noteMax = 96;
    bool spielbar = true;
    std::string grund;
    bool orgel = false;              ///< Expression = Schweller, Vibrato = Tremulant (T3a a)
    // --- Rohr: ein Zylinder, beidseits offen, wirksame Laenge je Note (Tonloecher offen) ---
    double radiusM = 0.0095;
    Kurve laengeJeNote;              ///< m (log), gestimmt
    Kurve radiusJeNote;              ///< leer = `radiusM` (Labialpfeifen: Mensur)
    int moden = 10;
    // --- Strahl und Mund ---
    double kanalHoeheM = 1e-3;       ///< h
    double abstandM = 4e-3;          ///< W (Kanal–Labium)
    double mundBreiteM = 0.012;      ///< H (Breite des Strahls)
    double mundFlaecheM2 = 4.8e-5;   ///< S_m
    /// > 0: die Breite des Munds als Anteil am Umfang des Rohrs je Note, die Flaeche Breite × Aufschnitt (Labialpfeifen:
    /// ein Viertel, Wikipedia — nur Bericht); 0 = `mundBreiteM`, `mundFlaecheM2` fest
    double mundJeUmfang = 0.0;
    double versatzM = 0.0;           ///< y0
    double alphaI = 350.0;           ///< 1/m (0,3–0,4/h, nur Bericht)
    double cpJeUj = 0.4;
    double alphaVc = 0.6;
    double strahlTiefpass = 0.1;     ///< f_j = Faktor·U_j/W (offen)
    /// κ = τ/T: die Laufzeit des Strahls je Periode, an der die Note ihr Register haelt — gesetzt am Modell (die
    /// Blockfloete D5 spielt bei 300 Pa mit κ = 0,26 den Grundton; Rechnung, offen)
    double laufzeitJePeriode = 0.26;
    // --- Druck: je Note aus κ (Floeten, Expression ×2^(e − 0,5)), gekappt auf pMin … pMax; Orgel: fester Wind pMax ---
    double druckMinPa = 200.0;
    double druckMaxPa = 700.0;
    // --- je Note gestimmt (`stimme`, Rechnung): das Fenster des Drucks, in dem die Note ihr erstes Register spielt
    // (Floeten, Expression geometrisch darin), bzw. der Aufschnitt W in der Mitte des Fensters (Orgel: die Intonation);
    // leer = aus κ ---
    Kurve druckUntenJeNote;          ///< Pa (log)
    Kurve druckObenJeNote;           ///< Pa (log)
    Kurve aufschnittJeNote;          ///< m (log), Orgel
    std::vector<int> registerJeNote; ///< ab noteMin: 1 = Grundregister, 2 = ueberblasen (Auskunft der Stimmung)
    double anblasS = 0.03;
    // --- Schneidenton, Rauschen ---
    double schneidentonPegel = 0.1;  ///< gegen den Ton (offen)
    double rauschPegel = 0.003;      ///< (offen, am Rest zu setzen)
    // --- Orgel ---
    double tremulantHz = 5.0;        ///< um 4–6 Hz (Pykett u. a., nur Bericht)
    double tremulantTiefe = 0.375;   ///< ±, „some 75%“ Spitze–Spitze (nur Bericht) — bei Tiefe 1 der Spur
    double ausgangBezug = 1.0;
    std::string kennzeichen = "ungemessen";
};

struct KettenNote
{
    double abS = 0.0;
    int note = 72;
    double hz = 523.25;
};

struct StrahlAnschlag
{
    std::vector<KettenNote> kette;
    double expression = 0.5;  ///< Floeten: Druck (und Ansatz); Orgel: Schweller
    double haltenS = 1.0;
    double vibratoCent = 0.0; ///< Floeten: Druckmodulation; Orgel: Tremulant (Tiefe 0 … 100 Cent → 0 … 1)
    double vibratoHz = 5.0;
    bool schneidenton = true;
    int rate = 48000;
};

struct StrahlMessung
{
    double spielHz = 0.0;
    bool schwingt = false;
    double einschwingS = 0.0;
};

[[nodiscard]] std::vector<ImpedanzMode> rohrModen(const StrahlSpiel& s, int note);
void rechneStrahlTon(const StrahlSpiel& s, const StrahlAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                     StrahlMessung* messung = nullptr);
/// Die wirksame Laenge je Note durch das Modell hindurch.
void stimme(StrahlSpiel& s);

[[nodiscard]] StrahlSpiel baueVorgabeBlockfloete(bool gestimmt = true);
[[nodiscard]] StrahlSpiel baueVorgabeLabialpfeife(bool gestimmt = true);
[[nodiscard]] StrahlSpiel baueVorgabeShakuhachi(bool gestimmt = true);
[[nodiscard]] StrahlSpiel baueVorgabeQuerfloete();
[[nodiscard]] StrahlSpiel baueVorgabeAltfloete();

}  // namespace ic::luftstrahl
