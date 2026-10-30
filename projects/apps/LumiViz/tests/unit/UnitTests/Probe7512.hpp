/**
 ****************************************************************************************
 * @file   Probe7512.hpp
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

#include "Hoerschwellen.hpp"
#include "Zielsumme.hpp"

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
inline std::vector<TastenSumme> liesBericht(const std::filesystem::path& pfad)
{
    std::ifstream ein(pfad, std::ios::binary);
    std::vector<std::string> zeilen;
    std::string zeile;
    bool mitAbschnitt = false;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        mitAbschnitt = mitAbschnitt || zeile == "## Je Taste und Groesse";
        zeilen.push_back(zeile);
    }
    std::vector<TastenSumme> tasten;
    bool drin = !mitAbschnitt;
    for (const std::string& z : zeilen)
    {
        if (z == "## Je Taste und Groesse")
        {
            drin = true;
            continue;
        }
        if (!drin) continue;
        if (z.rfind("## ", 0) == 0 || z.rfind("Gesamt:", 0) == 0)
        {
            if (mitAbschnitt) break;
            continue;
        }
        if (z.rfind("    ", 0) == 0 && !tasten.empty())
        {
            std::istringstream s(z);
            std::string skala, w;
            Abweichung a;
            std::string modell;
            s >> a.groesse >> skala >> w >> modell >> w >> a.ziel >> w >> a.toleranz >> w >> a.verhaeltnis;
            std::string rest;
            s >> rest;
            a.getroffen = rest == "getroffen";
            a.skala = skala == "[faktor]" ? Skala::Faktor : (skala == "[cent]" ? Skala::Cent : Skala::Dezibel);
            a.fehlt = modell == "-";
            if (!a.fehlt)
            {
                a.modell = std::stod(modell);
                const double d = a.skala == Skala::Faktor ? std::log(a.modell / a.ziel) : a.modell - a.ziel;
                a.residuum = d >= 0.0 ? a.verhaeltnis : -a.verhaeltnis;
            }
            else
                a.residuum = a.verhaeltnis;
            TastenSumme& t = tasten.back();
            t.getroffen += a.getroffen ? 1 : 0;
            t.fehlend += a.fehlt ? 1 : 0;
            t.groessen.push_back(std::move(a));
            continue;
        }
        if (const std::size_t strich = z.find(" | "); strich != std::string::npos && !z.empty() && std::isdigit(static_cast<unsigned char>(z.front())))
        {
            // die Summe aus der Zeile der Taste (aus den gerundeten r nachgerechnet wiche sie um Hundertstel ab)
            TastenSumme t;
            t.midi = std::stoi(z.substr(0, strich));
            t.summe = std::stod(z.substr(strich + 3));
            tasten.push_back(std::move(t));
        }
    }
    for (TastenSumme& t : tasten) t.fertig = !t.groessen.empty() && t.getroffen == static_cast<int>(t.groessen.size());
    return tasten;
}

/// Eine Groesse an einer Taste ueber die Startpunkte (HW2 a, s. Dateikopf).
inline bool eindeutigNachHw2(const Hoerschwellen& schwellen, const std::vector<const Abweichung*>& jeStart)
{
    if (jeStart.empty()) return false;
    double klein = 0.0, gross = 0.0;
    for (std::size_t i = 0; i < jeStart.size(); ++i)
    {
        if (jeStart[i]->fehlt) return false;
        klein = i == 0 ? jeStart[i]->modell : std::min(klein, jeStart[i]->modell);
        gross = i == 0 ? jeStart[i]->modell : std::max(gross, jeStart[i]->modell);
    }
    const Abweichung& a = *jeStart.front();
    const double d = a.skala == Skala::Faktor ? (klein > 0.0 ? std::log(gross / klein) : std::numeric_limits<double>::infinity()) : gross - klein;
    if (d <= a.toleranz) return true;
    return hoerbarGleich(schwellen, a.groesse, gross, klein) && hoerbarGleich(schwellen, a.groesse, klein, gross);
}

/// Der exakte Vorzeichentest, zweiseitig: k von n in eine Richtung, p = min(1, 2·P(X ≤ min(k, n − k))).
inline double vorzeichenTestP(int k, int n)
{
    if (n <= 0) return 1.0;
    const int m = std::min(k, n - k);
    double summe = 0.0;
    double binom = 1.0;  // C(n, i)
    for (int i = 0; i <= m; ++i)
    {
        summe += binom;
        binom = binom * (n - i) / (i + 1);
    }
    return std::min(1.0, 2.0 * summe * std::pow(0.5, n));
}

/// Holm: je p-Wert, ob er nach der schrittweisen Korrektur verworfen wird (α).
inline std::vector<bool> holm(const std::vector<double>& p, double alpha = 0.05)
{
    std::vector<std::size_t> reihe(p.size());
    std::iota(reihe.begin(), reihe.end(), std::size_t{0});
    std::stable_sort(reihe.begin(), reihe.end(), [&p](std::size_t a, std::size_t b) { return p[a] < p[b]; });
    std::vector<bool> verworfen(p.size(), false);
    const std::size_t m = p.size();
    for (std::size_t i = 0; i < m; ++i)
    {
        if (p[reihe[i]] > alpha / static_cast<double>(m - i)) break;
        verworfen[reihe[i]] = true;
    }
    return verworfen;
}

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

inline std::vector<GroessenBefund> systematik(const Hoerschwellen& schwellen, const std::vector<TastenSumme>& tasten, double alpha = 0.05)
{
    std::map<std::string, GroessenBefund> je;
    std::map<std::string, std::vector<double>> betraege;
    for (const TastenSumme& t : tasten)
        for (const Abweichung& a : t.groessen)
        {
            GroessenBefund& b = je[a.groesse];
            b.groesse = a.groesse;
            ++b.tasten;
            betraege[a.groesse].push_back(a.verhaeltnis);
            if (hoerbarGetroffen(schwellen, a)) continue;
            if (a.fehlt)
            {
                ++b.fehlend;
                continue;
            }
            ++b.nichtGetroffen;
            b.zuGross += a.residuum > 0.0 ? 1 : 0;
        }
    std::vector<GroessenBefund> alle;
    for (auto& [name, b] : je)
    {
        b.p = vorzeichenTestP(b.zuGross, b.nichtGetroffen);
        const double grenze = 3.0 * detail::median(betraege[name]);
        for (const TastenSumme& t : tasten)
            for (const Abweichung& a : t.groessen)
                if (a.groesse == name && !hoerbarGetroffen(schwellen, a) && a.verhaeltnis > grenze) b.ausreisser.push_back(t.midi);
        alle.push_back(b);
    }
    std::vector<double> p;
    std::vector<std::size_t> getestet;
    for (std::size_t i = 0; i < alle.size(); ++i)
        if (alle[i].nichtGetroffen > 0)
        {
            p.push_back(alle[i].p);
            getestet.push_back(i);
        }
    const std::vector<bool> v = holm(p, alpha);
    for (std::size_t i = 0; i < getestet.size(); ++i) alle[getestet[i]].systematisch = v[i];
    return alle;
}

/// Zahlen ueber alle Tasten: getroffen nach ZF1 a, hoerbar gleich allein (|d| ≤ S) und hoerbar getroffen (HW2 a).
struct HoerZahlen
{
    int groessen = 0;
    int getroffen = 0;
    int hoerbarGleich = 0;
    int hoerbarGetroffen = 0;
};

inline HoerZahlen hoerZahlen(const Hoerschwellen& schwellen, const std::vector<TastenSumme>& tasten)
{
    HoerZahlen z;
    for (const TastenSumme& t : tasten)
        for (const Abweichung& a : t.groessen)
        {
            ++z.groessen;
            z.getroffen += a.getroffen ? 1 : 0;
            z.hoerbarGleich += !a.fehlt && hoerbarGleich(schwellen, a.groesse, a.modell, a.ziel) ? 1 : 0;
            z.hoerbarGetroffen += hoerbarGetroffen(schwellen, a) ? 1 : 0;
        }
    return z;
}

}  // namespace lumi::messung
