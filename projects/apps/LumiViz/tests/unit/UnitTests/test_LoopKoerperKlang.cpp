/**
 ****************************************************************************************
 * @file   test_LoopKoerperKlang.cpp
 * @brief  Loop, Stufe 31c-7: die Antwort des Bodens und der Messteil (docs/audio/
 *         Spielweisen_Konzept.md §4.1, Entscheide Patrik S149/S150: AW1 a, FT1 a, PG1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung; der Steinway wird vom Arbeitsordner aus gelesen (die
 * Test-Exe aus dem Projekt-Root starten).
 *
 * - Ein Einmassenschwinger: die Antwort als Zeitreihe gegen die geschlossene Loesung
 *   a·(g''(t) + δ(t)), g(t) = e^(−σt) sin(ω_d t)/ω_d; der Weg Kraft → Druck.
 * - Die Laenge der Antwort am Steinway (AW1 a: tiefste Mode 38 Hz, η 1,1 % → rund 5 s).
 * - **Messteil** nur mit `LUMIVIZ_31C7_MESSUNG=1` (Release-clang, einige Minuten): Zeit und Speicher je
 *   Antwort, Rechenzeit der Faltung je Ton, Schalldruck am Hoerort fuer alle 88 Tasten bei der
 *   hoechsten Anschlagstufe → der Wert von `kSchalldruckBezug` (PG1 a). Meldet nur Zahlen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Antwort.hpp"
#include "loop/Boden.hpp"
#include "loop/HammerTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Koerper.hpp"

#include <QtGlobal>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <numbers>
#include <vector>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

double sekundenSeit(std::chrono::steady_clock::time_point t)
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t).count();
}

}  // namespace

TEST_CASE("Loop Koerper 31c-7 Antwort: ein Einmassenschwinger")
{
    // eine Mode bei 100 Hz, η = 2 %, φ = 2, G = 0,5: H = φG·(−ω²)/(ω_n² − ω² + jηω_nω)
    kp::ModeImKoerper m;
    m.frequenzHz = 100.0;
    m.verlustfaktor = 0.02;
    m.amStegpunkt = {2.0};
    m.abstrahlung = {std::complex<double>(0.5, 0.0)};
    const std::vector<kp::ModeImKoerper> moden{m};
    const int rate = 48000;
    const double t = 1.0 / rate;
    const std::vector<double> h = bodenAntwort(moden, 0, 0, rate, 1.0);
    REQUIRE(h.size() == static_cast<std::size_t>(rate));
    // geschlossen: g'' = e^(−σt)·((σ² − ω_d²) sin ω_d t − 2σω_d cos ω_d t)/ω_d, je Abtastwert mal T
    const double wn = 2.0 * std::numbers::pi * 100.0, sigma = 0.5 * 0.02 * wn, wd = wn * std::sqrt(1.0 - 0.0001);
    // g'' springt bei t = 0 um 2σ; das bandbegrenzte Signal gibt den Sprung mit einem Nachschwingen wieder,
    // das wie 1/k abklingt (bei k = 1 rund 0,6 % des groessten Werts) — verglichen wird ab 1 ms, dort
    // rund 10⁻⁴; die ersten Werte werden berichtet
    double groesste = 0.0, fehler = 0.0, fehlerAnfang = 0.0;
    std::size_t wo = 0;
    for (std::size_t k = 1; k < 4800; ++k)
    {
        const double tk = static_cast<double>(k) * t;
        const double g2 = std::exp(-sigma * tk) * ((sigma * sigma - wd * wd) * std::sin(wd * tk) - 2.0 * sigma * wd * std::cos(wd * tk)) / wd;
        const double soll = 1.0 * g2 * t;
        groesste = std::max(groesste, std::abs(soll));
        if (k < 48)
            fehlerAnfang = std::max(fehlerAnfang, std::abs(h[k] - soll));
        else if (std::abs(h[k] - soll) > fehler)
        {
            fehler = std::abs(h[k] - soll);
            wo = k;
        }
    }
    MESSAGE("Einmassenschwinger: groesster Fehler ab 1 ms ", fehler / groesste, " vom groessten Wert (bei k = ", wo,
            "), davor ", fehlerAnfang / groesste);
    CHECK(fehler < 1e-3 * groesste);
    // der Stoss traegt a·δ: der erste Wert ist rund φG = 1
    CHECK(h[0] == doctest::Approx(1.0).epsilon(0.01));
    // Kraft → Druck: ein Impuls von einer Einheit (kHammerKraftBezug N) gibt die Antwort mal 200
    std::vector<float> stoss(10, 0.0f);
    stoss[0] = 1.0f;
    const std::vector<double> p = schalldruck(stoss, h);
    REQUIRE(p.size() == stoss.size() + h.size() - 1);
    for (std::size_t k = 0; k < 1000; ++k) CHECK(p[k] == doctest::Approx(kHammerKraftBezug * h[k]).epsilon(1e-9).scale(1e-9));
}

TEST_CASE("Loop Koerper 31c-7 Antwort: Laenge am Steinway")
{
    kp::Koerper koerper;
    REQUIRE_MESSAGE(ladeKoerper(QStringLiteral("asset/loop/instrument_bodies/steinway_d.lvinstbody"), koerper),
                    "die Test-Exe aus dem Projekt-Root starten");
    const double laenge = kp::antwortLaengeS(alleModen(koerper));
    MESSAGE("Laenge der Antwort am Steinway: ", laenge, " s");
    CHECK(laenge > 4.0);
    CHECK(laenge <= 8.0);
    CHECK(stegpunktVon(koerper, 21) == 0);
    CHECK(stegpunktVon(koerper, 108) == 87);
    CHECK(stegpunktVon(koerper, 20) == -1);
}

TEST_CASE("Loop Koerper 31c-7 Messteil (AW1 a, FT1 a, PG1 a)")
{
    if (qEnvironmentVariableIntValue("LUMIVIZ_31C7_MESSUNG") != 1)
    {
        MESSAGE("LUMIVIZ_31C7_MESSUNG nicht gesetzt — der Messteil laeuft nicht");
        return;
    }
    kp::Koerper koerper;
    REQUIRE(ladeKoerper(QStringLiteral("asset/loop/instrument_bodies/steinway_d.lvinstbody"), koerper));
    const int rate = 48000;
    auto t0 = std::chrono::steady_clock::now();
    const std::vector<kp::ModeImKoerper> moden = alleModen(koerper);
    MESSAGE("Moden: ", koerper.moden.size(), " gerechnet, ", moden.size() - koerper.moden.size(), " gezogen; Ziehen ",
            sekundenSeit(t0), " s");
    const double laengeS = kp::antwortLaengeS(moden);
    const auto frames = static_cast<std::size_t>(std::ceil(laengeS * rate));
    MESSAGE("Antwort: ", laengeS, " s, ", frames, " Abtastwerte, FFT ", zweierpotenzAb(frames), "; als float ",
            frames * 4.0 / 1048576.0, " MB je Antwort, 88 x 2 Hoerorte ", 176.0 * frames * 4.0 / 1048576.0, " MB");

    // (1) Zeit je Antwort, ein Thread
    for (const int taste : {21, 40, 41, 60, 88, 108})
    {
        t0 = std::chrono::steady_clock::now();
        const std::vector<double> h = bodenAntwort(moden, static_cast<std::size_t>(stegpunktVon(koerper, taste)), 0, rate, laengeS);
        MESSAGE("Antwort Taste ", taste, ": ", sekundenSeit(t0), " s");
    }

    // (2) Ton, Faltung und Schalldruck je Taste bei der hoechsten Anschlagstufe (0), eine Sekunde gehalten
    const HammerSaite& saite = hammerSaiteVorgabe();
    const Tastatur tastatur{};
    struct Ergebnis
    {
        int taste = 0;
        double spitzePa[2] = {0.0, 0.0};
        double tonS = 0.0, antwortS = 0.0, faltungS = 0.0;
        std::size_t tonFrames = 0;
    };
    const auto rechne = [&](int taste) {
        Ergebnis e;
        e.taste = taste;
        HammerAnschlag a;
        a.taste = taste;
        a.hz = tonHz(taste);
        a.stufe = 0;
        a.halten = rate;
        a.gedaempft = taste < tastatur.ungedaempftAb;
        a.rate = rate;
        auto t = std::chrono::steady_clock::now();
        const std::int64_t frames = a.halten + hammerNachklang(saite, a, std::int64_t{30} * rate);
        std::vector<float> kraft;
        rechneHammerTon(saite, a, kraft, frames);
        e.tonS = sekundenSeit(t);
        e.tonFrames = kraft.size();
        const auto j = static_cast<std::size_t>(stegpunktVon(koerper, taste));
        for (std::size_t o = 0; o < 2; ++o)
        {
            t = std::chrono::steady_clock::now();
            const std::vector<double> h = bodenAntwort(moden, j, o, rate, laengeS);
            e.antwortS += sekundenSeit(t);
            t = std::chrono::steady_clock::now();
            const std::vector<double> p = schalldruck(kraft, h);
            e.faltungS += sekundenSeit(t);
            for (const double v : p) e.spitzePa[o] = std::max(e.spitzePa[o], std::abs(v));
        }
        return e;
    };
    t0 = std::chrono::steady_clock::now();
    std::vector<Ergebnis> alle;
    for (int von = 21; von <= 108; von += 8)  // je acht zugleich — eine Taste braucht rund 70 MB
    {
        std::vector<std::future<Ergebnis>> laufend;
        for (int taste = von; taste < von + 8 && taste <= 108; ++taste) laufend.push_back(std::async(std::launch::async, rechne, taste));
        for (auto& f : laufend) alle.push_back(f.get());
    }
    MESSAGE("88 Tasten, je acht zugleich: ", sekundenSeit(t0), " s");
    const Ergebnis* lauteste = &alle.front();
    double faltungMax = 0.0, antwortMax = 0.0;
    std::size_t tonMax = 0;
    for (const Ergebnis& e : alle)
    {
        if (std::max(e.spitzePa[0], e.spitzePa[1]) > std::max(lauteste->spitzePa[0], lauteste->spitzePa[1])) lauteste = &e;
        faltungMax = std::max(faltungMax, e.faltungS / 2.0);
        antwortMax = std::max(antwortMax, e.antwortS / 2.0);
        tonMax = std::max(tonMax, e.tonFrames);
        if (e.taste % 12 == 0 || e.taste == 21 || e.taste == 108)
            MESSAGE("Taste ", e.taste, ": Spitze ", 20.0 * std::log10(e.spitzePa[0] / 20e-6), " / ",
                    20.0 * std::log10(e.spitzePa[1] / 20e-6), " dB SPL (left / right), Ton ", e.tonFrames / double(rate),
                    " s, Antwort je Hoerort ", e.antwortS / 2.0, " s, Faltung je Hoerort ", e.faltungS / 2.0, " s");
    }
    const double spitze = std::max(lauteste->spitzePa[0], lauteste->spitzePa[1]);
    MESSAGE("lautester Ton: Taste ", lauteste->taste, ", Spitze ", spitze, " Pa = ", 20.0 * std::log10(spitze / 20e-6),
            " dB SPL; kSchalldruckBezug fuer −6 dBFS Spitze: ", spitze / std::pow(10.0, -6.0 / 20.0), " Pa");
    MESSAGE("hoechstens (parallel gemessen): Antwort ", antwortMax, " s, Faltung ", faltungMax, " s; laengster Ton ",
            tonMax / double(rate), " s");
}
