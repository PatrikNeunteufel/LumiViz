#include "loop/Fft.hpp"

#include <pocketfft_hdronly.h>

#include <algorithm>
#include <stdexcept>

namespace lumi::loop
{

namespace
{

/// pocketfft (externals/pocketfft, FF1 a): r2c und c2r in double, ohne eigenen Thread-Pool.
class FftPocket final : public Fft
{
public:
    [[nodiscard]] std::string name() const override { return "pocketfft c90e55b"; }

    void vorwaerts(std::span<const double> zeit, std::span<std::complex<double>> spektrum) const override
    {
        const std::size_t n = zeit.size();
        if (spektrum.size() < n / 2 + 1) throw std::invalid_argument("Fft::vorwaerts: spectrum too short");
        if (n == 0) return;
        pocketfft::r2c<double>({n}, {static_cast<std::ptrdiff_t>(sizeof(double))},
                               {static_cast<std::ptrdiff_t>(sizeof(std::complex<double>))}, 0, pocketfft::FORWARD,
                               zeit.data(), spektrum.data(), 1.0);
    }

    void rueckwaerts(std::span<const std::complex<double>> spektrum, std::span<double> zeit) const override
    {
        const std::size_t n = zeit.size();
        if (spektrum.size() < n / 2 + 1) throw std::invalid_argument("Fft::rueckwaerts: spectrum too short");
        if (n == 0) return;
        pocketfft::c2r<double>({n}, {static_cast<std::ptrdiff_t>(sizeof(std::complex<double>))},
                               {static_cast<std::ptrdiff_t>(sizeof(double))}, 0, pocketfft::BACKWARD, spektrum.data(),
                               zeit.data(), 1.0 / static_cast<double>(n));
    }
};

}  // namespace

std::vector<std::unique_ptr<Fft>> fftUmsetzungen()
{
    std::vector<std::unique_ptr<Fft>> alle;
    alle.push_back(std::make_unique<FftPocket>());
    return alle;
}

const Fft& fft()
{
    static const FftPocket vorgabe;
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
    std::vector<double> x(n, 0.0), y(n, 0.0);
    std::copy(a.begin(), a.end(), x.begin());
    std::copy(b.begin(), b.end(), y.begin());
    std::vector<std::complex<double>> fx(n / 2 + 1), fy(n / 2 + 1);
    umsetzung.vorwaerts(x, fx);
    umsetzung.vorwaerts(y, fy);
    for (std::size_t k = 0; k < fx.size(); ++k) fx[k] *= fy[k];
    umsetzung.rueckwaerts(fx, x);
    x.resize(laenge);
    return x;
}

}  // namespace lumi::loop
