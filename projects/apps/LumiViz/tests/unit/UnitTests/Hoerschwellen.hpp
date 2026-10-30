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

#include "Zielsumme.hpp"

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

inline HoerschwellenGelesen liesHoerschwellen(const std::filesystem::path& pfad)
{
    HoerschwellenGelesen g;
    std::ifstream ein(pfad, std::ios::binary);
    if (!ein)
    {
        g.fehler = "Datei fehlt: " + pfad.string();
        return g;
    }
    Hoerschwellen h;
    std::string zeile;
    int nummer = 0;
    while (std::getline(ein, zeile))
    {
        ++nummer;
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        if (zeile.empty() || zeile.front() == '#') continue;
        std::istringstream s(zeile);
        std::vector<std::string> f;
        std::string teil;
        while (std::getline(s, teil, '\t')) f.push_back(teil);
        const auto kaputt = [&](const std::string& was) {
            g.fehler = pfad.string() + " Zeile " + std::to_string(nummer) + ": " + was;
            return g;
        };
        if (f.size() < 4) return kaputt("weniger als vier Spalten");
        Hoerschwelle w;
        w.muster = f[0];
        if (f[1] == "faktor") w.skala = HoerSkala::Faktor;
        else if (f[1] == "db") w.skala = HoerSkala::Dezibel;
        else if (f[1] == "ewert") w.skala = HoerSkala::Ewert;
        else return kaputt("Skala " + f[1] + " unbekannt");
        if (w.skala != HoerSkala::Ewert)
        {
            try
            {
                w.unten = std::stod(f[2]);
                w.oben = std::stod(f[3]);
            }
            catch (const std::exception&)
            {
                return kaputt("Grenzen keine Zahlen");
            }
        }
        if (f.size() > 4) w.quelle = f[4];
        h.zeilen.push_back(std::move(w));
    }
    if (h.zeilen.empty())
    {
        g.fehler = "keine Zeilen in " + pfad.string();
        return g;
    }
    g.werte = std::move(h);
    return g;
}

/// Die Modulationstiefe m zu einer Spitze-Spitze-Tiefe in dB und zurueck (Ewert 2002).
inline double ewertM(double ppDb)
{
    return std::tanh(std::max(ppDb, 0.0) * std::numbers::ln10 / 40.0);
}

inline double ewertPp(double m)
{
    return m >= 1.0 ? std::numeric_limits<double>::infinity() : 40.0 / std::numbers::ln10 * std::atanh(std::max(m, 0.0));
}

/// Die Unterscheidungsschwelle D in dB (in 20 lg m) bei der kleineren Tiefe.
inline double ewertD(double m)
{
    const double lg = m > 0.0 ? 20.0 * std::log10(m) : -std::numeric_limits<double>::infinity();
    if (lg >= -18.0) return 1.2;
    if (lg >= -28.0) return 1.2 + (-18.0 - lg) / 10.0 * (4.1 - 1.2);
    return 4.1;
}

/// Zwei Tiefen (Spitze-Spitze in dB) hoerbar gleich nach Ewert (s. Dateikopf).
inline bool ewertGleich(double a, double b)
{
    const double klein = std::min(a, b);
    const double gross = std::max(a, b);
    if (gross < 0.87) return true;
    const double mk = ewertM(klein);
    return gross <= ewertPp(mk * std::pow(10.0, ewertD(mk) / 20.0));
}

/// Modell und Ziel einer Groesse hoerbar gleich nach HW1 a; ohne Hoerschwelle fuer die Groesse falsch.
inline bool hoerbarGleich(const Hoerschwellen& schwellen, const std::string& groesse, double modell, double ziel)
{
    const Hoerschwelle* h = schwellen.fuer(groesse);
    if (h == nullptr) return false;
    switch (h->skala)
    {
    case HoerSkala::Faktor:
    {
        if (modell <= 0.0 || ziel <= 0.0) return false;
        const double d = std::log(modell / ziel);
        return -h->unten <= d && d <= h->oben;
    }
    case HoerSkala::Dezibel:
    {
        const double d = modell - ziel;
        return -h->unten <= d && d <= h->oben;
    }
    case HoerSkala::Ewert: return ewertGleich(modell, ziel);
    }
    return false;
}

/// Hoerbar getroffen nach HW2 a: |d| ≤ τ (r ≤ 1, ZF1 a) oder hoerbar gleich; eine fehlende Groesse nie.
inline bool hoerbarGetroffen(const Hoerschwellen& schwellen, const Abweichung& a)
{
    return !a.fehlt && (a.getroffen || hoerbarGleich(schwellen, a.groesse, a.modell, a.ziel));
}

}  // namespace lumi::messung
