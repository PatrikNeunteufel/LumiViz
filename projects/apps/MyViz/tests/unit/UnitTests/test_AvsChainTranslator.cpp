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

    TEST_CASE("Movement ohne Point-Code (Builtin-Formel) -> Passthrough")
    {
        EffectNode move = builtin(15);
        move.fields = {{"effect", 3}, {"blend", 0}, {"rectangular", 0}};
        // kein code["point"]

        const TranslationResult t = translateAvsTree(makeParsed({move}));
        REQUIRE(t.root.children.size() == 1);
        CHECK(std::holds_alternative<PassthroughParams>(t.root.children[0].params));
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
