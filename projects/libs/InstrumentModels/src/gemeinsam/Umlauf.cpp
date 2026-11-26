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

// ---------------------------------------------------------------- zur Laufzeit (34k-7d-3)
// Rechnet Schritt fuer Schritt wie der Entwurf (`InstrumentMeasure/UmlaufEntwurf.cpp`: `teiltoeneVon`, `setzeVerlust`,
// `werteAn`), nur an den Stellen des Rasters, die er braucht — dieselben Zahlen Bit fuer Bit.

std::vector<double> verzugVon(const std::vector<std::complex<double>>& h)
{
    std::vector<double> p(h.size());
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

double grundtonDerSaite(const Saitenwerte& saite, double laenge)
{
    // np.interp(2π, verzug, F): der Verzug waechst laengs des Rasters; ausserhalb der Randwert
    const double x = 2.0 * kPi;
    double vor = umlaufDerSaite(saite, laenge, rasterHz(0)).verzug;
    if (x <= vor) return rasterHz(0);
    for (std::size_t i = 1; i < kRaster; ++i)
    {
        const double v = umlaufDerSaite(saite, laenge, rasterHz(i)).verzug;
        if (v > x)
        {
            const double d = v - vor;
            return d == 0.0 ? rasterHz(i) : rasterHz(i - 1) + (x - vor) * (rasterHz(i) - rasterHz(i - 1)) / d;
        }
        vor = v;
    }
    return vor >= x ? rasterHz(kRaster - 1) : 0.0;  // genau am Rand, sonst kein Grundton bis 20 kHz
}

bool stimmeAmGrundton(Umlauffilter& filter, const Saitenwerte& saite, double laenge)
{
    const double f1 = grundtonDerSaite(saite, laenge);
    if (!(f1 > 0.0)) return false;
    // interpF(f1, ·) auf dem Raster: die Stellen j, j + 1 und der Bruchteil u (am Rand der Randwert)
    const double t = (f1 - 1.0) / 0.5;
    std::size_t j = 0, k = 0;
    double u = 0.0;
    if (t <= 0.0)
        j = k = 0;
    else if (t >= static_cast<double>(kRaster - 1))
        j = k = kRaster - 1;
    else
    {
        j = static_cast<std::size_t>(t);
        k = j + 1;
        u = t - static_cast<double>(j);
    }
    // der Verzug des Verlusts, abgewickelt vom Anfang des Rasters bis k
    std::vector<std::complex<double>> hl(k + 1);
    for (std::size_t i = 0; i <= k; ++i) hl[i] = antwort(filter.b, filter.a, omegaVon(rasterHz(i)));
    const std::vector<double> vv = verzugVon(hl);
    const auto rest = [&](std::size_t i) { return umlaufDerSaite(saite, laenge, rasterHz(i)).verzug - vv[i]; };
    const auto kette = [&](std::size_t i) { return verzugKette(filter.kette, omegaVon(rasterHz(i))); };
    const double r = j == k ? rest(j) : rest(j) + u * (rest(k) - rest(j));
    const double kv = j == k ? kette(j) : kette(j) + u * (kette(k) - kette(j));
    return stimmeGlied(filter, omegaVon(f1), r - kv);
}

double sigmoid(double x) { return 1.0 / (1.0 + std::exp(-std::clamp(x, -kLogitGrenze, kLogitGrenze))); }

std::vector<double> legendre(double t, int grad)
{
    std::vector<double> v(static_cast<std::size_t>(grad) + 1);
    v[0] = 1.0;
    if (grad >= 1) v[1] = t;
    for (int j = 2; j <= grad; ++j)
        v[static_cast<std::size_t>(j)] = ((2.0 * j - 1.0) * t * v[static_cast<std::size_t>(j - 1)] - (j - 1.0) * v[static_cast<std::size_t>(j - 2)]) / j;
    return v;
}

std::vector<Biquad> Saitentafel::ketteAn(double lage) const
{
    const std::vector<double> v = legendre(2.0 * (lage / buende) - 1.0, grad);
    std::vector<Biquad> k(radiusReihe.size());
    for (std::size_t m = 0; m < radiusReihe.size(); ++m)
    {
        double sa = 0.0, sb = 0.0;
        for (int j = 0; j <= grad; ++j)
        {
            sa += radiusReihe[m][static_cast<std::size_t>(j)] * v[static_cast<std::size_t>(j)];
            sb += winkelReihe[m][static_cast<std::size_t>(j)] * v[static_cast<std::size_t>(j)];
        }
        k[m] = {kRadiusMax * sigmoid(sa), kPi * sigmoid(sb)};
    }
    return k;
}

void Saitentafel::verlustAn(double lage, std::vector<double>& b, std::vector<double>& a) const
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

std::optional<Umlauffilter> filterAn(const Saitentafel& tafel, const Saitenwerte& saite, double lage)
{
    if (tafel.verlust.empty() || tafel.buende <= 0) return std::nullopt;
    Umlauffilter f;
    f.kette = tafel.ketteAn(lage);
    tafel.verlustAn(lage, f.b, f.a);
    if (!stimmeAmGrundton(f, saite, tafel.mensur * std::exp2(-lage / 12.0))) return std::nullopt;
    return f;
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
