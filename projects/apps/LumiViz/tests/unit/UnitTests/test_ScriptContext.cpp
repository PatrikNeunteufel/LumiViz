/**
 ****************************************************************************************
 * @file   test_ScriptContext.cpp
 * @brief  Tests fuer den geteilten Skript-Kontext (Import-Phase Roadmap 4.1):
 *         gmegabuf-Teilung ueber Engines, reg/q-Sync ueber den ScriptSlotHost,
 *         q-Snapshot-Semantik (MilkDrop-Modell), Fehlerpfade des SlotHost
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "scripting/LuaScriptEngine.hpp"
#include "scripting/ScriptContext.hpp"
#include "scripting/ScriptSlotHost.hpp"

#include <memory>

using lumi::scripting::LuaScriptEngine;
using lumi::scripting::ScriptContext;
using lumi::scripting::ScriptSlotHost;
using Slot = ScriptSlotHost::Slot;

// =============================================================================
// ScriptContext pur
// =============================================================================

TEST_CASE("ScriptContext: reg/q Grenzen und Defaults")
{
    ScriptContext ctx;
    CHECK(ctx.reg(0) == 0.0);
    CHECK(ctx.reg(99) == 0.0);
    ctx.setReg(7, 1.5);
    CHECK(ctx.reg(7) == 1.5);
    ctx.setReg(-1, 9.0);   // out of range: ignoriert
    ctx.setReg(100, 9.0);
    CHECK(ctx.reg(-1) == 0.0);
    CHECK(ctx.reg(100) == 0.0);

    ctx.setQ(1, 2.0);
    ctx.setQ(64, 3.0);
    CHECK(ctx.q(1) == 2.0);
    CHECK(ctx.q(64) == 3.0);
    ctx.setQ(0, 9.0);      // q ist 1-basiert
    ctx.setQ(65, 9.0);
    CHECK(ctx.q(0) == 0.0);
    CHECK(ctx.q(65) == 0.0);
}

TEST_CASE("ScriptContext: q-Snapshot-Semantik (MilkDrop-Modell)")
{
    ScriptContext ctx;

    // "Init" schreibt q1=5 -> Snapshot einfrieren
    ctx.setQ(1, 5.0);
    ctx.captureInitSnapshot();

    // "Frame" veraendert q1 -> Frame-Snapshot
    ctx.setQ(1, 7.0);
    ctx.captureFrameSnapshot();

    // "Point"-Laeufe duerfen q veraendern ...
    ctx.setQ(1, 100.0);
    // ... Restore auf den Frame-Stand holt sie zurueck
    ctx.restoreFrameSnapshot();
    CHECK(ctx.q(1) == 7.0);

    // Frame-Beginn: zurueck auf den Post-Init-Stand
    ctx.restoreInitSnapshot();
    CHECK(ctx.q(1) == 5.0);
}

// =============================================================================
// gmegabuf-Teilung ueber Engines
// =============================================================================

TEST_CASE("ScriptContext: zwei Engines teilen gmegabuf ueber den Kontext")
{
    auto ctx = std::make_shared<ScriptContext>();
    LuaScriptEngine writer(ctx);
    LuaScriptEngine reader(ctx);

    REQUIRE(writer.compile(LuaScriptEngine::Slot::Frame,
                           "eel.gmbwrite(3, 42.5)", "ctx.writer"));
    REQUIRE(writer.run(LuaScriptEngine::Slot::Frame));

    double out = 0.0;
    REQUIRE(reader.evalNumber("eel.gmbread(3)", out));
    CHECK(out == 42.5);

    // megabuf bleibt engine-lokal
    REQUIRE(writer.compile(LuaScriptEngine::Slot::Frame,
                           "eel.mbwrite(3, 7.0)", "ctx.writer"));
    REQUIRE(writer.run(LuaScriptEngine::Slot::Frame));
    REQUIRE(reader.evalNumber("eel.mbread(3)", out));
    CHECK(out == 0.0);
}

TEST_CASE("ScriptContext: ohne expliziten Kontext bleiben Engines isoliert")
{
    LuaScriptEngine a;
    LuaScriptEngine b;

    REQUIRE(a.compile(LuaScriptEngine::Slot::Frame, "eel.gmbwrite(0, 1.0)", "iso.a"));
    REQUIRE(a.run(LuaScriptEngine::Slot::Frame));

    double out = 1.0;
    REQUIRE(b.evalNumber("eel.gmbread(0)", out));
    CHECK(out == 0.0);   // bisheriges Verhalten: private Kontexte
}

// =============================================================================
// ScriptSlotHost: EEL-Quartett + reg/q-Sync
// =============================================================================

TEST_CASE("ScriptSlotHost: EEL kompiliert und laeuft, Ergebnis in der Env")
{
    ScriptSlotHost host("test");
    host.setSource(Slot::Frame, "x=1+2; y=x*2");
    REQUIRE(host.compileAll());
    REQUIRE(host.run(Slot::Frame));
    CHECK(host.engine().number("x") == doctest::Approx(3.0));
    CHECK(host.engine().number("y") == doctest::Approx(6.0));
}

TEST_CASE("ScriptSlotHost: reg-Variablen wandern ueber den geteilten Kontext")
{
    auto ctx = std::make_shared<ScriptContext>();
    ScriptSlotHost producer("producer", ctx);
    ScriptSlotHost consumer("consumer", ctx);

    producer.setSource(Slot::Frame, "reg07=12.5; reg42=reg07*2");
    consumer.setSource(Slot::Frame, "r=reg42+reg07");
    REQUIRE(producer.compileAll());
    REQUIRE(consumer.compileAll());

    REQUIRE(producer.run(Slot::Frame));
    CHECK(ctx->reg(7) == doctest::Approx(12.5));
    CHECK(ctx->reg(42) == doctest::Approx(25.0));

    REQUIRE(consumer.run(Slot::Frame));
    CHECK(consumer.engine().number("r") == doctest::Approx(37.5));
}

TEST_CASE("ScriptSlotHost: q-Variablen synchronisieren mit dem Kontext")
{
    auto ctx = std::make_shared<ScriptContext>();
    ScriptSlotHost frameHost("frame", ctx);
    ScriptSlotHost pointHost("point", ctx);

    frameHost.setSource(Slot::Frame, "q1=3; q64=q1+1");
    pointHost.setSource(Slot::Point, "r=q64*10+q1");
    REQUIRE(frameHost.compileAll());
    REQUIRE(pointHost.compileAll());

    REQUIRE(frameHost.run(Slot::Frame));
    CHECK(ctx->q(1) == doctest::Approx(3.0));
    CHECK(ctx->q(64) == doctest::Approx(4.0));

    REQUIRE(pointHost.run(Slot::Point));
    CHECK(pointHost.engine().number("r") == doctest::Approx(43.0));
}

TEST_CASE("ScriptSlotHost: Skripte ohne reg/q lassen den Kontext unberuehrt")
{
    auto ctx = std::make_shared<ScriptContext>();
    ctx->setReg(0, 5.0);
    ScriptSlotHost host("clean", ctx);
    host.setSource(Slot::Frame, "a=1; b=2");
    REQUIRE(host.compileAll());
    REQUIRE(host.run(Slot::Frame));
    CHECK(ctx->reg(0) == 5.0);
}

TEST_CASE("ScriptSlotHost: Transpile-Fehler leert den Slot, andere laufen")
{
    ScriptSlotHost host("err");
    host.setSource(Slot::Init, "x=1");
    host.setSource(Slot::Frame, "x=((");   // Syntaxfehler
    CHECK_FALSE(host.compileAll());
    CHECK_FALSE(host.lastError().empty());
    CHECK(host.has(Slot::Init));
    CHECK_FALSE(host.has(Slot::Frame));
    REQUIRE(host.run(Slot::Init));
    CHECK(host.engine().number("x") == doctest::Approx(1.0));
}

TEST_CASE("ScriptSlotHost: sourceMentions ist wortgenau und case-insensitiv")
{
    ScriptSlotHost host("mention");
    host.setSource(Slot::Point, "RED=i; shredder=2; bored=3");
    CHECK(host.sourceMentions(Slot::Point, "red"));        // RED (EEL case-insensitiv)
    CHECK(host.sourceMentions(Slot::Point, "shredder"));
    CHECK_FALSE(host.sourceMentions(Slot::Point, "green"));  // nicht enthalten
    CHECK_FALSE(host.sourceMentions(Slot::Point, "blue"));   // "bored" zaehlt nicht
    CHECK_FALSE(host.sourceMentions(Slot::Point, "hred"));   // Teilwort zaehlt nicht
}

TEST_CASE("ScriptSlotHost: Superscope-artiger Ablauf ueber geteilten Kontext")
{
    // Zwei Skript-Traeger eines Presets: Frame-Traeger rechnet, Punkt-Traeger
    // konsumiert — das Muster der kommenden Grid-/LUT-Module.
    auto ctx = std::make_shared<ScriptContext>();
    ScriptSlotHost a("a", ctx);
    ScriptSlotHost b("b", ctx);

    a.setSource(Slot::Init, "reg00=100");
    a.setSource(Slot::Frame, "reg00=reg00+1");
    b.setSource(Slot::Point, "y=reg00/200");
    REQUIRE(a.compileAll());
    REQUIRE(b.compileAll());

    REQUIRE(a.run(Slot::Init));
    for (int frame = 0; frame < 3; ++frame)
    {
        REQUIRE(a.run(Slot::Frame));
        REQUIRE(b.run(Slot::Point));
    }
    CHECK(ctx->reg(0) == doctest::Approx(103.0));
    CHECK(b.engine().number("y") == doctest::Approx(103.0 / 200.0));
}
