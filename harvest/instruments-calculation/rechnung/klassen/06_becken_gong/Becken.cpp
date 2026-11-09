/**
 * @file Becken.cpp
 * @brief Klasse 6: der Schlag auf ein Becken oder einen Gong (Herleitung im Kopf von `Becken.hpp`)
 */
#include "klassen/06_becken_gong/Becken.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic::becken
{

namespace
{

constexpr double kPi = std::numbers::pi;

double jm(int m, double x) { return std::cyl_bessel_j(static_cast<double>(m), x); }
double im(int m, double x) { return std::cyl_bessel_i(static_cast<double>(m), x); }
double j1(int m, double x) { return m == 0 ? -jm(1, x) : jm(m - 1, x) - m / x * jm(m, x); }
double i1(int m, double x) { return m == 0 ? im(1, x) : im(m - 1, x) - m / x * im(m, x); }
double j2(int m, double x) { return -j1(m, x) / x - (1.0 - double(m * m) / (x * x)) * jm(m, x); }
double i2(int m, double x) { return -i1(m, x) / x + (1.0 + double(m * m) / (x * x)) * im(m, x); }

double winkelFaktor(int m) { return m == 0 ? 2.0 * kPi : kPi; }

/// Die Determinante der freien Randbedingungen (M_r = V_r = 0 bei ρ = 1), durch I_m(λ) geteilt.
double freiDet(int m, double l, double nu, double* c = nullptr)
{
    const double J = jm(m, l), Jd = j1(m, l), Jdd = j2(m, l);
    const double I = im(m, l), Id = i1(m, l), Idd = i2(m, l);
    const double mm = m * m;
    const double mJ = l * l * Jdd + nu * (l * Jd - mm * J);
    const double vJ = -l * l * l * Jd - (1.0 - nu) * mm * (l * Jd - J);
    const double mI = l * l * Idd + nu * (l * Id - mm * I);
    const double vI = l * l * l * Id - (1.0 - nu) * mm * (l * Id - I);
    if (c != nullptr) *c = -mJ / mI;
    return (mJ * vI - mI * vJ) / I;
}

/// Die eingespannte Determinante (Airy-Funktionen): J_m·I_m′ − I_m·J_m′, durch I_m geteilt.
double klemmDet(int m, double z)
{
    return (jm(m, z) * i1(m, z) - im(m, z) * j1(m, z)) / im(m, z);
}

/// Nullstellen einer Funktion in (von, bis] mit Schritt `schritt` und Halbierung.
template <class F>
std::vector<double> nullstellen(F f, double von, double bis, double schritt)
{
    std::vector<double> aus;
    double x = von, fx = f(x);
    while (x < bis)
    {
        const double y = x + schritt, fy = f(y);
        if ((fx < 0.0) != (fy < 0.0) && std::isfinite(fx) && std::isfinite(fy))
        {
            double lo = x, hi = y, flo = fx;
            for (int i = 0; i < 60; ++i)
            {
                const double mid = 0.5 * (lo + hi), fm = f(mid);
                if ((fm < 0.0) == (flo < 0.0))
                {
                    lo = mid;
                    flo = fm;
                }
                else
                    hi = mid;
            }
            aus.push_back(0.5 * (lo + hi));
        }
        x = y;
        fx = fy;
    }
    return aus;
}

/// Gauss-Legendre auf [0, 1].
void gaussLegendre(int n, std::vector<double>& x, std::vector<double>& w)
{
    x.resize(static_cast<std::size_t>(n));
    w.resize(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
    {
        double z = std::cos(kPi * (i + 0.75) / (n + 0.5)), pp = 0.0;
        for (int it = 0; it < 100; ++it)
        {
            double p1 = 1.0, p2 = 0.0;
            for (int j = 1; j <= n; ++j)
            {
                const double p3 = p2;
                p2 = p1;
                p1 = ((2.0 * j - 1.0) * z * p2 - (j - 1.0) * p3) / j;
            }
            pp = n * (z * p1 - p2) / (z * z - 1.0);
            const double z1 = z;
            z = z1 - p1 / pp;
            if (std::abs(z - z1) < 1e-15) break;
        }
        x[static_cast<std::size_t>(i)] = 0.5 * (1.0 - z);
        w[static_cast<std::size_t>(i)] = 1.0 / ((1.0 - z * z) * pp * pp);
    }
}

double winkel(int d, int c)
{
    return d == c ? (d == 0 ? 2.0 * kPi : kPi) : 0.0;
}

/// Eine radiale Funktion mit Ableitungen an den Stuetzstellen.
struct Radial
{
    std::vector<double> f, d1, d2;
};

Radial radialPlatte(const PlattenMode& pm, const std::vector<double>& x)
{
    Radial r;
    for (const double rho : x)
    {
        const double y = pm.lambda * rho;
        r.f.push_back(pm.norm * (jm(pm.m, y) + pm.c * im(pm.m, y)));
        r.d1.push_back(pm.norm * pm.lambda * (j1(pm.m, y) + pm.c * i1(pm.m, y)));
        r.d2.push_back(pm.norm * pm.lambda * pm.lambda * (j2(pm.m, y) + pm.c * i2(pm.m, y)));
    }
    return r;
}

}  // namespace

std::vector<PlattenMode> freiePlatte(double nu, double lambdaMax)
{
    std::vector<PlattenMode> aus;
    for (int m = 0;; ++m)
    {
        const std::vector<double> l = nullstellen([&](double x) { return freiDet(m, x, nu); }, std::max(0.5, 0.8 * m), lambdaMax, 0.01);
        if (l.empty()) break;
        for (const double lam : l)
        {
            PlattenMode pm;
            pm.m = m;
            pm.lambda = lam;
            freiDet(m, lam, nu, &pm.c);
            // ∫₀¹ W²ρ dρ mit Gauss
            std::vector<double> x, w;
            gaussLegendre(120, x, w);
            double s = 0.0;
            for (std::size_t i = 0; i < x.size(); ++i)
            {
                const double v = jm(m, lam * x[i]) + pm.c * im(m, lam * x[i]);
                s += w[i] * v * v * x[i];
            }
            pm.norm = 1.0 / std::sqrt(winkelFaktor(m) * s);
            aus.push_back(pm);
        }
    }
    std::sort(aus.begin(), aus.end(), [](const PlattenMode& a, const PlattenMode& b) { return a.lambda < b.lambda; });
    return aus;
}

void bereite(BeckenSpiel& s)
{
    auto k = std::make_shared<Kopplung>();
    const double steife = s.eModul * std::pow(s.dickeM, 3) / (12.0 * (1.0 - s.nu * s.nu));
    const double kappa = std::sqrt(steife / (s.dichte * s.dickeM));
    // λ_max aus bisHz: ω = (λ/a)²·κ
    const double lambdaMax = std::sqrt(2.0 * kPi * s.bisHz / kappa) * s.radiusM;
    k->moden = freiePlatte(s.nu, lambdaMax);
    for (PlattenMode& pm : k->moden) pm.hz = std::pow(pm.lambda / s.radiusM, 2) * kappa / (2.0 * kPi);
    int mMax = 0;
    for (const PlattenMode& pm : k->moden) mMax = std::max(mMax, pm.m);
    // die Spannungsmoden: eingespannt, m = 0 … 2·mMax, die `spannungsModen` kleinsten ζ
    struct Spannung
    {
        int m;
        double zeta, norm, c;
    };
    std::vector<Spannung> sp;
    for (int m = 0; m <= 2 * mMax; ++m)
        for (const double z : nullstellen([&](double x) { return klemmDet(m, x); }, std::max(0.5, 0.8 * m), 2.0 * lambdaMax + 10.0, 0.01))
            sp.push_back({m, z, 1.0, -jm(m, z) / im(m, z)});
    std::sort(sp.begin(), sp.end(), [](const Spannung& a, const Spannung& b) { return a.zeta < b.zeta; });
    if (static_cast<int>(sp.size()) > s.spannungsModen) sp.resize(static_cast<std::size_t>(s.spannungsModen));
    std::vector<double> x, w;
    gaussLegendre(160, x, w);
    std::vector<std::vector<double>> phi(sp.size());
    for (std::size_t n = 0; n < sp.size(); ++n)
    {
        double sum = 0.0;
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            const double v = jm(sp[n].m, sp[n].zeta * x[i]) + sp[n].c * im(sp[n].m, sp[n].zeta * x[i]);
            phi[n].push_back(v);
            sum += w[i] * v * v * x[i];
        }
        sp[n].norm = 1.0 / std::sqrt(winkelFaktor(sp[n].m) * sum);
        for (double& v : phi[n]) v *= sp[n].norm;
        k->zeta4.push_back(std::pow(sp[n].zeta, 4));
    }
    k->spannungsModen = static_cast<int>(sp.size());
    std::vector<Radial> rad;
    for (const PlattenMode& pm : k->moden) rad.push_back(radialPlatte(pm, x));
    // H^n_pq = ∫ρdρ Φ_n·[(A″(B′/ρ − b²B/ρ²) + B″(A′/ρ − a²A/ρ²))·∫cos a cos b cos c − 2ab(A′/ρ − A/ρ²)(B′/ρ − B/ρ²)·∫sin a sin b cos c]
    const std::size_t nm = k->moden.size();
    double groesst = 0.0;
    std::vector<Kopplung::Eintrag> roh;
    for (std::size_t p = 0; p < nm; ++p)
        for (std::size_t q = p; q < nm; ++q)
        {
            const int a = k->moden[p].m, b = k->moden[q].m;
            for (std::size_t n = 0; n < sp.size(); ++n)
            {
                const int c = sp[n].m;
                if (c != a + b && c != std::abs(a - b)) continue;
                const double icc = 0.5 * (winkel(std::abs(a - b), c) + winkel(a + b, c));
                const double iss = 0.5 * (winkel(std::abs(a - b), c) - winkel(a + b, c));
                const Radial& A = rad[p];
                const Radial& B = rad[q];
                double sum = 0.0;
                for (std::size_t i = 0; i < x.size(); ++i)
                {
                    const double r = x[i];
                    const double t1 = A.d2[i] * (B.d1[i] / r - b * b * B.f[i] / (r * r)) + B.d2[i] * (A.d1[i] / r - a * a * A.f[i] / (r * r));
                    const double t2 = -2.0 * a * b * (A.d1[i] / r - A.f[i] / (r * r)) * (B.d1[i] / r - B.f[i] / (r * r));
                    sum += w[i] * r * phi[n][i] * (t1 * icc + t2 * iss);
                }
                groesst = std::max(groesst, std::abs(sum));
                roh.push_back({static_cast<int>(n), static_cast<int>(p), static_cast<int>(q), sum});
            }
        }
    for (const Kopplung::Eintrag& e : roh)
        if (std::abs(e.h) > 1e-9 * groesst) k->h.push_back(e);
    s.kopplung = k;
}

void rechneBeckenTon(const BeckenSpiel& s, const BeckenAnschlag& a, std::vector<float>& mono, std::int64_t frames, BeckenMessung* messung,
                     bool ohneDaempfung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0 || !s.kopplung || s.kopplung->moden.empty()) return;
    const Kopplung& k = *s.kopplung;
    const std::size_t n = k.moden.size();
    const double dt = 1.0 / a.rate;
    const double rhoH = s.dichte * s.dickeM;
    const double r0 = a.stelle == Stelle::Kuppe ? 0.15 : a.stelle == Stelle::Rand ? 0.95 : 0.6;
    // Moden: Frequenz, Daempfung (Humbert: ξ = 8·10⁻³·ω^−0,4), Form am Schlagpunkt und an drei Abnahmepunkten
    std::vector<double> om2(n), sigma(n), wSchlag(n), wAus(n), omega(n);
    const double ra[3] = {0.3, 0.55, 0.85}, ta[3] = {0.4, 1.3, 2.4};
    for (std::size_t i = 0; i < n; ++i)
    {
        const PlattenMode& pm = k.moden[i];
        omega[i] = 2.0 * kPi * pm.hz;
        om2[i] = 2.0 * (1.0 - std::cos(omega[i] * dt)) / (dt * dt);
        sigma[i] = ohneDaempfung ? 0.0 : 8e-3 * std::pow(omega[i], -0.4) * omega[i];
        if (s.hihat) sigma[i] += (1.0 - std::clamp(a.oeffnung, 0.0, 1.0)) * 80.0;  // die Schalen liegen an (offen)
        const auto wAn = [&](double rho) { return pm.norm * (jm(pm.m, pm.lambda * rho) + pm.c * im(pm.m, pm.lambda * rho)) / s.radiusM; };
        wSchlag[i] = wAn(r0);
        for (int j = 0; j < 3; ++j) wAus[i] += wAn(ra[j]) * std::cos(pm.m * ta[j]) / 3.0;
    }
    // Skalen der Nichtlinearitaet: dimensional H = Ĥ/a⁵, ζ⁴ = ζ̂⁴/a⁴ → V = E/(8ρa⁶)·Σ η̂²/ζ̂⁴
    const double kv = s.eModul / (8.0 * s.dichte * std::pow(s.radiusM, 6));
    std::vector<double> q(n, 0.0), qv(n, 0.0), qn(n, 0.0), eta(static_cast<std::size_t>(k.spannungsModen), 0.0), g(n), rhs(n);
    double psi = 0.0;  // ψ^{n−½}
    const double eps = 1e-30;
    const double e = std::clamp(a.staerke, 0.0, 1.0);
    double kopfY = 0.0, kopfV = s.tempoPp * std::pow(s.tempoFf / s.tempoPp, e);
    bool beruehrt = false, frei = false;
    double erster = 0.0;
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    double eStart = -1.0;
    const auto block = static_cast<std::int64_t>(0.05 * a.rate);
    double eHoch = 0.0, eGesamt = 0.0;
    for (std::int64_t t = 0; t < frames; ++t)
    {
        // der Schlag: Hertz-Kontakt, explizit
        double kraft = 0.0;
        if (!frei)
        {
            double w0 = 0.0;
            for (std::size_t i = 0; i < n; ++i) w0 += q[i] * wSchlag[i];
            const double d = kopfY - w0;
            if (d > 0.0)
            {
                kraft = s.schlaegelQ0 * std::pow(d, s.schlaegelP);
                if (!beruehrt) erster = t * dt;
                beruehrt = true;
                if (messung != nullptr)
                {
                    messung->kontaktS = t * dt - erster;
                    messung->kraftSpitzeN = std::max(messung->kraftSpitzeN, kraft);
                }
            }
            else if (beruehrt && kopfV < 0.0)
                frei = true;
            kopfV -= kraft / s.schlaegelMasseKg * dt;
            kopfY += kopfV * dt;
        }
        // η und ∇V
        std::fill(eta.begin(), eta.end(), 0.0);
        for (const Kopplung::Eintrag& x : k.h) eta[static_cast<std::size_t>(x.n)] += (x.p == x.q ? 1.0 : 2.0) * x.h * q[static_cast<std::size_t>(x.p)] * q[static_cast<std::size_t>(x.q)];
        double v = 0.0;
        for (std::size_t j = 0; j < eta.size(); ++j) v += eta[j] * eta[j] / k.zeta4[j];
        v *= kv;
        std::fill(g.begin(), g.end(), 0.0);
        for (const Kopplung::Eintrag& x : k.h)
        {
            const double c = 4.0 * kv * eta[static_cast<std::size_t>(x.n)] / k.zeta4[static_cast<std::size_t>(x.n)] * x.h;  // ∂(η²)/∂q = 2η·∂η/∂q
            if (x.p == x.q)
                g[static_cast<std::size_t>(x.p)] += c * q[static_cast<std::size_t>(x.p)];
            else
            {
                g[static_cast<std::size_t>(x.p)] += c * q[static_cast<std::size_t>(x.q)];
                g[static_cast<std::size_t>(x.q)] += c * q[static_cast<std::size_t>(x.p)];
            }
        }
        const double psiJetzt = std::sqrt(2.0 * v + eps);
        double gqv = 0.0;
        for (std::size_t i = 0; i < n; ++i)
        {
            g[i] /= psiJetzt;
            gqv += g[i] * qv[i];
        }
        // Choke: die Hand ab chokeAbS (Daempfung in chokeS hoch)
        double extra = 0.0;
        if (a.chokeAbS >= 0.0 && t * dt >= a.chokeAbS) extra = std::min(1.0, (t * dt - a.chokeAbS) / s.chokeS) * 6.9 / s.chokeS;
        // rhs und Sherman–Morrison
        double gx = 0.0, gy = 0.0;
        for (std::size_t i = 0; i < n; ++i)
        {
            const double sg = (sigma[i] + extra) * dt;
            const double p = kraft * wSchlag[i] / rhoH;
            rhs[i] = (2.0 * q[i] - (1.0 - sg) * qv[i] - dt * dt * om2[i] * q[i] - dt * dt * (psi - 0.25 * gqv) * g[i] + dt * dt * p) / (1.0 + sg);
            gx += g[i] * rhs[i];
            gy += g[i] * (0.25 * dt * dt * g[i]) / (1.0 + sg);
        }
        const double faktor = gx / (1.0 + gy);
        double gdiff = 0.0, y = 0.0;
        for (std::size_t i = 0; i < n; ++i)
        {
            const double sg = (sigma[i] + extra) * dt;
            qn[i] = rhs[i] - (0.25 * dt * dt * g[i]) / (1.0 + sg) * faktor;
            gdiff += g[i] * (qn[i] - qv[i]);
            y += (qn[i] - 2.0 * q[i] + qv[i]) / (dt * dt) * wAus[i];
        }
        psi += 0.5 * gdiff;
        summe[static_cast<std::size_t>(t)] = y;
        // Messung: Energie (diskret: kinetisch + linear + ψ²/2), Anteil ueber 1 kHz je 50 ms
        if (messung != nullptr)
        {
            double en = 0.5 * psi * psi;
            for (std::size_t i = 0; i < n; ++i)
            {
                const double vel = (qn[i] - q[i]) / dt;
                const double ei = 0.5 * vel * vel + 0.5 * om2[i] * qn[i] * q[i];
                en += ei;
                eGesamt += ei;
                if (k.moden[i].hz > 1000.0) eHoch += ei;
            }
            if (frei && eStart < 0.0)
            {
                eStart = en;
                messung->energieAnfang = en;
            }
            if (eStart > 0.0) messung->energieMaxFehler = std::max(messung->energieMaxFehler, std::abs(en - eStart) / eStart);
            if ((t + 1) % block == 0)
            {
                messung->bandEnergie.push_back(eGesamt > 0.0 ? eHoch / eGesamt : 0.0);
                eHoch = eGesamt = 0.0;
            }
        }
        std::swap(qv, q);
        std::swap(q, qn);
    }
    const auto blende = std::min<std::int64_t>(frames, std::max<std::int64_t>(1, std::llround(0.005 * a.rate)));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double w = summe[static_cast<std::size_t>(i)] / s.ausgangBezug;
        const std::int64_t rest = frames - 1 - i;
        if (rest < blende) w *= static_cast<double>(rest) / static_cast<double>(blende);
        mono[static_cast<std::size_t>(i)] = static_cast<float>(w);
    }
}

}  // namespace ic::becken
