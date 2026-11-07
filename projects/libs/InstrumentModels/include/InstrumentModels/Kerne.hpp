/**
 * @file Kerne.hpp
 * @brief InstrumentModels: die eingebauten Kerne (Instrumente_IC_Konzept.md BK1 a, IK2-3 a)
 *
 * `InstrumentModels` traegt die Rechenkerne der Instrumentklassen ueber der Schnittstelle von `InstrumentCore`
 * (`Tonkern`, `Fabrik`, Tafel) — je Klasse ein Unterordner und Namensraum (`hammer/`, `lumi::kern::hammer`). Hier steht
 * die Fabrik, die sie alle kennt; die App und IC fragen sie beim Namen (`"core"` einer Schicht).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Tonkern.hpp"

namespace lumi::kern
{

/// Die eingebauten Kerne in der Reihenfolge ihrer Stufe: `hammerString` (34d); die Klassen aus IC folgen ab 34g.
[[nodiscard]] const Fabrik& kerne();

}  // namespace lumi::kern
