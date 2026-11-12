/**
 * @file Grundton.cpp
 * @brief Der Grundton aus den Teiltoenen (Herleitung im Kopf von `Grundton.hpp`)
 */
#include "InstrumentMeasure/Grundton.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

namespace lumi::messung
{

namespace
{

constexpr double kPi = std::numbers::pi;

/// |X(f)| des gefensterten Ausschnitts.
class Ausschnitt
{
public:
    Ausschnitt(std::span<const double> signal, std::size_t ab, std::size_t n, int rate) : m_rate(rate), m_x(n)
    {
        for (std::size_t i = 0; i < n; ++i) m_x[i] = (0.5 - 0.5 * std::cos(2.0 * kPi * static_cast<double>(i) / static_cast<double>(n))) * signal[ab + i];
    }
    [[nodiscard]] double betrag(double f) const
    {
        const std::complex<double> dreh = std::polar(1.0, -2.0 * kPi * f / m_rate);
        std::complex<double> w(1.0, 0.0), z(0.0, 0.0);
        for (std::size_t i = 0; i < m_x.size(); ++i)
        {
            z += m_x[i] * w;
            w *= dreh;
            if ((i & 1023u) == 1023u) w /= std::abs(w);  // die Drehung bleibt auf dem Einheitskreis
        }
        return std::abs(z);
    }

private:
    int m_rate;
    std::vector<double> m_x;
};

struct Spitze
{
    int k = 0;
    double hz = 0.0, betrag = 0.0;
    bool innen = false;
};

/// Die Spitze nahe `mitteHz` in ±`breiteCent`: Raster 1 Cent, dann Goldener Schnitt.
Spitze spitzeBei(const Ausschnitt& a, int k, double mitteHz, double breiteCent)
{
    Spitze s;
    s.k = k;
    const int schritte = static_cast<int>(std::lround(breiteCent));
    int bester = -schritte;
    for (int c = -schritte; c <= schritte; ++c)
    {
        const double b = a.betrag(mitteHz * std::exp2(c / 1200.0));
        if (b > s.betrag)
        {
            s.betrag = b;
            bester = c;
        }
    }
    s.innen = bester > -schritte && bester < schritte;
    double lo = bester - 1.0, hi = bester + 1.0;
    const double g = (std::sqrt(5.0) - 1.0) / 2.0;
    const auto wert = [&](double c) { return a.betrag(mitteHz * std::exp2(c / 1200.0)); };
    double x1 = hi - g * (hi - lo), x2 = lo + g * (hi - lo), f1 = wert(x1), f2 = wert(x2);
    while (hi - lo > 1e-5)
    {
        if (f1 > f2)
        {
            hi = x2;
            x2 = x1;
            f2 = f1;
            x1 = hi - g * (hi - lo);
            f1 = wert(x1);
        }
        else
        {
            lo = x1;
            x1 = x2;
            f1 = f2;
            x2 = lo + g * (hi - lo);
            f2 = wert(x2);
        }
    }
    const double c = 0.5 * (lo + hi);
    s.hz = mitteHz * std::exp2(c / 1200.0);
    s.betrag = std::max(s.betrag, wert(c));
    return s;
}

/// Die gewichtete Gerade (f_k/k)² = a + b·k² → f₀ = √a, B = b/a; mit einem Teilton B = 0.
Grundton gerade(const std::vector<Spitze>& sp)
{
    Grundton g;
    g.teiltoene = static_cast<int>(sp.size());
    if (sp.empty()) return g;
    if (sp.size() == 1)
    {
        g.hz = sp[0].hz / sp[0].k;
        g.gefunden = g.hz > 0.0;
        return g;
    }
    double sw = 0.0, sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    for (const Spitze& s : sp)
    {
        const double w = s.betrag * s.betrag, x = double(s.k) * s.k, y = std::pow(s.hz / s.k, 2.0);
        sw += w;
        sx += w * x;
        sy += w * y;
        sxx += w * x * x;
        sxy += w * x * y;
    }
    const double nenner = sw * sxx - sx * sx;
    const double b = std::abs(nenner) > 0.0 ? (sw * sxy - sx * sy) / nenner : 0.0;
    const double a = (sy - b * sx) / sw;
    if (!(a > 0.0)) return g;
    g.hz = std::sqrt(a);
    g.steife = b / a;
    g.gefunden = true;
    return g;
}

}  // namespace

Grundton grundtonAusTeiltoenen(std::span<const double> signal, int rate, double schaetzHz, double abS, double dauerS, int hoechstens,
                               double untergrenzeDb)
{
    Grundton g;
    if (rate <= 0 || !(schaetzHz > 0.0) || hoechstens < 1) return g;
    const auto ab = static_cast<std::size_t>(std::max(0.0, abS) * rate);
    if (signal.size() <= ab) return g;
    const std::size_t n = std::min(signal.size() - ab, static_cast<std::size_t>(dauerS * rate));
    if (n < 1000) return g;
    const Ausschnitt a(signal, ab, n, rate);
    const auto sammle = [&](const Grundton* vorher, double breiteCent) {
        std::vector<Spitze> sp;
        for (int k = 1; k <= hoechstens; ++k)
        {
            const double soll = vorher != nullptr ? k * vorher->hz * std::sqrt(1.0 + vorher->steife * k * k) : k * schaetzHz;
            if (soll * std::exp2(breiteCent / 1200.0) >= 0.45 * rate) break;
            const Spitze s = spitzeBei(a, k, soll, breiteCent);
            if (s.innen && s.betrag > 0.0) sp.push_back(s);
        }
        double staerkste = 0.0;
        for (const Spitze& s : sp) staerkste = std::max(staerkste, s.betrag);
        const double schwelle = staerkste * std::pow(10.0, untergrenzeDb / 20.0);
        sp.erase(std::remove_if(sp.begin(), sp.end(), [&](const Spitze& s) { return s.betrag < schwelle; }), sp.end());
        return gerade(sp);
    };
    const Grundton erster = sammle(nullptr, 50.0);
    if (!erster.gefunden) return erster;
    const Grundton zweiter = sammle(&erster, 20.0);
    return zweiter.gefunden ? zweiter : erster;
}

}  // namespace lumi::messung
