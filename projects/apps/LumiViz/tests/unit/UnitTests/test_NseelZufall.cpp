/**
 ****************************************************************************************
 * @file   test_NseelZufall.cpp
 * @brief  EEL-rand() des MilkDrop-Dialekts (ns-eel2) gegen die ORIGINAL-Werte
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * Die Sollwerte stammen aus dem Original-Kern: MilkdropRef mit der Messkopie
 * `tools/MilkdropRef/patched/nseel-cfunc.c` und `MILKDROPREF_RAND_LOG`, die ersten
 * acht Zuege eines frischen Prozesses — gezogen von `per_frame_init` des Presets
 * `martin - stratospheric turbulences` (Argumente 100, 2, 5, 5, 6, 1, 10, 3).
 ****************************************************************************************
 */

#include <doctest.h>

#include "scripting/NseelZufall.hpp"
#include "scripting/ScriptContext.hpp"
#include "visualizers/milkdrop/MilkdropPresetState.hpp"
#include "visualizers/milkdrop/MilkdropSerializer.hpp"

using lumi::scripting::NseelZufall;

TEST_CASE("NseelZufall: die ersten acht Zuege wie im Original-Kern")
{
    NseelZufall z;
    const double arg[8] = {100, 2, 5, 5, 6, 1, 10, 3};
    const double soll[8] = {22.772551216, 0.681488381, 4.468258842, 4.747965192,
                            4.097453932,  0.673078214, 6.137211405, 1.572439322};
    for (int i = 0; i < 8; ++i)
    {
        CAPTURE(i);
        CHECK(z.rand(arg[i]) == doctest::Approx(soll[i]).epsilon(1e-9));
    }
}

TEST_CASE("NseelZufall: Kommazahl, rand(1) nicht immer 0, Grenze floor(max(x,1))")
{
    NseelZufall z;
    int ueberHalb = 0;
    for (int i = 0; i < 1000; ++i)
    {
        const double v = z.rand(1.0);
        CHECK(v >= 0.0);
        CHECK(v <= 1.0);
        ueberHalb += v > 0.5 ? 1 : 0;
    }
    CHECK(ueberHalb > 400);  // gleichverteilt, nicht konstant 0
    CHECK(ueberHalb < 600);
    // rand(0.3) und rand(1.9) haben beide die Grenze 1
    NseelZufall a;
    NseelZufall b;
    CHECK(a.rand(0.3) == doctest::Approx(b.rand(1.9)));
}

TEST_CASE("NseelZufall: Zuruecksetzen = Prozessstart; ueber 624 Zuege hinweg stabil")
{
    NseelZufall z;
    const double erster = z.rand(100);
    for (int i = 0; i < 2000; ++i) (void)z.rand(1);  // Blockwechsel bei 624
    z.zuruecksetzen();
    CHECK(z.rand(100) == doctest::Approx(erster));
}

TEST_CASE("NseelZufall: Preset-Schalter Skript-Zufall — Speichern/Laden, fehlt = bisher")
{
    lumi::milkdrop::PresetState a;
    a.eelRandOriginal = true;
    CHECK(lumi::milkdrop::presetFromJson(lumi::milkdrop::presetToJson(a), nullptr)
              .eelRandOriginal);
    CHECK_FALSE(lumi::milkdrop::presetFromJson(lumi::milkdrop::presetToJson({}), nullptr)
                    .eelRandOriginal);
}

TEST_CASE("NseelZufall: ScriptContext schaltet je Kontext, reset() setzt den Strom zurueck")
{
    lumi::scripting::ScriptContext ctx;
    CHECK_FALSE(ctx.milkdropRand());
    ctx.setMilkdropRand(true);
    const double erster = ctx.nextMilkRand(100);
    CHECK(erster == doctest::Approx(22.772551216));
    ctx.reset();
    CHECK(ctx.nextMilkRand(100) == doctest::Approx(erster));
}
