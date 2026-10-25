/**
 * @file Freeze.hpp
 * @brief Loop: Freeze — der Fingerabdruck eines Clips und die Klangfassung
 *        (docs/audio/Spielweisen_Konzept.md §4.3/§4.4, Stufe 30d; Entscheide F1 a,
 *        F3 a, WX5 a, FZ1 a)
 *
 * Eine eingefrorene Wellenform heisst wie der Fingerabdruck ihres Clips: aendert
 * sich etwas, das in den Clip eingeht, entsteht ein neuer Name — ueberschrieben
 * wird nie, und ein Undo findet die alte Datei wieder (F1 a). In den Abdruck geht:
 * der Clip mit seinem Muster, seinen Effekten und seiner Kurve, Tempo, Swing und
 * Raster des Loops, die Rate, je Klang sein Inhalt (das Instrument mit seinen
 * Sample-Dateien und Zeichnungen, die Klangdatei) und die **Klangfassung** des
 * Programms. Jeder Verweis auf eine Datei geht ueber ihren **Inhalt** ein, nicht
 * ueber Ort oder Zeit (FZ2 a) — so passt ein Abdruck auch auf einem anderen
 * Rechner und nach einem Auschecken; der Inhalts-Hash einer Datei wird zu Ort,
 * Groesse und Aenderungszeit gemerkt und nur bei einer Aenderung neu gelesen. Nicht hinein
 * gehen, was live bleibt — Fader und Schalter des Kanals, die Lautstaerke des
 * Loops, der Limiter (F3 a, WX5 a) —, und was nicht klingt: Tags, Namen von Kanal
 * und Muster, die Stelle des Clips im Loop, seine Marke `frozen` (30f). Gleiche Clips
 * teilen so eine Datei.
 *
 * Die Klangfassung waechst mit jeder Aenderung am Rechenkern, die gerechnete Toene
 * aendert — sonst spielten alte Wellenformen still weiter (H3 b). Ein Test haelt
 * sie an `loop_werte.txt` fest: aendert sich diese Datei, ohne dass die Fassung
 * mitwaechst, wird er rot.
 */
#pragma once

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Auskunft.hpp"  // InstrumentVon (HT5 a); bringt Saiten.hpp mit

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <cstdint>
#include <optional>
#include <vector>

namespace lumi::loop
{

/// Die Klangfassung des Programms (F1 a). Hochzaehlen, wenn sich gerechnete Toene
/// aendern (dann auch `loop_werte.txt` neu) — und `werte/klangfassung.txt` neu
/// schreiben (`LUMIVIZ_KLANGFASSUNG_SCHREIBEN=1`).
/// 2 (S153, 31d-7, P7 a): die Hammer-Saite rechnet in zwei Ebenen, die Laengskraft je Saite.
inline constexpr int kKlangfassung = 2;

/// Der Fingerabdruck des Clips an `ort` fuer `rate`, als Hex (40 Zeichen); leer = den
/// Clip gibt es nicht. `saiten` wie beim Planen (`instrumentAuskunft`).
[[nodiscard]] QByteArray clipAbdruck(const Loop& loop, ClipOrt ort, int rate, const InstrumentVon& saiten);

/// Was `clipAbdruck` rechnet, ohne die Schluessel der Klaenge: der Clip allein auf Takt 0,
/// ohne Tags und Namen (fuer Tests und zum Nachsehen). Leer = den Clip gibt es nicht.
[[nodiscard]] std::optional<Loop> abdruckFassung(const Loop& loop, ClipOrt ort);

// --- die Dateien (Stufe 30e; Entscheide F5 a, FZ3 a) ---------------------------------
//
// Je Abdruck eine WAV mit 32-bit float im Ordner `frozen` des Benutzerbereichs, mit dem
// Abdruck als Namen. Ein Speicher, kein Werk: jederzeit neu zu rechnen, nicht in der
// Datensicherung. Ist die Grenze erreicht, gehen die am laengsten nicht gespielten — die
// Aenderungszeit einer Datei ist ihr letzter Gebrauch (`holeFrozen` setzt sie). Nur Dateien,
// die wie ein Abdruck heissen (40 Hex-Zeichen, `.wav`), gehoeren dazu.

/// Was in den Settings fuer die Grenze einstellbar ist (FZ3 a): 0,25 bis 256 GB, Vorgabe 2 GB.
inline constexpr int kFrozenVorgabeMb = 2048;
inline constexpr int kFrozenMinMb = 256;
inline constexpr int kFrozenMaxMb = 256 * 1024;

/// Der Ordner `frozen` im Benutzerbereich; leer = keiner.
[[nodiscard]] QString frozenOrdner();
/// Nur fuer Tests: ein Wegwerf-Ordner statt des Benutzerbereichs; leer = wieder der echte.
void setzeFrozenOrdnerFuerTests(const QString& ordner);
/// Die Datei zum Abdruck (ob es sie gibt oder nicht); leer = kein Ordner oder kein Abdruck.
[[nodiscard]] QString frozenDatei(const QByteArray& abdruck);
/// Eine Wellenform ablegen (32-bit float), danach gilt die Grenze. false = nicht geschrieben.
bool legeFrozen(const QByteArray& abdruck, const std::vector<float>& stereo, int rate);
/// Die Wellenform zum Abdruck lesen — sie zaehlt dann als gerade gespielt. Leer = keine da,
/// oder sie hat eine andere Rate.
[[nodiscard]] std::optional<std::vector<float>> holeFrozen(const QByteArray& abdruck, int rate);

/// Was `legeFrozenDaten` tat.
enum class FrozenAblage : int
{
    Gelegt,     ///< die Datei liegt jetzt unter dem Abdruck
    SchonDa,    ///< eine lesbare lag schon da — sie bleibt (FP3 a)
    Ungueltig,  ///< kein Abdruck, keine lesbare WAV oder eine andere Rate — nichts geschrieben
};
/// Eine fertige Wellenform als Bytes einer WAV ablegen (Publish, Stufe 30i, FP3 a): nur eine,
/// die sich lesen laesst und `rate` traegt; eine vorhandene Datei bleibt. Danach gilt die Grenze.
[[nodiscard]] FrozenAblage legeFrozenDaten(const QByteArray& abdruck, const QByteArray& wav, int rate);

struct FrozenStand
{
    int dateien = 0;
    qint64 bytes = 0;
};
[[nodiscard]] FrozenStand frozenStand();
/// Alle eingefrorenen Wellenformen loeschen; zurueck die Zahl der geloeschten.
int leereFrozen();
/// Die Grenze halten: die am laengsten nicht gespielten gehen, bis alles darunter liegt —
/// auch die zuletzt gelegte, wenn sie allein darueber liegt. Ohne Angabe die aus den Settings.
void halteFrozenGrenze();
void halteFrozenGrenze(qint64 grenzeBytes);
/// Die Grenze aus den Settings (`loop/frozenMB`), ohne Eintrag die Vorgabe; geklemmt.
[[nodiscard]] int gemerkteFrozenGrenzeMb();
/// Merken (geklemmt; die Vorgabe wird nicht eingetragen) und gleich halten.
void merkeFrozenGrenzeMb(int mb);

// --- abspielen (Stufe 30f; Entscheide F3 a, FV1 a, FV2 a) ------------------------------

/// Welche Clips jetzt ihre Wellenform spielen: jeder mit Marke `frozen`, dessen **jetziger**
/// Abdruck als Datei vorliegt (FV2 a) — ob er dem gespeicherten gleicht oder nicht; die
/// Datei zaehlt dann als gerade gebraucht. Die anderen spielen live. Ein Abdruck, der sich
/// nicht lesen liess (`vergissFrozen`), zaehlt nicht mehr, bis er neu abgelegt wird.
[[nodiscard]] Eingefroren eingefrorenVon(const Loop& loop, int rate, const InstrumentVon& saiten);
/// Waechst, sobald eine Wellenform abgelegt, vergessen oder geraeumt wird (Stufe 30g): wer
/// eingefrorene Clips spielt, plant dann neu — `eingefrorenVon` kann anders antworten, auch
/// wenn sich der Loop nicht geaendert hat. Von jedem Thread aus.
[[nodiscard]] std::uint64_t frozenFassung();
/// Eine Wellenform, die sich nicht lesen liess: die Datei geht (ein Speicher, kein Werk —
/// F5 a), und bis `legeFrozen` sie neu ablegt, spielt ihr Clip live. Von jedem Thread aus.
void vergissFrozen(const QByteArray& abdruck);
/// Fehlt unter `klaenge` (je Name einer) eine eingefrorene Wellenform aus `namen` — liess
/// sie sich nicht laden? Dann ist neu zu planen: `eingefrorenVon` nennt ihren Clip nicht mehr.
[[nodiscard]] bool frozenFehlt(const QStringList& namen, const Klaenge& klaenge);

// --- bedienen (Stufe 30g; Entscheide F2 a, F5 a, FV2 a, FZ4 a) -------------------------

/// Wie ein Clip zu Freeze steht. Das Rechnen („computing…“) weiss nur, wer den Auftrag haelt.
enum class FrozenZustand
{
    Aus,        ///< keine Marke: live
    Frozen,     ///< die Wellenform zum jetzigen Abdruck liegt vor
    Veraendert, ///< seit dem Einfrieren veraendert — der jetzige Abdruck ist nicht der gespeicherte („frozen · changed“), live
    Fehlt,      ///< unveraendert, aber die Datei fehlt (geraeumt, unlesbar, noch nie gerechnet) — neu zu rechnen (F5 a)
};

[[nodiscard]] FrozenZustand frozenZustand(const Loop& loop, ClipOrt ort, int rate, const InstrumentVon& saiten);

/// Die Marken der Clips an `orte` auf ihren jetzigen Abdruck setzen (*Freeze clip*, FZ4 a).
/// Zurueck die Clips, deren Wellenform dazu fehlt — sie sind zu rechnen.
std::vector<ClipOrt> setzeFrozenMarken(Loop& loop, const std::vector<ClipOrt>& orte, int rate, const InstrumentVon& saiten);
/// Beim Speichern (F2 a): die Marken aller eingefrorenen Clips auf ihren jetzigen Abdruck.
/// Zurueck die Clips, deren Wellenform dazu fehlt.
std::vector<ClipOrt> frischeFrozenMarken(Loop& loop, int rate, const InstrumentVon& saiten);
/// Die Marken der Clips an `orte` wegnehmen (*Unfreeze clip*); die Dateien bleiben im Speicher.
void loeseFrozenMarken(Loop& loop, const std::vector<ClipOrt>& orte);
/// Die eingefrorenen Clips, deren Datei fehlt, obwohl sie unveraendert sind (`FrozenZustand::Fehlt`).
[[nodiscard]] std::vector<ClipOrt> fehlendeFrozen(const Loop& loop, int rate, const InstrumentVon& saiten);

// --- messen (Stufe 30h; Entscheide F4 a, FZ5 a bis FZ7 a) ------------------------------

/// Die Rechenzeit eines Clips (FZ6 a): die Summe der gemessenen Zeiten seiner Klaenge aus dem
/// Tonspeicher, in ms — die Arbeit auf einem Kern. Leer = den Clip gibt es nicht, oder einer
/// seiner Klaenge liegt nicht (mehr) im Speicher. Laedt und rechnet nichts.
[[nodiscard]] std::optional<double> clipRechenzeitMs(const Loop& loop, ClipOrt ort, int rate, const InstrumentVon& saiten);

/// Welche Zahl die Sanduhr zeigt (FZ6 a mit Zusatz, FZ7 a).
enum class FreezeHinweisArt
{
    Arbeit,  ///< *computing work*: die Summe (Vorgabe)
    Warten,  ///< *waiting time*: die Summe geteilt durch die Threads, die rechnen
};

/// Die Schwelle in den Settings (FZ5 a D): 0,1 bis 60 s, Vorgabe 1 s.
inline constexpr double kFreezeHinweisVorgabeS = 1.0;
inline constexpr double kFreezeHinweisMinS = 0.1;
inline constexpr double kFreezeHinweisMaxS = 60.0;

[[nodiscard]] double gemerkterFreezeHinweisS();  ///< `loop/freezeHintS`, geklemmt
void merkeFreezeHinweisS(double sekunden);       ///< geklemmt; die Vorgabe wird nicht eingetragen
[[nodiscard]] FreezeHinweisArt gemerkteFreezeHinweisArt();  ///< `loop/freezeHintWaiting`
void merkeFreezeHinweisArt(FreezeHinweisArt art);

}  // namespace lumi::loop
