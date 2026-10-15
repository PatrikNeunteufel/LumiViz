/**
 ****************************************************************************************
 * @file   test_ComposerClipLautheit.cpp
 * @brief  Composer, Lautstaerke-Abgleich G5: einen Ton-Clip messen und gemessene
 *         Songs merken (docs/ui/Composer_Konzept.md §9g, L3b a, G5-3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * - Gemerkte Songs in einer Datei in einem Wegwerf-Ordner: merken, lesen,
 *   eine geaenderte Datei gilt nicht mehr, vergessen.
 * - Ein Loop: zwei Durchlaeufe, gemessen der zweite — gegen dieselbe Rechnung
 *   von Hand; Abbruch ueber den Fortschritt. Ohne Engine (Klaenge aus dem Repo).
 * - Ein Song ueber die echte BassEngine: ein gerechneter WAV mit Stille und
 *   einem Sinus von −23 dBFS; der Ausschnitt entscheidet, was gemessen wird.
 *   Ohne Ausgabegeraet meldet der Fall das und prueft nichts. Spielt keinen Ton.
 * - Messung: mit `LUMIVIZ_BELEG_SONG=<Datei>` dekodiert und misst der letzte
 *   Fall einen echten Song und meldet die Zeit (prueft nichts).
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "audio/BassEngine.hpp"
#include "composer/ClipLautheit.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QCoreApplication>
#include <QDataStream>
#include <QElapsedTimer>
#include <QFile>
#include <QTemporaryDir>

#include <cmath>
#include <memory>
#include <numbers>
#include <vector>

using namespace lumi::composer;

namespace
{

constexpr int kRate = 44100;

/// WAV, 16 Bit, Stereo, 44,1 kHz: erst `stilleS` Sekunden Stille, dann
/// `tonS` Sekunden Sinus 1 kHz mit der Spitze `dbfs` in beiden Kanaelen.
bool schreibeWav(const QString& datei, double stilleS, double tonS, double dbfs)
{
    QFile f(datei);
    if (!f.open(QIODevice::WriteOnly)) return false;
    const auto stille = static_cast<quint32>(std::llround(stilleS * kRate));
    const auto ton = static_cast<quint32>(std::llround(tonS * kRate));
    const quint32 datenBytes = (stille + ton) * 4;
    QDataStream s(&f);
    s.setByteOrder(QDataStream::LittleEndian);
    f.write("RIFF", 4);
    s << quint32(36 + datenBytes);
    f.write("WAVEfmt ", 8);
    s << quint32(16) << quint16(1) << quint16(2) << quint32(kRate) << quint32(kRate * 4) << quint16(4) << quint16(16);
    f.write("data", 4);
    s << datenBytes;
    const double amp = std::pow(10.0, dbfs / 20.0);
    for (quint32 n = 0; n < stille + ton; ++n)
    {
        const qint16 w = n < stille ? qint16(0)
                                    : static_cast<qint16>(std::lround(
                                          amp * std::sin(2.0 * std::numbers::pi * 1000.0 * (n - stille) / kRate) * 32767.0));
        s << w << w;
    }
    return f.error() == QFile::NoError;
}

struct App
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    std::unique_ptr<QCoreApplication> app;
    App()
    {
        if (QCoreApplication::instance() == nullptr) app = std::make_unique<QCoreApplication>(argc, argv);
    }
};

}  // namespace

TEST_SUITE("ComposerClipLautheit")
{
    TEST_CASE("G5 gemerkte Songs: Datei, Groesse und Aenderungszeit, vergessen")
    {
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString song = ordner.filePath(QStringLiteral("song.wav"));
        const QString anderer = ordner.filePath(QStringLiteral("anderer.wav"));
        REQUIRE(schreibeWav(song, 0.0, 1.0, -20.0));
        REQUIRE(schreibeWav(anderer, 0.0, 1.0, -20.0));
        const QString cache = ordner.filePath(QStringLiteral("cache/composer_loudness.json"));

        Lautheit wert;
        wert.lufs = -14.25;
        wert.spitzeDb = -0.5;
        wert.stille = false;
        {
            SongLautheiten s(cache);
            CHECK_FALSE(s.hole(song, 0, 1000, kRate).has_value());
            s.merke(song, 0, 1000, kRate, wert);
            s.merke(anderer, 0, 1000, kRate, wert);
            s.merke(ordner.filePath(QStringLiteral("fehlt.wav")), 0, 1000, kRate, wert);  // gibt es nicht: nichts
            CHECK(s.anzahl() == 2);
            REQUIRE(s.schreibe());  // legt auch den Ordner an
        }
        {
            SongLautheiten s(cache);  // nach einem Neustart
            const std::optional<Lautheit> da = s.hole(song, 0, 1000, kRate);
            REQUIRE(da.has_value());
            CHECK(da->lufs == doctest::Approx(-14.25));
            CHECK(da->spitzeDb == doctest::Approx(-0.5));
            CHECK_FALSE(da->stille);
            // ein anderer Ausschnitt, eine andere Rate: nicht gemessen
            CHECK_FALSE(s.hole(song, 500, 1000, kRate).has_value());
            CHECK_FALSE(s.hole(song, 0, 2000, kRate).has_value());
            CHECK_FALSE(s.hole(song, 0, 1000, 48000).has_value());
            // Measure again: vergessen, nur dieser Song
            s.vergiss(song);
            CHECK_FALSE(s.hole(song, 0, 1000, kRate).has_value());
            CHECK(s.hole(anderer, 0, 1000, kRate).has_value());
            REQUIRE(s.schreibe());
        }
        {
            SongLautheiten s(cache);
            CHECK(s.anzahl() == 1);
            // die Datei ersetzt (andere Groesse): gilt nicht mehr
            REQUIRE(schreibeWav(anderer, 0.0, 2.0, -20.0));
            CHECK_FALSE(s.hole(anderer, 0, 1000, kRate).has_value());
        }
        // eine kaputte oder fremde Datei: leer, kein Absturz
        {
            QFile f(cache);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write("{ kein json");
        }
        CHECK(SongLautheiten(cache).anzahl() == 0);
        // die Vorgabe laesst sich fuer Tests umlenken
        SongLautheiten::setzeDateiFuerTests(cache);
        CHECK(SongLautheiten::standardDatei() == cache);
        SongLautheiten::setzeDateiFuerTests({});
        CHECK(SongLautheiten::standardDatei() != cache);
    }

    TEST_CASE("G5 Loop messen: zwei Durchlaeufe, gemessen der zweite; Abbruch ueber den Fortschritt")
    {
        const App app;
        const QString pfad = looptest::repoWurzel().filePath(QStringLiteral("asset/loop/effects_demo.lvloop"));
        lumi::loop::Loop loop;
        REQUIRE(lumi::loop::ladeLoop(pfad, loop));
        loop.richte();
        const lumi::loop::Ablauf ablauf = lumi::loop::plane(loop, kRate);
        REQUIRE(ablauf.durchlaufFrames > 0);
        const auto klaenge =
            std::make_shared<const std::vector<lumi::loop::Klang>>(looptest::klaengeOhneEngine(ablauf));

        // von Hand: zwei Durchlaeufe am Stueck, gemessen der zweite
        Lautheit erwartet;
        {
            lumi::loop::Klangerzeuger erzeuger;
            erzeuger.setzeProgramm(lumi::loop::baueProgramm(loop, kRate, klaenge));
            const auto frames = static_cast<std::size_t>(ablauf.durchlaufFrames);
            std::vector<float> zwei(frames * 4);
            for (std::size_t stelle = 0; stelle < 2 * frames; stelle += 441)
                erzeuger.render(zwei.data() + 2 * stelle, static_cast<int>(std::min<std::size_t>(441, 2 * frames - stelle)));
            erwartet = messeLautheit(std::vector<float>(zwei.begin() + static_cast<std::ptrdiff_t>(2 * frames), zwei.end()),
                                     kRate);
        }
        REQUIRE_FALSE(erwartet.stille);

        double zuletzt = -1.0;
        int meldungen = 0;
        const std::optional<Lautheit> gemessen =
            messeLoop(lumi::loop::baueProgramm(loop, kRate, klaenge), kRate, [&](double anteil) {
                CHECK(anteil >= zuletzt);
                zuletzt = anteil;
                ++meldungen;
                return true;
            });
        REQUIRE(gemessen.has_value());
        CHECK(gemessen->lufs == doctest::Approx(erwartet.lufs).epsilon(1e-6));
        CHECK(gemessen->spitzeDb == doctest::Approx(erwartet.spitzeDb).epsilon(1e-6));
        CHECK(zuletzt == doctest::Approx(1.0));
        CHECK(meldungen > 10);
        MESSAGE("effects_demo: " << gemessen->lufs << " LUFS, Spitze " << gemessen->spitzeDb << " dB");

        // abgebrochen: kein Ergebnis
        int bis = 0;
        CHECK_FALSE(messeLoop(lumi::loop::baueProgramm(loop, kRate, klaenge), kRate, [&](double) {
                        return ++bis < 5;
                    }).has_value());
        CHECK(bis == 5);
        // kein Programm, keine Rate
        CHECK_FALSE(messeLoop(nullptr, kRate, {}).has_value());
        CHECK_FALSE(messeLoop(lumi::loop::baueProgramm(loop, kRate, klaenge), 0, {}).has_value());
    }

    TEST_CASE("G5 Song messen ueber die BassEngine: der Ausschnitt, stueckweise dekodiert")
    {
        const App app;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString song = ordner.filePath(QStringLiteral("stille_dann_ton.wav"));
        REQUIRE(schreibeWav(song, 5.0, 5.0, -23.0));  // EBU Tech 3341 Fall 1: −23 dBFS ergibt −23 LUFS

        BassEngine bass;
        if (!bass.initialize())
        {
            MESSAGE("kein Ausgabegeraet: nichts geprueft (" << bass.getLastError().toStdString() << ")");
            return;
        }
        IAudioEngine& engine = bass;
        const int rate = engine.getSampleRate();
        REQUIRE(rate > 0);

        // nur der Ton
        double zuletzt = 0.0;
        const std::optional<Lautheit> ton = messeSong(engine, song, 5000, 5000, rate, [&](double a) {
            zuletzt = a;
            return true;
        });
        REQUIRE(ton.has_value());
        CHECK_FALSE(ton->stille);
        CHECK(ton->lufs == doctest::Approx(-23.0).epsilon(0.005));  // ±0,1 LU
        CHECK(ton->spitzeDb == doctest::Approx(-23.0).epsilon(0.01));
        CHECK(zuletzt == doctest::Approx(1.0).epsilon(0.01));
        // nur die Stille
        const std::optional<Lautheit> still = messeSong(engine, song, 0, 4000, rate, {});
        REQUIRE(still.has_value());
        CHECK(still->stille);
        // bis zum Ende (Dauer 0): die Stille liegt unter dem Gate, gemessen wird
        // der Ton — dazu die Bloecke von 400 ms ueber der Grenze von Stille zu
        // Ton; sie liegen ueber beiden Gates und ziehen leicht nach unten
        // (gemessen −23,12 LUFS, so rechnet R128)
        const std::optional<Lautheit> ganz = messeSong(engine, song, 0, 0, rate, {});
        REQUIRE(ganz.has_value());
        CHECK(ganz->lufs < ton->lufs);
        CHECK(ganz->lufs > ton->lufs - 0.2);
        // abgebrochen, nicht lesbar
        CHECK_FALSE(messeSong(engine, song, 0, 0, rate, [](double) { return false; }).has_value());
        CHECK_FALSE(messeSong(engine, ordner.filePath(QStringLiteral("fehlt.wav")), 0, 0, rate, {}).has_value());
    }

    TEST_CASE("G5 Messung: einen echten Song dekodieren und messen (LUMIVIZ_BELEG_SONG, meldet nur Zahlen)")
    {
        const QString song = qEnvironmentVariable("LUMIVIZ_BELEG_SONG");
        if (song.isEmpty())
        {
            MESSAGE("LUMIVIZ_BELEG_SONG nicht gesetzt — nichts gemessen");
            return;
        }
        const App app;
        BassEngine bass;
        if (!bass.initialize())
        {
            MESSAGE("kein Ausgabegeraet: nichts gemessen");
            return;
        }
        const int rate = bass.getSampleRate();
        for (int lauf = 1; lauf <= 3; ++lauf)
        {
            QElapsedTimer uhr;
            uhr.start();
            const std::optional<Lautheit> l = messeSong(bass, song, 0, 0, rate, {});
            const qint64 ms = uhr.elapsed();
            REQUIRE(l.has_value());
            MESSAGE("Lauf " << lauf << ": " << ms << " ms — " << l->lufs << " LUFS, Spitze " << l->spitzeDb << " dB");
        }
    }
}
