/**
 ****************************************************************************************
 * @file   test_MeshWarpWrapper.cpp
 * @brief  Unit-Tests fuer den Mesh-Warp-Wrapper (Strang G1, Session 69):
 *         Vertex-Wrapper-Vertrag (#line, Identitaets-Fallback), Gitter-
 *         Erzeugung (Vertex-/Index-Zahlen, Randwerte, Windung) und der
 *         Serializer-Roundtrip des neuen Knotens
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/MeshWarpWrapper.hpp"

#include <QJsonObject>

#include <string>
#include <vector>

using namespace lumi::meshwarp;

// =============================================================================
// Vertex-Wrapper
// =============================================================================

TEST_CASE("MeshWarp: Wrapper traegt #line 1 vor dem Nutzer-Code")
{
    const std::string userCode = "vec2 warp(vec2 uv) { return uv + 0.1; }";
    const std::string wrapped = wrapVertex(userCode);
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

TEST_CASE("MeshWarp: leerer Code wird zur Identitaet")
{
    const std::string wrapped = wrapVertex("");
    CHECK(wrapped.find("vec2 warp(vec2 uv) { return uv; }") != std::string::npos);
}

TEST_CASE("MeshWarp: Uniform-Satz im Praeludium (Vertrag der Referenzseite)")
{
    const std::string wrapped = wrapVertex("vec2 warp(vec2 uv) { return uv; }");
    for (const char* u : {"uniform vec2 uResolution", "uniform float uTime",
                          "uniform float uDelta", "uniform int uFrame",
                          "uniform float bass", "uniform float mid",
                          "uniform float treb", "uniform float vol",
                          "uniform float beat"})
    {
        CAPTURE(u);
        CHECK(wrapped.find(u) != std::string::npos);
    }
}

TEST_CASE("MeshWarp: Starter definiert warp() und nutzt Audio")
{
    const std::string starter = starterWarp();
    CHECK(starter.find("vec2 warp(vec2 uv)") != std::string::npos);
    CHECK(starter.find("bass") != std::string::npos);
}

// =============================================================================
// Gitter-Erzeugung
// =============================================================================

TEST_CASE("MeshWarp: Gitter-Zahlen stimmen ((gx+1)*(gy+1) Punkte, 6*gx*gy Indizes)")
{
    const int gx = 5, gy = 3;
    const std::vector<float> v = buildGridVertices(gx, gy);
    const std::vector<unsigned int> idx = buildGridIndices(gx, gy);
    CHECK(v.size() == static_cast<std::size_t>((gx + 1) * (gy + 1) * 2));
    CHECK(idx.size() == static_cast<std::size_t>(gx * gy * 6));
    // Eckwerte: erster Punkt (0,0), letzter (1,1)
    CHECK(v.front() == doctest::Approx(0.0f));
    CHECK(v[1] == doctest::Approx(0.0f));
    CHECK(v[v.size() - 2] == doctest::Approx(1.0f));
    CHECK(v.back() == doctest::Approx(1.0f));
    // Jeder Index zeigt auf einen existierenden Punkt
    const auto points = static_cast<unsigned int>((gx + 1) * (gy + 1));
    for (const unsigned int i : idx) CHECK(i < points);
}

TEST_CASE("MeshWarp: Minimal-Gitter und Ungueltiges")
{
    CHECK(buildGridVertices(1, 1).size() == 8);   // 4 Punkte
    CHECK(buildGridIndices(1, 1).size() == 6);    // 2 Dreiecke
    CHECK(buildGridVertices(0, 4).empty());
    CHECK(buildGridIndices(4, 0).empty());
}

TEST_CASE("MeshWarp: Dreiecks-Windung ist konsistent (erstes Quad)")
{
    // Beide Dreiecke eines Quads muessen dieselbe Windung haben — sonst
    // wuerde Face-Culling (falls je aktiv) die Haelfte des Bilds fressen.
    const std::vector<float> v = buildGridVertices(2, 2);
    const std::vector<unsigned int> idx = buildGridIndices(2, 2);
    const auto signedArea = [&](unsigned int a, unsigned int b, unsigned int c) {
        const float ax = v[a * 2], ay = v[a * 2 + 1];
        const float bx = v[b * 2], by = v[b * 2 + 1];
        const float cx = v[c * 2], cy = v[c * 2 + 1];
        return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    };
    const float t1 = signedArea(idx[0], idx[1], idx[2]);
    const float t2 = signedArea(idx[3], idx[4], idx[5]);
    CHECK(t1 * t2 > 0.0f);
}

// =============================================================================
// Serializer-Roundtrip
// =============================================================================

TEST_CASE("MeshWarp: ChainSerializer-Roundtrip erhaelt alle Felder")
{
    using namespace lumi::multieffect;
    MeshWarpParams p;
    p.code = "vec2 warp(vec2 uv) { return uv * 0.9; }";
    p.gridX = 128;
    p.gridY = 100;
    p.mixAmount = 0.75;
    p.wrapUv = false;
    p.initCode = "mixamount=1;";
    p.frameCode = "gridx=64+32*bass;";
    p.beatCode = "gridy=96;";

    ChainNode root;
    root.params = ListParams{};
    ChainNode child;
    child.params = p;
    child.displayName = "Warp-Test";
    root.children.push_back(child);

    const QJsonObject json = nodeToJson(root);
    const ChainNode back = nodeFromJson(json, nullptr);
    REQUIRE(back.children.size() == 1);
    const auto* q = std::get_if<MeshWarpParams>(&back.children[0].params);
    REQUIRE(q != nullptr);
    CHECK(q->code == p.code);
    CHECK(q->gridX == p.gridX);
    CHECK(q->gridY == p.gridY);
    CHECK(q->mixAmount == doctest::Approx(p.mixAmount));
    CHECK(q->wrapUv == p.wrapUv);
    CHECK(q->initCode == p.initCode);
    CHECK(q->frameCode == p.frameCode);
    CHECK(q->beatCode == p.beatCode);
    CHECK(effectTypeKey(back.children[0].params) == "meshWarp");
}

TEST_CASE("MeshWarp: Leser klemmt Gitter und Mix")
{
    using namespace lumi::multieffect;
    ChainNode root;
    root.params = ListParams{};
    ChainNode child;
    MeshWarpParams p;
    p.gridX = 9999;   // Serialisieren schreibt roh …
    p.gridY = -5;
    p.mixAmount = 7.0;
    child.params = p;
    root.children.push_back(child);
    const ChainNode back = nodeFromJson(nodeToJson(root), nullptr);
    const auto* q = std::get_if<MeshWarpParams>(&back.children[0].params);
    REQUIRE(q != nullptr);
    // … der Leser klemmt auf die Wrapper-Grenzen
    CHECK(q->gridX == kMaxGridX);
    CHECK(q->gridY == kMinGrid);
    CHECK(q->mixAmount == doctest::Approx(1.0));
}
