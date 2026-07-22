/**
 ****************************************************************************************
 * @file   test_MilkdropPreset.cpp
 * @brief  Tests fuer den .milk->PresetState-Translator (Import-Phase Roadmap 6, M3):
 *         Original-Defaults (CState::Default), Key-Mapping, plus Korpus-Smoke
 *         (uebersetzen + per_frame/per_pixel-Transpile-Abdeckung, GL-frei)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <EelTranspiler.hpp>
#include <MilkParser.hpp>

#include "visualizers/MilkdropVisualizer.hpp"
#include "visualizers/milkdrop/MilkdropBlur.hpp"
#include "visualizers/milkdrop/MilkdropPresetState.hpp"

#include <QString>
#include <QStringList>

#include <cmath>
#include <filesystem>
#include <map>
#include <string>

using lumi::milk::parse;
using lumi::milk::parseFile;
using lumi::milkdrop::PresetState;
using lumi::milkdrop::translate;

namespace {

std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

} // namespace

TEST_CASE("MilkdropTranslator: leeres Preset traegt die Original-Defaults")
{
    const auto r = parse("[preset00]\nfRating=3.0\n");
    REQUIRE(r.ok);
    const PresetState s = translate(r);
    // CState::Default (state.cpp:536-680)
    CHECK(s.decay == doctest::Approx(0.98));
    CHECK(s.gammaAdj == doctest::Approx(2.0));
    CHECK(s.videoEchoZoom == doctest::Approx(2.0));
    CHECK(s.texWrap);
    CHECK(s.maximizeWaveColor);
    CHECK(s.waveAlpha == doctest::Approx(0.8));
    CHECK(s.waveSmoothing == doctest::Approx(0.75));
    CHECK(s.zoom == doctest::Approx(1.0));
    CHECK(s.warp == doctest::Approx(1.0));
    CHECK(s.cx == doctest::Approx(0.5));
    CHECK(s.obSize == doctest::Approx(0.01));
    CHECK(s.ibR == doctest::Approx(0.25));
    CHECK(s.mvX == doctest::Approx(12.0));
    CHECK(s.mvL == doctest::Approx(0.9));
    CHECK(s.perFrame.empty());
    CHECK(s.generation == 1);
}

TEST_CASE("MilkdropTranslator: Key-Mapping (Datei-Keys -> State-Felder)")
{
    const auto r = parse("MILKDROP_PRESET_VERSION=201\n"
                         "PSVERSION=2\n"
                         "[preset00]\n"
                         "fDecay=0.950\n"
                         "fGammaAdj=1.500\n"
                         "nVideoEchoOrientation=3\n"
                         "nWaveMode=6\n"
                         "bAdditiveWaves=1\n"
                         "bTexWrap=0\n"
                         "fWaveParam=-0.4\n"
                         "zoom=1.046\n"
                         "rot=0.020\n"
                         "warp=0.198\n"
                         "sx=0.990\n"
                         "wave_r=0.650\n"
                         "ob_size=0.005\n"
                         "ib_a=0.900\n"
                         "nMotionVectorsX=64.0\n"
                         "mv_l=0.850\n"
                         "per_frame_1=q1=bass;\n"
                         "per_pixel_1=rot=rot+0.01;\n");
    REQUIRE(r.ok);
    const PresetState s = translate(r);
    CHECK(s.decay == doctest::Approx(0.95));
    CHECK(s.gammaAdj == doctest::Approx(1.5));
    CHECK(s.videoEchoOrientation == 3);
    CHECK(s.waveMode == 6);
    CHECK(s.additiveWaves);
    CHECK_FALSE(s.texWrap);
    CHECK(s.waveMystery == doctest::Approx(-0.4));
    CHECK(s.zoom == doctest::Approx(1.046));
    CHECK(s.rot == doctest::Approx(0.02));
    CHECK(s.warp == doctest::Approx(0.198));
    CHECK(s.sx == doctest::Approx(0.99));
    CHECK(s.waveR == doctest::Approx(0.65));
    CHECK(s.obSize == doctest::Approx(0.005));
    CHECK(s.ibA == doctest::Approx(0.9));
    CHECK(s.mvX == doctest::Approx(64.0));
    CHECK(s.mvL == doctest::Approx(0.85));
    CHECK(s.perFrame == "q1=bass;");
    CHECK(s.perPixel == "rot=rot+0.01;");
    CHECK(s.generation == 2);
    CHECK(s.psVersion == 2);
}

TEST_CASE("MilkdropTranslator: Waves/Shapes mit Original-Defaults (M4)")
{
    const auto r = parse("[preset00]\n"
                         "wavecode_1_enabled=1\n"
                         "wavecode_1_samples=256\n"
                         "wavecode_1_bSpectrum=1\n"
                         "wavecode_1_scaling=2.5\n"
                         "wave_1_per_point1=x=sample;\n"
                         "shapecode_2_enabled=1\n"
                         "shapecode_2_sides=32\n"
                         "shapecode_2_num_inst=5\n"
                         "shapecode_2_textured=1\n"
                         "shape_2_init1=t1=3;\n");
    REQUIRE(r.ok);
    const PresetState s = translate(r);

    REQUIRE(s.waves.size() == 1);
    const auto& w = s.waves[0];
    CHECK(w.index == 1);
    CHECK(w.enabled);
    CHECK(w.samples == 256);
    CHECK(w.spectrum);
    CHECK(w.scaling == doctest::Approx(2.5));
    CHECK(w.smoothing == doctest::Approx(0.5));  // Default (state.cpp:599)
    CHECK(w.pointCode == "x=sample;");

    REQUIRE(s.shapes.size() == 1);
    const auto& sh = s.shapes[0];
    CHECK(sh.index == 2);
    CHECK(sh.sides == 32);
    CHECK(sh.instances == 5);
    CHECK(sh.textured);
    CHECK(sh.rad == doctest::Approx(0.1));       // Default (state.cpp:621)
    CHECK(sh.borderA == doctest::Approx(0.1));   // Default (state.cpp:634)
    CHECK(sh.initCode == "t1=3;");
}

// =============================================================================
// Korpus-Smoke: uebersetzen + Transpile-Abdeckung (umgebungsabhaengig)
// =============================================================================

TEST_CASE("MilkdropTranslator: Korpus uebersetzt + Milk-Code transpiliert")
{
    struct Corpus
    {
        const char* label;
        std::filesystem::path dir;
    };
    const Corpus corpora[3] = {
        // Kalibrier-Presets sind committet — die MUESSEN immer sauber laufen
        {"calibration", repoRoot() / "asset" / "calibration" / "milkdrop"},
        {"Milkdrop3", repoRoot() / "asset" / "Milkdrop3" / "presets"},
        {"winamp", repoRoot().parent_path() / "ref" / "winamp_orig" / "Src" / "resources" /
                       "data" / "Milkdrop2" / "presets"},
    };

    for (const Corpus& corpus : corpora)
    {
        if (!std::filesystem::exists(corpus.dir))
        {
            MESSAGE("Korpus ", corpus.label, " nicht vorhanden — uebersprungen");
            continue;
        }
        int files = 0;
        int withFrameCode = 0;
        int frameOk = 0;
        int withPixelCode = 0;
        int pixelOk = 0;
        std::string firstError;
        std::map<std::string, int> errorKinds;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(corpus.dir))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
            CAPTURE(entry.path().filename().string());
            const auto parsed = parseFile(entry.path());
            REQUIRE(parsed.ok);
            const PresetState s = translate(parsed);
            ++files;
            // Plausibilitaet: endlich + nicht absurd (Korpus enthaelt legitime
            // Extremwerte wie fDecay=0 und zoom=100!)
            CHECK(std::isfinite(s.decay));
            CHECK(s.decay >= 0.0);
            CHECK(std::isfinite(s.zoom));
            CHECK(std::fabs(s.zoom) <= 1000.0);

            if (!s.perFrame.empty())
            {
                ++withFrameCode;
                const auto t = lumi::eel::transpile(s.perFrame, lumi::eel::Dialect::Milkdrop);
                if (t.ok)
                {
                    ++frameOk;
                }
                else
                {
                    if (firstError.empty())
                    {
                        firstError = entry.path().filename().string() + ": " + t.error;
                    }
                    // Fehlerart ohne Positionsangabe buendeln ("Zeile N: " abschneiden)
                    std::string kind = t.error;
                    if (const auto pos = kind.find(": "); pos != std::string::npos)
                    {
                        kind = kind.substr(pos + 2);
                    }
                    ++errorKinds[kind];
                }
            }
            if (!s.perPixel.empty())
            {
                ++withPixelCode;
                const auto t = lumi::eel::transpile(s.perPixel, lumi::eel::Dialect::Milkdrop);
                if (t.ok) ++pixelOk;
            }
        }
        std::string kinds;
        for (const auto& [kind, count] : errorKinds)
        {
            kinds += " | " + kind + " x" + std::to_string(count);
        }
        MESSAGE(std::string(corpus.label), ": ", files, " Presets uebersetzt; per_frame ",
                frameOk, "/", withFrameCode, " transpilieren ok, per_pixel ", pixelOk, "/",
                withPixelCode,
                (firstError.empty() ? std::string() : "; 1. Fehler: " + firstError), kinds);
        // Regressions-Wache: die Milk-Dialekt-Abdeckung darf nicht einbrechen;
        // der committete Kalibrier-Satz muss zu 100 % laufen
        if (std::string(corpus.label) == "calibration")
        {
            CHECK(files >= 18);
            CHECK(frameOk == withFrameCode);
            CHECK(pixelOk == withPixelCode);
        }
        else
        {
            if (withFrameCode > 0) CHECK(frameOk >= withFrameCode * 90 / 100);
            if (withPixelCode > 0) CHECK(pixelOk >= withPixelCode * 90 / 100);
        }
    }
}

// =============================================================================
// M5: Blur-Regler-Keys + Blur-Pyramiden-Mathematik (MilkdropBlur.hpp)
// =============================================================================

TEST_CASE("MilkdropTranslator: Blur-Keys b1n/b1x/../b1ed mit Original-Defaults")
{
    const auto defaults = parse("[preset00]\nfRating=3.0\n");
    REQUIRE(defaults.ok);
    const PresetState d = translate(defaults);
    CHECK(d.blur1Min == doctest::Approx(0.0));
    CHECK(d.blur1Max == doctest::Approx(1.0));
    CHECK(d.blur3Max == doctest::Approx(1.0));
    CHECK(d.blur1EdgeDarken == doctest::Approx(0.25));

    const auto r = parse("[preset00]\n"
                         "b1n=0.100\nb2n=0.200\nb3n=0.300\n"
                         "b1x=0.900\nb2x=0.800\nb3x=0.700\n"
                         "b1ed=0.500\n");
    REQUIRE(r.ok);
    const PresetState s = translate(r);
    CHECK(s.blur1Min == doctest::Approx(0.1));
    CHECK(s.blur2Min == doctest::Approx(0.2));
    CHECK(s.blur3Min == doctest::Approx(0.3));
    CHECK(s.blur1Max == doctest::Approx(0.9));
    CHECK(s.blur2Max == doctest::Approx(0.8));
    CHECK(s.blur3Max == doctest::Approx(0.7));
    CHECK(s.blur1EdgeDarken == doctest::Approx(0.5));
}

TEST_CASE("MilkdropTranslator: Shader-Klassifikation haengt am PresetState")
{
    const auto r = parse("[preset00]\n"
                         "warp_1=`shader_body\n"
                         "warp_2=`{\n"
                         "warp_3=`ret = tex2D( sampler_main, uv ).xyz;\n"
                         "warp_4=`ret *= 0.97; //or try: ret -= 0.004;\n"
                         "warp_5=`}\n"
                         "comp_1=`shader_body\n"
                         "comp_2=`{\n"
                         "comp_3=`ret = tex2D(sampler_main, uv).xyz;\n"
                         "comp_4=`ret *= 1.50; //gamma\n"
                         "comp_5=`ret += GetBlur1(uv);\n"
                         "comp_6=`}\n");
    REQUIRE(r.ok);
    const PresetState s = translate(r);
    CHECK(s.warpInfo.shaderClass == lumi::milk::ShaderClass::Md1Default);
    CHECK(s.warpInfo.decayMul == doctest::Approx(0.97));
    CHECK(s.compInfo.shaderClass == lumi::milk::ShaderClass::Md1Plus);
    CHECK(s.compInfo.gain == doctest::Approx(1.5));
    CHECK(s.compInfo.blurAdd[0] == doctest::Approx(1.0));
    CHECK(s.compInfo.highestBlurLevel() == 1);
}

TEST_CASE("MilkdropBlur: Kernel-Konstanten aus den festen Gewichten w[8]")
{
    const auto kh = lumi::milkdrop::blurKernelH();
    CHECK(kh.w1 == doctest::Approx(7.8f));
    CHECK(kh.w2 == doctest::Approx(6.4f));
    CHECK(kh.w3 == doctest::Approx(3.1f));
    CHECK(kh.w4 == doctest::Approx(1.0f));
    CHECK(kh.d1 == doctest::Approx(2.0f * 3.8f / 7.8f));
    CHECK(kh.d4 == doctest::Approx(6.0f + 2.0f * 0.3f / 1.0f));
    CHECK(kh.wDiv == doctest::Approx(0.5f / (7.8f + 6.4f + 3.1f + 1.0f)));

    const auto kv = lumi::milkdrop::blurKernelV();
    CHECK(kv.w1 == doctest::Approx(14.2f));
    CHECK(kv.w2 == doctest::Approx(4.1f));
    CHECK(kv.d1 == doctest::Approx(2.0f * 6.4f / 14.2f));
    CHECK(kv.d2 == doctest::Approx(2.0f + 2.0f * 1.0f / 4.1f));
    CHECK(kv.wDiv == doctest::Approx(1.0f / ((14.2f + 4.1f) * 2.0f)));
}

TEST_CASE("MilkdropBlur: sichere Ranges — Monotonie und Kollaps enger Luecken")
{
    using lumi::milkdrop::computeSafeBlurRanges;

    // Default 0..1 bleibt unangetastet, Folge-Level klemmen in den Vorgaenger
    const auto r = computeSafeBlurRanges({0.0f, -0.5f, 0.2f}, {1.0f, 2.0f, 0.9f});
    CHECK(r.min[0] == doctest::Approx(0.0f));
    CHECK(r.max[0] == doctest::Approx(1.0f));
    CHECK(r.min[1] == doctest::Approx(0.0f));   // -0.5 -> max(min0, ...)
    CHECK(r.max[1] == doctest::Approx(1.0f));   // 2.0 -> min(max0, ...)
    CHECK(r.min[2] == doctest::Approx(0.2f));
    CHECK(r.max[2] == doctest::Approx(0.9f));

    // enge Luecke kollabiert auf einen Punkt (Referenz-Verhalten, PORT-Notiz)
    const auto c = computeSafeBlurRanges({0.5f, 0.0f, 0.0f}, {0.55f, 1.0f, 1.0f});
    CHECK(c.min[0] == doctest::Approx(0.475f));
    CHECK(c.max[0] == doctest::Approx(0.475f));

    // der Guard macht daraus einen endlichen Extremkontrast statt inf/NaN
    const auto s = lumi::milkdrop::computeBlurPassScales(c);
    CHECK(s.scale[0] == doctest::Approx(1024.0f));
    CHECK(std::isfinite(s.bias[0]));
}

TEST_CASE("MilkdropBlur: Texturgroessen-Kette (Referenz 1024er-Beispiel)")
{
    const auto sizes = lumi::milkdrop::blurTextureSizes(1024, 768);
    // main 1024 -> 512, 256 (blur1), 128, 128 (blur2), 64, 64 (blur3)
    CHECK(sizes[0][0] == 512);
    CHECK(sizes[0][1] == 384);
    CHECK(sizes[1][0] == 256);
    CHECK(sizes[1][1] == 192);
    CHECK(sizes[2][0] == 128);
    CHECK(sizes[3][0] == 128);
    CHECK(sizes[4][0] == 64);
    CHECK(sizes[5][0] == 64);
    CHECK(sizes[5][1] == 48);

    // Minimalgroesse 16 + Rundung (Breite /16, Hoehe /4)
    const auto tiny = lumi::milkdrop::blurTextureSizes(40, 40);
    CHECK(tiny[5][0] >= 16);
    CHECK(tiny[5][0] % 16 == 0);
    CHECK(tiny[5][1] % 4 == 0);
}

// Session-41-Regression: beim C2-Umbau gingen die tryTranspile-AUFRUFE in
// prepareCustomShaders verloren — die Custom-GLSL-Quellen blieben leer und
// jedes Preset lief still im MD1-Fallback. Dieses Gate prueft den LADEPFAD
// (loadMilkFile → applyState → prepareCustomShaders), nicht den Transpiler.
TEST_CASE("MilkdropVisualizer: c1-Kalibrier-Presets fuellen die Custom-GLSL-Quellen")
{
    const std::filesystem::path dir =
        repoRoot() / "asset" / "calibration" / "milkdrop" / "c1";
    REQUIRE(std::filesystem::exists(dir));

    int customShaders = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
        MilkdropVisualizer viz;
        QStringList report;
        REQUIRE_MESSAGE(
            viz.loadMilkFile(QString::fromStdWString(entry.path().wstring()), &report),
            entry.path().filename().string());

        const std::string name = entry.path().filename().string();
        const auto& s = viz.presetState();
        if (s.warpInfo.shaderClass == lumi::milk::ShaderClass::Custom)
        {
            ++customShaders;
            CHECK_MESSAGE(!viz.warpCustomSource().empty(),
                          name, ": Warp klassifiziert als Custom, aber GLSL-Quelle leer");
        }
        if (s.compInfo.shaderClass == lumi::milk::ShaderClass::Custom)
        {
            ++customShaders;
            CHECK_MESSAGE(!viz.compCustomSource().empty(),
                          name, ": Comp klassifiziert als Custom, aber GLSL-Quelle leer");
        }
    }
    // c1-Satz: 8 Custom-Shader (Gate identisch zum GL-Smoke-Test)
    CHECK(customShaders == 8);
}
