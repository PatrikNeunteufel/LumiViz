/**
 * @file Balken.hpp
 * @brief InstrumentModels, gemeinsam: Moden eines Balkens mit veraenderlichem Querschnitt (Stab, Zunge, Kragbalken)
 *        aus finiten Elementen (Klassen 4 und 10)
 *
 * Uebernommen aus IC `rechnung/gemeinsam/Balken.hpp`/`.cpp` (`bb7e6d6`) wortgleich bis auf Namensraum und Kopf; der
 * symmetrische Eigenloeser `zerlegeSymmetrisch` steht seit 34e im Core (`InstrumentCore/Eigenzerlegung.hpp`, ME2 a),
 * der Balken nutzt ihn von dort (Instrumente_IC_Konzept.md 34g-1, Uebernahme-Journal).
 *
 * Euler–Bernoulli-Balken, je Element zwei Knoten mit Auslenkung und Neigung (Hermite-Polynome dritter Ordnung), die
 * Steife EI und die Masse ρA je Element aus dem Querschnitt in der Elementmitte (Rechteck b × h: I = b·h³/12); dazu
 * auf Wunsch die Drehtraegheit nach Rayleigh (ρI, die konsistente Matrix der Neigung). Lager je Ende: frei, gelenkig,
 * eingespannt. Das verallgemeinerte Problem K·φ = ω²·M·φ wird mit Cholesky (M = L·Lᵀ) auf ein symmetrisches gebracht
 * und mit Jacobi-Drehungen geloest (zyklisch, bis die Nebendiagonale unter 10⁻¹⁴ der Diagonale liegt — langsam, aber
 * genau und deterministisch; bis rund 300 Freiheitsgrade).
 *
 * **Nicht enthalten (offen):** der Schub (Timoshenko) — er senkt bei dicken Staeben die oberen Moden um einige Prozent
 * (Einschaetzung); fuer die Vorgaben der Klasse 4 stehen die Verhaeltnisse der Moden ohnehin als Kurven aus der
 * Literatur, der Balken dient den Formen der Moden, dem analytischen Test (frei–frei 1 : 2,757 : 5,404) und dem
 * Bauplan-Weg. Torsion (Rucz & Ulveczki 2021, S. 7) steht in der Klasse 4 als Modenliste mit eigener Kurve, nicht hier.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include <functional>
#include <vector>

namespace lumi::kern
{

/// Wie ein Ende gelagert ist.
enum class Lager
{
    Frei,
    Gelenkig,
    Eingespannt,
};

/// Ein Balken: Laenge, Material, Querschnitt ueber x (0 … laenge).
struct Balken
{
    double laengeM = 0.5;
    double eModulPa = 1e10;
    double dichteKgM3 = 1000.0;
    std::function<double(double)> breiteM = [](double) { return 0.05; };
    std::function<double(double)> hoeheM = [](double) { return 0.02; };
    Lager links = Lager::Frei;
    Lager rechts = Lager::Frei;
    int elemente = 60;
    bool drehtraegheit = false;  ///< Rayleigh: ρI in der Massenmatrix
};

/// Eine Mode des Balkens: Kreisfrequenz und die Form an den Knoten (Auslenkung je Knoten), M-normiert
/// (Σ φᵀMφ = 1 kg, also modale Masse 1 kg in diesen Einheiten).
struct BalkenMode
{
    double hz = 0.0;
    std::vector<double> auslenkung;  ///< je Knoten (elemente + 1), x_i = i·L/elemente
};

/// Die tiefsten `anzahl` Biegemoden (Starrkoerpermoden eines freien Balkens fallen weg: Frequenz unter 10⁻³ der
/// ersten elastischen).
[[nodiscard]] std::vector<BalkenMode> biegeModen(const Balken& balken, int anzahl);

/// Die Form einer Mode an der Stelle x (linear zwischen den Knoten).
[[nodiscard]] double formAn(const BalkenMode& mode, const Balken& balken, double x);

/// Die frei–freien Verhaeltnisse f_n/f_1 eines gleichfoermigen Euler–Bernoulli-Balkens aus (β_n L)²: 4,7300;
/// 7,8532; 10,9956; 14,1372; dann (2n + 1)π/2 (Lehrbuch: cos βL · cosh βL = 1).
[[nodiscard]] std::vector<double> freiFreiVerhaeltnisse(int anzahl);

/// β_n L eines Kragbalkens (eingespannt–frei): cos βL · cosh βL = −1 → 1,8751; 4,6941; 7,8548; 10,9955; …
[[nodiscard]] std::vector<double> kragBetaL(int anzahl);

}  // namespace lumi::kern
