/**
 ****************************************************************************************
 * @file   EelTranspiler.hpp
 * @brief  Public API: EEL source -> Lua 5.4 source (one-shot import-time transpile)
 *
 * @author LumiPulse Team
 * @date   July 2026 (1.3.0: August 2026 — EEL-Division als eel.div, S67;
 *         Versionshinker 1.0→1.2 der Header glattgezogen, Stand s. Doku)
 * @version 1.3.0
 *
 * @details
 * Header-only library (no Qt, no app dependencies). Translates EEL scripts
 * (AVS Superscope slots, MilkDrop per_frame/per_vertex) into Lua source for
 * the LuaScriptEngine sandbox (eel-Prelude). Contract and dialect semantics:
 * projects/apps/LumiViz/docs/visuals/Import_Analyse_AVS_MilkDrop.md §7 + §10.
 *
 * Error philosophy (Import-Analyse §4.3): a syntax error yields ok=false with
 * a positioned message — the caller treats the script slot as empty (AVS
 * behavior) and reports it; transpilation never throws.
 ****************************************************************************************
 */

#pragma once

#include "EelTranspilerCodeGen.hpp"
#include "EelTranspilerLexer.hpp"
#include "EelTranspilerParser.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace lumi::eel {

namespace detail {

/**
 * @brief Zuweisungen aufspueren, die AVS als TEILAUSDRUCK ablehnt (S74)
 *
 * In AVS-EEL ist eine Zuweisung nur als eigenstaendige Anweisung erlaubt —
 * nie als Funktionsargument, in Klammern oder im Rumpf von `loop`. Am
 * laufenden Paar gemessen: `AvsRef` uebersetzt ein solches Skript GAR NICHT,
 * und der betroffene Effekt rendert dort **nichts**. Wir uebersetzen es und
 * zeichnen — fehlerhaft ist unser Ergebnis nicht, aber es ist ein anderes.
 *
 * Fuer MilkDrop gilt das NICHT: ns-eel2 kennt `_set` als Operator, und
 * `while(exec2(q=q+1, below(q,5)))` liefert dort auf beiden Seiten dasselbe
 * (ebenfalls gemessen, S74). Die Pruefung laeuft deshalb nur im AVS-Dialekt.
 *
 * @param anweisung true, wenn @p node an einer Anweisungsstelle steht
 */
inline void findeUnzulaessigeZuweisungen(const Node& node, bool anweisung,
                                         std::vector<std::string>& warnungen)
{
    if (node.kind == NodeKind::Assign && !anweisung)
    {
        warnungen.push_back(
            "Zeile " + std::to_string(node.line) +
            ": Zuweisung innerhalb eines Ausdrucks (z. B. als Funktionsargument"
            " oder im Rumpf von loop). AVS laesst das NICHT zu — dort scheitert"
            " die Uebersetzung des ganzen Skripts, und der Effekt bleibt"
            " unsichtbar. LumiViz fuehrt ihn aus; das Bild weicht damit vom"
            " Original ab. Gleichstand erreicht man, indem man die Zuweisung"
            " als eigene Anweisung davorzieht — oder den Knoten abschaltet,"
            " denn genau das tut AVS.");
    }
    // Nur die Kinder einer Anweisungsliste stehen wieder an einer
    // Anweisungsstelle; alles andere (Argumente, Operanden, Klammern,
    // Zuweisungswert) ist Ausdruckstelle.
    const bool kindAnweisung = node.kind == NodeKind::StmtList;
    for (const NodePtr& kid : node.kids)
    {
        if (kid != nullptr) findeUnzulaessigeZuweisungen(*kid, kindAnweisung, warnungen);
    }
}

} // namespace detail

/**
 * @brief Result of a transpilation run
 */
struct TranspileResult
{
    bool ok = false;
    std::string lua;                     ///< generated Lua source (empty on failure)
    std::string error;                   ///< first fatal error with position ("" if ok)
    std::vector<std::string> warnings;   ///< non-fatal findings (aliasing, unknown fns)
};

/**
 * @brief Translate one EEL script into Lua source
 * @param source  EEL source (one slot: init/frame/beat/point bzw. per_frame/...)
 * @param dialect Source dialect (Avs: loop cap 4096 + 8-char aliasing warning;
 *                Milkdrop: loop cap 2^20)
 *
 * Empty/whitespace-only input yields ok=true with empty Lua source.
 */
inline TranspileResult transpile(std::string_view source, Dialect dialect)
{
    TranspileResult result;

    detail::Lexer lexer(source);
    std::vector<detail::Token> tokens;
    if (!lexer.tokenize(tokens))
    {
        result.error = lexer.error();
        return result;
    }

    detail::Parser parser(std::move(tokens));
    const detail::NodePtr program = parser.parseProgram();
    if (program == nullptr)
    {
        result.error = parser.error();
        return result;
    }

    // AVS lehnt Zuweisungen in Teilausdruecken ab (S74) — melden, bevor der
    // Codegen laeuft, damit die Zeile vor den Codegen-Warnungen steht.
    if (dialect == Dialect::Avs)
    {
        detail::findeUnzulaessigeZuweisungen(*program, true, result.warnings);
    }

    detail::CodeGen codegen(dialect, result.warnings);
    result.lua = codegen.generate(*program);
    result.ok = true;
    return result;
}

} // namespace lumi::eel
