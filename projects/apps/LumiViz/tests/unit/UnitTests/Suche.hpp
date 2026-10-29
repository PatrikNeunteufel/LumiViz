/**
 ****************************************************************************************
 * @file   Suche.hpp
 * @brief  7.5.10 (SM4 b): die Suche ueber die Stuetzpunkte — Residuen am Ton, Jacobi-Matrix je Kurve, Dateien je
 *         Iteration (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S156/S157/S161: ZF5 a, ZF6 a, SP2 a,
 *         SP3 a, SG1 a)
 *
 * @details
 * Verbindet die Kurven der Suche (`Suchkurven.hpp`), den Ton des Modells (`Modellton.hpp`), das Ziel
 * (`Zielsumme.hpp`) und Levenberg–Marquardt (`LevenbergMarquardt.hpp`). Die Residuen liegen Taste fuer Taste, je Taste
 * in der Reihenfolge der Groessen ihres Ziels; ein Stuetzpunkt einer Taste wirkt nur auf deren Zeilen
 * (`zeilenJeParameter`), deshalb genuegt je Kurve eine Auswertung mit allen ihren Stuetzpunkten ausgelenkt
 * (`gruppenJeKurve`, Lesart Claude S161). Schritt der finiten Differenz und Ende der Suche: 1 % der Groesse (log 0,01,
 * linear 1 % des Bereichs). Je Iteration eine Datei mit den Parametern (Zeile `x`, voll genau, zur Wiederaufnahme),
 * den Kurven im Format von `baueVorgabe()`, dem Rest je Taste und der Diagonale von JᵀJ je Stuetzpunkt.
 ****************************************************************************************
 */

#pragma once

#include "LevenbergMarquardt.hpp"
#include "Modellton.hpp"
#include "Strukturmessung.hpp"
#include "Suchkurven.hpp"
#include "Zielsumme.hpp"

#include "loop/HammerSaite.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace modelltest
{

/// Was eine Suche braucht: Kurven, Tasten, Struktur und Ziel.
struct SuchAufgabe
{
    Suchraum raum;
    std::vector<int> tasten;
    lumi::messung::Strukturen strukturen;
    lumi::messung::Ziele ziele;
};

/// Grenzen und Ende in der Koordinate der Suche; das Ende ist zugleich der Schritt der finiten Differenz.
inline lumi::messung::LmRaum lmRaum(const Suchraum& raum)
{
    lumi::messung::LmRaum l;
    for (const SuchKurve& k : raum.kurven)
        for (std::size_t i = 0; i < k.tasten.size(); ++i)
        {
            l.unten.push_back(k.koordinate(k.unten[i]));
            l.oben.push_back(k.koordinate(k.oben[i]));
            l.ende.push_back(k.log ? 0.01 : 0.01 * (k.oben[i] - k.unten[i]));
        }
    return l;
}

/// Je Kurve ihre Parameter.
inline std::vector<std::vector<std::size_t>> gruppenJeKurve(const Suchraum& raum)
{
    std::vector<std::vector<std::size_t>> g;
    std::size_t j = 0;
    for (const SuchKurve& k : raum.kurven)
    {
        g.emplace_back();
        for (std::size_t i = 0; i < k.tasten.size(); ++i) g.back().push_back(j++);
    }
    return g;
}

/// Die Zeilen der Residuen je Taste (Taste fuer Taste, je Taste so viele wie ihr Ziel Groessen hat).
inline std::vector<std::vector<std::size_t>> zeilenJeTaste(const SuchAufgabe& a)
{
    std::vector<std::vector<std::size_t>> z;
    std::size_t zeile = 0;
    for (const int t : a.tasten)
    {
        z.emplace_back();
        const auto it = a.ziele.find(t);
        const std::size_t n = it != a.ziele.end() ? it->second.size() : 0;
        for (std::size_t i = 0; i < n; ++i) z.back().push_back(zeile++);
    }
    return z;
}

/// Die Zeilen, auf die ein Parameter wirkt: die seiner Taste.
inline std::vector<std::vector<std::size_t>> zeilenJeParameter(const SuchAufgabe& a)
{
    const std::vector<std::vector<std::size_t>> jeTaste = zeilenJeTaste(a);
    std::vector<std::vector<std::size_t>> z;
    for (const SuchKurve& k : a.raum.kurven)
        for (const int t : k.tasten)
        {
            const auto pos = static_cast<std::size_t>(std::find(a.tasten.begin(), a.tasten.end(), t) - a.tasten.begin());
            z.push_back(pos < jeTaste.size() ? jeTaste[pos] : std::vector<std::size_t>{});
        }
    return z;
}

/// Ein Punkt der Suche ausgewertet: Residuen und je Taste die Summe.
struct Bewertet
{
    std::vector<double> r;
    std::vector<lumi::messung::TastenSumme> tasten;
};

/// Viele Punkte auf einem Pool ausgewertet (`werteAus`).
inline std::vector<Bewertet> bewerte(const SuchAufgabe& a, const std::vector<std::vector<double>>& punkte, double* sekunden = nullptr)
{
    std::vector<lumi::loop::HammerSaite> saiten;
    for (const std::vector<double>& x : punkte) saiten.push_back(saiteAus(a.raum, x));
    const std::vector<Auswertung> aus = werteAus(saiten, a.tasten, a.strukturen, a.ziele, sekunden);
    std::vector<Bewertet> b;
    for (const Auswertung& w : aus) b.push_back({lumi::messung::residuen(w.tasten), w.tasten});
    return b;
}

/// Der Name des Feldes in `HammerSaite` (fuer die Kurven im Format von `baueVorgabe()`).
inline const char* feldName(KurvenFeld f)
{
    using lumi::loop::HammerSaite;
    if (f == &HammerSaite::abklingS) return "abklingS";
    if (f == &HammerSaite::abklingSpaetS) return "abklingSpaetS";
    if (f == &HammerSaite::abklingHoch) return "abklingHoch";
    if (f == &HammerSaite::verstimmungCent) return "verstimmungCent";
    if (f == &HammerSaite::stossLeise) return "stossLeise";
    if (f == &HammerSaite::stossLaut) return "stossLaut";
    if (f == &HammerSaite::polarisationWinkelGrad) return "polarisationWinkelGrad";
    if (f == &HammerSaite::polarisationCent) return "polarisationCent";
    if (f == &HammerSaite::waagrechtVerhaeltnis) return "waagrechtVerhaeltnis";
    return "?";
}

/// Die Kurven der Saite zu `x` im Format von `baueVorgabe()` (direkt einsetzbar), vier gueltige Stellen.
inline std::string kurvenText(const Suchraum& raum, const std::vector<double>& x)
{
    const lumi::loop::HammerSaite saite = saiteAus(raum, x);
    std::ostringstream s;
    s << std::setprecision(4);
    for (const SuchKurve& k : raum.kurven)
    {
        s << "    // " << k.name << " (" << k.herkunft << ")\n    s." << feldName(k.feld) << " = kurveAusPunkten({";
        const lumi::loop::Tastenkurve& kurve = saite.*k.feld;
        for (std::size_t i = 0; i < kurve.punkte.size(); ++i) s << (i ? ", " : "") << '{' << kurve.punkte[i].taste << ", " << kurve.punkte[i].wert << '}';
        s << "});\n";
    }
    return s.str();
}

/// Wie stark die Residuen auf einen Stuetzpunkt ueber seinen ganzen Bereich antworten: √diag(JᵀJ) · Bereich, in
/// Toleranzen. Unter `kNichtAufgeloest` gilt er als von der Aufnahme nicht aufgeloest (SP3 a; Lesart Claude S162).
inline constexpr double kNichtAufgeloest = 0.1;

inline std::string diagonaleText(const Suchraum& raum, const std::vector<double>& diagonale)
{
    std::ostringstream s;
    s << std::setprecision(4);
    std::size_t j = 0;
    for (const SuchKurve& k : raum.kurven)
        for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
        {
            const double wirkung = std::sqrt(diagonale[j]) * k.bereich(i);
            s << k.name << '\t' << k.tasten[i] << '\t' << diagonale[j] << '\t' << wirkung << (wirkung < kNichtAufgeloest ? "\tnicht aufgeloest" : "")
              << '\n';
        }
    return s.str();
}

/// Die Parameter an einer Grenze als „Kurve Taste unten|oben“.
inline std::string grenzenText(const Suchraum& raum, const lumi::messung::LmRaum& l, const std::vector<double>& x)
{
    std::ostringstream s;
    std::size_t j = 0;
    int zahl = 0;
    for (const SuchKurve& k : raum.kurven)
        for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
            if (x[j] <= l.unten[j] || x[j] >= l.oben[j])
            {
                s << ' ' << k.name << ' ' << k.tasten[i] << (x[j] <= l.unten[j] ? " unten" : " oben") << ';';
                ++zahl;
            }
    return std::to_string(zahl) + (zahl ? ":" + s.str() : std::string());
}

inline std::filesystem::path iterationsDatei(const std::filesystem::path& ordner, int start, int nummer)
{
    return ordner / ("iteration_" + std::to_string(start) + "_" + std::to_string(nummer) + ".txt");
}

/// Die Datei einer Iteration: Kopf mit Zahlen, `x` voll genau, die Kurven, der Rest je Taste, die Grenzen, die Diagonale.
inline bool schreibeIteration(const std::filesystem::path& ordner, int start, const lumi::messung::LmIteration& it, const SuchAufgabe& a,
                              const std::vector<lumi::messung::TastenSumme>& tasten, double dauerS)
{
    std::ofstream aus(iterationsDatei(ordner, start, it.nummer), std::ios::binary);
    if (!aus) return false;
    const lumi::messung::Gesamt g = lumi::messung::gesamt(tasten);
    aus << std::setprecision(6) << "# Suche 7.5.10 (SP2 a), Startpunkt " << start << ", Iteration " << it.nummer << '\n'
        << "rest\t" << it.rest << "\nrest_vorher\t" << it.restVorher << "\nlambda\t" << std::setprecision(17) << it.lambda << std::setprecision(6)
        << "\nlambda_vorher\t" << it.lambdaVorher << "\nversuche\t" << it.versuche << "\nverworfen\t" << it.verworfen << "\nangenommen\t"
        << (it.angenommen ? 1 : 0) << "\nschritt_anteil\t" << it.schrittAnteil << "\ndauer_s\t" << dauerS << "\ngetroffen\t" << g.getroffen
        << " / " << g.groessen << "\nende\t" << it.ende << "\nx";
    aus << std::setprecision(17);
    for (const double v : it.x) aus << '\t' << v;
    aus << std::setprecision(6) << "\n\n## Kurven (wie baueVorgabe)\n" << kurvenText(a.raum, it.x) << "\n## Rest je Taste\n"
        << lumi::messung::berichte(tasten) << "\n## An der Grenze\n" << grenzenText(a.raum, lmRaum(a.raum), it.x)
        << "\n\n## Diagonale von J^T J je Stuetzpunkt (vor dem Schritt): Kurve, Taste, diag, Wirkung ueber den Bereich in Toleranzen\n"
        << diagonaleText(a.raum, it.diagonale);
    return static_cast<bool>(aus);
}

/// Die letzte geschriebene Iteration eines Startpunkts: Nummer, x und λ — zur Wiederaufnahme.
struct LetzteIteration
{
    int nummer = 0;
    std::vector<double> x;
    double lambda = 0.0;
    std::string ende;
};

inline std::optional<LetzteIteration> liesLetzteIteration(const std::filesystem::path& ordner, int start)
{
    int n = 0;
    while (std::filesystem::exists(iterationsDatei(ordner, start, n + 1))) ++n;
    if (n == 0) return std::nullopt;
    std::ifstream ein(iterationsDatei(ordner, start, n), std::ios::binary);
    LetzteIteration l;
    l.nummer = n;
    std::string zeile;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        std::istringstream s(zeile);
        std::string kopf;
        std::getline(s, kopf, '\t');
        if (kopf == "lambda") s >> l.lambda;
        else if (kopf == "ende") std::getline(s, l.ende);
        else if (kopf == "x")
        {
            std::string teil;
            while (std::getline(s, teil, '\t')) l.x.push_back(std::stod(teil));
        }
    }
    if (l.x.empty()) return std::nullopt;
    return l;
}

}  // namespace modelltest
