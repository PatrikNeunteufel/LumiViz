/**
 ****************************************************************************************
 * @file   Hoerschwellen.hpp
 * @brief  7.5.13 B2 / 7.5.12: die Hoerschwellen je Messgroesse der Suche — lesen und anwenden
 *         (docs/audio/Spielweisen_Konzept.md §4.1, „7.5.13 B1“; Entscheide Patrik S164: HW1 a, HW2 a)
 *
 * @details
 * Reines C++ ohne Qt. Die Zahlen stehen von Hand in `werte/hoerschwellen.txt` (Format im Dateikopf dort): je Zeile ein
 * Groessenmuster (Praefix, der laengste passende gilt), die Skala und die Grenzen unten (Modell kuerzer bzw. leiser) und
 * oben (laenger bzw. lauter). `faktor`: gleich bei −unten ≤ ln(Modell/Ziel) ≤ oben; `db`: gleich bei −unten ≤ Modell −
 * Ziel ≤ oben; `ewert` (Schwebungs- und Einbruchtiefe, Spitze-Spitze in dB, nach Ewert 2002): m = tanh(pp·ln10/40),
 * gleich, wenn der groessere Wert hoechstens pp(m_klein·10^(D/20)) ist, D = 1,2 dB fuer 20 lg m_klein ≥ −18 dB, linear
 * bis 4,1 dB bei −28 dB, darunter 4,1 dB; beide unter 0,87 dB sind gleich. Hoerbar getroffen ist eine Groesse nach
 * HW2 a bei |d| ≤ τ (die Toleranz mit der Untergrenze UG1 a, also r ≤ 1) oder hoerbar gleich.
 ****************************************************************************************
 */

#pragma once

#include "InstrumentMeasure/Zielsumme.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <numbers>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace lumi::messung
{

enum class HoerSkala
{
    Faktor,
    Dezibel,
    Ewert
};

struct Hoerschwelle
{
    std::string muster;
    HoerSkala skala = HoerSkala::Faktor;
    double unten = 0.0;
    double oben = 0.0;
    std::string quelle;
};

struct Hoerschwellen
{
    std::vector<Hoerschwelle> zeilen;

    /// Die Zeile mit dem laengsten passenden Praefix, `nullptr` ohne.
    [[nodiscard]] const Hoerschwelle* fuer(const std::string& groesse) const
    {
        const Hoerschwelle* beste = nullptr;
        for (const Hoerschwelle& h : zeilen)
            if (groesse.rfind(h.muster, 0) == 0 && (beste == nullptr || h.muster.size() > beste->muster.size())) beste = &h;
        return beste;
    }
};

/// Gelesen oder der Fehler (Datei fehlt, Zeile kaputt).
struct HoerschwellenGelesen
{
    std::optional<Hoerschwellen> werte;
    std::string fehler;
};

HoerschwellenGelesen liesHoerschwellen(const std::filesystem::path& pfad);

/// Die Modulationstiefe m zu einer Spitze-Spitze-Tiefe in dB und zurueck (Ewert 2002).
double ewertM(double ppDb);

double ewertPp(double m);

/// Die Unterscheidungsschwelle D in dB (in 20 lg m) bei der kleineren Tiefe.
double ewertD(double m);

/// Zwei Tiefen (Spitze-Spitze in dB) hoerbar gleich nach Ewert (s. Dateikopf).
bool ewertGleich(double a, double b);

/// Modell und Ziel einer Groesse hoerbar gleich nach HW1 a; ohne Hoerschwelle fuer die Groesse falsch.
bool hoerbarGleich(const Hoerschwellen& schwellen, const std::string& groesse, double modell, double ziel);

/// Hoerbar getroffen nach HW2 a: |d| ≤ τ (r ≤ 1, ZF1 a) oder hoerbar gleich; eine fehlende Groesse nie.
bool hoerbarGetroffen(const Hoerschwellen& schwellen, const Abweichung& a);

}  // namespace lumi::messung
