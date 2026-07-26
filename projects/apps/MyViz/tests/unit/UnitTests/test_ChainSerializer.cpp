/**
 ****************************************************************************************
 * @file   test_ChainSerializer.cpp
 * @brief  Round-Trip-Tests fuer die Multieffekt-Ketten-JSON-Persistenz (Roadmap 5.6)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <filesystem>

using namespace lumi::multieffect;

namespace
{

std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

ChainNode buildRichChain()
{
    ChainNode root;
    ListParams rootList;
    rootList.clearEveryFrame = true;
    root.params = rootList;

    ChainNode fade;
    fade.params = FadeoutParams{20, 0x123456};
    root.children.push_back(std::move(fade));

    ChainNode scope;
    SuperScopeParams sp;
    sp.pointCode = "x=i;y=v";
    sp.frameCode = "t=t+0.1";
    sp.pointCount = 512;
    sp.renderMode = 2;
    sp.lineWidth = 3.5f;
    sp.audioChannel = 1;
    scope.params = std::move(sp);
    root.children.push_back(std::move(scope));

    ChainNode nested;
    ListParams nestedList;
    nestedList.blendIn = BlendMode::Buffer;
    nestedList.blendOut = BlendMode::FiftyFifty;
    nestedList.inAdjustAlpha = 200;
    nestedList.bufferIn = 4;
    nestedList.bufferOut = 2;
    nestedList.bufferInInvert = true;
    nestedList.bufferOutInvert = false;
    nestedList.onBeatRender = true;
    nestedList.onBeatFrames = 5;
    nestedList.useCode = true;
    nestedList.frameCode = "enabled=1";
    nested.params = nestedList;

    ChainNode mod;
    ColorModifierParams cm;
    cm.levelCode = "red=red*0.5";
    cm.recompute = false;
    mod.params = std::move(cm);
    nested.children.push_back(std::move(mod));

    root.children.push_back(std::move(nested));

    compileChain(root);
    return root;
}

} // namespace

TEST_SUITE("ChainSerializer")
{
    TEST_CASE("typeKey ist stabil und eindeutig fuer die Kern-Typen")
    {
        CHECK(effectTypeKey(EffectParams{ListParams{}}) == "list");
        CHECK(effectTypeKey(EffectParams{SuperScopeParams{}}) == "superScope");
        CHECK(effectTypeKey(EffectParams{ColorModifierParams{}}) == "colorModifier");
        CHECK(effectTypeKey(EffectParams{MosaicParams{}}) == "mosaic");
        CHECK(effectTypeKey(EffectParams{PassthroughParams{}}) == "passthrough");
    }

    TEST_CASE("Host-Gruppe ueberlebt den Round-Trip (HG1, .lvfx2-Kennzeichen)")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode group;
        HostGroupParams hg;
        hg.blendOut = BlendMode::Additive;
        hg.outAdjustAlpha = 99;
        hg.crossfadeSeconds = 3.5;
        hg.curveIn = 0;
        hg.curveOut = 0;
        hg.sourceFile = "presets/alt.lvfx";
        group.params = hg;
        ChainNode leaf;
        leaf.params = SuperScopeParams{};
        group.children.push_back(std::move(leaf));
        root.children.push_back(std::move(group));

        CHECK(effectTypeKey(EffectParams{HostGroupParams{}}) == "hostgroup");
        CHECK(chainHasHostGroup(root));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const ChainNode& g = restored.children[0];
        REQUIRE(g.isHostGroup());
        const auto& p = std::get<HostGroupParams>(g.params);
        CHECK(p.blendOut == BlendMode::Additive);
        CHECK(p.outAdjustAlpha == 99);
        CHECK(p.crossfadeSeconds == doctest::Approx(3.5));
        CHECK(p.sourceFile == "presets/alt.lvfx");
        // children der Gruppe ueberleben (Container wie eine Liste)
        REQUIRE(g.children.size() == 1);
        CHECK(std::holds_alternative<SuperScopeParams>(g.children[0].params));
    }

    TEST_CASE("Tiefenregel: verschachtelte Host-Gruppe wird zur Liste degradiert")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode outer;
        outer.params = HostGroupParams{};
        ChainNode inner;
        HostGroupParams innerHg;
        innerHg.blendOut = BlendMode::Maximum;
        inner.params = innerHg;
        ChainNode leaf;
        leaf.params = GrainParams{};
        inner.children.push_back(std::move(leaf));
        outer.children.push_back(std::move(inner));
        root.children.push_back(std::move(outer));

        const CompileResult r = compileChain(root);
        bool warned = false;
        for (const auto& w : r.warnings)
        {
            if (w.text.find("nested host group") != std::string::npos) warned = true;
        }
        CHECK(warned);
        const ChainNode& restoredInner = root.children[0].children[0];
        REQUIRE(restoredInner.isList());  // degradiert, nicht geloescht
        const auto& asList = std::get<ListParams>(restoredInner.params);
        CHECK(asList.blendOut == BlendMode::Maximum);  // blendOut zog um
        REQUIRE(restoredInner.children.size() == 1);   // Kinder bleiben
    }

    TEST_CASE("SuperScope-Farbfelder ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        SuperScopeParams sp;
        sp.colorBlend = 3;  // multiply
        sp.colorCycleFrames = 42;
        sp.gradientPreset = "Fire";
        sp.colors = {0x112233u, 0x445566u, 0x778899u};
        leaf.params = sp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<SuperScopeParams>(restored.children[0].params);
        CHECK(p.colorBlend == 3);
        CHECK(p.colorCycleFrames == 42);
        CHECK(p.gradientPreset == "Fire");
        REQUIRE(p.colors.size() == 3);
        CHECK(p.colors[0] == 0x112233u);
        CHECK(p.colors[2] == 0x778899u);
    }

    TEST_CASE("Grain/Scatter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode g;  g.params = GrainParams{40, true, 1};
        ChainNode s;  s.params = ScatterParams{};
        root.children.push_back(std::move(g));
        root.children.push_back(std::move(s));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 2);
        const auto& gp = std::get<GrainParams>(restored.children[0].params);
        CHECK(gp.amount == 40);
        CHECK(gp.staticGrain == true);
        CHECK(gp.blend == 1);
        CHECK(std::holds_alternative<ScatterParams>(restored.children[1].params));
        CHECK(effectTypeKey(EffectParams{ScatterParams{}}) == "scatter");
        CHECK(effectTypeKey(EffectParams{GrainParams{}}) == "grain");
        CHECK(effectTypeKey(EffectParams{WaterParams{}}) == "water");
        // Water round-trips as a bare type key
        ChainNode wroot; wroot.params = ListParams{};
        ChainNode w; w.params = WaterParams{}; wroot.children.push_back(std::move(w));
        const ChainNode wrestored = chainFromJson(chainToJson(wroot), nullptr);
        REQUIRE(wrestored.children.size() == 1);
        CHECK(std::holds_alternative<WaterParams>(wrestored.children[0].params));
    }

    TEST_CASE("APE-Effekte ueberleben den Round-Trip")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode cs; cs.params = ChannelShiftParams{4, true};
        ChainNode cr; cr.params = ColorReductionParams{3};
        ChainNode ml; ml.params = MultiplierParams{5};
        root.children.push_back(std::move(cs));
        root.children.push_back(std::move(cr));
        root.children.push_back(std::move(ml));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 3);
        const auto& c = std::get<ChannelShiftParams>(r.children[0].params);
        CHECK(c.mode == 4);
        CHECK(c.onBeat == true);
        CHECK(std::get<ColorReductionParams>(r.children[1].params).levels == 3);
        CHECK(std::get<MultiplierParams>(r.children[2].params).mode == 5);
        CHECK(effectTypeKey(EffectParams{ColorReductionParams{}}) == "colorReduction");

        // Video Delay round-trip
        ChainNode vroot; vroot.params = ListParams{};
        ChainNode vd; vd.params = VideoDelayParams{true, 42}; vroot.children.push_back(std::move(vd));
        const ChainNode vr = chainFromJson(chainToJson(vroot), nullptr);
        const auto& vp = std::get<VideoDelayParams>(vr.children[0].params);
        CHECK(vp.useBeats == true);
        CHECK(vp.delay == 42);
        CHECK(effectTypeKey(EffectParams{VideoDelayParams{}}) == "videoDelay");

        // Multi Delay round-trip
        ChainNode mroot; mroot.params = ListParams{};
        ChainNode md; md.params = MultiDelayParams{2, 4, 33, true}; mroot.children.push_back(std::move(md));
        const ChainNode mr = chainFromJson(chainToJson(mroot), nullptr);
        const auto& mp = std::get<MultiDelayParams>(mr.children[0].params);
        CHECK(mp.mode == 2);
        CHECK(mp.buffer == 4);
        CHECK(mp.delay == 33);
        CHECK(mp.useBeats == true);
        CHECK(effectTypeKey(EffectParams{MultiDelayParams{}}) == "multiDelay");
    }

    TEST_CASE("Dot-Renderer ueberleben den Round-Trip")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode g; DotGridParams gp; gp.colors = {0x112233u, 0x445566u};
        gp.spacing = 16; gp.xMove = 32; gp.yMove = -32; gp.blend = 2; g.params = gp;
        ChainNode pl; DotPlaneParams pp; pp.colors[0] = 0xABCDEFu; pp.rotVel = 20; pp.angle = -25; pl.params = pp;
        ChainNode fn; DotFountainParams fpp; fpp.colors[4] = 0x010203u; fpp.rotVel = 9; fn.params = fpp;
        root.children.push_back(std::move(g));
        root.children.push_back(std::move(pl));
        root.children.push_back(std::move(fn));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 3);
        const auto& g2 = std::get<DotGridParams>(r.children[0].params);
        CHECK(g2.colors.size() == 2);
        CHECK(g2.colors[0] == 0x112233u);
        CHECK(g2.spacing == 16);
        CHECK(g2.blend == 2);
        const auto& p2 = std::get<DotPlaneParams>(r.children[1].params);
        CHECK(p2.colors[0] == 0xABCDEFu);
        CHECK(p2.rotVel == 20);
        CHECK(p2.angle == -25);
        const auto& f2 = std::get<DotFountainParams>(r.children[2].params);
        CHECK(f2.colors[4] == 0x010203u);
        CHECK(f2.rotVel == 9);
        CHECK(effectTypeKey(EffectParams{DotFountainParams{}}) == "dotFountain");
    }

    TEST_CASE("Timescope-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        leaf.params = TimescopeParams{0x00FF80u, 1, 0, 200};
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<TimescopeParams>(restored.children[0].params);
        CHECK(p.color == 0x00FF80u);
        CHECK(p.blend == 1);
        CHECK(p.channel == 0);
        CHECK(p.bands == 200);
        CHECK(effectTypeKey(EffectParams{TimescopeParams{}}) == "timescope");
    }

    TEST_CASE("Movement-builtinRemap ueberlebt den Round-Trip (S44: fuzzify/blocky)")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        MovementParams mp;
        mp.builtinRemap = 7;   // blocky partial out
        mp.blend = true;
        mp.subpixel = false;   // r_trans schliesst 1/2/7 vom Subpixel aus
        mp.sourceMapped = 2;
        leaf.params = mp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<MovementParams>(restored.children[0].params);
        CHECK(p.builtinRemap == 7);
        CHECK(p.blend == true);
        CHECK(p.subpixel == false);
        CHECK(p.sourceMapped == 2);
        CHECK(p.code.empty());
    }

    TEST_CASE("Text- und AVI-Parameter ueberleben den Round-Trip (S44)")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode textNode;
        TextParams tp;
        tp.text = "HALLO;WELT;;dritte";
        tp.fontFace = "Impact";
        tp.fontHeight = -32;
        tp.fontWeight = 700;
        tp.italic = true;
        tp.color = 0x11FF77u;
        tp.blend = 2;
        tp.onBeat = true;
        tp.onBeatSpeed = 9;
        tp.insertBlank = true;
        tp.randomWord = true;
        tp.hAlign = 2;
        tp.vAlign = 0;
        tp.xShift = -10;
        tp.yShift = 5;
        tp.outline = true;
        tp.outlineColor = 0x102030u;
        tp.outlineSize = 3;
        textNode.params = tp;
        root.children.push_back(std::move(textNode));
        ChainNode aviNode;
        AviParams ap;
        ap.filename = "elvis_war.avi";
        ap.resolvedPath = "C:/x/elvis_war.avi";
        ap.blend = 1;
        ap.adapt = true;
        ap.persist = 12;
        ap.speedMs = 40;
        aviNode.params = ap;
        root.children.push_back(std::move(aviNode));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 2);
        const auto& t = std::get<TextParams>(restored.children[0].params);
        CHECK(t.text == "HALLO;WELT;;dritte");
        CHECK(t.fontFace == "Impact");
        CHECK(t.fontHeight == -32);
        CHECK(t.fontWeight == 700);
        CHECK(t.italic == true);
        CHECK(t.color == 0x11FF77u);
        CHECK(t.blend == 2);
        CHECK(t.onBeat == true);
        CHECK(t.onBeatSpeed == 9);
        CHECK(t.insertBlank == true);
        CHECK(t.randomWord == true);
        CHECK(t.hAlign == 2);
        CHECK(t.vAlign == 0);
        CHECK(t.xShift == -10);
        CHECK(t.yShift == 5);
        CHECK(t.outline == true);
        CHECK(t.outlineColor == 0x102030u);
        CHECK(t.outlineSize == 3);
        const auto& a = std::get<AviParams>(restored.children[1].params);
        CHECK(a.filename == "elvis_war.avi");
        CHECK(a.resolvedPath == "C:/x/elvis_war.avi");
        CHECK(a.blend == 1);
        CHECK(a.adapt == true);
        CHECK(a.persist == 12);
        CHECK(a.speedMs == 40);
        CHECK(effectTypeKey(EffectParams{TextParams{}}) == "text");
        CHECK(effectTypeKey(EffectParams{AviParams{}}) == "avi");
    }

    TEST_CASE("Starfield-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        StarfieldParams sp;
        sp.color = 0x8040FF; sp.warpSpeed = 9.5f; sp.maxStars = 700;
        sp.onBeat = true; sp.beatSpeed = 2.5f; sp.durationFrames = 20;
        leaf.params = sp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<StarfieldParams>(restored.children[0].params);
        CHECK(p.color == 0x8040FFu);
        CHECK(p.warpSpeed == doctest::Approx(9.5f));
        CHECK(p.maxStars == 700);
        CHECK(p.onBeat == true);
        CHECK(p.beatSpeed == doctest::Approx(2.5f));
        CHECK(effectTypeKey(EffectParams{StarfieldParams{}}) == "starfield");
    }

    TEST_CASE("Starfield-Blend + FyrewurX ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode star;
        StarfieldParams sp;
        sp.blend = 2;
        star.params = sp;
        root.children.push_back(std::move(star));
        ChainNode fw;
        FyrewurXParams fp;
        fp.sparks = 200; fp.speed = 1.2f; fp.gravity = 0.5f; fp.lifeSeconds = 2.5f;
        fw.params = fp;
        root.children.push_back(std::move(fw));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 2);
        CHECK(std::get<StarfieldParams>(restored.children[0].params).blend == 2);
        const auto& p = std::get<FyrewurXParams>(restored.children[1].params);
        CHECK(p.sparks == 200);
        CHECK(p.speed == doctest::Approx(1.2f));
        CHECK(p.gravity == doctest::Approx(0.5f));
        CHECK(p.lifeSeconds == doctest::Approx(2.5f));
        CHECK(effectTypeKey(EffectParams{FyrewurXParams{}}) == "fyrewurx");
    }

    TEST_CASE("Water-Bump-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        WaterBumpParams wp;
        wp.density = 7; wp.depth = 900; wp.randomDrop = false;
        wp.dropX = 2; wp.dropY = 0; wp.dropRadius = 55; wp.displaceScale = 9.5f;
        leaf.params = wp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<WaterBumpParams>(restored.children[0].params);
        CHECK(p.density == 7);
        CHECK(p.depth == 900);
        CHECK_FALSE(p.randomDrop);
        CHECK(p.dropX == 2);
        CHECK(p.dropRadius == 55);
        CHECK(p.displaceScale == doctest::Approx(9.5f));
        CHECK(effectTypeKey(EffectParams{WaterBumpParams{}}) == "waterBump");
    }

    TEST_CASE("Bump-Parameter + Licht-Code ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        BumpParams bp;
        bp.depth = 55; bp.depth2 = 80; bp.onBeat = true; bp.durationFrames = 8;
        bp.invert = true; bp.oldStyle = true; bp.blend = 2;
        bp.initCode = "t=0"; bp.frameCode = "x=0.7;y=0.3"; bp.beatCode = "t=0";
        leaf.params = bp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<BumpParams>(restored.children[0].params);
        CHECK(p.depth == 55);
        CHECK(p.onBeat == true);
        CHECK(p.invert == true);
        CHECK(p.oldStyle == true);
        CHECK(p.blend == 2);
        CHECK(p.frameCode == "x=0.7;y=0.3");
        CHECK(effectTypeKey(EffectParams{BumpParams{}}) == "bump");
    }

    TEST_CASE("Dynamic-Shift-Parameter + Code ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        DynamicShiftParams sp;
        sp.initCode = "d=0";
        sp.frameCode = "x=10;y=-5;d=d+0.1";
        sp.beatCode = "d=d+2";
        sp.blend = true;
        sp.bilinear = false;
        leaf.params = sp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<DynamicShiftParams>(restored.children[0].params);
        CHECK(p.initCode == "d=0");
        CHECK(p.frameCode == "x=10;y=-5;d=d+0.1");
        CHECK(p.beatCode == "d=d+2");
        CHECK(p.blend == true);
        CHECK(p.bilinear == false);
        CHECK(effectTypeKey(EffectParams{DynamicShiftParams{}}) == "dynamicShift");
    }

    TEST_CASE("Dynamic-Distance-Modifier-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        DynamicDistanceModifierParams dp;
        dp.initCode = "t=0";
        dp.frameCode = "t=t+1";
        dp.beatCode = "t=0";
        dp.pixelCode = "d=d*0.8";
        dp.blend = true;
        dp.bilinear = false;
        leaf.params = dp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<DynamicDistanceModifierParams>(restored.children[0].params);
        CHECK(p.pixelCode == "d=d*0.8");
        CHECK(p.initCode == "t=0");
        CHECK(p.blend == true);
        CHECK(p.bilinear == false);
        CHECK(effectTypeKey(EffectParams{DynamicDistanceModifierParams{}}) == "dynamicDistanceModifier");
    }

    TEST_CASE("Moving-Particle-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        MovingParticleParams mp;
        mp.color = 0x123456;
        mp.maxDistance = 24;
        mp.size = 10;
        mp.size2 = 30;
        mp.onBeatSize = true;
        mp.blend = 2;
        leaf.params = mp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<MovingParticleParams>(restored.children[0].params);
        CHECK(p.color == 0x123456u);
        CHECK(p.maxDistance == 24);
        CHECK(p.size2 == 30);
        CHECK(p.onBeatSize == true);
        CHECK(p.blend == 2);
        CHECK(effectTypeKey(EffectParams{MovingParticleParams{}}) == "movingParticle");
    }

    TEST_CASE("Color-Map-Parameter + Gradient ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        ColorMapParams cp;
        cp.key = 4;
        cp.blendMode = 7;
        cp.adjustBlend = 200;
        cp.stopPos = {0, 128, 255};
        cp.stopColor = {0x000000, 0x00FF00, 0xFFFFFF};
        leaf.params = cp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<ColorMapParams>(restored.children[0].params);
        CHECK(p.key == 4);
        CHECK(p.blendMode == 7);
        CHECK(p.adjustBlend == 200);
        REQUIRE(p.stopPos.size() == 3);
        CHECK(p.stopPos[1] == 128);
        CHECK(p.stopColor[1] == 0x00FF00u);
        CHECK(effectTypeKey(EffectParams{ColorMapParams{}}) == "colorMap");
    }

    TEST_CASE("Buffer-Blend-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        leaf.params = BufferBlendParams{3, 8, 6};
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<BufferBlendParams>(restored.children[0].params);
        CHECK(p.bufferA == 3);
        CHECK(p.bufferB == 8);
        CHECK(p.mode == 6);
        CHECK(effectTypeKey(EffectParams{BufferBlendParams{}}) == "bufferBlend");
    }

    TEST_CASE("Jheriko-Global-Parameter + Code ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        JherikoGlobalParams jp;
        jp.loadMode = 3;
        jp.initCode = "reg00=0";
        jp.frameCode = "reg00=reg00+1";
        jp.beatCode = "reg01=1";
        leaf.params = jp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<JherikoGlobalParams>(restored.children[0].params);
        CHECK(p.loadMode == 3);
        CHECK(p.initCode == "reg00=0");
        CHECK(p.frameCode == "reg00=reg00+1");
        CHECK(p.beatCode == "reg01=1");
        CHECK(effectTypeKey(EffectParams{JherikoGlobalParams{}}) == "jherikoGlobal");
    }

    TEST_CASE("Color-Clip / Unique-Tone / Interleave ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode a;
        a.params = ColorClipParams{2, 0x112233, 0x445566, 25};
        root.children.push_back(std::move(a));
        ChainNode b;
        UniqueToneParams ut; ut.color = 0x00FF00; ut.invert = true; ut.blend = 1;
        b.params = ut;
        root.children.push_back(std::move(b));
        ChainNode c;
        InterleaveParams il; il.x = 4; il.y = 0; il.color = 0x123456; il.blend = 2;
        il.onBeat = true; il.x2 = 8; il.y2 = 2; il.beatDuration = 6;
        c.params = il;
        root.children.push_back(std::move(c));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 3);
        const auto& pa = std::get<ColorClipParams>(restored.children[0].params);
        CHECK(pa.mode == 2);
        CHECK(pa.clipColor == 0x112233u);
        CHECK(pa.distance == 25);
        const auto& pb = std::get<UniqueToneParams>(restored.children[1].params);
        CHECK(pb.color == 0x00FF00u);
        CHECK(pb.invert == true);
        CHECK(pb.blend == 1);
        const auto& pc = std::get<InterleaveParams>(restored.children[2].params);
        CHECK(pc.x == 4);
        CHECK(pc.y == 0);
        CHECK(pc.onBeat == true);
        CHECK(pc.beatDuration == 6);
        CHECK(effectTypeKey(EffectParams{ColorClipParams{}}) == "colorClip");
        CHECK(effectTypeKey(EffectParams{UniqueToneParams{}}) == "uniqueTone");
        CHECK(effectTypeKey(EffectParams{InterleaveParams{}}) == "interleave");
    }

    TEST_CASE("Convolution / MultiFilter / Add Borders ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode a;
        ConvolutionParams cv;
        cv.edgeMode = 1; cv.absolute = true; cv.twoPass = true; cv.bias = 5; cv.scale = 9;
        cv.kernel[24] = 4; cv.kernel[0] = -1;
        a.params = cv;
        root.children.push_back(std::move(a));
        ChainNode b;
        b.params = MultiFilterParams{2, true};
        root.children.push_back(std::move(b));
        ChainNode c;
        c.params = AddBordersParams{0x123456, 7};
        root.children.push_back(std::move(c));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 3);
        const auto& pa = std::get<ConvolutionParams>(restored.children[0].params);
        CHECK(pa.edgeMode == 1);
        CHECK(pa.twoPass == true);
        CHECK(pa.scale == 9);
        CHECK(pa.kernel[24] == 4);
        CHECK(pa.kernel[0] == -1);
        const auto& pb = std::get<MultiFilterParams>(restored.children[1].params);
        CHECK(pb.effect == 2);
        CHECK(pb.onBeat == true);
        const auto& pc = std::get<AddBordersParams>(restored.children[2].params);
        CHECK(pc.color == 0x123456u);
        CHECK(pc.size == 7);
        CHECK(effectTypeKey(EffectParams{ConvolutionParams{}}) == "convolution");
        CHECK(effectTypeKey(EffectParams{NormaliseParams{}}) == "normalise");
        CHECK(effectTypeKey(EffectParams{MultiFilterParams{}}) == "multiFilter");
        CHECK(effectTypeKey(EffectParams{AddBordersParams{}}) == "addBorders");
    }

    TEST_CASE("Simple-Scope / Bass-Spin ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode a;
        SimpleScopeParams s;
        s.mode = 0; s.channel = 1; s.position = 0;  // solid analyzer (S48)
        s.colors = {0xFF0000, 0x00FF00};
        a.params = s;
        root.children.push_back(std::move(a));
        ChainNode b;
        b.params = BassSpinParams{false, true, 0x111111, 0x222222, 0};
        root.children.push_back(std::move(b));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 2);
        const auto& ps = std::get<SimpleScopeParams>(restored.children[0].params);
        CHECK(ps.mode == 0);
        CHECK(ps.channel == 1);
        REQUIRE(ps.colors.size() == 2);
        CHECK(ps.colors[1] == 0x00FF00u);

        // Alt-Dokument (vor S48): source/drawMode werden auf mode gemappt.
        QJsonObject legacy;
        legacy["type"] = "simpleScope";
        legacy["source"] = 1;    // waveform
        legacy["drawMode"] = 0;  // lines
        ChainNode legacyRoot;
        legacyRoot.params = ListParams{};
        QJsonObject doc;
        QJsonObject rootObj;
        rootObj["type"] = "list";
        QJsonArray kids;
        kids.append(legacy);
        rootObj["children"] = kids;
        doc["root"] = rootObj;
        const ChainNode fromLegacy = chainFromJson(doc, nullptr);
        REQUIRE(fromLegacy.children.size() == 1);
        CHECK(std::get<SimpleScopeParams>(fromLegacy.children[0].params).mode ==
              2);  // line scope
        const auto& pb = std::get<BassSpinParams>(restored.children[1].params);
        CHECK_FALSE(pb.left);
        CHECK(pb.right);
        CHECK(pb.colorRight == 0x222222u);
        CHECK(effectTypeKey(EffectParams{SimpleScopeParams{}}) == "simpleScope");
        CHECK(effectTypeKey(EffectParams{BassSpinParams{}}) == "bassSpin");
    }

    TEST_CASE("OscStar / Ring / Rotating Stars ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode a;
        OscStarParams os; os.channel = 1; os.position = 0; os.size = 10; os.rot = 5;
        os.colors = {0xFF0000, 0x00FF00};
        a.params = os;
        root.children.push_back(std::move(a));
        ChainNode b;
        OscRingParams rg; rg.source = 1; rg.size = 12; rg.colors = {0x0000FF};
        b.params = rg;
        root.children.push_back(std::move(b));
        ChainNode c;
        RotatingStarsParams rs; rs.colors = {0x111111, 0x222222, 0x333333};
        c.params = rs;
        root.children.push_back(std::move(c));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 3);
        const auto& pa = std::get<OscStarParams>(restored.children[0].params);
        CHECK(pa.channel == 1);
        CHECK(pa.rot == 5);
        REQUIRE(pa.colors.size() == 2);
        const auto& pb = std::get<OscRingParams>(restored.children[1].params);
        CHECK(pb.source == 1);
        CHECK(pb.size == 12);
        const auto& pc = std::get<RotatingStarsParams>(restored.children[2].params);
        REQUIRE(pc.colors.size() == 3);
        CHECK(pc.colors[2] == 0x333333u);
        CHECK(effectTypeKey(EffectParams{OscStarParams{}}) == "oscStar");
        CHECK(effectTypeKey(EffectParams{OscRingParams{}}) == "oscRing");
        CHECK(effectTypeKey(EffectParams{RotatingStarsParams{}}) == "rotatingStars");
    }

    TEST_CASE("Texer II / Triangle ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode a;
        TexerIIParams t2;
        t2.filename = "s.png"; t2.imageData = "QQ=="; t2.colorFiltering = true;
        t2.initCode = "n=10"; t2.pointCode = "x=i;y=0";
        a.params = t2;
        root.children.push_back(std::move(a));
        ChainNode b;
        TriangleParams tr;
        tr.frameCode = "n=2"; tr.pointCode = "x1=-1;y1=-1";
        b.params = tr;
        root.children.push_back(std::move(b));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 2);
        const auto& pa = std::get<TexerIIParams>(restored.children[0].params);
        CHECK(pa.filename == "s.png");
        CHECK(pa.pointCode == "x=i;y=0");
        CHECK(pa.colorFiltering);
        const auto& pb = std::get<TriangleParams>(restored.children[1].params);
        CHECK(pb.frameCode == "n=2");
        CHECK(effectTypeKey(EffectParams{TexerParams{}}) == "texer");
        CHECK(effectTypeKey(EffectParams{TexerIIParams{}}) == "texerII");
        CHECK(effectTypeKey(EffectParams{TriangleParams{}}) == "triangle");
    }

    TEST_CASE("Picture-II-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        PictureIIParams pp;
        pp.filename = "a.png";
        pp.imageData = "QUJD";
        pp.blend = 1;
        leaf.params = pp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<PictureIIParams>(restored.children[0].params);
        CHECK(p.filename == "a.png");
        CHECK(p.imageData == "QUJD");
        CHECK(p.blend == 1);
        CHECK(effectTypeKey(EffectParams{PictureIIParams{}}) == "pictureII");
    }

    TEST_CASE("Picture-Parameter + eingebettetes Bild ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        PictureParams pp;
        pp.filename = "bg.png";
        pp.imageData = "QUJDREVG";  // base64 stand-in
        pp.blend = 1;
        pp.keepAspect = false;
        leaf.params = pp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<PictureParams>(restored.children[0].params);
        CHECK(p.filename == "bg.png");
        CHECK(p.imageData == "QUJDREVG");
        CHECK(p.blend == 1);
        CHECK_FALSE(p.keepAspect);
        CHECK(effectTypeKey(EffectParams{PictureParams{}}) == "picture");
    }

    TEST_CASE("Interferences-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        InterferencesParams ip;
        ip.points = 6; ip.distance = 24; ip.alpha = 100; ip.rotation = 30;
        ip.rotationInc = 4; ip.distance2 = 50; ip.alpha2 = 210;
        ip.rotationInc2 = 12; ip.rgb = true; ip.onBeat = true;
        ip.speed = 0.35f; ip.blend = 2;
        leaf.params = ip;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<InterferencesParams>(restored.children[0].params);
        CHECK(p.points == 6);
        CHECK(p.distance == 24);
        CHECK(p.alpha2 == 210);
        CHECK(p.rgb == true);
        CHECK(p.onBeat == true);
        CHECK(p.speed == doctest::Approx(0.35f));
        CHECK(p.blend == 2);
        CHECK(effectTypeKey(EffectParams{InterferencesParams{}}) == "interferences");
    }

    TEST_CASE("Mosaic-Parameter ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        leaf.params = MosaicParams{30, 90, true, 24, 2};
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<MosaicParams>(restored.children[0].params);
        CHECK(p.quality == 30);
        CHECK(p.quality2 == 90);
        CHECK(p.onBeat == true);
        CHECK(p.durationFrames == 24);
        CHECK(p.blend == 2);
    }

    TEST_CASE("Fractal-2D-Parameter + Code ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        Fractal2DParams fp;
        fp.type = 4;  // Multibrot
        fp.centerX = 0.25f;
        fp.centerY = -0.5f;
        fp.zoom = 3.5f;
        fp.rotation = 0.75f;
        fp.maxIter = 512;
        fp.juliaX = -0.70176f;
        fp.juliaY = -0.3842f;
        fp.power = 3.0f;
        fp.escapeR = 16.0f;
        fp.smooth = false;
        fp.colorScale = 0.02f;
        fp.colorCycle = 0.5f;
        fp.insideColor = 0x102030;
        fp.gradientPreset = "Fire";
        fp.blend = 1;
        fp.frameCode = "zoom=zoom*1.01; cx=cx+bass*0.01";
        fp.beatCode = "power=power+1";
        leaf.params = fp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<Fractal2DParams>(restored.children[0].params);
        CHECK(p.type == 4);
        CHECK(p.centerX == doctest::Approx(0.25f));
        CHECK(p.centerY == doctest::Approx(-0.5f));
        CHECK(p.zoom == doctest::Approx(3.5f));
        CHECK(p.rotation == doctest::Approx(0.75f));
        CHECK(p.maxIter == 512);
        CHECK(p.juliaX == doctest::Approx(-0.70176f));
        CHECK(p.juliaY == doctest::Approx(-0.3842f));
        CHECK(p.power == doctest::Approx(3.0f));
        CHECK(p.escapeR == doctest::Approx(16.0f));
        CHECK(p.smooth == false);
        CHECK(p.colorScale == doctest::Approx(0.02f));
        CHECK(p.colorCycle == doctest::Approx(0.5f));
        CHECK(p.insideColor == 0x102030u);
        CHECK(p.gradientPreset == "Fire");
        CHECK(p.blend == 1);
        CHECK(p.frameCode == "zoom=zoom*1.01; cx=cx+bass*0.01");
        CHECK(p.beatCode == "power=power+1");
        CHECK(effectTypeKey(EffectParams{Fractal2DParams{}}) == "fractal2D");
    }

    TEST_CASE("Domain-Warp-Parameter + Code ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        DomainWarpParams dp;
        dp.octaves = 7;
        dp.lacunarity = 2.5f;
        dp.gain = 0.45f;
        dp.scale = 4.2f;
        dp.warp = 1.5f;
        dp.warpScale = 0.75f;
        dp.speed = -0.3f;
        dp.offsetX = 2.0f;
        dp.offsetY = -1.0f;
        dp.colorScale = 1.5f;
        dp.colorCycle = 0.25f;
        dp.gradientPreset = "Ocean";
        dp.blend = 2;
        dp.frameCode = "warp=0.5+bass*2; speed=0.2+treble";
        leaf.params = dp;
        root.children.push_back(std::move(leaf));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        const auto& p = std::get<DomainWarpParams>(restored.children[0].params);
        CHECK(p.octaves == 7);
        CHECK(p.lacunarity == doctest::Approx(2.5f));
        CHECK(p.gain == doctest::Approx(0.45f));
        CHECK(p.scale == doctest::Approx(4.2f));
        CHECK(p.warp == doctest::Approx(1.5f));
        CHECK(p.warpScale == doctest::Approx(0.75f));
        CHECK(p.speed == doctest::Approx(-0.3f));
        CHECK(p.offsetX == doctest::Approx(2.0f));
        CHECK(p.offsetY == doctest::Approx(-1.0f));
        CHECK(p.colorScale == doctest::Approx(1.5f));
        CHECK(p.colorCycle == doctest::Approx(0.25f));
        CHECK(p.gradientPreset == "Ocean");
        CHECK(p.blend == 2);
        CHECK(p.frameCode == "warp=0.5+bass*2; speed=0.2+treble");
        CHECK(effectTypeKey(EffectParams{DomainWarpParams{}}) == "domainWarp");
    }

    TEST_CASE("Set-Render-Mode-Parameter ueberleben den Round-Trip")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode leaf;
        SetRenderModeParams sp;
        sp.enabled = false;
        sp.lineWidth = 7;
        sp.lineBlend = 2;
        sp.adjustAlpha = 200;
        leaf.params = sp;
        root.children.push_back(std::move(leaf));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 1);
        const auto& p = std::get<SetRenderModeParams>(r.children[0].params);
        CHECK(p.enabled == false);
        CHECK(p.lineWidth == 7);
        CHECK(p.lineBlend == 2);
        CHECK(p.adjustAlpha == 200);
        CHECK(effectTypeKey(EffectParams{SetRenderModeParams{}}) == "setRenderMode");
    }

    TEST_CASE("Render-Scale-Parameter ueberleben den Round-Trip (S47)")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode leaf;
        RenderScaleParams sp;
        sp.divisor = 4;
        sp.filter = 1;
        leaf.params = sp;
        root.children.push_back(std::move(leaf));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 1);
        const auto& p = std::get<RenderScaleParams>(r.children[0].params);
        CHECK(p.divisor == 4);
        CHECK(p.filter == 1);
        CHECK(effectTypeKey(EffectParams{RenderScaleParams{}}) == "renderScale");
    }

    TEST_CASE("Bloom-Parameter ueberleben den Round-Trip (S48, Lights-Etappe 1)")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode leaf;
        BloomParams bp;
        bp.downsample = 3;
        bp.radius = 12;
        bp.intensity = 1.5f;
        bp.threshold = 0.25f;
        bp.vignette = true;
        bp.vignetteStrength = 0.4f;
        bp.post = false;  // Nicht-Default — muss den Roundtrip ueberleben
        leaf.params = bp;
        root.children.push_back(std::move(leaf));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 1);
        const auto& p = std::get<BloomParams>(r.children[0].params);
        CHECK(p.downsample == 3);
        CHECK(p.radius == 12);
        CHECK(p.intensity == doctest::Approx(1.5f));
        CHECK(p.threshold == doctest::Approx(0.25f));
        CHECK(p.vignette == true);
        CHECK(p.vignetteStrength == doctest::Approx(0.4f));
        CHECK(p.post == false);
        CHECK(BloomParams{}.post == true);  // Default: Anzeige-only (S48)
        CHECK(effectTypeKey(EffectParams{BloomParams{}}) == "bloom");
    }

    TEST_CASE("Camera3D-Parameter ueberleben den Round-Trip (S48, Lights-Etappe 1)")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode leaf;
        Camera3DParams cp;
        cp.px = 1.0f; cp.py = 2.0f; cp.pz = -8.0f;
        cp.tx = 0.5f; cp.ty = -0.5f; cp.tz = 3.0f;
        cp.fov = 45.0f;
        cp.roll = 90.0f;
        cp.fogStart = 2.0f;
        cp.fogEnd = 12.0f;
        cp.fogColor = 0x102030;
        cp.initCode = "px=1";
        cp.frameCode = "roll=roll+1";
        cp.beatCode = "fov=60";
        leaf.params = cp;
        root.children.push_back(std::move(leaf));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 1);
        const auto& p = std::get<Camera3DParams>(r.children[0].params);
        CHECK(p.px == doctest::Approx(1.0f));
        CHECK(p.py == doctest::Approx(2.0f));
        CHECK(p.pz == doctest::Approx(-8.0f));
        CHECK(p.tx == doctest::Approx(0.5f));
        CHECK(p.ty == doctest::Approx(-0.5f));
        CHECK(p.tz == doctest::Approx(3.0f));
        CHECK(p.fov == doctest::Approx(45.0f));
        CHECK(p.roll == doctest::Approx(90.0f));
        CHECK(p.fogStart == doctest::Approx(2.0f));
        CHECK(p.fogEnd == doctest::Approx(12.0f));
        CHECK(p.fogColor == 0x102030u);
        CHECK(p.initCode == "px=1");
        CHECK(p.frameCode == "roll=roll+1");
        CHECK(p.beatCode == "fov=60");
        CHECK(effectTypeKey(EffectParams{Camera3DParams{}}) == "camera3d");
    }

    TEST_CASE("SuperScope3D-Parameter ueberleben den Round-Trip (S48, Lights-Etappe 1)")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode leaf;
        SuperScope3DParams sp;
        sp.initCode = "n=800";
        sp.frameCode = "t=t+0.02";
        sp.beatCode = "t=0";
        sp.pointCode = "x=i;y=v;z=t;size=0.2";
        sp.pointCount = 512;
        sp.renderMode = 1;
        sp.size = 0.25f;
        sp.falloff = 6.0f;
        sp.audioChannel = 1;
        sp.spectrumSource = true;
        leaf.params = sp;
        root.children.push_back(std::move(leaf));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 1);
        const auto& p = std::get<SuperScope3DParams>(r.children[0].params);
        CHECK(p.initCode == "n=800");
        CHECK(p.frameCode == "t=t+0.02");
        CHECK(p.beatCode == "t=0");
        CHECK(p.pointCode == "x=i;y=v;z=t;size=0.2");
        CHECK(p.pointCount == 512);
        CHECK(p.renderMode == 1);
        CHECK(p.size == doctest::Approx(0.25f));
        CHECK(p.falloff == doctest::Approx(6.0f));
        CHECK(p.audioChannel == 1);
        CHECK(p.spectrumSource == true);
        CHECK(effectTypeKey(EffectParams{SuperScope3DParams{}}) == "superScope3d");
    }

    TEST_CASE("Terrain3D-Parameter ueberleben den Round-Trip (S48, Lights-Etappe 2)")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode leaf;
        Terrain3DParams tp;
        tp.resolution = 48;
        tp.extent = 6.0f;
        tp.baseAmp = 0.3f;
        tp.yOffset = -1.2f;
        tp.ringAmp = 2.0f;
        tp.relax = 0.25f;
        tp.flatten = 0.1f;
        tp.drawMesh = false;
        tp.meshColor = 0x202428;
        tp.drawDots = true;
        tp.dotSize = 0.08f;
        tp.falloff = 6.0f;
        tp.colorLow = 0x001020;
        tp.colorHigh = 0x80FFEE;
        tp.initCode = "x=1";
        tp.frameCode = "megabuf(0)=1";
        tp.beatCode = "b=1";
        tp.pointCode = "red=h";
        leaf.params = tp;
        root.children.push_back(std::move(leaf));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 1);
        const auto& p = std::get<Terrain3DParams>(r.children[0].params);
        CHECK(p.resolution == 48);
        CHECK(p.extent == doctest::Approx(6.0f));
        CHECK(p.baseAmp == doctest::Approx(0.3f));
        CHECK(p.yOffset == doctest::Approx(-1.2f));
        CHECK(p.ringAmp == doctest::Approx(2.0f));
        CHECK(p.relax == doctest::Approx(0.25f));
        CHECK(p.flatten == doctest::Approx(0.1f));
        CHECK(p.drawMesh == false);
        CHECK(p.meshColor == 0x202428u);
        CHECK(p.drawDots == true);
        CHECK(p.dotSize == doctest::Approx(0.08f));
        CHECK(p.falloff == doctest::Approx(6.0f));
        CHECK(p.colorLow == 0x001020u);
        CHECK(p.colorHigh == 0x80FFEEu);
        CHECK(p.initCode == "x=1");
        CHECK(p.frameCode == "megabuf(0)=1");
        CHECK(p.beatCode == "b=1");
        CHECK(p.pointCode == "red=h");
        CHECK(effectTypeKey(EffectParams{Terrain3DParams{}}) == "terrain3d");
    }

    TEST_CASE("GlowOrbs-Parameter ueberleben den Round-Trip (S48, Lights-Etappe 2)")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode leaf;
        GlowOrbsParams gp;
        gp.orbCount = 7;
        gp.haloScale = 3.0f;
        gp.haloIntensity = 1.2f;
        gp.falloff = 5.0f;
        gp.initCode = "n=3";
        gp.frameCode = "t=t+0.01";
        gp.beatCode = "flashall=1";
        gp.pointCode = "x=i;radius=0.5;flash=flashall";
        leaf.params = gp;
        root.children.push_back(std::move(leaf));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 1);
        const auto& p = std::get<GlowOrbsParams>(r.children[0].params);
        CHECK(p.orbCount == 7);
        CHECK(p.haloScale == doctest::Approx(3.0f));
        CHECK(p.haloIntensity == doctest::Approx(1.2f));
        CHECK(p.falloff == doctest::Approx(5.0f));
        CHECK(p.initCode == "n=3");
        CHECK(p.frameCode == "t=t+0.01");
        CHECK(p.beatCode == "flashall=1");
        CHECK(p.pointCode == "x=i;radius=0.5;flash=flashall");
        CHECK(effectTypeKey(EffectParams{GlowOrbsParams{}}) == "glowOrbs");
    }

    TEST_CASE("Batch-H Modul-Typkeys sind stabil und eindeutig")
    {
        CHECK(effectTypeKey(EffectParams{Fractal3DParams{}}) == "fractal3D");
        CHECK(effectTypeKey(EffectParams{LyapunovParams{}}) == "lyapunov");
        CHECK(effectTypeKey(EffectParams{KleinianParams{}}) == "kleinian");
        CHECK(effectTypeKey(EffectParams{FractalZoomerParams{}}) == "fractalZoomer");
        CHECK(effectTypeKey(EffectParams{StrangeAttractorParams{}}) == "strangeAttractor");
        CHECK(effectTypeKey(EffectParams{FlameParams{}}) == "flame");
        CHECK(effectTypeKey(EffectParams{ReactionDiffusionParams{}}) == "reactionDiffusion");
    }

    TEST_CASE("Fractal-3D-Parameter ueberleben den Round-Trip")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode leaf; Fractal3DParams fp;
        fp.type = 3; fp.yaw = 1.2f; fp.pitch = -0.4f; fp.dist = 5.0f;
        fp.power = 6.0f; fp.scale = 2.4f; fp.fold = 1.3f; fp.maxSteps = 128;
        fp.maxIter = 12; fp.juliaX = 0.4f; fp.ao = false; fp.background = 0x101820;
        fp.gradientPreset = "Ocean"; fp.blend = 2; fp.frameCode = "yaw=yaw+treble";
        leaf.params = fp; root.children.push_back(std::move(leaf));
        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        const auto& p = std::get<Fractal3DParams>(r.children[0].params);
        CHECK(p.type == 3);
        CHECK(p.dist == doctest::Approx(5.0f));
        CHECK(p.maxSteps == 128);
        CHECK(p.ao == false);
        CHECK(p.background == 0x101820u);
        CHECK(p.gradientPreset == "Ocean");
        CHECK(p.frameCode == "yaw=yaw+treble");
    }

    TEST_CASE("Lyapunov/Kleinian-Parameter ueberleben den Round-Trip")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode a; LyapunovParams lp;
        lp.sequence = "AABAB"; lp.aMin = 3.0f; lp.bMax = 3.9f; lp.iterations = 500;
        lp.negColor = 0x080810; lp.gradientPreset = "Fire"; lp.blend = 1;
        a.params = lp; root.children.push_back(std::move(a));
        ChainNode b; KleinianParams kp;
        kp.p = 7; kp.q = 3; kp.iterations = 40; kp.morph = 1.1f; kp.zoom = 1.5f;
        kp.frameCode = "morph=morph+0.01"; b.params = kp;
        root.children.push_back(std::move(b));
        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        const auto& lo = std::get<LyapunovParams>(r.children[0].params);
        CHECK(lo.sequence == "AABAB");
        CHECK(lo.iterations == 500);
        CHECK(lo.negColor == 0x080810u);
        const auto& ko = std::get<KleinianParams>(r.children[1].params);
        CHECK(ko.p == 7);
        CHECK(ko.q == 3);
        CHECK(ko.morph == doctest::Approx(1.1f));
        CHECK(ko.frameCode == "morph=morph+0.01");
    }

    TEST_CASE("Zoomer/Attractor/Flame/ReactionDiffusion ueberleben den Round-Trip")
    {
        ChainNode root; root.params = ListParams{};
        ChainNode z; FractalZoomerParams zp;
        zp.type = 2; zp.zoomSpeed = 1.05f; zp.feedback = 0.7f; zp.maxIter = 300;
        z.params = zp; root.children.push_back(std::move(z));
        ChainNode s; StrangeAttractorParams sp;
        sp.type = 3; sp.a = 0.95f; sp.points = 8000; sp.useGradient = false;
        sp.color = 0x223344; sp.blend = 2; s.params = sp;
        root.children.push_back(std::move(s));
        ChainNode fl; FlameParams flp;
        flp.variation = 3; flp.functions = 4; flp.points = 50000; flp.blend = 1;
        fl.params = flp; root.children.push_back(std::move(fl));
        ChainNode rd; ReactionDiffusionParams rp;
        rp.feed = 0.037f; rp.kill = 0.06f; rp.stepsPerFrame = 12; rp.seedOnBeat = false;
        rp.frameCode = "feed=0.03+bass*0.02"; rd.params = rp;
        root.children.push_back(std::move(rd));

        const ChainNode r = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(r.children.size() == 4);
        CHECK(std::get<FractalZoomerParams>(r.children[0].params).type == 2);
        CHECK(std::get<FractalZoomerParams>(r.children[0].params).feedback == doctest::Approx(0.7f));
        CHECK(std::get<StrangeAttractorParams>(r.children[1].params).type == 3);
        CHECK(std::get<StrangeAttractorParams>(r.children[1].params).useGradient == false);
        CHECK(std::get<StrangeAttractorParams>(r.children[1].params).color == 0x223344u);
        CHECK(std::get<FlameParams>(r.children[2].params).variation == 3);
        CHECK(std::get<FlameParams>(r.children[2].params).functions == 4);
        CHECK(std::get<ReactionDiffusionParams>(r.children[3].params).stepsPerFrame == 12);
        CHECK(std::get<ReactionDiffusionParams>(r.children[3].params).seedOnBeat == false);
        CHECK(std::get<ReactionDiffusionParams>(r.children[3].params).frameCode == "feed=0.03+bass*0.02");
    }

    TEST_CASE("Round-Trip erhaelt Struktur und Parameter")
    {
        const ChainNode original = buildRichChain();
        const QJsonObject json = chainToJson(original);

        QStringList report;
        const ChainNode restored = chainFromJson(json, &report);
        CHECK(report.isEmpty());

        // Struktur
        REQUIRE(restored.isList());
        REQUIRE(restored.children.size() == 3);
        CHECK(std::get<ListParams>(restored.params).clearEveryFrame == true);

        // Fadeout
        REQUIRE(std::holds_alternative<FadeoutParams>(restored.children[0].params));
        const auto& f = std::get<FadeoutParams>(restored.children[0].params);
        CHECK(f.fadeLen == 20);
        CHECK(f.color == 0x123456);

        // SuperScope
        REQUIRE(std::holds_alternative<SuperScopeParams>(restored.children[1].params));
        const auto& s = std::get<SuperScopeParams>(restored.children[1].params);
        CHECK(s.pointCode == "x=i;y=v");
        CHECK(s.frameCode == "t=t+0.1");
        CHECK(s.pointCount == 512);
        CHECK(s.renderMode == 2);
        CHECK(s.lineWidth == doctest::Approx(3.5f));
        CHECK(s.audioChannel == 1);

        // Verschachtelte Liste + Blend + Code
        REQUIRE(restored.children[2].isList());
        const auto& nl = std::get<ListParams>(restored.children[2].params);
        CHECK(nl.blendIn == BlendMode::Buffer);
        CHECK(nl.blendOut == BlendMode::FiftyFifty);
        CHECK(nl.inAdjustAlpha == 200);
        CHECK(nl.bufferIn == 4);
        CHECK(nl.bufferOut == 2);
        CHECK(nl.bufferInInvert == true);
        CHECK(nl.bufferOutInvert == false);
        CHECK(nl.onBeatRender == true);
        CHECK(nl.onBeatFrames == 5);
        CHECK(nl.useCode == true);
        CHECK(nl.frameCode == "enabled=1");
        REQUIRE(restored.children[2].children.size() == 1);
        const auto& cm = std::get<ColorModifierParams>(restored.children[2].children[0].params);
        CHECK(cm.levelCode == "red=red*0.5");
        CHECK(cm.recompute == false);
    }

    TEST_CASE("enabled-Flag und displayName ueberleben den Round-Trip")
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode inv;
        inv.params = InvertParams{};
        inv.enabled = false;
        inv.displayName = "Mein Invert";
        root.children.push_back(std::move(inv));
        compileChain(root);

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 1);
        CHECK(restored.children[0].enabled == false);
        CHECK(restored.children[0].displayName == "Mein Invert");
    }

    TEST_CASE("unbekannter Typ-Key laedt als Passthrough (Vorwaertskompat)")
    {
        QJsonObject node;
        node["type"] = "someFutureEffect";
        node["enabled"] = true;
        QJsonObject root;
        root["type"] = "list";
        QJsonArray children;
        children.append(node);
        root["children"] = children;
        QJsonObject doc;
        doc["root"] = root;

        QStringList report;
        const ChainNode restored = chainFromJson(doc, &report);
        REQUIRE(restored.children.size() == 1);
        CHECK(std::holds_alternative<PassthroughParams>(restored.children[0].params));
    }

    TEST_CASE("nicht-Listen-Root wird in eine Liste verpackt")
    {
        QJsonObject root;
        root["type"] = "invert";
        QJsonObject doc;
        doc["root"] = root;

        const ChainNode restored = chainFromJson(doc, nullptr);
        CHECK(restored.isList());
        REQUIRE(restored.children.size() == 1);
        CHECK(std::holds_alternative<InvertParams>(restored.children[0].params));
    }
}

// =============================================================================
// Milkdrop-Meganode (N1/N2, Session 41)
// =============================================================================

TEST_SUITE("MilkdropNode")
{
    TEST_CASE("Roundtrip: eingebettetes Preset + Node-Parameter")
    {
        ChainNode root;
        root.params = ListParams{};

        MilkdropNodeParams mp;
        mp.preset.name = "roundtrip";
        mp.preset.decay = 0.955;
        mp.preset.perFrameInit = "q1=1;";
        mp.preset.perFrame = "zoom=zoom+0.01*sin(time);";
        mp.preset.perPixel = "rot=rot+q1*0.02;";
        mp.preset.warpShaderText =
            "shader_body { ret = 1 - tex2D(sampler_main, uv).xyz; }";
        mp.preset.warpInfo = lumi::milk::analyzeWarpShader(mp.preset.warpShaderText);
        lumi::milkdrop::WaveState w;
        w.index = 2;
        w.enabled = true;
        w.pointCode = "x=i;y=v;";
        mp.preset.waves.push_back(w);
        mp.presetDir = "C:/tmp/presets";
        mp.meshX = 48;
        mp.meshY = 36;
        mp.debugGrid = true;
        ChainNode milk;
        milk.params = std::move(mp);
        root.children.push_back(std::move(milk));
        compileChain(root);

        CHECK(effectTypeKey(root.children[0].params) == "milkdrop");

        QStringList report;
        const ChainNode back = chainFromJson(chainToJson(root), &report);
        REQUIRE(back.children.size() == 1);
        const auto* rp = std::get_if<MilkdropNodeParams>(&back.children[0].params);
        REQUIRE(rp != nullptr);
        CHECK(rp->preset.name == "roundtrip");
        CHECK(rp->preset.decay == doctest::Approx(0.955));
        CHECK(rp->preset.perFrameInit == "q1=1;");
        CHECK(rp->preset.perFrame == "zoom=zoom+0.01*sin(time);");
        CHECK(rp->preset.perPixel == "rot=rot+q1*0.02;");
        CHECK(rp->preset.warpShaderText ==
              "shader_body { ret = 1 - tex2D(sampler_main, uv).xyz; }");
        // Klassifikation wird beim Laden NEU abgeleitet (SSOT = Shader-Text)
        CHECK(rp->preset.warpInfo.shaderClass == lumi::milk::ShaderClass::Custom);
        REQUIRE(rp->preset.waves.size() == 1);
        CHECK(rp->preset.waves[0].enabled);
        CHECK(rp->preset.waves[0].pointCode == "x=i;y=v;");
        CHECK(rp->presetDir == "C:/tmp/presets");
        CHECK(rp->meshX == 48);
        CHECK(rp->meshY == 36);
        CHECK(rp->debugGrid);
        CHECK(rp->revision >= 1);
    }

    TEST_CASE("Bild-Einbettung (S43): referenzierte bleiben, verwaiste entfallen")
    {
        // Entscheid Patrik S43: beim Speichern werden genau die aktuell
        // referenzierten Bilder eingebettet (Datei bevorzugt, sonst die
        // vorhandene Einbettung); nicht mehr referenzierte Alt-Eintraege
        // verschwinden dabei automatisch.
        ChainNode root;
        root.params = ListParams{};
        MilkdropNodeParams mp;
        mp.preset.name = "embed";
        lumi::milkdrop::SpriteState sp;
        sp.index = 1;
        sp.imageName = "triangle.png";
        mp.preset.sprites.push_back(sp);
        mp.preset.compShaderText =
            "sampler sampler_lines2;\n"
            "shader_body { ret = tex2D(sampler_lines2, uv).xyz; }";
        // presetDir zeigt ins Leere -> Datei nicht auffindbar, die VORHANDENE
        // Einbettung muss uebernommen werden; 'verwaist' ist nicht referenziert
        mp.presetDir = "C:/gibt/es/nicht";
        mp.embeddedImages["triangle.png"] = "U3ByaXRl";  // "Sprite" (Base64)
        mp.embeddedImages["lines2"] = "VGV4dHVy";        // "Textur"
        mp.embeddedImages["verwaist"] = "QWx0";          // nicht referenziert
        ChainNode milk;
        milk.params = std::move(mp);
        root.children.push_back(std::move(milk));
        compileChain(root);

        const ChainNode back = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(back.children.size() == 1);
        const auto* rp = std::get_if<MilkdropNodeParams>(&back.children[0].params);
        REQUIRE(rp != nullptr);
        CHECK(rp->embeddedImages.size() == 2);
        CHECK(rp->embeddedImages.count("triangle.png") == 1);
        CHECK(rp->embeddedImages.count("lines2") == 1);
        CHECK(rp->embeddedImages.count("verwaist") == 0);
        CHECK(rp->embeddedImages.at("lines2") == "VGV4dHVy");
    }

    TEST_CASE("compileChain clampt Mesh und stellt eine Revision sicher")
    {
        ChainNode root;
        root.params = ListParams{};
        MilkdropNodeParams mp;
        mp.meshX = 500;
        mp.meshY = 1;
        mp.revision = 0;
        ChainNode n;
        n.params = std::move(mp);
        root.children.push_back(std::move(n));
        compileChain(root);

        const auto* p = std::get_if<MilkdropNodeParams>(&root.children[0].params);
        REQUIRE(p != nullptr);
        CHECK(p->meshX == 96);
        CHECK(p->meshY == 6);
        CHECK(p->revision == 1);
        CHECK(root.children[0].displayName == "Milkdrop");
    }

    TEST_CASE("MultiEffectVisualizer::loadMilkFile installiert einen Milkdrop-Node")
    {
        const std::filesystem::path preset = repoRoot() / "asset" / "calibration" /
                                             "milkdrop" / "c1" / "01_warp_drift.milk";
        REQUIRE(std::filesystem::exists(preset));

        MultiEffectVisualizer host;
        QStringList report;
        REQUIRE(host.loadMilkFile(QString::fromStdWString(preset.wstring()), &report));

        const ChainNode& root = host.chain();
        REQUIRE(root.isList());
        REQUIRE(root.children.size() == 1);
        const auto* mp = std::get_if<MilkdropNodeParams>(&root.children[0].params);
        REQUIRE(mp != nullptr);
        CHECK(mp->preset.warpInfo.shaderClass == lumi::milk::ShaderClass::Custom);
        CHECK(!mp->preset.warpShaderText.empty());
        CHECK(mp->revision == 1);
        CHECK(!mp->presetDir.empty());
        CHECK(root.children[0].nodeId != 0);  // setChain hat kompiliert

        // Import-Report hat Paritaet zum alten Standalone-Pfad (Transpile-Note)
        bool hasTranspileNote = false;
        for (const QString& line : report)
        {
            if (line.contains(QStringLiteral("GLSL"))) hasTranspileNote = true;
        }
        CHECK(hasTranspileNote);
    }
}
