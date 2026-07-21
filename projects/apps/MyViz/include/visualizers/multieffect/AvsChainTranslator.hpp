/**
 ****************************************************************************************
 * @file   AvsChainTranslator.hpp
 * @brief  Translate a parsed AVS effect tree into a host EffectChain (Roadmap 5.5)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 *
 * @details
 * Turns a `lumi::avs::ParseResult` (from the Qt-free AvsParser lib) into a
 * `lumi::multieffect::ChainNode` tree the MultiEffectVisualizer can render.
 * GL-free and dependency-light (App module per decision E7 — it maps parser
 * data to host runtime params, no OpenGL).
 *
 * Mapping rules:
 *  - Effect lists -> ChainNode(ListParams) with blend/OnBeat/EEL slots.
 *  - Decoded builtins of the import core set -> their EffectParams.
 *  - Set Render Mode (40) -> SetRenderModeParams, a live state-setter node: the
 *    host applies its line width/blend to the render effects that follow it at
 *    render time (no import-time unroll, no Passthrough).
 *  - Everything else (unknown, non-decoded, exotic) -> PassthroughParams + a
 *    path-prefixed report entry. Never throws (AVS philosophy).
 *
 * Color fields are converted from the AVS on-disk COLORREF (0x00BBGGRR) to the
 * host's 0x00RRGGBB.
 ****************************************************************************************
 */

#pragma once

#include "AvsParser.hpp"  // lumi::avs::ParseResult / EffectNode
#include "visualizers/multieffect/EffectChain.hpp"

#include <string>
#include <vector>

namespace lumi::multieffect {

struct TranslationResult
{
    ChainNode root;                    ///< compiled + ready for setChain()
    std::vector<std::string> report;   ///< parser warnings + unsupported effects
    int effectCount = 0;               ///< mapped nodes (root excluded)
    int passthroughCount = 0;          ///< nodes conserved as Passthrough
};

/**
 * @brief Translate a parsed AVS tree into a host effect chain.
 * @param parsed Result of lumi::avs::parse* (must be ok; otherwise an empty
 *               root list + an error report entry is returned).
 *
 * The returned root is already run through compileChain().
 */
[[nodiscard]] TranslationResult translateAvsTree(const lumi::avs::ParseResult& parsed);

} // namespace lumi::multieffect
