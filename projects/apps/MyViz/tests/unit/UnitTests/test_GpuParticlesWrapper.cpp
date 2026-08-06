/**
 ****************************************************************************************
 * @file   test_GpuParticlesWrapper.cpp
 * @brief  Unit-Tests fuer den GPU-Partikel-Wrapper (Strang G2, Session 69):
 *         Shader-Vertraege (Uniform-Satz, #line, Kraftfeld-Injektion,
 *         gemeinsames Praeludium beider Paesse), Zustands-Zeilen-Rechnung
 *         und der Serializer-Roundtrip des neuen Knotens
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/GpuParticlesWrapper.hpp"

#include <QJsonObject>

#include <string>

using namespace lumi::gpuparticles;

// =============================================================================
// Shader-Bausteine
// =============================================================================

TEST_CASE("GpuParticles: Kraftfeld-Injektion traegt #line 1, leer = Null-Kraft")
{
    const std::string user = "vec2 kraft(vec2 pos, vec2 vel, float alter) { return vec2(0.0, bass); }";
    const std::string mitUser = updateFragment(user);
    const std::size_t line1 = mitUser.find("#line 1\n");
    const std::size_t userPos = mitUser.find(user);
    REQUIRE(line1 != std::string::npos);
    REQUIRE(userPos != std::string::npos);
    CHECK(line1 < userPos);
    CHECK(userPos < mitUser.find("#line 100000"));

    const std::string ohne = updateFragment("");
    CHECK(ohne.find("vec2 kraft(vec2 pos, vec2 vel, float alter) { return vec2(0.0); }") !=
          std::string::npos);
    CHECK(ohne.find("#line 1\n") == std::string::npos);  // kein Nutzer-Code
}

TEST_CASE("GpuParticles: beide Paesse teilen das Praeludium (Alter-Formeln)")
{
    // Update und Render muessen dieselbe Lebenslauf-Rechnung sehen — sonst
    // springt der Respawn zu einem anderen Zeitpunkt, als der Draw ausblendet.
    const std::string up = updateFragment("");
    const std::string rv = renderVertex();
    for (const char* baustein : {"_lumiHash(", "_lumiLeben(", "_lumiPhase(",
                                 "_lumiZyklus(", "_lumiAlter01(",
                                 "uniform float uLife;",
                                 "uniform float uLifeJitter;",
                                 "uniform float bass;"})
    {
        CAPTURE(baustein);
        CHECK(up.find(baustein) != std::string::npos);
        CHECK(rv.find(baustein) != std::string::npos);
    }
    CHECK(up.rfind("#version 330 core", 0) == 0);
    CHECK(rv.rfind("#version 330 core", 0) == 0);
}

TEST_CASE("GpuParticles: Render-Vertex nutzt Instanz-Id + Zustands-Fetch")
{
    const std::string rv = renderVertex();
    CHECK(rv.find("gl_InstanceID") != std::string::npos);
    CHECK(rv.find("texelFetch(uState") != std::string::npos);
    // Ecke kommt vom geteilten Fullscreen-Quad (aPos an Location 0)
    CHECK(rv.find("layout(location = 0) in vec2 aPos;") != std::string::npos);
}

TEST_CASE("GpuParticles: Starter definiert kraft() und nutzt Audio")
{
    const std::string starter = starterForce();
    CHECK(starter.find("vec2 kraft(vec2 pos, vec2 vel, float alter)") !=
          std::string::npos);
    CHECK(starter.find("bass") != std::string::npos);
}

// =============================================================================
// Zustands-Layout
// =============================================================================

TEST_CASE("GpuParticles: stateRows rundet auf ganze Zeilen")
{
    CHECK(stateRows(1) == 1);
    CHECK(stateRows(kStateWidth) == 1);
    CHECK(stateRows(kStateWidth + 1) == 2);
    CHECK(stateRows(kMaxCount) == kStateWidth);
}

// =============================================================================
// Serializer-Roundtrip
// =============================================================================

TEST_CASE("GpuParticles: ChainSerializer-Roundtrip erhaelt alle Felder")
{
    using namespace lumi::multieffect;
    GpuParticlesParams p;
    p.count = 12345;
    p.spawnX = 0.25;
    p.spawnY = 0.75;
    p.spawnSpread = 0.1;
    p.speed = 0.5;
    p.direction = 45.0;
    p.fan = 90.0;
    p.gravityX = 0.1;
    p.gravityY = -0.3;
    p.drag = 1.5;
    p.lifeSeconds = 4.0;
    p.lifeJitter = 0.2;
    p.sizePx = 12.0;
    p.sizeEndFactor = 0.1;
    p.colorStart = 0x00FF8040;
    p.colorEnd = 0x00102030;
    p.additive = false;
    p.forceCode = "vec2 kraft(vec2 pos, vec2 vel, float alter) { return -pos; }";
    p.initCode = "speed=0.5;";
    p.frameCode = "dir=dir+1;";
    p.beatCode = "speed=speed*2;";

    ChainNode root;
    root.params = ListParams{};
    ChainNode child;
    child.params = p;
    root.children.push_back(child);

    const ChainNode back = nodeFromJson(nodeToJson(root), nullptr);
    REQUIRE(back.children.size() == 1);
    const auto* q = std::get_if<GpuParticlesParams>(&back.children[0].params);
    REQUIRE(q != nullptr);
    CHECK(q->count == p.count);
    CHECK(q->spawnX == doctest::Approx(p.spawnX));
    CHECK(q->spawnY == doctest::Approx(p.spawnY));
    CHECK(q->spawnSpread == doctest::Approx(p.spawnSpread));
    CHECK(q->speed == doctest::Approx(p.speed));
    CHECK(q->direction == doctest::Approx(p.direction));
    CHECK(q->fan == doctest::Approx(p.fan));
    CHECK(q->gravityX == doctest::Approx(p.gravityX));
    CHECK(q->gravityY == doctest::Approx(p.gravityY));
    CHECK(q->drag == doctest::Approx(p.drag));
    CHECK(q->lifeSeconds == doctest::Approx(p.lifeSeconds));
    CHECK(q->lifeJitter == doctest::Approx(p.lifeJitter));
    CHECK(q->sizePx == doctest::Approx(p.sizePx));
    CHECK(q->sizeEndFactor == doctest::Approx(p.sizeEndFactor));
    CHECK(q->colorStart == p.colorStart);
    CHECK(q->colorEnd == p.colorEnd);
    CHECK(q->additive == p.additive);
    CHECK(q->forceCode == p.forceCode);
    CHECK(q->initCode == p.initCode);
    CHECK(q->frameCode == p.frameCode);
    CHECK(q->beatCode == p.beatCode);
    CHECK(effectTypeKey(back.children[0].params) == "gpuParticles");
}

TEST_CASE("GpuParticles: Leser klemmt Anzahl und Lebensdauer-Streuung")
{
    using namespace lumi::multieffect;
    ChainNode root;
    root.params = ListParams{};
    ChainNode child;
    GpuParticlesParams p;
    p.count = 999999;
    p.lifeJitter = 3.0;
    child.params = p;
    root.children.push_back(child);
    const ChainNode back = nodeFromJson(nodeToJson(root), nullptr);
    const auto* q = std::get_if<GpuParticlesParams>(&back.children[0].params);
    REQUIRE(q != nullptr);
    CHECK(q->count == kMaxCount);
    CHECK(q->lifeJitter == doctest::Approx(1.0));
}
