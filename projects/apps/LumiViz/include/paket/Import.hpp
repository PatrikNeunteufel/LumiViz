/**
 * @file Import.hpp
 * @brief Publish, Stufe P-A: ein Paket `.lvdata` Fassung 2 ansehen und
 *        einlesen (docs/core-services/Publish_Konzept.md §3; Entscheide P3 a,
 *        P5 a, P6 a, Patrik S125)
 *
 * Jede Zugehoerige kommt in den Benutzerordner ihrer Art, das Hauptding auch —
 * ausser einem Loop, der dorthin kommt, wo der Empfaenger ihn ablegt. Gibt es
 * einen Namen schon mit anderem Inhalt, entscheidet je Datei die Wahl (P6 a):
 * **Keep both** (Vorgabe) — die aus dem Paket heisst `<name>_2`, und jede
 * Nennung im Paket wird mitgeaendert; **Replace mine**; **Use mine** — das
 * Paket nennt dann die eigene. Was Byte fuer Byte gleich ist, wird nicht
 * geschrieben.
 *
 * Gelesen wird nur, was das Manifest nennt, und nur unter Pfaden, die zu Art
 * und Namen passen; alles andere im Archiv bleibt liegen. Kein Qt-Widget.
 */
#pragma once

#include "core/Datensicherung.hpp"
#include "paket/Sammler.hpp"

#include <QDateTime>
#include <QString>
#include <QStringList>

#include <map>
#include <optional>
#include <vector>

namespace lumi::paket
{

/// Die Art zu ihrem Text im Manifest (`artText`); leer = unbekannt.
[[nodiscard]] std::optional<Art> artAusText(const QString& text);

/// Eine Datei im Paket, gehalten gegen den Benutzerordner.
struct PaketDatei
{
    Art art = Art::Sample;
    QString pfad;  ///< im Paket (`data/samples/kick.wav`)
    QString name;  ///< wie sie im Paket heisst (`kick`)
    bool hauptding = false;
    bool eigenBeimAbsender = true;  ///< `origin` im Manifest: `own` oder `elsewhere`
    QStringList genanntVon;
    qint64 groesse = 0;
    QDateTime geaendert;
    lumi::core::datensicherung::Zustand zustand = lumi::core::datensicherung::Zustand::Neu;
    QString eigeneDatei;  ///< nur bei `Gleich` und `NameVergeben`: die Datei des Empfaengers
    qint64 eigeneGroesse = 0;
    QDateTime eigeneGeaendert;
};

struct PaketInhalt
{
    bool ok = false;
    QString fehler;
    QString appVersion;
    QDateTime exportiert;
    Art art = Art::Loop;
    QString titel;
    QString name;       ///< der Name des Hauptdings; leer bei einem Loop
    QString hauptPfad;  ///< im Paket
    /// Das Hauptding (wenn seine Art einen Ordner hat) und die Zugehoerigen,
    /// in der Reihenfolge des Manifests.
    std::vector<PaketDatei> dateien;
    std::vector<Nennung> mitgeliefert;
    /// Mitgeliefertes, das es beim Empfaenger nicht gibt (eine aeltere Fassung).
    std::vector<Nennung> mitgeliefertFehltHier;
    /// Was schon beim Absender fehlte.
    std::vector<Fehlend> fehlt;

    /// Braucht das Hauptding einen Ablageort (`PaketWahl::ziel`)? Ein Loop ja.
    [[nodiscard]] bool brauchtZiel() const { return art == Art::Loop; }
    /// Der Dateiname des Hauptdings, wie der Absender ihn hatte (`titan.lvloop`).
    [[nodiscard]] QString dateiname() const;
    /// So viele Namen gibt es schon mit anderem Inhalt.
    [[nodiscard]] int vergeben() const;
};

/// Ist `datei` ein Paket (Fassung 2 mit dem Block `package`)? Liest nur das Manifest.
[[nodiscard]] bool istPaket(const QString& datei);

/// Ein Paket ansehen und gegen den Benutzerordner halten. Schreibt nichts.
[[nodiscard]] PaketInhalt oeffnePaket(const QString& datei);

struct PaketWahl
{
    /// Pfad im Paket -> Wahl; was fehlt, ist **Keep both** (`Wahl::Beide`, P6 a).
    std::map<QString, lumi::core::datensicherung::Wahl> jeDatei;
    /// Wohin das Hauptding kommt, wenn `brauchtZiel()`: der ganze Pfad.
    QString ziel;
};

struct PaketErgebnis
{
    bool ok = false;
    QString fehler;
    int neu = 0;             ///< neue Namen geschrieben
    int ersetzt = 0;         ///< eigene ersetzt (Replace mine)
    int umbenannt = 0;       ///< unter neuem Namen dazugelegt (Keep both)
    int eigeneGenommen = 0;  ///< die eigene genommen (Use mine)
    int gleich = 0;          ///< schon da, Byte fuer Byte gleich
    /// Je umbenannte Datei „bass → bass_2", fuer die Meldung.
    QStringList umbenennungen;
    /// Wo das Hauptding jetzt liegt.
    QString hauptDatei;
};

/// Liest das Paket ein.
[[nodiscard]] PaketErgebnis importierePaket(const QString& datei, const PaketWahl& wahl);

/// Der Name, den **Keep both** einer Datei gibt: `<name>_2`, `_3` … — der
/// erste, den es weder beim Empfaenger noch im Paket gibt. Fuer den Dialog.
[[nodiscard]] QString keepBothName(const PaketInhalt& inhalt, const PaketDatei& datei);
/// Die Bytes einer Datei im Paket (`pfad` wie in `PaketDatei`); leer = keine.
[[nodiscard]] QByteArray paketDaten(const QString& paket, const QString& pfad);

}  // namespace lumi::paket
