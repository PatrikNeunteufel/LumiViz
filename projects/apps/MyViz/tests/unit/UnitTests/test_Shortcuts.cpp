/**
 ****************************************************************************************
 * @file   test_Shortcuts.cpp
 * @brief  Gates fuer die Hotkey-Schicht (S51): Registry-Vertrag,
 *         Reservierungs-Regel und die Blaetter-Logik des Import-Browsers
 *
 * Konzept: docs/ui/Hotkey_Konzept.md. Geprueft wird das, was ohne Fenster
 * pruefbar ist — die Tabelle (SSOT der Belegung), die Reservierung der
 * Transporttasten und die Auswahl der naechsten Preset-Zeile. Der
 * Ereignisfilter selbst braucht einen laufenden qApp mit Fokus und bleibt
 * Sichttest.
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/panels/ImportBrowserPanel.hpp"
#include "services/ShortcutRegistry.hpp"

#include <QList>

#include <set>
#include <string>

using namespace lumi::services;

TEST_SUITE("Shortcuts")
{
    TEST_CASE("Registry: Bezeichner und Vorbelegungen sind eindeutig")
    {
        std::set<std::string> ids;
        std::set<std::string> sequences;
        for (const ShortcutAction& a : shortcutActions())
        {
            CHECK(ids.insert(std::string(a.id)).second);  // kein doppelter Name
            CHECK_FALSE(a.id.empty());
            CHECK_FALSE(a.label.empty());
            if (!a.defaultSequence.empty())
            {
                // Zwei Aktionen mit derselben Vorbelegung waeren ein Konflikt,
                // den der Nutzer nie aufloesen koennte.
                CHECK(sequences.insert(std::string(a.defaultSequence)).second);
            }
            // Bezeichner sind punktiert (Kategorie.aktion) — haelt sie sortierbar.
            CHECK(a.id.find('.') != std::string_view::npos);
        }
    }

    TEST_CASE("Registry: Transport ist reserviert, Presets meiden Space")
    {
        bool sawTransport = false;
        for (const ShortcutAction& a : shortcutActions())
        {
            if (a.category == ShortcutCategory::Transport)
            {
                sawTransport = true;
                CHECK(a.reserved);  // §2: dauerhaft dem Player
            }
            if (a.category == ShortcutCategory::Preset)
            {
                // Space gehoert dem Transport — eine Preset-Taste darf ihn nicht
                // haben, auch nicht als Vorbelegung (§2).
                CHECK(a.defaultSequence != "Space");
            }
        }
        CHECK(sawTransport);
        // Die Stufe-1-Aktionen muessen existieren und verdrahtet sein.
        const ShortcutAction* next = shortcutAction("preset.next");
        REQUIRE(next != nullptr);
        CHECK(next->wired);
        CHECK(next->defaultSequence == "PageDown");
        const ShortcutAction* prev = shortcutAction("preset.previous");
        REQUIRE(prev != nullptr);
        CHECK(prev->wired);
        CHECK(prev->defaultSequence == "PageUp");
        CHECK(shortcutAction("gibtsnicht") == nullptr);
    }

    TEST_CASE("Reservierungs-Regel: fremde Kategorie wird abgewiesen")
    {
        // Space haelt transport.playPause und ist reserviert.
        CHECK(shortcutSequenceReservedFor("Space", "preset.next"));
        // Innerhalb des Transports ist Umbelegen erlaubt (dort gehoert sie hin).
        CHECK_FALSE(shortcutSequenceReservedFor("Space", "transport.next"));
        // Die haltende Aktion selbst darf ihre eigene Taste behalten.
        CHECK_FALSE(shortcutSequenceReservedFor("Space", "transport.playPause"));
        // Nicht reservierte und leere Sequenzen sind frei.
        CHECK_FALSE(shortcutSequenceReservedFor("PageDown", "view.fullscreen"));
        CHECK_FALSE(shortcutSequenceReservedFor("", "preset.next"));
    }

    TEST_CASE("Import-Browser: naechste Preset-Zeile ueberspringt Ordner")
    {
        using P = ImportBrowserPanel;
        // ".." , Ordner, Ordner, AVS, LVFX, MilkDrop
        const QList<int> types{P::Type_Up, P::Type_Dir, P::Type_Dir, P::Type_Avs,
                               P::Type_Lvfx, P::Type_Milk};

        // Ohne Auswahl faengt es am jeweiligen Ende an, damit der erste
        // Tastendruck etwas laedt.
        CHECK(P::nextPresetRow(types, -1, 1) == 3);
        CHECK(P::nextPresetRow(types, -1, -1) == 5);

        CHECK(P::nextPresetRow(types, 3, 1) == 4);
        CHECK(P::nextPresetRow(types, 4, 1) == 5);
        CHECK(P::nextPresetRow(types, 5, -1) == 4);
        // Aus einem Ordner heraus vorwaerts: erstes Preset danach.
        CHECK(P::nextPresetRow(types, 1, 1) == 3);
        // Rueckwaerts vor dem ersten Preset: nichts mehr (kein Umlauf).
        CHECK(P::nextPresetRow(types, 3, -1) == -1);
        // Am Ende anhalten (Entscheid 3), nicht bei 3 wieder anfangen.
        CHECK(P::nextPresetRow(types, 5, 1) == -1);
        // delta 0 bewegt nichts; leere Liste ergibt nichts.
        CHECK(P::nextPresetRow(types, 3, 0) == -1);
        CHECK(P::nextPresetRow({}, -1, 1) == -1);
        // Groessere Schrittweite wirkt wie ein Schritt in ihre Richtung — die
        // Zeilen sind keine gleichmaessige Skala.
        CHECK(P::nextPresetRow(types, 3, 5) == 4);
    }

    TEST_CASE("Import-Browser: Verzeichnis ohne Presets")
    {
        using P = ImportBrowserPanel;
        const QList<int> nurOrdner{P::Type_Up, P::Type_Dir, P::Type_Dir};
        CHECK(P::nextPresetRow(nurOrdner, -1, 1) == -1);
        CHECK(P::nextPresetRow(nurOrdner, -1, -1) == -1);
        CHECK(P::nextPresetRow(nurOrdner, 1, 1) == -1);

        // Auswahl ausserhalb des Bereichs (Liste hat sich geaendert) darf nicht
        // abstuerzen, sondern faengt am Ende an.
        const QList<int> mitPreset{P::Type_Up, P::Type_Avs};
        CHECK(P::nextPresetRow(mitPreset, 99, -1) == 1);
        CHECK(P::nextPresetRow(mitPreset, 99, 1) == 1);
    }
}
