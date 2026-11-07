/**
 * @file Anschlag.cpp
 * @brief InstrumentCore: die Anschlagstufe (bis S174 `src/loop/HammerSaite.cpp` der App, wortgleich)
 */
#include "InstrumentCore/Anschlag.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::kern
{

int anschlagStufe(double laut)
{
    if (!(laut > 0.0)) return kAnschlagStufeMin;
    const double db = 20.0 * std::log10(laut);
    const auto stufe = static_cast<int>(std::lround(db / kAnschlagSchrittDb));
    return std::clamp(stufe, kAnschlagStufeMin, 0);
}

double anschlagFaktor(int stufe)
{
    return std::pow(10.0, std::clamp(stufe, kAnschlagStufeMin, 0) * kAnschlagSchrittDb / 20.0);
}

double anschlagRest(double laut, int stufe)
{
    return laut / anschlagFaktor(stufe);
}

}  // namespace lumi::kern
