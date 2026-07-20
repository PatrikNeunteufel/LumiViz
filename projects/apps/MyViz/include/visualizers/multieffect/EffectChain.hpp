/**
 ****************************************************************************************
 * @file   EffectChain.hpp
 * @brief  GL-free runtime data model of the multi-effect chain (Import Roadmap 5.1)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 *
 * @details
 * Runtime tree mirroring the topology of an AvsParser::EffectNode tree: nested
 * effect lists (containers) plus concrete effect leaves. The tree is editable
 * (decision E5); after every mutation the owner must run compileChain() — the
 * chain compile pass (validation now; Set-Render-Mode roll-out and blend
 * resolution join in later steps, decision E4).
 *
 * GL-free by contract: this header describes *what* to render, never touches
 * OpenGL. The MultiEffectVisualizer walks the tree on the render thread.
 *
 * Effect coverage grows with the 5.x steps (design doc, decision E2). 5.1 ships
 * the trivial leaves (Clear, Fadeout, Invert), a host-only DebugBars leaf for
 * sight-testing the chain, and Passthrough for conserved unknown effects.
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace lumi::multieffect {

// =============================================================================
// Blend modes (AVS r_list.cpp order == mode value)
// =============================================================================

/**
 * The 14 AVS list blend modes (in- and out-blend). Batch 1 (decision E3)
 * implements the common ones; the exotic rest falls back to Replace with a
 * compile warning until its batch lands.
 */
enum class BlendMode : int
{
    Ignore = 0,
    Replace = 1,
    FiftyFifty = 2,
    Maximum = 3,
    Additive = 4,
    Subtractive12 = 5,
    Subtractive21 = 6,
    EveryOtherLine = 7,
    EveryOtherPixel = 8,
    Xor = 9,
    Adjustable = 10,
    Multiply = 11,
    Buffer = 12,
    Minimum = 13,
};

/** Batch 1 per decision E3 (Buffer needs the pool users from step 5.4). */
[[nodiscard]] inline bool isBlendModeImplemented(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Ignore:
        case BlendMode::Replace:
        case BlendMode::FiftyFifty:
        case BlendMode::Maximum:
        case BlendMode::Additive:
        case BlendMode::Adjustable:
        case BlendMode::Multiply:
        case BlendMode::Minimum:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] inline const char* blendModeName(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Ignore: return "Ignore";
        case BlendMode::Replace: return "Replace";
        case BlendMode::FiftyFifty: return "50/50";
        case BlendMode::Maximum: return "Maximum";
        case BlendMode::Additive: return "Additive";
        case BlendMode::Subtractive12: return "Subtractive 1-2";
        case BlendMode::Subtractive21: return "Subtractive 2-1";
        case BlendMode::EveryOtherLine: return "Every other line";
        case BlendMode::EveryOtherPixel: return "Every other pixel";
        case BlendMode::Xor: return "XOR";
        case BlendMode::Adjustable: return "Adjustable";
        case BlendMode::Multiply: return "Multiply";
        case BlendMode::Buffer: return "Buffer";
        case BlendMode::Minimum: return "Minimum";
    }
    return "?";
}

// =============================================================================
// Per-effect parameter structs (variant alternative == effect type)
// =============================================================================

/**
 * Container node (AVS Effect List): persistent list buffer (thisfb), in-/out-
 * blend, OnBeat activation and the optional EEL list slot pair (init/frame)
 * that drives enabled/clear/beat/alphain/alphaout at runtime.
 */
struct ListParams
{
    bool clearEveryFrame = false;  ///< clear the list buffer at frame start

    BlendMode blendIn = BlendMode::Ignore;    ///< parent -> list buffer
    BlendMode blendOut = BlendMode::Replace;  ///< list buffer -> parent
    int inAdjustAlpha = 128;                  ///< Adjustable in-alpha 0..255
    int outAdjustAlpha = 128;                 ///< Adjustable out-alpha 0..255

    bool onBeatRender = false;  ///< render only for N frames after a beat
    int onBeatFrames = 1;       ///< N (>= 1)

    bool useCode = false;   ///< run the EEL list slots
    std::string initCode;   ///< EEL, once after (re)compile
    std::string frameCode;  ///< EEL, per frame (enabled/clear/beat/alphain/alphaout)
};

/** AVS "Render / Clear screen" (ID 25), reduced to the 5.1 core. */
struct ClearParams
{
    uint32_t color = 0x000000;  ///< 0x00RRGGBB
    bool onlyFirst = false;     ///< clear only on the first frame
};

/** AVS "Trans / Fadeout" (ID 3): per-frame clamped step towards a target color. */
struct FadeoutParams
{
    int fadeLen = 16;           ///< per-frame step 0..92 (AVS range)
    uint32_t color = 0x000000;  ///< target color 0x00RRGGBB
};

/** AVS "Trans / Invert" (ID 37): XOR 0xFFFFFF. */
struct InvertParams
{
};

/**
 * AVS "Trans / Brightness" (ID 22): per-channel scale with an optional
 * exclusion color. Scale values are the AVS slider range -4096..4096
 * (0 = unchanged multiplier 1.0; see r_bright pixel math, filled in step 5.3).
 */
struct BrightnessParams
{
    int red = 0;    ///< -4096..4096
    int green = 0;  ///< -4096..4096
    int blue = 0;   ///< -4096..4096

    bool exclude = false;       ///< leave pixels near `color` untouched
    uint32_t color = 0x000000;  ///< exclusion color 0x00RRGGBB
    int distance = 16;          ///< exclusion radius 0..255
};

/** AVS "Trans / Fast Brightness" (ID 44): dir 0 = x2, 1 = x0.5, 2 = off. */
struct FastBrightnessParams
{
    int dir = 0;  ///< 0..2
};

/** AVS "Trans / Blur" (ID 6): box blur, strength selects the kernel. */
struct BlurParams
{
    int strength = 1;    ///< 1 = light, 2 = medium, 3 = heavy
    bool roundUp = true; ///< AVS "round mode" rounding bias
};

/** AVS "Trans / Mirror" (ID 26): reflect one screen half onto the other. */
struct MirrorParams
{
    bool leftToRight = true;   ///< left half onto right
    bool topToBottom = false;  ///< top half onto bottom
    bool onBeatRandom = false; ///< randomize active edges on beat
};

/** AVS "Render / OnBeat Clear" (ID 5): clear every N beats. */
struct OnBeatClearParams
{
    uint32_t color = 0x000000;  ///< clear color 0x00RRGGBB
    int everyNBeats = 1;        ///< N (>= 1)
    bool blend = false;         ///< 50/50 towards color instead of hard clear
};

/**
 * AVS "Trans / Colorfade" (ID 11): per-pixel channel-order classification adds
 * fader deltas — a cycling color shift. Fader triples are signed byte deltas;
 * the beat variant is used for `onBeatFrames` frames after a beat.
 */
struct ColorfadeParams
{
    int faderR = 8;   ///< -32..32
    int faderG = 8;
    int faderB = -8;
    int beatFaderR = 8;
    int beatFaderG = -8;
    int beatFaderB = 8;
    int onBeatFrames = 1;  ///< frames the beat faders stay active (>= 1)
};

/**
 * AVS "Trans / Color Modifier" (ID 45): per-channel 256-entry curve, scripted
 * (EEL level/frame/beat/init slots run by a ScriptLutModule). `recompute`
 * rebuilds the table every frame instead of once after compile.
 */
struct ColorModifierParams
{
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string levelCode;   ///< runs per LUT entry (red/green/blue in/out)
    bool recompute = true;
};

/**
 * AVS "Trans / Movement" (ID 15): coordinate remap via a scripted displacement
 * grid (ScriptGridModule runs the point expression per node). 5.4 covers the
 * user-code path; the 23 built-in formulas map to passthrough for now.
 */
struct MovementParams
{
    std::string code;         ///< AVS point expression (empty = identity)
    bool rectCoords = false;  ///< true = x/y, false (AVS default) = polar d/r
    bool wrap = false;        ///< wrap sampling coordinates instead of clamp
};

/**
 * AVS "Trans / Dynamic Movement" (ID 43): grid-based scripted remap with the
 * full EEL quartet and a configurable grid resolution.
 */
struct DynamicMovementParams
{
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pointCode;
    int xres = 16;
    int yres = 12;
    bool rectCoords = false;
    bool wrap = false;
};

/**
 * AVS "Trans / Blitter Feedback" (ID 4): zoom the current image and blend it
 * with itself — a scale-feedback trail. Zoom is a direct factor (1 = none);
 * the exact AVS scale-slider mapping is applied by the 5.5 translator.
 */
struct BlitterFeedbackParams
{
    float zoom = 1.03f;      ///< per-frame zoom factor (>1 magnifies)
    float beatZoom = 0.9f;   ///< zoom used on beat when `onBeat`
    bool onBeat = false;     ///< switch to beatZoom on a beat
    bool blend = true;       ///< 50/50 with the original instead of replace
};

/**
 * AVS "Trans / Roto Blitter" (ID 9): rotate + zoom the current image and blend
 * it with itself. Rotation accumulates over time at `rotationSpeed` deg/frame.
 */
struct RotoBlitterParams
{
    float zoom = 1.0f;            ///< zoom factor
    float rotationSpeed = 1.0f;  ///< degrees per frame (sign = direction)
    bool blend = true;           ///< 50/50 with the original instead of replace
};

/**
 * AVS "Misc / Buffer Save" (ID 18): copy the framebuffer to one of 8 global
 * buffers (save) or blend a stored buffer back (restore).
 */
struct BufferSaveParams
{
    int slot = 0;                          ///< global buffer index 0..7
    bool save = true;                      ///< true = save, false = restore
    BlendMode blend = BlendMode::Replace;  ///< restore blend mode
    int adjustAlpha = 128;                 ///< Adjustable restore alpha 0..255
};

/**
 * AVS "Misc / Custom BPM" (ID 33): mutates the beat signal for the effects
 * that follow it in the chain (arbitrary interval / skip / invert).
 */
struct CustomBpmParams
{
    bool arbitrary = false;  ///< emit a beat every `arbitraryMs`
    int arbitraryMs = 500;   ///< interval for arbitrary mode
    bool skip = false;       ///< only pass every (skipCount+1)-th beat
    int skipCount = 1;
    bool invert = false;     ///< invert the (possibly modified) beat
};

/**
 * AVS "Render / SuperScope" (ID 36): a scripted point/line scope. The point
 * script (EEL quartet) is run by a SuperscopeModule; the host draws the points
 * via the shared ScopeRenderer (decision E6). `renderMode` 0=dots 1=lines
 * 2=thick; `audioChannel` 0=L 1=R 2=mono 3=mid 4=side (SuperscopeAudioChannel).
 */
struct SuperScopeParams
{
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pointCode;
    int pointCount = 256;
    int renderMode = 1;    ///< 0=dots, 1=lines, 2=thick lines
    float lineWidth = 2.0f;
    float dotSize = 4.0f;
    int audioChannel = 2;  ///< 0=L 1=R 2=mono 3=mid 4=side
};

/**
 * Host-only debug leaf (no AVS counterpart): audio-reactive quad orbiting the
 * center. Exists so the 5.1 chain has a visible content source before the real
 * scope renderer arrives (step 5.4, decision E6).
 */
struct DebugBarsParams
{
    uint32_t color = 0xFF80FF;  ///< 0x00RRGGBB
    float orbitSpeed = 1.0f;    ///< revolutions factor (1 = one turn per ~6.3 s)
};

/** Conserved effect the host cannot render yet — passes the buffer through. */
struct PassthroughParams
{
    int32_t sourceId = 0;  ///< original AVS effect id (or 0 if hand-built)
    std::string note;      ///< import report text ("effect X not implemented")
};

using EffectParams =
    std::variant<ListParams, ClearParams, FadeoutParams, InvertParams,
                 BrightnessParams, FastBrightnessParams, BlurParams, MirrorParams,
                 OnBeatClearParams, ColorfadeParams, ColorModifierParams,
                 MovementParams, DynamicMovementParams, BlitterFeedbackParams,
                 RotoBlitterParams, BufferSaveParams, CustomBpmParams,
                 SuperScopeParams, DebugBarsParams, PassthroughParams>;

// =============================================================================
// Chain node
// =============================================================================

/**
 * One node of the runtime chain. Which variant alternative `params` holds
 * decides the effect type; `children` is only meaningful for ListParams.
 */
struct ChainNode
{
    std::string displayName;         ///< editor label ("" = derive from type)
    std::string description;         ///< free-text note (editor convenience, optional)
    bool enabled = true;             ///< disabled nodes are skipped entirely
    EffectParams params;             ///< effect type + its parameters
    std::vector<ChainNode> children; ///< child effects (lists only)

    /**
     * Stable node identity, assigned by compileChain() (0 = unassigned).
     * The render host keys GL/script resources by this id, so it survives
     * vector reallocation during edits; never reuse ids manually.
     */
    uint64_t nodeId = 0;

    [[nodiscard]] bool isList() const
    {
        return std::holds_alternative<ListParams>(params);
    }
};

// =============================================================================
// Chain compile pass (runs after every mutation, decision E4)
// =============================================================================

/** One structural finding, path-prefixed like the AvsParser import report. */
struct CompileMessage
{
    std::string path;  ///< e.g. "root/2/0"
    std::string text;
};

struct CompileResult
{
    bool ok = true;                        ///< false only on structural damage
    std::vector<CompileMessage> warnings;  ///< never a hard failure (AVS rule)
};

/** Human-readable effect-type name (also the displayName fallback). */
[[nodiscard]] inline const char* effectTypeName(const EffectParams& params)
{
    struct Visitor
    {
        const char* operator()(const ListParams&) const { return "Effect List"; }
        const char* operator()(const ClearParams&) const { return "Clear"; }
        const char* operator()(const FadeoutParams&) const { return "Fadeout"; }
        const char* operator()(const InvertParams&) const { return "Invert"; }
        const char* operator()(const BrightnessParams&) const { return "Brightness"; }
        const char* operator()(const FastBrightnessParams&) const { return "Fast Brightness"; }
        const char* operator()(const BlurParams&) const { return "Blur"; }
        const char* operator()(const MirrorParams&) const { return "Mirror"; }
        const char* operator()(const OnBeatClearParams&) const { return "OnBeat Clear"; }
        const char* operator()(const ColorfadeParams&) const { return "Colorfade"; }
        const char* operator()(const ColorModifierParams&) const { return "Color Modifier"; }
        const char* operator()(const MovementParams&) const { return "Movement"; }
        const char* operator()(const DynamicMovementParams&) const { return "Dynamic Movement"; }
        const char* operator()(const BlitterFeedbackParams&) const { return "Blitter Feedback"; }
        const char* operator()(const RotoBlitterParams&) const { return "Roto Blitter"; }
        const char* operator()(const BufferSaveParams&) const { return "Buffer Save"; }
        const char* operator()(const CustomBpmParams&) const { return "Custom BPM"; }
        const char* operator()(const SuperScopeParams&) const { return "SuperScope"; }
        const char* operator()(const DebugBarsParams&) const { return "Debug Bars"; }
        const char* operator()(const PassthroughParams&) const { return "Passthrough"; }
    };
    return std::visit(Visitor{}, params);
}

namespace detail {

inline uint64_t maxNodeId(const ChainNode& node)
{
    uint64_t maxId = node.nodeId;
    for (const ChainNode& child : node.children)
    {
        maxId = std::max(maxId, maxNodeId(child));
    }
    return maxId;
}

inline void warnFallbackBlend(BlendMode mode, const char* which,
                              const std::string& path, CompileResult& result)
{
    if (!isBlendModeImplemented(mode))
    {
        result.warnings.push_back(
            {path, std::string("blend mode \"") + blendModeName(mode) + "\" (" +
                       which + ") not implemented yet - falls back to Replace"});
    }
}

inline void compileNode(ChainNode& node, const std::string& path,
                        CompileResult& result, uint64_t& nextId)
{
    if (node.nodeId == 0) node.nodeId = nextId++;
    if (node.displayName.empty()) node.displayName = effectTypeName(node.params);

    if (!node.isList() && !node.children.empty())
    {
        result.warnings.push_back(
            {path, std::string(effectTypeName(node.params)) +
                       ": children on a non-list node are ignored"});
    }

    // Clamp effect parameters to their AVS ranges (editor may write anything).
    if (auto* fade = std::get_if<FadeoutParams>(&node.params))
    {
        fade->fadeLen = std::clamp(fade->fadeLen, 0, 92);
    }
    if (auto* bright = std::get_if<BrightnessParams>(&node.params))
    {
        bright->red = std::clamp(bright->red, -4096, 4096);
        bright->green = std::clamp(bright->green, -4096, 4096);
        bright->blue = std::clamp(bright->blue, -4096, 4096);
        bright->distance = std::clamp(bright->distance, 0, 255);
    }
    if (auto* fast = std::get_if<FastBrightnessParams>(&node.params))
    {
        fast->dir = std::clamp(fast->dir, 0, 2);
    }
    if (auto* blur = std::get_if<BlurParams>(&node.params))
    {
        blur->strength = std::clamp(blur->strength, 1, 3);
    }
    if (auto* clearBeat = std::get_if<OnBeatClearParams>(&node.params))
    {
        if (clearBeat->everyNBeats < 1) clearBeat->everyNBeats = 1;
    }
    if (auto* fade = std::get_if<ColorfadeParams>(&node.params))
    {
        auto clampFader = [](int& v) { v = std::clamp(v, -32, 32); };
        clampFader(fade->faderR);
        clampFader(fade->faderG);
        clampFader(fade->faderB);
        clampFader(fade->beatFaderR);
        clampFader(fade->beatFaderG);
        clampFader(fade->beatFaderB);
        if (fade->onBeatFrames < 1) fade->onBeatFrames = 1;
    }
    if (auto* bpm = std::get_if<CustomBpmParams>(&node.params))
    {
        if (bpm->arbitraryMs < 1) bpm->arbitraryMs = 1;
        if (bpm->skipCount < 1) bpm->skipCount = 1;
    }
    if (auto* dmove = std::get_if<DynamicMovementParams>(&node.params))
    {
        dmove->xres = std::clamp(dmove->xres, 2, 96);
        dmove->yres = std::clamp(dmove->yres, 2, 72);
    }
    if (auto* save = std::get_if<BufferSaveParams>(&node.params))
    {
        save->slot = std::clamp(save->slot, 0, 7);
        save->adjustAlpha = std::clamp(save->adjustAlpha, 0, 255);
        if (!save->save) warnFallbackBlend(save->blend, "restore", path, result);
    }
    if (auto* scope = std::get_if<SuperScopeParams>(&node.params))
    {
        scope->pointCount = std::clamp(scope->pointCount, 1, 4096);
        scope->renderMode = std::clamp(scope->renderMode, 0, 2);
        scope->audioChannel = std::clamp(scope->audioChannel, 0, 4);
        scope->lineWidth = std::clamp(scope->lineWidth, 1.0f, 20.0f);
        scope->dotSize = std::clamp(scope->dotSize, 1.0f, 50.0f);
    }
    if (auto* list = std::get_if<ListParams>(&node.params))
    {
        list->inAdjustAlpha = std::clamp(list->inAdjustAlpha, 0, 255);
        list->outAdjustAlpha = std::clamp(list->outAdjustAlpha, 0, 255);
        if (list->onBeatFrames < 1) list->onBeatFrames = 1;
        warnFallbackBlend(list->blendIn, "in", path, result);
        warnFallbackBlend(list->blendOut, "out", path, result);
    }

    if (node.isList())
    {
        for (size_t i = 0; i < node.children.size(); ++i)
        {
            compileNode(node.children[i], path + "/" + std::to_string(i), result,
                        nextId);
        }
    }
}

} // namespace detail

/**
 * @brief Validate + normalize the chain in place.
 *
 * Fills empty display names, clamps parameter ranges, reports structural
 * oddities as warnings (never hard-fails — AVS philosophy). Later steps add
 * the Set-Render-Mode roll-out and blend resolution here (decision E4:
 * re-propagated on every edit).
 *
 * @param root Root node; must be a list (warning + ok=false otherwise).
 */
inline CompileResult compileChain(ChainNode& root)
{
    CompileResult result;
    if (!root.isList())
    {
        result.ok = false;
        result.warnings.push_back({"root", "root node must be an effect list"});
        return result;
    }
    uint64_t nextId = detail::maxNodeId(root) + 1;
    detail::compileNode(root, "root", result, nextId);
    return result;
}

/** Number of nodes in the tree, root excluded (AvsParser::effectCount rule). */
[[nodiscard]] inline int nodeCount(const ChainNode& root)
{
    int count = 0;
    for (const ChainNode& child : root.children)
    {
        count += 1 + nodeCount(child);
    }
    return count;
}

} // namespace lumi::multieffect
