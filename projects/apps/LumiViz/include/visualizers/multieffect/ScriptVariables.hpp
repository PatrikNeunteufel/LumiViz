/**
 ****************************************************************************************
 * @file   ScriptVariables.hpp
 * @brief  SSOT der Skriptvariablen je Effektknoten — was die Slots eines Knotens
 *         lesen und schreiben duerfen
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Bis S78 stand dieses Wissen an drei Stellen: in den Bindungslisten der
 * Laufzeit (`MultiEffectVisualizer::run*`), im Struct-Kommentar
 * (`EffectChain.hpp`) und in einer handgeschriebenen Referenz des Panels. Die
 * drei liefen auseinander — acht Knoten versprachen Variablen, die nie gebunden
 * waren, und rund vierzig Knoten zeigten im Panel gar keine Referenz.
 *
 * Diese Tabelle ist jetzt die Quelle fuer die Referenz im Panel. Gegen die
 * Laufzeit sichert sie der Test `test_ScriptVariablesGl.cpp`: er faehrt jeden
 * Knoten mit einem Skript einen Frame und prueft, dass jeder hier genannte Name
 * von der Laufzeit auch tatsaechlich gesetzt oder gelesen wird.
 *
 * Wer einem Knoten eine neue Skriptvariable gibt, traegt sie HIER ein (plus die
 * Bindung in `run*`); der Test meldet, wenn nur eine der beiden Seiten folgt.
 ****************************************************************************************
 */
#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <string>
#include <vector>

namespace lumi::multieffect {

/// Eine Zeile der Variablenreferenz.
struct ScriptVar
{
    /// Ein Name oder mehrere, getrennt durch " / " (z. B. "red / green / blue").
    const char* names;
    /// "in", "out", "in/out" — oder "own": Name gehoert dem Skript, die
    /// Laufzeit fasst ihn nicht an (wird vom Test nicht geprueft).
    const char* dir;
    const char* range;
    const char* meaning;
    /// false = nicht einzeln pruefbar (Bereichsnamen wie `reg00..reg99`,
    /// Funktionen wie `gmegabuf(i)`) — nur Doku.
    bool probe = true;
};

/// Welche gemeinsamen Zeilen die Referenz unter die knotenspezifischen haengt.
enum class ScriptVarBase
{
    None,         ///< nur die eigenen Zeilen
    Audio,        ///< + Audio-Satz (bass/mid/treb/vol/beat/time)
    ParamScript,  ///< + `b`, `w`, `h` + Audio-Satz (Parameter-Skript, Strang D)
};

struct ScriptVarTable
{
    ScriptVarBase base = ScriptVarBase::None;
    std::vector<ScriptVar> vars;
};

/// Die Tabelle des Knotentyps, oder nullptr, wenn der Typ keine Skript-Slots hat.
[[nodiscard]] const ScriptVarTable* scriptVariables(const EffectParams& params);

/// Die gemeinsamen Zeilen einer Basis (leer fuer `None`).
[[nodiscard]] const std::vector<ScriptVar>& scriptVarBaseRows(ScriptVarBase base);

/// Zerlegt `ScriptVar::names` in einzelne Bezeichner ("a / b" -> {"a","b"}).
[[nodiscard]] std::vector<std::string> splitScriptVarNames(const char* names);

}  // namespace lumi::multieffect
