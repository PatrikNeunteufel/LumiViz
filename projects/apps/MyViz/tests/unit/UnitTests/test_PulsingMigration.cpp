/**
 ****************************************************************************************
 * @file   test_PulsingMigration.cpp
 * @brief  Unit-Tests fuer die Pulsing-Key-Migration (Phase 4 Schritt 5.2):
 *         Alias-Map alt->neu, PulseShapeModule-paramDescs, Roundtrip Alt-Preset ->
 *         Soll-Keys -> Save/Load im neuen Schema (Parameter_Key_Migration.md §2)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/PulsingVisualizer.hpp"
#include "visualizers/VisualizerPresetManager.hpp"
#include "visualizers/modules/PulseShapeModule.hpp"

#include <QTemporaryDir>

#include <string>

using lumi::VisualizerPreset;
using lumi::VisualizerPresetManager;
using lumi::modules::ParamValue;
using lumi::modules::PipelineStage;
using lumi::modules::PulseShapeModule;

namespace
{

/// Alt-Schema-Preset (formatVersion 1) mit Vertretern aller Key-Familien aus §2
VisualizerPreset makeLegacyPreset()
{
    VisualizerPreset p;
    p.name = "LegacyPulse";
    p.visualizerId = "pulsing";
    p.formatVersion = 1;
    p.parameters["audio.gain"] = 1.5f;                    // Stufe 1, unveraendert
    p.parameters["audio.bands"] = 32;                     // bleibt bei Pulsing (E2)
    p.parameters["shape.type"] = 1;                       // -> render.type (Ring)
    p.parameters["shape.sides"] = 8;                      // -> render.sides
    p.parameters["shape.innerRadius"] = 0.25f;            // -> render.innerRadius
    p.parameters["shape.minSize"] = 0.4f;                 // -> render.minSize
    p.parameters["shape.maxSize"] = 1.1f;                 // -> render.maxSize
    p.parameters["shape.rotation"] = 90.0f;               // -> render.rotation
    p.parameters["shape.beatReverse"] = true;             // -> render.beatReverse
    p.parameters["shape.color.angle"] = 45.0f;            // -> color.main.angle
    p.parameters["shape.color.beatBrightness"] = false;   // -> color.main.beatBrightness
    return p;
}

} // namespace

TEST_CASE("Pulsing 5.2: Alias-Map wird bei Instanziierung registriert")
{
    PulsingVisualizer viz;  // Konstruktor registriert die Alias-Map (einmalig)

    CHECK(VisualizerPresetManager::translateLegacyKey("pulsing", "shape.type")
          == "render.type");
    CHECK(VisualizerPresetManager::translateLegacyKey("pulsing", "shape.beatReverse")
          == "render.beatReverse");
    CHECK(VisualizerPresetManager::translateLegacyKey("pulsing", "shape.color.angle")
          == "color.main.angle");
    CHECK(VisualizerPresetManager::translateLegacyKey("pulsing", "shape.color.beatBrightness")
          == "color.main.beatBrightness");
    // Identitaet: audio.* bleibt — inkl. audio.bands (E2 gilt nur fuer den Equalizer)
    CHECK(VisualizerPresetManager::translateLegacyKey("pulsing", "audio.gain")
          == "audio.gain");
    CHECK(VisualizerPresetManager::translateLegacyKey("pulsing", "audio.bands")
          == "audio.bands");
}

TEST_CASE("Pulsing 5.2: PulseShapeModule liefert eigene paramDescs")
{
    PulseShapeModule shape;
    auto descs = shape.paramDescs();

    REQUIRE(descs.size() == 6);
    CHECK(descs[0].id == "type");
    CHECK(descs[1].id == "sides");
    CHECK(descs[1].dependsOn == "type");
    CHECK(descs[2].id == "innerRadius");
    CHECK(descs[3].id == "minSize");
    CHECK(descs[4].id == "maxSize");
    CHECK(descs[5].id == "rotation");

    // float-fuer-int-Vertrag: Enum/Int-Params akzeptieren float (JSON-Loader)
    CHECK(shape.setParam("type", 3.0f));  // Star als float
    ParamValue v;
    REQUIRE(shape.getParam("type", v));
    CHECK(std::get<int>(v) == 3);
    CHECK(shape.setParam("sides", 12.0f));
    REQUIRE(shape.getParam("sides", v));
    CHECK(std::get<int>(v) == 12);
}

TEST_CASE("Pulsing 5.2: paramDescs nur Soll-Keys, jede Stufe gesetzt")
{
    PulsingVisualizer viz;

    bool sawRenderType = false;
    bool sawBeatBrightness = false;
    for (const auto& desc : viz.paramDescs())
    {
        CAPTURE(desc.id);
        CHECK(desc.id.rfind("shape.", 0) != 0);  // Alt-Schema komplett weg
        CHECK(desc.stage != PipelineStage::None);

        sawRenderType |= (desc.id == "render.type");
        sawBeatBrightness |= (desc.id == "color.main.beatBrightness");
    }
    CHECK(sawRenderType);
    CHECK(sawBeatBrightness);
}

TEST_CASE("Pulsing 5.2: Alt-Preset (formatVersion 1) landet auf Soll-Keys")
{
    PulsingVisualizer viz;
    VisualizerPresetManager mgr;

    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));

    ParamValue v;
    REQUIRE(viz.getParam("audio.gain", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.5f));
    REQUIRE(viz.getParam("audio.bands", v));
    CHECK(std::get<int>(v) == 32);
    REQUIRE(viz.getParam("render.type", v));
    CHECK(std::get<int>(v) == 1);  // Ring
    REQUIRE(viz.getParam("render.sides", v));
    CHECK(std::get<int>(v) == 8);
    REQUIRE(viz.getParam("render.innerRadius", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.25f));
    REQUIRE(viz.getParam("render.minSize", v));
    CHECK(std::get<float>(v) == doctest::Approx(0.4f));
    REQUIRE(viz.getParam("render.maxSize", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.1f));
    REQUIRE(viz.getParam("render.rotation", v));
    CHECK(std::get<float>(v) == doctest::Approx(90.0f));
    REQUIRE(viz.getParam("render.beatReverse", v));
    CHECK(std::get<bool>(v) == true);
    REQUIRE(viz.getParam("color.main.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(45.0f));
    REQUIRE(viz.getParam("color.main.beatBrightness", v));
    CHECK(std::get<bool>(v) == false);
}

TEST_CASE("Pulsing 5.2: Capture/Save/Load-Roundtrip nur im neuen Schema")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());

    PulsingVisualizer viz;
    VisualizerPresetManager mgr;
    mgr.setPresetsDirectory(tmp.path());

    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));
    VisualizerPreset captured = mgr.capturePreset(&viz, "Migriert");

    // Gespeichert wird ausschliesslich im neuen Schema (kein Dual-Write, §9)
    for (const auto& [key, value] : captured.parameters)
    {
        CAPTURE(key);
        CHECK(key.rfind("shape.", 0) != 0);
    }

    REQUIRE(mgr.savePreset(captured));
    auto loaded = mgr.loadPreset("pulsing", "Migriert");
    REQUIRE(loaded.has_value());

    CHECK(loaded->formatVersion == VisualizerPresetManager::CURRENT_FORMAT_VERSION);
    REQUIRE(mgr.applyPreset(&viz, *loaded));

    ParamValue v;
    REQUIRE(viz.getParam("render.type", v));
    CHECK(std::get<int>(v) == 1);
    REQUIRE(viz.getParam("render.maxSize", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.1f));
    REQUIRE(viz.getParam("render.beatReverse", v));
    CHECK(std::get<bool>(v) == true);
}
