/**
 * @file Balken.cpp
 * @brief InstrumentModels, gemeinsam: Balken aus finiten Elementen (Euler–Bernoulli, Hermite); Herleitung und Herkunft
 *        im Kopf von `Balken.hpp`
 */
#include "InstrumentModels/gemeinsam/Balken.hpp"

#include "InstrumentCore/Eigenzerlegung.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace lumi::kern
{

namespace
{

double& bei(std::vector<double>& a, int n, int z, int s)
{
    return a[static_cast<std::size_t>(z) * static_cast<std::size_t>(n) + static_cast<std::size_t>(s)];
}

double beiC(const std::vector<double>& a, int n, int z, int s)
{
    return a[static_cast<std::size_t>(z) * static_cast<std::size_t>(n) + static_cast<std::size_t>(s)];
}

/// Nullstellen von f in [a, b] (n Stueck, aufsteigend), je durch Halbieren nach einer Vorzeichensuche.
std::vector<double> nullstellen(const std::function<double(double)>& f, double ab, int anzahl)
{
    std::vector<double> x;
    double links = ab;
    double fl = f(links);
    const double schritt = 0.01;
    while (static_cast<int>(x.size()) < anzahl)
    {
        const double rechts = links + schritt;
        const double fr = f(rechts);
        if (fl * fr <= 0.0)
        {
            double a = links, b = rechts, fa = fl;
            for (int i = 0; i < 200; ++i)
            {
                const double m = 0.5 * (a + b);
                const double fm = f(m);
                if (fa * fm <= 0.0)
                    b = m;
                else
                {
                    a = m;
                    fa = fm;
                }
            }
            x.push_back(0.5 * (a + b));
        }
        links = rechts;
        fl = fr;
    }
    return x;
}

}  // namespace

std::vector<BalkenMode> biegeModen(const Balken& b, int anzahl)
{
    const int ne = std::max(2, b.elemente);
    const int nk = ne + 1;
    const int nf = 2 * nk;  // je Knoten Auslenkung, Neigung
    std::vector<double> k(static_cast<std::size_t>(nf) * nf, 0.0);
    std::vector<double> m(static_cast<std::size_t>(nf) * nf, 0.0);
    const double l = b.laengeM / ne;
    for (int e = 0; e < ne; ++e)
    {
        const double x = (e + 0.5) * l;
        const double breite = b.breiteM(x);
        const double hoehe = b.hoeheM(x);
        const double flaeche = breite * hoehe;
        const double traeg = breite * hoehe * hoehe * hoehe / 12.0;
        const double ei = b.eModulPa * traeg;
        const double ra = b.dichteKgM3 * flaeche;
        const double ke[4][4] = {{12, 6 * l, -12, 6 * l}, {6 * l, 4 * l * l, -6 * l, 2 * l * l}, {-12, -6 * l, 12, -6 * l}, {6 * l, 2 * l * l, -6 * l, 4 * l * l}};
        const double me[4][4] = {{156, 22 * l, 54, -13 * l}, {22 * l, 4 * l * l, 13 * l, -3 * l * l}, {54, 13 * l, 156, -22 * l}, {-13 * l, -3 * l * l, -22 * l, 4 * l * l}};
        const double re[4][4] = {{36, 3 * l, -36, 3 * l}, {3 * l, 4 * l * l, -3 * l, -l * l}, {-36, -3 * l, 36, -3 * l}, {3 * l, -l * l, -3 * l, 4 * l * l}};
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
            {
                const int gi = 2 * e + i;
                const int gj = 2 * e + j;
                bei(k, nf, gi, gj) += ei / (l * l * l) * ke[i][j];
                bei(m, nf, gi, gj) += ra * l / 420.0 * me[i][j];
                if (b.drehtraegheit) bei(m, nf, gi, gj) += b.dichteKgM3 * traeg / (30.0 * l) * re[i][j];
            }
    }
    // die Lager: festgehaltene Freiheitsgrade fallen weg
    std::vector<bool> fest(static_cast<std::size_t>(nf), false);
    const auto lagere = [&](Lager lager, int knoten) {
        if (lager == Lager::Gelenkig || lager == Lager::Eingespannt) fest[static_cast<std::size_t>(2 * knoten)] = true;
        if (lager == Lager::Eingespannt) fest[static_cast<std::size_t>(2 * knoten + 1)] = true;
    };
    lagere(b.links, 0);
    lagere(b.rechts, nk - 1);
    std::vector<int> frei;
    for (int i = 0; i < nf; ++i)
        if (!fest[static_cast<std::size_t>(i)]) frei.push_back(i);
    const int n = static_cast<int>(frei.size());
    std::vector<double> kr(static_cast<std::size_t>(n) * n), mr(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
        {
            bei(kr, n, i, j) = beiC(k, nf, frei[static_cast<std::size_t>(i)], frei[static_cast<std::size_t>(j)]);
            bei(mr, n, i, j) = beiC(m, nf, frei[static_cast<std::size_t>(i)], frei[static_cast<std::size_t>(j)]);
        }
    // Cholesky M = L·Lᵀ
    std::vector<double> lc(static_cast<std::size_t>(n) * n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j <= i; ++j)
        {
            double s = beiC(mr, n, i, j);
            for (int q = 0; q < j; ++q) s -= beiC(lc, n, i, q) * beiC(lc, n, j, q);
            bei(lc, n, i, j) = i == j ? std::sqrt(std::max(s, 1e-300)) : s / beiC(lc, n, j, j);
        }
    // A = L⁻¹·K·L⁻ᵀ: erst Y = L⁻¹·K (vorwaerts je Spalte), dann A = Y·L⁻ᵀ = (L⁻¹·Yᵀ)ᵀ
    const auto vorwaerts = [&](std::vector<double>& x) {
        for (int i = 0; i < n; ++i)
        {
            double s = x[static_cast<std::size_t>(i)];
            for (int q = 0; q < i; ++q) s -= beiC(lc, n, i, q) * x[static_cast<std::size_t>(q)];
            x[static_cast<std::size_t>(i)] = s / beiC(lc, n, i, i);
        }
    };
    std::vector<double> y(static_cast<std::size_t>(n) * n);
    std::vector<double> spalte(static_cast<std::size_t>(n));
    for (int j = 0; j < n; ++j)
    {
        for (int i = 0; i < n; ++i) spalte[static_cast<std::size_t>(i)] = beiC(kr, n, i, j);
        vorwaerts(spalte);
        for (int i = 0; i < n; ++i) bei(y, n, i, j) = spalte[static_cast<std::size_t>(i)];
    }
    std::vector<double> a(static_cast<std::size_t>(n) * n);
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j) spalte[static_cast<std::size_t>(j)] = beiC(y, n, i, j);
        vorwaerts(spalte);
        for (int j = 0; j < n; ++j) bei(a, n, i, j) = spalte[static_cast<std::size_t>(j)];
    }
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) bei(a, n, i, j) = bei(a, n, j, i) = 0.5 * (beiC(a, n, i, j) + beiC(a, n, j, i));
    const SymmetrischeZerlegung z = zerlegeSymmetrisch(n, a);
    // die Form φ = L⁻ᵀ·v (rueckwaerts), M-normiert, weil v euklidisch normiert ist
    std::vector<BalkenMode> moden;
    // Starrkoerpermoden: frei–frei zwei (Verschiebung, Drehung), frei–gelenkig eine (Drehung um das Lager)
    const int lagerFest = (b.links == Lager::Frei ? 0 : 1) + (b.rechts == Lager::Frei ? 0 : 1);
    const bool eingespannt = b.links == Lager::Eingespannt || b.rechts == Lager::Eingespannt;
    const int starr = eingespannt ? 0 : std::max(0, 2 - lagerFest);
    for (int j = starr; j < n && static_cast<int>(moden.size()) < anzahl; ++j)
    {
        const double omega = std::sqrt(std::max(0.0, z.werte[static_cast<std::size_t>(j)]));
        std::vector<double> phi(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) phi[static_cast<std::size_t>(i)] = beiC(z.vektoren, n, i, j);
        for (int i = n - 1; i >= 0; --i)
        {
            double s = phi[static_cast<std::size_t>(i)];
            for (int q = i + 1; q < n; ++q) s -= beiC(lc, n, q, i) * phi[static_cast<std::size_t>(q)];
            phi[static_cast<std::size_t>(i)] = s / beiC(lc, n, i, i);
        }
        BalkenMode mode;
        mode.hz = omega / (2.0 * std::numbers::pi);
        mode.auslenkung.assign(static_cast<std::size_t>(nk), 0.0);
        for (int i = 0; i < n; ++i)
        {
            const int g = frei[static_cast<std::size_t>(i)];
            if (g % 2 == 0) mode.auslenkung[static_cast<std::size_t>(g / 2)] = phi[static_cast<std::size_t>(i)];
        }
        // Vorzeichen fest: die groesste Auslenkung positiv (deterministisch)
        const auto groesste = std::max_element(mode.auslenkung.begin(), mode.auslenkung.end(),
                                               [](double p, double q) { return std::abs(p) < std::abs(q); });
        if (*groesste < 0.0)
            for (double& w : mode.auslenkung) w = -w;
        moden.push_back(std::move(mode));
    }
    return moden;
}

double formAn(const BalkenMode& mode, const Balken& balken, double x)
{
    const auto nk = static_cast<int>(mode.auslenkung.size());
    if (nk < 2) return 0.0;
    const double t = std::clamp(x / balken.laengeM, 0.0, 1.0) * (nk - 1);
    const int i = std::min(nk - 2, static_cast<int>(t));
    const double r = t - i;
    return (1.0 - r) * mode.auslenkung[static_cast<std::size_t>(i)] + r * mode.auslenkung[static_cast<std::size_t>(i + 1)];
}

std::vector<double> freiFreiVerhaeltnisse(int anzahl)
{
    const std::vector<double> bl = nullstellen([](double x) { return std::cos(x) * std::cosh(x) - 1.0; }, 1.0, anzahl);
    std::vector<double> v;
    for (const double x : bl) v.push_back(x * x / (bl[0] * bl[0]));
    return v;
}

std::vector<double> kragBetaL(int anzahl)
{
    return nullstellen([](double x) { return std::cos(x) * std::cosh(x) + 1.0; }, 0.5, anzahl);
}

}  // namespace lumi::kern
