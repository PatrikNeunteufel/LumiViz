/**
 ****************************************************************************************
 * @file   test_ComposerNahtMixer.cpp
 * @brief  Composer, Naht N12: der Mixer in der Engine
 *         (docs/ui/Composer_Konzept.md §4 und §12.2, Stufe C5b)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Belegt die Mixer-Aufrufe von `IAudioEngine` an der echten `BassEngine`:
 * Quellen kommen angehalten in den Mixer, jede hat ihre Stelle und ihren
 * Pegel, laesst sich fuer sich halten und wegnehmen — und das Spektrum des
 * Mixers ist die Summe dessen, was zu hoeren ist. Dazu: eine Engine, die den
 * Mixer nicht kennt, meldet das, statt etwas zu tun.
 *
 * Der Fall mit Geraet spielt etwa zwei Sekunden sehr leisen Ton. Ohne
 * Ausgabegeraet meldet er das und prueft nichts.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "audio/BassEngine.hpp"

#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QTemporaryDir>
#include <QThread>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace
{

constexpr int kRate = 44100;
constexpr double kPi = 3.14159265358979323846;
constexpr float kFreqA = 1000.0f;
constexpr float kFreqB = 3000.0f;
constexpr float kLeise = 0.03f;  ///< Lautstaerke des Mixers im Test
constexpr int kFft = 2048;

/// Sinus als WAV: 16 Bit, Stereo, beide Kanaele gleich.
bool schreibeSinus(const QString& datei, float freq, float amp, int sekunden)
{
    QFile f(datei);
    if (!f.open(QIODevice::WriteOnly)) return false;
    const quint32 frames = static_cast<quint32>(kRate * sekunden);
    const quint32 datenBytes = frames * 4;
    QDataStream s(&f);
    s.setByteOrder(QDataStream::LittleEndian);
    f.write("RIFF", 4);
    s << quint32(36 + datenBytes);
    f.write("WAVEfmt ", 8);
    s << quint32(16) << quint16(1) << quint16(2) << quint32(kRate) << quint32(kRate * 4) << quint16(4)
      << quint16(16);
    f.write("data", 4);
    s << datenBytes;
    for (quint32 n = 0; n < frames; ++n)
    {
        const auto w = static_cast<qint16>(
            std::lround(amp * std::sin(2.0 * kPi * freq * n / kRate) * 32767.0));
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

/// Groesster Wert des Spektrums um `freq`, gelesen ueber die Engine.
float spitzeBei(IAudioEngine& engine, AudioStreamHandle stream, float freq)
{
    std::vector<float> fft(kFft / 2);
    if (!engine.getFFTData(stream, fft.data(), kFft)) return -1.0f;
    const int mitte = static_cast<int>(std::lround(freq * kFft / kRate));
    float spitze = 0.0f;
    for (int i = std::max(0, mitte - 2); i <= mitte + 2 && i < static_cast<int>(fft.size()); ++i)
        spitze = std::max(spitze, fft[static_cast<std::size_t>(i)]);
    return spitze;
}

}  // namespace

TEST_CASE("Naht N12: eine Engine ohne Mixer meldet das und tut nichts")
{
    papp::PappEngine engine;
    engine.laengen.insert(QStringLiteral("a.mp3"), 180000);
    CHECK(engine.createMixer() == INVALID_STREAM);
    CHECK(engine.mixerAddSource(1, QStringLiteral("a.mp3")) == INVALID_STREAM);
    CHECK(engine.geoeffnet.isEmpty());  // nichts geoeffnet
    engine.mixerRemoveSource(1);
    CHECK(engine.getSourcePositionMs(1) == 0);
    CHECK(!engine.setSourcePositionMs(1, 1000));
    CHECK(!engine.setSourcePaused(1, false));
    CHECK(!engine.isSourcePlaying(1));
}

TEST_CASE("Naht N12: der Mixer der BassEngine — Quellen, Stelle, Pegel, Summe")
{
    App app;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString dateiA = ordner.filePath(QStringLiteral("a_1000.wav"));
    const QString dateiB = ordner.filePath(QStringLiteral("b_3000.wav"));
    REQUIRE(schreibeSinus(dateiA, kFreqA, 0.5f, 3));
    REQUIRE(schreibeSinus(dateiB, kFreqB, 0.5f, 3));

    BassEngine bass;
    if (!bass.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << bass.getLastError().toStdString() << ")");
        return;
    }
    IAudioEngine& engine = bass;  // nur ueber die Schnittstelle

    const AudioStreamHandle mixer = engine.createMixer();
    REQUIRE(mixer != INVALID_STREAM);
    engine.setVolume(mixer, kLeise);

    // --- eine Datei, die es nicht gibt, kommt nicht hinein
    CHECK(engine.mixerAddSource(mixer, ordner.filePath(QStringLiteral("fehlt.wav"))) == INVALID_STREAM);

    const AudioStreamHandle a = engine.mixerAddSource(mixer, dateiA);
    const AudioStreamHandle b = engine.mixerAddSource(mixer, dateiB);
    REQUIRE(a != INVALID_STREAM);
    REQUIRE(b != INVALID_STREAM);
    CHECK(engine.getDurationMs(a) == doctest::Approx(3000).epsilon(0.01));

    // --- der Mixer laeuft, die Quellen kommen angehalten
    REQUIRE(engine.play(mixer));
    QThread::msleep(250);
    CHECK(engine.isPlaying(mixer));
    CHECK(!engine.isSourcePlaying(a));
    CHECK(!engine.isSourcePlaying(b));
    CHECK(engine.getSourcePositionMs(a) == 0);
    CHECK(spitzeBei(engine, mixer, kFreqA) < 0.005f);
    CHECK(spitzeBei(engine, mixer, kFreqB) < 0.005f);

    // --- A laeuft allein
    REQUIRE(engine.setSourcePaused(a, false));
    QThread::msleep(250);
    CHECK(engine.isSourcePlaying(a));
    const int posA = engine.getSourcePositionMs(a);
    CHECK(posA > 100);
    CHECK(posA < 600);
    const float nurA = spitzeBei(engine, mixer, kFreqA);
    CHECK(nurA > 0.3f);
    CHECK(spitzeBei(engine, mixer, kFreqB) < nurA / 50.0f);

    // --- B dazu: an eigener Stelle, mit halbem Pegel — die Summe mit den Pegeln
    REQUIRE(engine.setSourcePositionMs(b, 1000));
    REQUIRE(engine.setVolume(b, 0.5f));
    REQUIRE(engine.setSourcePaused(b, false));
    QThread::msleep(250);
    CHECK(engine.getSourcePositionMs(b) >= 1000);
    CHECK(engine.getSourcePositionMs(b) < 1600);
    CHECK(engine.getSourcePositionMs(a) > posA);  // A lief weiter
    const float aInSumme = spitzeBei(engine, mixer, kFreqA);
    const float bInSumme = spitzeBei(engine, mixer, kFreqB);
    MESSAGE("Summe: 1000 Hz " << std::to_string(aInSumme) << ", 3000 Hz " << std::to_string(bInSumme));
    CHECK(aInSumme == doctest::Approx(nurA).epsilon(0.1));
    CHECK(bInSumme == doctest::Approx(nurA * 0.5f).epsilon(0.2));  // gleiche Datei-Amplitude, halber Pegel

    // --- A haelt: aus der Summe, seine Stelle steht, B laeuft weiter
    REQUIRE(engine.setSourcePaused(a, true));
    QThread::msleep(200);
    CHECK(!engine.isSourcePlaying(a));
    const int stehtA = engine.getSourcePositionMs(a);
    QThread::msleep(100);
    CHECK(engine.getSourcePositionMs(a) == stehtA);
    CHECK(spitzeBei(engine, mixer, kFreqA) < nurA / 50.0f);
    CHECK(spitzeBei(engine, mixer, kFreqB) > 0.1f);

    // --- B wegnehmen: der Mixer laeuft ohne Quellen weiter und spielt Stille
    engine.mixerRemoveSource(b);
    QThread::msleep(200);
    CHECK(engine.isPlaying(mixer));
    CHECK(spitzeBei(engine, mixer, kFreqB) < 0.005f);

    // --- das Ende einer Quelle: sie schweigt und steht an ihrem Ende, bleibt
    //     aber im Mixer — zurueckgesetzt spielt sie von selbst wieder
    REQUIRE(engine.setSourcePositionMs(a, 2800));
    REQUIRE(engine.setSourcePaused(a, false));
    QThread::msleep(500);
    CHECK(!engine.isSourcePlaying(a));
    CHECK(engine.getSourcePositionMs(a) == doctest::Approx(3000).epsilon(0.01));
    CHECK(spitzeBei(engine, mixer, kFreqA) < nurA / 50.0f);
    REQUIRE(engine.setSourcePositionMs(a, 500));
    QThread::msleep(250);
    CHECK(engine.isSourcePlaying(a));
    CHECK(engine.getSourcePositionMs(a) > 500);
    CHECK(engine.getSourcePositionMs(a) < 1100);
    CHECK(spitzeBei(engine, mixer, kFreqA) > 0.3f);

    engine.mixerRemoveSource(a);
    engine.freeStream(mixer);
}
