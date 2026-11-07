/**
 * @file Kurve.cpp
 * @brief InstrumentCore: Kurven ueber den Tonumfang (Interpolation wie `Tastenkurve`, HK1 a; aus IC
 *        `rechnung/gemeinsam/Kurve.cpp` `bb7e6d6`, nur der Namensraum geaendert)
 */
#include "InstrumentCore/Kurve.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <sstream>

namespace lumi::kern
{

namespace
{

/// Monoton kubisch zwischen den Punkten: die Steigungen nach Fritsch und Butland, wie PCHIP — innen das gewichtete
/// harmonische Mittel der Nachbarsehnen (0 bei einem Wechsel der Richtung), am Rand die Formel aus drei Punkten, auf
/// die Richtung der ersten Sehne begrenzt. Mit zwei Punkten: die Gerade. (Wortgleich zum Vorbild.)
std::vector<double> steigungen(const std::vector<double>& x, const std::vector<double>& y)
{
    const std::size_t n = x.size();
    std::vector<double> m(n, 0.0);
    if (n < 2) return m;
    std::vector<double> h(n - 1);
    std::vector<double> d(n - 1);
    for (std::size_t k = 0; k + 1 < n; ++k)
    {
        h[k] = x[k + 1] - x[k];
        d[k] = (y[k + 1] - y[k]) / h[k];
    }
    if (n == 2)
    {
        m[0] = m[1] = d[0];
        return m;
    }
    for (std::size_t k = 1; k + 1 < n; ++k)
    {
        if (d[k - 1] * d[k] <= 0.0) continue;
        const double w1 = 2.0 * h[k] + h[k - 1];
        const double w2 = h[k] + 2.0 * h[k - 1];
        m[k] = (w1 + w2) / (w1 / d[k - 1] + w2 / d[k]);
    }
    const auto rand = [](double h0, double h1, double d0, double d1) {
        double s = ((2.0 * h0 + h1) * d0 - h0 * d1) / (h0 + h1);
        if (s * d0 <= 0.0)
            s = 0.0;
        else if (d0 * d1 < 0.0 && std::abs(s) > 3.0 * std::abs(d0))
            s = 3.0 * d0;
        return s;
    };
    m[0] = rand(h[0], h[1], d[0], d[1]);
    m[n - 1] = rand(h[n - 2], h[n - 3], d[n - 2], d[n - 3]);
    return m;
}

double inSkala(double wert, Skala skala)
{
    return skala == Skala::Logarithmisch ? std::log(std::max(wert, 1e-300)) : wert;
}

double ausSkala(double wert, Skala skala)
{
    return skala == Skala::Logarithmisch ? std::exp(wert) : wert;
}

void richtePaare(std::vector<NotenWert>& paare, double von, double bis, int noteMin, int noteMax)
{
    std::vector<NotenWert> gut;
    for (NotenWert p : paare)
    {
        if (!std::isfinite(p.wert)) continue;
        p.note = std::clamp(p.note, noteMin, noteMax);
        p.wert = std::clamp(p.wert, von, bis);
        gut.push_back(p);
    }
    // je Note der erste Eintrag
    std::stable_sort(gut.begin(), gut.end(), [](const NotenWert& a, const NotenWert& b) { return a.note < b.note; });
    gut.erase(std::unique(gut.begin(), gut.end(), [](const NotenWert& a, const NotenWert& b) { return a.note == b.note; }),
              gut.end());
    paare = std::move(gut);
}

}  // namespace

double wertAn(const Kurve& kurve, int note, Skala skala)
{
    for (const NotenWert& a : kurve.abweichungen)
        if (a.note == note) return a.wert;
    const auto& p = kurve.punkte;
    if (p.empty()) return 0.0;
    if (note <= p.front().note) return p.front().wert;
    if (note >= p.back().note) return p.back().wert;
    std::vector<double> x;
    std::vector<double> y;
    x.reserve(p.size());
    y.reserve(p.size());
    for (const NotenWert& w : p)
    {
        x.push_back(w.note);
        y.push_back(inSkala(w.wert, skala));
    }
    const std::vector<double> m = steigungen(x, y);
    const auto oben = std::upper_bound(x.begin(), x.end(), static_cast<double>(note));
    const auto k = static_cast<std::size_t>(std::distance(x.begin(), oben)) - 1;
    const double h = x[k + 1] - x[k];
    const double t = (note - x[k]) / h;
    const double t2 = t * t;
    const double t3 = t2 * t;
    const double wert = (2 * t3 - 3 * t2 + 1) * y[k] + (t3 - 2 * t2 + t) * h * m[k] + (-2 * t3 + 3 * t2) * y[k + 1] +
                        (t3 - t2) * h * m[k + 1];
    return ausSkala(wert, skala);
}

void richteKurve(Kurve& kurve, double von, double bis, const Kurve& ersatz, int noteMin, int noteMax)
{
    richtePaare(kurve.punkte, von, bis, noteMin, noteMax);
    richtePaare(kurve.abweichungen, von, bis, noteMin, noteMax);
    if (kurve.punkte.empty())
    {
        kurve.punkte = ersatz.punkte;
        kurve.belegkraft = ersatz.belegkraft;
        kurve.herkunft = ersatz.herkunft;
    }
}

Kurve kurveAusPunkten(std::initializer_list<NotenWert> punkte, Belegkraft belegkraft, std::string herkunft)
{
    Kurve kurve;
    kurve.punkte.assign(punkte.begin(), punkte.end());
    kurve.belegkraft = belegkraft;
    kurve.herkunft = std::move(herkunft);
    return kurve;
}

Kurve kurveKonstant(double wert, Belegkraft belegkraft, std::string herkunft, int noteMin, int noteMax)
{
    return kurveAusPunkten({{noteMin, wert}, {noteMax, wert}}, belegkraft, std::move(herkunft));
}

std::string kurveAlsText(const Kurve& kurve)
{
    std::ostringstream text;
    text.precision(6);
    for (std::size_t i = 0; i < kurve.punkte.size(); ++i)
        text << (i ? " " : "") << kurve.punkte[i].note << ':' << kurve.punkte[i].wert;
    return text.str();
}

}  // namespace lumi::kern
