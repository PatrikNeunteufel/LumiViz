/**
 * @file Eigenzerlegung.cpp
 * @brief Loop: Eigenwerte und Eigenvektoren kleiner komplexer Matrizen (Stufe 31d-7, P6 a)
 */
#include "loop/Eigenzerlegung.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumi::loop
{

namespace
{

using Komplex = std::complex<double>;

constexpr double kEps = std::numeric_limits<double>::epsilon();
/// So viele QR-Schritte je Eigenwert, dann gilt die Iteration als nicht zusammengelaufen.
constexpr int kSchritteJeWert = 40;

double normF(const KleineMatrix& a)
{
    double summe = 0.0;
    for (const Komplex& w : a.werte) summe += std::norm(w);
    return std::sqrt(summe);
}

KleineMatrix einheit(int n)
{
    KleineMatrix e(n);
    for (int i = 0; i < n; ++i) e(i, i) = 1.0;
    return e;
}

bool istDiagonal(const KleineMatrix& a)
{
    for (int z = 0; z < a.n; ++z)
        for (int s = 0; s < a.n; ++s)
            if (z != s && a(z, s) != Komplex(0.0, 0.0)) return false;
    return true;
}

/// Householder: `h` in Hessenberg-Form, `q` sammelt die Spiegelungen (A = Q·H·Qᴴ).
void hessenberg(KleineMatrix& h, KleineMatrix& q)
{
    const int n = h.n;
    std::vector<Komplex> v(static_cast<std::size_t>(n));
    for (int k = 0; k + 2 < n; ++k)
    {
        double laenge = 0.0;
        for (int i = k + 1; i < n; ++i) laenge += std::norm(h(i, k));
        laenge = std::sqrt(laenge);
        if (laenge == 0.0) continue;
        const Komplex x0 = h(k + 1, k);
        const Komplex phase = std::abs(x0) > 0.0 ? x0 / std::abs(x0) : Komplex(1.0, 0.0);
        const Komplex alpha = -phase * laenge;
        double vNorm = 0.0;
        for (int i = k + 1; i < n; ++i)
        {
            v[static_cast<std::size_t>(i)] = h(i, k) - (i == k + 1 ? alpha : Komplex(0.0, 0.0));
            vNorm += std::norm(v[static_cast<std::size_t>(i)]);
        }
        vNorm = std::sqrt(vNorm);
        if (vNorm == 0.0) continue;
        for (int i = k + 1; i < n; ++i) v[static_cast<std::size_t>(i)] /= vNorm;
        // H ← (I − 2vvᴴ)·H
        for (int s = 0; s < n; ++s)
        {
            Komplex summe(0.0, 0.0);
            for (int i = k + 1; i < n; ++i) summe += std::conj(v[static_cast<std::size_t>(i)]) * h(i, s);
            for (int i = k + 1; i < n; ++i) h(i, s) -= 2.0 * v[static_cast<std::size_t>(i)] * summe;
        }
        // H ← H·(I − 2vvᴴ), Q ← Q·(I − 2vvᴴ)
        for (KleineMatrix* m : {&h, &q})
            for (int z = 0; z < n; ++z)
            {
                Komplex summe(0.0, 0.0);
                for (int i = k + 1; i < n; ++i) summe += (*m)(z, i) * v[static_cast<std::size_t>(i)];
                for (int i = k + 1; i < n; ++i) (*m)(z, i) -= 2.0 * summe * std::conj(v[static_cast<std::size_t>(i)]);
            }
        h(k + 1, k) = alpha;
        for (int i = k + 2; i < n; ++i) h(i, k) = 0.0;
    }
}

/// Eine Drehung G = [[c, s], [−s̄, c]] mit G·[f, g]ᵀ = [r, 0]ᵀ.
struct Drehung
{
    double c = 1.0;
    Komplex s{0.0, 0.0};
};

Drehung drehung(Komplex f, Komplex g)
{
    const double betragF = std::abs(f);
    const double betragG = std::abs(g);
    if (betragG == 0.0) return {};
    if (betragF == 0.0) return {0.0, std::conj(g) / betragG};
    const double r = std::hypot(betragF, betragG);
    return {betragF / r, (f / betragF) * std::conj(g) / r};
}

/// Der Wilkinson-Wert des unteren 2 × 2-Blocks: der Eigenwert naeher an d.
Komplex wilkinson(Komplex a, Komplex b, Komplex c, Komplex d)
{
    const Komplex mitte = 0.5 * (a + d);
    const Komplex wurzel = std::sqrt(0.25 * (a - d) * (a - d) + b * c);
    const Komplex eins = mitte + wurzel;
    const Komplex zwei = mitte - wurzel;
    return std::abs(eins - d) < std::abs(zwei - d) ? eins : zwei;
}

/// QR mit Verschiebung auf der Hessenberg-Form bis zur Schur-Form T (oberes Dreieck); `q` sammelt mit.
bool schur(KleineMatrix& t, KleineMatrix& q)
{
    const int n = t.n;
    const double groesse = std::max(normF(t), std::numeric_limits<double>::min());
    int oben = n - 1;
    int schritte = 0;
    std::vector<Drehung> drehungen(static_cast<std::size_t>(n));
    while (oben > 0)
    {
        int unten = oben;
        while (unten > 0)
        {
            const double nachbar = std::abs(t(unten - 1, unten - 1)) + std::abs(t(unten, unten));
            if (std::abs(t(unten, unten - 1)) <= kEps * (nachbar > 0.0 ? nachbar : groesse)) break;
            --unten;
        }
        if (unten == oben)
        {
            t(oben, oben - 1) = 0.0;
            --oben;
            schritte = 0;
            continue;
        }
        if (unten > 0) t(unten, unten - 1) = 0.0;
        if (++schritte > kSchritteJeWert) return false;
        Komplex mu = wilkinson(t(oben - 1, oben - 1), t(oben - 1, oben), t(oben, oben - 1), t(oben, oben));
        // hin und wieder ein anderer Wert, falls die Iteration kreist
        if (schritte % 10 == 0) mu = t(oben, oben) + std::abs(t(oben, oben - 1));
        for (int i = unten; i <= oben; ++i) t(i, i) -= mu;
        for (int k = unten; k < oben; ++k)
        {
            const Drehung g = drehung(t(k, k), t(k + 1, k));
            drehungen[static_cast<std::size_t>(k)] = g;
            for (int s = k; s < n; ++s)
            {
                const Komplex a = t(k, s);
                const Komplex b = t(k + 1, s);
                t(k, s) = g.c * a + g.s * b;
                t(k + 1, s) = -std::conj(g.s) * a + g.c * b;
            }
        }
        for (int k = unten; k < oben; ++k)
        {
            const Drehung& g = drehungen[static_cast<std::size_t>(k)];
            const int bis = std::min(k + 1, oben);
            for (int z = 0; z <= bis; ++z)
            {
                const Komplex a = t(z, k);
                const Komplex b = t(z, k + 1);
                t(z, k) = a * g.c + b * std::conj(g.s);
                t(z, k + 1) = -a * g.s + b * g.c;
            }
            for (int z = 0; z < n; ++z)
            {
                const Komplex a = q(z, k);
                const Komplex b = q(z, k + 1);
                q(z, k) = a * g.c + b * std::conj(g.s);
                q(z, k + 1) = -a * g.s + b * g.c;
            }
        }
        for (int i = unten; i <= oben; ++i) t(i, i) += mu;
    }
    return true;
}

/// Loest M·x = b mit Pivotsuche; ein Pivot unter `klein` wird zu `klein` (inverse Iteration).
std::vector<Komplex> loese(KleineMatrix m, std::vector<Komplex> b, double klein)
{
    const int n = m.n;
    for (int k = 0; k < n; ++k)
    {
        int pivot = k;
        for (int i = k + 1; i < n; ++i)
            if (std::abs(m(i, k)) > std::abs(m(pivot, k))) pivot = i;
        if (pivot != k)
        {
            for (int s = 0; s < n; ++s) std::swap(m(k, s), m(pivot, s));
            std::swap(b[static_cast<std::size_t>(k)], b[static_cast<std::size_t>(pivot)]);
        }
        if (std::abs(m(k, k)) < klein) m(k, k) = klein;
        for (int i = k + 1; i < n; ++i)
        {
            const Komplex faktor = m(i, k) / m(k, k);
            for (int s = k; s < n; ++s) m(i, s) -= faktor * m(k, s);
            b[static_cast<std::size_t>(i)] -= faktor * b[static_cast<std::size_t>(k)];
        }
    }
    std::vector<Komplex> x(static_cast<std::size_t>(n));
    for (int i = n - 1; i >= 0; --i)
    {
        Komplex summe = b[static_cast<std::size_t>(i)];
        for (int s = i + 1; s < n; ++s) summe -= m(i, s) * x[static_cast<std::size_t>(s)];
        x[static_cast<std::size_t>(i)] = summe / m(i, i);
    }
    return x;
}

void aufEins(std::vector<Komplex>& v)
{
    double laenge = 0.0;
    for (const Komplex& w : v) laenge += std::norm(w);
    laenge = std::sqrt(laenge);
    if (laenge > 0.0)
        for (Komplex& w : v) w /= laenge;
}

/// Die Inverse mit Pivotsuche; falsch, wenn `m` nicht umkehrbar ist.
bool invertiere(const KleineMatrix& m, KleineMatrix& inverse, double groesse)
{
    const int n = m.n;
    KleineMatrix a = m;
    inverse = einheit(n);
    for (int k = 0; k < n; ++k)
    {
        int pivot = k;
        for (int i = k + 1; i < n; ++i)
            if (std::abs(a(i, k)) > std::abs(a(pivot, k))) pivot = i;
        if (std::abs(a(pivot, k)) <= kEps * groesse) return false;
        if (pivot != k)
            for (int s = 0; s < n; ++s)
            {
                std::swap(a(k, s), a(pivot, s));
                std::swap(inverse(k, s), inverse(pivot, s));
            }
        const Komplex kehr = 1.0 / a(k, k);
        for (int s = 0; s < n; ++s)
        {
            a(k, s) *= kehr;
            inverse(k, s) *= kehr;
        }
        for (int i = 0; i < n; ++i)
        {
            if (i == k) continue;
            const Komplex faktor = a(i, k);
            if (faktor == Komplex(0.0, 0.0)) continue;
            for (int s = 0; s < n; ++s)
            {
                a(i, s) -= faktor * a(k, s);
                inverse(i, s) -= faktor * inverse(k, s);
            }
        }
    }
    return true;
}

}  // namespace

Eigenzerlegung zerlegeKlein(const KleineMatrix& a)
{
    Eigenzerlegung z;
    const int n = std::clamp(a.n, 0, kKleineMatrixMax);
    if (n == 0 || a.werte.size() < static_cast<std::size_t>(n) * static_cast<std::size_t>(n)) return z;
    if (istDiagonal(a))
    {
        for (int i = 0; i < n; ++i) z.werte.push_back(a(i, i));
        z.rechts = einheit(n);
        z.links = einheit(n);
        return z;
    }
    const double groesse = std::max(normF(a), std::numeric_limits<double>::min());
    KleineMatrix t = a;
    KleineMatrix q = einheit(n);
    hessenberg(t, q);
    z.vollstaendig = schur(t, q);
    for (int i = 0; i < n; ++i) z.werte.push_back(t(i, i));

    // die Eigenvektoren des Dreiecks, rueckwaerts eingesetzt, dann zurueckgedreht
    const double klein = kEps * groesse;
    z.rechts = KleineMatrix(n);
    for (int m = 0; m < n; ++m)
    {
        std::vector<Komplex> y(static_cast<std::size_t>(n), Komplex(0.0, 0.0));
        y[static_cast<std::size_t>(m)] = 1.0;
        for (int i = m - 1; i >= 0; --i)
        {
            Komplex summe(0.0, 0.0);
            for (int j = i + 1; j <= m; ++j) summe += t(i, j) * y[static_cast<std::size_t>(j)];
            Komplex nenner = t(i, i) - t(m, m);
            if (std::abs(nenner) < klein) nenner = klein;
            y[static_cast<std::size_t>(i)] = -summe / nenner;
        }
        std::vector<Komplex> v(static_cast<std::size_t>(n), Komplex(0.0, 0.0));
        for (int r = 0; r < n; ++r)
            for (int j = 0; j <= m; ++j) v[static_cast<std::size_t>(r)] += q(r, j) * y[static_cast<std::size_t>(j)];
        aufEins(v);
        // ein isolierter Eigenwert: ein Schritt inverser Iteration schaerft den Vektor nach
        double abstand = std::numeric_limits<double>::infinity();
        for (int j = 0; j < n; ++j)
            if (j != m) abstand = std::min(abstand, std::abs(z.werte[static_cast<std::size_t>(j)] - z.werte[static_cast<std::size_t>(m)]));
        if (abstand > 1e-8 * groesse)
        {
            KleineMatrix verschoben = a;
            for (int i = 0; i < n; ++i) verschoben(i, i) -= z.werte[static_cast<std::size_t>(m)];
            std::vector<Komplex> besser = loese(verschoben, v, klein);
            aufEins(besser);
            bool endlich = true;
            for (const Komplex& w : besser) endlich = endlich && std::isfinite(w.real()) && std::isfinite(w.imag());
            if (endlich) v = besser;
        }
        for (int r = 0; r < n; ++r) z.rechts(r, m) = v[static_cast<std::size_t>(r)];
    }
    if (!invertiere(z.rechts, z.links, 1.0)) z.vollstaendig = false;
    return z;
}

}  // namespace lumi::loop
