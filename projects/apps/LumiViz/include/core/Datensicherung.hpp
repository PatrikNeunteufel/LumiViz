/**
 * @file Datensicherung.hpp
 * @brief Die eigenen Daten exportieren und importieren — die Regeln, ohne
 *        Fenster (docs/core-services/Datenablage_Konzept.md, Stufe A4;
 *        Entscheide Patrik S114: D5 a, D6 a, D7 a, E1 a)
 *
 * Eigenes mit Namen — Presets, Vorlagen, Trails, Blenden, Instrumente — liegt
 * im Benutzerordner der App, je Bereich in einem Unterordner. Der Export
 * schreibt gewaehlte Bereiche in EINE Datei `.lvdata` (ein Zip):
 *
 *     manifest.json            Format, Fassung, LumiViz-Version, Datum, Zahlen
 *     data/<Pfad im Benutzerordner>
 *     settings.json            nur wenn gewaehlt; nur was mitgehen darf
 *
 * Der Import liest eine solche Datei, sagt je Datei, ob es den Namen schon
 * gibt, und schreibt nach einer Regel. Er schreibt NUR in die Ordner der
 * bekannten Bereiche: ein Eintrag mit `..`, einem Laufwerk oder einem Pfad
 * ausserhalb wird nicht angefasst und gezaehlt (`Inhalt::ausgelassen`).
 *
 * Kein Qt-Widget. Die Einstellungen kommen als `QSettings` herein — im Test
 * eine Datei, im Programm die Registry.
 */
#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <functional>
#include <map>
#include <vector>

class QSettings;

namespace lumi::core::datensicherung
{

inline constexpr const char* kFormat = "lumiviz-data";
/// Die Fassung, die dieses Programm schreibt; eine hoehere liest es nicht.
inline constexpr int kFassung = 1;
inline constexpr const char* kEndung = ".lvdata";

/// Ein Bereich des Benutzerordners, wie er im Dialog als Zeile steht.
struct Bereich
{
    QString id;          ///< fest, steht im Manifest (`visuals`, `lyrics`)
    QString titel;       ///< wie er im Programm steht (Englisch)
    QStringList ordner;  ///< relativ zum Benutzerordner, mit allen Unterordnern
    QStringList ohne;    ///< Dateinamen, die nie mitgehen (laufender Stand, Zwischenspeicher)
    bool anfangsAn = true;  ///< im Dialog Export anfangs angehakt
};

/// Die Bereiche in der Reihenfolge des Dialogs.
[[nodiscard]] const std::vector<Bereich>& bereiche();
[[nodiscard]] const Bereich* findeBereich(const QString& id);

/// Der Benutzerordner der App (`AppDataLocation`); leer = Qt nennt keinen.
[[nodiscard]] QString benutzerOrdner();

struct Datei
{
    QString rel;  ///< relativ zum Benutzerordner, mit `/`
    qint64 groesse = 0;
    QDateTime geaendert;
};

/// Die Dateien eines Bereichs, nach Pfad sortiert. Fehlt der Ordner: leer.
[[nodiscard]] std::vector<Datei> sammle(const QString& benutzerOrdner, const Bereich& bereich);

// --- Einstellungen ----------------------------------------------------------

/// Geht dieser Schluessel in einen Export — und wird er aus einem Import
/// angenommen? Eine feste Liste (Datenablage_Konzept.md §4a): ein neuer
/// Schluessel geht erst mit, wenn er dort eingetragen ist. Fensterlage,
/// gemerkte Pfade, der Stand der letzten Sitzung und `shadertoy/apiKey`
/// gehen nie.
[[nodiscard]] bool einstellungGehtMit(const QString& schluessel);
/// Was von den Einstellungen mitgeht, als JSON.
[[nodiscard]] QJsonObject leseEinstellungen(QSettings& einstellungen);
/// Setzt aus `werte`, was mitgehen darf; alles andere bleibt unberuehrt,
/// auch wenn es in `werte` steht. Zurueck: so viele gesetzt.
int schreibeEinstellungen(QSettings& einstellungen, const QJsonObject& werte);

// --- Export -----------------------------------------------------------------

struct ExportWahl
{
    QStringList bereiche;        ///< die `id` der gewaehlten Bereiche
    bool einstellungen = false;  ///< `settings.json` dazu
};

struct ExportErgebnis
{
    bool ok = false;
    int dateien = 0;   ///< ohne Manifest und Einstellungen
    int bereiche = 0;  ///< Bereiche mit mindestens einer Datei
    QString fehler;
};

/// Schreibt `ziel` (ganz oder gar nicht). `einstellungen` nur, wenn die Wahl
/// sie verlangt (`leseEinstellungen`).
[[nodiscard]] ExportErgebnis exportiere(const QString& benutzerOrdner, const ExportWahl& wahl,
                                        const QJsonObject& einstellungen, const QString& ziel,
                                        const QString& appVersion);

// --- Import -----------------------------------------------------------------

enum class Zustand
{
    Neu,           ///< den Namen gibt es im Benutzerordner nicht
    Gleich,        ///< es gibt ihn, Byte fuer Byte gleich — zaehlt nicht als gleicher Name
    NameVergeben,  ///< es gibt ihn mit anderem Inhalt
};

struct ImportDatei
{
    QString rel;
    qint64 groesse = 0;
    QDateTime geaendert;
    Zustand zustand = Zustand::Neu;
    qint64 eigeneGroesse = 0;   ///< nur bei `NameVergeben`
    QDateTime eigeneGeaendert;  ///< nur bei `NameVergeben`
};

struct ImportBereich
{
    QString id;
    std::vector<ImportDatei> dateien;
    /// So viele Namen gibt es schon (mit anderem Inhalt).
    [[nodiscard]] int vergeben() const;
};

struct Inhalt
{
    bool ok = false;
    QString fehler;
    int fassung = 0;
    QString appVersion;
    QDateTime exportiert;
    /// Nur Bereiche, die in der Datei stecken, in der Reihenfolge von `bereiche()`.
    std::vector<ImportBereich> bereiche;
    bool hatEinstellungen = false;
    QJsonObject einstellungen;
    /// Eintraege, die in keinen bekannten Bereich gehoeren oder deren Pfad
    /// nicht in Ordnung ist — sie werden nie geschrieben.
    int ausgelassen = 0;
};

/// Eine `.lvdata` ansehen und gegen den Benutzerordner halten. Schreibt nichts.
[[nodiscard]] Inhalt oeffne(const QString& datei, const QString& benutzerOrdner);

/// Was mit einem Namen geschieht, den es schon gibt (D7 a).
enum class Regel
{
    MeineBehalten,  ///< die importierte Datei auslassen (Vorgabe)
    Ersetzen,       ///< die eigene ersetzen
    BeideBehalten,  ///< die importierte bekommt eine Zahl an den Namen
    JeDatei,        ///< nach `ImportWahl::jeDatei`
};

enum class Wahl
{
    Meine,
    Importierte,
    Beide,
};

struct ImportWahl
{
    QStringList bereiche;  ///< die `id` der gewaehlten Bereiche
    Regel regel = Regel::MeineBehalten;
    /// Nur bei `Regel::JeDatei`: Pfad (`rel`) -> Wahl. Was fehlt, behaelt die eigene.
    std::map<QString, Wahl> jeDatei;
};

struct ImportErgebnis
{
    bool ok = false;
    int eingelesen = 0;     ///< neue Dateien geschrieben
    int ersetzt = 0;        ///< eigene ersetzt
    int umbenannt = 0;      ///< unter neuem Namen dazugelegt
    int uebersprungen = 0;  ///< Name vergeben, die eigene blieb
    int gleich = 0;         ///< schon da, Byte fuer Byte gleich
    QString fehler;
};

/// Schreibt die gewaehlten Bereiche in den Benutzerordner. Die Einstellungen
/// setzt der Aufrufer selbst (`Inhalt::einstellungen`, `schreibeEinstellungen`).
[[nodiscard]] ImportErgebnis importiere(const QString& datei, const QString& benutzerOrdner, const ImportWahl& wahl);

/// Der Name, unter dem eine Datei neben einer gleichnamigen liegt:
/// `Neon.json` -> `Neon (2).json`, die erste Zahl, die `istVergeben` nicht kennt.
[[nodiscard]] QString nameMitZahl(const QString& rel, const std::function<bool(const QString&)>& istVergeben);

}  // namespace lumi::core::datensicherung
