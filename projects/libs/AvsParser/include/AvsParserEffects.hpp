/**
 ****************************************************************************************
 * @file   AvsParserEffects.hpp
 * @brief  Builtin effect table + config-blob decoders for the AVS core set
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Each decoder is a 1:1 transcription of the respective load_config in
 * ref/vis_avs/avs/vis_avs/r_*.cpp (BSD-3, Nullsoft 2005) — same field order,
 * same guards, same defaults. Covered: the ~17 core effects of the import
 * priority list (Import-Analyse §5.2) plus the shared code-quartet pattern.
 * Effects without a decoder keep their raw blob (EffectNode::decoded=false).
 *
 * EEL slot order in files is always [0]=Point/Level, [1]=Frame, [2]=Beat,
 * [3]=Init (verified against codehandle usage in r_sscope/r_dmove/r_dcolormod).
 ****************************************************************************************
 */

#pragma once

#include "AvsParserReader.hpp"
#include "AvsParserTypes.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

namespace lumi::avs::detail {

// =====================================================================================
// Builtin tables (registration order in rlib.cpp = preset ID)
// =====================================================================================

inline constexpr std::array<const char*, kBuiltinCount> kBuiltinNames = {
    "Simple",                       // 0
    "Dot Plane",                    // 1
    "Oscilliscope Star",            // 2
    "Fadeout",                      // 3
    "Blitter Feedback",             // 4
    "OnBeat Clear",                 // 5
    "Blur",                         // 6
    "Bass Spin",                    // 7
    "Moving Particle",              // 8
    "Roto Blitter",                 // 9
    "SVP Loader",                   // 10
    "Colorfade",                    // 11
    "Color Clip",                   // 12
    "Rotating Stars",               // 13
    "Ring",                         // 14
    "Movement",                     // 15
    "Scatter",                      // 16
    "Dot Grid",                     // 17
    "Buffer Save",                  // 18
    "Dot Fountain",                 // 19
    "Water",                        // 20
    "Comment",                      // 21
    "Brightness",                   // 22
    "Interleave",                   // 23
    "Grain",                        // 24
    "Clear Screen",                 // 25
    "Mirror",                       // 26
    "Starfield",                    // 27
    "Text",                         // 28
    "Bump",                         // 29
    "Mosaic",                       // 30
    "Water Bump",                   // 31
    "AVI",                          // 32
    "Custom BPM",                   // 33
    "Picture",                      // 34
    "Dynamic Distance Modifier",    // 35
    "SuperScope",                   // 36
    "Invert",                       // 37
    "Unique Tone",                  // 38
    "Timescope",                    // 39
    "Set Render Mode",              // 40
    "Interferences",                // 41
    "Dynamic Shift",                // 42
    "Dynamic Movement",             // 43
    "Fast Brightness",              // 44
    "Color Modifier"                // 45
};

/// Old named APEs that map onto builtins (ref rlib.cpp NamedApeToBuiltinTrans)
struct ApeAlias
{
    const char* apeId;
    std::int32_t builtinIndex;
};

inline constexpr std::array<ApeAlias, 12> kApeAliases = {{
    {"Winamp Brightness APE v1", 22},
    {"Winamp Interleave APE v1", 23},
    {"Winamp Grain APE v1", 24},
    {"Winamp ClearScreen APE v1", 25},
    {"Nullsoft MIRROR v1", 26},
    {"Winamp Starfield v1", 27},
    {"Winamp Text v1", 28},
    {"Winamp Bump v1", 29},
    {"Winamp Mosaic v1", 30},
    {"Winamp AVIAPE v1", 32},
    {"Nullsoft Picture Rendering v1", 34},
    {"Winamp Interf APE v1", 41}
}};

/// Builtin "APEs" compiled into AVS, serialized via ID string (ref rlib.cpp)
inline constexpr std::array<const char*, 5> kBuiltinApeNames = {
    "Channel Shift", "Color Reduction", "Multiplier",
    "Holden04: Video Delay", "Holden05: Multi Delay"
};

/// Pseudo entry inside effect lists that carries the list's EEL code
inline constexpr std::string_view kListCodeApeId = "AVS 2.8+ Effect List Config";

// =====================================================================================
// Decode helpers
// =====================================================================================

inline void addField(EffectNode& node, const char* name, std::int32_t value)
{
    node.fields.push_back(IntField{name, value});
}

/// Guarded field read + record; keeps the original "stop on short data" behavior.
inline bool readField(Reader& r, EffectNode& node, const char* name)
{
    std::int32_t v = 0;
    if (!r.tryI32(v)) return false;
    addField(node, name, v);
    return true;
}

/// Guarded field read with explicit default when the file is short (old presets).
inline void readFieldOr(Reader& r, EffectNode& node, const char* name,
                        std::int32_t defaultValue)
{
    std::int32_t v = defaultValue;
    if (!r.tryI32(v)) v = defaultValue;
    addField(node, name, v);
}

/**
 * @brief The shared EEL code quartet (SuperScope, Dynamic Movement, Color Modifier)
 *
 * New format: version byte 1, then 4 length-prefixed strings in file order
 * [0]=point/level, [1]=frame, [2]=beat, [3]=init. Old format: 1024-byte block
 * of four fixed 256-byte C strings in the same order.
 */
inline void readCodeQuartet(Reader& r, EffectNode& node,
                            const std::array<const char*, 4>& slotNames)
{
    if (r.peekByte() == 1)
    {
        r.skip(1);
        for (const char* name : slotNames)
            node.code.push_back(CodeSlot{name, r.loadString()});
    }
    else
    {
        const std::uint8_t* block = nullptr;
        if (r.tryBytes(1024, block))
        {
            for (int i = 0; i < 4; ++i)
                node.code.push_back(
                    CodeSlot{slotNames[i], r.fixedString(block + i * 256, 256)});
        }
    }
}

inline constexpr std::array<const char*, 4> kPointSlots = {"point", "frame", "beat",
                                                           "init"};
inline constexpr std::array<const char*, 4> kLevelSlots = {"level", "frame", "beat",
                                                           "init"};

// =====================================================================================
// Per-effect decoders (1:1 transcriptions of load_config)
// =====================================================================================

inline void decodeFadeout(Reader& r, EffectNode& n)   // r_fadeout.cpp
{
    readField(r, n, "fadelen") && readField(r, n, "color");
}

inline void decodeBlitterFeedback(Reader& r, EffectNode& n)   // r_blit.cpp
{
    if (readField(r, n, "scale") && readField(r, n, "scale2") &&
        readField(r, n, "blend") && readField(r, n, "beatch"))
    {
        readFieldOr(r, n, "subpixel", 0);
    }
}

inline void decodeOnBeatClear(Reader& r, EffectNode& n)   // r_nfclr.cpp
{
    readField(r, n, "color") && readField(r, n, "blend") && readField(r, n, "nf");
}

inline void decodeBlur(Reader& r, EffectNode& n)   // r_blur.cpp
{
    if (readField(r, n, "enabled")) readFieldOr(r, n, "roundmode", 0);
}

inline void decodeRotoBlitter(Reader& r, EffectNode& n)   // r_rotblit.cpp
{
    if (readField(r, n, "zoom_scale") && readField(r, n, "rot_dir") &&
        readField(r, n, "blend") && readField(r, n, "beatch") &&
        readField(r, n, "beatch_speed") && readField(r, n, "zoom_scale2") &&
        readField(r, n, "beatch_scale"))
    {
        readFieldOr(r, n, "subpixel", 0);
    }
}

inline void decodeColorfade(Reader& r, EffectNode& n)   // r_colorfade.cpp
{
    if (!(readField(r, n, "enabled") && readField(r, n, "fader_r") &&
          readField(r, n, "fader_g") && readField(r, n, "fader_b")))
        return;
    // beat faders default to the base faders when the file is short
    readFieldOr(r, n, "beatfader_r", n.field("fader_r"));
    readFieldOr(r, n, "beatfader_g", n.field("fader_g"));
    readFieldOr(r, n, "beatfader_b", n.field("fader_b"));
}

inline void decodeMovement(Reader& r, EffectNode& n)   // r_trans.cpp
{
    constexpr std::int32_t kUserEffect = 32767;
    constexpr std::int32_t kReffectMax = 23;

    std::int32_t effect = 0;
    if (!r.tryI32(effect)) return;

    bool rectangularHint = false;
    if (effect == kUserEffect)
    {
        // optional "!rect " marker in front of the expression
        static constexpr char kRectTag[] = "!rect ";
        bool hasTag = r.remaining() >= 6;
        for (int i = 0; hasTag && i < 6; ++i)
            hasTag = (r.peekByte(static_cast<std::size_t>(i)) ==
                      static_cast<std::uint8_t>(kRectTag[i]));
        if (hasTag)
        {
            r.skip(6);
            rectangularHint = true;
        }

        if (r.peekByte() == 1)
        {
            r.skip(1);
            n.code.push_back(CodeSlot{"point", r.loadString()});
        }
        else
        {
            const std::size_t blockLen = 256 - (rectangularHint ? 6U : 0U);
            const std::uint8_t* block = nullptr;
            if (r.remaining() >= 256 && r.tryBytes(blockLen, block))
                n.code.push_back(CodeSlot{"point", r.fixedString(block, blockLen)});
        }
    }

    std::int32_t v = 0;
    if (r.tryI32(v)) addField(n, "blend", v);
    if (r.tryI32(v)) addField(n, "sourcemapped", v);
    if (r.tryI32(v)) addField(n, "rectangular", v);
    readFieldOr(r, n, "subpixel", 0);
    readFieldOr(r, n, "wrap", 0);
    if (effect == 0 && r.tryI32(v)) effect = v;
    if ((effect != kUserEffect && effect > kReffectMax) || effect < 0) effect = 0;
    addField(n, "effect", effect);
}

inline void decodeBufferSave(Reader& r, EffectNode& n)   // r_stack.cpp
{
    constexpr std::int32_t kNumBuffers = 8;   // NBUF
    std::int32_t dir = 0;
    std::int32_t which = 0;
    if (r.tryI32(dir)) addField(n, "dir", dir);
    if (r.tryI32(which))
    {
        if (which < 0) which = 0;
        if (which >= kNumBuffers) which = kNumBuffers - 1;
        addField(n, "which", which);
    }
    readField(r, n, "blend") && readField(r, n, "adjblend_val");
}

inline void decodeBrightness(Reader& r, EffectNode& n)   // r_bright.cpp
{
    readField(r, n, "enabled") && readField(r, n, "blend") &&
        readField(r, n, "blendavg") && readField(r, n, "redp") &&
        readField(r, n, "greenp") && readField(r, n, "bluep") &&
        readField(r, n, "dissoc") && readField(r, n, "color") &&
        readField(r, n, "exclude") && readField(r, n, "distance");
}

inline void decodeClearScreen(Reader& r, EffectNode& n)   // r_clear.cpp
{
    readField(r, n, "enabled") && readField(r, n, "color") &&
        readField(r, n, "blend") && readField(r, n, "blendavg") &&
        readField(r, n, "onlyfirst");
}

inline void decodeMirror(Reader& r, EffectNode& n)   // r_mirror.cpp
{
    readField(r, n, "enabled") && readField(r, n, "mode") &&
        readField(r, n, "onbeat") && readField(r, n, "smooth") &&
        readField(r, n, "slower");
}

inline void decodeCustomBpm(Reader& r, EffectNode& n)   // r_bpm.cpp
{
    readField(r, n, "enabled") && readField(r, n, "arbitrary") &&
        readField(r, n, "skip") && readField(r, n, "invert") &&
        readField(r, n, "arbval") && readField(r, n, "skipval") &&
        readField(r, n, "skipfirst");
}

inline void decodeSuperScope(Reader& r, EffectNode& n)   // r_sscope.cpp
{
    readCodeQuartet(r, n, kPointSlots);
    if (!readField(r, n, "which_ch")) return;

    std::int32_t numColors = 0;
    if (!r.tryI32(numColors)) return;
    if (numColors <= 16)
    {
        std::int32_t c = 0;
        while (static_cast<std::int32_t>(n.colors.size()) < numColors && r.tryI32(c))
            n.colors.push_back(static_cast<std::uint32_t>(c));
    }
    else
    {
        numColors = 0;
    }
    addField(n, "num_colors", numColors);
    std::int32_t drawMode = 0;
    if (r.tryI32(drawMode)) addField(n, "drawmode", drawMode);
}

inline void decodeInvert(Reader& r, EffectNode& n)   // r_invert.cpp
{
    readField(r, n, "enabled");
}

inline void decodeMosaic(Reader& r, EffectNode& n)   // r_mosaic.cpp
{
    readField(r, n, "enabled") && readField(r, n, "quality") &&
        readField(r, n, "quality2") && readField(r, n, "blend") &&
        readField(r, n, "blendavg") && readField(r, n, "onbeat") &&
        readField(r, n, "durFrames");
}

inline void decodeGrain(Reader& r, EffectNode& n)   // r_grain.cpp
{
    readField(r, n, "enabled") && readField(r, n, "blend") &&
        readField(r, n, "blendavg") && readField(r, n, "smax") &&
        readField(r, n, "staticgrain");
}

inline void decodeScatter(Reader& r, EffectNode& n)   // r_scat.cpp
{
    readField(r, n, "enabled");
}

inline void decodeWater(Reader& r, EffectNode& n)   // r_water.cpp
{
    readField(r, n, "enabled");
}

inline void decodeDotGrid(Reader& r, EffectNode& n)   // r_dotgrid.cpp
{
    std::int32_t nc = 0;
    if (!r.tryI32(nc)) return;
    if (nc <= 16)
    {
        std::int32_t c = 0;
        while (static_cast<std::int32_t>(n.colors.size()) < nc && r.tryI32(c))
            n.colors.push_back(static_cast<std::uint32_t>(c));
    }
    else nc = 0;
    addField(n, "num_colors", nc);
    readField(r, n, "spacing") && readField(r, n, "x_move") &&
        readField(r, n, "y_move") && readField(r, n, "blend");
}

inline void decodeDotPlaneFountain(Reader& r, EffectNode& n)   // r_dotpln / r_dotfnt
{
    readField(r, n, "rotvel");
    std::int32_t c = 0;
    for (int i = 0; i < 5 && r.tryI32(c); ++i)
        n.colors.push_back(static_cast<std::uint32_t>(c));
    readField(r, n, "angle");
    readField(r, n, "r_raw");
}

inline void decodeTimescope(Reader& r, EffectNode& n)   // r_timescope.cpp
{
    readField(r, n, "enabled") && readField(r, n, "color") &&
        readField(r, n, "blend") && readField(r, n, "blendavg") &&
        readField(r, n, "which_ch") && readField(r, n, "nbands");
}

inline void decodeStarfield(Reader& r, EffectNode& n)   // r_stars.cpp
{
    // warpSpeed / beatSpeed are float32; stored as int32 bits, cast in translator.
    readField(r, n, "enabled") && readField(r, n, "color") &&
        readField(r, n, "blend") && readField(r, n, "blendavg") &&
        readField(r, n, "warpSpeed_bits") && readField(r, n, "maxStars") &&
        readField(r, n, "onbeat") && readField(r, n, "beatSpeed_bits") &&
        readField(r, n, "durFrames");
}

inline void decodeWaterBump(Reader& r, EffectNode& n)   // r_waterbump.cpp
{
    readField(r, n, "enabled") && readField(r, n, "density") &&
        readField(r, n, "depth") && readField(r, n, "random_drop") &&
        readField(r, n, "drop_x") && readField(r, n, "drop_y") &&
        readField(r, n, "drop_radius") && readField(r, n, "method");
}

inline void decodeBump(Reader& r, EffectNode& n)   // r_bump.cpp
{
    readField(r, n, "enabled") && readField(r, n, "onbeat") &&
        readField(r, n, "durFrames") && readField(r, n, "depth") &&
        readField(r, n, "depth2") && readField(r, n, "blend") &&
        readField(r, n, "blendavg");
    // Three length-prefixed EEL strings: code1 = frame, code2 = beat, code3 = init.
    n.code.push_back(CodeSlot{"frame", r.loadString()});
    n.code.push_back(CodeSlot{"beat", r.loadString()});
    n.code.push_back(CodeSlot{"init", r.loadString()});
    readFieldOr(r, n, "showlight", 0);
    readFieldOr(r, n, "invert", 0);
    readFieldOr(r, n, "oldstyle", 1);   // absent -> legacy scaling
    readFieldOr(r, n, "buffern", 0);
}

inline void decodeInterferences(Reader& r, EffectNode& n)   // r_interf.cpp
{
    // `speed` is a float32 on disk; stored as its raw int32 bit pattern here and
    // reinterpreted in the translator (the reader only yields int32 fields).
    readField(r, n, "enabled") && readField(r, n, "nPoints") &&
        readField(r, n, "rotation") && readField(r, n, "distance") &&
        readField(r, n, "alpha") && readField(r, n, "rotationinc") &&
        readField(r, n, "blend") && readField(r, n, "blendavg") &&
        readField(r, n, "distance2") && readField(r, n, "alpha2") &&
        readField(r, n, "rotationinc2") && readField(r, n, "rgb") &&
        readField(r, n, "onbeat") && readField(r, n, "speed_bits");
}

inline void decodeSetRenderMode(Reader& r, EffectNode& n)   // r_linemode.cpp
{
    // packed: bits 0-7 blend mode, 8-15 adjustable-blend value, 16-23 line width,
    // bit 31 enabled
    readField(r, n, "newmode");
}

inline void decodeDynamicMovement(Reader& r, EffectNode& n)   // r_dmove.cpp
{
    readCodeQuartet(r, n, kPointSlots);
    if (readField(r, n, "subpixel") && readField(r, n, "rectcoords") &&
        readField(r, n, "xres") && readField(r, n, "yres") &&
        readField(r, n, "blend") && readField(r, n, "wrap"))
    {
        readFieldOr(r, n, "buffern", 0);
        readFieldOr(r, n, "nomove", 0);
    }
}

inline void decodeDynamicDistanceModifier(Reader& r, EffectNode& n)   // r_ddm.cpp
{
    // File slot order = quartet [pixel/point, frame, beat, init], then blend/subpixel.
    readCodeQuartet(r, n, kPointSlots);
    readField(r, n, "blend") && readField(r, n, "subpixel");
}

inline void decodeMovingParticle(Reader& r, EffectNode& n)   // r_parts.cpp
{
    readField(r, n, "enabled") && readField(r, n, "colors") &&
        readField(r, n, "maxdist") && readField(r, n, "size") &&
        readField(r, n, "size2") && readField(r, n, "blend");
}

inline void decodeDynamicShift(Reader& r, EffectNode& n)   // r_shift.cpp
{
    // File slot order (r_shift load_config): [0]=init, [1]=frame, [2]=beat.
    static constexpr std::array<const char*, 3> kShiftSlots = {"init", "frame", "beat"};
    if (r.peekByte() == 1)
    {
        r.skip(1);
        for (const char* name : kShiftSlots)
            n.code.push_back(CodeSlot{name, r.loadString()});
    }
    else
    {
        const std::uint8_t* block = nullptr;
        if (r.tryBytes(3 * 256, block))
        {
            for (int i = 0; i < 3; ++i)
                n.code.push_back(
                    CodeSlot{kShiftSlots[i], r.fixedString(block + i * 256, 256)});
        }
    }
    readField(r, n, "blend") && readField(r, n, "subpixel");
}

/// Shared "effect + num_colors + colors[]" head (r_simple/oscstar/oscring).
inline void decodeScopeColors(Reader& r, EffectNode& n, bool hasEffect)
{
    if (hasEffect) readField(r, n, "effect");
    std::int32_t nc = 0;
    if (!r.tryI32(nc)) return;
    if (nc <= 16)
    {
        std::int32_t c = 0;
        while (static_cast<std::int32_t>(n.colors.size()) < nc && r.tryI32(c))
            n.colors.push_back(static_cast<std::uint32_t>(c));
    }
    else nc = 0;
    addField(n, "num_colors", nc);
}

inline void decodeSimple(Reader& r, EffectNode& n)   // r_simple.cpp
{
    decodeScopeColors(r, n, true);
}

inline void decodeOscStar(Reader& r, EffectNode& n)   // r_oscstar.cpp
{
    decodeScopeColors(r, n, true);
    readField(r, n, "size") && readField(r, n, "rot");
}

inline void decodeOscRing(Reader& r, EffectNode& n)   // r_oscring.cpp
{
    decodeScopeColors(r, n, true);
    readField(r, n, "size") && readField(r, n, "source");
}

inline void decodeRotatingStars(Reader& r, EffectNode& n)   // r_rotstar.cpp
{
    decodeScopeColors(r, n, false);  // num_colors + colors[] only (no effect field)
}

inline void decodeBassSpin(Reader& r, EffectNode& n)   // r_bspin.cpp
{
    readField(r, n, "enabled") && readField(r, n, "color0") &&
        readField(r, n, "color1") && readField(r, n, "mode");
}

inline void decodeColorClip(Reader& r, EffectNode& n)   // r_contrast.cpp
{
    if (!readField(r, n, "enabled")) return;
    if (!readField(r, n, "color_clip")) return;
    std::int32_t out = 0;
    if (r.tryI32(out)) addField(n, "color_clip_out", out);
    else addField(n, "color_clip_out", n.field("color_clip"));  // old files: same
    readField(r, n, "color_dist");
}

inline void decodeUniqueTone(Reader& r, EffectNode& n)   // r_onetone.cpp
{
    readField(r, n, "enabled") && readField(r, n, "color") &&
        readField(r, n, "blend") && readField(r, n, "blendavg") &&
        readField(r, n, "invert");
}

inline void decodeInterleave(Reader& r, EffectNode& n)   // r_interleave.cpp
{
    readField(r, n, "enabled") && readField(r, n, "x") && readField(r, n, "y") &&
        readField(r, n, "color") && readField(r, n, "blend") &&
        readField(r, n, "blendavg") && readField(r, n, "onbeat") &&
        readField(r, n, "x2") && readField(r, n, "y2") && readField(r, n, "beatdur");
}

inline void decodeFastBrightness(Reader& r, EffectNode& n)   // r_fastbright.cpp
{
    readFieldOr(r, n, "dir", 0);
}

inline void decodeColorModifier(Reader& r, EffectNode& n)   // r_dcolormod.cpp
{
    readCodeQuartet(r, n, kLevelSlots);
    readField(r, n, "recompute");
}

/**
 * @brief Decode the "Color Map" APE blob (format: grandchild/AVS-File-Decoder).
 *
 * Layout: key/blendMode/mapCycleMode (int32) · 4 packed bytes (adjustBlend, null,
 * dontSkipFastBeats, cycleSpeed) · 8 fixed 60-byte map headers (enabled, num, id,
 * 48-byte filename) · then per map `num` colour entries (position, colour, id).
 * Colours are 0x00RRGGBB (no COLORREF swap). The first enabled non-empty map is
 * captured into colors[] + cmpos<k> fields; map cycling is not imported yet.
 */
inline void decodeColorMap(Reader& r, EffectNode& n)   // "Color Map" APE
{
    std::int32_t key = 0;
    std::int32_t blend = 0;
    std::int32_t cycle = 0;
    if (!(r.tryI32(key) && r.tryI32(blend) && r.tryI32(cycle))) return;
    addField(n, "key", key);
    addField(n, "blendMode", blend);
    addField(n, "mapCycleMode", cycle);

    const std::uint8_t* packed = nullptr;
    std::int32_t adjust = 128;
    if (r.tryBytes(4, packed)) adjust = packed[0];  // adjustBlend, null, dsfb, speed
    addField(n, "adjustBlend", adjust);

    std::array<std::int32_t, 8> enabled{};
    std::array<std::int32_t, 8> num{};
    for (int i = 0; i < 8; ++i)
    {
        std::int32_t en = 0;
        std::int32_t nm = 0;
        std::int32_t id = 0;
        if (!(r.tryI32(en) && r.tryI32(nm) && r.tryI32(id))) return;
        enabled[static_cast<std::size_t>(i)] = en;
        num[static_cast<std::size_t>(i)] = nm;
        const std::uint8_t* nameBytes = nullptr;
        if (!r.tryBytes(48, nameBytes)) return;  // skip 48-byte filename
    }

    int chosen = -1;
    for (int i = 0; i < 8; ++i)
    {
        const std::int32_t cnt = num[static_cast<std::size_t>(i)];
        const bool take = chosen == -1 && enabled[static_cast<std::size_t>(i)] != 0 &&
                          cnt > 0;
        for (std::int32_t k = 0; k < cnt; ++k)
        {
            std::int32_t pos = 0;
            std::int32_t col = 0;
            std::int32_t cid = 0;
            if (!(r.tryI32(pos) && r.tryI32(col) && r.tryI32(cid))) return;
            if (take)
            {
                n.colors.push_back(static_cast<std::uint32_t>(col) & 0x00FFFFFFu);
                addField(n, ("cmpos" + std::to_string(k)).c_str(), pos);
            }
        }
        if (take)
        {
            chosen = i;
            addField(n, "cmcount", cnt);
        }
    }
    if (chosen == -1) addField(n, "cmcount", 0);
}

inline void decodeConvolution(Reader& r, EffectNode& n)   // "Holden03: Convolution Filter"
{
    readField(r, n, "enabled") && readField(r, n, "edgeMode") &&
        readField(r, n, "absolute") && readField(r, n, "twoPass");
    for (int i = 0; i < 49; ++i) readField(r, n, ("k" + std::to_string(i)).c_str());
    readField(r, n, "bias") && readField(r, n, "scale");
}

inline void decodeNormalise(Reader& r, EffectNode& n)   // "Trans: Normalise"
{
    readField(r, n, "enabled");
}

inline void decodeMultiFilter(Reader& r, EffectNode& n)   // "Jheriko : MULTIFILTER"
{
    readField(r, n, "enabled") && readField(r, n, "effect") &&
        readField(r, n, "onbeat") && readField(r, n, "null0");
}

inline void decodeAddBorders(Reader& r, EffectNode& n)   // "Virtual Effect: Addborders"
{
    readField(r, n, "enabled") && readField(r, n, "color") && readField(r, n, "size");
}

inline void decodeFramerateLimiter(Reader& r, EffectNode& n)   // "VFX FRAMERATE LIMITER"
{
    readField(r, n, "enabled") && readField(r, n, "limit");
}

inline void decodeBufferBlend(Reader& r, EffectNode& n)   // "Misc: Buffer blend" APE
{
    readField(r, n, "enabled") && readField(r, n, "bufferB") &&
        readField(r, n, "bufferA") && readField(r, n, "mode");
}

inline void decodeJherikoGlobal(Reader& r, EffectNode& n)   // "Jheriko: Global" APE
{
    std::int32_t load = 1;
    if (!r.tryI32(load)) return;
    addField(n, "load", load);
    r.skip(6 * 4);  // null0: 6 reserved int32
    // NtCodeIFB: three NUL-terminated strings in order init, frame, beat
    n.code.push_back(CodeSlot{"init", r.loadCString()});
    n.code.push_back(CodeSlot{"frame", r.loadCString()});
    n.code.push_back(CodeSlot{"beat", r.loadCString()});
    // file, saveRegRange, saveBufRange (NtString) — not imported
}

// =====================================================================================
// Dispatch
// =====================================================================================

/// @brief Decode the config blob of a builtin effect. Returns false when no
///        decoder exists for this index (node then stays raw-only).
inline bool decodeBuiltin(std::int32_t builtinIndex, Reader& r, EffectNode& node)
{
    switch (builtinIndex)
    {
        case 1:  decodeDotPlaneFountain(r, node); break;
        case 3:  decodeFadeout(r, node); break;
        case 4:  decodeBlitterFeedback(r, node); break;
        case 17: decodeDotGrid(r, node); break;
        case 19: decodeDotPlaneFountain(r, node); break;
        case 5:  decodeOnBeatClear(r, node); break;
        case 6:  decodeBlur(r, node); break;
        case 9:  decodeRotoBlitter(r, node); break;
        case 11: decodeColorfade(r, node); break;
        case 15: decodeMovement(r, node); break;
        case 18: decodeBufferSave(r, node); break;
        case 22: decodeBrightness(r, node); break;
        case 25: decodeClearScreen(r, node); break;
        case 16: decodeScatter(r, node); break;
        case 20: decodeWater(r, node); break;
        case 24: decodeGrain(r, node); break;
        case 26: decodeMirror(r, node); break;
        case 27: decodeStarfield(r, node); break;
        case 29: decodeBump(r, node); break;
        case 30: decodeMosaic(r, node); break;
        case 31: decodeWaterBump(r, node); break;
        case 33: decodeCustomBpm(r, node); break;
        case 39: decodeTimescope(r, node); break;
        case 41: decodeInterferences(r, node); break;
        case 36: decodeSuperScope(r, node); break;
        case 37: decodeInvert(r, node); break;
        case 40: decodeSetRenderMode(r, node); break;
        case 0:  decodeSimple(r, node); break;
        case 2:  decodeOscStar(r, node); break;
        case 7:  decodeBassSpin(r, node); break;
        case 13: decodeRotatingStars(r, node); break;
        case 14: decodeOscRing(r, node); break;
        case 8:  decodeMovingParticle(r, node); break;
        case 12: decodeColorClip(r, node); break;
        case 23: decodeInterleave(r, node); break;
        case 38: decodeUniqueTone(r, node); break;
        case 35: decodeDynamicDistanceModifier(r, node); break;
        case 42: decodeDynamicShift(r, node); break;
        case 43: decodeDynamicMovement(r, node); break;
        case 44: decodeFastBrightness(r, node); break;
        case 45: decodeColorModifier(r, node); break;
        default: return false;
    }
    node.decoded = true;
    return true;
}

/// @brief Decode the config blob of a compiled-in builtin APE (by id string).
///        Returns true (and sets decoded) for the core-set APEs.
inline bool decodeApe(std::string_view apeId, Reader& r, EffectNode& node)
{
    if (apeId == "Channel Shift")
        readField(r, node, "mode") && readField(r, node, "onbeat");
    else if (apeId == "Color Reduction")
        readField(r, node, "levels");
    else if (apeId == "Multiplier")
        readField(r, node, "ml");
    else if (apeId == "Holden04: Video Delay")
        readField(r, node, "enabled") && readField(r, node, "usebeats") &&
            readField(r, node, "delay");
    else if (apeId == "Holden05: Multi Delay")
    {
        readField(r, node, "mode");
        readField(r, node, "activebuffer");
        for (int i = 0; i < 6; ++i)   // 6 shared buffers: usebeats + delay each
        {
            readField(r, node, ("ub" + std::to_string(i)).c_str());
            readField(r, node, ("dl" + std::to_string(i)).c_str());
        }
    }
    else if (apeId == "Color Map")
        decodeColorMap(r, node);
    else if (apeId == "Misc: Buffer blend")
        decodeBufferBlend(r, node);
    else if (apeId == "Jheriko: Global")
        decodeJherikoGlobal(r, node);
    else if (apeId == "Holden03: Convolution Filter")
        decodeConvolution(r, node);
    else if (apeId == "Trans: Normalise")
        decodeNormalise(r, node);
    else if (apeId == "Jheriko : MULTIFILTER")   // note: space before the colon
        decodeMultiFilter(r, node);
    else if (apeId == "Virtual Effect: Addborders")
        decodeAddBorders(r, node);
    else if (apeId == "VFX FRAMERATE LIMITER")
        decodeFramerateLimiter(r, node);
    else
        return false;  // unknown APE: raw blob preserved
    node.decoded = true;
    return true;
}

/// @brief Resolve an APE ID string to a builtin index (alias table), -1 if none
inline std::int32_t apeAliasToBuiltin(std::string_view apeId)
{
    for (const ApeAlias& alias : kApeAliases)
    {
        if (apeId == alias.apeId) return alias.builtinIndex;
    }
    return -1;
}

/// @brief True when the APE ID names one of the five compiled-in builtin APEs
inline bool isBuiltinApe(std::string_view apeId)
{
    for (const char* name : kBuiltinApeNames)
    {
        if (apeId == name) return true;
    }
    return false;
}

} // namespace lumi::avs::detail
