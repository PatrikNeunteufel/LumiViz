/**
 * @file Austausch.hpp
 * @brief Loop: Muster, Clips und Spuren als Dateien — `.lvpattern`, `.lvclip`,
 *        `.lvtrack` (docs/audio/Loop_Konzept.md §4o, Stufe 33; Entscheide Patrik
 *        S167: PE1, PE1-1 a, PE2 a, PE3 b, LC1 a)
 *
 * Eigene Kreationen in anderen Loops wiederverwenden. Jede der drei Dateien traegt
 * einen **kleinen Loop** — geschrieben und gelesen wie eine `.lvloop` (`alsJson`,
 * `ausJson`), in einer Huelle mit eigenem Format:
 *
 * ```
 * {"format": "lumiviz-pattern" | "lumiviz-clip" | "lumiviz-track", "version": 1, "loop": {…}}
 * ```
 *
 * - `.lvpattern`: ein Muster, kein Kanal.
 * - `.lvclip`: ein Kanal mit einem Clip ab Takt 0 und sein Muster (LC1 a).
 * - `.lvtrack`: ein Kanal mit allen Clips an ihren Takten (PE3 b) und jedes Muster,
 *   das ein Clip nennt.
 *
 * Instrumente, Kits und Samples werden nur genannt, nicht mitgepackt. Raster und
 * Taktart stehen im kleinen Loop (`beatsPerBar`, `stepsPerBeat`) — der Import
 * rechnet danach um (PE2 a). Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/Loop.hpp"

#include <QString>
#include <QStringList>

#include <optional>

namespace lumi::loop
{

enum class AustauschArt : int
{
    Muster,  ///< `.lvpattern`
    Clip,    ///< `.lvclip`
    Spur,    ///< `.lvtrack`
};

inline constexpr int kAustauschFassung = 1;
inline constexpr const char* kMusterDateiEndung = ".lvpattern";
inline constexpr const char* kClipDateiEndung = ".lvclip";
inline constexpr const char* kSpurDateiEndung = ".lvtrack";

/// `.lvpattern`, `.lvclip`, `.lvtrack`.
[[nodiscard]] QString austauschEndung(AustauschArt art);
/// Die Art nach der Endung einer Datei; nichts = keine dieser drei.
[[nodiscard]] std::optional<AustauschArt> austauschArtVonDatei(const QString& datei);

/// Der Ordner im Benutzerordner, den die Dialoge vorschlagen (PE1): `patterns` fuer
/// `.lvpattern`, `tracks` fuer `.lvclip` und `.lvtrack`. Leer = kein Benutzerordner.
[[nodiscard]] QString eigenerAustauschOrdner(AustauschArt art);
/// In Tests: statt des Benutzerordners `basis` (darunter `patterns`, `tracks`); leer = zurueck.
void setzeAustauschBasisFuerTests(const QString& basis);

// --- herausloesen -------------------------------------------------------------
// Der kleine Loop traegt Tempo, Taktart, Raster und Swing des Loops, keine Tags;
// Lautstaerke und Limiter bleiben bei ihren Vorgaben (der Import uebernimmt sie nicht).

/// Das Muster `name` allein. nichts = es gibt kein Muster dieses Namens.
[[nodiscard]] std::optional<Loop> musterDatei(const Loop& loop, const QString& name);
/// Der Clip `clip` im Kanal `kanal` ab Takt 0, mit Effekten, *FX*, Expression und
/// Freeze-Abdruck, und sein Muster; der Kanal traegt nur seinen Namen.
/// nichts = kein solcher Clip oder sein Muster fehlt.
[[nodiscard]] std::optional<Loop> clipDatei(const Loop& loop, int kanal, int clip);
/// Der Kanal `kanal` (Name, an, Fader, alle Clips an ihren Takten) und jedes Muster,
/// das einer seiner Clips nennt, in der Reihenfolge des Loops. nichts = kein solcher Kanal.
[[nodiscard]] std::optional<Loop> spurDatei(const Loop& loop, int kanal);

// --- Datei --------------------------------------------------------------------

/// Schreibt den kleinen Loop in die Huelle der Art. Pfade eigener Dateien relativ
/// zur Datei, wenn sie darunter liegen — wie bei einer `.lvloop`.
bool speichereAustausch(AustauschArt art, const Loop& teil, const QString& pfad);
/// Liest eine Datei der Art `art`. false = keine solche Datei: anderes Format, eine
/// Fassung von morgen (`zuNeu`), kein Loop darin, oder nicht die Gestalt der Art
/// (`.lvpattern` ein Muster ohne Kanal, `.lvclip` ein Kanal mit einem Clip,
/// `.lvtrack` ein Kanal). Fehlt eine genannte Datei, steht das im `bericht`.
bool ladeAustausch(const QString& pfad, AustauschArt art, Loop& teil, QStringList* bericht = nullptr,
                   bool* zuNeu = nullptr);

// --- Umrechnung (PE2 a) ---------------------------------------------------------

/// Was die Umrechnung an einem Drum-Muster runden musste.
struct Umrechnung
{
    int verschoben = 0;      ///< Schlaege, die nicht genau auf einen Schritt fielen
    int zusammengelegt = 0;  ///< Schlaege, die auf einen Schritt mit einem anderen fielen (der lautere blieb)

    [[nodiscard]] bool operator==(const Umrechnung&) const = default;
};

/// Rechnet den kleinen Loop auf eine andere Taktart und ein anderes Raster um — nach
/// **Schlaegen** (PE2 a): die Zeit im Schlag bleibt.
/// - Noten: auf den Schritt an oder vor ihrem Anfang, den Rest traegt `versatzAnfang`;
///   die Dauer ebenso mit `versatzEnde` — zeitlich genau.
/// - Drum-Zeilen: jeder Schlag auf den naechsten Schritt (bei genau halbem Abstand
///   der fruehere); zwei auf einem werden einer, der lautere bleibt.
/// - Akkord-Eintraege auf den Schritt ihrer Noten, die Spuren ueber der Zeit genau
///   umgerechnet, die Expression am Clip (in Takten) ueber die Schlaege.
/// - Takte eines Musters aus seinen Schlaegen, aufgerundet; Start und Wiederholungen
///   der Clips bleiben (PE3 b).
/// Gleiche Taktart und gleiches Raster: nichts aendert sich, Bit fuer Bit.
Umrechnung rechneUm(Loop& teil, int schlaegeJeTakt, int schritteJeSchlag);

// --- Einsetzen ------------------------------------------------------------------

/// Was beim Einsetzen geschah — fuer die Meldung an der Oberflaeche.
struct Einsetzung
{
    Umrechnung umrechnung;
    bool umgerechnet = false;  ///< Taktart oder Raster wichen ab
    /// Je Muster, das unter anderem Namen steht, „alt → neu“ (gleicher Name, anderer Inhalt).
    QStringList umbenannt;
    int kanal = -1;            ///< der Kanal, in dem es nun steht
    bool neuerKanal = false;   ///< ein Clip passte nicht an seine Stelle (oder eine Spur)
};

/// Setzt den kleinen Loop einer Datei der Art `art` in `ziel` ein:
/// 1. umgerechnet auf Taktart und Raster von `ziel` (PE2 a);
/// 2. jedes Muster unter seinem Namen — steht dort eines mit gleichem Inhalt, wird es
///    mitbenutzt; mit anderem Inhalt der naechste freie Name `<name>_2`, `_3`, … (oder einer
///    davon mit gleichem Inhalt), wie *Keep both*;
/// 3. `.lvpattern`: ein Clip mit einer Wiederholung, `.lvclip`: der Clip — beide ab `takt` im
///    Kanal `kanal`; reicht dort der Platz bis zum naechsten Clip nicht (oder gibt es den Kanal
///    nicht), in einem neuen Kanal am Ende, benannt nach dem Muster bzw. dem Kanal der Datei.
///    `.lvtrack`: der Kanal am Ende, die Clips an ihren Takten (PE3 b); `kanal` und `takt`
///    zaehlen nicht.
/// Danach ist `ziel` gerichtet. Ein `teil` ohne die Gestalt seiner Art aendert nichts (kanal −1).
Einsetzung setzeEin(Loop& ziel, Loop teil, AustauschArt art, int kanal = -1, int takt = 0);

}  // namespace lumi::loop
