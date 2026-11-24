/**
 * @file Steg.cpp
 * @brief InstrumentCore: der Koerper am Steg (GZ5 a, GZ7 a; 34k-7b)
 */
#include "InstrumentCore/Steg.hpp"

#include <cmath>

namespace lumi::kern
{

const Steg::Punkt* Steg::punktZu(int taste) const
{
    for (const Steg::Punkt& p : punkte)
        if (p.taste == taste) return &p;
    return nullptr;
}

std::vector<std::string> pruefeSteg(const Steg& steg)
{
    std::vector<std::string> fehler;
    for (std::size_t i = 0; i < steg.moden.size(); ++i)
    {
        const Steg::Mode& m = steg.moden[i];
        const std::string wo = "mode " + std::to_string(i);
        if (!(m.hz > 0.0) || !std::isfinite(m.hz)) fehler.push_back(wo + ": hz must be positive");
        if (!(m.verlust > 0.0) || !std::isfinite(m.verlust)) fehler.push_back(wo + ": loss factor must be positive");
        if (!(std::abs(m.winkelGrad) <= 90.0)) fehler.push_back(wo + ": angle outside -90 ... 90");
        if (i > 0 && m.hz < steg.moden[i - 1].hz) fehler.push_back(wo + ": not ordered by hz");
    }
    for (std::size_t i = 0; i < steg.punkte.size(); ++i)
    {
        const Steg::Punkt& p = steg.punkte[i];
        const std::string wo = "point " + std::to_string(i);
        if (p.taste < 0 || p.taste > 127) fehler.push_back(wo + ": key outside 0 ... 127");
        for (std::size_t j = 0; j < i; ++j)
            if (steg.punkte[j].taste == p.taste)
            {
                fehler.push_back(wo + ": key " + std::to_string(p.taste) + " twice");
                break;
            }
        if (p.form.size() != steg.moden.size())
            fehler.push_back(wo + ": " + std::to_string(p.form.size()) + " shape values for " +
                             std::to_string(steg.moden.size()) + " modes");
        for (const double v : p.form)
            if (!std::isfinite(v))
            {
                fehler.push_back(wo + ": shape value not a number");
                break;
            }
        if (!(p.streuung >= 0.0) || !std::isfinite(p.streuung)) fehler.push_back(wo + ": spread negative");
    }
    const Steg::Dichte& d = steg.dichte;
    if (!(d.dichte >= 0.0) || !(d.anstieg >= 0.0) || !std::isfinite(d.dichte) || !std::isfinite(d.anstieg))
        fehler.push_back("density negative");
    if (d.dichte > 0.0 && !(d.bisHz > d.vonHz)) fehler.push_back("density: bisHz not above vonHz");
    return fehler;
}

}  // namespace lumi::kern
