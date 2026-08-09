/**
 ****************************************************************************************
 * @file   NodePresetStore.hpp
 * @brief  Benannte Voreinstellungen je Knotentyp (Knoten-Parameter-Konzept, Etappe 1)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 *
 * @details
 * Eine Voreinstellung ist **die `params` eines Knotens unter einem Namen** — nicht
 * mehr. Sie traegt damit auch die EEL-Formeln (die liegen als `initCode`/`frameCode`/
 * `beatCode`/`pointCode` in eben diesen `params`), aber weder `children` noch
 * `displayName` (Entscheid Patrik S53, Konzept §8.2).
 *
 * Der ganze Mechanismus ist **generisch**: er laeuft ueber `effectTypeKey()` und
 * `nodeToJson`/`nodeFromJson` aus dem ChainSerializer und gilt damit fuer alle
 * Knotentypen — auch fuer jeden kuenftigen, ohne eine Zeile hier.
 *
 * Ablage (Datei je Voreinstellung, UTF-8-JSON):
 * | Ort | Zweck |
 * |---|---|
 * | `asset/nodepresets/<typkey>/<name>.json` | mitgeliefert, im Repo |
 * | `<AppDataLocation>/nodepresets/<typkey>/<name>.json` | selbst gespeichert |
 *
 * Bei Namensgleichheit gewinnt der Benutzer-Ordner; nur Benutzer-Dateien lassen sich
 * ueberschreiben und loeschen.
 *
 * GL-frei und panel-frei (Qt-JSON + QFile), damit der Roundtrip im Unit-Test laeuft.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <QString>
#include <QStringList>

#include <vector>

namespace lumi::multieffect::nodepresets {

/** Eine gefundene Voreinstellung. */
struct Entry
{
    QString name;        ///< Anzeigename (= Dateiname ohne .json)
    QString path;        ///< absoluter Pfad
    bool builtin{false}; ///< true = mitgeliefert (asset/), nicht ueberschreibbar
};

/**
 * Voreinstellungen eines Typs, nach Namen sortiert. Traegt der Benutzer-Ordner
 * denselben Namen wie das Asset, erscheint nur der Benutzer-Eintrag.
 */
[[nodiscard]] std::vector<Entry> list(const QString& typeKey);

/**
 * Laedt eine Voreinstellung **ueber** die vorhandenen Parameter.
 *
 * Die Datei ueberschreibt genau die Felder, die sie enthaelt — alles andere
 * bleibt stehen (Merge, nicht Ersatz). Das ist der Unterschied zwischen einem
 * *vollstaendigen* Preset (alle Felder in der Datei) und einem *Teil*-Preset:
 * eine SuperScope-**Figur** traegt nur die vier EEL-Slots und `pointCount`, also
 * bleiben Farbe, Linienbreite und Blend des Knotens erhalten — genau das
 * Verhalten des alten Figur-Dropdowns.
 *
 * @param path      Pfad aus `list()`
 * @param expectKey erwarteter Typ-Schluessel — eine Datei mit anderem Typ wird
 *                  ABGELEHNT (sonst wechselt ein Knoten beim Laden die Art)
 * @param inOut     Basis (die aktuellen Parameter des Knotens) UND Ziel;
 *                  nur bei Rueckgabe true veraendert
 * @param report    optionale Meldungen des Deserialisierers
 */
[[nodiscard]] bool load(const QString& path, const QString& expectKey,
                        EffectParams& inOut, QStringList* report);

/**
 * Die Parameter-Feldnamen eines Knotentyps, so wie sie in einer Datei stehen —
 * ohne den Knoten-Rahmen (`type`/`name`/`description`/`enabled`/`children`).
 * Generisch aus `nodeToJson` gewonnen, also fuer jeden Typ verfuegbar; Grundlage
 * der Feldauswahl beim Speichern.
 */
[[nodiscard]] QStringList fieldNames(const EffectParams& params);

/**
 * Speichert `params` als Benutzer-Voreinstellung. Der Typ-Schluessel wird aus
 * `params` abgeleitet, das Verzeichnis bei Bedarf angelegt; eine gleichnamige
 * Benutzer-Datei wird ueberschrieben.
 *
 * @param name    Anzeigename; wird fuer den Dateinamen bereinigt (s. sanitize)
 * @param params  die zu sichernden Parameter
 * @param fields  welche Felder in die Datei sollen (leer = alle). Weggelassene
 *                Felder bleiben beim Laden unangetastet — so entsteht ein
 *                **Teil**-Preset, etwa „nur die Formeln, nicht die Farbe".
 * @param outPath optional: der geschriebene Pfad
 */
[[nodiscard]] bool save(const QString& name, const EffectParams& params,
                        const QStringList& fields, QString* outPath);

/** Loescht eine Benutzer-Voreinstellung. Asset-Dateien werden NICHT geloescht. */
[[nodiscard]] bool remove(const Entry& entry);

/** Verzeichnis der Benutzer-Voreinstellungen eines Typs (wird nicht angelegt). */
[[nodiscard]] QString userDir(const QString& typeKey);

/**
 * Verzeichnis der mitgelieferten Voreinstellungen eines Typs — leer, wenn
 * `asset/nodepresets` nicht gefunden wurde (Aufwaertssuche ab dem Programmordner,
 * wie bei den Format-Icons).
 */
[[nodiscard]] QString assetDir(const QString& typeKey);

/**
 * Dateinamens-Bereinigung: alles ausser `[A-Za-z0-9 _-]` wird zu `_`, Rand-Leerzeichen
 * fallen weg. Ein leerer oder vollstaendig bereinigter Name ergibt "" (= ungueltig).
 */
[[nodiscard]] QString sanitize(const QString& name);

/**
 * Test-Haken: setzt die Wurzel der BEIDEN Verzeichnisse auf `root` (Unterordner
 * `user/` und `asset/`) und umgeht damit AppDataLocation und die Aufwaertssuche.
 * Leerer String stellt das normale Verhalten wieder her.
 */
void setRootForTesting(const QString& root);

} // namespace lumi::multieffect::nodepresets
