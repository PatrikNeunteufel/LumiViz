/**
 * @file test_LoopAufnahme.cpp
 * @brief Loop, Stufe 11b, Schritt 1: der Kern hinter „Record sample" — eine
 *        Aufnahme zuschneiden, als WAV schreiben, zu den eigenen Samples
 *        legen; die Aufnahme ueber die Audio-Engine
 *        (docs/audio/Loop_Konzept.md §4h)
 *
 * Ohne Fenster. Die Aufnahmen sind gerechnet; eigene Samples liegen in einem
 * Wegwerf-Ordner. **Der letzte Fall nimmt ueber die BassEngine vom
 * Standard-Eingang auf (zusammen rund eine Sekunde) und laeuft deshalb nur
 * mit `LUMIVIZ_AUFNAHME_BELEG=1`** — gespeichert wird nichts.
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "audio/BassEngine.hpp"
#include "loop/Aufnahme.hpp"
#include "loop/Instrument.hpp"
#include "loop/Sample.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QThread>
#include <QtEndian>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 48000;
constexpr double kPi = 3.14159265358979323846;

/// Stille, ein Ton, Stille — in Sekunden; `grund` ist ein leises Rauschen ueber allem.
std::vector<float> mitPausen(double davor, double tonSek, double danach, float pegel = 0.5f, float grund = 0.0f)
{
    const auto a = static_cast<std::size_t>(davor * kRate);
    const auto b = static_cast<std::size_t>(tonSek * kRate);
    const auto c = static_cast<std::size_t>(danach * kRate);
    std::vector<float> daten(2 * (a + b + c), 0.0f);
    std::uint32_t x = 99u;
    for (std::size_t t = 0; t < a + b + c; ++t)
    {
        x = x * 1664525u + 1013904223u;
        float wert = grund * static_cast<float>(static_cast<double>(x >> 8) / 8388608.0 - 1.0);
        if (t >= a && t < a + b) wert += pegel * static_cast<float>(std::sin(2.0 * kPi * 440.0 * static_cast<double>(t - a) / kRate + 1.0));
        daten[2 * t] = wert;
        daten[2 * t + 1] = 0.5f * wert;
    }
    return daten;
}

quint32 lies32(const QByteArray& daten, int ab)
{
    return qFromLittleEndian<quint32>(daten.constData() + ab);
}
quint16 lies16(const QByteArray& daten, int ab)
{
    return qFromLittleEndian<quint16>(daten.constData() + ab);
}
qint16 liesWert(const QByteArray& daten, int index)
{
    return qFromLittleEndian<qint16>(daten.constData() + 44 + 2 * index);
}

/// Eigene Samples in einem Wegwerf-Ordner.
struct AufnahmeOrdner
{
    QTemporaryDir tmp;
    QString ordner;
    AufnahmeOrdner()
    {
        REQUIRE(tmp.isValid());
        ordner = QDir(tmp.path()).filePath(QStringLiteral("samples"));
        setzeEigenenSampleOrdnerFuerTests(ordner);
        vergissSampleDaten();
    }
    ~AufnahmeOrdner()
    {
        setzeEigenenSampleOrdnerFuerTests(QString());
        vergissSampleDaten();
    }
};

}  // namespace

TEST_SUITE("Loop Aufnahme")
{
    TEST_CASE("Loop Aufnahme: die Stille an den Raendern — der Ausschnitt mit etwas Luft davor und danach")
    {
        // 0,2 s Stille, 0,5 s Ton, 0,3 s Stille
        const std::vector<float> daten = mitPausen(0.2, 0.5, 0.3);
        const Ausschnitt a = ohneStille(daten, kRate);
        // 5 ms vor dem ersten Ton, 30 ms nach dem letzten
        CHECK(a.von == doctest::Approx(9600 - 240).epsilon(0.01));
        CHECK(a.bis == doctest::Approx(9600 + 24000 + 1440).epsilon(0.005));
        CHECK(a.frames() > 24000);
        CHECK(a.frames() < 24000 + 240 + 1440 + 100);

        // ein leises Grundrauschen zaehlt als Stille (unter 2 % der Spitze)
        const Ausschnitt mitRauschen = ohneStille(mitPausen(0.2, 0.5, 0.3, 0.5f, 0.004f), kRate);
        CHECK(mitRauschen.von == doctest::Approx(9600 - 240).epsilon(0.02));
        CHECK(mitRauschen.bis == doctest::Approx(9600 + 24000 + 1440).epsilon(0.01));

        // der Ton reicht bis an die Raender: nichts faellt weg, nichts geht ueber die Aufnahme hinaus
        const std::vector<float> voll = mitPausen(0.0, 0.5, 0.0);
        CHECK(ohneStille(voll, kRate) == Ausschnitt{0, 24000});

        // alles still, leer, ohne Rate: die Aufnahme bleibt ganz
        CHECK(ohneStille(std::vector<float>(2 * 4800, 0.0f), kRate) == Ausschnitt{0, 4800});
        CHECK(ohneStille({}, kRate) == Ausschnitt{0, 0});
        CHECK(ohneStille(daten, 0) == Ausschnitt{0, 48000});

        CHECK(spitzeIn(daten, {0, 48000}) == doctest::Approx(0.5f).epsilon(0.01));
        CHECK(spitzeIn(daten, {0, 9000}) == 0.0f);
        CHECK(spitzeIn(daten, {-50, 10000000}) == doctest::Approx(0.5f).epsilon(0.01));  // begrenzt, kein Absturz
    }

    TEST_CASE("Loop Aufnahme: ausschneiden — die Blende an beiden Enden, Normalize, die Seiten bleiben")
    {
        const std::vector<float> daten = mitPausen(0.2, 0.5, 0.3);
        const Ausschnitt mitte{12000, 24000};  // mitten im Ton
        const std::vector<float> stueck = schneideAus(daten, kRate, mitte, false);
        REQUIRE(stueck.size() == 2 * 12000);
        // die Enden liegen auf null, dazwischen steht die Aufnahme unveraendert
        CHECK(stueck[0] == 0.0f);
        CHECK(stueck[1] == 0.0f);
        CHECK(stueck[stueck.size() - 2] == 0.0f);
        CHECK(stueck[2 * 6000] == daten[2 * (12000 + 6000)]);
        CHECK(stueck[2 * 6000 + 1] == daten[2 * (12000 + 6000) + 1]);  // rechts bleibt rechts
        CHECK(stueck[2 * 96] == daten[2 * (12000 + 96)]);              // nach 2 ms ist die Blende vorbei
        CHECK(std::abs(stueck[2 * 48]) < std::abs(daten[2 * (12000 + 48)]) + 1e-6f);
        CHECK(spitzeIn(stueck, {0, 12000}) == doctest::Approx(0.5f).epsilon(0.01));

        // Normalize: der groesste Betrag wird 0,9; die Form bleibt
        const std::vector<float> laut = schneideAus(daten, kRate, mitte, true);
        CHECK(spitzeIn(laut, {0, 12000}) == doctest::Approx(0.9f).epsilon(0.001));
        CHECK(laut[2 * 6000] == doctest::Approx(stueck[2 * 6000] * 0.9f / spitzeIn(stueck, {0, 12000})).epsilon(0.001));
        // Stille bleibt Stille
        const std::vector<float> still = schneideAus(daten, kRate, {0, 4800}, true);
        CHECK(spitzeIn(still, {0, 4800}) == 0.0f);

        // der Ausschnitt wird auf die Aufnahme begrenzt; nichts darin: leer
        CHECK(schneideAus(daten, kRate, {40000, 99999999}, false).size() == 2 * 8000);
        CHECK(schneideAus(daten, kRate, {-100, 100}, false).size() == 2 * 100);
        CHECK(schneideAus(daten, kRate, {5000, 5000}, false).empty());
        CHECK(schneideAus(daten, kRate, {9000, 100}, false).empty());
        CHECK(schneideAus({}, kRate, {0, 100}, false).empty());
        // ein sehr kurzer Ausschnitt: die Blende nimmt hoechstens je die Haelfte
        const std::vector<float> kurz = schneideAus(daten, kRate, {12000, 12010}, false);
        REQUIRE(kurz.size() == 20);
        CHECK(kurz[0] == 0.0f);
        // derselbe Aufruf ergibt dieselben Werte
        CHECK(schneideAus(daten, kRate, mitte, true) == laut);
    }

    TEST_CASE("Loop Aufnahme: die WAV-Datei — Stereo, 16 Bit, und die BassEngine liest sie wieder")
    {
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const QString pfad = QDir(tmp.path()).filePath(QStringLiteral("probe.wav"));
        // vier Abtastwerte: links, rechts
        const std::vector<float> daten = {0.5f, -0.5f, 1.0f, -1.0f, 1.7f, -3.0f, 0.0f, 0.25f};
        REQUIRE(schreibeWav(pfad, daten, 44100));

        QFile datei(pfad);
        REQUIRE(datei.open(QIODevice::ReadOnly));
        const QByteArray roh = datei.readAll();
        REQUIRE(roh.size() == 44 + 16);
        CHECK(roh.left(4) == QByteArray("RIFF"));
        CHECK(lies32(roh, 4) == 36u + 16u);
        CHECK(roh.mid(8, 8) == QByteArray("WAVEfmt "));
        CHECK(lies32(roh, 16) == 16u);
        CHECK(lies16(roh, 20) == 1);        // PCM
        CHECK(lies16(roh, 22) == 2);        // Stereo
        CHECK(lies32(roh, 24) == 44100u);
        CHECK(lies32(roh, 28) == 44100u * 4u);
        CHECK(lies16(roh, 32) == 4);
        CHECK(lies16(roh, 34) == 16);
        CHECK(roh.mid(36, 4) == QByteArray("data"));
        CHECK(lies32(roh, 40) == 16u);
        CHECK(liesWert(roh, 0) == 16384);   // 0,5
        CHECK(liesWert(roh, 1) == -16384);
        CHECK(liesWert(roh, 2) == 32767);
        CHECK(liesWert(roh, 3) == -32767);
        CHECK(liesWert(roh, 4) == 32767);   // ueber 1: begrenzt
        CHECK(liesWert(roh, 5) == -32767);
        CHECK(liesWert(roh, 6) == 0);
        CHECK(liesWert(roh, 7) == 8192);

        // was sich nicht schreiben laesst
        CHECK_FALSE(schreibeWav(pfad, {}, 44100));
        CHECK_FALSE(schreibeWav(pfad, daten, 0));
        CHECK_FALSE(schreibeWav(QDir(tmp.path()).filePath(QStringLiteral("gibt/es/nicht/x.wav")), daten, 44100));

        // --- eine laengere Aufnahme, von der BassEngine dekodiert: dieselben Werte auf 16 Bit genau
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QCoreApplication app{argc, argv};
        BassEngine engine;
        if (!engine.initialize())
        {
            MESSAGE("kein Ausgabegeraet: das Wiederlesen ist nicht geprueft (" << engine.getLastError().toStdString() << ")");
            return;
        }
        const std::vector<float> ton = schneideAus(mitPausen(0.0, 0.25, 0.0), kRate, {0, 12000}, false);
        const QString pfadTon = QDir(tmp.path()).filePath(QStringLiteral("ton.wav"));
        REQUIRE(schreibeWav(pfadTon, ton, kRate));
        const std::vector<float> gelesen = engine.decodeStereo(pfadTon, kRate, 30.0);
        REQUIRE(gelesen.size() == ton.size());
        double abweichung = 0.0;
        for (std::size_t i = 0; i < ton.size(); ++i)
            abweichung = std::max(abweichung, static_cast<double>(std::abs(gelesen[i] - ton[i])));
        MESSAGE("WAV durch die BassEngine: " << gelesen.size() / 2 << " Abtastwerte, groesste Abweichung " << std::to_string(abweichung));
        CHECK(abweichung < 1.0 / 32000.0);
    }

    TEST_CASE("Loop Aufnahme: speichern unter den eigenen Samples — Name, Vorschlag, ersetzen")
    {
        const AufnahmeOrdner u;
        const std::vector<float> ton = schneideAus(mitPausen(0.0, 0.25, 0.0), kRate, {0, 12000}, false);

        CHECK(vorschlagAufnahmeTitel() == QStringLiteral("Recording"));
        const std::uint64_t vorher = instrumentFassung();
        CHECK(speichereAufnahme(ton, kRate, QStringLiteral("Table Knock")) == QStringLiteral("user:table_knock"));
        CHECK(QFileInfo::exists(QDir(u.ordner).filePath(QStringLiteral("table_knock.wav"))));
        CHECK(eigeneSamples() == QStringList{QStringLiteral("user:table_knock")});
        CHECK(instrumentFassung() > vorher);  // wer Toene gerechnet hat, rechnet neu

        // der Vorschlag: der erste freie
        CHECK(speichereAufnahme(ton, kRate, QStringLiteral("Recording")) == QStringLiteral("user:recording"));
        CHECK(vorschlagAufnahmeTitel() == QStringLiteral("Recording 2"));
        CHECK(speichereAufnahme(ton, kRate, QStringLiteral("Recording 2")) == QStringLiteral("user:recording_2"));
        CHECK(vorschlagAufnahmeTitel() == QStringLiteral("Recording 3"));

        // derselbe Name noch einmal: ersetzt; was dazu gemerkt war, ist vergessen
        merkeSampleDaten(QStringLiteral("user:table_knock"), kRate, ton);
        REQUIRE(sampleDaten(QStringLiteral("user:table_knock")) != nullptr);
        const std::vector<float> kuerzer(ton.begin(), ton.begin() + 2 * 4800);
        CHECK(speichereAufnahme(kuerzer, kRate, QStringLiteral("table knock")) == QStringLiteral("user:table_knock"));
        CHECK(sampleDaten(QStringLiteral("user:table_knock")) == nullptr);
        CHECK(QFileInfo(QDir(u.ordner).filePath(QStringLiteral("table_knock.wav"))).size() == 44 + 4 * 4800);
        CHECK(eigeneSamples().size() == 3);

        // lag unter dem Namen eine Datei mit anderer Endung, bleibt nur die Aufnahme
        {
            QFile alt(QDir(u.ordner).filePath(QStringLiteral("cello.mp3")));
            REQUIRE(alt.open(QIODevice::WriteOnly));
            alt.write("x");
        }
        CHECK(speichereAufnahme(ton, kRate, QStringLiteral("Cello")) == QStringLiteral("user:cello"));
        CHECK(QFileInfo::exists(QDir(u.ordner).filePath(QStringLiteral("cello.wav"))));
        CHECK_FALSE(QFileInfo::exists(QDir(u.ordner).filePath(QStringLiteral("cello.mp3"))));
        CHECK(sampleDatei(QStringLiteral("user:cello")) == QDir(u.ordner).filePath(QStringLiteral("cello.wav")));

        // woraus kein Name wird, und was leer ist
        CHECK(speichereAufnahme(ton, kRate, QStringLiteral("???")).isEmpty());
        CHECK(speichereAufnahme({}, kRate, QStringLiteral("Leer")).isEmpty());
        CHECK(speichereAufnahme(ton, 0, QStringLiteral("Ohne Rate")).isEmpty());
        CHECK(eigeneSamples().size() == 4);
    }

    TEST_CASE("Loop Aufnahme: eine Engine, die nicht aufnehmen kann, sagt es")
    {
        papp::PappMischEngine engine;
        CHECK(engine.getRecordDevices().empty());
        CHECK_FALSE(engine.recordStart(-1, kRate, 1.0));
        CHECK_FALSE(engine.isRecording());
        CHECK(engine.recordedFrames() == 0);
        CHECK(engine.recordLevel() == 0.0f);
        CHECK(engine.recordStop().empty());

        // die BassEngine ohne laufende Aufnahme: nichts abzuholen, falsche Angaben werden abgelehnt
        BassEngine bass;
        CHECK_FALSE(bass.isRecording());
        CHECK(bass.recordedFrames() == 0);
        CHECK(bass.recordStop().empty());
        CHECK_FALSE(bass.recordStart(-1, 0, 1.0));
        CHECK_FALSE(bass.recordStart(-1, kRate, 0.0));
        CHECK_FALSE(bass.isRecording());
    }

    TEST_CASE("Loop Aufnahme mit Eingang: die BassEngine nimmt auf — endet von selbst, laesst sich abbrechen")
    {
        if (qEnvironmentVariableIntValue("LUMIVIZ_AUFNAHME_BELEG") != 1)
        {
            MESSAGE("nimmt vom Standard-Eingang auf — laeuft nur mit LUMIVIZ_AUFNAHME_BELEG=1; nichts geprueft");
            return;
        }
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QCoreApplication app{argc, argv};
        BassEngine engine;
        const bool mitAusgabe = engine.initialize();

        const std::vector<AudioDeviceInfo> eingaenge = engine.getRecordDevices();
        std::string liste;
        for (const AudioDeviceInfo& e : eingaenge)
            liste += "\n  " + std::to_string(e.id) + ": " + e.name.toStdString() + (e.isDefault ? " [Standard]" : "") +
                     (e.isLoopback ? " [Mitschnitt der Ausgabe]" : "");
        MESSAGE("Eingaenge der BassEngine (" << (mitAusgabe ? std::string("mit") : std::string("ohne")) << " Ausgabegeraet):" << liste);
        if (!engine.recordStart(-1, kRate, 0.5))
        {
            MESSAGE("kein Eingang zu oeffnen (" << engine.getLastError().toStdString() << "): nichts geprueft");
            return;
        }
        CHECK(engine.isRecording());

        // --- eine halbe Sekunde: die Aufnahme endet von selbst, genau bei der verlangten Menge
        QElapsedTimer uhr;
        uhr.start();
        std::int64_t davor = 0;
        std::int64_t kleinsterSchritt = 0;
        float pegel = 0.0f;
        while (engine.isRecording() && uhr.elapsed() < 3000)
        {
            QThread::msleep(2);
            const std::int64_t jetzt = engine.recordedFrames();
            if (jetzt > davor && (kleinsterSchritt == 0 || jetzt - davor < kleinsterSchritt)) kleinsterSchritt = jetzt - davor;
            davor = jetzt;
            pegel = std::max(pegel, engine.recordLevel());
        }
        const qint64 dauerMs = uhr.elapsed();
        CHECK_FALSE(engine.isRecording());
        CHECK(engine.recordedFrames() == 24000);
        const std::vector<float> daten = engine.recordStop();
        CHECK(daten.size() == 2 * 24000);
        CHECK(engine.recordStop().empty());  // abgeholt ist abgeholt
        CHECK(engine.recordedFrames() == 0);
        MESSAGE("eine halbe Sekunde verlangt: zu Ende nach " << dauerMs << " ms, kleinster Zuwachs " << kleinsterSchritt
                << " Abtastwerte (" << std::to_string(1000.0 * static_cast<double>(kleinsterSchritt) / kRate)
                << " ms), groesster Pegel " << std::to_string(pegel) << ", Spitze der Aufnahme "
                << std::to_string(spitzeIn(daten, {0, 24000})));

        // --- abgebrochen nach rund 300 ms: was bis dahin kam
        REQUIRE(engine.recordStart(-1, kRate, 30.0));
        QThread::msleep(300);
        const std::vector<float> kurz = engine.recordStop();
        CHECK_FALSE(engine.isRecording());
        CHECK(kurz.size() % 2 == 0);
        CHECK(kurz.size() / 2 < 24000u);
        MESSAGE("nach 300 ms abgebrochen: " << kurz.size() / 2 << " Abtastwerte");
        // danach kommt nichts mehr an
        QThread::msleep(150);
        CHECK(engine.recordedFrames() == 0);

        // --- eine neue Aufnahme verwirft die laufende
        REQUIRE(engine.recordStart(-1, kRate, 30.0));
        QThread::msleep(120);
        REQUIRE(engine.recordStart(-1, kRate, 30.0));
        CHECK(engine.recordedFrames() < 4800);
        (void)engine.recordStop();
        if (mitAusgabe) CHECK(engine.isInitialized());
    }
}
