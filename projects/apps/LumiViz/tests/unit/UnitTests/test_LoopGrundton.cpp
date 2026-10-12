/**
 * @file test_LoopGrundton.cpp
 * @brief Loop, Stufe 11a, Schritt 3: den Grundton einer Aufnahme messen —
 *        fuer Detect im Instrument-Creator (docs/audio/Loop_Konzept.md §4h)
 *
 * Ohne Ton, ohne Fenster, ohne Qt-Anwendung: die Aufnahmen sind gerechnet.
 */

#include <doctest.h>

#include "loop/Grundton.hpp"
#include "loop/Instrument.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr double kPi = 3.14159265358979323846;

/// Ein Ton aus Teiltoenen: `anteile[k]` ist der Anteil des (k+1)-ten; Stereo, beide Seiten gleich.
std::vector<float> ton(double hz, double sekunden, int rate, const std::vector<double>& anteile = {1.0})
{
    const auto frames = static_cast<std::size_t>(sekunden * rate);
    std::vector<float> daten(2 * frames);
    double summe = 0.0;
    for (const double anteil : anteile) summe += std::abs(anteil);
    for (std::size_t t = 0; t < frames; ++t)
    {
        double wert = 0.0;
        for (std::size_t k = 0; k < anteile.size(); ++k)
            wert += anteile[k] * std::sin(2.0 * kPi * hz * static_cast<double>(k + 1) * static_cast<double>(t) / rate);
        daten[2 * t] = daten[2 * t + 1] = static_cast<float>(0.8 * wert / summe);
    }
    return daten;
}

/// Rauschen aus einem festen Zaehlwerk: jeder Lauf ergibt dieselben Werte.
std::vector<float> rauschen(double sekunden, int rate, double pegel = 0.8)
{
    const auto frames = static_cast<std::size_t>(sekunden * rate);
    std::vector<float> daten(2 * frames);
    std::uint32_t x = 4711u;
    for (std::size_t t = 0; t < frames; ++t)
    {
        x = x * 1664525u + 1013904223u;
        daten[2 * t] = daten[2 * t + 1] = static_cast<float>(pegel * (static_cast<double>(x >> 8) / 8388608.0 - 1.0));
    }
    return daten;
}

}  // namespace

TEST_SUITE("Loop Grundton")
{
    TEST_CASE("Loop Grundton: ein klarer Ton — Tonhoehe und Abweichung in Cent")
    {
        // A3 genau
        const Grundton a3 = messeGrundton(ton(220.0, 1.0, 48000), 48000);
        REQUIRE(a3.gefunden);
        CHECK(a3.hoehe == 57);
        CHECK(std::abs(a3.cent) < 3.0);
        CHECK(a3.hz == doctest::Approx(220.0).epsilon(0.002));

        // 446 Hz: A4 und rund 23 Cent darueber
        const Grundton hoch = messeGrundton(ton(446.0, 1.0, 48000), 48000);
        REQUIRE(hoch.gefunden);
        CHECK(hoch.hoehe == 69);
        CHECK(hoch.cent == doctest::Approx(23.4).epsilon(0.15));
        // 430 Hz: A4 und rund 40 Cent darunter
        const Grundton tief = messeGrundton(ton(430.0, 1.0, 48000), 48000);
        REQUIRE(tief.gefunden);
        CHECK(tief.hoehe == 69);
        CHECK(tief.cent == doctest::Approx(-39.8).epsilon(0.1));

        // die Raender des Bereichs, und eine andere Abtastrate
        const Grundton bass = messeGrundton(ton(55.0, 1.0, 48000), 48000);
        REQUIRE(bass.gefunden);
        CHECK(bass.hoehe == 33);  // A1
        CHECK(std::abs(bass.cent) < 5.0);
        const Grundton spitz = messeGrundton(ton(1760.0, 1.0, 48000), 48000);
        REQUIRE(spitz.gefunden);
        CHECK(spitz.hoehe == 93);  // A6
        CHECK(std::abs(spitz.cent) < 8.0);
        const Grundton andereRate = messeGrundton(ton(261.63, 1.0, 44100), 44100);
        REQUIRE(andereRate.gefunden);
        CHECK(andereRate.hoehe == 60);  // C4
        CHECK(std::abs(andereRate.cent) < 3.0);

        // derselbe Aufruf ergibt dieselben Werte
        CHECK(messeGrundton(ton(446.0, 1.0, 48000), 48000) == hoch);
    }

    TEST_CASE("Loop Grundton: Obertoene taeuschen nicht — auch wenn der zweite Teilton lauter ist als der Grundton")
    {
        // A2 mit doppelt so lautem zweitem Teilton: gesucht ist die Periode, nicht der lauteste Teilton
        const Grundton a2 = messeGrundton(ton(110.0, 1.0, 48000, {0.4, 1.0, 0.3}), 48000);
        REQUIRE(a2.gefunden);
        CHECK(a2.hoehe == 45);
        CHECK(std::abs(a2.cent) < 5.0);

        // eine gezupfte Saite aus dem Klangerzeuger (A3): sie klingt aus und beginnt mit Rauschen
        Instrument gezupft;
        gezupft.schichten[0].schwingung = Schwingung::Saite;
        const Klang saite = rendereTon(gezupft, 57, 900.0, 44100);
        const Grundton ausSaite = messeGrundton(saite.daten, 44100);
        REQUIRE(ausSaite.gefunden);
        CHECK(ausSaite.hoehe == 57);
        CHECK(std::abs(ausSaite.cent) < 20.0);

        // ein Saegezahn aus dem Klangerzeuger (E4)
        Instrument saege;
        saege.schichten[0].schwingung = Schwingung::Saege;
        saege.schichten[0].halten = 1.0;
        const Grundton ausSaege = messeGrundton(rendereTon(saege, 64, 900.0, 44100).daten, 44100);
        REQUIRE(ausSaege.gefunden);
        CHECK(ausSaege.hoehe == 64);
        CHECK(std::abs(ausSaege.cent) < 5.0);
    }

    TEST_CASE("Loop Grundton: ab einer Stelle messen — was davor liegt, zaehlt nicht")
    {
        // eine halbe Sekunde Rauschen, dann der Ton
        std::vector<float> daten = rauschen(0.5, 48000);
        const std::vector<float> danach = ton(330.0, 1.0, 48000);
        daten.insert(daten.end(), danach.begin(), danach.end());
        const Grundton abTon = messeGrundton(daten, 48000, 24000);
        REQUIRE(abTon.gefunden);
        CHECK(abTon.hoehe == 64);  // E4
        CHECK(std::abs(abTon.cent) < 5.0);
        // hinter dem Ende, oder davor: kein Absturz
        CHECK_FALSE(messeGrundton(daten, 48000, 10000000).gefunden);
        CHECK(messeGrundton(danach, 48000, -5).gefunden);
    }

    TEST_CASE("Loop Grundton: kein klarer Ton — Rauschen, Stille, zu kurz")
    {
        CHECK_FALSE(messeGrundton(rauschen(1.0, 48000), 48000).gefunden);
        CHECK_FALSE(messeGrundton(std::vector<float>(96000, 0.0f), 48000).gefunden);
        // ein Ton unter der Hoerschwelle der Messung zaehlt als Stille
        std::vector<float> fluestern = ton(220.0, 1.0, 48000);
        for (float& wert : fluestern) wert *= 1e-4f;
        CHECK_FALSE(messeGrundton(fluestern, 48000).gefunden);
        CHECK_FALSE(messeGrundton(ton(220.0, 0.01, 48000), 48000).gefunden);  // 480 Abtastwerte
        CHECK_FALSE(messeGrundton({}, 48000).gefunden);
        CHECK_FALSE(messeGrundton(ton(220.0, 1.0, 48000), 0).gefunden);
        // ein Ton, im Rauschen kaum zu hoeren
        std::vector<float> gemischt = rauschen(1.0, 48000);
        const std::vector<float> leise = ton(220.0, 1.0, 48000);
        for (std::size_t i = 0; i < gemischt.size(); ++i) gemischt[i] += 0.05f * leise[i];
        CHECK_FALSE(messeGrundton(gemischt, 48000).gefunden);
        // kurz, aber lang genug: rund 60 ms eines hohen Tons
        const Grundton kurz = messeGrundton(ton(880.0, 0.06, 48000), 48000);
        REQUIRE(kurz.gefunden);
        CHECK(kurz.hoehe == 81);  // A5
    }

    TEST_CASE("Loop Grundton: der Text — „A3 +12 ct“")
    {
        Grundton g;
        CHECK(grundtonText(g).isEmpty());
        g.gefunden = true;
        g.hoehe = 57;
        g.cent = 12.4;
        CHECK(grundtonText(g) == QStringLiteral("A3 +12 ct"));
        g.cent = -7.6;
        CHECK(grundtonText(g) == QStringLiteral("A3 -8 ct"));
        g.cent = 0.3;
        CHECK(grundtonText(g) == QStringLiteral("A3"));
        g.hoehe = 61;
        CHECK(grundtonText(g) == QStringLiteral("C#4"));
    }
}
