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

#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QJsonArray>
#include <QJsonObject>

using namespace lumi::multieffect;

namespace
{

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
        s.source = 0; s.channel = 1; s.position = 0; s.drawMode = 1;
        s.colors = {0xFF0000, 0x00FF00};
        a.params = s;
        root.children.push_back(std::move(a));
        ChainNode b;
        b.params = BassSpinParams{false, true, 0x111111, 0x222222, 0};
        root.children.push_back(std::move(b));

        const ChainNode restored = chainFromJson(chainToJson(root), nullptr);
        REQUIRE(restored.children.size() == 2);
        const auto& ps = std::get<SimpleScopeParams>(restored.children[0].params);
        CHECK(ps.source == 0);
        CHECK(ps.drawMode == 1);
        REQUIRE(ps.colors.size() == 2);
        CHECK(ps.colors[1] == 0x00FF00u);
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
