/**
 ****************************************************************************************
 * @file   test_ComposerLautheit.cpp
 * @brief  Composer, Lautstaerke-Abgleich G3: die Messung nach EBU R128 und die
 *         Belege vor dem Dialog (docs/ui/Composer_Konzept.md §9g)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Audio-Engine.
 *
 * - Die Messung an gerechneten Toenen: die Faelle 1 bis 5 aus EBU Tech 3341
 *   (Fassung 2023, Tabelle 1 „Minimum requirements test signals", gedruckte
 *   Seite 8) — Stereo-Sinus 1000 Hz, je Kanal die genannte Spitze, gleichphasig;
 *   erwartet die genannte integrierte Lautheit ±0,1 LU. Dazu die Spitze, Stille,
 *   stueckweises Fuettern und warum es R128 ist (der Bass zaehlt weniger).
 * - Seit S139 (Stufe 27a, die Messung liegt jetzt unter `audio/`): die Maxima
 *   der Momentary- und Short-term-Lautheit an den Faellen 1, 2, 9, 10, 12, 13
 *   derselben Tabelle.
 * - Beleg zu L3b a: je mitgeliefertem Loop gleicht ein Durchlauf dem naechsten?
 *   Gerechnet werden drei Durchlaeufe am Stueck, verglichen der zweite mit dem
 *   dritten; der erste faengt ohne Nachklang an und wird nur gemeldet.
 * - Rechenzeit: zehn Minuten Ton messen (meldet nur Zahlen; nur unter
 *   Release-clang aussagekraeftig).
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "audio/Lautheit.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QCoreApplication>
#include <QDir>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <numbers>
#include <vector>

using lumi::audio::kLufsStille;
using lumi::audio::Lautheit;
using lumi::audio::LautheitsMessung;
using lumi::audio::messeLautheit;

namespace
{

/// Ein Stueck Stereo-Sinus: `sekunden` lang, je Kanal die Spitze `dbfs`
/// (unter −150 dBFS: Stille), beide Kanaele gleichphasig.
struct Ton
{
    double sekunden;
    double dbfs;
};

std::vector<float> sinus(const std::vector<Ton>& stuecke, int rate, double hz = 1000.0)
{
    std::vector<float> aus;
    std::size_t n = 0;
    for (const Ton& t : stuecke)
    {
        const double amplitude = t.dbfs < -150.0 ? 0.0 : std::pow(10.0, t.dbfs / 20.0);
        const auto frames = static_cast<std::size_t>(std::llround(t.sekunden * rate));
        for (std::size_t i = 0; i < frames; ++i, ++n)
        {
            const auto x = static_cast<float>(amplitude * std::sin(2.0 * std::numbers::pi * hz *
                                                                   static_cast<double>(n) / rate));
            aus.push_back(x);
            aus.push_back(x);
        }
    }
    return aus;
}

struct LoopApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

}  // namespace

TEST_SUITE("ComposerLautheit")
{
    TEST_CASE("Lautheit: die Faelle 1 bis 5 aus EBU Tech 3341, bei 48 und 44,1 kHz")
    {
        for (const int rate : {48000, 44100})
        {
            CAPTURE(rate);
            // 1: −23 dBFS, 20 s → −23,0 LUFS
            CHECK(std::abs(messeLautheit(sinus({{20.0, -23.0}}, rate), rate).lufs + 23.0) <= 0.1);
            // 2: −33 dBFS, 20 s → −33,0 LUFS
            CHECK(std::abs(messeLautheit(sinus({{20.0, -33.0}}, rate), rate).lufs + 33.0) <= 0.1);
            // 3: 10 s −36, 60 s −23, 10 s −36 → −23,0 (das relative Gate nimmt die leisen Teile heraus)
            CHECK(std::abs(messeLautheit(sinus({{10.0, -36.0}, {60.0, -23.0}, {10.0, -36.0}}, rate), rate).lufs +
                           23.0) <= 0.1);
            // 4: dazu je 10 s −72 vorn und hinten → −23,0 (das absolute Gate)
            CHECK(std::abs(messeLautheit(sinus({{10.0, -72.0}, {10.0, -36.0}, {60.0, -23.0}, {10.0, -36.0},
                                                {10.0, -72.0}},
                                               rate),
                                         rate)
                               .lufs +
                           23.0) <= 0.1);
            // 5: 20 s −26, 20,1 s −20, 20 s −26 → −23,0
            CHECK(std::abs(messeLautheit(sinus({{20.0, -26.0}, {20.1, -20.0}, {20.0, -26.0}}, rate), rate).lufs +
                           23.0) <= 0.1);
        }
    }

    // S139, Stufe 27a: Momentary und Short-term (EBU Tech 3341, 2023, Tabelle 1, gedruckte Seite 8).
    // Die Faelle fuer Messungen an Dateien („file-based meters"): 10 und 13 je Abschnitt eine eigene
    // Messung — der Ton liegt um i * 0,15 s bzw. i * 20 ms versetzt und ist genau so lang wie das
    // Fenster; nur ein Fenster, das je Abtastwert gleitet, trifft ihn ganz. 11 und 14 sind fuer
    // laufende Anzeigen („live meters") und hier nicht nachgestellt.
    TEST_CASE("Lautheit: Momentary und Short-term — die Faelle 1, 2, 9, 10, 12, 13 aus EBU Tech 3341")
    {
        for (const int rate : {48000, 44100})
        {
            CAPTURE(rate);
            // 1 und 2: M, S = −23,0 bzw. −33,0 LUFS
            const Lautheit eins = messeLautheit(sinus({{20.0, -23.0}}, rate), rate);
            CHECK(std::abs(eins.momentanMaxLufs + 23.0) <= 0.1);
            CHECK(std::abs(eins.kurzzeitMaxLufs + 23.0) <= 0.1);
            const Lautheit zwei = messeLautheit(sinus({{20.0, -33.0}}, rate), rate);
            CHECK(std::abs(zwei.momentanMaxLufs + 33.0) <= 0.1);
            CHECK(std::abs(zwei.kurzzeitMaxLufs + 33.0) <= 0.1);

            // 9: (1,34 s bei −20, 1,66 s bei −30) fuenfmal — S = −23,0, gleich nach 3 s, also auch das Maximum
            std::vector<Ton> neun;
            for (int i = 0; i < 5; ++i)
            {
                neun.push_back({1.34, -20.0});
                neun.push_back({1.66, -30.0});
            }
            CHECK(std::abs(messeLautheit(sinus(neun, rate), rate).kurzzeitMaxLufs + 23.0) <= 0.1);

            // 12: (0,18 s bei −20, 0,22 s bei −30) 25-mal — M = −23,0 nach 1 s, also auch das Maximum
            std::vector<Ton> zwoelf;
            for (int i = 0; i < 25; ++i)
            {
                zwoelf.push_back({0.18, -20.0});
                zwoelf.push_back({0.22, -30.0});
            }
            CHECK(std::abs(messeLautheit(sinus(zwoelf, rate), rate).momentanMaxLufs + 23.0) <= 0.1);

            // 10: je Abschnitt i * 0,15 s Stille, 3 s bei −23, 1 s Stille → Max S = −23,0
            // 13: je Abschnitt i * 20 ms Stille, 400 ms bei −23, 1 s Stille → Max M = −23,0
            for (int i = 0; i < 20; ++i)
            {
                CAPTURE(i);
                const Lautheit zehn = messeLautheit(sinus({{i * 0.15, -200.0}, {3.0, -23.0}, {1.0, -200.0}}, rate), rate);
                CHECK(std::abs(zehn.kurzzeitMaxLufs + 23.0) <= 0.1);
                const Lautheit dreizehn =
                    messeLautheit(sinus({{i * 0.02, -200.0}, {0.4, -23.0}, {1.0, -200.0}}, rate), rate);
                CHECK(std::abs(dreizehn.momentanMaxLufs + 23.0) <= 0.1);
            }
        }
    }

    TEST_CASE("Lautheit: Momentary und Short-term — kuerzer als das Fenster, Stille, stueckweise")
    {
        constexpr int rate = 48000;
        // kuerzer als 3 s: kein Short-term, aber Momentary
        const Lautheit kurz = messeLautheit(sinus({{1.0, -23.0}}, rate), rate);
        CHECK(kurz.kurzzeitMaxLufs == kLufsStille);
        CHECK(std::abs(kurz.momentanMaxLufs + 23.0) <= 0.1);
        // kuerzer als 0,4 s: keines
        CHECK(messeLautheit(sinus({{0.3, -10.0}}, rate), rate).momentanMaxLufs == kLufsStille);
        // Stille: keines
        const Lautheit still = messeLautheit(std::vector<float>(2 * rate * 4, 0.0f), rate);
        CHECK(still.momentanMaxLufs == kLufsStille);
        CHECK(still.kurzzeitMaxLufs == kLufsStille);
        // ein lauter Schlag in langer Stille: das Maximum haengt nicht an der Laenge der Stille
        const Lautheit schlag = messeLautheit(sinus({{30.0, -200.0}, {0.4, -14.0}, {30.0, -200.0}}, rate), rate);
        CHECK(std::abs(schlag.momentanMaxLufs + 14.0) <= 0.1);
        // stueckweise in ungeraden Happen wie am Stueck
        const std::vector<float> ton = sinus({{3.0, -30.0}, {7.0, -18.0}, {2.0, -40.0}}, rate);
        LautheitsMessung m(rate);
        std::size_t frame = 0;
        for (std::size_t happen = 1; frame < ton.size() / 2; happen = happen * 3 % 4097 + 1)
        {
            const std::size_t n = std::min(happen, ton.size() / 2 - frame);
            m.fuettere(ton.data() + 2 * frame, n);
            frame += n;
        }
        const Lautheit ganz = messeLautheit(ton, rate);
        CHECK(m.ergebnis().momentanMaxLufs == doctest::Approx(ganz.momentanMaxLufs).epsilon(1e-9));
        CHECK(m.ergebnis().kurzzeitMaxLufs == doctest::Approx(ganz.kurzzeitMaxLufs).epsilon(1e-9));
        CHECK(std::abs(ganz.kurzzeitMaxLufs + 18.0) <= 0.1);
    }

    TEST_CASE("Lautheit: Spitze, Stille, kurzer Ton, stueckweise gefuettert")
    {
        constexpr int rate = 48000;
        const Lautheit halb = messeLautheit(sinus({{2.0, 20.0 * std::log10(0.5)}}, rate), rate);
        CHECK(halb.spitzeDb == doctest::Approx(20.0 * std::log10(0.5)).epsilon(0.001));
        CHECK_FALSE(halb.stille);

        const Lautheit still = messeLautheit(std::vector<float>(2 * rate * 3, 0.0f), rate);
        CHECK(still.stille);
        CHECK(still.lufs == kLufsStille);
        CHECK(still.spitzeDb == kLufsStille);
        // alles unter −70 LUFS zaehlt als Stille
        CHECK(messeLautheit(sinus({{5.0, -80.0}}, rate), rate).stille);
        // kuerzer als ein Block von 400 ms: keine Lautheit, aber eine Spitze
        const Lautheit kurz = messeLautheit(sinus({{0.3, -10.0}}, rate), rate);
        CHECK(kurz.stille);
        CHECK(kurz.spitzeDb == doctest::Approx(-10.0).epsilon(0.01));

        // stueckweise in ungeraden Happen wie am Stueck
        const std::vector<float> ton = sinus({{3.0, -30.0}, {7.0, -18.0}, {2.0, -40.0}}, rate);
        LautheitsMessung m(rate);
        std::size_t frame = 0;
        for (std::size_t happen = 1; frame < ton.size() / 2; happen = happen * 3 % 4097 + 1)
        {
            const std::size_t n = std::min(happen, ton.size() / 2 - frame);
            m.fuettere(ton.data() + 2 * frame, n);
            frame += n;
        }
        CHECK(m.ergebnis().lufs == doctest::Approx(messeLautheit(ton, rate).lufs).epsilon(1e-9));
    }

    TEST_CASE("Lautheit: der Bass zaehlt weniger — ein Sinus bei 50 Hz misst leiser als einer bei 1 kHz")
    {
        constexpr int rate = 48000;
        const double tief = messeLautheit(sinus({{5.0, -20.0}}, rate, 50.0), rate).lufs;
        const double mitte = messeLautheit(sinus({{5.0, -20.0}}, rate, 1000.0), rate).lufs;
        MESSAGE("50 Hz: " << tief << " LUFS, 1 kHz: " << mitte << " LUFS");
        CHECK(mitte - tief > 3.0);
    }

    TEST_CASE("Beleg L3b: gleicht ein Durchlauf jedes mitgelieferten Loops dem naechsten?")
    {
        const LoopApp app;
        constexpr int rate = 44100;
        const QDir wurzel(looptest::repoWurzel().filePath(QStringLiteral("asset/loop")));
        int geprueft = 0;
        int bitgleich = 0;
        // nur die mitgelieferten (asset/Asset.cmake), keine Arbeitsdateien (S129, W2 a)
        for (const QString& pfad : looptest::mitgelieferteLoops())
        {
            CAPTURE(wurzel.relativeFilePath(pfad).toStdString());
            lumi::loop::Loop loop;
            if (!lumi::loop::ladeLoop(pfad, loop)) continue;
            const lumi::loop::Ablauf ablauf = lumi::loop::plane(loop, rate);
            const auto klaenge =
                std::make_shared<const std::vector<lumi::loop::Klang>>(looptest::klaengeOhneEngine(ablauf));
            lumi::loop::Klangerzeuger erzeuger;
            erzeuger.setzeProgramm(lumi::loop::baueProgramm(loop, rate, klaenge));
            const auto frames = static_cast<std::size_t>(ablauf.durchlaufFrames);
            if (frames == 0) continue;
            std::vector<float> drei(frames * 2 * 3);
            for (std::size_t stelle = 0; stelle < 3 * frames; stelle += 441)
                erzeuger.render(drei.data() + 2 * stelle, static_cast<int>(std::min<std::size_t>(441, 3 * frames - stelle)));
            const auto durchlauf = [&](std::size_t i) {
                return std::vector<float>(drei.begin() + static_cast<std::ptrdiff_t>(2 * frames * i),
                                          drei.begin() + static_cast<std::ptrdiff_t>(2 * frames * (i + 1)));
            };
            const std::vector<float> erster = durchlauf(0);
            const std::vector<float> zweiter = durchlauf(1);
            const std::vector<float> dritter = durchlauf(2);
            float abweichung = 0.0f;
            for (std::size_t i = 0; i < zweiter.size(); ++i)
                abweichung = std::max(abweichung, std::abs(zweiter[i] - dritter[i]));
            const Lautheit l1 = messeLautheit(erster, rate);
            const Lautheit l2 = messeLautheit(zweiter, rate);
            const Lautheit l3 = messeLautheit(dritter, rate);
            MESSAGE(wurzel.relativeFilePath(pfad).toStdString()
                    << ": 1. " << l1.lufs << ", 2. " << l2.lufs << ", 3. " << l3.lufs
                    << " LUFS; groesste Abweichung 2./3. " << abweichung);
            ++geprueft;
            if (abweichung == 0.0f) ++bitgleich;
            // fuer den Abgleich genuegt: zweiter und dritter messen gleich laut
            if (!l2.stille) CHECK(std::abs(l2.lufs - l3.lufs) <= 0.05);
        }
        MESSAGE(geprueft << " Loops, davon " << bitgleich << " mit bitgleichem 2. und 3. Durchlauf");
        CHECK(geprueft > 10);
    }

    TEST_CASE("Messung: Rechenzeit fuer zehn Minuten Stereo bei 44,1 kHz (meldet nur Zahlen)")
    {
        constexpr int rate = 44100;
        const std::vector<float> ton = sinus({{600.0, -14.0}}, rate);
        const auto vorher = std::chrono::steady_clock::now();
        const Lautheit l = messeLautheit(ton, rate);
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - vorher).count();
        MESSAGE("zehn Minuten gemessen in " << ms << " ms, " << l.lufs << " LUFS");
        CHECK_FALSE(l.stille);
    }
}
