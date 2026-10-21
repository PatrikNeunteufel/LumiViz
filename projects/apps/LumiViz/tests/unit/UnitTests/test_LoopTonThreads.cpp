/**
 ****************************************************************************************
 * @file   test_LoopTonThreads.cpp
 * @brief  Loop: Toene auf mehreren Threads gleichzeitig gerechnet, Bit fuer Bit
 *         wie allein (docs/audio/Spielweisen_Konzept.md §4.5, Stufe 29a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Audio-Engine. Die Toene sind die, die das
 * Programm rechnen wuerde: alle Tonnamen der mitgelieferten Loops (geplant mit
 * der Auskunft ueber die Saiten, wie im Programm) und je mitgeliefertem
 * Instrument eine Probe. Zuerst rechnet ein Thread jeden Ton allein und haelt
 * eine Pruefsumme fest; dann holen mehrere Threads die Toene aus einer
 * gemeinsamen Liste, jeder holt sein Instrument selbst (`holeInstrument`) —
 * nach `vergissTonformen`, damit auch die Formen der Shape-Schichten auf
 * mehreren Threads zugleich entstehen. Jede Pruefsumme muss gleich sein.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "shape/Tonform.hpp"

#include <QCoreApplication>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <thread>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct ThreadApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

/// Zahl der Abtastwerte und FNV-1a ueber ihre Bytes.
struct Abdruck
{
    std::int64_t frames = -1;
    std::uint64_t pruefsumme = 0;

    bool operator==(const Abdruck&) const = default;
};

Abdruck abdruck(const Klang& klang)
{
    Abdruck a;
    a.frames = klang.frames();
    std::uint64_t summe = 1469598103934665603ull;
    for (const float wert : klang.daten)
    {
        unsigned char bytes[sizeof(float)];
        std::memcpy(bytes, &wert, sizeof(float));
        for (const unsigned char byte : bytes)
        {
            summe ^= byte;
            summe *= 1099511628211ull;
        }
    }
    a.pruefsumme = summe;
    return a;
}

/// Ein Ton wie im Programm: das Instrument holen, dann rechnen. Nicht gefunden = −1 Frames.
Abdruck rechne(const TonAngabe& ton)
{
    Instrument instrument;
    if (!holeInstrument(ton.instrument, instrument)) return {};
    return abdruck(rendereTon(instrument, ton, kRate));
}

/// Alle Toene: die der mitgelieferten Loops und je Instrument eine Probe (C4, 400 ms).
std::vector<TonAngabe> alleToene()
{
    std::vector<TonAngabe> toene;
    QStringList gesehen;
    const SaitenVon saiten = saitenAusInstrumenten();
    for (const QString& pfad : looptest::mitgelieferteLoops())
    {
        Loop loop;
        if (!ladeLoop(pfad, loop)) continue;
        loop.richte();
        for (const QString& name : loop.klangNamen(saiten))
        {
            TonAngabe ton;
            if (gesehen.contains(name) || !zerlegeTon(name, ton)) continue;
            gesehen << name;
            toene.push_back(std::move(ton));
        }
    }
    for (const Instrument& instrument : instrumente())
    {
        TonAngabe ton;
        ton.instrument = instrument.name;
        ton.hoehe = 60;
        ton.dauerMs = 400;
        toene.push_back(std::move(ton));
    }
    return toene;
}

}  // namespace

TEST_CASE("Loop Ton Threads 29a: auf mehreren Threads wie allein")
{
    ThreadApp app;
    const std::vector<TonAngabe> toene = alleToene();
    REQUIRE(toene.size() > instrumente().size());  // die Loops tragen Toene bei
    for (const Instrument& instrument : instrumente()) looptest::merkeSamplesOhneEngine(instrument);

    std::vector<Abdruck> allein;
    allein.reserve(toene.size());
    for (const TonAngabe& ton : toene) allein.push_back(rechne(ton));
    const auto gerechnet = std::count_if(allein.begin(), allein.end(), [](const Abdruck& a) { return a.frames > 0; });
    REQUIRE(gerechnet == static_cast<std::ptrdiff_t>(toene.size()));  // jedes Instrument gefunden

    lumi::shape::vergissTonformen();  // die Formen entstehen gleich neu, auf mehreren Threads
    const unsigned zahl = std::clamp(std::thread::hardware_concurrency(), 4u, 8u);
    std::vector<Abdruck> zusammen(toene.size());
    std::atomic<std::size_t> naechster{0};
    std::vector<std::thread> threads;
    for (unsigned t = 0; t < zahl; ++t)
        threads.emplace_back([&] {
            for (std::size_t i = naechster++; i < toene.size(); i = naechster++) zusammen[i] = rechne(toene[i]);
        });
    for (std::thread& thread : threads) thread.join();

    int anders = 0;
    for (std::size_t i = 0; i < toene.size(); ++i)
        if (!(zusammen[i] == allein[i]))
        {
            ++anders;
            MESSAGE("anders: " << toene[i].instrument.toStdString() << " " << toene[i].hoehe << " " << toene[i].dauerMs << " ms");
        }
    MESSAGE(toene.size() << " Toene auf " << zahl << " Threads");
    CHECK(anders == 0);
}
