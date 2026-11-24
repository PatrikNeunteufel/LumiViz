/**
 * @file Umlauf.cpp
 * @brief InstrumentModels, gemeinsam: der Umlauf der Saite und sein Filter (UL3 a, UL3-1 a; 34k-7c)
 */
#include "InstrumentModels/gemeinsam/Umlauf.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace lumi::kern::umlauf
{

namespace
{
constexpr double kPi = std::numbers::pi;

double binom(int n, int k)
{
    double c = 1.0;
    for (int i = 1; i <= k; ++i) c = c * (n - k + i) / i;
    return c;
}
}  // namespace

std::complex<double> wellenzahl(const Saitenwerte& s, double omega)
{
    const double T = s.zug, m = s.masse, EI = s.biegesteife;
    const double k2 = EI > 0.0 ? (-T + std::sqrt(T * T + 4.0 * EI * m * omega * omega)) / (2.0 * EI) : omega * omega * m / T;
    const double biegung = EI * s.verlustBiegung * k2 / (T + EI * k2);
    double eta = 0.0;
    if (s.durchmesser > 0.0)
    {
        // Woodhouse & Lynch-Aird 2019 Gl. 11, 12, 14, 15; der Zuschlag (d + 0,2 mm) auf die Luft (Pruefstand S199)
        const double f = std::max(std::abs(omega) / (2.0 * kPi), 20.0);
        const double M = s.durchmesser / 4.0 * std::sqrt(2.0 * kPi * f / 1.5e-5);
        eta = (s.durchmesser * 1000.0 + 0.2) * 1.2 / s.dichte * (2.0 * std::sqrt(2.0) * M + 1.0) / (M * M) + biegung;
    }
    else
        eta = T * (s.verlustInnen + s.verlustLuft / omega) / (T + EI * k2) + biegung;
    return std::sqrt(k2) * std::complex<double>(1.0, -0.5 * eta);
}

Umlaufwert umlaufDerSaite(const Saitenwerte& saite, double laenge, double hz)
{
    const std::complex<double> k = wellenzahl(saite, 2.0 * kPi * hz);
    return {std::exp(2.0 * k.imag() * laenge), 2.0 * k.real() * laenge};
}

double verzugKette(const std::vector<Biquad>& kette, double w)
{
    double s = 0.0;
    for (const Biquad& q : kette)
    {
        const double r = q.radius, th = q.winkel;
        s += 2.0 * w + 2.0 * std::atan2(r * std::sin(w - th), 1.0 - r * std::cos(w - th)) +
             2.0 * std::atan2(r * std::sin(w + th), 1.0 - r * std::cos(w + th));
    }
    return s;
}

std::complex<double> antwort(const std::vector<double>& b, const std::vector<double>& a, double w)
{
    std::complex<double> zb = 0.0, za = 0.0;
    for (std::size_t k = 0; k < b.size(); ++k) zb += b[k] * std::polar(1.0, -static_cast<double>(k) * w);
    for (std::size_t k = 0; k < a.size(); ++k) za += a[k] * std::polar(1.0, -static_cast<double>(k) * w);
    return zb / za;
}

std::vector<double> thiranNenner(double D, int N)
{
    std::vector<double> a(static_cast<std::size_t>(N) + 1);
    a[0] = 1.0;  // das Produkt kuerzt sich zu 1 (bei D = N stuende 0/0 da)
    for (int k = 1; k <= N; ++k)
    {
        double p = 1.0;
        for (int n = 0; n <= N; ++n) p *= (D - N + n) / (D - N + k + n);
        a[static_cast<std::size_t>(k)] = ((k % 2) ? -1.0 : 1.0) * binom(N, k) * p;
    }
    return a;
}

double verzugThiran(double D, int N, double w)
{
    // H = z^(−N) Ā / A mit A(e^{iω}) = Σ a_k e^{−ikω}: arg H = −N ω − 2 arg A, also Verzug N ω + 2 arg A — stetig, weil
    // der Verzug des Glieds nahe D ω liegt und arg A ≈ (D − N) ω / 2 klein bleibt. (S220: die Abwicklung in einem
    // Schritt von 10⁻⁶ bis ω lag ueber rund 4,8 kHz um 2π daneben; die Vorlage wickelt ueber alle Teiltoene ab.)
    const std::vector<double> a = thiranNenner(D, N);
    std::complex<double> s = 0.0;
    for (std::size_t k = 0; k < a.size(); ++k) s += a[k] * std::polar(1.0, -static_cast<double>(k) * w);
    return N * w + 2.0 * std::arg(s);
}

std::complex<double> Umlauffilter::antwort(double w) const
{
    std::complex<double> h = umlauf::antwort(b, a, w);
    const std::complex<double> e = std::polar(1.0, -w);
    for (const Biquad& q : kette)
    {
        const double c = q.radius * std::cos(q.winkel), r2 = q.radius * q.radius;
        h *= (r2 - 2.0 * c * e + e * e) / (1.0 - 2.0 * c * e + r2 * e * e);
    }
    const std::vector<double> an = thiranNenner(gliedVerzug, gliedOrdnung);
    const std::vector<double> bn(an.rbegin(), an.rend());
    return h * umlauf::antwort(bn, an, w);
}

bool stimmeGlied(Umlauffilter& f, double w1, double soll)
{
    const double lo = verzugUnten(f.gliedOrdnung);
    if (soll / w1 < lo + 1.0) return false;
    const int N = static_cast<int>(std::floor(soll / w1 - lo));
    const double ziel = soll - w1 * N;
    double D = soll / w1 - N;
    for (int i = 0; i < 8; ++i)
    {
        const double g0 = verzugThiran(D, f.gliedOrdnung, w1) - ziel;
        const double ab = (verzugThiran(D + 1e-6, f.gliedOrdnung, w1) - verzugThiran(D - 1e-6, f.gliedOrdnung, w1)) / 2e-6;
        D -= g0 / ab;
    }
    f.verzugGanz = N;
    f.gliedVerzug = D;
    return true;
}

Iir::Iir(std::vector<double> b, std::vector<double> a) : m_b(std::move(b)), m_a(std::move(a))
{
    const std::size_t n = std::max(m_b.size(), m_a.size());
    m_b.resize(n, 0.0);
    m_a.resize(n, 0.0);
    const double a0 = m_a[0];
    for (double& v : m_b) v /= a0;
    for (double& v : m_a) v /= a0;
    m_z.assign(n, 0.0);
}

double Iir::schritt(double x)
{
    const std::size_t n = m_b.size();
    if (n == 0) return x;
    const double y = m_b[0] * x + m_z[0];
    for (std::size_t k = 1; k < n; ++k) m_z[k - 1] = m_b[k] * x - m_a[k] * y + (k < n - 1 ? m_z[k] : 0.0);
    return y;
}

void Iir::leere() { std::fill(m_z.begin(), m_z.end(), 0.0); }

Umlauflauf::Umlauflauf(const Umlauffilter& f) : m_verlust(f.b, f.a)
{
    for (const Biquad& q : f.kette)
    {
        const double c = q.radius * std::cos(q.winkel), r2 = q.radius * q.radius;
        m_kette.emplace_back(std::vector<double>{r2, -2.0 * c, 1.0}, std::vector<double>{1.0, -2.0 * c, r2});
    }
    const std::vector<double> a = thiranNenner(f.gliedVerzug, f.gliedOrdnung);
    m_glied = Iir(std::vector<double>(a.rbegin(), a.rend()), a);
}

double Umlauflauf::schritt(double x)
{
    double y = m_verlust.schritt(x);
    for (Iir& q : m_kette) y = q.schritt(y);
    return m_glied.schritt(y);
}

void Umlauflauf::leere()
{
    m_verlust.leere();
    for (Iir& q : m_kette) q.leere();
    m_glied.leere();
}

}  // namespace lumi::kern::umlauf
