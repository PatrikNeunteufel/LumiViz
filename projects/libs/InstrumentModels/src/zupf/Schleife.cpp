/**
 * @file Schleife.cpp
 * @brief Die Saitenschleife des Kerns `pluckedString` Rechenfassung 2 (34k-7d-4 (a)) — siehe Kopf
 */
#include "InstrumentModels/zupf/Schleife.hpp"

#include "InstrumentCore/Ausgleich.hpp"
#include "InstrumentCore/Fft.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace lumi::kern::zupf
{

namespace
{
using cplx = std::complex<double>;
constexpr double kPi = std::numbers::pi;
constexpr cplx kI{0.0, 1.0};

/// GZ6 a: die Stufe kippt das Spektrum um 1 kHz, k dB je Oktave je dB Stufe — ohne den Pegel, den traegt die Auslenkung
/// (`neigung_faktor` des Bezugs; bis S224 mit dem Pegel wie `staerke_faktor` des Pruefstands, die Stufe zaehlte doppelt).
double neigungFaktor(cplx w, double stufe, double kipp)
{
    if (stufe == 0.0 || kipp == 0.0) return 1.0;
    const double f = std::max(std::abs(w.real()) / (2.0 * kPi), 20.0);
    return std::pow(10.0, stufe * kipp * std::log2(f / 1000.0) / 20.0);
}

/// ∫₀ᴸ y₀(x) e^(−i κ x) dx fuer das Dreieck y₀ (Spitze d bei a, null an beiden Enden), geschlossen.
cplx dreieckIntegral(cplx kappa, double a, double L, double d)
{
    const cplx Ea = std::exp(-kI * kappa * a), EL = std::exp(-kI * kappa * L);
    return d * (Ea - 1.0) / (a * kappa * kappa) - d * (EL - Ea) / ((L - a) * kappa * kappa);
}

/// Die Quelle g(ω) einer Saite (`quelle` des Bezugs, Geschwindigkeit als Kraftstoss).
cplx quelleBei(const umlauf::Saitenwerte& sa, double L, const Zupfangabe& z, cplx w)
{
    const Welle wl = welleBei(sa, w);
    const double T = sa.zug, m = sa.masse, xp = z.ort, d = z.auslenkung;
    const cplx arg = w * z.breite / std::sqrt(T / m);
    const cplx breit = std::exp(-(arg * arg)) * neigungFaktor(w, z.neigung, z.kipp);
    const cplx f = -(T * d * L / (xp * (L - xp))) / (kI * w) * breit;
    cplx g = f / (2.0 * wl.z0) * (std::exp(-kI * wl.k * xp) - std::exp(-kI * wl.k * (2.0 * L - xp)));
    if (z.tempo != 0.0)
    {
        const cplx q = -m * (z.tempo / d) * breit;  // Kraftstoss je Laenge und Auslenkung: μ v₀(x) = q y₀(x)
        g += q / (2.0 * wl.z0) *
             (dreieckIntegral(wl.k, xp, L, d) - std::exp(-2.0 * kI * wl.k * L) * dreieckIntegral(-wl.k, xp, L, d));
    }
    return g;
}

/// Eine Mode im Takt, „genau fuer stueckweise lineare Kraft“ (`moden_diskret` „foh“ des Kandidaten S200).
struct TaktMode
{
    cplx pol, b0, b1;
    double senkrecht = 0.0, parallel = 0.0;  ///< e: Formwert mal Richtung
};

TaktMode taktMode(double hz, double eta, double senkrecht, double parallel)
{
    const double dt = 1.0 / kSchleifenRate;
    const double wn = 2.0 * kPi * hz;
    const double beta = eta * wn;
    const double wd = std::sqrt(std::max(1e-12, wn * wn - 0.25 * beta * beta));
    const cplx lam(-0.5 * beta, wd);
    const cplx c = lam / (kI * wd);  // h(t) = Re c exp(λ t), h(0) = 1
    const cplx pol = std::exp(lam * dt);
    const cplx x = lam * dt;
    TaktMode m;
    m.pol = pol;
    m.b0 = c * (pol - 1.0 - x) / (lam * x);
    m.b1 = c * (pol * (x - 1.0) + 1.0) / (lam * x);
    m.senkrecht = senkrecht;
    m.parallel = parallel;
    return m;
}
}  // namespace

Daempfer daempferAn(const Steg::Dichte& dichte, double streuung)
{
    Daempfer d;
    d.streuung = streuung;
    if (dichte.dichte <= 0.0) return d;
    std::vector<double> f;
    for (double x = 500.0; x <= 4500.0; x += 250.0) f.push_back(x);
    for (double x = 5250.0; x <= 20000.0; x += 250.0) f.push_back(x);
    const std::size_t n = kDaempferMitten.size();
    Matrix A(f.size(), n);
    std::vector<double> b(f.size());
    for (std::size_t i = 0; i < f.size(); ++i)
    {
        const double ziel = f[i] >= dichte.vonHz ? (dichte.dichte + dichte.anstieg * (f[i] - dichte.vonHz)) * streuung * streuung / 4.0 : 0.0;
        const double gew = f[i] < dichte.vonHz ? 3.0 : 1.0;
        const double w = 2.0 * kPi * f[i];
        for (std::size_t j = 0; j < n; ++j)
        {
            const double wn = 2.0 * kPi * kDaempferMitten[j];
            A(i, j) = (kI * w / (wn * wn - w * w + kI * kDaempferVerlust * wn * w)).real() * gew;
        }
        b[i] = ziel * gew;
    }
    const std::vector<double> phi2 = nnls(A, b);
    for (std::size_t j = 0; j < n; ++j) d.gewichte[j] = phi2[j];
    return d;
}

Welle welleBei(const umlauf::Saitenwerte& s, cplx w)
{
    const double T = s.zug, m = s.masse, EI = s.biegesteife;
    const cplx k2 = EI > 0.0 ? (-T + std::sqrt(T * T + 4.0 * EI * m * w * w)) / (2.0 * EI) : w * w * m / T;
    const cplx biegung = EI * s.verlustBiegung * k2 / (T + EI * k2);
    cplx eta;
    if (s.durchmesser > 0.0)
    {
        const double f = std::max(std::abs(w.real()) / (2.0 * kPi), 20.0);
        const double M = s.durchmesser / 4.0 * std::sqrt(2.0 * kPi * f / 1.5e-5);
        eta = (s.durchmesser * 1000.0 + 0.2) * 1.2 / s.dichte * (2.0 * std::sqrt(2.0) * M + 1.0) / (M * M) + biegung;
    }
    else
        eta = T * (s.verlustInnen + s.verlustLuft / w) / (T + EI * k2) + biegung;
    const cplx k = std::sqrt(k2) * (1.0 - 0.5 * kI * eta);
    return {k, T * k / w};
}

std::vector<double> quelleImTakt(const umlauf::Saitenwerte& saite, double laenge, const Zupfangabe& zupf, std::size_t frames, double vorlauf)
{
    // Die FFT reicht ueber die ganze Note (hoechstens 2¹⁸ Werte, 5,5 s), mindestens ueber das Achtfache von Vorlauf und
    // Umlauf. g selbst ist kurz (zwei Stufen im Abstand 2 (L − x_p) / c), aber die Neigung (GZ6 a) knickt das Spektrum bei
    // 20 Hz und gibt g einen langsamen Auslaeufer: mit 2¹⁵ Werten (0,68 s) lagen die spaeten Pegel der Gitarre pp bis 5 dB
    // neben dem Bezug, ueber die Note wie der Kandidat S200 (S224). Was ueber 2¹⁸ hinausreicht, faellt um e^(−s₀ n / f_s)
    // ≈ 2 · 10⁻⁵ gedaempft auf den Anfang.
    const double c0 = std::sqrt(saite.zug / saite.masse);
    const double traeger = vorlauf + 2.0 * laenge / c0;
    const std::size_t n = zweierpotenzAb(std::max<std::size_t>(std::min<std::size_t>(frames, std::size_t{1} << 18),
                                                               static_cast<std::size_t>(8.0 * traeger * kSchleifenRate)));
    std::vector<cplx> spektrum(n / 2 + 1);
    for (std::size_t k = 0; k < spektrum.size(); ++k)
    {
        const cplx w(2.0 * kPi * static_cast<double>(k) * kSchleifenRate / static_cast<double>(n), -kQuelleS0);
        spektrum[k] = quelleBei(saite, laenge, zupf, w) * std::exp(-kI * w * vorlauf);
    }
    std::vector<double> zeit(n);
    fft().rueckwaerts(spektrum, zeit);
    std::vector<double> g(frames, 0.0);
    for (std::size_t t = 0; t < std::min(frames, n); ++t)
        g[t] = zeit[t] * kSchleifenRate * std::exp(kQuelleS0 * static_cast<double>(t) / kSchleifenRate);
    return g;
}

Schleifenkraft rechneSchleife(const Schleifenauftrag& a)
{
    if (a.saiten.empty() || a.steg == nullptr || a.punkt == nullptr)
        throw std::invalid_argument("plucked string loop: strings, bridge and bridge point are required");
    if (a.punkt->form.size() != a.steg->moden.size())
        throw std::invalid_argument("plucked string loop: the bridge point does not carry a value for every mode");
    const std::size_t S = a.saiten.size(), P = 2 * S;
    for (const SchleifenSaite& s : a.saiten)
        if (s.filter.verzugGanz < 1) throw std::invalid_argument("plucked string loop: the loop filter needs an integer delay of at least 1");

    Schleifenkraft aus;
    aus.senkrecht.assign(a.frames, 0.0);
    aus.parallel.assign(a.frames, 0.0);
    aus.daempfer = daempferAn(a.steg->dichte, a.punkt->streuung);

    // die Moden am Stegpunkt: die der Datei, dann der Daempfer nur senkrecht
    std::vector<TaktMode> moden;
    moden.reserve(a.steg->moden.size() + kDaempferMitten.size());
    for (std::size_t n = 0; n < a.steg->moden.size(); ++n)
    {
        const Steg::Mode& m = a.steg->moden[n];
        const double w = m.winkelGrad * kPi / 180.0, phi = a.punkt->form[n];
        moden.push_back(taktMode(m.hz, m.verlust, phi * std::cos(w), phi * std::sin(w)));
    }
    for (std::size_t j = 0; j < kDaempferMitten.size(); ++j)
        if (aus.daempfer.gewichte[j] > 0.0)
            moden.push_back(taktMode(kDaempferMitten[j], kDaempferVerlust, std::sqrt(aus.daempfer.gewichte[j]), 0.0));

    // der Durchgriff: M = Σ Re B0 e eᵀ (je Saite dieselben zwei Richtungen), Z je Saite √(T m)
    double m00 = 0.0, m01 = 0.0, m11 = 0.0;
    for (const TaktMode& m : moden)
    {
        m00 += m.b0.real() * m.senkrecht * m.senkrecht;
        m01 += m.b0.real() * m.senkrecht * m.parallel;
        m11 += m.b0.real() * m.parallel * m.parallel;
    }
    const auto mVon = [&](std::size_t p, std::size_t q) {
        const std::size_t x = p % 2, y = q % 2;
        return x == 0 && y == 0 ? m00 : x == 1 && y == 1 ? m11 : m01;
    };
    std::vector<double> z0(P);
    for (std::size_t i = 0; i < S; ++i) z0[2 * i] = z0[2 * i + 1] = std::sqrt(a.saiten[i].werte.zug * a.saiten[i].werte.masse);
    Matrix IMZ(P, P), zM(P, P);
    for (std::size_t p = 0; p < P; ++p)
        for (std::size_t q = 0; q < P; ++q)
        {
            IMZ(p, q) = (p == q ? 1.0 : 0.0) + mVon(p, q) * z0[q];
            zM(p, q) = 2.0 * mVon(p, q) * z0[q];
        }
    Matrix A(P, P);  // (I + M Z)⁻¹, spaltenweise
    for (std::size_t q = 0; q < P; ++q)
    {
        std::vector<double> e(P, 0.0);
        e[q] = 1.0;
        const std::vector<double> x = loese(IMZ, e);
        for (std::size_t p = 0; p < P; ++p) A(p, q) = x[p];
    }

    // die Quelle je Saite, der Umlauf je Saite und Polarisation
    const double wz = a.zupf.winkelGrad * kPi / 180.0;
    const double cz = std::cos(wz), sz = std::sin(wz);
    std::vector<std::vector<double>> g(S);
    std::vector<umlauf::Umlauflauf> lauf;
    std::vector<std::vector<double>> ring(P);
    std::vector<std::size_t> stelle(P, 0);
    for (std::size_t i = 0; i < S; ++i)
    {
        g[i] = quelleImTakt(a.saiten[i].werte, a.saiten[i].laenge, a.zupf, a.frames, a.vorlauf);
        for (int pol = 0; pol < 2; ++pol)
        {
            lauf.emplace_back(a.saiten[i].filter);
            ring[2 * i + static_cast<std::size_t>(pol)].assign(static_cast<std::size_t>(a.saiten[i].filter.verzugGanz), 0.0);
        }
    }

    std::vector<cplx> z(moden.size(), 0.0);
    std::vector<double> v(P), rechts(P), u(P), F(P, 0.0);
    double fs = 0.0, fp = 0.0;  // Summe der Kraefte senkrecht / parallel aus dem letzten Takt
    for (std::size_t t = 0; t < a.frames; ++t)
    {
        for (std::size_t p = 0; p < P; ++p)
        {
            const double quelle = g[p / 2][t] * (p % 2 == 0 ? cz : sz);
            v[p] = quelle - lauf[p].schritt(ring[p][stelle[p]]);
        }
        double uf0 = 0.0, uf1 = 0.0;
        for (std::size_t n = 0; n < moden.size(); ++n)
        {
            TaktMode& m = moden[n];
            z[n] = z[n] * m.pol + m.b1 * (m.senkrecht * fs + m.parallel * fp);
            uf0 += m.senkrecht * z[n].real();
            uf1 += m.parallel * z[n].real();
        }
        for (std::size_t p = 0; p < P; ++p)
        {
            double s = p % 2 == 0 ? uf0 : uf1;
            for (std::size_t q = 0; q < P; ++q) s += zM(p, q) * v[q];
            rechts[p] = s;
        }
        fs = fp = 0.0;
        for (std::size_t p = 0; p < P; ++p)
        {
            double s = 0.0;
            for (std::size_t q = 0; q < P; ++q) s += A(p, q) * rechts[q];
            u[p] = s;
            F[p] = z0[p] * (2.0 * v[p] - u[p]);
            (p % 2 == 0 ? fs : fp) += F[p];
        }
        for (std::size_t n = 0; n < moden.size(); ++n) z[n] += moden[n].b0 * (moden[n].senkrecht * fs + moden[n].parallel * fp);
        for (std::size_t p = 0; p < P; ++p)
        {
            ring[p][stelle[p]] = u[p] - v[p];
            stelle[p] = (stelle[p] + 1) % ring[p].size();
        }
        aus.senkrecht[t] = fs;
        aus.parallel[t] = fp;
    }
    return aus;
}

}  // namespace lumi::kern::zupf
