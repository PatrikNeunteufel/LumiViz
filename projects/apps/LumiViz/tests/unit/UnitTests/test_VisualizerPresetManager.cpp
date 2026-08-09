/**
 ****************************************************************************************
 * @file   test_VisualizerPresetManager.cpp
 * @brief  Unit-Tests für den VisualizerPresetManager (Preset-Save/Load-Roundtrip —
 *         Sicherheitsnetz für die Config-Pipeline in Phase 4)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/VisualizerPresetManager.hpp"

#include <QTemporaryDir>

#include <string>

using lumi::modules::ParamValue;
using lumi::VisualizerPreset;
using lumi::VisualizerPresetManager;

namespace
{

struct ManagerFixture
{
    ManagerFixture()
    {
        REQUIRE(tmp.isValid());
        mgr.setPresetsDirectory(tmp.path());
    }

    VisualizerPreset makePreset(const QString& name = "UnitTestPreset") const
    {
        VisualizerPreset p;
        p.name = name;
        p.visualizerId = "equalizer";
        p.description = "Testbeschreibung";
        p.author = "UnitTest";
        p.parameters["bands"] = ParamValue{64};
        p.parameters["gain"] = ParamValue{1.5f};
        p.parameters["peak.enabled"] = ParamValue{true};
        p.parameters["label"] = ParamValue{std::string("hallo")};
        return p;
    }

    QTemporaryDir tmp;
    VisualizerPresetManager mgr;
};

} // namespace

// =============================================================================
// Roundtrip
// =============================================================================

// VERTRAG (dokumentiert in jsonToPreset): JSON unterscheidet int/float nicht — Zahlen
// kommen IMMER als float zurueck. setParam-Implementierungen muessen float fuer
// int-Parameter akzeptieren -> Anforderung an die Config-Pipeline (Phase 4)!
TEST_CASE("PresetManager: savePreset/loadPreset-Roundtrip (Zahlen kommen als float zurueck)")
{
    ManagerFixture fx;
    REQUIRE(fx.mgr.savePreset(fx.makePreset()));

    auto loaded = fx.mgr.loadPreset("equalizer", "UnitTestPreset");
    REQUIRE(loaded.has_value());

    CHECK(loaded->name == "UnitTestPreset");
    CHECK(loaded->visualizerId == "equalizer");
    CHECK(loaded->description == "Testbeschreibung");

    REQUIRE(loaded->parameters.count("bands") == 1);
    CHECK(std::get<float>(loaded->parameters.at("bands")) == doctest::Approx(64.0f));
    REQUIRE(loaded->parameters.count("gain") == 1);
    CHECK(std::get<float>(loaded->parameters.at("gain")) == doctest::Approx(1.5f));
    REQUIRE(loaded->parameters.count("peak.enabled") == 1);
    CHECK(std::get<bool>(loaded->parameters.at("peak.enabled")) == true);
    REQUIRE(loaded->parameters.count("label") == 1);
    CHECK(std::get<std::string>(loaded->parameters.at("label")) == "hallo");
}

// =============================================================================
// Verwaltung
// =============================================================================

TEST_CASE("PresetManager: presetExists/availablePresets")
{
    ManagerFixture fx;
    CHECK_FALSE(fx.mgr.presetExists("equalizer", "UnitTestPreset"));

    REQUIRE(fx.mgr.savePreset(fx.makePreset()));
    CHECK(fx.mgr.presetExists("equalizer", "UnitTestPreset"));
    CHECK(fx.mgr.availablePresets("equalizer").contains("UnitTestPreset"));

    // Presets sind je Visualizer getrennt
    CHECK_FALSE(fx.mgr.presetExists("waveform", "UnitTestPreset"));
    CHECK(fx.mgr.availablePresets("waveform").isEmpty());
}

TEST_CASE("PresetManager: renamePreset")
{
    ManagerFixture fx;
    REQUIRE(fx.mgr.savePreset(fx.makePreset()));

    CHECK(fx.mgr.renamePreset("equalizer", "UnitTestPreset", "Umbenannt"));
    CHECK_FALSE(fx.mgr.presetExists("equalizer", "UnitTestPreset"));
    CHECK(fx.mgr.presetExists("equalizer", "Umbenannt"));

    auto loaded = fx.mgr.loadPreset("equalizer", "Umbenannt");
    REQUIRE(loaded.has_value());
    CHECK(std::get<float>(loaded->parameters.at("bands")) == doctest::Approx(64.0f));
}

TEST_CASE("PresetManager: deletePreset")
{
    ManagerFixture fx;
    REQUIRE(fx.mgr.savePreset(fx.makePreset()));

    CHECK(fx.mgr.deletePreset("equalizer", "UnitTestPreset"));
    CHECK_FALSE(fx.mgr.presetExists("equalizer", "UnitTestPreset"));
    CHECK_FALSE(fx.mgr.loadPreset("equalizer", "UnitTestPreset").has_value());
    CHECK_FALSE(fx.mgr.deletePreset("equalizer", "UnitTestPreset")); // doppelt loeschen
}

TEST_CASE("PresetManager: Ueberschreiben aktualisiert den Inhalt")
{
    ManagerFixture fx;
    REQUIRE(fx.mgr.savePreset(fx.makePreset()));

    auto p2 = fx.makePreset();
    p2.parameters["bands"] = ParamValue{128};
    REQUIRE(fx.mgr.savePreset(p2));

    auto loaded = fx.mgr.loadPreset("equalizer", "UnitTestPreset");
    REQUIRE(loaded.has_value());
    CHECK(std::get<float>(loaded->parameters.at("bands")) == doctest::Approx(128.0f));
}
