/**
 * @file ZupfStimmen.cpp
 * @brief Messen am Kern `pluckedString` (Herleitung im Kopf von `ZupfStimmen.hpp`)
 */
#include "InstrumentMeasure/ZupfStimmen.hpp"

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

double hzVon(double note)
{
    return 440.0 * std::exp2((note - 69.0) / 12.0);
}

}  // namespace

ZupfTon messeZupfSumme(std::span<const double> summe, int note, int rate)
{
    ZupfTon m;
    const auto frames = static_cast<std::int64_t>(summe.size());
    if (frames <= 0 || rate <= 0) return m;
    const double fs = rate;
    // IC Zupf.cpp:206–231 wortgleich: die staerkste Spitze nahe der Note, Hann-Fenster 0,1 … 0,5 s, Raster 0,5 Cent ueber
    // ±60 Cent
    const auto ab = static_cast<std::size_t>(0.1 * fs);
    if (frames <= static_cast<std::int64_t>(ab)) return m;  // IC las hier ueber das Ende hinaus
    const auto n = static_cast<std::size_t>(std::min<std::int64_t>(frames - static_cast<std::int64_t>(ab), static_cast<std::int64_t>(0.4 * fs)));
    double best = 0.0;
    for (double c = -60.0; c <= 60.0 && n > 1000; c += 0.5)
    {
        const double f = hzVon(note) * std::exp2(c / 1200.0);
        std::complex<double> z(0.0, 0.0);
        const std::complex<double> dreh = std::polar(1.0, -2.0 * kPi * f / fs);
        std::complex<double> w(1.0, 0.0);
        for (std::size_t i = 0; i < n; ++i)
        {
            z += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * summe[ab + i] * w;
            w *= dreh;
        }
        if (std::abs(z) > best)
        {
            best = std::abs(z);
            m.spielHz = f;
        }
    }
    m.schwingt = best > 0.0;
    return m;
}

ZupfTon messeZupfTon(const kern::zupf::ZupfSpiel& spiel, const kern::zupf::ZupfAnschlag& a, std::int64_t frames)
{
    std::vector<double> summe;
    kern::zupf::rechneZupfSumme(spiel, a, summe, frames);
    if (summe.empty() || spiel.laengeJeNote.punkte.empty()) return {};
    return messeZupfSumme(summe, a.note, a.rate);
}

}  // namespace lumi::messung
