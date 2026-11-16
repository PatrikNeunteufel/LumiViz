// S200, 34k-6 — Rechenzeit des Zeitbereichs-Kandidaten (2026-11-16_S200_zeitbereich_kandidat.py, `schleife`): eine Saite,
// zwei Richtungen, N Moden am Steg (z = pol z + B1 eF_alt, u = Re sum e z, Durchgriff 2x2, z += B0 eF), der Umlauf der
// Saite als FIR mit K Werten je Richtung. Die Werte sind erfunden (stabil, Groessenordnung wie am Cembalo); gemessen
// wird nur die Zeit. Skalar, ein Thread, -O2 ohne fast-math wie der Kern.
//
//   clang++ -std=c++20 -O2 -ffp-contract=off -fno-fast-math <diese Datei> -o <ziel.exe>
//   <ziel.exe>            (je Fall dreimal, davor und danach eine Taktprobe)
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <vector>

namespace
{
constexpr double kFs = 48000.0;
constexpr double kPi = 3.141592653589793;

double jetzt()
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Taktprobe: eine feste Rechnung, deren Dauer zeigt, ob der Prozessor gerade drosselt
double taktprobe()
{
    const double t0 = jetzt();
    volatile double s = 0.0;
    for (int i = 1; i < 60000000; ++i) s = s + 1.0 / (static_cast<double>(i) * i);
    return jetzt() - t0;
}

struct Mode
{
    double pr, pi, b0r, b0i, b1r, b1i, e0, e1, zr = 0.0, zi = 0.0;
};

double lauf(int moden, int werte, double sekunden, double& pruefsumme)
{
    std::vector<Mode> m(static_cast<std::size_t>(moden));
    double m00 = 0.0, m01 = 0.0, m11 = 0.0;
    const double dt = 1.0 / kFs;
    for (int j = 0; j < moden; ++j)
    {
        const double f = 40.0 + 4960.0 * (j + 0.5) / moden;
        const double wn = 2.0 * kPi * f, beta = 0.02 * wn;
        const double wd = std::sqrt(wn * wn - 0.25 * beta * beta);
        const std::complex<double> lam(-0.5 * beta, wd), c = lam / std::complex<double>(0.0, wd), pol = std::exp(lam * dt), x = lam * dt;
        const std::complex<double> b0 = c * (pol - 1.0 - x) / (lam * x), b1 = c * (pol * (x - 1.0) + 1.0) / (lam * x);
        Mode& q = m[static_cast<std::size_t>(j)];
        q.pr = pol.real(), q.pi = pol.imag(), q.b0r = b0.real(), q.b0i = b0.imag(), q.b1r = b1.real(), q.b1i = b1.imag();
        q.e0 = 0.4 * std::cos(1.7 * j), q.e1 = 0.15 * std::sin(2.3 * j);
        m00 += q.b0r * q.e0 * q.e0, m01 += q.b0r * q.e0 * q.e1, m11 += q.b0r * q.e1 * q.e1;
    }
    const double z0 = 0.2;
    const double a11 = 1.0 + z0 * m00, a12 = z0 * m01, a22 = 1.0 + z0 * m11, det = a11 * a22 - a12 * a12;
    // der Umlauf: ein Stoss beim Verzug werte - 8, davor und danach kleine Werte, Summe der Betraege < 1
    std::vector<double> h(static_cast<std::size_t>(werte), 0.0);
    for (int k = 0; k < werte; ++k) h[static_cast<std::size_t>(k)] = (k == werte - 8 ? 0.97 : 0.02 / werte * ((k % 2) ? 1.0 : -1.0));
    const auto n = static_cast<std::size_t>(sekunden * kFs);
    const auto K = static_cast<std::size_t>(werte);
    std::vector<double> a0(n + K, 0.0), a1(n + K, 0.0);
    double f0 = 0.0, f1 = 0.0;
    pruefsumme = 0.0;
    const double t0 = jetzt();
    for (std::size_t i = 0; i < n; ++i)
    {
        double v0 = i < 96 ? 0.01 : 0.0, v1 = i < 96 ? 0.004 : 0.0;
        const double* p0 = a0.data() + i;
        const double* p1 = a1.data() + i;
        for (std::size_t k = 0; k < K; ++k)
        {
            v0 -= h[k] * p0[k];
            v1 -= h[k] * p1[k];
        }
        double u0 = 0.0, u1 = 0.0;
        for (Mode& q : m)
        {
            const double ef = q.e0 * f0 + q.e1 * f1;
            const double r = q.zr * q.pr - q.zi * q.pi + q.b1r * ef;
            const double s = q.zr * q.pi + q.zi * q.pr + q.b1i * ef;
            q.zr = r, q.zi = s;
            u0 += q.e0 * r, u1 += q.e1 * r;
        }
        const double r0 = u0 + 2.0 * z0 * (m00 * v0 + m01 * v1), r1 = u1 + 2.0 * z0 * (m01 * v0 + m11 * v1);
        const double w0 = (a22 * r0 - a12 * r1) / det, w1 = (a11 * r1 - a12 * r0) / det;
        f0 = z0 * (2.0 * v0 - w0), f1 = z0 * (2.0 * v1 - w1);
        for (Mode& q : m)
        {
            const double ef = q.e0 * f0 + q.e1 * f1;
            q.zr += q.b0r * ef, q.zi += q.b0i * ef;
        }
        a0[K + i] = w0 - v0, a1[K + i] = w1 - v1;
        pruefsumme += f0;
    }
    return (jetzt() - t0) / sekunden;
}
// Mitschwingen: `saiten` Saiten an `punkte` Stegpunkten (D = 2 punkte Richtungen am Steg); je Mode ein Skalarprodukt
// ueber D, der Durchgriff als D x D vorab invertiert (u = P u_frei + Q v), je Saite und Richtung ein Umlauf mit K Werten.
double laufViele(int moden, int werte, int saiten, int punkte, double sekunden, double& pruefsumme)
{
    const std::size_t D = 2 * static_cast<std::size_t>(punkte), N = static_cast<std::size_t>(moden), K = static_cast<std::size_t>(werte);
    const std::size_t L = 2 * static_cast<std::size_t>(saiten);  // Umlaeufe
    std::vector<double> pr(N), pi(N), b0r(N), b0i(N), b1r(N), b1i(N), zr(N, 0.0), zi(N, 0.0), e(N * D);
    const double dt = 1.0 / kFs;
    for (std::size_t j = 0; j < N; ++j)
    {
        const double f = 40.0 + 4960.0 * (j + 0.5) / moden, wn = 2.0 * kPi * f, beta = 0.02 * wn;
        const double wd = std::sqrt(wn * wn - 0.25 * beta * beta);
        const std::complex<double> lam(-0.5 * beta, wd), c = lam / std::complex<double>(0.0, wd), pol = std::exp(lam * dt), x = lam * dt;
        const std::complex<double> b0 = c * (pol - 1.0 - x) / (lam * x), b1 = c * (pol * (x - 1.0) + 1.0) / (lam * x);
        pr[j] = pol.real(), pi[j] = pol.imag(), b0r[j] = b0.real(), b0i[j] = b0.imag(), b1r[j] = b1.real(), b1i[j] = b1.imag();
        for (std::size_t d = 0; d < D; ++d) e[j * D + d] = 0.3 * std::cos(1.7 * j + 0.9 * d);
    }
    std::vector<double> et(N * D), ef(N, 0.0);
    for (std::size_t j = 0; j < N; ++j)
        for (std::size_t d = 0; d < D; ++d) et[d * N + j] = e[j * D + d];
    // P und Q: hier nur besetzt wie im Ernstfall (voll), Werte nahe der Einheit — fuer die Zeit ohne Belang
    std::vector<double> P(D * D), Q(D * D);
    for (std::size_t a = 0; a < D; ++a)
        for (std::size_t b = 0; b < D; ++b) P[a * D + b] = (a == b ? 1.0 : 1e-4), Q[a * D + b] = 1e-4;
    std::vector<double> h(K, 0.0);
    for (std::size_t k = 0; k < K; ++k) h[k] = (k == K - 8 ? 0.97 : 0.02 / werte * ((k % 2) ? 1.0 : -1.0));
    const auto n = static_cast<std::size_t>(sekunden * kFs);
    std::vector<std::vector<double>> aus(L, std::vector<double>(n + K, 0.0));
    std::vector<double> v(L), uf(D), u(D), F(D, 0.0), vp(D);
    const double z0 = 0.2;
    pruefsumme = 0.0;
    const double t0 = jetzt();
    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t l = 0; l < L; ++l)
        {
            double s = (l < 2 && i < 96) ? 0.01 : 0.0;
            const double* p = aus[l].data() + i;
            double t0s = 0.0, t1s = 0.0, t2s = 0.0, t3s = 0.0;
            for (std::size_t k = 0; k + 4 <= K; k += 4)
                t0s += h[k] * p[k], t1s += h[k + 1] * p[k + 1], t2s += h[k + 2] * p[k + 2], t3s += h[k + 3] * p[k + 3];
            v[l] = s - ((t0s + t1s) + (t2s + t3s));
        }
        // je Stegpunkt die Summe der ankommenden Wellen seiner Saiten (Saite l haengt am Punkt l mod punkte)
        for (std::size_t d = 0; d < D; ++d) vp[d] = 0.0, uf[d] = 0.0;
        for (std::size_t l = 0; l < L; ++l) vp[(l / 2 % static_cast<std::size_t>(punkte)) * 2 + l % 2] += v[l];
        // die Moden als Vektor: e liegt je Richtung d hintereinander (et[d * N + j]); die Summe ueber die Richtungen
        // laeuft je Mode in fester Reihenfolge, die Summe ueber die Moden in vier festen Bahnen (ohne fast-math
        // vektorisierbar, das Ergebnis haengt nicht vom Compiler ab)
        for (std::size_t j = 0; j < N; ++j) ef[j] = 0.0;
        for (std::size_t d = 0; d < D; ++d)
        {
            const double* ed = et.data() + d * N;
            const double fd = F[d];
            for (std::size_t j = 0; j < N; ++j) ef[j] += ed[j] * fd;
        }
        for (std::size_t j = 0; j < N; ++j)
        {
            const double r = zr[j] * pr[j] - zi[j] * pi[j] + b1r[j] * ef[j], s = zr[j] * pi[j] + zi[j] * pr[j] + b1i[j] * ef[j];
            zr[j] = r, zi[j] = s;
        }
        for (std::size_t d = 0; d < D; ++d)
        {
            const double* ed = et.data() + d * N;
            double s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0;
            std::size_t j = 0;
            for (; j + 4 <= N; j += 4)
                s0 += ed[j] * zr[j], s1 += ed[j + 1] * zr[j + 1], s2 += ed[j + 2] * zr[j + 2], s3 += ed[j + 3] * zr[j + 3];
            for (; j < N; ++j) s0 += ed[j] * zr[j];
            uf[d] = (s0 + s1) + (s2 + s3);
        }
        for (std::size_t a = 0; a < D; ++a)
        {
            const double* pa = P.data() + a * D;
            const double* qa = Q.data() + a * D;
            double s0 = 0.0, s1 = 0.0;
            for (std::size_t b = 0; b < D; b += 2) s0 += pa[b] * uf[b] + qa[b] * vp[b], s1 += pa[b + 1] * uf[b + 1] + qa[b + 1] * vp[b + 1];
            u[a] = s0 + s1;
        }
        for (std::size_t d = 0; d < D; ++d) F[d] = z0 * (2.0 * vp[d] - u[d]);
        for (std::size_t j = 0; j < N; ++j) ef[j] = 0.0;
        for (std::size_t d = 0; d < D; ++d)
        {
            const double* ed = et.data() + d * N;
            const double fd = F[d];
            for (std::size_t j = 0; j < N; ++j) ef[j] += ed[j] * fd;
        }
        for (std::size_t j = 0; j < N; ++j) zr[j] += b0r[j] * ef[j], zi[j] += b0i[j] * ef[j];
        for (std::size_t l = 0; l < L; ++l) aus[l][K + i] = u[(l / 2 % static_cast<std::size_t>(punkte)) * 2 + l % 2] - v[l];
        pruefsumme += F[0];
    }
    return (jetzt() - t0) / sekunden;
}
// Wege fuer das Mitschwingen (Vorlage S200). REAL = double (Vorgabe) oder float (-DREAL=float).
//   weg 0  Durchgriff je Stegpunkt (2 x 2) statt der vollen Matrix; der Durchgriff zwischen den Punkten wirkt einen Takt
//          spaeter (er ist klein: B0 ~ dt / 2)
//   weg 1  dazu Rueckwirkung nur der gespielten Saite: die Moden hoeren nur die Kraft am Punkt 0, die uebrigen Punkte
//          folgen (u = u_frei), ihre Saiten laufen mit, wirken aber nicht zurueck
#ifndef REAL
#define REAL double
#endif
double laufWege(int moden, int werte, int saiten, int punkte, int weg, double sekunden, double& pruefsumme)
{
    using R = REAL;
    const std::size_t D = 2 * static_cast<std::size_t>(punkte), N = static_cast<std::size_t>(moden), K = static_cast<std::size_t>(werte);
    const std::size_t L = 2 * static_cast<std::size_t>(saiten);
    std::vector<R> pr(N), pi(N), b0r(N), b0i(N), b1r(N), b1i(N), zr(N, 0), zi(N, 0), et(N * D), ef(N, 0);
    std::vector<double> mpp(static_cast<std::size_t>(punkte) * 3, 0.0);  // je Punkt m00, m01, m11
    const double dt = 1.0 / kFs;
    for (std::size_t j = 0; j < N; ++j)
    {
        const double f = 40.0 + 4960.0 * (j + 0.5) / moden, wn = 2.0 * kPi * f, beta = 0.02 * wn;
        const double wd = std::sqrt(wn * wn - 0.25 * beta * beta);
        const std::complex<double> lam(-0.5 * beta, wd), c = lam / std::complex<double>(0.0, wd), pol = std::exp(lam * dt), x = lam * dt;
        const std::complex<double> b0 = c * (pol - 1.0 - x) / (lam * x), b1 = c * (pol * (x - 1.0) + 1.0) / (lam * x);
        pr[j] = static_cast<R>(pol.real()), pi[j] = static_cast<R>(pol.imag());
        b0r[j] = static_cast<R>(b0.real()), b0i[j] = static_cast<R>(b0.imag()), b1r[j] = static_cast<R>(b1.real()), b1i[j] = static_cast<R>(b1.imag());
        for (std::size_t d = 0; d < D; ++d) et[d * N + j] = static_cast<R>(0.3 * std::cos(1.7 * j + 0.9 * d));
        for (std::size_t p = 0; p < static_cast<std::size_t>(punkte); ++p)
        {
            const double e0 = et[(2 * p) * N + j], e1 = et[(2 * p + 1) * N + j];
            mpp[3 * p] += b0.real() * e0 * e0, mpp[3 * p + 1] += b0.real() * e0 * e1, mpp[3 * p + 2] += b0.real() * e1 * e1;
        }
    }
    std::vector<R> h(K, 0);
    for (std::size_t k = 0; k < K; ++k) h[k] = static_cast<R>(k == K - 8 ? 0.97 : 0.02 / werte * ((k % 2) ? 1.0 : -1.0));
    const auto n = static_cast<std::size_t>(sekunden * kFs);
    std::vector<std::vector<R>> aus(L, std::vector<R>(n + K, 0));
    std::vector<R> v(L), uf(D), u(D), F(D, 0), vp(D);
    const R z0 = static_cast<R>(0.2);
    const std::size_t Dr = weg == 1 ? 2 : D;  // Richtungen, deren Kraft die Moden hoeren
    pruefsumme = 0.0;
    const double t0 = jetzt();
    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t l = 0; l < L; ++l)
        {
            const R s = (l < 2 && i < 96) ? static_cast<R>(0.01) : 0;
            const R* p = aus[l].data() + i;
            R t0s = 0, t1s = 0, t2s = 0, t3s = 0;
            for (std::size_t k = 0; k + 4 <= K; k += 4)
                t0s += h[k] * p[k], t1s += h[k + 1] * p[k + 1], t2s += h[k + 2] * p[k + 2], t3s += h[k + 3] * p[k + 3];
            v[l] = s - ((t0s + t1s) + (t2s + t3s));
        }
        for (std::size_t d = 0; d < D; ++d) vp[d] = 0;
        for (std::size_t l = 0; l < L; ++l) vp[(l / 2 % static_cast<std::size_t>(punkte)) * 2 + l % 2] += v[l];
        for (std::size_t j = 0; j < N; ++j) ef[j] = 0;
        for (std::size_t d = 0; d < Dr; ++d)
        {
            const R* ed = et.data() + d * N;
            const R fd = F[d];
            for (std::size_t j = 0; j < N; ++j) ef[j] += ed[j] * fd;
        }
        for (std::size_t j = 0; j < N; ++j)
        {
            const R r = zr[j] * pr[j] - zi[j] * pi[j] + b1r[j] * ef[j], s = zr[j] * pi[j] + zi[j] * pr[j] + b1i[j] * ef[j];
            zr[j] = r, zi[j] = s;
        }
        for (std::size_t d = 0; d < D; ++d)
        {
            const R* ed = et.data() + d * N;
            R s[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            std::size_t j = 0;
            for (; j + 8 <= N; j += 8)
                for (std::size_t q = 0; q < 8; ++q) s[q] += ed[j + q] * zr[j + q];
            for (; j < N; ++j) s[0] += ed[j] * zr[j];
            uf[d] = ((s[0] + s[1]) + (s[2] + s[3])) + ((s[4] + s[5]) + (s[6] + s[7]));
        }
        const std::size_t loesen = weg == 1 ? 1 : static_cast<std::size_t>(punkte);
        for (std::size_t p = 0; p < static_cast<std::size_t>(punkte); ++p)
        {
            if (p < loesen)
            {
                const double m00 = mpp[3 * p], m01 = mpp[3 * p + 1], m11 = mpp[3 * p + 2];
                const double a11 = 1.0 + z0 * m00, a12 = z0 * m01, a22 = 1.0 + z0 * m11, det = a11 * a22 - a12 * a12;
                const double r0 = uf[2 * p] + 2.0 * z0 * (m00 * vp[2 * p] + m01 * vp[2 * p + 1]);
                const double r1 = uf[2 * p + 1] + 2.0 * z0 * (m01 * vp[2 * p] + m11 * vp[2 * p + 1]);
                u[2 * p] = static_cast<R>((a22 * r0 - a12 * r1) / det), u[2 * p + 1] = static_cast<R>((a11 * r1 - a12 * r0) / det);
            }
            else
                u[2 * p] = uf[2 * p], u[2 * p + 1] = uf[2 * p + 1];
        }
        for (std::size_t d = 0; d < D; ++d) F[d] = z0 * (2 * vp[d] - u[d]);
        for (std::size_t j = 0; j < N; ++j) ef[j] = 0;
        for (std::size_t d = 0; d < Dr; ++d)
        {
            const R* ed = et.data() + d * N;
            const R fd = F[d];
            for (std::size_t j = 0; j < N; ++j) ef[j] += ed[j] * fd;
        }
        for (std::size_t j = 0; j < N; ++j) zr[j] += b0r[j] * ef[j], zi[j] += b0i[j] * ef[j];
        for (std::size_t l = 0; l < L; ++l) aus[l][K + i] = u[(l / 2 % static_cast<std::size_t>(punkte)) * 2 + l % 2] - v[l];
        pruefsumme += F[0];
    }
    return (jetzt() - t0) / sekunden;
}
}  // namespace

int main(int argc, char**)
{
    if (argc > 1)  // irgendein Argument: nur die Wege fuer das Mitschwingen
    {
        struct Weg
        {
            const char* name;
            int moden, saiten, punkte;
        };
        const Weg wege[] = {{"Gitarre: 89 Moden, 6 Saiten an 6 Punkten", 89, 6, 6},   {"Laute: 142 Moden, 15 Saiten an 8 Punkten", 142, 15, 8},
                            {"Oud: 257 Moden, 11 Saiten an 6 Punkten", 257, 11, 6},   {"Cembalo: 403 Moden, 3 Saiten an 2 Punkten", 403, 3, 2},
                            {"Harfe: 159 Moden, 20 Saiten (O+Q)", 159, 20, 20},       {"Harfe: 159 Moden, 33 Saiten (O+Q+T)", 159, 33, 33},
                            {"Harfe: 159 Moden, 46 Saiten (alle)", 159, 46, 46}};
        std::printf("Wege fuer das Mitschwingen, REAL = %s; Taktprobe vorher %.3f s\n", sizeof(REAL) == 4 ? "float" : "double", taktprobe());
        for (const Weg& f : wege)
            for (int weg = 0; weg < 2; ++weg)
            {
                double w[3], summe = 0.0;
                for (double& x : w) x = 100.0 * laufWege(f.moden, 16, f.saiten, f.punkte, weg, 4.0, summe);
                std::printf("%-48s %-28s %5.2f / %5.2f / %5.2f %% der Echtzeit   (Pruefsumme %.6e)\n", f.name,
                            weg == 0 ? "Durchgriff je Punkt" : "dazu Rueckwirkung nur Punkt 0", w[0], w[1], w[2], summe);
            }
        std::printf("Taktprobe nachher %.3f s\n", taktprobe());
        return 0;
    }
    struct Fall
    {
        const char* name;
        int moden, werte;
    };
    const Fall faelle[] = {{"Gitarre, 75 + 14 Moden, Umlauf als FIR 778 (E2)", 89, 778},  {"Gitarre, 89 Moden, FIR 272 (E5)", 89, 272},
                           {"Gitarre, 89 Moden, Umlauf mit 16 Werten (Filter niedriger Ordnung)", 89, 16},
                           {"Harfe, 145 + 14 Moden, 16 Werte", 159, 16},                  {"Laute, 128 + 14 Moden, 16 Werte", 142, 16},
                           {"Oud, 243 + 14 Moden, 16 Werte", 257, 16},                    {"Cembalo, 389 + 14 Moden, FIR 928 (C2)", 403, 928},
                           {"Cembalo, 403 Moden, 16 Werte", 403, 16},                     {"nur der Umlauf: 0 Moden, 16 Werte", 0, 16}};
    std::printf("Taktprobe vorher %.3f s\n", taktprobe());
    for (const Fall& f : faelle)
    {
        double w[3], summe = 0.0;
        for (double& x : w) x = 100.0 * lauf(f.moden, f.werte, 4.0, summe);
        std::printf("%-70s %5.2f / %5.2f / %5.2f %% der Echtzeit   (Pruefsumme %.6e)\n", f.name, w[0], w[1], w[2], summe);
    }
    struct Viele
    {
        const char* name;
        int moden, saiten, punkte;
    };
    const Viele viele[] = {{"Gitarre mit Mitschwingen: 89 Moden, 6 Saiten an 6 Punkten", 89, 6, 6},
                           {"Laute: 142 Moden, 15 Saiten an 8 Punkten", 142, 15, 8},
                           {"Oud: 257 Moden, 11 Saiten an 6 Punkten", 257, 11, 6},
                           {"Cembalo 8' + 8' + 4': 403 Moden, 3 Saiten an 2 Punkten", 403, 3, 2},
                           {"Harfe: 159 Moden, 7 Saiten (Oktaven)", 159, 7, 7},
                           {"Harfe: 159 Moden, 13 Saiten (O+Q nah)", 159, 13, 13},
                           {"Harfe: 159 Moden, 20 Saiten (O+Q)", 159, 20, 20},
                           {"Harfe: 159 Moden, 33 Saiten (O+Q+T)", 159, 33, 33},
                           {"Harfe: 159 Moden, 46 Saiten (alle)", 159, 46, 46},
                           // gekuerzte Kopplung: die uebrigen Saiten kennen nur die Moden bis zur Grenze (dazu kommt die
                           // gespielte Saite allein mit allen Moden: der Fall "Harfe, 145 + 14 Moden, 16 Werte" oben)
                           {"Harfe, Kopplung bis 2 kHz: 36 Moden, 20 Saiten (O+Q)", 36, 20, 20},
                           {"Harfe, Kopplung bis 1 kHz: 15 Moden, 20 Saiten (O+Q)", 15, 20, 20},
                           {"Laute, Kopplung bis 2 kHz: 34 Moden, 15 Saiten an 8 Punkten", 34, 15, 8},
                           {"Oud, Kopplung bis 2 kHz: 73 Moden, 11 Saiten an 6 Punkten", 73, 11, 6}};
    std::printf("Taktprobe dazwischen %.3f s\n", taktprobe());
    for (const Viele& f : viele)
    {
        double w[3], summe = 0.0;
        for (double& x : w) x = 100.0 * laufViele(f.moden, 16, f.saiten, f.punkte, 4.0, summe);
        std::printf("%-70s %5.2f / %5.2f / %5.2f %% der Echtzeit   (Pruefsumme %.6e)\n", f.name, w[0], w[1], w[2], summe);
    }
    std::printf("Taktprobe nachher %.3f s\n", taktprobe());
    return 0;
}
