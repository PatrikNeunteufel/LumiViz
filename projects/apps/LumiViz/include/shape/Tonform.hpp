/**
 * @file Tonform.hpp
 * @brief Klang aus einer Form: eine Zeichnung so aufbereitet, dass eine Schicht
 *        sie spielen kann — eine Kontur, oder alle gewaehlten nacheinander in
 *        einem Umlauf (docs/audio/Shape_Konzept.md §5 und §9; Stufe 19c;
 *        Entscheide Patrik B2 d, S120, und T1 a, S121)
 *
 * **Eine Kontur:** um ihren Schwerpunkt gelegt (kein Gleichanteil im Ton) und
 * so gross gemacht, dass ihr **fernster Punkt den Abstand 1** zur Mitte hat
 * (T1 a) — dann bleibt die Spitze des Tons bei jedem Winkel bei hoechstens
 * rund 1.
 *
 * **Mehrere Konturen (As one path, B2 d):** jede behaelt ihre eigene Reihe und
 * ihre Lage in der Zeichnung; ein Umlauf des Stifts ist auf sie verteilt, nach
 * ihrer Laenge, dazwischen je ein kurzer gerader Sprung (`kSprungAnteil` des
 * Umlaufs). Gesprungen wird in Reihenfolge der Liste (B1, die Vorgabe): von der
 * ersten zur zweiten dort, wo sie sich am naechsten kommen; jede weitere wird
 * an dem Punkt betreten, der dem letzten am naechsten liegt; eine Kontur wird
 * an dem Punkt verlassen, an dem sie betreten wurde. Der Mittelwert des ganzen
 * Wegs liegt im Nullpunkt, der fernste Punkt hat den Abstand 1.
 * Die Arten „kuerzeste Wege" und „von Hand" kommen mit dem Fenster (Stufe 20d);
 * bis dahin spielt jede Zeichnung in Reihenfolge der Liste.
 *
 * Aufbereiten kostet (abtasten, Reihe bis zur 512. Harmonischen) — `tonformFuer`
 * merkt sich das Ergebnis je Nennung und Kontur.
 *
 * Kein Widget, keine Audio-Engine, keine Qt-Anwendung.
 */
#pragma once

#include "shape/Kontur.hpp"

#include <QString>

#include <memory>
#include <vector>

namespace lumi::shape
{

/// So viel vom Umlauf nimmt ein Sprung zwischen zwei Konturen (As one path).
inline constexpr double kSprungAnteil = 0.003;

/// Eine Kontur der Form, wie der Ton sie braucht.
struct TonKontur
{
    Reihe reihe;          ///< mit `mitte`: ihre Lage im gemeinsamen Bild
    double anteil = 1.0;  ///< so viel vom Umlauf gehoert ihr
    double start = 0.0;   ///< hier (0 … 1 auf ihrem Zug) wird sie betreten und verlassen
};

struct Tonform
{
    std::vector<TonKontur> konturen;
    double sprung = 0.0;  ///< Anteil eines Sprungs am Umlauf; 0 bei einer Kontur

    [[nodiscard]] bool leer() const { return konturen.empty(); }
};

/// Die Form zu einer Zeichnung. `kontur` = 0: jede Kontur mit Haken, in
/// Reihenfolge der Liste, als ein Pfad; sonst die eine mit dieser Kennung
/// (auch ohne Haken). Leer = keine solche Kontur.
[[nodiscard]] Tonform tonform(const Zeichnung& zeichnung, int kontur);

/// Der Ort des Stifts an der Stelle `umlauf` (0 … 1, laeuft um) mit
/// `harmonische` Teiltoenen je Kontur. `hoechste` ist die Grenze fuer eine
/// Kontur, der der ganze Umlauf gehoert: eine, die nur einen Teil bekommt, wird
/// entsprechend schneller durchlaufen und traegt anteilig weniger (mindestens
/// einen Teilton).
[[nodiscard]] Punkt tonwert(const Tonform& form, double umlauf, double harmonische, double hoechste = kHarmonischeMax);

/// Die Form, die eine Schicht nennt (`holeZeichnung`), aufbereitet und gemerkt.
/// nullptr = nicht gefunden, nicht lesbar oder ohne die genannte Kontur — auch
/// das wird gemerkt, bis `vergissTonformen` laeuft.
[[nodiscard]] std::shared_ptr<const Tonform> tonformFuer(const QString& nennung, int kontur);

/// Das Gemerkte verwerfen: eine Zeichnung ist geschrieben oder geloescht, oder
/// der Ordner der eigenen hat gewechselt. `Kontur.cpp` ruft es selbst.
void vergissTonformen();

}  // namespace lumi::shape
