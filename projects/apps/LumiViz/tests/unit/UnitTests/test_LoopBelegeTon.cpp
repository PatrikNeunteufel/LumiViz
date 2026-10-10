/**
 ****************************************************************************************
 * @file   test_LoopBelegeTon.cpp
 * @brief  Loop, vor Stufe 3: Belege zur selbst rechnenden Quelle im Mixer
 *         (docs/audio/Loop_Konzept.md §5 Stufe 2, Entscheid L1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Diese Tests bauen NICHTS am Loop. Sie halten fest, was BASS und BASSmix an
 * dieser Installation mit einer Quelle tun, die ihren Ton in einem Rueckruf
 * selbst rechnet — bevor der Klangerzeuger darauf gebaut wird:
 *
 * - Teil 1 (ohne Geraet): die Quelle im dekodierenden Mixer. Kommt jeder
 *   Abtastwert dort an, wo ihn der Rueckruf hingelegt hat? Addieren sich zwei
 *   solche Quellen? Wie weit holt der Mixer voraus, und wer fuehrt die Stelle?
 * - Teil 2 (mit Geraet): die Quelle im spielenden Mixer der `BassEngine`. In
 *   welchem Thread laeuft der Rueckruf, wie gross sind die Bloecke, wie lange
 *   braucht eine Aenderung bis ans Ohr?
 *
 * Der Fall mit Geraet spielt etwa zwei Sekunden sehr leisen Ton. Ohne
 * Ausgabegeraet meldet er das und prueft nichts.
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/BassEngine.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>

#include <algorithm>
#include <atomic>
#include <climits>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <bass.h>
#include <bassmix.h>

namespace
{

constexpr int kLoopRate = 44100;
constexpr double kLoopPi = 3.14159265358979323846;
constexpr float kLoopLeise = 0.03f;  ///< Lautstaerke des Falls mit Geraet
/// Abstand der Klicks in Abtastwerten — mit Absicht kein Vielfaches einer
/// Blockgroesse (125 ms bei 44100 Hz waeren 5512,5).
constexpr int kSchritt = 5513;

/// Eine Quelle, die Klicks rechnet: an jedem Vielfachen von `schritt` der Wert
/// `klick`, sonst `grund`. Die Stelle fuehrt sie selbst.
struct KlickQuelle
{
    std::atomic<std::int64_t> frame{0};  ///< der naechste Abtastwert, den der Rueckruf rechnet
    int schritt = kSchritt;
    float klick = 0.5f;
    float grund = 0.0f;
    std::atomic<int> rufe{0};
    std::atomic<int> kleinsterBlock{INT_MAX};
    std::atomic<int> groessterBlock{0};
};

void zaehleBlock(std::atomic<int>& rufe, std::atomic<int>& kleinster, std::atomic<int>& groesster, int frames)
{
    rufe.fetch_add(1);
    kleinster.store(std::min(kleinster.load(), frames));
    groesster.store(std::max(groesster.load(), frames));
}

DWORD CALLBACK rechneKlick(HSTREAM /*handle*/, void* buffer, DWORD length, void* user)
{
    auto* q = static_cast<KlickQuelle*>(user);
    auto* aus = static_cast<float*>(buffer);
    const int frames = static_cast<int>(length / (2 * sizeof(float)));
    std::int64_t n = q->frame.load();
    for (int i = 0; i < frames; ++i, ++n)
    {
        const float v = n % q->schritt == 0 ? q->klick : q->grund;
        aus[2 * i] = v;
        aus[2 * i + 1] = v;
    }
    q->frame.store(n);
    zaehleBlock(q->rufe, q->kleinsterBlock, q->groessterBlock, frames);
    return static_cast<DWORD>(frames * 2 * sizeof(float));
}

/// Eine Quelle, die einen Sinus rechnet; die Amplitude laesst sich von aussen setzen.
struct TonQuelle
{
    std::atomic<std::int64_t> frame{0};
    std::atomic<float> amp{0.5f};
    int rate = kLoopRate;
    float freq = 1000.0f;
    std::atomic<int> rufe{0};
    std::atomic<int> kleinsterBlock{INT_MAX};
    std::atomic<int> groessterBlock{0};
    std::atomic<std::size_t> faden{0};  ///< Kennung des Threads, in dem der Rueckruf zuletzt lief
};

DWORD CALLBACK rechneTon(HSTREAM /*handle*/, void* buffer, DWORD length, void* user)
{
    auto* q = static_cast<TonQuelle*>(user);
    auto* aus = static_cast<float*>(buffer);
    const int frames = static_cast<int>(length / (2 * sizeof(float)));
    const float amp = q->amp.load();
    std::int64_t n = q->frame.load();
    for (int i = 0; i < frames; ++i, ++n)
    {
        const auto v = static_cast<float>(amp * std::sin(2.0 * kLoopPi * q->freq * static_cast<double>(n) / q->rate));
        aus[2 * i] = v;
        aus[2 * i + 1] = v;
    }
    q->frame.store(n);
    q->faden.store(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    zaehleBlock(q->rufe, q->kleinsterBlock, q->groessterBlock, frames);
    return static_cast<DWORD>(frames * 2 * sizeof(float));
}

/// Qt-Anwendung fuer einen Fall (die Engine fragt nach dem Ordner der Exe).
struct LoopApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    std::unique_ptr<QCoreApplication> app;
    LoopApp()
    {
        if (QCoreApplication::instance() == nullptr) app = std::make_unique<QCoreApplication>(argc, argv);
    }
};

/// `frames` Abtastwerte (Stereo, float) aus einem dekodierenden Kanal lesen.
std::vector<float> liesFrames(DWORD kanal, int frames)
{
    std::vector<float> puffer(static_cast<std::size_t>(frames) * 2);
    const DWORD bytes = static_cast<DWORD>(puffer.size() * sizeof(float));
    const DWORD gelesen = BASS_ChannelGetData(kanal, puffer.data(), bytes | BASS_DATA_FLOAT);
    puffer.resize(gelesen == static_cast<DWORD>(-1) ? 0 : gelesen / sizeof(float));
    return puffer;
}

HSTREAM klickStream(KlickQuelle& q)
{
    return BASS_StreamCreate(kLoopRate, 2, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT, &rechneKlick, &q);
}

}  // namespace

// =============================================================================
// Teil 1 — die selbst rechnende Quelle im dekodierenden Mixer (ohne Geraet)
// =============================================================================

TEST_CASE("Beleg Loop 1: die selbst rechnende Quelle im dekodierenden Mixer")
{
    // Geraet 0 = kein Ton: dekodieren geht ohne Ausgabegeraet
    REQUIRE(BASS_Init(0, kLoopRate, 0, nullptr, nullptr) != 0);
    const HSTREAM mixer = BASS_Mixer_StreamCreate(kLoopRate, 2, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
    REQUIRE(mixer != 0);

    KlickQuelle q;
    const HSTREAM quelle = klickStream(q);
    REQUIRE_MESSAGE(quelle != 0, "BASS_StreamCreate mit Rueckruf: Fehler " << BASS_ErrorGetCode());
    REQUIRE_MESSAGE(BASS_Mixer_StreamAddChannel(mixer, quelle, BASS_MIXER_CHAN_NORAMPIN) != 0,
                    "die Quelle laesst sich nicht in den Mixer haengen: Fehler " << BASS_ErrorGetCode());

    SUBCASE("jeder Klick kommt auf den Abtastwert genau an, gleich wie gelesen wird")
    {
        // in ungleichen Stuecken lesen: die Lage der Klicks darf daran nicht haengen
        std::vector<float> alles;
        for (const int stueck : {1000, 4096, 777, 20000, 3})
        {
            const std::vector<float> teil = liesFrames(mixer, stueck);
            REQUIRE(teil.size() == static_cast<std::size_t>(stueck) * 2u);
            alles.insert(alles.end(), teil.begin(), teil.end());
        }
        int falsch = 0;
        int klicks = 0;
        std::int64_t ersterFalsch = -1;
        for (std::size_t i = 0; i + 1 < alles.size(); i += 2)
        {
            const auto n = static_cast<std::int64_t>(i / 2);
            const float soll = n % kSchritt == 0 ? 0.5f : 0.0f;
            if (soll != 0.0f) ++klicks;
            if (alles[i] != soll || alles[i + 1] != soll)
            {
                ++falsch;
                if (ersterFalsch < 0) ersterFalsch = n;
            }
        }
        MESSAGE("gelesen " << alles.size() / 2 << " Abtastwerte, " << klicks << " Klicks, falsch " << falsch
                           << " (erster bei " << ersterFalsch << "); Rueckruf " << q.rufe.load()
                           << " mal, Bloecke " << q.kleinsterBlock.load() << " bis " << q.groessterBlock.load()
                           << " Abtastwerte; die Quelle steht bei " << q.frame.load());
        CHECK(klicks == 5);
        CHECK(falsch == 0);
    }

    SUBCASE("zwei selbst rechnende Quellen addieren sich")
    {
        KlickQuelle grund;
        grund.klick = 0.25f;
        grund.grund = 0.25f;  // ueberall 0.25
        const HSTREAM zweite = klickStream(grund);
        REQUIRE(zweite != 0);
        REQUIRE(BASS_Mixer_StreamAddChannel(mixer, zweite, BASS_MIXER_CHAN_NORAMPIN) != 0);
        const std::vector<float> summe = liesFrames(mixer, 12000);
        REQUIRE(summe.size() == 12000u * 2u);
        int falsch = 0;
        for (std::size_t i = 0; i + 1 < summe.size(); i += 2)
        {
            const float soll = (i / 2) % kSchritt == 0 ? 0.75f : 0.25f;
            if (summe[i] != soll || summe[i + 1] != soll) ++falsch;
        }
        MESSAGE("Summe aus Klick und Grund: falsch " << falsch << " von 12000");
        CHECK(falsch == 0);
        BASS_StreamFree(zweite);
    }

    SUBCASE("die Stelle fuehrt die Quelle selbst — der Mixer holt nicht voraus")
    {
        (void)liesFrames(mixer, 3000);
        const std::int64_t voraus = q.frame.load() - 3000;
        // die Quelle springt: 100 Abtastwerte vor den zehnten Klick
        q.frame.store(10LL * kSchritt - 100);
        const std::vector<float> danach = liesFrames(mixer, 1000);
        REQUIRE(danach.size() == 1000u * 2u);
        int klickBei = -1;
        for (std::size_t i = 0; i + 1 < danach.size(); i += 2)
        {
            if (danach[i] != 0.0f)
            {
                klickBei = static_cast<int>(i / 2);
                break;
            }
        }
        MESSAGE("nach 3000 gelesenen Abtastwerten hat der Mixer " << voraus
                                                                  << " voraus geholt; nach dem Sprung liegt der Klick bei "
                                                                  << klickBei << " (soll 100)");
        CHECK(voraus == 0);
        CHECK(klickBei == 100);
    }

    SUBCASE("was BASSmix von der Stelle einer solchen Quelle weiss")
    {
        (void)liesFrames(mixer, 3000);
        const QWORD bytes = BASS_Mixer_ChannelGetPosition(quelle, BASS_POS_BYTE);
        const auto frames =
            bytes == static_cast<QWORD>(-1) ? -1 : static_cast<std::int64_t>(bytes / (2 * sizeof(float)));
        // setzen: BASS meldet Erfolg, die Quelle erfaehrt davon nichts — sie
        // rechnet weiter, wo sie stand. Wer die Stelle setzen will, setzt sie
        // an der Quelle selbst (Fall darueber).
        const BOOL gesetzt = BASS_Mixer_ChannelSetPosition(quelle, 0, BASS_POS_BYTE);
        const int fehler = gesetzt != 0 ? 0 : BASS_ErrorGetCode();
        const std::string ergebnis = gesetzt != 0 ? "meldet Erfolg" : "meldet Fehler";
        MESSAGE("BASS_Mixer_ChannelGetPosition nach 3000 Abtastwerten: " << frames
                << "; BASS_Mixer_ChannelSetPosition auf 0: " << ergebnis << " (Fehler " << fehler
                << "); die Quelle steht danach bei " << q.frame.load());
        CHECK(frames == 3000);
        CHECK(gesetzt != 0);
        CHECK(q.frame.load() == 3000);
    }

    BASS_StreamFree(mixer);
    BASS_StreamFree(quelle);
    BASS_Free();
}

// =============================================================================
// Teil 2 — die selbst rechnende Quelle im spielenden Mixer der Engine
// =============================================================================

TEST_CASE("Beleg Loop 2: die selbst rechnende Quelle im spielenden Mixer der BassEngine")
{
    LoopApp app;
    BassEngine engine;
    if (!engine.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << engine.getLastError().toStdString() << ")");
        return;
    }
    TonQuelle t;
    t.rate = engine.getSampleRate();
    const AudioStreamHandle mixer = engine.createMixer();
    REQUIRE(mixer != INVALID_STREAM);
    const HSTREAM quelle = BASS_StreamCreate(static_cast<DWORD>(t.rate), 2,
                                             BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT, &rechneTon, &t);
    REQUIRE_MESSAGE(quelle != 0, "BASS_StreamCreate mit Rueckruf: Fehler " << BASS_ErrorGetCode());
    REQUIRE(BASS_Mixer_StreamAddChannel(static_cast<HSTREAM>(mixer), quelle, 0) != 0);
    engine.setVolume(mixer, kLoopLeise);

    QElapsedTimer uhr;
    uhr.start();
    REQUIRE(engine.play(mixer));
    QThread::msleep(1000);
    const qint64 vergangenMs = uhr.elapsed();
    const std::int64_t gerechnet = t.frame.load();
    const double gerechnetMs = 1000.0 * static_cast<double>(gerechnet) / t.rate;
    const std::size_t hier = std::hash<std::thread::id>{}(std::this_thread::get_id());
    MESSAGE("Rate " << t.rate << " Hz; in " << vergangenMs << " ms gerechnet: " << gerechnet << " Abtastwerte = "
                    << std::to_string(gerechnetMs) << " ms Ton; Rueckruf " << t.rufe.load() << " mal, Bloecke "
                    << t.kleinsterBlock.load() << " bis " << t.groessterBlock.load() << " Abtastwerte ("
                    << std::to_string(1000.0 * t.kleinsterBlock.load() / t.rate) << " bis "
                    << std::to_string(1000.0 * t.groessterBlock.load() / t.rate) << " ms)");
    CHECK(engine.isPlaying(mixer));
    CHECK(t.rufe.load() > 0);
    // der Rueckruf laeuft nicht im Thread, der den Mixer gestartet hat
    CHECK(t.faden.load() != hier);
    // der Mixer holt, was er spielt — nicht wesentlich mehr, nicht wesentlich weniger
    CHECK(gerechnetMs > 0.6 * static_cast<double>(vergangenMs));
    CHECK(gerechnetMs < 1.5 * static_cast<double>(vergangenMs));

    float links = 0.0f, rechts = 0.0f;
    REQUIRE(engine.getChannelLevels(mixer, links, rechts));
    const float vorher = links;
    CHECK(vorher > 0.1f);

    // eine Aenderung von aussen (wie ein Dreh im Panel): wie lange bis ans Ohr?
    t.amp.store(0.0f);
    int verzugMs = -1;
    QElapsedTimer seit;
    seit.start();
    while (seit.elapsed() <= 800)
    {
        REQUIRE(engine.getChannelLevels(mixer, links, rechts));
        if (links < vorher / 20.0f)
        {
            verzugMs = static_cast<int>(seit.elapsed());
            break;
        }
        QThread::msleep(5);
    }
    MESSAGE("Pegel vorher " << std::to_string(vorher) << "; die Amplitude auf null kommt nach etwa " << verzugMs
                            << " ms im Pegel des Mixers an");
    CHECK(verzugMs >= 0);
    CHECK(verzugMs < 200);

    engine.mixerRemoveSource(static_cast<AudioStreamHandle>(quelle));
    engine.freeStream(mixer);
}
