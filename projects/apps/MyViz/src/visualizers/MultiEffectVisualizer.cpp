/**
 ****************************************************************************************
 * @file   MultiEffectVisualizer.cpp
 * @brief  Implementation of the multi-effect chain host (Import Roadmap 5.1/5.2)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.2.0
 ****************************************************************************************
 */

#include "visualizers/MultiEffectVisualizer.hpp"

#include "visualizers/multieffect/AvsChainTranslator.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/milkdrop/MilkdropSerializer.hpp"
#include "visualizers/modules/ColorGradientModule.hpp"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFunctions>
#include <QByteArray>
#include <QVector4D>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QString>

#include <algorithm>
#include <cmath>
#include <filesystem>

using namespace lumi::multieffect;
using lumi::scripting::ScriptSlotHost;
using Slot = lumi::scripting::LuaScriptEngine::Slot;

namespace {

// Defined further down; forward-declared for the earlier render handlers.
void computeAudioBands(const std::vector<float>& spec, float& bass, float& mid,
                       float& treble);


// -----------------------------------------------------------------------------
// Shaders (GLSL 330 core, matching FeedbackBuffer)
// -----------------------------------------------------------------------------

const char* kQuadVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
out vec2 vTex;
void main()
{
    vTex = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// AVS Fadeout: per-channel clamped step towards a target color.
const char* kFadeoutFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec3 uTarget;
uniform float uStep;
out vec4 fragColor;
void main()
{
    vec3 col = texture(uTex, vTex).rgb;
    vec3 delta = clamp(uTarget - col, vec3(-uStep), vec3(uStep));
    fragColor = vec4(col + delta, 1.0);
}
)";

// AVS Invert: XOR 0xFFFFFF == 1 - color.
const char* kInvertFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
out vec4 fragColor;
void main()
{
    fragColor = vec4(vec3(1.0) - texture(uTex, vTex).rgb, 1.0);
}
)";

// List blend engine (decision E3). Mode values == BlendMode enum; the full AVS
// set is covered (batch 2, Session 35 added Subtractive/Every-other/XOR/Buffer).
// Reference math: r_list.cpp render_list in/out switch (o == dst, tfb == src).
const char* kBlendFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uDst;
uniform sampler2D uSrc;
uniform sampler2D uBuf;   // global buffer for mode 12 (Buffer)
uniform int uMode;
uniform float uAlpha;
uniform bool uBufInvert;
out vec4 fragColor;
void main()
{
    vec3 d = texture(uDst, vTex).rgb;
    vec3 s = texture(uSrc, vTex).rgb;
    vec3 r;
    if (uMode == 2)       r = (d + s) * 0.5;         // 50/50
    else if (uMode == 3)  r = max(d, s);             // Maximum
    else if (uMode == 4)  r = min(d + s, vec3(1.0)); // Additive
    else if (uMode == 5)  r = max(d - s, vec3(0.0)); // Subtractive 1-2 (dst-src)
    else if (uMode == 6)  r = max(s - d, vec3(0.0)); // Subtractive 2-1 (src-dst)
    else if (uMode == 7)                             // Every other line
        r = ((int(gl_FragCoord.y) & 1) == 0) ? s : d;
    else if (uMode == 8)                             // Every other pixel (checkerboard)
        r = (((int(gl_FragCoord.x) + int(gl_FragCoord.y)) & 1) == 0) ? s : d;
    else if (uMode == 9)                             // XOR (per 8-bit channel)
    {
        ivec3 di = ivec3(d * 255.0 + 0.5);
        ivec3 si = ivec3(s * 255.0 + 0.5);
        r = vec3(di ^ si) / 255.0;
    }
    else if (uMode == 10) r = mix(d, s, uAlpha);     // Adjustable
    else if (uMode == 11) r = d * s;                 // Multiply
    else if (uMode == 12)                            // Buffer: depth of buffer -> alpha
    {
        vec3 b = texture(uBuf, vTex).rgb;
        float v = max(max(b.r, b.g), b.b);           // depthof(): max channel
        if (uBufInvert) v = 1.0 - v;
        r = mix(d, s, v);                            // BLEND_ADJ(src, dst, v)
    }
    else if (uMode == 13) r = min(d, s);             // Minimum
    else                  r = s;                     // Replace (+ safety fallback)
    fragColor = vec4(r, 1.0);
}
)";

// AVS Brightness (ID 22) + Fast Brightness (ID 44): per-channel factor with
// clamp, optional exclusion color. Factor = slider -> multiplier, computed CPU
// side (r_bright.cpp:188). Channels are plain RGB here (FBO texture, no BGR).
const char* kBrightnessFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec3 uFactor;
uniform bool uExclude;
uniform vec3 uExColor;
uniform float uDistance;
out vec4 fragColor;
void main()
{
    vec3 col = texture(uTex, vTex).rgb;
    if (uExclude &&
        all(lessThanEqual(abs(col - uExColor), vec3(uDistance))))
    {
        fragColor = vec4(col, 1.0);
        return;
    }
    fragColor = vec4(clamp(col * uFactor, 0.0, 1.0), 1.0);
}
)";

// AVS Blur (ID 6): 4-connected box kernel; weights select the strength
// (r_blur.cpp normal 651 / light 214 / heavy 443). Clamp-to-edge sampling.
const char* kBlurFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uTexel;
uniform float uCenter;
uniform float uNeighbor;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    vec3 l = texture(uTex, clamp(vTex + vec2(-uTexel.x, 0.0), 0.0, 1.0)).rgb;
    vec3 r = texture(uTex, clamp(vTex + vec2( uTexel.x, 0.0), 0.0, 1.0)).rgb;
    vec3 u = texture(uTex, clamp(vTex + vec2(0.0,  uTexel.y), 0.0, 1.0)).rgb;
    vec3 d = texture(uTex, clamp(vTex + vec2(0.0, -uTexel.y), 0.0, 1.0)).rgb;
    fragColor = vec4(c * uCenter + (l + r + u + d) * uNeighbor, 1.0);
}
)";

// AVS Mirror (ID 26): reflect one half onto the other (r_mirror.cpp:166-247).
// r_mirror: four directed half-copies with per-direction smooth factors
// (BLEND_ADAPT divisor 0..16 -> uF 0..1). Sequential-buffer semantics are
// approximated in one pass: each stage mixes with the ORIGINAL texture, which
// differs from AVS only when opposing directions are active simultaneously.
const char* kMirrorFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec4 uF;  // factors: x=top->bottom, y=bottom->top, z=left->right, w=right->left
out vec4 fragColor;
void main()
{
    vec2 uv = vTex;
    vec3 c = texture(uTex, uv).rgb;
    vec3 mx = texture(uTex, vec2(1.0 - uv.x, uv.y)).rgb;
    vec3 my = texture(uTex, vec2(uv.x, 1.0 - uv.y)).rgb;
    if (uv.x > 0.5) c = mix(c, mx, uF.z);   // left -> right
    if (uv.x < 0.5) c = mix(c, mx, uF.w);   // right -> left
    if (uv.y < 0.5) c = mix(c, my, uF.x);   // top -> bottom
    if (uv.y > 0.5) c = mix(c, my, uF.y);   // bottom -> top
    fragColor = vec4(c, 1.0);
}
)";

// AVS Colorfade (ID 11): per-pixel channel-order classification adds fader
// deltas (r_colorfade.cpp:116-119 + 236-246). Faders passed as fs1/fs2/fs3.
const char* kColorfadeFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec3 uFaders;   // (fs1, fs2, fs3) in [-32,32]/255
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    float R = c.r, G = c.g, B = c.b;
    float d1 = uFaders.x, d2 = uFaders.y, d3 = uFaders.z;
    vec3 o;
    if (G > R && G > B)       o = vec3(R + d1, G + d2, B + d3); // green max
    else if (B > R && B > G)  o = vec3(R + d3, G + d1, B + d2); // blue max
    else if (R > G && R > B)  o = vec3(R + d2, G + d3, B + d1); // red max
    else                      o = vec3(R + d3, G + d3, B + d3); // tie
    fragColor = vec4(clamp(o, 0.0, 1.0), 1.0);
}
)";

// AVS Color Modifier (ID 45): remap each channel through its 256-entry curve.
// The curve is a 256x1 RGB texture (row: R=lutR, G=lutG, B=lutB).
const char* kLutFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uLut;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    float r = texture(uLut, vec2(c.r, 0.5)).r;
    float g = texture(uLut, vec2(c.g, 0.5)).g;
    float b = texture(uLut, vec2(c.b, 0.5)).b;
    fragColor = vec4(r, g, b, 1.0);
}
)";

// AVS Movement / Dynamic Movement: a displacement grid mesh. Each vertex sits
// at its grid position (NDC) and samples the source at the scripted (u,v).
const char* kWarpVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in float aAlpha;
out vec2 vTex;
out vec2 vOrigTex;
out float vAlpha;
void main()
{
    vTex = aTex;
    vOrigTex = aPos * 0.5 + 0.5;  // unwarped screen position of this vertex
    vAlpha = aAlpha;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// r_dmove.cpp LOOPS: with `blend` the moved pixel is mixed onto the ORIGINAL
// image by the script's per-cell alpha (BLEND_ADJ(moved, orig, a)); `nomove`
// skips the displacement and alpha-blends the SOURCE at the same position
// (framebuffer source -> BLEND_ADJ(0, orig, a) = fade to black). The source
// image is uSrcTex: the current frame, or a global buffer when `buffern` is
// set. Without blend the alpha output is ignored, exactly like the original.
const char* kWarpFragmentShader = R"(
#version 330 core
in vec2 vTex;
in vec2 vOrigTex;
in float vAlpha;
uniform sampler2D uTex;     // current frame (blend target / "orig")
uniform sampler2D uSrcTex;  // warp source (frame or global buffer)
uniform bool uWrap;
uniform bool uBlend;
uniform bool uNomove;
uniform bool uBufSrc;       // uSrcTex is a global buffer (nomove semantics)
out vec4 fragColor;
void main()
{
    vec2 uv = uWrap ? fract(vTex) : clamp(vTex, 0.0, 1.0);
    float a = clamp(vAlpha, 0.0, 1.0);
    vec3 orig = texture(uTex, vOrigTex).rgb;
    vec3 c;
    if (uNomove)
    {
        c = uBufSrc ? mix(orig, texture(uSrcTex, vOrigTex).rgb, a)
                    : orig * (1.0 - a);
    }
    else
    {
        vec3 moved = texture(uSrcTex, uv).rgb;
        c = uBlend ? mix(orig, moved, a) : moved;
    }
    fragColor = vec4(c, 1.0);
}
)";

// AVS Roto / Blitter Feedback (ID 9 / 4): sample the current image rotated and
// zoomed about the center, blend with the original — a scale/rotate feedback.
const char* kFeedbackFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform float uZoomInv;  // 1/zoom
uniform float uCos;
uniform float uSin;
uniform float uAspect;   // w/h (keeps rotation square)
uniform bool uBlend;
out vec4 fragColor;
void main()
{
    vec2 d = vTex - 0.5;
    d.x *= uAspect;
    vec2 s = uZoomInv * vec2(uCos * d.x - uSin * d.y, uSin * d.x + uCos * d.y);
    s.x /= uAspect;
    vec2 src = s + 0.5;
    vec3 t = texture(uTex, clamp(src, 0.0, 1.0)).rgb;
    vec3 o = texture(uTex, vTex).rgb;
    fragColor = vec4(uBlend ? mix(o, t, 0.5) : t, 1.0);
}
)";

// DebugBars: positioned quad, scaled/placed via uniforms (host debug leaf).
const char* kBarsVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform vec2 uCenter;
uniform vec2 uSize;
void main()
{
    gl_Position = vec4(uCenter + aPos * uSize, 0.0, 1.0);
}
)";

const char* kBarsFragmentShader = R"(
#version 330 core
uniform vec3 uColor;
out vec4 fragColor;
void main()
{
    fragColor = vec4(uColor, 1.0);
}
)";

// AVS "Trans / Mosaic" (ID 30): sample the image on a coarse uCells x uCells
// grid (block centre, like r_mosaic's half-cell start), optionally blended with
// the untouched image. uBlend: 0 replace, 1 additive, 2 50/50.
const char* kMosaicFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform float uCells;
uniform int uBlend;
out vec4 fragColor;
void main()
{
    vec3 orig = texture(uTex, vTex).rgb;
    vec2 muv = (floor(vTex * uCells) + 0.5) / uCells;  // block centre
    vec3 mos = texture(uTex, muv).rgb;
    vec3 r;
    if (uBlend == 1)      r = min(orig + mos, vec3(1.0));  // additive
    else if (uBlend == 2) r = (orig + mos) * 0.5;          // 50/50
    else                  r = mos;                         // replace
    fragColor = vec4(r, 1.0);
}
)";

// AVS "Trans / Grain" (ID 24): darken a random subset of pixels by a random
// factor (r_grain.cpp). uAmount = gated fraction; uSeed 0 = static noise.
const char* kGrainFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uRes;
uniform float uAmount;
uniform float uSeed;
uniform int uBlend;
out vec4 fragColor;
float h(vec2 p, float s) { return fract(sin(dot(p, vec2(127.1, 311.7)) + s) * 43758.5453); }
void main()
{
    vec3 o = texture(uTex, vTex).rgb;
    vec2 px = floor(vTex * uRes);
    float g = h(px, uSeed);          // gate
    float s = h(px, uSeed + 7.3);    // darkening factor
    vec3 c = (g < uAmount) ? o * s : vec3(0.0);
    vec3 r;
    if (uBlend == 1)      r = min(o + c, vec3(1.0));  // additive
    else if (uBlend == 2) r = (o + c) * 0.5;          // 50/50
    else                  r = c;                      // replace
    fragColor = vec4(r, 1.0);
}
)";

// AVS "Trans / Scatter" (ID 16): per-pixel random displacement in a ~4px window
// (r_scat.cpp), refreshed each frame via uSeed.
const char* kScatterFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uRes;
uniform float uSeed;
uniform float uRange;
out vec4 fragColor;
float h(vec2 p, float s) { return fract(sin(dot(p, vec2(127.1, 311.7)) + s) * 43758.5453); }
void main()
{
    vec2 px = floor(vTex * uRes);
    float rx = h(px, uSeed) * 2.0 - 1.0;
    float ry = h(px, uSeed + 31.7) * 2.0 - 1.0;
    vec2 off = vec2(rx, ry) * uRange / uRes;
    fragColor = vec4(texture(uTex, vTex + off).rgb, 1.0);
}
)";

// AVS "Trans / Interferences" (ID 41): accumulate uPoints rotated copies of the
// image, each weighted by uAlpha (r_interf.cpp). uRgb splits copies across the
// R/G/B channels; uBlend combines with the original.
const char* kInterfFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform int uPoints;
uniform vec2 uOffsets[8];
uniform float uAlpha;
uniform int uRgb;
uniform int uBlend;
out vec4 fragColor;
void main()
{
    vec3 orig = texture(uTex, vTex).rgb;
    vec3 acc = vec3(0.0);
    for (int i = 0; i < uPoints && i < 8; ++i)
    {
        vec3 s = texture(uTex, vTex - uOffsets[i]).rgb * uAlpha;
        if (uRgb == 1) { int ch = i - (i / 3) * 3; acc[ch] += s[ch]; }
        else acc += s;
    }
    acc = min(acc, vec3(1.0));
    vec3 r;
    if (uBlend == 1)      r = min(orig + acc, vec3(1.0));  // additive
    else if (uBlend == 2) r = (orig + acc) * 0.5;          // 50/50
    else                  r = acc;                         // replace
    fragColor = vec4(r, 1.0);
}
)";

// AVS "Trans / Water" (ID 20): neighbour average of the current frame minus the
// previous frame -> a color-space ripple (r_water.cpp). uCur = this frame,
// uLast = the previous frame's image.
const char* kWaterFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uCur;
uniform sampler2D uLast;
uniform vec2 uTexel;
out vec4 fragColor;
void main()
{
    vec3 l = texture(uCur, vTex + vec2(-uTexel.x, 0.0)).rgb;
    vec3 r = texture(uCur, vTex + vec2( uTexel.x, 0.0)).rgb;
    vec3 u = texture(uCur, vTex + vec2(0.0, -uTexel.y)).rgb;
    vec3 d = texture(uCur, vTex + vec2(0.0,  uTexel.y)).rgb;
    vec3 avg = (l + r + u + d) * 0.5;               // wave equation term
    vec3 last = texture(uLast, vTex).rgb;
    fragColor = vec4(clamp(avg - last, 0.0, 1.0), 1.0);
}
)";

// AVS "Trans / Bump" (ID 29): per-pixel bump lighting from the luminance
// gradient, lit by a movable source at uLight (r_bump.cpp). Bright near the
// light where the surface is flat; falls off with distance.
const char* kBumpFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uRes;
uniform vec2 uLight;
uniform float uDepth;
uniform int uInvert;
uniform int uBlend;
out vec4 fragColor;
float dpth(vec2 uv)
{
    vec3 c = texture(uTex, uv).rgb;
    float m = max(max(c.r, c.g), c.b);
    return uInvert == 1 ? 1.0 - m : m;
}
void main()
{
    vec3 orig = texture(uTex, vTex).rgb;
    vec2 t = 1.0 / uRes;
    float gx = (dpth(vTex + vec2(t.x, 0.0)) - dpth(vTex - vec2(t.x, 0.0))) * 255.0;
    float gy = (dpth(vTex + vec2(0.0, t.y)) - dpth(vTex - vec2(0.0, t.y))) * 255.0;
    float lx = (vTex.x - uLight.x) * uRes.x;
    float ly = (vTex.y - uLight.y) * uRes.y;
    float c1 = 127.0 - abs(gx - lx);
    float c2 = 127.0 - abs(gy - ly);
    float bright = (c1 <= 0.0 || c2 <= 0.0) ? 0.0 : c1 * c2 * uDepth / 16384.0;
    // r_bump setdepth(): per-channel orig + brightness, capped at 254 (ADDITIVE
    // light on top of the image; bright=0 keeps the pixel).
    vec3 lit = min(orig + vec3(clamp(bright / 255.0, 0.0, 1.0)), vec3(254.0 / 255.0));
    vec3 r;
    if (uBlend == 1)      r = min(orig + lit, vec3(1.0));
    else if (uBlend == 2) r = (orig + lit) * 0.5;
    else                  r = lit;
    fragColor = vec4(r, 1.0);
}
)";

// AVS "Trans / Dynamic Shift" (ID 42): a global image translation by uOffset
// (normalized; output pixel samples input at vTex-uOffset), black fill outside,
// optional 50/50 blend with the original (r_shift.cpp). The EEL that drives the
// offset runs on the CPU (ScriptSlotHost); this shader just applies it.
const char* kDynamicShiftFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uOffset;
uniform int uBlend;
uniform float uAlpha;
out vec4 fragColor;
void main()
{
    vec2 uv = vTex - uOffset;
    vec3 shifted = vec3(0.0);
    if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0)
        shifted = texture(uTex, uv).rgb;
    vec3 orig = texture(uTex, vTex).rgb;
    vec3 r = (uBlend == 1) ? mix(shifted, orig, clamp(uAlpha, 0.0, 1.0)) : shifted;
    fragColor = vec4(r, 1.0);
}
)";

// AVS "Trans / Dynamic Distance Modifier" (ID 35): radial resample. uLut[256]
// maps input distance (0..1 of half-diagonal uMaxD) to output distance; each
// pixel samples the source at the remapped distance along its own angle
// (r_ddm.cpp). uBlend 1 = 50/50 with the original.
const char* kDdmFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uRes;
uniform float uMaxD;
uniform float uLut[256];
uniform int uBlend;
out vec4 fragColor;
void main()
{
    vec2 p = (vTex - vec2(0.5)) * uRes;   // pixels from center
    float dist = length(p);
    vec3 orig = texture(uTex, vTex).rgb;
    vec3 res = orig;
    if (dist > 0.0001)
    {
        float din = clamp(dist / uMaxD, 0.0, 1.0);
        float fidx = din * 255.0;
        int i0 = int(floor(fidx));
        int i1 = min(i0 + 1, 255);
        float dout = mix(uLut[i0], uLut[i1], fidx - float(i0));
        vec2 uv = clamp(vec2(0.5) + (p / dist) * (dout * uMaxD) / uRes, 0.0, 1.0);
        res = texture(uTex, uv).rgb;
    }
    fragColor = vec4(uBlend == 1 ? (res + orig) * 0.5 : res, 1.0);
}
)";

// AVS APE "Misc: Buffer blend": combine two textures (uSrcA, uSrcB) per uMode.
const char* kBufferBlendFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uSrcA;
uniform sampler2D uSrcB;
uniform int uMode;
out vec4 fragColor;
void main()
{
    vec3 a = texture(uSrcA, vTex).rgb;
    vec3 b = texture(uSrcB, vTex).rgb;
    vec3 r;
    if (uMode == 0)      r = b;                       // replace
    else if (uMode == 1) r = min(a + b, vec3(1.0));   // additive
    else if (uMode == 2) r = max(a, b);               // maximum
    else if (uMode == 3) r = (a + b) * 0.5;           // 50/50
    else if (uMode == 4) r = clamp(a - b, 0.0, 1.0);  // sub dest-src
    else if (uMode == 5) r = clamp(b - a, 0.0, 1.0);  // sub src-dest
    else if (uMode == 6) r = a * b;                   // multiply
    else if (uMode == 7) r = (a + b) * 0.5;           // adjustable (no weight -> 50/50)
    else if (uMode == 8) r = vec3(ivec3(a * 255.0) ^ ivec3(b * 255.0)) / 255.0;  // xor
    else if (uMode == 9) r = min(a, b);               // minimum
    else                 r = abs(a - b);              // absolute difference
    fragColor = vec4(r, 1.0);
}
)";

// AVS APE "Holden03: Convolution Filter": 7x7 kernel over the image.
const char* kConvolutionFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uRes;
uniform float uKernel[49];
uniform float uScale;
uniform float uBias;
uniform int uAbsolute;
uniform int uEdge;
out vec4 fragColor;
vec2 sampleUV(vec2 uv) { return uEdge == 1 ? fract(uv) : clamp(uv, 0.0, 1.0); }
void main()
{
    vec2 texel = 1.0 / uRes;
    vec3 sum = vec3(0.0);
    for (int j = 0; j < 7; j++)
        for (int i = 0; i < 7; i++)
        {
            float k = uKernel[j * 7 + i];
            vec2 off = vec2(float(i - 3), float(j - 3)) * texel;
            sum += texture(uTex, sampleUV(vTex + off)).rgb * k;
        }
    vec3 r = sum / uScale + vec3(uBias / 255.0);
    if (uAbsolute == 1) r = abs(r);
    fragColor = vec4(clamp(r, 0.0, 1.0), 1.0);
}
)";

// AVS APE "Trans: Normalise": stretch the image between measured min/max.
const char* kNormaliseFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform float uLo;
uniform float uHi;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    float d = max(uHi - uLo, 1.0 / 255.0);
    fragColor = vec4(clamp((c - vec3(uLo)) / d, 0.0, 1.0), 1.0);
}
)";

// AVS APE "Jheriko : MULTIFILTER": fixed chrome/root filters (math approximated).
const char* kMultiFilterFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform int uEffect;
out vec4 fragColor;
vec3 chrome(vec3 c) { return vec3(1.0) - abs(2.0 * c - vec3(1.0)); }
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    vec3 r;
    if (uEffect == 0)      r = chrome(c);
    else if (uEffect == 1) r = chrome(chrome(c));
    else if (uEffect == 2) r = chrome(chrome(chrome(c)));
    else                   r = sqrt(c);
    fragColor = vec4(r, 1.0);
}
)";

// AVS APE "Virtual Effect: Addborders": solid border of uColor, uSize pixels.
const char* kAddBordersFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uRes;
uniform vec3 uColor;
uniform int uSize;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    ivec2 p = ivec2(vTex * uRes);
    ivec2 res = ivec2(uRes);
    bool border = p.x < uSize || p.y < uSize ||
                  p.x >= res.x - uSize || p.y >= res.y - uSize;
    fragColor = vec4(border ? uColor : c, 1.0);
}
)";

// Sprite (Texer/Texer II): a quad centred at uCenter, half-size uHalf, textured.
const char* kSpriteVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform vec2 uCenter;
uniform vec2 uHalf;
out vec2 vTex;
void main()
{
    vTex = aPos * 0.5 + 0.5;
    gl_Position = vec4(uCenter + aPos * uHalf, 0.0, 1.0);
}
)";
const char* kSpriteFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uImg;
uniform vec3 uTint;
uniform int uColorFilter;
out vec4 fragColor;
void main()
{
    vec4 c = texture(uImg, vec2(vTex.x, 1.0 - vTex.y));
    vec3 rgb = uColorFilter == 1 ? c.rgb * uTint : c.rgb;
    fragColor = vec4(rgb, c.a);
}
)";

// AVS "Render / Picture" (ID 34): draw an embedded image (uImg) over the frame,
// letterboxed when uKeepAspect, blended per uBlend (0 replace, 1 add, 2 50/50).
const char* kPictureFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uImg;
uniform int uBlend;
uniform int uKeepAspect;
uniform vec2 uImgSize;
uniform vec2 uRes;
out vec4 fragColor;
void main()
{
    vec3 fb = texture(uTex, vTex).rgb;
    vec2 uv = vTex;
    if (uKeepAspect == 1)
    {
        float sa = uRes.x / uRes.y;
        float ia = uImgSize.x / max(uImgSize.y, 1.0);
        vec2 scale = vec2(1.0);
        if (ia > sa) scale.y = sa / ia; else scale.x = ia / sa;
        uv = (vTex - vec2(0.5)) / scale + vec2(0.5);
    }
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        fragColor = vec4(fb, 1.0);
        return;
    }
    vec3 img = texture(uImg, vec2(uv.x, 1.0 - uv.y)).rgb;  // image is top-down
    vec3 r;
    if (uBlend == 1)      r = min(fb + img, vec3(1.0));
    else if (uBlend == 2) r = (fb + img) * 0.5;
    else                  r = img;
    fragColor = vec4(r, 1.0);
}
)";

// AVS "Trans / Color Clip" (ID 12): replace pixels matching a colour threshold.
const char* kColorClipFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform int uMode;
uniform vec3 uClip;
uniform vec3 uOut;
uniform float uDist;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    bool hit;
    if (uMode == 1)      hit = all(lessThanEqual(c, uClip));
    else if (uMode == 2) hit = all(greaterThanEqual(c, uClip));
    else { vec3 d = c - uClip; hit = dot(d, d) <= uDist * uDist; }
    fragColor = vec4(hit ? uOut : c, 1.0);
}
)";

// AVS "Trans / Unique Tone" (ID 38): tint to a single hue by luminance.
const char* kUniqueToneFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec3 uColor;
uniform int uInvert;
uniform int uBlend;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    float d = max(max(c.r, c.g), c.b);
    if (uInvert == 1) d = 1.0 - d;
    vec3 tone = uColor * d;
    vec3 r;
    if (uBlend == 1)      r = min(c + tone, vec3(1.0));
    else if (uBlend == 2) r = (c + tone) * 0.5;
    else                  r = tone;
    fragColor = vec4(r, 1.0);
}
)";

// AVS "Trans / Interleave" (ID 23): overlay a stripe/grid of uColor at spacing
// uSpacing (px; component 0 = that axis off), matching r_interleave's toggling.
const char* kInterleaveFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uRes;
uniform ivec2 uSpacing;
uniform vec3 uColor;
uniform int uBlend;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    int px = int(vTex.x * uRes.x);
    int py = int(vTex.y * uRes.y);
    int tx = uSpacing.x;
    int ty = uSpacing.y;
    bool colored = false;
    if (ty > 0)
    {
        int yy = py + (int(uRes.y) % ty) / 2;
        bool ystat = ((yy / ty) & 1) == 1;
        if (!ystat) colored = true;
        else if (tx > 0)
        {
            int xx = px + (int(uRes.x) % tx) / 2;
            if (((xx / tx) & 1) == 0) colored = true;
        }
    }
    else if (tx > 0)
    {
        int xx = px + (int(uRes.x) % tx) / 2;
        if (((xx / tx) & 1) == 0) colored = true;
    }
    vec3 r = c;
    if (colored)
    {
        if (uBlend == 1)      r = min(c + uColor, vec3(1.0));
        else if (uBlend == 2) r = (c + uColor) * 0.5;
        else                  r = uColor;
    }
    fragColor = vec4(r, 1.0);
}
)";

// AVS APE "Color Map": pick an input value per pixel (uKey selects the channel),
// look it up in a 256-entry gradient LUT (uLut), and blend the mapped colour onto
// the image per uBlend (0..9). uAdjust is the ADJUSTABLE weight.
const char* kColorMapFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uLut;
uniform int uKey;
uniform int uBlend;
uniform float uAdjust;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    float idx;
    if (uKey == 0)      idx = c.r;
    else if (uKey == 1) idx = c.g;
    else if (uKey == 2) idx = c.b;
    else if (uKey == 3) idx = (c.r + c.g + c.b) * 0.5;
    else if (uKey == 4) idx = max(max(c.r, c.g), c.b);
    else                idx = (c.r + c.g + c.b) / 3.0;
    vec3 m = texture(uLut, vec2(clamp(idx, 0.0, 1.0), 0.5)).rgb;
    vec3 r;
    if (uBlend == 0)      r = m;
    else if (uBlend == 1) r = min(c + m, vec3(1.0));
    else if (uBlend == 2) r = max(c, m);
    else if (uBlend == 3) r = min(c, m);
    else if (uBlend == 4) r = (c + m) * 0.5;
    else if (uBlend == 5) r = clamp(c - m, 0.0, 1.0);
    else if (uBlend == 6) r = clamp(m - c, 0.0, 1.0);
    else if (uBlend == 7) r = c * m;
    else if (uBlend == 8) r = vec3(ivec3(c * 255.0) ^ ivec3(m * 255.0)) / 255.0;
    else                  r = mix(c, m, clamp(uAdjust, 0.0, 1.0));
    fragColor = vec4(r, 1.0);
}
)";

// AVS "Trans / Water Bump" (ID 31): height-field wave propagation. The height
// buffer packs .r = current, .g = previous; a beat adds a drop. (r_waterbump)
const char* kWaterBumpPropShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uH;
uniform vec2 uTexel;
uniform float uDamp;
uniform int uDrop;
uniform vec2 uDropC;
uniform float uDropR;
uniform float uDropAmp;
out vec4 fragColor;
void main()
{
    vec2 t = uTexel;
    float s =
        texture(uH, vTex + vec2( t.x, 0.0)).r + texture(uH, vTex + vec2(-t.x, 0.0)).r +
        texture(uH, vTex + vec2(0.0,  t.y)).r + texture(uH, vTex + vec2(0.0, -t.y)).r +
        texture(uH, vTex + vec2( t.x,  t.y)).r + texture(uH, vTex + vec2(-t.x,  t.y)).r +
        texture(uH, vTex + vec2( t.x, -t.y)).r + texture(uH, vTex + vec2(-t.x, -t.y)).r;
    float cur = texture(uH, vTex).r;
    float prev = texture(uH, vTex).g;
    float nh = s * 0.25 - prev;   // wave equation (AVS sum8>>2 - prev)
    nh -= nh * uDamp;             // damping
    if (uDrop == 1)
    {
        float d = distance(vTex, uDropC) / max(uDropR, 1e-4);
        if (d < 1.0) nh += cos(d * 1.5707963) * uDropAmp;
    }
    fragColor = vec4(nh, cur, 0.0, 1.0);   // new height, old current -> previous
}
)";

// Water Bump refraction: displace the image by the height gradient (r_waterbump).
const char* kWaterBumpDispShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uImg;
uniform sampler2D uH;
uniform vec2 uTexel;
uniform vec2 uRes;
uniform float uScale;
out vec4 fragColor;
void main()
{
    float hc = texture(uH, vTex).r;
    float hr = texture(uH, vTex + vec2(uTexel.x, 0.0)).r;
    float hd = texture(uH, vTex + vec2(0.0, uTexel.y)).r;
    vec2 off = vec2(hc - hr, hc - hd) * uScale / uRes;
    fragColor = vec4(texture(uImg, vTex + off).rgb, 1.0);
}
)";

// AVS "Render / Timescope" (ID 39): one spectrum column, tinted by uColor.
// Drawn scissored to a single x each frame (r_timescope.cpp).
const char* kTimescopeFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uSpec;
uniform vec3 uColor;
uniform float uBands;
out vec4 fragColor;
void main()
{
    float band = floor(vTex.y * uBands) / uBands;
    float c = texture(uSpec, vec2(band, 0.5)).r;
    fragColor = vec4(uColor * c, 1.0);
}
)";

// AVS core-set APEs: Channel Shift (uType 0), Color Reduction (1), Multiplier (2).
const char* kApeFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform int uType;
uniform int uMode;
uniform float uLevels;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    vec3 r = c;
    if (uType == 0)          // channel shift (permute)
    {
        if (uMode == 1)      r = c.rbg;
        else if (uMode == 2) r = c.gbr;
        else if (uMode == 3) r = c.grb;
        else if (uMode == 4) r = c.brg;
        else if (uMode == 5) r = c.bgr;
        else                 r = c.rgb;
    }
    else if (uType == 1)     // colour reduction (quantise per channel)
    {
        float L = max(uLevels - 1.0, 1.0);
        r = floor(c * L + 0.5) / L;
    }
    else                     // multiplier
    {
        if (uMode == 1)      r = c * 8.0;
        else if (uMode == 2) r = c * 4.0;
        else if (uMode == 3) r = c * 2.0;
        else if (uMode == 4) r = c * 0.5;
        else if (uMode == 5) r = c * 0.25;
        else if (uMode == 6) r = c * 0.125;
        else if (uMode == 0) r = c * 8.0;   // saturate-ish
        else                 r = c;         // 7: keep
    }
    fragColor = vec4(clamp(r, 0.0, 1.0), 1.0);
}
)";

// Fractal 2D (Batch H, host-native): escape-time / root-finding fractals in a
// single fullscreen pass. uType selects the formula; escaping (or converging)
// iterations map through the gradient LUT, the interior gets uInside. uBlend
// composites over the current framebuffer.
const char* kFractal2DFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;    // current framebuffer (blend source)
uniform sampler2D uLut;    // 256x1 palette
uniform vec2  uCenter;
uniform float uZoom;
uniform float uRot;
uniform float uAspect;     // width / height
uniform int   uType;
uniform int   uMaxIter;
uniform vec2  uJulia;
uniform float uPower;
uniform float uEscapeR;    // |z|^2 escape threshold
uniform bool  uSmooth;
uniform float uColorScale;
uniform float uColorPhase;
uniform vec3  uInside;
uniform int   uBlend;
out vec4 fragColor;

vec2 cmul(vec2 a, vec2 b){ return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x); }
vec2 cdiv(vec2 a, vec2 b){ float d = dot(b,b) + 1e-12; return vec2(a.x*b.x + a.y*b.y, a.y*b.x - a.x*b.y) / d; }
vec2 cpow(vec2 z, float p){
    float r = length(z);
    if (r < 1e-12) return vec2(0.0);
    float th = atan(z.y, z.x);
    float rp = pow(r, p);
    return rp * vec2(cos(p*th), sin(p*th));
}

void main()
{
    vec2 uv = (vTex - 0.5) * 2.0;
    uv.x *= uAspect;
    float s = sin(uRot), c = cos(uRot);
    uv = mat2(c, -s, s, c) * uv;
    vec2 coord = uCenter + uv / uZoom;

    int i = 0;
    bool hit = false;         // escaped or converged
    float it = 0.0;
    vec2 z, cc;

    if (uType == 5) {          // Newton p(z) = z^3 - 1
        z = coord;
        for (i = 0; i < uMaxIter; ++i){
            vec2 z2 = cmul(z, z);
            vec2 f  = cmul(z2, z) - vec2(1.0, 0.0);
            vec2 df = 3.0 * z2;
            z = z - cdiv(f, df);
            if (dot(f, f) < 1e-6){ hit = true; break; }
        }
        it = float(i);
    }
    else if (uType == 6) {     // Phoenix: z' = z^2 + Re(c) + Im(c)*zprev
        z = coord; vec2 zprev = vec2(0.0);
        for (i = 0; i < uMaxIter; ++i){
            vec2 zn = cmul(z, z) + vec2(uJulia.x, 0.0) + uJulia.y * zprev;
            zprev = z; z = zn;
            if (dot(z, z) > uEscapeR){ hit = true; break; }
        }
        it = float(i);
    }
    else if (uType == 7) {     // Magnet type 1: z' = ((z^2+c-1)/(2z+c-2))^2
        z = coord; cc = coord;
        for (i = 0; i < uMaxIter; ++i){
            vec2 num = cmul(z, z) + cc - vec2(1.0, 0.0);
            vec2 den = 2.0 * z + cc - vec2(2.0, 0.0);
            vec2 q = cdiv(num, den);
            z = cmul(q, q);
            vec2 d = z - vec2(1.0, 0.0);
            if (dot(d, d) < 1e-6 || dot(z, z) > 1e6){ hit = true; break; }
        }
        it = float(i);
    }
    else if (uType == 8) {     // Nova: relaxed Newton z - (z^p-1)/(p z^(p-1)) + c
        z = coord; cc = coord;
        for (i = 0; i < uMaxIter; ++i){
            vec2 f  = cpow(z, uPower) - vec2(1.0, 0.0);
            vec2 df = uPower * cpow(z, uPower - 1.0);
            vec2 zn = z - cdiv(f, df) + cc;
            vec2 d = zn - z;
            z = zn;
            if (dot(d, d) < 1e-6){ hit = true; break; }
        }
        it = float(i);
    }
    else {                     // escape-time family
        if (uType == 1){ z = coord; cc = uJulia; }   // Julia
        else           { z = vec2(0.0); cc = coord; } // Mandelbrot etc.
        for (i = 0; i < uMaxIter; ++i){
            if (uType == 2){ z = abs(z); z = cmul(z, z) + cc; }               // Burning Ship
            else if (uType == 3){ z = vec2(z.x, -z.y); z = cmul(z, z) + cc; } // Tricorn (conj^2 + c)
            else if (uType == 4){ z = cpow(z, uPower) + cc; }                 // Multibrot
            else { z = cmul(z, z) + cc; }                                     // Mandelbrot / Julia
            float m2 = dot(z, z);
            if (m2 > uEscapeR){
                hit = true;
                if (uSmooth){
                    float lz = log(m2) * 0.5;
                    it = float(i) + 1.0 - log(lz / log(2.0)) / log(2.0);
                } else it = float(i);
                break;
            }
        }
    }

    vec3 col;
    if (!hit) col = uInside;
    else {
        float t = fract(it * uColorScale + uColorPhase);
        col = texture(uLut, vec2(t, 0.5)).rgb;
    }

    if (uBlend == 0) fragColor = vec4(col, 1.0);
    else {
        vec3 d = texture(uTex, vTex).rgb;
        if (uBlend == 1) fragColor = vec4(min(d + col, vec3(1.0)), 1.0);
        else             fragColor = vec4((d + col) * 0.5, 1.0);
    }
}
)";

// Domain-warp fBm (Batch H, host-native): a fullscreen "plasma / nebula" field.
// Value-noise fBm warped by two further fBm fields, animated by uTime, coloured
// through the gradient LUT. uBlend composites over the current framebuffer.
const char* kDomainWarpFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uLut;
uniform float uAspect;
uniform int   uOctaves;
uniform float uLac;
uniform float uGain;
uniform float uScale;
uniform float uWarp;
uniform float uWarpScale;
uniform vec2  uOffset;
uniform float uTime;
uniform float uColorScale;
uniform float uColorPhase;
uniform int   uBlend;
out vec4 fragColor;

float hash(vec2 p){ p = fract(p * vec2(123.34, 456.21)); p += dot(p, p + 45.32); return fract(p.x * p.y); }
float vnoise(vec2 p){
    vec2 i = floor(p), f = fract(p);
    float a = hash(i), b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0)), d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}
float fbm(vec2 p){
    float sum = 0.0, amp = 0.5, freq = 1.0;
    for (int i = 0; i < uOctaves; ++i){ sum += amp * vnoise(p * freq); freq *= uLac; amp *= uGain; }
    return sum;
}

void main()
{
    vec2 uv = (vTex - 0.5) * 2.0;
    uv.x *= uAspect;
    vec2 p = uv * uScale + uOffset;
    vec2 q = vec2(fbm(p + vec2(0.0, uTime)), fbm(p + vec2(5.2, 1.3 - uTime)));
    vec2 r = vec2(fbm(p + uWarp * q * uWarpScale + vec2(1.7, 9.2)),
                  fbm(p + uWarp * q * uWarpScale + vec2(8.3, 2.8)));
    float v = fbm(p + uWarp * r);
    float t = fract(v * uColorScale + uColorPhase);
    vec3 col = texture(uLut, vec2(t, 0.5)).rgb;
    if (uBlend == 0) fragColor = vec4(col, 1.0);
    else {
        vec3 d = texture(uTex, vTex).rgb;
        if (uBlend == 1) fragColor = vec4(min(d + col, vec3(1.0)), 1.0);
        else             fragColor = vec4((d + col) * 0.5, 1.0);
    }
}
)";

// Fractal 3D (Batch H): raymarched distance-estimator fractals. uType selects the
// signed-distance field; the hit is normal-lit and coloured through the LUT.
const char* kFractal3DFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uLut;
uniform float uAspect;
uniform int   uType;
uniform vec3  uCam;
uniform float uFov;
uniform float uPower;
uniform float uScale;
uniform float uFold;
uniform int   uMaxSteps;
uniform int   uMaxIter;
uniform vec4  uJulia;
uniform vec3  uLight;
uniform float uAmbient;
uniform bool  uAO;
uniform float uColorScale;
uniform float uColorPhase;
uniform vec3  uBg;
uniform int   uBlend;
out vec4 fragColor;

float sdBox(vec3 p, vec3 b){ vec3 q = abs(p) - b; return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0); }
float deMandelbulb(vec3 p){
    vec3 z = p; float dr = 1.0; float r = 0.0;
    for (int i = 0; i < uMaxIter; ++i){
        r = length(z); if (r > 2.0) break;
        float th = acos(clamp(z.z / r, -1.0, 1.0)); float ph = atan(z.y, z.x);
        dr = pow(r, uPower - 1.0) * uPower * dr + 1.0;
        float zr = pow(r, uPower); th *= uPower; ph *= uPower;
        z = zr * vec3(sin(th) * cos(ph), sin(th) * sin(ph), cos(th)) + p;
    }
    return 0.5 * log(max(r, 1e-6)) * r / dr;
}
float deMandelbox(vec3 p){
    vec3 z = p; float dr = 1.0;
    for (int i = 0; i < uMaxIter; ++i){
        z = clamp(z, -uFold, uFold) * 2.0 - z;
        float m = dot(z, z);
        if (m < 0.25){ z *= 4.0; dr *= 4.0; }
        else if (m < 1.0){ z /= m; dr /= m; }
        z = uScale * z + p; dr = dr * abs(uScale) + 1.0;
    }
    return length(z) / abs(dr);
}
float deMenger(vec3 p){
    float d = sdBox(p, vec3(1.0)); float s = 1.0;
    for (int i = 0; i < uMaxIter; ++i){
        vec3 a = mod(p * s, 2.0) - 1.0;
        s *= 3.0;
        vec3 r = abs(1.0 - 3.0 * abs(a));
        float da = max(r.x, r.y), db = max(r.y, r.z), dc = max(r.z, r.x);
        float c = (min(da, min(db, dc)) - 1.0) / s;
        d = max(d, c);
    }
    return d;
}
vec4 qmul(vec4 a, vec4 b){ return vec4(a.x * b.x - dot(a.yzw, b.yzw), a.x * b.yzw + b.x * a.yzw + cross(a.yzw, b.yzw)); }
float deQJulia(vec3 p){
    vec4 z = vec4(p, 0.0); float dz2 = 1.0;
    for (int i = 0; i < uMaxIter; ++i){
        dz2 *= 4.0 * dot(z, z);
        z = qmul(z, z) + uJulia;
        if (dot(z, z) > 4.0) break;
    }
    float r = length(z);
    return 0.5 * r * log(max(r, 1e-6)) / sqrt(max(dz2, 1e-6));
}
float deKIFS(vec3 p){
    float s = 1.0;
    for (int i = 0; i < uMaxIter; ++i){
        p = abs(p);
        if (p.x < p.y) p.xy = p.yx;
        if (p.x < p.z) p.xz = p.zx;
        if (p.y < p.z) p.yz = p.zy;
        p = p * uScale - vec3(uScale - 1.0);
        s *= uScale;
    }
    return (length(p) - 0.5) / s;
}
float mapDE(vec3 p){
    if (uType == 0) return deMandelbulb(p);
    if (uType == 1) return deMandelbox(p);
    if (uType == 2) return deMenger(p);
    if (uType == 3) return deQJulia(p);
    return deKIFS(p);
}
vec3 calcNormal(vec3 p){
    vec2 e = vec2(0.0015, 0.0);
    return normalize(vec3(mapDE(p + e.xyy) - mapDE(p - e.xyy),
                          mapDE(p + e.yxy) - mapDE(p - e.yxy),
                          mapDE(p + e.yyx) - mapDE(p - e.yyx)));
}
void main()
{
    vec2 uv = (vTex - 0.5) * 2.0;
    uv.x *= uAspect;
    vec3 ro = uCam;
    vec3 fwd = normalize(-ro);
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), fwd));
    vec3 up = cross(fwd, right);
    vec3 rd = normalize(fwd + uFov * (uv.x * right + uv.y * up));
    float t = 0.0; bool hit = false; int steps = 0;
    for (int i = 0; i < uMaxSteps; ++i){
        vec3 p = ro + rd * t;
        float d = mapDE(p);
        steps = i;
        if (d < 0.001 * t + 0.0004){ hit = true; break; }
        t += d;
        if (t > 20.0) break;
    }
    vec3 col;
    if (hit){
        vec3 p = ro + rd * t;
        vec3 n = calcNormal(p);
        float diff = max(dot(n, uLight), 0.0);
        float ao = uAO ? 1.0 - float(steps) / float(uMaxSteps) : 1.0;
        float lit = uAmbient + diff * (1.0 - uAmbient);
        float ci = fract((t * 0.12 + float(steps) * 0.015) * uColorScale + uColorPhase);
        col = texture(uLut, vec2(ci, 0.5)).rgb * lit * ao;
    } else col = uBg;
    if (uBlend == 0) fragColor = vec4(col, 1.0);
    else {
        vec3 d = texture(uTex, vTex).rgb;
        if (uBlend == 1) fragColor = vec4(min(d + col, vec3(1.0)), 1.0);
        else             fragColor = vec4((d + col) * 0.5, 1.0);
    }
}
)";

// Lyapunov (Batch H, "Zircon Zity"): per (a,b) pixel iterate the logistic map with
// r alternating along uSeq, accumulate the Lyapunov exponent; ordered zones tint,
// chaotic zones map through the LUT.
const char* kLyapunovFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uLut;
uniform vec4  uView;      // aMin, aMax, bMin, bMax
uniform int   uSeq[64];
uniform int   uSeqLen;
uniform int   uWarmup;
uniform int   uIter;
uniform vec3  uNeg;
uniform float uColorScale;
uniform float uColorPhase;
uniform int   uBlend;
out vec4 fragColor;
void main()
{
    float a = mix(uView.x, uView.y, vTex.x);
    float b = mix(uView.z, uView.w, vTex.y);
    float x = 0.5;
    for (int i = 0; i < uWarmup; ++i){ float r = (uSeq[i % uSeqLen] == 0) ? a : b; x = r * x * (1.0 - x); }
    float lyap = 0.0;
    for (int i = 0; i < uIter; ++i){
        float r = (uSeq[i % uSeqLen] == 0) ? a : b;
        x = r * x * (1.0 - x);
        lyap += log(abs(r * (1.0 - 2.0 * x)) + 1e-9);
    }
    lyap /= float(uIter);
    vec3 col;
    if (lyap < 0.0) col = uNeg * clamp(-lyap, 0.0, 1.0);
    else { float t = fract(lyap * uColorScale + uColorPhase); col = texture(uLut, vec2(t, 0.5)).rgb; }
    if (uBlend == 0) fragColor = vec4(col, 1.0);
    else {
        vec3 d = texture(uTex, vTex).rgb;
        if (uBlend == 1) fragColor = vec4(min(d + col, vec3(1.0)), 1.0);
        else             fragColor = vec4((d + col) * 0.5, 1.0);
    }
}
)";

// Kleinian (Batch H): a stylized hyperbolic {p,q} tiling on the Poincaré disk via
// wedge folding + inversion in an orthogonal circle; reflection count → LUT.
// Geometry is sight-test-calibrated (recognizable tiling, not a rigorous group).
const char* kKleinianFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uLut;
uniform float uAspect;
uniform float uZoom;
uniform float uRot;
uniform int   uP;
uniform int   uQ;
uniform int   uIters;
uniform float uMorph;
uniform float uColorScale;
uniform float uColorPhase;
uniform int   uBlend;
out vec4 fragColor;
const float PI = 3.14159265;
void main()
{
    vec2 uvp = (vTex - 0.5) * 2.0;
    uvp.x *= uAspect;
    float s = sin(uRot), co = cos(uRot);
    uvp = mat2(co, -s, s, co) * uvp;
    vec2 z = uvp / uZoom;
    vec3 col = vec3(0.0);
    if (dot(z, z) < 1.0){
        float wedge = 2.0 * PI / float(uP);
        float rc = (1.0 / max(tan(PI / float(uQ)), 1e-3)) * (1.0 + 0.3 * sin(uMorph));
        float cx = sqrt(rc * rc + 1.0);
        int count = 0;
        for (int i = 0; i < uIters; ++i){
            float ang = atan(z.y, z.x);
            float r = length(z);
            ang = mod(ang, wedge);
            if (ang > wedge * 0.5) ang = wedge - ang;
            z = r * vec2(cos(ang), sin(ang));
            vec2 dvec = z - vec2(cx, 0.0);
            float dd = dot(dvec, dvec);
            if (dd < rc * rc){ z = vec2(cx, 0.0) + dvec * (rc * rc / dd); count++; }
            else break;
        }
        float t = fract(float(count) * uColorScale + uColorPhase);
        col = texture(uLut, vec2(t, 0.5)).rgb;
        col *= smoothstep(1.0, 0.88, length(uvp / uZoom));
    }
    if (uBlend == 0) fragColor = vec4(col, 1.0);
    else {
        vec3 d = texture(uTex, vTex).rgb;
        if (uBlend == 1) fragColor = vec4(min(d + col, vec3(1.0)), 1.0);
        else             fragColor = vec4((d + col) * 0.5, 1.0);
    }
}
)";

// Reaction-Diffusion (Batch H, Gray-Scott). uMode: 0 seed (A=1, centre blob of B),
// 1 sim step (9-point Laplacian), 2 show (B → LUT, blended over the frame).
const char* kReactionDiffusionFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uState;
uniform sampler2D uTex;
uniform sampler2D uLut;
uniform vec2  uTexel;
uniform int   uMode;
uniform float uFeed;
uniform float uKill;
uniform float uDA;
uniform float uDB;
uniform float uColorScale;
uniform float uColorPhase;
uniform int   uBlend;
uniform bool  uDoSeed;
uniform vec2  uSeed;
uniform float uSeedR;
out vec4 fragColor;
void main()
{
    if (uMode == 0){
        float d = distance(vTex, vec2(0.5));
        fragColor = vec4(1.0, d < 0.06 ? 1.0 : 0.0, 0.0, 1.0);
        return;
    }
    if (uMode == 1){
        vec2 c = texture(uState, vTex).rg;
        vec2 lap = vec2(0.0);
        lap += texture(uState, vTex + vec2(-1.0, 0.0) * uTexel).rg * 0.2;
        lap += texture(uState, vTex + vec2( 1.0, 0.0) * uTexel).rg * 0.2;
        lap += texture(uState, vTex + vec2( 0.0,-1.0) * uTexel).rg * 0.2;
        lap += texture(uState, vTex + vec2( 0.0, 1.0) * uTexel).rg * 0.2;
        lap += texture(uState, vTex + vec2(-1.0,-1.0) * uTexel).rg * 0.05;
        lap += texture(uState, vTex + vec2( 1.0,-1.0) * uTexel).rg * 0.05;
        lap += texture(uState, vTex + vec2(-1.0, 1.0) * uTexel).rg * 0.05;
        lap += texture(uState, vTex + vec2( 1.0, 1.0) * uTexel).rg * 0.05;
        lap -= c;
        float A = c.r, B = c.g;
        float reaction = A * B * B;
        float na = A + (uDA * lap.r - reaction + uFeed * (1.0 - A));
        float nb = B + (uDB * lap.g + reaction - (uKill + uFeed) * B);
        if (uDoSeed && distance(vTex, uSeed) < uSeedR) nb = 1.0;
        fragColor = vec4(clamp(na, 0.0, 1.0), clamp(nb, 0.0, 1.0), 0.0, 1.0);
        return;
    }
    float B = texture(uState, vTex).g;
    float t = fract(B * uColorScale + uColorPhase);
    vec3 col = texture(uLut, vec2(t, 0.5)).rgb;
    if (uBlend == 0) fragColor = vec4(col, 1.0);
    else {
        vec3 d = texture(uTex, vTex).rgb;
        if (uBlend == 1) fragColor = vec4(min(d + col, vec3(1.0)), 1.0);
        else             fragColor = vec4((d + col) * 0.5, 1.0);
    }
}
)";

QVector3D colorToVec(uint32_t color)
{
    return {static_cast<float>((color >> 16) & 0xFF) / 255.0f,
            static_cast<float>((color >> 8) & 0xFF) / 255.0f,
            static_cast<float>(color & 0xFF) / 255.0f};
}

} // namespace

MultiEffectVisualizer::MultiEffectVisualizer()
    : VisualizerBase("multieffect", "Multi Effect",
                     "AVS-style effect chain host (import target)"),
      m_scriptContext(std::make_shared<lumi::scripting::ScriptContext>())
{
    // Hand-built default chain: root trails + a nested additive list with its
    // own faster orbit — visible proof of nesting + blend (5.2 sight test).
    ChainNode root;
    root.params = ListParams{};

    ChainNode fade;
    fade.params = FadeoutParams{12, 0x000000};
    root.children.push_back(std::move(fade));

    // Real SuperScope (E6): an audio-reactive circular scope via the shared
    // ScopeRenderer — replaces the 5.1 DebugBars placeholder as content source.
    ChainNode scope;
    SuperScopeParams scopeParams;
    scopeParams.frameCode = "t=t+0.02";
    scopeParams.pointCode =
        "d=0.35+v*0.35; x=cos(i*6.28318+t)*d; y=sin(i*6.28318+t)*d; "
        "red=0.5+0.5*sin(i*6.28318); green=0.6; blue=1-red";
    scopeParams.pointCount = 360;
    scopeParams.renderMode = 1;  // lines
    scopeParams.lineWidth = 2.0f;
    scope.params = std::move(scopeParams);
    root.children.push_back(std::move(scope));

    // Colorfade cycles the trail colors — visible proof of the 5.3 effects.
    ChainNode fade5_3;
    fade5_3.params = ColorfadeParams{};
    root.children.push_back(std::move(fade5_3));

    ChainNode nested;
    ListParams nestedParams;
    nestedParams.blendIn = BlendMode::Ignore;    // keep own trails
    nestedParams.blendOut = BlendMode::Additive; // add onto the parent image
    nested.params = nestedParams;

    ChainNode nestedFade;
    nestedFade.params = FadeoutParams{4, 0x000000};
    nested.children.push_back(std::move(nestedFade));

    ChainNode nestedBars;
    nestedBars.params = DebugBarsParams{0x40C0FF, -2.5f};
    nested.children.push_back(std::move(nestedBars));

    root.children.push_back(std::move(nested));

    setChain(std::move(root));
}

CompileResult MultiEffectVisualizer::setChain(ChainNode root)
{
    m_root = std::move(root);
    return recompileChain();
}

CompileResult MultiEffectVisualizer::recompileChain()
{
    return compileChain(m_root);
}

namespace {
/// Resolve Picture filenames against the .avs directory and base64-embed the
/// image bytes into the params (decision: self-contained .lvfx). Missing files
/// leave imageData empty (Picture then renders as a no-op) + a report note.
void embedOneImage(std::string& filename, std::string& imageData, const char* label,
                   const QString& baseDir, QStringList* report)
{
    if (!imageData.empty() || filename.empty()) return;
    const QString fname = QString::fromStdString(filename);
    QString resolved = QDir(baseDir).filePath(fname);
    if (!QFileInfo::exists(resolved))
        resolved = QDir(baseDir).filePath(QFileInfo(fname).fileName());
    if (QFileInfo::exists(resolved))
    {
        QFile file(resolved);
        if (file.open(QIODevice::ReadOnly))
            imageData = file.readAll().toBase64().toStdString();
    }
    if (imageData.empty() && report != nullptr)
        report->append(QStringLiteral("%1: image not found: %2")
                           .arg(QString::fromLatin1(label), fname));
}

void embedPictureImages(ChainNode& node, const QString& baseDir, QStringList* report)
{
    if (auto* p = std::get_if<PictureParams>(&node.params))
        embedOneImage(p->filename, p->imageData, "Picture", baseDir, report);
    else if (auto* p = std::get_if<PictureIIParams>(&node.params))
        embedOneImage(p->filename, p->imageData, "Picture II", baseDir, report);
    else if (auto* p = std::get_if<TexerParams>(&node.params))
        embedOneImage(p->filename, p->imageData, "Texer", baseDir, report);
    else if (auto* p = std::get_if<TexerIIParams>(&node.params))
        embedOneImage(p->filename, p->imageData, "Texer II", baseDir, report);
    for (ChainNode& child : node.children) embedPictureImages(child, baseDir, report);
}
}  // namespace

bool MultiEffectVisualizer::loadAvsFile(const QString& path, QStringList* outReport)
{
    const lumi::avs::ParseResult parsed =
        lumi::avs::parseFile(std::filesystem::path(path.toStdWString()));
    TranslationResult translated = translateAvsTree(parsed);

    if (outReport != nullptr)
    {
        outReport->clear();
        for (const std::string& line : translated.report)
        {
            outReport->append(QString::fromStdString(line));
        }
    }

    // Resolve + embed Picture images relative to the .avs directory (appends any
    // "image not found" notes after the translation report).
    embedPictureImages(translated.root, QFileInfo(path).absolutePath(), outReport);
    // The new tree reuses node ids 1..N, colliding with the old runtimes; flag
    // a reset so the render thread frees their GL objects and starts fresh.
    m_pendingRuntimeReset = true;
    setChain(std::move(translated.root));
    return parsed.ok;
}

namespace {

/// N1: eine Wurzel-Liste mit genau einem Milkdrop-Meganode (Entscheid E1)
ChainNode makeMilkdropRoot(lumi::milkdrop::PresetState state, const QString& presetDir)
{
    MilkdropNodeParams p;
    p.presetDir = presetDir.toStdString();
    p.revision = 1;
    ChainNode node;
    node.displayName = state.name.empty() ? std::string("Milkdrop") : state.name;
    p.preset = std::move(state);
    node.params = std::move(p);

    ChainNode root;
    root.params = ListParams{};
    root.children.push_back(std::move(node));
    return root;
}

} // namespace

bool MultiEffectVisualizer::loadMilkFile(const QString& path, QStringList* outReport)
{
    if (outReport != nullptr) outReport->clear();
    const lumi::milk::ParseResult parsed =
        lumi::milk::parseFile(std::filesystem::path(path.toStdWString()));
    if (!parsed.ok)
    {
        if (outReport != nullptr)
        {
            outReport->append(QStringLiteral("Parser: %1")
                                  .arg(QString::fromStdString(parsed.error)));
        }
        return false;
    }
    if (outReport != nullptr)
    {
        for (const std::string& w : parsed.warnings)
        {
            outReport->append(QStringLiteral("Parser: %1").arg(QString::fromStdString(w)));
        }
        if (!parsed.sprites.empty())
        {
            outReport->append(QStringLiteral("ℹ %1 Sprite(s) — werden gerendert "
                                             "(Bild-Status folgt unten)")
                                  .arg(parsed.sprites.size()));
        }
    }

    lumi::milkdrop::PresetState state = lumi::milkdrop::translate(parsed);
    state.name = QFileInfo(path).completeBaseName().toStdString();
    const QString dir = QFileInfo(path).absolutePath();

    // Report-Paritaet zum frueheren Standalone-Import: eine GL-freie Wegwerf-
    // Instanz erzeugt dieselben Klassifikations-/Transpile-/Textur-Meldungen
    // (der Render-Thread uebernimmt den Node-State spaeter still per Revision)
    {
        MilkdropVisualizer probe;
        probe.applyPresetState(state, dir, outReport);
    }

    m_pendingRuntimeReset = true;  // neue Node-Ids — alte GL-Runtimes freigeben
    setChain(makeMilkdropRoot(std::move(state), dir));
    return true;
}

bool MultiEffectVisualizer::loadMilkDocument(const QString& path, QStringList* outReport)
{
    if (outReport != nullptr) outReport->clear();
    lumi::milkdrop::PresetState state;
    if (!lumi::milkdrop::loadPresetFromFile(path, state, outReport)) return false;
    if (state.name.empty())
    {
        state.name = QFileInfo(path).completeBaseName().toStdString();
    }
    const QString dir = QFileInfo(path).absolutePath();
    {
        MilkdropVisualizer probe;
        probe.applyPresetState(state, dir, outReport);
    }
    m_pendingRuntimeReset = true;
    setChain(makeMilkdropRoot(std::move(state), dir));
    return true;
}

bool MultiEffectVisualizer::saveChainFile(const QString& path) const
{
    return saveChainToFile(m_root, path);
}

bool MultiEffectVisualizer::loadChainFile(const QString& path, QStringList* outReport)
{
    ChainNode loaded;
    if (!loadChainFromFile(path, loaded, outReport)) return false;
    m_pendingRuntimeReset = true;  // new node ids — free old GL runtimes (render thread)
    m_root = std::move(loaded);
    return true;
}

// =============================================================================
// GL lifecycle (render thread)
// =============================================================================

void MultiEffectVisualizer::onInitialize()
{
    m_firstFrame = true;
    m_time = 0.0f;
    m_audioLevel = 0.0f;
    m_frameBeat = false;
    ensurePipelines();
}

void MultiEffectVisualizer::onResize(const QSize& size)
{
    Q_UNUSED(size);
    // Surfaces are (re)sized lazily in onRender from the actual GL viewport
    // (physical pixels — the logical QSize misses the devicePixelRatio).
}

void MultiEffectVisualizer::onCleanup()
{
    destroySurfaces();
    resetRuntimes();
    m_scopeRenderer.destroy();
    m_fadeShader.reset();
    m_invertShader.reset();
    m_barsShader.reset();
    m_blendShader.reset();
    m_brightShader.reset();
    m_blurShader.reset();
    m_mirrorShader.reset();
    m_colorfadeShader.reset();
    m_lutShader.reset();
    m_warpShader.reset();
    m_feedbackShader.reset();
    m_mosaicShader.reset();
    m_grainShader.reset();
    m_scatterShader.reset();
    m_interfShader.reset();
    m_waterShader.reset();
    m_bumpShader.reset();
    m_shiftShader.reset();
    m_ddmShader.reset();
    m_colorMapShader.reset();
    m_bufferBlendShader.reset();
    m_pictureShader.reset();
    m_spriteShader.reset();
    m_colorClipShader.reset();
    m_uniqueToneShader.reset();
    m_interleaveShader.reset();
    m_convolutionShader.reset();
    m_normaliseShader.reset();
    m_multiFilterShader.reset();
    m_addBordersShader.reset();
    m_reduceFbo.reset();
    m_wbPropShader.reset();
    m_wbDispShader.reset();
    m_timescopeShader.reset();
    m_apeShader.reset();
    m_fractal2DShader.reset();
    m_domainWarpShader.reset();
    m_fractal3DShader.reset();
    m_lyapunovShader.reset();
    m_kleinianShader.reset();
    m_rdShader.reset();
    if (m_specTex != 0)
    {
        if (auto* ctx = QOpenGLContext::currentContext())
            ctx->functions()->glDeleteTextures(1, &m_specTex);
        m_specTex = 0;
    }
    m_quadVao.reset();
    m_quadVbo.reset();
    m_warpVao.reset();
    m_warpVbo.reset();
}

bool MultiEffectVisualizer::ensurePipelines()
{
    if (m_fadeShader != nullptr) return true;

    auto makeProgram = [](const char* vs, const char* fs) {
        auto program = std::make_unique<QOpenGLShaderProgram>();
        if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vs) ||
            !program->addShaderFromSourceCode(QOpenGLShader::Fragment, fs) ||
            !program->link())
        {
            return std::unique_ptr<QOpenGLShaderProgram>{};
        }
        return program;
    };

    m_fadeShader = makeProgram(kQuadVertexShader, kFadeoutFragmentShader);
    m_invertShader = makeProgram(kQuadVertexShader, kInvertFragmentShader);
    m_barsShader = makeProgram(kBarsVertexShader, kBarsFragmentShader);
    m_blendShader = makeProgram(kQuadVertexShader, kBlendFragmentShader);
    m_brightShader = makeProgram(kQuadVertexShader, kBrightnessFragmentShader);
    m_blurShader = makeProgram(kQuadVertexShader, kBlurFragmentShader);
    m_mirrorShader = makeProgram(kQuadVertexShader, kMirrorFragmentShader);
    m_colorfadeShader = makeProgram(kQuadVertexShader, kColorfadeFragmentShader);
    m_lutShader = makeProgram(kQuadVertexShader, kLutFragmentShader);
    m_warpShader = makeProgram(kWarpVertexShader, kWarpFragmentShader);
    m_feedbackShader = makeProgram(kQuadVertexShader, kFeedbackFragmentShader);
    m_mosaicShader = makeProgram(kQuadVertexShader, kMosaicFragmentShader);
    m_grainShader = makeProgram(kQuadVertexShader, kGrainFragmentShader);
    m_scatterShader = makeProgram(kQuadVertexShader, kScatterFragmentShader);
    m_interfShader = makeProgram(kQuadVertexShader, kInterfFragmentShader);
    m_waterShader = makeProgram(kQuadVertexShader, kWaterFragmentShader);
    m_bumpShader = makeProgram(kQuadVertexShader, kBumpFragmentShader);
    m_shiftShader = makeProgram(kQuadVertexShader, kDynamicShiftFragmentShader);
    m_ddmShader = makeProgram(kQuadVertexShader, kDdmFragmentShader);
    m_colorMapShader = makeProgram(kQuadVertexShader, kColorMapFragmentShader);
    m_bufferBlendShader = makeProgram(kQuadVertexShader, kBufferBlendFragmentShader);
    m_pictureShader = makeProgram(kQuadVertexShader, kPictureFragmentShader);
    m_spriteShader = makeProgram(kSpriteVertexShader, kSpriteFragmentShader);
    m_colorClipShader = makeProgram(kQuadVertexShader, kColorClipFragmentShader);
    m_uniqueToneShader = makeProgram(kQuadVertexShader, kUniqueToneFragmentShader);
    m_interleaveShader = makeProgram(kQuadVertexShader, kInterleaveFragmentShader);
    m_convolutionShader = makeProgram(kQuadVertexShader, kConvolutionFragmentShader);
    m_normaliseShader = makeProgram(kQuadVertexShader, kNormaliseFragmentShader);
    m_multiFilterShader = makeProgram(kQuadVertexShader, kMultiFilterFragmentShader);
    m_addBordersShader = makeProgram(kQuadVertexShader, kAddBordersFragmentShader);
    m_wbPropShader = makeProgram(kQuadVertexShader, kWaterBumpPropShader);
    m_wbDispShader = makeProgram(kQuadVertexShader, kWaterBumpDispShader);
    m_timescopeShader = makeProgram(kQuadVertexShader, kTimescopeFragmentShader);
    m_apeShader = makeProgram(kQuadVertexShader, kApeFragmentShader);
    m_fractal2DShader = makeProgram(kQuadVertexShader, kFractal2DFragmentShader);
    m_domainWarpShader = makeProgram(kQuadVertexShader, kDomainWarpFragmentShader);
    m_fractal3DShader = makeProgram(kQuadVertexShader, kFractal3DFragmentShader);
    m_lyapunovShader = makeProgram(kQuadVertexShader, kLyapunovFragmentShader);
    m_kleinianShader = makeProgram(kQuadVertexShader, kKleinianFragmentShader);
    m_rdShader = makeProgram(kQuadVertexShader, kReactionDiffusionFragmentShader);
    if (m_fadeShader == nullptr || m_invertShader == nullptr ||
        m_barsShader == nullptr || m_blendShader == nullptr ||
        m_brightShader == nullptr || m_blurShader == nullptr ||
        m_mirrorShader == nullptr || m_colorfadeShader == nullptr ||
        m_lutShader == nullptr || m_warpShader == nullptr ||
        m_feedbackShader == nullptr || m_mosaicShader == nullptr ||
        m_grainShader == nullptr || m_scatterShader == nullptr ||
        m_interfShader == nullptr || m_waterShader == nullptr ||
        m_bumpShader == nullptr || m_shiftShader == nullptr ||
        m_ddmShader == nullptr || m_colorMapShader == nullptr ||
        m_bufferBlendShader == nullptr || m_pictureShader == nullptr ||
        m_spriteShader == nullptr || m_colorClipShader == nullptr ||
        m_uniqueToneShader == nullptr || m_interleaveShader == nullptr ||
        m_convolutionShader == nullptr || m_normaliseShader == nullptr ||
        m_multiFilterShader == nullptr || m_addBordersShader == nullptr ||
        m_wbPropShader == nullptr ||
        m_wbDispShader == nullptr || m_timescopeShader == nullptr ||
        m_apeShader == nullptr || m_fractal2DShader == nullptr ||
        m_domainWarpShader == nullptr || m_fractal3DShader == nullptr ||
        m_lyapunovShader == nullptr || m_kleinianShader == nullptr ||
        m_rdShader == nullptr)
    {
        m_fadeShader.reset();
        m_invertShader.reset();
        m_barsShader.reset();
        m_blendShader.reset();
        m_brightShader.reset();
        m_blurShader.reset();
        m_mirrorShader.reset();
        m_colorfadeShader.reset();
            m_lutShader.reset();
        m_warpShader.reset();
        m_feedbackShader.reset();
        m_mosaicShader.reset();
        m_grainShader.reset();
        m_scatterShader.reset();
        m_interfShader.reset();
        m_waterShader.reset();
        m_bumpShader.reset();
        m_wbPropShader.reset();
        m_wbDispShader.reset();
        m_timescopeShader.reset();
        m_apeShader.reset();
        m_fractal2DShader.reset();
        m_domainWarpShader.reset();
        m_fractal3DShader.reset();
        m_lyapunovShader.reset();
        m_kleinianShader.reset();
        m_rdShader.reset();
        return false;
    }

    auto* f = QOpenGLContext::currentContext()->functions();

    static const float kQuad[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
    m_quadVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_quadVao->create();
    m_quadVao->bind();
    m_quadVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_quadVbo->create();
    m_quadVbo->bind();
    m_quadVbo->allocate(kQuad, sizeof(kQuad));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    m_quadVao->release();
    m_quadVbo->release();

    // Dynamic mesh for the grid-warp effects (pos.xy + tex.xy + alpha,
    // re-uploaded per frame).
    m_warpVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_warpVao->create();
    m_warpVao->bind();
    m_warpVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_warpVbo->create();
    m_warpVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_warpVbo->bind();
    m_warpVbo->allocate(nullptr, 0);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                             reinterpret_cast<void*>(2 * sizeof(float)));
    f->glEnableVertexAttribArray(2);
    f->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                             reinterpret_cast<void*>(4 * sizeof(float)));
    m_warpVao->release();
    m_warpVbo->release();

    m_scopeRenderer.ensure();  // shared scope draw (SuperScope effect, E6)
    return true;
}

bool MultiEffectVisualizer::ensureSurfacePair(SurfacePair& pair, int width,
                                              int height, bool* outResized)
{
    if (outResized != nullptr) *outResized = false;
    if (width <= 0 || height <= 0) return false;
    if (pair.ready() && pair.fbo[0]->width() == width &&
        pair.fbo[0]->height() == height)
    {
        return true;
    }

    // Resize policy 5.2: discard content (list-buffer blit is a later polish).
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    for (auto& fbo : pair.fbo)
    {
        fbo = std::make_unique<QOpenGLFramebufferObject>(width, height, format);
        if (!fbo->isValid())
        {
            pair.destroy();
            return false;
        }
    }
    pair.currentIndex = 0;
    if (outResized != nullptr) *outResized = true;
    return true;
}

void MultiEffectVisualizer::resetRuntimes()
{
    if (auto* ctx = QOpenGLContext::currentContext())
    {
        auto* f = ctx->functions();
        for (auto& [id, rt] : m_leafRuntimes)
        {
            if (rt.lutTexture != 0) f->glDeleteTextures(1, &rt.lutTexture);
            if (rt.cmTexture != 0) f->glDeleteTextures(1, &rt.cmTexture);
            if (rt.picTexture != 0) f->glDeleteTextures(1, &rt.picTexture);
            if (rt.fracLut != 0) f->glDeleteTextures(1, &rt.fracLut);
            // Meganode: der Milkdrop-Kern gibt seine GL-Objekte selbst frei
            // (braucht den current Context — deshalb hier, nicht im Dtor)
            if (rt.milk != nullptr) rt.milk->cleanup();
        }
    }
    m_listRuntimes.clear();  // slot hosts / FBOs die with their GL-frame owner
    m_leafRuntimes.clear();
    m_groupRuntimes.clear();  // HG1: Gruppen-Surfaces/-Pools sterben mit
    m_bufferPool.clear();
    for (auto& ring : m_mdRing) ring.clear();  // Multi Delay shared rings
    for (int& head : m_mdHead) head = 0;
    m_mdW = 0;
    m_mdH = 0;
}

void MultiEffectVisualizer::destroySurfaces()
{
    m_rootSurface.destroy();
    m_bufferScratch.destroy();
    for (auto& [id, runtime] : m_listRuntimes)
    {
        runtime.surface.destroy();
    }
    for (auto& [id, runtime] : m_groupRuntimes)
    {
        runtime.surface.destroy();
        if (runtime.pool != nullptr) runtime.pool->clear();
    }
    m_surfaceStack.clear();
    m_surfaceWidth = 0;
    m_surfaceHeight = 0;
}

void MultiEffectVisualizer::bindActive()
{
    auto* f = QOpenGLContext::currentContext()->functions();
    active().current()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
}

// =============================================================================
// Frame
// =============================================================================

void MultiEffectVisualizer::onRender(float deltaTime)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    m_time += deltaTime;
    m_deltaTime = deltaTime;

    // A freshly installed chain (loadAvsFile) reuses node ids — free the old
    // per-node GL runtimes here on the render thread before walking the new tree.
    if (m_pendingRuntimeReset)
    {
        resetRuntimes();
        m_pendingRuntimeReset = false;
        m_firstFrame = true;
    }

    // Waveform RMS: smoothed level for DebugBars/vol.
    const std::vector<float> waveform = getWaveform();
    float rms = 0.0f;
    if (!waveform.empty())
    {
        float sum = 0.0f;
        for (float sample : waveform) sum += sample * sample;
        rms = std::sqrt(sum / static_cast<float>(waveform.size()));
        m_audioLevel += (rms - m_audioLevel) * 0.3f;
    }

    // Chain-scoped beat, AVS-faithful (ref main.cpp:290-329): onset from the
    // per-channel mean |waveform| (max of L/R), then refined/predicted by the
    // bpm.cpp port — its return value IS the beat, exactly like the original
    // (b=refineBeat(avs_beat)). List scripts and Custom BPM may still mutate
    // m_frameBeat mid-chain.
    auto meanAbs = [](const std::vector<float>& v) {
        if (v.empty()) return 0.0f;
        float sum = 0.0f;
        for (float sample : v) sum += std::abs(sample);
        return sum / static_cast<float>(v.size());
    };
    const float level =
        std::max(meanAbs(getWaveformChannel(0)), meanAbs(getWaveformChannel(1)));
    const bool onset = m_beat.updateAvsOnset(level);
    m_frameBeat =
        m_beatEstimator.refine(onset, lumi::modules::BeatEstimator::steadyNowMs());

    // Working surface in physical pixels (the GL viewport is authoritative).
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    bool resized = false;
    if (!ensurePipelines() ||
        !ensureSurfacePair(m_rootSurface, viewport[2], viewport[3], &resized))
    {
        return;
    }
    m_surfaceWidth = viewport[2];
    m_surfaceHeight = viewport[3];
    if (resized) m_firstFrame = true;

    const GLboolean blendWasEnabled = f->glIsEnabled(GL_BLEND);
    f->glDisable(GL_BLEND);

    m_surfaceStack.clear();
    m_surfaceStack.push_back(&m_rootSurface);
    for (auto& [id, runtime] : m_listRuntimes) runtime.seenThisFrame = false;
    for (auto& [id, runtime] : m_groupRuntimes) runtime.seenThisFrame = false;
    m_renderMode = RenderMode{};  // Set Render Mode is per-frame, reset before the walk
    buildVisData();               // audio for getspec/getosc, shared by all scripts
    bindActive();

    const auto& rootList = std::get<ListParams>(m_root.params);
    if (m_firstFrame || rootList.clearEveryFrame)
    {
        f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
    }

    for (const ChainNode& child : m_root.children)
    {
        renderNode(child);
    }

    // Present: blit the root surface to the window framebuffer.
    m_rootSurface.current()->release();
    if (QOpenGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        auto* extra = QOpenGLContext::currentContext()->extraFunctions();
        extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rootSurface.current()->handle());
        extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, viewport[0],
                                 viewport[1], viewport[0] + viewport[2],
                                 viewport[1] + viewport[3], GL_COLOR_BUFFER_BIT,
                                 GL_NEAREST);
        extra->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    f->glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Drop runtimes of nodes no longer in the chain (edited away).
    for (auto it = m_listRuntimes.begin(); it != m_listRuntimes.end();)
    {
        it = it->second.seenThisFrame ? std::next(it) : m_listRuntimes.erase(it);
    }
    for (auto it = m_groupRuntimes.begin(); it != m_groupRuntimes.end();)
    {
        it = it->second.seenThisFrame ? std::next(it) : m_groupRuntimes.erase(it);
    }

    if (blendWasEnabled == GL_TRUE) f->glEnable(GL_BLEND);
    m_firstFrame = false;
}

void MultiEffectVisualizer::renderNode(const ChainNode& node)
{
    if (!node.enabled)
    {
        // r_list fake_enabled (r_list.h:162): a statically DISABLED list with
        // "on beat render" still activates for N frames after each beat — the
        // window gate lives in renderList; everything else stays skipped.
        // HG2: eine deaktivierte Host-Gruppe rendert weiter, solange ihr
        // Blend-Gewicht > 0 ist (Ausblend-Phase des Crossfades).
        const auto* list = std::get_if<ListParams>(&node.params);
        if (list == nullptr || !list->onBeatRender)
        {
            if (!node.isHostGroup()) return;
            const auto it = m_groupRuntimes.find(node.nodeId);
            if (it == m_groupRuntimes.end() || it->second.blendWeight <= 0.0)
            {
                return;
            }
        }
    }

    struct Visitor
    {
        MultiEffectVisualizer& self;
        const ChainNode& node;

        void operator()(const ListParams& params) const { self.renderList(node, params); }
        void operator()(const HostGroupParams& params) const { self.renderHostGroup(node, params); }
        void operator()(const ClearParams& params) const { self.runClear(params); }
        void operator()(const FadeoutParams& params) const { self.runFadeout(params); }
        void operator()(const InvertParams&) const { self.runInvert(); }
        void operator()(const BrightnessParams& params) const { self.runBrightness(params); }
        void operator()(const FastBrightnessParams& params) const { self.runFastBrightness(params); }
        void operator()(const BlurParams& params) const { self.runBlur(params); }
        void operator()(const MirrorParams& params) const { self.runMirror(node, params); }
        void operator()(const OnBeatClearParams& params) const { self.runOnBeatClear(node, params); }
        void operator()(const ColorfadeParams& params) const { self.runColorfade(node, params); }
        void operator()(const ColorModifierParams& params) const { self.runColorModifier(node, params); }
        void operator()(const MovementParams& params) const { self.runMovement(node, params); }
        void operator()(const DynamicMovementParams& params) const { self.runDynamicMovement(node, params); }
        void operator()(const DynamicShiftParams& params) const { self.runDynamicShift(node, params); }
        void operator()(const DynamicDistanceModifierParams& params) const { self.runDynamicDistanceModifier(node, params); }
        void operator()(const MovingParticleParams& params) const { self.runMovingParticle(node, params); }
        void operator()(const BlitterFeedbackParams& params) const { self.runBlitterFeedback(params); }
        void operator()(const RotoBlitterParams& params) const { self.runRotoBlitter(node, params); }
        void operator()(const BufferSaveParams& params) const { self.runBufferSave(node, params); }
        void operator()(const CustomBpmParams& params) const { self.runCustomBpm(node, params); }
        void operator()(const SetRenderModeParams& params) const { self.runSetRenderMode(params); }
        void operator()(const SuperScopeParams& params) const { self.runSuperScope(node, params); }
        void operator()(const MosaicParams& params) const { self.runMosaic(node, params); }
        void operator()(const GrainParams& params) const { self.runGrain(params); }
        void operator()(const ScatterParams& params) const { self.runScatter(params); }
        void operator()(const InterferencesParams& params) const { self.runInterferences(node, params); }
        void operator()(const WaterParams& params) const { self.runWater(node, params); }
        void operator()(const BumpParams& params) const { self.runBump(node, params); }
        void operator()(const WaterBumpParams& params) const { self.runWaterBump(node, params); }
        void operator()(const FyrewurXParams& params) const { self.runFyrewurX(node, params); }
        void operator()(const StarfieldParams& params) const { self.runStarfield(node, params); }
        void operator()(const TimescopeParams& params) const { self.runTimescope(node, params); }
        void operator()(const DotGridParams& params) const { self.runDotGrid(node, params); }
        void operator()(const DotPlaneParams& params) const { self.runDotPlane(node, params); }
        void operator()(const DotFountainParams& params) const { self.runDotFountain(node, params); }
        void operator()(const ColorMapParams& params) const { self.runColorMap(node, params); }
        void operator()(const BufferBlendParams& params) const { self.runBufferBlend(params); }
        void operator()(const JherikoGlobalParams& params) const { self.runJherikoGlobal(node, params); }
        void operator()(const ColorClipParams& params) const { self.runColorClip(params); }
        void operator()(const UniqueToneParams& params) const { self.runUniqueTone(params); }
        void operator()(const InterleaveParams& params) const { self.runInterleave(node, params); }
        void operator()(const ConvolutionParams& params) const { self.runConvolution(params); }
        void operator()(const NormaliseParams&) const { self.runNormalise(); }
        void operator()(const MultiFilterParams& params) const { self.runMultiFilter(params); }
        void operator()(const AddBordersParams& params) const { self.runAddBorders(params); }
        void operator()(const SimpleScopeParams& params) const { self.runSimpleScope(node, params); }
        void operator()(const BassSpinParams& params) const { self.runBassSpin(node, params); }
        void operator()(const OscStarParams& params) const { self.runOscStar(node, params); }
        void operator()(const OscRingParams& params) const { self.runOscRing(node, params); }
        void operator()(const RotatingStarsParams& params) const { self.runRotatingStars(node, params); }
        void operator()(const PictureParams& params) const { self.runPicture(node, params); }
        void operator()(const PictureIIParams& params) const { self.runPictureII(node, params); }
        void operator()(const TexerParams& params) const { self.runTexer(node, params); }
        void operator()(const TexerIIParams& params) const { self.runTexerII(node, params); }
        void operator()(const TriangleParams& params) const { self.runTriangle(node, params); }
        void operator()(const ChannelShiftParams& params) const { self.runChannelShift(node, params); }
        void operator()(const ColorReductionParams& params) const { self.runColorReduction(params); }
        void operator()(const MultiplierParams& params) const { self.runMultiplier(params); }
        void operator()(const VideoDelayParams& params) const { self.runVideoDelay(node, params); }
        void operator()(const MultiDelayParams& params) const { self.runMultiDelay(params); }
        void operator()(const Fractal2DParams& params) const { self.runFractal2D(node, params); }
        void operator()(const DomainWarpParams& params) const { self.runDomainWarp(node, params); }
        void operator()(const Fractal3DParams& params) const { self.runFractal3D(node, params); }
        void operator()(const LyapunovParams& params) const { self.runLyapunov(node, params); }
        void operator()(const KleinianParams& params) const { self.runKleinian(node, params); }
        void operator()(const FractalZoomerParams& params) const { self.runFractalZoomer(node, params); }
        void operator()(const StrangeAttractorParams& params) const { self.runStrangeAttractor(node, params); }
        void operator()(const FlameParams& params) const { self.runFlame(node, params); }
        void operator()(const ReactionDiffusionParams& params) const { self.runReactionDiffusion(node, params); }
        void operator()(const DebugBarsParams& params) const { self.runDebugBars(params); }
        void operator()(const MilkdropNodeParams& params) const { self.runMilkdropNode(node, params); }
        void operator()(const PassthroughParams&) const { /* conserved, no-op */ }
    };
    std::visit(Visitor{*this, node}, node.params);
}

// =============================================================================
// Lists (AVS thisfb semantics)
// =============================================================================

void MultiEffectVisualizer::renderList(const ChainNode& node,
                                       const ListParams& params)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    ListRuntime& runtime = m_listRuntimes[node.nodeId];
    runtime.seenThisFrame = true;

    // --- EEL list slots (lazy compile; errors disable silently — no render-
    //     thread logging, the host's lastError surfaces via the editor later)
    bool scriptEnabled = true;
    bool scriptClear = false;
    int alphaIn = params.inAdjustAlpha;
    int alphaOut = params.outAdjustAlpha;
    if (params.useCode)
    {
        if (runtime.slotHost == nullptr || runtime.compiledInit != params.initCode ||
            runtime.compiledFrame != params.frameCode)
        {
            runtime.slotHost = std::make_unique<ScriptSlotHost>(
                "effectlist", activeContext(), ScriptSlotHost::Dialect::Avs);
            runtime.slotHost->setSource(Slot::Init, params.initCode);
            runtime.slotHost->setSource(Slot::Frame, params.frameCode);
            runtime.slotHost->compileAll();
            runtime.compiledInit = params.initCode;
            runtime.compiledFrame = params.frameCode;
            auto& engine = runtime.slotHost->engine();
            engine.setNumber("enabled", 1.0);
            engine.setNumber("clear", 0.0);
            // EEL contract uses 0.0..1.0 (r_list.cpp:404 use_inblendval/255.0)
            engine.setNumber("alphain", static_cast<double>(alphaIn) / 255.0);
            engine.setNumber("alphaout", static_cast<double>(alphaOut) / 255.0);
            runtime.slotHost->run(Slot::Init);
        }
        if (runtime.slotHost->has(Slot::Frame))
        {
            auto& engine = runtime.slotHost->engine();
            feedAudio(engine);
            engine.setNumber("beat", m_frameBeat ? 1.0 : 0.0);
            engine.setNumber("w", static_cast<double>(m_surfaceWidth));
            engine.setNumber("h", static_cast<double>(m_surfaceHeight));
            runtime.slotHost->run(Slot::Frame);
            scriptEnabled = engine.number("enabled") != 0.0;
            scriptClear = engine.number("clear") != 0.0;
            m_frameBeat = engine.number("beat") != 0.0;  // beat is mutable (§5.1)
            // Back from the EEL 0..1 scale (r_list.cpp:413 alphain*255)
            alphaIn = std::clamp(
                static_cast<int>(engine.number("alphain") * 255.0), 0, 255);
            alphaOut = std::clamp(
                static_cast<int>(engine.number("alphaout") * 255.0), 0, 255);
        }
    }
    if (!scriptEnabled) return;

    // --- OnBeat activation window: gates ONLY statically disabled lists
    // (r_list enabled() = !bit1 || fake_enabled — an enabled list always
    // renders, beat_render has no effect on it).
    if (params.onBeatRender && !node.enabled)
    {
        if (m_frameBeat) runtime.beatFramesLeft = params.onBeatFrames;
        if (runtime.beatFramesLeft <= 0) return;
        --runtime.beatFramesLeft;
    }

    // --- Persistent list buffer (thisfb)
    bool resized = false;
    if (!ensureSurfacePair(runtime.surface, m_surfaceWidth, m_surfaceHeight, &resized))
    {
        return;
    }
    if (resized) runtime.needsClear = true;

    if (runtime.needsClear || params.clearEveryFrame || scriptClear)
    {
        runtime.surface.current()->bind();
        f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
        f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
        runtime.surface.current()->release();
        runtime.needsClear = false;
    }

    // --- In-blend: parent image -> list buffer (Ignore keeps own trails)
    const unsigned int parentTexture = active().current()->texture();
    if (params.blendIn != BlendMode::Ignore)
    {
        const unsigned int bufTex =
            params.blendIn == BlendMode::Buffer ? poolTexture(params.bufferIn) : 0;
        blendPass(runtime.surface, parentTexture, params.blendIn, alphaIn, bufTex,
                  params.bufferInInvert);
    }

    // --- Children render on the list surface
    m_surfaceStack.push_back(&runtime.surface);
    bindActive();
    for (const ChainNode& child : node.children)
    {
        renderNode(child);
    }
    active().current()->release();
    m_surfaceStack.pop_back();

    // --- Out-blend: list buffer -> parent
    if (params.blendOut != BlendMode::Ignore)
    {
        const unsigned int bufTex =
            params.blendOut == BlendMode::Buffer ? poolTexture(params.bufferOut) : 0;
        blendPass(active(), runtime.surface.current()->texture(), params.blendOut,
                  alphaOut, bufTex, params.bufferOutInvert);
    }
    bindActive();  // chain continues on the parent's (possibly swapped) buffer
}

// =============================================================================
// Host groups (HG1/HG2 — HostGruppen_Crossfade_Entwurf.md)
// =============================================================================

namespace
{
/// HG2-Kurven-Hook: 0 = linear; weitere Kurven (ease/exponentiell/S) folgen —
/// der Entwurf haelt Ein-/Ausgangskurve bewusst je Gruppe individuell.
double applyBlendCurve(int curve, double t)
{
    (void)curve;  // 0 = linear; weitere Kurven bekommen hier ihren Dispatch
    return t;
}
}  // namespace

void MultiEffectVisualizer::renderHostGroup(const ChainNode& node,
                                            const HostGroupParams& params)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    GroupRuntime& runtime = m_groupRuntimes[node.nodeId];
    runtime.seenThisFrame = true;

    // --- HG2: Blend-Gewicht Richtung Soll bewegen (enabled=1, disabled=0).
    // Ein Wechsel A→B ist "A deaktivieren + B aktivieren": beide leben
    // waehrend des Blends (echtes Doppel-Rendering, Entscheid E5); ein
    // erneuter Toggle mitten im Blend kehrt einfach die Richtung um.
    {
        const double target = node.enabled ? 1.0 : 0.0;
        const double dur = params.crossfadeSeconds;
        if (dur <= 1e-6)
        {
            runtime.blendWeight = target;
        }
        else
        {
            const double step = static_cast<double>(m_deltaTime) / dur;
            if (runtime.blendWeight < target)
                runtime.blendWeight = std::min(target, runtime.blendWeight + step);
            else
                runtime.blendWeight = std::max(target, runtime.blendWeight - step);
        }
        if (!node.enabled && runtime.blendWeight <= 0.0)
        {
            // Fertig ausgeblendet: Buffer fuers naechste Einblenden frisch
            // starten (Milkdrop-Verhalten: der alte Zustand stirbt mit A)
            runtime.needsClear = true;
            return;
        }
    }
    if (runtime.pool == nullptr)
    {
        runtime.pool = std::make_unique<lumi::render::OffscreenBufferPool>();
    }
    if (runtime.context == nullptr)
    {
        runtime.context = std::make_shared<lumi::scripting::ScriptContext>();
    }

    // Persistenter Gruppen-Buffer — bewusst KEIN per-Frame-Clear: das Visual
    // regelt sein Feedback selbst (AVS-Clear-Effekte bzw. Milkdrop-decay)
    bool resized = false;
    if (!ensureSurfacePair(runtime.surface, m_surfaceWidth, m_surfaceHeight,
                           &resized))
    {
        return;
    }
    if (resized) runtime.needsClear = true;
    if (runtime.needsClear)
    {
        runtime.surface.current()->bind();
        f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
        f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
        runtime.surface.current()->release();
        runtime.needsClear = false;
    }

    // Scope-Wechsel (Stack-Disziplin): gruppen-eigener Buffer-Pool und
    // ScriptContext — Buffer-Save-Slots und reg/q/gmegabuf leaken nicht
    // zwischen Gruppen (Runtime-Trennung fuer den HG2-Crossfade)
    lumi::render::OffscreenBufferPool* prevPool = m_activePool;
    std::shared_ptr<lumi::scripting::ScriptContext> prevCtx = m_activeContext;
    m_activePool = runtime.pool.get();
    m_activeContext = runtime.context;

    m_surfaceStack.push_back(&runtime.surface);
    bindActive();
    for (const ChainNode& child : node.children)
    {
        renderNode(child);
    }
    active().current()->release();
    m_surfaceStack.pop_back();

    m_activePool = prevPool;
    m_activeContext = prevCtx;

    // Out-Blend: Gruppen-Bild -> Parent. Mehrere gleichzeitig aktive Gruppen
    // stapeln sich hierueber (Entwurf §2.6). HG2: bei Blend-Gewicht < 1 wird
    // linear per Adjustable gemischt — Einblenden folgt der Eingangs-, Aus-
    // blenden der Ausgangskurve der Gruppe (§2.4; sequentieller Mix in
    // Ketten-Reihenfolge, exakter paarweiser 2er-Mix = Feinschliff HG3).
    if (params.blendOut != BlendMode::Ignore)
    {
        const double curved = applyBlendCurve(
            node.enabled ? params.curveIn : params.curveOut, runtime.blendWeight);
        if (curved >= 0.999)
        {
            blendPass(active(), runtime.surface.current()->texture(),
                      params.blendOut, params.outAdjustAlpha, 0, false);
        }
        else
        {
            blendPass(active(), runtime.surface.current()->texture(),
                      BlendMode::Adjustable,
                      static_cast<int>(std::lround(std::clamp(curved, 0.0, 1.0) *
                                                   255.0)),
                      0, false);
        }
    }
    bindActive();
}

// =============================================================================
// Leaves
// =============================================================================

void MultiEffectVisualizer::runClear(const ClearParams& params)
{
    if (params.onlyFirst && !m_firstFrame) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    const QVector3D color = colorToVec(params.color);

    // Blend mode 3 follows the current Set-Render-Mode line blend (BLEND_LINE).
    int mode = params.blend;
    if (mode == 3) mode = m_renderMode.lineBlend;

    if (mode == 0)
    {
        f->glClearColor(color.x(), color.y(), color.z(), 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    // Additive / 50-50 clear: full-screen color quad with GL blending
    // (same pattern as runOnBeatClear).
    f->glEnable(GL_BLEND);
    if (mode == 1)
    {
        f->glBlendFunc(GL_ONE, GL_ONE);
    }
    else
    {
        f->glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
        f->glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);
    }
    m_barsShader->bind();
    m_quadVao->bind();
    m_barsShader->setUniformValue("uCenter", QVector2D(0.0f, 0.0f));
    m_barsShader->setUniformValue("uSize", QVector2D(1.0f, 1.0f));
    m_barsShader->setUniformValue("uColor", color);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_barsShader->release();
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runFadeout(const FadeoutParams& params)
{
    m_fadeShader->bind();
    m_fadeShader->setUniformValue("uTarget", colorToVec(params.color));
    m_fadeShader->setUniformValue("uStep",
                                  static_cast<float>(params.fadeLen) / 255.0f);
    m_fadeShader->release();
    transformPass(*m_fadeShader);
}

void MultiEffectVisualizer::runInvert()
{
    transformPass(*m_invertShader);
}

void MultiEffectVisualizer::runBrightness(const BrightnessParams& params)
{
    // Slider -> multiplier (r_bright.cpp:188): negative darkens to 0, positive
    // brightens up to 17x.
    auto factor = [](int p) {
        return p < 0 ? 1.0f + static_cast<float>(p) / 4096.0f
                     : 1.0f + static_cast<float>(p) / 256.0f;
    };
    m_brightShader->bind();
    m_brightShader->setUniformValue(
        "uFactor", QVector3D(factor(params.red), factor(params.green),
                             factor(params.blue)));
    m_brightShader->setUniformValue("uExclude", params.exclude);
    m_brightShader->setUniformValue("uExColor", colorToVec(params.color));
    m_brightShader->setUniformValue("uDistance",
                                    static_cast<float>(params.distance) / 255.0f);
    m_brightShader->release();
    transformPass(*m_brightShader);
}

void MultiEffectVisualizer::runFastBrightness(const FastBrightnessParams& params)
{
    if (params.dir == 2) return;  // identity — no work (r_fastbright dir==2)
    const float scale = params.dir == 0 ? 2.0f : 0.5f;  // x2 (clamped) / x0.5
    m_brightShader->bind();
    m_brightShader->setUniformValue("uFactor", QVector3D(scale, scale, scale));
    m_brightShader->setUniformValue("uExclude", false);
    m_brightShader->release();
    transformPass(*m_brightShader);
}

void MultiEffectVisualizer::runBlur(const BlurParams& params)
{
    // Weights per strength (r_blur.cpp). All kernels sum to 1.
    float center = 0.5f;
    float neighbor = 0.125f;
    if (params.strength == 2) { center = 0.75f; neighbor = 0.0625f; }
    else if (params.strength == 3) { center = 0.0f; neighbor = 0.25f; }

    m_blurShader->bind();
    m_blurShader->setUniformValue(
        "uTexel", QVector2D(1.0f / static_cast<float>(m_surfaceWidth),
                            1.0f / static_cast<float>(m_surfaceHeight)));
    m_blurShader->setUniformValue("uCenter", center);
    m_blurShader->setUniformValue("uNeighbor", neighbor);
    m_blurShader->release();
    transformPass(*m_blurShader);
}

void MultiEffectVisualizer::runMirror(const ChainNode& node,
                                      const MirrorParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Active direction bits this frame (r_mirror.cpp:146 rbeat=(rand()%16)&mode).
    int target = params.mode & 15;
    if (params.onBeatRandom)
    {
        if (m_frameBeat)
        {
            rt.mirrorRBeat = static_cast<int>(nextRandom() % 16u) & params.mode;
        }
        target = rt.mirrorRBeat;
    }

    // Per-direction factors: hard switch, or a 16-step ramp advancing every
    // `slower` frames (BLEND_ADAPT divisors, r_mirror.cpp:249-257).
    const bool step = params.smooth &&
                      (++rt.mirrorFrames % std::max(1, params.slower)) == 0;
    bool any = false;
    for (int i = 0; i < 4; ++i)
    {
        const float goal = ((target >> i) & 1) != 0 ? 1.0f : 0.0f;
        float& fac = rt.mirrorF[i];
        if (!params.smooth)
        {
            fac = goal;
        }
        else if (step)
        {
            const float delta = 1.0f / 16.0f;
            fac = goal > fac ? std::min(goal, fac + delta)
                             : std::max(goal, fac - delta);
        }
        if (fac > 0.0f) any = true;
    }
    if (!any) return;  // nothing to mirror this frame

    m_mirrorShader->bind();
    m_mirrorShader->setUniformValue(
        "uF", QVector4D(rt.mirrorF[0], rt.mirrorF[1], rt.mirrorF[2], rt.mirrorF[3]));
    m_mirrorShader->release();
    transformPass(*m_mirrorShader);
}

void MultiEffectVisualizer::runOnBeatClear(const ChainNode& node,
                                           const OnBeatClearParams& params)
{
    // Beat counter -> clear every Nth beat (r_nfclr.cpp:97).
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!m_frameBeat) return;
    if (++rt.beatCounter < params.everyNBeats) return;
    rt.beatCounter = 0;

    auto* f = QOpenGLContext::currentContext()->functions();
    const QVector3D color = colorToVec(params.color);
    if (params.blend)
    {
        // 50/50 towards color: full-screen quad with constant-alpha 0.5 blend
        // → out = 0.5*color + 0.5*existing (r_nfclr BLEND_AVG).
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
        f->glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);
        m_barsShader->bind();
        m_quadVao->bind();
        m_barsShader->setUniformValue("uCenter", QVector2D(0.0f, 0.0f));
        m_barsShader->setUniformValue("uSize", QVector2D(1.0f, 1.0f));
        m_barsShader->setUniformValue("uColor", color);
        f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_quadVao->release();
        m_barsShader->release();
        f->glDisable(GL_BLEND);
    }
    else
    {
        f->glClearColor(color.x(), color.y(), color.z(), 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
    }
}

void MultiEffectVisualizer::runColorfade(const ChainNode& node,
                                         const ColorfadeParams& params)
{
    // Beat faders replace the base faders for onBeatFrames frames after a beat.
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (m_frameBeat) rt.beatFramesLeft = params.onBeatFrames;
    const bool onBeat = rt.beatFramesLeft > 0;
    if (rt.beatFramesLeft > 0) --rt.beatFramesLeft;

    const int f1 = onBeat ? params.beatFaderR : params.faderR;
    const int f2 = onBeat ? params.beatFaderG : params.faderG;
    const int f3 = onBeat ? params.beatFaderB : params.faderB;

    m_colorfadeShader->bind();
    m_colorfadeShader->setUniformValue(
        "uFaders", QVector3D(static_cast<float>(f1) / 255.0f,
                             static_cast<float>(f2) / 255.0f,
                             static_cast<float>(f3) / 255.0f));
    m_colorfadeShader->release();
    transformPass(*m_colorfadeShader);
}

void MultiEffectVisualizer::runColorModifier(const ChainNode& node,
                                             const ColorModifierParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // (Re)build the LUT module when its sources change (render-thread compile).
    const std::string combined = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + '\n' + params.levelCode;
    if (rt.lut == nullptr || rt.lutCompiled != combined)
    {
        rt.lut = std::make_unique<lumi::modules::ScriptLutModule>(activeContext());
        rt.lut->setInitCode(params.initCode);
        rt.lut->setFrameCode(params.frameCode);
        rt.lut->setBeatCode(params.beatCode);
        rt.lut->setLevelCode(params.levelCode);
        rt.lutCompiled = combined;
    }
    rt.lut->setRecompute(params.recompute);
    rt.lut->setVisData(m_visdata.data(), m_time);  // getspec/getosc for the level code
    {
        float bass, mid, treble;
        computeAudioBands(getSpectrum(), bass, mid, treble);
        rt.lut->setVariable("bass", bass);
        rt.lut->setVariable("mid", mid);
        rt.lut->setVariable("treb", treble);
        rt.lut->setVariable("treble", treble);
        rt.lut->setVariable("vol", m_audioLevel);
        rt.lut->setVariable("beat", m_frameBeat ? 1.0 : 0.0);
    }
    rt.lut->execute(m_frameBeat, m_deltaTime);

    // Upload the three 256-entry tables into a 256x1 RGB texture.
    auto* f = QOpenGLContext::currentContext()->functions();
    unsigned char pixels[256 * 3];
    for (int i = 0; i < 256; ++i)
    {
        auto toByte = [](float v) {
            return static_cast<unsigned char>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
        };
        pixels[i * 3 + 0] = toByte(rt.lut->lut(0, i));
        pixels[i * 3 + 1] = toByte(rt.lut->lut(1, i));
        pixels[i * 3 + 2] = toByte(rt.lut->lut(2, i));
    }
    if (rt.lutTexture == 0)
    {
        f->glGenTextures(1, &rt.lutTexture);
        f->glBindTexture(GL_TEXTURE_2D, rt.lutTexture);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        f->glBindTexture(GL_TEXTURE_2D, rt.lutTexture);
    }
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                    pixels);

    // Apply: sample current through the LUT.
    m_lutShader->bind();
    m_lutShader->setUniformValue("uLut", 1);
    m_lutShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.lutTexture);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_lutShader);
}

void MultiEffectVisualizer::runMovement(const ChainNode& node,
                                        const MovementParams& params)
{
    if (params.code.empty()) return;  // built-in formulas -> passthrough (5.4)
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Movement formulas are STATIC (r_trans builds a per-pixel table once per
    // size/effect change) — so the grid can be fine: it is evaluated only on
    // compile/resize, not per frame (staticField below).
    constexpr int kXres = 96, kYres = 72;
    const std::string combined = "M:" + params.code;
    if (rt.grid == nullptr || rt.gridCompiled != combined)
    {
        rt.grid = std::make_unique<lumi::modules::ScriptGridModule>(activeContext());
        rt.grid->setPointCode(params.code);
        rt.grid->setGridSize(kXres, kYres);
        rt.gridCompiled = combined;
    }
    rt.grid->setRectCoords(params.rectCoords);

    // sourcemapped runtime state (r_trans: bit1 toggles bit0 on every beat).
    if (rt.moveSourceMapped < 0) rt.moveSourceMapped = params.sourceMapped & 3;
    if ((rt.moveSourceMapped & 2) != 0 && m_frameBeat) rt.moveSourceMapped ^= 1;

    GridWarpOptions opt;
    opt.wrap = params.wrap;
    // r_trans `blend` = 50/50 of moved and original; the warp path realizes it
    // via the alpha default 0.5 (no script sets alpha in Movement code).
    opt.blend = params.blend;
    opt.staticField = true;
    opt.subpixel = params.subpixel;

    if ((rt.moveSourceMapped & 1) != 0)
    {
        applyGridScatter(rt, kXres, kYres, opt);
        return;
    }
    applyGridWarp(rt, kXres, kYres, opt);
}

void MultiEffectVisualizer::applyGridScatter(LeafRuntime& rt, int xres, int yres,
                                             const GridWarpOptions& opt)
{
    // r_trans "source mapped" (r_trans.cpp:543-600): every SOURCE pixel is
    // pushed to its warp target and MAX-blended (BLEND_MAX) onto black — or
    // onto a copy of the frame when `blend` is set. GPU approximation: the
    // warp mesh is drawn with swapped roles (vertex position = warp target,
    // texcoord = source position) under GL_MAX blending; stretched triangles
    // fill where the original scatter would leave gaps (sight-calibrate).
    if (rt.grid == nullptr || xres < 2 || yres < 2) return;
    const bool needExecute = rt.grid->field().empty() ||
                             rt.gridFieldW != m_surfaceWidth ||
                             rt.gridFieldH != m_surfaceHeight;
    if (needExecute)
    {
        rt.grid->execute(static_cast<float>(m_surfaceWidth),
                         static_cast<float>(m_surfaceHeight), m_frameBeat,
                         m_deltaTime);
        rt.gridFieldW = m_surfaceWidth;
        rt.gridFieldH = m_surfaceHeight;
    }
    const auto& field = rt.grid->field();
    if (static_cast<int>(field.size()) < xres * yres) return;

    // Inverse mesh: position = clamped warp target, texcoord = source cell.
    m_warpVertices.clear();
    m_warpVertices.reserve(static_cast<size_t>(xres - 1) * (yres - 1) * 6 * 5);
    auto pushVertex = [&](int gx, int gy) {
        const float sx = static_cast<float>(gx) / (xres - 1);
        const float sy = static_cast<float>(gy) / (yres - 1);
        const auto& n = field[static_cast<size_t>(gy) * xres + gx];
        m_warpVertices.push_back(std::clamp(n.u, -1.0f, 1.0f));
        m_warpVertices.push_back(std::clamp(n.v, -1.0f, 1.0f));
        m_warpVertices.push_back(sx);
        m_warpVertices.push_back(sy);
        m_warpVertices.push_back(1.0f);
    };
    for (int gy = 0; gy < yres - 1; ++gy)
    {
        for (int gx = 0; gx < xres - 1; ++gx)
        {
            pushVertex(gx, gy);     pushVertex(gx + 1, gy);   pushVertex(gx, gy + 1);
            pushVertex(gx, gy + 1); pushVertex(gx + 1, gy);   pushVertex(gx + 1, gy + 1);
        }
    }

    auto* f = QOpenGLContext::currentContext()->functions();
    SurfacePair& pair = active();
    pair.current()->release();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);

    if (opt.blend)
    {
        // Base = copy of the frame (r_trans memcpy), scatter maxes on top.
        auto* extra = QOpenGLContext::currentContext()->extraFunctions();
        extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, pair.current()->handle());
        extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pair.partner()->handle());
        extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0,
                                 m_surfaceWidth, m_surfaceHeight,
                                 GL_COLOR_BUFFER_BIT, GL_NEAREST);
        extra->glBindFramebuffer(GL_FRAMEBUFFER, pair.partner()->handle());
    }
    else
    {
        f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
    }

    f->glEnable(GL_BLEND);
    f->glBlendEquation(GL_MAX);
    f->glBlendFunc(GL_ONE, GL_ONE);

    m_warpShader->bind();
    m_warpShader->setUniformValue("uWrap", false);
    m_warpShader->setUniformValue("uBlend", false);
    m_warpShader->setUniformValue("uNomove", false);
    m_warpShader->setUniformValue("uBufSrc", false);
    m_warpVao->bind();
    m_warpVbo->bind();
    m_warpVbo->allocate(m_warpVertices.data(),
                        static_cast<int>(m_warpVertices.size() * sizeof(float)));
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_warpShader->setUniformValue("uTex", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_warpShader->setUniformValue("uSrcTex", 1);
    f->glActiveTexture(GL_TEXTURE0);
    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_warpVertices.size() / 5));
    m_warpVbo->release();
    m_warpVao->release();
    m_warpShader->release();

    f->glBlendEquation(GL_FUNC_ADD);
    f->glDisable(GL_BLEND);

    pair.partner()->release();
    pair.swap();
    bindActive();
}

void MultiEffectVisualizer::runDynamicMovement(const ChainNode& node,
                                               const DynamicMovementParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    const std::string combined = "D:" + std::to_string(params.xres) + "x" +
                                 std::to_string(params.yres) + "\n" + params.initCode +
                                 '\n' + params.frameCode + '\n' + params.beatCode +
                                 '\n' + params.pointCode;
    if (rt.grid == nullptr || rt.gridCompiled != combined)
    {
        rt.grid = std::make_unique<lumi::modules::ScriptGridModule>(activeContext());
        rt.grid->setInitCode(params.initCode);
        rt.grid->setFrameCode(params.frameCode);
        rt.grid->setBeatCode(params.beatCode);
        rt.grid->setPointCode(params.pointCode);
        rt.grid->setGridSize(params.xres, params.yres);
        rt.gridCompiled = combined;
    }
    rt.grid->setRectCoords(params.rectCoords);

    GridWarpOptions opt;
    opt.wrap = params.wrap;
    opt.blend = params.blend;
    opt.nomove = params.nomove;
    opt.subpixel = params.subpixel;
    if (params.buffern > 0)
    {
        // r_dmove.cpp:289-290: source is a global buffer; when it does not
        // exist (nothing saved yet) the effect is a plain passthrough.
        QOpenGLFramebufferObject* pool = activePool().get(
            params.buffern - 1, m_surfaceWidth, m_surfaceHeight, false);
        if (pool == nullptr) return;
        opt.srcTexture = pool->texture();
    }
    applyGridWarp(rt, params.xres, params.yres, opt);
}

void MultiEffectVisualizer::applyGridWarp(LeafRuntime& rt, int xres, int yres,
                                          const GridWarpOptions& opt)
{
    if (rt.grid == nullptr || xres < 2 || yres < 2) return;

    // Static fields (Movement) are evaluated once per compile/resize; dynamic
    // ones (Dynamic Movement) run their frame/beat/point scripts every frame.
    const bool needExecute = !opt.staticField || rt.grid->field().empty() ||
                             rt.gridFieldW != m_surfaceWidth ||
                             rt.gridFieldH != m_surfaceHeight;
    if (needExecute)
    {
        rt.grid->setVisData(m_visdata.data(), m_time);  // getspec/getosc backing
        {
            float bass, mid, treble;
            computeAudioBands(getSpectrum(), bass, mid, treble);
            rt.grid->setVariable("bass", bass);
            rt.grid->setVariable("mid", mid);
            rt.grid->setVariable("treb", treble);
            rt.grid->setVariable("treble", treble);
            rt.grid->setVariable("vol", m_audioLevel);
            rt.grid->setVariable("beat", m_frameBeat ? 1.0 : 0.0);
        }
        rt.grid->execute(static_cast<float>(m_surfaceWidth),
                         static_cast<float>(m_surfaceHeight), m_frameBeat,
                         m_deltaTime);
        rt.gridFieldW = m_surfaceWidth;
        rt.gridFieldH = m_surfaceHeight;
    }
    const auto& field = rt.grid->field();
    if (static_cast<int>(field.size()) < xres * yres) return;

    // Build the triangulated grid mesh: vertex at grid NDC, texcoord = (u,v),
    // plus the script's per-cell alpha (r_dmove blend weight).
    m_warpVertices.clear();
    m_warpVertices.reserve(static_cast<size_t>(xres - 1) * (yres - 1) * 6 * 5);
    auto pushVertex = [&](int gx, int gy) {  // NB: not "emit" — Qt reserves it
        const float px = -1.0f + 2.0f * static_cast<float>(gx) / (xres - 1);
        const float py = -1.0f + 2.0f * static_cast<float>(gy) / (yres - 1);
        const auto& n = field[static_cast<size_t>(gy) * xres + gx];
        m_warpVertices.push_back(px);
        m_warpVertices.push_back(py);
        m_warpVertices.push_back(n.u * 0.5f + 0.5f);
        m_warpVertices.push_back(n.v * 0.5f + 0.5f);
        m_warpVertices.push_back(n.alpha);
    };
    for (int gy = 0; gy < yres - 1; ++gy)
    {
        for (int gx = 0; gx < xres - 1; ++gx)
        {
            pushVertex(gx, gy);     pushVertex(gx + 1, gy);   pushVertex(gx, gy + 1);
            pushVertex(gx, gy + 1); pushVertex(gx + 1, gy);   pushVertex(gx + 1, gy + 1);
        }
    }

    auto* f = QOpenGLContext::currentContext()->functions();
    SurfacePair& pair = active();

    pair.current()->release();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);

    // Warp source: the current frame or a global buffer (r_dmove buffern).
    const unsigned int srcTex =
        opt.srcTexture != 0 ? opt.srcTexture : pair.current()->texture();

    m_warpShader->bind();
    m_warpShader->setUniformValue("uWrap", opt.wrap);
    m_warpShader->setUniformValue("uBlend", opt.blend);
    m_warpShader->setUniformValue("uNomove", opt.nomove);
    m_warpShader->setUniformValue("uBufSrc", opt.srcTexture != 0);
    m_warpVao->bind();
    m_warpVbo->bind();
    m_warpVbo->allocate(m_warpVertices.data(),
                        static_cast<int>(m_warpVertices.size() * sizeof(float)));
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_warpShader->setUniformValue("uTex", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, srcTex);
    // subpixel off = nearest sampling of the warp source (r_dmove/r_trans
    // toggle); restored to linear right after the draw.
    const GLint filter = opt.subpixel ? GL_LINEAR : GL_NEAREST;
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    m_warpShader->setUniformValue("uSrcTex", 1);
    f->glActiveTexture(GL_TEXTURE0);
    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_warpVertices.size() / 5));
    if (!opt.subpixel)
    {
        f->glActiveTexture(GL_TEXTURE1);
        f->glBindTexture(GL_TEXTURE_2D, srcTex);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        f->glActiveTexture(GL_TEXTURE0);
    }
    m_warpVbo->release();
    m_warpVao->release();
    m_warpShader->release();

    pair.partner()->release();
    pair.swap();
    bindActive();
}

void MultiEffectVisualizer::runBlitterFeedback(const BlitterFeedbackParams& params)
{
    const float zoom = (params.onBeat && m_frameBeat) ? params.beatZoom : params.zoom;
    feedbackPass(zoom, 0.0f, params.blend);
}

void MultiEffectVisualizer::runRotoBlitter(const ChainNode& node,
                                           const RotoBlitterParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    rt.rotoAngle += params.rotationSpeed * 0.01745329f;  // deg -> rad per frame
    feedbackPass(params.zoom, rt.rotoAngle, params.blend);
}

void MultiEffectVisualizer::feedbackPass(float zoom, float angleRad, bool blend)
{
    if (zoom <= 0.0001f) zoom = 1.0f;
    const float aspect = m_surfaceHeight > 0
                             ? static_cast<float>(m_surfaceWidth) / m_surfaceHeight
                             : 1.0f;
    m_feedbackShader->bind();
    m_feedbackShader->setUniformValue("uZoomInv", 1.0f / zoom);
    m_feedbackShader->setUniformValue("uCos", std::cos(angleRad));
    m_feedbackShader->setUniformValue("uSin", std::sin(angleRad));
    m_feedbackShader->setUniformValue("uAspect", aspect);
    m_feedbackShader->setUniformValue("uBlend", blend);
    m_feedbackShader->release();
    transformPass(*m_feedbackShader);
}

void MultiEffectVisualizer::runBufferSave(const ChainNode& node,
                                          const BufferSaveParams& params)
{
    // Direction per frame (r_stack.cpp:125-126): dir 0/1 are fixed, dir 2/3
    // alternate save/restore every render via the per-node toggle.
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    int tDir = params.dir;
    if (params.dir >= 2)
    {
        tDir = (params.dir & 1) ^ (rt.bufDirCh ? 1 : 0);
        rt.bufDirCh = !rt.bufDirCh;
    }

    if (tDir == 0)
    {
        // Write the current working buffer into global buffer `slot`. The blend
        // applies in BOTH directions in AVS (fbin/fbout swap, r_stack.cpp:127).
        QOpenGLFramebufferObject* pool =
            activePool().get(params.slot, m_surfaceWidth, m_surfaceHeight, true);
        if (pool == nullptr) return;
        const bool hasBlit = QOpenGLFramebufferObject::hasOpenGLFramebufferBlit();
        if (params.blend == BlendMode::Replace || !hasBlit ||
            !ensureSurfacePair(m_bufferScratch, m_surfaceWidth, m_surfaceHeight,
                               nullptr))
        {
            if (hasBlit)
            {
                auto* extra = QOpenGLContext::currentContext()->extraFunctions();
                extra->glBindFramebuffer(GL_READ_FRAMEBUFFER,
                                         active().current()->handle());
                extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pool->handle());
                extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0,
                                         m_surfaceWidth, m_surfaceHeight,
                                         GL_COLOR_BUFFER_BIT, GL_NEAREST);
            }
        }
        else
        {
            // pool -> scratch, blend the frame onto it, scratch -> pool.
            auto* extra = QOpenGLContext::currentContext()->extraFunctions();
            extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, pool->handle());
            extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
                                     m_bufferScratch.current()->handle());
            extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0,
                                     m_surfaceWidth, m_surfaceHeight,
                                     GL_COLOR_BUFFER_BIT, GL_NEAREST);
            blendPass(m_bufferScratch, active().current()->texture(), params.blend,
                      params.adjustAlpha);
            extra->glBindFramebuffer(GL_READ_FRAMEBUFFER,
                                     m_bufferScratch.current()->handle());
            extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pool->handle());
            extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0,
                                     m_surfaceWidth, m_surfaceHeight,
                                     GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
        bindActive();  // restore the draw target for the following effects
    }
    else
    {
        // Blend a previously saved buffer back onto the working surface.
        QOpenGLFramebufferObject* pool =
            activePool().get(params.slot, m_surfaceWidth, m_surfaceHeight, false);
        if (pool == nullptr) return;  // nothing saved yet
        blendPass(active(), pool->texture(), params.blend, params.adjustAlpha);
        bindActive();
    }
}

void MultiEffectVisualizer::runCustomBpm(const ChainNode& node,
                                         const CustomBpmParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    bool beat = m_frameBeat;

    if (params.arbitrary)
    {
        const std::int64_t now = lumi::modules::BeatEstimator::steadyNowMs();
        if (rt.customLastMs == 0) rt.customLastMs = now;
        if (now - rt.customLastMs >= params.arbitraryMs)
        {
            beat = true;
            rt.customLastMs = now;
        }
        else
        {
            beat = false;
        }
    }

    if (params.skip && beat)
    {
        // Pass only every skipCount-th beat.
        if (++rt.customSkipCount < params.skipCount) beat = false;
        else rt.customSkipCount = 0;
    }

    if (params.invert) beat = !beat;

    m_frameBeat = beat;  // mutates the beat for the following effects (§5.1)
}

void MultiEffectVisualizer::runSetRenderMode(const SetRenderModeParams& params)
{
    // No visual output — sets the host render mode for the following render
    // effects (AVS r_linemode). A disabled node leaves the current blend
    // UNCHANGED (r_linemode.cpp:96-104 writes only when bit31 is set); a width
    // of 0 leaves each effect's own line width.
    m_renderMode.set = true;
    if (params.lineWidth > 0) m_renderMode.lineWidth = params.lineWidth;
    if (params.enabled) m_renderMode.lineBlend = std::clamp(params.lineBlend, 0, 2);
    m_renderMode.alpha = std::clamp(params.adjustAlpha, 0, 255);
}

void MultiEffectVisualizer::runSuperScope(const ChainNode& node,
                                          const SuperScopeParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // A preceding Set Render Mode node overrides line width + blend (AVS state).
    const int effLineBlend = m_renderMode.set ? m_renderMode.lineBlend : params.lineBlend;
    const float effLineWidth = (m_renderMode.set && m_renderMode.lineWidth > 0)
                                   ? static_cast<float>(m_renderMode.lineWidth)
                                   : params.lineWidth;

    const std::string combined = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + '\n' + params.pointCode;
    if (rt.scope == nullptr || rt.scopeCompiled != combined)
    {
        rt.scope = std::make_unique<lumi::modules::SuperscopeModule>();
        rt.scope->setLuaMode(true);  // EEL quartet -> Lua (import path)
        rt.scope->setInitCode(params.initCode);
        rt.scope->setBeatCode(params.beatCode);
        rt.scope->setFrameCode(params.frameCode);
        rt.scope->setPointCode(params.pointCode);
        rt.scopeCompiled = combined;
        rt.scopeGradientLoaded.clear();  // fresh module defaults to Neon -> reload
    }
    rt.scope->setPointCount(params.pointCount);
    rt.scope->setRenderMode(
        static_cast<lumi::modules::SuperscopeRenderMode>(params.renderMode));
    rt.scope->setLineWidth(effLineWidth);
    rt.scope->setDotSize(params.dotSize);
    rt.scope->setAudioChannel(
        static_cast<lumi::modules::SuperscopeAudioChannel>(params.audioChannel));
    // Keep the module's gradient in sync (it bakes it into points when the point
    // code sets no color); reload only on change.
    if (rt.scopeGradientLoaded != params.gradientPreset)
    {
        rt.scope->colorGradient().loadPreset(params.gradientPreset);
        rt.scopeGradientLoaded = params.gradientPreset;
    }
    // Base color (gradient x cycled table, per mode). The module pre-seeds
    // red/green/blue with it before the point script runs (AVS r_sscope).
    rt.scope->setColorTable(params.colors);
    rt.scope->setColorBlend(params.colorBlend);
    rt.scope->setColorCycleFrames(params.colorCycleFrames);

    // Audio: mono waveform/spectrum fed to both channels (host has no L/R split
    // yet); a zero buffer keeps the base shape visible without a track.
    static const std::vector<float> kSilence(576, 0.0f);
    const std::vector<float> wave = getWaveform();
    const std::vector<float> spec = getSpectrum();
    const float* w = wave.empty() ? kSilence.data() : wave.data();
    const float* s = spec.empty() ? kSilence.data() : spec.data();
    const int sampleCount = wave.empty() ? static_cast<int>(kSilence.size())
                                         : static_cast<int>(wave.size());

    // Audio contract for the point/frame code: getspec/getosc backing data (+
    // gettime clock) plus the bass/mid/treb short vars (E1, shared with all modules).
    rt.scope->setVisData(m_visdata.data(), m_time);
    {
        float bass, mid, treble;
        computeAudioBands(getSpectrum(), bass, mid, treble);
        rt.scope->setVariable("bass", bass);
        rt.scope->setVariable("mid", mid);
        rt.scope->setVariable("treb", treble);
        rt.scope->setVariable("treble", treble);
        rt.scope->setVariable("vol", m_audioLevel);
    }

    const std::vector<lumi::modules::SuperscopePoint> points = rt.scope->execute(
        w, w, s, s, sampleCount, m_surfaceWidth, m_surfaceHeight, m_frameBeat,
        m_deltaTime);

    // Blend onto the working buffer. AVS default is additive; a preceding Set
    // Render Mode can switch it to replace or 50/50 (params.lineBlend).
    if (!m_scopeRenderer.ready()) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    switch (effLineBlend)
    {
        case 0:  f->glBlendFunc(GL_ONE, GL_ZERO); break;                       // replace
        case 2:  f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;  // 50/50
        default: f->glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;                  // additive
    }
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = static_cast<lumi::modules::SuperscopeRenderMode>(params.renderMode);
    rp.lineWidth = effLineWidth;
    // Scripted drawmode/linesize win over UI params (r_sscope EEL vars).
    if (rt.scope->scriptDrawModeActive())
    {
        rp.mode = rt.scope->scriptWantsLines()
                      ? lumi::modules::SuperscopeRenderMode::Lines
                      : lumi::modules::SuperscopeRenderMode::Dots;
    }
    if (rt.scope->scriptLineSizeActive()) rp.lineWidth = rt.scope->scriptLineSize();
    rp.dotSize = params.dotSize;
    rp.glowEnabled = false;

    if (rt.scope->pointDrawModeActive())
    {
        // Point code switches drawmode mid-scope (r_sscope): split into runs
        // of equal mode. A lines-run includes the previous point so the
        // connecting segment is kept.
        size_t start = 0;
        while (start < points.size())
        {
            size_t end = start + 1;
            while (end < points.size() &&
                   points[end].drawLines == points[start].drawLines)
            {
                ++end;
            }
            std::vector<lumi::modules::SuperscopePoint> run;
            if (points[start].drawLines && start > 0)
            {
                run.push_back(points[start - 1]);
            }
            run.insert(run.end(), points.begin() + static_cast<long long>(start),
                       points.begin() + static_cast<long long>(end));
            rp.mode = points[start].drawLines
                          ? lumi::modules::SuperscopeRenderMode::Lines
                          : lumi::modules::SuperscopeRenderMode::Dots;
            m_scopeRenderer.draw(run, rp);
            start = end;
        }
    }
    else
    {
        m_scopeRenderer.draw(points, rp);
    }
    f->glDisable(GL_BLEND);
}

uint32_t MultiEffectVisualizer::nextRandom()
{
    // Numerical Recipes LCG — host-local, deterministic (no global rand()).
    m_rng = m_rng * 1664525u + 1013904223u;
    return m_rng;
}

// =============================================================================
// Milkdrop-Meganode (Import Roadmap 6, N1 — Entscheid E1)
// =============================================================================

void MultiEffectVisualizer::feedMilkAudio(MilkdropVisualizer& milk)
{
    // Kanal-Kopien des Hosts zu interleaved Stereo zusammensetzen (Vertrag von
    // VisualizerBase::updateAudioStereo); Mono-Fallback liefern die Getter selbst
    const std::vector<float> wl = getWaveformChannel(0);
    const std::vector<float> wr = getWaveformChannel(1);
    const std::vector<float> sl = getSpectrumChannel(0);
    const std::vector<float> sr = getSpectrumChannel(1);
    const int frames = static_cast<int>(std::min(wl.size(), wr.size()));
    const int bins = static_cast<int>(std::min(sl.size(), sr.size()));
    if (frames == 0 && bins == 0) return;

    m_milkWaveScratch.assign(static_cast<std::size_t>(frames) * 2, 0.0f);
    for (int i = 0; i < frames; ++i)
    {
        m_milkWaveScratch[static_cast<std::size_t>(i) * 2 + 0] =
            wl[static_cast<std::size_t>(i)];
        m_milkWaveScratch[static_cast<std::size_t>(i) * 2 + 1] =
            wr[static_cast<std::size_t>(i)];
    }
    m_milkSpecScratch.assign(static_cast<std::size_t>(bins) * 2, 0.0f);
    for (int b = 0; b < bins; ++b)
    {
        m_milkSpecScratch[static_cast<std::size_t>(b) * 2 + 0] =
            sl[static_cast<std::size_t>(b)];
        m_milkSpecScratch[static_cast<std::size_t>(b) * 2 + 1] =
            sr[static_cast<std::size_t>(b)];
    }
    milk.updateAudioStereo(m_milkSpecScratch.data(), bins, m_milkWaveScratch.data(),
                           frames, 2);
}

void MultiEffectVisualizer::runMilkdropNode(const ChainNode& node,
                                            const MilkdropNodeParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (rt.milk == nullptr)
    {
        rt.milk = std::make_unique<MilkdropVisualizer>();
        rt.milk->initialize();
        rt.milkRevision = 0;
    }
    if (rt.milkRevision != params.revision)
    {
        rt.milkRevision = params.revision;
        // Node-Params sind SSOT: Kopie uebernehmen — kompiliert Skripte und
        // transpiliert Shader (GL-frei; GL-Programme baut der naechste Kern-
        // Frame lazy ueber die Custom-Rev)
        rt.milk->applyPresetState(params.preset,
                                  QString::fromStdString(params.presetDir), nullptr);
        rt.milk->setParam("render.meshX", lumi::modules::ParamValue{params.meshX});
        rt.milk->setParam("render.meshY", lumi::modules::ParamValue{params.meshY});
        rt.milk->setParam("render.debugGrid",
                          lumi::modules::ParamValue{params.debugGrid});
    }

    feedMilkAudio(*rt.milk);
    // Chain-Buffer-Groesse (physische Pixel) — width()/height() des Kerns
    // steuern dessen Composite-Viewport und die Feedback-Buffer-Groesse
    rt.milk->resize(QSize(m_surfaceWidth, m_surfaceHeight));

    // Der Kern erfasst beim Frame-Start das gebundene Draw-FBO als Composite-
    // Ziel — das muss der aktive Chain-Buffer sein. Danach Host-Zustand
    // wiederherstellen (der Kern verbog FBO/Viewport fuer seine Paesse).
    bindActive();
    rt.milk->render(m_deltaTime);
    bindActive();
}

void MultiEffectVisualizer::runDebugBars(const DebugBarsParams& params)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    const float angle = m_time * params.orbitSpeed;
    const float size = 0.04f + m_audioLevel * 0.35f;

    m_barsShader->bind();
    m_quadVao->bind();
    m_barsShader->setUniformValue(
        "uCenter", QVector2D(std::cos(angle) * 0.6f, std::sin(angle) * 0.6f));
    m_barsShader->setUniformValue("uSize", QVector2D(size, size));
    m_barsShader->setUniformValue("uColor", colorToVec(params.color));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_barsShader->release();
}

void MultiEffectVisualizer::runMosaic(const ChainNode& node, const MosaicParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (rt.mosaicQuality <= 0.0f) rt.mosaicQuality = static_cast<float>(params.quality);

    // Pick this frame's block count, then step the ease-back (r_mosaic.cpp).
    if (params.onBeat && m_frameBeat)
    {
        rt.mosaicQuality = static_cast<float>(params.quality2);
        rt.mosaicFramesLeft = std::max(1, params.durationFrames);
    }
    else if (rt.mosaicFramesLeft == 0)
    {
        rt.mosaicQuality = static_cast<float>(params.quality);
    }
    const float thisQuality = rt.mosaicQuality;
    if (rt.mosaicFramesLeft > 0)
    {
        if (--rt.mosaicFramesLeft > 0)
        {
            const float step = std::abs(static_cast<float>(params.quality - params.quality2)) /
                               static_cast<float>(std::max(1, params.durationFrames));
            rt.mosaicQuality += params.quality2 > params.quality ? -step : step;
        }
        else
        {
            rt.mosaicQuality = static_cast<float>(params.quality);
        }
    }

    const int cells = std::clamp(static_cast<int>(thisQuality), 1, 100);
    if (cells >= 100) return;  // no pixelation (AVS gate: thisQuality < 100)

    m_mosaicShader->bind();
    m_mosaicShader->setUniformValue("uCells", static_cast<float>(cells));
    m_mosaicShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_mosaicShader->release();
    transformPass(*m_mosaicShader);
}

void MultiEffectVisualizer::runGrain(const GrainParams& params)
{
    const QVector2D res(static_cast<float>(m_surfaceWidth),
                        static_cast<float>(m_surfaceHeight));
    m_grainShader->bind();
    m_grainShader->setUniformValue("uRes", res);
    m_grainShader->setUniformValue("uAmount",
                                   std::clamp(params.amount, 0, 100) / 100.0f);
    // Static = frozen pattern; else advance the seed so the grain shimmers.
    m_grainShader->setUniformValue("uSeed", params.staticGrain ? 0.0f : m_time * 60.0f);
    m_grainShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_grainShader->release();
    transformPass(*m_grainShader);
}

void MultiEffectVisualizer::runScatter(const ScatterParams&)
{
    const QVector2D res(static_cast<float>(m_surfaceWidth),
                        static_cast<float>(m_surfaceHeight));
    m_scatterShader->bind();
    m_scatterShader->setUniformValue("uRes", res);
    m_scatterShader->setUniformValue("uSeed", m_time * 60.0f);
    m_scatterShader->setUniformValue("uRange", 4.0f);  // r_scat ~4px window
    m_scatterShader->release();
    transformPass(*m_scatterShader);
}

void MultiEffectVisualizer::runInterferences(const ChainNode& node,
                                             const InterferencesParams& params)
{
    constexpr float kPi = 3.14159265358979323846f;
    const int points = std::clamp(params.points, 1, 8);
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!rt.interfSeeded)
    {
        rt.interfRotation = static_cast<float>(params.rotation);
        rt.interfStatus = kPi;
        rt.interfSeeded = true;
    }

    // Beat morph between the two parameter sets via sin(status) (r_interf).
    if (params.onBeat && m_frameBeat && rt.interfStatus >= kPi) rt.interfStatus = 0.0f;
    const float s = std::sin(rt.interfStatus);
    const float rotInc =
        params.rotationInc + (params.rotationInc2 - params.rotationInc) * s;
    const float alpha = params.alpha + (params.alpha2 - params.alpha) * s;
    const float dist = params.distance + (params.distance2 - params.distance) * s;

    // Copy offsets, evenly spaced around the accumulating rotation `a`.
    float a = rt.interfRotation / 255.0f * 2.0f * kPi;
    const float angle = 2.0f * kPi / static_cast<float>(points);
    QVector2D offsets[8];
    for (int i = 0; i < points; ++i)
    {
        offsets[i] = QVector2D(std::cos(a) * dist / static_cast<float>(m_surfaceWidth),
                               std::sin(a) * dist / static_cast<float>(m_surfaceHeight));
        a += angle;
    }

    m_interfShader->bind();
    m_interfShader->setUniformValue("uPoints", points);
    m_interfShader->setUniformValueArray("uOffsets", offsets, 8);
    m_interfShader->setUniformValue("uAlpha", std::clamp(alpha, 0.0f, 255.0f) / 255.0f);
    m_interfShader->setUniformValue("uRgb", params.rgb ? 1 : 0);
    m_interfShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_interfShader->release();
    transformPass(*m_interfShader);

    // Advance rotation + morph phase for the next frame.
    rt.interfRotation += rotInc;
    if (rt.interfRotation > 255.0f) rt.interfRotation -= 255.0f;
    if (rt.interfRotation < -255.0f) rt.interfRotation += 255.0f;
    rt.interfStatus = std::min(rt.interfStatus + params.speed, kPi);
}

void MultiEffectVisualizer::runWater(const ChainNode& node, const WaterParams&)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();

    // Persistent previous-frame buffer (r_water lastframe), (re)created on resize.
    if (rt.waterLast == nullptr || rt.waterW != m_surfaceWidth ||
        rt.waterH != m_surfaceHeight)
    {
        rt.waterLast =
            std::make_unique<QOpenGLFramebufferObject>(m_surfaceWidth, m_surfaceHeight);
        rt.waterW = m_surfaceWidth;
        rt.waterH = m_surfaceHeight;
        rt.waterLast->bind();
        f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
        rt.waterLast->release();
    }
    if (!rt.waterLast->isValid()) return;

    SurfacePair& pair = active();
    QOpenGLFramebufferObject* curFbo = pair.current();
    const unsigned int curTex = curFbo->texture();

    // Ripple: neighbour average of the current frame minus the previous frame.
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_waterShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, curTex);
    m_waterShader->setUniformValue("uCur", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.waterLast->texture());
    m_waterShader->setUniformValue("uLast", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_waterShader->setUniformValue(
        "uTexel", QVector2D(1.0f / static_cast<float>(m_surfaceWidth),
                            1.0f / static_cast<float>(m_surfaceHeight)));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_waterShader->release();
    pair.partner()->release();
    pair.swap();

    // Save THIS frame's input as the previous frame for the next pass.
    if (QOpenGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        auto* extra = QOpenGLContext::currentContext()->extraFunctions();
        extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, curFbo->handle());
        extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt.waterLast->handle());
        extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0,
                                 m_surfaceWidth, m_surfaceHeight, GL_COLOR_BUFFER_BIT,
                                 GL_NEAREST);
    }
    bindActive();
}

void MultiEffectVisualizer::runBump(const ChainNode& node, const BumpParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Light-position script (init/frame/beat -> x, y). Errors disable a slot.
    const std::string combined = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode;
    if (rt.bumpHost == nullptr || rt.bumpCompiled != combined)
    {
        rt.bumpHost = std::make_unique<ScriptSlotHost>("bump", activeContext(),
                                                       ScriptSlotHost::Dialect::Avs);
        rt.bumpHost->setSource(Slot::Init, params.initCode);
        rt.bumpHost->setSource(Slot::Frame, params.frameCode);
        rt.bumpHost->setSource(Slot::Beat, params.beatCode);
        rt.bumpHost->compileAll();
        rt.bumpCompiled = combined;
        auto& engine = rt.bumpHost->engine();
        engine.setNumber("x", 0.5);
        engine.setNumber("y", 0.5);
        engine.setNumber("bi", 1.0);
        rt.bumpHost->run(Slot::Init);
    }
    {
        auto& engine = rt.bumpHost->engine();
        engine.setNumber("isbeat", m_frameBeat ? 1.0 : 0.0);
        engine.setNumber("islbeat", m_frameBeat ? 1.0 : 0.0);
        feedAudio(rt.bumpHost->engine());
        if (rt.bumpHost->has(Slot::Frame)) rt.bumpHost->run(Slot::Frame);
        if (m_frameBeat && rt.bumpHost->has(Slot::Beat)) rt.bumpHost->run(Slot::Beat);
        double lx = engine.number("x");
        double ly = engine.number("y");
        if (params.oldStyle) { lx /= 100.0; ly /= 100.0; }
        rt.bumpX = static_cast<float>(lx);
        rt.bumpY = static_cast<float>(ly);
    }

    // Depth ease on beat (r_bump thisDepth/nF, same shape as Mosaic).
    if (rt.bumpDepth <= 0.0f) rt.bumpDepth = static_cast<float>(params.depth);
    if (params.onBeat && m_frameBeat)
    {
        rt.bumpDepth = static_cast<float>(params.depth2);
        rt.bumpFramesLeft = std::max(1, params.durationFrames);
    }
    else if (rt.bumpFramesLeft == 0)
    {
        rt.bumpDepth = static_cast<float>(params.depth);
    }
    const float thisDepth = rt.bumpDepth;
    if (rt.bumpFramesLeft > 0)
    {
        if (--rt.bumpFramesLeft > 0)
        {
            const float step = std::abs(static_cast<float>(params.depth - params.depth2)) /
                               static_cast<float>(std::max(1, params.durationFrames));
            rt.bumpDepth += params.depth2 > params.depth ? -step : step;
        }
        else
        {
            rt.bumpDepth = static_cast<float>(params.depth);
        }
    }

    m_bumpShader->bind();
    m_bumpShader->setUniformValue("uRes",
                                  QVector2D(static_cast<float>(m_surfaceWidth),
                                            static_cast<float>(m_surfaceHeight)));
    m_bumpShader->setUniformValue("uLight", QVector2D(rt.bumpX, rt.bumpY));
    m_bumpShader->setUniformValue("uDepth", thisDepth * 256.0f / 100.0f);
    m_bumpShader->setUniformValue("uInvert", params.invert ? 1 : 0);
    m_bumpShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_bumpShader->release();
    transformPass(*m_bumpShader);
}

void MultiEffectVisualizer::runDynamicShift(const ChainNode& node,
                                            const DynamicShiftParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Global-offset script (init/frame/beat -> x, y in pixels). Errors disable a slot.
    const std::string combined = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode;
    if (rt.shiftHost == nullptr || rt.shiftCompiled != combined)
    {
        rt.shiftHost = std::make_unique<ScriptSlotHost>("dshift", activeContext(),
                                                        ScriptSlotHost::Dialect::Avs);
        rt.shiftHost->setSource(Slot::Init, params.initCode);
        rt.shiftHost->setSource(Slot::Frame, params.frameCode);
        rt.shiftHost->setSource(Slot::Beat, params.beatCode);
        rt.shiftHost->compileAll();
        rt.shiftCompiled = combined;
        auto& engine = rt.shiftHost->engine();
        engine.setNumber("x", 0.0);
        engine.setNumber("y", 0.0);
        engine.setNumber("w", static_cast<double>(m_surfaceWidth));
        engine.setNumber("h", static_cast<double>(m_surfaceHeight));
        engine.setNumber("alpha", 0.5);
        rt.shiftHost->run(Slot::Init);
    }

    double ox = 0.0;
    double oy = 0.0;
    double alpha = 0.5;
    {
        auto& engine = rt.shiftHost->engine();
        engine.setNumber("w", static_cast<double>(m_surfaceWidth));
        engine.setNumber("h", static_cast<double>(m_surfaceHeight));
        engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
        feedAudio(rt.shiftHost->engine());
        if (rt.shiftHost->has(Slot::Frame)) rt.shiftHost->run(Slot::Frame);
        if (m_frameBeat && rt.shiftHost->has(Slot::Beat)) rt.shiftHost->run(Slot::Beat);
        ox = engine.number("x");
        oy = engine.number("y");
        alpha = engine.number("alpha");
    }

    const float offX = m_surfaceWidth > 0
                           ? static_cast<float>(ox) / static_cast<float>(m_surfaceWidth)
                           : 0.0f;
    const float offY = m_surfaceHeight > 0
                           ? static_cast<float>(oy) / static_cast<float>(m_surfaceHeight)
                           : 0.0f;

    m_shiftShader->bind();
    m_shiftShader->setUniformValue("uOffset", QVector2D(offX, offY));
    m_shiftShader->setUniformValue("uBlend", params.blend ? 1 : 0);
    m_shiftShader->setUniformValue(
        "uAlpha", static_cast<float>(std::clamp(alpha, 0.0, 1.0)));
    m_shiftShader->release();
    transformPass(*m_shiftShader);
}

namespace {
/// AVS scope colour-table cycling (r_simple/oscstar/oscring/rotstar): advances
/// colorPos and returns the interpolated 0x00RRGGBB colour.
uint32_t cycleScopeColor(const std::vector<uint32_t>& colors, int& colorPos)
{
    if (colors.empty()) return 0xFFFFFFu;
    const int n = static_cast<int>(colors.size());
    if (++colorPos >= n * 64) colorPos = 0;
    const int p = colorPos / 64;
    const int r = colorPos & 63;
    const uint32_t c1 = colors[static_cast<std::size_t>(p)];
    const uint32_t c2 = (p + 1 < n) ? colors[static_cast<std::size_t>(p + 1)] : colors[0];
    auto ch = [&](int sh) {
        return (static_cast<int>((c1 >> sh) & 0xFF) * (63 - r) +
                static_cast<int>((c2 >> sh) & 0xFF) * r) / 64;
    };
    return static_cast<uint32_t>((ch(16) << 16) | (ch(8) << 8) | ch(0));
}

/// Build a 256-entry RGB LUT (row-major RGB bytes) from Color Map gradient stops.
void buildColorMapLut(const ColorMapParams& params, std::array<unsigned char, 768>& px)
{
    auto put = [&](int i, float r, float g, float b) {
        px[static_cast<std::size_t>(i) * 3 + 0] =
            static_cast<unsigned char>(std::clamp(r, 0.0f, 255.0f));
        px[static_cast<std::size_t>(i) * 3 + 1] =
            static_cast<unsigned char>(std::clamp(g, 0.0f, 255.0f));
        px[static_cast<std::size_t>(i) * 3 + 2] =
            static_cast<unsigned char>(std::clamp(b, 0.0f, 255.0f));
    };
    std::vector<std::pair<int, uint32_t>> stops;
    for (std::size_t i = 0; i < params.stopPos.size() && i < params.stopColor.size(); ++i)
        stops.emplace_back(std::clamp(params.stopPos[i], 0, 255), params.stopColor[i]);
    std::sort(stops.begin(), stops.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    if (stops.empty())  // no gradient imported: identity greyscale ramp
    {
        for (int i = 0; i < 256; ++i)
            put(i, static_cast<float>(i), static_cast<float>(i), static_cast<float>(i));
        return;
    }
    auto rgb = [](uint32_t c, int shift) { return static_cast<float>((c >> shift) & 0xFF); };
    for (int i = 0; i < 256; ++i)
    {
        if (i <= stops.front().first)
        {
            const uint32_t c = stops.front().second;
            put(i, rgb(c, 16), rgb(c, 8), rgb(c, 0));
            continue;
        }
        if (i >= stops.back().first)
        {
            const uint32_t c = stops.back().second;
            put(i, rgb(c, 16), rgb(c, 8), rgb(c, 0));
            continue;
        }
        std::size_t s = 0;
        while (s + 1 < stops.size() && stops[s + 1].first <= i) ++s;
        const int p0 = stops[s].first;
        const int p1 = stops[s + 1].first;
        const uint32_t c0 = stops[s].second;
        const uint32_t c1 = stops[s + 1].second;
        const float t = p1 > p0 ? static_cast<float>(i - p0) / static_cast<float>(p1 - p0)
                                : 0.0f;
        put(i, rgb(c0, 16) + (rgb(c1, 16) - rgb(c0, 16)) * t,
            rgb(c0, 8) + (rgb(c1, 8) - rgb(c0, 8)) * t,
            rgb(c0, 0) + (rgb(c1, 0) - rgb(c0, 0)) * t);
    }
}
}  // namespace

void MultiEffectVisualizer::runColorMap(const ChainNode& node,
                                        const ColorMapParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();

    // Rebuild the LUT texture only when the stops change.
    std::string snap;
    for (std::size_t i = 0; i < params.stopPos.size() && i < params.stopColor.size(); ++i)
    {
        snap += std::to_string(params.stopPos[i]);
        snap += ':';
        snap += std::to_string(params.stopColor[i]);
        snap += ';';
    }
    if (rt.cmTexture == 0 || rt.cmSnapshot != snap)
    {
        std::array<unsigned char, 768> pixels{};
        buildColorMapLut(params, pixels);
        if (rt.cmTexture == 0)
        {
            f->glGenTextures(1, &rt.cmTexture);
            f->glBindTexture(GL_TEXTURE_2D, rt.cmTexture);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
            f->glBindTexture(GL_TEXTURE_2D, rt.cmTexture);
        }
        f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                        pixels.data());
        rt.cmSnapshot = snap;
    }

    m_colorMapShader->bind();
    m_colorMapShader->setUniformValue("uKey", std::clamp(params.key, 0, 5));
    m_colorMapShader->setUniformValue("uBlend", std::clamp(params.blendMode, 0, 9));
    m_colorMapShader->setUniformValue(
        "uAdjust", static_cast<float>(params.adjustBlend) / 255.0f);
    m_colorMapShader->setUniformValue("uLut", 1);
    m_colorMapShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.cmTexture);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_colorMapShader);
}

void MultiEffectVisualizer::runBufferBlend(const BufferBlendParams& params)
{
    auto* f = QOpenGLContext::currentContext()->functions();
    SurfacePair& pair = active();
    const unsigned int cur = pair.current()->texture();
    unsigned int texA = params.bufferA >= 8 ? cur : poolTexture(params.bufferA);
    unsigned int texB = params.bufferB >= 8 ? cur : poolTexture(params.bufferB);
    if (texA == 0) texA = cur;  // empty pool slot -> current frame
    if (texB == 0) texB = cur;

    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_bufferBlendShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, texA);
    m_bufferBlendShader->setUniformValue("uSrcA", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, texB);
    m_bufferBlendShader->setUniformValue("uSrcB", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_bufferBlendShader->setUniformValue("uMode", std::clamp(params.mode, 0, 10));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_bufferBlendShader->release();
    pair.partner()->release();
    pair.swap();
    bindActive();
}

void MultiEffectVisualizer::drawScopeShape(
    const std::vector<lumi::modules::SuperscopePoint>& pts, bool dots)
{
    if (!m_scopeRenderer.ready()) return;
    if (pts.size() < (dots ? 1u : 2u)) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // additive (AVS scope default)
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = dots ? lumi::modules::SuperscopeRenderMode::Dots
                   : lumi::modules::SuperscopeRenderMode::Lines;
    rp.lineWidth = 1.0f;
    rp.dotSize = 2.0f;
    rp.glowEnabled = false;
    m_scopeRenderer.draw(pts, rp);
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runSimpleScope(const ChainNode& node,
                                           const SimpleScopeParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const QVector3D c = colorToVec(cycleScopeColor(params.colors, rt.scopeColorPos));
    const float yoff = params.position == 0 ? 0.5f : (params.position == 1 ? -0.5f : 0.0f);

    const std::vector<float> data =
        params.source == 1 ? getWaveform() : getSpectrum();
    const int n = static_cast<int>(data.size());
    if (n < 2) return;

    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
    {
        lumi::modules::SuperscopePoint p;
        p.x = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(n - 1);
        // waveform ~[-1,1] centred at yoff; spectrum [0,1] grows up from yoff-0.5
        p.y = params.source == 1 ? yoff + data[static_cast<std::size_t>(i)] * 0.4f
                                 : yoff - 0.5f + data[static_cast<std::size_t>(i)];
        p.r = c.x();
        p.g = c.y();
        p.b = c.z();
        p.a = 1.0f;
        pts.push_back(p);
    }
    drawScopeShape(pts, params.drawMode == 1);
}

void MultiEffectVisualizer::runBassSpin(const ChainNode& node,
                                        const BassSpinParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const std::vector<float> spec = getSpectrum();
    const int n = static_cast<int>(spec.size());

    for (int chn = 0; chn < 2; ++chn)
    {
        if (chn == 0 ? !params.left : !params.right) continue;

        float d = 0.0f;
        for (int x = 0; x < 44 && x < n; ++x) d += spec[static_cast<std::size_t>(x)] * 255.0f;
        float a = (d * 512.0f) / (rt.bsLastA[chn] + 30.0f * 256.0f);
        rt.bsLastA[chn] = d;
        if (a > 255.0f) a = 255.0f;
        rt.bsV[chn] = 0.7f * (std::max(a - 104.0f, 12.0f) / 96.0f) + 0.3f * rt.bsV[chn];
        rt.bsRv[chn] += 3.14159f / 6.0f * rt.bsV[chn] * (chn == 0 ? -1.0f : 1.0f);

        const float radius = a / 256.0f * 0.5f;  // fraction of half-panel (NDC)
        const float cx = chn == 0 ? -0.5f : 0.5f;
        const float xp = std::cos(rt.bsRv[chn]) * radius;
        const float yp = std::sin(rt.bsRv[chn]) * radius;
        const QVector3D col = colorToVec(chn == 0 ? params.colorLeft : params.colorRight);

        auto mk = [&](float x, float y) {
            lumi::modules::SuperscopePoint p;
            p.x = cx + x;
            p.y = y;
            p.r = col.x();
            p.g = col.y();
            p.b = col.z();
            p.a = 1.0f;
            return p;
        };
        // Two spokes from the panel centre (mode 1 = line-approximated fill).
        std::vector<lumi::modules::SuperscopePoint> pts{mk(xp, yp), mk(0.0f, 0.0f),
                                                        mk(-xp, -yp)};
        drawScopeShape(pts, false);
    }
}

bool MultiEffectVisualizer::ensureEmbeddedTexture(LeafRuntime& rt,
                                                  const std::string& imageData,
                                                  bool fallbackDot)
{
    if (rt.picTexture != 0) return true;
    auto* f = QOpenGLContext::currentContext()->functions();
    QImage img;
    bool loaded = false;
    if (!imageData.empty())
    {
        const QByteArray raw =
            QByteArray::fromBase64(QByteArray::fromStdString(imageData));
        loaded = img.loadFromData(raw);
    }
    if (!loaded)
    {
        // Texer/Texer II render their built-in default when the image is
        // missing (Acko default texture): a small soft white dot. Picture
        // effects keep the hard fail.
        if (!fallbackDot) return false;
        constexpr int kDot = 16;
        img = QImage(kDot, kDot, QImage::Format_RGBA8888);
        for (int y = 0; y < kDot; ++y)
        {
            for (int x = 0; x < kDot; ++x)
            {
                const float dx = (x + 0.5f) / kDot * 2.0f - 1.0f;
                const float dy = (y + 0.5f) / kDot * 2.0f - 1.0f;
                const float d = std::sqrt(dx * dx + dy * dy);
                const int v = static_cast<int>(
                    std::clamp(1.0f - d, 0.0f, 1.0f) * 255.0f);
                img.setPixel(x, y, qRgba(v, v, v, v));
            }
        }
    }
    img = img.convertToFormat(QImage::Format_RGBA8888);
    f->glGenTextures(1, &rt.picTexture);
    f->glBindTexture(GL_TEXTURE_2D, rt.picTexture);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, img.constBits());
    rt.picW = img.width();
    rt.picH = img.height();
    return true;
}

void MultiEffectVisualizer::drawEmbeddedImage(LeafRuntime& rt,
                                              const std::string& imageData, int blend,
                                              bool keepAspect)
{
    if (!ensureEmbeddedTexture(rt, imageData)) return;
    auto* f = QOpenGLContext::currentContext()->functions();

    SurfacePair& pair = active();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_pictureShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_pictureShader->setUniformValue("uTex", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.picTexture);
    m_pictureShader->setUniformValue("uImg", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_pictureShader->setUniformValue("uBlend", std::clamp(blend, 0, 2));
    m_pictureShader->setUniformValue("uKeepAspect", keepAspect ? 1 : 0);
    m_pictureShader->setUniformValue(
        "uImgSize", QVector2D(static_cast<float>(std::max(1, rt.picW)),
                              static_cast<float>(std::max(1, rt.picH))));
    m_pictureShader->setUniformValue("uRes",
                                     QVector2D(static_cast<float>(m_surfaceWidth),
                                               static_cast<float>(m_surfaceHeight)));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_pictureShader->release();
    pair.partner()->release();
    pair.swap();
    bindActive();
}

void MultiEffectVisualizer::runPicture(const ChainNode& node, const PictureParams& params)
{
    drawEmbeddedImage(m_leafRuntimes[node.nodeId], params.imageData, params.blend,
                      params.keepAspect);
}

void MultiEffectVisualizer::runPictureII(const ChainNode& node,
                                         const PictureIIParams& params)
{
    // Picture II stretches to fill (no aspect lock); bilinear is already on.
    drawEmbeddedImage(m_leafRuntimes[node.nodeId], params.imageData, params.blend, false);
}

void MultiEffectVisualizer::runTexer(const ChainNode& node, const TexerParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!ensureEmbeddedTexture(rt, params.imageData, /*fallbackDot=*/true)) return;
    const std::vector<float> wave = getWaveform();
    const int wn = static_cast<int>(wave.size());
    const int n = std::clamp(params.particles, 1, 4096);

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(params.blend == 0 ? GL_ONE : GL_SRC_ALPHA, GL_ONE);
    m_spriteShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, rt.picTexture);
    m_spriteShader->setUniformValue("uImg", 0);
    m_spriteShader->setUniformValue("uTint", QVector3D(1.0f, 1.0f, 1.0f));
    m_spriteShader->setUniformValue("uColorFilter", 0);
    const float hx = static_cast<float>(rt.picW) / static_cast<float>(m_surfaceWidth);
    const float hy = static_cast<float>(rt.picH) / static_cast<float>(m_surfaceHeight);
    m_spriteShader->setUniformValue("uHalf", QVector2D(hx, hy));
    for (int pt = 0; pt < n; ++pt)
    {
        const float t = n > 1 ? static_cast<float>(pt) / static_cast<float>(n - 1) : 0.0f;
        const float x = t * 2.0f - 1.0f;
        const float y = wn > 0 ? wave[static_cast<std::size_t>(std::clamp(
                                     static_cast<int>(t * (wn - 1)), 0, wn - 1))] * 0.8f
                               : 0.0f;
        m_spriteShader->setUniformValue("uCenter", QVector2D(x, y));
        f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    m_quadVao->release();
    m_spriteShader->release();
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runTexerII(const ChainNode& node, const TexerIIParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!ensureEmbeddedTexture(rt, params.imageData, /*fallbackDot=*/true)) return;

    const std::string combined = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + '\n' + params.pointCode;
    if (rt.texerHost == nullptr || rt.texerCompiled != combined)
    {
        rt.texerHost = std::make_unique<ScriptSlotHost>("texer2", activeContext(),
                                                        ScriptSlotHost::Dialect::Avs);
        rt.texerHost->setSource(Slot::Init, params.initCode);
        rt.texerHost->setSource(Slot::Frame, params.frameCode);
        rt.texerHost->setSource(Slot::Beat, params.beatCode);
        rt.texerHost->setSource(Slot::Point, params.pointCode);
        rt.texerHost->compileAll();
        rt.texerCompiled = combined;
        rt.texerHost->engine().setNumber("n", 100.0);
        rt.texerHost->run(Slot::Init);
    }
    auto& engine = rt.texerHost->engine();
    engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
    feedAudio(rt.texerHost->engine());
    if (rt.texerHost->has(Slot::Frame)) rt.texerHost->run(Slot::Frame);
    if (m_frameBeat && rt.texerHost->has(Slot::Beat)) rt.texerHost->run(Slot::Beat);
    const int n = std::clamp(static_cast<int>(engine.number("n")), 1, 4096);

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    m_spriteShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, rt.picTexture);
    m_spriteShader->setUniformValue("uImg", 0);
    m_spriteShader->setUniformValue("uColorFilter", params.colorFiltering ? 1 : 0);
    const float baseHx = static_cast<float>(rt.picW) / static_cast<float>(m_surfaceWidth);
    const float baseHy = static_cast<float>(rt.picH) / static_cast<float>(m_surfaceHeight);
    for (int pt = 0; pt < n; ++pt)
    {
        engine.setNumber("i", n > 1 ? static_cast<double>(pt) / (n - 1) : 0.0);
        engine.setNumber("x", 0.0);
        engine.setNumber("y", 0.0);
        engine.setNumber("sizex", 1.0);
        engine.setNumber("sizey", 1.0);
        engine.setNumber("red", 1.0);
        engine.setNumber("green", 1.0);
        engine.setNumber("blue", 1.0);
        if (rt.texerHost->has(Slot::Point)) rt.texerHost->run(Slot::Point);
        const float x = static_cast<float>(engine.number("x"));
        const float y = static_cast<float>(engine.number("y"));
        const float sx = static_cast<float>(engine.number("sizex"));
        const float sy = static_cast<float>(engine.number("sizey"));
        m_spriteShader->setUniformValue("uCenter", QVector2D(x, -y));  // AVS y is down
        m_spriteShader->setUniformValue("uHalf", QVector2D(baseHx * sx, baseHy * sy));
        m_spriteShader->setUniformValue(
            "uTint", QVector3D(static_cast<float>(engine.number("red")),
                               static_cast<float>(engine.number("green")),
                               static_cast<float>(engine.number("blue"))));
        f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    m_quadVao->release();
    m_spriteShader->release();
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runTriangle(const ChainNode& node, const TriangleParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const std::string combined = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + '\n' + params.pointCode;
    if (rt.triHost == nullptr || rt.triCompiled != combined)
    {
        rt.triHost = std::make_unique<ScriptSlotHost>("triangle", activeContext(),
                                                      ScriptSlotHost::Dialect::Avs);
        rt.triHost->setSource(Slot::Init, params.initCode);
        rt.triHost->setSource(Slot::Frame, params.frameCode);
        rt.triHost->setSource(Slot::Beat, params.beatCode);
        rt.triHost->setSource(Slot::Point, params.pointCode);
        rt.triHost->compileAll();
        rt.triCompiled = combined;
        rt.triHost->engine().setNumber("n", 1.0);
        rt.triHost->run(Slot::Init);
    }
    auto& engine = rt.triHost->engine();
    engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
    feedAudio(rt.triHost->engine());
    if (rt.triHost->has(Slot::Frame)) rt.triHost->run(Slot::Frame);
    if (m_frameBeat && rt.triHost->has(Slot::Beat)) rt.triHost->run(Slot::Beat);
    const int n = std::clamp(static_cast<int>(engine.number("n")), 0, 4096);

    // Filled triangles are approximated as wireframe outlines via the ScopeRenderer.
    for (int pt = 0; pt < n; ++pt)
    {
        engine.setNumber("i", n > 1 ? static_cast<double>(pt) / (n - 1) : 0.0);
        engine.setNumber("red", 1.0);
        engine.setNumber("green", 1.0);
        engine.setNumber("blue", 1.0);
        if (rt.triHost->has(Slot::Point)) rt.triHost->run(Slot::Point);
        const QVector3D col(static_cast<float>(engine.number("red")),
                            static_cast<float>(engine.number("green")),
                            static_cast<float>(engine.number("blue")));
        auto mk = [&](const char* xn, const char* yn) {
            lumi::modules::SuperscopePoint p;
            p.x = static_cast<float>(engine.number(xn));
            p.y = -static_cast<float>(engine.number(yn));  // AVS y is down
            p.r = col.x();
            p.g = col.y();
            p.b = col.z();
            p.a = 1.0f;
            return p;
        };
        const std::vector<lumi::modules::SuperscopePoint> pts{
            mk("x1", "y1"), mk("x2", "y2"), mk("x3", "y3"), mk("x1", "y1")};
        drawScopeShape(pts, false);
    }
}

void MultiEffectVisualizer::runOscStar(const ChainNode& node, const OscStarParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const QVector3D c = colorToVec(cycleScopeColor(params.colors, rt.scopeColorPos));
    const std::vector<float> wave = getWaveform();
    const int n = static_cast<int>(wave.size());
    if (n < 2) return;

    rt.scopeRot += static_cast<float>(params.rot - 8) * 0.02f;  // 8 = still (sight-test)
    const float len = std::clamp(params.size, 0, 16) / 16.0f;   // spoke length (NDC)
    const float cx = params.position == 0 ? -0.5f : (params.position == 1 ? 0.5f : 0.0f);
    const float aspect = m_surfaceHeight > 0
                             ? static_cast<float>(m_surfaceWidth) / static_cast<float>(m_surfaceHeight)
                             : 1.0f;

    for (int q = 0; q < 5; ++q)
    {
        const float ang = rt.scopeRot + static_cast<float>(q) * (2.0f * 3.14159f / 5.0f);
        const float dx = std::cos(ang);
        const float dy = std::sin(ang);
        std::vector<lumi::modules::SuperscopePoint> pts;
        pts.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i)
        {
            const float t = static_cast<float>(i) / static_cast<float>(n - 1);
            const float rad = t * len;
            const float off = wave[static_cast<std::size_t>(i)] * len * 0.5f;
            lumi::modules::SuperscopePoint p;
            p.x = cx + (dx * rad - dy * off) / aspect;  // reduce x/y ellipse distortion
            p.y = dy * rad + dx * off;
            p.r = c.x();
            p.g = c.y();
            p.b = c.z();
            p.a = 1.0f;
            pts.push_back(p);
        }
        drawScopeShape(pts, false);
    }
}

void MultiEffectVisualizer::runOscRing(const ChainNode& node, const OscRingParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const QVector3D c = colorToVec(cycleScopeColor(params.colors, rt.scopeColorPos));
    const std::vector<float> data = params.source == 0 ? getWaveform() : getSpectrum();
    const int n = static_cast<int>(data.size());
    if (n < 2) return;

    const float rad0 = std::clamp(params.size, 0, 16) / 16.0f;
    const float cx = params.position == 0 ? -0.5f : (params.position == 1 ? 0.5f : 0.0f);
    const float aspect = m_surfaceHeight > 0
                             ? static_cast<float>(m_surfaceWidth) / static_cast<float>(m_surfaceHeight)
                             : 1.0f;

    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(81);
    for (int q = 0; q <= 80; ++q)
    {
        const float a = -static_cast<float>(q) * (2.0f * 3.14159f / 80.0f);
        const int idx = std::clamp((q % 80) * (n - 1) / 80, 0, n - 1);
        const float sca = 0.1f + std::abs(data[static_cast<std::size_t>(idx)]) * 0.9f;
        const float rr = rad0 * sca;
        lumi::modules::SuperscopePoint p;
        p.x = cx + std::cos(a) * rr / aspect;
        p.y = std::sin(a) * rr;
        p.r = c.x();
        p.g = c.y();
        p.b = c.z();
        p.a = 1.0f;
        pts.push_back(p);
    }
    drawScopeShape(pts, false);
}

void MultiEffectVisualizer::runRotatingStars(const ChainNode& node,
                                             const RotatingStarsParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const QVector3D c = colorToVec(cycleScopeColor(params.colors, rt.scopeColorPos));
    const std::vector<float> spec = getSpectrum();
    rt.scopeRot += 0.05f;

    float peak = 0.0f;
    for (int l = 3; l < 14 && l < static_cast<int>(spec.size()); ++l)
        peak = std::max(peak, spec[static_cast<std::size_t>(l)]);
    const float vw = (peak * 0.5f + 0.12f) * 0.5f;  // star radius (NDC)
    const float ox = std::cos(rt.scopeRot) * 0.5f;
    const float oy = std::sin(rt.scopeRot) * 0.5f;

    for (int ch = 0; ch < 2; ++ch)
    {
        const float bx = ch == 0 ? ox : -ox;
        const float by = ch == 0 ? oy : -oy;
        float r2 = -rt.scopeRot;
        std::vector<lumi::modules::SuperscopePoint> pts;
        pts.reserve(6);
        for (int t = 0; t <= 5; ++t)
        {
            lumi::modules::SuperscopePoint p;
            p.x = bx + std::cos(r2) * vw;
            p.y = by + std::sin(r2) * vw;
            p.r = c.x();
            p.g = c.y();
            p.b = c.z();
            p.a = 1.0f;
            pts.push_back(p);
            r2 += 3.14159f * 4.0f / 5.0f;  // pentagram step
        }
        drawScopeShape(pts, false);
    }
}

void MultiEffectVisualizer::runColorClip(const ColorClipParams& params)
{
    const QVector3D clip = colorToVec(params.clipColor);
    const QVector3D outc = colorToVec(params.outColor);
    m_colorClipShader->bind();
    m_colorClipShader->setUniformValue("uMode", std::clamp(params.mode, 1, 3));
    m_colorClipShader->setUniformValue("uClip", clip);
    m_colorClipShader->setUniformValue("uOut", outc);
    m_colorClipShader->setUniformValue(
        "uDist", static_cast<float>(params.distance) * 2.0f / 255.0f);
    m_colorClipShader->release();
    transformPass(*m_colorClipShader);
}

void MultiEffectVisualizer::runUniqueTone(const UniqueToneParams& params)
{
    m_uniqueToneShader->bind();
    m_uniqueToneShader->setUniformValue("uColor", colorToVec(params.color));
    m_uniqueToneShader->setUniformValue("uInvert", params.invert ? 1 : 0);
    m_uniqueToneShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_uniqueToneShader->release();
    transformPass(*m_uniqueToneShader);
}

void MultiEffectVisualizer::runInterleave(const ChainNode& node,
                                          const InterleaveParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!rt.interSeeded)
    {
        rt.interCurX = static_cast<float>(params.x);
        rt.interCurY = static_cast<float>(params.y);
        rt.interSeeded = true;
    }
    // Ease towards x/y (r_interleave sc1), jump to x2/y2 on beat.
    const float sc1 = (static_cast<float>(params.beatDuration) + 512.0f - 64.0f) / 512.0f;
    rt.interCurX = rt.interCurX * sc1 + static_cast<float>(params.x) * (1.0f - sc1);
    rt.interCurY = rt.interCurY * sc1 + static_cast<float>(params.y) * (1.0f - sc1);
    if (m_frameBeat && params.onBeat)
    {
        rt.interCurX = static_cast<float>(params.x2);
        rt.interCurY = static_cast<float>(params.y2);
    }

    m_interleaveShader->bind();
    m_interleaveShader->setUniformValue("uRes",
                                        QVector2D(static_cast<float>(m_surfaceWidth),
                                                  static_cast<float>(m_surfaceHeight)));
    m_interleaveShader->setUniformValue(
        "uSpacing", QPoint(std::max(0, static_cast<int>(rt.interCurX)),
                           std::max(0, static_cast<int>(rt.interCurY))));
    m_interleaveShader->setUniformValue("uColor", colorToVec(params.color));
    m_interleaveShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_interleaveShader->release();
    transformPass(*m_interleaveShader);
}

void MultiEffectVisualizer::runConvolution(const ConvolutionParams& params)
{
    std::array<float, 49> kf{};
    for (int i = 0; i < 49; ++i)
        kf[static_cast<std::size_t>(i)] = static_cast<float>(params.kernel[static_cast<std::size_t>(i)]);
    const float scale = params.scale != 0 ? static_cast<float>(params.scale) : 1.0f;

    auto pass = [&] {
        m_convolutionShader->bind();
        m_convolutionShader->setUniformValue(
            "uRes", QVector2D(static_cast<float>(m_surfaceWidth),
                              static_cast<float>(m_surfaceHeight)));
        m_convolutionShader->setUniformValueArray("uKernel", kf.data(), 49, 1);
        m_convolutionShader->setUniformValue("uScale", scale);
        m_convolutionShader->setUniformValue("uBias", static_cast<float>(params.bias));
        m_convolutionShader->setUniformValue("uAbsolute", params.absolute ? 1 : 0);
        m_convolutionShader->setUniformValue("uEdge", std::clamp(params.edgeMode, 0, 1));
        m_convolutionShader->release();
        transformPass(*m_convolutionShader);
    };
    pass();
    if (params.twoPass) pass();
}

void MultiEffectVisualizer::runNormalise()
{
    auto* f = QOpenGLContext::currentContext()->functions();
    auto* extra = QOpenGLContext::currentContext()->extraFunctions();
    constexpr int kR = 32;
    if (m_reduceFbo == nullptr)
    {
        m_reduceFbo = std::make_unique<QOpenGLFramebufferObject>(kR, kR);
    }

    // Downscale the current buffer into the small FBO, then read it back to find
    // the luminance min/max (small readback — one GPU sync per Normalise node).
    SurfacePair& pair = active();
    extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, pair.current()->handle());
    extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_reduceFbo->handle());
    extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0, kR, kR,
                             GL_COLOR_BUFFER_BIT, GL_LINEAR);

    m_reduceFbo->bind();
    std::array<unsigned char, kR * kR * 4> px{};
    f->glReadPixels(0, 0, kR, kR, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    m_reduceFbo->release();

    float lo = 1.0f;
    float hi = 0.0f;
    for (int i = 0; i < kR * kR; ++i)
    {
        const float m = static_cast<float>(std::max({px[i * 4], px[i * 4 + 1],
                                                      px[i * 4 + 2]})) / 255.0f;
        lo = std::min(lo, m);
        hi = std::max(hi, m);
    }

    bindActive();  // restore the working surface before the stretch pass
    m_normaliseShader->bind();
    m_normaliseShader->setUniformValue("uLo", lo);
    m_normaliseShader->setUniformValue("uHi", hi);
    m_normaliseShader->release();
    transformPass(*m_normaliseShader);
}

void MultiEffectVisualizer::runMultiFilter(const MultiFilterParams& params)
{
    if (params.onBeat && !m_frameBeat) return;  // only on beat frames
    m_multiFilterShader->bind();
    m_multiFilterShader->setUniformValue("uEffect", std::clamp(params.effect, 0, 3));
    m_multiFilterShader->release();
    transformPass(*m_multiFilterShader);
}

void MultiEffectVisualizer::runAddBorders(const AddBordersParams& params)
{
    m_addBordersShader->bind();
    m_addBordersShader->setUniformValue("uRes",
                                        QVector2D(static_cast<float>(m_surfaceWidth),
                                                  static_cast<float>(m_surfaceHeight)));
    m_addBordersShader->setUniformValue("uColor", colorToVec(params.color));
    m_addBordersShader->setUniformValue("uSize", std::max(0, params.size));
    m_addBordersShader->release();
    transformPass(*m_addBordersShader);
}

void MultiEffectVisualizer::runJherikoGlobal(const ChainNode& node,
                                             const JherikoGlobalParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const std::string combined = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode;
    if (rt.jherikoHost == nullptr || rt.jherikoCompiled != combined)
    {
        rt.jherikoHost = std::make_unique<ScriptSlotHost>("jheriko", activeContext(),
                                                          ScriptSlotHost::Dialect::Avs);
        rt.jherikoHost->setSource(Slot::Init, params.initCode);
        rt.jherikoHost->setSource(Slot::Frame, params.frameCode);
        rt.jherikoHost->setSource(Slot::Beat, params.beatCode);
        rt.jherikoHost->compileAll();
        rt.jherikoCompiled = combined;
        rt.jherikoInited = false;
    }
    // load 0 none / 1 once / 2 code / 3 every-frame (file reload not imported —
    // here it only governs when Init re-runs).
    if ((!rt.jherikoInited || params.loadMode == 3) && rt.jherikoHost->has(Slot::Init))
    {
        rt.jherikoHost->run(Slot::Init);
    }
    rt.jherikoInited = true;
    feedAudio(rt.jherikoHost->engine());
    if (rt.jherikoHost->has(Slot::Frame)) rt.jherikoHost->run(Slot::Frame);
    if (m_frameBeat && rt.jherikoHost->has(Slot::Beat)) rt.jherikoHost->run(Slot::Beat);
    // No visual output — globals are shared through the ScriptContext.
}

void MultiEffectVisualizer::runDynamicDistanceModifier(
    const ChainNode& node, const DynamicDistanceModifierParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    const std::string combined = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + '\n' + params.pixelCode;
    if (rt.ddmHost == nullptr || rt.ddmCompiled != combined)
    {
        rt.ddmHost = std::make_unique<ScriptSlotHost>("ddm", activeContext(),
                                                      ScriptSlotHost::Dialect::Avs);
        rt.ddmHost->setSource(Slot::Init, params.initCode);
        rt.ddmHost->setSource(Slot::Frame, params.frameCode);
        rt.ddmHost->setSource(Slot::Beat, params.beatCode);
        rt.ddmHost->setSource(Slot::Point, params.pixelCode);
        rt.ddmHost->compileAll();
        rt.ddmCompiled = combined;
        rt.ddmHost->engine().setNumber("d", 0.0);
        rt.ddmHost->engine().setNumber("b", 0.0);
        rt.ddmHost->run(Slot::Init);
    }

    auto& engine = rt.ddmHost->engine();
    engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
    feedAudio(rt.ddmHost->engine());
    if (rt.ddmHost->has(Slot::Frame)) rt.ddmHost->run(Slot::Frame);
    if (m_frameBeat && rt.ddmHost->has(Slot::Beat)) rt.ddmHost->run(Slot::Beat);

    // 1-D distance LUT: input ring distance (0..1) -> output distance (0..1).
    std::array<float, 256> lut{};
    const bool hasPixel = rt.ddmHost->has(Slot::Point);
    for (int i = 0; i < 256; ++i)
    {
        const double din = static_cast<double>(i) / 255.0;
        if (hasPixel)
        {
            engine.setNumber("d", din);
            rt.ddmHost->run(Slot::Point);
            lut[static_cast<std::size_t>(i)] = static_cast<float>(engine.number("d"));
        }
        else
        {
            lut[static_cast<std::size_t>(i)] = static_cast<float>(din);
        }
    }

    const float maxD = 0.5f * std::sqrt(static_cast<float>(m_surfaceWidth) *
                                            static_cast<float>(m_surfaceWidth) +
                                        static_cast<float>(m_surfaceHeight) *
                                            static_cast<float>(m_surfaceHeight));
    m_ddmShader->bind();
    m_ddmShader->setUniformValue("uRes",
                                 QVector2D(static_cast<float>(m_surfaceWidth),
                                           static_cast<float>(m_surfaceHeight)));
    m_ddmShader->setUniformValue("uMaxD", maxD);
    m_ddmShader->setUniformValueArray("uLut", lut.data(), 256, 1);
    m_ddmShader->setUniformValue("uBlend", params.blend ? 1 : 0);
    m_ddmShader->release();
    transformPass(*m_ddmShader);
}

void MultiEffectVisualizer::runMovingParticle(const ChainNode& node,
                                              const MovingParticleParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!rt.mpSeeded)
    {
        rt.mpPx = -0.6f;
        rt.mpPy = 0.3f;
        rt.mpVx = -0.01551f;
        rt.mpVy = 0.0f;
        rt.mpCx = 0.0f;
        rt.mpCy = 0.0f;
        rt.mpSize = static_cast<float>(params.size);
        rt.mpSeeded = true;
    }

    auto rnd = [this] {
        return (static_cast<int>(nextRandom() % 33) - 16) / 48.0f;
    };
    if (m_frameBeat)
    {
        rt.mpCx = rnd();
        rt.mpCy = rnd();
    }
    rt.mpVx -= 0.004f * (rt.mpPx - rt.mpCx);
    rt.mpVy -= 0.004f * (rt.mpPy - rt.mpCy);
    rt.mpPx += rt.mpVx;
    rt.mpPy += rt.mpVy;
    rt.mpVx *= 0.991f;
    rt.mpVy *= 0.991f;

    // Size ease (r_parts s_pos): jump to size2 on beat, then relax to size.
    if (m_frameBeat && params.onBeatSize) rt.mpSize = static_cast<float>(params.size2);
    const float sz = rt.mpSize;
    rt.mpSize = (rt.mpSize + static_cast<float>(params.size)) * 0.5f;

    const float ss = std::min(static_cast<float>(m_surfaceHeight) * 0.5f,
                              static_cast<float>(m_surfaceWidth) * 3.0f / 8.0f);
    const float scale = ss * static_cast<float>(params.maxDistance) / 32.0f;
    const float halfW = static_cast<float>(m_surfaceWidth) * 0.5f;
    const float halfH = static_cast<float>(m_surfaceHeight) * 0.5f;
    if (halfW <= 0.0f || halfH <= 0.0f || !m_scopeRenderer.ready()) return;

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    float alpha = 1.0f;
    switch (params.blend)
    {
        case 0:  f->glBlendFunc(GL_ONE, GL_ZERO); break;              // replace
        case 2:  f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); alpha = 0.5f; break;  // 50/50
        default: f->glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;        // additive / line
    }

    lumi::modules::SuperscopePoint pt;
    pt.x = std::clamp(rt.mpPx * scale / halfW, -1.0f, 1.0f);
    pt.y = std::clamp(-rt.mpPy * scale / halfH, -1.0f, 1.0f);  // AVS y is top-down
    const QVector3D c = colorToVec(params.color);
    pt.r = c.x();
    pt.g = c.y();
    pt.b = c.z();
    pt.a = alpha;
    const std::vector<lumi::modules::SuperscopePoint> points{pt};

    lumi::render::ScopeRenderer::Params rp;
    rp.mode = lumi::modules::SuperscopeRenderMode::Dots;
    rp.dotSize = std::clamp(sz, 1.0f, 128.0f);
    rp.glowEnabled = false;
    m_scopeRenderer.draw(points, rp);
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runWaterBump(const ChainNode& node,
                                         const WaterBumpParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();

    // RGBA16F height ping-pong (.r current, .g previous), (re)made on resize.
    if (rt.wbHeight[0] == nullptr || rt.wbW != m_surfaceWidth ||
        rt.wbH != m_surfaceHeight)
    {
        QOpenGLFramebufferObjectFormat fmt;
        fmt.setInternalTextureFormat(GL_RGBA16F);
        for (auto& fbo : rt.wbHeight)
        {
            fbo = std::make_unique<QOpenGLFramebufferObject>(m_surfaceWidth,
                                                             m_surfaceHeight, fmt);
            fbo->bind();
            f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            f->glClear(GL_COLOR_BUFFER_BIT);
            fbo->release();
        }
        rt.wbCur = 0;
        rt.wbW = m_surfaceWidth;
        rt.wbH = m_surfaceHeight;
    }
    if (!rt.wbHeight[0]->isValid() || !rt.wbHeight[1]->isValid()) return;

    const QVector2D texel(1.0f / static_cast<float>(m_surfaceWidth),
                          1.0f / static_cast<float>(m_surfaceHeight));

    // Drop on beat: random spot or a position code (0 near / 1 mid / 2 far).
    int drop = 0;
    QVector2D dropC(0.5f, 0.5f);
    if (m_frameBeat)
    {
        drop = 1;
        if (params.randomDrop)
        {
            dropC = QVector2D(static_cast<float>(nextRandom() & 0xffff) / 65535.0f,
                              static_cast<float>(nextRandom() & 0xffff) / 65535.0f);
        }
        else
        {
            auto code = [](int c) { return c <= 0 ? 0.25f : (c >= 2 ? 0.75f : 0.5f); };
            dropC = QVector2D(code(params.dropX), code(params.dropY));
        }
    }

    // --- Wave propagation: current page -> the other page --------------------
    const int src = rt.wbCur;
    const int dst = 1 - rt.wbCur;
    rt.wbHeight[dst]->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_wbPropShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, rt.wbHeight[src]->texture());
    m_wbPropShader->setUniformValue("uH", 0);
    m_wbPropShader->setUniformValue("uTexel", texel);
    m_wbPropShader->setUniformValue(
        "uDamp", 1.0f / static_cast<float>(1 << std::clamp(params.density, 1, 12)));
    m_wbPropShader->setUniformValue("uDrop", drop);
    m_wbPropShader->setUniformValue("uDropC", dropC);
    m_wbPropShader->setUniformValue(
        "uDropR", static_cast<float>(params.dropRadius) / static_cast<float>(m_surfaceWidth));
    m_wbPropShader->setUniformValue("uDropAmp", -static_cast<float>(params.depth) / 100.0f);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_wbPropShader->release();
    rt.wbHeight[dst]->release();
    rt.wbCur = dst;

    // --- Refraction: warp the image by the new height gradient ---------------
    SurfacePair& pair = active();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_wbDispShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_wbDispShader->setUniformValue("uImg", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.wbHeight[rt.wbCur]->texture());
    m_wbDispShader->setUniformValue("uH", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_wbDispShader->setUniformValue("uTexel", texel);
    m_wbDispShader->setUniformValue("uRes",
                                    QVector2D(static_cast<float>(m_surfaceWidth),
                                              static_cast<float>(m_surfaceHeight)));
    m_wbDispShader->setUniformValue("uScale", params.displaceScale);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_wbDispShader->release();
    pair.partner()->release();
    pair.swap();
    bindActive();
}

void MultiEffectVisualizer::runStarfield(const ChainNode& node,
                                         const StarfieldParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const int count = std::clamp(params.maxStars, 1, 8192);
    auto frand = [this] { return static_cast<float>(nextRandom() & 0xffff) / 65535.0f; };
    auto respawn = [&](Star& s) {
        s.x = frand() * 2.0f - 1.0f;
        s.y = frand() * 2.0f - 1.0f;
        s.z = 0.02f + frand() * 0.98f;
        s.speed = 0.1f + frand() * 0.8f;
    };

    if (static_cast<int>(rt.stars.size()) != count)
    {
        rt.stars.resize(static_cast<std::size_t>(count));
        for (Star& s : rt.stars) respawn(s);
    }

    // Warp-speed ease on beat (r_stars incBeat, same shape as Mosaic depth).
    if (rt.starSpeed <= 0.0f) rt.starSpeed = params.warpSpeed;
    if (params.onBeat && m_frameBeat)
    {
        rt.starSpeed = params.beatSpeed;
        rt.starBeatFrames = std::max(1, params.durationFrames);
    }
    else if (rt.starBeatFrames == 0)
    {
        rt.starSpeed = params.warpSpeed;
    }
    if (rt.starBeatFrames > 0)
    {
        if (--rt.starBeatFrames > 0)
        {
            const float step = (params.warpSpeed - params.beatSpeed) /
                               static_cast<float>(std::max(1, params.durationFrames));
            rt.starSpeed += step;
        }
        else
        {
            rt.starSpeed = params.warpSpeed;
        }
    }

    const QVector3D tint = colorToVec(params.color);
    std::vector<lumi::modules::SuperscopePoint> points;
    points.reserve(rt.stars.size());
    for (Star& s : rt.stars)
    {
        s.z -= s.speed * rt.starSpeed / 255.0f;
        if (s.z <= 0.02f) { respawn(s); continue; }
        const float px = s.x / s.z;
        const float py = s.y / s.z;
        if (px <= -1.0f || px >= 1.0f || py <= -1.0f || py >= 1.0f) continue;
        const float bright = std::clamp((1.0f - s.z) * s.speed * 1.6f, 0.0f, 1.0f);
        lumi::modules::SuperscopePoint p;
        p.x = px;
        p.y = py;
        p.r = tint.x();
        p.g = tint.y();
        p.b = tint.z();
        p.a = bright;
        points.push_back(p);
    }

    if (points.empty() || !m_scopeRenderer.ready()) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    // r_stars.cpp:245 BLEND/BLEND_AVG/replace; brightness stays premodulated
    // via the point alpha, so "replace" keeps SRC_ALPHA weighting into black.
    switch (params.blend)
    {
        case 1:  f->glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;                  // additive
        case 2:  f->glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
                 f->glBlendColor(0.0f, 0.0f, 0.0f, 0.5f); break;               // 50/50
        default: f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;  // replace
    }
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = lumi::modules::SuperscopeRenderMode::Dots;
    rp.dotSize = 2.0f;
    rp.glowEnabled = false;
    m_scopeRenderer.draw(points, rp);
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runFyrewurX(const ChainNode& node,
                                        const FyrewurXParams& params)
{
    // Behavioral rebuild of the closed-source "FunkyFX FyrewurX v1" APE:
    // every beat launches a firework burst; sparks fly out radially, gravity
    // pulls them down (AVS convention: +y = screen bottom), and they fade out
    // over their lifetime. Constants are sight-calibration points.
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    if (m_frameBeat && rt.fwSparks.size() < 4096)
    {
        auto frand = [this] { return (nextRandom() % 10000u) / 10000.0f; };
        const float cx = frand() * 1.6f - 0.8f;         // burst center
        const float cy = frand() * 1.0f - 0.7f;         // upper screen area
        // Firework hue per burst: bright saturated color, slight per-spark drift.
        const float hue = frand() * 6.0f;
        auto hueRgb = [](float h, float& r, float& g, float& b) {
            const float x = 1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f);
            switch (static_cast<int>(h) % 6)
            {
                case 0: r = 1; g = x; b = 0; break;
                case 1: r = x; g = 1; b = 0; break;
                case 2: r = 0; g = 1; b = x; break;
                case 3: r = 0; g = x; b = 1; break;
                case 4: r = x; g = 0; b = 1; break;
                default: r = 1; g = 0; b = x; break;
            }
        };
        for (int i = 0; i < params.sparks; ++i)
        {
            FwSpark s;
            const float ang = frand() * 6.2831853f;
            const float spd = params.speed * (0.25f + 0.75f * frand());
            s.x = cx;
            s.y = cy;
            s.vx = std::cos(ang) * spd;
            s.vy = std::sin(ang) * spd;
            s.lifeMax = params.lifeSeconds * (0.6f + 0.4f * frand());
            s.life = s.lifeMax;
            hueRgb(hue + (frand() - 0.5f) * 0.6f, s.r, s.g, s.b);
            rt.fwSparks.push_back(s);
        }
    }

    // Integrate + cull.
    const float dt = std::clamp(m_deltaTime, 0.0f, 0.1f);
    for (FwSpark& s : rt.fwSparks)
    {
        s.vy += params.gravity * dt;
        s.x += s.vx * dt;
        s.y += s.vy * dt;
        s.life -= dt;
    }
    std::erase_if(rt.fwSparks, [](const FwSpark& s) {
        return s.life <= 0.0f || s.x < -1.2f || s.x > 1.2f || s.y > 1.2f;
    });
    if (rt.fwSparks.empty() || !m_scopeRenderer.ready()) return;

    std::vector<lumi::modules::SuperscopePoint> points;
    points.reserve(rt.fwSparks.size());
    for (const FwSpark& s : rt.fwSparks)
    {
        lumi::modules::SuperscopePoint p;
        p.x = s.x;
        p.y = s.y;
        p.r = s.r;
        p.g = s.g;
        p.b = s.b;
        p.a = std::clamp(s.life / s.lifeMax, 0.0f, 1.0f);  // fade out
        points.push_back(p);
    }

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // additive sparks
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = lumi::modules::SuperscopeRenderMode::Dots;
    rp.dotSize = 2.0f;
    rp.glowEnabled = false;
    m_scopeRenderer.draw(points, rp);
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runTimescope(const ChainNode& node,
                                         const TimescopeParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();
    if (m_surfaceWidth <= 0) return;
    rt.timescopeX = (rt.timescopeX + 1) % m_surfaceWidth;

    // Upload this frame's spectrum to a 1D texture (R32F).
    const std::vector<float> spec = getSpectrum();
    static const float kZero = 0.0f;
    const int n = spec.empty() ? 1 : static_cast<int>(spec.size());
    if (m_specTex == 0) f->glGenTextures(1, &m_specTex);
    f->glBindTexture(GL_TEXTURE_2D, m_specTex);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, n, 1, 0, GL_RED, GL_FLOAT,
                    spec.empty() ? &kZero : spec.data());

    // Draw one column in place (no swap) into the working buffer.
    SurfacePair& pair = active();
    pair.current()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    f->glEnable(GL_SCISSOR_TEST);
    f->glScissor(rt.timescopeX, 0, 1, m_surfaceHeight);
    if (params.blend == 1)
    {
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_ONE, GL_ONE);  // additive
    }
    else if (params.blend == 2)
    {
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
        f->glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);  // 50/50
    }
    else
    {
        f->glDisable(GL_BLEND);
    }

    m_timescopeShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, m_specTex);
    m_timescopeShader->setUniformValue("uSpec", 0);
    m_timescopeShader->setUniformValue("uColor", colorToVec(params.color));
    m_timescopeShader->setUniformValue("uBands",
                                       static_cast<float>(std::clamp(params.bands, 1, 576)));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_timescopeShader->release();

    f->glDisable(GL_SCISSOR_TEST);
    f->glDisable(GL_BLEND);
    pair.current()->release();
    bindActive();
}

namespace {
// Draw a batch of points additively via the scope renderer (Dot renderers).
void drawDots(lumi::render::ScopeRenderer& renderer,
              const std::vector<lumi::modules::SuperscopePoint>& points, float dotSize)
{
    if (points.empty() || !renderer.ready()) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = lumi::modules::SuperscopeRenderMode::Dots;
    rp.dotSize = dotSize;
    rp.glowEnabled = false;
    renderer.draw(points, rp);
    f->glDisable(GL_BLEND);
}

// 5-stop colour gradient (Dot Plane/Fountain), t in [0,1] -> rgb.
QVector3D grad5(const uint32_t (&colors)[5], float t)
{
    t = std::clamp(t, 0.0f, 1.0f) * 4.0f;
    const int i = std::min(3, static_cast<int>(t));
    const float f = t - static_cast<float>(i);
    auto rgb = [](uint32_t c) {
        return QVector3D(static_cast<float>((c >> 16) & 0xFF) / 255.0f,
                         static_cast<float>((c >> 8) & 0xFF) / 255.0f,
                         static_cast<float>(c & 0xFF) / 255.0f);
    };
    return rgb(colors[i]) * (1.0f - f) + rgb(colors[i + 1]) * f;
}
}  // namespace

void MultiEffectVisualizer::runDotGrid(const ChainNode& node, const DotGridParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const int spacing = std::max(2, params.spacing);
    const int nc = static_cast<int>(params.colors.size());
    if (nc == 0) return;

    // Cycle the colour table over time (r_dotgrid color_pos).
    rt.dotColorPos += 1.0f;
    const float span = static_cast<float>(nc) * 64.0f;
    if (rt.dotColorPos >= span) rt.dotColorPos -= span;
    const int cp = static_cast<int>(rt.dotColorPos);
    const int p0 = cp / 64;
    const float cf = static_cast<float>(cp % 64) / 64.0f;
    auto rgb = [](uint32_t c) {
        return QVector3D(static_cast<float>((c >> 16) & 0xFF) / 255.0f,
                         static_cast<float>((c >> 8) & 0xFF) / 255.0f,
                         static_cast<float>(c & 0xFF) / 255.0f);
    };
    const QVector3D col =
        rgb(params.colors[static_cast<size_t>(p0)]) * (1.0f - cf) +
        rgb(params.colors[static_cast<size_t>((p0 + 1) % nc)]) * cf;

    rt.dotOffX += static_cast<float>(params.xMove) / 256.0f;
    rt.dotOffY += static_cast<float>(params.yMove) / 256.0f;
    const float ox = std::fmod(rt.dotOffX, static_cast<float>(spacing));
    const float oy = std::fmod(rt.dotOffY, static_cast<float>(spacing));

    std::vector<lumi::modules::SuperscopePoint> pts;
    for (float py = oy; py < m_surfaceHeight; py += spacing)
    {
        for (float px = ox; px < m_surfaceWidth; px += spacing)
        {
            lumi::modules::SuperscopePoint p;
            p.x = px / static_cast<float>(m_surfaceWidth) * 2.0f - 1.0f;
            p.y = py / static_cast<float>(m_surfaceHeight) * 2.0f - 1.0f;
            p.r = col.x(); p.g = col.y(); p.b = col.z(); p.a = 1.0f;
            pts.push_back(p);
        }
    }
    drawDots(m_scopeRenderer, pts, 2.0f);
}

void MultiEffectVisualizer::runDotPlane(const ChainNode& node, const DotPlaneParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    rt.dotRot += static_cast<float>(params.rotVel) * 0.002f;

    constexpr int kN = 28;
    const std::vector<float> spec = getSpectrum();
    const int sl = spec.empty() ? 1 : static_cast<int>(spec.size());
    const float ca = std::cos(rt.dotRot);
    const float sa = std::sin(rt.dotRot);
    const float tilt = static_cast<float>(params.angle) / 90.0f;  // viewing tilt

    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(kN * kN);
    for (int fz = 0; fz < kN; ++fz)
    {
        const float z = static_cast<float>(fz) / (kN - 1) * 2.0f - 1.0f;
        for (int fx = 0; fx < kN; ++fx)
        {
            const float x = static_cast<float>(fx) / (kN - 1) * 2.0f - 1.0f;
            const float hgt = spec[static_cast<size_t>(fx * sl / kN)] * 0.8f;
            const float rx = x * ca - z * sa;   // rotate around the vertical axis
            const float rz = x * sa + z * ca;
            const float wz = rz + 2.5f;          // push away from the camera
            if (wz <= 0.1f) continue;
            const float sx = rx / wz * 1.6f;
            const float sy = (hgt - 0.4f - rz * tilt) / wz * 1.6f;
            if (sx <= -1.0f || sx >= 1.0f || sy <= -1.0f || sy >= 1.0f) continue;
            const QVector3D c = grad5(params.colors, hgt / 0.8f);
            lumi::modules::SuperscopePoint p;
            p.x = sx; p.y = sy; p.r = c.x(); p.g = c.y(); p.b = c.z(); p.a = 1.0f;
            pts.push_back(p);
        }
    }
    drawDots(m_scopeRenderer, pts, 2.0f);
}

void MultiEffectVisualizer::runDotFountain(const ChainNode& node,
                                           const DotFountainParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    constexpr int kCount = 400;
    auto frand = [this] { return static_cast<float>(nextRandom() & 0xffff) / 65535.0f; };
    if (static_cast<int>(rt.fountain.size()) != kCount)
        rt.fountain.assign(kCount, FountainP{});

    rt.dotRot += static_cast<float>(params.rotVel) * 0.002f;
    const float level = 0.3f + m_audioLevel * 0.9f;  // emission speed from audio
    const float tilt = static_cast<float>(params.angle) / 90.0f;

    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(kCount);
    for (FountainP& fp : rt.fountain)
    {
        if (fp.vh == 0.0f && fp.h == 0.0f)  // (re)spawn at the nozzle
        {
            fp.a = frand() * 6.2831853f;
            fp.r = 0.0f;
            fp.h = 0.0f;
            fp.vh = 0.02f + frand() * 0.03f * level;
        }
        fp.h += fp.vh;
        fp.vh -= 0.0016f;   // gravity
        fp.r += 0.012f;     // spread outward
        if (fp.h < 0.0f) { fp.vh = 0.0f; fp.h = 0.0f; continue; }

        const float wx = fp.r * std::cos(fp.a + rt.dotRot);
        const float wz = fp.r * std::sin(fp.a + rt.dotRot) + 2.0f;
        if (wz <= 0.1f) continue;
        const float sx = wx / wz * 1.6f;
        const float sy = (fp.h - 0.3f - wz * tilt * 0.2f) / wz * 1.6f;
        if (sx <= -1.0f || sx >= 1.0f || sy <= -1.0f || sy >= 1.0f) continue;
        const QVector3D c = grad5(params.colors, std::clamp(fp.h * 1.5f, 0.0f, 1.0f));
        lumi::modules::SuperscopePoint p;
        p.x = sx; p.y = sy; p.r = c.x(); p.g = c.y(); p.b = c.z(); p.a = 1.0f;
        pts.push_back(p);
    }
    drawDots(m_scopeRenderer, pts, 2.0f);
}

void MultiEffectVisualizer::runChannelShift(const ChainNode& node,
                                            const ChannelShiftParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    int mode = std::clamp(params.mode, 0, 5);
    if (params.onBeat)
    {
        if (m_frameBeat || rt.apeChanMode < 0)
            rt.apeChanMode = static_cast<int>(nextRandom() % 6u);
        mode = rt.apeChanMode;
    }
    m_apeShader->bind();
    m_apeShader->setUniformValue("uType", 0);
    m_apeShader->setUniformValue("uMode", mode);
    m_apeShader->release();
    transformPass(*m_apeShader);
}

void MultiEffectVisualizer::runColorReduction(const ColorReductionParams& params)
{
    const int levels = std::clamp(params.levels, 1, 8);
    m_apeShader->bind();
    m_apeShader->setUniformValue("uType", 1);
    m_apeShader->setUniformValue("uLevels", static_cast<float>(1 << levels));
    m_apeShader->release();
    transformPass(*m_apeShader);
}

void MultiEffectVisualizer::runMultiplier(const MultiplierParams& params)
{
    m_apeShader->bind();
    m_apeShader->setUniformValue("uType", 2);
    m_apeShader->setUniformValue("uMode", std::clamp(params.mode, 0, 7));
    m_apeShader->release();
    transformPass(*m_apeShader);
}

void MultiEffectVisualizer::runVideoDelay(const ChainNode& node,
                                          const VideoDelayParams& params)
{
    if (!QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* extra = QOpenGLContext::currentContext()->extraFunctions();

    // ~30 frames/beat is a rough conversion (no per-node BPM here); capped for VRAM.
    const int delay =
        std::clamp(params.useBeats ? params.delay * 30 : params.delay, 1, 128);
    const int size = delay + 1;

    if (static_cast<int>(rt.delayRing.size()) != size || rt.delayW != m_surfaceWidth ||
        rt.delayH != m_surfaceHeight)
    {
        rt.delayRing.clear();
        auto* f = QOpenGLContext::currentContext()->functions();
        for (int i = 0; i < size; ++i)
        {
            auto fbo = std::make_unique<QOpenGLFramebufferObject>(m_surfaceWidth,
                                                                  m_surfaceHeight);
            fbo->bind();
            f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            f->glClear(GL_COLOR_BUFFER_BIT);
            fbo->release();
            rt.delayRing.push_back(std::move(fbo));
        }
        rt.delayHead = 0;
        rt.delayFilled = 0;
        rt.delayW = m_surfaceWidth;
        rt.delayH = m_surfaceHeight;
    }

    QOpenGLFramebufferObject* cur = active().current();
    auto blit = [&](GLuint from, GLuint to) {
        extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, from);
        extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to);
        extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0,
                                 m_surfaceWidth, m_surfaceHeight, GL_COLOR_BUFFER_BIT,
                                 GL_NEAREST);
    };

    // Store this frame, then replace the working image with the delayed one.
    blit(cur->handle(), rt.delayRing[static_cast<size_t>(rt.delayHead)]->handle());
    const int srcIdx = rt.delayFilled >= delay
                           ? (rt.delayHead - delay + size) % size
                           : rt.delayHead;  // not enough history yet -> current
    blit(rt.delayRing[static_cast<size_t>(srcIdx)]->handle(), cur->handle());

    rt.delayHead = (rt.delayHead + 1) % size;
    if (rt.delayFilled < size) ++rt.delayFilled;
    bindActive();
}

void MultiEffectVisualizer::runMultiDelay(const MultiDelayParams& params)
{
    if (params.mode == 0) return;  // inactive
    if (!QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()) return;
    const int b = std::clamp(params.buffer, 0, 5);
    auto* extra = QOpenGLContext::currentContext()->extraFunctions();

    // Shared rings are dropped on a surface-size change.
    if (m_mdW != m_surfaceWidth || m_mdH != m_surfaceHeight)
    {
        for (auto& ring : m_mdRing) ring.clear();
        for (int& head : m_mdHead) head = 0;
        m_mdW = m_surfaceWidth;
        m_mdH = m_surfaceHeight;
    }

    const int delay =
        std::clamp(params.useBeats ? params.delay * 30 : params.delay, 1, 128);
    auto& ring = m_mdRing[static_cast<size_t>(b)];
    int& head = m_mdHead[static_cast<size_t>(b)];
    QOpenGLFramebufferObject* cur = active().current();
    auto blit = [&](GLuint from, GLuint to) {
        extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, from);
        extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to);
        extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0,
                                 m_surfaceWidth, m_surfaceHeight, GL_COLOR_BUFFER_BIT,
                                 GL_NEAREST);
    };

    if (params.mode == 1)  // input: (re)size the ring, store this frame
    {
        const int size = delay + 1;
        if (static_cast<int>(ring.size()) != size)
        {
            ring.clear();
            auto* f = QOpenGLContext::currentContext()->functions();
            for (int i = 0; i < size; ++i)
            {
                auto fbo = std::make_unique<QOpenGLFramebufferObject>(m_surfaceWidth,
                                                                      m_surfaceHeight);
                fbo->bind();
                f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                f->glClear(GL_COLOR_BUFFER_BIT);
                fbo->release();
                ring.push_back(std::move(fbo));
            }
            head = 0;
        }
        blit(cur->handle(), ring[static_cast<size_t>(head)]->handle());
        head = (head + 1) % size;
    }
    else if (params.mode == 2 && !ring.empty())  // output: read the delayed frame
    {
        const int size = static_cast<int>(ring.size());
        const int d = std::min(delay, size - 1);
        const int srcIdx = (head - d - 1 + size) % size;  // head points past newest
        blit(ring[static_cast<size_t>(srcIdx)]->handle(), cur->handle());
    }
    bindActive();
}

void MultiEffectVisualizer::runFractal2D(const ChainNode& node,
                                         const Fractal2DParams& params)
{
    if (m_fractal2DShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();

    // --- Gradient palette LUT (256x1), rebuilt only when the preset changes.
    if (rt.fracLut == 0 || rt.fracLutSnapshot != params.gradientPreset)
    {
        lumi::modules::ColorGradientModule grad;
        grad.loadPreset(params.gradientPreset);
        std::array<unsigned char, 768> px{};
        for (int i = 0; i < 256; ++i)
        {
            const lumi::modules::Color4f col =
                grad.sample(static_cast<float>(i) / 255.0f);
            px[static_cast<size_t>(i) * 3 + 0] =
                static_cast<unsigned char>(std::clamp(col[0], 0.0f, 1.0f) * 255.0f);
            px[static_cast<size_t>(i) * 3 + 1] =
                static_cast<unsigned char>(std::clamp(col[1], 0.0f, 1.0f) * 255.0f);
            px[static_cast<size_t>(i) * 3 + 2] =
                static_cast<unsigned char>(std::clamp(col[2], 0.0f, 1.0f) * 255.0f);
        }
        if (rt.fracLut == 0)
        {
            f->glGenTextures(1, &rt.fracLut);
            f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
            f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
        }
        f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                        px.data());
        rt.fracLutSnapshot = params.gradientPreset;
    }

    // --- View parameters: EEL-driven when any slot has code, else the raw params
    //     (so the editor can tweak a static fractal directly). The EEL reads audio
    //     (bass/mid/treble/vol/beat/time) and writes cx,cy,zoom,rot,jx,jy,power.
    float cx = params.centerX, cy = params.centerY, zoom = params.zoom;
    float rot = params.rotation, jx = params.juliaX, jy = params.juliaY;
    float power = params.power;

    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        // Snapshot includes the seed values: editing a seed re-seeds the engine.
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(cx) + ',' +
                                 std::to_string(cy) + ',' + std::to_string(zoom) + ',' +
                                 std::to_string(rot) + ',' + std::to_string(jx) + ',' +
                                 std::to_string(jy) + ',' + std::to_string(power);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("fractal2d", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            auto& e = rt.fracHost->engine();
            e.setNumber("cx", cx);      e.setNumber("cy", cy);
            e.setNumber("zoom", zoom);  e.setNumber("rot", rot);
            e.setNumber("jx", jx);      e.setNumber("jy", jy);
            e.setNumber("power", power);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }

        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);

        cx = static_cast<float>(e.number("cx"));
        cy = static_cast<float>(e.number("cy"));
        zoom = static_cast<float>(e.number("zoom"));
        rot = static_cast<float>(e.number("rot"));
        jx = static_cast<float>(e.number("jx"));
        jy = static_cast<float>(e.number("jy"));
        power = static_cast<float>(e.number("power"));
    }
    else
    {
        rt.fracHost.reset();  // reclaim the engine once the slots are cleared
        rt.fracCompiled.clear();
    }

    if (zoom < 1e-6f) zoom = 1e-6f;
    rt.fracColorPhase += params.colorCycle * m_deltaTime;

    const float aspect = m_surfaceHeight > 0
                             ? static_cast<float>(m_surfaceWidth) /
                                   static_cast<float>(m_surfaceHeight)
                             : 1.0f;

    m_fractal2DShader->bind();
    m_fractal2DShader->setUniformValue("uCenter", QVector2D(cx, cy));
    m_fractal2DShader->setUniformValue("uZoom", zoom);
    m_fractal2DShader->setUniformValue("uRot", rot);
    m_fractal2DShader->setUniformValue("uAspect", aspect);
    m_fractal2DShader->setUniformValue("uType", std::clamp(params.type, 0, 8));
    m_fractal2DShader->setUniformValue("uMaxIter", std::clamp(params.maxIter, 1, 2048));
    m_fractal2DShader->setUniformValue("uJulia", QVector2D(jx, jy));
    m_fractal2DShader->setUniformValue("uPower", std::clamp(power, 1.0f, 16.0f));
    m_fractal2DShader->setUniformValue("uEscapeR", std::max(params.escapeR, 1.0f));
    m_fractal2DShader->setUniformValue("uSmooth", params.smooth);
    m_fractal2DShader->setUniformValue("uColorScale", params.colorScale);
    m_fractal2DShader->setUniformValue("uColorPhase", rt.fracColorPhase);
    m_fractal2DShader->setUniformValue("uInside", colorToVec(params.insideColor));
    m_fractal2DShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_fractal2DShader->setUniformValue("uLut", 1);
    m_fractal2DShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_fractal2DShader);
}

void MultiEffectVisualizer::runDomainWarp(const ChainNode& node,
                                          const DomainWarpParams& params)
{
    if (m_domainWarpShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();

    // --- Gradient palette LUT (256x1), rebuilt only when the preset changes.
    if (rt.fracLut == 0 || rt.fracLutSnapshot != params.gradientPreset)
    {
        lumi::modules::ColorGradientModule grad;
        grad.loadPreset(params.gradientPreset);
        std::array<unsigned char, 768> px{};
        for (int i = 0; i < 256; ++i)
        {
            const lumi::modules::Color4f col =
                grad.sample(static_cast<float>(i) / 255.0f);
            px[static_cast<size_t>(i) * 3 + 0] =
                static_cast<unsigned char>(std::clamp(col[0], 0.0f, 1.0f) * 255.0f);
            px[static_cast<size_t>(i) * 3 + 1] =
                static_cast<unsigned char>(std::clamp(col[1], 0.0f, 1.0f) * 255.0f);
            px[static_cast<size_t>(i) * 3 + 2] =
                static_cast<unsigned char>(std::clamp(col[2], 0.0f, 1.0f) * 255.0f);
        }
        if (rt.fracLut == 0)
        {
            f->glGenTextures(1, &rt.fracLut);
            f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
            f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
        }
        f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                        px.data());
        rt.fracLutSnapshot = params.gradientPreset;
    }

    // --- Field parameters: EEL-driven when any slot has code, else raw params.
    //     EEL reads audio (bass/mid/treble/vol/beat/time), writes scale,warp,speed,ox,oy.
    float scale = params.scale, warp = params.warp, speed = params.speed;
    float ox = params.offsetX, oy = params.offsetY;

    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(scale) + ',' +
                                 std::to_string(warp) + ',' + std::to_string(speed) + ',' +
                                 std::to_string(ox) + ',' + std::to_string(oy);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("domainwarp", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            auto& e = rt.fracHost->engine();
            e.setNumber("scale", scale);  e.setNumber("warp", warp);
            e.setNumber("speed", speed);  e.setNumber("ox", ox);
            e.setNumber("oy", oy);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }

        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);

        scale = static_cast<float>(e.number("scale"));
        warp = static_cast<float>(e.number("warp"));
        speed = static_cast<float>(e.number("speed"));
        ox = static_cast<float>(e.number("ox"));
        oy = static_cast<float>(e.number("oy"));
    }
    else
    {
        rt.fracHost.reset();
        rt.fracCompiled.clear();
    }

    rt.fracTime += speed * m_deltaTime;
    rt.fracColorPhase += params.colorCycle * m_deltaTime;

    const float aspect = m_surfaceHeight > 0
                             ? static_cast<float>(m_surfaceWidth) /
                                   static_cast<float>(m_surfaceHeight)
                             : 1.0f;

    m_domainWarpShader->bind();
    m_domainWarpShader->setUniformValue("uAspect", aspect);
    m_domainWarpShader->setUniformValue("uOctaves", std::clamp(params.octaves, 1, 10));
    m_domainWarpShader->setUniformValue("uLac", params.lacunarity);
    m_domainWarpShader->setUniformValue("uGain", params.gain);
    m_domainWarpShader->setUniformValue("uScale", scale);
    m_domainWarpShader->setUniformValue("uWarp", warp);
    m_domainWarpShader->setUniformValue("uWarpScale", params.warpScale);
    m_domainWarpShader->setUniformValue("uOffset", QVector2D(ox, oy));
    m_domainWarpShader->setUniformValue("uTime", rt.fracTime);
    m_domainWarpShader->setUniformValue("uColorScale", params.colorScale);
    m_domainWarpShader->setUniformValue("uColorPhase", rt.fracColorPhase);
    m_domainWarpShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_domainWarpShader->setUniformValue("uLut", 1);
    m_domainWarpShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_domainWarpShader);
}

namespace {
/// Average the spectrum into bass/mid/treble thirds (Batch H audio binding).
void computeAudioBands(const std::vector<float>& spec, float& bass, float& mid,
                       float& treble)
{
    bass = mid = treble = 0.0f;
    if (spec.empty()) return;
    const size_t n = spec.size(), t1 = n / 3, t2 = 2 * n / 3;
    auto avg = [&](size_t lo, size_t hi) {
        float s = 0.0f;
        for (size_t i = lo; i < hi; ++i) s += spec[i];
        return hi > lo ? s / static_cast<float>(hi - lo) : 0.0f;
    };
    bass = avg(0, t1);
    mid = avg(t1, t2);
    treble = avg(t2, n);
}

/// Bake a gradient preset into a 256-entry CPU colour table (point-cloud colouring).
void buildCpuGradientLut(const std::string& preset, std::array<QVector3D, 256>& out)
{
    lumi::modules::ColorGradientModule grad;
    grad.loadPreset(preset);
    for (int i = 0; i < 256; ++i)
    {
        const lumi::modules::Color4f c = grad.sample(static_cast<float>(i) / 255.0f);
        out[static_cast<size_t>(i)] = QVector3D(c[0], c[1], c[2]);
    }
}
}  // namespace

void MultiEffectVisualizer::ensureFractalLut(LeafRuntime& rt, const std::string& preset)
{
    if (rt.fracLut != 0 && rt.fracLutSnapshot == preset) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    lumi::modules::ColorGradientModule grad;
    grad.loadPreset(preset);
    std::array<unsigned char, 768> px{};
    for (int i = 0; i < 256; ++i)
    {
        const lumi::modules::Color4f col = grad.sample(static_cast<float>(i) / 255.0f);
        px[static_cast<size_t>(i) * 3 + 0] =
            static_cast<unsigned char>(std::clamp(col[0], 0.0f, 1.0f) * 255.0f);
        px[static_cast<size_t>(i) * 3 + 1] =
            static_cast<unsigned char>(std::clamp(col[1], 0.0f, 1.0f) * 255.0f);
        px[static_cast<size_t>(i) * 3 + 2] =
            static_cast<unsigned char>(std::clamp(col[2], 0.0f, 1.0f) * 255.0f);
    }
    if (rt.fracLut == 0)
    {
        f->glGenTextures(1, &rt.fracLut);
        f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
    }
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                    px.data());
    rt.fracLutSnapshot = preset;
}

void MultiEffectVisualizer::buildVisData()
{
    // Per-channel data (getSpectrumChannel/getWaveformChannel fall back to the
    // mono copy when no stereo has been fed — so mono streams still fill L=R).
    const std::vector<float> specL = getSpectrumChannel(0);
    const std::vector<float> specR = getSpectrumChannel(1);
    const std::vector<float> waveL = getWaveformChannel(0);
    const std::vector<float> waveR = getWaveformChannel(1);
    // AVS pipes every Winamp spectrum byte through g_logtab (ref main.cpp:242-249):
    // a = log(x*60/255+1)/log(60). Linear FFT magnitudes are far too small in the
    // upper bands presets sample via getspec(0.5..0.8) — without this curve their
    // beat-driven motion (ti=getspec(...)) stalls near zero. kSpecGain approximates
    // the Winamp byte scale before the curve (visual calibration point;
    // 12 was "etwas zu schnell" in the Session-38 sight test -> 8).
    constexpr float kSpecGain = 8.0f;
    auto specByte = [](const std::vector<float>& v, int i) -> unsigned char {
        if (v.empty()) return 0;
        const float s = v[static_cast<size_t>(i) * v.size() / 576];
        const float lin = std::clamp(s * kSpecGain, 0.0f, 1.0f);
        const float logv = std::log(lin * 60.0f + 1.0f) / std::log(60.0f);
        return static_cast<unsigned char>(std::clamp(logv, 0.0f, 1.0f) * 255.0f);
    };
    // Waveform -> signed byte (two's complement); getvis decodes (b^128)-128.
    auto waveByte = [](const std::vector<float>& v, int i) -> unsigned char {
        if (v.empty()) return 0;  // byte 0 -> getvis (0^128)-128 = 0 (silence)
        const float w = v[static_cast<size_t>(i) * v.size() / 576];
        const int sw = std::clamp(static_cast<int>(w * 127.0f), -128, 127);
        return static_cast<unsigned char>(sw & 0xFF);
    };
    for (int i = 0; i < 576; ++i)
    {
        m_visdata[static_cast<size_t>(i)] = specByte(specL, i);          // spectrum L
        m_visdata[static_cast<size_t>(i) + 576] = specByte(specR, i);    // spectrum R
        m_visdata[static_cast<size_t>(i) + 1152] = waveByte(waveL, i);   // waveform L
        m_visdata[static_cast<size_t>(i) + 1728] = waveByte(waveR, i);   // waveform R
    }
}

void MultiEffectVisualizer::feedAudio(lumi::scripting::LuaScriptEngine& engine)
{
    engine.setVisData(m_visdata.data());
    engine.setScriptTime(m_time);
    float bass, mid, treble;
    computeAudioBands(getSpectrum(), bass, mid, treble);
    engine.setNumber("bass", bass);
    engine.setNumber("mid", mid);
    engine.setNumber("treb", treble);
    engine.setNumber("treble", treble);  // alias (MilkDrop uses treb)
    engine.setNumber("vol", m_audioLevel);
    engine.setNumber("beat", m_frameBeat ? 1.0 : 0.0);
    engine.setNumber("time", m_time);
}

void MultiEffectVisualizer::runFractal3D(const ChainNode& node,
                                         const Fractal3DParams& params)
{
    if (m_fractal3DShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();
    ensureFractalLut(rt, params.gradientPreset);

    float yaw = params.yaw, pitch = params.pitch, dist = params.dist;
    float power = params.power, scale = params.scale, fold = params.fold;
    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(yaw) + ',' +
                                 std::to_string(pitch) + ',' + std::to_string(dist) + ',' +
                                 std::to_string(power) + ',' + std::to_string(scale) + ',' +
                                 std::to_string(fold);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("fractal3d", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            auto& e = rt.fracHost->engine();
            e.setNumber("yaw", yaw);      e.setNumber("pitch", pitch);
            e.setNumber("dist", dist);    e.setNumber("power", power);
            e.setNumber("scale", scale);  e.setNumber("fold", fold);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }
        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);
        yaw = static_cast<float>(e.number("yaw"));
        pitch = static_cast<float>(e.number("pitch"));
        dist = static_cast<float>(e.number("dist"));
        power = static_cast<float>(e.number("power"));
        scale = static_cast<float>(e.number("scale"));
        fold = static_cast<float>(e.number("fold"));
    }
    else
    {
        rt.fracHost.reset();
        rt.fracCompiled.clear();
    }
    dist = std::max(dist, 0.1f);
    rt.fracColorPhase += params.colorCycle * m_deltaTime;

    const float cp = std::cos(pitch);
    const QVector3D cam(dist * cp * std::cos(yaw), dist * std::sin(pitch),
                        dist * cp * std::sin(yaw));
    const float lp = std::cos(params.lightPitch);
    QVector3D light(lp * std::cos(params.lightYaw), std::sin(params.lightPitch),
                    lp * std::sin(params.lightYaw));
    light.normalize();
    const float aspect = m_surfaceHeight > 0
                             ? static_cast<float>(m_surfaceWidth) /
                                   static_cast<float>(m_surfaceHeight)
                             : 1.0f;

    m_fractal3DShader->bind();
    m_fractal3DShader->setUniformValue("uAspect", aspect);
    m_fractal3DShader->setUniformValue("uType", std::clamp(params.type, 0, 4));
    m_fractal3DShader->setUniformValue("uCam", cam);
    m_fractal3DShader->setUniformValue("uFov", params.fov);
    m_fractal3DShader->setUniformValue("uPower", std::clamp(power, 1.0f, 16.0f));
    m_fractal3DShader->setUniformValue("uScale", scale);
    m_fractal3DShader->setUniformValue("uFold", fold);
    m_fractal3DShader->setUniformValue("uMaxSteps", std::clamp(params.maxSteps, 8, 512));
    m_fractal3DShader->setUniformValue("uMaxIter", std::clamp(params.maxIter, 1, 64));
    m_fractal3DShader->setUniformValue(
        "uJulia", QVector4D(params.juliaX, params.juliaY, params.juliaZ, params.juliaW));
    m_fractal3DShader->setUniformValue("uLight", light);
    m_fractal3DShader->setUniformValue("uAmbient", params.ambient);
    m_fractal3DShader->setUniformValue("uAO", params.ao);
    m_fractal3DShader->setUniformValue("uColorScale", params.colorScale);
    m_fractal3DShader->setUniformValue("uColorPhase", rt.fracColorPhase);
    m_fractal3DShader->setUniformValue("uBg", colorToVec(params.background));
    m_fractal3DShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_fractal3DShader->setUniformValue("uLut", 1);
    m_fractal3DShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_fractal3DShader);
}

void MultiEffectVisualizer::runLyapunov(const ChainNode& node,
                                        const LyapunovParams& params)
{
    if (m_lyapunovShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();
    ensureFractalLut(rt, params.gradientPreset);

    float aMin = params.aMin, aMax = params.aMax, bMin = params.bMin, bMax = params.bMax;
    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(aMin) + ',' +
                                 std::to_string(aMax) + ',' + std::to_string(bMin) + ',' +
                                 std::to_string(bMax);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("lyapunov", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            auto& e = rt.fracHost->engine();
            e.setNumber("amin", aMin); e.setNumber("amax", aMax);
            e.setNumber("bmin", bMin); e.setNumber("bmax", bMax);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }
        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);
        aMin = static_cast<float>(e.number("amin"));
        aMax = static_cast<float>(e.number("amax"));
        bMin = static_cast<float>(e.number("bmin"));
        bMax = static_cast<float>(e.number("bmax"));
    }
    else
    {
        rt.fracHost.reset();
        rt.fracCompiled.clear();
    }
    rt.fracColorPhase += params.colorCycle * m_deltaTime;

    // Sequence string -> int array (A/a -> 0, B/b -> 1), max 64.
    std::array<int, 64> seq{};
    int len = 0;
    for (char ch : params.sequence)
    {
        if (len >= 64) break;
        if (ch == 'A' || ch == 'a') seq[static_cast<size_t>(len++)] = 0;
        else if (ch == 'B' || ch == 'b') seq[static_cast<size_t>(len++)] = 1;
    }
    if (len == 0) { seq[0] = 0; seq[1] = 1; len = 2; }

    m_lyapunovShader->bind();
    m_lyapunovShader->setUniformValue("uView", QVector4D(aMin, aMax, bMin, bMax));
    m_lyapunovShader->setUniformValueArray("uSeq", seq.data(), len);
    m_lyapunovShader->setUniformValue("uSeqLen", len);
    m_lyapunovShader->setUniformValue("uWarmup", std::clamp(params.warmup, 0, 2000));
    m_lyapunovShader->setUniformValue("uIter", std::clamp(params.iterations, 1, 4000));
    m_lyapunovShader->setUniformValue("uNeg", colorToVec(params.negColor));
    m_lyapunovShader->setUniformValue("uColorScale", params.colorScale);
    m_lyapunovShader->setUniformValue("uColorPhase", rt.fracColorPhase);
    m_lyapunovShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_lyapunovShader->setUniformValue("uLut", 1);
    m_lyapunovShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_lyapunovShader);
}

void MultiEffectVisualizer::runKleinian(const ChainNode& node,
                                        const KleinianParams& params)
{
    if (m_kleinianShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();
    ensureFractalLut(rt, params.gradientPreset);

    float morph = params.morph, zoom = params.zoom, rot = params.rotation;
    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(morph) + ',' +
                                 std::to_string(zoom) + ',' + std::to_string(rot);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("kleinian", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            auto& e = rt.fracHost->engine();
            e.setNumber("morph", morph); e.setNumber("zoom", zoom); e.setNumber("rot", rot);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }
        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);
        morph = static_cast<float>(e.number("morph"));
        zoom = static_cast<float>(e.number("zoom"));
        rot = static_cast<float>(e.number("rot"));
    }
    else
    {
        rt.fracHost.reset();
        rt.fracCompiled.clear();
    }
    if (zoom < 1e-3f) zoom = 1e-3f;
    rt.fracColorPhase += params.colorCycle * m_deltaTime;
    const float aspect = m_surfaceHeight > 0
                             ? static_cast<float>(m_surfaceWidth) /
                                   static_cast<float>(m_surfaceHeight)
                             : 1.0f;

    m_kleinianShader->bind();
    m_kleinianShader->setUniformValue("uAspect", aspect);
    m_kleinianShader->setUniformValue("uZoom", zoom);
    m_kleinianShader->setUniformValue("uRot", rot);
    m_kleinianShader->setUniformValue("uP", std::clamp(params.p, 3, 20));
    m_kleinianShader->setUniformValue("uQ", std::clamp(params.q, 3, 20));
    m_kleinianShader->setUniformValue("uIters", std::clamp(params.iterations, 1, 200));
    m_kleinianShader->setUniformValue("uMorph", morph);
    m_kleinianShader->setUniformValue("uColorScale", params.colorScale);
    m_kleinianShader->setUniformValue("uColorPhase", rt.fracColorPhase);
    m_kleinianShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_kleinianShader->setUniformValue("uLut", 1);
    m_kleinianShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_kleinianShader);
}

void MultiEffectVisualizer::runFractalZoomer(const ChainNode& node,
                                             const FractalZoomerParams& params)
{
    if (m_fractal2DShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();
    ensureFractalLut(rt, params.gradientPreset);

    float cx = params.centerX, cy = params.centerY;
    float zoomSpeed = params.zoomSpeed, rotSpeed = params.rotationSpeed;
    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(cx) + ',' +
                                 std::to_string(cy) + ',' + std::to_string(zoomSpeed) + ',' +
                                 std::to_string(rotSpeed);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("fractalzoom", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            auto& e = rt.fracHost->engine();
            e.setNumber("cx", cx); e.setNumber("cy", cy);
            e.setNumber("zoomspeed", zoomSpeed); e.setNumber("rotspeed", rotSpeed);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }
        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);
        cx = static_cast<float>(e.number("cx"));
        cy = static_cast<float>(e.number("cy"));
        zoomSpeed = static_cast<float>(e.number("zoomspeed"));
        rotSpeed = static_cast<float>(e.number("rotspeed"));
    }
    else
    {
        rt.fracHost.reset();
        rt.fracCompiled.clear();
    }

    // Persistent zoom (fracTime) + rotation (rotoAngle) advance every frame.
    if (rt.fracTime <= 0.0f) rt.fracTime = 1.0f;
    rt.fracTime *= (zoomSpeed > 1e-4f ? zoomSpeed : 1.0f);
    if (rt.fracTime > 1e12f) rt.fracTime = 1.0f;  // loop the trip
    rt.rotoAngle += rotSpeed;
    rt.fracColorPhase += params.colorCycle * m_deltaTime;
    const float aspect = m_surfaceHeight > 0
                             ? static_cast<float>(m_surfaceWidth) /
                                   static_cast<float>(m_surfaceHeight)
                             : 1.0f;
    // type maps to the escape-time subset of the Fractal2D shader (0,1,2).
    const int t2type = std::clamp(params.type, 0, 2);

    m_fractal2DShader->bind();
    m_fractal2DShader->setUniformValue("uCenter", QVector2D(cx, cy));
    m_fractal2DShader->setUniformValue("uZoom", rt.fracTime);
    m_fractal2DShader->setUniformValue("uRot", rt.rotoAngle);
    m_fractal2DShader->setUniformValue("uAspect", aspect);
    m_fractal2DShader->setUniformValue("uType", t2type);
    m_fractal2DShader->setUniformValue("uMaxIter", std::clamp(params.maxIter, 1, 2048));
    m_fractal2DShader->setUniformValue("uJulia", QVector2D(params.juliaX, params.juliaY));
    m_fractal2DShader->setUniformValue("uPower", 2.0f);
    m_fractal2DShader->setUniformValue("uEscapeR", 4.0f);
    m_fractal2DShader->setUniformValue("uSmooth", true);
    m_fractal2DShader->setUniformValue("uColorScale", params.colorScale);
    m_fractal2DShader->setUniformValue("uColorPhase", rt.fracColorPhase);
    m_fractal2DShader->setUniformValue("uInside", colorToVec(params.insideColor));
    m_fractal2DShader->setUniformValue("uBlend", params.feedback > 0.01f ? 2 : 0);
    m_fractal2DShader->setUniformValue("uLut", 1);
    m_fractal2DShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_fractal2DShader);
}

void MultiEffectVisualizer::runStrangeAttractor(const ChainNode& node,
                                                const StrangeAttractorParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!m_scopeRenderer.ready()) return;

    float a = params.a, b = params.b, c = params.c, d = params.d, rotation = params.rotation;
    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(a) + ',' +
                                 std::to_string(b) + ',' + std::to_string(c) + ',' +
                                 std::to_string(d);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("attractor", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            auto& e = rt.fracHost->engine();
            e.setNumber("a", a); e.setNumber("b", b); e.setNumber("c", c);
            e.setNumber("d", d); e.setNumber("rotation", rotation);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }
        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);
        a = static_cast<float>(e.number("a")); b = static_cast<float>(e.number("b"));
        c = static_cast<float>(e.number("c")); d = static_cast<float>(e.number("d"));
        rotation = static_cast<float>(e.number("rotation"));
    }
    else
    {
        rt.fracHost.reset();
        rt.fracCompiled.clear();
    }

    if (!rt.saSeeded || !std::isfinite(rt.saX) || !std::isfinite(rt.saY) ||
        !std::isfinite(rt.saZ))
    {
        rt.saX = 0.1; rt.saY = 0.0; rt.saZ = 0.0;
        rt.saSeeded = true;
    }
    rt.saRot += params.rotationSpeed * m_deltaTime;
    const float rc = std::cos(rt.saRot + rotation), rs = std::sin(rt.saRot + rotation);

    std::array<QVector3D, 256> cpuLut{};
    if (params.useGradient) buildCpuGradientLut(params.gradientPreset, cpuLut);
    const QVector3D fixedCol = colorToVec(params.color);

    const int n = std::clamp(params.points, 1, 100000);
    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(static_cast<size_t>(n));
    double x = rt.saX, y = rt.saY, z = rt.saZ;
    for (int i = 0; i < n; ++i)
    {
        double px, py;
        if (params.type == 0)  // Lorenz (integrated)
        {
            const double sig = a * 7.142857, rho = b * 17.5, bet = c * 2.6667, dt = 0.005;
            const double dx = sig * (y - x), dy = x * (rho - z) - y, dz = x * y - bet * z;
            x += dx * dt; y += dy * dt; z += dz * dt;
            px = x * 0.033; py = (z - 25.0) * 0.033;
        }
        else if (params.type == 1)  // Clifford
        {
            const double nx = std::sin(a * y) + c * std::cos(a * x);
            const double ny = std::sin(b * x) + d * std::cos(b * y);
            x = nx; y = ny; px = x * 0.4; py = y * 0.4;
        }
        else if (params.type == 2)  // De Jong
        {
            const double nx = std::sin(a * y) - std::cos(b * x);
            const double ny = std::sin(c * x) - std::cos(d * y);
            x = nx; y = ny; px = x * 0.4; py = y * 0.4;
        }
        else  // Aizawa (integrated)
        {
            const double e = 0.25, ff = 0.1, dt = 0.01;
            const double dx = (z - b) * x - d * y;
            const double dy = d * x + (z - b) * y;
            const double dz = c + a * z - (z * z * z) / 3.0 -
                              (x * x + y * y) * (1.0 + e * z) + ff * z * (x * x * x);
            x += dx * dt; y += dy * dt; z += dz * dt;
            px = x * 0.6; py = y * 0.6;
        }
        // scale + view rotation
        const float sx = static_cast<float>(px) * params.scale;
        const float sy = static_cast<float>(py) * params.scale;
        lumi::modules::SuperscopePoint pt;
        pt.x = std::clamp(sx * rc - sy * rs, -1.0f, 1.0f);
        pt.y = std::clamp(sx * rs + sy * rc, -1.0f, 1.0f);
        const QVector3D col =
            params.useGradient
                ? cpuLut[static_cast<size_t>(i) * 255 / static_cast<size_t>(n)]
                : fixedCol;
        pt.r = col.x(); pt.g = col.y(); pt.b = col.z(); pt.a = 1.0f;
        pts.push_back(pt);
    }
    rt.saX = x; rt.saY = y; rt.saZ = z;

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    switch (params.blend)
    {
        case 0:  f->glBlendFunc(GL_ONE, GL_ZERO); break;
        case 2:  f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
        default: f->glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
    }
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = lumi::modules::SuperscopeRenderMode::Dots;
    rp.dotSize = std::clamp(params.dotSize, 1.0f, 32.0f);
    rp.glowEnabled = false;
    m_scopeRenderer.draw(pts, rp);
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runFlame(const ChainNode& node, const FlameParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!m_scopeRenderer.ready()) return;

    float rotation = params.rotation;
    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(rotation);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("flame", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            rt.fracHost->engine().setNumber("rotation", rotation);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }
        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);
        rotation = static_cast<float>(e.number("rotation"));
    }
    else
    {
        rt.fracHost.reset();
        rt.fracCompiled.clear();
    }

    if (!rt.saSeeded || !std::isfinite(rt.saX) || !std::isfinite(rt.saY))
    {
        rt.saX = 0.05; rt.saY = 0.05; rt.saSeeded = true;
    }
    rt.saRot += params.rotationSpeed * m_deltaTime;
    const float rc = std::cos(rt.saRot + rotation), rs = std::sin(rt.saRot + rotation);

    const int nfun = std::clamp(params.functions, 2, 4);
    // Affine attractors on a circle (contraction 0.5 towards each anchor).
    QVector2D anchor[4];
    for (int k = 0; k < nfun; ++k)
    {
        const float ang = 6.2831853f * static_cast<float>(k) / static_cast<float>(nfun);
        anchor[k] = QVector2D(std::cos(ang) * 0.8f, std::sin(ang) * 0.8f);
    }
    auto variation = [&](float vx, float vy) -> QVector2D {
        const float r2 = vx * vx + vy * vy + 1e-6f;
        switch (params.variation)
        {
            case 1: return {std::sin(vx), std::sin(vy)};                    // sinusoidal
            case 2: return {vx / r2, vy / r2};                             // spherical
            case 3: return {vx * std::sin(r2) - vy * std::cos(r2),
                            vx * std::cos(r2) + vy * std::sin(r2)};        // swirl
            case 4: { const float r = std::sqrt(r2);
                      return {(vx - vy) * (vx + vy) / r, 2.0f * vx * vy / r}; }  // horseshoe
            default: return {vx, vy};                                       // linear
        }
    };

    std::array<QVector3D, 256> cpuLut{};
    buildCpuGradientLut(params.gradientPreset, cpuLut);

    const int n = std::clamp(params.points, 1, 200000);
    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(static_cast<size_t>(n));
    float x = static_cast<float>(rt.saX), y = static_cast<float>(rt.saY);
    for (int i = 0; i < n; ++i)
    {
        const int k = static_cast<int>(nextRandom() % static_cast<uint32_t>(nfun));
        x = 0.5f * (x + anchor[k].x());
        y = 0.5f * (y + anchor[k].y());
        const QVector2D v = variation(x, y);
        const float fx = 0.5f * (x + v.x());
        const float fy = 0.5f * (y + v.y());
        if (!std::isfinite(fx) || !std::isfinite(fy)) { x = 0.05f; y = 0.05f; continue; }
        x = fx; y = fy;
        const float sx = x * params.scale, sy = y * params.scale;
        lumi::modules::SuperscopePoint pt;
        pt.x = std::clamp(sx * rc - sy * rs, -1.0f, 1.0f);
        pt.y = std::clamp(sx * rs + sy * rc, -1.0f, 1.0f);
        const QVector3D col = cpuLut[static_cast<size_t>(k) * 255 / static_cast<size_t>(nfun)];
        pt.r = col.x(); pt.g = col.y(); pt.b = col.z(); pt.a = 1.0f;
        pts.push_back(pt);
    }
    rt.saX = x; rt.saY = y;

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    switch (params.blend)
    {
        case 0:  f->glBlendFunc(GL_ONE, GL_ZERO); break;
        case 2:  f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
        default: f->glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
    }
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = lumi::modules::SuperscopeRenderMode::Dots;
    rp.dotSize = std::clamp(params.dotSize, 1.0f, 32.0f);
    rp.glowEnabled = false;
    m_scopeRenderer.draw(pts, rp);
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runReactionDiffusion(const ChainNode& node,
                                                 const ReactionDiffusionParams& params)
{
    if (m_rdShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();
    ensureFractalLut(rt, params.gradientPreset);

    float feed = params.feed, kill = params.kill;
    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(feed) + ',' +
                                 std::to_string(kill);
        if (rt.fracHost == nullptr || rt.fracCompiled != snap)
        {
            rt.fracHost = std::make_unique<ScriptSlotHost>("reactdiff", activeContext(),
                                                           ScriptSlotHost::Dialect::Avs);
            rt.fracHost->setSource(Slot::Init, params.initCode);
            rt.fracHost->setSource(Slot::Frame, params.frameCode);
            rt.fracHost->setSource(Slot::Beat, params.beatCode);
            rt.fracHost->compileAll();
            rt.fracHost->engine().setNumber("feed", feed);
            rt.fracHost->engine().setNumber("kill", kill);
            rt.fracHost->run(Slot::Init);
            rt.fracCompiled = snap;
        }
        feedAudio(rt.fracHost->engine());
        auto& e = rt.fracHost->engine();
        if (rt.fracHost->has(Slot::Frame)) rt.fracHost->run(Slot::Frame);
        if (m_frameBeat && rt.fracHost->has(Slot::Beat)) rt.fracHost->run(Slot::Beat);
        feed = static_cast<float>(e.number("feed"));
        kill = static_cast<float>(e.number("kill"));
    }
    else
    {
        rt.fracHost.reset();
        rt.fracCompiled.clear();
    }
    rt.fracColorPhase += params.colorCycle * m_deltaTime;

    // Simulate at a reduced resolution for speed (half the surface, min 64).
    const int simW = std::max(64, m_surfaceWidth / 2);
    const int simH = std::max(64, m_surfaceHeight / 2);
    if (!rt.rdBuf[0] || rt.rdW != simW || rt.rdH != simH)
    {
        QOpenGLFramebufferObjectFormat fmt;
        fmt.setInternalTextureFormat(GL_RGBA16F);
        for (auto& buf : rt.rdBuf)
            buf = std::make_unique<QOpenGLFramebufferObject>(simW, simH, fmt);
        rt.rdW = simW; rt.rdH = simH; rt.rdCur = 0; rt.rdSeeded = false;
    }
    if (!rt.rdBuf[0] || !rt.rdBuf[0]->isValid()) return;

    m_quadVao->bind();
    // Seed both buffers on first use.
    if (!rt.rdSeeded)
    {
        m_rdShader->bind();
        m_rdShader->setUniformValue("uMode", 0);
        for (auto& buf : rt.rdBuf)
        {
            buf->bind();
            f->glViewport(0, 0, simW, simH);
            f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            buf->release();
        }
        m_rdShader->release();
        rt.rdSeeded = true;
    }

    // Simulation steps (ping-pong).
    m_rdShader->bind();
    m_rdShader->setUniformValue("uMode", 1);
    m_rdShader->setUniformValue("uFeed", feed);
    m_rdShader->setUniformValue("uKill", kill);
    m_rdShader->setUniformValue("uDA", params.diffA);
    m_rdShader->setUniformValue("uDB", params.diffB);
    m_rdShader->setUniformValue(
        "uTexel", QVector2D(1.0f / static_cast<float>(simW), 1.0f / static_cast<float>(simH)));
    m_rdShader->setUniformValue("uState", 0);
    const int steps = std::clamp(params.stepsPerFrame, 1, 64);
    for (int s = 0; s < steps; ++s)
    {
        const bool seedNow = params.seedOnBeat && m_frameBeat && s == 0;
        m_rdShader->setUniformValue("uDoSeed", seedNow);
        if (seedNow)
        {
            const float sx = static_cast<float>(nextRandom() % 1000) / 1000.0f;
            const float sy = static_cast<float>(nextRandom() % 1000) / 1000.0f;
            m_rdShader->setUniformValue("uSeed", QVector2D(sx, sy));
            m_rdShader->setUniformValue("uSeedR", 0.04f);
        }
        QOpenGLFramebufferObject* src = rt.rdBuf[static_cast<size_t>(rt.rdCur)].get();
        QOpenGLFramebufferObject* dst = rt.rdBuf[static_cast<size_t>(1 - rt.rdCur)].get();
        dst->bind();
        f->glViewport(0, 0, simW, simH);
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, src->texture());
        f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        dst->release();
        rt.rdCur = 1 - rt.rdCur;
    }
    m_rdShader->release();

    // Show pass: B -> LUT, blended over the current frame (transformPass-style).
    SurfacePair& pair = active();
    pair.current()->release();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_rdShader->bind();
    m_rdShader->setUniformValue("uMode", 2);
    m_rdShader->setUniformValue("uColorScale", params.colorScale);
    m_rdShader->setUniformValue("uColorPhase", rt.fracColorPhase);
    m_rdShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_rdShader->setUniformValue("uState", 0);
    m_rdShader->setUniformValue("uTex", 1);
    m_rdShader->setUniformValue("uLut", 2);
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, rt.rdBuf[static_cast<size_t>(rt.rdCur)]->texture());
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    f->glActiveTexture(GL_TEXTURE2);
    f->glBindTexture(GL_TEXTURE_2D, rt.fracLut);
    f->glActiveTexture(GL_TEXTURE0);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_rdShader->release();
    pair.partner()->release();
    pair.swap();
    m_quadVao->release();
    bindActive();
}

// =============================================================================
// Passes
// =============================================================================

void MultiEffectVisualizer::transformPass(QOpenGLShaderProgram& shader)
{
    auto* f = QOpenGLContext::currentContext()->functions();
    SurfacePair& pair = active();

    pair.current()->release();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);

    shader.bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    shader.setUniformValue("uTex", 0);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    shader.release();

    pair.partner()->release();
    pair.swap();
    bindActive();  // chain continues on the (new) current buffer
}

unsigned int MultiEffectVisualizer::poolTexture(int n)
{
    QOpenGLFramebufferObject* pool =
        activePool().get(n, m_surfaceWidth, m_surfaceHeight, false);
    return pool != nullptr ? pool->texture() : 0u;
}

void MultiEffectVisualizer::blendPass(SurfacePair& dst, unsigned int srcTexture,
                                      BlendMode mode, int adjustAlpha,
                                      unsigned int bufferTexture, bool bufferInvert)
{
    if (mode == BlendMode::Ignore) return;
    // Buffer blend with no allocated source buffer: leave dst as-is (AVS breaks).
    if (mode == BlendMode::Buffer && bufferTexture == 0) return;

    auto* f = QOpenGLContext::currentContext()->functions();
    const int shaderMode =
        isBlendModeImplemented(mode) ? static_cast<int>(mode)
                                     : static_cast<int>(BlendMode::Replace);

    dst.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);

    m_blendShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, dst.current()->texture());
    m_blendShader->setUniformValue("uDst", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, srcTexture);
    m_blendShader->setUniformValue("uSrc", 1);
    // Unit 2 is only sampled for Buffer mode; bind srcTexture as a harmless
    // filler otherwise so no driver samples an unbound unit.
    f->glActiveTexture(GL_TEXTURE2);
    f->glBindTexture(GL_TEXTURE_2D, bufferTexture != 0 ? bufferTexture : srcTexture);
    m_blendShader->setUniformValue("uBuf", 2);
    f->glActiveTexture(GL_TEXTURE0);
    m_blendShader->setUniformValue("uMode", shaderMode);
    m_blendShader->setUniformValue("uAlpha",
                                   static_cast<float>(adjustAlpha) / 255.0f);
    m_blendShader->setUniformValue("uBufInvert", bufferInvert);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_blendShader->release();

    dst.partner()->release();
    dst.swap();
}
