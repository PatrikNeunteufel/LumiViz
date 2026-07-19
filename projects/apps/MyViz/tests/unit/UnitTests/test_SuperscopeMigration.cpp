/**
 ****************************************************************************************
 * @file   test_SuperscopeMigration.cpp
 * @brief  Unit-Tests fuer die Superscope-Key-Migration (Phase 4 Schritt 5.5):
 *         Alias-Map alt->neu (E6 render.preset, Doppel-Audio-Aufloesung ->
 *         map.audioSource/audioChannel, Glow/Hold -> post.*), getrennte Maps
 *         trotz geteiltem "scope."-Praefix (§7.5), Roundtrip
 *         (Parameter_Key_Migration.md §5)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/OscilloscopeVisualizer.hpp"
#include "visualizers/SuperscopeVisualizer.hpp"
#include "visualizers/VisualizerPresetManager.hpp"

#include <QTemporaryDir>

#include <string>

using lumi::VisualizerPreset;
using lumi::VisualizerPresetManager;
using lumi::modules::ParamValue;
using lumi::modules::PipelineStage;

namespace
{

/// Alt-Schema-Preset (formatVersion 1) mit Vertretern aller Key-Familien aus §5
VisualizerPreset makeLegacyPreset()
{
    VisualizerPreset p;
    p.name = "LegacySuper";
    p.visualizerId = "superscope";
    p.formatVersion = 1;
    p.parameters["audio.gain"] = 1.5f;              // Stufe 1, unveraendert
    p.parameters["scope.pointCount"] = 256;         // -> map.pointCount
    p.parameters["scope.audioSource"] = 1;          // -> map.audioSource (Spectrum)
    p.parameters["scope.audioChannel"] = 2;         // -> map.audioChannel (Mono)
    p.parameters["scope.renderMode"] = 1;           // -> render.mode (Lines)
    p.parameters["scope.lineWidth"] = 3.0f;         // -> render.lineWidth
    p.parameters["scope.stretchX"] = 1.2f;          // -> render.stretchX
    p.parameters["scope.glowEnabled"] = true;       // -> post.glow.enabled
    p.parameters["scope.glowIntensity"] = 0.7f;     // -> post.glow.intensity
    p.parameters["scope.holdEnabled"] = true;       // -> post.hold.enabled
    p.parameters["scope.fadeTime"] = 2.0f;          // -> post.hold.fadeTime
    p.parameters["scope.maxHoldFrames"] = 20;       // -> post.hold.maxFrames
    p.parameters["scope.color.angle"] = 45.0f;      // -> color.main.angle
    return p;
}

} // namespace

TEST_CASE("Superscope 5.5: Alias-Map wird bei Instanziierung registriert")
{
    SuperscopeVisualizer viz;  // Konstruktor registriert die Alias-Map (einmalig)

    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.preset")
          == "render.preset");  // E6: Preset-Dropdown bleibt Stufe 4
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.pointCount")
          == "map.pointCount");
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.renderMode")
          == "render.mode");  // renderMode -> mode (Redundanz zum Praefix)
    // Doppel-"Audio"-Gruppe aufgeloest: Datenquelle der v-Variable -> map
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.audioSource")
          == "map.audioSource");
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.audioChannel")
          == "map.audioChannel");
    // Glow/Hold -> Post
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.glowSize")
          == "post.glow.size");
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.maxHoldFrames")
          == "post.hold.maxFrames");
    // Gradient-Handle
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.color.gradientData")
          == "color.main.gradientData");
    // Identitaet: audio.* bleibt
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "audio.gain")
          == "audio.gain");
}

TEST_CASE("Superscope 5.5: Alias-Maps strikt pro Visualizer trotz 'scope.'-Praefix (§7.5)")
{
    SuperscopeVisualizer superscope;
    OscilloscopeVisualizer oscilloscope;  // registriert seine eigene Map

    // Gleicher Alt-Praefix, verschiedene Ziele je Visualizer
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.pointCount")
          == "map.pointCount");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.pointCount")
          == "scope.pointCount");  // dort unbekannt -> passthrough
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.timePerDiv")
          == "map.timePerDiv");
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.timePerDiv")
          == "scope.timePerDiv");  // dort unbekannt -> passthrough
    // scope.lineWidth existiert in BEIDEN Maps mit verschiedener Bedeutung
    CHECK(VisualizerPresetManager::translateLegacyKey("superscope", "scope.lineWidth")
          == "render.lineWidth");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.ch1.lineWidth")
          == "render.ch1.lineWidth");
}

TEST_CASE("Superscope 5.5: paramDescs nur Soll-Keys, jede Stufe gesetzt")
{
    SuperscopeVisualizer viz;

    bool sawMapAudioSource = false;
    bool sawRenderPreset = false;
    bool sawPostGlow = false;
    bool sawColorMain = false;
    for (const auto& desc : viz.paramDescs())
    {
        CAPTURE(desc.id);
        CHECK(desc.id.rfind("scope.", 0) != 0);  // Alt-Schema komplett weg
        CHECK(desc.stage != PipelineStage::None);

        sawMapAudioSource |= (desc.id == "map.audioSource");
        sawRenderPreset |= (desc.id == "render.preset");
        sawPostGlow |= (desc.id == "post.glow.enabled");
        sawColorMain |= (desc.id == "color.main.mode");
    }
    CHECK(sawMapAudioSource);
    CHECK(sawRenderPreset);
    CHECK(sawPostGlow);
    CHECK(sawColorMain);
}

TEST_CASE("Superscope 5.5: Alt-Preset (formatVersion 1) landet auf Soll-Keys")
{
    SuperscopeVisualizer viz;
    VisualizerPresetManager mgr;

    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));

    ParamValue v;
    REQUIRE(viz.getParam("audio.gain", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.5f));
    REQUIRE(viz.getParam("map.pointCount", v));
    CHECK(std::get<int>(v) == 256);
    REQUIRE(viz.getParam("map.audioSource", v));
    CHECK(std::get<int>(v) == 1);
    REQUIRE(viz.getParam("map.audioChannel", v));
    CHECK(std::get<int>(v) == 2);
    REQUIRE(viz.getParam("render.mode", v));
    CHECK(std::get<int>(v) == 1);
    REQUIRE(viz.getParam("render.lineWidth", v));
    CHECK(std::get<float>(v) == doctest::Approx(3.0f));
    REQUIRE(viz.getParam("render.stretchX", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.2f));
    REQUIRE(viz.getParam("post.glow.enabled", v));
    CHECK(std::get<bool>(v) == true);
    REQUIRE(viz.getParam("post.glow.intensity", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.7f));
    REQUIRE(viz.getParam("post.hold.enabled", v));
    CHECK(std::get<bool>(v) == true);
    REQUIRE(viz.getParam("post.hold.fadeTime", v));
    CHECK(std::get<float>(v) == doctest::Approx(2.0f));
    REQUIRE(viz.getParam("post.hold.maxFrames", v));
    CHECK(std::get<int>(v) == 20);
    REQUIRE(viz.getParam("color.main.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(45.0f));
}

TEST_CASE("Superscope 5.5: Capture/Save/Load-Roundtrip nur im neuen Schema")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());

    SuperscopeVisualizer viz;
    VisualizerPresetManager mgr;
    mgr.setPresetsDirectory(tmp.path());

    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));
    VisualizerPreset captured = mgr.capturePreset(&viz, "Migriert");

    // Gespeichert wird ausschliesslich im neuen Schema (kein Dual-Write, §9)
    for (const auto& [key, value] : captured.parameters)
    {
        CAPTURE(key);
        CHECK(key.rfind("scope.", 0) != 0);
    }

    REQUIRE(mgr.savePreset(captured));
    auto loaded = mgr.loadPreset("superscope", "Migriert");
    REQUIRE(loaded.has_value());

    CHECK(loaded->formatVersion == VisualizerPresetManager::CURRENT_FORMAT_VERSION);
    REQUIRE(mgr.applyPreset(&viz, *loaded));

    ParamValue v;
    REQUIRE(viz.getParam("map.pointCount", v));
    CHECK(std::get<int>(v) == 256);
    REQUIRE(viz.getParam("post.glow.intensity", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.7f));
    REQUIRE(viz.getParam("color.main.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(45.0f));
}
