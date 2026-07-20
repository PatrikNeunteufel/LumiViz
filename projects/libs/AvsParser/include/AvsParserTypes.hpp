/**
 ****************************************************************************************
 * @file   AvsParserTypes.hpp
 * @brief  Data model for parsed .avs presets (effect tree + import report)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Header-only library (no Qt, no app dependencies). The parse result mirrors the
 * on-disk structure of "Nullsoft AVS Preset 0.1/0.2" files: a root effect list
 * with nested effect nodes. Every node keeps its raw config blob; nodes of the
 * core effect set are additionally decoded into named int fields, color tables
 * and EEL code slots (EEL stays source text here — transpilation to Lua is the
 * EelTranspiler's job, translation into LumiViz presets happens in Roadmap 5).
 *
 * Reference for all layouts: ref/vis_avs (BSD-3, Nullsoft 2005) — see
 * projects/apps/MyViz/docs/visuals/Import_Analyse_AVS_MilkDrop.md §5.
 ****************************************************************************************
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::avs {

/// Effect-ID of a nested effect list in the file format (GET_INT reads it as -2)
inline constexpr std::int32_t kListId = -2;
/// IDs >= this are APE effects: a 32-byte ID string follows in the file
inline constexpr std::int32_t kApeIdBase = 16384;
/// Number of builtin effects (registration order = preset ID, ref rlib.cpp)
inline constexpr int kBuiltinCount = 46;

/**
 * @brief One named int32 config field, in file order
 */
struct IntField
{
    std::string name;
    std::int32_t value = 0;
};

/**
 * @brief One EEL code slot (source text, NOT transpiled)
 */
struct CodeSlot
{
    std::string name;   ///< "point"/"frame"/"beat"/"init"/"level"
    std::string code;   ///< EEL source ("" if empty)
};

/**
 * @brief Config of an effect list node (root or nested "Effect List")
 *
 * Bit layout of @ref mode (ref r_list.h): bit 0 = clear every frame,
 * bit 1 = disabled, bits 8-12 = input blend, bits 16-20 = output blend XOR 1,
 * bits 24-31 = extended data size (file bookkeeping only).
 */
struct ListInfo
{
    std::int32_t mode = 0;
    // extended data (AVS 2.8+, absent in old files -> defaults)
    std::int32_t inBlendVal = 0;
    std::int32_t outBlendVal = 0;
    std::int32_t bufferIn = 0;
    std::int32_t bufferOut = 0;
    std::int32_t inInvert = 0;
    std::int32_t outInvert = 0;
    std::int32_t beatRender = 0;
    std::int32_t beatRenderFrames = 0;
    // EEL slot pair from the "AVS 2.8+ Effect List Config" pseudo entry
    std::int32_t useCode = 0;
    std::string initCode;    ///< effect_exp[0] — executed once
    std::string frameCode;   ///< effect_exp[1] — executed per frame

    [[nodiscard]] bool clearEveryFrame() const { return (mode & 1) != 0; }
    [[nodiscard]] bool enabled() const { return (mode & 2) == 0; }
    [[nodiscard]] int blendIn() const { return (mode >> 8) & 31; }
    [[nodiscard]] int blendOut() const { return ((mode >> 16) & 31) ^ 1; }
};

/**
 * @brief One node of the effect tree
 *
 * Invariants: @ref rawConfig always holds the untouched blob (fidelity for the
 * later translation step and for unknown effects). @ref decoded is true only
 * when a layout decoder ran; unknown/undecoded effects still carry id/apeId/
 * name and the raw blob (the AVS "unknown effect = passthrough" philosophy).
 */
struct EffectNode
{
    std::int32_t id = 0;              ///< builtin index, kListId, or kApeIdBase for APEs
    std::string apeId;                ///< APE ID string from the file ("" for builtins)
    std::string name;                 ///< resolved display name ("" if unknown)
    std::vector<std::uint8_t> rawConfig;

    bool decoded = false;
    std::vector<IntField> fields;         ///< named int32 fields in file order
    std::vector<std::uint32_t> colors;    ///< color table (0x00RRGGBB), e.g. SuperScope
    std::vector<CodeSlot> code;           ///< EEL slots in file order

    bool isList = false;
    ListInfo list;                        ///< valid when isList
    std::vector<EffectNode> children;     ///< valid when isList

    /// @brief Lookup of a decoded field by name (0 if absent — EEL-like default)
    [[nodiscard]] std::int32_t field(std::string_view fieldName) const
    {
        for (const IntField& f : fields)
        {
            if (f.name == fieldName) return f.value;
        }
        return 0;
    }

    /// @brief Lookup of a code slot by name ("" if absent)
    [[nodiscard]] std::string_view slot(std::string_view slotName) const
    {
        for (const CodeSlot& s : code)
        {
            if (s.name == slotName) return s.code;
        }
        return {};
    }
};

/**
 * @brief Result of parsing one .avs file
 *
 * ok=false only when the data is not an AVS preset at all (bad signature /
 * too short). Everything after a valid signature is best effort: structural
 * problems truncate parsing at that point and land in @ref warnings —
 * never a hard failure (Import-Analyse §4.3).
 */
struct ParseResult
{
    bool ok = false;
    std::string error;                  ///< set when !ok
    int formatVersion = 0;              ///< 1 or 2 ("Nullsoft AVS Preset 0.<v>")
    EffectNode root;                    ///< isList=true, name "Main"
    std::vector<std::string> warnings;  ///< import report findings (path-prefixed)

    /// @brief Total number of effect nodes (excluding the root list itself)
    [[nodiscard]] int effectCount() const { return countChildren(root); }

private:
    static int countChildren(const EffectNode& node)
    {
        int n = 0;
        for (const EffectNode& c : node.children) n += 1 + countChildren(c);
        return n;
    }
};

} // namespace lumi::avs
