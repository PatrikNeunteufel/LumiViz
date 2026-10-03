/**
 ****************************************************************************************
 * @file   TrailPresetStore.hpp
 * @brief  Trail-Presets der Karaoke-Ebene: die Trail-Liste samt Kindern als Datei
 *
 * @details
 * Session 92 (Wunsch Patrik): die Kette, die den Trail der Karaoke-Ebene formt,
 * laesst sich fuer sich als Preset halten — ohne den Lyrics-Knoten der Ebene
 * (Schrift, Farben, Lage bleiben, wenn man nur den Trail tauscht).
 *
 * Ein Trail-Preset ist **eine Effect List mit ihren Kindern**, der Lyrics Trail
 * am Ende eingeschlossen (Hold, Versatz, Mischart und Staerke gehoeren zum
 * Aussehen). Die Datei traegt das gewoehnliche Kettenformat (`saveChainToFile`,
 * Wurzel = die Liste) unter der Endung `.lvtrail` — so laesst sich dieselbe
 * Liste auch in eine Effektkette einfuegen oder aus einer herausnehmen
 * (`fuerKette` / `fuerEbene`).
 *
 * Die Knoten-Voreinstellungen (`NodePresetStore`) passen dafuer nicht: sie
 * tragen nur die Parameter EINES Knotens, keine Kinder.
 *
 * Ablage wie dort: mitgeliefert unter `asset/karaoke/trails/` (Aufwaertssuche
 * ab dem Programmordner, Anker `README.md`), eigene im Benutzerordner
 * `<AppData>/karaoke/trails/`. Bei gleichem Namen gewinnt der Benutzerordner;
 * mitgelieferte Dateien lassen sich in der App nicht ueberschreiben oder loeschen.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QString>
#include <QStringList>

#include <vector>

namespace lumi::multieffect::trailpresets {

using Entry = nodepresets::Entry;

/// Endung der Trail-Presets, mit Punkt.
inline constexpr const char* kEndung = ".lvtrail";

/// Alle Trail-Presets: erst die mitgelieferten, dann die eigenen, je nach
/// Namen sortiert. Ein eigenes mit dem Namen eines mitgelieferten verdraengt es.
[[nodiscard]] std::vector<Entry> list();

/// Eintrag mit diesem Namen (ohne Gross/Klein); leerer Pfad = gibt es nicht.
[[nodiscard]] Entry finde(const QString& name);

/**
 * Trail-Preset lesen. Die Liste kommt so zurueck, wie die Ebene sie braucht
 * (`fuerEbene`). false = unlesbar oder keine Effect List.
 */
[[nodiscard]] bool load(const QString& path, ChainNode& outListe, QStringList* report);

/// `liste` als eigenes Trail-Preset speichern (Name wird bereinigt, eine
/// gleichnamige eigene Datei ueberschrieben). false = ungueltiger Name,
/// keine Effect List oder Schreibfehler.
[[nodiscard]] bool save(const QString& name, const ChainNode& liste, QString* outPath);

/// Ein eigenes Trail-Preset loeschen. Mitgelieferte werden NICHT geloescht.
[[nodiscard]] bool remove(const Entry& entry);

/**
 * Die Liste, wie die Karaoke-Ebene sie fuehrt: mischt nicht in den Puffer
 * (In / Out: Ignore — aufgelegt wird beim Present), heisst „Karaoke Trail",
 * traegt genau EINEN Lyrics Trail, und der steht am Ende (fehlt er, kommt einer
 * mit den Vorgaben dazu). Ids sind geloescht, und die Liste steht an — „Trail
 * on" gehoert nicht zum Preset (`setzeKaraokeTrailListe` behaelt den Schalter).
 */
[[nodiscard]] ChainNode fuerEbene(ChainNode liste);

/**
 * Dieselbe Liste fuer eine Effektkette: Out = Additive (dort legt niemand
 * sie auf — sie muss selbst ins Bild mischen), Name = `name`. Ids geloescht.
 * In der Kette braucht sie einen Lyrics-Knoten, dem der Trail folgt.
 */
[[nodiscard]] ChainNode fuerKette(ChainNode liste, const QString& name);

/// Ist der Knoten eine Trail-Liste — eine Effect List, die (unmittelbar) einen
/// Lyrics Trail enthaelt? Daran erkennt der Kettenbaum sie und gibt ihr das
/// Icon der Trail-Presets.
[[nodiscard]] bool istTrailListe(const ChainNode& node);

/// Sind zwei Trail-Listen dasselbe Preset? Verglichen wird, was `fuerEbene`
/// aus ihnen macht — Ids, Name und Mischung der Liste zaehlen nicht.
[[nodiscard]] bool gleich(const ChainNode& a, const ChainNode& b);

[[nodiscard]] QString userDir();   ///< wird nicht angelegt
[[nodiscard]] QString assetDir();  ///< leer, wenn `asset/karaoke/trails` nicht gefunden wurde

/// Test-Haken wie `nodepresets::setRootForTesting`: Unterordner `user/` und
/// `asset/` unter `root`; leer = normales Verhalten.
void setRootForTesting(const QString& root);

}  // namespace lumi::multieffect::trailpresets
