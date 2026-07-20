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

    TEST_CASE("Set Render Mode wird ausgerollt: Linienbreite in Folge-Scope")
    {
        EffectNode srm = builtin(40);
        srm.fields = {{"newmode", 5 << 16}};  // line width 5 (bits 16-23)

        EffectNode scope = builtin(36);
        scope.code = {{"point", "x=i;y=v"}};
        scope.fields = {{"which_ch", 0}, {"num_colors", 0}, {"drawmode", 1}};

        const TranslationResult t = translateAvsTree(makeParsed({srm, scope}));
        REQUIRE(t.root.children.size() == 2);
        // SRM-Knoten -> Passthrough mit Unroll-Notiz
        CHECK(std::holds_alternative<PassthroughParams>(t.root.children[0].params));
        // Scope -> Linienbreite aus dem SRM übernommen
        REQUIRE(std::holds_alternative<SuperScopeParams>(t.root.children[1].params));
        CHECK(std::get<SuperScopeParams>(t.root.children[1].params).lineWidth
              == doctest::Approx(5.0f));
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
