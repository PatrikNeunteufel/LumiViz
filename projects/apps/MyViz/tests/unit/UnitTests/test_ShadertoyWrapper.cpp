/**
 ****************************************************************************************
 * @file   test_ShadertoyWrapper.cpp
 * @brief  Tests fuer den GLSL-ES→330-Wrapper des Shadertoy-Nodes (Strang S, S65):
 *         Praelude-Vertrag (Uniform-Satz, #line 1), Einbettung, Blend-Epilog,
 *         Starter-Shader, Serializer-Roundtrip des Node-Typs
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/ShadertoyImport.hpp"
#include "visualizers/multieffect/ShadertoyWrapper.hpp"

#include <QJsonObject>

#include <string>

using lumi::multieffect::ChainNode;
using lumi::multieffect::EffectParams;
using lumi::multieffect::IsfFilterParams;
using lumi::multieffect::ShadertoyParams;

namespace {

[[nodiscard]] bool contains(const std::string& hay, const std::string& needle)
{
    return hay.find(needle) != std::string::npos;
}

} // namespace

TEST_CASE("ShadertoyWrapper: Praelude traegt den vollen Uniform-Satz + #line 1")
{
    const std::string frag = lumi::shadertoy::wrapFragment(
        "void mainImage(out vec4 c, in vec2 p) { c = vec4(1.0); }\n");

    // GLSL-330-Kopf, einmalig
    CHECK(frag.rfind("#version 330 core", 0) == 0);

    // Shadertoy-Uniform-Vertrag (Plan §S1)
    for (const char* u :
         {"uniform vec3 iResolution", "uniform float iTime", "uniform float iTimeDelta",
          "uniform int iFrame", "uniform float iFrameRate", "uniform vec4 iMouse",
          "uniform vec4 iDate", "uniform float iSampleRate",
          "uniform float iChannelTime[4]", "uniform vec3 iChannelResolution[4]",
          "uniform sampler2D iChannel0", "uniform sampler2D iChannel3"})
    {
        CHECK_MESSAGE(contains(frag, u), u);
    }
    // LumiViz-Audio-Uniforms (Plan §S2 — Nachruestbarkeit)
    for (const char* u : {"uniform float bass", "uniform float mid",
                          "uniform float treb", "uniform float vol",
                          "uniform float beat"})
    {
        CHECK_MESSAGE(contains(frag, u), u);
    }

    // #line 1 steht DIREKT vor dem Nutzer-Code: Treiber-Logs melden Nutzer-Zeilen
    const std::size_t line1 = frag.find("#line 1\n");
    REQUIRE(line1 != std::string::npos);
    CHECK(frag.find("void mainImage") == line1 + 8);
    // Wrapper-Zeilen sind per #line 100000 unverwechselbar markiert
    CHECK(contains(frag, "#line 100000"));

    // Epilog: main() ruft mainImage mit gl_FragCoord und blendet gegen _lumiPrev
    CHECK(contains(frag, "mainImage(_lumiColor, gl_FragCoord.xy)"));
    CHECK(contains(frag, "_lumiBlend"));
    CHECK(contains(frag, "_lumiPrev"));
}

TEST_CASE("ShadertoyWrapper: Nutzer-Code ohne Schluss-Newline wird sauber getrennt")
{
    const std::string frag =
        lumi::shadertoy::wrapFragment("void mainImage(out vec4 c, in vec2 p){c=vec4(0);}");
    // kein Zusammenkleben von Nutzer-Code und Epilog auf einer Zeile
    CHECK(contains(frag, "c=vec4(0);}\n"));
}

TEST_CASE("ShadertoyWrapper: Starter-Shader ist ein kompletter mainImage-Block")
{
    const std::string starter = lumi::shadertoy::starterShader();
    CHECK(contains(starter, "void mainImage"));
    CHECK(contains(starter, "iChannel0"));  // Audio-Textur wird vorgefuehrt
    CHECK(contains(starter, "bass"));       // LumiViz-Uniform wird vorgefuehrt
}

TEST_CASE("ChainSerializer: Shadertoy-Node roundtrippt (inkl. Metadaten)")
{
    ShadertoyParams p;
    p.code = "void mainImage(out vec4 c, in vec2 f) { c = vec4(f.x); }";
    p.imageInput = {-1, -1, lumi::multieffect::kShadertoyInputAudio, -1};
    p.blend = 1;
    p.herkunft.name = "Testbild";
    p.herkunft.author = "LumiViz";
    p.herkunft.url = "https://example.invalid/xyz";
    p.herkunft.license = "CC BY-NC-SA 3.0";

    ChainNode node;
    node.params = p;
    const QJsonObject o = lumi::multieffect::nodeToJson(node);
    CHECK(o.value("type").toString() == "shadertoy");

    QStringList report;
    const ChainNode back = lumi::multieffect::nodeFromJson(o, &report);
    const auto* q = std::get_if<ShadertoyParams>(&back.params);
    REQUIRE(q != nullptr);
    CHECK(q->code == p.code);
    CHECK(lumi::multieffect::shadertoyAudioChannel(q->imageInput) == 2);
    CHECK(q->imageInput[2] == lumi::multieffect::kShadertoyInputAudio);
    CHECK(q->blend == 1);
    CHECK(q->herkunft.name == p.herkunft.name);
    CHECK(q->herkunft.author == p.herkunft.author);
    CHECK(q->herkunft.url == p.herkunft.url);
    CHECK(q->herkunft.license == p.herkunft.license);
    // Die Herkunft steht in einem EIGENEN Unterobjekt (S72) …
    REQUIRE(o.value("herkunft").isObject());
    CHECK(o.value("herkunft").toObject().value("name").toString() == "Testbild");
    CHECK(o.value("herkunft").toObject().value("author").toString() == "LumiViz");
    CHECK(o.value("herkunft").toObject().value("license").toString() ==
          "CC BY-NC-SA 3.0");

    // fehlende Felder ⇒ Defaults (audioChannel 0, blend 0, Herkunft leer)
    QJsonObject bare;
    bare["type"] = "shadertoy";
    bare["code"] = "void mainImage(out vec4 c, in vec2 f) { c = vec4(0.0); }";
    const ChainNode d = lumi::multieffect::nodeFromJson(bare, &report);
    const auto* qd = std::get_if<ShadertoyParams>(&d.params);
    REQUIRE(qd != nullptr);
    CHECK(lumi::multieffect::shadertoyAudioChannel(qd->imageInput) == 0);
    CHECK(qd->blend == 0);
    CHECK(qd->herkunft.leer());
    // Leere Herkunft schreibt KEINEN Schluessel (schlankes JSON — und der
    // Grund, warum sie nicht im Feld-Inventar auftaucht).
    CHECK_FALSE(lumi::multieffect::nodeToJson(d).contains("herkunft"));
}

TEST_CASE("ChainSerializer: die neuen iChannel-Quellen roundtrippen (S72)")
{
    using namespace lumi::multieffect;
    // Die Kodierung ist SSOT im Header — die Reihenfolge haengt daran, weil
    // der Panel-Combo-Index schlicht `Wert + 1` ist.
    CHECK(kShadertoyInputAudio == 4);
    CHECK(kShadertoyInputChain == 5);
    CHECK(kShadertoyInputAvsBase == 6);
    CHECK(kShadertoyInputMax == 13);

    ShadertoyParams p;
    p.code = "void mainImage(out vec4 c, in vec2 f) { c = vec4(0.0); }";
    p.imageInput = {kShadertoyInputChain,      // Ketten-Eingang
                    kShadertoyInputAvsBase,     // AVS-Buffer 1
                    kShadertoyInputMax,         // AVS-Buffer 8
                    kShadertoyInputAudio};

    ChainNode node;
    node.params = p;
    QStringList report;
    const ChainNode back =
        lumi::multieffect::nodeFromJson(lumi::multieffect::nodeToJson(node), &report);
    const auto* q = std::get_if<ShadertoyParams>(&back.params);
    REQUIRE(q != nullptr);
    CHECK(q->imageInput[0] == kShadertoyInputChain);
    CHECK(q->imageInput[1] == kShadertoyInputAvsBase);
    CHECK(q->imageInput[2] == kShadertoyInputMax);
    CHECK(lumi::multieffect::shadertoyAudioChannel(q->imageInput) == 3);

    // Ein Wert JENSEITS der Kodierung wird geklemmt — sonst bindet der
    // Renderer einen Slot, den es nicht gibt.
    QJsonObject o;
    o["type"] = "shadertoy";
    QJsonArray zuGross;
    zuGross.append(99);
    zuGross.append(-7);
    zuGross.append(kShadertoyInputMax);
    zuGross.append(0);
    o["imageInput"] = zuGross;
    const ChainNode k = lumi::multieffect::nodeFromJson(o, &report);
    const auto* qk = std::get_if<ShadertoyParams>(&k.params);
    REQUIRE(qk != nullptr);
    CHECK(qk->imageInput[0] == kShadertoyInputMax);
    CHECK(qk->imageInput[1] == kShadertoyInputNone);
}

TEST_CASE("ChainSerializer: Herkunft kollidiert nicht mehr mit dem Knotennamen")
{
    // BEFUND S72: bis S71 schrieb der Shadertoy-Visitor `name` FLACH an den
    // Knoten — dorthin, wo `nodeToJson` schon `o["name"] = displayName` gelegt
    // hatte. Der Visitor laeuft danach, also gewann der Shader-Name: ein
    // umbenannter Knoten hiess nach dem Speichern wieder wie sein Shader.
    ShadertoyParams p;
    p.code = "void mainImage(out vec4 c, in vec2 f) { c = vec4(1.0); }";
    p.herkunft.name = "Seascape";
    p.herkunft.author = "TDM";

    ChainNode node;
    node.params = p;
    node.displayName = "Mein Wasser";  // <- der Nutzer hat umbenannt

    QStringList report;
    const ChainNode back =
        lumi::multieffect::nodeFromJson(lumi::multieffect::nodeToJson(node), &report);
    CHECK(back.displayName == "Mein Wasser");
    const auto* q = std::get_if<ShadertoyParams>(&back.params);
    REQUIRE(q != nullptr);
    CHECK(q->herkunft.name == "Seascape");
}

TEST_CASE("ChainSerializer: Alt-Format der Shadertoy-Metadaten liest weiter")
{
    // Bestehende `.lvfx` tragen die vier Felder flach am Knoten. Sie muessen
    // ohne Zutun in der `Herkunft` landen (Abwaertskompatibilitaet S72).
    QJsonObject alt;
    alt["type"] = "shadertoy";
    alt["code"] = "void mainImage(out vec4 c, in vec2 f) { c = vec4(0.5); }";
    alt["name"] = "Altes Testbild";
    alt["author"] = "autorin";
    alt["url"] = "https://www.shadertoy.com/view/Xds3zN";
    alt["license"] = "CC BY-NC-SA 3.0";

    QStringList report;
    const ChainNode n = lumi::multieffect::nodeFromJson(alt, &report);
    const auto* q = std::get_if<ShadertoyParams>(&n.params);
    REQUIRE(q != nullptr);
    CHECK(q->herkunft.name == "Altes Testbild");
    CHECK(q->herkunft.author == "autorin");
    CHECK(q->herkunft.url == "https://www.shadertoy.com/view/Xds3zN");
    CHECK(q->herkunft.license == "CC BY-NC-SA 3.0");

    // Gegenprobe: ein Knoten OHNE Herkunftsfelder darf seinen Anzeigenamen
    // nicht als Herkunft missverstehen — sonst bekaeme jeder alte Knoten eine
    // Pseudo-Herkunft aus dem eigenen Namen.
    QJsonObject schlicht;
    schlicht["type"] = "isfFilter";
    schlicht["name"] = "Mein Filter";
    const ChainNode s = lumi::multieffect::nodeFromJson(schlicht, &report);
    const auto* sf = std::get_if<IsfFilterParams>(&s.params);
    REQUIRE(sf != nullptr);
    CHECK(s.displayName == "Mein Filter");
    CHECK(sf->herkunft.leer());

    CHECK(std::string(lumi::multieffect::effectTypeName(EffectParams{ShadertoyParams{}})) ==
          "Shadertoy");
}

// --- Strang S3: URL-/ID-Import (netz-/GL-freie Haelfte) -----------------------

TEST_CASE("ShadertoyImport: extractShaderId erkennt URL-Formen und Roh-IDs")
{
    using lumi::shadertoy::extractShaderId;
    CHECK(extractShaderId("https://www.shadertoy.com/view/Xds3zN") == "Xds3zN");
    CHECK(extractShaderId("http://shadertoy.com/view/Xds3zN?paused=true") == "Xds3zN");
    CHECK(extractShaderId("www.shadertoy.com/view/Xds3zN#comment") == "Xds3zN");
    CHECK(extractShaderId("  Xds3zN  ") == "Xds3zN");
    CHECK(extractShaderId("") == "");
    CHECK(extractShaderId("https://example.com/view/Xds3zN") == "");
    CHECK(extractShaderId("kein shader") == "");

    const QUrl url = lumi::shadertoy::apiRequestUrl("Xds3zN", "MEINKEY");
    CHECK(url.toString() ==
          "https://www.shadertoy.com/api/v1/shaders/Xds3zN?key=MEINKEY");
}

TEST_CASE("ShadertoyImport: parseApiReply uebernimmt image+common, mappt Audio")
{
    const QByteArray reply = R"({
        "Shader": {
            "info": {"id": "Xds3zN", "name": "Testshader", "username": "autorin"},
            "renderpass": [
                {"type": "buffer", "name": "Buffer A", "code": "// buf",
                 "inputs": []},
                {"type": "common", "code": "float helper() { return 1.0; }",
                 "inputs": []},
                {"type": "image",
                 "code": "void mainImage(out vec4 c, in vec2 f){c=vec4(helper());}",
                 "inputs": [
                    {"channel": 1, "ctype": "music"},
                    {"channel": 2, "ctype": "texture"}
                 ]}
            ]
        }
    })";
    const auto r = lumi::shadertoy::parseApiReply(reply, "Xds3zN");
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());
    CHECK(r.params.herkunft.name == "Testshader");
    CHECK(r.params.herkunft.author == "autorin");
    CHECK(r.params.herkunft.url == "https://www.shadertoy.com/view/Xds3zN");
    CHECK(r.params.herkunft.license.find("CC BY-NC-SA") != std::string::npos);
    // common wird vorangestellt, image folgt
    CHECK(r.params.code.find("float helper()") != std::string::npos);
    CHECK(r.params.code.find("void mainImage") != std::string::npos);
    CHECK(r.params.code.find("float helper()") < r.params.code.find("void mainImage"));
    // music-Input auf iChannel1 gemappt; Textur + Multipass im Report
    CHECK(lumi::multieffect::shadertoyAudioChannel(r.params.imageInput) == 1);
    CHECK(r.params.imageInput[1] == lumi::multieffect::kShadertoyInputAudio);
    // S4: der buffer-Pass wird ÜBERNOMMEN (common ebenfalls vorangestellt)
    REQUIRE(r.params.buffers.size() == 1);
    CHECK(r.params.buffers[0].code.find("float helper()") != std::string::npos);
    CHECK(r.params.buffers[0].code.find("// buf") != std::string::npos);
    REQUIRE(r.report.size() == 3);
    CHECK(r.report[0].contains("iChannel1"));
    CHECK(r.report[1].contains("texture"));
    CHECK(r.report[2].contains("Multipass"));
}

TEST_CASE("ShadertoyImport: Buffer-Topologie (S4) — Output-Ids werden aufgeloest")
{
    const QByteArray reply = R"({
        "Shader": {
            "info": {"id": "abc123", "name": "Feedback", "username": "u"},
            "renderpass": [
                {"type": "buffer", "name": "Buf A",
                 "code": "void mainImage(out vec4 c, in vec2 f){ /*A*/ }",
                 "outputs": [{"id": 257, "channel": 0}],
                 "inputs": [{"channel": 0, "ctype": "buffer", "id": 257}]},
                {"type": "image",
                 "code": "void mainImage(out vec4 c, in vec2 f){}",
                 "inputs": [
                    {"channel": 2, "ctype": "buffer", "id": 257},
                    {"channel": 0, "ctype": "music"}
                 ]}
            ]
        }
    })";
    const auto r = lumi::shadertoy::parseApiReply(reply, "abc123");
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());
    REQUIRE(r.params.buffers.size() == 1);
    // Selbst-Referenz von Buffer A auf iChannel0 (liest das Vorframe)
    CHECK(r.params.buffers[0].input[0] == 0);
    // Image liest Buffer A auf iChannel2, Audio auf iChannel0
    CHECK(r.params.imageInput[2] == 0);
    CHECK(r.params.imageInput[0] == lumi::multieffect::kShadertoyInputAudio);
    CHECK(lumi::multieffect::shadertoyAudioChannel(r.params.imageInput) == 0);
}

TEST_CASE("ShadertoyWrapper: Buffer-Epilog schreibt roh (kein Blend/Clamp)")
{
    const std::string frag = lumi::shadertoy::wrapBufferFragment(
        "void mainImage(out vec4 c, in vec2 p) { c = vec4(-2.0); }\n");
    CHECK(frag.find("#line 1") != std::string::npos);
    CHECK(frag.find("_lumiFrag = _lumiColor;") != std::string::npos);
    CHECK(frag.find("_lumiPrevRgb") == std::string::npos);  // kein Blend-Epilog
}

TEST_CASE("ChainSerializer: Shadertoy-Buffer roundtrippen + Alt-Migration (S4)")
{
    using lumi::multieffect::kShadertoyInputAudio;
    using lumi::multieffect::nodeFromJson;
    using lumi::multieffect::nodeToJson;

    ShadertoyParams p;
    p.code = "void mainImage(out vec4 c, in vec2 f){}";
    p.imageInput = {kShadertoyInputAudio, -1, 0, -1};
    lumi::multieffect::ShadertoyPass buf;
    buf.code = "void mainImage(out vec4 c, in vec2 f){ /*A*/ }";
    buf.input = {0, -1, -1, kShadertoyInputAudio};
    p.buffers.push_back(buf);

    ChainNode node;
    node.params = p;
    QStringList report;
    const ChainNode back = nodeFromJson(nodeToJson(node), &report);
    const auto* q = std::get_if<ShadertoyParams>(&back.params);
    REQUIRE(q != nullptr);
    CHECK(q->imageInput == p.imageInput);
    CHECK(lumi::multieffect::shadertoyAudioChannel(q->imageInput) == 0);
    REQUIRE(q->buffers.size() == 1);
    CHECK(q->buffers[0].code == buf.code);
    CHECK(q->buffers[0].input == buf.input);

    // Migration: Bestands-Dokument (S1–S3) kennt nur audioChannel
    QJsonObject alt;
    alt["type"] = "shadertoy";
    alt["code"] = "void mainImage(out vec4 c, in vec2 f){}";
    alt["audioChannel"] = 2;
    const ChainNode m = nodeFromJson(alt, &report);
    const auto* qm = std::get_if<ShadertoyParams>(&m.params);
    REQUIRE(qm != nullptr);
    CHECK(qm->imageInput[2] == kShadertoyInputAudio);
    CHECK(lumi::multieffect::shadertoyAudioChannel(qm->imageInput) == 2);
    CHECK(qm->buffers.empty());
}

TEST_CASE("ShadertoyImport: Query-URL + Antwort-Parser (Browser-Panel)")
{
    using lumi::shadertoy::parseQueryReply;
    using lumi::shadertoy::queryRequestUrl;

    CHECK(queryRequestUrl("ocean waves", "popular", "KEY").toString(QUrl::FullyEncoded) ==
          "https://www.shadertoy.com/api/v1/shaders/query/"
          "ocean%20waves?sort=popular&from=0&num=24&key=KEY");
    // leerer Suchtext = Top der Sortierung (kein Pfad-Segment)
    CHECK(queryRequestUrl("  ", "newest", "KEY", 8).toString() ==
          "https://www.shadertoy.com/api/v1/shaders/query"
          "?sort=newest&from=0&num=8&key=KEY");
    CHECK(lumi::shadertoy::thumbnailUrl("Xds3zN").toString() ==
          "https://www.shadertoy.com/media/shaders/Xds3zN.jpg");

    const auto ok = parseQueryReply(R"({"Shaders": 42, "Results": ["aaa", "bbb"]})");
    REQUIRE(ok.ok);
    CHECK(ok.total == 42);
    REQUIRE(ok.ids.size() == 2);
    CHECK(ok.ids[0] == "aaa");

    const auto apiErr = parseQueryReply(R"({"Error":"Invalid key"})");
    CHECK_FALSE(apiErr.ok);
    CHECK(apiErr.error.contains("Invalid key"));

    const auto empty = parseQueryReply(R"({"Shaders": 0, "Results": null})");
    CHECK(empty.ok);
    CHECK(empty.ids.isEmpty());

    CHECK_FALSE(parseQueryReply("<html>").ok);
}

TEST_CASE("ShadertoyImport: parseApiReply meldet Fehlerfaelle sauber")
{
    // API-Error (nicht gefunden / nicht public+api)
    const auto notFound =
        lumi::shadertoy::parseApiReply(R"({"Error":"Shader not found"})", "abc123");
    CHECK_FALSE(notFound.ok);
    CHECK(notFound.error.contains("Shader not found"));

    // kaputtes JSON
    const auto broken = lumi::shadertoy::parseApiReply("<html>502</html>", "abc123");
    CHECK_FALSE(broken.ok);

    // Antwort ohne image-Pass
    const auto soundOnly = lumi::shadertoy::parseApiReply(
        R"({"Shader":{"info":{},"renderpass":[{"type":"sound","code":"x"}]}})",
        "abc123");
    CHECK_FALSE(soundOnly.ok);
    CHECK(soundOnly.error.contains("image"));

    // ohne Audio-Input bleibt der Kanal aus (-1)
    const auto plain = lumi::shadertoy::parseApiReply(
        R"({"Shader":{"info":{"name":"n","username":"u"},
            "renderpass":[{"type":"image","code":"void mainImage(out vec4 c, in vec2 f){}","inputs":[]}]}})",
        "abc123");
    REQUIRE(plain.ok);
    CHECK(lumi::multieffect::shadertoyAudioChannel(plain.params.imageInput) == -1);
    CHECK(plain.report.isEmpty());
}
