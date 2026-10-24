#include "loop/Boden.hpp"

#include "InstrumentBody/Antwort.hpp"
#include "InstrumentBody/Statistik.hpp"
#include "loop/HammerTon.hpp"

#include <cmath>

namespace lumi::loop
{

std::vector<koerper::ModeImKoerper> alleModen(const koerper::Koerper& koerper)
{
    std::vector<koerper::ModeImKoerper> alle = koerper.moden;
    for (koerper::ModeImKoerper& m : koerper::ziehe(koerper.statistik)) alle.push_back(std::move(m));
    return alle;
}

int stegpunktVon(const koerper::Koerper& koerper, int taste)
{
    for (std::size_t j = 0; j < koerper.stegTasten.size(); ++j)
        if (koerper.stegTasten[j] == taste) return static_cast<int>(j);
    return -1;
}

std::vector<double> bodenAntwort(const std::vector<koerper::ModeImKoerper>& moden, std::size_t stegpunkt,
                                 std::size_t hoerort, int rate, double laengeS, const Fft& umsetzung)
{
    const auto frames = static_cast<std::size_t>(std::ceil(laengeS * rate));
    if (frames == 0) return {};
    const std::size_t n = zweierpotenzAb(frames);
    const std::vector<std::complex<double>> h = koerper::antwortSpektrum(moden, stegpunkt, hoerort, rate, n);
    std::vector<double> zeit(n);
    umsetzung.rueckwaerts(h, zeit);
    zeit.resize(frames);
    return zeit;
}

std::vector<double> schalldruck(std::span<const float> kraft, std::span<const double> antwort, const Fft& umsetzung)
{
    std::vector<double> f(kraft.size());
    for (std::size_t i = 0; i < kraft.size(); ++i) f[i] = static_cast<double>(kraft[i]) * kHammerKraftBezug;
    return falte(umsetzung, f, antwort);
}

}  // namespace lumi::loop
