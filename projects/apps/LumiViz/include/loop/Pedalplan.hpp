/**
 * @file Pedalplan.hpp
 * @brief Loop: wie lange Sustain und Sostenuto eine Note halten
 *        (docs/audio/Spielweisen_Konzept.md, Stufe 28b; Entscheide Patrik S140:
 *        SK5 a, SK6 a, SF7 a)
 *
 * Die Pedale verlaengern eine Note ueber ihr Ende hinaus — so wird ihr Ton
 * laenger gehalten und klingt erst danach aus. Gerechnet wird je **Vorkommen**
 * einer Note in einem Clip, denn wie lange sie haelt, haengt an der Stelle im
 * Clip (SK5 a):
 *
 * - Die Spur wird wie das Muster gelesen, Durchlauf fuer Durchlauf. Vor dem
 *   ersten Punkt eines Durchlaufs ist das Pedal oben — an der Naht haelt es nur,
 *   wenn die Spur dort einen Punkt traegt, der gedrueckt ist.
 * - Am letzten Ende des Clips gilt das Pedal als losgelassen.
 * - Eine Synth-Schicht liest beide Pedale als Schalter (SK6 a): ab
 *   `kPedalSchwelle` gedrueckt. Den Andruck eines Daempfers rechnet erst ein
 *   Modell mit Damper (ab der Hammer-Saite).
 * - **Sustain** haelt eine Note, deren Ende bei gedruecktem Pedal liegt, bis es
 *   losgelassen wird. **Sostenuto** haelt nur die Noten, die beim Treten schon
 *   angeschlagen und noch nicht zu Ende sind (eine im selben Moment
 *   angeschlagene zaehlt mit), bis es losgelassen wird.
 * - Schlaegt dieselbe Tonhoehe im Clip neu an, endet der gehaltene Ton dort
 *   (von Claude gesetzt: eine Taste hat eine Saite — der neue Anschlag nimmt den
 *   gehaltenen Ton ab, wie im Rechenkern derselbe Ton sich selbst abschneidet).
 *
 * Ein Muster ohne Punkte auf den Pedalen bleibt, wie es war: seine Toene
 * heissen wie bis Stufe 27 (`tonKlang` mit `notenMs`). Kein Qt-Widget, keine
 * Audio-Engine.
 */
#pragma once

#include "loop/Loop.hpp"

#include <cstddef>

namespace lumi::loop
{

/// Ab hier gilt ein Pedal an einer Synth-Schicht als gedrueckt (SK6 a).
inline constexpr double kPedalSchwelle = 0.5;

/// Traegt das Muster einen Punkt auf Sustain oder Sostenuto?
[[nodiscard]] bool hatPedal(const Muster& muster);

/// Bis wohin die Note `note` in der Wiederholung `runde` eines Clips mit
/// `wiederholungen` Wiederholungen klingt, in Schritten ab dem Anfang des Clips:
/// ihr Ende — oder spaeter, wenn ein Pedal sie haelt. Nie hinter dem Ende des
/// Clips, nie hinter dem naechsten Anschlag derselben Tonhoehe darin.
[[nodiscard]] double gehaltenBis(const Loop& loop, const Muster& muster, int wiederholungen, int runde, std::size_t note);

/// Wie lange der Ton dieses Vorkommens gehalten wird, in Millisekunden: ohne
/// Pedal genau `Loop::notenMs`, sonst die Zeit vom Anschlag bis `gehaltenBis`.
[[nodiscard]] int gehaltenMs(const Loop& loop, const Muster& muster, int wiederholungen, int runde, std::size_t note);

}  // namespace lumi::loop
