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
 * **Rechenfassung 2** (34j-6, ZV4 a, S191): die Gleichungen wie oben (aus IC, J67), aber jede Groesse der Zunge, des
 * Kanals, der Kanzelle und des Ventils ist eine Kurve ueber die Tonhoehe der Zunge (`ZungenSpiel`, Herleitung
 * `docs/modellierung/zunge/2026-11-11_S190_vorgaben_je_taste.md`): IC stufte nur die Laenge (∝ 1/√f), damit wuchsen
 * Schliessdruck und Ansprechschwelle mit f² (Befund S188). Ein Wind fuer alle Tasten (ZV2 a): `windMinPa` bei
 * Expression 0 bis `windMaxPa` bei 1, geometrisch; gestimmt wird f₀ am mittleren Druck (ZV5 a), die Tonhoehe folgt dem
 * Druck wie gemessen.
 *
 * **Bend** (Akkordeon, Melodica; F-2 a, ZV7 a–ZV9 a): die Ventiloeffnung wird kleiner (Adachi et al. 2026 — die
 * Tonhoehe faellt), nur abwaerts; welche Oeffnung welchen Bend gibt, misst der Stimmer je Taste am mittleren Druck, und
 * die Grenze der Taste kappt (bei hoeherem Druck wird derselbe Bend tiefer, wie gemessen).
 * **Choere** (F-4 a): je Chor eine eigene Zunge, gegeneinander verstimmt (Schwebung in Hz ueber den Umfang, Hergert); die
 * Groessen der Zunge gelten an ihrer eigenen Tonhoehe (Note plus Fusslage).
 *
 * Uebernommen aus IC `rechnung/klassen/10_zunge/` (`4409088`, Vorlage unter `harvest/`), die Rechnung wortgleich bis auf
 * die Groessen je Taste; geaendert (Instrumente_IC_Konzept.md 34j-2, 34j-6, Uebernahme-Journal J67, J70): Namensraum und
 * Includes; das Messen zieht aus dem Kern (IK2-2 a) — `ZungenMessung` traegt die innere Groesse `reedTip` und auf
 * Wunsch die Auslenkung, Tonhoehe und Ansprechen misst `InstrumentMeasure/ZungeStimmen`; `zungeDesChors` nennt f₀,
 * Ziel und Tonhoehe der Zunge eines Chors; der Chor ohne Namen und das Spiel ohne `kennzeichen` (IK2-9).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"
#include "InstrumentCore/Tonauftrag.hpp"

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

/// Die Vorgaben eines Instruments. Die Kurven laufen ueber die Tonhoehe der Zunge (MIDI) und werden logarithmisch
/// gelesen; ausserhalb ihrer Stuetzpunkte gilt der Randwert.
struct ZungenSpiel
{
    std::string name;
    int noteMin = 53;
    int noteMax = 93;
    // --- die Zunge je Taste ---
    Kurve laengeM;          ///< wirksame Laenge
    Kurve breiteM;
    Kurve flaechenmasse;    ///< Masse je wirksamer Flaeche ½·L·W in kg/m² (∝ Dicke); die Masse ist sie mal ½·L·W
    Kurve zeta;             ///< Daempfungsgrad
    Kurve ruheM;            ///< Aufbiegung (Ruheoeffnung ueber dem Schlitz)
    Kurve seitenspaltM;     ///< a
    // --- Kanzelle, Kanal, Ventil je Taste ---
    Kurve kanzelleM3;
    Kurve kanalLaengeM, kanalFlaecheM2;  ///< L₂, S₂: die Traegheit der Stroemung treibt die Schwingung
    Kurve ventilFlaecheM2;               ///< die Klappe offen
    // --- Wind (ZV2 a): ein Druck fuer alle Tasten ---
    double windMinPa = 200.0, windMaxPa = 1600.0;
    std::vector<Chor> choere{{0, 0.0, 1.0}};  // 8′
    int bassAb = 0;                                    ///< Noten darunter: Bass-Seite (Stradella, F-5 a); 0 = keine
    std::vector<Chor> bassChoere;
    double gehaeuseTiefpassHz = 0.0;                   ///< Harmonium/Cassotto (offen); 0 = ohne
    bool bendUeberVentil = false;                      ///< ZV7 a: Akkordeon und Melodica; sonst ohne Bend
    Kurve eigenfrequenzJeNote;                         ///< f₀ der Zunge je Note (Rechnung, Stimmer)
    /// ZV8 a: der Bend je Taste am mittleren Druck — die Tonhoehe gegen offen (Cent, ≤ 0) bei den Ventiloeffnungen
    /// `kBendStufen`; unter der Grenze der Taste traegt eine Stufe den Wert der Grenze
    std::vector<Kurve> bendCentJeStufe = std::vector<Kurve>(7);  // kBendStufen
    Kurve bendGrenzeOeffnung;  ///< die kleinste Oeffnung (gegen offen), bei der die Zunge noch anspricht; 1 = kein Bend
    Kurve bendGrenzeCent;      ///< die Tonhoehe dort (Cent gegen offen, ≤ 0)
    double ausgangBezug = 1.0;
};

/// Die Ventiloeffnungen des Rasters (ZV8 a): 2^(−k/4), k = 1 … 7 (0,84 … 0,30; die tiefste Grenze liegt bei rund 0,26).
inline constexpr int kBendStufen = 7;
static_assert(kBendStufen == 7, "ZungenSpiel::bendCentJeStufe mitziehen");
[[nodiscard]] double bendStufe(int k);  ///< k = 0 … kBendStufen − 1

/// Der Bend einer Taste, aus den gestimmten Kurven gelesen: die Punkte (Oeffnung, Cent) von offen bis zur Grenze.
struct BendDerTaste
{
    bool bendet = false;
    std::vector<double> oeffnung;  ///< fallend, zuerst 1
    std::vector<double> cent;      ///< zuerst 0
};
[[nodiscard]] BendDerTaste bendAn(const ZungenSpiel& s, int note);
/// Die Ventiloeffnung zu einem Bend in Cent (ZV9 a): nur abwaerts, gekappt an der Grenze der Taste, linear in
/// ln(Oeffnung); 1 ohne Bend.
[[nodiscard]] double oeffnungFuer(const BendDerTaste& b, double cent);
/// Dasselbe an einer Note: `oeffnungFuer(bendAn(s, note), cent)`.
[[nodiscard]] double ventilFuerCent(const ZungenSpiel& s, int note, double cent);

/// Die Groessen einer Zunge an ihrer Tonhoehe, wie der Kern sie aus den Kurven liest.
struct ZungeAnDerTaste
{
    double laenge = 0.0, breite = 0.0, masse = 0.0;
    double zeta = 0.0, ruhe = 0.0, spalt = 0.0;
    double kanzelle = 0.0, kanalLaenge = 0.0, kanalFlaeche = 0.0, ventil = 0.0;
};
[[nodiscard]] ZungeAnDerTaste zungeAn(const ZungenSpiel& s, int zungenNote);

/// Der Druck des Balgs bei einer Expression (0 … 1): `windMinPa` · (`windMaxPa`/`windMinPa`)^e.
[[nodiscard]] double windBei(const ZungenSpiel& s, double expression);
/// Der mittlere Druck (Expression 0,5), an dem gestimmt wird (ZV5 a).
[[nodiscard]] double windMitte(const ZungenSpiel& s);

struct ZungenAnschlag
{
    int note = 60;
    double druckPa = 0.0;      ///< > 0: der Druck unmittelbar (fuer den Stimmer); 0 = aus Expression
    double expression = 0.5;
    double haltenS = 1.0;
    double pitchCent = 0.0;     ///< Bend ueber das Ventil (ZV7 a, ZV9 a): nur abwaerts, ueber die Kurve der Taste, gekappt
    double ventilFaktor = 0.0;  ///< > 0: die Ventiloeffnung gegen offen unmittelbar (fuer den Stimmer); 0 = aus `pitchCent`
    double vibratoCent = 0.0;  ///< Druckmodulation (Balg-, Atemvibrato)
    double vibratoHz = 5.0;
    double balgwechselS = -1.0;  ///< ≥ 0: Druckeinbruch (F-3 a)
    int rate = 48000;
    /// 34j-7 (ZV3 a): die Verlaeufe ueber die Zeit, in Frames ab dem Beginn des Tons (linear zwischen den Punkten wie
    /// `spurWert`); ohne Punkte gilt der feste Wert oben. `balg`: die Richtung des Balgs 0/1 — an jedem Wechsel (der
    /// Wert geht ueber 0,5) bricht der Druck ein wie bei `balgwechselS`.
    Spur expressionVerlauf, pitchVerlauf, vibratoVerlauf, balgVerlauf;
};

/// Die innere Groesse `reedTip`: die Spitze der Auslenkung in der zweiten Haelfte des Haltens; mit `auslenkung` die
/// Auslenkung der Spitze je Ausgangswert (fuer den Stimmer, IK2-2 a).
struct ZungenMessung
{
    double amplitudeRel = 0.0;  ///< Spitze der Auslenkung / Zungenlaenge (Cottingham: ≈ 15 %)
    std::vector<double>* auslenkung = nullptr;
};

void rechneZungenTon(const ZungenSpiel& s, const ZungenAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                     ZungenMessung* messung = nullptr);
/// Eine einzelne Zunge mit der Eigenfrequenz f₀ an der Tonhoehe `zungenNote` (fuer den Stimmer und Tests).
void rechneEineZunge(const ZungenSpiel& s, double f0, int zungenNote, const ZungenAnschlag& a, std::vector<double>& aus, std::int64_t frames,
                     ZungenMessung* messung);
/// Die Zunge eines Chors an einer Note: f₀ aus der Stimmung, um die Schwebung verschoben, die Zielfrequenz und die
/// Tonhoehe der Zunge (der Weg von `rechneZungenTon`; der Stimmer und die Tests rechnen damit die erste Zunge allein).
struct ZungeDesChors
{
    double f0 = 0.0;
    double hz = 0.0;
    int note = 60;  ///< Note plus Fusslage, auf den Umfang geklemmt — dort gelten die Groessen und f₀
};
[[nodiscard]] ZungeDesChors zungeDesChors(const ZungenSpiel& s, const Chor& c, int note);

/// Die Vorgaben ohne gestimmte Werte und Bezug (die setzt `InstrumentTuneTool`).
[[nodiscard]] ZungenSpiel baueVorgabeAkkordeon();
[[nodiscard]] ZungenSpiel baueVorgabeHarmonium();
[[nodiscard]] ZungenSpiel baueVorgabeMelodica();
[[nodiscard]] ZungenSpiel baueVorgabeMundharmonika();

}  // namespace lumi::kern::zunge
