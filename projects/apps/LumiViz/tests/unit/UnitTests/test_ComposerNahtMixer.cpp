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

namespace
{

/// Ein ID3v2.3-Rahmen mit Latin-1-Text.
QByteArray id3Rahmen(const char* kennung, const QByteArray& inhalt)
{
    QByteArray r(kennung, 4);
    const auto n = static_cast<quint32>(inhalt.size());
    r.append(static_cast<char>(n >> 24)).append(static_cast<char>(n >> 16));
    r.append(static_cast<char>(n >> 8)).append(static_cast<char>(n));
    r.append(2, '\0');  // Flags
    return r + inhalt;
}

/// Eine MP3 aus stillen Rahmen (MPEG-1 Layer III, 128 kbit/s, 44,1 kHz) mit
/// ID3v2.3-Kopf: Titel, Kuenstler, Album, Jahr und ein Songtext (USLT).
bool schreibeMp3MitTags(const QString& datei)
{
    QByteArray rahmen;
    rahmen += id3Rahmen("TIT2", QByteArray(1, '\0') + "Titel der Quelle");
    rahmen += id3Rahmen("TPE1", QByteArray(1, '\0') + "Kuenstler");
    rahmen += id3Rahmen("TALB", QByteArray(1, '\0') + "Album");
    rahmen += id3Rahmen("TYER", QByteArray(1, '\0') + "1999");
    rahmen += id3Rahmen("USLT", QByteArray(1, '\0') + "eng" + QByteArray(1, '\0') + "erste Zeile\nzweite Zeile");
    QByteArray kopf("ID3\x03\x00\x00", 6);
    const auto n = static_cast<quint32>(rahmen.size());  // syncsafe: je Byte sieben Bit
    kopf.append(static_cast<char>((n >> 21) & 0x7F)).append(static_cast<char>((n >> 14) & 0x7F));
    kopf.append(static_cast<char>((n >> 7) & 0x7F)).append(static_cast<char>(n & 0x7F));

    QByteArray still(417, '\0');  // 144 * 128000 / 44100, ohne Padding
    still[0] = static_cast<char>(0xFF);
    still[1] = static_cast<char>(0xFB);
    still[2] = static_cast<char>(0x90);
    still[3] = static_cast<char>(0x00);

    QFile f(datei);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(kopf + rahmen);
    for (int i = 0; i < 120; ++i) f.write(still);  // etwa drei Sekunden
    return f.error() == QFile::NoError;
}

}  // namespace

TEST_CASE("Naht N12: Tags und Songtext liest die Engine auch an einer Quelle des Mixers")
{
    // Beleg vor C5b Schritt 2: der laufende Song der Komposition bekommt seine
    // Tags von der Quelle — dieselben, die ein Stream des Players traegt.
    App app;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString datei = ordner.filePath(QStringLiteral("mit_tags.mp3"));
    REQUIRE(schreibeMp3MitTags(datei));

    BassEngine bass;
    if (!bass.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << bass.getLastError().toStdString() << ")");
        return;
    }
    IAudioEngine& engine = bass;

    // Voraussetzung: am Stream des Players stehen die Tags
    const AudioStreamHandle stream = engine.createStream(datei);
    REQUIRE(stream != INVALID_STREAM);
    QString titel, kuenstler, album, jahr, genre, nummer, lrc, text;
    REQUIRE(engine.getMetadata(stream, titel, kuenstler, album));
    REQUIRE(titel == QStringLiteral("Titel der Quelle"));
    (void)engine.getMetadataExtra(stream, jahr, genre, nummer);
    REQUIRE(jahr == QStringLiteral("1999"));
    REQUIRE(engine.getLyrics(stream, lrc, text));
    const QString textAmStream = text;
    REQUIRE(textAmStream.contains(QStringLiteral("zweite Zeile")));
    const int dauer = engine.getDurationMs(stream);
    engine.freeStream(stream);

    // dieselbe Datei als Quelle im Mixer: dasselbe
    const AudioStreamHandle mixer = engine.createMixer();
    REQUIRE(mixer != INVALID_STREAM);
    const AudioStreamHandle quelle = engine.mixerAddSource(mixer, datei);
    REQUIRE(quelle != INVALID_STREAM);
    titel.clear();
    jahr.clear();
    text.clear();
    CHECK(engine.getMetadata(quelle, titel, kuenstler, album));
    CHECK(titel == QStringLiteral("Titel der Quelle"));
    CHECK(kuenstler == QStringLiteral("Kuenstler"));
    CHECK(album == QStringLiteral("Album"));
    (void)engine.getMetadataExtra(quelle, jahr, genre, nummer);
    CHECK(jahr == QStringLiteral("1999"));
    CHECK(engine.getLyrics(quelle, lrc, text));
    CHECK(text == textAmStream);
    CHECK(engine.getDurationMs(quelle) == doctest::Approx(dauer).epsilon(0.02));

    engine.mixerRemoveSource(quelle);
    engine.freeStream(mixer);
}
