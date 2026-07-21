/**
 ****************************************************************************************
 * @file   test_AvsChainTranslator.cpp
 * @brief  Tests fuer den AVS-Baum -> Host-EffectChain-Uebersetzer (Roadmap 5.5)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/AvsChainTranslator.hpp"

#include <cstring>
#include <filesystem>

using namespace lumi::multieffect;
using lumi::avs::EffectNode;
using lumi::avs::ParseResult;

namespace
{

/// Baut einen minimalen ParseResult mit einem Root-Listenknoten + Kindern.
ParseResult makeParsed(std::vector<EffectNode> children)
{
    ParseResult r;
    r.ok = true;
    r.formatVersion = 2;
    r.root.isList = true;
    r.root.id = lumi::avs::kListId;
    r.root.name = "Main";
    r.root.children = std::move(children);
    return r;
}

EffectNode builtin(int32_t id)
{
    EffectNode n;
    n.id = id;
    n.decoded = true;
    return n;
}

/// Pfad zum Referenz-Korpus (lokal, unversioniert) — wie test_AvsParser.
std::filesystem::path corpusDir()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p.parent_path() / "ref" / "vis_avs" / "avs" / "vis_avs" / "presets";
}

} // namespace

TEST_SUITE("AvsChainTranslator")
{
    TEST_CASE("nicht-ok ParseResult ergibt leere Liste + Report")
    {
        ParseResult bad;
        bad.ok = false;
        bad.error = "kaputt";
        const TranslationResult t = translateAvsTree(bad);
        CHECK(t.root.isList());
        CHECK(t.root.children.empty());
        REQUIRE(t.report.size() == 1);
        CHECK(t.report[0].find("kaputt") != std::string::npos);
    }

    TEST_CASE("dekodierte Kern-Effekte werden auf ihre Params gemappt")
    {
        EffectNode fade = builtin(3);
        fade.fields = {{"fadelen", 20}, {"color", 0x0000FF /* COLORREF: rot */}};

        EffectNode invert = builtin(37);
        invert.fields = {{"enabled", 1}};

        const TranslationResult t = translateAvsTree(makeParsed({fade, invert}));
        REQUIRE(t.root.children.size() == 2);
        CHECK(t.effectCount == 2);
        CHECK(t.passthroughCount == 0);

        REQUIRE(std::holds_alternative<FadeoutParams>(t.root.children[0].params));
        const auto& f = std::get<FadeoutParams>(t.root.children[0].params);
        CHECK(f.fadeLen == 20);
        CHECK(f.color == 0xFF0000);  // COLORREF 0x0000FF -> host 0xFF0000

        CHECK(std::holds_alternative<InvertParams>(t.root.children[1].params));
    }

    TEST_CASE("nicht dekodierter Effekt wird als Passthrough konserviert")
    {
        EffectNode unknown;
        unknown.id = 20;  // Water — kein Decoder
        unknown.name = "Water";
        unknown.decoded = false;

        const TranslationResult t = translateAvsTree(makeParsed({unknown}));
        REQUIRE(t.root.children.size() == 1);
        CHECK(t.passthroughCount == 1);
        REQUIRE(std::holds_alternative<PassthroughParams>(t.root.children[0].params));
        CHECK(std::get<PassthroughParams>(t.root.children[0].params).sourceId == 20);
        CHECK(t.report.size() >= 1);
    }

    TEST_CASE("Set Render Mode wird ein Live-Node (kein Passthrough)")
    {
        EffectNode srm = builtin(40);
        srm.fields = {{"newmode", 5 << 16}};  // line width 5 (bits 16-23), not enabled

        EffectNode scope = builtin(36);
        scope.code = {{"point", "x=i;y=v"}};
        scope.fields = {{"which_ch", 0}, {"num_colors", 0}, {"drawmode", 1}};

        const TranslationResult t = translateAvsTree(makeParsed({srm, scope}));
        REQUIRE(t.root.children.size() == 2);
        // SRM ist jetzt ein echter Knoten, kein Passthrough, keine Notiz.
        REQUIRE(std::holds_alternative<SetRenderModeParams>(t.root.children[0].params));
        const auto& p = std::get<SetRenderModeParams>(t.root.children[0].params);
        CHECK(p.lineWidth == 5);
        CHECK(p.enabled == false);
        // Der folgende Scope bekommt NICHTS eingebacken (Host-Render-Mode zur Laufzeit).
        REQUIRE(std::holds_alternative<SuperScopeParams>(t.root.children[1].params));
        CHECK(std::get<SuperScopeParams>(t.root.children[1].params).lineWidth
              == doctest::Approx(2.0f));  // Struct-Default
        // Kein Passthrough-Zähler mehr für Set Render Mode.
        for (const std::string& note : t.report)
            CHECK(note.find("Set Render Mode") == std::string::npos);
    }

    TEST_CASE("Set Render Mode: Blend-Bits werden korrekt auf den Knoten gemappt")
    {
        EffectNode srm = builtin(40);
        // enabled (bit 31) | line width 6 (bits 16-23) | alpha 128 (bits 8-15) | blend 3 = 50/50
        srm.fields = {{"newmode",
                       static_cast<int32_t>(0x80000000u) | (6 << 16) | (128 << 8) | 3}};

        const TranslationResult t = translateAvsTree(makeParsed({srm}));
        REQUIRE(t.root.children.size() == 1);
        REQUIRE(std::holds_alternative<SetRenderModeParams>(t.root.children[0].params));
        const auto& p = std::get<SetRenderModeParams>(t.root.children[0].params);
        CHECK(p.enabled == true);
        CHECK(p.lineWidth == 6);
        CHECK(p.lineBlend == 2);   // blend bits 3 -> 50/50
        CHECK(p.adjustAlpha == 128);
    }

    TEST_CASE("Movement-Builtin-Formel -> MovementParams mit AVS-Point-Code")
    {
        EffectNode move = builtin(15);
        move.fields = {{"effect", 3}, {"blend", 0}, {"wrap", 1}};  // 3 = big swirl out
        // kein code["point"] -> Formel aus der Tabelle

        const TranslationResult t = translateAvsTree(makeParsed({move}));
        REQUIRE(t.root.children.size() == 1);
        REQUIRE(std::holds_alternative<MovementParams>(t.root.children[0].params));
        const auto& p = std::get<MovementParams>(t.root.children[0].params);
        CHECK(p.code.find("d = d * 0.96") != std::string::npos);
        CHECK_FALSE(p.rectCoords);  // formula 3 is polar
        CHECK(p.wrap);
    }

    TEST_CASE("Movement rect-Builtin (gridley) setzt rectCoords")
    {
        EffectNode move = builtin(15);
        move.fields = {{"effect", 20}};  // 20 = gridley (uses_rect)
        const TranslationResult t = translateAvsTree(makeParsed({move}));
        const auto& p = std::get<MovementParams>(t.root.children[0].params);
        CHECK(p.rectCoords);
        CHECK(p.code.find("cos(y * 18)") != std::string::npos);
    }

    TEST_CASE("Movement Nicht-Remap-Builtins (none/fuzzify/blocky) -> Passthrough")
    {
        for (int effect : {0, 1, 7})
        {
            EffectNode move = builtin(15);
            move.fields = {{"effect", effect}};
            const TranslationResult t = translateAvsTree(makeParsed({move}));
            REQUIRE(t.root.children.size() == 1);
            CHECK(std::holds_alternative<PassthroughParams>(t.root.children[0].params));
        }
    }

    TEST_CASE("Dynamic Shift (id 42) -> DynamicShiftParams mit Code + Flags")
    {
        EffectNode shift = builtin(42);
        shift.code = {{"init", "d=0"}, {"frame", "x=sin(d);y=cos(d)"}, {"beat", "d=d+2"}};
        shift.fields = {{"blend", 1}, {"subpixel", 0}};
        const TranslationResult t = translateAvsTree(makeParsed({shift}));
        REQUIRE(t.root.children.size() == 1);
        REQUIRE(std::holds_alternative<DynamicShiftParams>(t.root.children[0].params));
        const auto& p = std::get<DynamicShiftParams>(t.root.children[0].params);
        CHECK(p.initCode == "d=0");
        CHECK(p.frameCode == "x=sin(d);y=cos(d)");
        CHECK(p.beatCode == "d=d+2");
        CHECK(p.blend);
        CHECK_FALSE(p.bilinear);
    }

    TEST_CASE("Dynamic Distance Modifier (id 35) -> Params mit Pixel-Code")
    {
        EffectNode ddm = builtin(35);
        ddm.code = {{"point", "d=d*0.9"}, {"frame", "t=t+1"}, {"beat", ""}, {"init", "t=0"}};
        ddm.fields = {{"blend", 0}, {"subpixel", 1}};
        const TranslationResult t = translateAvsTree(makeParsed({ddm}));
        REQUIRE(std::holds_alternative<DynamicDistanceModifierParams>(t.root.children[0].params));
        const auto& p = std::get<DynamicDistanceModifierParams>(t.root.children[0].params);
        CHECK(p.pixelCode == "d=d*0.9");
        CHECK(p.initCode == "t=0");
        CHECK_FALSE(p.blend);
        CHECK(p.bilinear);
    }

    TEST_CASE("Moving Particle (id 8) -> Params; enabled-Bits gemappt")
    {
        EffectNode mp = builtin(8);
        mp.fields = {{"enabled", 1 | 2}, {"colors", 0x0000FF /* COLORREF: rot */},
                     {"maxdist", 24}, {"size", 10}, {"size2", 30}, {"blend", 2}};
        const TranslationResult t = translateAvsTree(makeParsed({mp}));
        REQUIRE(std::holds_alternative<MovingParticleParams>(t.root.children[0].params));
        const auto& p = std::get<MovingParticleParams>(t.root.children[0].params);
        CHECK(p.color == 0xFF0000u);  // COLORREF -> host RRGGBB
        CHECK(p.maxDistance == 24);
        CHECK(p.size2 == 30);
        CHECK(p.onBeatSize);          // enabled bit 1
        CHECK(p.blend == 2);
        CHECK(t.root.children[0].enabled);  // enabled bit 0
    }

    TEST_CASE("Comment (id 21) -> stiller no-op ohne Report-Notiz")
    {
        EffectNode comment;
        comment.id = 21;
        comment.name = "Comment";
        const TranslationResult t = translateAvsTree(makeParsed({comment}));
        REQUIRE(t.root.children.size() == 1);
        CHECK(std::holds_alternative<PassthroughParams>(t.root.children[0].params));
        CHECK(t.passthroughCount == 0);  // not counted as an unrendered passthrough
        CHECK(t.report.empty());         // no warning line
    }

    TEST_CASE("Color Map (APE) -> ColorMapParams mit Gradient-Stops")
    {
        EffectNode cm;
        cm.id = lumi::avs::kApeIdBase;
        cm.apeId = "Color Map";
        cm.decoded = true;
        cm.fields = {{"key", 1}, {"blendMode", 4}, {"adjustBlend", 200},
                     {"cmcount", 2}, {"cmpos0", 0}, {"cmpos1", 255}};
        cm.colors = {0x000000, 0xFFFFFF};
        const TranslationResult t = translateAvsTree(makeParsed({cm}));
        REQUIRE(t.root.children.size() == 1);
        REQUIRE(std::holds_alternative<ColorMapParams>(t.root.children[0].params));
        const auto& p = std::get<ColorMapParams>(t.root.children[0].params);
        CHECK(p.key == 1);
        CHECK(p.blendMode == 4);
        CHECK(p.adjustBlend == 200);
        REQUIRE(p.stopPos.size() == 2);
        CHECK(p.stopPos[1] == 255);
        CHECK(p.stopColor[1] == 0xFFFFFFu);
    }

    TEST_CASE("Buffer blend (APE) -> BufferBlendParams")
    {
        EffectNode bb;
        bb.id = lumi::avs::kApeIdBase;
        bb.apeId = "Misc: Buffer blend";
        bb.decoded = true;
        bb.fields = {{"enabled", 1}, {"bufferB", 8}, {"bufferA", 2}, {"mode", 3}};
        const TranslationResult t = translateAvsTree(makeParsed({bb}));
        REQUIRE(std::holds_alternative<BufferBlendParams>(t.root.children[0].params));
        const auto& p = std::get<BufferBlendParams>(t.root.children[0].params);
        CHECK(p.bufferA == 2);
        CHECK(p.bufferB == 8);  // CURRENT
        CHECK(p.mode == 3);
        CHECK(t.root.children[0].enabled);
    }

    TEST_CASE("Jheriko: Global (APE) -> JherikoGlobalParams mit Code")
    {
        EffectNode jg;
        jg.id = lumi::avs::kApeIdBase;
        jg.apeId = "Jheriko: Global";
        jg.decoded = true;
        jg.fields = {{"load", 3}};
        jg.code = {{"init", "reg00=0"}, {"frame", "reg00=reg00+1"}, {"beat", ""}};
        const TranslationResult t = translateAvsTree(makeParsed({jg}));
        REQUIRE(std::holds_alternative<JherikoGlobalParams>(t.root.children[0].params));
        const auto& p = std::get<JherikoGlobalParams>(t.root.children[0].params);
        CHECK(p.loadMode == 3);
        CHECK(p.initCode == "reg00=0");
        CHECK(p.frameCode == "reg00=reg00+1");
    }

    TEST_CASE("Color Clip (id 12): Modus + Farben; enabled=0 -> disabled")
    {
        EffectNode cc = builtin(12);
        cc.fields = {{"enabled", 2}, {"color_clip", 0x0000FF /* COLORREF rot */},
                     {"color_clip_out", 0x00FF00 /* gruen */}, {"color_dist", 20}};
        const TranslationResult t = translateAvsTree(makeParsed({cc}));
        REQUIRE(std::holds_alternative<ColorClipParams>(t.root.children[0].params));
        const auto& p = std::get<ColorClipParams>(t.root.children[0].params);
        CHECK(p.mode == 2);
        CHECK(p.clipColor == 0xFF0000u);  // COLORREF -> RRGGBB
        CHECK(p.outColor == 0x00FF00u);
        CHECK(p.distance == 20);
        CHECK(t.root.children[0].enabled);
    }

    TEST_CASE("Unique Tone (id 38) + Interleave (id 23) mappen Blend/Color")
    {
        EffectNode ut = builtin(38);
        ut.fields = {{"enabled", 1}, {"color", 0x00FFFF}, {"blend", 0},
                     {"blendavg", 1}, {"invert", 1}};
        EffectNode il = builtin(23);
        il.fields = {{"enabled", 1}, {"x", 4}, {"y", 0}, {"color", 0x123456},
                     {"blend", 1}, {"blendavg", 0}, {"onbeat", 1}, {"x2", 8},
                     {"y2", 2}, {"beatdur", 6}};
        const TranslationResult t = translateAvsTree(makeParsed({ut, il}));
        const auto& u = std::get<UniqueToneParams>(t.root.children[0].params);
        CHECK(u.invert);
        CHECK(u.blend == 2);  // blendavg -> 50/50
        const auto& i = std::get<InterleaveParams>(t.root.children[1].params);
        CHECK(i.x == 4);
        CHECK(i.y == 0);
        CHECK(i.blend == 1);  // blend -> additive
        CHECK(i.onBeat);
        CHECK(i.x2 == 8);
        CHECK(i.beatDuration == 6);
    }

    TEST_CASE("Convolution (APE) -> Kernel + Flags gemappt")
    {
        EffectNode cv;
        cv.id = lumi::avs::kApeIdBase;
        cv.apeId = "Holden03: Convolution Filter";
        cv.decoded = true;
        cv.fields = {{"enabled", 1}, {"edgeMode", 1}, {"absolute", 1},
                     {"twoPass", 1}, {"bias", 5}, {"scale", 9}, {"k24", 4}};
        const TranslationResult t = translateAvsTree(makeParsed({cv}));
        REQUIRE(std::holds_alternative<ConvolutionParams>(t.root.children[0].params));
        const auto& p = std::get<ConvolutionParams>(t.root.children[0].params);
        CHECK(p.edgeMode == 1);
        CHECK(p.absolute);
        CHECK(p.twoPass);
        CHECK(p.scale == 9);
        CHECK(p.kernel[24] == 4);  // center weight
    }

    TEST_CASE("MultiFilter + Add Borders (APEs) gemappt")
    {
        EffectNode mf;
        mf.id = lumi::avs::kApeIdBase;
        mf.apeId = "Jheriko : MULTIFILTER";
        mf.decoded = true;
        mf.fields = {{"enabled", 1}, {"effect", 2}, {"onbeat", 1}, {"null0", 0}};
        EffectNode ab;
        ab.id = lumi::avs::kApeIdBase;
        ab.apeId = "Virtual Effect: Addborders";
        ab.decoded = true;
        ab.fields = {{"enabled", 1}, {"color", 0x0000FF}, {"size", 7}};
        const TranslationResult t = translateAvsTree(makeParsed({mf, ab}));
        const auto& m = std::get<MultiFilterParams>(t.root.children[0].params);
        CHECK(m.effect == 2);
        CHECK(m.onBeat);
        const auto& b = std::get<AddBordersParams>(t.root.children[1].params);
        CHECK(b.color == 0xFF0000u);  // COLORREF -> RRGGBB
        CHECK(b.size == 7);
    }

    TEST_CASE("Framerate Limiter (APE) -> stiller no-op mit Notiz")
    {
        EffectNode fl;
        fl.id = lumi::avs::kApeIdBase;
        fl.apeId = "VFX FRAMERATE LIMITER";
        fl.decoded = true;
        fl.fields = {{"enabled", 1}, {"limit", 30}};
        const TranslationResult t = translateAvsTree(makeParsed({fl}));
        CHECK(std::holds_alternative<PassthroughParams>(t.root.children[0].params));
        CHECK(t.passthroughCount == 0);  // not counted as unsupported
        REQUIRE(t.report.size() == 1);
        CHECK(t.report[0].find("Framerate Limiter") != std::string::npos);
    }

    TEST_CASE("Simple (id 0): effect-Bitfeld -> Source/Channel/Position/Dots")
    {
        EffectNode s = builtin(0);
        s.fields = {{"effect", 2 | (1 << 2) | (2 << 4) | (1 << 6)}};
        s.colors = {0x0000FF};  // COLORREF blau -> host rot
        const TranslationResult t = translateAvsTree(makeParsed({s}));
        REQUIRE(std::holds_alternative<SimpleScopeParams>(t.root.children[0].params));
        const auto& p = std::get<SimpleScopeParams>(t.root.children[0].params);
        CHECK(p.source == 1);    // waveform (bit 1)
        CHECK(p.channel == 1);   // right
        CHECK(p.position == 2);  // center
        CHECK(p.drawMode == 1);  // dots
        REQUIRE(p.colors.size() == 1);
        CHECK(p.colors[0] == 0xFF0000u);
    }

    TEST_CASE("Bass Spin (id 7): enabled-Bits + Farben")
    {
        EffectNode b = builtin(7);
        b.fields = {{"enabled", 2}, {"color0", 0x00FF00}, {"color1", 0x123456},
                    {"mode", 0}};
        const TranslationResult t = translateAvsTree(makeParsed({b}));
        REQUIRE(std::holds_alternative<BassSpinParams>(t.root.children[0].params));
        const auto& p = std::get<BassSpinParams>(t.root.children[0].params);
        CHECK_FALSE(p.left);
        CHECK(p.right);
        CHECK(p.mode == 0);
        CHECK(p.colorRight == 0x563412u);  // 0x123456 COLORREF -> RRGGBB
    }

    TEST_CASE("Oscilliscope Star / Ring / Rotating Stars gemappt")
    {
        EffectNode os = builtin(2);
        os.fields = {{"effect", (1 << 2) | (0 << 4)}, {"size", 10}, {"rot", 5}};
        os.colors = {0x0000FF};
        EffectNode rg = builtin(14);
        rg.fields = {{"effect", (2 << 2)}, {"size", 12}, {"source", 1}};
        rg.colors = {0x00FF00};
        EffectNode rs = builtin(13);
        rs.colors = {0xFFFFFF, 0x808080};
        const TranslationResult t = translateAvsTree(makeParsed({os, rg, rs}));
        const auto& a = std::get<OscStarParams>(t.root.children[0].params);
        CHECK(a.channel == 1);
        CHECK(a.size == 10);
        CHECK(a.rot == 5);
        CHECK(a.colors[0] == 0xFF0000u);
        const auto& b = std::get<OscRingParams>(t.root.children[1].params);
        CHECK(b.source == 1);   // spectrum
        CHECK(b.channel == 2);
        CHECK(b.size == 12);
        const auto& c = std::get<RotatingStarsParams>(t.root.children[2].params);
        REQUIRE(c.colors.size() == 2);
    }

    TEST_CASE("Texer / Texer II / Triangle (APEs) + Text-no-op")
    {
        auto ape = [](const char* id) {
            EffectNode n;
            n.id = lumi::avs::kApeIdBase;
            n.apeId = id;
            n.decoded = true;
            return n;
        };
        EffectNode tx = ape("Texer");
        tx.code = {{"filename", "p.bmp"}};
        tx.fields = {{"flags", 4}, {"particles", 50}};
        EffectNode t2 = ape("Acko.net: Texer II");
        t2.code = {{"filename", "s.png"}, {"init", "n=10"}, {"frame", ""},
                   {"beat", ""}, {"point", "x=i"}};
        t2.fields = {{"colorFiltering", 1}};
        EffectNode tr = ape("Render: Triangle");
        tr.code = {{"init", ""}, {"frame", "n=1"}, {"beat", ""}, {"point", "x1=-1"}};
        EffectNode txt;
        txt.id = 28;
        txt.name = "Text";

        const TranslationResult t = translateAvsTree(makeParsed({tx, t2, tr, txt}));
        CHECK(std::get<TexerParams>(t.root.children[0].params).particles == 50);
        CHECK(std::get<TexerParams>(t.root.children[0].params).blend == 1);  // flags bit 2
        CHECK(std::get<TexerIIParams>(t.root.children[1].params).pointCode == "x=i");
        CHECK(std::get<TexerIIParams>(t.root.children[1].params).colorFiltering);
        CHECK(std::get<TriangleParams>(t.root.children[2].params).frameCode == "n=1");
        CHECK(std::holds_alternative<PassthroughParams>(t.root.children[3].params));  // Text
    }

    TEST_CASE("Picture II (APE): Dateiname + Blend")
    {
        EffectNode pic;
        pic.id = lumi::avs::kApeIdBase;
        pic.apeId = "Picture II";
        pic.decoded = true;
        pic.code = {{"filename", "logo.png"}};
        pic.fields = {{"blendMode", 1}};
        const TranslationResult t = translateAvsTree(makeParsed({pic}));
        REQUIRE(std::holds_alternative<PictureIIParams>(t.root.children[0].params));
        const auto& p = std::get<PictureIIParams>(t.root.children[0].params);
        CHECK(p.filename == "logo.png");
        CHECK(p.blend == 1);
        CHECK(p.imageData.empty());
    }

    TEST_CASE("Picture (id 34): Dateiname + Blend/Aspect, kein Embed im Translator")
    {
        EffectNode pic = builtin(34);
        pic.code = {{"filename", "bg.bmp"}};
        pic.fields = {{"enabled", 1}, {"blend", 0}, {"blendavg", 1}, {"ratio", 1}};
        const TranslationResult t = translateAvsTree(makeParsed({pic}));
        REQUIRE(std::holds_alternative<PictureParams>(t.root.children[0].params));
        const auto& p = std::get<PictureParams>(t.root.children[0].params);
        CHECK(p.filename == "bg.bmp");
        CHECK(p.blend == 2);          // blendavg -> 50/50
        CHECK(p.keepAspect);          // ratio != 0
        CHECK(p.imageData.empty());   // embed happens app-side, not here
    }

    TEST_CASE("verschachtelte Liste mit Blend wird uebernommen")
    {
        EffectNode inner;
        inner.isList = true;
        inner.id = lumi::avs::kListId;
        inner.list.mode = (4 << 8);  // in-blend = Additive (bits 8-12)
        inner.list.inBlendVal = 200;
        inner.children.push_back([] {
            EffectNode inv;
            inv.id = 37;
            inv.decoded = true;
            inv.fields = {{"enabled", 1}};
            return inv;
        }());

        const TranslationResult t = translateAvsTree(makeParsed({inner}));
        REQUIRE(t.root.children.size() == 1);
        REQUIRE(t.root.children[0].isList());
        const auto& lp = std::get<ListParams>(t.root.children[0].params);
        CHECK(lp.blendIn == BlendMode::Additive);
        CHECK(lp.inAdjustAlpha == 200);
        CHECK(t.root.children[0].children.size() == 1);
    }

    TEST_CASE("Buffer-Blend: Index + Invert werden uebernommen und geklammert")
    {
        EffectNode inner;
        inner.isList = true;
        inner.id = lumi::avs::kListId;
        // in-blend = Buffer (12, bits 8-12); out-blend = Buffer -> (12^1)=13 in bits 16-20
        inner.list.mode = (12 << 8) | (13 << 16);
        inner.list.bufferIn = 3;
        inner.list.bufferOut = 99;   // ausserhalb 0..7 -> clamp auf 7
        inner.list.inInvert = 1;
        inner.list.outInvert = 0;

        const TranslationResult t = translateAvsTree(makeParsed({inner}));
        REQUIRE(t.root.children.size() == 1);
        const auto& lp = std::get<ListParams>(t.root.children[0].params);
        CHECK(lp.blendIn == BlendMode::Buffer);
        CHECK(lp.blendOut == BlendMode::Buffer);
        CHECK(lp.bufferIn == 3);
        CHECK(lp.bufferOut == 7);
        CHECK(lp.bufferInInvert);
        CHECK_FALSE(lp.bufferOutInvert);
    }

    TEST_CASE("Mosaic: Felder gemappt, Blend-Flags -> Modus, Werte geklammert")
    {
        EffectNode mos = builtin(30);
        mos.fields = {{"enabled", 1}, {"quality", 200}, {"quality2", 0},
                      {"blend", 0},   {"blendavg", 1}, {"onbeat", 1},
                      {"durFrames", 8}};

        const TranslationResult t = translateAvsTree(makeParsed({mos}));
        REQUIRE(t.root.children.size() == 1);
        const auto& p = std::get<MosaicParams>(t.root.children[0].params);
        CHECK(p.quality == 100);   // 200 -> clamp 100
        CHECK(p.quality2 == 1);    // 0   -> clamp 1
        CHECK(p.onBeat);
        CHECK(p.durationFrames == 8);
        CHECK(p.blend == 2);       // blendavg set -> 50/50
    }

    TEST_CASE("Mosaic: additiver Blend hat Vorrang vor blendavg")
    {
        EffectNode mos = builtin(30);
        mos.fields = {{"quality", 50}, {"blend", 1}, {"blendavg", 1}};
        const TranslationResult t = translateAvsTree(makeParsed({mos}));
        const auto& p = std::get<MosaicParams>(t.root.children[0].params);
        CHECK(p.blend == 1);  // additive wins
    }

    TEST_CASE("SuperScope: AVS-Farbtabelle -> colors (COLORREF-Swap) + Table-Modus")
    {
        EffectNode ss = builtin(36);
        ss.colors = {0x000000FFu, 0x00FF0000u};  // AVS COLORREF 0x00BBGGRR

        const TranslationResult t = translateAvsTree(makeParsed({ss}));
        REQUIRE(t.root.children.size() == 1);
        const auto& p = std::get<SuperScopeParams>(t.root.children[0].params);
        CHECK(p.colorBlend == 1);  // colors present -> table mode
        REQUIRE(p.colors.size() == 2);
        CHECK(p.colors[0] == 0xFF0000u);  // -> RRGGBB red
        CHECK(p.colors[1] == 0x0000FFu);  // -> RRGGBB blue
    }

    TEST_CASE("SuperScope ohne AVS-Farben bleibt im Gradient-Modus")
    {
        const TranslationResult t = translateAvsTree(makeParsed({builtin(36)}));
        const auto& p = std::get<SuperScopeParams>(t.root.children[0].params);
        CHECK(p.colorBlend == 0);
        CHECK(p.colors.empty());
    }

    TEST_CASE("Grain: Felder + Blend-Flags gemappt")
    {
        EffectNode g = builtin(24);
        g.fields = {{"enabled", 1}, {"blend", 0}, {"blendavg", 1},
                    {"smax", 60},   {"staticgrain", 1}};
        const TranslationResult t = translateAvsTree(makeParsed({g}));
        REQUIRE(t.root.children.size() == 1);
        const auto& p = std::get<GrainParams>(t.root.children[0].params);
        CHECK(p.amount == 60);
        CHECK(p.staticGrain);
        CHECK(p.blend == 2);  // blendavg -> 50/50
    }

    TEST_CASE("Scatter -> ScatterParams (parameterlos)")
    {
        const TranslationResult t = translateAvsTree(makeParsed({builtin(16)}));
        REQUIRE(t.root.children.size() == 1);
        CHECK(std::holds_alternative<ScatterParams>(t.root.children[0].params));
    }

    TEST_CASE("Water -> WaterParams (parameterlos)")
    {
        const TranslationResult t = translateAvsTree(makeParsed({builtin(20)}));
        REQUIRE(t.root.children.size() == 1);
        CHECK(std::holds_alternative<WaterParams>(t.root.children[0].params));
    }

    TEST_CASE("APE-Dispatch: Channel Shift / Color Reduction / Multiplier")
    {
        auto ape = [](const char* id) {
            EffectNode n;
            n.id = lumi::avs::kApeIdBase;
            n.apeId = id;
            n.decoded = true;
            return n;
        };

        EffectNode cs = ape("Channel Shift");
        cs.fields = {{"mode", 1020}, {"onbeat", 1}};  // IDC_RBG -> 1
        const auto tcs = translateAvsTree(makeParsed({cs}));
        const auto& pcs = std::get<ChannelShiftParams>(tcs.root.children[0].params);
        CHECK(pcs.mode == 1);
        CHECK(pcs.onBeat);

        EffectNode cr = ape("Color Reduction");
        cr.fields = {{"levels", 4}};
        const auto tcr = translateAvsTree(makeParsed({cr}));
        CHECK(std::get<ColorReductionParams>(tcr.root.children[0].params).levels == 4);

        EffectNode ml = ape("Multiplier");
        ml.fields = {{"ml", 2}};
        const auto tml = translateAvsTree(makeParsed({ml}));
        CHECK(std::get<MultiplierParams>(tml.root.children[0].params).mode == 2);

        EffectNode vd = ape("Holden04: Video Delay");
        vd.fields = {{"enabled", 1}, {"usebeats", 0}, {"delay", 20}};
        const auto tvd = translateAvsTree(makeParsed({vd}));
        const auto& pvd = std::get<VideoDelayParams>(tvd.root.children[0].params);
        CHECK(pvd.delay == 20);
        CHECK_FALSE(pvd.useBeats);

        // Multi Delay: mode/buffer + per-buffer delay picked by activebuffer.
        EffectNode md = ape("Holden05: Multi Delay");
        md.fields = {{"mode", 2}, {"activebuffer", 3},
                     {"ub3", 0}, {"dl3", 45}};
        const auto tmd = translateAvsTree(makeParsed({md}));
        const auto& pmd = std::get<MultiDelayParams>(tmd.root.children[0].params);
        CHECK(pmd.mode == 2);
        CHECK(pmd.buffer == 3);
        CHECK(pmd.delay == 45);
    }

    TEST_CASE("Dot Grid: Farbtabelle (Swap) + Felder gemappt")
    {
        EffectNode dg = builtin(17);
        dg.colors = {0x000000FF, 0x00FF0000};  // COLORREF
        dg.fields = {{"num_colors", 2}, {"spacing", 12}, {"x_move", 64},
                     {"y_move", -64}, {"blend", 1}};
        const TranslationResult t = translateAvsTree(makeParsed({dg}));
        const auto& p = std::get<DotGridParams>(t.root.children[0].params);
        REQUIRE(p.colors.size() == 2);
        CHECK(p.colors[0] == 0xFF0000u);
        CHECK(p.spacing == 12);
        CHECK(p.xMove == 64);
        CHECK(p.blend == 1);
    }

    TEST_CASE("Dot Plane / Fountain: 5 Farben (Swap) + rotVel/angle")
    {
        EffectNode dp = builtin(1);
        dp.colors = {0x000000FF, 0x0000FF00, 0x00FF0000, 0x00FFFFFF, 0x00000000};
        dp.fields = {{"rotvel", 24}, {"angle", -30}};
        const TranslationResult t = translateAvsTree(makeParsed({dp}));
        const auto& p = std::get<DotPlaneParams>(t.root.children[0].params);
        CHECK(p.colors[0] == 0xFF0000u);
        CHECK(p.colors[1] == 0x00FF00u);
        CHECK(p.rotVel == 24);
        CHECK(p.angle == -30);

        EffectNode df = builtin(19);
        df.colors = {0x000000FF};
        df.fields = {{"rotvel", 8}, {"angle", 10}};
        const TranslationResult t2 = translateAvsTree(makeParsed({df}));
        CHECK(std::holds_alternative<DotFountainParams>(t2.root.children[0].params));
    }

    TEST_CASE("Timescope: Felder + Color-Swap gemappt")
    {
        EffectNode ts = builtin(39);
        ts.fields = {{"color", 0x0000FF00},  // COLORREF -> RRGGBB gruen
                     {"blend", 0}, {"blendavg", 1}, {"which_ch", 2}, {"nbands", 128}};
        const TranslationResult t = translateAvsTree(makeParsed({ts}));
        REQUIRE(t.root.children.size() == 1);
        const auto& p = std::get<TimescopeParams>(t.root.children[0].params);
        CHECK(p.color == 0x00FF00u);
        CHECK(p.blend == 2);   // blendavg -> 50/50
        CHECK(p.channel == 2);
        CHECK(p.bands == 128);
    }

    TEST_CASE("Starfield: Felder, Color-Swap + float-Bits gemappt")
    {
        std::int32_t warpBits = 0, beatBits = 0;
        const float warp = 8.0f, beat = 3.0f;
        std::memcpy(&warpBits, &warp, sizeof(float));
        std::memcpy(&beatBits, &beat, sizeof(float));

        EffectNode st = builtin(27);
        st.fields = {{"color", 0x000000FF},           // COLORREF -> RRGGBB rot
                     {"blend", 0}, {"blendavg", 0},
                     {"warpSpeed_bits", warpBits},     {"maxStars", 500},
                     {"onbeat", 1}, {"beatSpeed_bits", beatBits}, {"durFrames", 12}};

        const TranslationResult t = translateAvsTree(makeParsed({st}));
        REQUIRE(t.root.children.size() == 1);
        const auto& p = std::get<StarfieldParams>(t.root.children[0].params);
        CHECK(p.color == 0xFF0000u);
        CHECK(p.warpSpeed == doctest::Approx(8.0f));
        CHECK(p.maxStars == 500);
        CHECK(p.onBeat);
        CHECK(p.beatSpeed == doctest::Approx(3.0f));
    }

    TEST_CASE("Water Bump: Felder gemappt + geklammert")
    {
        EffectNode wb = builtin(31);
        wb.fields = {{"density", 20},  {"depth", 800},   {"random_drop", 0},
                     {"drop_x", 5},    {"drop_y", 2},    {"drop_radius", 0},
                     {"method", 0}};
        const TranslationResult t = translateAvsTree(makeParsed({wb}));
        REQUIRE(t.root.children.size() == 1);
        const auto& p = std::get<WaterBumpParams>(t.root.children[0].params);
        CHECK(p.density == 12);       // 20 -> clamp 12
        CHECK(p.depth == 800);
        CHECK_FALSE(p.randomDrop);
        CHECK(p.dropX == 2);          // 5 -> clamp 2
        CHECK(p.dropY == 2);
        CHECK(p.dropRadius == 1);     // 0 -> max 1
    }

    TEST_CASE("Bump: Felder + Licht-Code-Slots gemappt")
    {
        EffectNode b = builtin(29);
        b.fields = {{"depth", 40},  {"depth2", 90}, {"onbeat", 1}, {"durFrames", 10},
                    {"blend", 1},   {"blendavg", 0}, {"invert", 1}, {"oldstyle", 1}};
        b.code = {{"frame", "x=0.5"}, {"beat", ""}, {"init", "t=0"}};

        const TranslationResult t = translateAvsTree(makeParsed({b}));
        REQUIRE(t.root.children.size() == 1);
        const auto& p = std::get<BumpParams>(t.root.children[0].params);
        CHECK(p.depth == 40);
        CHECK(p.depth2 == 90);
        CHECK(p.onBeat);
        CHECK(p.invert);
        CHECK(p.oldStyle);
        CHECK(p.blend == 1);
        CHECK(p.frameCode == "x=0.5");
        CHECK(p.initCode == "t=0");
    }

    TEST_CASE("Interferences: Felder + speed-Bitmuster (float) gemappt")
    {
        std::int32_t speedBits = 0;
        const float sp = 0.5f;
        std::memcpy(&speedBits, &sp, sizeof(float));

        EffectNode e = builtin(41);
        e.fields = {{"nPoints", 4},    {"rotation", 10},    {"distance", 20},
                    {"alpha", 100},    {"rotationinc", 5},  {"blend", 1},
                    {"blendavg", 0},   {"distance2", 40},   {"alpha2", 200},
                    {"rotationinc2", 15}, {"rgb", 1},        {"onbeat", 1},
                    {"speed_bits", speedBits}};

        const TranslationResult t = translateAvsTree(makeParsed({e}));
        REQUIRE(t.root.children.size() == 1);
        const auto& p = std::get<InterferencesParams>(t.root.children[0].params);
        CHECK(p.points == 4);
        CHECK(p.alpha == 100);
        CHECK(p.distance2 == 40);
        CHECK(p.rgb);
        CHECK(p.onBeat);
        CHECK(p.blend == 1);
        CHECK(p.speed == doctest::Approx(0.5f));
    }

    // Korpus-Smoke: alle Referenz-Presets uebersetzen ohne Absturz (umgebungsabh.)
    TEST_CASE("Korpus: 35 Referenz-Presets uebersetzen ohne Absturz")
    {
        const std::filesystem::path dir = corpusDir();
        if (!std::filesystem::exists(dir))
        {
            MESSAGE("Referenz-Korpus nicht vorhanden - synthetischer Teil genuegt");
            return;
        }

        int files = 0;
        int totalEffects = 0;
        for (const auto& entry : std::filesystem::directory_iterator(dir))
        {
            if (entry.path().extension() != ".avs") continue;
            ++files;
            const ParseResult parsed = lumi::avs::parseFile(entry.path());
            const TranslationResult t = translateAvsTree(parsed);
            CHECK(t.root.isList());
            // Jeder Kindknoten hat einen gültigen (kompilierten) Zustand.
            CHECK(t.root.nodeId != 0);
            totalEffects += t.effectCount + t.passthroughCount;
        }
        CHECK(files >= 1);
        MESSAGE("Korpus: " << files << " Presets, " << totalEffects << " Knoten uebersetzt");
    }
}
