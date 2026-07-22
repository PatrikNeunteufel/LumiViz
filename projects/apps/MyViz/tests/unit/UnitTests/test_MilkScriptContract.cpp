/**
 ****************************************************************************************
 * @file   test_MilkScriptContract.cpp
 * @brief  Golden-Tests fuer den Milk-Skript-Vertrag (Import-Phase Roadmap 6, M2):
 *         Funktions-Deltas (int=floor, $pi), reale per_frame-Snippets end-to-end,
 *         q1-q64-Snapshot-Fluss (per_frame_init -> per_frame -> Wave/per_pixel),
 *         t1-t8-Restore-Muster, MilkLoudness (bass/mid/treb + *_att)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <EelTranspiler.hpp>

#include "scripting/LuaScriptEngine.hpp"
#include "scripting/ScriptContext.hpp"
#include "scripting/ScriptSlotHost.hpp"
#include "visualizers/modules/processing/MilkLoudness.hpp"

#include <cmath>
#include <memory>
#include <string>

using lumi::eel::Dialect;
using lumi::eel::transpile;
using lumi::modules::MilkLoudness;
using lumi::scripting::LuaScriptEngine;
using lumi::scripting::ScriptContext;
using lumi::scripting::ScriptSlotHost;
using Slot = LuaScriptEngine::Slot;

namespace
{

/// Milk-EEL transpilieren, im Frame-Slot ausfuehren, eine Variable lesen
double evalMilk(const std::string& src, const char* resultVar = "r")
{
    auto result = transpile(src, Dialect::Milkdrop);
    CAPTURE(src);
    CAPTURE(result.error);
    REQUIRE(result.ok);
    LuaScriptEngine engine;
    CAPTURE(result.lua);
    REQUIRE(engine.compile(Slot::Frame, result.lua, "milk"));
    CAPTURE(engine.lastError());
    REQUIRE(engine.run(Slot::Frame));
    return engine.number(resultVar);
}

} // namespace

// =============================================================================
// Funktions-Deltas (Korpus-Messung Session 39: int() = 1153 Aufrufstellen)
// =============================================================================

TEST_CASE("MilkVertrag: int() ist floor-Alias (nseel-eval.c:284) — KEIN trunc/round")
{
    CHECK(evalMilk("r=int(3.7);") == doctest::Approx(3.0));
    CHECK(evalMilk("r=int(3.2);") == doctest::Approx(3.0));
    // floor(-3.7) = -4 — Truncation Richtung 0 waere -3, Rundung waere -4
    CHECK(evalMilk("r=int(-3.7);") == doctest::Approx(-4.0));
    CHECK(evalMilk("r=int(-3.2);") == doctest::Approx(-4.0));
    CHECK(evalMilk("r=int(5);") == doctest::Approx(5.0));
}

TEST_CASE("MilkVertrag: $pi/$e/$phi-Literale (EEL2)")
{
    CHECK(evalMilk("r=$pi;") == doctest::Approx(3.14159265).epsilon(1e-6));
    CHECK(evalMilk("r=$e;") == doctest::Approx(2.71828183).epsilon(1e-6));
    CHECK(evalMilk("r=$phi;") == doctest::Approx(1.61803399).epsilon(1e-6));
}

TEST_CASE("MilkVertrag: Statement-Sequenzen in Funktionsargumenten (colorwall-Muster)")
{
    // ';'-Sequenz als if-Zweig, inkl. Leer-Statement vor dem Komma (`o9=0;,0`)
    auto result = transpile("bt=1; if(bt, t0=5; pk=4+2, 0); r=t0+pk;"
                            "if(bt, o8=0; o9=7;, 0);",
                            Dialect::Milkdrop);
    CAPTURE(result.error);
    REQUIRE(result.ok);
    LuaScriptEngine engine;
    REQUIRE(engine.compile(Slot::Frame, result.lua, "milk"));
    REQUIRE(engine.run(Slot::Frame));
    CHECK(engine.number("t0") == doctest::Approx(5.0));
    CHECK(engine.number("pk") == doctest::Approx(6.0));
    CHECK(engine.number("r") == doctest::Approx(11.0));
    CHECK(engine.number("o9") == doctest::Approx(7.0));

    // Sequenz-Wert = letztes Statement: x = if(1, a=2;a*3, 0) -> 6
    CHECK(evalMilk("r=if(1, a=2; a*3, 0);") == doctest::Approx(6.0));
    // Lazy bleibt gewahrt: falscher Zweig laeuft nicht
    CHECK(evalMilk("n=0; r=if(0, n=99; 1, 2);") == doctest::Approx(2.0));
    CHECK(evalMilk("n=0; x=if(0, n=99; 1, 2); r=n;") == doctest::Approx(0.0));
}

// =============================================================================
// Reale Preset-Snippets end-to-end (Milk-Dialekt)
// =============================================================================

TEST_CASE("MilkVertrag: per_frame-Snippet aus 'Dragon Science' laeuft (Beat-Gate)")
{
    // Konkateniert wie vom MilkParser geliefert (Zeilen ohne Backtick nahtlos)
    const std::string perFrame =
        "le=1+.5+2*sin(bass_att);"
        "bpulse=band(above(le,bth),above(le-bth,bblock));"
        "bblock=le-bth;"
        "bth=if(above(le,bth),le+114/(le+10)-7.407,"
        "bth+bth*.07/(bth-12)+below(bth,2.7)*.1*(2.7-bth));"
        "bth=if(above(bth,6),6,bth);"
        "q1=bpulse;";

    auto result = transpile(perFrame, Dialect::Milkdrop);
    CAPTURE(result.error);
    REQUIRE(result.ok);

    LuaScriptEngine engine;
    REQUIRE(engine.compile(Slot::Frame, result.lua, "dragon.per_frame"));
    engine.setNumber("bass_att", 1.2);
    engine.setNumber("bth", 0.0);
    REQUIRE(engine.run(Slot::Frame));
    // Zahlenwerk plausibel: le = 1.5 + 2*sin(1.2)
    CHECK(engine.number("le") == doctest::Approx(1.5 + 2.0 * std::sin(1.2)).epsilon(1e-9));
    // q1 ist 0 oder 1 (band/above-Ergebnis)
    const double q1 = engine.number("q1");
    CHECK((q1 == doctest::Approx(0.0) || q1 == doctest::Approx(1.0)));
}

TEST_CASE("MilkVertrag: per_pixel-Snippet (zoom/rot auf Vertex-Inputs) laeuft")
{
    const std::string perPixel =
        "zoom = zoom + 0.01 * ( sin(2*cos(3*(sqrt(2)-rad)*ang)) + 1 );"
        "rot = 0.012;";
    auto result = transpile(perPixel, Dialect::Milkdrop);
    REQUIRE(result.ok);

    LuaScriptEngine engine;
    REQUIRE(engine.compile(Slot::Point, result.lua, "milk.per_pixel"));
    engine.setNumber("zoom", 1.0);
    engine.setNumber("rad", 0.5);
    engine.setNumber("ang", 1.0);
    REQUIRE(engine.run(Slot::Point));
    CHECK(engine.number("zoom") > 1.0);
    CHECK(engine.number("zoom") < 1.03);
    CHECK(engine.number("rot") == doctest::Approx(0.012));
}

// =============================================================================
// q1-q64-Snapshot-Fluss (MilkDrop-Frame-Modell, milkdropfs.cpp:491-493, 673-674)
// =============================================================================

TEST_CASE("MilkVertrag: q-Fluss init -> frame -> wave ueber ScriptContext-Snapshots")
{
    auto ctx = std::make_shared<ScriptContext>();
    ScriptSlotHost preset("preset", ctx, ScriptSlotHost::Dialect::Milkdrop);
    ScriptSlotHost wave("wave0", ctx, ScriptSlotHost::Dialect::Milkdrop);

    preset.setSource(Slot::Init, "q1=5; q2=10;");
    preset.setSource(Slot::Frame, "q1=q1+1;");
    wave.setSource(Slot::Frame, "wq=q1; q1=99;");  // Wave liest q1, verschmutzt es
    REQUIRE(preset.compileAll());
    REQUIRE(wave.compileAll());

    // per_frame_init einmalig; danach q-Werte einfrieren (q_values_after_init)
    REQUIRE(preset.run(Slot::Init));
    ctx->captureInitSnapshot();
    CHECK(ctx->q(1) == doctest::Approx(5.0));

    for (int frame = 0; frame < 3; ++frame)
    {
        CAPTURE(frame);
        // Frame-Beginn: q auf Init-Stand — JEDEN Frame (nicht kumulativ!)
        ctx->restoreInitSnapshot();
        REQUIRE(preset.run(Slot::Frame));
        CHECK(ctx->q(1) == doctest::Approx(6.0));  // immer 5+1, nie 7/8/...
        ctx->captureFrameSnapshot();

        // Wave sieht den post-frame-Stand; ihre Schreibzugriffe leaken nicht
        // in den naechsten Frame (restoreInitSnapshot ueberschreibt sie)
        REQUIRE(wave.run(Slot::Frame));
        CHECK(wave.engine().number("wq") == doctest::Approx(6.0));
        CHECK(ctx->q(1) == doctest::Approx(99.0));  // bis zum naechsten restore

        // Point-Laeufe wuerden restoreFrameSnapshot nutzen: post-frame, nicht 99
        ctx->restoreFrameSnapshot();
        CHECK(ctx->q(1) == doctest::Approx(6.0));
    }

    // q2 blieb ueber alle Frames auf dem Init-Stand
    CHECK(ctx->q(2) == doctest::Approx(10.0));
}

// =============================================================================
// t1-t8: wave-/shape-lokale Snapshots (milkdropfs.cpp:2278-2285, 2415)
// =============================================================================

TEST_CASE("MilkVertrag: t1-t8-Restore-Muster auf Engine-Ebene (M3-Mechanik)")
{
    ScriptSlotHost wave("wave0", nullptr, ScriptSlotHost::Dialect::Milkdrop);
    wave.setSource(Slot::Init, "t1=3; t2=0.5;");
    wave.setSource(Slot::Frame, "t1=t1*2; t2=t2+1;");
    REQUIRE(wave.compileAll());

    // Nach dem Wave-Init: t-Werte einfrieren (t_values_after_init_code)
    REQUIRE(wave.run(Slot::Init));
    const double t1AfterInit = wave.engine().number("t1");
    const double t2AfterInit = wave.engine().number("t2");
    CHECK(t1AfterInit == doctest::Approx(3.0));

    for (int frame = 0; frame < 3; ++frame)
    {
        CAPTURE(frame);
        // Frame-Beginn: t auf Init-Stand zuruecksetzen — Aenderungen des
        // Vorframes verfallen (Original-Verhalten, KEIN Akkumulieren)
        wave.engine().setNumber("t1", t1AfterInit);
        wave.engine().setNumber("t2", t2AfterInit);
        REQUIRE(wave.run(Slot::Frame));
        CHECK(wave.engine().number("t1") == doctest::Approx(6.0));
        CHECK(wave.engine().number("t2") == doctest::Approx(1.5));
    }
}

// =============================================================================
// MilkLoudness: bass/mid/treb (imm_rel) + *_att (avg_rel), plugin.cpp:8749-8779
// =============================================================================

TEST_CASE("MilkLoudness: stationaeres Signal pendelt auf 1.0 ein")
{
    MilkLoudness loud;
    for (int i = 0; i < 300; ++i) loud.update(0.4, 0.3, 0.2, 30.0);
    CHECK(loud.bass() == doctest::Approx(1.0).epsilon(0.02));
    CHECK(loud.mid() == doctest::Approx(1.0).epsilon(0.02));
    CHECK(loud.treb() == doctest::Approx(1.0).epsilon(0.02));
    CHECK(loud.bassAtt() == doctest::Approx(1.0).epsilon(0.02));
    CHECK(loud.trebAtt() == doctest::Approx(1.0).epsilon(0.02));
}

TEST_CASE("MilkLoudness: Beat-Spike — bass springt sofort, bass_att gedaempft")
{
    MilkLoudness loud;
    for (int i = 0; i < 300; ++i) loud.update(0.4, 0.3, 0.2, 30.0);

    loud.update(0.8, 0.3, 0.2, 30.0);  // Bass verdoppelt fuer einen Frame
    CHECK(loud.bass() > 1.5);                  // imm_rel folgt sofort
    CHECK(loud.bassAtt() > 1.0);               // avg_rel zieht an ...
    CHECK(loud.bassAtt() < loud.bass());       // ... aber gedaempft
    CHECK(loud.mid() == doctest::Approx(1.0).epsilon(0.02));  // andere Baender ruhig

    // Release: zurueck auf Basis — bass faellt sofort, att haengt nach
    loud.update(0.4, 0.3, 0.2, 30.0);
    CHECK(loud.bass() == doctest::Approx(1.0).epsilon(0.05));
    CHECK(loud.bassAtt() > loud.bass() + 0.05);
}

TEST_CASE("MilkLoudness: Stille liest sich als 1.0 (Guard), nicht inf/NaN")
{
    MilkLoudness loud;
    for (int i = 0; i < 200; ++i) loud.update(0.0, 0.0, 0.0, 30.0);
    CHECK(loud.bass() == doctest::Approx(1.0));
    CHECK(loud.bassAtt() == doctest::Approx(1.0));
    CHECK(std::isfinite(loud.trebAtt()));
}

TEST_CASE("MilkLoudness: fps-Korrektur — 60fps-Doppelschritte ~ 30fps-Schritte")
{
    MilkLoudness at30;
    MilkLoudness at60;
    // identische Signal-Zeitachse: 2 s Basis, dann 1 s lauter
    for (int i = 0; i < 60; ++i) at30.update(0.4, 0.3, 0.2, 30.0);
    for (int i = 0; i < 120; ++i) at60.update(0.4, 0.3, 0.2, 60.0);
    for (int i = 0; i < 30; ++i) at30.update(0.6, 0.3, 0.2, 30.0);
    for (int i = 0; i < 60; ++i) at60.update(0.6, 0.3, 0.2, 60.0);
    CHECK(at30.bassAtt() == doctest::Approx(at60.bassAtt()).epsilon(0.05));
    CHECK(at30.bass() == doctest::Approx(at60.bass()).epsilon(0.05));
}

TEST_CASE("MilkLoudness: reset() setzt auf Kaltstart zurueck")
{
    MilkLoudness loud;
    for (int i = 0; i < 100; ++i) loud.update(0.4, 0.3, 0.2, 30.0);
    loud.reset();
    CHECK(loud.frame() == 0);
    // Kaltstart: erster Frame seedet die Mittelwerte -> rel = 1.0 (kein Spike)
    loud.update(0.7, 0.1, 0.1, 30.0);
    CHECK(loud.bass() == doctest::Approx(1.0).epsilon(0.01));
}
