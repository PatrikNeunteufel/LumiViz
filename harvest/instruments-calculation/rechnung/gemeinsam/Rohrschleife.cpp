/**
 * @file Rohrschleife.cpp
 * @brief Gemeinsam: die Schleife mit Wanderwellen (Herleitung im Kopf von `Rohrschleife.hpp`)
 */
#include "gemeinsam/Rohrschleife.hpp"

#include "gemeinsam/Fft.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr std::size_t kRing = 1 << 15;  // 0,68 s bei 48 kHz: laenger als jede Verzoegerung

/// Ein FIR aus seinem Spektrum (n/2 + 1 Werte), gefenstert und auf 99,99 % der Energie gekuerzt.
std::vector<double> firAus(const std::vector<Komplex>& spektrum, std::size_t n, std::size_t laengeMax)
{
    std::vector<double> h(n);
    fft().rueckwaerts(spektrum, h);
    double gesamt = 0.0;
    for (const double x : h) gesamt += x * x;
    double summe = 0.0;
    std::size_t laenge = 1;
    for (std::size_t i = 0; i < std::min(n / 2, laengeMax); ++i)
    {
        summe += h[i] * h[i];
        laenge = i + 1;
        if (summe >= 0.9999 * gesamt) break;
    }
    h.resize(laenge);
    // die letzten 20 % weich auslaufen lassen
    const std::size_t blende = std::max<std::size_t>(1, laenge / 5);
    for (std::size_t i = 0; i < blende; ++i) h[laenge - 1 - i] *= 0.5 - 0.5 * std::cos(kPi * static_cast<double>(i) / blende);
    return h;
}

/// Ein Skalarprodukt in vier Teilsummen fester Reihenfolge (E1 a: dieselbe Reihenfolge bei jedem Aufruf) — so laesst
/// es sich ohne fast-math parallel rechnen.
double skalar(const double* a, const double* b, std::size_t n)
{
    double s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0;
    std::size_t j = 0;
    for (; j + 4 <= n; j += 4)
    {
        s0 += a[j] * b[j];
        s1 += a[j + 1] * b[j + 1];
        s2 += a[j + 2] * b[j + 2];
        s3 += a[j + 3] * b[j + 3];
    }
    for (; j < n; ++j) s0 += a[j] * b[j];
    return (s0 + s1) + (s2 + s3);
}

}  // namespace

SchleifenFilter schleifenFilter(const Bohrung& rest, double zylinderRadiusM, double zylinderLaengeM, int rate, const Luft& luft,
                                std::size_t laengeMax)
{
    SchleifenFilter f;
    const std::size_t n = 1 << 14;
    const double zc = luft.dichte * luft.schall / (kPi * zylinderRadiusM * zylinderRadiusM);
    f.zylinderZc = zc;
    std::vector<Komplex> r(n / 2 + 1), a(n / 2 + 1);
    for (std::size_t k = 0; k <= n / 2; ++k)
    {
        const double hz = std::max(1e-3, static_cast<double>(k) * rate / n);
        const Kettenmatrix km = kettenmatrix(rest, hz, luft);
        const Komplex zl = abschlussImpedanz(rest, hz, luft);
        const Komplex z = rest.ende == Abschluss::Geschlossen ? km.a / km.c : (km.a * zl + km.b) / (km.c * zl + km.d);
        const Komplex refl = (z - zc) / (z + zc);
        // die Verluste des Zylinders ohne seine Laufzeit: e^(−(Γ − ik)·L) je Weg
        const Komplex g = wellenzahl(zylinderRadiusM, hz, luft, true) - Komplex(0.0, 2.0 * kPi * hz / luft.schall);
        const Komplex weg = std::exp(-g * zylinderLaengeM);
        r[k] = refl * weg * weg;
        // die Abstrahlung: p1 = (1 + R)·p⁺, U1 = (1 − R)·p⁺/Z_c am Anfang des Rests; [p2; U2] = K⁻¹·[p1; U1]
        const Komplex p1 = 1.0 + refl;
        const Komplex u1 = (1.0 - refl) / zc;
        const Komplex p2 = km.d * p1 - km.b * u1;
        Komplex u2 = -km.c * p1 + km.a * u1;
        // mit Daempfer strahlt nur der Fluss durch die Restflaeche ab
        if (rest.daempferHz > 0.0 && rest.ende == Abschluss::Offen)
        {
            Bohrung ohne = rest;
            ohne.daempferHz = 0.0;
            u2 = p2 * std::max(1e-6, rest.daempferRest) / abschlussImpedanz(ohne, hz, luft);
        }
        const double omega = 2.0 * kPi * hz;
        a[k] = Komplex(0.0, omega) * luft.dichte / (4.0 * kPi) * u2 * weg;
    }
    // Gleichanteil und Nyquist reell
    r[0] = r[0].real();
    r[n / 2] = r[n / 2].real();
    a[0] = 0.0;
    a[n / 2] = a[n / 2].real();
    f.reflexion = firAus(r, n, laengeMax);
    f.abstrahlung = firAus(a, n, laengeMax);
    return f;
}

Rohrschleife::Rohrschleife(SchleifenFilter filter, double kesselVolumenM3, double zylinderLaengeM, int rate, const Luft& luft)
    : m_f(std::move(filter)), m_luft(luft), m_rate(rate), m_laenge(zylinderLaengeM)
{
    m_c = kesselVolumenM3 / (luft.dichte * luft.schall * luft.schall);
    const std::size_t l = std::max(m_f.reflexion.size(), m_f.abstrahlung.size()) + 1;
    m_historie.assign(2 * l, 0.0);
    m_kesselPol = std::exp(-1.0 / (m_rate * m_f.zylinderZc * m_c));
    m_kopf = 0;
    m_zurueck.assign(kRing, 0.0);
}

void Rohrschleife::setzeZweitFilter(SchleifenFilter zweit)
{
    m_zweit = std::move(zweit);
    m_mitZweit = true;
    const std::size_t l = std::max({m_f.reflexion.size(), m_f.abstrahlung.size(), m_zweit.reflexion.size(), m_zweit.abstrahlung.size()}) + 1;
    if (2 * l > m_historie.size() && m_zeit == 0.0)
    {
        m_historie.assign(2 * l, 0.0);
        m_kopf = 0;
    }
}

void Rohrschleife::setzeZylinderLaenge(double laengeM)
{
    m_laenge = std::clamp(laengeM, 0.01, 0.4 * kRing * m_luft.schall / m_rate);
}

double Rohrschleife::schritt(double fluss)
{
    const double n = m_zeit;
    // 1. p⁺ am Ende jetzt: zwischen den Ankuenften linear (Ankunftszeiten nicht fallend)
    double amEnde = 0.0;
    while (m_hin.size() >= 2 && m_hin[1].first <= n) m_hin.pop_front();
    if (!m_hin.empty() && m_hin.front().first <= n)
    {
        if (m_hin.size() >= 2)
        {
            const auto& [t0, w0] = m_hin[0];
            const auto& [t1, w1] = m_hin[1];
            amEnde = t1 > t0 ? w0 + (w1 - w0) * (n - t0) / (t1 - t0) : w1;
        }
        else
            amEnde = m_hin.front().second;
    }
    // 2. Reflexion und Abstrahlung am Ende (FIR ueber die Geschichte am Ende)
    // die Geschichte am Ende rueckwaerts und zusammenhaengend (doppelt geschrieben): das Skalarprodukt laeuft ohne
    // Modulo und laesst sich vektorisieren
    m_kopf = m_kopf == 0 ? m_historie.size() / 2 - 1 : m_kopf - 1;
    m_historie[m_kopf] = amEnde;
    m_historie[m_kopf + m_historie.size() / 2] = amEnde;
    const double* h = m_historie.data() + m_kopf;
    double refl = skalar(m_f.reflexion.data(), h, m_f.reflexion.size());
    double rad = skalar(m_f.abstrahlung.data(), h, m_f.abstrahlung.size());
    if (m_mitZweit && m_mischung > 0.0)
    {
        const double w = std::clamp(m_mischung, 0.0, 1.0);
        refl += w * (skalar(m_zweit.reflexion.data(), h, m_zweit.reflexion.size()) - refl);
        rad += w * (skalar(m_zweit.abstrahlung.data(), h, m_zweit.abstrahlung.size()) - rad);
    }
    // 3. zurueck: p⁻ kommt nach L/c am Eingang an (gebrochen gelesen)
    m_zurueck[m_pos] = refl;
    const double zurueckT = m_laenge / m_luft.schall * m_rate;
    const double lesen = static_cast<double>(m_pos) - zurueckT + kRing;
    const auto i0 = static_cast<std::size_t>(std::floor(lesen));
    const double frac = lesen - std::floor(lesen);
    const double pMinus = (1.0 - frac) * m_zurueck[i0 % kRing] + frac * m_zurueck[(i0 + 1) % kRing];
    // 4. p⁺ in den Zylinder; seine Ankunft am Ende mit der Geschwindigkeit c + β·p⁺/(ρc)
    const double pPlus = m_pk - pMinus;
    const double c = m_luft.schall + m_beta * pPlus / (m_luft.dichte * m_luft.schall);
    double ankunft = n + m_laenge / std::max(0.5 * m_luft.schall, c) * m_rate;
    if (ankunft < m_letzteAnkunft) ankunft = m_letzteAnkunft;  // ueberholt: Stoss, die Werte verschmelzen
    m_letzteAnkunft = ankunft;
    m_hin.emplace_back(ankunft, pPlus);
    // 5. der Kessel: ṗ = (U − (p − 2p⁻)/Z_c)/C, exakt ueber den Takt bei festem U und p⁻
    const double ziel = m_f.zylinderZc * fluss + 2.0 * pMinus;
    m_pk = ziel + (m_pk - ziel) * m_kesselPol;
    m_pos = (m_pos + 1) % kRing;
    m_zeit += 1.0;
    return rad;
}

}  // namespace ic
