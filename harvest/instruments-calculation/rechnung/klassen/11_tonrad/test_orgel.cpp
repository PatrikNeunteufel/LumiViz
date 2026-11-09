/**
 * @file test_orgel.cpp
 * @brief Klasse 11 Tonrad: Radfrequenzen und Abweichung von der gleichstufigen Stimmung, Rueckfaltung je Zugriegel,
 *        gemeinsame Raeder addieren sich phasengleich, 3 dB je Stufe, Perkussion mit Einzeltrigger, Scanner V3 ≈ 1,5 %,
 *        Rotary laeuft hoch, Tastenklick, Determinismus, Abdruck, Rechenzeit
 */
#include "doctest.h"

#include "klassen/11_tonrad/Orgel.hpp"
#include "messung/Bausteine.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <complex>
#include <cstdio>
#include <numbers>

using namespace ic;
using namespace ic::tonrad;

namespace
{

constexpr double kPi = std::numbers::pi;

double cent(int rad)
{
    return 1200.0 * std::log2(radHz(rad) / (440.0 * std::exp2((rad + 23 - 69) / 12.0)));
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < t.size(); ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * double(t[ab + i]) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

OrgelSpiel trocken()
{
    OrgelSpiel s = baueVorgabeOrgel();
    s.rotary = false;
    s.uebersprechen = 0.0;
    s.saettigung = 0.0;  // linear (die Mischung pruefen)
    return s;
}

}  // namespace

TEST_CASE("Tonrad: die Raeder nach den Uebersetzungen (Bericht §1): A = 20 × 16 × 88/64 = 440, Abweichungen bis −0,71 / +1,98 Cent")
{
    CHECK(radHz(46) == doctest::Approx(440.0).epsilon(1e-12));
    CHECK(radHz(1) == doctest::Approx(32.692).epsilon(1e-4));
    CHECK(cent(1) == doctest::Approx(-0.58).epsilon(0.02));    // C
    CHECK(cent(9) == doctest::Approx(-0.71).epsilon(0.02));    // G♯
    CHECK(cent(86) == doctest::Approx(1.98).epsilon(0.02));    // C♯8
    CHECK(cent(91) == doctest::Approx(1.36).epsilon(0.02));    // F♯8
    for (int w = 1; w <= 91; ++w)
    {
        CAPTURE(w);
        CHECK(std::abs(cent(w)) < 2.0);
    }
}

TEST_CASE("Tonrad: Rueckfaltung je Zugriegel fuer 61 Tasten (Bericht §3)")
{
    const int erwartet[9][2] = {{13, 61}, {20, 80}, {13, 73}, {25, 85}, {32, 80}, {37, 85}, {41, 89}, {44, 80}, {49, 85}};
    for (int r = 0; r < 9; ++r)
    {
        CAPTURE(r);
        CHECK(radFuer(1, r) == erwartet[r][0]);
        CHECK(radFuer(61, r) == erwartet[r][1]);
    }
    // 16′: die tiefste Oktave faltet hoch (Taste 1 … 12 → Rad 13 … 24)
    CHECK(radFuer(12, 0) == 24);
    CHECK(radFuer(13, 0) == 13);
}

TEST_CASE("Tonrad: zwei Tasten auf demselben Rad addieren sich phasengleich (+6 dB), die Stufe gibt 3 dB")
{
    OrgelSpiel s = trocken();
    s.register_ = {0, 0, 8, 0, 0, 0, 0, 0, 0};
    OrgelSpur spur;
    spur.schweller = 1.0;
    std::vector<float> eins, zwei;
    // C4 (8′) allein; dann C4 (8′) und C3 mit 4′ — beide auf Rad 37 (C4)
    rechneOrgel(s, {{0.0, 1.0, 60, 0.7}}, spur, eins, 48000);
    OrgelSpiel s4 = s;
    s4.register_ = {0, 0, 8, 8, 0, 0, 0, 0, 0};
    rechneOrgel(s4, {{0.0, 1.0, 60, 0.7}, {0.0, 1.0, 48, 0.7}}, spur, zwei, 48000);
    const double f = radHz(37);
    const double a1 = band(eins, 9600, 32768, f), a2 = band(zwei, 9600, 32768, f);
    std::printf("Rad 37 allein %.1f dB, mit C3 4′ dazu %.1f dB (phasengleich: +6 dB)\n", dB(a1), dB(a2));
    CHECK(dB(a2 / a1) == doctest::Approx(6.02).epsilon(0.05));
    OrgelSpiel s7 = s;
    s7.register_ = {0, 0, 7, 0, 0, 0, 0, 0, 0};
    std::vector<float> sieben;
    rechneOrgel(s7, {{0.0, 1.0, 60, 0.7}}, spur, sieben, 48000);
    CHECK(dB(band(sieben, 9600, 32768, f) / a1) == doctest::Approx(-3.0).epsilon(0.03));
}

TEST_CASE("Tonrad: Perkussion mit Einzeltrigger — eine zweite Taste bei gehaltener erster startet sie nicht neu")
{
    OrgelSpiel s = baueVorgabeHausorgel();
    s.rotary = false;
    OrgelSpur spur;
    OrgelMessung m;
    std::vector<float> t;
    rechneOrgel(s, {{0.0, 1.0, 60, 0.7}, {0.3, 1.0, 64, 0.7}, {1.2, 1.5, 67, 0.7}}, spur, t, 76800, 48000, &m);
    CHECK(m.perkussionStarts == 2.0);
    // die 3. Harmonische (2⅔′) klingt mit der Perkussion ab: frueh staerker als spaet
    const double f3 = radHz(radFuer(25, 4));
    CHECK(band(t, 480, 4096, f3) > 3.0 * band(t, 14400, 4096, f3));
}

TEST_CASE("Tonrad: Scanner V3 hebt und senkt um ≈ 1,5 % (Handbuch, nur Bericht) mit 6,87 Hz")
{
    OrgelSpiel s = trocken();
    s.register_ = {0, 0, 8, 0, 0, 0, 0, 0, 0};
    s.scanner = Scanner::V3;
    OrgelSpur spur;
    spur.schweller = 1.0;
    std::vector<float> t;
    rechneOrgel(s, {{0.0, 2.0, 69, 0.7}}, spur, t, 96000);
    std::vector<double> d(t.begin() + 24000, t.begin() + 90000);
    const messung::Tonhoehe th = messung::tonhoehe(d, 48000, 300.0, 600.0, 0.02, 0.005);
    double lo = 1e9, hi = 0.0;
    for (const double f : th.hz)
        if (f > 0.0)
        {
            lo = std::min(lo, f);
            hi = std::max(hi, f);
        }
    std::printf("Scanner V3: %.1f … %.1f Hz (%.2f %% Spitze–Spitze)\n", lo, hi, 100.0 * (hi - lo) / radHz(46));
    CHECK((hi - lo) / radHz(46) > 0.01);
    CHECK((hi - lo) / radHz(46) < 0.025);
}

TEST_CASE("Tonrad: Rotary laeuft mit getrennten Zeitkonstanten hoch, Tastenklick im Einsatz")
{
    OrgelSpiel s = baueVorgabeOrgel();
    OrgelSpur spur;
    spur.rotarySchnell = {{0.2, true}};
    OrgelMessung m;
    std::vector<float> t;
    rechneOrgel(s, {{0.0, 2.0, 60, 0.7}}, spur, t, 96000, 48000, &m);
    REQUIRE(m.hornUpm.size() > 150);
    std::printf("Horn: %.0f U/min am Anfang, %.0f nach 1 s schnell (Ziel 400, τ 0,8 s)\n", m.hornUpm[10], m.hornUpm[120]);
    CHECK(m.hornUpm[10] == doctest::Approx(48.0).epsilon(0.02));
    CHECK(m.hornUpm[120] > 250.0);
    CHECK(m.hornUpm[120] < 400.0);
    // Klick: im Einsatz (0–15 ms) mehr Energie ueber 3 kHz als im gehaltenen Ton
    OrgelSpiel tr = trocken();
    std::vector<float> k;
    rechneOrgel(tr, {{0.01, 1.0, 60, 0.7}}, OrgelSpur{}, k, 48000);
    double einsatz = 0.0, halt = 0.0;
    for (double f = 3000.0; f < 8000.0; f += 250.0)
    {
        einsatz += band(k, 480, 720, f);
        halt += band(k, 24000, 720, f);
    }
    CHECK(einsatz > 3.0 * halt);
}

TEST_CASE("Tonrad: Determinismus, Abdruck, Rechenzeit (ein Akkord mit Rotary)")
{
    const OrgelSpiel s = baueVorgabeOrgel();
    const std::vector<OrgelNote> akkord = {{0.0, 1.0, 60, 0.7}, {0.0, 1.0, 64, 0.7}, {0.0, 1.0, 67, 0.7}};
    std::vector<float> a;
    rechneOrgel(s, akkord, OrgelSpur{}, a, 28800);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneOrgel(s, akkord, OrgelSpur{}, q[i], 28800); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
    const std::string ist = abdruck(a);
    CHECK(test::erwarteterAbdruck("tonrad_Orgel_C-Dur", ist) == ist);
    std::vector<float> t;
    const double anteil = echtzeitAnteil([&] { rechneOrgel(s, akkord, OrgelSpur{}, t, 96000); }, 2.0);
    std::printf("Rechenzeit Akkord mit Rotary: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
    CHECK(anteil < test::echtzeitGrenzeHier());
}
