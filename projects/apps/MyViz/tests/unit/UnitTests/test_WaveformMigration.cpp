/**
 ****************************************************************************************
 * @file   test_WaveformMigration.cpp
 * @brief  Unit-Tests fuer die Waveform-Key-Migration (Phase 4 Schritt 5.3):
 *         Alias-Map alt->neu (inkl. waveform.color.*-Legacy), E3-Wert-Konverter
 *         (waveform.smoothing -> audio.smooth.timeMs), Roundtrip Alt-Preset ->
 *         Soll-Keys -> Save/Load im neuen Schema (Parameter_Key_Migration.md §3)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/WaveformVisualizer.hpp"
#include "visualizers/VisualizerPresetManager.hpp"
#include "visualizers/modules/processing/SmoothingModule.hpp"

#include <QTemporaryDir>

#include <cmath>
#include <string>
#include <vector>

using lumi::VisualizerPreset;
using lumi::VisualizerPresetManager;
using lumi::modules::ParamValue;
using lumi::modules::PipelineStage;

namespace
{

/// Alt-Schema-Preset (formatVersion 1) mit Vertretern aller Key-Familien aus §3
VisualizerPreset makeLegacyPreset()
{
    VisualizerPreset p;
    p.name = "LegacyWave";
    p.visualizerId = "waveform";
    p.formatVersion = 1;
    p.parameters["audio.gain"] = 1.5f;                  // Stufe 1, unveraendert
    p.parameters["waveform.channelMode"] = 2;           // -> map.channelMode (Both)
    p.parameters["waveform.sampleCount"] = 256;         // -> map.sampleCount (Resize!)
    p.parameters["waveform.smoothing"] = 0.9f;          // -> audio.smooth.timeMs (E3!)
    p.parameters["waveform.monoOffset"] = 0.25f;        // -> render.mono.offset
    p.parameters["waveform.leftAmplitude"] = 0.7f;      // -> render.left.amplitude
    p.parameters["waveform.lineStyle"] = 2;             // -> render.lineStyle (Dashed)
    p.parameters["waveform.rightFillOpacity"] = 0.6f;   // -> render.right.fillOpacity
    p.parameters["waveform.mirrorEnabled"] = true;      // -> post.mirror.enabled
    p.parameters["waveform.holdEnabled"] = true;        // -> post.hold.enabled
    p.parameters["waveform.fadeTime"] = 2.5f;           // -> post.hold.fadeTime
    p.parameters["waveform.maxHoldFrames"] = 30;        // -> post.hold.maxFrames
    p.parameters["waveform.monoColor.angle"] = 45.0f;   // -> color.mono.angle
    p.parameters["waveform.leftColor.angle"] = 90.0f;   // -> color.left.angle
    p.parameters["waveform.color.outlineWidth"] = 3.0f; // Legacy-Alias -> color.mono.outlineWidth (§7.4)
    return p;
}

} // namespace

TEST_CASE("Waveform 5.3: Alias-Map wird bei Instanziierung registriert")
{
    WaveformVisualizer viz;  // Konstruktor registriert die Alias-Map (einmalig)

    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.channelMode")
          == "map.channelMode");
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.sampleCount")
          == "map.sampleCount");
    // Kanal-strukturierte Render-Keys (E8)
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.monoOffset")
          == "render.mono.offset");
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.rightFillBrightness")
          == "render.right.fillBrightness");
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.displayWidth")
          == "render.displayWidth");  // kanal-unabhaengig -> flach
    // Post-Effekte
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.mirrorEnabled")
          == "post.mirror.enabled");
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.maxHoldFrames")
          == "post.hold.maxFrames");
    // Farb-Handles + Legacy-Alias waveform.color.* (§7.4)
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.monoColor.angle")
          == "color.mono.angle");
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.rightColor.gradientData")
          == "color.right.gradientData");
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "waveform.color.angle")
          == "color.mono.angle");
    // Identitaet: audio.* bleibt
    CHECK(VisualizerPresetManager::translateLegacyKey("waveform", "audio.gain")
          == "audio.gain");
}

TEST_CASE("Waveform 5.3: E3-Wert-Konverter smoothing -> audio.smooth.timeMs")
{
    WaveformVisualizer viz;

    auto [key, value] = VisualizerPresetManager::translateLegacyParam(
        "waveform", "waveform.smoothing", 0.9f);
    CHECK(key == "audio.smooth.timeMs");
    CHECK(std::get<float>(value) == doctest::Approx(158.2f).epsilon(0.01));

    // Randfaelle: s <= 0 -> 0 ms; s >= 1 -> geclampt (endlich)
    auto [k0, v0] = VisualizerPresetManager::translateLegacyParam(
        "waveform", "waveform.smoothing", 0.0f);
    CHECK(std::get<float>(v0) == 0.0f);
    auto [k1, v1] = VisualizerPresetManager::translateLegacyParam(
        "waveform", "waveform.smoothing", 1.0f);
    CHECK(std::isfinite(std::get<float>(v1)));
    CHECK(std::get<float>(v1) > 0.0f);
}

TEST_CASE("Waveform 5.3: SmoothingModule glaettet Arrays mit unabhaengigem per-Index-Zustand")
{
    using lumi::modules::SmoothingAlgorithm;
    using lumi::modules::SmoothingModule;

    SmoothingModule smoother;
    smoother.setAlgorithm(SmoothingAlgorithm::EMA);
    smoother.setTimeMs(100.0f);

    // Erster Frame primt den Zustand (Ausgabe = Eingabe)
    std::vector<float> frame1{0.0f, 1.0f};
    std::vector<float> out(2);
    smoother.processArrayPerIndex(frame1.data(), 2, 1.0f / 60.0f, out.data());
    CHECK(out[0] == doctest::Approx(0.0f));
    CHECK(out[1] == doctest::Approx(1.0f));

    // Zweiter Frame: jeder Index bewegt sich unabhaengig Richtung Zielwert
    std::vector<float> frame2{1.0f, 0.0f};
    smoother.processArrayPerIndex(frame2.data(), 2, 1.0f / 60.0f, out.data());
    const float alpha = 1.0f - std::exp(-(1.0f / 60.0f) / 0.1f);  // EMA-Formel des Moduls
    CHECK(out[0] == doctest::Approx(alpha).epsilon(0.001));
    CHECK(out[1] == doctest::Approx(1.0f - alpha).epsilon(0.001));

    // timeMs = 0 -> Passthrough
    smoother.setTimeMs(0.0f);
    smoother.processArrayPerIndex(frame2.data(), 2, 1.0f / 60.0f, out.data());
    CHECK(out[0] == doctest::Approx(1.0f));
    CHECK(out[1] == doctest::Approx(0.0f));
}

TEST_CASE("Waveform 5.3: paramDescs nur Soll-Keys, jede Stufe gesetzt, kein smoothing")
{
    WaveformVisualizer viz;

    bool sawMapSampleCount = false;
    bool sawPostFadeTime = false;
    bool sawLeftColor = false;
    for (const auto& desc : viz.paramDescs())
    {
        CAPTURE(desc.id);
        CHECK(desc.id.rfind("waveform.", 0) != 0);  // Alt-Schema komplett weg
        CHECK(desc.stage != PipelineStage::None);
        // E3: der Glaettungs-Skalar hat KEINEN Nachfolge-Key
        CHECK(desc.id.find("smoothing") == std::string::npos);

        sawMapSampleCount |= (desc.id == "map.sampleCount");
        sawPostFadeTime |= (desc.id == "post.hold.fadeTime");
        sawLeftColor |= (desc.id == "color.left.mode");
    }
    CHECK(sawMapSampleCount);
    CHECK(sawPostFadeTime);
    CHECK(sawLeftColor);
}

TEST_CASE("Waveform 5.3: Alt-Preset (formatVersion 1) landet auf Soll-Keys")
{
    WaveformVisualizer viz;
    VisualizerPresetManager mgr;

    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));

    ParamValue v;
    REQUIRE(viz.getParam("audio.gain", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.5f));
    REQUIRE(viz.getParam("map.channelMode", v));
    CHECK(std::get<int>(v) == 2);
    REQUIRE(viz.getParam("map.sampleCount", v));
    CHECK(std::get<int>(v) == 256);
    // E3: alter Glaettungs-Skalar wirkt jetzt als audio.smooth.timeMs
    REQUIRE(viz.getParam("audio.smooth.timeMs", v));
    CHECK(std::get<float>(v) == doctest::Approx(158.2f).epsilon(0.01));
    REQUIRE(viz.getParam("render.mono.offset", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.25f));
    REQUIRE(viz.getParam("render.left.amplitude", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.7f));
    REQUIRE(viz.getParam("render.lineStyle", v));
    CHECK(std::get<int>(v) == 2);
    REQUIRE(viz.getParam("render.right.fillOpacity", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.6f));
    REQUIRE(viz.getParam("post.mirror.enabled", v));
    CHECK(std::get<bool>(v) == true);
    REQUIRE(viz.getParam("post.hold.enabled", v));
    CHECK(std::get<bool>(v) == true);
    REQUIRE(viz.getParam("post.hold.fadeTime", v));
    CHECK(std::get<float>(v) == doctest::Approx(2.5f));
    REQUIRE(viz.getParam("post.hold.maxFrames", v));
    CHECK(std::get<int>(v) == 30);
    REQUIRE(viz.getParam("color.mono.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(45.0f));
    REQUIRE(viz.getParam("color.left.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(90.0f));
    // Legacy-Alias waveform.color.* -> Mono-Gradient (§7.4)
    REQUIRE(viz.getParam("color.mono.outlineWidth", v));
    CHECK(std::get<float>(v) == doctest::Approx(3.0f));
}

TEST_CASE("Waveform 5.3: Capture/Save/Load-Roundtrip nur im neuen Schema")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());

    WaveformVisualizer viz;
    VisualizerPresetManager mgr;
    mgr.setPresetsDirectory(tmp.path());

    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));
    VisualizerPreset captured = mgr.capturePreset(&viz, "Migriert");

    // Gespeichert wird ausschliesslich im neuen Schema (kein Dual-Write, §9)
    for (const auto& [key, value] : captured.parameters)
    {
        CAPTURE(key);
        CHECK(key.rfind("waveform.", 0) != 0);
        CHECK(key.find("smoothing") == std::string::npos);
    }

    REQUIRE(mgr.savePreset(captured));
    auto loaded = mgr.loadPreset("waveform", "Migriert");
    REQUIRE(loaded.has_value());

    CHECK(loaded->formatVersion == VisualizerPresetManager::CURRENT_FORMAT_VERSION);
    REQUIRE(mgr.applyPreset(&viz, *loaded));

    ParamValue v;
    REQUIRE(viz.getParam("map.sampleCount", v));
    CHECK(std::get<int>(v) == 256);
    REQUIRE(viz.getParam("post.hold.fadeTime", v));
    CHECK(std::get<float>(v) == doctest::Approx(2.5f));
    REQUIRE(viz.getParam("color.left.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(90.0f));
}
