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
 * und Muster, die Stelle des Clips im Loop. Gleiche Clips teilen so eine Datei.
 *
 * Die Klangfassung waechst mit jeder Aenderung am Rechenkern, die gerechnete Toene
 * aendert — sonst spielten alte Wellenformen still weiter (H3 b). Ein Test haelt
 * sie an `loop_werte.txt` fest: aendert sich diese Datei, ohne dass die Fassung
 * mitwaechst, wird er rot.
 */
#pragma once

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"

#include <QByteArray>
#include <QString>

#include <optional>

namespace lumi::loop
{

/// Die Klangfassung des Programms (F1 a). Hochzaehlen, wenn sich gerechnete Toene
/// aendern (dann auch `loop_werte.txt` neu) — und `werte/klangfassung.txt` neu
/// schreiben (`LUMIVIZ_KLANGFASSUNG_SCHREIBEN=1`).
inline constexpr int kKlangfassung = 1;

/// Der Fingerabdruck des Clips an `ort` fuer `rate`, als Hex (40 Zeichen); leer = den
/// Clip gibt es nicht. `saiten` wie beim Planen (`saitenAusInstrumenten`).
[[nodiscard]] QByteArray clipAbdruck(const Loop& loop, ClipOrt ort, int rate, const SaitenVon& saiten);

/// Was `clipAbdruck` rechnet, ohne die Schluessel der Klaenge: der Clip allein auf Takt 0,
/// ohne Tags und Namen (fuer Tests und zum Nachsehen). Leer = den Clip gibt es nicht.
[[nodiscard]] std::optional<Loop> abdruckFassung(const Loop& loop, ClipOrt ort);

}  // namespace lumi::loop
