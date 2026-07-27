/**
 ****************************************************************************************
 * @file   test_Screenshot.cpp
 * @brief  Gates fuer die Screenshot-Ablage (S52): Ordnername je Lauf,
 *         Dateiname aus dem Preset-Pfad, Kollisionsfolge
 *
 * Geprueft wird das, was ohne Fenster pruefbar ist — die drei reinen Regeln.
 * Aufnehmen selbst braucht einen GL-Kontext und bleibt Sichttest.
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/managers/ScreenshotManager.hpp"

#include <QDate>
#include <QDateTime>
#include <QTime>

#include <set>

TEST_SUITE("Screenshot")
{
    TEST_CASE("Ordnername eines Laufs sortiert sich chronologisch")
    {
        const QDateTime t(QDate(2026, 7, 27), QTime(11, 47, 57));
        CHECK(ScreenshotManager::sessionFolderName(t) ==
              QStringLiteral("2026-07-27_11-47-57"));

        // Alphabetisch = chronologisch: sonst findet man den letzten Lauf nicht.
        const QDateTime later(QDate(2026, 7, 27), QTime(11, 48, 0));
        CHECK(ScreenshotManager::sessionFolderName(t) <
              ScreenshotManager::sessionFolderName(later));
        // Keine Doppelpunkte — unter Windows in Pfaden verboten.
        CHECK_FALSE(ScreenshotManager::sessionFolderName(t).contains(QLatin1Char(':')));
    }

    TEST_CASE("Dateiname behaelt die Endung als Namensteil")
    {
        // S45: .avs und sein .lvfx-Zwilling teilen den Basisnamen — ohne die
        // Endung im Namen ueberschreibt der zweite den Screenshot des ersten.
        CHECK(ScreenshotManager::shotBaseName(
                  QStringLiteral("C:/presets/Alien Alloy.avs")) ==
              QStringLiteral("Alien Alloy_avs"));
        CHECK(ScreenshotManager::shotBaseName(
                  QStringLiteral("C:/presets/Alien Alloy.lvfx")) ==
              QStringLiteral("Alien Alloy_lvfx"));
        CHECK(ScreenshotManager::shotBaseName(QStringLiteral("C:/p/a.b.milk")) ==
              QStringLiteral("a_b_milk"));

        // Ohne Preset (eigene Kette) trotzdem ein brauchbarer Name.
        CHECK(ScreenshotManager::shotBaseName(QString()) == QStringLiteral("visual"));
    }

    TEST_CASE("Dateiname ist auch bei rohen Presetnamen schreibbar")
    {
        // AVS-Presetnamen tragen regelmaessig Zeichen, die Windows in
        // Dateinamen ablehnt — ohne Ersetzung schluege das Speichern still fehl.
        const QString name =
            ScreenshotManager::shotBaseName(QStringLiteral("C:/p/Whacko: V?.avs"));
        for (const QChar c : QStringLiteral("<>:\"/\\|?*"))
        {
            CAPTURE(c.unicode());
            CHECK_FALSE(name.contains(c));
        }
        CHECK(name.endsWith(QStringLiteral("_avs")));
    }

    TEST_CASE("Zweite Aufnahme desselben Presets ueberschreibt nicht")
    {
        std::set<QString> vorhanden;
        const auto exists = [&vorhanden](const QString& n) {
            return vorhanden.count(n) != 0;
        };

        const QString base = QStringLiteral("Alien Alloy_avs");
        CHECK(ScreenshotManager::uniqueBaseName(base, exists) == base);

        vorhanden.insert(base);
        CHECK(ScreenshotManager::uniqueBaseName(base, exists) == base + "_2");

        vorhanden.insert(base + "_2");
        CHECK(ScreenshotManager::uniqueBaseName(base, exists) == base + "_3");

        // Ein anderes Preset bleibt vom Zaehler unberuehrt.
        CHECK(ScreenshotManager::uniqueBaseName(QStringLiteral("Andere_avs"), exists) ==
              QStringLiteral("Andere_avs"));
    }
}
