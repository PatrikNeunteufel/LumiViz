/**
 ****************************************************************************************
 * @file   test_LoopExpression.cpp
 * @brief  Loop, Stufe 28c-1a: Expression als Pegel im Rechenkern
 *         (docs/audio/Spielweisen_Konzept.md §3, Entscheide Patrik S140: SF5 d, SK4 a,
 *         SK8 a, SK9 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Engine, ohne Dateien aus dem Repo: die Klaenge sind
 * gerechnete Sinustoene.
 *
 * - Der Verlauf: die Spur des Musters je Durchlauf (vor dem ersten Punkt der erste,
 *   an der Naht ein Sprung), die Kurve am Clip in Takten, malgenommen; hinter dem Ende
 *   des Clips der Wert dort.
 * - Der Pegel ist e² (SK9 a): 0,5 sind −12 dB — an einem Drumkit mit der Kurve am Clip
 *   gemessen, Wert fuer Wert.
 * - Ein Instrument ohne Saiten: die Spur formt den Ton ueber der Zeit.
 * - Ohne Spur und Kurve bleibt alles, wie es war; ein Instrument mit Saiten bekommt
 *   Expression erst mit Stufe 28c-2.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <cmath>
#include <numbers>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Ein Takt Drums (16 Schritte, 120 BPM: ein Schritt 125 ms) mit Schlaegen auf 0 und 8,
/// als Clip mit `mal` Wiederholungen.
Loop drumLoop(int mal = 1)
{
    Loop loop;
    loop.laut = 1.0;
    Muster m;
    m.name = QStringLiteral("D");
    m.takte = 1;
    m.spuren = {Spur(QStringLiteral("Kick"), QStringLiteral("kit:kick"))};
    m.zeilen = {std::vector<double>(16, 0.0)};
    m.zeilen[0][0] = 1.0;
    m.zeilen[0][8] = 1.0;
    loop.muster = {m};
    loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("D"), 0, mal)}}};
    loop.richte();
    return loop;
}

/// Ein Takt eines Instruments ohne Saiten mit einer Note ueber den ganzen Takt.
Loop instrumentLoop()
{
    Loop loop;
    loop.laut = 1.0;
    Muster m;
    m.name = QStringLiteral("P");
    m.art = MusterArt::Instrument;
    m.takte = 1;
    m.instrument = QStringLiteral("probe");
    m.noten = {Note{0, 16, 69}};
    loop.muster = {m};
    loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("P"), 0, 1)}}};
    loop.richte();
    return loop;
}

/// Je Klang ein Sinus bei 220 Hz, Spitze 0,5, `ms` lang.
Klaenge sinus(const Ablauf& ablauf, int ms)
{
    Klang k;
    const int frames = kRate * ms / 1000;
    for (int i = 0; i < frames; ++i)
    {
        const auto x = static_cast<float>(0.5 * std::sin(2.0 * std::numbers::pi * 220.0 * i / kRate));
        k.daten.push_back(x);
        k.daten.push_back(x);
    }
    return Klaenge(static_cast<std::size_t>(ablauf.klangNamen.size()), std::make_shared<const Klang>(std::move(k)));
}

std::vector<float> rechneLoop(const Loop& loop, int ms)
{
    const Ablauf ablauf = plane(loop, kRate);
    std::vector<float> aus(static_cast<std::size_t>(ablauf.durchlaufFrames) * 2);
    rechne(ablauf, sinus(ablauf, ms), 0, static_cast<int>(ablauf.durchlaufFrames), aus.data());
    return aus;
}

double effektiv(const std::vector<float>& aus, double vonMs, double bisMs)
{
    const auto von = static_cast<std::size_t>(vonMs * kRate / 1000.0);
    const auto bis = std::min(aus.size() / 2, static_cast<std::size_t>(bisMs * kRate / 1000.0));
    double summe = 0.0;
    for (std::size_t f = von; f < bis; ++f) summe += static_cast<double>(aus[2 * f]) * aus[2 * f];
    return bis > von ? std::sqrt(summe / static_cast<double>(bis - von)) : 0.0;
}

}  // namespace

TEST_SUITE("LoopExpression")
{
    TEST_CASE("Loop Expression 28c-1a: der Verlauf — Spur je Durchlauf, Kurve am Clip, malgenommen, hinter dem Ende gehalten")
    {
        Loop loop = instrumentLoop();
        loop.muster.front().zeitspuren.von(Steuerspur::Expression) = {{4.0, 0.5}, {12.0, 1.0}};
        loop.kanaele.front().clips.front().wiederholungen = 2;
        loop.kanaele.front().clips.front().expression = {{0.0, 1.0}, {2.0, 0.0}};  // in Takten
        loop.richte();
        const Ablauf ablauf = plane(loop, kRate);
        REQUIRE(ablauf.ausdruecke.size() == 1);
        const Ausdruck& a = ablauf.ausdruecke.front();
        const double schritt = ablauf.schrittFrames;
        const auto bei = [&](double s) { return a.wert(std::llround(s * schritt)); };
        // Spur: vor dem ersten Punkt der erste (0,5), dann linear, am Ende des Durchlaufs der letzte
        // Kurve: 1 bei Takt 0, 0 bei Takt 2 — bei Schritt s also 1 − s/32
        CHECK(bei(0.0) == doctest::Approx(0.5 * 1.0));
        CHECK(bei(8.0) == doctest::Approx(0.75 * (1.0 - 8.0 / 32.0)).epsilon(1e-3));
        CHECK(bei(14.0) == doctest::Approx(1.0 * (1.0 - 14.0 / 32.0)).epsilon(1e-3));
        // an der Naht springt die Spur auf ihren ersten Wert
        CHECK(bei(16.0) == doctest::Approx(0.5 * 0.5).epsilon(1e-3));
        // hinter dem Ende des Clips (Schritt 32) gilt der Wert dort
        CHECK(a.wert(a.ende + 10000) == doctest::Approx(a.wert(a.ende)));
        CHECK(a.pegel(std::llround(16.0 * schritt)) == doctest::Approx(0.25 * 0.25).epsilon(1e-3));  // e²
        // jeder Schlag des Clips traegt den Verlauf
        for (const Stimme& stimme : ablauf.stimmen)
            for (const Schlag& s : stimme.schlaege) CHECK(s.ausdruck == 0);
    }

    TEST_CASE("Loop Expression 28c-1a: der Pegel ist e² — an einem Drumkit mit der Kurve am Clip, Wert fuer Wert")
    {
        const Loop ohne = drumLoop();
        Loop halb = ohne;
        halb.kanaele.front().clips.front().expression = {{0.0, 0.5}};
        halb.richte();
        const std::vector<float> a = rechneLoop(ohne, 300);
        const std::vector<float> b = rechneLoop(halb, 300);
        REQUIRE(a.size() == b.size());
        double groesste = 0.0;
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            CHECK(b[i] == doctest::Approx(0.25 * a[i]).epsilon(1e-6));  // 0,5² — −12 dB
            groesste = std::max(groesste, static_cast<double>(std::abs(a[i])));
        }
        CHECK(groesste > 0.4);
        MESSAGE("e = 0,5: " << 20.0 * std::log10(0.25) << " dB (40 lg e)");
    }

    TEST_CASE("Loop Expression 28c-1a: ein Instrument ohne Saiten — die Spur formt den Ton ueber der Zeit")
    {
        Loop loop = instrumentLoop();
        // ein Crescendo ueber den Takt: 0,2 bei 0, 0,95 bei 15 (ein Punkt am Ende des Musters,
        // Schritt 16, faellt beim Richten weg wie eine Note dort)
        loop.muster.front().zeitspuren.von(Steuerspur::Expression) = {{0.0, 0.2}, {15.0, 0.95}};
        loop.richte();
        REQUIRE(loop.muster.front().zeitspuren.von(Steuerspur::Expression).size() == 2);
        const std::vector<float> aus = rechneLoop(loop, 2000);
        const std::vector<float> flach = rechneLoop(instrumentLoop(), 2000);
        // um 250 ms (Schritt 2) ist e = 0,2 + 0,75 · 2/15 = 0,3, um 1750 ms (Schritt 14) 0,9
        const double frueh = effektiv(aus, 200.0, 300.0) / effektiv(flach, 200.0, 300.0);
        const double spaet = effektiv(aus, 1700.0, 1800.0) / effektiv(flach, 1700.0, 1800.0);
        MESSAGE("Pegel frueh " << frueh << ", spaet " << spaet);
        CHECK(frueh == doctest::Approx(0.3 * 0.3).epsilon(0.05));
        CHECK(spaet == doctest::Approx(0.9 * 0.9).epsilon(0.05));
    }

    TEST_CASE("Loop Expression 28c-1a: ohne Spur und Kurve wie bisher; mit Saiten noch keine Expression")
    {
        const Ablauf ohne = plane(drumLoop(), kRate);
        CHECK(ohne.ausdruecke.empty());
        for (const Stimme& stimme : ohne.stimmen)
            for (const Schlag& s : stimme.schlaege) CHECK(s.ausdruck == -1);

        // ein Instrument mit Saiten: die Spur wirkt erst mit 28c-2 (ueber die Tonabnehmer)
        Loop saiten = instrumentLoop();
        saiten.muster.front().zeitspuren.von(Steuerspur::Expression) = {{0.0, 0.5}};
        saiten.richte();
        const InstrumentVon gitarre = [](const QString&) {
            Saitensatz satz;
            satz.leer = stimmungsvorlagen()[0].leer;
            return satz;
        };
        CHECK(plane(saiten, kRate, gitarre).ausdruecke.empty());
        CHECK(plane(saiten, kRate).ausdruecke.size() == 1);  // ohne Auskunft gilt es als Synth
    }
}
