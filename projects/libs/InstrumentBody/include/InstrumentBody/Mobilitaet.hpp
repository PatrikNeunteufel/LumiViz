/**
 * @file Mobilitaet.hpp
 * @brief Die Mobilitaet am Steg aus den Moden des Koerpers — fuer den Pruefstein
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S149: PS1 a, KV1 a; Stufe 31c-6)
 *
 * **Treiberpunkt-Mobilitaet** (Geschwindigkeit je Kraft, s/kg) am Stegpunkt A nach MAESSTRO
 * (Elie, Cotté & Boutillon 2022) Gl. 10, die Formen bezueglich M normiert (m_n = 1):
 *
 *     Y_A(ω) = jω Σ_n φ_n(A)² / (ω_n² + j η_n ω_n ω − ω²)
 *
 * **Mittlere Mobilitaet** je Stegpunkt nach MAESSTRO Gl. 12: das geometrische Mittel von |Y_A| ueber
 * die Frequenzpunkte von 25 bis 5000 Hz (hier in Schritten von 1 Hz), in dB re 1 s/kg.
 *
 * **Terzbaender** nach IEC 61260 (Basis 10): Mitten 10^(n/10) Hz, Grenzen Mitte · 10^(±1/20). Je Band
 * das geometrische Mittel von |Y| ueber die Frequenzpunkte im Band und ueber die gewaehlten
 * Stegpunkte (in dB gemittelt); die Modendichte je Band als Zahl der Moden durch die Breite.
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek.
 */
#pragma once

#include "InstrumentBody/Auswertung.hpp"

#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <vector>

namespace lumi::koerper
{

/// Ab dieser Taste zaehlt ein Stegpunkt zum Diskant (PF2 a, S150): MAESSTRO Abb. 6 zaehlt die Noten
/// ab C1 (Index 1–84 = MIDI 24–107), der Bassteg traegt die Indizes 1–17 = MIDI 24–40 — wie in
/// RT-0425 Anhang A, wo der Bass-Arm bei Taste 40 endet. PS1 a mittelt die Baender ueber den Diskant.
inline constexpr int kDiskantAb = 41;

/// Y_A bei `frequenzHz` am Stegpunkt `stegpunkt` aus den Moden (MAESSTRO Gl. 10), s/kg.
[[nodiscard]] inline std::complex<double> mobilitaet(const std::vector<ModeImKoerper>& moden, std::size_t stegpunkt, double frequenzHz)
{
    const double w = 2.0 * std::numbers::pi * frequenzHz;
    std::complex<double> summe{0.0, 0.0};
    for (const ModeImKoerper& m : moden)
    {
        if (stegpunkt >= m.amStegpunkt.size()) continue;
        const double wn = 2.0 * std::numbers::pi * m.frequenzHz;
        const double phi = m.amStegpunkt[stegpunkt];
        summe += phi * phi / std::complex<double>(wn * wn - w * w, m.verlustfaktor * wn * w);
    }
    return std::complex<double>(0.0, w) * summe;
}

/// 20 log10 |Y| in dB re 1 s/kg.
[[nodiscard]] inline double mobilitaetDb(std::complex<double> y)
{
    return 20.0 * std::log10(std::abs(y));
}

/// Das geometrische Mittel von |Y_A| ueber [vonHz, bisHz] in Schritten von `schrittHz`
/// (MAESSTRO Gl. 12), in dB re 1 s/kg.
[[nodiscard]] inline double mittlereMobilitaetDb(const std::vector<ModeImKoerper>& moden, std::size_t stegpunkt, double vonHz = 25.0,
                                                 double bisHz = 5000.0, double schrittHz = 1.0)
{
    double summeDb = 0.0;
    int zahl = 0;
    for (double f = vonHz; f <= bisHz + 1e-9; f += schrittHz)
    {
        summeDb += mobilitaetDb(mobilitaet(moden, stegpunkt, f));
        ++zahl;
    }
    return zahl > 0 ? summeDb / zahl : 0.0;
}

/// Ein Terzband (IEC 61260, Basis 10).
struct Terzband
{
    double mitteHz = 0.0;
    double vonHz = 0.0;
    double bisHz = 0.0;
};

/// Die Terzbaender von dem mit der Nennmitte `vonHz` bis zu dem mit der Nennmitte `bisHz`
/// (Mitten 10^(n/10) Hz; die Nennmitte „5000 Hz" ist gerechnet 5012 Hz — gewaehlt wird ueber n).
[[nodiscard]] inline std::vector<Terzband> terzbaender(double vonHz, double bisHz)
{
    std::vector<Terzband> baender;
    const int von = static_cast<int>(std::lround(10.0 * std::log10(vonHz)));
    const int bis = static_cast<int>(std::lround(10.0 * std::log10(bisHz)));
    const double faktor = std::pow(10.0, 1.0 / 20.0);
    for (int n = von; n <= bis; ++n)
    {
        const double mitte = std::pow(10.0, n / 10.0);
        baender.push_back({mitte, mitte / faktor, mitte * faktor});
    }
    return baender;
}

/// Die mittlere Mobilitaet im Band: das geometrische Mittel von |Y| ueber die Frequenzpunkte im
/// Band (Schritte von `schrittHz`, ab der unteren Grenze) und ueber die Stegpunkte `stegpunkte`,
/// in dB re 1 s/kg.
[[nodiscard]] inline double bandMobilitaetDb(const std::vector<ModeImKoerper>& moden, const std::vector<std::size_t>& stegpunkte,
                                             const Terzband& band, double schrittHz = 1.0)
{
    double summeDb = 0.0;
    int zahl = 0;
    for (const std::size_t j : stegpunkte)
        for (double f = band.vonHz; f < band.bisHz; f += schrittHz)
        {
            summeDb += mobilitaetDb(mobilitaet(moden, j, f));
            ++zahl;
        }
    return zahl > 0 ? summeDb / zahl : 0.0;
}

/// Die Modendichte im Band: Zahl der Moden in [vonHz, bisHz) durch die Breite, je Hz.
[[nodiscard]] inline double bandDichte(const std::vector<ModeImKoerper>& moden, const Terzband& band)
{
    int zahl = 0;
    for (const ModeImKoerper& m : moden) zahl += (m.frequenzHz >= band.vonHz && m.frequenzHz < band.bisHz) ? 1 : 0;
    return zahl / (band.bisHz - band.vonHz);
}

}  // namespace lumi::koerper
