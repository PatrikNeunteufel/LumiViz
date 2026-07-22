/**
 ****************************************************************************************
 * @file   test_MilkShaderClassifier.cpp
 * @brief  Tests fuer die Warp-/Comp-Shader-Klassifikation (M5, Stufe B):
 *         generierte MD1-Defaults (GenWarp/GenCompPShaderText-Muster), Md1Plus-
 *         Extras (Blur-Mix/Gain), Custom-Erkennung, Feature-Flags
 *         + Korpus-Klassifikations-Gate ueber beide Preset-Packs
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <MilkParser.hpp>
#include <MilkShaderClassifier.hpp>

#include <filesystem>

using lumi::milk::ShaderClass;
using lumi::milk::ShaderInfo;
using lumi::milk::analyzeCompShader;
using lumi::milk::analyzeWarpShader;

namespace {

std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

std::filesystem::path milkdrop3CorpusDir()
{
    return repoRoot() / "asset" / "Milkdrop3" / "presets";
}

std::filesystem::path winampCorpusDir()
{
    return repoRoot().parent_path() / "ref" / "winamp_orig" / "Src" / "resources" / "data" /
           "Milkdrop2" / "presets";
}

} // namespace

// =============================================================================
// Warp: generierte Default-Familie
// =============================================================================

TEST_CASE("ShaderClassifier: leerer Text -> None")
{
    CHECK(analyzeWarpShader("").shaderClass == ShaderClass::None);
    CHECK(analyzeCompShader("").shaderClass == ShaderClass::None);
}

TEST_CASE("ShaderClassifier: generierter Warp-Default (GenWarpPShaderText)")
{
    const ShaderInfo info = analyzeWarpShader(
        "shader_body\n"
        "{\n"
        "    // sample previous frame\n"
        "    ret = tex2D( sampler_main, uv ).xyz;\n"
        "    \n"
        "    // darken (decay) over time\n"
        "    ret *= 0.98; //or try: ret -= 0.004;\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Default);
    CHECK(info.decayMul == doctest::Approx(0.98));
    CHECK(info.decaySub == doctest::Approx(0.0));
    CHECK(info.wrapSampler);
    CHECK(info.codeLines == 2);
}

TEST_CASE("ShaderClassifier: Warp-Default mit Clamp-Sampler (bWrap=0)")
{
    const ShaderInfo info = analyzeWarpShader(
        "shader_body\n{\n"
        "    ret = tex2D( sampler_fc_main, uv ).xyz;\n"
        "    ret *= 1.00; //or try: ret -= 0.004;\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Default);
    CHECK_FALSE(info.wrapSampler);
}

TEST_CASE("ShaderClassifier: Warp-Datei-Default (ret -= 0.004) -> subtraktiver Decay")
{
    const ShaderInfo info = analyzeWarpShader(
        "shader_body\n{\n"
        "    ret = tex2D( sampler_main, uv ).xyz;\n"
        "    ret -= 0.004;\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Default);
    CHECK(info.decayMul == doctest::Approx(-1.0));  // keine Multiplikations-Zeile
    CHECK(info.decaySub == doctest::Approx(0.004));
}

TEST_CASE("ShaderClassifier: unbekannte Warp-Zeile -> Custom + Feature-Flags")
{
    const ShaderInfo info = analyzeWarpShader(
        "shader_body\n{\n"
        "    ret = tex2D( sampler_main, uv ).xyz;\n"
        "    ret = max(ret, GetBlur2(uv));\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Custom);
    CHECK(info.usesBlur[1]);
    CHECK(info.codeLines == 2);
}

// =============================================================================
// Comp: generierte Default-Familie (eingebackene Konstanten)
// =============================================================================

TEST_CASE("ShaderClassifier: Comp-Default ohne Echo (Basis + Gamma)")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    ret = tex2D(sampler_main, uv).xyz;\n"
        "    ret *= 1.00; //gamma\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Default);
    CHECK(info.hasBase);
    CHECK(info.echoAlpha == doctest::Approx(0.0));
    CHECK(info.gain == doctest::Approx(1.0));
}

TEST_CASE("ShaderClassifier: Comp-Default mit Echo/Hue/Filtern (GenCompPShaderText)")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    float2 uv_echo = (uv - 0.5)*0.500*float2(1,-1) + 0.5;\n"
        "    ret = lerp( tex2D(sampler_main, uv).xyz, \n"
        "                tex2D(sampler_main, uv_echo).xyz, \n"
        "                0.30 \n"
        "              ); //video echo\n"
        "    ret *= 1.98; //gamma\n"
        "    ret *= 0.70 + 0.30*hue_shader; //old hue shader effect\n"
        "    ret = sqrt(ret); //brighten\n"
        "    ret = 1 - ret; //invert\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Default);
    CHECK(info.echoAlpha == doctest::Approx(0.30));
    CHECK(info.echoZoom == doctest::Approx(2.0));
    CHECK(info.echoOrient == 2);  // float2(1,-1) = V-Flip
    CHECK(info.gain == doctest::Approx(1.98));
    CHECK(info.hueMix == doctest::Approx(0.30));
    CHECK(info.brighten);
    CHECK(info.invert);
    CHECK_FALSE(info.darken);
    CHECK_FALSE(info.solarize);
}

TEST_CASE("ShaderClassifier: 'ret = ret; //brighten' ist die No-op-Schreibweise")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    ret = tex2D(sampler_main, uv).xyz;\n"
        "    ret *= 2.00; //gamma\n"
        "    ret = ret; //brighten\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Default);
    CHECK_FALSE(info.brighten);
    CHECK(info.gain == doctest::Approx(2.0));
}

// =============================================================================
// Comp: Md1Plus-Extras (Blur-Mix / Gain) — affines Modell
// =============================================================================

TEST_CASE("ShaderClassifier: 'ret += GetBlur1(uv)' -> Md1Plus mit Blur-Koeffizient")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    ret = tex2D(sampler_main, uv).xyz;\n"
        "    ret *= 1.00; //gamma\n"
        "    ret += GetBlur1(uv);\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Plus);
    CHECK(info.blurAdd[0] == doctest::Approx(1.0));
    CHECK(info.highestBlurLevel() == 1);
}

TEST_CASE("ShaderClassifier: Blur-Lerp ueberschreibt fruehere Rechnung (Original-Semantik)")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    ret = tex2D(sampler_main, uv).xyz;\n"
        "    ret *= 2.00; //gamma\n"
        "    ret = lerp(GetBlur2(uv),GetPixel(uv),0.4);\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Plus);
    CHECK(info.gain == doctest::Approx(0.4));       // Gamma-Faktor ist toter Code
    CHECK(info.blurAdd[1] == doctest::Approx(0.6));
    CHECK(info.highestBlurLevel() == 2);
}

TEST_CASE("ShaderClassifier: nachgestelltes 'ret *= k' skaliert auch die Blur-Terme")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    ret = tex2D(sampler_main, uv).xyz;\n"
        "    ret += GetBlur1(uv)*0.5;\n"
        "    ret *= 2.0;\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Plus);
    CHECK(info.gain == doctest::Approx(2.0));
    CHECK(info.blurAdd[0] == doctest::Approx(1.0));
}

TEST_CASE("ShaderClassifier: GetPixel*a + GetBlurN*b als Basis-Ersatz")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    ret = tex2D(sampler_main, uv).xyz;\n"
        "    ret = GetPixel(uv)*0.7 + GetBlur3(uv)*0.6;\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Md1Plus);
    CHECK(info.gain == doctest::Approx(0.7));
    CHECK(info.blurAdd[2] == doctest::Approx(0.6));
    CHECK(info.highestBlurLevel() == 3);
}

TEST_CASE("ShaderClassifier: negative Koeffizienten -> Custom (additives Compositing)")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    ret = tex2D(sampler_main, uv).xyz;\n"
        "    ret += GetBlur1(uv)*-0.5;\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Custom);
}

TEST_CASE("ShaderClassifier: affine Zeile NACH Filter-Zeile -> Custom (Reihenfolge fix)")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    ret = tex2D(sampler_main, uv).xyz;\n"
        "    ret = 1 - ret; //invert\n"
        "    ret *= 2.0;\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Custom);
}

// =============================================================================
// Feature-Flags (Praefix-bewusste Sampler-Erkennung)
// =============================================================================

TEST_CASE("ShaderClassifier: Noise-/Textur-/Zufalls-Erkennung inkl. fc/pc/fw/pw-Praefixe")
{
    const ShaderInfo info = analyzeCompShader(
        "shader_body\n{\n"
        "    float4 n = tex2D(sampler_pw_noise_lq, uv);\n"
        "    float4 t = tex2D(sampler_fw_clouds, uv + rand_frame.xy);\n"
        "    ret = n.xyz + t.xyz;\n"
        "}\n");
    CHECK(info.shaderClass == ShaderClass::Custom);
    CHECK(info.usesNoise);
    CHECK(info.usesTexture);
    CHECK(info.usesRand);
    REQUIRE(info.textures.size() == 1);
    CHECK(info.textures[0] == "clouds");
}

// =============================================================================
// Korpus-Gate: Klassifikation ueber beide Packs (falls lokal vorhanden)
// =============================================================================

namespace {

struct ClassCounts
{
    int files = 0;
    int noShader = 0;       // warp UND comp None
    int warpDefault = 0;    // Md1Default
    int warpCustom = 0;
    int compNone = 0;
    int compDefault = 0;
    int compPlus = 0;
    int compCustom = 0;
    int compBlurConsumers = 0;  // Md1Plus mit highestBlurLevel() > 0
};

ClassCounts classifyCorpus(const std::filesystem::path& dir)
{
    ClassCounts c;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
        const lumi::milk::ParseResult r = lumi::milk::parseFile(entry.path());
        if (!r.ok) continue;
        ++c.files;
        const ShaderInfo w = analyzeWarpShader(r.warpShader);
        const ShaderInfo p = analyzeCompShader(r.compShader);
        if (w.shaderClass == ShaderClass::None && p.shaderClass == ShaderClass::None)
            ++c.noShader;
        if (w.shaderClass == ShaderClass::Md1Default) ++c.warpDefault;
        if (w.shaderClass == ShaderClass::Custom) ++c.warpCustom;
        if (p.shaderClass == ShaderClass::Md1Default) ++c.compDefault;
        if (p.shaderClass == ShaderClass::Md1Plus)
        {
            ++c.compPlus;
            if (p.highestBlurLevel() > 0) ++c.compBlurConsumers;
        }
        if (p.shaderClass == ShaderClass::None) ++c.compNone;
        if (p.shaderClass == ShaderClass::Custom) ++c.compCustom;
    }
    return c;
}

} // namespace

TEST_CASE("ShaderClassifier: Korpus-Klassifikation beider Packs (Statistik-Gate M5)")
{
    const std::filesystem::path md3 = milkdrop3CorpusDir();
    const std::filesystem::path winamp = winampCorpusDir();
    if (!std::filesystem::exists(md3) || !std::filesystem::exists(winamp))
    {
        MESSAGE("Korpus nicht vollstaendig vorhanden — Gate uebersprungen");
        return;
    }
    ClassCounts total = classifyCorpus(md3);
    const ClassCounts w = classifyCorpus(winamp);
    total.files += w.files;
    total.noShader += w.noShader;
    total.warpDefault += w.warpDefault;
    total.warpCustom += w.warpCustom;
    total.compNone += w.compNone;
    total.compDefault += w.compDefault;
    total.compPlus += w.compPlus;
    total.compCustom += w.compCustom;
    total.compBlurConsumers += w.compBlurConsumers;

    MESSAGE("Korpus-Klassifikation: ", total.files, " Presets, ohne Shader ", total.noShader,
            "; warp Default/Custom = ", total.warpDefault, "/", total.warpCustom,
            "; comp Default/Plus/Custom = ", total.compDefault, "/", total.compPlus, "/",
            total.compCustom, "; Blur-Konsumenten (exakt) = ", total.compBlurConsumers);

    // Messwerte Session 40 (M5.1): 910 Presets, 310 ohne Shader, warp 20/554
    // Default/Custom, comp 20/13/565 Default/Plus/Custom, 13 exakte
    // Blur-Konsumenten. Floors statt Gleichheit: das Milkdrop3-Pack darf wachsen.
    CHECK(total.files >= 910);
    CHECK(total.noShader >= 310);
    CHECK(total.warpDefault >= 20);
    CHECK(total.compDefault >= 20);
    CHECK(total.compPlus >= 13);
    CHECK(total.compBlurConsumers >= 13);
    CHECK(total.compNone + total.compDefault + total.compPlus + total.compCustom ==
          total.files);
}
