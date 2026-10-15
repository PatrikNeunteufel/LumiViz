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

#include <bass.h>  // Beleg G4: ein DSP am Mixer

#include <QCoreApplication>
#include <QDataStream>
#include <QElapsedTimer>
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
    // ... und alles, was `MainWindow::onAudioUpdate` an einem Stream liest
    // (Naht N13), liefert auch der Mixer: Wellenform, Kanalzahl, Spektrum je Kanal
    {
        std::vector<float> welle(1024, 0.0f);
        CHECK(engine.getWaveformData(mixer, welle.data(), 1024));
        float groesste = 0.0f;
        for (const float w : welle) groesste = std::max(groesste, std::abs(w));
        CHECK(groesste > 0.1f);
        CHECK(engine.getStreamChannels(mixer) == 2);
        std::vector<float> jeKanal(1024 * 2, 0.0f);
        CHECK(engine.getFFTDataStereo(mixer, jeKanal.data(), 1024));
    }

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

TEST_CASE("Messung: wie weit eine Quelle des Mixers nach Einstieg, Sprung und Start neben der Uhr liegt")
{
    // Befund im Hoertest S98 (Log): die Fuehrung setzte Quellen etwa eine Sekunde nach einem
    // Einstieg um rund 280 ms nach. Hier gemessen, woher das kommt: wie lange das Oeffnen und
    // das Setzen dauern, und wie die Stelle, die die Engine meldet, danach gegen die Wanduhr
    // laeuft. Prueft nichts, meldet Zahlen. Mit LUMIVIZ_BELEG_SONG=<Datei> an einem echten
    // Song, sonst an einem Sinus von 20 s.
    App app;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    QString datei = QString::fromLocal8Bit(qgetenv("LUMIVIZ_BELEG_SONG"));
    int ziel = 60000;
    if (datei.isEmpty() || !QFile::exists(datei))
    {
        datei = ordner.filePath(QStringLiteral("sinus_20s.wav"));
        REQUIRE(schreibeSinus(datei, kFreqA, 0.5f, 20));
        ziel = 5000;
    }
    BassEngine bass;
    if (!bass.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts gemessen");
        return;
    }
    IAudioEngine& engine = bass;
    const AudioStreamHandle mixer = engine.createMixer();
    REQUIRE(mixer != INVALID_STREAM);
    engine.setVolume(mixer, 0.02f);
    REQUIRE(engine.play(mixer));
    QThread::msleep(300);

    QElapsedTimer uhr;

    // --- 1) kalter Einstieg: oeffnen, setzen, laufen lassen — wie ein Sprung mitten in einen Clip
    uhr.start();
    const AudioStreamHandle a = engine.mixerAddSource(mixer, datei);
    const qint64 msOeffnen = uhr.elapsed();
    REQUIRE(a != INVALID_STREAM);
    uhr.restart();
    REQUIRE(engine.setSourcePositionMs(a, ziel));
    const qint64 msSetzen = uhr.elapsed();
    REQUIRE(engine.setSourcePaused(a, false));
    uhr.restart();
    MESSAGE("oeffnen " << std::to_string(msOeffnen) << " ms, setzen " << std::to_string(msSetzen) << " ms ("
                       << datei.toStdString() << ")");
    const auto messe = [&](const char* name, int basis) {
        std::string zeile = std::string(name) + ": Stelle minus Uhr [ms] alle 100 ms:";
        for (int i = 0; i < 15; ++i)
        {
            QThread::msleep(100);
            const qint64 soll = basis + uhr.elapsed();
            zeile += " " + std::to_string(engine.getSourcePositionMs(a) - soll);
        }
        MESSAGE(zeile);
    };
    messe("kalter Einstieg", ziel);

    // --- 2) Sprung bei laufender Quelle
    REQUIRE(engine.setSourcePositionMs(a, ziel + 30000 > engine.getDurationMs(a) ? 1000 : ziel + 30000));
    const int nachSprung = ziel + 30000 > engine.getDurationMs(a) ? 1000 : ziel + 30000;
    uhr.restart();
    messe("Sprung im Lauf", nachSprung);

    // --- 3) der Mixer haelt und laeuft wieder (Pause, Play)
    REQUIRE(engine.pause(mixer));
    QThread::msleep(500);
    const int gehalten = engine.getSourcePositionMs(a);
    REQUIRE(engine.play(mixer));
    uhr.restart();
    messe("nach Pause und Play", gehalten);

    // --- 4) gesetzt bei haltendem Mixer, dann Play (Klick ins Lineal bei stehender Uhr)
    REQUIRE(engine.pause(mixer));
    QThread::msleep(300);
    REQUIRE(engine.setSourcePositionMs(a, ziel));
    MESSAGE("gesetzt bei haltendem Mixer: die Engine meldet "
            << std::to_string(engine.getSourcePositionMs(a) - ziel) << " ms neben dem Ziel");
    REQUIRE(engine.play(mixer));
    uhr.restart();
    messe("gesetzt im Halt, dann Play", ziel);

    engine.mixerRemoveSource(a);
    engine.freeStream(mixer);
}

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

namespace
{

/// Ein DSP, der alles mit einem Faktor malnimmt (Fliesskomma, wie der Mixer).
void CALLBACK halbiere(HDSP, DWORD, void* puffer, DWORD laenge, void* faktor)
{
    auto* werte = static_cast<float*>(puffer);
    const float f = *static_cast<const float*>(faktor);
    for (DWORD i = 0; i < laenge / sizeof(float); ++i) werte[i] *= f;
}

}  // namespace

// Beleg vor G4 (Composer_Konzept §9g, L4 a): sehen die Visuals das Signal
// VOR oder NACH einem Baustein (DSP) am Mixer? Sie lesen Wellenform und
// Spektrum am Mixer (`MainWindow::onAudioUpdate`). Ausserdem: kommt das Gain
// einer Quelle (BASS_ATTRIB_VOL, auch ueber 1) VOR dem DSP an?
TEST_CASE("Beleg G4: Wellenform und Spektrum des Mixers zeigen das Signal hinter einem DSP am Mixer")
{
    App app;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString datei = ordner.filePath(QStringLiteral("a_1000.wav"));
    REQUIRE(schreibeSinus(datei, kFreqA, 0.25f, 8));  // laenger als der ganze Fall

    BassEngine bass;
    if (!bass.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << bass.getLastError().toStdString() << ")");
        return;
    }
    IAudioEngine& engine = bass;
    const AudioStreamHandle mixer = engine.createMixer();
    REQUIRE(mixer != INVALID_STREAM);
    engine.setVolume(mixer, kLeise);
    const AudioStreamHandle a = engine.mixerAddSource(mixer, datei);
    REQUIRE(a != INVALID_STREAM);
    REQUIRE(engine.play(mixer));
    REQUIRE(engine.setSourcePaused(a, false));
    QThread::msleep(300);

    const auto welleSpitze = [&] {
        std::vector<float> welle(2048, 0.0f);
        REQUIRE(engine.getWaveformData(mixer, welle.data(), 2048));
        float groesste = 0.0f;
        for (const float w : welle) groesste = std::max(groesste, std::abs(w));
        return groesste;
    };
    const float vorher = welleSpitze();
    const float spektrumVorher = spitzeBei(engine, mixer, kFreqA);
    MESSAGE("ohne DSP: Spitze der Wellenform " << vorher << ", Spektrum " << spektrumVorher);
    CHECK(vorher == doctest::Approx(0.25f).epsilon(0.1));

    // ein DSP am Mixer, der halbiert
    static float faktor = 0.5f;
    const HDSP dsp = BASS_ChannelSetDSP(static_cast<HSTREAM>(mixer), &halbiere, &faktor, 0);
    REQUIRE(dsp != 0);
    QThread::msleep(400);
    const float mitDsp = welleSpitze();
    const float spektrumMitDsp = spitzeBei(engine, mixer, kFreqA);
    MESSAGE("mit DSP x0,5: Spitze der Wellenform " << mitDsp << ", Spektrum " << spektrumMitDsp);
    CHECK(mitDsp == doctest::Approx(vorher * 0.5f).epsilon(0.1));
    CHECK(spektrumMitDsp == doctest::Approx(spektrumVorher * 0.5f).epsilon(0.15));

    // das Gain der Quelle ueber 1 kommt vor dem DSP an: ×4 an der Quelle, ×0,5 am Mixer = ×2
    REQUIRE(engine.setVolume(a, 4.0f));
    CHECK(engine.getVolume(a) == doctest::Approx(4.0f));  // bis S126 bei 1 geklemmt
    REQUIRE(engine.setVolume(a, 100.0f));
    CHECK(engine.getVolume(a) == doctest::Approx(kHoechsteLautstaerke));
    REQUIRE(engine.setVolume(a, 4.0f));
    QThread::msleep(400);
    const float gainUndDsp = welleSpitze();
    MESSAGE("Quelle x4, DSP x0,5: Spitze " << gainUndDsp << " (die Summe kann ueber 1 gehen)");
    CHECK(gainUndDsp == doctest::Approx(vorher * 2.0f).epsilon(0.1));

    REQUIRE(BASS_ChannelRemoveDSP(static_cast<HSTREAM>(mixer), dsp) != 0);

    // --- G4: der Limiter der Engine am Mixer. Quelle ×8: die Summe stuende bei 2,0
    REQUIRE(engine.setVolume(a, 8.0f));
    QThread::msleep(400);
    const float ohneLimiter = welleSpitze();
    MESSAGE("Quelle x8, ohne Limiter: Spitze " << ohneLimiter);
    CHECK(ohneLimiter == doctest::Approx(2.0f).epsilon(0.1));
    CHECK(engine.mixerLimiterFrames(mixer) == -1);
    REQUIRE(engine.setMixerLimiter(mixer, true));
    REQUIRE(engine.setMixerLimiter(mixer, true));  // zweimal an: einer
    QThread::msleep(400);
    const float mitLimiter = welleSpitze();
    MESSAGE("Quelle x8, mit Limiter: Spitze " << mitLimiter << ", begrenzte Frames "
                                               << engine.mixerLimiterFrames(mixer));
    CHECK(mitLimiter <= 0.99f);
    CHECK(mitLimiter > 0.9f);
    CHECK(engine.mixerLimiterFrames(mixer) > 0);
    // unter der Decke greift er nicht: Quelle ×1 (0,25) — nach dem Rueckweg wie ohne
    REQUIRE(engine.setVolume(a, 1.0f));
    QThread::msleep(1600);
    CHECK(welleSpitze() == doctest::Approx(vorher).epsilon(0.02));
    REQUIRE(engine.setMixerLimiter(mixer, false));
    CHECK(engine.mixerLimiterFrames(mixer) == -1);
    REQUIRE(engine.setVolume(a, 8.0f));
    QThread::msleep(400);
    CHECK(welleSpitze() == doctest::Approx(2.0f).epsilon(0.1));

    engine.mixerRemoveSource(a);
    REQUIRE(engine.setMixerLimiter(mixer, true));
    engine.freeStream(mixer);  // nimmt den Limiter mit
    CHECK(engine.mixerLimiterFrames(mixer) == -1);
}

TEST_CASE("G4: eine Engine ohne Limiter meldet das")
{
    papp::PappEngine engine;
    CHECK_FALSE(engine.setMixerLimiter(1, true));
    CHECK(engine.mixerLimiterFrames(1) == -1);
}
