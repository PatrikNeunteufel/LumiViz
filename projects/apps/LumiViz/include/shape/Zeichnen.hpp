/**
 * @file Zeichnen.hpp
 * @brief Klang aus einer Form: was die Werkzeuge Pen und Move point im Fenster
 *        Edit shape mit einem Pfad tun (docs/audio/Shape_Konzept.md §8, §8a;
 *        Stufe 20a; Entscheid Patrik Z1 a, S122)
 *
 * **Pen:** ein Zug frei Hand wird geglaettet, auf wenige Ankerpunkte
 * vereinfacht und als Kurven durch diese Punkte angehaengt; ein Klick haengt
 * eine gerade Strecke an. Beides mischt sich in einem Pfad.
 *
 * **Move point:** die Ankerpunkte eines Pfads sind die Enden seiner Strecken
 * und Kurven; wird einer geschoben, gehen die Griffe der Kurven daneben mit, so
 * dass die Kurve ihre Gestalt behaelt.
 *
 * Kein Widget, keine Qt-Anwendung: alles in den Koordinaten der Zeichnung.
 */
#pragma once

#include <QPainterPath>
#include <QPointF>

#include <vector>

namespace lumi::shape
{

/// Einen Zug frei Hand anhaengen. Ist `pfad` leer, beginnt er am ersten Punkt
/// des Zugs; sonst fuehrt eine gerade Strecke vom letzten Punkt zum Anfang des
/// Zugs. Der Zug wird geglaettet und so vereinfacht, dass kein Punkt weiter als
/// `toleranz` vom Gezogenen abweicht; durch die uebrigen Punkte laufen Kurven.
/// Zurueck: so viele Ankerpunkte kamen dazu.
int haengeFreihandAn(QPainterPath& pfad, const std::vector<QPointF>& zug, double toleranz);

/// Eine gerade Strecke zu `punkt` anhaengen (ein Klick mit Pen); ein leerer
/// Pfad beginnt dort. Liegt der Punkt auf dem letzten, geschieht nichts.
/// Zurueck: true = ein Ankerpunkt kam dazu.
bool haengeEckeAn(QPainterPath& pfad, QPointF punkt);

/// Die Ankerpunkte eines Pfads, in seiner Reihenfolge — bei einem
/// geschlossenen Zug ohne den letzten, der auf dem ersten liegt.
[[nodiscard]] std::vector<QPointF> ankerpunkte(const QPainterPath& pfad);

/// Laesst sich daraus eine Kontur schliessen: mindestens drei Ankerpunkte,
/// oder zwei mit einer Kurve dazwischen?
[[nodiscard]] bool laesstSichSchliessen(const QPainterPath& pfad);

/// Der Ankerpunkt, der `punkt` am naechsten liegt, wenn nicht weiter als
/// `radius`; -1 = keiner.
[[nodiscard]] int naechsterAnker(const QPainterPath& pfad, QPointF punkt, double radius);

/// Den Ankerpunkt `anker` (wie `ankerpunkte` zaehlt) nach `ziel` schieben; die
/// Griffe der Kurven, die an ihm haengen, gehen um dasselbe Stueck mit. Bei
/// einem geschlossenen Zug geht der Anfang mit dem Ende. false = es gibt den
/// Punkt nicht; `pfad` bleibt.
bool schiebeAnker(QPainterPath& pfad, int anker, QPointF ziel);

}  // namespace lumi::shape
