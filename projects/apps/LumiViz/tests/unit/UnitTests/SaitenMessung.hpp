/**
 ****************************************************************************************
 * @file   SaitenMessung.hpp
 * @brief  Saiten-Tests (Stufe 23c, 23d): am gerechneten Ton messen — der Betrag bei
 *         einer Frequenz, die Spitze in einem Bereich, Cent und dB
 *
 * @details
 * Gemeinsam fuer `test_LoopSaiteKs.cpp` und `test_LoopSaiteWg.cpp`. Gemessen wird die
 * linke Seite mit einem Hann-Fenster; die Abtastrate ist `kSaitenRate`.
 ****************************************************************************************
 */

#pragma once

#include "loop/Rechenkern.hpp"  // Klang

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace saitentest
{

inline constexpr int kSaitenRate = 44100;
inline constexpr double kPi = 3.14159265358979323846;

/// Der Betrag bei `hz` im Fenster ab `vonS`, `langS` lang (linke Seite, Hann-Fenster).
inline double betrag(const lumi::loop::Klang& klang, double hz, double vonS, double langS)
{
    const auto von = static_cast<std::int64_t>(vonS * kSaitenRate);
    const auto lang = static_cast<std::int64_t>(langS * kSaitenRate);
    double re = 0.0;
    double im = 0.0;
    for (std::int64_t n = 0; n < lang && von + n < klang.frames(); ++n)
    {
        const double fenster = 0.5 - 0.5 * std::cos(2.0 * kPi * n / (lang - 1));
        const double x = klang.daten[static_cast<std::size_t>(2 * (von + n))] * fenster;
        const double phase = 2.0 * kPi * hz * n / kSaitenRate;
        re += x * std::cos(phase);
        im -= x * std::sin(phase);
    }
    return 2.0 * std::sqrt(re * re + im * im) / (0.5 * lang);
}

/// Die Frequenz des groessten Betrags um `ungefaehr` (± `centWeit`), in Schritten von 0,25 Cent.
inline double spitze(const lumi::loop::Klang& klang, double ungefaehr, double centWeit, double vonS = 0.2, double langS = 1.0)
{
    double beste = ungefaehr;
    double besterBetrag = -1.0;
    for (double cent = -centWeit; cent <= centWeit; cent += 0.25)
    {
        const double hz = ungefaehr * std::exp2(cent / 1200.0);
        const double b = betrag(klang, hz, vonS, langS);
        if (b > besterBetrag)
        {
            besterBetrag = b;
            beste = hz;
        }
    }
    return beste;
}

inline double cent(double hz, double soll)
{
    return 1200.0 * std::log2(hz / soll);
}

inline double db(double verhaeltnis)
{
    return 20.0 * std::log10(std::max(verhaeltnis, 1e-12));
}

/// Der Betrag des k-ten Teiltons zu Beginn (0,05 … 0,25 s), an seiner gemessenen Stelle.
inline double teilton(const lumi::loop::Klang& klang, double f0, int k)
{
    const double hz = spitze(klang, f0 * k, 40.0, 0.05, 0.2);
    return betrag(klang, hz, 0.05, 0.2);
}

}  // namespace saitentest
