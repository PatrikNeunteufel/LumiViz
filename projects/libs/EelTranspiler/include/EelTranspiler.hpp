/**
 ****************************************************************************************
 * @file   EelTranspiler.hpp
 * @brief  Public API: EEL source -> Lua 5.4 source (one-shot import-time transpile)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Header-only library (no Qt, no app dependencies). Translates EEL scripts
 * (AVS Superscope slots, MilkDrop per_frame/per_vertex) into Lua source for
 * the LuaScriptEngine sandbox (eel-Prelude). Contract and dialect semantics:
 * projects/apps/MyViz/docs/visuals/Import_Analyse_AVS_MilkDrop.md §7 + §10.
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

    detail::CodeGen codegen(dialect, result.warnings);
    result.lua = codegen.generate(*program);
    result.ok = true;
    return result;
}

} // namespace lumi::eel
