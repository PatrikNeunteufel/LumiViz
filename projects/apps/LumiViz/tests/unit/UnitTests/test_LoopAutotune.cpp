/**
 ****************************************************************************************
 * @file   test_LoopAutotune.cpp
 * @brief  Loop, Stufe 15i: die Spur Autotune rechnen (docs/audio/Loop_Konzept.md §4p;
 *         Entscheide Patrik WV4 a, WV9 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung.
 * - Die Korrektur je Rahmen an einem gebauten Verlauf: Strength, Speed (95 % nach Speed), ein
 *   stummer Rahmen haelt, ausserhalb klingt sie aus.
 * - *Detect notes*: Tonart, Hysterese gegen Vibrato, kurze Luecke gehoert zur Note, kurze Noten
 *   fallen weg, eine Stimme erkannt.
 * - Ersetzen im Bereich, Teilen, Verbinden.
 * - Der Name des Klangs: Abschnitte darin, genau zurueckgelesen; ohne Abschnitte wie vorher.
 * - Gerechnet: ein gesungener Ton 30 Cent zu hoch landet auf dem Zielton — mit *General* und
 *   *Voice*, mit der Spur Pitch dazu; und ueber den Tonauftrag wie im Programm.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Autotune.hpp"
#include "loop/Bearbeiten.hpp"  // musterIn
#include "loop/Loop.hpp"
#include "loop/PYin.hpp"
#include "loop/Tonauftrag.hpp"
#include "loop/Tonspeicher.hpp"
#include "loop/WaveKlang.hpp"
#include "TonhoehenOrdner.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Ein gebauter Verlauf: je Rahmen (256 bei 44,1 kHz) die Frequenz aus `hz(t)`, 0 = stumm.
template <typename F>
Tonhoehenverlauf verlaufAus(double sekunden, F hz)
{
    Tonhoehenverlauf v;
    v.rate = kRate;
    v.schritt = 256;
    const auto rahmen = static_cast<std::size_t>(sekunden * kRate / 256.0);
    for (std::size_t k = 0; k < rahmen; ++k)
    {
        const double f = hz(v.zeitS(k));
        v.hz.push_back(static_cast<float>(f));
        v.stimmhaft.push_back(f > 0.0 ? 0.9f : 0.0f);
    }
    return v;
}

double hzVon(double midi)
{
    return 440.0 * std::pow(2.0, (midi - 69.0) / 12.0);
}

/// Ein gesungener Ton mit Obertoenen (1 / k bis 5 kHz), `hz(t)`, Stereo.
template <typename F>
std::vector<float> ton(double sekunden, F hz)
{
    const auto n = static_cast<std::int64_t>(sekunden * kRate);
    std::vector<float> d(static_cast<std::size_t>(2 * n));
    double phase = 0.0;
    for (std::int64_t i = 0; i < n; ++i)
    {
        const double f = hz(static_cast<double>(i) / kRate);
        phase += 2.0 * std::numbers::pi * f / kRate;
        double w = 0.0;
        for (int k = 1; k * f < 5000.0; ++k) w += std::sin(k * phase) / k;
        d[static_cast<std::size_t>(2 * i)] = d[static_cast<std::size_t>(2 * i + 1)] = static_cast<float>(0.2 * w);
    }
    return d;
}

/// Der Grundton in der Mitte eines Klangs (pYIN, Median der gesungenen Rahmen).
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

double cent(double ist, double soll)
{
    return 1200.0 * std::log2(ist / soll);
}

/// Ein Loop mit einem Wave-Muster ueber die ganze Datei (2 s = ein Takt bei 120 BPM, 4/4).
Loop waveLoop(const QString& datei, const std::vector<AutotuneAbschnitt>& abschnitte)
{
    Loop loop;
    loop.bpm = 120.0;
    Muster m;
    m.name = QStringLiteral("Voc");
    m.art = MusterArt::Wave;
    m.takte = 1;
    m.wave.datei = datei;
    m.wave.autotune.abschnitte = abschnitte;
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("Vocals"), true, {{QStringLiteral("Voc"), 0, 1}}}};
    loop.richte();
    return loop;
}

}  // namespace

TEST_SUITE("LoopAutotune")
{
    TEST_CASE("Loop Autotune 15i Korrektur: Strength, Speed, stumme Rahmen, Ausklingen")
    {
        // A4 30 Cent zu hoch, 2 s; von 0,5 s bis 0,6 s stumm
        const Tonhoehenverlauf v = verlaufAus(2.0, [](double t) { return t >= 0.5 && t < 0.6 ? 0.0 : hzVon(69.3); });
        const auto rahmenBei = [&v](double t) { return static_cast<std::size_t>(t * v.rate / v.schritt); };
        SUBCASE("Speed 0, Strength 1: genau auf den Zielton, ausserhalb 0")
        {
            const std::vector<double> k = autotuneKorrektur({{0.2, 1.5, 69, 0, 1.0, 0.0}}, v);
            REQUIRE(k.size() == v.hz.size());
            CHECK(k[rahmenBei(0.1)] == 0.0);
            CHECK(k[rahmenBei(0.3)] == doctest::Approx(-0.3));
            CHECK(k[rahmenBei(0.55)] == doctest::Approx(-0.3));  // stumm: gehalten
            CHECK(k[rahmenBei(1.6)] == 0.0);
            CHECK(autotuneBei(k, v, 0.3) == doctest::Approx(-0.3));
            CHECK(autotuneBei(k, v, 5.0) == 0.0);
        }
        SUBCASE("Strength 0,5 und ein Zielton mit Cent")
        {
            const std::vector<double> k = autotuneKorrektur({{0.2, 1.5, 69, 10, 0.5, 0.0}}, v);
            CHECK(k[rahmenBei(0.3)] == doctest::Approx(0.5 * (69.1 - 69.3)));
        }
        SUBCASE("Speed 120 ms: nach 120 ms 95 % des Wegs, danach ausgeklungen")
        {
            const std::vector<double> k = autotuneKorrektur({{0.2, 1.0, 69, 0, 1.0, 120.0}}, v);
            const std::size_t anfang = rahmenBei(0.2) + 1;  // der erste Rahmen im Abschnitt
            const std::size_t nach = anfang + static_cast<std::size_t>(std::lround(0.12 * v.rate / v.schritt));
            CHECK(std::abs(k[nach]) == doctest::Approx(0.3 * 0.95).epsilon(0.03));
            CHECK(std::abs(k[anfang]) < 0.3 * 0.2);
            CHECK(std::abs(k[rahmenBei(1.0) + 1]) > 0.2);  // klingt aus (τ 40 ms), springt nicht
            CHECK(std::abs(k[rahmenBei(1.5)]) < 0.001);
        }
    }

    TEST_CASE("Loop Autotune 15i Detect notes: Tonart, Hysterese, Luecken, kurze Noten, eine Stimme")
    {
        Autotune vorgaben;
        vorgaben.staerke = 0.8;
        vorgaben.speedMs = 30.0;
        // 0–0,5 C4 +20 Cent; 30 ms stumm; 0,53–1,0 C4 −10; 0,2 s stumm; 1,2–1,8 E4 mit Vibrato ±60 Cent
        // (ueber die Mitte zu Es und F — die Hysterese haelt E);
        // dazwischen 1,5–1,55 ein F4 (zu kurz); 1,9–2,4 C#4 +10
        const Tonhoehenverlauf v = verlaufAus(2.5, [](double t) {
            if (t < 0.5) return hzVon(60.2);
            if (t < 0.53) return 0.0;
            if (t < 1.0) return hzVon(59.9);
            if (t < 1.2) return 0.0;
            if (t >= 1.5 && t < 1.55) return hzVon(65.0);
            if (t < 1.8) return hzVon(64.0 + 0.6 * std::sin(2.0 * std::numbers::pi * 5.5 * t));
            if (t < 1.9) return 0.0;
            return hzVon(61.1);
        });
        SUBCASE("chromatisch")
        {
            const NotenErkennung n = erkenneNoten(v, vorgaben, 0.0, 2.5);
            REQUIRE(n.abschnitte.size() == 4);
            CHECK(n.abschnitte[0].zielHoehe == 60);  // die kurze Luecke gehoert zur Note
            CHECK(n.abschnitte[0].anfang == doctest::Approx(0.0).epsilon(0.01));
            CHECK(n.abschnitte[0].ende == doctest::Approx(1.0).epsilon(0.01));
            CHECK(n.abschnitte[0].staerke == 0.8);
            CHECK(n.abschnitte[0].speedMs == 30.0);
            CHECK(n.abschnitte[1].zielHoehe == 64);  // Vibrato bleibt E4
            CHECK(n.abschnitte[1].anfang == doctest::Approx(1.2).epsilon(0.01));
            CHECK(n.abschnitte[2].zielHoehe == 64);  // das kurze F4 fiel weg, E4 laeuft weiter
            CHECK(n.abschnitte[3].zielHoehe == 61);
            CHECK(n.eineStimme);
        }
        SUBCASE("C-Dur: C#4 +10 geht nach D4")
        {
            vorgaben.skala = Skala::Dur;
            const NotenErkennung n = erkenneNoten(v, vorgaben, 1.85, 2.5);
            REQUIRE(n.abschnitte.size() == 1);
            CHECK(n.abschnitte[0].zielHoehe == 62);
        }
        SUBCASE("nur im Bereich")
        {
            const NotenErkennung n = erkenneNoten(v, vorgaben, 0.2, 0.4);
            REQUIRE(n.abschnitte.size() == 1);
            CHECK(n.abschnitte[0].anfang == doctest::Approx(0.2).epsilon(0.02));
            CHECK(n.abschnitte[0].ende == doctest::Approx(0.4).epsilon(0.02));
        }
        SUBCASE("Stille ist keine Stimme")
        {
            const NotenErkennung n = erkenneNoten(verlaufAus(1.0, [](double) { return 0.0; }), vorgaben, 0.0, 1.0);
            CHECK(n.abschnitte.empty());
            CHECK_FALSE(n.eineStimme);
        }
        CHECK(naechsterTon(61.1, 0, Skala::Moll) == 62);  // C-Moll: C, D, Es … — D liegt 0,9 entfernt, C 1,1
        CHECK(inTonart(70, 0, Skala::Moll));               // B
        CHECK_FALSE(inTonart(71, 0, Skala::Moll));         // H
        CHECK(inTonart(66, 7, Skala::Dur));                // G-Dur: Fis
    }

    TEST_CASE("Loop Autotune 15i Bearbeiten: ersetzen im Bereich, teilen, verbinden")
    {
        Autotune at;
        at.abschnitte = {{0.0, 1.0, 60, 0, 1.0, 0.0}, {1.0, 2.0, 62, 0, 0.5, 10.0}, {2.0, 3.0, 64, 0, 1.0, 0.0}};
        Autotune a = at;
        ersetzeAbschnitte(a, {{1.2, 1.8, 67, 0, 1.0, 0.0}}, 0.5, 2.5);
        REQUIRE(a.abschnitte.size() == 3);
        CHECK(a.abschnitte[0] == AutotuneAbschnitt{0.0, 0.5, 60, 0, 1.0, 0.0});
        CHECK(a.abschnitte[1] == AutotuneAbschnitt{1.2, 1.8, 67, 0, 1.0, 0.0});
        CHECK(a.abschnitte[2] == AutotuneAbschnitt{2.5, 3.0, 64, 0, 1.0, 0.0});

        Autotune t = at;
        CHECK(teileAbschnitt(t, 1.5));
        REQUIRE(t.abschnitte.size() == 4);
        CHECK(t.abschnitte[1].ende == 1.5);
        CHECK(t.abschnitte[2] == AutotuneAbschnitt{1.5, 2.0, 62, 0, 0.5, 10.0});
        CHECK_FALSE(teileAbschnitt(t, 1.51));  // zu nah am Rand
        CHECK_FALSE(teileAbschnitt(t, 5.0));   // dort keiner
        CHECK(verbindeAbschnitte(t, 1));
        CHECK(t.abschnitte == at.abschnitte);
        CHECK_FALSE(verbindeAbschnitte(t, 2));
    }

    TEST_CASE("Loop Autotune 15i Name: die Abschnitte genau zurueck, ohne sie wie vorher")
    {
        const Loop ohne = waveLoop(QStringLiteral("C:/x/voc.wav"), {});
        const Loop mit = waveLoop(QStringLiteral("C:/x/voc.wav"), {{0.25, 1.125, 57, -12, 0.75, 33.5}, {1.5, 1.75, 60, 0, 0.0, 0.0}});
        const QString nameOhne = ohne.klangNamen().front();
        const QString nameMit = mit.klangNamen().front();
        CHECK_FALSE(nameOhne.contains(QStringLiteral(";at:")));
        CHECK(nameMit.contains(QStringLiteral(";at:0.25:1.125:57:-12:0.75:33.5")));
        CHECK_FALSE(nameMit.contains(QStringLiteral(":60:0:0:0")));  // Strength 0 tut nichts, steht nicht da
        WaveKlang w;
        REQUIRE(zerlegeWaveKlang(nameMit, w));
        REQUIRE(w.autotune.size() == 1);
        CHECK(w.autotune[0] == AutotuneAbschnitt{0.25, 1.125, 57, -12, 0.75, 33.5});
        CHECK(waveKlangName(w) == nameMit);
        // mit Voice
        Loop stimme = mit;
        musterIn(stimme, QStringLiteral("Voc"))->wave.tonhoehenArt = TonhoehenArt::Stimme;
        CHECK(stimme.klangNamen().front().endsWith(QStringLiteral(";voice|C:/x/voc.wav")));
        WaveKlang s;
        REQUIRE(zerlegeWaveKlang(stimme.klangNamen().front(), s));
        CHECK(s.stimme);
        CHECK_FALSE(zerlegeWaveKlang(QStringLiteral("wave:0,0,0,1000,0,0,0;at:1:2:3|C:/x"), w));
    }

    TEST_CASE("Loop Autotune 15i gerechnet: der gesungene Ton landet auf dem Zielton")
    {
        // A3 30 Cent zu hoch, 2 s
        const std::vector<float> datei = ton(2.0, [](double) { return hzVon(57.3); });
        const Tonhoehenverlauf verlauf = messeTonhoehenverlauf(datei, kRate);
        for (const bool stimme : {false, true})
        {
            CAPTURE(stimme);
            WaveKlang w;
            w.laengeMs = 2000.0;
            w.autotune = {{0.0, 2.0, 57, 0, 1.0, 0.0}};
            w.stimme = stimme;
            const Klang klang = rechneWave(w, datei, kRate, &verlauf);
            REQUIRE(klang.frames() == 2 * kRate);
            CHECK(std::abs(cent(grundton(klang.daten, 0.5, 1.5), hzVon(57.0))) < 3.0);
            // ohne Verlauf wirkt Autotune nicht
            CHECK(rechneWave(w, datei, kRate).daten == datei);
            // mit der Spur Pitch dazu: +2 Halbtoene auf den Zielton
            w.tonhoehe = {{0.0, 2.0}};
            const Klang hoeher = rechneWave(w, datei, kRate, &verlauf);
            CHECK(std::abs(cent(grundton(hoeher.daten, 0.5, 1.5), hzVon(59.0))) < 3.0);
            // Strength 0,5: halb hin
            w.tonhoehe.clear();
            w.autotune[0].staerke = 0.5;
            const Klang halb = rechneWave(w, datei, kRate, &verlauf);
            CHECK(cent(grundton(halb.daten, 0.5, 1.5), hzVon(57.0)) == doctest::Approx(15.0).epsilon(0.2));
        }
    }

    TEST_CASE("Loop Autotune 15i Tonauftrag: misst die Datei, merkt den Verlauf, rechnet wie rechneWave")
    {
        TonhoehenOrdner tonhoehe;
        tonspeicher().leere();
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString pfad = QDir(ordner.path()).filePath(QStringLiteral("voc.wav"));
        {
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));  // gelesen wird ueber den Dekodierer
            f.write("autotune");
        }
        const std::vector<float> datei = ton(2.0, [](double) { return hzVon(57.3); });
        int mal = 0;
        const Dekodierer dekodierer = [&](const QString&, int rate, double) {
            ++mal;
            REQUIRE(rate == kRate);
            return datei;
        };
        const Loop loop = waveLoop(pfad, {{0.0, 2.0, 57, 0, 1.0, 0.0}});
        Tonauftrag auftrag(loop.klangNamen(), kRate, dekodierer);
        auftrag.rechneHier();
        CHECK(auftrag.bericht().isEmpty());
        REQUIRE(auftrag.klaenge().size() == 1);
        REQUIRE(auftrag.klaenge()[0] != nullptr);
        WaveKlang w;
        REQUIRE(zerlegeWaveKlang(loop.klangNamen().front(), w));
        const Tonhoehenverlauf verlauf = messeTonhoehenverlauf(datei, kRate);
        CHECK(auftrag.klaenge()[0]->daten == rechneWave(w, datei, kRate, &verlauf).daten);
        CHECK(mal == 2);  // einmal der Ausschnitt, einmal die ganze Datei fuer pYIN
        // der Verlauf liegt im Ordner `pitch`
        CHECK(QDir(tonhoehe.ordner.path()).entryList({QStringLiteral("*.lvpitch")}, QDir::Files).size() == 1);
        tonspeicher().leere();
    }
}
