/**
 * @file test_LoopBelegeAufnahme.cpp
 * @brief Loop, vor Stufe 11b: Belege zur Tonaufnahme mit BASS
 *        (docs/audio/Loop_Konzept.md §4h, „Record sample")
 *
 * Diese Tests bauen NICHTS am Loop. Sie halten fest, was BASS an dieser
 * Installation mit einem Eingang tut — bevor das Fenster „Record sample"
 * darauf gebaut wird:
 *
 * - Teil 1 (nimmt nichts auf): welche Eingaenge nennt BASS?
 * - Teil 2 (nimmt auf): laesst sich der Eingang oeffnen, waehrend die
 *   `BassEngine` ihr Ausgabegeraet haelt? Kommt der Ton in dem Format, das
 *   verlangt wurde (48000 Hz, Stereo, float)? In welchem Thread laeuft der
 *   Rueckruf, wie gross sind die Bloecke, kommt so viel an, wie die Uhr sagt?
 *
 * **Teil 2 nimmt rund eine Sekunde vom Standard-Eingang auf und laeuft
 * deshalb nur mit `LUMIVIZ_AUFNAHME_BELEG=1`.** Gespeichert wird nichts;
 * gemeldet werden Zahlen (Menge, Bloecke, groesster Betrag).
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
#include <string>
#include <thread>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <bass.h>

namespace
{

constexpr int kAufnahmeRate = 48000;

/// Was der Rueckruf der Aufnahme zaehlt.
struct Aufnahme
{
    std::atomic<std::int64_t> frames{0};
    std::atomic<int> rufe{0};
    std::atomic<int> kleinsterBlock{INT_MAX};
    std::atomic<int> groessterBlock{0};
    std::atomic<float> spitze{0.0f};
    std::atomic<bool> ungerade{false};      ///< ein Block war kein Vielfaches von Stereo-float
    std::atomic<std::size_t> faden{0};
};

BOOL CALLBACK nimmAuf(HRECORD /*handle*/, const void* buffer, DWORD length, void* user)
{
    auto* a = static_cast<Aufnahme*>(user);
    const auto* werte = static_cast<const float*>(buffer);
    if (length % (2 * sizeof(float)) != 0) a->ungerade.store(true);
    const int frames = static_cast<int>(length / (2 * sizeof(float)));
    float spitze = a->spitze.load();
    for (int i = 0; i < 2 * frames; ++i) spitze = std::max(spitze, std::abs(werte[i]));
    a->spitze.store(spitze);
    a->frames.fetch_add(frames);
    a->rufe.fetch_add(1);
    a->kleinsterBlock.store(std::min(a->kleinsterBlock.load(), frames));
    a->groessterBlock.store(std::max(a->groessterBlock.load(), frames));
    a->faden.store(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    return TRUE;  // weiter aufnehmen
}

}  // namespace

TEST_SUITE("Loop Belege Aufnahme")
{
    TEST_CASE("Beleg Loop Aufnahme 1: die Eingaenge, die BASS nennt")
    {
        int zahl = 0;
        int nutzbar = 0;
        std::string liste;
        BASS_DEVICEINFO info;
        for (DWORD geraet = 0; BASS_RecordGetDeviceInfo(geraet, &info) != 0; ++geraet)
        {
            ++zahl;
            const bool an = (info.flags & BASS_DEVICE_ENABLED) != 0;
            if (an) ++nutzbar;
            liste += "\n  " + std::to_string(geraet) + ": " + (info.name != nullptr ? info.name : "?") +
                     (an ? "" : " [aus]") + ((info.flags & BASS_DEVICE_DEFAULT) != 0 ? " [Standard]" : "") +
                     ((info.flags & BASS_DEVICE_LOOPBACK) != 0 ? " [Mitschnitt der Ausgabe]" : "");
        }
        MESSAGE("BASS nennt " << zahl << " Eingaenge, davon nutzbar " << nutzbar << ":" << liste);
        CHECK(zahl >= 0);  // ein Rechner ohne Eingang ist kein Fehler
    }

    TEST_CASE("Beleg Loop Aufnahme 2: aufnehmen neben der laufenden BassEngine — Format, Bloecke, Menge")
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

        // die Engine haelt ihr Ausgabegeraet, wie im Programm
        BassEngine engine;
        const bool mitAusgabe = engine.initialize();
        if (!mitAusgabe) MESSAGE("kein Ausgabegeraet (" << engine.getLastError().toStdString() << ") — die Aufnahme wird trotzdem versucht");

        if (BASS_RecordInit(-1) == 0)
        {
            MESSAGE("BASS_RecordInit(-1): Fehler " << BASS_ErrorGetCode() << " — kein Eingang, oder Windows gibt das Mikrofon nicht frei; nichts geprueft");
            return;
        }
        BASS_DEVICEINFO info;
        const DWORD geraet = BASS_RecordGetDevice();
        const bool infoDa = BASS_RecordGetDeviceInfo(geraet, &info) != 0;

        Aufnahme a;
        QElapsedTimer uhr;
        uhr.start();
        const HRECORD kanal = BASS_RecordStart(static_cast<DWORD>(kAufnahmeRate), 2, BASS_SAMPLE_FLOAT, &nimmAuf, &a);
        REQUIRE_MESSAGE(kanal != 0, "BASS_RecordStart: Fehler " << BASS_ErrorGetCode());
        QThread::msleep(1000);
        // vor dem Stop fragen: der Stop gibt einen Aufnahme-Kanal frei (erster Lauf: danach „keine Auskunft")
        BASS_CHANNELINFO form;
        const bool formDa = BASS_ChannelGetInfo(kanal, &form) != 0;
        const bool gestoppt = BASS_ChannelStop(kanal) != 0;
        const qint64 vergangenMs = uhr.elapsed();
        const std::int64_t frames = a.frames.load();
        const double aufgenommenMs = 1000.0 * static_cast<double>(frames) / kAufnahmeRate;
        const std::size_t hier = std::hash<std::thread::id>{}(std::this_thread::get_id());

        MESSAGE("Eingang " << geraet << (infoDa && info.name != nullptr ? std::string(" (") + info.name + ")" : std::string())
                           << std::string(mitAusgabe ? ", neben der laufenden BassEngine" : ", ohne Ausgabegeraet") << "; in " << vergangenMs
                           << " ms aufgenommen: " << frames << " Abtastwerte = " << std::to_string(aufgenommenMs)
                           << " ms; Rueckruf " << a.rufe.load() << " mal, Bloecke " << a.kleinsterBlock.load() << " bis "
                           << a.groessterBlock.load() << " Abtastwerte; groesster Betrag " << std::to_string(a.spitze.load())
                           << (formDa ? "; Kanal: " + std::to_string(form.freq) + " Hz, " + std::to_string(form.chans) +
                                            " Kanaele, float " + ((form.flags & BASS_SAMPLE_FLOAT) != 0 ? "ja" : "nein")
                                      : std::string("; Kanal: keine Auskunft")));
        CHECK(gestoppt);
        CHECK(a.rufe.load() > 0);
        CHECK_FALSE(a.ungerade.load());
        // der Rueckruf laeuft nicht im Thread, der die Aufnahme gestartet hat
        CHECK(a.faden.load() != hier);
        // es kommt ungefaehr so viel an, wie die Uhr sagt
        CHECK(aufgenommenMs > 0.6 * static_cast<double>(vergangenMs));
        CHECK(aufgenommenMs < 1.2 * static_cast<double>(vergangenMs));
        if (formDa)
        {
            CHECK(form.freq == static_cast<DWORD>(kAufnahmeRate));
            CHECK(form.chans == 2);
            CHECK((form.flags & BASS_SAMPLE_FLOAT) != 0);
        }
        // nach dem Stop kommt nichts mehr
        const std::int64_t danach = a.frames.load();
        QThread::msleep(150);
        CHECK(a.frames.load() == danach);
        // die Ausgabe der Engine hat die Aufnahme nicht gestoert
        if (mitAusgabe) CHECK(engine.isInitialized());
        CHECK(BASS_RecordFree() != 0);
    }
}
