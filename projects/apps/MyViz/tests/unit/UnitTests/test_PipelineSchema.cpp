/**
 ****************************************************************************************
 * @file   test_PipelineSchema.cpp
 * @brief  Unit-Tests fuer das Phase-4-Schema: PipelineStage, GradientHandles,
 *         TapPoints und die Legacy-Key-Migration des PresetManagers
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/IVisualizer.hpp"
#include "visualizers/VisualizerPresetManager.hpp"
#include "visualizers/modules/AudioUtil.hpp"
#include "visualizers/modules/ColorGradientModule.hpp"
#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/JsonPresetParser.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

using lumi::modules::ModuleParamDesc;
using lumi::modules::ParamBuilder;
using lumi::modules::ParamType;
using lumi::modules::ParamValue;
using lumi::modules::PipelineStage;

// =============================================================================
// Fake-Visualizer (zeichnet setParam auf; liefert Handle + TapPoint)
// =============================================================================

namespace
{

class FakeVisualizer : public IVisualizer
{
public:
    [[nodiscard]] QString visualizerId() const override { return "fake"; }
    [[nodiscard]] QString visualizerName() const override { return "Fake"; }
    [[nodiscard]] QString visualizerDescription() const override { return "Test"; }
    void initialize() override {}
    void render(float) override {}
    void resize(const QSize&) override {}
    void cleanup() override {}
    [[nodiscard]] bool isInitialized() const override { return true; }

    [[nodiscard]] bool hasParameterSupport() const override { return true; }

    bool setParam(const std::string& id, const ParamValue& value) override
    {
        receivedKeys.push_back(id);
        lastValue = value;
        return true;
    }

    [[nodiscard]] std::vector<GradientHandle> gradients() override
    {
        return {{"main", "Color", "shape.color.", &gradient}};
    }

    [[nodiscard]] std::vector<TapPoint> tapPoints() override
    {
        return {{"tap.map", "Mapping", PipelineStage::Mapping,
                 [this]() { return mapData; }}};
    }

    lumi::modules::ColorGradientModule gradient;
    std::vector<float> mapData{0.1f, 0.5f, 0.9f};
    std::vector<std::string> receivedKeys;
    ParamValue lastValue;
};

} // namespace

// =============================================================================
// PipelineStage im Parameter-Schema
// =============================================================================

TEST_CASE("Schema: ModuleParamDesc hat default PipelineStage::None (unmigriert)")
{
    ModuleParamDesc desc;
    CHECK(desc.stage == PipelineStage::None);
}

TEST_CASE("Schema: ParamBuilder setzt die Pipeline-Stufe")
{
    auto desc = ParamBuilder("bands", ParamType::Int)
                    .displayName("Bands")
                    .stage(PipelineStage::Mapping)
                    .build();

    CHECK(desc.stage == PipelineStage::Mapping);
    CHECK(desc.id == "bands");
}

TEST_CASE("Schema: PipelineStage-Werte folgen der Datenfluss-Reihenfolge 1..6")
{
    CHECK(static_cast<int>(PipelineStage::AudioSource) == 1);
    CHECK(static_cast<int>(PipelineStage::Mapping) == 2);
    CHECK(static_cast<int>(PipelineStage::Color) == 3);
    CHECK(static_cast<int>(PipelineStage::Render) == 4);
    CHECK(static_cast<int>(PipelineStage::PeakParticle) == 5);
    CHECK(static_cast<int>(PipelineStage::Post) == 6);
}

// =============================================================================
// GradientHandles & TapPoints
// =============================================================================

TEST_CASE("Schema: IVisualizer liefert default keine Handles/TapPoints")
{
    class Minimal : public FakeVisualizer
    {
    public:
        [[nodiscard]] std::vector<GradientHandle> gradients() override
        {
            return IVisualizer::gradients();  // Default
        }
        [[nodiscard]] std::vector<TapPoint> tapPoints() override
        {
            return IVisualizer::tapPoints();  // Default
        }
    };

    Minimal viz;
    CHECK(viz.gradients().empty());
    CHECK(viz.tapPoints().empty());
}

TEST_CASE("Schema: GradientHandle liefert benannten Zugriff auf das Gradient-Modul")
{
    FakeVisualizer viz;
    auto handles = viz.gradients();

    REQUIRE(handles.size() == 1);
    CHECK(handles[0].id == "main");
    CHECK(handles[0].paramPrefix == "shape.color.");
    CHECK(handles[0].gradient == &viz.gradient);
}

TEST_CASE("Schema: TapPoint sampelt die Stufen-Ausgangsdaten (pull-basiert)")
{
    FakeVisualizer viz;
    auto taps = viz.tapPoints();

    REQUIRE(taps.size() == 1);
    CHECK(taps[0].id == "tap.map");
    CHECK(taps[0].stage == PipelineStage::Mapping);

    auto data = taps[0].sample();
    REQUIRE(data.size() == 3);
    CHECK(data[1] == doctest::Approx(0.5f));

    // Aenderung der Quelldaten schlaegt beim naechsten sample() durch
    viz.mapData = {1.0f};
    CHECK(taps[0].sample().size() == 1);
}

// =============================================================================
// Shared-Module-Helfer (5.6): AudioUtil + JsonPresetParser
// =============================================================================

TEST_CASE("AudioUtil: splitStereoData + resampleNearest (eine Implementierung, N3)")
{
    using lumi::modules::resampleNearest;
    using lumi::modules::splitStereoData;

    std::vector<float> interleaved{0.1f, -0.1f, 0.2f, -0.2f, 0.3f, -0.3f};
    std::vector<float> left, right;
    splitStereoData(interleaved, left, right);
    REQUIRE(left.size() == 3);
    CHECK(left[1] == doctest::Approx(0.2f));
    CHECK(right[2] == doctest::Approx(-0.3f));

    std::vector<float> resampled;
    resampleNearest(left, resampled, 6, 2.0f);
    REQUIRE(resampled.size() == 6);
    CHECK(resampled[0] == doctest::Approx(0.2f));   // left[0] * gain
    CHECK(resampled[5] == doctest::Approx(0.6f));   // left[2] * gain

    // Leere Quelle laesst das Ziel unangetastet
    std::vector<float> empty;
    resampleNearest(empty, resampled, 4, 1.0f);
    CHECK(resampled.size() == 6);
}

TEST_CASE("JsonPresetParser: Skalar- und Array-Extraktion (Modul-Preset-Format)")
{
    using lumi::modules::JsonPresetParser;

    JsonPresetParser parser(R"({
  "name": "Testpreset",
  "mode": 1,
  "angle": 45.5,
  "clamp01": true,
  "stops": [[0,1,0,0,1], [1,0,0,1,1]],
  "midpoints": [0.5]
})");

    CHECK(parser.getString("name") == "Testpreset");
    CHECK(parser.getInt("mode") == 1);
    CHECK(parser.getFloat("angle") == doctest::Approx(45.5f));
    CHECK(parser.getBool("clamp01") == true);
    // Fallbacks bei fehlenden Keys
    CHECK(parser.getString("fehlt", "fallback") == "fallback");
    CHECK(parser.getInt("fehlt", 8) == 8);
    CHECK(parser.getBool("fehlt") == false);
    // Verschachteltes Array via Klammerzaehlung
    CHECK(parser.getArrayContent("stops") == "[0,1,0,0,1], [1,0,0,1,1]");
    CHECK(parser.getArrayContent("midpoints") == "0.5");
    CHECK(parser.getArrayContent("fehlt").empty());
}

// =============================================================================
// Legacy-Key-Migration (Alias-Map + formatVersion)
// =============================================================================

TEST_CASE("PresetManager: translateLegacyKey uebersetzt registrierte Keys, Rest passthrough")
{
    using lumi::VisualizerPresetManager;
    VisualizerPresetManager::clearKeyAliases();

    VisualizerPresetManager::registerKeyAliases(
        "fake", {{"eq.bands", "map.bands"}, {"color.domain", "color.main.domain"}});

    CHECK(VisualizerPresetManager::translateLegacyKey("fake", "eq.bands") == "map.bands");
    CHECK(VisualizerPresetManager::translateLegacyKey("fake", "color.domain")
          == "color.main.domain");
    // Unbekannter Key und unbekannter Visualizer: unveraendert
    CHECK(VisualizerPresetManager::translateLegacyKey("fake", "audio.gain") == "audio.gain");
    CHECK(VisualizerPresetManager::translateLegacyKey("other", "eq.bands") == "eq.bands");

    VisualizerPresetManager::clearKeyAliases();
}

TEST_CASE("PresetManager: applyPreset uebersetzt Keys NUR bei altem formatVersion")
{
    using lumi::VisualizerPreset;
    using lumi::VisualizerPresetManager;
    VisualizerPresetManager::clearKeyAliases();
    VisualizerPresetManager::registerKeyAliases("fake", {{"eq.bands", "map.bands"}});

    VisualizerPresetManager manager;
    FakeVisualizer viz;

    VisualizerPreset preset;
    preset.name = "Legacy";
    preset.visualizerId = "fake";
    preset.parameters["eq.bands"] = 32;

    // Altes Schema (formatVersion < CURRENT) -> Key wird uebersetzt
    preset.formatVersion = VisualizerPresetManager::CURRENT_FORMAT_VERSION - 1;
    CHECK(manager.applyPreset(&viz, preset));
    REQUIRE(viz.receivedKeys.size() == 1);
    CHECK(viz.receivedKeys[0] == "map.bands");

    // Aktuelles Schema -> Key unveraendert
    viz.receivedKeys.clear();
    preset.formatVersion = VisualizerPresetManager::CURRENT_FORMAT_VERSION;
    CHECK(manager.applyPreset(&viz, preset));
    REQUIRE(viz.receivedKeys.size() == 1);
    CHECK(viz.receivedKeys[0] == "eq.bands");

    VisualizerPresetManager::clearKeyAliases();
}

// =============================================================================
// Wert-Konverter (E3: alterKey -> (neuerKey, Konverter))
// =============================================================================

namespace
{

/// E3-Referenzformel: EMA-Faktor s -> Glaettungszeit in ms (60-FPS-Annahme)
ParamValue smoothingToTimeMs(const ParamValue& value)
{
    const float s = std::get<float>(value);
    if (s <= 0.0f)
    {
        return 0.0f;  // keine Glaettung
    }
    const float clamped = std::min(s, 0.999f);  // s >= 1 clampen (ln(1) = 0)
    return -16.67f / std::log(clamped);
}

} // namespace

TEST_CASE("PresetManager: translateLegacyParam wendet registrierten Wert-Konverter an")
{
    using lumi::VisualizerPresetManager;
    VisualizerPresetManager::clearKeyAliases();

    VisualizerPresetManager::registerKeyAliases("fake", {{"eq.bands", "map.bands"}});
    VisualizerPresetManager::registerKeyConverter(
        "fake", "waveform.smoothing", "audio.smooth.timeMs", smoothingToTimeMs);

    // Konverter-Eintrag: Key UND Wert uebersetzt (s = 0.9 -> ~158 ms)
    auto [key, value] =
        VisualizerPresetManager::translateLegacyParam("fake", "waveform.smoothing", 0.9f);
    CHECK(key == "audio.smooth.timeMs");
    CHECK(std::get<float>(value) == doctest::Approx(158.2f).epsilon(0.01));

    // Randfaelle laut Migrationstabelle: s <= 0 -> 0 ms; s >= 1 -> geclampt (endlich, > 0)
    auto [k0, v0] =
        VisualizerPresetManager::translateLegacyParam("fake", "waveform.smoothing", 0.0f);
    CHECK(std::get<float>(v0) == 0.0f);
    auto [k1, v1] =
        VisualizerPresetManager::translateLegacyParam("fake", "waveform.smoothing", 1.0f);
    CHECK(std::get<float>(v1) > 0.0f);
    CHECK(std::isfinite(std::get<float>(v1)));

    // Reiner Key-Alias: Wert unveraendert
    auto [k2, v2] = VisualizerPresetManager::translateLegacyParam("fake", "eq.bands", 32.0f);
    CHECK(k2 == "map.bands");
    CHECK(std::get<float>(v2) == 32.0f);

    // Unbekannter Key: beides passthrough
    auto [k3, v3] = VisualizerPresetManager::translateLegacyParam("fake", "audio.gain", 1.5f);
    CHECK(k3 == "audio.gain");
    CHECK(std::get<float>(v3) == 1.5f);

    VisualizerPresetManager::clearKeyAliases();
}

TEST_CASE("PresetManager: applyPreset konvertiert Werte NUR bei altem formatVersion")
{
    using lumi::VisualizerPreset;
    using lumi::VisualizerPresetManager;
    VisualizerPresetManager::clearKeyAliases();
    VisualizerPresetManager::registerKeyConverter(
        "fake", "waveform.smoothing", "audio.smooth.timeMs", smoothingToTimeMs);

    VisualizerPresetManager manager;
    FakeVisualizer viz;

    VisualizerPreset preset;
    preset.name = "Legacy";
    preset.visualizerId = "fake";
    preset.parameters["waveform.smoothing"] = 0.9f;

    // Altes Schema -> Key uebersetzt + Wert konvertiert
    preset.formatVersion = VisualizerPresetManager::CURRENT_FORMAT_VERSION - 1;
    CHECK(manager.applyPreset(&viz, preset));
    REQUIRE(viz.receivedKeys.size() == 1);
    CHECK(viz.receivedKeys[0] == "audio.smooth.timeMs");
    CHECK(std::get<float>(viz.lastValue) == doctest::Approx(158.2f).epsilon(0.01));

    // Aktuelles Schema -> unangetastet
    viz.receivedKeys.clear();
    preset.formatVersion = VisualizerPresetManager::CURRENT_FORMAT_VERSION;
    CHECK(manager.applyPreset(&viz, preset));
    REQUIRE(viz.receivedKeys.size() == 1);
    CHECK(viz.receivedKeys[0] == "waveform.smoothing");
    CHECK(std::get<float>(viz.lastValue) == 0.9f);

    VisualizerPresetManager::clearKeyAliases();
}
