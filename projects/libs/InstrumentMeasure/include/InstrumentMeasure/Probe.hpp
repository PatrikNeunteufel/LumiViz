/**
 ****************************************************************************************
 * @file   Probe.hpp
 * @brief  7.5.12: die Auswertung der Probe nach HW2 a, HW3 a und HW4 a — Berichte lesen, eindeutig ueber die
 *         Startpunkte, Vorzeichentest mit Holm, Ausreisser (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik
 *         S164)
 *
 * @details
 * Reines C++ ohne Qt. `liesBericht` liest die Groessen je Taste aus einem Bericht von `berichte(…, true)` (Abschnitt
 * „## Je Taste und Groesse“ einer Ergebnisdatei der Suche); das Vorzeichen des Residuums kommt aus Modell und Ziel.
 * Eindeutig (HW2 a) ist eine Groesse, wenn die Startpunkte an ihr hoechstens max(τ, S) auseinander enden: der groesste
 * Abstand (kleinster gegen groessten Modellwert) liegt in τ oder ist in beide Richtungen hoerbar gleich (Lesart Claude
 * S165: bei ungleichen Grenzen unten/oben gilt so die engere); fehlt die Groesse an einem Startpunkt, ist sie nicht
 * eindeutig (Lesart S165). Systematisch (HW4 a) ist eine Groesse, wenn ihre nicht hoerbar getroffenen Tasten im
 * Vorzeichentest (zweiseitig, exakt) in eine Richtung abweichen, Holm ueber die Groessen mit mindestens einer solchen
 * Taste, α = 0,05; fehlende Modellwerte haben kein Vorzeichen und zaehlen nicht (Lesart S165). Ausreisser: nicht hoerbar
 * getroffen und |r| ueber dem Dreifachen des Medians von |r| ueber alle Tasten der Groesse.
 ****************************************************************************************
 */

#pragma once

#include "InstrumentMeasure/Hoerschwellen.hpp"
#include "InstrumentMeasure/Zielsumme.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace lumi::messung
{

/// Die Groessen je Taste aus einem Bericht `berichte(…, true)`: nur der Abschnitt „## Je Taste und Groesse“, wenn die
/// Datei ihn hat, sonst die ganze Datei.
std::vector<TastenSumme> liesBericht(const std::filesystem::path& pfad);

/// Eine Groesse an einer Taste ueber die Startpunkte (HW2 a, s. Dateikopf).
bool eindeutigNachHw2(const Hoerschwellen& schwellen, const std::vector<const Abweichung*>& jeStart);

/// Der exakte Vorzeichentest, zweiseitig: k von n in eine Richtung, p = min(1, 2·P(X ≤ min(k, n − k))).
double vorzeichenTestP(int k, int n);

/// Holm: je p-Wert, ob er nach der schrittweisen Korrektur verworfen wird (α).
std::vector<bool> holm(const std::vector<double>& p, double alpha = 0.05);

/// Je Groesse (HW4 a): die nicht hoerbar getroffenen Tasten, wie viele davon zu gross (Modell ueber Ziel), p, ob
/// systematisch, die Ausreisser.
struct GroessenBefund
{
    std::string groesse;
    int tasten = 0;
    int nichtGetroffen = 0;  ///< mit Vorzeichen (ohne fehlende)
    int fehlend = 0;
    int zuGross = 0;
    double p = 1.0;
    bool systematisch = false;
    std::vector<int> ausreisser;
};

std::vector<GroessenBefund> systematik(const Hoerschwellen& schwellen, const std::vector<TastenSumme>& tasten, double alpha = 0.05);

/// Zahlen ueber alle Tasten: getroffen nach ZF1 a, hoerbar gleich allein (|d| ≤ S) und hoerbar getroffen (HW2 a).
struct HoerZahlen
{
    int groessen = 0;
    int getroffen = 0;
    int hoerbarGleich = 0;
    int hoerbarGetroffen = 0;
};

HoerZahlen hoerZahlen(const Hoerschwellen& schwellen, const std::vector<TastenSumme>& tasten);

}  // namespace lumi::messung
