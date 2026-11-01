/**
 * @file Bounce.hpp
 * @brief Loop: *Bounce to wave clip* — ein Drum- oder Instrument-Clip wird zur Tondatei und zum
 *        Wave-Clip an seinem Ort (docs/audio/Loop_Konzept.md §4p; Entscheid Patrik WV7 a, Stufe 15k-2)
 *
 * Gerechnet wird wie *Export audio…* und Freeze (`rechneClip`, F3 a): alle Wiederholungen, mit
 * Effekten und Kurve am Clip, dahinter der Nachklang. Die Datei wird ein eigenes Sample (WV2 a);
 * das neue Wave-Muster ist so lang wie der Clip plus die Takte des Nachklangs — hoechstens bis zum
 * Anfang des naechsten Clips im Kanal —, Bezugstempo = Tempo des Loops, *Follow tempo* aus. Der Clip
 * wird **an seinem Ort ersetzt**: eine Wiederholung, keine Effekte, keine Expression (beides ist
 * eingerechnet). Das Wave-Muster merkt sich die **Herkunft** (der Ursprungsclip als kleiner Loop,
 * `clipDatei`); das urspruengliche Muster bleibt im Loop. Weicht das gleichnamige Muster von der
 * Herkunft ab (oder fehlt es), ist die **Quelle geaendert** (*source changed*); *Revert to source*
 * setzt den Ursprungsclip zurueck, *Bounce again* rechnet ihn neu.
 *
 * Hier nur, was den Loop aendert und prueft — ohne Datei, Ton und Oberflaeche.
 */
#pragma once

#include "loop/Bearbeiten.hpp"  // ClipOrt
#include "loop/Loop.hpp"
#include "loop/Wellenform.hpp"

#include <QString>

#include <optional>

namespace lumi::loop
{

/// Wird der Nachklang am naechsten Clip abgeschnitten, blendet das Wave-Muster so lange aus (von
/// Claude gesetzt): sonst knackte es dort.
inline constexpr double kBounceBlendeMs = 20.0;

/// Laesst sich der Clip an `ort` bouncen? Ein Drum- oder Instrument-Clip, dessen Muster es gibt.
[[nodiscard]] bool bounceMoeglich(const Loop& loop, ClipOrt ort);

/// So viele Takte bekommt das Wave-Muster: Clip plus Nachklang, aufgerundet; hoechstens bis zum
/// naechsten Clip im Kanal, mindestens die Laenge des Clips. `abgeschnitten` sagt, ob der
/// Nachklang dort endet.
[[nodiscard]] int bounceTakte(const Loop& loop, ClipOrt ort, const Wellenform& welle, bool* abgeschnitten = nullptr);

/// Den Clip an `ort` durch einen Wave-Clip ersetzen, der `nennung` (die gebouncte Datei, etwa
/// `user:<name>`) spielt; das Wave-Muster heisst wie das Muster mit „ bounce“ (frei gemacht).
/// Gibt den Namen des Wave-Musters zurueck; leer = nicht moeglich.
QString setzeBounceEin(Loop& loop, ClipOrt ort, const QString& nennung, const Wellenform& welle);

/// *Bounce again*: das Wave-Muster `wave` nennt die neue Datei, Laenge und Herkunft neu — aus
/// `quelle`, dem Loop, in dem an `ort` wieder der Ursprungsclip steht (`setzeQuelleZurueck`).
/// false = das gibt es nicht.
bool erneuereBounce(Loop& loop, const QString& wave, const Loop& quelle, ClipOrt ort, const QString& nennung,
                    const Wellenform& welle);

/// Hat das Wave-Muster eine Herkunft?
[[nodiscard]] bool hatQuelle(const Muster& wave);
/// Weicht das gleichnamige Muster im Loop von der Herkunft ab, oder fehlt es (*source changed*)?
[[nodiscard]] bool quelleGeaendert(const Loop& loop, const Muster& wave);

/// *Revert to source*: an `ort` (ein Wave-Clip mit Herkunft) wieder der Ursprungsclip, mit dem
/// jetzigen gleichnamigen Muster; fehlt es, kommt es aus der Herkunft zurueck. Das Wave-Muster
/// bleibt im Loop. false = kein Wave-Clip mit Herkunft, oder der Platz reicht nicht.
bool setzeQuelleZurueck(Loop& loop, ClipOrt ort);

}  // namespace lumi::loop
