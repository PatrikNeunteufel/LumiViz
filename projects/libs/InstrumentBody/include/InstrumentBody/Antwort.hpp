/**
 * @file Antwort.hpp
 * @brief Die Antwort des Bodens: Schalldruck am Hoerort je Kraft am Stegpunkt, aus den Moden
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S149: AW1 a, FT1 a, PG1 a;
 *        Stufe 31c-7)
 *
 * Je Mode n (bezueglich M normiert) ist die modale Koordinate q_n die Antwort eines Einmassen-
 * schwingers auf φ_n(A)·F, und der Druck am Hoerort p = Σ q̈_n G_n (`Auswertung.hpp`). Im Spektrum:
 *
 *     H(ω) = Σ_n φ_n(A) G_n · (−ω²) / (ω_n² − ω² + j η_n ω_n ω)      in Pa/N
 *
 * Die Antwort wird nach AW1 a direkt im Spektrum gebildet, an den Frequenzpunkten der FFT, mit der
 * sie danach gefaltet wird (`antwortSpektrum`). Die Rueckwandlung in eine Zeitreihe gibt die
 * Koeffizienten des Filters je Abtastwert: IDFT(H(ω_k)) = T · h(kT).
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek.
 */
#pragma once

#include "InstrumentBody/Auswertung.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <vector>

namespace lumi::koerper
{

/// H bei `frequenzHz` fuer eine Kraft am Stegpunkt `stegpunkt` und den Druck am Hoerort `hoerort`, Pa/N.
[[nodiscard]] inline std::complex<double> antwort(const std::vector<ModeImKoerper>& moden, std::size_t stegpunkt,
                                                  std::size_t hoerort, double frequenzHz)
{
    const double w = 2.0 * std::numbers::pi * frequenzHz;
    std::complex<double> summe{0.0, 0.0};
    for (const ModeImKoerper& m : moden)
    {
        if (stegpunkt >= m.amStegpunkt.size() || hoerort >= m.abstrahlung.size()) continue;
        const double wn = 2.0 * std::numbers::pi * m.frequenzHz;
        summe += m.amStegpunkt[stegpunkt] * m.abstrahlung[hoerort] * (-w * w) /
                 std::complex<double>(wn * wn - w * w, m.verlustfaktor * wn * w);
    }
    return summe;
}

/// H an den n/2 + 1 Frequenzpunkten einer FFT der Laenge n bei der Abtastrate `rate` (f_k = k·rate/n),
/// in Pa/N — gleich zu falten mit dem Spektrum der Kraft in N (AW1 a).
[[nodiscard]] inline std::vector<std::complex<double>> antwortSpektrum(const std::vector<ModeImKoerper>& moden,
                                                                       std::size_t stegpunkt, std::size_t hoerort,
                                                                       double rate, std::size_t n)
{
    // Real- und Imaginaerteil getrennt und die Division von Hand: a·(−ω²)/(d_r + j d_i)
    // = −ω²/(d_r² + d_i²) · a·(d_r − j d_i) — so laesst sich die innere Schleife vektorisieren
    // (die Division zweier std::complex prueft auf Unendlich und NaN)
    const std::size_t bins = n / 2 + 1;
    std::vector<double> w(bins), w2(bins), re(bins, 0.0), im(bins, 0.0);
    const double dw = 2.0 * std::numbers::pi * rate / static_cast<double>(n);
    for (std::size_t k = 0; k < bins; ++k)
    {
        w[k] = dw * static_cast<double>(k);
        w2[k] = w[k] * w[k];
    }
    for (const ModeImKoerper& m : moden)
    {
        if (stegpunkt >= m.amStegpunkt.size() || hoerort >= m.abstrahlung.size()) continue;
        const std::complex<double> a = m.amStegpunkt[stegpunkt] * m.abstrahlung[hoerort];
        const double ar = a.real(), ai = a.imag();
        const double wn = 2.0 * std::numbers::pi * m.frequenzHz;
        const double wn2 = wn * wn;
        const double daempfung = m.verlustfaktor * wn;
        for (std::size_t k = 1; k < bins; ++k)  // bei 0 Hz ist H = 0
        {
            const double dr = wn2 - w2[k];
            const double di = daempfung * w[k];
            const double t = -w2[k] / (dr * dr + di * di);
            re[k] += t * (ar * dr + ai * di);
            im[k] += t * (ai * dr - ar * di);
        }
    }
    std::vector<std::complex<double>> h(bins);
    for (std::size_t k = 0; k < bins; ++k) h[k] = {re[k], im[k]};
    return h;
}

/// So lang klingt die Antwort: bis die langsamste Mode um 60 dB gefallen ist (Amplitude
/// e^(−η ω_n t / 2)), hoechstens `hoechstensS` (AW1 a: 8 s).
[[nodiscard]] inline double antwortLaengeS(const std::vector<ModeImKoerper>& moden, double hoechstensS = 8.0)
{
    double laengste = 0.0;
    for (const ModeImKoerper& m : moden)
    {
        const double sigma = 0.5 * m.verlustfaktor * 2.0 * std::numbers::pi * m.frequenzHz;
        if (sigma > 0.0) laengste = std::max(laengste, std::log(1000.0) / sigma);
    }
    return std::min(laengste, hoechstensS);
}

}  // namespace lumi::koerper
