/**
 ****************************************************************************************
 * @file   test_EffectPalette.cpp
 * @brief  Waechter der Palettentabelle (S79): Such- und Filterdaten vollstaendig
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Das Palettenfenster sucht in Name, Originalname und Kurzbeschreibung und
 * filtert nach Herkunft und Rolle. Ein neuer Eintrag ohne Beschreibung faende
 * sich nur ueber seinen Namen; ein doppelter Name verwirrte Favoriten und
 * „Recently used" (sie merken sich Namen, nicht Indizes).
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/panels/EffectPalette.hpp"

#include <set>
#include <string>

using namespace lumi::ui;

TEST_CASE("EffectPalette: beginnt mit einem Kategorie-Kopf")
{
    const auto& palette = effectPalette();
    REQUIRE_FALSE(palette.empty());
    CHECK(palette.front().isHeader());
    CHECK(paletteCategoryTitle(palette.front()) == QStringLiteral("Structure & Control"));
}

TEST_CASE("EffectPalette: jeder Eintrag ist beschrieben und eindeutig")
{
    std::set<std::string> names;
    std::set<std::size_t> variantIndices;
    int entries = 0;
    for (const EffectType& t : effectPalette())
    {
        if (t.isHeader()) continue;
        ++entries;
        INFO(t.name);
        CHECK(names.insert(t.name).second);  // Name eindeutig
        CHECK(std::string(t.summary).size() > 10);
        if (t.origin == Origin::Avs) CHECK_FALSE(std::string(t.originalName).empty());
        // Typ nur einmal — ausser „aus Datei"-Eintraegen (S82), die bewusst
        // eine Host-Gruppe bauen und sie aus einer Preset-Datei fuellen
        if (t.make != nullptr && !t.fromFile)
            CHECK(variantIndices.insert(t.make().index()).second);
    }
    CHECK(entries >= 85);
}

TEST_CASE("EffectPalette: Preset as Group baut eine Host-Gruppe (S82)")
{
    int ausDatei = 0;
    for (const EffectType& t : effectPalette())
    {
        if (!t.fromFile) continue;
        ++ausDatei;
        INFO(t.name);
        REQUIRE(t.make != nullptr);
        CHECK(std::holds_alternative<lumi::multieffect::HostGroupParams>(t.make()));
        // Herkunft eines Gruppenknotens bleibt die des Host-Group-Eintrags
        CHECK(originForParams(t.make()) == Origin::Native);
    }
    CHECK(ausDatei == 1);
}
