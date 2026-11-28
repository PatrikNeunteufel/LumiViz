/**
 * @file Schleife.cpp
 * @brief Die Saitenschleife des Kerns `pluckedString` Rechenfassung 2 (34k-7d-4 (a)) — siehe Kopf
 */
#include "InstrumentModels/zupf/Schleife.hpp"

#include "InstrumentCore/Ausgleich.hpp"
#include "InstrumentCore/Fft.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <mutex>
#include <numbers>
#include <stdexcept>

namespace lumi::kern::zupf
{

namespace
{
using cplx = std::complex<double>;
constexpr double kPi = std::numbers::pi;
constexpr cplx kI{0.0, 1.0};

/// ∫₀ᴸ y₀(x) e^(−i κ x) dx fuer das Dreieck y₀ (Spitze d bei a, null an beiden Enden), geschlossen.
cplx dreieckIntegral(cplx kappa, double a, double L, double d)
{
    const cplx Ea = std::exp(-kI * kappa * a), EL = std::exp(-kI * kappa * L);
    return d * (Ea - 1.0) / (a * kappa * kappa) - d * (EL - Ea) / ((L - a) * kappa * kappa);
}

/// Die Quelle g(ω) einer Saite (`quelle` des Bezugs, Geschwindigkeit als Kraftstoss); `filter` der Faktor des
/// Quellfilters an diesem ω (Neigung je Stufe und Bandgrenze, kausal: `quellfilterKausal`, VL2 a, VL5 a).
cplx quelleBei(const umlauf::Saitenwerte& sa, double L, const Zupfangabe& z, cplx w, cplx filter)
{
    const Welle wl = welleBei(sa, L, w);
    const double T = sa.zug, m = sa.masse, xp = z.ort, d = z.auslenkung;
    const cplx arg = w * z.breite / std::sqrt(T / m);
    const cplx breit = std::exp(-(arg * arg)) * filter;
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

double leitwertFuerAbklingen(double z0, double umlaeufe, double sekunden)
{
    const double ziel = -std::log(1000.0) / std::max(umlaeufe * sekunden, 1e-12);  // ∫₀¹ ln ρ dx
    const auto integral = [](double a) { return -((1.0 - a) * std::log(1.0 - a) + (1.0 + a) * std::log(1.0 + a)) / a; };
    double lo = 1e-12, hi = 0.999;
    if (integral(hi) > ziel) return hi / z0;  // auch der staerkste Daempfer reicht nicht in der Zeit
    for (int i = 0; i < 200; ++i)
    {
        const double mitte = 0.5 * (lo + hi);
        (integral(mitte) > ziel ? lo : hi) = mitte;
    }
    return 0.5 * (lo + hi) / z0;
}

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

Welle welleBei(const umlauf::Saitenwerte& s, double laenge, cplx w)
{
    // VL7 a: der Verlust kausal — die verlustfreie Wellenzahl mit angepasstem Zug und Biegesteife, fortgesetzt auf die
    // Gerade ω − i s₀, und Δk + i Im k als eine analytische Funktion dorthin fortgesetzt (`umlauf::kausalerVerlustAuf`;
    // `wellen_kausal` des Bezugs). z₀ mit dem Zug der Saite.
    const double T = s.zug, m = s.masse, EI = s.biegesteife;
    const std::shared_ptr<const umlauf::KausalerVerlustAufGerade> v = umlauf::kausalerVerlustAuf(s, -w.imag());
    const umlauf::KausalAngepasst a = umlauf::kausalAngepasst(s, laenge);
    const cplx k = umlauf::wellenzahlVerlustfrei(T * a.zugFaktor, m, EI * a.steifeFaktor, w) + v->bei(w.real());
    return {k, T * k / w};
}

std::shared_ptr<const std::vector<double>> quellfilterKausal(double neigungDb)
{
    // VL2 a, VL5 a: der Betrag 10^(p lb(max(f, 20 Hz) / 1 kHz) / 20) mal der Bandgrenze (1 bis kBandDurchlass, darueber mit
    // einem halben Kosinus bis f_s / 2 auf kBandBodenDb) auf kNeigungFein Werten der reellen Achse, das reelle Cepstrum
    // gefaltet (kausal), h = IFFT(exp(FFT(gefaltet))) — wie `quellfilter` des Bezugs. Je p einmal gerechnet.
    static std::mutex sperre;
    static std::map<double, std::shared_ptr<const std::vector<double>>> merk;
    {
        const std::lock_guard<std::mutex> l(sperre);
        if (const auto it = merk.find(neigungDb); it != merk.end()) return it->second;
    }
    const std::size_t N = kNeigungFein;
    std::vector<cplx> spektrum(N / 2 + 1);
    for (std::size_t k = 0; k < spektrum.size(); ++k)
    {
        const double f = std::max(static_cast<double>(k) * kSchleifenRate / static_cast<double>(N), 20.0);
        const double fk = static_cast<double>(k) * kSchleifenRate / static_cast<double>(N);
        const double x = std::clamp((fk - kBandDurchlass) / (0.5 * kSchleifenRate - kBandDurchlass), 0.0, 1.0);
        const double db = neigungDb * std::log2(f / 1000.0) + kBandBodenDb * 0.5 * (1.0 - std::cos(kPi * x));
        spektrum[k] = db / 20.0 * std::numbers::ln10;  // ln |H|
    }
    std::vector<double> c(N);
    fft().rueckwaerts(spektrum, c);
    for (std::size_t q = 1; q < N / 2; ++q) c[q] *= 2.0;
    for (std::size_t q = N / 2 + 1; q < N; ++q) c[q] = 0.0;
    fft().vorwaerts(c, spektrum);
    for (cplx& x : spektrum) x = std::exp(x);
    std::vector<double> h(N);
    fft().rueckwaerts(spektrum, h);
    h.resize(kQuelleLaenge);
    auto fertig = std::make_shared<const std::vector<double>>(std::move(h));
    const std::lock_guard<std::mutex> l(sperre);
    return merk.emplace(neigungDb, std::move(fertig)).first->second;
}

std::vector<double> quelleImTakt(const umlauf::Saitenwerte& saite, double laenge, const Zupfangabe& zupf, std::size_t frames, double vorlauf)
{
    // VL3 a: die FFT richtet sich nach der Quelle, nicht nach der Note — immer kQuelleLaenge Werte auf der Geraden
    // ω − i s₀ mit s₀ · Dauer = 8 (wie der Bezug: 4 s · 2 1/s); genutzt wird nur die erste Haelfte. Was vor dem Loslassen
    // liegt, faltet die FFT ans Ende (dort um e^(s₀ n) = e⁸ gehoben), was ueber das Ende reicht, um e⁻⁸ gedaempft auf den
    // Anfang; beides bleibt so ausserhalb der genutzten Haelfte bzw. unter −70 dB. Bis S224 reichte die FFT ueber die Note
    // (hoechstens 2¹⁸) mit s₀ = 2: an Noten ueber 5,4 s sprang die Kraft um 20–90 dB (`proben/2026-11-27_S224_vorlauf.txt`).
    // g selbst ist kurz (zwei Stufen im Abstand 2 (L − x_p) / c, geglaettet), die Neigung (VL2 a, kausal) gibt ihm einen
    // Auslaeufer, der in der halben FFT (2,7 s) abklingt. Das Quellfilter (Neigung und Bandgrenze, VL5 a) gilt immer.
    const std::size_t n = kQuelleLaenge;
    if (vorlauf * kSchleifenRate >= static_cast<double>(n / 2))
        throw std::invalid_argument("plucked string loop: the lead-in must be shorter than half the source window");
    const double s0 = kQuelleS0;
    std::vector<cplx> filter(n / 2 + 1);
    {
        const std::shared_ptr<const std::vector<double>> h = quellfilterKausal(zupf.neigung * zupf.kipp);
        std::vector<double> gedaempft(n);
        for (std::size_t t = 0; t < n; ++t) gedaempft[t] = (*h)[t] * std::exp(-s0 * static_cast<double>(t) / kSchleifenRate);
        fft().vorwaerts(gedaempft, filter);
    }
    std::vector<cplx> spektrum(n / 2 + 1);
    for (std::size_t k = 0; k < spektrum.size(); ++k)
    {
        const cplx w(2.0 * kPi * static_cast<double>(k) * kSchleifenRate / static_cast<double>(n), -s0);
        spektrum[k] = quelleBei(saite, laenge, zupf, w, filter[k]) * std::exp(-kI * w * vorlauf);
    }
    std::vector<double> zeit(n);
    fft().rueckwaerts(spektrum, zeit);
    std::vector<double> g(frames, 0.0);
    for (std::size_t t = 0; t < std::min(frames, n / 2); ++t)
        g[t] = zeit[t] * kSchleifenRate * std::exp(s0 * static_cast<double>(t) / kSchleifenRate);
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
            const SchleifenSaite& sa = a.saiten[p / 2];
            const double quelle = g[p / 2][t] * (p % 2 == 0 ? cz : sz);
            double rueck = lauf[p].schritt(ring[p][stelle[p]]);
            if (sa.daempferAb >= 0 && static_cast<long long>(t) >= sa.daempferAb)
            {
                const double anteil = sa.daempferRampe > 0
                                          ? std::min(1.0, static_cast<double>(static_cast<long long>(t) - sa.daempferAb) / static_cast<double>(sa.daempferRampe))
                                          : 1.0;
                const double zg = z0[p] * sa.daempferLeitwert * anteil;
                rueck *= (1.0 - zg) / (1.0 + zg);
            }
            v[p] = quelle - rueck;
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
