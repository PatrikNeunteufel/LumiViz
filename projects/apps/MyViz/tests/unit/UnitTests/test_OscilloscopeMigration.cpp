/**
 ****************************************************************************************
 * @file   test_OscilloscopeMigration.cpp
 * @brief  Unit-Tests fuer die Oscilloscope-Key-Migration (Phase 4 Schritt 5.4):
 *         Alias-Map alt->neu (Trigger -> map.trigger.*, E4/E5-Entscheide,
 *         6 Farb-Handles), Roundtrip Alt-Preset -> Soll-Keys -> Save/Load
 *         (Parameter_Key_Migration.md §4)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/OscilloscopeVisualizer.hpp"
#include "visualizers/VisualizerPresetManager.hpp"

#include <QTemporaryDir>

#include <string>

using lumi::VisualizerPreset;
using lumi::VisualizerPresetManager;
using lumi::modules::ParamValue;
using lumi::modules::PipelineStage;

namespace
{

/// Alt-Schema-Preset (formatVersion 1) mit Vertretern aller Key-Familien aus §4
VisualizerPreset makeLegacyPreset()
{
    VisualizerPreset p;
    p.name = "LegacyScope";
    p.visualizerId = "oscilloscope";
    p.formatVersion = 1;
    p.parameters["audio.gain"] = 1.5f;                 // Stufe 1, unveraendert
    p.parameters["scope.timePerDiv"] = 20.0f;          // -> map.timePerDiv
    p.parameters["scope.sampleCount"] = 1024;          // -> map.sampleCount (Resize!)
    p.parameters["scope.triggerEnabled"] = true;       // -> map.trigger.enabled
    p.parameters["scope.triggerLevel"] = 0.4f;         // -> map.trigger.level
    p.parameters["scope.triggerEdge"] = 1;             // -> map.trigger.edge
    p.parameters["scope.triggerIndicator"] = 1;        // -> render.triggerIndicator (E4)
    p.parameters["scope.triggerFadeTime"] = 3.0f;      // -> post.trigger.fadeTime
    p.parameters["scope.ch2.visible"] = true;          // -> render.ch2.visible
    p.parameters["scope.ch2.source"] = 3;              // -> map.ch2.source (Mid)
    p.parameters["scope.ch2.voltsPerDiv"] = 1.5f;      // -> render.ch2.voltsPerDiv (E5)
    p.parameters["scope.m1.visible"] = true;           // -> render.m1.visible
    p.parameters["scope.m1.operation"] = 1;            // -> map.m1.operation (A - B)
    p.parameters["scope.gridBrightness"] = 0.8f;       // -> render.gridBrightness
    p.parameters["scope.ch1Color.angle"] = 45.0f;      // -> color.ch1.angle
    p.parameters["scope.m2Color.angle"] = 90.0f;       // -> color.m2.angle
    return p;
}

} // namespace

TEST_CASE("Oscilloscope 5.4: Alias-Map wird bei Instanziierung registriert")
{
    OscilloscopeVisualizer viz;  // Konstruktor registriert die Alias-Map (einmalig)

    // Timebase + Trigger -> map (Konzept-Beispiele)
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.timePerDiv")
          == "map.timePerDiv");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.triggerLevel")
          == "map.trigger.level");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.triggerChannel")
          == "map.trigger.channel");
    // E4: Indicator ist Anzeige-Overlay -> Render; Fade -> Post
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.triggerIndicator")
          == "render.triggerIndicator");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.triggerFadeTime")
          == "post.trigger.fadeTime");
    // E5: voltsPerDiv/offset -> render.chN/mN; Datenwahl -> map
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.ch3.voltsPerDiv")
          == "render.ch3.voltsPerDiv");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.ch3.source")
          == "map.ch3.source");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.m2.operation")
          == "map.m2.operation");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.m2.offset")
          == "render.m2.offset");
    // 6 Farb-Handles
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.ch4Color.gradientData")
          == "color.ch4.gradientData");
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "scope.m1Color.preset")
          == "color.m1.preset");
    // Identitaet: audio.* bleibt; Maps strikt pro Visualizer (§7.5)
    CHECK(VisualizerPresetManager::translateLegacyKey("oscilloscope", "audio.gain")
          == "audio.gain");
}

TEST_CASE("Oscilloscope 5.4: paramDescs nur Soll-Keys, jede Stufe gesetzt")
{
    OscilloscopeVisualizer viz;

    bool sawTriggerLevel = false;
    bool sawCh1Visible = false;
    bool sawM2Color = false;
    bool sawPostFade = false;
    for (const auto& desc : viz.paramDescs())
    {
        CAPTURE(desc.id);
        CHECK(desc.id.rfind("scope.", 0) != 0);  // Alt-Schema komplett weg
        CHECK(desc.stage != PipelineStage::None);

        sawTriggerLevel |= (desc.id == "map.trigger.level");
        sawCh1Visible |= (desc.id == "render.ch1.visible");
        sawM2Color |= (desc.id == "color.m2.mode");
        sawPostFade |= (desc.id == "post.trigger.fadeTime");
    }
    CHECK(sawTriggerLevel);
    CHECK(sawCh1Visible);
    CHECK(sawM2Color);
    CHECK(sawPostFade);
}

TEST_CASE("Oscilloscope 5.4: Alt-Preset (formatVersion 1) landet auf Soll-Keys")
{
    OscilloscopeVisualizer viz;
    VisualizerPresetManager mgr;

    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));

    ParamValue v;
    REQUIRE(viz.getParam("audio.gain", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.5f));
    REQUIRE(viz.getParam("map.timePerDiv", v));
    CHECK(std::get<float>(v) == doctest::Approx(20.0f));
    REQUIRE(viz.getParam("map.sampleCount", v));
    CHECK(std::get<int>(v) == 1024);
    REQUIRE(viz.getParam("map.trigger.enabled", v));
    CHECK(std::get<bool>(v) == true);
    REQUIRE(viz.getParam("map.trigger.level", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.4f));
    REQUIRE(viz.getParam("map.trigger.edge", v));
    CHECK(std::get<int>(v) == 1);
    REQUIRE(viz.getParam("render.triggerIndicator", v));
    CHECK(std::get<int>(v) == 1);
    REQUIRE(viz.getParam("post.trigger.fadeTime", v));
    CHECK(std::get<float>(v) == doctest::Approx(3.0f));
    REQUIRE(viz.getParam("render.ch2.visible", v));
    CHECK(std::get<bool>(v) == true);
    REQUIRE(viz.getParam("map.ch2.source", v));
    CHECK(std::get<int>(v) == 3);
    REQUIRE(viz.getParam("render.ch2.voltsPerDiv", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.5f));
    REQUIRE(viz.getParam("map.m1.operation", v));
    CHECK(std::get<int>(v) == 1);
    REQUIRE(viz.getParam("render.gridBrightness", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.8f));
    REQUIRE(viz.getParam("color.ch1.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(45.0f));
    REQUIRE(viz.getParam("color.m2.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(90.0f));
}

TEST_CASE("Oscilloscope 5.4: Capture/Save/Load-Roundtrip nur im neuen Schema")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());

    OscilloscopeVisualizer viz;
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
    auto loaded = mgr.loadPreset("oscilloscope", "Migriert");
    REQUIRE(loaded.has_value());

    CHECK(loaded->formatVersion == VisualizerPresetManager::CURRENT_FORMAT_VERSION);
    REQUIRE(mgr.applyPreset(&viz, *loaded));

    ParamValue v;
    REQUIRE(viz.getParam("map.trigger.level", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.4f));
    REQUIRE(viz.getParam("render.ch2.voltsPerDiv", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.5f));
    REQUIRE(viz.getParam("color.m2.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(90.0f));
}
