/**
 ****************************************************************************************
 * @file   test_ScriptModules.cpp
 * @brief  Tests fuer die skriptbaren Feld-Module (Import-Phase Roadmap 4.2):
 *         ScriptGridModule (pro Gitterknoten) und ScriptLutModule (pro LUT-Eintrag)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/modules/scripting/ScriptGridModule.hpp"
#include "visualizers/modules/scripting/ScriptLutModule.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"

#include <array>
#include <cmath>
#include <memory>
#include <vector>

using lumi::modules::GridNode;
using lumi::modules::ScriptGridModule;
using lumi::modules::ScriptLutModule;
using lumi::modules::SuperscopeModule;
using lumi::scripting::ScriptContext;

// =============================================================================
// ScriptGridModule
// =============================================================================

TEST_CASE("ScriptGridModule: ohne Point-Skript liefert das Feld die Identitaet")
{
    ScriptGridModule grid;
    grid.setGridSize(5, 5);
    grid.execute(800.0f, 600.0f, false, 1.0f / 60.0f);

    const GridNode& corner = grid.node(0, 0);
    CHECK(corner.u == doctest::Approx(-1.0f));
    CHECK(corner.v == doctest::Approx(-1.0f));
    const GridNode& center = grid.node(2, 2);
    CHECK(center.u == doctest::Approx(0.0f));
    CHECK(center.v == doctest::Approx(0.0f));
    CHECK(center.alpha == doctest::Approx(1.0f));
    CHECK(grid.field().size() == 25);
}

TEST_CASE("ScriptGridModule: Rect-Modus — Identitaets-Skript und Zoom")
{
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    grid.setRectCoords(true);

    SUBCASE("Identitaet: x=x; y=y")
    {
        grid.setPointCode("x=x; y=y");
        grid.execute(100.0f, 100.0f, false, 0.016f);
        CHECK(grid.lastScriptError().empty());
        CHECK(grid.node(0, 0).u == doctest::Approx(-1.0f));
        CHECK(grid.node(2, 2).u == doctest::Approx(1.0f));
        CHECK(grid.node(1, 1).u == doctest::Approx(0.0f));
    }

    SUBCASE("Zoom: Quellkoordinate skaliert (x*0.5 = 2x-Zoom)")
    {
        grid.setPointCode("x=x*0.5; y=y*0.5");
        grid.execute(100.0f, 100.0f, false, 0.016f);
        CHECK(grid.node(0, 0).u == doctest::Approx(-0.5f));
        CHECK(grid.node(0, 0).v == doctest::Approx(-0.5f));
        CHECK(grid.node(2, 2).u == doctest::Approx(0.5f));
    }
}

TEST_CASE("ScriptGridModule: Polar-Modus (AVS-Default) — d-Manipulation")
{
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    // Radius halbieren: klassisches "Zoom-In"-Movement
    grid.setPointCode("d=d*0.5");
    grid.execute(100.0f, 100.0f, false, 0.016f);
    CHECK(grid.lastScriptError().empty());

    // Ecke (-1,-1): d=sqrt(2) -> 0.7071..., Richtung bleibt 225 Grad
    const GridNode& corner = grid.node(0, 0);
    CHECK(corner.u == doctest::Approx(-0.5f).epsilon(0.001));
    CHECK(corner.v == doctest::Approx(-0.5f).epsilon(0.001));
    // Zentrum bleibt im Zentrum
    CHECK(grid.node(1, 1).u == doctest::Approx(0.0f));
    CHECK(grid.node(1, 1).v == doctest::Approx(0.0f));
}

TEST_CASE("ScriptGridModule: absolutes d nutzt AVS-Normierung (Ecke=1)")
{
    // AVS-Builtin #15 "psychotic beaming": d=0.15 setzt den Radius absolut.
    // Neue Konvention: d ist auf die Ecke (=1) normiert, zurueck * sqrt(2) ->
    // die Ecke landet bei cos(225°)*0.15*sqrt(2) = -0.15 (alt waere -0.106).
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    grid.setPointCode("d = 0.15");
    grid.execute(100.0f, 100.0f, false, 0.016f);
    CHECK(grid.lastScriptError().empty());

    const GridNode& corner = grid.node(0, 0);
    CHECK(corner.u == doctest::Approx(-0.15f).epsilon(0.01));
    CHECK(corner.v == doctest::Approx(-0.15f).epsilon(0.01));
}

TEST_CASE("ScriptGridModule: Rotation um 90 Grad im Polar-Modus")
{
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    grid.setPointCode("r=r+$PI/2");
    grid.execute(100.0f, 100.0f, false, 0.016f);

    // Punkt rechts (1, 0) wandert im AVS-SCREEN-Raum nach unten (S46,
    // Befund A: Skript-Rand spiegelt y — GL v=-1; vorher drehten
    // Rotations-Skripte spiegelverkehrt und das Gate erwartete +1)
    const GridNode& right = grid.node(2, 1);
    CHECK(right.u == doctest::Approx(0.0f).epsilon(0.001));
    CHECK(right.v == doctest::Approx(-1.0f).epsilon(0.001));
}

TEST_CASE("ScriptGridModule: d/r im PIXEL-Raum auf nicht-quadratischer Flaeche (S2)")
{
    // r_dmove.cpp:324-332 / r_trans.cpp:459-464: d = Pixel-Abstand / halbe
    // Diagonale, r = atan2 ueber Pixel-Offsets. Flaeche 200x100: halfW=100,
    // halfH=50, maxD=sqrt(100^2+50^2)=111.8034. Reine d-Skalierung ist in
    // beiden Konventionen identisch — diskriminierend sind Rotation und
    // absolute d-Werte.
    ScriptGridModule grid;
    grid.setGridSize(3, 3);

    SUBCASE("Identitaet bleibt Identitaet")
    {
        grid.setPointCode("d=d");
        grid.execute(200.0f, 100.0f, false, 0.016f);
        CHECK(grid.node(0, 0).u == doctest::Approx(-1.0f).epsilon(0.001));
        CHECK(grid.node(0, 0).v == doctest::Approx(-1.0f).epsilon(0.001));
    }

    SUBCASE("Rotation ist starr in Pixeln: rechts (100 px) -> unten (100 px = 2*halfH)")
    {
        grid.setPointCode("r=r+$PI/2");
        grid.execute(200.0f, 100.0f, false, 0.016f);
        const GridNode& right = grid.node(2, 1);
        CHECK(right.u == doctest::Approx(0.0f).epsilon(0.001));
        // NDC-Bug lieferte hier 1.0 (Rotation im normierten Quadrat);
        // S46 Befund A: AVS dreht im Screen-Raum (y+ unten) -> GL v=-2
        CHECK(right.v == doctest::Approx(-2.0f).epsilon(0.001));
    }

    SUBCASE("absolutes d: 0.5 = halbe Diagonale in Pixeln")
    {
        grid.setPointCode("d=0.5");
        grid.execute(200.0f, 100.0f, false, 0.016f);
        const GridNode& right = grid.node(2, 1);
        // 0.5 * 111.8034 px / halfW = 0.559 (NDC-Bug: 0.707)
        CHECK(right.u == doctest::Approx(0.559017f).epsilon(0.001));
        CHECK(right.v == doctest::Approx(0.0f).epsilon(0.001));
    }
}

TEST_CASE("ScriptGridModule: alpha nur wenn im Skript erwaehnt")
{
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    grid.setRectCoords(true);
    grid.setPointCode("x=x; y=y; alpha=0.25");
    grid.execute(100.0f, 100.0f, false, 0.016f);
    CHECK(grid.node(1, 1).alpha == doctest::Approx(0.25f));
}

TEST_CASE("ScriptGridModule: Frame-Skript steuert das Feld ueber Variablen")
{
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    grid.setRectCoords(true);
    grid.setInitCode("zoom=1");
    grid.setFrameCode("zoom=zoom*0.5");
    grid.setPointCode("x=x*zoom; y=y*zoom");

    grid.execute(100.0f, 100.0f, false, 0.016f);   // zoom = 0.5
    CHECK(grid.node(2, 2).u == doctest::Approx(0.5f));
    grid.execute(100.0f, 100.0f, false, 0.016f);   // zoom = 0.25
    CHECK(grid.node(2, 2).u == doctest::Approx(0.25f));
}

TEST_CASE("ScriptGridModule: geteilter Kontext verbindet Grid und andere Traeger")
{
    auto ctx = std::make_shared<ScriptContext>();
    ScriptGridModule grid(ctx);
    grid.setGridSize(3, 3);
    grid.setRectCoords(true);
    ctx->setReg(5, 0.5);
    grid.setPointCode("x=x*reg05; y=y");
    grid.execute(100.0f, 100.0f, false, 0.016f);
    CHECK(grid.node(2, 2).u == doctest::Approx(0.5f));
}

TEST_CASE("ScriptGridModule: Syntaxfehler -> Identitaet + Fehlermeldung")
{
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    grid.setPointCode("d=((");
    grid.execute(100.0f, 100.0f, false, 0.016f);
    CHECK_FALSE(grid.lastScriptError().empty());
    CHECK(grid.node(0, 0).u == doctest::Approx(-1.0f));   // Identitaet
}

TEST_CASE("ScriptGridModule: Frame laeuft VOR Beat (r_dmove.cpp:297-298, S47)")
{
    // AVS-Reihenfolge: Frame-Code rechnet mit den Beat-Werten des VORHERIGEN
    // Frames — Beat-vor-Frame liess jede Beat-Wirkung einen Frame zu frueh los.
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    grid.setRectCoords(true);
    grid.setBeatCode("ti=0.5");
    grid.setFrameCode("t=t-ti");
    grid.setPointCode("x=x+t; y=y");
    grid.execute(100.0f, 100.0f, true, 0.016f);   // Beat-Frame 0
    CHECK(grid.lastScriptError().empty());
    CHECK(grid.node(1, 1).u == doctest::Approx(0.0f));    // t noch 0 (ti nachher)
    grid.execute(100.0f, 100.0f, false, 0.016f);
    CHECK(grid.node(1, 1).u == doctest::Approx(-0.5f));   // jetzt t = -ti
}

TEST_CASE("ScriptGridModule: visdata VOR dem Erst-Compile erreicht den Frame-0-Beat (S47)")
{
    // Regressionsgate: setVisData() vor dem ersten execute() verpuffte (kein
    // Skript-Host) — Beat/Init des allerersten Frames sahen Null-Audio und
    // getspec lieferte 0 (der Wormhole verpasste so seinen Frame-0-Beat).
    std::array<unsigned char, 576 * 4> vis{};
    for (int i = 0; i < 576 * 2; ++i) vis[i] = 200;   // Spektrum L/R
    ScriptGridModule grid;
    grid.setGridSize(3, 3);
    grid.setRectCoords(true);
    grid.setBeatCode("amp=getspec(0.5,1,0)");
    grid.setPointCode("x=x; y=y; alpha=amp");
    grid.setVisData(vis.data(), 0.0);                 // VOR dem ersten execute()
    grid.execute(100.0f, 100.0f, true, 0.016f);
    CHECK(grid.lastScriptError().empty());
    CHECK(grid.node(1, 1).alpha == doctest::Approx(200.0f / 255.0f).epsilon(0.01));
}

// =============================================================================
// ScriptLutModule
// =============================================================================

TEST_CASE("ScriptLutModule: ohne Level-Skript ist die LUT die Identitaet")
{
    ScriptLutModule lut;
    lut.execute(false, 0.016f);
    CHECK(lut.lut(0, 0) == doctest::Approx(0.0f));
    CHECK(lut.lut(1, 128) == doctest::Approx(128.0f / 255.0f));
    CHECK(lut.lut(2, 255) == doctest::Approx(1.0f));
}

TEST_CASE("ScriptLutModule: Invert-Skript spiegelt die LUT")
{
    ScriptLutModule lut;
    lut.setLevelCode("red=1-red; green=1-green; blue=1-blue");
    lut.execute(false, 0.016f);
    CHECK(lut.lastScriptError().empty());
    CHECK(lut.lut(0, 0) == doctest::Approx(1.0f));
    CHECK(lut.lut(0, 255) == doctest::Approx(0.0f));
    CHECK(lut.lut(1, 128) == doctest::Approx(1.0f - 128.0f / 255.0f));
}

TEST_CASE("ScriptLutModule: Kanaele unabhaengig, Werte geklemmt")
{
    ScriptLutModule lut;
    lut.setLevelCode("red=red*2; green=0.5; blue=blue-10");
    lut.execute(false, 0.016f);
    CHECK(lut.lut(0, 255) == doctest::Approx(1.0f));    // 2.0 geklemmt
    CHECK(lut.lut(0, 64) == doctest::Approx(128.0f / 255.0f).epsilon(0.01));
    CHECK(lut.lut(1, 200) == doctest::Approx(0.5f));
    CHECK(lut.lut(2, 100) == doctest::Approx(0.0f));    // negativ geklemmt
}

TEST_CASE("ScriptLutModule: recompute=false rechnet einmal, true jeden Frame")
{
    ScriptLutModule lut;
    lut.setInitCode("gain=1");
    lut.setFrameCode("gain=gain*0.5");
    lut.setLevelCode("red=red*gain; green=green*gain; blue=blue*gain");

    SUBCASE("einmalig (Default)")
    {
        lut.execute(false, 0.016f);   // gain=0.5, LUT gebaut
        const float first = lut.lut(0, 255);
        CHECK(first == doctest::Approx(0.5f));
        lut.execute(false, 0.016f);   // gain=0.25, LUT bleibt
        CHECK(lut.lut(0, 255) == doctest::Approx(first));
    }

    SUBCASE("recompute")
    {
        lut.setRecompute(true);
        lut.execute(false, 0.016f);   // gain=0.5
        CHECK(lut.lut(0, 255) == doctest::Approx(0.5f));
        lut.execute(false, 0.016f);   // gain=0.25
        CHECK(lut.lut(0, 255) == doctest::Approx(0.25f));
    }
}

TEST_CASE("ScriptLutModule: Beat-Skript wirkt auf die naechste Neuberechnung")
{
    ScriptLutModule lut;
    lut.setRecompute(true);
    lut.setInitCode("boost=1");
    lut.setBeatCode("boost=2");
    lut.setLevelCode("red=red*boost; green=green; blue=blue");

    lut.execute(false, 0.016f);
    CHECK(lut.lut(0, 128) == doctest::Approx(128.0f / 255.0f));
    lut.execute(true, 0.016f);   // Beat: boost=2
    CHECK(lut.lut(0, 128) == doctest::Approx(1.0f));   // geklemmt bei 2*0.502
}

TEST_CASE("ScriptLutModule: geteilter Kontext speist das Level-Skript")
{
    auto ctx = std::make_shared<ScriptContext>();
    ScriptLutModule lut(ctx);
    ctx->setReg(3, 0.5);
    lut.setLevelCode("red=red*reg03; green=green*reg03; blue=blue*reg03");
    lut.execute(false, 0.016f);
    CHECK(lut.lut(0, 255) == doctest::Approx(0.5f));
}

// =============================================================================
// SuperscopeModule — Basisfarbe wird vor dem Point-Code vorbelegt (AVS r_sscope)
// =============================================================================

TEST_CASE("SuperscopeModule: Frame VOR Beat + visdata beim Erst-Compile (S47)")
{
    // r_sscope.cpp:271-273: Init einmal, dann FRAME, dann Beat. Und: visdata,
    // das VOR dem ersten execute() gesetzt wurde, muss den Frame-0-Beat
    // erreichen (getspec > 0) — beides zusammen ist das Wormhole-Gate.
    std::array<unsigned char, 576 * 4> vis{};
    for (int i = 0; i < 576 * 2; ++i) vis[i] = 200;   // Spektrum L/R
    const std::vector<float> silence(576, 0.0f);
    SuperscopeModule ss;
    ss.setLuaMode(true);
    ss.setPointCount(2);
    ss.setInitCode("n=2");
    ss.setBeatCode("ti=getspec(0.5,1,0)");
    ss.setFrameCode("t=t-ti");
    ss.setPointCode("x=i*2-1; y=t");
    ss.setVisData(vis.data(), 0.0);                   // VOR dem ersten execute()
    auto pts = ss.execute(silence.data(), silence.data(), silence.data(),
                          silence.data(), 576, 100, 100, true, 0.016f);
    REQUIRE(!pts.empty());
    CHECK(pts[0].y == doctest::Approx(0.0f));         // Frame vor Beat: t noch 0
    pts = ss.execute(silence.data(), silence.data(), silence.data(),
                     silence.data(), 576, 100, 100, false, 0.016f);
    REQUIRE(!pts.empty());
    // t = -ti mit ti = getspec = 200/255 (Vorzeichen je nach y-Konvention)
    CHECK(std::abs(pts[0].y) == doctest::Approx(200.0f / 255.0f).epsilon(0.01));
}

TEST_CASE("SuperscopeModule: Table-Basisfarbe wird vorbelegt; Code kann modulieren")
{
    const std::vector<float> silence(576, 0.0f);
    auto run = [&](const char* point) {
        SuperscopeModule ss;
        ss.setLuaMode(true);
        ss.setPointCount(4);
        ss.setColorTable({0x00FF0000u});  // reine Rot-Tabelle
        ss.setColorBlend(1);              // Table only
        ss.setPointCode(point);
        return ss.execute(silence.data(), silence.data(), silence.data(),
                          silence.data(), 576, 100, 100, false, 0.016f);
    };

    SUBCASE("Code ohne Farbe -> behaelt die Basisfarbe")
    {
        const auto pts = run("x=i*2-1; y=0");
        REQUIRE(!pts.empty());
        CHECK(pts[0].r == doctest::Approx(1.0f));
        CHECK(pts[0].g == doctest::Approx(0.0f));
        CHECK(pts[0].b == doctest::Approx(0.0f));
    }
    SUBCASE("Code moduliert die Basisfarbe (red=red*0.5)")
    {
        const auto pts = run("x=i*2-1; y=0; red=red*0.5");
        REQUIRE(!pts.empty());
        CHECK(pts[0].r == doctest::Approx(0.5f));
    }
    SUBCASE("Code ueberschreibt die Basisfarbe absolut")
    {
        const auto pts = run("x=i*2-1; y=0; blue=1; red=0");
        REQUIRE(!pts.empty());
        CHECK(pts[0].r == doctest::Approx(0.0f));
        CHECK(pts[0].b == doctest::Approx(1.0f));
    }
}
