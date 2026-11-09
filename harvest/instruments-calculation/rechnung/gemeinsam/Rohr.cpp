/**
 * @file Rohr.cpp
 * @brief Gemeinsam: Eingangsimpedanz einer Bohrung, ihre Moden, der Rohr-Kern im Zeitbereich
 *
 * Kegelstuempfe werden als Treppe kurzer Zylinder gerechnet (Stufen hoechstens 1 mm, Radius in der Stufenmitte) —
 * das vermeidet die Kegelformel aus dem Gedaechtnis; die Stufe ist so fein, dass die Spitzen sich bei halber Stufe
 * um weniger als 10⁻³ verschieben (Test).
 */
#include "gemeinsam/Rohr.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic
{

namespace
{

constexpr double kStufeM = 1e-3;

struct Kette
{
    Komplex a{1.0, 0.0}, b{0.0, 0.0}, c{0.0, 0.0}, d{1.0, 0.0};
};

/// Die Matrix eines Zylinders (Radius r, Laenge l) bei ω.
Kette zylinder(double r, double l, double omega, const Luft& luft, bool verluste)
{
    const double flaeche = std::numbers::pi * r * r;
    const double zc = luft.dichte * luft.schall / flaeche;
    const double k = omega / luft.schall;
    Komplex gamma(0.0, k);
    if (verluste && omega > 0.0)
    {
        const double rv = r * std::sqrt(omega * luft.dichte / luft.zaehigkeit);
        const double eps = (1.0 + (luft.gamma - 1.0) / std::sqrt(luft.prandtl)) / (std::sqrt(2.0) * rv);
        gamma = Komplex(0.0, k) * (1.0 + Komplex(1.0, -1.0) * eps);
    }
    const Komplex ch = std::cosh(gamma * l);
    const Komplex sh = std::sinh(gamma * l);
    return {ch, zc * sh, sh / zc, ch};
}

Kette mal(const Kette& x, const Kette& y)
{
    return {x.a * y.a + x.b * y.c, x.a * y.b + x.b * y.d, x.c * y.a + x.d * y.c, x.c * y.b + x.d * y.d};
}

}  // namespace

double durchgriffAus(const std::vector<ImpedanzMode>& moden, double rate)
{
    // die Impulsantwort springt bei t = 0 auf Σ C_n; abgetastet zaehlt der Sprung zur Haelfte (wie die Trapezregel)
    // — sonst traefe der Kern Z(ω) zwischen den Spitzen um rund 10 % zu hoch (Test)
    double d = 0.0;
    for (const ImpedanzMode& m : moden) d += m.spitze * 2.0 * std::numbers::pi * m.hz / m.q;
    return 0.5 * d / rate;
}

double laenge(const Bohrung& b)
{
    double l = 0.0;
    for (const Abschnitt& a : b.abschnitte) l += a.laengeM;
    return l;
}

double wellenImpedanz(const Bohrung& b, const Luft& luft)
{
    if (b.abschnitte.empty()) return 0.0;
    const double r = b.abschnitte.front().radiusEinM;
    return luft.dichte * luft.schall / (std::numbers::pi * r * r);
}

Komplex wellenzahl(double radiusM, double hz, const Luft& luft, bool verluste)
{
    const double omega = 2.0 * std::numbers::pi * hz;
    const double k = omega / luft.schall;
    if (!verluste || omega <= 0.0) return Komplex(0.0, k);
    const double rv = radiusM * std::sqrt(omega * luft.dichte / luft.zaehigkeit);
    const double eps = (1.0 + (luft.gamma - 1.0) / std::sqrt(luft.prandtl)) / (std::sqrt(2.0) * rv);
    return Komplex(0.0, k) * (1.0 + Komplex(1.0, -1.0) * eps);
}

Kettenmatrix kettenmatrix(const Bohrung& b, double hz, const Luft& luft)
{
    const double omega = 2.0 * std::numbers::pi * hz;
    Kette kette;
    for (const Abschnitt& a : b.abschnitte)
    {
        const int stufen = a.radiusEinM == a.radiusAusM ? 1 : std::max(1, static_cast<int>(std::ceil(a.laengeM / kStufeM)));
        const double l = a.laengeM / stufen;
        for (int s = 0; s < stufen; ++s)
        {
            const double t = (s + 0.5) / stufen;
            const double r = a.radiusEinM + t * (a.radiusAusM - a.radiusEinM);
            kette = mal(kette, zylinder(r, l, omega, luft, b.verluste));
        }
    }
    return {kette.a, kette.b, kette.c, kette.d};
}

Komplex abschlussImpedanz(const Bohrung& b, double hz, const Luft& luft)
{
    if (b.ende == Abschluss::Geschlossen) return Komplex(1e300, 0.0);
    if (b.ende == Abschluss::Ideal || b.abschnitte.empty()) return Komplex(0.0, 0.0);
    const double omega = 2.0 * std::numbers::pi * hz;
    const double r = b.abschnitte.back().radiusAusM;
    const double zc = luft.dichte * luft.schall / (std::numbers::pi * r * r);
    const double ka = omega / luft.schall * r;
    Komplex rad = zc * Komplex(ka * ka / 4.0, 0.6133 * ka);
    if (b.gitterGrenzeHz > 0.0)
    {
        const double x8 = std::pow(hz / b.gitterGrenzeHz, 8.0);
        const double w = x8 / (1.0 + x8);
        rad = (1.0 - w) * rad + w * zc;
    }
    if (b.daempferHz <= 0.0) return rad;
    // der Daempfer: die Abstrahlung der Restflaeche parallel zum Helmholtz-Zweig
    const Komplex zr = rad / std::max(1e-6, b.daempferRest);
    const double w0 = 2.0 * std::numbers::pi * b.daempferHz;
    const Komplex zd = zc * Komplex(1.0 / b.daempferQ, omega / w0 - w0 / std::max(omega, 1e-9));
    return zr * zd / (zr + zd);
}

Komplex eingangsImpedanz(const Bohrung& b, double hz, const Luft& luft)
{
    if (b.abschnitte.empty()) return Komplex(0.0, 0.0);
    const Kettenmatrix k = kettenmatrix(b, hz, luft);
    // der Abschluss
    if (b.ende == Abschluss::Geschlossen) return k.a / k.c;  // U = 0 am Ende
    if (b.ende == Abschluss::Ideal) return k.b / k.d;        // p = 0 am Ende
    const Komplex zl = abschlussImpedanz(b, hz, luft);
    return (k.a * zl + k.b) / (k.c * zl + k.d);
}

std::vector<ImpedanzMode> impedanzModen(const Bohrung& b, double bisHz, bool admittanz, double schrittHz, const Luft& luft)
{
    return modenAusBetrag(
        [&](double f) {
            const Komplex z = eingangsImpedanz(b, f, luft);
            return admittanz ? 1.0 / std::abs(z) : std::abs(z);
        },
        schrittHz, bisHz, schrittHz);
}

std::vector<ImpedanzMode> modenAusBetrag(const std::function<double(double)>& betrag, double vonHz, double bisHz, double schrittHz,
                                         std::size_t hoechstens)
{
    std::vector<ImpedanzMode> moden;
    double f0 = vonHz, f1 = vonHz + schrittHz;
    double y0 = betrag(f0), y1 = betrag(f1);
    for (double f2 = vonHz + 2 * schrittHz; f2 <= bisHz; f2 += schrittHz)
    {
        const double y2 = betrag(f2);
        if (y1 > y0 && y1 >= y2)
        {
            // Goldener Schnitt auf [f0, f2]
            double a = f0, c = f2;
            const double g = 0.5 * (std::sqrt(5.0) - 1.0);
            double x1 = c - g * (c - a), x2 = a + g * (c - a);
            double v1 = betrag(x1), v2 = betrag(x2);
            while (c - a > 1e-6)
            {
                if (v1 > v2)
                {
                    c = x2;
                    x2 = x1;
                    v2 = v1;
                    x1 = c - g * (c - a);
                    v1 = betrag(x1);
                }
                else
                {
                    a = x1;
                    x1 = x2;
                    v1 = v2;
                    x2 = a + g * (c - a);
                    v2 = betrag(x2);
                }
            }
            const double fm = 0.5 * (a + c);
            const double ym = betrag(fm);
            // die Halbwertsbreite: |Z| = Spitze/√2 links und rechts (halbiert)
            const auto rand = [&](double richtung) {
                double innen = fm, aussen = fm;
                double schritt = fm * 1e-3;
                while (betrag(aussen) > ym / std::sqrt(2.0) && std::abs(aussen - fm) < 0.5 * fm)
                {
                    innen = aussen;
                    aussen += richtung * schritt;
                    schritt *= 1.5;
                }
                for (int i = 0; i < 80; ++i)
                {
                    const double m = 0.5 * (innen + aussen);
                    if (betrag(m) > ym / std::sqrt(2.0))
                        innen = m;
                    else
                        aussen = m;
                }
                return 0.5 * (innen + aussen);
            };
            const double breite = rand(1.0) - rand(-1.0);
            moden.push_back({fm, breite > 0.0 ? fm / breite : 1e6, ym});
            if (hoechstens > 0 && moden.size() >= hoechstens) break;
        }
        f0 = f1;
        y0 = y1;
        f1 = f2;
        y1 = y2;
    }
    return moden;
}

RohrKern::RohrKern(const std::vector<ImpedanzMode>& moden, double rate) : m_rate(rate)
{
    setze(moden, m_pol, m_gewicht);
    m_zustand.assign(m_pol.size(), Komplex(0.0, 0.0));
    m_durchgriff = durchgriffAus(moden, rate);
}

void RohrKern::setze(const std::vector<ImpedanzMode>& moden, std::vector<Komplex>& pol, std::vector<Komplex>& gewicht) const
{
    pol.clear();
    gewicht.clear();
    const double dt = 1.0 / m_rate;
    for (const ImpedanzMode& m : moden)
    {
        const double wn = 2.0 * std::numbers::pi * m.hz;
        const double beta = wn / m.q;
        const double wd = std::sqrt(std::max(1e-12, wn * wn - 0.25 * beta * beta));
        const Komplex lambda(-0.5 * beta, wd);
        const double c = m.spitze * beta;  // C_n = Z_max·ω_n/Q_n
        const Komplex r = c * lambda / Komplex(0.0, 2.0 * wd);
        pol.push_back(std::exp(lambda * dt));
        gewicht.push_back(r * dt);
    }
}

double RohrKern::ausgang(const std::vector<Komplex>& gewichte) const
{
    double p = 0.0;
    for (std::size_t i = 0; i < m_zustand.size() && i < gewichte.size(); ++i) p += 2.0 * (gewichte[i] * m_zustand[i]).real();
    return p;
}

double RohrKern::vorher() const
{
    double p = 0.0;
    for (const Komplex& z : m_zustand) p += 2.0 * z.real();
    return p;
}

void RohrKern::nachher(double fluss)
{
    if (m_blende > 0)
    {
        const double t = 1.0 - static_cast<double>(m_blende - 1) / m_blendeSchritte;
        for (std::size_t i = 0; i < m_pol.size(); ++i)
        {
            m_pol[i] = m_polStart[i] + t * (m_polZiel[i] - m_polStart[i]);
            m_gewicht[i] = m_gewichtStart[i] + t * (m_gewichtZiel[i] - m_gewichtStart[i]);
        }
        --m_blende;
    }
    for (std::size_t i = 0; i < m_zustand.size(); ++i) m_zustand[i] = m_pol[i] * (m_zustand[i] + m_gewicht[i] * fluss);
}

void RohrKern::blendeAuf(const std::vector<ImpedanzMode>& moden, int schritte)
{
    std::vector<Komplex> pol, gewicht;
    setze(moden, pol, gewicht);
    m_durchgriff = durchgriffAus(moden, m_rate);
    if (pol.size() != m_pol.size() || schritte <= 0)
    {
        m_pol = pol;
        m_gewicht = gewicht;
        m_zustand.resize(m_pol.size(), Komplex(0.0, 0.0));
        m_blende = 0;
        return;
    }
    m_polStart = m_pol;
    m_gewichtStart = m_gewicht;
    m_polZiel = pol;
    m_gewichtZiel = gewicht;
    m_blende = schritte;
    m_blendeSchritte = schritte;
}

double zylinderSpitzeHz(double laengeM, int n, bool endeOffen, const Luft& luft)
{
    return endeOffen ? (2.0 * n - 1.0) * luft.schall / (4.0 * laengeM) : n * luft.schall / (2.0 * laengeM);
}

}  // namespace ic
