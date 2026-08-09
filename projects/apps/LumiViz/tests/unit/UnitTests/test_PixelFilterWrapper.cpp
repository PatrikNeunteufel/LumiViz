/**
 ****************************************************************************************
 * @file   test_PixelFilterWrapper.cpp
 * @brief  Unit-Tests fuer den Pixel-Filter-Wrapper (Stilfilter-Strang,
 *         Session 70): Fragment-Wrapper-Vertrag (#line, Identitaets-
 *         Fallback, Uniform-Satz) und der Serializer-Roundtrip des Knotens
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/PixelFilterWrapper.hpp"

#include <QJsonObject>

#include <string>

using namespace lumi::pixelfilter;

// =============================================================================
// Fragment-Wrapper
// =============================================================================

TEST_CASE("PixelFilter: Wrapper traegt #line 1 vor dem Nutzer-Code")
{
    const std::string userCode =
        "vec4 farbe(vec2 uv, vec4 src) { return vec4(1.0 - src.rgb, 1.0); }";
    const std::string wrapped = wrapFragment(userCode);
    const std::size_t line1 = wrapped.find("#line 1\n");
    const std::size_t user = wrapped.find(userCode);
    REQUIRE(line1 != std::string::npos);
    REQUIRE(user != std::string::npos);
    CHECK(line1 < user);
    // Nutzer-Code VOR dem Wrapper-main (Epilog markiert sich mit #line 100000)
    CHECK(user < wrapped.find("#line 100000"));
    CHECK(wrapped.find("void main()") != std::string::npos);
    CHECK(wrapped.rfind("#version 330 core", 0) == 0);
}

TEST_CASE("PixelFilter: leerer Code wird zur Identitaet")
{
    const std::string wrapped = wrapFragment("");
    CHECK(wrapped.find("vec4 farbe(vec2 uv, vec4 src) { return src; }") !=
          std::string::npos);
    // Befund S70: `filter` ist in GLSL RESERVIERT — der Wrapper selbst darf
    // das Wort nirgends als Bezeichner verwenden.
    CHECK(wrapped.find("filter(") == std::string::npos);
}

TEST_CASE("PixelFilter: Uniform-Satz im Praeludium (Vertrag der Referenzseite)")
{
    const std::string wrapped =
        wrapFragment("vec4 farbe(vec2 uv, vec4 src) { return src; }");
    for (const char* u :
         {"uniform sampler2D uTex", "uniform vec2 uResolution",
          "uniform float uTime", "uniform float uDelta", "uniform int uFrame",
          "uniform float bass", "uniform float mid", "uniform float treb",
          "uniform float vol", "uniform float beat", "uniform float uMixAmount"})
    {
        CAPTURE(u);
        CHECK(wrapped.find(u) != std::string::npos);
    }
}

TEST_CASE("PixelFilter: Epilog mischt ueber uMixAmount gegen das Original")
{
    const std::string wrapped = wrapFragment("");
    // Der Mix-Vertrag lebt im Epilog — Beautify/Editoren duerfen sich darauf
    // verlassen, dass mixAmount NIE im Nutzer-Code stehen muss.
    CHECK(wrapped.find("mix(_lumiSrc.rgb, _lumiErg.rgb") != std::string::npos);
    CHECK(wrapped.find("clamp(uMixAmount, 0.0, 1.0)") != std::string::npos);
}

TEST_CASE("PixelFilter: Starter definiert farbe() und nutzt Audio + Nachbarn")
{
    const std::string starter = starterFilter();
    CHECK(starter.find("vec4 farbe(vec2 uv, vec4 src)") != std::string::npos);
    CHECK(starter.find("bass") != std::string::npos);
    CHECK(starter.find("texture(uTex,") != std::string::npos);
}

// =============================================================================
// Serializer-Roundtrip
// =============================================================================

TEST_CASE("PixelFilter: ChainSerializer-Roundtrip erhaelt alle Felder")
{
    using namespace lumi::multieffect;
    PixelFilterParams p;
    p.code = "vec4 farbe(vec2 uv, vec4 src) { return src.gbra; }";
    p.mixAmount = 0.4;
    p.initCode = "mixamount=1;";
    p.frameCode = "mixamount=0.5+0.5*bass;";
    p.beatCode = "mixamount=1;";

    ChainNode root;
    root.params = ListParams{};
    ChainNode child;
    child.params = p;
    child.displayName = "Filter-Test";
    root.children.push_back(child);

    const QJsonObject json = nodeToJson(root);
    const ChainNode back = nodeFromJson(json, nullptr);
    REQUIRE(back.children.size() == 1);
    const auto* q = std::get_if<PixelFilterParams>(&back.children[0].params);
    REQUIRE(q != nullptr);
    CHECK(q->code == p.code);
    CHECK(q->mixAmount == doctest::Approx(p.mixAmount));
    CHECK(q->initCode == p.initCode);
    CHECK(q->frameCode == p.frameCode);
    CHECK(q->beatCode == p.beatCode);
    CHECK(effectTypeKey(back.children[0].params) == "pixelFilter");
}

TEST_CASE("PixelFilter: Leser klemmt den Mix auf 0..1")
{
    using namespace lumi::multieffect;
    ChainNode root;
    root.params = ListParams{};
    ChainNode child;
    PixelFilterParams p;
    p.mixAmount = 7.0;  // Serialisieren schreibt roh …
    child.params = p;
    root.children.push_back(child);
    const ChainNode back = nodeFromJson(nodeToJson(root), nullptr);
    const auto* q = std::get_if<PixelFilterParams>(&back.children[0].params);
    REQUIRE(q != nullptr);
    // … der Leser klemmt auf den Regler-Bereich
    CHECK(q->mixAmount == doctest::Approx(1.0));
}
