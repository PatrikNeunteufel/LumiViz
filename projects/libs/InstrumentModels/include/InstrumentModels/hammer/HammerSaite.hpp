/**
 * @file HammerSaite.hpp
 * @brief InstrumentModels, Kern `hammerString`: die Groessen der Hammer-Saite als Kurven ueber die Tastatur, ihre
 *        Grenzen und Vorgaben (Spielweisen_Konzept.md §4.1, Stufe 31a; HS1 a, HA1 a bis HA4 a, HK1 a, HK2 a;
 *        Instrumente_IC_Konzept.md 34d, IK2-2 a, IK2-3 a)
 *
 * Das **Spiel** des Fluegel-Kerns (IK2-3 a: der Adapter haelt das Spiel seiner Klasse). Eine Groesse des Fluegels —
 * Inharmonizitaet, Abklingzeit, Hammer, Daempfer — ist eine `Kurve` aus Stuetzpunkten an Tasten (HA1 a), dazwischen
 * monoton kubisch in der Skala der Groesse (HK1 a). Bis S175 stand das mit Qt-Datei in `loop/HammerSaite.hpp` der App
 * (`Tastenkurve`); die Kurve ist jetzt die der Bibliothek (`Kurve.hpp`, Bit fuer Bit dieselbe Rechnung).
 *
 * `kurvenAngaben()` ist die eine Liste der 22 Kurven: Feld, Name in der Datei, Titel, Gruppe, Einheit, Skala,
 * Grenzen. Richten, Tafel und Datei lesen sie; keine Groesse steht zweimal.
 *
 * Die Vorgaben stammen, wo belegt, aus der Literatur und der Messung an *Salamander* (Befunde S146–S159); jede Kurve
 * traegt ihre Belegkraft und Herkunft (IK2-4 a). Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"

#include <string_view>
#include <vector>

namespace lumi::kern::hammer
{

/// Die Tasten eines Fluegels als MIDI-Nummern: A0 bis C8. Taste n der Literatur (1 … 88) ist MIDI n + 20.
inline constexpr int kTasteMin = 21;
inline constexpr int kTasteMax = 108;

/// Grenzen der Saiten im Chor.
inline constexpr int kChorMin = 1;
inline constexpr int kChorMax = 3;

/// Die Anschlagstufen, an denen `stossLeise` und `stossLaut` gelten (US2 a): die Mitte der Ebenen v6–v10 und
/// v12–v16 von Salamander nach V1 a (Stufe = −30 + 2·(Ebene − 1)).
inline constexpr int kStossStufeLeise = -16;
inline constexpr int kStossStufeLaut = -4;

/// Die Hammer-Saite: je Groesse eine Kurve ueber die Tastatur. Die Skala, die Grenzen und die Einheit jeder Groesse
/// stehen in `kurvenAngaben()`.
struct HammerSaite
{
    /// Inharmonicity B (log): f_k = k·f0·√(1 + B·k²). 1e-6 … 0,1.
    Kurve inharmonizitaet;
    /// Decay (log): so lange, bis der Grundton in der ersten Stufe um 60 dB fiele, in s. 0,05 … 120. Bei einer Saite
    /// im Chor die ganze Abklingzeit.
    Kurve abklingS;
    /// Late decay (log, HT3 a): dasselbe fuer die zweite Stufe (die Gegenlagen des Chors), in s, 0,05 … 120; kuerzer
    /// als *Decay* gilt als *Decay*.
    Kurve abklingSpaetS;
    /// High decay (log): die Abklingzeit eines Teiltons bei der zehnfachen Frequenz des Grundtons im Verhaeltnis zu
    /// der des Grundtons, 0,001 … 1. Die Verluste wachsen dazwischen mit dem Quadrat der Frequenz (31b).
    Kurve abklingHoch;
    /// Strings (linear, gerundet): Saiten im Chor, `kChorMin` … `kChorMax`.
    Kurve chor;
    /// Detune (linear): so weit liegen die aeusseren Saiten des Chors auseinander, in Cent, 0 … 20.
    Kurve verstimmungCent;
    /// Hammer mass (linear), in g, 1 … 30.
    Kurve hammerMasseG;
    /// Hammer stiffness Q0 (log), in N/mm^p (F = Q0·u^p bei langsamer Last, Stulov Gl. 4), 1 … 1e6.
    Kurve hammerSteife;
    /// Hammer exponent p (linear), 1 … 6.
    Kurve hammerExponent;
    /// Hammer hysteresis ε (linear, HU3-1 c): der Anteil der Steife, den der Filz mit der Zeit τ abbaut — Stulov 2005 /
    /// Stulov & Kartofelev 2009, Gl. 2: F = F0·[u^p − (ε/τ)·∫u^p(ξ)·e^(−(t−ξ)/τ)dξ] mit F0 = Q0/(1 − ε); 0 … 0,999.
    Kurve hammerHysterese;
    /// Hammer relaxation τ (log), in µs, 0,01 … 1000.
    Kurve hammerRelaxationUs;
    /// Strike position (linear): wo der Hammer trifft, als Anteil der Saite von der Agraffe aus, 0,02 … 0,5.
    Kurve anschlagStelle;
    /// Damper (log): so lange, bis ein Ton unter dem Daempfer um 60 dB gefallen ist, in s, 0,01 … 10.
    Kurve daempferS;
    /// String length (log): die schwingende Laenge, in m, 0,02 … 8.
    Kurve saitenLaengeM;
    /// String mass (log): die Masse je Laenge µ, in g/m, 0,5 … 500.
    Kurve saitenMasseGm;
    /// Longitudinal ratio (linear): die Grundfrequenz der Laengsschwingung im Verhaeltnis zum Grundton, 5 … 40.
    Kurve laengsVerhaeltnis;
    /// Bridge angle (linear, HT4 a): der Winkel der Saite gegen die Ebene am Steg, in Grad, 0 … 10.
    Kurve stegWinkelGrad;
    /// Polarization angle γ (linear): die erste Eigenrichtung gegen die Senkrechte, in Grad, 0 … 90.
    Kurve polarisationWinkelGrad;
    /// Polarization detune δ (linear): so viel hoeher liegt die zweite Eigenrichtung, in Cent, 0 … 20.
    Kurve polarisationCent;
    /// Horizontal ratio h (linear, P3-1 a): σ_H = h·σ_V, 0 … 1.
    Kurve waagrechtVerhaeltnis;
    /// Strike unevenness, soft (linear): u bei leisem Anschlag, 0 … 1 (US2 a).
    Kurve stossLeise;
    /// Strike unevenness, loud (linear): u bei lautem Anschlag, 0 … 1.
    Kurve stossLaut;

    /// gleich heisst: dieselben Werte (die Herkunft zaehlt nicht, `Kurve::operator==`)
    [[nodiscard]] bool operator==(const HammerSaite&) const = default;
};

/// Eine Kurve der Hammer-Saite mit allem, was sie beschreibt.
struct KurvenAngabe
{
    Kurve HammerSaite::*feld;
    std::string_view name;    ///< in `coreParams` (wie bis Fassung 20 im Block `hammerString`)
    std::string_view titel;   ///< im Creator
    std::string_view gruppe;  ///< Gruppe der Tafel (§6a)
    std::string_view einheit;
    Skala skala;
    double von;
    double bis;
};

/// Die 22 Kurven in der Reihenfolge der Tafel.
[[nodiscard]] const std::vector<KurvenAngabe>& kurvenAngaben();

/// Die Saiten im Chor an einer Taste: die Kurve, auf eine ganze Zahl gerundet (HK1 a).
[[nodiscard]] int chorAn(const HammerSaite& saite, int taste);

/// Die Vorgaben einer neuen Hammer-Saite (Befunde S146, S152, S159): Hammer nach Stulov & Kartofelev 2009 Gl. 9
/// (HU3-1 c); B bis E2 gemessen an *Salamander*, darueber nach Rigaud Gl. 5; Laenge, Masse, Laengsverhaeltnis und
/// Anschlagstelle aus RT-0425 Anhang A; Decay, High decay, Detune und Bridge angle gemessen an *Salamander* durch das
/// Modell hindurch (31d-4); Late decay, Damper, die zweite Ebene und der Stoss vorlaeufig. Jede Kurve mit Belegkraft
/// und Herkunft.
[[nodiscard]] const HammerSaite& hammerSaiteVorgabe();

/// Jede Kurve in ihre Grenzen (`kurvenAngaben()`); eine leere bekommt die Vorgabe.
void richteHammerSaite(HammerSaite& saite);

// --- die uebrigen Instrumente der Klasse 1 auf diesem Kern (34i-1, aus IC `VorgabenRest.cpp`) -----------------------

/// Upright (`upright_piano`, KL2 a): der Fluegel mit gekappten Basslaengen (Yamaha U1, 46,8″ — nur Bericht), bei
/// gleicher Spannung schwereren Saiten und groesserem B (B ∝ 1/L², Einschaetzung im Bericht); der Boden an der Schicht
/// (KL3 a). Ungemessen bis K18.
[[nodiscard]] HammerSaite baueVorgabeUpright();
/// Honky-tonk (`honky_tonk_piano`): das Upright mit ±1 Hz bei A4 zwischen den aeusseren Saiten des Chors ab MIDI 41
/// (Stimmerpraxis, nur Bericht). Ungemessen.
[[nodiscard]] HammerSaite baueVorgabeHonkytonk();
/// CP-70 (`electric_grand_piano`): der Fluegel mit einer Saite bis MIDI 42, darueber zwei (smem, Modartt — nur
/// Bericht); 73 Tasten E1–E7, kein Boden (der Piezo misst die Stegkraft). Ohne freie Aufnahme ungemessen (IC F-6 b).
[[nodiscard]] HammerSaite baueVorgabeCp70();

/// Chabassier & Durufle, RT-0425, Gl. 2–4 — die Groessen des Hammers an Taste i (1 … 88, i = MIDI − 20): Masse in g,
/// Exponent p, Steife K_H in N·m⁻ᵖ und umgerechnet Q0 = K_H·10⁻³ᵖ in N/mm^p (HT2 a).
[[nodiscard]] double rtHammerMasseG(int i);
[[nodiscard]] double rtHammerExponent(int i);
[[nodiscard]] double rtHammerSteifeNm(int i);
[[nodiscard]] double rtHammerSteife(int i);
/// Die Groessen des Hammers an Taste n (1 … 88, n = MIDI − 20): Masse in g, Exponent p, statische Steife Q0 in N/mm^p,
/// Hysterese ε und Relaxation τ in µs. Masse, p, τ nach Stulov & Kartofelev 2009, Gl. 9 (nach Stulov 2005; HU3-1 c);
/// ε und Q0 = F0·(1 − ε) nach Kartofelev & Stulov 2014, Gl. 5 und 7 (HS-3 a, S187 — p und τ dort gleich).
[[nodiscard]] double stulovHammerMasseG(int n);
[[nodiscard]] double stulovHammerExponent(int n);
[[nodiscard]] double stulovHammerSteife(int n);
[[nodiscard]] double stulovHammerHysterese(int n);
[[nodiscard]] double stulovHammerRelaxationUs(int n);
/// Rigaud, David & Daudet 2011, Gl. 4/5: B an MIDI-Nummer m, mit dem Diskantsteg der Arbeit (s_T = 9,26·10⁻²,
/// y_T = −13,64) und dem Bassteg `sB`, `yB`.
[[nodiscard]] double rigaudB(int m, double sB, double yB);
/// Der Bassteg der Vorgabe: abgelesen aus Abb. 1 der Arbeit (S146, vorlaeufig bis 31d).
inline constexpr double kRigaudBassSteigung = -0.0618;
inline constexpr double kRigaudBassAchse = -6.89;

/// So schnell trifft der Hammer bei voller Lautstaerke der Note, in m/s (KTH, forte, „about 5 m/s“; Stulov: „not
/// greater than 5 m/s“).
inline constexpr double kHammerTempoMax = 5.0;

/// Die Hammergeschwindigkeit einer Stufe in m/s: vMax · `anschlagFaktor(stufe)`.
[[nodiscard]] double hammerTempo(int stufe);

}  // namespace lumi::kern::hammer
