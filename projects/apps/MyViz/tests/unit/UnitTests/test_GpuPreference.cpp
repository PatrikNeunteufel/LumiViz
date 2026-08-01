/**
 ****************************************************************************************
 * @file   test_GpuPreference.cpp
 * @brief  Tests fuer die GpuPreference-Token-Logik (Session 62) — pur, ohne
 *         Registry: parseToken/upsertToken auf dem Windows-Datenformat
 *         "GpuPreference=N;" inkl. fremder Tokens im selben Wert
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "core/GpuPreference.hpp"

using Mode = GpuPreference::Mode;

TEST_CASE("GpuPreference: parseToken liest die drei Windows-Stufen")
{
    CHECK(GpuPreference::parseToken("GpuPreference=0;") == Mode::Automatic);
    CHECK(GpuPreference::parseToken("GpuPreference=1;") == Mode::PowerSaving);
    CHECK(GpuPreference::parseToken("GpuPreference=2;") == Mode::HighPerformance);
}

TEST_CASE("GpuPreference: parseToken ohne oder mit unbekanntem Token -> nullopt")
{
    CHECK_FALSE(GpuPreference::parseToken("").has_value());
    CHECK_FALSE(GpuPreference::parseToken("SwapEffectUpgradeEnable=1;").has_value());
    // unbekannte Stufe: nicht raten (Windows koennte neue Werte einfuehren)
    CHECK_FALSE(GpuPreference::parseToken("GpuPreference=7;").has_value());
    CHECK_FALSE(GpuPreference::parseToken("GpuPreference=;").has_value());
    // aehnlicher, aber fremder Token darf nicht matchen
    CHECK_FALSE(GpuPreference::parseToken("XGpuPreference=2;").has_value());
}

TEST_CASE("GpuPreference: parseToken findet den Token zwischen fremden Tokens")
{
    CHECK(GpuPreference::parseToken(
              "SwapEffectUpgradeEnable=1;GpuPreference=2;DirectXUserGlobalSettings=1;")
          == Mode::HighPerformance);
    // auch ohne abschliessendes Semikolon (defensive Lesart)
    CHECK(GpuPreference::parseToken("GpuPreference=1") == Mode::PowerSaving);
}

TEST_CASE("GpuPreference: upsertToken auf leerem Wert erzeugt genau den Token")
{
    CHECK(GpuPreference::upsertToken("", Mode::HighPerformance)
          == "GpuPreference=2;");
    CHECK(GpuPreference::upsertToken("", Mode::Automatic)
          == "GpuPreference=0;");
}

TEST_CASE("GpuPreference: upsertToken ersetzt in place und erhaelt fremde Tokens")
{
    CHECK(GpuPreference::upsertToken("GpuPreference=1;", Mode::HighPerformance)
          == "GpuPreference=2;");
    CHECK(GpuPreference::upsertToken(
              "SwapEffectUpgradeEnable=1;GpuPreference=0;", Mode::PowerSaving)
          == "SwapEffectUpgradeEnable=1;GpuPreference=1;");
    // Token fehlt: anhaengen, Bestand unangetastet
    CHECK(GpuPreference::upsertToken("SwapEffectUpgradeEnable=1;",
                                     Mode::HighPerformance)
          == "SwapEffectUpgradeEnable=1;GpuPreference=2;");
}

TEST_CASE("GpuPreference: upsert-parse-Roundtrip ueber alle Stufen")
{
    for (const Mode mode :
         {Mode::Automatic, Mode::PowerSaving, Mode::HighPerformance})
    {
        const std::string value = GpuPreference::upsertToken(
            "SwapEffectUpgradeEnable=1;", mode);
        CHECK(GpuPreference::parseToken(value) == mode);
    }
}
