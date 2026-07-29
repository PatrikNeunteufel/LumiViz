#pragma once

#include <QString>

/**
 * @file FieldDocs.hpp
 * @brief Erklaerungstext je Knotenfeld — die Tooltip-Quelle des Panels.
 *
 * Knoten-Parameter-Konzept §10: jedes Feld erklaert sich beim Ueberfahren.
 * Der Text wird NICHT hier von Hand gepflegt — er steht als Doxygen-Kommentar
 * am Feld des `…Params`-Structs in `EffectChain.hpp` und ist damit dort, wo ihn
 * auch liest, wer den Code aendert. `FieldDocs.cpp` ist daraus **erzeugt**:
 *
 * @code
 * python asset/calibration/fields/harvest_field_docs.py
 * @endcode
 *
 * Der Wachhund dagegen ist `test_FieldDocs.cpp`: er erzeugt die Tabelle im Test
 * nicht neu, sondern prueft, dass jedes Feld aus `nodepresets::fieldNames()`
 * einen Eintrag hat — bis auf eine benannte, nur schrumpfende Ausnahmeliste.
 * Ein neues Feld ohne Kommentar faellt damit sofort auf.
 */
namespace lumi::multieffect::fielddocs
{

/**
 * Erklaerungstext zu `<typkey>.<feld>`; leer, wenn keiner hinterlegt ist.
 *
 * @param typeKey Knotentyp wie in `effectTypeKey()` (z. B. `"mirror"`).
 * @param field   Feldname wie in `nodeToJson()` (z. B. `"slower"`).
 */
[[nodiscard]] QString tooltip(const QString& typeKey, const QString& field);

/** Alle hinterlegten Schluessel als `<typkey>.<feld>` — fuer den Wachhund. */
[[nodiscard]] QStringList documentedKeys();

}  // namespace lumi::multieffect::fielddocs
