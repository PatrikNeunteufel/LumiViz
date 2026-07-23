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

TEST_CASE("HlslTranspiler: implizite HLSL-Verengung bei Compound-Zuweisung (S43)")
{
    // GreatWho-Sichttest-Befund: fxc kuerzt breitere RHS-Vektoren bei += -= *=
    // stillschweigend auf die LHS-Breite — GLSL lehnt das ab (Kompilierfehler
    // -> stiller MD1-Fallback). Muster aus 'In The Spotlight V1/V2'.
    const HlslResult r = transpile(
        body("ret.xy *= 1 - GetBlur1(uv * sin(bass));\n"
             "ret -= tex2D(sampler_main, uv);"),
        ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, ").xy;"));   // vec3-RHS auf vec2 gekuerzt
    CHECK(contains(r.glslBody, ").xyz;"));  // vec4-RHS (tex2D) auf vec3 gekuerzt
}

TEST_CASE("HlslTranspiler: log10 -> log(x) * 1/ln(10)")
{
    // GreatWho - Addicted log10: GLSL 330 hat kein log10 (komponentenweise)
    const HlslResult r = transpile(body("ret = log10(ret + 1);"), ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "log("));
    CHECK(contains(r.glslBody, "0.43429448190325176"));
}

TEST_CASE("HlslTranspiler C3: while-Schleife mit int-Zaehler und n++ (S43)")
{
    // Muster aus 'martin + Stahlregen - martin in da mash 12b'
    const HlslResult r = transpile(
        std::string("float3 ret1, neu;\n") +
            body("int anz = 7;\n"
                 "int n = 0;\n"
                 "while (n <= anz) {\n"
                 "  neu = tex2D(sampler_main, uv);\n"
                 "  ret1 = max(ret1, neu - .0);\n"
                 "  n++;\n"
                 "}\n"
                 "ret = ret1;"),
        ShaderKind::Comp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "while ("));
    CHECK(contains(r.glslBody, "(n++)"));
}

TEST_CASE("HlslTranspiler C3: for-Schleife mit Init-Deklaration + break")
{
    const HlslResult r = transpile(
        body("float sum = 0;\n"
             "for (int i = 0; i < 8; i++) {\n"
             "  sum += tex2D(sampler_main, uv + i * 0.01).x;\n"
             "  if (sum > 3) break;\n"
             "}\n"
             "ret = sum;"),
        ShaderKind::Warp);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "for (; "));
    CHECK(contains(r.glslBody, "break;"));
}

TEST_CASE("HlslTranspiler C3: tex3D auf noisevol (auch als 'sampler' redeklariert)")
{
    const HlslResult r = transpile(
        std::string("sampler sampler_noisevol_hq;\n") +
            body("ret = tex3D(sampler_noisevol_hq, float3(uv, time * 0.1)).xyz;"),
        ShaderKind::Warp);
    REQUIRE(r.ok);
    CHECK(r.usesTex3d);
    CHECK(contains(r.glslBody, "texture(sampler_noisevol_hq, "));
    CHECK(r.customSamplers.empty());  // Builtin — kein Custom-Sampler
}

TEST_CASE("HlslTranspiler C3: include.fx-Konstanten + Alias-#define auf Funktion")
{
    // 'chaos layered tokamak' (M_INV_PI_2) + 'glassworks 3' (#define sat saturate)
    const HlslResult r = transpile(
        std::string("#define sat saturate\n") +
            body("float a = ang * M_INV_PI_2;\n"
                 "ret = sat(a + M_PI - M_PI_2);"),
        ShaderKind::Comp);
    INFO(r.error);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "M_INV_PI_2"));
    CHECK(contains(r.glslBody, "clamp("));
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

TEST_CASE("HlslTranspiler C3-Rest: Arrays, do/while, #if, q-Schattenkopie")
{
    const HlslResult arr = transpile(
        body("float w[4];\n"
             "for (int i = 0; i < 4; i++) w[i] = i * 0.25;\n"
             "ret = w[1] + w[3];"),
        ShaderKind::Comp);
    INFO(arr.error);
    REQUIRE(arr.ok);
    CHECK(contains(arr.glslBody, "float w[4];"));
    CHECK(contains(arr.glslBody, "w[int("));

    const HlslResult dw = transpile(
        body("int n = 0;\ndo { n++; } while (n < 4);\nret = n;"), ShaderKind::Comp);
    INFO(dw.error);
    REQUIRE(dw.ok);
    CHECK(contains(dw.glslBody, "} while ("));

    const HlslResult pp = transpile(
        body("#if 1\nret = 1;\n#else\nret = 0;\n#endif"), ShaderKind::Comp);
    INFO(pp.error);
    REQUIRE(pp.ok);
    CHECK(contains(pp.glslBody, "ret = vec3(1.0)"));
    CHECK_FALSE(contains(pp.glslBody, "ret = vec3(0.0)"));

    // Preset schreibt eine q-Uniform -> lokale Schattenkopie im Prolog
    const HlslResult qs = transpile(body("q5 = q5 * 2;\nret = q5;"), ShaderKind::Comp);
    INFO(qs.error);
    REQUIRE(qs.ok);
    CHECK(contains(qs.glslBody, "float q5 = q5;"));
}

TEST_CASE("HlslTranspiler C3-Rest: float2x3 + mul-Formen (glassworks-Muster)")
{
    const HlslResult r = transpile(
        std::string("static const float3 t = float3(1,0,0), s = float3(0,1,0);\n") +
            body("float3 screen = float3(uv, 1);\n"
                 "float z = 0.08 / mul(cross(t, s), screen);\n"
                 "ret = float3(mul(float2x3(t, s), screen) * z, -z);"),
        ShaderKind::Comp);
    INFO(r.error);
    REQUIRE(r.ok);
    CHECK(contains(r.glslBody, "mat3x2("));  // HLSL float2x3 = GLSL mat3x2
    CHECK(contains(r.glslBody, "dot("));     // mul(vec, vec) = Skalarprodukt
}

TEST_CASE("HlslTranspiler C3-Rest: Bool-Arithmetik + out-Parameter")
{
    // `x * (a > b)` — beliebtes Gate-Muster (fxc: bool implizit numerisch)
    const HlslResult ba = transpile(
        body("float2 dz = uv;\n"
             "dz *= GetBlur1(uv).x > 0.06;\n"
             "ret = float3(dz * (rad > 0.5), 0);"),
        ShaderKind::Comp);
    INFO(ba.error);
    REQUIRE(ba.ok);
    CHECK(contains(ba.glslBody, "? 1.0 : 0.0"));

    const HlslResult op = transpile(
        std::string("void split(float x, out float ip, out float fp) {\n"
                    "  ip = floor(x); fp = x - ip; }\n") +
            body("float a; float b;\nsplit(rad * 4, a, b);\nret = a + b;"),
        ShaderKind::Comp);
    INFO(op.error);
    REQUIRE(op.ok);
    CHECK(contains(op.glslGlobals, "out float ip"));
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
