/**
 * @file Zunge.hpp
 * @brief InstrumentModels, Klasse 10 Durchschlagende Zunge: ein Ton eines Akkordeons, Harmoniums, einer Melodica oder Mundharmonika —
 *        Wind × Zunge mit Kanzelle (`klassen/Klasse 10 - Durchschlagende Zunge/modell-entwurf.md`; Entscheide Patrik
 *        2026-10-29: F-1 … F-9 a ueber FRAGEN_KONSOLIDIERT T1, T2, T3a, T3b, T5, T7, T8, E2)
 *
 * **Zunge:** die erste Biegemode als Masse-Feder-Daempfer (Millot & Baumann 2007 — nur Bericht), Auslenkung y der
 * Spitze, Ruhelage y₀ ueber dem Schlitz; Kraft S_r·p₂ mit dem Druck an der Zunge p₂ = ½ρ(U/S_u)² (S_r = ½·L·W,
 * offen). **Stroemung** (Puranik & Scavone 2023 Gl. 1–5, nachgelesen S. 1811): die Kanzelle (Volumen V₁) haelt die
 * Masse — V₁/(ρc²)·ṗ₁ = U_ein − U − S_r·ẏ; die Stroemung zur Zunge traege — ρ·(L₂/S₂)·U̇ = p₁ − p₂ mit der Durchflussflaeche
 * S_u = W·√((y + y₀)² + a²) (ebd. S. 1812, nur Bericht; a der Seitenspalt); U_ein ueber das Ventil (Taste) aus dem Balg p₀.
 * Die Traegheit der Stroemung treibt die Schwingung („the inertial effect of the upstream air flow“, St. Hilaire nach
 * Puranik & Scavone, nachgelesen S. 1809). **Ausgang:** ρ/(4π)·dU/dt. Vierfach abgetastet (wie Puranik & Scavone:
 * „4 × 44100 Hz“, nachgelesen S. 1813).
 *
 * **Bend** (Akkordeon, Melodica; F-2 a): die Ventiloeffnung wird kleiner (Kp nach Adachi et al. 2026 — die Steife sinkt
 * wirksam, die Tonhoehe faellt: −2,5 … −5,9 Hz/kPa, nachgelesen), nur abwaerts, gekappt an `bendGrenzeCent`.
 * **Choere** (F-4 a): je Chor eine eigene Zunge, gegeneinander verstimmt (Schwebung in Hz ueber den Umfang, Hergert).
 *
 * Uebernommen aus IC `rechnung/klassen/10_zunge/` (`4409088`, Vorlage unter `harvest/`), die Rechnung wortgleich;
 * geaendert (Instrumente_IC_Konzept.md 34j-2, Uebernahme-Journal J67): Namensraum und Includes; das Messen zieht aus
 * dem Kern (IK2-2 a) — `ZungenMessung` traegt nur die innere Groesse `reedTip`, Tonhoehe und Einschwingen misst
 * `InstrumentMeasure/ZungeStimmen`, `stimme` zieht dorthin (34j-3); `zungeDesChors` nennt f₀ und Ziel eines Chors (die
 * zwei Zeilen aus `rechneZungenTon`, dieselben Rechenschritte); der Chor ohne Namen und das Spiel ohne `kennzeichen`
 * (beides ohne Wirkung auf den Ton; „ungemessen“ folgt aus der Herkunft, IK2-9).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace lumi::kern::zunge
{

/// Ein Chor: Fusslage (Halbtoene gegen die Note) und Verstimmung als Schwebung gegen den Grundchor.
struct Chor
{
    int halbtoene = 0;        ///< 16′ −12, 8′ 0, 4′ +12
    double schwebungA4Hz = 0.0;  ///< Musette: die Schwebung an A4, ueber den Umfang ∝ √(f/440) steigend (offen)
    double pegel = 1.0;
};

struct ZungenSpiel
{
    std::string name;
    int noteMin = 53;
    int noteMax = 93;
    // --- die Zunge (Bezug C4: Adachi et al. 2026 Tab. 1, nachgelesen) ---
    double laengeC4M = 32.8e-3, breiteM = 3.45e-3;
    double masseC4Kg = 2.0 / 3.0 * 0.248e-3;   ///< „(2/3) × 0.248 g“
    double zeta = 0.003;
    double ruheM = 0.3e-3;                      ///< x_rest (nur Bericht)
    double seitenspaltM = 0.14e-3;              ///< a (nur Bericht)
    // --- Kanzelle und Kanal ---
    double kanzelleM3 = 14.8e-3 * 14.6e-3 * 20e-3;  ///< Querschnitt 14,8 × 14,6 mm² (nur Bericht), Tiefe 20 mm (offen)
    /// L₂, S₂ (offen; Harmonium 50 mm, 6,4·10⁻⁵ m² — nur Bericht): die Traegheit treibt die Schwingung; mit 5 mm
    /// facht sie in 400 ms an, mit 30 mm in rund 100 ms (Befund IC Session 4)
    double kanalLaengeM = 30e-3, kanalFlaecheM2 = 3.45e-3 * 2e-3;
    double ventilFlaecheM2 = 3e-4;                     ///< Klappe offen (offen)
    // --- Wind ---
    double druckPpPa = 300.0, druckFfPa = 2000.0;      ///< (Akkordeon 0–2,5 kPa Laborachse — nur Bericht; offen)
    double bendGrenzeCent = 100.0;
    std::vector<Chor> choere{{0, 0.0, 1.0}};  // 8′
    int bassAb = 0;                                    ///< Noten darunter: Bass-Seite (Stradella, F-5 a); 0 = keine
    std::vector<Chor> bassChoere;
    double gehaeuseTiefpassHz = 0.0;                   ///< Harmonium/Cassotto (offen); 0 = ohne
    Kurve eigenfrequenzJeNote;                         ///< f₀ der Zunge je Note (Rechnung, `stimme`)
    Kurve druckUntenJeNote, druckObenJeNote;           ///< das Fenster, in dem die Zunge anspricht (Rechnung); leer = pp … ff
    double ausgangBezug = 1.0;
};

struct ZungenAnschlag
{
    int note = 60;
    double druckPa = 0.0;      ///< > 0: der Druck unmittelbar (fuer `stimme`); 0 = aus Expression
    double expression = 0.5;
    double haltenS = 1.0;
    double pitchCent = 0.0;    ///< nur abwaerts (Ventil), gekappt
    double vibratoCent = 0.0;  ///< Druckmodulation (Balg-, Atemvibrato)
    double vibratoHz = 5.0;
    double balgwechselS = -1.0;  ///< ≥ 0: Druckeinbruch (F-3 a)
    int rate = 48000;
};

/// Die innere Groesse `reedTip`: die Spitze der Auslenkung in der zweiten Haelfte des Haltens. Tonhoehe und
/// Einschwingen misst `InstrumentMeasure/ZungeStimmen` an der ersten Zunge (IK2-2 a; IC mass sie hier).
struct ZungenMessung
{
    double amplitudeRel = 0.0;  ///< Spitze der Auslenkung / Zungenlaenge (Cottingham: ≈ 15 %)
};

void rechneZungenTon(const ZungenSpiel& s, const ZungenAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                     ZungenMessung* messung = nullptr);
/// Eine einzelne Zunge mit der Eigenfrequenz f₀ (fuer den Stimmer und Tests): der Ton und die innere Groesse.
void rechneEineZunge(const ZungenSpiel& s, double f0, double noteHz, const ZungenAnschlag& a, std::vector<double>& aus, std::int64_t frames,
                     ZungenMessung* messung);
/// Die Zunge eines Chors an einer Note: f₀ aus der Stimmung, um die Schwebung verschoben, und die Zielfrequenz (der
/// Weg von `rechneZungenTon`; der Stimmer und die Tests rechnen damit die erste Zunge allein).
struct ZungeDesChors
{
    double f0 = 0.0;
    double hz = 0.0;
};
[[nodiscard]] ZungeDesChors zungeDesChors(const ZungenSpiel& s, const Chor& c, int note);

/// Die Vorgaben ohne gestimmte Werte und Bezug (die setzt `InstrumentTuneTool`; IC: `Stimmung.inc` und `normiere`).
[[nodiscard]] ZungenSpiel baueVorgabeAkkordeon();
[[nodiscard]] ZungenSpiel baueVorgabeHarmonium();
[[nodiscard]] ZungenSpiel baueVorgabeMelodica();
[[nodiscard]] ZungenSpiel baueVorgabeMundharmonika();

}  // namespace lumi::kern::zunge
