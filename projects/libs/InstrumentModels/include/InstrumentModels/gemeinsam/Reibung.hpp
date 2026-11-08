/**
 * @file Reibung.hpp
 * @brief InstrumentModels, gemeinsam: die Reibung zwischen Bogen und Schwinger (Klasse 4 Stab gestrichen, spaeter
 *        Klasse 3 Saite — IC FRAGEN_KONSOLIDIERT T1 a: „der gestrichene Stab ist das Paar Bogen × Stab und kommt mit dem
 *        Bogenmodell der Klasse 3“)
 *
 * Uebernommen aus IC `rechnung/gemeinsam/Reibung.hpp`/`.cpp` (`bb7e6d6`) wortgleich bis auf Namensraum und Kopf
 * (Instrumente_IC_Konzept.md 34g-1, KS3 a, Uebernahme-Journal).
 *
 * μ(|Δv|) nach Galluzzo et al. 2017 Gl. 1 (in IC nachgelesen S. 5); Haften bis μ_s·F_B. Der Schwinger antwortet am
 * Bogenpunkt im Takt mit v = v_h + f/z_eff (v_h: ohne die Kraft dieses Takts; z_eff: die Impedanz, die der Bogen im Takt
 * sieht — an der Saite 2·Z0, beide Seiten; am Stab m/(Δt·Σφ²)). Gesucht ist der Schnitt dieser Geraden mit der
 * Reibkurve nach Friedlander mit Hysterese: es haftet weiter, solange die Haftkraft reicht; gleitet es, der aeusserste
 * Schnittpunkt.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

namespace lumi::kern
{

/// Galluzzo et al. 2017 Gl. 1: μ ueber |Δv| (m/s).
[[nodiscard]] double reibwertGalluzzo(double x);

/// Die Reibkraft am Bogenpunkt (Kraft auf den Schwinger) aus v_h, v_B, F_B; `haftet` traegt die Hysterese, `letzte`
/// den letzten Gleitpunkt (Startwert fuer Newton, anfangs −1).
[[nodiscard]] double reibung(double vh, double vb, double fb, double zEff, double muS, bool& haftet, double& letzte);

}  // namespace lumi::kern
