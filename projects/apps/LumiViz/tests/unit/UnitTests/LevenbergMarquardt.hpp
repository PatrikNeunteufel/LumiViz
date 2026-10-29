/**
 ****************************************************************************************
 * @file   LevenbergMarquardt.hpp
 * @brief  7.5.10 (SM4 b): Levenberg–Marquardt mit Grenzen fuer das Setzen der Hammer-Saite an den Stuetzpunkten
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S156/S157: ZF6 a, SP2 a)
 *
 * @details
 * Reines C++ ohne Qt und ohne Eigen — die Normalgleichungen sind klein (rund 200 Parameter). Je Iteration wird die
 * Jacobi-Matrix J am Punkt x einmal gebildet; der Schritt loest (JᵀJ + λ·diag(JᵀJ))·δ = −Jᵀr (Marquardt-Skalierung,
 * Gauss mit Spaltenpivot). Ein Schritt mit kleinerem Rest Σr² wird genommen und λ durch `lambdaFaktor` geteilt, ein
 * schlechterer verworfen und λ mit ihm malgenommen (bis `versucheHoechstens` Versuche je Iteration). Die Grenzen
 * wirken durch Abschneiden nach dem Schritt. Ende, wenn jeder Schritt unter `LmRaum::ende` liegt (der Auftrag 7.5.10:
 * 1 % der Groesse — log 0,01, linear 1 % des Bereichs), wenn kein Versuch den Rest senkt, oder nach
 * `iterationenHoechstens`. Die Jacobi-Matrix je Gruppe (`jacobiJeGruppe`) lenkt alle Parameter einer Gruppe zugleich
 * aus — gueltig, wenn jeder Parameter der Gruppe nur auf seine eigenen Zeilen wirkt (Lesart Claude S161: ein
 * Stuetzpunkt wirkt nur auf den Ton seiner Taste).
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace lumi::messung
{

/// Eine dichte Matrix, zeilenweise.
struct Matrix
{
    std::size_t zeilen = 0;
    std::size_t spalten = 0;
    std::vector<double> werte;

    Matrix() = default;
    Matrix(std::size_t z, std::size_t s) : zeilen(z), spalten(s), werte(z * s, 0.0) {}
    double& operator()(std::size_t i, std::size_t j) { return werte[i * spalten + j]; }
    [[nodiscard]] double operator()(std::size_t i, std::size_t j) const { return werte[i * spalten + j]; }
    [[nodiscard]] bool operator==(const Matrix&) const = default;
};

inline double quadratsumme(const std::vector<double>& r)
{
    double s = 0.0;
    for (const double v : r) s += v * v;
    return s;
}

/// Loest a·x = b (Gauss mit Spaltenpivot); eine Spalte ohne Pivot (singulaer) bekommt x = 0.
inline std::vector<double> loese(Matrix a, std::vector<double> b)
{
    const std::size_t n = a.zeilen;
    std::vector<std::size_t> reihe(n);
    for (std::size_t i = 0; i < n; ++i) reihe[i] = i;
    std::vector<bool> frei(n, false);
    double groesst = 0.0;
    for (const double v : a.werte) groesst = std::max(groesst, std::abs(v));
    const double klein = groesst * 1e-14;
    for (std::size_t k = 0; k < n; ++k)
    {
        std::size_t p = k;
        for (std::size_t i = k + 1; i < n; ++i)
            if (std::abs(a(i, k)) > std::abs(a(p, k))) p = i;
        if (std::abs(a(p, k)) <= klein)
        {
            frei[k] = true;
            continue;
        }
        if (p != k)
        {
            for (std::size_t j = 0; j < n; ++j) std::swap(a(k, j), a(p, j));
            std::swap(b[k], b[p]);
        }
        for (std::size_t i = k + 1; i < n; ++i)
        {
            const double f = a(i, k) / a(k, k);
            if (f == 0.0) continue;
            for (std::size_t j = k; j < n; ++j) a(i, j) -= f * a(k, j);
            b[i] -= f * b[k];
        }
    }
    std::vector<double> x(n, 0.0);
    for (std::size_t k = n; k-- > 0;)
    {
        if (frei[k]) continue;
        double s = b[k];
        for (std::size_t j = k + 1; j < n; ++j) s -= a(k, j) * x[j];
        x[k] = s / a(k, k);
    }
    return x;
}

/// Die Diagonale von JᵀJ — je Parameter, wie stark die Residuen auf ihn antworten.
inline std::vector<double> diagonaleJtJ(const Matrix& j)
{
    std::vector<double> d(j.spalten, 0.0);
    for (std::size_t i = 0; i < j.zeilen; ++i)
        for (std::size_t k = 0; k < j.spalten; ++k) d[k] += j(i, k) * j(i, k);
    return d;
}

/// Der Schritt δ aus (JᵀJ + λ·diag(JᵀJ))·δ = −Jᵀr; eine Diagonale von null wird fuer die Daempfung auf ein Billionstel
/// der groessten gehoben (sonst bliebe die Matrix bei einem Parameter ohne Wirkung singulaer).
inline std::vector<double> lmSchritt(const Matrix& j, const std::vector<double>& r, double lambda)
{
    const std::size_t n = j.spalten;
    Matrix a(n, n);
    std::vector<double> g(n, 0.0);
    for (std::size_t i = 0; i < j.zeilen; ++i)
        for (std::size_t p = 0; p < n; ++p)
        {
            const double jp = j(i, p);
            if (jp == 0.0) continue;
            g[p] -= jp * r[i];
            for (std::size_t q = p; q < n; ++q) a(p, q) += jp * j(i, q);
        }
    double groesst = 0.0;
    for (std::size_t p = 0; p < n; ++p)
    {
        for (std::size_t q = 0; q < p; ++q) a(p, q) = a(q, p);
        groesst = std::max(groesst, a(p, p));
    }
    const double boden = groesst > 0.0 ? groesst * 1e-12 : 1e-12;
    for (std::size_t p = 0; p < n; ++p) a(p, p) += lambda * std::max(a(p, p), boden);
    return loese(std::move(a), std::move(g));
}

/// Grenzen und Ende je Parameter (in der Koordinate der Suche).
struct LmRaum
{
    std::vector<double> unten;
    std::vector<double> oben;
    /// Ein Schritt darunter gilt als klein; sind alle klein, endet die Suche.
    std::vector<double> ende;
};

struct LmEinstellungen
{
    double lambdaStart = 1e-3;
    double lambdaFaktor = 10.0;
    int iterationenHoechstens = 20;
    int versucheHoechstens = 10;
};

/// Was eine Iteration getan hat (fuer Bericht und Dateien).
struct LmIteration
{
    int nummer = 0;
    std::vector<double> x;    ///< nach der Iteration
    std::vector<double> r;
    double rest = 0.0;
    double restVorher = 0.0;
    double lambdaVorher = 0.0;
    double lambda = 0.0;      ///< nach der Iteration
    int versuche = 0;         ///< ausgewertete Schritte
    int verworfen = 0;
    bool angenommen = false;
    double schrittAnteil = 0.0;  ///< groesster |Schritt| / ende ueber die Parameter (< 1: klein)
    std::vector<double> diagonale;  ///< von JᵀJ am Punkt vor dem Schritt
    std::vector<std::size_t> anGrenze;
    int auswertungenResiduen = 0;
    std::string ende;  ///< leer, solange die Suche weiterlaeuft
};

struct LmErgebnis
{
    std::vector<double> x;
    std::vector<double> r;
    double rest = 0.0;
    double lambda = 0.0;
    int iterationen = 0;
    int auswertungen = 0;  ///< Auswertungen der Residuen (ohne die der Jacobi-Matrix)
    std::vector<std::size_t> anGrenze;
    std::string ende;
};

/// Wo die Suche beginnt: am Punkt `x` mit `lambda`, nach `iteration` schon gelaufenen Iterationen; `r` (wenn nicht leer)
/// sind die Residuen dort — dann wird x nicht neu ausgewertet (Wiederaufnahme).
struct LmStart
{
    LmStart() = default;
    LmStart(std::vector<double> x_, double lambda_ = 1e-3, int iteration_ = 0, std::vector<double> r_ = {})
        : x(std::move(x_)), lambda(lambda_), iteration(iteration_), r(std::move(r_))
    {
    }

    std::vector<double> x;
    double lambda = 1e-3;
    int iteration = 0;
    std::vector<double> r;
};

using LmResiduen = std::function<std::vector<double>(const std::vector<double>&)>;
using LmJacobi = std::function<Matrix(const std::vector<double>& x, const std::vector<double>& r)>;
using LmBeobachter = std::function<void(const LmIteration&)>;

inline std::vector<double> klemmeIn(const LmRaum& raum, std::vector<double> x)
{
    for (std::size_t j = 0; j < x.size(); ++j) x[j] = std::clamp(x[j], raum.unten[j], raum.oben[j]);
    return x;
}

inline std::vector<std::size_t> anGrenzeIn(const LmRaum& raum, const std::vector<double>& x)
{
    std::vector<std::size_t> g;
    for (std::size_t j = 0; j < x.size(); ++j)
        if (x[j] <= raum.unten[j] || x[j] >= raum.oben[j]) g.push_back(j);
    return g;
}

/// Levenberg–Marquardt mit Grenzen (s. Dateikopf).
inline LmErgebnis levenbergMarquardt(const LmResiduen& residuen, const LmJacobi& jacobi, const LmStart& start, const LmRaum& raum,
                                     const LmEinstellungen& einst = {}, const LmBeobachter& beobachter = {})
{
    LmErgebnis e;
    e.x = klemmeIn(raum, start.x);
    if (start.r.empty())
    {
        e.r = residuen(e.x);
        ++e.auswertungen;
    }
    else
        e.r = start.r;
    e.rest = quadratsumme(e.r);
    e.lambda = start.lambda;
    e.iterationen = start.iteration;
    while (e.ende.empty())
    {
        if (e.iterationen >= einst.iterationenHoechstens)
        {
            e.ende = "Iterationen";
            break;
        }
        LmIteration it;
        it.nummer = ++e.iterationen;
        it.restVorher = e.rest;
        it.lambdaVorher = e.lambda;
        const Matrix j = jacobi(e.x, e.r);
        it.diagonale = diagonaleJtJ(j);
        bool klein = false;
        while (it.versuche < einst.versucheHoechstens)
        {
            const std::vector<double> delta = lmSchritt(j, e.r, e.lambda);
            std::vector<double> xn(e.x.size());
            for (std::size_t k = 0; k < xn.size(); ++k) xn[k] = e.x[k] + delta[k];
            xn = klemmeIn(raum, std::move(xn));
            double anteil = 0.0;
            for (std::size_t k = 0; k < xn.size(); ++k) anteil = std::max(anteil, std::abs(xn[k] - e.x[k]) / raum.ende[k]);
            it.schrittAnteil = anteil;
            if (anteil < 1.0)
            {
                klein = true;
                break;
            }
            std::vector<double> rn = residuen(xn);
            ++e.auswertungen;
            ++it.versuche;
            ++it.auswertungenResiduen;
            const double restNeu = quadratsumme(rn);
            if (restNeu < e.rest)
            {
                e.x = std::move(xn);
                e.r = std::move(rn);
                e.rest = restNeu;
                e.lambda /= einst.lambdaFaktor;
                it.angenommen = true;
                break;
            }
            ++it.verworfen;
            e.lambda *= einst.lambdaFaktor;
        }
        if (klein) e.ende = "Schritte klein";
        else if (!it.angenommen) e.ende = "kein besserer Schritt";
        it.x = e.x;
        it.r = e.r;
        it.rest = e.rest;
        it.lambda = e.lambda;
        it.anGrenze = anGrenzeIn(raum, e.x);
        if (e.ende.empty() && e.iterationen >= einst.iterationenHoechstens) e.ende = "Iterationen";
        it.ende = e.ende;
        if (beobachter) beobachter(it);
    }
    e.anGrenze = anGrenzeIn(raum, e.x);
    return e;
}

/// Viele Punkte auf einmal ausgewertet (die Toene eines Aufrufs teilen sich einen Pool).
using LmResiduenViele = std::function<std::vector<std::vector<double>>(const std::vector<std::vector<double>>&)>;

/// Der Schritt der finiten Differenz je Parameter: `mass[j]`, nach unten gerichtet, wo der Schritt nach oben die Grenze
/// ueberschritte.
inline std::vector<double> differenzSchritte(const LmRaum& raum, const std::vector<double>& x, const std::vector<double>& mass)
{
    std::vector<double> h(x.size());
    for (std::size_t j = 0; j < x.size(); ++j) h[j] = x[j] + mass[j] <= raum.oben[j] ? mass[j] : -mass[j];
    return h;
}

/// Die Jacobi-Matrix je Gruppe: je Gruppe eine Auswertung mit allen ihren Parametern um h ausgelenkt; die Spalte eines
/// Parameters traegt nur seine Zeilen (`zeilen[j]`), sonst null. Alle Gruppen in einem Aufruf von `viele`.
inline Matrix jacobiJeGruppe(const LmResiduenViele& viele, const std::vector<double>& x, const std::vector<double>& r,
                             const std::vector<std::vector<std::size_t>>& gruppen, const std::vector<double>& h,
                             const std::vector<std::vector<std::size_t>>& zeilen)
{
    std::vector<std::vector<double>> punkte;
    for (const std::vector<std::size_t>& g : gruppen)
    {
        std::vector<double> p = x;
        for (const std::size_t j : g) p[j] += h[j];
        punkte.push_back(std::move(p));
    }
    const std::vector<std::vector<double>> rs = viele(punkte);
    Matrix jm(r.size(), x.size());
    for (std::size_t gi = 0; gi < gruppen.size(); ++gi)
        for (const std::size_t j : gruppen[gi])
            for (const std::size_t i : zeilen[j]) jm(i, j) = (rs[gi][i] - r[i]) / h[j];
    return jm;
}

/// Die Jacobi-Matrix einzeln: je Parameter eine Auswertung, alle Zeilen (zum Vergleich mit `jacobiJeGruppe`).
inline Matrix jacobiEinzeln(const LmResiduenViele& viele, const std::vector<double>& x, const std::vector<double>& r, const std::vector<double>& h)
{
    std::vector<std::vector<double>> punkte;
    for (std::size_t j = 0; j < x.size(); ++j)
    {
        std::vector<double> p = x;
        p[j] += h[j];
        punkte.push_back(std::move(p));
    }
    const std::vector<std::vector<double>> rs = viele(punkte);
    Matrix jm(r.size(), x.size());
    for (std::size_t j = 0; j < x.size(); ++j)
        for (std::size_t i = 0; i < r.size(); ++i) jm(i, j) = (rs[j][i] - r[i]) / h[j];
    return jm;
}

}  // namespace lumi::messung
