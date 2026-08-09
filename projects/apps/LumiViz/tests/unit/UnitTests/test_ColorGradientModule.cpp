/**
 ****************************************************************************************
 * @file   test_ColorGradientModule.cpp
 * @brief  Unit-Tests für das ColorGradientModule (Sicherheitsnetz für Phase 4:
 *         Gradient-Verhalten und -Serialisierung müssen über den Config-Pipeline-
 *         Umbau hinweg stabil bleiben)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/modules/ColorGradientModule.hpp"

#include <QTemporaryDir>

using namespace lumi::modules;
using doctest::Approx;

namespace
{

Color4f red()  { return Color4f{1.0f, 0.0f, 0.0f, 1.0f}; }
Color4f blue() { return Color4f{0.0f, 0.0f, 1.0f, 1.0f}; }

} // namespace

// =============================================================================
// Solid & Sampling
// =============================================================================

TEST_CASE("ColorGradientModule: Solid-Modus liefert die Solid-Farbe fuer jedes t")
{
    ColorGradientModule g;
    g.setMode(GradientMode::Solid);
    g.setSolidColor(0.2f, 0.4f, 0.6f, 0.8f);

    for (float t : {0.0f, 0.3f, 1.0f})
    {
        auto c = g.sample(t);
        CHECK(c[0] == Approx(0.2f));
        CHECK(c[1] == Approx(0.4f));
        CHECK(c[2] == Approx(0.6f));
        CHECK(c[3] == Approx(0.8f));
    }
}

TEST_CASE("ColorGradientModule: clearStops haelt die Mindest-Stops-Invariante (>= 2)")
{
    ColorGradientModule g;
    g.clearStops();
    // ensureMinimumStops(): ein Gradient hat nie weniger als 2 Stops
    CHECK(g.stopCount() == 2);
}

TEST_CASE("ColorGradientModule: Linear-Gradient interpoliert zwischen den Stops")
{
    ColorGradientModule g;
    g.setMode(GradientMode::Linear);
    g.clearStops(); // laesst 2 Default-Stops zurueck -> per updateStop ersetzen
    g.updateStop(0, 0.0f, red());
    g.updateStop(1, 1.0f, blue());
    REQUIRE(g.stopCount() == 2);

    auto start = g.sample(0.0f);
    CHECK(start[0] == Approx(1.0f));
    CHECK(start[2] == Approx(0.0f));

    auto end = g.sample(1.0f);
    CHECK(end[0] == Approx(0.0f));
    CHECK(end[2] == Approx(1.0f));

    auto mid = g.sample(0.5f); // Default-Midpoint 0.5 -> lineare Mitte
    CHECK(mid[0] == Approx(0.5f).epsilon(0.05));
    CHECK(mid[2] == Approx(0.5f).epsilon(0.05));
}

TEST_CASE("ColorGradientModule: Stops werden nach Position sortiert gehalten")
{
    ColorGradientModule g;
    g.clearStops(); // 2 Default-Stops an 0.0 und 1.0
    g.addStop(0.5f, Color4f{0.0f, 1.0f, 0.0f, 1.0f});

    const auto& stops = g.stops();
    REQUIRE(stops.size() == 3);
    CHECK(stops[0].position == Approx(0.0f));
    CHECK(stops[1].position == Approx(0.5f));
    CHECK(stops[2].position == Approx(1.0f));
}

TEST_CASE("ColorGradientModule: Midpoint-Roundtrip (set/get)")
{
    ColorGradientModule g;
    g.clearStops();
    g.updateStop(0, 0.0f, red());
    g.updateStop(1, 1.0f, blue());

    g.setMidpoint(0, 0.25f);
    CHECK(g.midpoint(0) == Approx(0.25f));
}

// =============================================================================
// Serialisierung — das Herzstueck fuer Phase 4
// =============================================================================

TEST_CASE("ColorGradientModule: toJson/fromJson-Roundtrip erhaelt die Konfiguration")
{
    ColorGradientModule original;
    original.setMode(GradientMode::Linear);
    original.setAngle(42.5f);
    original.clearStops();
    original.updateStop(0, 0.0f, red());
    original.updateStop(1, 1.0f, blue());
    original.addStop(0.7f, Color4f{0.1f, 0.9f, 0.3f, 0.5f});
    original.setMidpoint(0, 0.3f);

    const std::string json = original.toJson();
    REQUIRE_FALSE(json.empty());

    ColorGradientModule restored;
    REQUIRE(restored.fromJson(json));

    CHECK(restored.mode() == GradientMode::Linear);
    CHECK(restored.angle() == Approx(42.5f));
    REQUIRE(restored.stopCount() == 3);
    CHECK(restored.stops()[1].position == Approx(0.7f));
    CHECK(restored.stops()[1].color[1] == Approx(0.9f));
    CHECK(restored.stops()[1].color[3] == Approx(0.5f));
    CHECK(restored.midpoint(0) == Approx(0.3f));

    // Semantik-Check: identisches Sampling
    for (float t : {0.0f, 0.35f, 0.7f, 1.0f})
    {
        auto a = original.sample(t);
        auto b = restored.sample(t);
        for (int i = 0; i < 4; ++i)
            CHECK(a[i] == Approx(b[i]));
    }
}

TEST_CASE("ColorGradientModule: getParam liefert fuer jeden deklarierten Parameter einen Wert")
{
    ColorGradientModule g;

    for (const auto& desc : g.paramDescs())
    {
        ParamValue v;
        CAPTURE(desc.id);
        CHECK(g.getParam(desc.id, v));
        // Roundtrip: denselben Wert setzen muss akzeptiert werden
        CHECK(g.setParam(desc.id, v));
    }
}

// =============================================================================
// Presets (User-Preset-Verzeichnis auf Temp umgebogen!)
// =============================================================================

TEST_CASE("ColorGradientModule: User-Preset speichern/laden/loeschen")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    ColorGradientModule::setUserPresetsDirectory(tmp.path().toStdString());

    ColorGradientModule g;
    g.setMode(GradientMode::Radial);
    g.clearStops();
    g.updateStop(0, 0.0f, red());
    g.updateStop(1, 1.0f, blue());
    g.savePreset("UnitTestPreset");

    auto names = g.presetNames();
    CHECK(std::find(names.begin(), names.end(), "UnitTestPreset") != names.end());

    ColorGradientModule other;
    other.loadPreset("UnitTestPreset");
    CHECK(other.mode() == GradientMode::Radial);
    CHECK(other.stopCount() == 2);

    CHECK(g.deletePreset("UnitTestPreset"));
    auto after = g.presetNames();
    CHECK(std::find(after.begin(), after.end(), "UnitTestPreset") == after.end());
}
