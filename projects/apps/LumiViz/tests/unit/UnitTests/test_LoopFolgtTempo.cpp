/**
 ****************************************************************************************
 * @file   test_LoopFolgtTempo.cpp
 * @brief  Loop, Stufe 15j: *Follow tempo* — ein Wave-Muster auf das Tempo des Loops gedehnt
 *         (docs/audio/Loop_Konzept.md §4p; Entscheid Patrik WV1-1 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Die „Datei“: 2 s, die erste Sekunde ein Ton mit
 * Obertoenen (220 Hz), die zweite still.
 * - Der Faktor: Tempo des Loops durch Bezugstempo, nur mit *Follow tempo*; im Namen `;tempo:`.
 * - Faktor 2 und 0,5: der Ton liegt halb bzw. doppelt so lang im Muster, die Tonhoehe bleibt —
 *   mit *General* und *Voice*.
 * - Die Lage bleibt in Schritten; ein Tempowechsel des Loops gibt einen anderen Klang.
 * - Faktor genau 1 (Bezugstempo = Tempo des Loops): Bit fuer Bit wie ohne *Follow tempo*.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"  // musterIn
#include "loop/Loop.hpp"
#include "loop/PYin.hpp"
#include "loop/WaveKlang.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Zwei Sekunden: die erste ein Ton mit Obertoenen (220 Hz, 1 / k bis 5 kHz), die zweite still.
std::vector<float> datei()
{
    std::vector<float> d(static_cast<std::size_t>(2 * 2 * kRate), 0.0f);
    for (std::int64_t i = 0; i < kRate; ++i)
    {
        double w = 0.0;
        for (int k = 1; k * 220.0 < 5000.0; ++k) w += std::sin(2.0 * std::numbers::pi * k * 220.0 * static_cast<double>(i) / kRate) / k;
        d[static_cast<std::size_t>(2 * i)] = d[static_cast<std::size_t>(2 * i + 1)] = static_cast<float>(0.2 * w);
    }
    return d;
}

/// Ein Loop: `bpm`, 4/4, vier Schritte je Schlag; ein Wave-Muster von einem Takt mit Bezugstempo
/// `bezug`, *Follow tempo* `folgt`, Lage `versatz` Schritte.
Loop waveLoop(double bpm, double bezug, bool folgt, double versatz = 0.0, TonhoehenArt art = TonhoehenArt::Allgemein)
{
    Loop loop;
    loop.bpm = bpm;
    Muster m;
    m.name = QStringLiteral("Voc");
    m.art = MusterArt::Wave;
    m.takte = 1;
    m.wave.datei = QStringLiteral("C:/x/voc.wav");
    m.wave.bezugBpm = bezug;
    m.wave.folgtTempo = folgt;
    m.wave.versatz = versatz;
    m.wave.tonhoehenArt = art;
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("Vocals"), true, {{QStringLiteral("Voc"), 0, 1}}}};
    loop.richte();
    return loop;
}

double effektiv(const std::vector<float>& d, double von, double bis)
{
    const auto a = static_cast<std::size_t>(von * kRate);
    const auto b = std::min(d.size() / 2, static_cast<std::size_t>(bis * kRate));
    double summe = 0.0;
    for (std::size_t i = a; i < b; ++i) summe += static_cast<double>(d[2 * i]) * d[2 * i];
    return b > a ? std::sqrt(summe / static_cast<double>(b - a)) : 0.0;
}

double grundton(const std::vector<float>& d, double von, double bis)
{
    const std::vector<float> stueck(d.begin() + static_cast<std::ptrdiff_t>(2 * std::llround(von * kRate)),
                                    d.begin() + static_cast<std::ptrdiff_t>(2 * std::llround(bis * kRate)));
    std::vector<float> hz;
    for (const float f : messeTonhoehenverlauf(stueck, kRate).hz)
        if (f > 0.0f) hz.push_back(f);
    if (hz.empty()) return 0.0;
    std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
    return hz[hz.size() / 2];
}

Klang gerechnet(const Loop& loop)
{
    WaveKlang w;
    REQUIRE(zerlegeWaveKlang(loop.klangNamen().front(), w));
    return rechneWave(w, datei(), kRate);
}

}  // namespace

TEST_SUITE("LoopFolgtTempo")
{
    TEST_CASE("Loop Follow tempo 15j: der Faktor und der Name")
    {
        WaveAngaben w;
        w.bezugBpm = 60.0;
        CHECK(waveTempoFaktor(w, 120.0) == 1.0);  // aus: im Original
        w.folgtTempo = true;
        CHECK(waveTempoFaktor(w, 120.0) == 2.0);
        CHECK(waveTempoFaktor(w, 45.0) == 0.75);
        CHECK(waveTempoFaktor(w, 0.0) == 1.0);

        const QString aus = waveLoop(120.0, 60.0, false).klangNamen().front();
        const QString an = waveLoop(120.0, 60.0, true).klangNamen().front();
        CHECK_FALSE(aus.contains(QStringLiteral(";tempo:")));
        CHECK(an.contains(QStringLiteral(";tempo:2|")));
        WaveKlang k;
        REQUIRE(zerlegeWaveKlang(an, k));
        CHECK(k.tempo == 2.0);
        CHECK(waveKlangName(k) == an);
        // mit Voice
        const QString stimme = waveLoop(120.0, 60.0, true, 0.0, TonhoehenArt::Stimme).klangNamen().front();
        CHECK(stimme.contains(QStringLiteral(";tempo:2;voice|")));
        REQUIRE(zerlegeWaveKlang(stimme, k));
        CHECK(k.stimme);
        // ein Tempowechsel des Loops gibt einen anderen Klang, das Bezugstempo auch
        CHECK(waveLoop(90.0, 60.0, true).klangNamen().front().contains(QStringLiteral(";tempo:1.5|")));
        CHECK(waveLoop(120.0, 80.0, true).klangNamen().front().contains(QStringLiteral(";tempo:1.5|")));
        // kaputt
        CHECK_FALSE(zerlegeWaveKlang(QStringLiteral("wave:0,0,0,1000,0,0,0;tempo:0|C:/x"), k));
        CHECK_FALSE(zerlegeWaveKlang(QStringLiteral("wave:0,0,0,1000,0,0,0;tempo:1|C:/x"), k));
        CHECK_FALSE(zerlegeWaveKlang(QStringLiteral("wave:0,0,0,1000,0,0,0;tempo:2;100:3|C:/x"), k));
    }

    TEST_CASE("Loop Follow tempo 15j: Faktor genau 1 klingt Bit fuer Bit wie aus")
    {
        const Loop aus = waveLoop(120.0, 120.0, false);
        const Loop an = waveLoop(120.0, 120.0, true);
        CHECK(an.klangNamen() == aus.klangNamen());
        CHECK(gerechnet(an).daten == gerechnet(aus).daten);
    }

    TEST_CASE("Loop Follow tempo 15j: doppelt und halb so schnell, die Tonhoehe bleibt")
    {
        for (const TonhoehenArt art : {TonhoehenArt::Allgemein, TonhoehenArt::Stimme})
        {
            CAPTURE(static_cast<int>(art));
            // 120 BPM, Bezug 60: Faktor 2 — der Ton (1 s der Datei) liegt in 0 … 0,5 s des Musters
            const Klang schnell = gerechnet(waveLoop(120.0, 60.0, true, 0.0, art));
            CHECK(schnell.frames() == kRate);  // die 2 s der Datei in 1 s, das Muster ist 2 s lang
            CHECK(effektiv(schnell.daten, 0.1, 0.4) > 0.1);
            CHECK(effektiv(schnell.daten, 0.6, 0.9) < 0.01);
            CHECK(std::abs(1200.0 * std::log2(grundton(schnell.daten, 0.1, 0.4) / 220.0)) < 5.0);
            // 60 BPM, Bezug 120: Faktor 0,5 — der Ton in 0 … 2 s des Musters (4 s lang)
            const Klang langsam = gerechnet(waveLoop(60.0, 120.0, true, 0.0, art));
            CHECK(langsam.frames() == 4 * kRate);
            CHECK(effektiv(langsam.daten, 0.2, 1.8) > 0.1);
            CHECK(effektiv(langsam.daten, 2.2, 3.8) < 0.01);
            CHECK(std::abs(1200.0 * std::log2(grundton(langsam.daten, 0.5, 1.5) / 220.0)) < 5.0);
        }
    }

    TEST_CASE("Loop Follow tempo 15j: die Lage bleibt in Schritten, der Bedarf an Datei waechst")
    {
        // vier Schritte bei 120 BPM = 0,5 s: der Ton beginnt dort, gedehnt auf 0,5 s
        const Klang spaet = gerechnet(waveLoop(120.0, 60.0, true, 4.0));
        CHECK(effektiv(spaet.daten, 0.05, 0.45) == 0.0);
        CHECK(effektiv(spaet.daten, 0.6, 0.9) > 0.1);
        CHECK(effektiv(spaet.daten, 1.1, 1.4) < 0.01);
        // ein Auftakt: zwei Schritte (0,25 s) frueher — bei Faktor 2 faellt die erste halbe Sekunde der
        // Datei vor den Anfang, vom Ton bleiben 0,5 s Datei = 0,25 s im Muster
        const Klang frueh = gerechnet(waveLoop(120.0, 60.0, true, -2.0));
        CHECK(effektiv(frueh.daten, 0.05, 0.2) > 0.1);
        CHECK(effektiv(frueh.daten, 0.3, 0.45) < 0.01);
        // so viel der Datei braucht der Klang: das Muster (2 s) mal 2
        WaveKlang w;
        REQUIRE(zerlegeWaveKlang(waveLoop(120.0, 60.0, true).klangNamen().front(), w));
        CHECK(waveBedarfSekunden(w) == doctest::Approx(4.01));
    }
}
