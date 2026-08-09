/**
 ****************************************************************************************
 * @file   ShortcutRegistry.hpp
 * @brief  SSOT der Hotkey-Aktionen: Bezeichner, Vorbelegung, Kategorie,
 *         Reservierung (Konzept: docs/ui/Hotkey_Konzept.md)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Eine Aktion ist ein STABILER Bezeichner, keine Taste (§3 des Konzepts). Die
 * Belegung ist Datum, nicht Code — sie liegt hier in einer Tabelle, und eine
 * Aenderung ist ein Einzeiler.
 *
 * Qt-frei mit Absicht: die Sequenzen sind Text im `QKeySequence`-Format
 * ("PageDown", "Ctrl+Right"), damit die Registry ohne Qt testbar bleibt. Das
 * Uebersetzen und Abgleichen macht der `ShortcutManager` in der UI-Schicht.
 *
 * **Reservierung (§2):** die Standard-Transporttasten gehoeren dauerhaft dem
 * Audio-Player — auch solange sie noch nichts tun. Presets bekommen deshalb
 * eigene, unmodifizierte Tasten und ausdruecklich NICHT `Space`.
 ****************************************************************************************
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace lumi::services {

/// Gliedert den Editor und traegt die Reservierungs-Regel.
enum class ShortcutCategory
{
    Transport,  ///< Audio-Wiedergabe (reserviert, §2)
    Preset,     ///< Preset-Navigation (aktives Verzeichnis / spaeter Playlist)
    View        ///< Fenster/Ansicht
};

struct ShortcutAction
{
    std::string_view id;               ///< z. B. "preset.next" — wandert nie
    std::string_view label;            ///< Anzeigename im Editor
    ShortcutCategory category;
    std::string_view defaultSequence;  ///< QKeySequence-Text; leer = keine Taste
    /// true = diese Taste darf nur eine Aktion DIESER Kategorie tragen (§2)
    bool reserved = false;
    /// false = registriert und reserviert, aber noch ohne Wirkung (Stufe 2)
    bool wired = true;
};

/**
 * @brief Die Aktionstabelle.
 *
 * Transport ist bewusst schon vollstaendig vertreten, obwohl `wired=false`:
 * so sind die Tasten ab sofort belegt und koennen nicht versehentlich an
 * Preset-Aktionen vergeben werden.
 */
[[nodiscard]] inline const std::vector<ShortcutAction>& shortcutActions()
{
    static const std::vector<ShortcutAction> kActions = {
        // --- Transport: reserviert (§2), seit Stufe 2 verdrahtet --------------
        {"transport.playPause", "Wiedergabe / Pause", ShortcutCategory::Transport,
         "Space", true},
        {"transport.next", "Nächster Song", ShortcutCategory::Transport,
         "Ctrl+Right", true},
        {"transport.previous", "Voriger Song", ShortcutCategory::Transport,
         "Ctrl+Left", true},
        {"transport.volumeUp", "Lauter", ShortcutCategory::Transport,
         "Ctrl+Up", true},
        {"transport.volumeDown", "Leiser", ShortcutCategory::Transport,
         "Ctrl+Down", true},

        // --- Preset: Stufe 1, wirkt auf das aktive Verzeichnis ----------------
        // Bild ab/Bild auf statt Pfeiltasten (die braucht jede Liste und jeder
        // Editor) und statt Space/Backspace der MilkDrop-Konvention (Space ist
        // nach §2 Transport). ACHTUNG: die Sequenzen sind QKeySequence-Text —
        // Qt kennt diese Tasten als "PgDown"/"PgUp", "PageDown" ergibt
        // Key_unknown und damit eine tote Vorbelegung (Session 52).
        {"preset.next", "Nächstes Preset", ShortcutCategory::Preset, "PgDown"},
        {"preset.previous", "Voriges Preset", ShortcutCategory::Preset, "PgUp"},

        // --- Ansicht ----------------------------------------------------------
        {"view.fullscreen", "Vollbild", ShortcutCategory::View, "F11", false, false},
        // Druck: nimmt das Visual auf statt des Bildschirms. Windows oeffnet auf
        // dieselbe Taste eventuell das Snipping Tool — das ist eine
        // Windows-Einstellung und liegt ausserhalb der App.
        {"view.screenshot", "Screenshot des Visuals", ShortcutCategory::View, "Print"},
    };
    return kActions;
}

/// @brief Aktion per Bezeichner; nullptr wenn unbekannt.
[[nodiscard]] inline const ShortcutAction* shortcutAction(std::string_view id)
{
    for (const ShortcutAction& a : shortcutActions())
    {
        if (a.id == id) return &a;
    }
    return nullptr;
}

/// @brief Anzeigename einer Kategorie (Editor-Gruppierung).
[[nodiscard]] inline std::string_view shortcutCategoryLabel(ShortcutCategory c)
{
    switch (c)
    {
        case ShortcutCategory::Transport: return "Wiedergabe";
        case ShortcutCategory::Preset: return "Presets";
        case ShortcutCategory::View: return "Ansicht";
    }
    return "";
}

/**
 * @brief Darf @p id die Sequenz @p sequence bekommen, oder ist sie reserviert?
 *
 * Reserviert heisst: eine ANDERE Aktion haelt sie als Vorbelegung und ihre
 * Kategorie ist geschuetzt. Dann darf nur eine Aktion derselben Kategorie sie
 * tragen (§2) — sonst nimmt eine Preset-Taste dem Transport `Space` weg.
 * Der Vergleich laeuft ueber die kanonische Textform, die der Aufrufer liefert.
 */
[[nodiscard]] inline bool shortcutSequenceReservedFor(std::string_view sequence,
                                                     std::string_view id)
{
    if (sequence.empty()) return false;
    const ShortcutAction* target = shortcutAction(id);
    for (const ShortcutAction& a : shortcutActions())
    {
        if (!a.reserved || a.id == id) continue;
        if (a.defaultSequence != sequence) continue;
        if (target == nullptr || target->category != a.category) return true;
    }
    return false;
}

}  // namespace lumi::services
