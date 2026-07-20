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
    nestedList.blendIn = BlendMode::Additive;
    nestedList.blendOut = BlendMode::FiftyFifty;
    nestedList.inAdjustAlpha = 200;
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
        CHECK(effectTypeKey(EffectParams{PassthroughParams{}}) == "passthrough");
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
        CHECK(nl.blendIn == BlendMode::Additive);
        CHECK(nl.blendOut == BlendMode::FiftyFifty);
        CHECK(nl.inAdjustAlpha == 200);
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
