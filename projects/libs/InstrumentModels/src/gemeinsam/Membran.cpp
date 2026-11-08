/**
 * @file Membran.cpp
 * @brief InstrumentModels, gemeinsam: Moden kreisrunder Felle mit Luftlast und Kessel; Herleitung und Herkunft im Kopf
 *        von `Membran.hpp`
 */
#include "InstrumentModels/gemeinsam/Membran.hpp"

#include "InstrumentCore/Eigenzerlegung.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace lumi::kern
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr double kRho = 1.2041;
constexpr double kC = 343.0;

double jm(int m, double x)
{
    return std::cyl_bessel_j(static_cast<double>(m), x);
}

/// W_n(κ) = ∫₀^a J_m(α r)·J_m(κ r)·r dr mit α = j/a, J_m(j) = 0 (Lommel).
double hankel(int m, double j, double a, double kappa)
{
    const double alpha = j / a;
    const double jStrich = -jm(m + 1, j);  // J_m′(j) an der Nullstelle
    if (std::abs(alpha - kappa) < 1e-7 * alpha) return 0.5 * a * a * jStrich * jStrich;
    return -a * alpha * jStrich * jm(m, kappa * a) / (alpha * alpha - kappa * kappa);
}

/// Cholesky A = L·Lᵀ (n × n, zeilenweise), L unten; falsch, wenn nicht positiv.
bool cholesky(int n, const std::vector<double>& a, std::vector<double>& l)
{
    l.assign(static_cast<std::size_t>(n * n), 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j <= i; ++j)
        {
            double s = a[static_cast<std::size_t>(i * n + j)];
            for (int k = 0; k < j; ++k) s -= l[static_cast<std::size_t>(i * n + k)] * l[static_cast<std::size_t>(j * n + k)];
            if (i == j)
            {
                if (s <= 0.0) return false;
                l[static_cast<std::size_t>(i * n + i)] = std::sqrt(s);
            }
            else
                l[static_cast<std::size_t>(i * n + j)] = s / l[static_cast<std::size_t>(j * n + j)];
        }
    return true;
}

/// K·x = λ·M·x (symmetrisch, M positiv): Werte aufsteigend, Vektoren in Spalten (zeilenweise n × n).
SymmetrischeZerlegung verallgemeinert(int n, const std::vector<double>& k, const std::vector<double>& mm)
{
    std::vector<double> l;
    SymmetrischeZerlegung leer;
    if (!cholesky(n, mm, l)) return leer;
    // B = L⁻¹·K·L⁻ᵀ
    std::vector<double> y(static_cast<std::size_t>(n * n), 0.0);  // Y = L⁻¹·K
    for (int c = 0; c < n; ++c)
        for (int i = 0; i < n; ++i)
        {
            double s = k[static_cast<std::size_t>(i * n + c)];
            for (int j = 0; j < i; ++j) s -= l[static_cast<std::size_t>(i * n + j)] * y[static_cast<std::size_t>(j * n + c)];
            y[static_cast<std::size_t>(i * n + c)] = s / l[static_cast<std::size_t>(i * n + i)];
        }
    std::vector<double> b(static_cast<std::size_t>(n * n), 0.0);  // B = Y·L⁻ᵀ = (L⁻¹·Yᵀ)ᵀ
    for (int r = 0; r < n; ++r)
        for (int i = 0; i < n; ++i)
        {
            double s = y[static_cast<std::size_t>(r * n + i)];
            for (int j = 0; j < i; ++j) s -= l[static_cast<std::size_t>(i * n + j)] * b[static_cast<std::size_t>(r * n + j)];
            b[static_cast<std::size_t>(r * n + i)] = s / l[static_cast<std::size_t>(i * n + i)];
        }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < i; ++j)
        {
            const double m = 0.5 * (b[static_cast<std::size_t>(i * n + j)] + b[static_cast<std::size_t>(j * n + i)]);
            b[static_cast<std::size_t>(i * n + j)] = b[static_cast<std::size_t>(j * n + i)] = m;
        }
    SymmetrischeZerlegung z = zerlegeSymmetrisch(n, b);
    // x = L⁻ᵀ·v
    std::vector<double> v = z.vektoren;
    for (int c = 0; c < n; ++c)
        for (int i = n - 1; i >= 0; --i)
        {
            double s = v[static_cast<std::size_t>(i * n + c)];
            for (int j = i + 1; j < n; ++j) s -= l[static_cast<std::size_t>(j * n + i)] * z.vektoren[static_cast<std::size_t>(j * n + c)];
            z.vektoren[static_cast<std::size_t>(i * n + c)] = s / l[static_cast<std::size_t>(i * n + i)];
        }
    return z;
}

/// Die Luft einer Seite bei ω fuer die Basis einer Ordnung m: Masse und Widerstand (N × N).
void luft(int m, const std::vector<double>& nullen, double a, double omega, std::vector<double>& masse, std::vector<double>& widerstand)
{
    const int n = static_cast<int>(nullen.size());
    masse.assign(static_cast<std::size_t>(n * n), 0.0);
    widerstand.assign(static_cast<std::size_t>(n * n), 0.0);
    const double am = m == 0 ? 2.0 * kPi : kPi;
    const double k0 = std::max(omega / kC, 1e-6);
    std::vector<double> jStrich(static_cast<std::size_t>(n)), alpha(static_cast<std::size_t>(n)), w(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
    {
        jStrich[static_cast<std::size_t>(i)] = -jm(m + 1, nullen[static_cast<std::size_t>(i)]);
        alpha[static_cast<std::size_t>(i)] = nullen[static_cast<std::size_t>(i)] / a;
    }
    // W_n(κ) mit einer Bessel-Auswertung je Stuetzstelle
    const auto fuelle = [&](double kappa) {
        const double jk = jm(m, kappa * a);
        for (int i = 0; i < n; ++i)
        {
            const double al = alpha[static_cast<std::size_t>(i)];
            const double js = jStrich[static_cast<std::size_t>(i)];
            w[static_cast<std::size_t>(i)] = std::abs(al - kappa) < 1e-7 * al ? 0.5 * a * a * js * js : -a * al * js * jk / (al * al - kappa * kappa);
        }
    };
    const auto addiere = [&](std::vector<double>& ziel, double g) {
        for (int i = 0; i < n; ++i)
        {
            const double wi = g * w[static_cast<std::size_t>(i)];
            for (int j = 0; j < n; ++j) ziel[static_cast<std::size_t>(i * n + j)] += wi * w[static_cast<std::size_t>(j)];
        }
    };
    // κ < k₀: κ = k₀·sin θ (der Widerstand, ohne Singularitaet)
    constexpr int kTheta = 120;
    const double dTheta = 0.5 * kPi / kTheta;
    for (int t = 0; t < kTheta; ++t)
    {
        const double kappa = k0 * std::sin((t + 0.5) * dTheta);
        fuelle(kappa);
        addiere(widerstand, am * kRho * omega * kappa * dTheta);
    }
    // κ > k₀: κ = k₀·cosh u (die Masse, ohne Singularitaet)
    const double kMax = std::max(6.0 * nullen.back() / a, 4.0 * k0);
    const double uMax = std::acosh(kMax / k0);
    constexpr int kU = 1500;
    const double dU = uMax / kU;
    for (int t = 0; t < kU; ++t)
    {
        const double kappa = k0 * std::cosh((t + 0.5) * dU);
        fuelle(kappa);
        addiere(masse, am * kRho * kappa * dU);
    }
}

}  // namespace

double besselNull(int m, int k)
{
    double x = 0.5 + m, vor = jm(m, x);
    int gefunden = 0;
    while (true)
    {
        const double neu = x + 0.05;
        const double jn = jm(m, neu);
        if ((vor < 0.0) != (jn < 0.0))
        {
            if (++gefunden == k)
            {
                double lo = x, hi = neu;
                for (int i = 0; i < 60; ++i)
                {
                    const double mid = 0.5 * (lo + hi);
                    if ((jm(m, mid) < 0.0) == (vor < 0.0))
                        lo = mid;
                    else
                        hi = mid;
                }
                return 0.5 * (lo + hi);
            }
        }
        x = neu;
        vor = jn;
    }
}

namespace
{

/// Die Matrizen einer Ordnung m mit `nb` Funktionen je Fell.
struct Ordnung
{
    int m = 0, fz = 1, nb = 0, n = 0;
    std::vector<std::vector<double>> nullen;
    std::vector<double> kk, mm, cc;
    std::vector<double> tafel;  ///< J_m(j_i·r/200), r = 0 … 200, zeilenweise (201 × nb)
};

Ordnung baueOrdnung(const std::vector<Fell>& felle, const Kessel& kessel, int m, int nb)
{
    Ordnung o;
    o.m = m;
    o.fz = static_cast<int>(std::min<std::size_t>(2, felle.size()));
    o.nb = nb;
    o.n = o.fz * nb;
    const int n = o.n;
    const double am = m == 0 ? 2.0 * kPi : kPi;
    o.nullen.resize(static_cast<std::size_t>(o.fz));
    for (int f = 0; f < o.fz; ++f)
        for (int k = 1; k <= nb; ++k) o.nullen[static_cast<std::size_t>(f)].push_back(besselNull(m, k));
    o.tafel.resize(static_cast<std::size_t>(201 * nb));
    for (int r = 0; r <= 200; ++r)
        for (int i = 0; i < nb; ++i) o.tafel[static_cast<std::size_t>(r * nb + i)] = jm(m, o.nullen.front()[static_cast<std::size_t>(i)] * r / 200.0);
    o.kk.assign(static_cast<std::size_t>(n * n), 0.0);
    o.mm.assign(static_cast<std::size_t>(n * n), 0.0);
    o.cc.assign(static_cast<std::size_t>(n * n), 0.0);
    std::vector<double> g(static_cast<std::size_t>(n), 0.0);
    for (int f = 0; f < o.fz; ++f)
    {
        const Fell& fe = felle[static_cast<std::size_t>(f)];
        const double a = fe.radiusM;
        for (int i = 0; i < nb; ++i)
        {
            const double ji = o.nullen[static_cast<std::size_t>(f)][static_cast<std::size_t>(i)];
            const double norm = 0.5 * a * a * std::pow(jm(m + 1, ji), 2);
            const int ii = f * nb + i;
            o.kk[static_cast<std::size_t>(ii * n + ii)] = fe.spannungNm * am * (ji / a) * (ji / a) * norm;
            o.cc[static_cast<std::size_t>(ii * n + ii)] = am * norm * (fe.d1 + fe.d3 * (ji / a) * (ji / a));
            if (m == 0) g[static_cast<std::size_t>(ii)] = 2.0 * kPi * a * a * jm(1, ji) / ji;
            for (int j = 0; j < nb; ++j)
            {
                const double jj = o.nullen[static_cast<std::size_t>(f)][static_cast<std::size_t>(j)];
                double sum = 0.0;
                if (fe.belag)
                {
                    constexpr int kR = 400;
                    for (int r = 0; r < kR; ++r)
                    {
                        const double x = (r + 0.5) / kR;
                        sum += fe.belag(x) * jm(m, ji * x) * jm(m, jj * x) * x;
                    }
                    sum *= a * a / kR;
                }
                else if (i == j)
                    sum = norm;
                o.mm[static_cast<std::size_t>(ii * n + f * nb + j)] = fe.dichteKgM2 * am * sum;
            }
        }
    }
    // der Kessel (m = 0): die Luftfeder ueber dasselbe Volumen
    if (m == 0 && kessel.volumenM3 > 0.0)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                o.kk[static_cast<std::size_t>(i * n + j)] += kRho * kC * kC / kessel.volumenM3 * g[static_cast<std::size_t>(i)] * g[static_cast<std::size_t>(j)];
    return o;
}

/// Die Mode `idx` der Ordnung `o` bei ω (mit Luft, wenn `mitLuft`): fertig ausgewertet.
FellMode werteAus(const std::vector<Fell>& felle, const Ordnung& o, int idx, bool mitLuft, double omegaStart, const SymmetrischeZerlegung& ohneLuft)
{
    const int n = o.n, nb = o.nb, fz = o.fz, m = o.m;
    const double am = m == 0 ? 2.0 * kPi : kPi;
    double omega = omegaStart;
    std::vector<double> mges = o.mm, rges(static_cast<std::size_t>(n * n), 0.0);
    SymmetrischeZerlegung z = ohneLuft;
    for (int runde = 0; mitLuft && runde < 6; ++runde)
    {
        mges = o.mm;
        std::fill(rges.begin(), rges.end(), 0.0);
        for (int f = 0; f < fz; ++f)
        {
            const Fell& fe = felle[static_cast<std::size_t>(f)];
            if (!fe.abstrahlung && fe.innenLuft <= 0.0) continue;
            std::vector<double> ml, rl;
            luft(m, o.nullen[static_cast<std::size_t>(f)], fe.radiusM, omega, ml, rl);
            const double faktor = (fe.abstrahlung ? 1.0 : 0.0) + fe.innenLuft;
            for (int i = 0; i < nb; ++i)
                for (int j = 0; j < nb; ++j)
                {
                    mges[static_cast<std::size_t>((f * nb + i) * n + f * nb + j)] += faktor * ml[static_cast<std::size_t>(i * nb + j)];
                    if (fe.abstrahlung) rges[static_cast<std::size_t>((f * nb + i) * n + f * nb + j)] += rl[static_cast<std::size_t>(i * nb + j)];
                }
        }
        z = verallgemeinert(n, o.kk, mges);
        if (z.werte.empty()) break;
        const double neu = std::sqrt(std::max(0.0, z.werte[static_cast<std::size_t>(idx)]));
        const bool ruhig = std::abs(neu - omega) < 1e-4 * omega;
        omega = neu;
        if (ruhig) break;
    }
    FellMode fm;
    if (z.werte.empty()) return fm;
    omega = std::sqrt(std::max(0.0, z.werte[static_cast<std::size_t>(idx)]));
    fm.m = m;
    fm.felle = fz;
    fm.hz = omega / (2.0 * kPi);
    fm.nullen = o.nullen.front();
    fm.koeffizienten.resize(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) fm.koeffizienten[static_cast<std::size_t>(i)] = z.vektoren[static_cast<std::size_t>(i * n + idx)];
    // Normierung: der groesste Betrag der Form auf 1 (positiv)
    const auto form = [&](int r, int f) {
        double w = 0.0;
        for (int i = 0; i < nb; ++i) w += fm.koeffizienten[static_cast<std::size_t>(f * nb + i)] * o.tafel[static_cast<std::size_t>(r * nb + i)];
        return w;
    };
    double groesst = 0.0, vorz = 1.0;
    for (int f = 0; f < fz; ++f)
        for (int r = 0; r <= 200; ++r)
        {
            const double w = form(r, f);
            if (std::abs(w) > groesst)
            {
                groesst = std::abs(w);
                vorz = w < 0.0 ? -1.0 : 1.0;
            }
        }
    for (double& x : fm.koeffizienten) x *= vorz / std::max(groesst, 1e-300);
    // die Ordnung n: Vorzeichenwechsel in r auf dem staerkeren Fell, + 1
    int staerker = 0;
    double best = 0.0;
    for (int f = 0; f < fz; ++f)
    {
        double e = 0.0;
        for (int i = 0; i < nb; ++i) e += std::pow(fm.koeffizienten[static_cast<std::size_t>(f * nb + i)], 2);
        if (e > best)
        {
            best = e;
            staerker = f;
        }
    }
    int wechsel = 0;
    double vor = form(0, staerker);
    for (int r = 1; r < 200; ++r)
    {
        const double w = form(r, staerker);
        if (std::abs(w) > 1e-6)
        {
            if ((w < 0.0) != (vor < 0.0)) ++wechsel;
            vor = w;
        }
    }
    fm.n = wechsel + 1;
    const auto quad = [&](const std::vector<double>& mat) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j) sum += fm.koeffizienten[static_cast<std::size_t>(i)] * mat[static_cast<std::size_t>(i * n + j)] * fm.koeffizienten[static_cast<std::size_t>(j)];
        return sum;
    };
    fm.masseKg = quad(mges);
    const double sigma = (quad(o.cc) + quad(rges)) / (2.0 * fm.masseKg);
    fm.t60S = std::log(1000.0) / std::max(sigma, 1e-6);
    // die Fernfeld-Staerke unter 45°: Σ Koeffizient·W(k₀·sin 45°) ueber beide Felle (beide strahlen nach aussen)
    double s45 = 0.0;
    for (int f = 0; f < fz; ++f)
        for (int i = 0; i < nb; ++i)
            s45 += fm.koeffizienten[static_cast<std::size_t>(f * nb + i)] *
                   hankel(m, o.nullen[static_cast<std::size_t>(f)][static_cast<std::size_t>(i)], felle[static_cast<std::size_t>(f)].radiusM, omega / kC * std::sqrt(0.5));
    fm.abstrahlung = am * s45;
    return fm;
}

int basisBis(const std::vector<Fell>& felle, int m, double hz, int hoechstens)
{
    int nb = 0;
    for (std::size_t f = 0; f < std::min<std::size_t>(2, felle.size()); ++f)
    {
        const double c = std::sqrt(felle[f].spannungNm / felle[f].dichteKgM2);
        int k = 0;
        while (k < hoechstens && besselNull(m, k + 1) / (2.0 * kPi * felle[f].radiusM) * c < hz) ++k;
        nb = std::max(nb, k);
    }
    return nb;
}

}  // namespace

std::vector<FellMode> fellModen(const std::vector<Fell>& felle, const Kessel& kessel, double bisHz, double luftBisHz, int basis)
{
    std::vector<FellMode> aus;
    if (felle.empty()) return aus;
    for (int m = 0;; ++m)
    {
        const int nbGross = basisBis(felle, m, 1.3 * bisHz, basis);
        if (nbGross == 0) break;
        // unten (ohne Luft unter `luftBisHz`): kleine Basis mit Luft, je Mode iteriert
        const int nbKlein = std::min(basis, std::max(4, basisBis(felle, m, 1.3 * luftBisHz, basis) + 3));
        const Ordnung klein = baueOrdnung(felle, kessel, m, nbKlein);
        const SymmetrischeZerlegung z0 = verallgemeinert(klein.n, klein.kk, klein.mm);
        for (int idx = 0; idx < klein.n && !z0.werte.empty(); ++idx)
        {
            const double omega = std::sqrt(std::max(0.0, z0.werte[static_cast<std::size_t>(idx)]));
            if (omega / (2.0 * kPi) >= luftBisHz) break;
            FellMode fm = werteAus(felle, klein, idx, true, omega, z0);
            if (fm.hz > 0.0 && fm.hz <= bisHz) aus.push_back(fm);
        }
        // oben: grosse Basis ohne Luftmasse (Abstrahlung nur aus der Form, Daempfung innen)
        if (luftBisHz < bisHz)
        {
            const Ordnung gross = baueOrdnung(felle, kessel, m, std::max(nbGross + 2, 4));
            const SymmetrischeZerlegung zg = verallgemeinert(gross.n, gross.kk, gross.mm);
            for (int idx = 0; idx < gross.n && !zg.werte.empty(); ++idx)
            {
                const double omega = std::sqrt(std::max(0.0, zg.werte[static_cast<std::size_t>(idx)]));
                const double hz = omega / (2.0 * kPi);
                if (hz < luftBisHz) continue;
                if (hz > bisHz) break;
                aus.push_back(werteAus(felle, gross, idx, false, omega, zg));
            }
        }
    }
    std::sort(aus.begin(), aus.end(), [](const FellMode& x, const FellMode& y) { return x.hz < y.hz; });
    return aus;
}

double formAn(const FellMode& mode, double rRel, int fell)
{
    const int nb = static_cast<int>(mode.koeffizienten.size()) / std::max(1, mode.felle);
    double w = 0.0;
    for (int i = 0; i < nb && i < static_cast<int>(mode.nullen.size()); ++i)
        w += mode.koeffizienten[static_cast<std::size_t>(fell * nb + i)] * jm(mode.m, mode.nullen[static_cast<std::size_t>(i)] * rRel);
    return w;
}

}  // namespace lumi::kern
