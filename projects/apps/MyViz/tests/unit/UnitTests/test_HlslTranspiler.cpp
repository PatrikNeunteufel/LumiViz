/**
 ****************************************************************************************
 * @file   test_HlslTranspiler.cpp
 * @brief  Tests fuer den HLSL→GLSL-Transpiler (Stufe C1): Ausdruecke, Swizzles,
 *         Intrinsics, Typ-Promotions, if/ternary, globale Deklarationen,
 *         C3-Grenzen (Loops/tex3D/Arrays) + Korpus-Messung ueber beide Packs
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <HlslTranspiler.hpp>
#include <MilkParser.hpp>

#include <filesystem>
#include <map>
#include <string>

using lumi::hlsl::HlslResult;
using lumi::hlsl::ShaderKind;
using lumi::hlsl::transpile;

namespace {

std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

/// shader_body-Wrapper fuer kompakte Fixtures
std::string body(const std::string& code)
{
    return "shader_body\n{\n" + code + "\n}\n";
}

[[nodiscard]] bool contains(const std::string& hay, const std::string& needle)
{
    return hay.find(needle) != std::string::npos;
}

} // namespace

// =============================================================================
// Grundlagen
// =============================================================================

TEST_CASE("HlslTranspiler: Basis-Zuweisung mit tex2D -> texture")
{
    const HlslResult r = transpile(body("ret = tex2D(sampler_main, uv).xyz;"),
                                   ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "ret = texture(sampler_main, uv).xyz;"));
}

TEST_CASE("HlslTranspiler: Skalar->Vektor-Promotion bei Zuweisung und Deklaration")
{
    const HlslResult r = transpile(body("float3 c = 0;\n"
                                        "ret = 0.5;\n"
                                        "c = ret * 2;"),
                                   ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "vec3 c = vec3(0.0);"));
    CHECK(contains(r.glslBody, "ret = vec3(0.5);"));
    CHECK(contains(r.glslBody, "c = (ret * 2.0);"));
}

TEST_CASE("HlslTranspiler: Integer-Literale bekommen Dezimalpunkt")
{
    const HlslResult r = transpile(body("ret = ret * 2 + float3(1, 2, 3);"),
                                   ShaderKind::Warp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "2.0"));
    CHECK(contains(r.glslBody, "vec3(1.0, 2.0, 3.0)"));
}

TEST_CASE("HlslTranspiler: Intrinsic-Mapping lerp/frac/saturate/atan2/fmod/rsqrt")
{
    const HlslResult r = transpile(
        body("float a = frac(atan2(uv.y, uv.x));\n"
             "float b = fmod(a, 0.5) + rsqrt(a + 1);\n"
             "ret = saturate(lerp(ret, GetBlur1(uv), b));"),
        ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "fract(atan(uv.y, uv.x))"));
    CHECK(contains(r.glslBody, "mod(a, 0.5)"));
    CHECK(contains(r.glslBody, "inversesqrt("));
    CHECK(contains(r.glslBody, "clamp(mix(ret, GetBlur1(uv), b), 0.0, 1.0)"));
}

TEST_CASE("HlslTranspiler: pow promotet Skalar-Exponent auf Vektor")
{
    const HlslResult r = transpile(body("ret = pow(ret, 2);"), ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "pow(ret, vec3(2.0))"));
}

TEST_CASE("HlslTranspiler: lerp mit gemischten a/b-Typen promotet auf Vektor")
{
    const HlslResult r = transpile(body("ret = lerp(0, GetPixel(uv), 0.3);"),
                                   ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "mix(vec3(0.0), GetPixel(uv), 0.3)"));
}

TEST_CASE("HlslTranspiler: numerische Bedingungen werden boolifiziert")
{
    const HlslResult r = transpile(body("float k = q1;\n"
                                        "if (k) { ret = 0; }\n"
                                        "ret = ret * (k > 0.5 ? 2.0 : 1.0);"),
                                   ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "if (((k) != 0.0))"));
    CHECK(contains(r.glslBody, "((k > 0.5) ? 2.0 : 1.0)"));
}

TEST_CASE("HlslTranspiler: float4 -> float3 Zuweisung kuerzt per Swizzle")
{
    const HlslResult r = transpile(body("ret = tex2D(sampler_main, uv);"),
                                   ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "(texture(sampler_main, uv)).xyz"));
}

TEST_CASE("HlslTranspiler: Splat-Swizzle auf Skalar (q1.xxx) wird Konstruktor")
{
    const HlslResult r = transpile(body("ret = q1.xxx;"), ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "ret = vec3(q1);"));
}

TEST_CASE("HlslTranspiler: mul() wird zu * (auch fuer Matrizen)")
{
    const HlslResult r = transpile(
        body("float2x2 m = float2x2(0.8, -0.6, 0.6, 0.8);\n"
             "float2 p = mul(uv - 0.5, m) + 0.5;\n"
             "ret = tex2D(sampler_main, p).xyz;"),
        ShaderKind::Warp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "mat2 m = mat2("));
    CHECK(contains(r.glslBody, "((uv - 0.5) * m)"));  // vec2 - float ist in GLSL gueltig
}

TEST_CASE("HlslTranspiler: globale sampler-/texsize-Deklarationen -> Features")
{
    const HlslResult r = transpile("sampler sampler_fw_clouds;\n"
                                   "float4 texsize_clouds;\n" +
                                       body("ret = tex2D(sampler_fw_clouds, uv).xyz;"),
                                   ShaderKind::Comp);
    REQUIRE(r.ok);
    REQUIRE(r.customSamplers.size() == 1);
    CHECK(r.customSamplers[0] == "sampler_fw_clouds");
    REQUIRE(r.customTexsizes.size() == 1);
    CHECK(r.customTexsizes[0] == "texsize_clouds");
}

TEST_CASE("HlslTranspiler: globale Variablen — Deklaration global, Init am main-Anfang")
{
    // GLSL-Global-Initialisierer muessen konstant sein (q1 & Co. sind Uniforms)
    // → Deklaration mit Null-Init global, Zuweisung als erstes Body-Statement
    const HlslResult r = transpile("float scale = 3.5;\nfloat2 d, e;\n" +
                                       body("ret = ret * scale + d.x + e.y;"),
                                   ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslGlobals, "float scale = float(0.0);"));
    CHECK(contains(r.glslGlobals, "vec2 d = vec2(0.0);"));
    CHECK(contains(r.glslBody, "scale = 3.5;"));
}

TEST_CASE("HlslTranspiler: #define-Makros (Objekt + Funktionsform) und Semantics")
{
    const HlslResult r = transpile(
        "#define TWO_PI 6.2831853\n"
        "#define rot2(a) float2x2(cos(a), -sin(a), sin(a), cos(a))\n"
        "float2 spin(float2 p : TEXCOORD0) : COLOR0 { return mul(p, rot2(TWO_PI*0.25)); }\n" +
            body("ret = tex2D(sampler_main, spin(uv - 0.5) + 0.5).xyz;"),
        ShaderKind::Warp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslGlobals, "vec2 spin(vec2 p)"));
    CHECK(contains(r.glslGlobals, "6.2831853"));
    CHECK(contains(r.glslBody, "spin("));
}

TEST_CASE("HlslTranspiler: Komma-Operator und Initialisierer-Liste")
{
    const HlslResult r = transpile(body("float3 c = {1, 0.5, 0};\n"
                                        "float a = (1.0, 2.0);\n"
                                        "ret = c * a;"),
                                   ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "vec3 c = vec3(1.0, 0.5, 0.0);"));
    CHECK(contains(r.glslBody, "(1.0, 2.0)"));
}

// =============================================================================
// C3-Grenzen: sauberer Fehler statt Falschuebersetzung
// =============================================================================

TEST_CASE("HlslTranspiler: Schleifen, Arrays, tex3D -> klare C3-Fehler")
{
    const HlslResult loop = transpile(
        body("for (int i=0; i<4; i+=1) { ret += GetBlur1(uv); }"), ShaderKind::Comp);
    CHECK_FALSE(loop.ok);
    CHECK(contains(loop.error, "C3"));

    const HlslResult arr = transpile(body("float w[4];"), ShaderKind::Comp);
    CHECK_FALSE(arr.ok);
    CHECK(contains(arr.error, "C3"));

    const HlslResult vol = transpile(
        body("ret = tex3D(sampler_noisevol_lq, float3(uv, time)).xyz;"), ShaderKind::Comp);
    CHECK_FALSE(vol.ok);
    CHECK(vol.usesTex3d);
}

TEST_CASE("HlslTranspiler: unbekannter Bezeichner -> Fehler mit Zeile")
{
    const HlslResult r = transpile(body("ret = nixda * 2;"), ShaderKind::Comp);
    CHECK_FALSE(r.ok);
    CHECK(contains(r.error, "nixda"));
    CHECK(contains(r.error, "Zeile"));
}

// =============================================================================
// Kalibrier-Satz c1: MUSS parser-warnungsfrei sein und vollstaendig transpiliern
// (Patrik testet Sicht nur mit dialogfreien Presets — Session-40-Vereinbarung)
// =============================================================================

TEST_CASE("HlslTranspiler: Kalibrier-Satz c1 — warnungsfrei + 100% uebersetzt")
{
    const std::filesystem::path dir =
        repoRoot() / "asset" / "calibration" / "milkdrop" / "c1";
    REQUIRE(std::filesystem::exists(dir));  // committeter Pflicht-Korpus
    int files = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
        ++files;
        const lumi::milk::ParseResult parsed = lumi::milk::parseFile(entry.path());
        REQUIRE(parsed.ok);
        CHECK_MESSAGE(parsed.warnings.empty(), entry.path().filename().string());
        if (!parsed.warpShader.empty())
        {
            const HlslResult r = transpile(parsed.warpShader, ShaderKind::Warp);
            CHECK_MESSAGE(r.ok, entry.path().filename().string(), ": ", r.error);
        }
        if (!parsed.compShader.empty())
        {
            const HlslResult r = transpile(parsed.compShader, ShaderKind::Comp);
            CHECK_MESSAGE(r.ok, entry.path().filename().string(), ": ", r.error);
        }
    }
    CHECK(files == 8);
}

// =============================================================================
// Korpus-Messung (falls lokal vorhanden) — C1-Abdeckung ueber beide Packs
// =============================================================================

TEST_CASE("HlslTranspiler: Korpus-Abdeckung warp/comp (Statistik-Gate C1)")
{
    const std::filesystem::path md3 = repoRoot() / "asset" / "Milkdrop3" / "presets";
    const std::filesystem::path winamp = repoRoot().parent_path() / "ref" / "winamp_orig" /
                                         "Src" / "resources" / "data" / "Milkdrop2" /
                                         "presets";
    if (!std::filesystem::exists(md3) || !std::filesystem::exists(winamp))
    {
        MESSAGE("Korpus nicht vollstaendig vorhanden — Gate uebersprungen");
        return;
    }

    int warpTotal = 0;
    int warpOk = 0;
    int compTotal = 0;
    int compOk = 0;
    std::map<std::string, int> errorKinds;
    std::string firstError;
    std::vector<std::string> samples;  // Diagnose: erste Fehlschlag-Schnipsel

    const auto run = [&](const std::filesystem::path& dir) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
            const lumi::milk::ParseResult parsed = lumi::milk::parseFile(entry.path());
            if (!parsed.ok) continue;
            const auto measure = [&](const std::string& text, ShaderKind kind, int& total,
                                     int& okCount) {
                if (text.empty()) return;
                ++total;
                const HlslResult r = transpile(text, kind);
                if (r.ok)
                {
                    ++okCount;
                    return;
                }
                std::string kind2 = r.error;
                if (const auto pos = kind2.find(": "); pos != std::string::npos)
                    kind2 = kind2.substr(pos + 2);
                const bool firstOfKind = errorKinds[kind2] == 0;
                ++errorKinds[kind2];
                if (firstOfKind && samples.size() < 8)
                {
                    samples.push_back(kind2 + "\n--- " +
                                      entry.path().filename().string() + " ---\n" +
                                      text.substr(0, 500));
                }
                if (firstError.empty())
                    firstError = entry.path().filename().string() + ": " + r.error;
            };
            measure(parsed.warpShader, ShaderKind::Warp, warpTotal, warpOk);
            measure(parsed.compShader, ShaderKind::Comp, compTotal, compOk);
        }
    };
    run(md3);
    run(winamp);

    std::string kinds;
    int shown = 0;
    for (const auto& [kind, count] : errorKinds)
    {
        if (++shown > 12) break;
        kinds += " | " + kind + " x" + std::to_string(count);
    }
    MESSAGE("HLSL-Korpus: warp ", warpOk, "/", warpTotal, " ok, comp ", compOk, "/",
            compTotal, " ok",
            (firstError.empty() ? std::string() : "; 1. Fehler: " + firstError), kinds);
    for (const std::string& s : samples) MESSAGE("SAMPLE >>> ", s);

    // Messwerte Session 40 (C1): warp 462/574 (80 %), comp 409/598 (68 %) —
    // der Rest ist ueberwiegend Stufe C3 (Loops/tex3D/Arrays/#if) + Langschwanz.
    CHECK(warpTotal >= 570);
    CHECK(compTotal >= 590);
    CHECK(warpOk >= 460);
    CHECK(compOk >= 405);
}
