/**
 ****************************************************************************************
 * @file   MilkdropFft.hpp
 * @brief  MilkDrops eigene Audio-Aufbereitung fuer Custom-Waves: FFT mit Huellkurve
 *         und Entzerrung (Port von `vis_milk2/fft.cpp`) — Session 80
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Befund S80 an `martin - stratospheric turbulences`: Custom-Waves mit
 * `bSpectrum=1` bekamen bei uns das Spektrum der App (BASS-FFT) mit einem per
 * Auge geschaetzten Faktor (S39). Das Original rechnet ein EIGENES FFT aus den
 * 576 8-bit-Samples, die es auch den Skripten gibt (`pluginshell.cpp:1655`):
 *
 * 1. `fWaveform[i] = (pcm[i] ^ 128) - 128` — Werte −128…127;
 * 2. Daempfung `temp[i] = 0.5*(w[i] + w[i-1])` (bei i = 0: w[0] doppelt);
 * 3. FFT ueber 1024 Punkte (576 Samples, Rest 0), Sinus-Huellkurve (Potenz 1),
 *    Entzerrung `−0.02·ln((512−i)/512)` je Band, Betrag → 512 Werte.
 *
 * Custom-Waves multiplizieren danach mit 0.15 (Spektrum) bzw. 0.004
 * (Wellenform) — `milkdropfs.cpp:2555`. Rechnung in `float` wie im Original,
 * damit dieselbe Eingabe dieselben Zahlen ergibt.
 *
 * Der Code folgt `fft.cpp` (Copyright 2005-2013 Nullsoft, BSD-Lizenz —
 * THIRD_PARTY_NOTICES) Schritt fuer Schritt.
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <cmath>
#include <vector>

namespace lumi::milkdrop {

class MilkdropFft
{
public:
    static constexpr int kEingang = 576;  ///< Samples je Kanal (Winamp)
    static constexpr int kBaender = 512;  ///< NUM_FREQUENCIES
    static constexpr int kN = kBaender * 2;  ///< FFT-Laenge (NFREQ)

    MilkdropFft()
    {
        // Bit-Umkehr-Tabelle (InitBitRevTable)
        for (int i = 0; i < kN; ++i) m_bitrev[i] = i;
        for (int i = 0, j = 0; i < kN; ++i)
        {
            if (j > i)
            {
                const int t = m_bitrev[i];
                m_bitrev[i] = m_bitrev[j];
                m_bitrev[j] = t;
            }
            int m = kN >> 1;
            while (m >= 1 && j >= m)
            {
                j -= m;
                m >>= 1;
            }
            j += m;
        }
        // cos/sin je Stufe (InitCosSinTable)
        constexpr float kPi = 3.141592653589793238462643383279502884197169399f;
        for (int dft = 2, t = 0; dft <= kN; dft <<= 1, ++t)
        {
            const float theta = -2.0f * kPi / static_cast<float>(dft);
            m_cossin.push_back({std::cos(theta), std::sin(theta)});
        }
        // Huellkurve, Potenz 1 (InitEnvelopeTable)
        const float mult = 1.0f / static_cast<float>(kEingang) * 6.2831853f;
        for (int i = 0; i < kEingang; ++i)
            m_huelle[i] = 0.5f + 0.5f * std::sin(static_cast<float>(i) * mult - 1.5707963268f);
        // Entzerrung (InitEqualizeTable)
        const float inv = 1.0f / static_cast<float>(kN / 2);
        for (int i = 0; i < kN / 2; ++i)
            m_entzerr[i] = -0.02f * std::log(static_cast<float>(kN / 2 - i) * inv);
    }

    /// 8-bit-Wellenform (−128…127) aus einer Float-Wellenform (−1…1), wie der
    /// MilkdropRef-Host sie baut (`lround(x·127)`) und Winamp sie liefert.
    static void quantisiere(const float* wave, std::array<float, kEingang>& aus)
    {
        for (int i = 0; i < kEingang; ++i)
        {
            float x = wave[i];
            if (x > 1.0f) x = 1.0f;
            if (x < -1.0f) x = -1.0f;
            aus[static_cast<std::size_t>(i)] = static_cast<float>(std::lround(x * 127.0));
        }
    }

    /// Spektrum wie `m_sound.fSpectrum` aus einer 8-bit-Wellenform (−128…127),
    /// inklusive der Daempfung davor (pluginshell.cpp:1676).
    void spektrum(const std::array<float, kEingang>& welle, std::array<float, kBaender>& aus)
    {
        std::array<float, kEingang> gedaempft{};
        int alt = 0;
        for (int i = 0; i < kEingang; ++i)
        {
            gedaempft[static_cast<std::size_t>(i)] =
                0.5f * (welle[static_cast<std::size_t>(i)] + welle[static_cast<std::size_t>(alt)]);
            alt = i;
        }
        zeitZuFrequenz(gedaempft.data(), aus.data());
    }

private:
    /// time_to_frequency_domain (fft.cpp), Huellkurve + Entzerrung aktiv
    void zeitZuFrequenz(const float* ein, float* aus)
    {
        for (int i = 0; i < kN; ++i)
        {
            const int idx = m_bitrev[i];
            m_re[i] = idx < kEingang ? ein[idx] * m_huelle[idx] : 0.0f;
            m_im[i] = 0.0f;
        }
        int t = 0;
        for (int dft = 2; dft <= kN; dft <<= 1, ++t)
        {
            const float wpr = m_cossin[static_cast<std::size_t>(t)][0];
            const float wpi = m_cossin[static_cast<std::size_t>(t)][1];
            float wr = 1.0f;
            float wi = 0.0f;
            const int h = dft >> 1;
            for (int m = 0; m < h; ++m)
            {
                for (int i = m; i < kN; i += dft)
                {
                    const int j = i + h;
                    const float tr = wr * m_re[j] - wi * m_im[j];
                    const float ti = wr * m_im[j] + wi * m_re[j];
                    m_re[j] = m_re[i] - tr;
                    m_im[j] = m_im[i] - ti;
                    m_re[i] += tr;
                    m_im[i] += ti;
                }
                const float wtemp = wr;
                wr = wtemp * wpr - wi * wpi;
                wi = wi * wpr + wtemp * wpi;
            }
        }
        for (int i = 0; i < kN / 2; ++i)
            aus[i] = m_entzerr[i] * std::sqrt(m_re[i] * m_re[i] + m_im[i] * m_im[i]);
    }

    std::array<int, kN> m_bitrev{};
    std::vector<std::array<float, 2>> m_cossin;
    std::array<float, kEingang> m_huelle{};
    std::array<float, kN / 2> m_entzerr{};
    std::array<float, kN> m_re{};
    std::array<float, kN> m_im{};
};

}  // namespace lumi::milkdrop
