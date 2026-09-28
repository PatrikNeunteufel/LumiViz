/**
 ****************************************************************************************
 * @file   NseelZufall.hpp
 * @brief  `rand()` des MilkDrop-EEL (ns-eel2) — Mersenne-Twister mit festem Startwert
 *         und KOMMAZAHL-Ergebnis (Session 80)
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Befund S80 an `martin - stratospheric turbulences`: MilkDrop wuerfelt im
 * Skript anders als AVS.
 *
 * - **AVS** (ns-eel 1): `rand(x) = rand() % max(x,1)` aus der MSVC-Laufzeit —
 *   ganze Zahlen (ScriptContext::nextRand, S49).
 * - **MilkDrop 2** (ns-eel2, `nseel-cfunc.c:101`, gebaut OHNE
 *   `NSEEL_EEL1_COMPAT_MODE` — `vis_milk2/plugin.vcxproj`): Mersenne-Twister
 *   MT19937 mit dem FESTEN Startwert `0x4141f00d`, ein Strom je Prozess, und
 *   `rand(x) = genrand * (1/0xFFFFFFFF) * floor(max(x,1))` — eine
 *   **Kommazahl** in [0, ⌊x⌋]. `rand(1)` ist also gleichverteilt in [0,1],
 *   nicht immer 0.
 *
 * Presets verlassen sich darauf (`if(rand(1)>0.5, …)`, `x0=rand(1)`,
 * `movarg=rand(100)`). Der Generator hier ist `genrand_int32` aus
 * `nseel-cfunc.c:42` Zeile fuer Zeile.
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <cmath>
#include <cstdint>

namespace lumi::scripting {

class NseelZufall
{
public:
    static constexpr std::uint32_t kStart = 0x4141f00du;

    /// Naechste 32-bit-Zahl (genrand_int32, nseel-cfunc.c:42)
    std::uint32_t naechste()
    {
        constexpr int N = 624;
        constexpr int M = 397;
        constexpr std::uint32_t kMatrixA = 0x9908b0dfu;
        constexpr std::uint32_t kUpper = 0x80000000u;
        constexpr std::uint32_t kLower = 0x7fffffffu;
        const std::uint32_t mag01[2] = {0x0u, kMatrixA};

        if (m_mti == 0 && !m_init)
        {
            m_mt[0] = kStart;
            for (m_mti = 1; m_mti < N; ++m_mti)
            {
                m_mt[m_mti] = 1812433253u * (m_mt[m_mti - 1] ^ (m_mt[m_mti - 1] >> 30)) +
                              static_cast<std::uint32_t>(m_mti);
            }
            m_init = true;
        }
        if (m_mti >= N)
        {
            int kk = 0;
            std::uint32_t y = 0;
            for (; kk < N - M; ++kk)
            {
                y = (m_mt[kk] & kUpper) | (m_mt[kk + 1] & kLower);
                m_mt[kk] = m_mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1u];
            }
            for (; kk < N - 1; ++kk)
            {
                y = (m_mt[kk] & kUpper) | (m_mt[kk + 1] & kLower);
                m_mt[kk] = m_mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1u];
            }
            y = (m_mt[N - 1] & kUpper) | (m_mt[0] & kLower);
            m_mt[N - 1] = m_mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1u];
            m_mti = 0;
        }
        std::uint32_t y = m_mt[m_mti++];
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680u;
        y ^= (y << 15) & 0xefc60000u;
        y ^= (y >> 18);
        return y;
    }

    /// EEL `rand(x)` im MilkDrop-Dialekt (nseel_int_rand, ohne COMPAT_MODE)
    double rand(double x)
    {
        double grenze = std::floor(x);
        if (grenze < 1.0) grenze = 1.0;
        return static_cast<double>(naechste()) * (1.0 / static_cast<double>(0xFFFFFFFFu)) *
               grenze;
    }

    /// Zurueck auf den Prozessstart-Zustand
    void zuruecksetzen()
    {
        m_mti = 0;
        m_init = false;
    }

private:
    // Im Original ist `mti` beim Prozessstart 0 und wird beim ersten Aufruf auf
    // N gesetzt, NACHDEM das Feld aus dem Startwert gefuellt ist (Schleife endet
    // mit mti == N) — `m_init` bildet dieses „schon gefuellt" nach.
    std::array<std::uint32_t, 624> m_mt{};
    int m_mti = 0;
    bool m_init = false;
};

} // namespace lumi::scripting
