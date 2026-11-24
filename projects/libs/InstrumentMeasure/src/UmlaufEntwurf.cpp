/**
 * @file UmlaufEntwurf.cpp
 * @brief Der Entwurf des Umlauf-Filters (UL3 a, UL3-1 a, ZK1 a; 34k-7c) — siehe Kopf
 */
#include "InstrumentMeasure/UmlaufEntwurf.hpp"

#include "InstrumentCore/Eigenzerlegung.hpp"
#include "InstrumentCore/Fft.hpp"
#include "InstrumentCore/Pool.hpp"
#include "InstrumentMeasure/LevenbergMarquardt.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <functional>
#include <numbers>
#include <numeric>
#include <stdexcept>

namespace lumi::messung::umlauf
{

namespace
{
namespace ku = kern::umlauf;
using cplx = std::complex<double>;
using Vek = std::vector<double>;
constexpr double kPi = std::numbers::pi;
constexpr double kKf = 1200.0 / std::numbers::ln2;
constexpr std::size_t kRaster = 39999;     // 1 … 20 000 Hz in 0,5 Hz
constexpr std::size_t kNfft = 1u << 16;    // Cepstrum der Vorgabe (bank_voll.py NFFT)
constexpr std::size_t kNebenbedingungen = 4097;  // weg2.py WG
constexpr std::size_t kPruefraster = (1u << 16) + 1;  // bank_voll.py WD

// ------------------------------------------------------------------ Raster
const Vek& rasterF()
{
    static const Vek f = [] {
        Vek v(kRaster);
        for (std::size_t i = 0; i < kRaster; ++i) v[i] = 1.0 + 0.5 * static_cast<double>(i);
        return v;
    }();
    return f;
}

double omegaVon(double hz) { return 2.0 * kPi * hz / kRate; }

// np.interp: xp aufsteigend (nicht fallend), ausserhalb der Randwert
double interp(double x, const Vek& xp, const Vek& fp)
{
    if (x <= xp.front()) return fp.front();
    if (x >= xp.back()) return fp.back();
    const auto it = std::upper_bound(xp.begin(), xp.end(), x);
    const std::size_t j = static_cast<std::size_t>(it - xp.begin()) - 1;
    const double d = xp[j + 1] - xp[j];
    return d == 0.0 ? fp[j + 1] : fp[j] + (x - xp[j]) * (fp[j + 1] - fp[j]) / d;
}

// auf dem gleichmaessigen Raster F (schneller als die Suche)
double interpF(double hz, const Vek& fp)
{
    const double t = (hz - 1.0) / 0.5;
    if (t <= 0.0) return fp.front();
    if (t >= static_cast<double>(kRaster - 1)) return fp.back();
    const auto j = static_cast<std::size_t>(t);
    const double u = t - static_cast<double>(j);
    return fp[j] + u * (fp[j + 1] - fp[j]);
}

// np.gradient bei gleichem Abstand h
Vek gradient(const Vek& y, double h)
{
    const std::size_t n = y.size();
    Vek g(n, 0.0);
    if (n < 2) return g;
    g[0] = (y[1] - y[0]) / h;
    g[n - 1] = (y[n - 1] - y[n - 2]) / h;
    for (std::size_t i = 1; i + 1 < n; ++i) g[i] = (y[i + 1] - y[i - 1]) / (2.0 * h);
    return g;
}

// −unwrap(angle(h))
Vek verzugVon(const std::vector<cplx>& h)
{
    Vek p(h.size());
    double off = 0.0;
    double vor = 0.0;
    for (std::size_t i = 0; i < h.size(); ++i)
    {
        const double a = std::arg(h[i]);
        if (i > 0)
        {
            const double d = a - vor;
            if (d > kPi) off -= 2.0 * kPi * std::floor((d + kPi) / (2.0 * kPi));
            else if (d < -kPi) off += 2.0 * kPi * std::floor((-d + kPi) / (2.0 * kPi));
        }
        vor = a;
        p[i] = -(a + off);
    }
    return p;
}

/// Jenseits dieser Grenze ist sigmoid konstant (wie np.clip(…, −40, 40) der Vorlage); die Logit-Parameter der Kette werden
/// in der Anpassung darauf gehalten (TK2 a).
constexpr double kLogitGrenze = 40.0;
double sigmoid(double x) { return 1.0 / (1.0 + std::exp(-std::clamp(x, -kLogitGrenze, kLogitGrenze))); }

// ------------------------------------------------------------------ pchip (scipy PchipInterpolator)
struct Pchip
{
    Vek x, y, d;
    Pchip(Vek xs, Vek ys) : x(std::move(xs)), y(std::move(ys)), d(x.size(), 0.0)
    {
        const std::size_t n = x.size();
        if (n < 2) return;
        Vek h(n - 1), m(n - 1);
        for (std::size_t k = 0; k + 1 < n; ++k)
        {
            h[k] = x[k + 1] - x[k];
            m[k] = (y[k + 1] - y[k]) / h[k];
        }
        if (n == 2)
        {
            d[0] = d[1] = m[0];
            return;
        }
        for (std::size_t k = 1; k + 1 < n; ++k)
        {
            if (m[k - 1] * m[k] <= 0.0) continue;
            const double w1 = 2.0 * h[k] + h[k - 1], w2 = h[k] + 2.0 * h[k - 1];
            d[k] = (w1 + w2) / (w1 / m[k - 1] + w2 / m[k]);
        }
        auto rand = [](double h0, double h1, double m0, double m1) {
            double dd = ((2.0 * h0 + h1) * m0 - h0 * m1) / (h0 + h1);
            const auto sg = [](double v) { return (v > 0.0) - (v < 0.0); };
            if (sg(dd) != sg(m0)) dd = 0.0;
            else if (sg(m0) != sg(m1) && std::abs(dd) > 3.0 * std::abs(m0)) dd = 3.0 * m0;
            return dd;
        };
        d[0] = rand(h[0], h[1], m[0], m[1]);
        d[n - 1] = rand(h[n - 2], h[n - 3], m[n - 2], m[n - 3]);
    }
    double operator()(double t) const
    {
        const auto it = std::upper_bound(x.begin(), x.end(), t);
        std::size_t k = it == x.begin() ? 0 : static_cast<std::size_t>(it - x.begin()) - 1;
        k = std::min(k, x.size() - 2);
        const double h = x[k + 1] - x[k], s = (t - x[k]) / h;
        const double h00 = (1 + 2 * s) * (1 - s) * (1 - s), h10 = s * (1 - s) * (1 - s), h01 = s * s * (3 - 2 * s),
                     h11 = s * s * (s - 1);
        return h00 * y[k] + h10 * h * d[k] + h01 * y[k + 1] + h11 * h * d[k + 1];
    }
};

// ------------------------------------------------------------------ Lineare Algebra (klein)
// Householder-QR einer Matrix (zeilen × spalten, zeilen ≥ spalten): R oben, Qᵀ angewandt auf die rechten Seiten.
struct Qr
{
    std::size_t m = 0, n = 0;
    Vek a;     // zeilenweise, nach der Zerlegung R im oberen Dreieck
    Vek beta;  // je Spiegelung
    std::vector<Vek> v;
    Qr(const Matrix& A) : m(A.zeilen), n(A.spalten), a(A.werte), beta(n, 0.0), v(n)
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
    void qtMal(Vek& b) const
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
    [[nodiscard]] double r(std::size_t i, std::size_t j) const { return a[i * n + j]; }
    // R x = c (oberes Dreieck, n × n); ein Pivot von null gibt 0
    [[nodiscard]] Vek rueck(const Vek& c) const
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
};

// min ||A x − b|| (zeilen ≥ spalten)
Vek kleinsteQuadrate(const Matrix& A, Vek b)
{
    const Qr qr(A);
    qr.qtMal(b);
    b.resize(A.spalten);
    return qr.rueck(b);
}

// NNLS nach Lawson & Hanson 1974 (Kap. 23): min ||A x − b||, x ≥ 0. `ldpLaenge`: im LDP der LSI die Laenge, auf die jede
// Spalte normiert wurde — dann gilt eine Spalte als fertig, wenn die Verletzung ihrer Nebenbedingung (w_j · Laenge / |r_n|,
// in den Einheiten der Nebenbedingung) unter 10⁻⁹ liegt, nicht erst w_j unter einer festen Schranke (S220)
Vek nnls(const Matrix& A, const Vek& b, const Vek* ldpLaenge = nullptr)
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

// die Wurzeln eines Polynoms, c[0] zur hoechsten Potenz (np.roots; fuehrende Nullen gestrichen)
std::vector<cplx> wurzeln(Vek c)
{
    while (!c.empty() && c.front() == 0.0) c.erase(c.begin());
    std::size_t nullen = 0;
    while (!c.empty() && c.back() == 0.0)
    {
        c.pop_back();
        ++nullen;
    }
    std::vector<cplx> w(nullen, 0.0);
    const int n = static_cast<int>(c.size()) - 1;
    if (n < 1) return w;
    if (n > kern::kKleineMatrixMax) throw std::runtime_error("polynomial degree above the limit of the eigen solver");
    kern::KleineMatrix a(n);
    for (int j = 0; j < n; ++j) a(0, j) = -c[static_cast<std::size_t>(j) + 1] / c[0];
    for (int i = 1; i < n; ++i) a(i, i - 1) = 1.0;
    const kern::Eigenzerlegung z = kern::zerlegeKlein(a);
    w.insert(w.end(), z.werte.begin(), z.werte.end());
    return w;
}

// np.poly: Beiwerte aus den Wurzeln (fuehrend 1), reell genommen
Vek polyAus(const std::vector<cplx>& w)
{
    std::vector<cplx> p{1.0};
    for (const cplx& r : w)
    {
        std::vector<cplx> q(p.size() + 1, 0.0);
        for (std::size_t k = 0; k < p.size(); ++k)
        {
            q[k] += p[k];
            q[k + 1] -= r * p[k];
        }
        p = std::move(q);
    }
    Vek out(p.size());
    for (std::size_t k = 0; k < p.size(); ++k) out[k] = p[k].real();
    return out;
}

// groesster Betrag auf 0 … π (bank_voll.py maxbetrag)
double maxBetrag(const Vek& b, const Vek& a)
{
    double best = 0.0;
    std::size_t ib = 0;
    const double schritt = kPi / static_cast<double>(kPruefraster - 1);
    for (std::size_t i = 0; i < kPruefraster; ++i)
    {
        const double h = std::abs(ku::antwort(b, a, schritt * static_cast<double>(i)));
        if (h > best)
        {
            best = h;
            ib = i;
        }
    }
    // goldener Schnitt zwischen den Nachbarn
    double lo = schritt * static_cast<double>(ib == 0 ? 0 : ib - 1), hi = schritt * static_cast<double>(std::min(ib + 1, kPruefraster - 1));
    const double gr = (std::sqrt(5.0) - 1.0) / 2.0;
    double c = hi - gr * (hi - lo), d = lo + gr * (hi - lo);
    double fc = std::abs(ku::antwort(b, a, c)), fd = std::abs(ku::antwort(b, a, d));
    for (int i = 0; i < 100 && hi - lo > 1e-12; ++i)
    {
        if (fc > fd)
        {
            hi = d;
            d = c;
            fd = fc;
            c = hi - gr * (hi - lo);
            fc = std::abs(ku::antwort(b, a, c));
        }
        else
        {
            lo = c;
            c = d;
            fc = fd;
            d = lo + gr * (hi - lo);
            fd = std::abs(ku::antwort(b, a, d));
        }
    }
    return std::max({best, fc, fd});
}

// ------------------------------------------------------------------ Ziel und Teiltoene
struct Ziel
{
    Vek betrag, verzug;
};

Ziel zielVon(const Saitenwerte& s, double laenge)
{
    const Vek& F = rasterF();
    Ziel z{Vek(kRaster), Vek(kRaster)};
    for (std::size_t i = 0; i < kRaster; ++i)
    {
        const ku::Umlaufwert u = ku::umlaufDerSaite(s, laenge, F[i]);
        z.betrag[i] = u.betrag;
        z.verzug[i] = u.verzug;
    }
    return z;
}

struct Teiltoene
{
    Vek hz, sigma;
};

Teiltoene teiltoeneVon(const Vek& betrag, const Vek& verzug)
{
    const Vek& F = rasterF();
    Teiltoene t;
    const int n = static_cast<int>(std::floor(verzug.back() / (2.0 * kPi)));
    const Vek tau = gradient(verzug, 2.0 * kPi * 0.5);
    for (int k = 1; k <= n; ++k)
    {
        const double f = interp(2.0 * kPi * k, verzug, F);
        t.hz.push_back(f);
        t.sigma.push_back(-std::log(interpF(f, betrag)) / interpF(f, tau));
    }
    return t;
}

// ------------------------------------------------------------------ Verlust (UL3 a)
struct Vorgabe
{
    Vek fk, g;
    std::vector<cplx> hd;
};

Vorgabe vorgabeVon(const Vek& betrag, const Teiltoene& tt)
{
    const Vek& F = rasterF();
    Vorgabe v;
    for (double f : tt.hz)
        if (f < 0.45 * kRate) v.fk.push_back(f);
    const double f0 = tt.hz.front();
    for (double f : v.fk)
    {
        double g = std::min(interp(f, F, betrag), 1.0 - 1e-12);
        const double tau = -1.0 / (f0 * std::log(g));  // ohne Glaettung: g bleibt (bis auf Rundung)
        g = std::exp(-1.0 / (f0 * tau));
        v.g.push_back(g);
    }
    // Betrag auf 0 … fs/2 (NFFT/2 + 1 Stellen), Mindestphase ueber das Cepstrum
    const std::size_t nh = kNfft / 2 + 1;
    std::vector<cplx> spek(nh);
    Vek gu(nh);
    std::optional<Pchip> ip;
    if (v.fk.size() >= 2) ip.emplace(v.fk, v.g);
    for (std::size_t i = 0; i < nh; ++i)
    {
        const double fu = static_cast<double>(i) * (kRate / 2.0) / static_cast<double>(nh - 1);
        double w = fu < v.fk.front() ? v.g.front() : (fu > v.fk.back() ? v.g.back() : (ip ? (*ip)(fu) : v.g.front()));
        gu[i] = std::max(w, 1e-6);
        spek[i] = std::log(gu[i]);
    }
    const kern::Fft& fft = kern::fft();
    Vek c(kNfft);
    fft.rueckwaerts(spek, c);
    for (std::size_t i = 1; i < kNfft / 2; ++i) c[i] *= 2.0;
    for (std::size_t i = kNfft / 2 + 1; i < kNfft; ++i) c[i] = 0.0;
    fft.vorwaerts(c, spek);
    // Phase: unwrap(angle(exp(X))) = Im X, abgewickelt
    std::vector<cplx> e(nh);
    for (std::size_t i = 0; i < nh; ++i) e[i] = std::exp(spek[i]);
    Vek ph = verzugVon(e);
    for (double& p : ph) p = -p;
    Vek fu(nh);
    for (std::size_t i = 0; i < nh; ++i) fu[i] = static_cast<double>(i) * (kRate / 2.0) / static_cast<double>(nh - 1);
    for (std::size_t k = 0; k < v.fk.size(); ++k) v.hd.push_back(v.g[k] * std::polar(1.0, interp(v.fk[k], fu, ph)));
    return v;
}

Vek gewichtRel(const Vek& g)
{
    Vek w(g.size());
    double mx = 0.0;
    for (std::size_t k = 0; k < g.size(); ++k)
    {
        const double l = g[k] * std::log(g[k]);
        w[k] = 1.0 / (l * l);
        mx = std::max(mx, w[k]);
    }
    for (double& x : w) x = std::sqrt(x / mx);
    return w;
}

void nenner(const Vorgabe& v, int N, Vek& b, Vek& a)
{
    const Vek sw = gewichtRel(v.g);
    const std::size_t K = v.fk.size(), n = static_cast<std::size_t>(2 * N + 1);
    Matrix M(2 * K, n);
    Vek rr(2 * K);
    for (std::size_t k = 0; k < K; ++k)
    {
        const double x = omegaVon(v.fk[k]);
        for (int j = 0; j <= N; ++j)
        {
            const cplx e = std::polar(sw[k], -x * j);
            M(k, static_cast<std::size_t>(j)) = e.real();
            M(K + k, static_cast<std::size_t>(j)) = e.imag();
            if (j >= 1)
            {
                const cplx h = -v.hd[k] * e;
                M(k, static_cast<std::size_t>(N + j)) = h.real();
                M(K + k, static_cast<std::size_t>(N + j)) = h.imag();
            }
        }
        rr[k] = (v.hd[k] * sw[k]).real();
        rr[K + k] = (v.hd[k] * sw[k]).imag();
    }
    const Vek p = kleinsteQuadrate(M, rr);
    b.assign(p.begin(), p.begin() + N + 1);
    a.assign(1, 1.0);
    a.insert(a.end(), p.begin() + N + 1, p.end());
    std::vector<cplx> r = wurzeln(a);
    bool aus = false;
    double teil = 1.0;
    for (cplx& z : r)
        if (std::abs(z) >= 1.0)
        {
            aus = true;
            teil *= std::abs(z);
            z = 1.0 / std::conj(z);
        }
    if (aus)
    {
        for (double& x : b) x /= teil;
        a = polyAus(r);
    }
}

// LSI → LDP → NNLS (weg2.py lsi): min ||E x − f|| mit G x ≥ h
Vek lsi(const Matrix& E, const Vek& f, const Matrix& G, const Vek& h)
{
    const std::size_t n = E.spalten;
    const Qr qr(E);
    Vek f1 = f;
    qr.qtMal(f1);
    f1.resize(n);
    // Ri = R⁻¹ spaltenweise
    Matrix Ri(n, n);
    for (std::size_t j = 0; j < n; ++j)
    {
        Vek e(n, 0.0);
        e[j] = 1.0;
        const Vek s = qr.rueck(e);
        for (std::size_t i = 0; i < n; ++i) Ri(i, j) = s[i];
    }
    const std::size_t m = G.zeilen;
    Matrix Em(n + 1, m);
    for (std::size_t r = 0; r < m; ++r)
    {
        double gf = 0.0;
        for (std::size_t j = 0; j < n; ++j)
        {
            double s = 0.0;
            for (std::size_t k = 0; k < n; ++k) s += G(r, k) * Ri(k, j);
            Em(j, r) = s;
            gf += s * f1[j];
        }
        Em(n, r) = h[r] - gf;
    }
    // jede Spalte (eine Nebenbedingung) auf Laenge 1: der NNLS hat dieselbe Loesung bis auf den Faktor je Spalte, aber die
    // Schranke und die Wahl der naechsten Spalte messen dann alle Nebenbedingungen gleich (S220: geteilt durch |A|² lagen
    // die Spalten an Harfe T104 um Groessenordnungen auseinander, die Loesung verletzte die Nebenbedingung mit −234)
    Vek laenge(m, 1.0);
    for (std::size_t r = 0; r < m; ++r)
    {
        double s = 0.0;
        for (std::size_t i = 0; i <= n; ++i) s += Em(i, r) * Em(i, r);
        if (s > 0.0)
        {
            laenge[r] = std::sqrt(s);
            for (std::size_t i = 0; i <= n; ++i) Em(i, r) /= laenge[r];
        }
    }
    Vek fv(n + 1, 0.0);
    fv[n] = 1.0;
    const Vek u = nnls(Em, fv, &laenge);
    Vek rv(n + 1, 0.0);
    for (std::size_t i = 0; i <= n; ++i)
    {
        double s = -fv[i];
        for (std::size_t r = 0; r < m; ++r) s += Em(i, r) * u[r];
        rv[i] = s;
    }
    double nrm = 0.0;
    for (double x : rv) nrm += x * x;
    if (std::sqrt(nrm) < 1e-12) throw std::runtime_error("constraints incompatible");
    Vek z(n);
    for (std::size_t j = 0; j < n; ++j) z[j] = -rv[j] / rv[n] + f1[j];
    Vek x(n, 0.0);
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j) x[i] += Ri(i, j) * z[j];
    return x;
}

Vek spektralfaktor(const Vek& r)
{
    const std::size_t N = r.size() - 1;
    Vek c;
    for (std::size_t k = N + 1; k-- > 0;) c.push_back(r[k]);
    for (std::size_t k = 1; k <= N; ++k) c.push_back(r[k]);
    std::vector<cplx> w = wurzeln(c);
    // Wurzeln am Einheitskreis: eine doppelte Nullstelle von R (R beruehrt 0) zerfaellt beim Rechnen in zwei Wurzeln auf
    // dem Kreis mit wenig verschiedenem Winkel; je Paar eine Wurzel mit dem mittleren Winkel (S220). Die uebrigen wie
    // weg2.py: die N kleinsten Betraege.
    constexpr double kKreis = 1e-5;
    std::vector<cplx> innen, kreis;
    for (const cplx& x : w)
    {
        if (std::abs(std::abs(x) - 1.0) <= kKreis) kreis.push_back(x);
        else if (std::abs(x) < 1.0) innen.push_back(x);
    }
    if (!kreis.empty() && kreis.size() % 2 == 0 && innen.size() + kreis.size() / 2 == N)
    {
        // je Wurzel die naechste noch freie (Sehne, also auch ueber ±π hinweg); die Mitte auf den Kreis gelegt
        w = innen;
        std::vector<bool> frei(kreis.size(), true);
        for (std::size_t i = 0; i < kreis.size(); ++i)
        {
            if (!frei[i]) continue;
            frei[i] = false;
            std::size_t jb = i;
            double db = std::numeric_limits<double>::infinity();
            for (std::size_t j = 0; j < kreis.size(); ++j)
                if (frei[j] && std::abs(kreis[j] - kreis[i]) < db)
                {
                    db = std::abs(kreis[j] - kreis[i]);
                    jb = j;
                }
            frei[jb] = false;
            const cplx mitte = 0.5 * (kreis[i] + kreis[jb]);
            w.push_back(std::abs(mitte) > 0.0 ? mitte / std::abs(mitte) : kreis[i]);
        }
    }
    else
    {
        std::sort(w.begin(), w.end(), [](const cplx& x, const cplx& y) { return std::abs(x) < std::abs(y); });
        w.resize(N);
    }
    Vek b = polyAus(w);
    double best = -std::numeric_limits<double>::infinity(), wj = 0.0;
    for (std::size_t i = 0; i < kNebenbedingungen; ++i)
    {
        const double om = kPi * static_cast<double>(i) / static_cast<double>(kNebenbedingungen - 1);
        double R = r[0];
        for (std::size_t k = 1; k <= N; ++k) R += 2.0 * std::cos(om * static_cast<double>(k)) * r[k];
        if (std::abs(R) > best)
        {
            best = std::abs(R);
            wj = om;
        }
    }
    double Rj = r[0];
    for (std::size_t k = 1; k <= N; ++k) Rj += 2.0 * std::cos(wj * static_cast<double>(k)) * r[k];
    cplx Bj = 0.0;
    for (std::size_t k = 0; k < b.size(); ++k) Bj += b[k] * std::polar(1.0, -wj * static_cast<double>(k));
    const double s = std::sqrt(Rj) / std::abs(Bj);
    for (double& x : b) x *= s;
    return b;
}

Vek zaehlerPassiv(const Vorgabe& v, const Vek& a, int N)
{
    const double eps = 1e-7, delta = 1e-9;
    const std::size_t K = v.fk.size(), n = static_cast<std::size_t>(N + 1);
    const Vek sw0 = gewichtRel(v.g);
    auto betragA2 = [&](double x) {
        cplx s = 0.0;
        for (std::size_t j = 0; j < a.size(); ++j) s += a[j] * std::polar(1.0, -x * static_cast<double>(j));
        return std::norm(s);
    };
    Matrix E(K, n);
    Vek f(K);
    for (std::size_t k = 0; k < K; ++k)
    {
        const double x = omegaVon(v.fk[k]);
        const double sw = sw0[k] / (2.0 * v.g[k]);
        const double ak2 = betragA2(x);
        for (std::size_t j = 0; j < n; ++j) E(k, j) = (j == 0 ? 1.0 : 2.0) * std::cos(x * static_cast<double>(j)) / ak2 * sw;
        f[k] = v.g[k] * v.g[k] * sw;
    }
    // Stellen der Nebenbedingung: das Raster von weg2.py, dazu im Austausch jede Stelle, an der die Loesung sie zwischen
    // den Rasterpunkten verletzt (S220, Harfe T104: ein Pol mit Radius 0,998 bei 2,80 rad — R wurde zwischen zwei
    // Rasterpunkten negativ, der Spektralfaktor hing dann an Rundungen: |H| 1,00001 / 1,024 aus demselben R in Python,
    // 17 in C++)
    Vek stellen(kNebenbedingungen);
    for (std::size_t i = 0; i < kNebenbedingungen; ++i) stellen[i] = kPi * static_cast<double>(i) / static_cast<double>(kNebenbedingungen - 1);
    auto Rvon = [&](const Vek& r, double om) {
        double R = r[0];
        for (std::size_t k = 1; k < r.size(); ++k) R += 2.0 * std::cos(om * static_cast<double>(k)) * r[k];
        return R;
    };
    Vek r;
    for (int runde = 0; runde < 10; ++runde)
    {
        const std::size_t m = stellen.size();
        Matrix G(2 * m, n);
        Vek h(2 * m);
        for (std::size_t i = 0; i < m; ++i)
        {
            // δ |A|² ≤ R ≤ (1 − ε) |A|², jede Zeile durch |A|² geteilt (dieselbe zulaessige Menge wie weg2.py): die LSI
            // misst die Verletzung dann im Verhaeltnis |H|², nicht absolut — an einem Pol nahe z = 1 (|A(1)|² um 10⁻⁹,
            // Laute Chor 1.1 Bund 12) gaben 7·10⁻¹² absolut sonst |H| = 1,0028 (S220)
            const double ag2 = betragA2(stellen[i]);
            for (std::size_t j = 0; j < n; ++j)
            {
                const double cg = (j == 0 ? 1.0 : 2.0) * std::cos(stellen[i] * static_cast<double>(j)) / ag2;
                G(i, j) = cg;
                G(m + i, j) = -cg;
            }
            h[i] = delta;
            h[m + i] = -(1.0 - eps);
        }
        r = lsi(E, f, G, h);
        // Austausch: auf dem Pruefraster die oertlichen Verletzungen suchen (Verhaeltnis R / |A|² ueber 1 − ε/2 oder unter δ/2)
        const double schritt = kPi / static_cast<double>(kPruefraster - 1);
        Vek q(kPruefraster);
        for (std::size_t i = 0; i < kPruefraster; ++i)
        {
            const double om = schritt * static_cast<double>(i);
            q[i] = Rvon(r, om) / betragA2(om);
        }
        bool neu = false;
        for (std::size_t i = 0; i < kPruefraster; ++i)
        {
            const double l = i > 0 ? q[i - 1] : q[i], rr = i + 1 < kPruefraster ? q[i + 1] : q[i];
            const bool oben = q[i] > 1.0 - 0.5 * eps && q[i] >= l && q[i] >= rr;
            const bool unten = q[i] < 0.5 * delta && q[i] <= l && q[i] <= rr;
            if (oben || unten)
            {
                stellen.push_back(schritt * static_cast<double>(i));
                neu = true;
            }
        }
        if (!neu) break;
    }
    return spektralfaktor(r);
}

// der Verlust einer Stelle: Ordnung 8, sonst 4 oder 2; false, wenn zu wenig Teiltoene
bool verlustEntwurf(const Vek& betrag, const Teiltoene& tt, Vek& b, Vek& a)
{
    int K = 0;
    for (double f : tt.hz)
        if (f < 0.45 * kRate) ++K;
    for (int N : {8, 4, 2})
    {
        if (2 * N + 1 > 2 * K) continue;
        const Vorgabe v = vorgabeVon(betrag, tt);
        nenner(v, N, b, a);
        b = zaehlerPassiv(v, a, N);
        const double mx = maxBetrag(b, a);
        if (mx > 1.0)
            for (double& x : b) x = x / mx * (1.0 - 1e-12);
        return true;
    }
    return false;
}

// ------------------------------------------------------------------ eine Stelle
struct Stelle
{
    double lage = 0.0;  // Buende
    double x = 0.0;     // lage / buende
    Ziel ziel;
    Teiltoene tt;
    Vek fz, wn, n, restN;
    Vek b, a;
    Vek verlustBetrag, verlustVerzug, rest;  // auf F
    bool gut = false;
};

void setzeVerlust(Stelle& s)
{
    const Vek& F = rasterF();
    std::vector<cplx> hl(kRaster);
    for (std::size_t i = 0; i < kRaster; ++i) hl[i] = ku::antwort(s.b, s.a, omegaVon(F[i]));
    s.verlustBetrag.resize(kRaster);
    for (std::size_t i = 0; i < kRaster; ++i) s.verlustBetrag[i] = std::abs(hl[i]);
    s.verlustVerzug = verzugVon(hl);
    s.rest.resize(kRaster);
    for (std::size_t i = 0; i < kRaster; ++i) s.rest[i] = s.ziel.verzug[i] - s.verlustVerzug[i];
    s.restN.clear();
    for (double f : s.fz) s.restN.push_back(interpF(f, s.rest));
}

// `verlustVon`: nullptr = entwerfen; sonst zwischen den Stellen gemittelt
Stelle stelleVon(const Saitenwerte& sa, double mensur, double u, int buende, const std::vector<Stelle>* verlustVon)
{
    Stelle s;
    s.lage = u;
    s.x = u / buende;
    s.ziel = zielVon(sa, mensur * std::exp2(-u / 12.0));
    s.tt = teiltoeneVon(s.ziel.betrag, s.ziel.verzug);
    if (s.tt.hz.empty()) return s;
    for (std::size_t k = 0; k < s.tt.hz.size(); ++k)
    {
        const double f = s.tt.hz[k], sg = s.tt.sigma[k];
        if (!(f < 0.45 * kRate)) continue;
        if (sg <= kGrenzeAbklingen || (sg <= kZugabe * kGrenzeAbklingen && f <= 19500.0)) s.fz.push_back(f);
    }
    for (std::size_t k = 0; k < s.fz.size(); ++k)
    {
        s.wn.push_back(omegaVon(s.fz[k]));
        s.n.push_back(static_cast<double>(k + 1));
    }
    if (verlustVon == nullptr)
    {
        if (!verlustEntwurf(s.ziel.betrag, s.tt, s.b, s.a)) return s;
    }
    else
    {
        const auto& st = *verlustVon;
        std::size_t k = 0;
        while (k + 2 < st.size() && st[k + 1].lage <= u) ++k;
        const double t = (u - st[k].lage) / (st[k + 1].lage - st[k].lage);
        const std::size_t nb = std::max(st[k].b.size(), st[k + 1].b.size()), na = std::max(st[k].a.size(), st[k + 1].a.size());
        s.b.assign(nb, 0.0);
        s.a.assign(na, 0.0);
        for (std::size_t i = 0; i < nb; ++i)
            s.b[i] = (1 - t) * (i < st[k].b.size() ? st[k].b[i] : 0.0) + t * (i < st[k + 1].b.size() ? st[k + 1].b[i] : 0.0);
        for (std::size_t i = 0; i < na; ++i)
            s.a[i] = (1 - t) * (i < st[k].a.size() ? st[k].a[i] : 0.0) + t * (i < st[k + 1].a.size() ? st[k + 1].a[i] : 0.0);
    }
    setzeVerlust(s);
    s.gut = !s.fz.empty();
    return s;
}

// ------------------------------------------------------------------ Abel & Smith an der Restphase (umlauf_umfang.py)
void abelSmith(const Vek& rest, double fd, Vek& alpha, Vek& cc, double beta = 0.85)
{
    const Vek& F = rasterF();
    alpha.clear();
    cc.clear();
    const double wd = omegaVon(fd);
    const double rd = interpF(fd, rest);
    Vek W(kRaster);
    for (std::size_t i = 0; i < kRaster; ++i) W[i] = omegaVon(F[i]);
    const Vek tau = gradient(rest, W[1] - W[0]);
    const double mu0 = interpF(fd, tau);
    const int nap = static_cast<int>(std::floor((rd - mu0 * wd) / (2.0 * kPi)));
    if (nap < 1) return;
    const double mu = (rd - 2.0 * kPi * nap) / wd;
    Vek psi, wm;
    double lauf = -std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < kRaster && F[i] <= fd; ++i)
    {
        lauf = std::max(lauf, rest[i] - mu * W[i]);
        psi.push_back(lauf);
        wm.push_back(W[i]);
    }
    Vek theta;
    for (int k = 1; k <= nap; ++k) theta.push_back(interp(kPi * (2 * k - 1), psi, wm));
    for (std::size_t k = 0; k < theta.size(); ++k)
    {
        const double delta = (theta[k] - (k == 0 ? -theta[0] : theta[k - 1])) / 2.0;
        const double eta = (1.0 - beta * std::cos(delta)) / (1.0 - beta);
        alpha.push_back(std::sqrt(eta * eta - 1.0) - eta);
        cc.push_back(std::cos(theta[k]));
    }
}

int abelSmithZahl(const Stelle& s)
{
    Vek al, cc;
    abelSmith(s.rest, std::min(s.fz.back() * 1.02, 19500.0), al, cc);
    return static_cast<int>(al.size());
}

void startVon(const Vek& rest, double fd, int M, Vek& r, Vek& th)
{
    Vek al, cc;
    abelSmith(rest, fd, al, cc);
    r.clear();
    th.clear();
    for (std::size_t k = 0; k < al.size(); ++k)
    {
        r.push_back(-al[k]);
        th.push_back(std::acos(std::clamp(cc[k], -1.0, 1.0)));
    }
    for (int k = 0; static_cast<int>(r.size()) < M; ++k)
    {
        r.push_back(0.45);
        th.push_back(0.02 + 0.03 * k);
    }
    r.resize(static_cast<std::size_t>(M));
    th.resize(static_cast<std::size_t>(M));
    for (int k = 0; k < M; ++k)
    {
        r[static_cast<std::size_t>(k)] = std::clamp(r[static_cast<std::size_t>(k)], 0.02, kRadiusMax - 1e-3);
        th[static_cast<std::size_t>(k)] = std::clamp(th[static_cast<std::size_t>(k)], 1e-3, kPi - 1e-3);
    }
}

// ------------------------------------------------------------------ Bewertung an einer Stelle (gs.werte)
struct Werte
{
    Bewertung bew;
    Umlauffilter filter;
};

std::optional<Werte> werteAn(const Stelle& s, const std::vector<Biquad>& kette, int ordnung)
{
    const Vek& F = rasterF();
    Vek kv(kRaster);
    for (std::size_t i = 0; i < kRaster; ++i) kv[i] = ku::verzugKette(kette, omegaVon(F[i]));
    const double f1 = s.tt.hz.front(), w1 = omegaVon(f1);
    const double tot = interpF(f1, s.rest) - interpF(f1, kv);
    Werte w;
    w.filter.gliedOrdnung = ordnung;
    w.filter.kette = kette;
    w.filter.b = s.b;
    w.filter.a = s.a;
    if (!ku::stimmeGlied(w.filter, w1, tot)) return std::nullopt;
    // Verzug des Modells auf F: Verlust + Kette + Glied + N ω
    const Vek an = ku::thiranNenner(w.filter.gliedVerzug, ordnung);
    const Vek bn(an.rbegin(), an.rend());
    std::vector<cplx> hf(kRaster);
    for (std::size_t i = 0; i < kRaster; ++i) hf[i] = ku::antwort(bn, an, omegaVon(F[i]));
    const Vek gv = verzugVon(hf);
    Vek phm(kRaster);
    for (std::size_t i = 0; i < kRaster; ++i)
    {
        phm[i] = s.verlustVerzug[i] + kv[i] + gv[i] + omegaVon(F[i]) * w.filter.verzugGanz;
        if (i > 0 && phm[i] <= phm[i - 1]) return std::nullopt;
    }
    const Teiltoene tm = teiltoeneVon(s.verlustBetrag, phm);
    const std::size_t n = std::min(s.tt.hz.size(), tm.hz.size());
    Bewertung& b = w.bew;
    b.auswertbar = n > 0;
    b.centGroesster = 0.0;
    b.abklingenGroesster = 0.0;
    for (std::size_t k = 0; k < n; ++k)
    {
        const double c = 1200.0 * std::log2(tm.hz[k] / s.tt.hz[k]);
        const double p = 100.0 * (tm.sigma[k] / s.tt.sigma[k] - 1.0);
        if (k == 0) b.grundtonCent = c;
        if (!(s.tt.sigma[k] <= kGrenzeAbklingen)) continue;
        ++b.zaehlend;
        b.centGroesster = std::max(b.centGroesster, std::abs(c));
        b.abklingenGroesster = std::max(b.abklingenGroesster, std::abs(p));
        if (p >= kAbklingenUnten && p <= kAbklingenOben) ++b.inSchwelle;
    }
    return w;
}

// ------------------------------------------------------------------ trf wie scipy `least_squares` (S220)
// Die Proben rechnen mit `least_squares(method="trf", x_scale="jac")`; ein eigenes LM fand aus denselben Starts andere
// oertliche Minima (Cembalo T89, M 8: vier Biquads am Rand θ = 0, 9,4 Cent statt 0,004). Deshalb hier derselbe Weg:
// Vertrauensbereich ueber die SVD (`solve_lsq_trust_region`), ohne Grenzen `trf_no_bounds`, mit Grenzen `trf_bounds`
// (Skalierung nach Coleman & Li, Schritt am Rand gespiegelt). ftol = xtol = gtol = 10⁻⁸.

double punkt(const Vek& a, const Vek& b)
{
    double s = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) s += a[i] * b[i];
    return s;
}
double norm2(const Vek& a) { return std::sqrt(punkt(a, a)); }

// einseitiges Jacobi-Verfahren (Hestenes): die Spalten `c` orthogonalisieren, die Drehungen in `w` (Spalten) gesammelt
void jacobiSpalten(std::vector<Vek>& c, std::vector<Vek>& w)
{
    const std::size_t q = c.size();
    w.assign(q, Vek(q, 0.0));
    for (std::size_t j = 0; j < q; ++j) w[j][j] = 1.0;
    for (int sweep = 0; sweep < 80; ++sweep)
    {
        bool gedreht = false;
        for (std::size_t i = 0; i + 1 < q; ++i)
            for (std::size_t j = i + 1; j < q; ++j)
            {
                const double a = punkt(c[i], c[i]), b = punkt(c[j], c[j]), g = punkt(c[i], c[j]);
                if (g == 0.0 || std::abs(g) <= 1e-15 * std::sqrt(a * b)) continue;
                gedreht = true;
                const double zeta = (b - a) / (2.0 * g);
                const double t = (zeta >= 0.0 ? 1.0 : -1.0) / (std::abs(zeta) + std::sqrt(1.0 + zeta * zeta));
                const double cs = 1.0 / std::sqrt(1.0 + t * t), sn = cs * t;
                for (std::size_t k = 0; k < c[i].size(); ++k)
                {
                    const double x = c[i][k], y = c[j][k];
                    c[i][k] = cs * x - sn * y;
                    c[j][k] = sn * x + cs * y;
                }
                for (std::size_t k = 0; k < q; ++k)
                {
                    const double x = w[i][k], y = w[j][k];
                    w[i][k] = cs * x - sn * y;
                    w[j][k] = sn * x + cs * y;
                }
            }
        if (!gedreht) break;
    }
}

// duenne SVD von A (m × n): Singulaerwerte absteigend, V als Spalten (Laenge n), dazu uf = Uᵀ f
struct Svd
{
    Vek s, uf;
    std::vector<Vek> v;
};
Svd svdMit(const Matrix& A, const Vek& f)
{
    const std::size_t m = A.zeilen, n = A.spalten;
    std::vector<Vek> c, w;
    Svd out;
    std::vector<std::pair<double, std::size_t>> ord;
    if (m >= n)
    {
        const Qr qr(A);
        Vek qf = f;
        qr.qtMal(qf);
        c.assign(n, Vek(n, 0.0));
        for (std::size_t j = 0; j < n; ++j)
            for (std::size_t i = 0; i <= j; ++i) c[j][i] = qr.r(i, j);
        jacobiSpalten(c, w);
        for (std::size_t j = 0; j < n; ++j) ord.emplace_back(norm2(c[j]), j);
        std::sort(ord.begin(), ord.end(), [](const auto& x, const auto& y) { return x.first > y.first; });
        for (const auto& [sj, j] : ord)
        {
            out.s.push_back(sj);
            out.v.push_back(w[j]);
            double u = 0.0;
            if (sj > 0.0)
                for (std::size_t i = 0; i < n; ++i) u += c[j][i] / sj * qf[i];
            out.uf.push_back(u);
        }
    }
    else
    {
        // Aᵀ = U' S V'ᵀ  →  A = V' S U'ᵀ: U = V', V = U'
        c.assign(m, Vek(n, 0.0));
        for (std::size_t i = 0; i < m; ++i)
            for (std::size_t j = 0; j < n; ++j) c[i][j] = A(i, j);
        jacobiSpalten(c, w);
        for (std::size_t j = 0; j < m; ++j) ord.emplace_back(norm2(c[j]), j);
        std::sort(ord.begin(), ord.end(), [](const auto& x, const auto& y) { return x.first > y.first; });
        for (const auto& [sj, j] : ord)
        {
            out.s.push_back(sj);
            Vek vj(n, 0.0);
            if (sj > 0.0)
                for (std::size_t i = 0; i < n; ++i) vj[i] = c[j][i] / sj;
            out.v.push_back(std::move(vj));
            out.uf.push_back(sj > 0.0 ? punkt(w[j], f) : 0.0);
        }
    }
    return out;
}

// scipy `solve_lsq_trust_region` (Moré 1977): Schritt im Vertrauensbereich Δ aus der SVD
Vek schrittImBereich(std::size_t n, std::size_t m, const Svd& d, double Delta, double& alpha)
{
    const std::size_t k = d.s.size();
    Vek suf(k);
    for (std::size_t i = 0; i < k; ++i) suf[i] = d.s[i] * d.uf[i];
    const double eps = std::numeric_limits<double>::epsilon();
    const bool vollerRang = m >= n && k > 0 && d.s[k - 1] > eps * static_cast<double>(m) * d.s[0];
    auto ausV = [&](const Vek& koeff) {
        Vek p(n, 0.0);
        for (std::size_t i = 0; i < k; ++i)
            for (std::size_t j = 0; j < n; ++j) p[j] += d.v[i][j] * koeff[i];
        return p;
    };
    if (vollerRang)
    {
        Vek kf(k);
        for (std::size_t i = 0; i < k; ++i) kf[i] = -d.uf[i] / d.s[i];
        Vek p = ausV(kf);
        if (norm2(p) <= Delta)
        {
            alpha = 0.0;
            return p;
        }
    }
    auto phi = [&](double al, double& ableitung) {
        double pn = 0.0, s3 = 0.0;
        for (std::size_t i = 0; i < k; ++i)
        {
            const double den = d.s[i] * d.s[i] + al;
            pn += (suf[i] / den) * (suf[i] / den);
            s3 += suf[i] * suf[i] / (den * den * den);
        }
        pn = std::sqrt(pn);
        ableitung = -s3 / pn;
        return pn - Delta;
    };
    if (!vollerRang)
    {
        // TK2 a, abweichend von scipy: bei Rangverlust (eine Spalte von J exakt null — ein Logit-Parameter an seiner
        // Grenze) liegt die Wurzel von phi unter 0, wenn der Schritt kleinster Norm schon im Bereich liegt; scipy rechnet
        // dann mit alpha < 0 einen Schritt, der das Modell vergroessert (S220 Cembalo T73). Hier: dieser Schritt, alpha 0.
        Vek kf(k, 0.0);
        for (std::size_t i = 0; i < k; ++i)
            if (d.s[i] > eps * static_cast<double>(std::max(m, n)) * d.s[0]) kf[i] = -d.uf[i] / d.s[i];
        Vek p = ausV(kf);
        if (norm2(p) <= Delta)
        {
            alpha = 0.0;
            return p;
        }
    }
    double oben = norm2(suf) / Delta, unten = 0.0;
    if (vollerRang)
    {
        double ab = 0.0;
        const double ph = phi(0.0, ab);
        unten = -ph / ab;
    }
    if (!vollerRang && alpha == 0.0) alpha = std::max(0.001 * oben, std::sqrt(unten * oben));
    for (int it = 0; it < 10; ++it)
    {
        if (alpha < unten || alpha > oben) alpha = std::max(0.001 * oben, std::sqrt(unten * oben));
        double ab = 0.0;
        const double ph = phi(alpha, ab);
        if (ph < 0.0) oben = alpha;
        const double ratio = ph / ab;
        unten = std::max(unten, alpha - ratio);
        alpha -= (ph + Delta) * ratio / Delta;
        if (std::abs(ph) < 0.01 * Delta) break;
    }
    Vek kf(k);
    for (std::size_t i = 0; i < k; ++i) kf[i] = -suf[i] / (d.s[i] * d.s[i] + alpha);
    Vek p = ausV(kf);
    const double pn = norm2(p);
    if (pn > 0.0)
        for (double& x : p) x *= Delta / pn;
    return p;
}

// ½‖J s‖² + ½ sᵀ diag(dg) s + gᵀ s
double quadratisch(const Matrix& J, const Vek& g, const Vek& s, const Vek* dg)
{
    double q = 0.0;
    for (std::size_t i = 0; i < J.zeilen; ++i)
    {
        double t = 0.0;
        for (std::size_t j = 0; j < J.spalten; ++j) t += J(i, j) * s[j];
        q += t * t;
    }
    if (dg != nullptr)
        for (std::size_t j = 0; j < s.size(); ++j) q += s[j] * s[j] * (*dg)[j];
    return 0.5 * q + punkt(g, s);
}

Matrix spaltenMal(const Matrix& J, const Vek& d)
{
    Matrix out = J;
    for (std::size_t i = 0; i < J.zeilen; ++i)
        for (std::size_t j = 0; j < J.spalten; ++j) out(i, j) *= d[j];
    return out;
}

Vek gradientVon(const Matrix& J, const Vek& f)
{
    Vek g(J.spalten, 0.0);
    for (std::size_t i = 0; i < J.zeilen; ++i)
        for (std::size_t j = 0; j < J.spalten; ++j) g[j] += J(i, j) * f[i];
    return g;
}

void jacSkala(const Matrix& J, Vek& skalaInv, bool erstes)
{
    Vek s(J.spalten, 0.0);
    for (std::size_t i = 0; i < J.zeilen; ++i)
        for (std::size_t j = 0; j < J.spalten; ++j) s[j] += J(i, j) * J(i, j);
    for (std::size_t j = 0; j < s.size(); ++j)
    {
        s[j] = std::sqrt(s[j]);
        if (erstes) skalaInv[j] = s[j] == 0.0 ? 1.0 : s[j];
        else skalaInv[j] = std::max(s[j], skalaInv[j]);
    }
}

// scipy `update_tr_radius`
double neuerRadius(double Delta, double ist, double vorher, double schrittNorm, bool amRand, double& ratio)
{
    if (vorher > 0.0) ratio = ist / vorher;
    else if (vorher == 0.0 && ist == 0.0) ratio = 1.0;
    else ratio = 0.0;
    if (ratio < 0.25) return 0.25 * schrittNorm;
    if (ratio > 0.75 && amRand) return 2.0 * Delta;
    return Delta;
}

bool fertigNach(double dF, double F, double dxNorm, double xNorm, double ratio)
{
    const bool ftol = dF < 1e-8 * F && ratio > 0.25;
    const bool xtol = dxNorm < 1e-8 * (1e-8 + xNorm);
    return ftol || xtol;
}

// scipy `step_size_to_bound`: kleinster Schritt t ≥ 0 bis zu einer Grenze; `treffer` je Koordinate das Vorzeichen
double bisZurGrenze(const Vek& x, const Vek& s, const Vek& lb, const Vek& ub, std::vector<int>* treffer)
{
    const std::size_t n = x.size();
    Vek st(n, std::numeric_limits<double>::infinity());
    for (std::size_t i = 0; i < n; ++i)
        if (s[i] != 0.0) st[i] = std::max((lb[i] - x[i]) / s[i], (ub[i] - x[i]) / s[i]);
    const double mn = *std::min_element(st.begin(), st.end());
    if (treffer != nullptr)
    {
        treffer->assign(n, 0);
        for (std::size_t i = 0; i < n; ++i)
            if (st[i] == mn) (*treffer)[i] = (s[i] > 0.0) - (s[i] < 0.0);
    }
    return mn;
}

// scipy `minimize_quadratic_1d`
std::pair<double, double> minQuad1(double a, double b, double lo, double hi, double c)
{
    Vek t{lo, hi};
    if (a != 0.0)
    {
        const double e = -0.5 * b / a;
        if (lo < e && e < hi) t.push_back(e);
    }
    double bt = t[0], by = t[0] * (a * t[0] + b) + c;
    for (std::size_t i = 1; i < t.size(); ++i)
    {
        const double y = t[i] * (a * t[i] + b) + c;
        if (y < by)
        {
            by = y;
            bt = t[i];
        }
    }
    return {bt, by};
}

// scipy `least_squares(method="trf", x_scale="jac")`. `logits`: so viele fuehrende Parameter gehen durch sigmoid und
// werden nach jedem angenommenen Schritt auf ±kLogitGrenze gehalten (TK2 a, abweichend von scipy) — jenseits davon aendern
// sie keinen Wert, liessen aber die Norm von x wachsen und damit den Abbruch nach xtol greifen (S220 Cembalo T73: 10¹²).
Vek trf(const std::function<Vek(const Vek&)>& res, const std::function<Matrix(const Vek&)>& jac, Vek x, const Vek& lb, const Vek& ub,
        int maxNfev, std::size_t logits)
{
    const std::size_t n = x.size();
    bool grenzen = false;
    for (std::size_t i = 0; i < n; ++i)
        if (std::isfinite(lb[i]) || std::isfinite(ub[i])) grenzen = true;
    Vek f = res(x);
    int nfev = 1;
    Matrix J = jac(x);
    const std::size_t m = f.size();
    double cost = 0.5 * punkt(f, f);
    Vek g = gradientVon(J, f);
    Vek skalaInv(n, 1.0);
    jacSkala(J, skalaInv, true);
    double alpha = 0.0;
    bool fertig = false;

    auto clVektor = [&](Vek& v, Vek& dv) {
        v.assign(n, 1.0);
        dv.assign(n, 0.0);
        for (std::size_t i = 0; i < n; ++i)
        {
            if (g[i] < 0.0 && std::isfinite(ub[i]))
            {
                v[i] = ub[i] - x[i];
                dv[i] = -1.0;
            }
            if (g[i] > 0.0 && std::isfinite(lb[i]))
            {
                v[i] = x[i] - lb[i];
                dv[i] = 1.0;
            }
        }
    };

    double Delta;
    if (!grenzen)
    {
        Vek t(n);
        for (std::size_t i = 0; i < n; ++i) t[i] = x[i] * skalaInv[i];
        Delta = norm2(t);
    }
    else
    {
        Vek v, dv;
        clVektor(v, dv);
        Vek t(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            if (dv[i] != 0.0) v[i] *= skalaInv[i];
            t[i] = x[i] * skalaInv[i] / std::sqrt(v[i]);
        }
        Delta = norm2(t);
    }
    if (Delta == 0.0) Delta = 1.0;

    while (true)
    {
        Vek v(n, 1.0), dv(n, 0.0);
        double gNorm = 0.0;
        if (grenzen)
        {
            clVektor(v, dv);
            for (std::size_t i = 0; i < n; ++i) gNorm = std::max(gNorm, std::abs(g[i] * v[i]));
        }
        else
            for (std::size_t i = 0; i < n; ++i) gNorm = std::max(gNorm, std::abs(g[i]));
        if (gNorm < 1e-8) fertig = true;
        if (fertig || nfev >= maxNfev) break;

        Vek d(n), gh(n), diagH(n, 0.0);
        for (std::size_t i = 0; i < n; ++i)
        {
            const double skala = 1.0 / skalaInv[i];
            if (grenzen)
            {
                if (dv[i] != 0.0) v[i] *= skalaInv[i];
                d[i] = std::sqrt(v[i]) * skala;
                diagH[i] = g[i] * dv[i] * skala;
            }
            else
                d[i] = skala;
            gh[i] = d[i] * g[i];
        }
        const Matrix Jh = spaltenMal(J, d);
        Svd sv;
        if (grenzen)
        {
            Matrix Ja(m + n, n);
            for (std::size_t i = 0; i < m; ++i)
                for (std::size_t j = 0; j < n; ++j) Ja(i, j) = Jh(i, j);
            for (std::size_t j = 0; j < n; ++j) Ja(m + j, j) = std::sqrt(diagH[j]);
            Vek fa(m + n, 0.0);
            std::copy(f.begin(), f.end(), fa.begin());
            sv = svdMit(Ja, fa);
        }
        else
            sv = svdMit(Jh, f);
        const double theta = std::max(0.995, 1.0 - gNorm);

        double ist = -1.0, schrittNorm = 0.0;
        Vek xNeu, fNeu;
        double costNeu = cost;
        while (ist <= 0.0 && nfev < maxNfev)
        {
            Vek ph = schrittImBereich(n, m, sv, Delta, alpha);
            Vek p(n), step, stepH;
            for (std::size_t i = 0; i < n; ++i) p[i] = d[i] * ph[i];
            double vorher;
            if (!grenzen)
            {
                vorher = -quadratisch(Jh, gh, ph, nullptr);
                step = p;
                stepH = ph;
            }
            else
            {
                // scipy `select_step`
                bool innen = true;
                for (std::size_t i = 0; i < n; ++i)
                    if (!(x[i] + p[i] >= lb[i] && x[i] + p[i] <= ub[i])) innen = false;
                if (innen)
                {
                    vorher = -quadratisch(Jh, gh, ph, &diagH);
                    step = p;
                    stepH = ph;
                }
                else
                {
                    std::vector<int> tr;
                    const double pStride = bisZurGrenze(x, p, lb, ub, &tr);
                    Vek rh = ph;
                    for (std::size_t i = 0; i < n; ++i)
                        if (tr[i] != 0) rh[i] = -rh[i];
                    Vek r(n);
                    for (std::size_t i = 0; i < n; ++i) r[i] = d[i] * rh[i];
                    for (std::size_t i = 0; i < n; ++i)
                    {
                        p[i] *= pStride;
                        ph[i] *= pStride;
                    }
                    Vek xRand(n);
                    for (std::size_t i = 0; i < n; ++i) xRand[i] = x[i] + p[i];
                    // intersect_trust_region(p_h, r_h, Delta)
                    const double qa = punkt(rh, rh), qb = punkt(ph, rh), qc = punkt(ph, ph) - Delta * Delta;
                    double toTr = 0.0;
                    if (qa > 0.0 && qc <= 0.0)
                    {
                        const double dd = std::sqrt(qb * qb - qa * qc);
                        const double qq = -(qb + std::copysign(dd, qb));
                        const double t1 = qq / qa, t2 = qc / qq;
                        toTr = std::max(t1, t2);
                    }
                    const double toBound = bisZurGrenze(xRand, r, lb, ub, nullptr);
                    double rStride = std::min(toBound, toTr), rLo, rHi;
                    if (rStride > 0.0)
                    {
                        rLo = (1.0 - theta) * pStride / rStride;
                        rHi = rStride == toBound ? theta * toBound : toTr;
                    }
                    else
                    {
                        rLo = 0.0;
                        rHi = -1.0;
                    }
                    double rWert = std::numeric_limits<double>::infinity();
                    if (rLo <= rHi)
                    {
                        // build_quadratic_1d(J_h, g_h, r_h, s0=p_h, diag=diag_h)
                        Vek jv(m, 0.0), ju(m, 0.0);
                        for (std::size_t i = 0; i < m; ++i)
                            for (std::size_t j = 0; j < n; ++j)
                            {
                                jv[i] += Jh(i, j) * rh[j];
                                ju[i] += Jh(i, j) * ph[j];
                            }
                        double a = punkt(jv, jv), b = punkt(gh, rh), c = 0.5 * punkt(ju, ju) + punkt(gh, ph);
                        for (std::size_t j = 0; j < n; ++j)
                        {
                            a += rh[j] * rh[j] * diagH[j];
                            b += ph[j] * rh[j] * diagH[j];
                            c += 0.5 * ph[j] * ph[j] * diagH[j];
                        }
                        a *= 0.5;
                        b += punkt(ju, jv);
                        const auto [ts, ys] = minQuad1(a, b, rLo, rHi, c);
                        rWert = ys;
                        for (std::size_t i = 0; i < n; ++i)
                        {
                            rh[i] = rh[i] * ts + ph[i];
                            r[i] = rh[i] * d[i];
                        }
                    }
                    for (std::size_t i = 0; i < n; ++i)
                    {
                        p[i] *= theta;
                        ph[i] *= theta;
                    }
                    const double pWert = quadratisch(Jh, gh, ph, &diagH);
                    Vek agh(n), ag(n);
                    for (std::size_t i = 0; i < n; ++i)
                    {
                        agh[i] = -gh[i];
                        ag[i] = d[i] * agh[i];
                    }
                    const double toTr2 = Delta / norm2(agh);
                    const double toB2 = bisZurGrenze(x, ag, lb, ub, nullptr);
                    const double agStrideMax = toB2 < toTr2 ? theta * toB2 : toTr2;
                    Vek jg(m, 0.0);
                    for (std::size_t i = 0; i < m; ++i)
                        for (std::size_t j = 0; j < n; ++j) jg[i] += Jh(i, j) * agh[j];
                    double a2 = punkt(jg, jg);
                    for (std::size_t j = 0; j < n; ++j) a2 += agh[j] * agh[j] * diagH[j];
                    a2 *= 0.5;
                    const auto [agS, agWert] = minQuad1(a2, punkt(gh, agh), 0.0, agStrideMax, 0.0);
                    for (std::size_t i = 0; i < n; ++i)
                    {
                        agh[i] *= agS;
                        ag[i] *= agS;
                    }
                    if (pWert < rWert && pWert < agWert)
                    {
                        step = p;
                        stepH = ph;
                        vorher = -pWert;
                    }
                    else if (rWert < pWert && rWert < agWert)
                    {
                        step = r;
                        stepH = rh;
                        vorher = -rWert;
                    }
                    else
                    {
                        step = ag;
                        stepH = agh;
                        vorher = -agWert;
                    }
                }
            }
            xNeu.assign(n, 0.0);
            for (std::size_t i = 0; i < n; ++i)
            {
                xNeu[i] = x[i] + step[i];
                if (grenzen)  // make_strictly_feasible(rstep = 0)
                {
                    if (xNeu[i] <= lb[i]) xNeu[i] = std::nextafter(lb[i], ub[i]);
                    else if (xNeu[i] >= ub[i]) xNeu[i] = std::nextafter(ub[i], lb[i]);
                    if (xNeu[i] < lb[i] || xNeu[i] > ub[i]) xNeu[i] = 0.5 * (lb[i] + ub[i]);
                }
            }
            fNeu = res(xNeu);
            ++nfev;
            const double stepHNorm = norm2(stepH);
            bool endlich = true;
            for (double t : fNeu)
                if (!std::isfinite(t)) endlich = false;
            if (!endlich)
            {
                Delta = 0.25 * stepHNorm;
                continue;
            }
            costNeu = 0.5 * punkt(fNeu, fNeu);
            ist = cost - costNeu;
            double ratio = 0.0;
            const double DeltaNeu = neuerRadius(Delta, ist, vorher, stepHNorm, stepHNorm > 0.95 * Delta, ratio);
            schrittNorm = norm2(step);
            if (fertigNach(ist, cost, schrittNorm, norm2(x), ratio))
            {
                fertig = true;
                break;
            }
            alpha *= Delta / DeltaNeu;
            Delta = DeltaNeu;
        }
        if (ist > 0.0)
        {
            for (std::size_t i = 0; i < logits; ++i) xNeu[i] = std::clamp(xNeu[i], -kLogitGrenze, kLogitGrenze);
            x = std::move(xNeu);
            f = std::move(fNeu);
            cost = costNeu;
            J = jac(x);
            g = gradientVon(J, f);
            jacSkala(J, skalaInv, false);
        }
    }
    return x;
}

// ------------------------------------------------------------------ gebundene Anpassung (gs.gebunden)
struct Reihe
{
    int M = 0, P = 0;
    Vek A, B;  // M × (P + 1)
    [[nodiscard]] std::vector<Biquad> an(double x) const
    {
        const Vek v = legendre(2.0 * x - 1.0, P);
        std::vector<Biquad> k(static_cast<std::size_t>(M));
        for (int m = 0; m < M; ++m)
        {
            double sa = 0.0, sb = 0.0;
            for (int j = 0; j <= P; ++j)
            {
                sa += A[static_cast<std::size_t>(m * (P + 1) + j)] * v[static_cast<std::size_t>(j)];
                sb += B[static_cast<std::size_t>(m * (P + 1) + j)] * v[static_cast<std::size_t>(j)];
            }
            k[static_cast<std::size_t>(m)] = {kRadiusMax * sigmoid(sa), kPi * sigmoid(sb)};
        }
        return k;
    }
    static Vek legendre(double t, int P)
    {
        Vek v(static_cast<std::size_t>(P) + 1);
        v[0] = 1.0;
        if (P >= 1) v[1] = t;
        for (int j = 2; j <= P; ++j)
            v[static_cast<std::size_t>(j)] = ((2.0 * j - 1.0) * t * v[static_cast<std::size_t>(j - 1)] - (j - 1.0) * v[static_cast<std::size_t>(j - 2)]) / j;
        return v;
    }
};

Reihe gebunden(const std::vector<const Stelle*>& st, int M, int P, const Vek& r0, const Vek& th0, int ordnung, double reg)
{
    const std::size_t K = st.size();
    const std::size_t P1 = static_cast<std::size_t>(P) + 1, nk = static_cast<std::size_t>(M) * P1, npar = 2 * nk + K;
    std::vector<Vek> V(K), kf(K);
    std::vector<std::size_t> off(K + 1, 0);
    for (std::size_t k = 0; k < K; ++k)
    {
        V[k] = Reihe::legendre(2.0 * st[k]->x - 1.0, P);
        for (double n : st[k]->n) kf[k].push_back(kKf / (2.0 * kPi * n) * (n == 1.0 ? 10.0 : 1.0));
        off[k + 1] = off[k] + st[k]->wn.size();
    }
    std::vector<std::size_t> idx;
    for (std::size_t i = 0; i < 2 * nk; ++i)
        if (i % P1) idx.push_back(i);
    const double lo = ku::verzugUnten(ordnung);
    Vek Nint(K, 0.0);
    bool rein = true;

    auto rth = [&](const Vek& p, std::size_t k, Vek& r, Vek& th) {
        r.assign(static_cast<std::size_t>(M), 0.0);
        th.assign(static_cast<std::size_t>(M), 0.0);
        for (std::size_t m = 0; m < static_cast<std::size_t>(M); ++m)
        {
            double sa = 0.0, sb = 0.0;
            for (std::size_t j = 0; j < P1; ++j)
            {
                sa += p[m * P1 + j] * V[k][j];
                sb += p[nk + m * P1 + j] * V[k][j];
            }
            r[m] = kRadiusMax * sigmoid(sa);
            th[m] = kPi * sigmoid(sb);
        }
    };
    auto verzugBq = [](const Vek& r, const Vek& th, double w) {
        double s = 0.0;
        for (std::size_t m = 0; m < r.size(); ++m)
            s += 2.0 * w + 2.0 * std::atan2(r[m] * std::sin(w - th[m]), 1.0 - r[m] * std::cos(w - th[m])) +
                 2.0 * std::atan2(r[m] * std::sin(w + th[m]), 1.0 - r[m] * std::cos(w + th[m]));
        return s;
    };
    auto vz = [&](std::size_t k, double D, double w) {
        return rein ? D * w : Nint[k] * w + ku::verzugThiran(D, ordnung, w);
    };
    auto residuen = [&](const Vek& p) {
        Vek out(off[K] + idx.size());
        for (std::size_t q = 0; q < idx.size(); ++q) out[off[K] + q] = reg * p[idx[q]];
        Vek r, th;
        for (std::size_t k = 0; k < K; ++k)
        {
            rth(p, k, r, th);
            const double D = p[2 * nk + k];
            const Stelle& s = *st[k];
            for (std::size_t i = 0; i < s.wn.size(); ++i)
                out[off[k] + i] = kf[k][i] * (vz(k, D, s.wn[i]) + verzugBq(r, th, s.wn[i]) - s.restN[i]);
        }
        return out;
    };
    auto jacobi = [&](const Vek& p, const Vek&) {
        Matrix J(off[K] + idx.size(), npar);
        for (std::size_t q = 0; q < idx.size(); ++q) J(off[K] + q, idx[q]) = reg;
        Vek r, th;
        for (std::size_t k = 0; k < K; ++k)
        {
            rth(p, k, r, th);
            const double D = p[2 * nk + k];
            const Stelle& s = *st[k];
            for (std::size_t i = 0; i < s.wn.size(); ++i)
            {
                const double w = s.wn[i], c = kf[k][i];
                const std::size_t row = off[k] + i;
                for (std::size_t m = 0; m < static_cast<std::size_t>(M); ++m)
                {
                    const double rm = r[m], tm = th[m];
                    auto gr = [&](double u) { return std::sin(u) / (1.0 - 2.0 * rm * std::cos(u) + rm * rm); };
                    auto gu = [&](double u) { return (rm * std::cos(u) - rm * rm) / (1.0 - 2.0 * rm * std::cos(u) + rm * rm); };
                    const double dr = c * 2.0 * (gr(w - tm) + gr(w + tm)) * (rm * (1.0 - rm / kRadiusMax));
                    const double dt = c * 2.0 * (gu(w + tm) - gu(w - tm)) * (tm * (1.0 - tm / kPi));
                    for (std::size_t j = 0; j < P1; ++j)
                    {
                        J(row, m * P1 + j) = dr * V[k][j];
                        J(row, nk + m * P1 + j) = dt * V[k][j];
                    }
                }
                J(row, 2 * nk + k) = rein ? c * w : c * (ku::verzugThiran(D + 1e-5, ordnung, w) - ku::verzugThiran(D - 1e-5, ordnung, w)) / 2e-5;
            }
        }
        return J;
    };

    Vek p(npar, 0.0);
    for (std::size_t m = 0; m < static_cast<std::size_t>(M); ++m)
    {
        p[m * P1] = std::log(r0[m] / (kRadiusMax - r0[m]));
        p[nk + m * P1] = std::log(th0[m] / (kPi - th0[m]));
    }
    for (std::size_t k = 0; k < K; ++k)
        p[2 * nk + k] = (st[k]->restN[0] - verzugBq(r0, th0, st[k]->wn[0])) / st[k]->wn[0];

    Vek unten(npar, -std::numeric_limits<double>::infinity()), oben(npar, std::numeric_limits<double>::infinity());
    const auto jac = [&](const Vek& q) { return jacobi(q, Vek{}); };
    p = trf(residuen, jac, p, unten, oben, 600, 2 * nk);

    rein = false;  // zweite Stufe: das Glied mit seiner genauen Phase
    for (std::size_t k = 0; k < K; ++k)
    {
        Nint[k] = std::floor(p[2 * nk + k] - lo);
        p[2 * nk + k] -= Nint[k];
        unten[2 * nk + k] = lo - 0.25;
        oben[2 * nk + k] = lo + 1.25;
    }
    for (int runde = 1; runde <= 12; ++runde)
    {
        for (std::size_t k = 0; k < K; ++k) p[2 * nk + k] = std::clamp(p[2 * nk + k], lo - 0.24, lo + 1.24);
        p = trf(residuen, jac, p, unten, oben, 400, 2 * nk);
        bool fertig = true;
        for (std::size_t k = 0; k < K; ++k)
        {
            const double tot = Nint[k] + p[2 * nk + k];
            const double neu = std::floor(tot - lo);
            if (neu != Nint[k]) fertig = false;
            Nint[k] = neu;
            p[2 * nk + k] = tot - neu;
        }
        if (fertig) break;
    }
    Reihe e;
    e.M = M;
    e.P = P;
    e.A.assign(p.begin(), p.begin() + static_cast<std::ptrdiff_t>(nk));
    e.B.assign(p.begin() + static_cast<std::ptrdiff_t>(nk), p.begin() + static_cast<std::ptrdiff_t>(2 * nk));
    return e;
}

// ------------------------------------------------------------------ Zufall (eigener Generator, nicht numpy)
struct Zufall
{
    std::uint64_t z;
    explicit Zufall(std::uint64_t s) : z(s) {}
    double gleich()
    {
        z += 0x9E3779B97F4A7C15ull;
        std::uint64_t x = z;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        x ^= x >> 31;
        return (static_cast<double>(x >> 11) + 0.5) / 9007199254740992.0;
    }
    double normal(double mu, double sigma)
    {
        const double u1 = gleich(), u2 = gleich();
        return mu + sigma * std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * kPi * u2);
    }
};

bool besser(const Werte& neu, const std::optional<Werte>& alt)
{
    if (!alt) return true;
    const bool na = !neu.bew.abklingenHaelt(), aa = !alt->bew.abklingenHaelt();
    if (na != aa) return na < aa;
    return neu.bew.centGroesster < alt->bew.centGroesster;
}

}  // namespace

// ================================================================== oeffentlich
Bewertung bewerte(const Saitenwerte& saite, double laenge, Umlauffilter& filter)
{
    Stelle s;
    s.ziel = zielVon(saite, laenge);
    s.tt = teiltoeneVon(s.ziel.betrag, s.ziel.verzug);
    if (s.tt.hz.empty()) return {};
    s.b = filter.b;
    s.a = filter.a;
    setzeVerlust(s);
    const std::optional<Werte> w = werteAn(s, filter.kette, filter.gliedOrdnung);
    if (!w) return {};
    filter = w->filter;
    return w->bew;
}

std::optional<TafelTaste> entwirfJeTaste(const Saitenwerte& saite, double laenge)
{
    const Stelle s = stelleVon(saite, laenge, 0.0, 1, nullptr);
    if (!s.gut) return std::nullopt;
    const int nap = abelSmithZahl(s);
    std::vector<int> leiter;
    if (nap <= 1)
        leiter = {1, 2, 3, 4, 6, 8};
    else
    {
        leiter = {nap, nap + 2, static_cast<int>(std::ceil(1.5 * (nap + 2))), 2 * (nap + 2)};
        std::sort(leiter.begin(), leiter.end());
        leiter.erase(std::unique(leiter.begin(), leiter.end()), leiter.end());
    }
    Zufall zufall(205);
    std::optional<Werte> bestes;
    const std::vector<const Stelle*> st{&s};
    for (int ordnung : {5, 3, 1})
    {
        for (int M : leiter)
        {
            std::vector<std::pair<Vek, Vek>> starts;
            for (double fak : {1.02, 1.3})
            {
                Vek r0, th0;
                startVon(s.rest, std::min(s.fz.back() * fak, 19500.0), M, r0, th0);
                starts.emplace_back(r0, th0);
            }
            if (M <= 4)
                for (int i = 0; i < 10; ++i)
                {
                    Vek r0(static_cast<std::size_t>(M)), th0(static_cast<std::size_t>(M));
                    for (int m = 0; m < M; ++m) r0[static_cast<std::size_t>(m)] = std::clamp(kRadiusMax * sigmoid(zufall.normal(-0.5, 1.2)), 0.02, kRadiusMax - 1e-3);
                    for (int m = 0; m < M; ++m) th0[static_cast<std::size_t>(m)] = std::clamp(kPi * sigmoid(zufall.normal(-1.0, 1.5)), 1e-3, kPi - 1e-3);
                    starts.emplace_back(r0, th0);
                }
            std::optional<Werte> diese;
            for (const auto& [r0, th0] : starts)
            {
                Reihe reihe;
                try
                {
                    reihe = gebunden(st, M, 0, r0, th0, ordnung, 0.0);
                }
                catch (const std::exception&)
                {
                    continue;
                }
                const std::optional<Werte> w = werteAn(s, reihe.an(0.0), ordnung);
                if (w && besser(*w, diese)) diese = w;
            }
            if (!diese) continue;
            if (besser(*diese, bestes)) bestes = diese;
            if (diese->bew.haelt()) break;
        }
        if (bestes && bestes->bew.haelt()) break;
    }
    if (!bestes) return std::nullopt;
    TafelTaste t;
    t.filter = bestes->filter;
    t.bewertung = bestes->bew;
    t.grundHz = s.tt.hz.front();
    t.zaehlendMitZugabe = static_cast<int>(s.fz.size());
    t.abelSmith = nap;
    return t;
}

std::vector<Biquad> TafelSaite::ketteAn(double lage) const
{
    Reihe r;
    r.M = static_cast<int>(radiusReihe.size());
    r.P = grad;
    for (const Vek& v : radiusReihe) r.A.insert(r.A.end(), v.begin(), v.end());
    for (const Vek& v : winkelReihe) r.B.insert(r.B.end(), v.begin(), v.end());
    return r.an(lage / buende);
}

void TafelSaite::verlustAn(double lage, std::vector<double>& b, std::vector<double>& a) const
{
    std::size_t k = 0;
    while (k + 2 < verlust.size() && verlust[k + 1].lage <= lage) ++k;
    const auto& v0 = verlust[k];
    const auto& v1 = verlust[std::min(k + 1, verlust.size() - 1)];
    const double t = v1.lage > v0.lage ? (lage - v0.lage) / (v1.lage - v0.lage) : 0.0;
    b.assign(std::max(v0.b.size(), v1.b.size()), 0.0);
    a.assign(std::max(v0.a.size(), v1.a.size()), 0.0);
    for (std::size_t i = 0; i < b.size(); ++i) b[i] = (1 - t) * (i < v0.b.size() ? v0.b[i] : 0.0) + t * (i < v1.b.size() ? v1.b[i] : 0.0);
    for (std::size_t i = 0; i < a.size(); ++i) a[i] = (1 - t) * (i < v0.a.size() ? v0.a[i] : 0.0) + t * (i < v1.a.size() ? v1.a[i] : 0.0);
}

namespace
{
// der groesste Fehler ueber Stellen (gs.zeile); nicht auswertbar, wenn eine Stelle nicht auswertbar ist
Bewertung zeile(const std::vector<Stelle>& stellen, const Reihe& reihe, int ordnung)
{
    std::vector<std::optional<Werte>> w(stellen.size());
    kern::parallelJe(stellen.size(), [&](std::size_t i) { w[i] = werteAn(stellen[i], reihe.an(stellen[i].x), ordnung); });
    Bewertung b;
    b.auswertbar = true;
    b.centGroesster = 0.0;
    b.abklingenGroesster = 0.0;
    for (const auto& x : w)
    {
        if (!x)
            return {};
        b.centGroesster = std::max(b.centGroesster, x->bew.centGroesster);
        b.abklingenGroesster = std::max(b.abklingenGroesster, x->bew.abklingenGroesster);
        b.zaehlend += x->bew.zaehlend;
        b.inSchwelle += x->bew.inSchwelle;
    }
    return b;
}

std::vector<Stelle> stellenVon(const Saitenwerte& sa, double mensur, int nb, const Vek& lagen, const std::vector<Stelle>* verlust)
{
    std::vector<Stelle> s(lagen.size());
    kern::parallelJe(lagen.size(), [&](std::size_t i) { s[i] = stelleVon(sa, mensur, lagen[i], nb, verlust); });
    return s;
}
}  // namespace

std::optional<TafelSaite> entwirfJeSaite(const Saitenwerte& saite, double mensur, int nb)
{
    Vek lb, lh, l8, l16;
    for (int k = 0; k <= nb; ++k) lb.push_back(k);
    for (int k = 0; k < nb; ++k) lh.push_back(k + 0.5);
    for (int k = 0; k < nb; ++k)
        for (double q : {0.125, 0.25, 0.375, 0.625, 0.75, 0.875}) l8.push_back(k + q);
    for (int k = 0; k < nb; ++k)
        for (int q = 1; q < 16; q += 2) l16.push_back(k + q / 16.0);
    const std::vector<Stelle> bund = stellenVon(saite, mensur, nb, lb, nullptr);
    const std::vector<Stelle> halb = stellenVon(saite, mensur, nb, lh, nullptr);
    for (const Stelle& s : bund)
        if (!s.gut) return std::nullopt;
    for (const Stelle& s : halb)
        if (!s.gut) return std::nullopt;
    std::vector<Stelle> st;
    st.insert(st.end(), bund.begin(), bund.end());
    st.insert(st.end(), halb.begin(), halb.end());
    std::sort(st.begin(), st.end(), [](const Stelle& a, const Stelle& b) { return a.lage < b.lage; });
    const std::vector<Stelle> achtel = stellenVon(saite, mensur, nb, l8, &st);
    int napMax = 0;
    for (const Stelle& s : bund) napMax = std::max(napMax, abelSmithZahl(s));
    const int M = std::max(8, napMax + 2);
    const int P = 3;
    std::vector<const Stelle*> zeiger;
    for (const Stelle& s : st) zeiger.push_back(&s);

    struct Kandidat
    {
        double guete;
        Reihe reihe;
        double fak, reg;
        Bewertung b, h, a;
    };
    std::optional<Kandidat> best;
    for (const auto& [fak, reg] : std::vector<std::pair<double, double>>{{1.02, 0.05}, {1.02, 0.1}, {1.3, 0.05}, {1.3, 0.1}})
    {
        Vek r0, th0;
        startVon(bund[0].rest, std::min(bund[0].fz.back() * fak, 19500.0), M, r0, th0);
        Reihe reihe;
        try
        {
            reihe = gebunden(zeiger, M, P, r0, th0, 5, reg);
        }
        catch (const std::exception&)
        {
            continue;
        }
        const Bewertung eb = zeile(bund, reihe, 5), eh = zeile(halb, reihe, 5), ea = zeile(achtel, reihe, 5);
        const double guete = (eb.auswertbar && eh.auswertbar && ea.auswertbar)
                                 ? std::max({eb.centGroesster, eh.centGroesster, ea.centGroesster})
                                 : std::numeric_limits<double>::infinity();
        if (!best || guete < best->guete) best = Kandidat{guete, reihe, fak, reg, eb, eh, ea};
    }
    if (!best) return std::nullopt;
    TafelSaite t;
    t.buende = nb;
    t.mensur = mensur;
    t.grad = P;
    for (int m = 0; m < M; ++m)
    {
        t.radiusReihe.emplace_back(best->reihe.A.begin() + m * (P + 1), best->reihe.A.begin() + (m + 1) * (P + 1));
        t.winkelReihe.emplace_back(best->reihe.B.begin() + m * (P + 1), best->reihe.B.begin() + (m + 1) * (P + 1));
    }
    for (const Stelle& s : st) t.verlust.push_back({s.lage, s.b, s.a});
    t.startFaktor = best->fak;
    t.daempfung = best->reg;
    t.buendeBewertung = best->b;
    t.halbBewertung = best->h;
    t.achtelBewertung = best->a;
    const std::vector<Stelle> fremd = stellenVon(saite, mensur, nb, l16, &st);
    t.fremdBewertung = zeile(fremd, best->reihe, 5);
    return t;
}

std::optional<Umlauffilter> filterAn(const TafelSaite& tafel, const Saitenwerte& saite, double lage, Bewertung* bewertung)
{
    Umlauffilter f;
    f.kette = tafel.ketteAn(lage);
    tafel.verlustAn(lage, f.b, f.a);
    const Bewertung b = bewerte(saite, tafel.mensur * std::exp2(-lage / 12.0), f);
    if (bewertung != nullptr) *bewertung = b;
    if (!b.auswertbar) return std::nullopt;
    return f;
}

}  // namespace lumi::messung::umlauf
