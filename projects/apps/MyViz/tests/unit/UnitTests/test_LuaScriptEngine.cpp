/**
 ****************************************************************************************
 * @file   test_LuaScriptEngine.cpp
 * @brief  Unit-Tests fuer die sandboxed Lua-Engine (Import-Phase Roadmap 1):
 *         Sandbox-Whitelist, EEL-Prelude-Golden-Tests (Import-Analyse §7.2),
 *         Slot-Modell (Init/Beat/Frame/Point), Superscope-Lua-Modus und eine
 *         Performance-Messung (1000 Punkte, Ziel laut Analyse: << 1 Frame-Budget)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "scripting/LuaScriptEngine.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <vector>

using lumi::scripting::LuaScriptEngine;
using Slot = LuaScriptEngine::Slot;

namespace
{

double evalOrFail(LuaScriptEngine& lua, const char* expr)
{
    double out = 0.0;
    const bool ok = lua.evalNumber(expr, out);
    CAPTURE(expr);
    CAPTURE(lua.lastError());
    REQUIRE(ok);
    return out;
}

} // namespace

// =============================================================================
// Audio analysis (getspec/getosc/gettime, AVS-faithful)
// =============================================================================

TEST_CASE("LuaEngine: Audio — getspec/getosc/gettime lesen die VisData (AVS-treu)")
{
    LuaScriptEngine lua;

    // Ohne VisData: alles 0.
    CHECK(evalOrFail(lua, "getspec(0.5, 0.05, 0)") == doctest::Approx(0.0));
    CHECK(evalOrFail(lua, "getosc(0.5, 0.05, 0)") == doctest::Approx(0.0));

    std::array<unsigned char, 576 * 4> vis{};
    for (int i = 0; i < 576; ++i)
    {
        vis[static_cast<size_t>(i)] = 255;          // spectrum L = voll
        vis[static_cast<size_t>(i) + 576] = 255;    // spectrum R = voll
        vis[static_cast<size_t>(i) + 1152] = 127;   // waveform L = max (+127)
        vis[static_cast<size_t>(i) + 1728] = 127;   // waveform R = max
    }
    lua.setVisData(vis.data());

    // getspec center: (255+255)/255 * 0.5 = 1.0
    CHECK(evalOrFail(lua, "getspec(0.5, 0.05, 0)") == doctest::Approx(1.0));
    // getspec L: 255/127.5 * 0.5 = 1.0
    CHECK(evalOrFail(lua, "getspec(0.5, 0.05, 1)") == doctest::Approx(1.0));
    // getosc center: (127+127)/255 = 254/255 ; getosc hat kein *0.5
    CHECK(evalOrFail(lua, "getosc(0.5, 0.05, 0)") == doctest::Approx(254.0 / 255.0));
    // getosc L: 127/127.5
    CHECK(evalOrFail(lua, "getosc(0.5, 0.05, 1)") == doctest::Approx(127.0 / 127.5));

    // gettime(sc) = scriptTime - sc.
    lua.setScriptTime(12.5);
    CHECK(evalOrFail(lua, "gettime(0)") == doctest::Approx(12.5));
    CHECK(evalOrFail(lua, "gettime(2.5)") == doctest::Approx(10.0));
}

// =============================================================================
// Sandbox
// =============================================================================

TEST_CASE("LuaEngine: Sandbox — Whitelist, keine Stdlib, unbekannte Variablen = 0")
{
    LuaScriptEngine lua;

    // Unbekannte Variablen lesen 0.0 (EEL-Semantik via __index)
    CHECK(evalOrFail(lua, "nosuchvar") == doctest::Approx(0.0));

    // io/os/load sind im Environment nicht vorhanden (lesen als 0.0, nicht als Tabelle)
    CHECK(evalOrFail(lua, "io") == doctest::Approx(0.0));
    CHECK(evalOrFail(lua, "os") == doctest::Approx(0.0));
    CHECK(evalOrFail(lua, "load") == doctest::Approx(0.0));

    // Math-Teilmenge ist unqualifiziert verfuegbar
    CHECK(evalOrFail(lua, "sin(pi/2)") == doctest::Approx(1.0));
    CHECK(evalOrFail(lua, "atan2(1, 1)") == doctest::Approx(3.14159265 / 4.0));
    CHECK(evalOrFail(lua, "mod(5.5, 2)") == doctest::Approx(1.5));  // mod = fmod (nativ)
    CHECK(evalOrFail(lua, "pi2") == doctest::Approx(6.28318530717958647692));

    // Variablen-Roundtrip ueber die C++-Schnittstelle
    lua.setNumber("answer", 42.0);
    CHECK(lua.number("answer") == doctest::Approx(42.0));
    CHECK(lua.number("neverSet") == doctest::Approx(0.0));
}

// =============================================================================
// EEL-Prelude — Golden-Tests (Semantik-Vertrag Import-Analyse §7.2)
// =============================================================================

TEST_CASE("LuaEngine: eel-Prelude — Integer-Modulo (nseel_asm_mod, S48)")
{
    LuaScriptEngine lua;
    CHECK(evalOrFail(lua, "eel.mod(7, 3)") == doctest::Approx(1.0));
    // UNSIGNED Rest: int32(-7) wickelt ueber 2^32 -> 4294967289 % 3 = 0
    // (AvsRef-Probe S48; die S44-|Rest|-Annahme war falsch — der Neon
    // Coaster haengt mit mf=getosc(..)*200%4 an genau dieser Semantik).
    CHECK(evalOrFail(lua, "eel.mod(-7, 3)") == doctest::Approx(0.0));
    CHECK(evalOrFail(lua, "eel.mod(5, 0)") == doctest::Approx(0.0));    // Divisor -> max(,1)
    CHECK(evalOrFail(lua, "eel.mod(7.4, 3.2)") == doctest::Approx(1.0)); // round: 7 % 3
}

TEST_CASE("LuaEngine: eel-Prelude — Epsilon-Logik (1e-5)")
{
    LuaScriptEngine lua;
    CHECK(evalOrFail(lua, "eel.equal(1, 1.000001)") == doctest::Approx(1.0));
    CHECK(evalOrFail(lua, "eel.equal(1, 1.00002)") == doctest::Approx(0.0));
    CHECK(evalOrFail(lua, "eel.band(0.000001, 5)") == doctest::Approx(0.0)); // |x|<=eps unwahr
    CHECK(evalOrFail(lua, "eel.band(1, 2)") == doctest::Approx(1.0));
    CHECK(evalOrFail(lua, "eel.bor(0, 0.5)") == doctest::Approx(1.0));
    CHECK(evalOrFail(lua, "eel.bnot(0.000005)") == doctest::Approx(1.0));
    CHECK(evalOrFail(lua, "eel.above(2, 1)") == doctest::Approx(1.0));
    CHECK(evalOrFail(lua, "eel.below(2, 1)") == doctest::Approx(0.0));
}

TEST_CASE("LuaEngine: eel-Prelude — sqrt/sign/sigmoid/toint/Bit-Ops")
{
    LuaScriptEngine lua;
    CHECK(evalOrFail(lua, "eel.sqrt(-4)") == doctest::Approx(2.0));  // sqrt(|x|), nie NaN
    CHECK(evalOrFail(lua, "eel.sign(-3.2)") == doctest::Approx(-1.0));
    CHECK(evalOrFail(lua, "eel.sign(0)") == doctest::Approx(0.0));
    CHECK(evalOrFail(lua, "eel.sigmoid(0, 1)") == doctest::Approx(0.5));
    CHECK(evalOrFail(lua, "eel.toint(2.6)") == doctest::Approx(3.0));
    CHECK(evalOrFail(lua, "eel.bitand(6.4, 3)") == doctest::Approx(2.0)); // round(6.4)=6, 6&3
    CHECK(evalOrFail(lua, "eel.bitor(5, 2)") == doctest::Approx(7.0));
}

TEST_CASE("LuaEngine: eel-Prelude — rand ist Integer, deterministisch seedbar")
{
    LuaScriptEngine luaA;
    LuaScriptEngine luaB;
    luaA.seedRandom(42);
    luaB.seedRandom(42);

    // Gleicher Seed -> gleiche Sequenz (eigener PRNG, kein math.random)
    for (int k = 0; k < 5; ++k)
    {
        CHECK(evalOrFail(luaA, "rand(1000)") == doctest::Approx(evalOrFail(luaB, "rand(1000)")));
    }

    // Wertebereich 0..x-1, ganzzahlig (Entscheid Import-Analyse §10.1)
    for (int k = 0; k < 100; ++k)
    {
        const double r = evalOrFail(luaA, "rand(10)");
        CHECK(r >= 0.0);
        CHECK(r <= 9.0);
        CHECK(r == doctest::Approx(std::floor(r)));
    }
}

TEST_CASE("LuaEngine: eel-Prelude — megabuf engine-lokal, Index-Regeln")
{
    LuaScriptEngine lua;
    CHECK(evalOrFail(lua, "eel.mbwrite(5, 3.5)") == doctest::Approx(3.5)); // assign-Semantik
    CHECK(evalOrFail(lua, "eel.mbread(5)") == doctest::Approx(3.5));
    CHECK(evalOrFail(lua, "eel.mbread(99)") == doctest::Approx(0.0));      // Default 0
    CHECK(evalOrFail(lua, "eel.mbread(5.00005)") == doctest::Approx(3.5)); // floor(i+1e-4)
    evalOrFail(lua, "eel.mbwrite(-1, 7)");                                  // out-of-range: No-op
    CHECK(evalOrFail(lua, "eel.mbread(-1)") == doctest::Approx(0.0));

    // Engine-lokal (= preset-lokal, Entscheid §10.3): zweite Engine sieht nichts
    LuaScriptEngine other;
    CHECK(evalOrFail(other, "eel.mbread(5)") == doctest::Approx(0.0));
}

TEST_CASE("LuaEngine: app-globales Atomic-Register-Set (Entscheid §10.3)")
{
    LuaScriptEngine luaA;
    LuaScriptEngine luaB;
    CHECK(evalOrFail(luaA, "app.gset(3, 1.25)") == doctest::Approx(1.25));
    CHECK(evalOrFail(luaB, "app.gget(3)") == doctest::Approx(1.25));  // prozessweit geteilt
    CHECK(evalOrFail(luaB, "app.gget(99)") == doctest::Approx(0.0));  // out-of-range -> 0
    evalOrFail(luaA, "app.gset(3, 0)");  // aufraeumen (statisch — Test-Isolation)
}

// =============================================================================
// Slot-Modell
// =============================================================================

TEST_CASE("LuaEngine: Slots — Init einmal, Frame kumuliert, Env geteilt")
{
    LuaScriptEngine lua;
    REQUIRE(lua.compile(Slot::Init, "counter = 10", "t.init"));
    REQUIRE(lua.compile(Slot::Frame, "counter = counter + 1", "t.frame"));
    CHECK(lua.has(Slot::Init));
    CHECK_FALSE(lua.has(Slot::Point));

    REQUIRE(lua.run(Slot::Init));
    REQUIRE(lua.run(Slot::Frame));
    REQUIRE(lua.run(Slot::Frame));
    REQUIRE(lua.run(Slot::Frame));
    CHECK(lua.number("counter") == doctest::Approx(13.0));

    // Leere Quelle raeumt den Slot (kein Fehler)
    REQUIRE(lua.compile(Slot::Frame, "   \n", "t.frame"));
    CHECK_FALSE(lua.has(Slot::Frame));
}

TEST_CASE("LuaEngine: Fehlerbehandlung — Compile-Fehler und Laufzeit-Abschaltung")
{
    LuaScriptEngine lua;

    // Syntaxfehler: Slot bleibt leer, lastError gesetzt
    CHECK_FALSE(lua.compile(Slot::Point, "x = = 2", "t.point"));
    CHECK_FALSE(lua.has(Slot::Point));
    CHECK_FALSE(lua.lastError().empty());

    // Laufzeitfehler: Slot deaktiviert sich (kein Fehler-Spam im Render-Loop).
    // error() steht in der Sandbox bewusst NICHT zur Verfuegung — der Aufruf
    // einer unbekannten Funktion (= 0.0 via __index) ist selbst der Fehler.
    lua.clearError();
    REQUIRE(lua.compile(Slot::Point, "boom()", "t.point"));
    CHECK_FALSE(lua.run(Slot::Point));
    CHECK_FALSE(lua.has(Slot::Point));
    CHECK(lua.lastError().find("boom") != std::string::npos);
}

// =============================================================================
// Superscope-Lua-Modus (Keimzelle)
// =============================================================================

namespace
{

/// execute()-Aufruf mit synthetischem Audio (konstante Waveform)
std::vector<lumi::modules::SuperscopePoint> runScope(lumi::modules::SuperscopeModule& scope,
                                                     float audioValue = 0.25f,
                                                     bool isBeat = false)
{
    std::vector<float> wave(64, audioValue);
    return scope.execute(wave.data(), wave.data(), wave.data(), wave.data(),
                         static_cast<int>(wave.size()), 800, 600, isBeat, 1.0f / 60.0f);
}

} // namespace

TEST_CASE("Superscope-Lua: Point-Skript liefert Geometrie, Frame-Skript setzt n")
{
    lumi::modules::SuperscopeModule scope;
    scope.setPointCount(32);
    scope.setAspectCorrection(false);  // sonst skaliert w/h=800/600 das x
    scope.setFrameCode("n = 16");
    scope.setPointCode("x = i*2 - 1; y = 0.5");
    scope.setLuaMode(true);

    const auto points = runScope(scope);
    CAPTURE(scope.lastScriptError());
    REQUIRE(points.size() == 16);  // Frame-Skript hat n überschrieben
    CHECK(points.front().x == doctest::Approx(-1.0f));
    CHECK(points.back().x == doctest::Approx(1.0f).epsilon(0.01));
    for (const auto& pt : points)
    {
        // S46 Befund A: Skript-y lebt im AVS-Raum (y+ = unten) — der Modul-
        // rand uebersetzt nach GL (y+ = oben): Skript 0.5 -> Punkt -0.5
        CHECK(pt.y == doctest::Approx(-0.5f).epsilon(0.01));
    }
    CHECK(scope.lastScriptError().empty());
}

TEST_CASE("Superscope-Lua: Skript-Farben überschreiben den Gradient")
{
    lumi::modules::SuperscopeModule scope;
    scope.setPointCount(8);
    scope.setPointCode("x = i; y = 0; red = 1; green = 0; blue = 0");
    scope.setLuaMode(true);

    const auto points = runScope(scope);
    CAPTURE(scope.lastScriptError());
    REQUIRE_FALSE(points.empty());
    CHECK(points.front().r == doctest::Approx(1.0f));
    CHECK(points.front().g == doctest::Approx(0.0f));
    CHECK(points.front().b == doctest::Approx(0.0f));
}

TEST_CASE("Superscope-Lua: v transportiert Audio, Variablen persistieren über Frames")
{
    lumi::modules::SuperscopeModule scope;
    scope.setPointCount(8);
    scope.setInitCode("frames = 0");
    scope.setFrameCode("frames = frames + 1");
    scope.setPointCode("x = 0; y = v");
    scope.setLuaMode(true);

    runScope(scope, 0.75f);
    const auto points = runScope(scope, 0.75f);
    CAPTURE(scope.lastScriptError());
    REQUIRE_FALSE(points.empty());
    // S46 Befund A: y=v landet via AVS->GL-Rand bei -v (Skript-y+ = unten)
    CHECK(points.front().y == doctest::Approx(-0.75f).epsilon(0.01));
    CHECK(scope.getVariable("frames") == doctest::Approx(2.0));
}

TEST_CASE("Superscope-Lua: ohne Point-Skript fällt der Preset-Pfad zurück")
{
    lumi::modules::SuperscopeModule scope;
    scope.setPreset(lumi::modules::SuperscopePreset::HorizontalScope);
    scope.setLuaMode(true);  // Lua an, aber Preset-Code kommt aus loadPresetCode

    const auto points = runScope(scope, 0.5f);
    REQUIRE_FALSE(points.empty());
    // Kein toter Bildschirm: Punkte folgen irgendeiner Geometrie (nicht alle 0)
    bool anyNonZero = false;
    for (const auto& pt : points)
    {
        if (std::fabs(pt.x) > 0.001f || std::fabs(pt.y) > 0.001f) anyNonZero = true;
    }
    CHECK(anyNonZero);
}

TEST_CASE("Superscope-Lua: Laufzeitfehler im Point-Skript -> Fallback statt Absturz")
{
    lumi::modules::SuperscopeModule scope;
    scope.setPointCount(8);
    scope.setPointCode("kaputt()");  // unbekannte Funktion -> Laufzeitfehler
    scope.setLuaMode(true);

    const auto points = runScope(scope);
    REQUIRE_FALSE(points.empty());  // Fallback-Geometrie
    CHECK(scope.lastScriptError().find("kaputt") != std::string::npos);
}

TEST_CASE("Superscope-Lua: Builtin-Preset laeuft als EEL-Skript, t gehoert dem Skript")
{
    // Regression (Session 32): der Host darf `t` nie ueberschreiben — die
    // Preset-Strings akkumulieren es selbst (Spiral: Frame "t=t+0.015").
    lumi::modules::SuperscopeModule scope;
    scope.setPreset(lumi::modules::SuperscopePreset::Spiral);
    scope.setLuaMode(true);

    runScope(scope);
    runScope(scope);
    CAPTURE(scope.lastScriptError());
    CHECK(scope.lastScriptError().empty());
    // 2 Frames x 0.015 — waere t von der Host-Zeit geklobbert, stuende hier ~2.0
    CHECK(scope.getVariable("t") == doctest::Approx(0.03));
}

TEST_CASE("Superscope-Lua: Preset-Variablen kollidieren nicht mit Host-Namen")
{
    // Regression (Session 32b): Lissajous nutzte `b` (= Host-Beat-Variable,
    // wird jeden Frame geklobbert) und Hypocycloid `R`+`r` (EEL/Lua sind
    // case-insensitiv -> dieselbe Variable, R-r=0 -> Punktkollaps).

    // Lissajous: y muss ueber i variieren (fb-Frequenz), nicht konstant sein
    {
        lumi::modules::SuperscopeModule scope;
        scope.setPreset(lumi::modules::SuperscopePreset::Lissajous);
        scope.setLuaMode(true);
        const auto points = runScope(scope, 0.0f);  // v=0 -> reine Kurvenform
        CAPTURE(scope.lastScriptError());
        REQUIRE_FALSE(points.empty());
        float minY = points.front().y;
        float maxY = points.front().y;
        for (const auto& pt : points)
        {
            minY = std::min(minY, pt.y);
            maxY = std::max(maxY, pt.y);
        }
        CHECK(maxY - minY > 0.5f);  // cos-Spanne ±0.8 — vor dem Fix ~0
        CHECK(scope.lastScriptError().empty());
    }

    // Hypocycloid: Punkte duerfen nicht alle auf einem Fleck liegen
    {
        lumi::modules::SuperscopeModule scope;
        scope.setPreset(lumi::modules::SuperscopePreset::Hypocycloid);
        scope.setLuaMode(true);
        const auto points = runScope(scope, 0.0f);
        CAPTURE(scope.lastScriptError());
        REQUIRE_FALSE(points.empty());
        float minX = points.front().x;
        float maxX = points.front().x;
        for (const auto& pt : points)
        {
            minX = std::min(minX, pt.x);
            maxX = std::max(maxX, pt.x);
        }
        CHECK(maxX - minX > 0.1f);  // vor dem Fix: ein einziger Punkt
        CHECK(scope.lastScriptError().empty());
    }
}

TEST_CASE("Superscope-Lua: DNA hat leere Slots und faellt auf C++ zurueck")
{
    lumi::modules::SuperscopeModule scope;
    scope.setPreset(lumi::modules::SuperscopePreset::DNA);
    scope.setLuaMode(true);

    const auto points = runScope(scope, 0.5f);
    CHECK(scope.lastScriptError().empty());
    REQUIRE_FALSE(points.empty());  // native DNA-Geometrie rendert
}

TEST_CASE("Superscope-Lua: Param-Schnittstelle script.lua schaltet den Modus")
{
    lumi::modules::SuperscopeModule scope;
    lumi::modules::ParamValue value;

    REQUIRE(scope.getParam("script.lua", value));
    CHECK(std::get<bool>(value) == false);

    REQUIRE(scope.setParam("script.lua", true));
    REQUIRE(scope.getParam("script.lua", value));
    CHECK(std::get<bool>(value) == true);
    CHECK(scope.luaMode());
}

// =============================================================================
// Performance (Analyse-Versprechen: 1000 Punkte × 60 fps << Frame-Budget)
// =============================================================================

TEST_CASE("Superscope-Lua: Performance — 1000 Punkte Spiral-Skript")
{
    lumi::modules::SuperscopeModule scope;
    scope.setPointCount(1000);
    scope.setPointCode(
        "r = 0.1 + i*0.7 + v*0.15; a = i*pi*8 + t*0.015; x = sin(a)*r; y = cos(a)*r");
    scope.setLuaMode(true);

    runScope(scope);  // Warmup inkl. Compile/Init

    constexpr int kFrames = 300;
    const auto start = std::chrono::steady_clock::now();
    for (int f = 0; f < kFrames; ++f)
    {
        runScope(scope);
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const double msPerFrame =
        std::chrono::duration<double, std::milli>(elapsed).count() / kFrames;

    CAPTURE(scope.lastScriptError());
    MESSAGE("Superscope-Lua: 1000 Punkte -> " << msPerFrame << " ms/Frame");
    // 60-fps-Budget ist 16.7 ms; die Analyse erwartet <1 ms — 5 ms als robuste Schranke
    CHECK(msPerFrame < 5.0);
}
