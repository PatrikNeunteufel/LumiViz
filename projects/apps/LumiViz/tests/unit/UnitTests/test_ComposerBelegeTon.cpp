/**
 ****************************************************************************************
 * @file   test_ComposerBelegeTon.cpp
 * @brief  Composer, vor C5b: Belege zum Ueberlagern der Audio-Kanaele
 *         (docs/ui/Composer_Konzept.md §4, Befund Patrik S96)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Diese Tests bauen NICHTS am Composer. Sie halten fest, was BASS und BASSmix
 * an unserer Engine koennen — bevor entschieden wird, woher die Streams der
 * Audio-Kanaele kommen und wessen Spektrum die Visuals sehen:
 *
 * - Teil 1: zwei Streams der echten `BassEngine` spielen zugleich; was Pegel
 *   und Spektrum eines Streams von seiner Lautstaerke merken; gleitende
 *   Lautstaerke.
 * - Teil 2: ein Mixer-Stream (BASSmix) liefert die Summe seiner Quellen mit
 *   ihren Pegeln — abgezaehlt am dekodierenden Mixer, und als Spektrum am
 *   spielenden.
 *
 * Die Faelle mit Geraet spielen etwa eine Sekunde sehr leisen Ton. Ohne
 * Ausgabegeraet melden sie das und pruefen nichts.
 *
 * BASSmix wird zur Laufzeit aus `externals/bass/bassmix24` geladen, nicht
 * gelinkt: CMakeCraft v0.9.0 reicht `external_options` eines App-Tests nicht
 * durch (AppCreate.cmake:767 uebergibt fest `{}`), `BASS_MIX` am Test-Target
 * bleibt deshalb ohne Wirkung. Die Konstanten unten stehen so in `bassmix.h`.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/BassEngine.hpp"

#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QTemporaryDir>
#include <QThread>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <bass.h>

#include <QDir>
#include <QFileInfo>
#include <QLibrary>

namespace
{

// --- BASSmix, zur Laufzeit geladen (Werte und Signaturen aus bassmix.h) -------
constexpr DWORD kMixNoRampIn = 0x800000;  // BASS_MIXER_CHAN_NORAMPIN
constexpr DWORD kMixEnvVol = 2;           // BASS_MIXER_ENV_VOL
struct MixKnoten                          // BASS_MIXER_NODE
{
    QWORD pos;
    float value;
};

struct Mix
{
    QLibrary lib;
    HSTREAM(WINAPI* StreamCreate)(DWORD, DWORD, DWORD) = nullptr;
    BOOL(WINAPI* StreamAddChannel)(HSTREAM, DWORD, DWORD) = nullptr;
    BOOL(WINAPI* ChannelSetPosition)(DWORD, QWORD, DWORD) = nullptr;
    QWORD(WINAPI* ChannelGetPosition)(DWORD, DWORD) = nullptr;
    BOOL(WINAPI* ChannelSetEnvelope)(DWORD, DWORD, const MixKnoten*, DWORD) = nullptr;
    bool gut = false;
    QString pfad;

    Mix()
    {
        // .../projects/apps/LumiViz/tests/unit/UnitTests -> Wurzel des Repos
        QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
        for (int i = 0; i < 6; ++i) wurzel.cdUp();
        pfad = wurzel.filePath(QStringLiteral("externals/bass/bassmix24/win/x64/bassmix.dll"));
        lib.setFileName(pfad);
        if (!lib.load()) return;
        StreamCreate = reinterpret_cast<decltype(StreamCreate)>(lib.resolve("BASS_Mixer_StreamCreate"));
        StreamAddChannel =
            reinterpret_cast<decltype(StreamAddChannel)>(lib.resolve("BASS_Mixer_StreamAddChannel"));
        ChannelSetPosition =
            reinterpret_cast<decltype(ChannelSetPosition)>(lib.resolve("BASS_Mixer_ChannelSetPosition"));
        ChannelGetPosition =
            reinterpret_cast<decltype(ChannelGetPosition)>(lib.resolve("BASS_Mixer_ChannelGetPosition"));
        ChannelSetEnvelope =
            reinterpret_cast<decltype(ChannelSetEnvelope)>(lib.resolve("BASS_Mixer_ChannelSetEnvelope"));
        gut = StreamCreate != nullptr && StreamAddChannel != nullptr && ChannelSetPosition != nullptr &&
              ChannelGetPosition != nullptr && ChannelSetEnvelope != nullptr;
    }
};

constexpr int kRate = 44100;
constexpr double kPi = 3.14159265358979323846;
constexpr float kFreqA = 1000.0f;
constexpr float kFreqB = 3000.0f;
constexpr float kAmpA = 0.5f;
constexpr float kAmpB = 0.25f;
constexpr float kLeise = 0.03f;  ///< Lautstaerke der Faelle mit Geraet
constexpr int kFft = 2048;

/// Der Wert, den die Sinus-Datei am Abtastwert `n` traegt (16 Bit, wie BASS ihn liest).
float sinusWert(float freq, float amp, int n)
{
    const double v = amp * std::sin(2.0 * kPi * freq * n / kRate);
    return static_cast<float>(std::lround(v * 32767.0)) / 32768.0f;
}

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

/// Die zwei Sinus-Dateien eines Falls.
struct Dateien
{
    QTemporaryDir ordner;
    QString a, b;
    bool gut = false;
    Dateien()
    {
        if (!ordner.isValid()) return;
        a = ordner.filePath(QStringLiteral("a_1000.wav"));
        b = ordner.filePath(QStringLiteral("b_3000.wav"));
        gut = schreibeSinus(a, kFreqA, kAmpA, 3) && schreibeSinus(b, kFreqB, kAmpB, 3);
    }
};

/// Qt-Anwendung fuer einen Fall (die Engine fragt nach dem Ordner der Exe).
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

/// Groesster Wert des Spektrums um `freq` (zwei Linien links und rechts).
float spitzeBei(const std::vector<float>& fft, float freq)
{
    const int mitte = static_cast<int>(std::lround(freq * kFft / kRate));
    float spitze = 0.0f;
    for (int i = std::max(0, mitte - 2); i <= mitte + 2 && i < static_cast<int>(fft.size()); ++i)
        spitze = std::max(spitze, fft[static_cast<std::size_t>(i)]);
    return spitze;
}

std::vector<float> spektrum(DWORD kanal)
{
    std::vector<float> fft(kFft / 2);
    if (BASS_ChannelGetData(kanal, fft.data(), BASS_DATA_FFT2048) == static_cast<DWORD>(-1)) fft.clear();
    return fft;
}

DWORD dekodierQuelle(const QString& datei)
{
    const std::string pfad = datei.toLocal8Bit().constData();
    return BASS_StreamCreateFile(FALSE, pfad.c_str(), 0, 0, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
}

std::string zahl(double v)
{
    return std::to_string(v);
}

}  // namespace

// =============================================================================
// Teil 1 — zwei Streams an der echten Engine
// =============================================================================

TEST_CASE("Beleg Ton 1: zwei Streams der BassEngine spielen zugleich")
{
    App app;
    Dateien dateien;
    REQUIRE(dateien.gut);
    BassEngine engine;
    if (!engine.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << engine.getLastError().toStdString() << ")");
        return;
    }

    const AudioStreamHandle a = engine.createStream(dateien.a);
    const AudioStreamHandle b = engine.createStream(dateien.b);
    REQUIRE(a != INVALID_STREAM);
    REQUIRE(b != INVALID_STREAM);
    REQUIRE(a != b);
    engine.setVolume(a, kLeise);
    engine.setVolume(b, kLeise);
    REQUIRE(engine.play(a));
    REQUIRE(engine.play(b));
    QThread::msleep(300);

    SUBCASE("beide laufen, jeder mit seiner Stelle")
    {
        CHECK(engine.isPlaying(a));
        CHECK(engine.isPlaying(b));
        const int posA = engine.getPositionMs(a);
        const int posB = engine.getPositionMs(b);
        CHECK(posA > 100);
        CHECK(posB > 100);
        // jeder laesst sich fuer sich setzen, der andere laeuft weiter
        REQUIRE(engine.setPositionMs(b, 1500));
        QThread::msleep(150);
        CHECK(engine.getPositionMs(b) >= 1500);
        CHECK(engine.getPositionMs(a) > posA);
        CHECK(engine.getPositionMs(a) < 1000);
        // und fuer sich halten
        REQUIRE(engine.pause(a));
        CHECK(!engine.isPlaying(a));
        CHECK(engine.isPlaying(b));
    }

    SUBCASE("das Spektrum eines Streams kennt nur ihn — keine Summe")
    {
        const std::vector<float> fftA = spektrum(a);
        const std::vector<float> fftB = spektrum(b);
        REQUIRE(!fftA.empty());
        REQUIRE(!fftB.empty());
        MESSAGE("Stream A: 1000 Hz " << zahl(spitzeBei(fftA, kFreqA)) << ", 3000 Hz "
                                     << zahl(spitzeBei(fftA, kFreqB)));
        MESSAGE("Stream B: 1000 Hz " << zahl(spitzeBei(fftB, kFreqA)) << ", 3000 Hz "
                                     << zahl(spitzeBei(fftB, kFreqB)));
        CHECK(spitzeBei(fftA, kFreqA) > 20.0f * spitzeBei(fftA, kFreqB));
        CHECK(spitzeBei(fftB, kFreqB) > 20.0f * spitzeBei(fftB, kFreqA));
    }

    SUBCASE("Spektrum und Pegel eines Streams: was sie von der Lautstaerke merken")
    {
        const float beiLeise = spitzeBei(spektrum(a), kFreqA);
        float lLeise = 0.0f, rLeise = 0.0f;
        engine.getChannelLevels(a, lLeise, rLeise);
        engine.setVolume(a, kLeise / 10.0f);
        QThread::msleep(200);
        const float beiLeiser = spitzeBei(spektrum(a), kFreqA);
        float lLeiser = 0.0f, rLeiser = 0.0f;
        engine.getChannelLevels(a, lLeiser, rLeiser);
        MESSAGE("Spektrum 1000 Hz bei Lautstaerke " << zahl(kLeise) << ": " << zahl(beiLeise) << ", bei "
                                                    << zahl(kLeise / 10.0f) << ": " << zahl(beiLeiser));
        MESSAGE("Pegel links bei " << zahl(kLeise) << ": " << zahl(lLeise) << ", bei " << zahl(kLeise / 10.0f)
                                   << ": " << zahl(lLeiser));
        // LUECKE fuer „die Visuals sehen die Summe mit den Pegeln": Spektrum und
        // Pegel eines Streams sind die der Datei — seine Lautstaerke aendert nichts.
        CHECK(beiLeise > 0.1f);
        CHECK(beiLeiser == doctest::Approx(beiLeise).epsilon(0.01));
        CHECK(lLeise > 0.1f);
        CHECK(lLeiser == doctest::Approx(lLeise).epsilon(0.01));
    }

    SUBCASE("gleitende Lautstaerke: BASS kann sie, die Engine reicht sie nicht durch")
    {
        REQUIRE(BASS_ChannelSlideAttribute(a, BASS_ATTRIB_VOL, 0.0f, 200) != 0);
        CHECK(BASS_ChannelIsSliding(a, BASS_ATTRIB_VOL) != 0);
        QThread::msleep(80);
        const float unterwegs = engine.getVolume(a);
        QThread::msleep(250);
        MESSAGE("Lautstaerke unterwegs: " << zahl(unterwegs) << ", am Ende: " << zahl(engine.getVolume(a)));
        CHECK(unterwegs > 0.0f);
        CHECK(unterwegs < kLeise);
        CHECK(engine.getVolume(a) == doctest::Approx(0.0f));
        CHECK(BASS_ChannelIsSliding(a, BASS_ATTRIB_VOL) == 0);
    }

    engine.freeStream(a);
    engine.freeStream(b);
}

// =============================================================================
// Teil 2 — der Mixer (BASSmix)
// =============================================================================

TEST_CASE("Beleg Ton 2a: der dekodierende Mixer liefert die Summe mit den Pegeln, abgezaehlt")
{
    Dateien dateien;
    REQUIRE(dateien.gut);
    // Geraet 0 = kein Ton: dekodieren geht ohne Ausgabegeraet
    REQUIRE(BASS_Init(0, kRate, 0, nullptr, nullptr) != 0);
    Mix mix;
    REQUIRE_MESSAGE(mix.gut, "bassmix.dll nicht geladen: " << mix.pfad.toStdString());

    const DWORD a = dekodierQuelle(dateien.a);
    const DWORD b = dekodierQuelle(dateien.b);
    REQUIRE(a != 0);
    REQUIRE(b != 0);
    const HSTREAM mixer = mix.StreamCreate(kRate, 2, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
    REQUIRE(mixer != 0);
    const auto lies = [mixer](int frames) {
        std::vector<float> puffer(static_cast<std::size_t>(frames) * 2);
        const DWORD bytes = static_cast<DWORD>(puffer.size() * sizeof(float));
        const DWORD gelesen = BASS_ChannelGetData(mixer, puffer.data(), bytes | BASS_DATA_FLOAT);
        puffer.resize(gelesen == static_cast<DWORD>(-1) ? 0 : gelesen / sizeof(float));
        return puffer;
    };
    /// Groesste Abweichung des Mixes von `pegelA * A + pegelB * B` ab Abtastwert `ab`.
    const auto abweichung = [](const std::vector<float>& mix, int ab, float pegelA, float pegelB) {
        float groesste = 0.0f;
        for (std::size_t i = 0; i + 1 < mix.size(); i += 2)
        {
            const int n = ab + static_cast<int>(i / 2);
            const float soll = pegelA * sinusWert(kFreqA, kAmpA, n) + pegelB * sinusWert(kFreqB, kAmpB, n);
            groesste = std::max(groesste, std::abs(mix[i] - soll));
            groesste = std::max(groesste, std::abs(mix[i + 1] - soll));
        }
        return groesste;
    };

    SUBCASE("Summe mit festem Pegel je Quelle")
    {
        REQUIRE(BASS_ChannelSetAttribute(b, BASS_ATTRIB_VOL, 0.5f) != 0);
        REQUIRE(mix.StreamAddChannel(mixer, a, kMixNoRampIn) != 0);
        REQUIRE(mix.StreamAddChannel(mixer, b, kMixNoRampIn) != 0);
        const std::vector<float> summe = lies(4096);
        REQUIRE(summe.size() == 4096u * 2u);
        const float fehler = abweichung(summe, 0, 1.0f, 0.5f);
        MESSAGE("groesste Abweichung von A + 0.5 * B: " << zahl(fehler));
        CHECK(fehler < 2e-3f);
        // und die Gegenprobe: ohne den Pegel von B stimmt es nicht
        CHECK(abweichung(summe, 0, 1.0f, 1.0f) > 0.05f);
    }

    SUBCASE("das Spektrum des Mixers zeigt beide Quellen im Verhaeltnis ihrer Pegel")
    {
        REQUIRE(BASS_ChannelSetAttribute(b, BASS_ATTRIB_VOL, 0.5f) != 0);
        REQUIRE(mix.StreamAddChannel(mixer, a, kMixNoRampIn) != 0);
        REQUIRE(mix.StreamAddChannel(mixer, b, kMixNoRampIn) != 0);
        const std::vector<float> fft = spektrum(mixer);
        REQUIRE(!fft.empty());
        const float beiA = spitzeBei(fft, kFreqA);
        const float beiB = spitzeBei(fft, kFreqB);
        MESSAGE("Mixer: 1000 Hz " << zahl(beiA) << ", 3000 Hz " << zahl(beiB) << ", Verhaeltnis "
                                  << zahl(beiA / beiB));
        // A traegt 0.5, B traegt 0.25 * 0.5: Verhaeltnis 4
        CHECK(beiA / beiB == doctest::Approx(4.0f).epsilon(0.15));
    }

    SUBCASE("Pegelkurve je Quelle: eine Huellkurve blendet A aus, B bleibt")
    {
        REQUIRE(mix.StreamAddChannel(mixer, a, kMixNoRampIn) != 0);
        REQUIRE(mix.StreamAddChannel(mixer, b, kMixNoRampIn) != 0);
        // von 1 auf 0 in den ersten 8192 Abtastwerten
        const MixKnoten knoten[2] = {{0, 1.0f}, {8192u * 2u * sizeof(float), 0.0f}};
        REQUIRE(mix.ChannelSetEnvelope(a, kMixEnvVol, knoten, 2) != 0);
        const std::vector<float> anfang = lies(4096);  // mitten in der Blende
        const std::vector<float> danach = lies(8192);  // 4096 … 12288: die Blende endet darin
        const std::vector<float> spaet = lies(4096);   // 12288 …: A ist aus
        REQUIRE(spaet.size() == 4096u * 2u);
        // in der Mitte der Blende (Abtastwert 4096) steht A auf halbem Pegel
        const std::vector<float> mitte(danach.begin(), danach.begin() + 64);
        MESSAGE("Abweichung am Anfang von A + B: " << zahl(abweichung({anfang.begin(), anfang.begin() + 64}, 0, 1.0f, 1.0f))
                << ", in der Mitte von 0.5 * A + B: " << zahl(abweichung(mitte, 4096, 0.5f, 1.0f))
                << ", danach von B allein: " << zahl(abweichung(spaet, 12288, 0.0f, 1.0f)));
        CHECK(abweichung(mitte, 4096, 0.5f, 1.0f) < 1e-2f);
        CHECK(abweichung(spaet, 12288, 0.0f, 1.0f) < 2e-3f);
    }

    SUBCASE("jede Quelle hat ihre Stelle und laesst sich fuer sich setzen")
    {
        REQUIRE(mix.StreamAddChannel(mixer, a, kMixNoRampIn) != 0);
        REQUIRE(mix.StreamAddChannel(mixer, b, kMixNoRampIn) != 0);
        (void)lies(4096);
        // B eine Sekunde vor, A bleibt
        const QWORD ziel = BASS_ChannelSeconds2Bytes(b, 1.0);
        REQUIRE(mix.ChannelSetPosition(b, ziel, BASS_POS_BYTE) != 0);
        const std::vector<float> summe = lies(4096);
        REQUIRE(summe.size() == 4096u * 2u);
        // A steht bei 4096, B bei 44100
        float groesste = 0.0f;
        for (std::size_t i = 512; i + 1 < summe.size(); i += 2)
        {
            const int n = static_cast<int>(i / 2);
            const float soll = sinusWert(kFreqA, kAmpA, 4096 + n) + sinusWert(kFreqB, kAmpB, kRate + n);
            groesste = std::max(groesste, std::abs(summe[i] - soll));
        }
        MESSAGE("Abweichung nach dem Sprung von B: " << zahl(groesste));
        CHECK(groesste < 2e-3f);
    }

    BASS_StreamFree(mixer);
    BASS_StreamFree(a);
    BASS_StreamFree(b);
    BASS_Free();
}

TEST_CASE("Beleg Ton 2b: der spielende Mixer — sein Spektrum ist die Summe, die zu hoeren ist")
{
    App app;
    Dateien dateien;
    REQUIRE(dateien.gut);
    BassEngine engine;
    if (!engine.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << engine.getLastError().toStdString() << ")");
        return;
    }
    Mix mix;
    REQUIRE_MESSAGE(mix.gut, "bassmix.dll nicht geladen: " << mix.pfad.toStdString());

    const DWORD a = dekodierQuelle(dateien.a);
    const DWORD b = dekodierQuelle(dateien.b);
    REQUIRE(a != 0);
    REQUIRE(b != 0);
    const HSTREAM mixer = mix.StreamCreate(kRate, 2, BASS_SAMPLE_FLOAT);
    REQUIRE(mixer != 0);
    REQUIRE(mix.StreamAddChannel(mixer, a, 0) != 0);
    REQUIRE(mix.StreamAddChannel(mixer, b, 0) != 0);
    // der Mixer ist fuer die Engine ein Stream wie jeder andere
    const auto griff = static_cast<AudioStreamHandle>(mixer);
    engine.setVolume(griff, kLeise);
    REQUIRE(engine.play(griff));
    QThread::msleep(300);
    CHECK(engine.isPlaying(griff));

    std::vector<float> fft(kFft / 2);
    REQUIRE(engine.getFFTData(griff, fft.data(), kFft));
    const float beiA = spitzeBei(fft, kFreqA);
    const float beiB = spitzeBei(fft, kFreqB);
    MESSAGE("spielender Mixer, beide voll: 1000 Hz " << zahl(beiA) << ", 3000 Hz " << zahl(beiB));
    CHECK(beiA > 0.01f);
    CHECK(beiB > 0.01f);

    // B auf null: nach dem Puffer des Mixers ist B aus dem Spektrum
    REQUIRE(BASS_ChannelSetAttribute(b, BASS_ATTRIB_VOL, 0.0f) != 0);
    // wie lange es dauert, bis die Aenderung im Spektrum (also am Ohr) ankommt:
    // der Mixer rechnet voraus, ein neuer Pegel gilt erst hinter seinem Puffer
    int verzugMs = -1;
    for (int ms = 0; ms <= 700; ms += 10)
    {
        REQUIRE(engine.getFFTData(griff, fft.data(), kFft));
        if (spitzeBei(fft, kFreqB) < beiB / 20.0f)
        {
            verzugMs = ms;
            break;
        }
        QThread::msleep(10);
    }
    float pufferSek = 0.0f;
    BASS_ChannelGetAttribute(mixer, BASS_ATTRIB_BUFFER, &pufferSek);
    MESSAGE("Pegelwechsel einer Quelle kommt nach etwa " << verzugMs << " ms an; Puffer des Mixers "
            << zahl(pufferSek) << " s, BASS_CONFIG_BUFFER " << BASS_GetConfig(BASS_CONFIG_BUFFER) << " ms");
    CHECK(verzugMs >= 0);
    QThread::msleep(static_cast<unsigned long>(std::max(0, 700 - verzugMs)));
    REQUIRE(engine.getFFTData(griff, fft.data(), kFft));
    MESSAGE("spielender Mixer, B auf null: 1000 Hz " << zahl(spitzeBei(fft, kFreqA)) << ", 3000 Hz "
                                                     << zahl(spitzeBei(fft, kFreqB)));
    CHECK(spitzeBei(fft, kFreqA) > 0.01f);
    CHECK(spitzeBei(fft, kFreqB) < beiB / 20.0f);

    // die Stelle jeder Quelle, wie sie zu hoeren ist
    const double stelleA = BASS_ChannelBytes2Seconds(a, mix.ChannelGetPosition(a, BASS_POS_BYTE));
    MESSAGE("Stelle von A im Mixer nach etwa 1 s: " << zahl(stelleA) << " s; Stelle des dekodierten Kanals: "
            << zahl(BASS_ChannelBytes2Seconds(a, BASS_ChannelGetPosition(a, BASS_POS_BYTE))) << " s");
    CHECK(stelleA > 0.7);
    CHECK(stelleA < 1.4);

    BASS_StreamFree(mixer);
    BASS_StreamFree(a);
    BASS_StreamFree(b);
}

TEST_CASE("Beleg Ton 2c: der Verzug des spielenden Mixers haengt an seinem Puffer")
{
    App app;
    Dateien dateien;
    REQUIRE(dateien.gut);
    BassEngine engine;
    if (!engine.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << engine.getLastError().toStdString() << ")");
        return;
    }
    Mix mix;
    REQUIRE_MESSAGE(mix.gut, "bassmix.dll nicht geladen: " << mix.pfad.toStdString());

    /// Zeit, bis ein Pegelwechsel von B im Spektrum des Mixers ankommt; -1 = nie.
    const auto verzug = [&](float pufferSek) {
        const DWORD a = dekodierQuelle(dateien.a);
        const DWORD b = dekodierQuelle(dateien.b);
        const HSTREAM mixer = mix.StreamCreate(kRate, 2, BASS_SAMPLE_FLOAT);
        REQUIRE(mixer != 0);
        REQUIRE(BASS_ChannelSetAttribute(mixer, BASS_ATTRIB_BUFFER, pufferSek) != 0);
        REQUIRE(mix.StreamAddChannel(mixer, a, 0) != 0);
        REQUIRE(mix.StreamAddChannel(mixer, b, 0) != 0);
        const auto griff = static_cast<AudioStreamHandle>(mixer);
        engine.setVolume(griff, kLeise);
        REQUIRE(engine.play(griff));
        QThread::msleep(400);
        std::vector<float> fft(kFft / 2);
        REQUIRE(engine.getFFTData(griff, fft.data(), kFft));
        const float beiB = spitzeBei(fft, kFreqB);
        REQUIRE(beiB > 0.01f);
        REQUIRE(BASS_ChannelSetAttribute(b, BASS_ATTRIB_VOL, 0.0f) != 0);
        int ms = -1;
        for (int t = 0; t <= 800; t += 5)
        {
            REQUIRE(engine.getFFTData(griff, fft.data(), kFft));
            if (spitzeBei(fft, kFreqB) < beiB / 20.0f)
            {
                ms = t;
                break;
            }
            QThread::msleep(5);
        }
        BASS_StreamFree(mixer);
        BASS_StreamFree(a);
        BASS_StreamFree(b);
        return ms;
    };

    const int beiHalb = verzug(0.5f);
    const int beiZehntel = verzug(0.1f);
    const int beiNull = verzug(0.0f);
    MESSAGE("Verzug eines Pegelwechsels: Puffer 0.5 s -> " << beiHalb << " ms, 0.1 s -> " << beiZehntel
            << " ms, 0 s -> " << beiNull << " ms");
    CHECK(beiHalb > 300);
    CHECK(beiZehntel >= 0);
    CHECK(beiZehntel < beiHalb);
    CHECK(beiNull >= 0);
    CHECK(beiNull < 150);
}
