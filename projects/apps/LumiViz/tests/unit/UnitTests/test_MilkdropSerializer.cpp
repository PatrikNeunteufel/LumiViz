/**
 ****************************************************************************************
 * @file   test_MilkdropSerializer.cpp
 * @brief  Tests fuer die .lvfx-Schwester-Persistenz des Milkdrop-Hosts (M6):
 *         Voll-Roundtrip (Scalars, Code, Shader, Waves/Shapes, Blur), Defaults
 *         bei fehlenden Feldern, Typ-Erkennung, Korpus-Roundtrip-Gate
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <MilkParser.hpp>

#include "visualizers/milkdrop/MilkdropPresetState.hpp"
#include "visualizers/milkdrop/MilkdropSerializer.hpp"

#include <QFile>
#include <QJsonObject>
#include <QTemporaryDir>

#include <filesystem>

using lumi::milk::parse;
using lumi::milkdrop::PresetState;
using lumi::milkdrop::presetFromJson;
using lumi::milkdrop::presetToJson;
using lumi::milkdrop::translate;

namespace {

std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

/// Synthetisches Preset mit allen Baustein-Arten (Wave, Shape, Shader, Blur)
PresetState makeFullState()
{
    const auto r = parse(
        "MILKDROP_PRESET_VERSION=201\n"
        "PSVERSION=2\n"
        "[preset00]\n"
        "fDecay=0.955\n"
        "fGammaAdj=1.700\n"
        "fVideoEchoAlpha=0.350\n"
        "fVideoEchoZoom=1.250\n"
        "nVideoEchoOrientation=3\n"
        "bInvert=1\n"
        "nWaveMode=5\n"
        "wave_r=0.100\nwave_g=0.200\nwave_b=0.300\n"
        "zoom=1.023\nrot=0.020\ncx=0.400\ncy=0.600\nwarp=0.750\n"
        "ob_size=0.020\nob_r=0.500\n"
        "mv_a=0.250\nmv_x=16.000\n"
        "b1n=0.100\nb1x=0.900\nb2n=0.200\nb3x=0.800\nb1ed=0.500\n"
        "per_frame_init_1=q1=0.5;\n"
        "per_frame_1=zoom=zoom+0.01*sin(time);\n"
        "per_pixel_1=rot=rot+0.02*rad;\n"
        "wavecode_0_enabled=1\n"
        "wavecode_0_samples=256\n"
        "wavecode_0_bSpectrum=1\n"
        "wavecode_0_scaling=1.500\n"
        "wave_0_per_point1=x=sample; y=0.5+0.2*sin(sample*6.28);\n"
        "shapecode_2_enabled=1\n"
        "shapecode_2_sides=6\n"
        "shapecode_2_num_inst=4\n"
        "shapecode_2_x=0.300\n"
        "shapecode_2_r2=0.700\n"
        "shape_2_per_frame1=t1=t1+0.1;\n"
        "comp_1=`shader_body\n"
        "comp_2=`{\n"
        "comp_3=`ret = tex2D(sampler_main, uv).xyz;\n"
        "comp_4=`ret *= 1.70; //gamma\n"
        "comp_5=`ret += GetBlur1(uv)*0.5;\n"
        "comp_6=`}\n");
    REQUIRE(r.ok);
    PresetState s = translate(r);
    s.name = "roundtrip_fixture";
    return s;
}

} // namespace

TEST_CASE("MilkdropSerializer: Voll-Roundtrip erhaelt alle Felder")
{
    const PresetState a = makeFullState();
    const QJsonObject doc = presetToJson(a);
    QStringList report;
    const PresetState b = presetFromJson(doc, &report);
    CHECK(report.isEmpty());

    CHECK(b.decay == doctest::Approx(a.decay));
    CHECK(b.gammaAdj == doctest::Approx(a.gammaAdj));
    CHECK(b.videoEchoAlpha == doctest::Approx(a.videoEchoAlpha));
    CHECK(b.videoEchoZoom == doctest::Approx(a.videoEchoZoom));
    CHECK(b.videoEchoOrientation == a.videoEchoOrientation);
    CHECK(b.invert == a.invert);
    CHECK(b.waveMode == a.waveMode);
    CHECK(b.waveR == doctest::Approx(a.waveR));
    CHECK(b.zoom == doctest::Approx(a.zoom));
    CHECK(b.rot == doctest::Approx(a.rot));
    CHECK(b.cx == doctest::Approx(a.cx));
    CHECK(b.warp == doctest::Approx(a.warp));
    CHECK(b.obSize == doctest::Approx(a.obSize));
    CHECK(b.obR == doctest::Approx(a.obR));
    CHECK(b.mvA == doctest::Approx(a.mvA));
    CHECK(b.mvX == doctest::Approx(a.mvX));
    CHECK(b.blur1Min == doctest::Approx(a.blur1Min));
    CHECK(b.blur1Max == doctest::Approx(a.blur1Max));
    CHECK(b.blur2Min == doctest::Approx(a.blur2Min));
    CHECK(b.blur3Max == doctest::Approx(a.blur3Max));
    CHECK(b.blur1EdgeDarken == doctest::Approx(a.blur1EdgeDarken));
    CHECK(b.perFrameInit == a.perFrameInit);
    CHECK(b.perFrame == a.perFrame);
    CHECK(b.perPixel == a.perPixel);
    CHECK(b.warpShaderText == a.warpShaderText);
    CHECK(b.compShaderText == a.compShaderText);
    CHECK(b.generation == a.generation);
    CHECK(b.psVersion == a.psVersion);
    CHECK(b.name == a.name);

    // Klassifikation wird aus dem Shader-Text neu abgeleitet (SSOT)
    CHECK(b.compInfo.shaderClass == a.compInfo.shaderClass);
    CHECK(b.compInfo.shaderClass == lumi::milk::ShaderClass::Md1Plus);
    CHECK(b.compInfo.gain == doctest::Approx(a.compInfo.gain));
    CHECK(b.compInfo.blurAdd[0] == doctest::Approx(0.5));

    REQUIRE(b.waves.size() == a.waves.size());
    REQUIRE(b.waves.size() == 1);
    CHECK(b.waves[0].index == a.waves[0].index);
    CHECK(b.waves[0].enabled == a.waves[0].enabled);
    CHECK(b.waves[0].samples == a.waves[0].samples);
    CHECK(b.waves[0].spectrum == a.waves[0].spectrum);
    CHECK(b.waves[0].scaling == doctest::Approx(a.waves[0].scaling));
    CHECK(b.waves[0].pointCode == a.waves[0].pointCode);

    REQUIRE(b.shapes.size() == a.shapes.size());
    REQUIRE(b.shapes.size() == 1);
    CHECK(b.shapes[0].index == a.shapes[0].index);
    CHECK(b.shapes[0].sides == a.shapes[0].sides);
    CHECK(b.shapes[0].instances == a.shapes[0].instances);
    CHECK(b.shapes[0].x == doctest::Approx(a.shapes[0].x));
    CHECK(b.shapes[0].r2 == doctest::Approx(a.shapes[0].r2));
    CHECK(b.shapes[0].frameCode == a.shapes[0].frameCode);
}

TEST_CASE("MilkdropSerializer: fehlende Felder -> CState-Defaults, kein Hard-Fail")
{
    QJsonObject header;
    header["type"] = "milkdrop";
    QJsonObject doc;
    doc["header"] = header;
    doc["preset"] = QJsonObject();  // leer

    QStringList report;
    const PresetState s = presetFromJson(doc, &report);
    CHECK(s.decay == doctest::Approx(0.98));
    CHECK(s.gammaAdj == doctest::Approx(2.0));
    CHECK(s.blur1Max == doctest::Approx(1.0));
    CHECK(s.waves.empty());
    CHECK(s.warpInfo.shaderClass == lumi::milk::ShaderClass::None);

    // ganz ohne preset-Objekt: Defaults + Report-Notiz
    QJsonObject broken;
    broken["header"] = header;
    QStringList report2;
    const PresetState d = presetFromJson(broken, &report2);
    CHECK(d.decay == doctest::Approx(0.98));
    CHECK(report2.size() == 1);
}

TEST_CASE("MilkdropSerializer: Datei-Roundtrip + Typ-Erkennung")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString path = tmp.filePath("fixture.lvfx");

    const PresetState a = makeFullState();
    REQUIRE(lumi::milkdrop::savePresetToFile(a, path));
    CHECK(lumi::milkdrop::isMilkdropFile(path));

    PresetState b;
    QStringList report;
    REQUIRE(lumi::milkdrop::loadPresetFromFile(path, b, &report));
    CHECK(b.name == a.name);
    CHECK(b.compInfo.shaderClass == lumi::milk::ShaderClass::Md1Plus);

    // Fremdformat wird sauber abgelehnt (MultiEffect-Chain hat keinen Milk-Typ)
    const QString chainPath = tmp.filePath("chain.lvfx");
    {
        QFile f(chainPath);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("{\"header\":{\"formatVersion\":1},\"root\":{\"type\":\"list\"}}");
    }
    CHECK_FALSE(lumi::milkdrop::isMilkdropFile(chainPath));
    PresetState c;
    QStringList report2;
    CHECK_FALSE(lumi::milkdrop::loadPresetFromFile(chainPath, c, &report2));
    CHECK(report2.size() == 1);
}

TEST_CASE("MilkdropSerializer: Korpus-Roundtrip (falls lokal vorhanden)")
{
    const std::filesystem::path dir = repoRoot() / "asset" / "calibration" / "milkdrop";
    if (!std::filesystem::exists(dir))
    {
        MESSAGE("Kalibrier-Korpus nicht vorhanden — Gate uebersprungen");
        return;
    }
    int files = 0;
    int mismatches = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
        const auto parsed = lumi::milk::parseFile(entry.path());
        if (!parsed.ok) continue;
        ++files;
        const PresetState a = translate(parsed);
        const PresetState b = presetFromJson(presetToJson(a), nullptr);
        const bool same = b.decay == a.decay && b.zoom == a.zoom &&
                          b.waveMode == a.waveMode && b.perFrame == a.perFrame &&
                          b.perPixel == a.perPixel &&
                          b.warpShaderText == a.warpShaderText &&
                          b.compShaderText == a.compShaderText &&
                          b.waves.size() == a.waves.size() &&
                          b.shapes.size() == a.shapes.size() &&
                          b.warpInfo.shaderClass == a.warpInfo.shaderClass &&
                          b.compInfo.shaderClass == a.compInfo.shaderClass;
        if (!same) ++mismatches;
    }
    MESSAGE("Kalibrier-Roundtrip: ", files, " Presets, ", mismatches, " Abweichungen");
    CHECK(files >= 26);       // m3 (10) + m4 (8) + m5 (8)
    CHECK(mismatches == 0);
}

TEST_CASE("MilkdropSerializer: Sprite-Roundtrip (MilkDrop2077-Sektionen)")
{
    PresetState s;
    lumi::milkdrop::SpriteState sp;
    sp.index = 3;
    sp.imageName = "sprites\\Jello1.png";
    sp.colorKey = 0x102030u;
    sp.layer = 2;
    sp.blendMode = 4;
    sp.alpha = 0.5;
    sp.burn = 1.0;
    sp.x = 0.25;
    sp.y = 0.75;
    sp.sx = -0.8;
    sp.sy = 0.8;
    sp.rot = 1.5;
    sp.speed = 2.0;
    sp.repeatX = 3.0;
    sp.repeatY = 4.0;
    sp.code = "x=0.5+0.1*sin(time);";
    s.sprites.push_back(sp);

    const PresetState back = presetFromJson(presetToJson(s), nullptr);
    REQUIRE(back.sprites.size() == 1);
    const auto& b = back.sprites[0];
    CHECK(b.index == 3);
    CHECK(b.imageName == "sprites\\Jello1.png");
    CHECK(b.colorKey == 0x102030u);
    CHECK(b.layer == 2);
    CHECK(b.blendMode == 4);
    CHECK(b.alpha == doctest::Approx(0.5));
    CHECK(b.burn == doctest::Approx(1.0));
    CHECK(b.x == doctest::Approx(0.25));
    CHECK(b.y == doctest::Approx(0.75));
    CHECK(b.sx == doctest::Approx(-0.8));
    CHECK(b.sy == doctest::Approx(0.8));
    CHECK(b.rot == doctest::Approx(1.5));
    CHECK(b.speed == doctest::Approx(2.0));
    CHECK(b.repeatX == doctest::Approx(3.0));
    CHECK(b.repeatY == doctest::Approx(4.0));
    CHECK(b.code == "x=0.5+0.1*sin(time);");
}

// Strang R (S65), Waechter R2: Regelwerk-Felder roundtrippen; fehlende Felder
// migrieren auf Legacy + Auto (alle Bestands-Dokumente laden unveraendert).
TEST_CASE("MilkdropSerializer: Regelwerk-Roundtrip (gesetzte Felder)")
{
    using lumi::milkdrop::PsOverride;
    using lumi::milkdrop::Regelwerk;

    PresetState a;
    a.regelwerk = Regelwerk::Benutzerdefiniert;
    a.divVertragD3d9 = false;
    a.unormTrunkierung = true;
    a.qGarbageEpsilon = false;
    a.uvSanitize = true;
    a.psWarp = PsOverride::PS3;
    a.psComp = PsOverride::MD1erzwingen;

    const PresetState b = presetFromJson(presetToJson(a), nullptr);
    CHECK(b.regelwerk == Regelwerk::Benutzerdefiniert);
    CHECK_FALSE(b.divVertragD3d9);
    CHECK(b.unormTrunkierung);
    CHECK_FALSE(b.qGarbageEpsilon);
    CHECK(b.uvSanitize);
    CHECK(b.psWarp == PsOverride::PS3);
    CHECK(b.psComp == PsOverride::MD1erzwingen);

    // Modern roundtrippt ebenso (Schalter-Rohfelder bleiben erhalten)
    PresetState m;
    m.regelwerk = Regelwerk::Modern;
    const PresetState m2 = presetFromJson(presetToJson(m), nullptr);
    CHECK(m2.regelwerk == Regelwerk::Modern);
    const auto e = lumi::milkdrop::effektiveSchalter(m2);
    CHECK_FALSE(e.divVertragD3d9);
    CHECK_FALSE(e.unormTrunkierung);
}

TEST_CASE("MilkdropSerializer: Regelwerk-Migration (fehlende/unbekannte Felder)")
{
    using lumi::milkdrop::PsOverride;
    using lumi::milkdrop::Regelwerk;

    QJsonObject header;
    header["type"] = "milkdrop";

    SUBCASE("Bestands-Dokument ohne Regelwerk-Felder -> Legacy + Auto")
    {
        QJsonObject preset;
        preset["decay"] = 0.95;  // ein Alt-Feld, sonst nichts
        QJsonObject doc;
        doc["header"] = header;
        doc["preset"] = preset;

        const PresetState s = presetFromJson(doc, nullptr);
        CHECK(s.regelwerk == Regelwerk::Legacy);
        CHECK(s.psWarp == PsOverride::Auto);
        CHECK(s.psComp == PsOverride::Auto);
        const auto e = lumi::milkdrop::effektiveSchalter(s);
        CHECK(e.divVertragD3d9);
        CHECK(e.unormTrunkierung);
        CHECK(e.qGarbageEpsilon);
        CHECK(e.uvSanitize);
    }
    SUBCASE("unbekannte Enum-Strings fallen auf Legacy/Auto zurueck")
    {
        QJsonObject preset;
        preset["regelwerk"] = "zukunftsmodus";
        preset["psWarp"] = "ps9";
        QJsonObject doc;
        doc["header"] = header;
        doc["preset"] = preset;

        const PresetState s = presetFromJson(doc, nullptr);
        CHECK(s.regelwerk == Regelwerk::Legacy);
        CHECK(s.psWarp == PsOverride::Auto);
    }
}

TEST_CASE("MilkdropTranslator: s1-Kalibrier-Presets tragen ihre Sprites")
{
    const std::filesystem::path dir =
        repoRoot() / "asset" / "calibration" / "milkdrop" / "s1";
    REQUIRE(std::filesystem::exists(dir));
    int presets = 0;
    int sprites = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
        const auto parsed = lumi::milk::parseFile(entry.path());
        REQUIRE(parsed.ok);
        const PresetState s = translate(parsed);
        ++presets;
        sprites += static_cast<int>(s.sprites.size());
        for (const auto& spr : s.sprites)
        {
            CHECK(!spr.imageName.empty());
            CHECK(spr.blendMode >= 0);
            CHECK(spr.blendMode <= 4);
        }
    }
    CHECK(presets == 3);
    CHECK(sprites == 4);  // 01: 1 · 02: 2 · 03: 1
}
