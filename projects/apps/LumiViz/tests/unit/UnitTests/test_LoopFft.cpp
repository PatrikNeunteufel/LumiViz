/**
 ****************************************************************************************
 * @file   test_LoopFft.cpp
 * @brief  Loop, Stufe 31c-7: die reelle FFT hinter `lumi::loop::Fft` und die Faltung
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S149/S150: FT1 a, FF1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Jeder Fall laeuft gegen jede Umsetzung aus
 * `fftUmsetzungen()` (Memory „Externals austauschbar einbinden“).
 *
 * - Hin und zurueck gibt das Signal wieder, fuer Zweierpotenzen und andere Laengen.
 * - Das Spektrum gegen die direkte DFT (Summe von Hand).
 * - Die Faltung gegen die direkte Faltung an kurzen Signalen; ein Bezugswert von Hand; der
 *   Impuls laesst ein Signal, wie es ist.
 * - `zweierpotenzAb`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Fft.hpp"

#include <cmath>
#include <complex>
#include <cstdint>
#include <numbers>
#include <vector>

using namespace lumi::loop;

namespace
{

/// Zahlen in [−1, 1), auf jedem Rechner dieselben (SplitMix64).
std::vector<double> zufall(std::size_t n, std::uint64_t samen)
{
    std::vector<double> x(n);
    for (double& v : x)
    {
        samen += 0x9E3779B97F4A7C15ull;
        std::uint64_t z = samen;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        z ^= z >> 31;
        v = static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0;
    }
    return x;
}

std::vector<double> direkteFaltung(const std::vector<double>& a, const std::vector<double>& b)
{
    std::vector<double> y(a.size() + b.size() - 1, 0.0);
    for (std::size_t i = 0; i < a.size(); ++i)
        for (std::size_t j = 0; j < b.size(); ++j) y[i + j] += a[i] * b[j];
    return y;
}

}  // namespace

TEST_CASE("Loop Fft 31c-7 hin und zurueck")
{
    for (const auto& u : fftUmsetzungen())
    {
        CAPTURE(u->name());
        for (const std::size_t n : {1u, 2u, 7u, 8u, 1000u, 1024u, 65536u})
        {
            CAPTURE(n);
            const std::vector<double> x = zufall(n, n);
            std::vector<std::complex<double>> spektrum(n / 2 + 1);
            u->vorwaerts(x, spektrum);
            std::vector<double> zurueck(n);
            u->rueckwaerts(spektrum, zurueck);
            double groesste = 0.0;
            for (std::size_t t = 0; t < n; ++t) groesste = std::max(groesste, std::abs(zurueck[t] - x[t]));
            CHECK(groesste < 1e-12);
        }
    }
}

TEST_CASE("Loop Fft 31c-7 gegen die direkte DFT")
{
    for (const auto& u : fftUmsetzungen())
    {
        CAPTURE(u->name());
        for (const std::size_t n : {16u, 15u, 64u})
        {
            CAPTURE(n);
            const std::vector<double> x = zufall(n, 100 + n);
            std::vector<std::complex<double>> spektrum(n / 2 + 1);
            u->vorwaerts(x, spektrum);
            for (std::size_t k = 0; k <= n / 2; ++k)
            {
                std::complex<double> soll{0.0, 0.0};
                for (std::size_t t = 0; t < n; ++t)
                    soll += x[t] * std::polar(1.0, -2.0 * std::numbers::pi * static_cast<double>(k * t) / static_cast<double>(n));
                CHECK_MESSAGE(std::abs(spektrum[k] - soll) < 1e-12 * static_cast<double>(n), "k = ", k);
            }
        }
    }
}

TEST_CASE("Loop Fft 31c-7 Faltung")
{
    for (const auto& u : fftUmsetzungen())
    {
        CAPTURE(u->name());
        // ein Bezugswert von Hand: (1, 2, 3) * (0, 1, 0,5) = (0, 1, 2,5, 4, 1,5)
        const std::vector<double> hand = falte(*u, std::vector<double>{1.0, 2.0, 3.0}, std::vector<double>{0.0, 1.0, 0.5});
        const std::vector<double> sollHand{0.0, 1.0, 2.5, 4.0, 1.5};
        REQUIRE(hand.size() == sollHand.size());
        for (std::size_t i = 0; i < hand.size(); ++i) CHECK(hand[i] == doctest::Approx(sollHand[i]).epsilon(1e-12));
        // gegen die direkte Faltung an kurzen Signalen verschiedener Laenge
        for (std::size_t la = 1; la <= 33; la += 4)
            for (std::size_t lb = 1; lb <= 70; lb += 23)
            {
                CAPTURE(la);
                CAPTURE(lb);
                const std::vector<double> a = zufall(la, la), b = zufall(lb, 1000 + lb);
                const std::vector<double> ist = falte(*u, a, b), soll = direkteFaltung(a, b);
                REQUIRE(ist.size() == soll.size());
                double groesste = 0.0;
                for (std::size_t i = 0; i < ist.size(); ++i) groesste = std::max(groesste, std::abs(ist[i] - soll[i]));
                CHECK(groesste < 1e-12);
            }
        // der Impuls laesst ein Signal, wie es ist; leer bleibt leer
        const std::vector<double> x = zufall(500, 7);
        const std::vector<double> gleich = falte(*u, x, std::vector<double>{1.0});
        REQUIRE(gleich.size() == x.size());
        for (std::size_t i = 0; i < x.size(); ++i) CHECK(gleich[i] == doctest::Approx(x[i]).epsilon(1e-12));
        CHECK(falte(*u, x, std::vector<double>{}).empty());
    }
    CHECK(fft().name() == fftUmsetzungen().front()->name());
}

TEST_CASE("Loop Fft 31c-7 zweierpotenzAb")
{
    CHECK(zweierpotenzAb(0) == 1);
    CHECK(zweierpotenzAb(1) == 1);
    CHECK(zweierpotenzAb(2) == 2);
    CHECK(zweierpotenzAb(3) == 4);
    CHECK(zweierpotenzAb(1024) == 1024);
    CHECK(zweierpotenzAb(1025) == 2048);
    CHECK(zweierpotenzAb(48000 * 9) == 524288);
}
