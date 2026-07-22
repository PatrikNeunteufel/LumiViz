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

#include "visualizers/milkdrop/MilkdropPresetState.hpp"

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
