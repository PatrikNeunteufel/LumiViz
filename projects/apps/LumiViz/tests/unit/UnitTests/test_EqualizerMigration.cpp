/**
 ****************************************************************************************
 * @file   test_EqualizerMigration.cpp
 * @brief  Unit-Tests fuer die Equalizer-Key-Migration (Phase 4 Schritt 5.1):
 *         Alias-Map alt->neu, formatVersion-Gate, Roundtrip Alt-Preset ->
 *         Soll-Keys -> Save/Load im neuen Schema (Parameter_Key_Migration.md §6/§9)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/EqualizerVisualizer.hpp"
#include "visualizers/VisualizerPresetManager.hpp"
#include "visualizers/modules/IModule.hpp"

#include <QTemporaryDir>

#include <string>

using lumi::VisualizerPreset;
using lumi::VisualizerPresetManager;
using lumi::modules::Color4f;
using lumi::modules::ParamValue;
using lumi::modules::PipelineStage;

namespace
{

/// Alt-Schema-Preset (formatVersion 1) mit Vertretern aller Key-Familien aus §6
VisualizerPreset makeLegacyPreset()
{
    VisualizerPreset p;
    p.name = "LegacyEq";
    p.visualizerId = "equalizer";
    p.formatVersion = 1;
    p.parameters["audio.gain"] = 1.5f;              // Stufe 1, unveraendert (E1!)
    p.parameters["eq.bands"] = 32;                  // -> map.bands (E2)
    p.parameters["eq.orientation"] = 1;             // -> map.orientation
    p.parameters["eq.barGap"] = 5.0f;               // -> render.barGap
    p.parameters["color.domain"] = 1;               // -> color.main.domain
    p.parameters["color.angle"] = 45.0f;            // -> color.main.angle
    p.parameters["thickness.base"] = 7.0f;          // -> peak.thickness.base
    p.parameters["spring.enabled"] = true;          // -> peak.spring.enabled
    p.parameters["spring.k"] = 80.0f;               // -> peak.spring.k
    p.parameters["peak.holdDelay"] = 300.0f;        // unveraendert (Identitaet)
    p.parameters["particle.maxPerBand"] = 16;       // unveraendert (Identitaet)
    p.parameters["peakColor.auto"] = false;         // -> peak.color.auto
    p.parameters["peakColor.fixed"] =
        lumi::modules::makeColorValue(Color4f{0.1f, 0.2f, 0.3f, 0.4f});
    return p;
}

} // namespace

TEST_CASE("Equalizer 5.1: Alias-Map wird bei Instanziierung registriert")
{
    EqualizerVisualizer viz;  // Konstruktor registriert die Alias-Map (einmalig)

    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "eq.bands")
          == "map.bands");
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "audio.bands")
          == "map.bands");  // E2: beide Alt-Keys -> map.bands
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "eq.barGap")
          == "render.barGap");
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "color.domain")
          == "color.main.domain");
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "color.gradientData")
          == "color.main.gradientData");
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "thickness.mode")
          == "peak.thickness.mode");
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "spring.useDelay")
          == "peak.spring.useDelay");
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "peakColor.fixed")
          == "peak.color.fixed");
    // Identitaets-Eintraege: unveraenderte Keys bleiben
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "audio.gain")
          == "audio.gain");
    CHECK(VisualizerPresetManager::translateLegacyKey("equalizer", "peak.enabled")
          == "peak.enabled");
}

TEST_CASE("Equalizer 5.1: paramDescs nur Soll-Keys, jede Stufe gesetzt")
{
    EqualizerVisualizer viz;

    bool sawMapBands = false;
    bool sawHeightScale = false;
    for (const auto& desc : viz.paramDescs())
    {
        CAPTURE(desc.id);
        // Kein Parameter mehr im Alt-Schema
        CHECK(desc.id.rfind("eq.", 0) != 0);
        CHECK(desc.id.rfind("thickness.", 0) != 0);
        CHECK(desc.id.rfind("spring.", 0) != 0);
        CHECK(desc.id.rfind("peakColor.", 0) != 0);
        CHECK(desc.id != "color.domain");
        CHECK(desc.id != "audio.bands");
        // Migriert = jede Stufe explizit (ConfigPanel sortiert nach Stage)
        CHECK(desc.stage != PipelineStage::None);

        sawMapBands |= (desc.id == "map.bands");
        sawHeightScale |= (desc.id == "render.heightScale");
    }
    CHECK(sawMapBands);
    CHECK(sawHeightScale);  // E1: neuer Parameter ohne Alt-Key
}

TEST_CASE("Equalizer 5.1: Alt-Preset (formatVersion 1) landet auf Soll-Keys")
{
    EqualizerVisualizer viz;
    VisualizerPresetManager mgr;

    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));

    ParamValue v;
    REQUIRE(viz.getParam("audio.gain", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.5f));
    REQUIRE(viz.getParam("map.bands", v));
    CHECK(std::get<int>(v) == 32);
    REQUIRE(viz.getParam("map.orientation", v));
    CHECK(std::get<int>(v) == 1);
    REQUIRE(viz.getParam("render.barGap", v));
    CHECK(std::get<float>(v) == doctest::Approx(5.0f));
    REQUIRE(viz.getParam("color.main.domain", v));
    CHECK(std::get<int>(v) == 1);
    REQUIRE(viz.getParam("color.main.angle", v));
    CHECK(std::get<float>(v) == doctest::Approx(45.0f));
    REQUIRE(viz.getParam("peak.thickness.base", v));
    CHECK(std::get<float>(v) == doctest::Approx(7.0f));
    REQUIRE(viz.getParam("peak.spring.enabled", v));
    CHECK(std::get<bool>(v) == true);
    REQUIRE(viz.getParam("peak.spring.k", v));
    CHECK(std::get<float>(v) == doctest::Approx(80.0f));
    REQUIRE(viz.getParam("peak.holdDelay", v));
    CHECK(std::get<float>(v) == doctest::Approx(300.0f));
    REQUIRE(viz.getParam("particle.maxPerBand", v));
    CHECK(std::get<int>(v) == 16);
    REQUIRE(viz.getParam("peak.color.auto", v));
    CHECK(std::get<bool>(v) == false);
    REQUIRE(viz.getParam("peak.color.fixed", v));
    REQUIRE(lumi::modules::holdsColor(v));
    CHECK(lumi::modules::getColor(v)[2] == doctest::Approx(0.3f));

    // map.bands treibt BEIDE Module (Puffer-Resize-Kopplung §7.2)
    CHECK(viz.audioSource()->bandCount() == 32);
    CHECK(viz.equalizerModule().bandCount() == 32);
}

TEST_CASE("Equalizer 5.1: eq.bands gewinnt gegen widerspruechliches audio.bands (§7.1)")
{
    EqualizerVisualizer viz;
    VisualizerPresetManager mgr;

    VisualizerPreset preset;
    preset.name = "Konflikt";
    preset.visualizerId = "equalizer";
    preset.formatVersion = 1;
    preset.parameters["audio.bands"] = 16;
    preset.parameters["eq.bands"] = 48;  // wirksamer UI-Key -> gewinnt

    REQUIRE(mgr.applyPreset(&viz, preset));
    CHECK(viz.equalizerModule().bandCount() == 48);
}

TEST_CASE("Equalizer 5.1: render.heightScale wirkt und ist persistierbar (E1)")
{
    EqualizerVisualizer viz;

    ParamValue v;
    REQUIRE(viz.getParam("render.heightScale", v));
    CHECK(std::get<float>(v) == doctest::Approx(1.0f));  // Default: neutral

    REQUIRE(viz.setParam("render.heightScale", 2.5f));
    CHECK(viz.equalizerModule().heightScale() == doctest::Approx(2.5f));
}

TEST_CASE("Equalizer 5.1: Capture/Save/Load-Roundtrip nur im neuen Schema")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());

    EqualizerVisualizer viz;
    VisualizerPresetManager mgr;
    mgr.setPresetsDirectory(tmp.path());

    // Alt-Preset einspielen, dann Zustand einfangen
    REQUIRE(mgr.applyPreset(&viz, makeLegacyPreset()));
    VisualizerPreset captured = mgr.capturePreset(&viz, "Migriert");

    // Gespeichert wird ausschliesslich im neuen Schema (kein Dual-Write, §9)
    for (const auto& [key, value] : captured.parameters)
    {
        CAPTURE(key);
        CHECK(key.rfind("eq.", 0) != 0);
        CHECK(key.rfind("thickness.", 0) != 0);
        CHECK(key.rfind("spring.", 0) != 0);
        CHECK(key.rfind("peakColor.", 0) != 0);
        CHECK(key != "color.domain");
    }

    REQUIRE(mgr.savePreset(captured));
    auto loaded = mgr.loadPreset("equalizer", "Migriert");
    REQUIRE(loaded.has_value());

    // Neu geladen: aktuelles Format -> keine Uebersetzung mehr noetig
    CHECK(loaded->formatVersion == VisualizerPresetManager::CURRENT_FORMAT_VERSION);
    REQUIRE(mgr.applyPreset(&viz, *loaded));

    ParamValue v;
    REQUIRE(viz.getParam("map.bands", v));
    CHECK(std::get<int>(v) == 32);
    REQUIRE(viz.getParam("render.barGap", v));
    CHECK(std::get<float>(v) == doctest::Approx(5.0f));
    REQUIRE(viz.getParam("peak.spring.k", v));
    CHECK(std::get<float>(v) == doctest::Approx(80.0f));
}
