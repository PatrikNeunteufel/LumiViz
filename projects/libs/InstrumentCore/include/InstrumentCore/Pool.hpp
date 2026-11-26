/**
 * @file Pool.hpp
 * @brief InstrumentCore: unabhaengige Aufgaben auf Threads (`parallelJe`; Instrumente_IC_Konzept.md IK2-2 a, KN2 a)
 *
 * Aus IC `rechnung/gemeinsam/Werkzeug.hpp` (`bb7e6d6`); bis S174 stand eine eigene Fassung mit vertauschten
 * Parametern in den Tests (`Modellton.hpp`, `modelltest::parallelJe`). Jede Aufgabe schreibt nur ihr eigenes
 * Ergebnis, die Reihenfolge der Threads aendert nichts (IC AUFTRAG_BAU §8).
 *
 * **Abweichung von IC:** eine Ausnahme in einer Aufgabe beendet nicht das Programm (`std::terminate` aus dem Thread),
 * sondern die erste wird nach dem Ende aller Threads weitergeworfen; die uebrigen Aufgaben laufen zu Ende
 * (Uebernahme-Journal).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include <cstddef>
#include <functional>

namespace lumi::kern
{

/// So viele Threads rechnen hoechstens (LumiViz S156, IC L7: mehr werden am Rechner des Projekts langsamer —
/// Waermegrenze).
inline constexpr unsigned kPoolHoechstens = 11;

/// Seit 0.9.0 (S222): eine Grenze fuer den ganzen Prozess, 1 … `kPoolHoechstens` — ein Werkzeug setzt sie fuer lange
/// Laeufe (der Rechner des Projekts stuerzte unter einem Entwurf auf 11 Threads ab); sie gilt dann auch fuer eine Angabe.
/// 0 hebt sie auf (wie vor 0.9.0). Die Ergebnisse haengen nicht an der Zahl der Threads.
void setzePoolHoechstens(unsigned threads);
/// Die heutige Grenze (`kPoolHoechstens`, wenn keine gesetzt ist).
[[nodiscard]] unsigned poolHoechstens();

/// Die Zahl der Threads ohne Angabe: Zahl der Kerne, 1 … `poolHoechstens()`.
[[nodiscard]] unsigned poolThreads();

/// Rechnet `aufgabe(i)` fuer i = 0 … anzahl − 1 auf bis zu `threads` Threads (0 = `poolThreads()`; mit gesetzter Grenze
/// auch eine Angabe hoechstens sie), hoechstens so viele wie Aufgaben; mit einem Thread nacheinander auf dem rufenden. Jede
/// Aufgabe muss fuer sich rechnen.
void parallelJe(std::size_t anzahl, const std::function<void(std::size_t)>& aufgabe, unsigned threads = 0);

}  // namespace lumi::kern
