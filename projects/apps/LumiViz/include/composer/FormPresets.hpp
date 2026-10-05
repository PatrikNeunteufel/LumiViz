/**
 * @file FormPresets.hpp
 * @brief Composer: Kurvenformen als Presets — Blenden und Stuecke, je in
 *        eigenem Ordner (docs/ui/Composer_Konzept.md §9, Stufe C4 Teil 2)
 *
 * Ein Preset ist **eine Pegelform**, ohne Zeit: dieselbe Form passt auf jede
 * Dauer oder Weite, eine Blende auf beide Seiten des Clips (Vorgabe Patrik
 * S95 / S96). Die beiden Arten sind nicht austauschbar und liegen getrennt:
 *
 * | Art | Endung | mitgeliefert | eigene |
 * |---|---|---|---|
 * | Blende | `.lvfade` | `asset/composer/fades/` | `<AppData>/composer/fades/` |
 * | Stueck | `.lvshape` | `asset/composer/shapes/` | `<AppData>/composer/shapes/` |
 *
 * Ablage wie bei den Knoten-Voreinstellungen und den Trail-Presets (Entscheid
 * Patrik S96: a): Aufwaertssuche ab dem Programmordner, Anker `README.md`; bei
 * gleichem Namen gewinnt der Benutzerordner; mitgelieferte Dateien lassen sich
 * in der App nicht ueberschreiben oder loeschen.
 *
 * Kein Widget, kein GL.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`; dazu
 * der Ordner `asset/composer`.
 */
#pragma once

#include "composer/Pegelform.hpp"

#include <QString>

#include <vector>

namespace lumi::composer::formpresets
{

struct Eintrag
{
    QString name;               ///< Anzeigename (= Dateiname ohne Endung)
    QString pfad;               ///< absolut; leer = gibt es nicht
    bool mitgeliefert = false;  ///< aus `asset/` — nicht ueberschreibbar
};

/// Endung der Presets dieser Art, mit Punkt.
[[nodiscard]] const char* endung(FormArt art);

/// Alle Presets der Art: erst die mitgelieferten, dann die eigenen, je nach
/// Namen sortiert. Ein eigenes mit dem Namen eines mitgelieferten verdraengt es.
[[nodiscard]] std::vector<Eintrag> liste(FormArt art);

/// Eintrag mit diesem Namen (ohne Gross/Klein); leerer Pfad = gibt es nicht.
[[nodiscard]] Eintrag finde(FormArt art, const QString& name);

/// Ein Preset lesen. false = unlesbar, kein Kurven-Preset oder eines der
/// anderen Art — `form` bleibt dann, wie sie war.
[[nodiscard]] bool lade(const QString& pfad, FormArt art, Pegelform& form);

/// `form` als eigenes Preset ihrer Art speichern (der Name wird bereinigt,
/// eine gleichnamige eigene Datei ueberschrieben). false = ungueltiger Name
/// oder Schreibfehler.
[[nodiscard]] bool speichere(const QString& name, const Pegelform& form, QString* pfad = nullptr);

/// Ein eigenes Preset loeschen. Mitgelieferte werden NICHT geloescht.
[[nodiscard]] bool loesche(FormArt art, const Eintrag& eintrag);

[[nodiscard]] QString benutzerOrdner(FormArt art);  ///< wird nicht angelegt
[[nodiscard]] QString assetOrdner(FormArt art);     ///< leer, wenn nicht gefunden

/// Test-Haken: Unterordner `user/` und `asset/` unter `wurzel`; leer =
/// normales Verhalten.
void setzeWurzelFuerTests(const QString& wurzel);

}  // namespace lumi::composer::formpresets
