/**
 * @file ComposerAbfragen.hpp
 * @brief Loop: was der Loop vom Composer wissen muss — als Schnittstelle, bei der
 *        sich der Composer anmeldet (S128, Entscheid Patrik N1 b)
 *
 * Der Composer steht über dem Loop (Entscheid Patrik S108, L4): führt er den
 * Ton, spielt das Panel Loop nicht. Seit S127 zeigt das Panel außerdem die
 * Stelle, an der der Composer den Loop spielt (W1 a, W2 a), und rollt mit der
 * Marke so, wie es für den Composer eingestellt ist (W3 a).
 *
 * Die Abhängigkeit läuft in einer Richtung: der Composer kennt den Loop, der
 * Loop kennt den Composer nicht. Er fragt über die drei Funktionen hier; der
 * Composer meldet beim Aufbau der App ein, was sie antworten
 * (`composer::meldeLoopAbfragenAn`). Ohne Anmeldung — etwa in einem Test ohne
 * Composer — gilt: der Composer führt nicht, spielt den Loop nicht, die Marke
 * rollt stetig.
 *
 * Nur im UI-Thread benutzt; angemeldet wird einmal, bevor ein Panel fragt.
 */
#pragma once

#include <QString>

#include <functional>

namespace lumi::loop
{

struct ComposerAbfragen
{
    /// Führt der Composer gerade den Ton?
    std::function<bool()> fuehrtTon;
    /// Die Stelle im Durchlauf des Loops `datei`, wie der Composer ihn spielt (ms); -1 = er spielt ihn nicht.
    std::function<double(const QString& datei)> loopStelleMs;
    /// Rollt eine Achse mit der Marke seitenweise statt stetig (die Einstellung des Composers)?
    std::function<bool()> mitlaufenSeitenweise;
};

/// Die Antworten des Composers anmelden; eine leere Funktion gilt als nicht angemeldet.
void setzeComposerAbfragen(ComposerAbfragen abfragen);

[[nodiscard]] bool composerFuehrtTon();
[[nodiscard]] double composerLoopStelleMs(const QString& datei);
[[nodiscard]] bool mitlaufenSeitenweise();

}  // namespace lumi::loop
