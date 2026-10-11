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

#include "loop/Loop.hpp"

#include <QString>

#include <cstdint>

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

}  // namespace lumi::loop
