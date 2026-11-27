/**
 * @file Ausgleich.cpp
 * @brief Die Rechnung zu `InstrumentCore/Ausgleich.hpp` — wortgleich aus `InstrumentMeasure` (`LevenbergMarquardt.cpp`:
 *        `loese`; `UmlaufEntwurf.cpp`: `Qr`, `kleinsteQuadrate`, `nnls`) umgezogen (34k-7d-4)
 */
#include "InstrumentCore/Ausgleich.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace lumi::kern
{

namespace
{
using Vek = std::vector<double>;
}  // namespace

std::vector<double> loese(Matrix a, std::vector<double> b)
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

Qr::Qr(const Matrix& A) : m(A.zeilen), n(A.spalten), a(A.werte), beta(n, 0.0), v(n)
{
    for (std::size_t k = 0; k < n && k < m; ++k)
    {
        double norm = 0.0;
        for (std::size_t i = k; i < m; ++i) norm += a[i * n + k] * a[i * n + k];
        norm = std::sqrt(norm);
        Vek vk(m - k, 0.0);
        if (norm == 0.0)
        {
            v[k] = vk;
            continue;
        }
        const double alpha = a[k * n + k] > 0 ? -norm : norm;
        for (std::size_t i = k; i < m; ++i) vk[i - k] = a[i * n + k];
        vk[0] -= alpha;
        double vv = 0.0;
        for (double t : vk) vv += t * t;
        beta[k] = vv > 0.0 ? 2.0 / vv : 0.0;
        for (std::size_t j = k; j < n; ++j)
        {
            double s = 0.0;
            for (std::size_t i = k; i < m; ++i) s += vk[i - k] * a[i * n + j];
            s *= beta[k];
            for (std::size_t i = k; i < m; ++i) a[i * n + j] -= s * vk[i - k];
        }
        v[k] = std::move(vk);
    }
}

void Qr::qtMal(Vek& b) const
{
    for (std::size_t k = 0; k < n && k < m; ++k)
    {
        if (beta[k] == 0.0) continue;
        double s = 0.0;
        for (std::size_t i = k; i < m; ++i) s += v[k][i - k] * b[i];
        s *= beta[k];
        for (std::size_t i = k; i < m; ++i) b[i] -= s * v[k][i - k];
    }
}

Vek Qr::rueck(const Vek& c) const
{
    Vek x(n, 0.0);
    for (std::size_t ii = n; ii-- > 0;)
    {
        double s = c[ii];
        for (std::size_t j = ii + 1; j < n; ++j) s -= r(ii, j) * x[j];
        x[ii] = r(ii, ii) != 0.0 ? s / r(ii, ii) : 0.0;
    }
    return x;
}

Vek kleinsteQuadrate(const Matrix& A, Vek b)
{
    const Qr qr(A);
    qr.qtMal(b);
    b.resize(A.spalten);
    return qr.rueck(b);
}

Vek nnls(const Matrix& A, const Vek& b, const Vek* ldpLaenge)
{
    const std::size_t m = A.zeilen, n = A.spalten;
    Vek x(n, 0.0);
    std::vector<bool> passiv(n, false);
    auto rest = [&](const Vek& xx) {
        Vek r = b;
        for (std::size_t i = 0; i < m; ++i)
            for (std::size_t j = 0; j < n; ++j)
                if (xx[j] != 0.0) r[i] -= A(i, j) * xx[j];
        return r;
    };
    auto gradientW = [&](const Vek& r) {
        Vek w(n, 0.0);
        for (std::size_t j = 0; j < n; ++j)
        {
            double s = 0.0;
            for (std::size_t i = 0; i < m; ++i) s += A(i, j) * r[i];
            w[j] = s;
        }
        return w;
    };
    // absolute Schranke wie scipy `nnls` (atol 10 · max(m, n) · ε): eine Schranke mit dem groessten Eintrag von A brach
    // im LDP der Verlustfilter zu frueh ab (A enthaelt R⁻¹; S220 an der Laute |H| = 1,0028)
    const double tol = 10.0 * static_cast<double>(std::max(m, n)) * std::numeric_limits<double>::epsilon();
    auto loesePassiv = [&]() {
        std::vector<std::size_t> p;
        for (std::size_t j = 0; j < n; ++j)
            if (passiv[j]) p.push_back(j);
        Matrix Ap(m, p.size());
        for (std::size_t i = 0; i < m; ++i)
            for (std::size_t q = 0; q < p.size(); ++q) Ap(i, q) = A(i, p[q]);
        Vek z(n, 0.0);
        if (p.empty()) return z;
        Vek zp;
        if (m >= p.size())
            zp = kleinsteQuadrate(Ap, b);
        else
        {
            // unterbestimmt: kleinste Norm ueber AAᵀ
            Matrix aat(m, m);
            for (std::size_t i = 0; i < m; ++i)
                for (std::size_t k = 0; k < m; ++k)
                {
                    double s = 0.0;
                    for (std::size_t q = 0; q < p.size(); ++q) s += Ap(i, q) * Ap(k, q);
                    aat(i, k) = s;
                }
            const Vek y = loese(aat, b);
            zp.assign(p.size(), 0.0);
            for (std::size_t q = 0; q < p.size(); ++q)
                for (std::size_t i = 0; i < m; ++i) zp[q] += Ap(i, q) * y[i];
        }
        for (std::size_t q = 0; q < p.size(); ++q) z[p[q]] = zp[q];
        return z;
    };
    for (std::size_t iter = 0; iter < 30 * n + 100; ++iter)
    {
        const Vek rr = rest(x);
        const Vek w = gradientW(rr);
        std::size_t jmax = n;
        double wmax = 0.0;
        double rn = 0.0, rnorm = 0.0;
        if (ldpLaenge != nullptr)
        {
            rn = std::abs(rr[m - 1]);
            for (double t : rr) rnorm += t * t;
            rnorm = std::sqrt(rnorm);
        }
        for (std::size_t j = 0; j < n; ++j)
        {
            if (passiv[j]) continue;
            const double schwelle = ldpLaenge == nullptr
                                        ? tol
                                        : std::max(64.0 * std::numeric_limits<double>::epsilon() * rnorm, 1e-9 * rn / (*ldpLaenge)[j]);
            if (w[j] > schwelle && w[j] > wmax)
            {
                wmax = w[j];
                jmax = j;
            }
        }
        if (jmax == n) break;
        passiv[jmax] = true;
        for (int inner = 0; inner < static_cast<int>(3 * n); ++inner)
        {
            Vek z = loesePassiv();
            bool alleGut = true;
            for (std::size_t j = 0; j < n; ++j)
                if (passiv[j] && z[j] <= 0.0) alleGut = false;
            if (alleGut)
            {
                x = std::move(z);
                break;
            }
            double alpha = 1.0;
            for (std::size_t j = 0; j < n; ++j)
                if (passiv[j] && z[j] <= 0.0) alpha = std::min(alpha, x[j] / (x[j] - z[j]));
            for (std::size_t j = 0; j < n; ++j)
                if (passiv[j])
                {
                    x[j] += alpha * (z[j] - x[j]);
                    if (std::abs(x[j]) <= tol)
                    {
                        x[j] = 0.0;
                        passiv[j] = false;
                    }
                }
        }
    }
    return x;
}

}  // namespace lumi::kern
