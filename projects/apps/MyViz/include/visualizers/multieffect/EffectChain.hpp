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
#include <array>
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

/**
 * True for every mode the blend engine can render. Batch 1 (E3) covered the
 * arithmetic modes; batch 2 (Session 35) added the exotic ones (Subtractive,
 * Every-other-line/-pixel, XOR, Buffer), so the whole set is now implemented.
 */
[[nodiscard]] inline bool isBlendModeImplemented(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Ignore:
        case BlendMode::Replace:
        case BlendMode::FiftyFifty:
        case BlendMode::Maximum:
        case BlendMode::Additive:
        case BlendMode::Subtractive12:
        case BlendMode::Subtractive21:
        case BlendMode::EveryOtherLine:
        case BlendMode::EveryOtherPixel:
        case BlendMode::Xor:
        case BlendMode::Adjustable:
        case BlendMode::Multiply:
        case BlendMode::Buffer:
        case BlendMode::Minimum:
            return true;
    }
    return false;
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

    // Buffer blend mode (E3 batch 2): a global buffer's per-pixel depth drives
    // the mix. Index = OffscreenBufferPool slot (0..7, same numbering as Buffer
    // Save); invert flips depth (AVS ininvert/outinvert).
    int bufferIn = 0;             ///< pool slot for Buffer in-blend
    int bufferOut = 0;            ///< pool slot for Buffer out-blend
    bool bufferInInvert = false;  ///< invert in-blend buffer depth
    bool bufferOutInvert = false; ///< invert out-blend buffer depth

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
    /// 0 = replace, 1 = additive, 2 = 50/50, 3 = current line-blend
    /// (r_clear.cpp: blend==1 -> BLEND, blendavg -> BLEND_AVG, blend==2 -> BLEND_LINE)
    int blend = 0;
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

/**
 * AVS "Render / Simple" (ID 0, r_simple.cpp): the classic scope — an oscilloscope
 * (waveform) or analyzer (spectrum), drawn as lines or dots, at a screen position,
 * colour-cycled through `colors`. `source` 0 spectrum, 1 waveform.
 */
struct SimpleScopeParams
{
    int source = 1;    ///< 0 analyzer (spectrum), 1 oscilloscope (waveform)
    int channel = 2;   ///< 0 L, 1 R, 2 center
    int position = 2;  ///< 0 top, 1 bottom, 2 center
    int drawMode = 0;  ///< 0 lines, 1 dots
    std::vector<uint32_t> colors{0xFFFFFF};  ///< cycled colour table
};

/**
 * AVS "Render / Oscilliscope Star" (ID 2, r_oscstar.cpp): the waveform drawn along
 * 5 rotating radial spokes. `size`/`rot` 0..16; colour-cycled through `colors`.
 */
struct OscStarParams
{
    int channel = 2;   ///< 0 L, 1 R, 2 center
    int position = 2;  ///< 0 left, 1 right, 2 center
    int size = 8;      ///< spoke length 0..16
    int rot = 3;       ///< rotation speed 0..16 (8 = still)
    std::vector<uint32_t> colors{0xFFFFFF};
};

/**
 * AVS "Render / Ring" (ID 14, r_oscring.cpp): the waveform/spectrum drawn as a
 * closed 80-segment ring whose radius is modulated by the audio. `source`
 * 0 waveform, 1 spectrum. `size` 0..16.
 */
struct OscRingParams
{
    int source = 0;    ///< 0 waveform, 1 spectrum
    int channel = 2;   ///< 0 L, 1 R, 2 center
    int position = 2;  ///< 0 left, 1 right, 2 center
    int size = 8;      ///< base radius 0..16
    std::vector<uint32_t> colors{0xFFFFFF};
};

/**
 * AVS "Render / Rotating Stars" (ID 13, r_rotstar.cpp): two 5-pointed stars
 * orbiting the centre, sized by the peak spectrum energy; colour-cycled.
 */
struct RotatingStarsParams
{
    std::vector<uint32_t> colors{0xFFFFFF};
};

/**
 * AVS "Render / Bass Spin" (ID 7, r_bspin.cpp): two bass-reactive spinning shapes
 * (left/right channel), drawn in the left/right screen half. `mode` 0 lines,
 * 1 filled (line-approximated here). Spin speed follows the bass energy.
 */
struct BassSpinParams
{
    bool left = true;
    bool right = true;
    uint32_t colorLeft = 0xFFFFFF;
    uint32_t colorRight = 0xFFFFFF;
    int mode = 1;  ///< 0 lines, 1 filled (approximated as lines)
};

/**
 * AVS "Trans / Color Clip" (ID 12, r_contrast.cpp): replace pixels matching a
 * colour threshold with `outColor`. `mode` 1 below (all channels <= clip),
 * 2 above (all >= clip), 3 near (within `distance` of clip). COLORREF-swapped.
 */
struct ColorClipParams
{
    int mode = 1;                    ///< 1 below, 2 above, 3 near
    uint32_t clipColor = 0x202020;   ///< threshold colour 0x00RRGGBB
    uint32_t outColor = 0x202020;    ///< replacement colour
    int distance = 10;               ///< match radius (near mode)
};

/**
 * AVS "Trans / Unique Tone" (ID 38, r_onetone.cpp): tint the image to a single
 * hue — output = `color` * luminance(max channel), optionally inverted. `blend`
 * 0 replace, 1 additive, 2 50/50.
 */
struct UniqueToneParams
{
    uint32_t color = 0xFFFFFF;  ///< tone colour 0x00RRGGBB
    bool invert = false;        ///< invert the depth ramp
    int blend = 0;              ///< 0 replace, 1 additive, 2 50/50
};

/**
 * AVS "Trans / Interleave" (ID 23, r_interleave.cpp): overlay a stripe/grid of
 * `color` at spacings `x`/`y` (0 = that axis off). On beat the spacing eases to
 * `x2`/`y2` over `beatDuration`. `blend` 0 replace, 1 additive, 2 50/50.
 */
struct InterleaveParams
{
    int x = 1;                  ///< horizontal stripe spacing (px, 0 = off)
    int y = 1;                  ///< vertical stripe spacing (px, 0 = off)
    uint32_t color = 0x000000;  ///< stripe colour 0x00RRGGBB
    int blend = 0;              ///< 0 replace, 1 additive, 2 50/50
    bool onBeat = false;        ///< ease to x2/y2 on beat
    int x2 = 1;
    int y2 = 1;
    int beatDuration = 4;       ///< ease length
};

/**
 * AVS "Render / Picture" (ID 34, r_picture.cpp): display a bitmap over the frame.
 * The original referenced an external file (`filename`); on import the resolved
 * image bytes are base64-embedded into `imageData` (decision: self-contained
 * .lvfx). `blend` 0 replace, 1 additive, 2 50/50; `keepAspect` letterboxes.
 * Empty `imageData` (file not found) renders as a no-op.
 */
struct PictureParams
{
    std::string filename;    ///< original AVS filename (reference only)
    std::string imageData;   ///< base64 of the raw image file ("" = unresolved)
    int blend = 2;           ///< 0 replace, 1 additive, 2 50/50
    bool keepAspect = true;  ///< preserve the image aspect (letterbox)
};

/**
 * AVS APE "Picture II": display an embedded image stretched to fill, blended per
 * `blend` (community APE). Like Picture but always fills the frame (bilinear).
 */
struct PictureIIParams
{
    std::string filename;
    std::string imageData;   ///< base64 of the raw image ("" = unresolved)
    int blend = 2;           ///< 0 replace, 1 additive, 2 50/50
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
    /// r_mirror direction bits: 1 = top->bottom (HORIZONTAL1),
    /// 2 = bottom->top (HORIZONTAL2), 4 = left->right (VERTICAL1),
    /// 8 = right->left (VERTICAL2)
    int mode = 4;
    bool onBeatRandom = false;  ///< randomize active edges on beat
    bool smooth = false;        ///< gradual transition (BLEND_ADAPT ramp)
    int slower = 4;             ///< frames per ramp step (1..16)
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
    bool blend = false;       ///< 50/50 blend of moved pixel with original (r_trans)
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
    /// r_dmove flags: `blend` mixes the moved pixel onto the original by the
    /// script's per-cell alpha; `nomove` skips displacement and only alpha-fades.
    bool blend = false;
    bool nomove = false;
};

/**
 * AVS "Trans / Dynamic Distance Modifier" (ID 35): a scripted RADIAL remap. The
 * pixel EEL runs once per distance ring (`d` normalized 0..1 in/out); the host
 * builds a 1-D distance→distance LUT and a shader resamples each pixel at the
 * remapped distance along its angle (r_ddm.cpp). `blend` = 50/50 with the
 * original; `bilinear` = subpixel sampling. Vars: d (in/out), b, plus init/frame.
 */
struct DynamicDistanceModifierParams
{
    std::string initCode = "u=1;t=0";
    std::string frameCode =
        "t=t+u;t=min(100,t);t=max(0,t);u=if(equal(t,100),-1,u);u=if(equal(t,0),1,u)";
    std::string beatCode;
    std::string pixelCode = "d=d-sigmoid((t-50)/100,2)";
    bool blend = false;    ///< 50/50 with the original image
    bool bilinear = true;  ///< subpixel (bilinear) sampling
};

/**
 * AVS "Render / Moving Particle" (ID 8): a single spring-driven particle that
 * bounces around (target re-randomized on beat) and is drawn as a filled circle
 * (r_parts.cpp). `maxDistance` scales the travel radius; `size`/`size2` the
 * radius (size2 on beat when `onBeatSize`); `blend` 0 replace, 1 additive,
 * 2 50/50, 3 line (~additive). Rendered as a sized dot via the ScopeRenderer.
 */
struct MovingParticleParams
{
    uint32_t color = 0xFFFFFF;  ///< particle colour 0x00RRGGBB
    int maxDistance = 16;       ///< travel-radius scale (AVS maxdist)
    int size = 8;               ///< particle radius (px)
    int size2 = 8;              ///< on-beat radius
    bool onBeatSize = false;    ///< jump to size2 on beat (AVS enabled bit 1)
    int blend = 1;              ///< 0 replace, 1 additive, 2 50/50, 3 line
};

/**
 * AVS "Trans / Dynamic Shift" (ID 42): a scripted GLOBAL image translation. The
 * frame/beat EEL sets `x,y` (pixel offset); the whole image is shifted by (x,y)
 * with black fill (decision: uniform offset, not a grid remap — r_shift is
 * image-global affine). `blend` = 50/50 with the original by `alpha`; `bilinear`
 * = subpixel sampling. Variables: x,y (out, pixels), w,h, b, alpha (default 0.5).
 */
struct DynamicShiftParams
{
    std::string initCode = "d=0;";
    std::string frameCode = "x=sin(d)*1.4; y=1.4*cos(d); d=d+0.01;";
    std::string beatCode = "d=d+2.0";
    bool blend = false;    ///< 50/50 with the original image
    bool bilinear = true;  ///< subpixel (bilinear) sampling
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
    /// Direction (r_stack.cpp): 0 = save, 1 = restore,
    /// 2 = alternate starting with save, 3 = alternate starting with restore
    int dir = 0;
    BlendMode blend = BlendMode::Replace;  ///< blend mode (applies both directions)
    int adjustAlpha = 128;                 ///< Adjustable blend alpha 0..255
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
 * AVS "Render / Set Render Mode" (ID 40): a stateful command that sets the line
 * width + blend used by the render effects that FOLLOW it in the chain (it draws
 * nothing itself). Modelled as a live host node — at render time it writes the
 * host's current render mode, which the following scope effects read (the AVS
 * behaviour; previously import-time-unrolled into the next SuperScope). `enabled`
 * = AVS bit 31 (override the blend; when false the blend falls back to additive);
 * `lineWidth` px (0 = leave each effect's own); `lineBlend` 0 replace, 1 additive,
 * 2 50/50; `adjustAlpha` the Adjustable-blend alpha 0..255 (bits 8-15).
 */
struct SetRenderModeParams
{
    bool enabled = true;    ///< override the blend (AVS bit 31)
    int lineWidth = 1;      ///< line width for following scopes (px, 0 = leave)
    int lineBlend = 1;      ///< 0 replace, 1 additive, 2 50/50
    int adjustAlpha = 128;  ///< Adjustable-blend alpha 0..255
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
    int lineBlend = 1;     ///< onto framebuffer: 0 replace, 1 additive, 2 50/50
                           ///< (set by a preceding Set Render Mode; default additive)

    // Base color (point code that sets red/green/blue always overrides this).
    // Two orthogonal sources — a time-cycled AVS color table and a per-point
    // gradient — combined by `colorBlend`. Default (0 + Neon) = the historical
    // look, so existing chains render unchanged.
    int colorBlend = 0;  ///< 0 gradient, 1 table, 2 additive, 3 multiply, 4 average
    std::vector<uint32_t> colors;         ///< AVS color table (0x00RRGGBB), cycled
    int colorCycleFrames = 60;            ///< frames per table step (>= 1)
    std::string gradientPreset = "Neon";  ///< ColorGradientModule preset name
};

/**
 * AVS "Trans / Mosaic" (ID 30): pixelate into `quality`×`quality` blocks. On a
 * beat it can jump to `quality2` and ease linearly back to `quality` over
 * `durationFrames` (r_mosaic.cpp). quality 100 = no pixelation. `blend` mixes
 * the mosaic with the untouched image: 0 replace, 1 additive, 2 50/50.
 */
struct MosaicParams
{
    int quality = 50;         ///< block resolution 1..100 (100 = off)
    int quality2 = 50;        ///< on-beat resolution 1..100
    bool onBeat = false;      ///< jump to quality2 on beat, then ease back
    int durationFrames = 16;  ///< ease-back length in frames (>= 1)
    int blend = 0;            ///< 0 replace, 1 additive, 2 50/50
};

/**
 * AVS "Trans / Grain" (ID 24): darken a random subset of (non-black) pixels by
 * a random factor (r_grain.cpp). `amount` 0..100 = the gated fraction; `blend`
 * 0 replace, 1 additive, 2 50/50; `staticGrain` freezes the noise pattern.
 */
struct GrainParams
{
    int amount = 100;         ///< gated pixel fraction 0..100 (smax)
    bool staticGrain = false; ///< frozen noise vs. per-frame shimmer
    int blend = 0;            ///< 0 replace, 1 additive, 2 50/50
};

/** AVS "Trans / Scatter" (ID 16): per-pixel random displacement within a small
 *  (~4 px) window (r_scat.cpp). No parameters. */
struct ScatterParams
{
};

/** AVS "Trans / Water" (ID 20): color-space water ripple — neighbour average of
 *  the current frame minus the previous frame (r_water.cpp). No parameters;
 *  needs a persistent per-node "last frame" buffer. */
struct WaterParams
{
};

/**
 * AVS "Trans / Water Bump" (ID 31): a height-field water simulation — waves
 * propagate on a persistent height buffer and refract the image (r_waterbump).
 * On a beat a drop is added (`randomDrop` = random spot, else `dropX/dropY`
 * position code 0/1/2 = near/mid/far). `density` damps the waves, `depth` sets
 * the drop strength. `displaceScale` tunes the refraction (sight-test).
 */
struct WaterBumpParams
{
    int density = 5;        ///< wave damping (higher = longer-lived waves)
    int depth = 600;        ///< drop amplitude
    bool randomDrop = true; ///< random drop spot vs. fixed dropX/dropY
    int dropX = 1;          ///< 0 near / 1 mid / 2 far (position code)
    int dropY = 1;
    int dropRadius = 40;    ///< drop radius (px)
    float displaceScale = 6.0f;  ///< refraction strength (host tuning)
};

/**
 * AVS "Trans / Bump" (ID 29): per-pixel bump lighting from the image luminance
 * gradient, lit by a movable light source (r_bump.cpp). The light position
 * `x,y` comes from EEL (init/frame/beat, `x,y` output; `t` script-owned). On a
 * beat `depth` can jump to `depth2` for `durationFrames`. `blend` 0 replace,
 * 1 additive, 2 50/50. `oldStyle` scales script x,y by 1/100 (legacy presets).
 */
struct BumpParams
{
    int depth = 30;            ///< bump strength 0..100
    int depth2 = 100;          ///< on-beat strength
    bool onBeat = false;       ///< jump to depth2 on beat, ease back
    int durationFrames = 15;   ///< ease-back length
    bool invert = false;       ///< invert the depth (luminance)
    bool oldStyle = false;     ///< legacy x,y in 0..100 instead of 0..1
    int blend = 0;             ///< 0 replace, 1 additive, 2 50/50
    std::string initCode = "t=0;";
    std::string frameCode = "x=0.5+cos(t)*0.3;\ny=0.5+sin(t)*0.3;\nt=t+0.1;";
    std::string beatCode;
};

/**
 * AVS "Trans / Interferences" (ID 41): `points` rotated copies of the image
 * accumulated with `alpha` (r_interf.cpp). The rotation advances by
 * `rotationInc`/frame; on a beat it morphs to the *2 parameter set over
 * `speed`. `rgb` splits copies across R/G/B channels. `blend` 0 replace,
 * 1 additive, 2 50/50 with the original.
 */
struct InterferencesParams
{
    int points = 2;          ///< number of copies 1..8
    int distance = 10;       ///< copy offset radius (px)
    int alpha = 128;         ///< per-copy weight 0..255
    int rotation = 0;        ///< initial rotation 0..255 (of a full turn)
    int rotationInc = 0;     ///< rotation delta per frame
    int distance2 = 32;      ///< on-beat target distance
    int alpha2 = 192;        ///< on-beat target alpha
    int rotationInc2 = 25;   ///< on-beat target rotation delta
    bool rgb = false;        ///< split copies across channels
    bool onBeat = false;     ///< morph to the *2 set on a beat
    float speed = 0.2f;      ///< beat-morph transition speed
    int blend = 0;           ///< 0 replace, 1 additive, 2 50/50
};

/**
 * AVS "Render / Starfield" (ID 27): a 3D star field flying towards the viewer
 * (r_stars.cpp). Stars move by `warpSpeed` (jumping to `beatSpeed` for
 * `durationFrames` on a beat); brightness rises as they approach. `color`
 * tints them. Drawn additively via the shared ScopeRenderer.
 */
/**
 * Community APE "FunkyFX FyrewurX v1" (closed source) — behavioral REBUILD, no
 * original code: on-beat firework bursts of gravity-bound sparks, drawn as
 * additive dots. The original's config word was never varied in the wild (all
 * 146 corpus instances carry identical bytes), so the parameters here are our
 * own, sight-calibrated ones.
 */
struct FyrewurXParams
{
    int sparks = 80;           ///< sparks per burst
    float speed = 0.7f;        ///< initial spark speed scale (NDC/s)
    float gravity = 0.8f;      ///< downward pull (NDC/s^2)
    float lifeSeconds = 1.6f;  ///< spark lifetime
};

struct StarfieldParams
{
    uint32_t color = 0xFFFFFF;   ///< tint 0x00RRGGBB
    float warpSpeed = 6.0f;      ///< base fly-through speed
    int maxStars = 350;          ///< star count
    bool onBeat = false;         ///< jump to beatSpeed on a beat
    float beatSpeed = 4.0f;      ///< on-beat speed
    int durationFrames = 15;     ///< ease-back length
    int blend = 0;               ///< 0 replace, 1 additive, 2 50/50 (r_stars)
};

/**
 * AVS "Render / Timescope" (ID 39): a scrolling spectrogram — one spectrum
 * column is drawn per frame at an advancing x, building up over time
 * (r_timescope.cpp). `color` tints it; `bands` sets the vertical resolution;
 * `blend` 0 replace, 1 additive, 2 50/50 with the existing image.
 */
struct TimescopeParams
{
    uint32_t color = 0xFFFFFF;  ///< tint 0x00RRGGBB
    int blend = 0;              ///< 0 replace, 1 additive, 2 50/50
    int channel = 2;           ///< 0 L, 1 R, 2 center
    int bands = 576;           ///< vertical spectrum resolution
};

/** AVS "Render / Dot Grid" (ID 17): a scrolling grid of dots whose colour cycles
 *  through `colors` (r_dotgrid.cpp). `spacing` px, `xMove/yMove` scroll speed. */
struct DotGridParams
{
    std::vector<uint32_t> colors{0xFFFFFF};  ///< cycled colour table
    int spacing = 8;   ///< grid spacing (px)
    int xMove = 128;   ///< horizontal scroll (fixed-point /256 per frame)
    int yMove = 128;   ///< vertical scroll
    int blend = 0;     ///< 0 replace, 1 additive, 2 50/50
};

/** AVS "Render / Dot Plane" (ID 1): a rotating audio-reactive point plane, height
 *  from the spectrum, coloured by a 5-stop gradient (r_dotpln.cpp). 3D projection
 *  scale is host tuning (sight-test). */
struct DotPlaneParams
{
    uint32_t colors[5] = {0x0000FF, 0x00FFFF, 0x00FF00, 0xFFFF00, 0xFF0000};
    int rotVel = 16;   ///< rotation speed (-50..50 in AVS)
    int angle = -20;   ///< viewing tilt angle
};

/** AVS "Render / Dot Fountain" (ID 19): a 3D particle fountain coloured by a
 *  5-stop gradient, rotating (r_dotfnt.cpp). Simplified particle model here;
 *  projection/physics scale is host tuning (sight-test). */
struct DotFountainParams
{
    uint32_t colors[5] = {0x0000FF, 0x00FFFF, 0x00FF00, 0xFFFF00, 0xFF0000};
    int rotVel = 16;
    int angle = -20;
};

/**
 * AVS APE "Misc: Buffer blend": combine two buffers (A and B) with a blend mode
 * and output the result (community APE, format per grandchild/AVS-File-Decoder).
 * `bufferA`/`bufferB` select a global pool slot 0..7 or 8 = CURRENT frame; `mode`
 * 0..10 (replace/additive/max/50-50/sub d-s/sub s-d/multiply/adjustable/xor/min/
 * abs-diff). Uses the shared OffscreenBufferPool (same slots as Buffer Save).
 */
struct BufferBlendParams
{
    int bufferA = 8;  ///< 0..7 pool slot, 8 = current frame
    int bufferB = 8;  ///< 0..7 pool slot, 8 = current frame
    int mode = 0;     ///< BufferBlendMode 0..10
};

/**
 * AVS APE "Jheriko: Global": runs EEL that sets preset-global registers/gmegabuf,
 * persisting them across frames for other effects (community APE). Init/frame/beat
 * code shares this preset's ScriptContext. `loadMode` 0 none, 1 once, 2 code, 3
 * every-frame. File I/O + save-ranges are not imported. No visual output.
 */
struct JherikoGlobalParams
{
    int loadMode = 1;  ///< 0 none, 1 once, 2 code-control, 3 every-frame
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Holden03: Convolution Filter": a 7x7 convolution kernel over the image
 * (community APE). `edgeMode` 0 extend (clamp), 1 wrap. `absolute` takes |result|.
 * `twoPass` applies the kernel twice. `kernel` is 49 ints (row-major); the sum is
 * divided by `scale` (0 -> 1) and offset by `bias`.
 */
struct ConvolutionParams
{
    bool absolute = false;
    bool twoPass = false;
    int edgeMode = 0;   ///< 0 extend, 1 wrap
    int bias = 0;
    int scale = 1;
    std::array<int, 49> kernel = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  ///< identity (center=1)
};

/**
 * AVS APE "Trans: Normalise": auto-levels — stretch the image so the darkest
 * pixel becomes black and the brightest white (community APE). No parameters
 * (enabled maps to the node's enabled flag). Min/max are measured on the GPU
 * frame each render.
 */
struct NormaliseParams
{
};

/**
 * AVS APE "Jheriko : MULTIFILTER" (note the space before the colon): applies one
 * of a few fixed pixel filters (community APE, closed source — the exact math is
 * approximated). `effect` 0 chrome, 1 double, 2 triple, 3 infinite-root. `onBeat`
 * only applies on beat frames.
 */
struct MultiFilterParams
{
    int effect = 0;       ///< 0 chrome, 1 double chrome, 2 triple chrome, 3 root
    bool onBeat = false;  ///< apply only on beat frames
};

/**
 * AVS APE "Virtual Effect: Addborders": draw a solid border of `color` and
 * `size` pixels around the image (community APE).
 */
struct AddBordersParams
{
    uint32_t color = 0xFFFFFF;  ///< border colour 0x00RRGGBB
    int size = 2;               ///< border width (px)
};

/**
 * AVS APE "Texer": draw an embedded image (sprite) at points along the waveform
 * (community APE, no EEL). `blend` 0 replace, 1 additive, 2 50/50; `particles`
 * = number of sprites. Sprite geometry is sight-test-calibrated.
 */
struct TexerParams
{
    std::string filename;
    std::string imageData;
    int blend = 1;
    int particles = 100;
};

/**
 * AVS APE "Acko.net: Texer II": draw an embedded image at each point of a scoped
 * point loop (community APE). The point EEL sets, per point, `x,y` (-1..1),
 * `sizex,sizey` (scale), `red,green,blue` (tint when colorFiltering); `n` = point
 * count. EEL variable contract + sprite geometry are sight-test-calibrated.
 */
struct TexerIIParams
{
    std::string filename;
    std::string imageData;
    bool resizing = false;
    bool wrapAround = false;
    bool colorFiltering = true;
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pointCode = "x=(i*2)-1; y=0;";
};

/**
 * AVS APE "Render: Triangle": draw EEL-scripted filled triangles. The point EEL
 * sets, per triangle, `x1,y1,x2,y2,x3,y3` (-1..1) and `red,green,blue`; `n` =
 * triangle count. EEL variable contract is sight-test-calibrated.
 */
struct TriangleParams
{
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pointCode;
};

/**
 * AVS APE "Color Map": map a per-pixel input value (selected channel) through a
 * gradient LUT, then blend the result onto the image (UnConeD, community APE —
 * format per grandchild/AVS-File-Decoder). `key` picks the input (0 red, 1 green,
 * 2 blue, 3 (r+g+b)/2, 4 max, 5 (r+g+b)/3). `blendMode` 0..9 (replace/additive/
 * max/min/50-50/sub d-s/sub s-d/multiply/xor/adjustable). The gradient is a list
 * of stops (position 0..255 + 0x00RRGGBB); the host interpolates a 256-entry LUT.
 * On-beat map cycling (mapCycleMode) is not imported yet — the first enabled map
 * is used.
 */
struct ColorMapParams
{
    int key = 0;              ///< input selector 0..5
    int blendMode = 0;        ///< 0..9 (see above)
    int adjustBlend = 128;    ///< ADJUSTABLE weight 0..255
    std::vector<int> stopPos;         ///< gradient stop positions 0..255
    std::vector<uint32_t> stopColor;  ///< gradient stop colours 0x00RRGGBB
};

/** AVS APE "Channel Shift": permute the R/G/B channels (r_chanshift). `mode`
 *  0 RGB, 1 RBG, 2 GBR, 3 GRB, 4 BRG, 5 BGR; `onBeat` picks a random one each beat. */
struct ChannelShiftParams
{
    int mode = 1;         ///< channel permutation 0..5
    bool onBeat = false;  ///< random permutation on each beat
};

/** AVS APE "Color Reduction": quantise each channel to 2^`levels` values
 *  (r_colorreduction). levels 1..8 (8 = unchanged). */
struct ColorReductionParams
{
    int levels = 8;  ///< bit depth per channel 1..8
};

/** AVS APE "Multiplier": scale pixel values (r_multiplier). `mode` 0 saturate,
 *  1 x8, 2 x4, 3 x2, 4 x0.5, 5 x0.25, 6 x0.125, 7 zero-else-keep. */
struct MultiplierParams
{
    int mode = 3;  ///< 0..7 (see above)
};

/** AVS APE "Video Delay" (Holden04): output the image from `delay` frames ago
 *  via a per-node frame ring buffer (r_videodelay). `useBeats` measures the
 *  delay in beats instead of frames. */
struct VideoDelayParams
{
    bool useBeats = false;  ///< delay in beats vs frames
    int delay = 10;         ///< delay amount (frames, capped for VRAM)
};

/** AVS APE "Multi Delay" (Holden05): one of 6 host-shared frame ring buffers
 *  (r_multidelay). `mode` 0 none, 1 input (write this frame), 2 output (read the
 *  delayed frame). `buffer` 0..5 selects the shared ring; `delay`/`useBeats` set
 *  its length (input nodes size the ring). */
struct MultiDelayParams
{
    int mode = 0;           ///< 0 none, 1 input, 2 output
    int buffer = 0;         ///< shared buffer index 0..5
    int delay = 10;         ///< delay length
    bool useBeats = false;  ///< delay in beats vs frames
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

/**
 * Batch H (host-native, no AVS origin): an escape-time 2-D fractal generator.
 * A fullscreen fragment shader evaluates the iteration per pixel and colours it
 * through a gradient LUT (smooth iteration count). `type` selects the formula
 * (Mandelbrot, Julia, Burning Ship, Tricorn, Multibrot, Newton, Phoenix, Magnet,
 * Nova). The optional EEL slots (init/frame/beat) drive the view live: they read
 * audio (bass/mid/treble/vol, beat, time) and write cx,cy,zoom,rot,jx,jy,power —
 * so a preset can zoom, morph the Julia seed, spin or pulse to the beat. `blend`
 * composites the result over the current image (0 replace, 1 additive, 2 50/50).
 */
struct Fractal2DParams
{
    int type = 0;           ///< 0 Mandelbrot 1 Julia 2 Burning Ship 3 Tricorn
                            ///< 4 Multibrot 5 Newton 6 Phoenix 7 Magnet 8 Nova
    float centerX = -0.5f;  ///< view centre (real part)
    float centerY = 0.0f;   ///< view centre (imag part)
    float zoom = 1.0f;      ///< view scale (>1 magnifies)
    float rotation = 0.0f;  ///< view rotation (radians)
    int maxIter = 128;      ///< escape-time iteration cap
    float juliaX = -0.8f;   ///< Julia / Phoenix seed (real)
    float juliaY = 0.156f;  ///< Julia / Phoenix seed (imag)
    float power = 2.0f;     ///< Multibrot / Nova exponent
    float escapeR = 4.0f;   ///< escape radius (|z|^2 threshold)

    bool smooth = true;               ///< continuous (fractional) iteration colouring
    float colorScale = 0.05f;         ///< iteration→LUT position factor
    float colorCycle = 0.0f;          ///< LUT offset advance per second (palette drift)
    uint32_t insideColor = 0x000000;  ///< colour of the non-escaping set (0x00RRGGBB)
    std::string gradientPreset = "Neon";  ///< ColorGradientModule palette name

    int blend = 0;  ///< 0 replace, 1 additive, 2 50/50 over the current image

    std::string initCode;   ///< EEL, once (seed cx,cy,zoom,…)
    std::string frameCode;  ///< EEL, per frame (audio-reactive view)
    std::string beatCode;   ///< EEL, on beat
};

/**
 * Batch H (host-native): a domain-warped fBm field — the "plasma / nebula" look.
 * A fullscreen shader sums `octaves` of value noise (fBm), warps the sample point
 * by two further fBm fields (`warp` strength), animates it over time (`speed`) and
 * colours the result through the shared gradient LUT. Cheap and very audio-reactive:
 * the optional EEL slots read audio (bass/mid/treble/vol/beat/time) and write
 * scale,warp,speed,ox,oy. `blend` composites over the current image.
 */
struct DomainWarpParams
{
    int octaves = 5;          ///< fBm octave count (1..10)
    float lacunarity = 2.0f;  ///< frequency multiplier per octave
    float gain = 0.5f;        ///< amplitude falloff per octave
    float scale = 3.0f;       ///< base spatial frequency
    float warp = 0.5f;        ///< domain-warp strength
    float warpScale = 1.0f;   ///< warp-field frequency relative to base
    float speed = 0.2f;       ///< time-evolution speed
    float offsetX = 0.0f;     ///< pan (x)
    float offsetY = 0.0f;     ///< pan (y)

    float colorScale = 1.0f;  ///< field→LUT position factor
    float colorCycle = 0.0f;  ///< LUT offset advance per second
    std::string gradientPreset = "Neon";  ///< ColorGradientModule palette

    int blend = 0;  ///< 0 replace, 1 additive, 2 50/50 over the current image

    std::string initCode;   ///< EEL, once
    std::string frameCode;  ///< EEL, per frame (audio-reactive field)
    std::string beatCode;   ///< EEL, on beat
};

/**
 * Batch H (host-native): a raymarched 3-D distance-estimator fractal. A fullscreen
 * shader marches a ray per pixel through a signed-distance field and shades the hit
 * with a normal-lit gradient. `type` selects the field (Mandelbulb, Mandelbox,
 * Menger sponge, Quaternion-Julia, KIFS). EEL (init/frame/beat) reads audio and
 * writes yaw,pitch,dist,power,scale,fold for live camera / morph. `blend`
 * composites over the current image.
 */
struct Fractal3DParams
{
    int type = 0;         ///< 0 Mandelbulb 1 Mandelbox 2 Menger 3 Quaternion-Julia 4 KIFS
    float yaw = 0.6f;     ///< camera orbit azimuth (radians)
    float pitch = 0.3f;   ///< camera orbit elevation (radians)
    float dist = 3.2f;    ///< camera distance from origin
    float fov = 1.0f;     ///< field-of-view scale
    float power = 8.0f;   ///< Mandelbulb exponent
    float scale = 2.0f;   ///< Mandelbox / KIFS scale
    float fold = 1.0f;    ///< Mandelbox fold limit
    int maxSteps = 96;    ///< raymarch step cap
    int maxIter = 8;      ///< DE iteration count
    float juliaX = 0.2f;  ///< Quaternion-Julia seed
    float juliaY = 0.3f;
    float juliaZ = 0.1f;
    float juliaW = 0.0f;
    float lightYaw = 0.7f;    ///< key-light direction (azimuth)
    float lightPitch = 0.8f;  ///< key-light direction (elevation)
    float ambient = 0.2f;     ///< ambient floor
    bool ao = true;           ///< cheap ambient occlusion

    float colorScale = 1.0f;
    float colorCycle = 0.0f;
    std::string gradientPreset = "Neon";
    uint32_t background = 0x000000;  ///< miss colour (0x00RRGGBB)

    int blend = 0;
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): the Markus-Lyapunov fractal ("Zircon Zity"). For each
 * point of an (a,b) plane the logistic map is iterated with r alternating between
 * a and b along the `sequence` string (e.g. "AB"), and the Lyapunov exponent is
 * accumulated: negative (stable/ordered) zones get `negColor`, positive (chaotic)
 * zones map through the gradient. EEL writes the view rectangle (aMin/aMax/bMin/bMax).
 */
struct LyapunovParams
{
    std::string sequence = "AB";  ///< a/b pattern (A/B chars; others ignored)
    float aMin = 2.5f;            ///< view rectangle (a axis)
    float aMax = 4.0f;
    float bMin = 2.5f;            ///< view rectangle (b axis)
    float bMax = 4.0f;
    int warmup = 100;            ///< settle iterations before measuring
    int iterations = 400;        ///< measured iterations
    uint32_t negColor = 0x000030;  ///< ordered-zone colour (negative exponent)

    float colorScale = 1.0f;
    float colorCycle = 0.0f;
    std::string gradientPreset = "Fire";

    int blend = 0;
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a hyperbolic {p,q} tiling on the Poincaré disk, rendered
 * by repeated inversion into the fundamental domain. `morph` animates the tiling,
 * `zoom`/`rotation` frame it. Colour comes from the reflection count through the
 * gradient LUT. EEL writes p,q (as floats),morph,zoom,rotation.
 */
struct KleinianParams
{
    int p = 5;            ///< polygon sides
    int q = 4;            ///< polygons per vertex (1/p + 1/q < 1/2 → hyperbolic)
    int iterations = 30;  ///< inversion iteration cap
    float morph = 0.0f;   ///< tiling morph phase
    float zoom = 1.0f;
    float rotation = 0.0f;

    float colorScale = 1.0f;
    float colorCycle = 0.0f;
    std::string gradientPreset = "Neon";

    int blend = 0;
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): an endless-zoom trip — an escape-time fractal whose zoom
 * (and optional rotation) advances every frame, blended with the previous frame
 * (`feedback`) for motion trails. `type` 0 Mandelbrot 1 Julia 2 Burning Ship. EEL
 * writes centerX,centerY (zoom target),zoomSpeed,rotationSpeed.
 */
struct FractalZoomerParams
{
    int type = 0;            ///< 0 Mandelbrot 1 Julia 2 Burning Ship
    float centerX = -0.743643887f;  ///< zoom target (a classic Misiurewicz point)
    float centerY = 0.131825904f;
    float juliaX = -0.8f;
    float juliaY = 0.156f;
    int maxIter = 200;
    float zoomSpeed = 1.02f;      ///< per-frame zoom factor (>1 zooms in)
    float rotationSpeed = 0.0f;   ///< radians per frame
    float feedback = 0.5f;        ///< trail persistence 0..1 (0 = no trails)

    float colorScale = 0.05f;
    float colorCycle = 0.0f;
    std::string gradientPreset = "Neon";
    uint32_t insideColor = 0x000000;

    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a strange-attractor point cloud. Each frame iterates the
 * chosen map for `points` steps and draws the orbit as additive dots (shared
 * ScopeRenderer). `type` 0 Lorenz 1 Clifford 2 De Jong 3 Aizawa; a,b,c,d are the
 * map coefficients. EEL writes a,b,c,d,rotation for live morphing.
 */
struct StrangeAttractorParams
{
    int type = 0;      ///< 0 Lorenz 1 Clifford 2 De Jong 3 Aizawa
    float a = 1.4f;
    float b = 1.6f;
    float c = 1.0f;
    float d = 0.7f;
    int points = 6000;
    float scale = 0.28f;
    float rotation = 0.0f;
    float rotationSpeed = 0.08f;

    uint32_t color = 0x66CCFF;
    bool useGradient = true;      ///< colour dots along the orbit via the gradient
    std::string gradientPreset = "Neon";
    float dotSize = 2.0f;
    int blend = 1;                ///< 0 replace, 1 additive, 2 50/50

    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a Flame / IFS (chaos-game) renderer. Each frame runs the
 * chaos game for `points` iterations over a small affine function system with a
 * non-linear `variation`, drawing the samples as additive dots. `functions`
 * selects the preset system (2..4 maps). EEL writes variation params / rotation.
 */
struct FlameParams
{
    int variation = 0;   ///< 0 linear 1 sinusoidal 2 spherical 3 swirl 4 horseshoe
    int functions = 3;   ///< affine map count (2..4)
    int points = 20000;
    float scale = 0.5f;
    float rotation = 0.0f;
    float rotationSpeed = 0.04f;

    std::string gradientPreset = "Fire";
    float dotSize = 1.5f;
    int blend = 1;

    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a Gray-Scott reaction-diffusion simulation on a persistent
 * ping-pong buffer (chemical A in .r, B in .g). `stepsPerFrame` sim iterations run
 * per frame; B's concentration maps through the gradient LUT. On a beat a fresh
 * seed blob is stamped (`seedOnBeat`). EEL writes feed,kill for live pattern shifts.
 */
struct ReactionDiffusionParams
{
    float feed = 0.055f;      ///< feed rate
    float kill = 0.062f;      ///< kill rate
    float diffA = 1.0f;       ///< diffusion rate of A
    float diffB = 0.5f;       ///< diffusion rate of B
    int stepsPerFrame = 8;    ///< sim iterations per rendered frame
    bool seedOnBeat = true;   ///< stamp a new seed blob on a beat

    float colorScale = 1.0f;
    float colorCycle = 0.0f;
    std::string gradientPreset = "Neon";

    int blend = 0;
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
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
                 MovementParams, DynamicMovementParams, DynamicShiftParams,
                 DynamicDistanceModifierParams, MovingParticleParams,
                 BlitterFeedbackParams,
                 RotoBlitterParams, BufferSaveParams, CustomBpmParams,
                 SuperScopeParams, MosaicParams, GrainParams, ScatterParams,
                 InterferencesParams, WaterParams, BumpParams, WaterBumpParams,
                 FyrewurXParams,
                 StarfieldParams, TimescopeParams, DotGridParams, DotPlaneParams,
                 DotFountainParams, ColorMapParams, BufferBlendParams,
                 JherikoGlobalParams, ColorClipParams, UniqueToneParams,
                 InterleaveParams, ConvolutionParams, NormaliseParams,
                 MultiFilterParams, AddBordersParams, SimpleScopeParams,
                 BassSpinParams, OscStarParams, OscRingParams, RotatingStarsParams,
                 PictureParams, PictureIIParams, TexerParams, TexerIIParams,
                 TriangleParams, ChannelShiftParams, ColorReductionParams,
                 MultiplierParams, VideoDelayParams, MultiDelayParams,
                 Fractal2DParams, DomainWarpParams, Fractal3DParams,
                 LyapunovParams, KleinianParams, FractalZoomerParams,
                 StrangeAttractorParams, FlameParams, ReactionDiffusionParams,
                 SetRenderModeParams, DebugBarsParams, PassthroughParams>;

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
        const char* operator()(const DynamicShiftParams&) const { return "Dynamic Shift"; }
        const char* operator()(const DynamicDistanceModifierParams&) const { return "Dynamic Distance Modifier"; }
        const char* operator()(const MovingParticleParams&) const { return "Moving Particle"; }
        const char* operator()(const BlitterFeedbackParams&) const { return "Blitter Feedback"; }
        const char* operator()(const RotoBlitterParams&) const { return "Roto Blitter"; }
        const char* operator()(const BufferSaveParams&) const { return "Buffer Save"; }
        const char* operator()(const CustomBpmParams&) const { return "Custom BPM"; }
        const char* operator()(const SuperScopeParams&) const { return "SuperScope"; }
        const char* operator()(const MosaicParams&) const { return "Mosaic"; }
        const char* operator()(const GrainParams&) const { return "Grain"; }
        const char* operator()(const ScatterParams&) const { return "Scatter"; }
        const char* operator()(const InterferencesParams&) const { return "Interferences"; }
        const char* operator()(const WaterParams&) const { return "Water"; }
        const char* operator()(const BumpParams&) const { return "Bump"; }
        const char* operator()(const WaterBumpParams&) const { return "Water Bump"; }
        const char* operator()(const FyrewurXParams&) const { return "FyrewurX"; }
        const char* operator()(const StarfieldParams&) const { return "Starfield"; }
        const char* operator()(const TimescopeParams&) const { return "Timescope"; }
        const char* operator()(const DotGridParams&) const { return "Dot Grid"; }
        const char* operator()(const DotPlaneParams&) const { return "Dot Plane"; }
        const char* operator()(const DotFountainParams&) const { return "Dot Fountain"; }
        const char* operator()(const ColorMapParams&) const { return "Color Map"; }
        const char* operator()(const BufferBlendParams&) const { return "Buffer Blend"; }
        const char* operator()(const JherikoGlobalParams&) const { return "Global Variables"; }
        const char* operator()(const ColorClipParams&) const { return "Color Clip"; }
        const char* operator()(const UniqueToneParams&) const { return "Unique Tone"; }
        const char* operator()(const InterleaveParams&) const { return "Interleave"; }
        const char* operator()(const ConvolutionParams&) const { return "Convolution"; }
        const char* operator()(const NormaliseParams&) const { return "Normalise"; }
        const char* operator()(const MultiFilterParams&) const { return "MultiFilter"; }
        const char* operator()(const AddBordersParams&) const { return "Add Borders"; }
        const char* operator()(const SimpleScopeParams&) const { return "Simple"; }
        const char* operator()(const BassSpinParams&) const { return "Bass Spin"; }
        const char* operator()(const OscStarParams&) const { return "Oscilliscope Star"; }
        const char* operator()(const OscRingParams&) const { return "Ring"; }
        const char* operator()(const RotatingStarsParams&) const { return "Rotating Stars"; }
        const char* operator()(const PictureParams&) const { return "Picture"; }
        const char* operator()(const PictureIIParams&) const { return "Picture II"; }
        const char* operator()(const TexerParams&) const { return "Texer"; }
        const char* operator()(const TexerIIParams&) const { return "Texer II"; }
        const char* operator()(const TriangleParams&) const { return "Triangle"; }
        const char* operator()(const ChannelShiftParams&) const { return "Channel Shift"; }
        const char* operator()(const ColorReductionParams&) const { return "Color Reduction"; }
        const char* operator()(const MultiplierParams&) const { return "Multiplier"; }
        const char* operator()(const VideoDelayParams&) const { return "Video Delay"; }
        const char* operator()(const MultiDelayParams&) const { return "Multi Delay"; }
        const char* operator()(const Fractal2DParams&) const { return "Fractal 2D"; }
        const char* operator()(const DomainWarpParams&) const { return "Domain Warp"; }
        const char* operator()(const Fractal3DParams&) const { return "Fractal 3D"; }
        const char* operator()(const LyapunovParams&) const { return "Lyapunov"; }
        const char* operator()(const KleinianParams&) const { return "Kleinian"; }
        const char* operator()(const FractalZoomerParams&) const { return "Fractal Zoomer"; }
        const char* operator()(const StrangeAttractorParams&) const { return "Strange Attractor"; }
        const char* operator()(const FlameParams&) const { return "Flame"; }
        const char* operator()(const ReactionDiffusionParams&) const { return "Reaction Diffusion"; }
        const char* operator()(const SetRenderModeParams&) const { return "Set Render Mode"; }
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
    if (auto* fw = std::get_if<FyrewurXParams>(&node.params))
    {
        fw->sparks = std::clamp(fw->sparks, 1, 1024);
        fw->speed = std::clamp(fw->speed, 0.05f, 5.0f);
        fw->gravity = std::clamp(fw->gravity, 0.0f, 10.0f);
        fw->lifeSeconds = std::clamp(fw->lifeSeconds, 0.1f, 10.0f);
    }
    if (auto* save = std::get_if<BufferSaveParams>(&node.params))
    {
        save->slot = std::clamp(save->slot, 0, 7);
        save->dir = std::clamp(save->dir, 0, 3);
        save->adjustAlpha = std::clamp(save->adjustAlpha, 0, 255);
        if (save->dir != 0) warnFallbackBlend(save->blend, "restore", path, result);
    }
    if (auto* scope = std::get_if<SuperScopeParams>(&node.params))
    {
        scope->pointCount = std::clamp(scope->pointCount, 1, 4096);
        scope->renderMode = std::clamp(scope->renderMode, 0, 2);
        scope->audioChannel = std::clamp(scope->audioChannel, 0, 4);
        scope->lineWidth = std::clamp(scope->lineWidth, 1.0f, 20.0f);
        scope->dotSize = std::clamp(scope->dotSize, 1.0f, 50.0f);
        scope->lineBlend = std::clamp(scope->lineBlend, 0, 2);
    }
    if (auto* frac = std::get_if<Fractal2DParams>(&node.params))
    {
        frac->type = std::clamp(frac->type, 0, 8);
        frac->maxIter = std::clamp(frac->maxIter, 1, 2048);
        frac->zoom = std::max(frac->zoom, 1e-6f);
        frac->power = std::clamp(frac->power, 1.0f, 16.0f);
        frac->escapeR = std::max(frac->escapeR, 1.0f);
        frac->blend = std::clamp(frac->blend, 0, 2);
    }
    if (auto* warp = std::get_if<DomainWarpParams>(&node.params))
    {
        warp->octaves = std::clamp(warp->octaves, 1, 10);
        warp->blend = std::clamp(warp->blend, 0, 2);
    }
    if (auto* f3 = std::get_if<Fractal3DParams>(&node.params))
    {
        f3->type = std::clamp(f3->type, 0, 4);
        f3->maxSteps = std::clamp(f3->maxSteps, 8, 512);
        f3->maxIter = std::clamp(f3->maxIter, 1, 64);
        f3->power = std::clamp(f3->power, 1.0f, 16.0f);
        f3->dist = std::max(f3->dist, 0.1f);
        f3->blend = std::clamp(f3->blend, 0, 2);
    }
    if (auto* ly = std::get_if<LyapunovParams>(&node.params))
    {
        ly->warmup = std::clamp(ly->warmup, 0, 2000);
        ly->iterations = std::clamp(ly->iterations, 1, 4000);
        ly->blend = std::clamp(ly->blend, 0, 2);
    }
    if (auto* kl = std::get_if<KleinianParams>(&node.params))
    {
        kl->p = std::clamp(kl->p, 3, 20);
        kl->q = std::clamp(kl->q, 3, 20);
        kl->iterations = std::clamp(kl->iterations, 1, 200);
        kl->blend = std::clamp(kl->blend, 0, 2);
    }
    if (auto* fz = std::get_if<FractalZoomerParams>(&node.params))
    {
        fz->type = std::clamp(fz->type, 0, 2);
        fz->maxIter = std::clamp(fz->maxIter, 1, 2048);
        fz->feedback = std::clamp(fz->feedback, 0.0f, 1.0f);
    }
    if (auto* sa = std::get_if<StrangeAttractorParams>(&node.params))
    {
        sa->type = std::clamp(sa->type, 0, 3);
        sa->points = std::clamp(sa->points, 1, 100000);
        sa->blend = std::clamp(sa->blend, 0, 2);
    }
    if (auto* fl = std::get_if<FlameParams>(&node.params))
    {
        fl->variation = std::clamp(fl->variation, 0, 4);
        fl->functions = std::clamp(fl->functions, 2, 4);
        fl->points = std::clamp(fl->points, 1, 200000);
        fl->blend = std::clamp(fl->blend, 0, 2);
    }
    if (auto* rd = std::get_if<ReactionDiffusionParams>(&node.params))
    {
        rd->stepsPerFrame = std::clamp(rd->stepsPerFrame, 1, 64);
        rd->blend = std::clamp(rd->blend, 0, 2);
    }
    if (auto* srm = std::get_if<SetRenderModeParams>(&node.params))
    {
        srm->lineWidth = std::clamp(srm->lineWidth, 0, 255);
        srm->lineBlend = std::clamp(srm->lineBlend, 0, 2);
        srm->adjustAlpha = std::clamp(srm->adjustAlpha, 0, 255);
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
