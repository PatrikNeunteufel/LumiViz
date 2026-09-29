/**
 ****************************************************************************************
 * @file   EffectPalette.hpp
 * @brief  Die Effekt-Palette: welche Module sich in eine Kette einfuegen lassen,
 *         mit Kategorie, Herkunft, Rolle und Kurzbeschreibung
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Bis S79 lag die Tabelle anonym in `MultiEffectPanel.cpp` und kannte nur Name,
 * Fabrik und Herkunft — genug fuer eine Combo, zu wenig fuer eine Suche. Seit
 * S79 liest sie ausserdem das Palettenfenster (`EffectPaletteWindow`), und
 * jeder Eintrag traegt Rolle, Kurzbeschreibung und den Originalnamen (AVS/APE),
 * damit man auch nach „Trans / Movement" findet.
 *
 * Eintraege mit `make == nullptr` und ohne `milkElement` sind Kategorie-Koepfe.
 ****************************************************************************************
 */
#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <QIcon>
#include <QString>

#include <vector>

namespace lumi::ui {

/// Woher ein Modul stammt. Avs = Nullsoft-AVS-Builtin, Ape = AVS-Community-
/// Plugin (APE), MilkDrop = aus MilkDrop portiert, Native = LumiViz-eigen.
enum class Origin
{
    Avs,
    Ape,
    MilkDrop,
    Native
};

/// Was ein Modul mit dem Bild tut (Wunsch Patrik 2026-08-06: Mesh Warp allein
/// in der Kette blieb schwarz — dass es ein Umformer ist, sah man nicht).
enum class Role
{
    Render,     ///< zeichnet neuen Inhalt
    Transform,  ///< bewegt, verformt oder koppelt das vorhandene Bild zurueck
    Color,      ///< aendert Farben/Pixel des vorhandenen Bilds
    Control,    ///< zeichnet nichts: Struktur, Takt, Puffer, Zustand
};

/// Inhalts-Elemente eines Milkdrop-Nodes (N3.1/N3.3): kein Chain-Effekt — sie
/// landen im PresetState des gewaehlten Milkdrop-Nodes.
enum class MilkElement
{
    None,
    Wave,
    Shape,
    Sprite
};

struct EffectType
{
    const char* name;
    lumi::multieffect::EffectParams (*make)() = nullptr;
    Role role = Role::Control;
    Origin origin = Origin::Avs;
    const char* summary = "";       ///< ein Satz, Englisch (UI-Sprache)
    const char* originalName = "";  ///< Name im Original (AVS-Menue, APE-Titel)
    MilkElement milkElement = MilkElement::None;
    /// „Preset as Group" (S82): vor dem Einfuegen eine Preset-Datei waehlen
    /// (.avs/.milk/.lvfx) — `make` liefert die Host-Gruppe, ihr Inhalt kommt
    /// aus der Datei (MultiEffectVisualizer::buildPresetTree).
    bool fromFile = false;
    /// „Preset Reference" (S82): wie fromFile, aber als Presetverweis — die
    /// Kette speichert nur den Pfad (HostGroupParams::verweis)
    bool asReference = false;

    [[nodiscard]] bool isHeader() const
    {
        return make == nullptr && milkElement == MilkElement::None;
    }
};

/// Die Palette in Anzeige-Reihenfolge (Koepfe und Eintraege gemischt).
[[nodiscard]] const std::vector<EffectType>& effectPalette();

/// Kategorie-Titel ohne Zierstriche ("— Fractals —" -> "Fractals").
[[nodiscard]] QString paletteCategoryTitle(const EffectType& header);

/// Herkunft eines vorhandenen Knotens, aufgeloest ueber die Palette.
[[nodiscard]] Origin originForParams(const lumi::multieffect::EffectParams& params);

[[nodiscard]] const QIcon& originIcon(Origin origin);
[[nodiscard]] QString originText(Origin origin);
[[nodiscard]] QString originToolTip(Origin origin);
[[nodiscard]] QString roleText(Role role);

}  // namespace lumi::ui
