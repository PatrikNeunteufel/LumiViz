/**
 * @file Fft.cpp
 * @brief Gemeinsam: die FFT mit pocketfft (BSD-3) und die direkte DFT fuer die Tests
 */
#include "gemeinsam/Fft.hpp"

#include "pocketfft_hdronly.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic
{

namespace
{

class PocketFft final : public Fft
{
public:
    [[nodiscard]] std::string name() const override { return "pocketfft"; }

    void vorwaerts(std::span<const double> zeit, std::span<std::complex<double>> spektrum) const override
    {
        const std::size_t n = zeit.size();
        if (n == 0) return;
        const pocketfft::shape_t form{n};
        const pocketfft::stride_t schrittZeit{static_cast<std::ptrdiff_t>(sizeof(double))};
        const pocketfft::stride_t schrittSpektrum{static_cast<std::ptrdiff_t>(sizeof(std::complex<double>))};
        // ein Thread: dieselbe Reihenfolge der Rechnung bei jedem Aufruf (E1 a)
        pocketfft::r2c(form, schrittZeit, schrittSpektrum, 0, pocketfft::FORWARD, zeit.data(), spektrum.data(), 1.0, 1);
    }

    void rueckwaerts(std::span<const std::complex<double>> spektrum, std::span<double> zeit) const override
    {
        const std::size_t n = zeit.size();
        if (n == 0) return;
        const pocketfft::shape_t form{n};
        const pocketfft::stride_t schrittZeit{static_cast<std::ptrdiff_t>(sizeof(double))};
        const pocketfft::stride_t schrittSpektrum{static_cast<std::ptrdiff_t>(sizeof(std::complex<double>))};
        pocketfft::c2r(form, schrittSpektrum, schrittZeit, 0, pocketfft::BACKWARD, spektrum.data(), zeit.data(),
                       1.0 / static_cast<double>(n), 1);
    }
};

/// Die DFT nach Definition — O(n²), nur als zweite Umsetzung fuer die Tests.
class DirekteDft final : public Fft
{
public:
    [[nodiscard]] std::string name() const override { return "direkt"; }

    void vorwaerts(std::span<const double> zeit, std::span<std::complex<double>> spektrum) const override
    {
        const std::size_t n = zeit.size();
        for (std::size_t k = 0; k <= n / 2; ++k)
        {
            std::complex<double> summe(0.0, 0.0);
            for (std::size_t t = 0; t < n; ++t)
            {
                const double w = -2.0 * std::numbers::pi * static_cast<double>((k * t) % n) / static_cast<double>(n);
                summe += zeit[t] * std::complex<double>(std::cos(w), std::sin(w));
            }
            spektrum[k] = summe;
        }
    }

    void rueckwaerts(std::span<const std::complex<double>> spektrum, std::span<double> zeit) const override
    {
        const std::size_t n = zeit.size();
        for (std::size_t t = 0; t < n; ++t)
        {
            double summe = 0.0;
            for (std::size_t k = 0; k < n; ++k)
            {
                // das volle Spektrum aus der Haelfte (hermitesch)
                const std::complex<double> x = k <= n / 2 ? spektrum[k] : std::conj(spektrum[n - k]);
                const double w = 2.0 * std::numbers::pi * static_cast<double>((k * t) % n) / static_cast<double>(n);
                summe += (x * std::complex<double>(std::cos(w), std::sin(w))).real();
            }
            zeit[t] = summe / static_cast<double>(n);
        }
    }
};

}  // namespace

std::vector<std::unique_ptr<Fft>> fftUmsetzungen()
{
    std::vector<std::unique_ptr<Fft>> alle;
    alle.push_back(std::make_unique<PocketFft>());
    alle.push_back(std::make_unique<DirekteDft>());
    return alle;
}

const Fft& fft()
{
    static const PocketFft vorgabe;
    return vorgabe;
}

std::size_t zweierpotenzAb(std::size_t n)
{
    std::size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

std::vector<double> falte(const Fft& umsetzung, std::span<const double> a, std::span<const double> b)
{
    if (a.empty() || b.empty()) return {};
    const std::size_t laenge = a.size() + b.size() - 1;
    const std::size_t n = zweierpotenzAb(laenge);
    std::vector<double> za(n, 0.0);
    std::vector<double> zb(n, 0.0);
    std::copy(a.begin(), a.end(), za.begin());
    std::copy(b.begin(), b.end(), zb.begin());
    std::vector<std::complex<double>> sa(n / 2 + 1);
    std::vector<std::complex<double>> sb(n / 2 + 1);
    umsetzung.vorwaerts(za, sa);
    umsetzung.vorwaerts(zb, sb);
    for (std::size_t k = 0; k < sa.size(); ++k) sa[k] *= sb[k];
    umsetzung.rueckwaerts(sa, za);
    za.resize(laenge);
    return za;
}

}  // namespace ic
