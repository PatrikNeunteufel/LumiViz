/**
 * @file Offen.hpp
 * @brief Loop: der Loop, den das Loop-Panel gerade offen hat — fuer alle, die
 *        dieselbe Datei spielen (docs/audio/Loop_Konzept.md §4e, Stufe 7)
 *
 * Entscheid Patrik S108, L7 a: liegt der Loop als Clip in der Komposition, ist
 * jede Aenderung im Loop-Panel ueber den Composer zu hoeren. Dafuer meldet das
 * Panel hier seine Fassung — auch die ungespeicherte —, und die Tonfuehrung
 * des Composers fragt je Takt, ob es zu ihrer Datei eine neuere gibt.
 *
 * Eine Ablage fuer das ganze Programm, nur im UI-Thread benutzt: kein Qt-Objekt,
 * kein Ereignis-Bus, keine Sperre.
 */
#pragma once

#include "audio/AudioEvents.hpp"  // TrackInfo
#include "loop/Loop.hpp"

#include <QString>

#include <cstdint>
#include <functional>

namespace lumi::loop
{

/// Das Loop-Panel hat diese Datei mit dieser Fassung offen. Eine leere Datei
/// heisst: keine (nichts geladen, oder der Loop ist noch nie gespeichert).
void meldeOffen(const QString& datei, const Loop& loop);
/// Zaehlt jede Meldung — wer sie sich merkt, sieht billig, ob sich etwas getan hat.
[[nodiscard]] std::uint64_t offenFassung();
/// Die offene Fassung dieser Datei. false = das Panel hat sie nicht offen;
/// `loop` bleibt dann, wie er war.
bool offenerLoop(const QString& datei, Loop& loop);

// --- der Loop als laufender Song (S127, Entscheid Patrik 7a) -------------------

/// Der Loop als Song, wie Player-Panel, Text-Vorlagen und Karaoke ihn lesen
/// (Entscheid Patrik S109): die Tags, ohne Titel der Dateiname; die Laenge ein
/// Durchlauf. Fuer den Loop-Clip im Composer und das Panel Loop.
[[nodiscard]] TrackInfo loopAlsSong(const Loop& loop, const QString& datei, int rate);
/// Das Panel Loop spielt den ganzen Loop allein und meldet ihn als laufenden
/// Song (`song`), oder nicht mehr (nullptr). Das Player-Panel zeigt dann
/// „Loop - <Titel>“.
void meldeLaufendenLoop(const TrackInfo* song);
/// Der Loop, den das Panel Loop gerade als laufenden Song meldet; nullptr = keiner.
[[nodiscard]] const TrackInfo* laufenderLoop();

// --- vor dem Beenden der App --------------------------------------------------

/// Das Loop-Panel traegt hier ein, wie es vor dem Beenden nach Ungespeichertem
/// fragt (`wer` = das Panel selbst). Ein zweites Panel loest das erste ab.
void meldeBeendenFrage(const void* wer, std::function<bool(const QString& titel)> frage);
/// Das Panel geht: seine Frage gilt nicht mehr. Die eines anderen Panels bleibt.
void vergissBeendenFrage(const void* wer);
/// Darf die App beenden? true = es gibt kein Panel, nichts Ungespeichertes, oder
/// es ist gespeichert oder verworfen; false = bleiben. `titel` ist der Titel
/// der Rueckfrage.
[[nodiscard]] bool darfBeenden(const QString& titel);

}  // namespace lumi::loop
