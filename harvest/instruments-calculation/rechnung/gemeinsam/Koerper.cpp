/**
 * @file Koerper.cpp
 * @brief Gemeinsam: Koerper aus Moden und der Luftraum (gekoppelte Kolben ueber den Druck im Hohlraum)
 */
#include "gemeinsam/Koerper.hpp"

#include "gemeinsam/Eigenzerlegung.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic
{

namespace
{

/// Die Groessen des gekoppelten Systems: M, K, C (diagonal bzw. voll), die Flaechen a, der Antrieb f.
struct System
{
    int d = 0;
    std::vector<double> masse;
    std::vector<double> steife;  // d × d
    std::vector<double> verlust;
    std::vector<double> flaeche;  // Beitrag zum Volumenfluss
    std::vector<double> antrieb;
};

System baue(const Luftraum& r)
{
    System s;
    const double alpha = r.dichte * r.schall * r.schall / r.volumenM3;
    const auto n = r.waende.size();
    const auto o = r.oeffnungen.size();
    s.d = static_cast<int>(n + o);
    const auto d = static_cast<std::size_t>(s.d);
    s.masse.assign(d, 0.0);
    s.steife.assign(d * d, 0.0);
    s.verlust.assign(d, 0.0);
    s.flaeche.assign(d, 0.0);
    s.antrieb.assign(d, 0.0);
    std::vector<double> kopplung(d, 0.0);  // die Flaeche, mit der ein Kolben den Druck aendert
    for (std::size_t k = 0; k < n; ++k)
    {
        const KoerperMode& w = r.waende[k];
        const double omega = 2.0 * std::numbers::pi * w.hz;
        const double q = omega * w.t60S / (2.0 * kLn1000);  // Q = ω/(2σ)
        s.masse[k] = w.masseKg;
        // die Frequenz gilt bei geschlossener Oeffnung: der Hohlraum steift schon mit (Wright Gl. A.12, A.28)
        s.steife[k * d + k] = w.masseKg * omega * omega - alpha * w.flaecheM2 * w.flaecheM2;
        s.verlust[k] = w.masseKg * omega / q;
        kopplung[k] = w.flaecheM2;
        s.flaeche[k] = w.flaecheM2;
        s.antrieb[k] = w.getrieben ? 1.0 : 0.0;
    }
    for (std::size_t j = 0; j < o; ++j)
    {
        const Oeffnung& h = r.oeffnungen[j];
        const double omega = 2.0 * std::numbers::pi * h.hz;
        const std::size_t i = n + j;
        s.masse[i] = alpha * h.flaecheM2 * h.flaecheM2 / (omega * omega);  // Gl. A.14
        s.verlust[i] = s.masse[i] * omega / h.q;
        kopplung[i] = h.flaecheM2;
        s.flaeche[i] = h.flaecheM2 * h.abstand;
    }
    // P = −α·Σ a_i x_i wirkt mit a_i auf jeden Kolben: K += α·a·aᵀ
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j) s.steife[i * d + j] += alpha * kopplung[i] * kopplung[j];
    return s;
}

}  // namespace

Komplex mobilitaet(const ModenKoerper& koerper, double hz)
{
    const double omega = 2.0 * std::numbers::pi * hz;
    Komplex y(0.0, 0.0);
    for (const KoerperMode& m : koerper.moden)
    {
        if (!m.getrieben) continue;
        const double wk = 2.0 * std::numbers::pi * m.hz;
        const double sigma = sigmaAusT60(m.t60S);
        const double w0 = wk * wk + sigma * sigma;
        y += Komplex(0.0, omega) / (m.masseKg * Komplex(w0 - omega * omega, 2.0 * sigma * omega));
    }
    return y;
}

std::vector<Mode> koerperModen(const ModenKoerper& koerper)
{
    std::vector<Mode> moden;
    for (const KoerperMode& m : koerper.moden)
        moden.push_back(modeAus(m.hz, m.t60S, m.masseKg, m.getrieben ? 1.0 : 0.0, 1.0, m.abstrahl, Ausgang::Beschleunigung));
    return moden;
}

std::vector<double> koerperAntwort(const ModenKoerper& koerper, double rate, std::int64_t frames)
{
    return impulsantwort(koerperModen(koerper), rate, frames);
}

LuftraumModen luftraumModen(const Luftraum& raum)
{
    LuftraumModen ergebnis;
    const System s = baue(raum);
    const int d = s.d;
    if (d == 0 || 2 * d > kKleineMatrixMax)
    {
        ergebnis.vollstaendig = false;
        return ergebnis;
    }
    const auto du = static_cast<std::size_t>(d);
    // ẏ = A·y + B·F, y = [x, v]
    KleineMatrix a(2 * d);
    std::vector<Komplex> b(2 * du, 0.0);
    std::vector<Komplex> c(2 * du, 0.0);  // dU/dt = aᵀ·ẍ = aᵀ·M⁻¹·(−K·x − C·v) (+ aᵀM⁻¹f·F, der Durchgriff)
    for (int i = 0; i < d; ++i)
    {
        a(i, d + i) = 1.0;
        const double mi = s.masse[static_cast<std::size_t>(i)];
        for (int j = 0; j < d; ++j) a(d + i, j) = -s.steife[static_cast<std::size_t>(i) * du + static_cast<std::size_t>(j)] / mi;
        a(d + i, d + i) = -s.verlust[static_cast<std::size_t>(i)] / mi;
        b[du + static_cast<std::size_t>(i)] = s.antrieb[static_cast<std::size_t>(i)] / mi;
    }
    for (int j = 0; j < 2 * d; ++j)
    {
        Komplex summe(0.0, 0.0);
        for (int i = 0; i < d; ++i) summe += s.flaeche[static_cast<std::size_t>(i)] * a(d + i, j);
        c[static_cast<std::size_t>(j)] = summe;
    }
    for (int i = 0; i < d; ++i)
        ergebnis.durchgriff += s.flaeche[static_cast<std::size_t>(i)] * s.antrieb[static_cast<std::size_t>(i)] / s.masse[static_cast<std::size_t>(i)];
    const Eigenzerlegung z = zerlegeKlein(a);
    ergebnis.vollstaendig = z.vollstaendig;
    ergebnis.entartet = z.entartet;
    for (int m = 0; m < 2 * d; ++m)
    {
        const Komplex lambda = z.werte[static_cast<std::size_t>(m)];
        if (lambda.imag() < 0.0) continue;  // der konjugierte Partner traegt dasselbe
        Komplex aus(0.0, 0.0);
        Komplex ein(0.0, 0.0);
        for (int i = 0; i < 2 * d; ++i)
        {
            aus += c[static_cast<std::size_t>(i)] * z.rechts(i, m);
            ein += z.links(m, i) * b[static_cast<std::size_t>(i)];
        }
        const Komplex g = aus * ein;
        Mode mode;
        mode.lambda = lambda;
        // Im(e·e^(λt)) = 2·Re(g·e^(λt)) mit e = 2i·g (ein Paar); eine reelle Mode allein: e = i·g
        mode.eingang = lambda.imag() > 0.0 ? Komplex(0.0, 2.0) * g : Komplex(0.0, 1.0) * g;
        mode.aus = Komplex(1.0, 0.0);
        mode.abnahme = 0.0;
        ergebnis.moden.push_back(mode);
    }
    return ergebnis;
}

double luftraumBetrag(const Luftraum& raum, double hz)
{
    const System s = baue(raum);
    const int d = s.d;
    const auto du = static_cast<std::size_t>(d);
    const double omega = 2.0 * std::numbers::pi * hz;
    // (K − ω²M + iωC)·X = f
    std::vector<Komplex> m(du * du);
    std::vector<Komplex> x(du);
    for (std::size_t i = 0; i < du; ++i)
    {
        for (std::size_t j = 0; j < du; ++j) m[i * du + j] = s.steife[i * du + j];
        m[i * du + i] += Komplex(-omega * omega * s.masse[i], omega * s.verlust[i]);
        x[i] = s.antrieb[i];
    }
    for (std::size_t k = 0; k < du; ++k)
    {
        std::size_t pivot = k;
        for (std::size_t i = k + 1; i < du; ++i)
            if (std::abs(m[i * du + k]) > std::abs(m[pivot * du + k])) pivot = i;
        if (pivot != k)
        {
            for (std::size_t j = 0; j < du; ++j) std::swap(m[k * du + j], m[pivot * du + j]);
            std::swap(x[k], x[pivot]);
        }
        for (std::size_t i = k + 1; i < du; ++i)
        {
            const Komplex f = m[i * du + k] / m[k * du + k];
            for (std::size_t j = k; j < du; ++j) m[i * du + j] -= f * m[k * du + j];
            x[i] -= f * x[k];
        }
    }
    for (std::size_t i = du; i-- > 0;)
    {
        Komplex summe = x[i];
        for (std::size_t j = i + 1; j < du; ++j) summe -= m[i * du + j] * x[j];
        x[i] = summe / m[i * du + i];
    }
    Komplex u(0.0, 0.0);
    for (std::size_t i = 0; i < du; ++i) u += s.flaeche[i] * x[i];
    return std::abs(-omega * omega * u);
}

}  // namespace ic
