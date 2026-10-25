/**
 ****************************************************************************************
 * @file   test_Teiltonmessung.cpp
 * @brief  Loop, Stufe 31d-2: die Teiltonmessung an synthetischen Signalen
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S151: UF3 a, IK1)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Jedes Signal ist aus bekannten Teiltoenen gebaut
 * (Frequenz, Amplitude, Abklingen je Stufe), mit Stille vor dem Einsatz und leisem Rauschen;
 * die Messung muss die Groessen wiederfinden:
 *
 * - Einsatz, f0 und B im Bass und im Diskant, jeder Teilton an seiner Stelle.
 * - Zwei Stufen (in dB zwei Geraden mit Knick) und eine Stufe (kein Knick erfunden).
 * - Schwebung zweier verstimmter Saiten; eine Saite allein schwebt nicht.
 * - Helligkeit zu Beginn (Pegel je Teilton) und `highDecay` wie im Modell.
 * - Das Rauschen begrenzt den gemessenen Bereich.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Teiltonmessung.hpp"

#include <cmath>
#include <complex>
#include <cstdint>
#include <numbers>
#include <vector>

using namespace lumi::messung;

namespace
{

constexpr int kRate = 48000;

struct Komponente
{
    double frequenz = 0.0;
    double amplitude = 1.0;
    double t60Frueh = 10.0;
    double t60Spaet = 10.0;
    double knickS = 1e9;  ///< ab hier (nach dem Einsatz) faellt die Komponente mit `t60Spaet`
};

/// Die Summe der Komponenten ab `einsatzS`, davor Stille; dazu Rauschen gleichverteilt in
/// ±`rauschen` (SplitMix64, auf jedem Rechner dasselbe).
std::vector<double> erzeuge(const std::vector<Komponente>& komponenten, double dauerS, double einsatzS, double rauschen)
{
    const auto n = static_cast<std::size_t>(dauerS * kRate);
    const auto einsatz = static_cast<std::size_t>(einsatzS * kRate);
    std::vector<double> x(n, 0.0);
    double phase = 0.0;
    for (const Komponente& k : komponenten)
    {
        phase += 0.7;  // die Teiltoene starten nicht alle mit derselben Phase
        const auto knick = einsatz + static_cast<std::size_t>(std::min(k.knickS, dauerS) * kRate);
        const double w = 2.0 * std::numbers::pi * k.frequenz / kRate;
        const std::complex<double> frueh = std::exp(std::complex<double>(-std::log(1000.0) / (k.t60Frueh * kRate), w));
        const std::complex<double> spaet = std::exp(std::complex<double>(-std::log(1000.0) / (k.t60Spaet * kRate), w));
        std::complex<double> z = std::polar(k.amplitude, phase);
        for (std::size_t i = einsatz; i < n; ++i)
        {
            x[i] += z.imag();
            z *= i < knick ? frueh : spaet;
        }
    }
    std::uint64_t samen = 31;
    for (double& v : x)
    {
        samen += 0x9E3779B97F4A7C15ull;
        std::uint64_t z = samen;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        z ^= z >> 31;
        v += rauschen * (static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0);
    }
    return x;
}

double teiltonFrequenz(double f0, double b, int k)
{
    return k * f0 * std::sqrt(1.0 + b * k * k);
}

/// Teiltoene 1/k laut mit derselben Abklingzeit.
std::vector<Komponente> saite(double f0, double b, int teiltoene, double t60)
{
    std::vector<Komponente> k;
    for (int i = 1; i <= teiltoene && teiltonFrequenz(f0, b, i) < 0.45 * kRate; ++i)
        k.push_back({teiltonFrequenz(f0, b, i), 0.3 / i, t60, t60});
    return k;
}

}  // namespace

TEST_CASE("Loop Teiltonmessung 31d-2 Einsatz, f0 und B")
{
    const struct
    {
        double f0;
        double b;
        int teiltoene;
    } faelle[] = {{55.0, 2e-4, 40}, {261.6, 4e-4, 30}, {1046.5, 1.5e-3, 18}};
    for (const auto& fall : faelle)
    {
        CAPTURE(fall.f0);
        const std::vector<Komponente> teile = saite(fall.f0, fall.b, fall.teiltoene, 10.0);
        const std::vector<double> ton = erzeuge(teile, 4.0, 0.2, 1e-6);
        const Ergebnis e = miss(ton, kRate, fall.f0 * std::sqrt(1.0 + fall.b), lumi::loop::fft());
        REQUIRE(e.erkannt);
        MESSAGE("f0 " << fall.f0 << ": gemessen f0 " << e.f0 << ", B " << e.b << " ± " << e.bFehler << ", "
                      << e.teiltoene.size() << " Teiltoene, Einsatz " << e.einsatzS << " s");
        CHECK(std::abs(e.einsatzS - 0.2) < 0.001);
        CHECK(e.f0 == doctest::Approx(fall.f0).epsilon(1e-4).scale(0.0));
        CHECK(e.b == doctest::Approx(fall.b).epsilon(0.01).scale(0.0));
        CHECK(e.bFehler < 0.01 * fall.b);
        CHECK(e.teiltoene.size() == teile.size());  // jeder unter 0,45·fs
        for (const Teilton& t : e.teiltoene)
        {
            CAPTURE(t.k);
            CHECK(t.frequenz == doctest::Approx(teiltonFrequenz(fall.f0, fall.b, t.k)).epsilon(1e-4).scale(0.0));
        }
    }
}

TEST_CASE("Loop Teiltonmessung 31d-2 zwei Stufen und eine")
{
    SUBCASE("zwei Stufen: Knick bei 1,5 s, frueh 3 s, spaet 12 s")
    {
        std::vector<Komponente> k = saite(110.0, 1e-4, 12, 3.0);
        for (Komponente& c : k)
        {
            c.t60Spaet = 12.0;
            c.knickS = 1.5;
        }
        const Ergebnis e = miss(erzeuge(k, 6.0, 0.1, 1e-6), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 8; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK(t->abklingen.zweiStufen);
            CHECK(t->abklingen.t60Frueh == doctest::Approx(3.0).epsilon(0.03).scale(0.0));
            CHECK(t->abklingen.t60Spaet == doctest::Approx(12.0).epsilon(0.03).scale(0.0));
            CHECK(t->abklingen.knickS == doctest::Approx(1.5).epsilon(0.03).scale(0.0));
        }
    }
    SUBCASE("eine Stufe: 4 s, kein Knick")
    {
        const Ergebnis e = miss(erzeuge(saite(110.0, 1e-4, 12, 4.0), 6.0, 0.1, 1e-6), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 8; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK_FALSE(t->abklingen.zweiStufen);
            CHECK(t->abklingen.t60Frueh == doctest::Approx(4.0).epsilon(0.03).scale(0.0));
            CHECK(t->abklingen.t60Spaet == t->abklingen.t60Frueh);
        }
        for (const Teilton& t : e.teiltoene)
        {
            CAPTURE(t.k);
            CHECK_FALSE(t.abklingen.zweiStufen);
        }
    }
}

TEST_CASE("Loop Teiltonmessung 31d-2 Schwebung")
{
    // zwei Saiten, 4 Cent auseinander, die zweite halb so laut: Schwebung k·Δf
    const double f0 = 220.0;
    const double fa = f0 * std::exp2(-2.0 / 1200.0);
    const double fb = f0 * std::exp2(2.0 / 1200.0);
    std::vector<Komponente> zwei = saite(fa, 0.0, 10, 20.0);
    for (const Komponente& c : saite(fb, 0.0, 10, 20.0)) zwei.push_back({c.frequenz, 0.5 * c.amplitude, 20.0, 20.0});
    const Ergebnis e = miss(erzeuge(zwei, 6.0, 0.1, 1e-6), kRate, f0, lumi::loop::fft());
    REQUIRE(e.erkannt);
    for (int n = 1; n <= 8; ++n)
    {
        CAPTURE(n);
        const Teilton* t = e.teilton(n);
        REQUIRE(t != nullptr);
        REQUIRE(t->schwebung.erkannt);
        MESSAGE("Teilton " << n << ": Schwebung " << t->schwebung.frequenz << " Hz, Tiefe " << t->schwebung.tiefeDb << " dB");
        CHECK(t->schwebung.frequenz == doctest::Approx(n * (fb - fa)).epsilon(0.02).scale(0.0));
    }

    const Ergebnis allein = miss(erzeuge(saite(f0, 0.0, 10, 20.0), 6.0, 0.1, 1e-6), kRate, f0, lumi::loop::fft());
    REQUIRE(allein.erkannt);
    for (int n = 1; n <= 8; ++n)
    {
        CAPTURE(n);
        REQUIRE(allein.teilton(n) != nullptr);
        CHECK_FALSE(allein.teilton(n)->schwebung.erkannt);
    }
}

TEST_CASE("Loop Teiltonmessung 31d-2 Helligkeit und highDecay")
{
    SUBCASE("Pegel je Teilton zu Beginn: 1/k")
    {
        const Ergebnis e = miss(erzeuge(saite(110.0, 1e-4, 20, 10.0), 3.0, 0.1, 1e-6), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        const Teilton* erster = e.teilton(1);
        REQUIRE(erster != nullptr);
        CHECK(std::abs(erster->anfangDb - 20.0 * std::log10(0.3)) < 0.5);
        for (int n = 2; n <= 16; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            CHECK(std::abs(t->anfangDb - erster->anfangDb + 20.0 * std::log10(n)) < 0.5);
        }
    }
    SUBCASE("highDecay 0,2 wie im Modell: σ_k = σ_1·(1 + h·((f_k/f_1)² − 1))")
    {
        const double t60 = 8.0;
        const double high = 0.2;
        const double h = (1.0 / high - 1.0) / 99.0;
        std::vector<Komponente> k = saite(110.0, 0.0, 30, t60);
        for (std::size_t i = 0; i < k.size(); ++i)
        {
            const double r = static_cast<double>(i + 1);
            k[i].t60Frueh = k[i].t60Spaet = t60 / (1.0 + h * (r * r - 1.0));
        }
        const Ergebnis e = miss(erzeuge(k, 8.0, 0.1, 1e-7), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        REQUIRE(e.hochErkannt);
        MESSAGE("highDecay gemessen " << e.highDecay << " (σ = " << e.sigmaB1 << " + " << e.sigmaB3 << "·ω²)");
        // eine Stufe je Teilton: auch wo der Teilton frueh im Boden ankommt (langsame tiefe
        // Teiltoene legen ihr Leck darunter), wird keine zweite erfunden
        for (const Teilton& t : e.teiltoene)
        {
            CAPTURE(t.k);
            CHECK_FALSE(t.abklingen.zweiStufen);
        }
        CHECK(e.highDecay == doctest::Approx(high).epsilon(0.05).scale(0.0));
        for (int n = 1; n <= 10; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK(t->abklingen.t60Frueh == doctest::Approx(k[static_cast<std::size_t>(n - 1)].t60Frueh).epsilon(0.03).scale(0.0));
        }
    }
}

TEST_CASE("Loop Teiltonmessung 31d-2 das Rauschen begrenzt den Bereich")
{
    // Teilton 1 bei −10,5 dB faellt mit 60 dB in 2 s; Rauschen ±1e-4: der Bereich endet, bevor die
    // Huellkurve im Rauschen liegt, und nach mindestens 40 dB
    const Ergebnis e = miss(erzeuge(saite(110.0, 0.0, 6, 2.0), 5.0, 0.1, 1e-4), kRate, 110.0, lumi::loop::fft());
    REQUIRE(e.erkannt);
    const Teilton* t = e.teilton(1);
    REQUIRE(t != nullptr);
    REQUIRE(t->abklingen.erkannt);
    MESSAGE("Rauschen " << t->rauschenDb << " dB, Bereich " << t->abklingen.vonS << " bis " << t->abklingen.bisS << " s");
    CHECK(t->rauschenDb < t->anfangDb - 50.0);
    CHECK(t->abklingen.bisS < 4.9);
    CHECK(t->abklingen.bisS > 40.0 / 30.0);  // 30 dB je Sekunde
    const auto ende = static_cast<std::size_t>(std::lround(t->abklingen.bisS / e.schrittS));
    CHECK(t->pegelDb[ende] > t->rauschenDb + 10.0);
    CHECK(t->abklingen.t60Frueh == doctest::Approx(2.0).epsilon(0.03).scale(0.0));
}
