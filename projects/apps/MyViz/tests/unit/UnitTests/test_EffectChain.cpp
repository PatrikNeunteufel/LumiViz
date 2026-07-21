/**
 ****************************************************************************************
 * @file   test_EffectChain.cpp
 * @brief  Tests fuer das GL-freie Ketten-Datenmodell des Multieffekt-Hosts
 *         (Import-Phase Roadmap 5.1) — Baumaufbau, Compile-Pass, Zaehlung
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/EffectChain.hpp"

using namespace lumi::multieffect;

namespace
{

ChainNode makeList()
{
    ChainNode node;
    node.params = ListParams{};
    return node;
}

ChainNode makeLeaf(EffectParams params)
{
    ChainNode node;
    node.params = std::move(params);
    return node;
}

} // namespace

TEST_SUITE("EffectChain")
{
    TEST_CASE("leerer Listen-Root compiliert ok ohne Warnungen")
    {
        ChainNode root = makeList();
        const CompileResult result = compileChain(root);
        CHECK(result.ok);
        CHECK(result.warnings.empty());
        CHECK(nodeCount(root) == 0);
        CHECK(root.isList());
    }

    TEST_CASE("Nicht-Listen-Root wird abgelehnt (ok=false)")
    {
        ChainNode root = makeLeaf(InvertParams{});
        const CompileResult result = compileChain(root);
        CHECK_FALSE(result.ok);
        REQUIRE(result.warnings.size() == 1);
        CHECK(result.warnings[0].path == "root");
    }

    TEST_CASE("Compile-Pass fuellt leere displayNames aus dem Typ")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(ClearParams{}));
        root.children.push_back(makeLeaf(FadeoutParams{}));
        root.children.push_back(makeLeaf(DebugBarsParams{}));

        REQUIRE(compileChain(root).ok);
        CHECK(root.displayName == "Effect List");
        CHECK(root.children[0].displayName == "Clear");
        CHECK(root.children[1].displayName == "Fadeout");
        CHECK(root.children[2].displayName == "Debug Bars");
    }

    TEST_CASE("gesetzte displayNames bleiben unangetastet")
    {
        ChainNode root = makeList();
        ChainNode leaf = makeLeaf(InvertParams{});
        leaf.displayName = "Mein Invert";
        root.children.push_back(std::move(leaf));

        REQUIRE(compileChain(root).ok);
        CHECK(root.children[0].displayName == "Mein Invert");
    }

    TEST_CASE("Kinder an einem Blatt-Knoten ergeben pfad-praefixierte Warnung")
    {
        ChainNode root = makeList();
        ChainNode leaf = makeLeaf(InvertParams{});
        leaf.children.push_back(makeLeaf(ClearParams{}));
        root.children.push_back(makeList());
        root.children.push_back(std::move(leaf));

        const CompileResult result = compileChain(root);
        CHECK(result.ok);  // Warnung, kein Hard-Fail (AVS-Philosophie)
        REQUIRE(result.warnings.size() == 1);
        CHECK(result.warnings[0].path == "root/1");
        CHECK(result.warnings[0].text.find("Invert") != std::string::npos);
    }

    TEST_CASE("fadeLen wird auf den AVS-Bereich 0..92 geklammert")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(FadeoutParams{500, 0}));
        root.children.push_back(makeLeaf(FadeoutParams{-3, 0}));

        REQUIRE(compileChain(root).ok);
        CHECK(std::get<FadeoutParams>(root.children[0].params).fadeLen == 92);
        CHECK(std::get<FadeoutParams>(root.children[1].params).fadeLen == 0);
    }

    TEST_CASE("nodeCount zaehlt rekursiv ohne Root (AvsParser-Regel)")
    {
        ChainNode root = makeList();
        ChainNode inner = makeList();
        inner.children.push_back(makeLeaf(InvertParams{}));
        inner.children.push_back(makeLeaf(FadeoutParams{}));
        root.children.push_back(std::move(inner));
        root.children.push_back(makeLeaf(ClearParams{}));

        REQUIRE(compileChain(root).ok);
        CHECK(nodeCount(root) == 4);  // Liste + 2 Kinder + Clear
    }

    TEST_CASE("verschachtelte Warnungspfade sind korrekt")
    {
        ChainNode root = makeList();
        ChainNode inner = makeList();
        ChainNode bad = makeLeaf(PassthroughParams{77, "unbekannt"});
        bad.children.push_back(makeLeaf(InvertParams{}));
        inner.children.push_back(std::move(bad));
        root.children.push_back(std::move(inner));

        const CompileResult result = compileChain(root);
        CHECK(result.ok);
        REQUIRE(result.warnings.size() == 1);
        CHECK(result.warnings[0].path == "root/0/0");
    }

    TEST_CASE("Passthrough konserviert Quell-ID und Notiz")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(PassthroughParams{42, "Effekt 42 fehlt"}));

        REQUIRE(compileChain(root).ok);
        const auto& pass = std::get<PassthroughParams>(root.children[0].params);
        CHECK(pass.sourceId == 42);
        CHECK(pass.note == "Effekt 42 fehlt");
    }

    // --- 5.2: Blend-Modi, nodeId, OnBeat/Adjustable-Clamps -------------------

    TEST_CASE("Alle 14 AVS-Blend-Modi gelten als implementiert (Batch 2)")
    {
        CHECK(isBlendModeImplemented(BlendMode::Ignore));
        CHECK(isBlendModeImplemented(BlendMode::Replace));
        CHECK(isBlendModeImplemented(BlendMode::FiftyFifty));
        CHECK(isBlendModeImplemented(BlendMode::Maximum));
        CHECK(isBlendModeImplemented(BlendMode::Additive));
        CHECK(isBlendModeImplemented(BlendMode::Subtractive12));
        CHECK(isBlendModeImplemented(BlendMode::Subtractive21));
        CHECK(isBlendModeImplemented(BlendMode::EveryOtherLine));
        CHECK(isBlendModeImplemented(BlendMode::EveryOtherPixel));
        CHECK(isBlendModeImplemented(BlendMode::Xor));
        CHECK(isBlendModeImplemented(BlendMode::Adjustable));
        CHECK(isBlendModeImplemented(BlendMode::Multiply));
        CHECK(isBlendModeImplemented(BlendMode::Buffer));
        CHECK(isBlendModeImplemented(BlendMode::Minimum));
    }

    TEST_CASE("Enum-Wert entspricht der AVS-Reihenfolge (Shader-uMode)")
    {
        CHECK(static_cast<int>(BlendMode::Replace) == 1);
        CHECK(static_cast<int>(BlendMode::Additive) == 4);
        CHECK(static_cast<int>(BlendMode::Adjustable) == 10);
        CHECK(static_cast<int>(BlendMode::Minimum) == 13);
    }

    TEST_CASE("Das vollstaendige Blend-Set kompiliert warnungsfrei")
    {
        // Batch 2: jeder frueher exotische Modus wird jetzt gerendert -> keine
        // Fallback-Warnung mehr, egal ob als In- oder Out-Blend.
        for (int m = 0; m <= 13; ++m)
        {
            ChainNode root = makeList();
            ChainNode list = makeList();
            auto& lp = std::get<ListParams>(list.params);
            lp.blendIn = static_cast<BlendMode>(m);
            lp.blendOut = static_cast<BlendMode>(m);
            root.children.push_back(std::move(list));

            const CompileResult result = compileChain(root);
            CHECK(result.ok);
            CHECK(result.warnings.empty());
        }
    }

    TEST_CASE("Adjustable-Alpha und OnBeat-Frames werden geklammert")
    {
        ChainNode root = makeList();
        ChainNode list = makeList();
        auto& lp = std::get<ListParams>(list.params);
        lp.inAdjustAlpha = 999;
        lp.outAdjustAlpha = -5;
        lp.onBeatFrames = 0;
        root.children.push_back(std::move(list));

        REQUIRE(compileChain(root).ok);
        const auto& out = std::get<ListParams>(root.children[0].params);
        CHECK(out.inAdjustAlpha == 255);
        CHECK(out.outAdjustAlpha == 0);
        CHECK(out.onBeatFrames == 1);
    }

    TEST_CASE("Fractal-2D-Parameter werden auf gueltige Bereiche geklammert")
    {
        ChainNode root = makeList();
        ChainNode leaf;
        Fractal2DParams fp;
        fp.type = 42;      // out of range -> 0..8
        fp.maxIter = 99999;  // -> 1..2048
        fp.zoom = -3.0f;   // -> >= 1e-6
        fp.power = 100.0f; // -> 1..16
        fp.escapeR = 0.1f; // -> >= 1
        fp.blend = 9;      // -> 0..2
        leaf.params = fp;
        root.children.push_back(std::move(leaf));

        REQUIRE(compileChain(root).ok);
        const auto& out = std::get<Fractal2DParams>(root.children[0].params);
        CHECK(out.type == 8);
        CHECK(out.maxIter == 2048);
        CHECK(out.zoom >= 1e-6f);
        CHECK(out.power == doctest::Approx(16.0f));
        CHECK(out.escapeR == doctest::Approx(1.0f));
        CHECK(out.blend == 2);
        CHECK(std::string(effectTypeName(EffectParams{Fractal2DParams{}})) == "Fractal 2D");
    }

    TEST_CASE("Domain-Warp-Parameter werden auf gueltige Bereiche geklammert")
    {
        ChainNode root = makeList();
        ChainNode leaf;
        DomainWarpParams dp;
        dp.octaves = 99;  // -> 1..10
        dp.blend = -3;    // -> 0..2
        leaf.params = dp;
        root.children.push_back(std::move(leaf));

        REQUIRE(compileChain(root).ok);
        const auto& out = std::get<DomainWarpParams>(root.children[0].params);
        CHECK(out.octaves == 10);
        CHECK(out.blend == 0);
        CHECK(std::string(effectTypeName(EffectParams{DomainWarpParams{}})) == "Domain Warp");
    }

    TEST_CASE("Batch-H Restmodule liefern Anzeigenamen + Bereichsklemmen")
    {
        CHECK(std::string(effectTypeName(EffectParams{Fractal3DParams{}})) == "Fractal 3D");
        CHECK(std::string(effectTypeName(EffectParams{LyapunovParams{}})) == "Lyapunov");
        CHECK(std::string(effectTypeName(EffectParams{KleinianParams{}})) == "Kleinian");
        CHECK(std::string(effectTypeName(EffectParams{FractalZoomerParams{}})) == "Fractal Zoomer");
        CHECK(std::string(effectTypeName(EffectParams{StrangeAttractorParams{}})) == "Strange Attractor");
        CHECK(std::string(effectTypeName(EffectParams{FlameParams{}})) == "Flame");
        CHECK(std::string(effectTypeName(EffectParams{ReactionDiffusionParams{}})) == "Reaction Diffusion");
        CHECK(std::string(effectTypeName(EffectParams{SetRenderModeParams{}})) == "Set Render Mode");

        ChainNode root = makeList();
        ChainNode f3; Fractal3DParams f3p; f3p.type = 9; f3p.maxSteps = 9999; f3p.maxIter = 999;
        f3.params = f3p; root.children.push_back(std::move(f3));
        ChainNode kl; KleinianParams klp; klp.p = 99; klp.q = 1; kl.params = klp;
        root.children.push_back(std::move(kl));
        ChainNode fl; FlameParams flp; flp.functions = 9; flp.variation = 9; fl.params = flp;
        root.children.push_back(std::move(fl));

        REQUIRE(compileChain(root).ok);
        const auto& o3 = std::get<Fractal3DParams>(root.children[0].params);
        CHECK(o3.type == 4);
        CHECK(o3.maxSteps == 512);
        CHECK(o3.maxIter == 64);
        const auto& okl = std::get<KleinianParams>(root.children[1].params);
        CHECK(okl.p == 20);
        CHECK(okl.q == 3);
        const auto& ofl = std::get<FlameParams>(root.children[2].params);
        CHECK(ofl.functions == 4);
        CHECK(ofl.variation == 4);
    }

    // --- 5.3: neue Transform-Effekte -----------------------------------------

    TEST_CASE("neue Effekt-Typen liefern korrekte Anzeigenamen")
    {
        CHECK(std::string(effectTypeName(EffectParams{BrightnessParams{}})) == "Brightness");
        CHECK(std::string(effectTypeName(EffectParams{FastBrightnessParams{}})) == "Fast Brightness");
        CHECK(std::string(effectTypeName(EffectParams{BlurParams{}})) == "Blur");
        CHECK(std::string(effectTypeName(EffectParams{MirrorParams{}})) == "Mirror");
        CHECK(std::string(effectTypeName(EffectParams{OnBeatClearParams{}})) == "OnBeat Clear");
        CHECK(std::string(effectTypeName(EffectParams{ColorfadeParams{}})) == "Colorfade");
    }

    TEST_CASE("Brightness-Parameter werden auf AVS-Bereiche geklammert")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(BrightnessParams{9000, -9000, 0, false, 0, 900}));

        REQUIRE(compileChain(root).ok);
        const auto& b = std::get<BrightnessParams>(root.children[0].params);
        CHECK(b.red == 4096);
        CHECK(b.green == -4096);
        CHECK(b.distance == 255);
    }

    TEST_CASE("Fast Brightness dir und Blur strength werden geklammert")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(FastBrightnessParams{7}));
        root.children.push_back(makeLeaf(BlurParams{9, true}));
        root.children.push_back(makeLeaf(BlurParams{0, true}));

        REQUIRE(compileChain(root).ok);
        CHECK(std::get<FastBrightnessParams>(root.children[0].params).dir == 2);
        CHECK(std::get<BlurParams>(root.children[1].params).strength == 3);
        CHECK(std::get<BlurParams>(root.children[2].params).strength == 1);
    }

    TEST_CASE("Colorfade-Fader und OnBeat-Clear-N werden geklammert")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(ColorfadeParams{99, -99, 0, 0, 0, 0, 0}));
        root.children.push_back(makeLeaf(OnBeatClearParams{0, 0, false}));

        REQUIRE(compileChain(root).ok);
        const auto& cf = std::get<ColorfadeParams>(root.children[0].params);
        CHECK(cf.faderR == 32);
        CHECK(cf.faderG == -32);
        CHECK(cf.onBeatFrames == 1);
        CHECK(std::get<OnBeatClearParams>(root.children[1].params).everyNBeats == 1);
    }

    // --- 5.4: Skript-Modul-Effekte -------------------------------------------

    TEST_CASE("5.4-Effekt-Typen liefern korrekte Anzeigenamen")
    {
        CHECK(std::string(effectTypeName(EffectParams{ColorModifierParams{}})) == "Color Modifier");
        CHECK(std::string(effectTypeName(EffectParams{MovementParams{}})) == "Movement");
        CHECK(std::string(effectTypeName(EffectParams{DynamicMovementParams{}})) == "Dynamic Movement");
        CHECK(std::string(effectTypeName(EffectParams{CustomBpmParams{}})) == "Custom BPM");
    }

    TEST_CASE("Dynamic-Movement-Gitter wird auf Modul-Grenzen geklammert")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(DynamicMovementParams{"", "", "", "", 999, 0, false, false}));

        REQUIRE(compileChain(root).ok);
        const auto& d = std::get<DynamicMovementParams>(root.children[0].params);
        CHECK(d.xres == 96);
        CHECK(d.yres == 2);
    }

    TEST_CASE("Custom-BPM-Parameter werden geklammert")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(CustomBpmParams{true, 0, true, 0, false}));

        REQUIRE(compileChain(root).ok);
        const auto& b = std::get<CustomBpmParams>(root.children[0].params);
        CHECK(b.arbitraryMs == 1);
        CHECK(b.skipCount == 1);
    }

    TEST_CASE("Feedback-/Buffer-Effekt-Typen liefern korrekte Anzeigenamen")
    {
        CHECK(std::string(effectTypeName(EffectParams{BlitterFeedbackParams{}})) == "Blitter Feedback");
        CHECK(std::string(effectTypeName(EffectParams{RotoBlitterParams{}})) == "Roto Blitter");
        CHECK(std::string(effectTypeName(EffectParams{BufferSaveParams{}})) == "Buffer Save");
        CHECK(std::string(effectTypeName(EffectParams{MosaicParams{}})) == "Mosaic");
        CHECK(std::string(effectTypeName(EffectParams{GrainParams{}})) == "Grain");
        CHECK(std::string(effectTypeName(EffectParams{ScatterParams{}})) == "Scatter");
        CHECK(std::string(effectTypeName(EffectParams{InterferencesParams{}})) == "Interferences");
        CHECK(std::string(effectTypeName(EffectParams{WaterParams{}})) == "Water");
        CHECK(std::string(effectTypeName(EffectParams{BumpParams{}})) == "Bump");
        CHECK(std::string(effectTypeName(EffectParams{WaterBumpParams{}})) == "Water Bump");
        CHECK(std::string(effectTypeName(EffectParams{StarfieldParams{}})) == "Starfield");
        CHECK(std::string(effectTypeName(EffectParams{TimescopeParams{}})) == "Timescope");
        CHECK(std::string(effectTypeName(EffectParams{DotGridParams{}})) == "Dot Grid");
        CHECK(std::string(effectTypeName(EffectParams{DotPlaneParams{}})) == "Dot Plane");
        CHECK(std::string(effectTypeName(EffectParams{DotFountainParams{}})) == "Dot Fountain");
        CHECK(std::string(effectTypeName(EffectParams{ChannelShiftParams{}})) == "Channel Shift");
        CHECK(std::string(effectTypeName(EffectParams{ColorReductionParams{}})) == "Color Reduction");
        CHECK(std::string(effectTypeName(EffectParams{MultiplierParams{}})) == "Multiplier");
        CHECK(std::string(effectTypeName(EffectParams{VideoDelayParams{}})) == "Video Delay");
        CHECK(std::string(effectTypeName(EffectParams{MultiDelayParams{}})) == "Multi Delay");
    }

    TEST_CASE("Buffer-Save-Slot/Alpha werden geklammert (Restore-Blend jetzt implementiert)")
    {
        ChainNode root = makeList();
        root.children.push_back(makeLeaf(BufferSaveParams{99, 1, BlendMode::Xor, 999}));

        const CompileResult result = compileChain(root);
        CHECK(result.ok);
        const auto& s = std::get<BufferSaveParams>(root.children[0].params);
        CHECK(s.slot == 7);
        CHECK(s.adjustAlpha == 255);
        CHECK(result.warnings.empty());  // XOR ist seit Batch 2 implementiert
    }

    TEST_CASE("SuperScope: Anzeigename + Parameter-Clamps")
    {
        CHECK(std::string(effectTypeName(EffectParams{SuperScopeParams{}})) == "SuperScope");

        ChainNode root = makeList();
        SuperScopeParams p;
        p.pointCount = 99999;
        p.renderMode = 7;
        p.audioChannel = 9;
        p.lineWidth = 100.0f;
        p.dotSize = 0.1f;
        root.children.push_back(makeLeaf(p));

        REQUIRE(compileChain(root).ok);
        const auto& s = std::get<SuperScopeParams>(root.children[0].params);
        CHECK(s.pointCount == 4096);
        CHECK(s.renderMode == 2);
        CHECK(s.audioChannel == 4);
        CHECK(s.lineWidth == doctest::Approx(20.0f));
        CHECK(s.dotSize == doctest::Approx(1.0f));
    }

    TEST_CASE("compileChain vergibt eindeutige, stabile nodeIds")
    {
        ChainNode root = makeList();
        ChainNode inner = makeList();
        inner.children.push_back(makeLeaf(InvertParams{}));
        root.children.push_back(std::move(inner));
        root.children.push_back(makeLeaf(ClearParams{}));

        REQUIRE(compileChain(root).ok);
        const uint64_t rootId = root.nodeId;
        const uint64_t innerId = root.children[0].nodeId;
        const uint64_t invertId = root.children[0].children[0].nodeId;
        const uint64_t clearId = root.children[1].nodeId;

        CHECK(rootId != 0);
        CHECK(innerId != 0);
        CHECK(invertId != 0);
        CHECK(clearId != 0);
        CHECK(rootId != innerId);
        CHECK(innerId != invertId);
        CHECK(invertId != clearId);

        // Bestehende IDs bleiben ueber einen erneuten Compile stabil ...
        compileChain(root);
        CHECK(root.nodeId == rootId);
        CHECK(root.children[0].nodeId == innerId);

        // ... und ein neuer Knoten bekommt eine frische, kollisionsfreie ID.
        root.children.push_back(makeLeaf(FadeoutParams{}));
        REQUIRE(compileChain(root).ok);
        const uint64_t freshId = root.children[2].nodeId;
        CHECK(freshId != 0);
        CHECK(freshId != rootId);
        CHECK(freshId != innerId);
        CHECK(freshId != invertId);
        CHECK(freshId != clearId);
    }
}
