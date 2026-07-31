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

#include <BasicLogger.h>

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFunctions>
#include <QByteArray>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector4D>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QString>

#include <algorithm>
#include <cmath>
#include <filesystem>

// AVI effect (r_avi.cpp port): decoded via Video for Windows — the same API the
// original uses, so the legacy codecs (Cinepak/Indeo/MSVideo1) keep working.
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vfw.h>
#pragma comment(lib, "vfw32")
#endif

using namespace lumi::multieffect;
using lumi::scripting::ScriptSlotHost;
using Slot = lumi::scripting::LuaScriptEngine::Slot;

namespace {

// Defined further down; forward-declared for the earlier render handlers.
void computeAudioBands(const std::vector<float>& spec, float& bass, float& mid,
                       float& treble);

/// Das einheitliche Audio-Input-Set (Entscheid E1) in ein Skript-Modul. Die
/// Namen gelten in JEDEM Dialekt; kollidierende Preset-eigene Bezeichner werden
/// beim Import auf `_p` umbenannt (Entscheid D2), nicht hier zur Laufzeit
/// ausgespart.
template <class Module>
void feedAudioInputSet(Module& mod, float bass, float mid, float treble,
                       double vol, bool beat)
{
    mod.setVariable("bass", bass);
    mod.setVariable("mid", mid);
    mod.setVariable("treb", treble);
    mod.setVariable("treble", treble);
    mod.setVariable("vol", vol);
    mod.setVariable("beat", beat ? 1.0 : 0.0);
}


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

// Present: 1:1-Kopie der Root-Surface aufs Fenster. Als Quad-Draw statt
// glBlitFramebuffer, weil das App-Fenster multisampled ist (samples=4) und
// ein SKALIERENDER Blit in ein MS-Ziel GL_INVALID_OPERATION wirft — mit
// Render-Scale fror das Bild ein (Befund S47). Der Upscale-Filter kommt
// von den Textur-Parametern (nearest/linear).
const char* kPresentFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
out vec4 fragColor;
void main()
{
    fragColor = vec4(texture(uTex, vTex).rgb, 1.0);
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

// AVS Movement builtins WITHOUT eval_desc (r_trans.cpp): per-pixel index
// remaps. Mode 1 "slight fuzzify" = static +-1 random neighbour (the AVS
// table is built ONCE per size, :316-323 — hence a position hash, not
// per-frame noise). Mode 7 "blocky partial out" = 2x2 blocks in a 4x4 raster
// sample a 7/8 centre zoom on even-aligned coords (:339-358). uBlend is the
// r_trans 50/50 blend of moved and original.
const char* kMoveRemapFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform int uMode;
uniform bool uBlend;
out vec4 fragColor;
void main()
{
    ivec2 size = textureSize(uTex, 0);
    ivec2 p = ivec2(gl_FragCoord.xy);
    ivec2 src = p;
    if (uMode == 1)
    {
        uint h = uint(p.x) * 374761393u + uint(p.y) * 668265263u;
        h = (h ^ (h >> 13u)) * 1274126177u;
        src = p + ivec2(int(h % 3u) - 1, int((h / 3u) % 3u) - 1);
    }
    else if (uMode == 7)
    {
        if ((p.x & 2) == 0 && (p.y & 2) == 0)
            src = ivec2(size.x / 2 + (((p.x & ~1) - size.x / 2) * 7) / 8,
                        size.y / 2 + (((p.y & ~1) - size.y / 2) * 7) / 8);
    }
    src = clamp(src, ivec2(0), size - 1);
    vec3 moved = texelFetch(uTex, src, 0).rgb;
    if (uBlend) moved = (moved + texelFetch(uTex, p, 0).rgb) * 0.5;
    fragColor = vec4(moved, 1.0);
}
)";

// Text/AVI overlay (r_text / r_avi): composes a top-down RGBA layer over the
// base — blend applies ONLY where the layer has alpha (AVS colour-key
// equivalent). AVI frames are converted to opaque top-down RGBA on upload.
const char* kTextFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uImg;
uniform int uBlend;   // 0 replace, 1 additive, 2 50/50
out vec4 fragColor;
void main()
{
    vec3 fb = texture(uTex, vTex).rgb;
    vec4 t = texture(uImg, vec2(vTex.x, 1.0 - vTex.y));
    vec3 mixed;
    if (uBlend == 1)      mixed = min(fb + t.rgb, vec3(1.0));
    else if (uBlend == 2) mixed = (fb + t.rgb) * 0.5;
    else                  mixed = t.rgb;
    fragColor = vec4(mix(fb, mixed, t.a), 1.0);
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
    // 50/50 ist in AVS die ABGESCHNITTENE Byte-Mittelung: jeder Operand wird
    // fuer sich halbiert (`psrlq 1` + Maske gegen das geborgte Bit), dann
    // addiert — floor(a/2)+floor(b/2), NICHT (a+b)/2. Der Unterschied ist
    // hoechstens 1 von 255 und faellt einmalig nicht auf; in einer Rueckkopplung
    // (Buffer Save 50/50 in den Puffer, den eine Dynamic Movement im naechsten
    // Frame wieder liest) treibt er das Bild ueber die Frames auseinander
    // (Befund S58, Alternate Reality). Gleiche Klasse wie der Blur in S57.
    if (uMode == 2)                                  // 50/50 (r_defs.h:504-544)
        r = (floor(round(d * 255.0) * 0.5) + floor(round(s * 255.0) * 0.5)) / 255.0;
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
// AVS "Trans / Blur" rechnet in 8-BIT-GANZZAHLEN: `DIV_2`/`DIV_4`/`DIV_8`/
// `DIV_16` (r_blur.cpp) sind Byte-Shifts, die JEDEN Teilterm einzeln
// ABSCHNEIDEN. Deshalb verliert jede Anwendung Helligkeit, und der Schalter
// „round mode" legt je Kernel einen festen Ausgleich obendrauf: +4 (leicht),
// +5 (mittel), +3 (stark). Unsere Chain-Surfaces sind GL_RGBA8, die Werte
// liegen also auf demselben 1/255-Raster — die Arithmetik ist nachbildbar.
//
// Der Mittelterm braucht ZWEI Gewichte, weil die Referenz bei der mittleren
// Staerke `DIV_2(f[0]) + DIV_4(f[0])` rechnet: zwei getrennt abgeschnittene
// Terme sind nicht dasselbe wie einer mit 0,75 (bei Byte-Wert 103 ergibt die
// Referenz 76, ein einzelner Term 77).
//
// Bis S57 rechnete diese Stelle in float mit exakter Gewichtssumme 1 — kein
// Verlust, kein Ausgleich, und `roundUp` wurde nirgends gelesen (Feld-Sonde
// stumm). NICHT nachgebildet ist der Byte-UEBERLAUF der Referenz (dort addiert
// eine 32-Bit-Addition den Ausgleich wortweise, ein Kanal kann in den naechsten
// laufen) und die eigenen Ausgleichswerte der RANDzeilen; hier wird geklemmt.
const char* kBlurFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uTexel;
uniform float uCenterA;
uniform float uCenterB;
uniform float uNeighbor;
uniform float uRoundAdj;
out vec4 fragColor;
vec3 anteil(vec3 v, float w)
{
    return floor(v * 255.0 * w);
}
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    vec3 l = texture(uTex, clamp(vTex + vec2(-uTexel.x, 0.0), 0.0, 1.0)).rgb;
    vec3 r = texture(uTex, clamp(vTex + vec2( uTexel.x, 0.0), 0.0, 1.0)).rgb;
    vec3 u = texture(uTex, clamp(vTex + vec2(0.0,  uTexel.y), 0.0, 1.0)).rgb;
    vec3 d = texture(uTex, clamp(vTex + vec2(0.0, -uTexel.y), 0.0, 1.0)).rgb;
    vec3 summe = anteil(c, uCenterA) + anteil(c, uCenterB) +
                 anteil(l, uNeighbor) + anteil(r, uNeighbor) +
                 anteil(u, uNeighbor) + anteil(d, uNeighbor) +
                 vec3(uRoundAdj);
    fragColor = vec4(min(summe, 255.0) / 255.0, 1.0);
}
)";

// LumiViz Bloom (Lights-Etappe 1), Pass 1: Downsample auf das Glow-RT mit
// optionalem Helligkeits-Threshold (Referenz "Lights" nutzt KEINEN — 0 ist
// der Referenz-Pfad). Die Quelle wird fuer den Draw linear gefiltert.
const char* kBloomDownFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform float uThreshold;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    fragColor = vec4(max(c - vec3(uThreshold), 0.0), 1.0);
}
)";

// LumiViz Bloom, Pass 2+3: separierbarer Gauss mit fix 25 Taps (Referenz
// buildKernel(sigma), Lights/EffectComposer). uDir = ein Texel in
// Blur-Richtung; die Gewichte werden im Shader normiert (Summe = 1, damit
// der Blur die Energie erhaelt — Gate-Bedingung).
const char* kBloomGaussFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uDir;
uniform float uSigma;
out vec4 fragColor;
void main()
{
    vec3 sum = vec3(0.0);
    float wsum = 0.0;
    for (int i = -12; i <= 12; ++i)
    {
        float w = exp(-float(i * i) / (2.0 * uSigma * uSigma));
        sum += w * texture(uTex, clamp(vTex + float(i) * uDir, 0.0, 1.0)).rgb;
        wsum += w;
    }
    fragColor = vec4(sum / wsum, 1.0);
}
)";

// LumiViz Bloom, Pass 4: additives Composite (min(a+b,1)) + optionale
// Vignette 1 - r^2 * strength (Referenz-Abschluss, r = 0 in der Mitte).
const char* kBloomCompFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uBase;
uniform sampler2D uGlow;
uniform float uIntensity;
uniform bool uVignette;
uniform float uVigStrength;
out vec4 fragColor;
void main()
{
    vec3 c = min(texture(uBase, vTex).rgb +
                 texture(uGlow, vTex).rgb * uIntensity, vec3(1.0));
    if (uVignette)
    {
        vec2 p = vTex * 2.0 - 1.0;
        c *= clamp(1.0 - dot(p, p) * uVigStrength, 0.0, 1.0);
    }
    fragColor = vec4(c, 1.0);
}
)";

// LumiViz SuperScope 3D (Lights-Etappe 1): additive Soft-Sprite-Punktwolke.
// Vertex: pro Punkt 6 Vertices (zwei Dreiecke) — Center in NDC, Corner -1..1,
// halbe Sprite-Ausdehnung in NDC (bereits 1/w-attenuiert), Farbe (gefoggt).
const char* kSprite3DVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aCenter;
layout(location = 1) in vec2 aCorner;
layout(location = 2) in vec2 aHalf;
layout(location = 3) in vec3 aColor;
out vec2 vCorner;
out vec3 vColor;
void main()
{
    vCorner = aCorner;
    vColor = aColor;
    gl_Position = vec4(aCenter + aCorner * aHalf, 0.0, 1.0);
}
)";

// Fragment: eingebauter radialer Falloff exp(-r^2*k) (≈ dot.png der Lights-
// Referenz, KEIN Bild-Asset); minus exp(-k), damit der Quad-Rand exakt auf 0
// auslaeuft (keine sichtbare Kante). Additiv geblendet (GL_ONE, GL_ONE).
const char* kSprite3DFragmentShader = R"(
#version 330 core
in vec2 vCorner;
in vec3 vColor;
uniform float uFalloff;
out vec4 fragColor;
void main()
{
    float w = exp(-dot(vCorner, vCorner) * uFalloff) - exp(-uFalloff);
    fragColor = vec4(vColor * max(w, 0.0), 1.0);
}
)";

// LumiViz Terrain 3D (Lights-Etappe 2): dunkles opakes Heightfield-Mesh mit
// Distanz-Fog (das Mesh DARF zur Fog-Farbe mischen — anders als die
// additiven Sprites, die nur daempfen).
const char* kTerrain3DVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uVp;
uniform vec3 uCamPos;
out float vDist;
void main()
{
    vDist = length(aPos - uCamPos);
    gl_Position = uVp * vec4(aPos, 1.0);
}
)";

const char* kTerrain3DFragmentShader = R"(
#version 330 core
in float vDist;
uniform vec3 uColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uFogColor;
out vec4 fragColor;
void main()
{
    vec3 c = uColor;
    if (uFogEnd > uFogStart)
        c = mix(c, uFogColor,
                clamp((vDist - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0));
    fragColor = vec4(c, 1.0);
}
)";

// LumiViz Glow Orbs (Lights-Etappe 2): Einheitskugel (Position = Richtung),
// per Uniform zum Ellipsoid skaliert; Zwei-Farb-Vertex-Verlauf ueber die
// Kugel-Hoehe (Original: mix(color2, color, ny)) + flash als additiver
// Offset (addRGB-Beat-Blitz), Distanz-Fog wie das Terrain.
const char* kOrb3DVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uVp;
uniform vec3 uCenter;
uniform vec3 uScale;
out float vNy;
out float vDist;
uniform vec3 uCamPos;
void main()
{
    vec3 world = uCenter + aPos * uScale;
    vNy = aPos.y * 0.5 + 0.5;
    vDist = length(world - uCamPos);
    gl_Position = uVp * vec4(world, 1.0);
}
)";

const char* kOrb3DFragmentShader = R"(
#version 330 core
in float vNy;
in float vDist;
uniform vec3 uColor;
uniform vec3 uColor2;
uniform float uFlash;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uFogColor;
out vec4 fragColor;
void main()
{
    vec3 c = mix(uColor2, uColor, vNy) + vec3(uFlash);
    if (uFogEnd > uFogStart)
        c = mix(c, uFogColor,
                clamp((vDist - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0));
    fragColor = vec4(clamp(c, 0.0, 1.0), 1.0);
}
)";

// Flat-Color-Fill (S48): my_triangle-Ersatz — gefuellte Dreiecke in einer
// Uniform-Farbe, Replace ohne Blend (linedraw.cpp my_triangle kennt keinen
// Blendmode). Genutzt von Bass Spin Modus 1.
const char* kFlatVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
)";

const char* kFlatFragmentShader = R"(
#version 330 core
uniform vec3 uColor;
out vec4 fragColor;
void main() { fragColor = vec4(uColor, 1.0); }
)";

// AVS Mirror (ID 26): reflect one half onto the other (r_mirror.cpp:166-247).
// r_mirror: four directed half-copies with per-direction smooth factors
// (BLEND_ADAPT divisor 0..16 -> uF 0..1). Sequential-buffer semantics are
// approximated in one pass: each stage mixes with the ORIGINAL texture, which
// differs from AVS only when opposing directions are active simultaneously.
// AVS Mirror (ID 26) — EINE Richtung je Durchgang (r_mirror.cpp:167-250).
// Das Original laeuft vier Schleifen NACHEINANDER ueber den Framebuffer
// (VERTICAL1, VERTICAL2, HORIZONTAL1, HORIZONTAL2); jede sieht das Ergebnis
// der vorigen. Frueher stand das hier in EINEM Durchgang, der alle vier Regeln
// aus der UNVERAENDERTEN Textur las und sich dabei selbst ueberschrieb — bei
// zwei aktiven Achsen kam damit weder eine Spiegelung noch ein symmetrisches
// Bild heraus (Befund S52: die Referenz spiegelte exakt, Fehler 0,0000, wir
// 0,042). uDir: 0 = top->bottom, 1 = bottom->top, 2 = left->right,
// 3 = right->left. uFac ist die Smooth-Rampe (1 = harte Kopie).
const char* kMirrorFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform int uDir;
uniform float uFac;
out vec4 fragColor;
void main()
{
    vec2 uv = vTex;
    vec3 c = texture(uTex, uv).rgb;
    bool vertical = uDir >= 2;
    vec2 src = vertical ? vec2(1.0 - uv.x, uv.y) : vec2(uv.x, 1.0 - uv.y);
    bool hit = uDir == 0 ? uv.y < 0.5
             : uDir == 1 ? uv.y > 0.5
             : uDir == 2 ? uv.x > 0.5
                         : uv.x < 0.5;
    if (hit) c = mix(c, texture(uTex, src).rgb, uFac);
    fragColor = vec4(c, 1.0);
}
)";

// APE "Metaballs 3D" (UnConeD) — Verhaltens-Nachbau (S52). Summiertes
// 1/r²-Feld mehrerer Kugeln, an `uThreshold` geschwellt; die Oberflaeche traegt
// die Palette-Farbe der Kugel mit dem groessten Beitrag, der Rand faellt weich
// ab. `uSphere` = xyz + Radius (bereits perspektivisch skaliert), `uTint` die
// Farbe je Kugel.
const char* kMetaballFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform int uCount;
uniform float uThreshold;
uniform float uAspect;
uniform vec4 uSphere[16];
uniform vec3 uTint[16];
out vec4 fragColor;

float fieldAt(vec2 p)
{
    float f = 0.0;
    for (int i = 0; i < 16; ++i)
    {
        if (i >= uCount) break;
        vec2 d = p - uSphere[i].xy;
        f += (uSphere[i].w * uSphere[i].w) / max(dot(d, d), 1e-5);
    }
    return f;
}

/// Hoehe der Kuppel ueber der Isoflaeche — am Rand 0, nach innen beschraenkt.
float domeAt(vec2 p)
{
    return sqrt(max(fieldAt(p) / max(uThreshold, 1e-5) - 1.0, 0.0));
}

void main()
{
    vec2 p = (vTex * 2.0 - 1.0);
    p.x *= uAspect;

    // Farbe GEWICHTET mischen, nicht "naechste Kugel gewinnt": die Referenz
    // zeigt EINEN verschmolzenen Koerper mit weichem Farbverlauf, die
    // Naechster-Nachbar-Wahl ergab harte Facetten (Sichtvergleich S52).
    float field = 0.0;
    vec3 tint = vec3(0.0);
    for (int i = 0; i < 16; ++i)
    {
        if (i >= uCount) break;
        vec2 d = p - uSphere[i].xy;
        float c = (uSphere[i].w * uSphere[i].w) / max(dot(d, d), 1e-5);
        field += c;
        tint += uTint[i] * c;
    }
    tint /= max(field, 1e-5);
    // Ausserhalb der Isoflaeche wird NICHTS gezeichnet — die Blobs der Referenz
    // sind deckende Koerper, kein Leuchten (Sichtvergleich S52).
    if (field < uThreshold) discard;

    // Normale aus einer KUPPELHOEHE statt direkt aus dem Feld: `sqrt(f/t - 1)`
    // ist am Rand 0 und waechst nach innen beschraenkt. Der rohe
    // 1/r²-Gradient explodiert dagegen dicht an den Zentren — die Normale kippt
    // dort in die Waagerechte und hinterliess einen dunklen Fleck je Kugel
    // (Sichtvergleich S52).
    float e = 0.006;
    float gx = domeAt(p + vec2(e, 0.0)) - domeAt(p - vec2(e, 0.0));
    float gy = domeAt(p + vec2(0.0, e)) - domeAt(p - vec2(0.0, e));
    vec3 n = normalize(vec3(-gx, -gy, 2.0 * e));
    vec3 lightDir = normalize(vec3(-0.45, 0.55, 0.85));
    float diff = max(dot(n, lightDir), 0.0);
    float spec = pow(max(dot(reflect(-lightDir, n), vec3(0.0, 0.0, 1.0)), 0.0), 24.0);
    vec3 col = tint * (0.30 + 0.70 * diff) + vec3(spec * 0.6);
    fragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
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
uniform bool uTrunc;        // subpixel: sample via the exact BLEND4 replica
out vec4 fragColor;

// Befund B (S46): AVS' bilinearer Resampler BLEND4 (r_defs.h MMX-Pfad) ist
// KEIN GL-bilinear: zwei Stufen (a*(255-p)+b*p)>>8 mit Integer-Trunkierung
// und Gewichtssumme 255/256 je Stufe — jedes Resample verliert ~0.8 % plus
// Rundung, Feedback-Trails (Zoom!) dunkeln ab und sterben aus. Dazu zaehlt
// AVS Pixel-RASTERPUNKTE (c=0 == Texel 0, kein Zentren-Offset). GL-float-
// bilinear ist verlustfrei -> Trails saettigten die Flaeche.
vec3 avsBlend4(vec2 uv)
{
    vec2 size = vec2(textureSize(uSrcTex, 0));
    vec2 c = uv * size;                 // AVS: (x_ndc+1)*half = Rasterpunkt
    vec2 f = floor(min(c, size - 1.0));
    ivec2 i0 = ivec2(f);
    ivec2 i1 = min(i0 + 1, ivec2(size) - 1);
    vec2 fpart = clamp(floor((c - f) * 256.0), 0.0, 255.0);  // 16.16 >> 8
    vec3 p0 = floor(texelFetch(uSrcTex, ivec2(i0.x, i0.y), 0).rgb * 255.0 + 0.5);
    vec3 p1 = floor(texelFetch(uSrcTex, ivec2(i1.x, i0.y), 0).rgb * 255.0 + 0.5);
    vec3 p2 = floor(texelFetch(uSrcTex, ivec2(i0.x, i1.y), 0).rgb * 255.0 + 0.5);
    vec3 p3 = floor(texelFetch(uSrcTex, ivec2(i1.x, i1.y), 0).rgb * 255.0 + 0.5);
    vec3 top = floor((p0 * (255.0 - fpart.x) + p1 * fpart.x) / 256.0);
    vec3 bot = floor((p2 * (255.0 - fpart.x) + p3 * fpart.x) / 256.0);
    vec3 res = floor((top * (255.0 - fpart.y) + bot * fpart.y) / 256.0);
    return res / 255.0;
}

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
        vec3 moved = uTrunc ? avsBlend4(uv) : texture(uSrcTex, uv).rgb;
        c = uBlend ? mix(orig, moved, a) : moved;
    }
    fragColor = vec4(c, 1.0);
}
)";

// Movement bit-treu (r_trans.cpp:553-720, S49). r_trans hat KEIN Gitter: die
// Tabelle haelt je PIXEL einen int — Ziel-Offset (22 Bit) plus die auf 5 Bit
// quantisierten Subpixel-Anteile. Der Shader dekodiert genauso und mischt mit
// BLEND4 (Gewichte 255-w, drei trunkierende Stufen).
const char* kMoveTabFragmentShader = R"(
#version 330 core
uniform sampler2D uTex;    // aktuelles Bild (Quelle UND Blend-Ziel)
uniform isampler2D uTab;   // r_trans-Tabelle, AVS-Zeilenordnung (0 = oben)
uniform int uResX;
uniform int uResY;
uniform bool uBlend;       // BLEND_AVG mit dem unbewegten Pixel
uniform bool uSubpixel;
out vec4 fragColor;

ivec3 texelAvsLinear(int addr)
{
    // r_trans adressiert linear (framebuffer+offs, +1, +w, +w+1) — bei der
    // Tabellen-Klemmung auf w-2/h-2 bleibt das innerhalb des Bildes.
    addr = clamp(addr, 0, uResX * uResY - 1);
    int ay = addr / uResX;
    int ax = addr - ay * uResX;
    return ivec3(texelFetch(uTex, ivec2(ax, uResY - 1 - ay), 0).rgb * 255.0 + 0.5);
}

void main()
{
    int dx = int(gl_FragCoord.x);
    int dy = uResY - 1 - int(gl_FragCoord.y);
    int v = texelFetch(uTab, ivec2(dx, dy), 0).r;
    int offs = v & ((1 << 22) - 1);

    ivec3 moved;
    if (uSubpixel)
    {
        int xw = (v >> 24) & (31 << 3);  // 5 Bit, auf 0..248 gespreizt
        int yw = (v >> 19) & (31 << 3);
        ivec3 c00 = texelAvsLinear(offs);
        ivec3 c10 = texelAvsLinear(offs + 1);
        ivec3 c01 = texelAvsLinear(offs + uResX);
        ivec3 c11 = texelAvsLinear(offs + uResX + 1);
        ivec3 top = (c00 * (255 - xw) + c10 * xw) >> 8;
        ivec3 bot = (c01 * (255 - xw) + c11 * xw) >> 8;
        moved = (top * (255 - yw) + bot * yw) >> 8;
    }
    else
    {
        moved = texelAvsLinear(offs);
    }

    if (uBlend)
    {
        // BLEND_AVG: (a>>1)+(b>>1) je Kanal, ohne Uebertrag — die Bit-0-Verluste
        // sind der Grund, warum float-mix hier nicht deckungsgleich ist.
        ivec3 orig = ivec3(texelFetch(uTex, ivec2(dx, uResY - 1 - dy), 0).rgb * 255.0 + 0.5);
        moved = (orig >> 1) + (moved >> 1);
    }
    fragColor = vec4(vec3(moved) / 255.0, 1.0);
}
)";

// Dynamic Movement bit-treu (r_dmove.cpp:372-578, S49). Das Original legt die
// Skript-Ergebnisse als 16.16-Fixpunkt-Tabelle (XRES x YRES) ab und interpoliert
// sie SEPARABEL mit Ganzzahl-Arithmetik auf die volle Bildgroesse: je Gitterband
// eine trunkierende Division (`/yseek`, `/seek`) und danach reine Additionen.
// Das ist pro Pixel geschlossen berechenbar — Bandindex, Restweg und Bandbreite
// stehen fest, der Rest ist `start + rest * ((ende-start)/breite)`. Unser altes
// Dreiecksnetz interpolierte dagegen baryzentrisch in float: andere Kurve (Netz
// statt bilinear) UND kein Trunkierungsverlust, der sich ueber Feedback-Ketten
// aufsummiert (gleiche Familie wie der Roto-Befund S48).
const char* kWarpFxFragmentShader = R"(
#version 330 core
uniform sampler2D uTex;      // aktuelles Bild (Blend-Ziel = "framebuffer")
uniform sampler2D uSrcTex;   // Warp-Quelle (Bild oder Global-Buffer)
uniform isampler2D uTab;     // Gittertabelle (x16, y16, a16), AVS-Zeilen (0=oben)
// Merkregel S48: Qt sendet QPoint-Uniforms als FLOAT — Integer-Uniforms deshalb
// immer einzeln (setUniformValue(int) trifft glUniform1i).
uniform int uResX;
uniform int uResY;
uniform int uGridX;          // XRES
uniform int uGridY;          // YRES
uniform int uXcDpos;         // (w<<16)/(XRES-1) — trunkiert wie das Original
uniform int uYcDpos;
uniform int uWAdj;           // (w-2)<<16 mit subpixel, sonst (w-1)<<16
uniform int uHAdj;
uniform bool uWrap;
uniform bool uBlend;
uniform bool uNomove;
uniform bool uBufSrc;
uniform bool uSubpixel;
out vec4 fragColor;

// C-Semantik von %: GLSL laesst % mit negativen Operanden offen.
int cmod(int v, int m) { return v - (v / m) * m; }

// Band eines Ausgabepixels: (Index, Rest im Band, Bandbreite). Die Baender sind
// [ (b*dpos)>>16 , ((b+1)*dpos)>>16 ) — durch die Trunkierung von dpos NICHT
// gleich breit, deshalb der Korrekturschritt.
ivec3 bandOf(int p, int dpos, int n)
{
    int b = clamp((p << 16) / dpos, 0, n - 2);
    if (((b + 1) * dpos) >> 16 <= p) b = min(b + 1, n - 2);
    if ((b * dpos) >> 16 > p) b = max(b - 1, 0);
    int lo = (b * dpos) >> 16;
    int hi = ((b + 1) * dpos) >> 16;
    return ivec3(b, p - lo, max(hi - lo, 1));
}

ivec3 texelAvs(sampler2D tex, int x, int y)
{
    x = clamp(x, 0, uResX - 1);
    y = clamp(y, 0, uResY - 1);
    // AVS-Zeile 0 = oben, unsere Textur ist bottom-up.
    return ivec3(texelFetch(tex, ivec2(x, uResY - 1 - y), 0).rgb * 255.0 + 0.5);
}

// BLEND_ADJ (r_defs.h, MMX-Zweig): Gewichte v und 255-v, EINE Trunkierung.
ivec3 blendAdj(ivec3 a, ivec3 b, int v)
{
    return (a * v + b * (255 - v)) >> 8;
}

void main()
{
    int dx = int(gl_FragCoord.x);
    int dy = uResY - 1 - int(gl_FragCoord.y);  // AVS-Zeile
    ivec3 bx = bandOf(dx, uXcDpos, uGridX);
    ivec3 by = bandOf(dy, uYcDpos, uGridY);

    // Zeilen-Interpolation (r_dmove.cpp:418-432 + 469-471): stab[] startet auf
    // dem oberen Gitterwert und addiert je Ausgabezeile (unten-oben)/yseek.
    ivec3 t00 = texelFetch(uTab, ivec2(bx.x, by.x), 0).rgb;
    ivec3 t01 = texelFetch(uTab, ivec2(bx.x, by.x + 1), 0).rgb;
    ivec3 t10 = texelFetch(uTab, ivec2(bx.x + 1, by.x), 0).rgb;
    ivec3 t11 = texelFetch(uTab, ivec2(bx.x + 1, by.x + 1), 0).rgb;
    ivec3 left  = t00 + by.y * ((t01 - t00) / by.z);
    ivec3 right = t10 + by.y * ((t11 - t10) / by.z);
    // Spalten-Interpolation (r_dmove.cpp:463-468 + NORMAL_LOOP): dito je Pixel.
    ivec3 v = left + bx.y * ((right - left) / bx.z);

    int xp = v.x;
    int yp = v.y;
    if (uWrap)
    {
        // WRAPPING_LOOPS korrigiert einmal je Schritt; da |d| < w_adj bleibt,
        // ist das mathematisch das volle Modulo (DO_LOOPS-Argument wie Roto).
        xp = cmod(xp, uWAdj); if (xp < 0) xp += uWAdj;
        yp = cmod(yp, uHAdj); if (yp < 0) yp += uHAdj;
    }
    else
    {
        // CLAMPED_LOOPS klemmt den AKKUMULATOR; da beide Stuetzstellen bereits
        // in [0, w_adj] liegen, deckt sich das mit dem Klemmen des Endwerts.
        xp = clamp(xp, 0, uWAdj - 1);
        yp = clamp(yp, 0, uHAdj - 1);
    }
    int ap = clamp(v.z >> 16, 0, 255);

    ivec3 moved;
    if (uSubpixel)
    {
        // BLEND4_16 (r_defs.h): Gewichte (p>>8)&0xff gegen 255-w (mmx_blend4_revn
        // = 0x00ff!), jede der drei Stufen trunkiert mit >>8.
        int xw = (xp >> 8) & 0xff;
        int yw = (yp >> 8) & 0xff;
        int ix = xp >> 16;
        int iy = yp >> 16;
        ivec3 c00 = texelAvs(uSrcTex, ix, iy);
        ivec3 c10 = texelAvs(uSrcTex, ix + 1, iy);
        ivec3 c01 = texelAvs(uSrcTex, ix, iy + 1);
        ivec3 c11 = texelAvs(uSrcTex, ix + 1, iy + 1);
        ivec3 top = (c00 * (255 - xw) + c10 * xw) >> 8;
        ivec3 bot = (c01 * (255 - xw) + c11 * xw) >> 8;
        moved = (top * (255 - yw) + bot * yw) >> 8;
    }
    else
    {
        moved = texelAvs(uSrcTex, xp >> 16, yp >> 16);
    }

    ivec3 orig = texelAvs(uTex, dx, dy);
    ivec3 res;
    if (uNomove)
    {
        // r_dmove.cpp:531-543: ohne Verschiebung wird die QUELLE am selben Ort
        // eingeblendet — ohne Global-Buffer ist die Quelle 0 (Ausblenden).
        res = blendAdj(uBufSrc ? texelAvs(uSrcTex, dx, dy) : ivec3(0), orig, ap);
    }
    else
    {
        res = uBlend ? blendAdj(moved, orig, ap) : moved;
    }
    fragColor = vec4(vec3(res) / 255.0, 1.0);
}
)";

// AVS Roto / Blitter Feedback (ID 9 / 4): sample the current image rotated and
// zoomed about the center, blend with the original — a scale/rotate feedback.
// Feedback-Abbildung (Blitter/Roto Blitter, S48 nach r_blit/r_rotblit
// umgebaut): affines Sampling in PIXELN — sp = uMap*px + uOff. Roto WRAPPT
// wie das Original (s %= ds -> Kacheln), Blitter clampt. uSubpixel = das
// Original-BLEND4 (bilinear von Hand, die Surface-Textur filtert NEAREST);
// ohne subpixel kaskadiert Nearest-Zoom zum Mosaik (Matrix-Befund 04/09).
// uBlend = BLEND_AVG mit dem unbewegten Original.
const char* kFeedbackFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform vec2 uRes;
uniform mat2 uMap;      // affiner Modus (Blitter): sp = uMap*px + uOff
uniform vec2 uOff;
uniform bool uAvsLinear;  // r_rotblit-Modus (s. unten)
uniform int uDsDx;      // 16.16-Fixpunkt wie das Original (int-Arithmetik!)
uniform int uDsDy;
uniform int uDtDx;
uniform int uDtDy;
uniform int uSStart;    // sstart/tstart fuer dest (0,0), inkl. Positiv-Offset
uniform int uTStart;
uniform bool uBlend;
uniform bool uSubpixel;
out vec4 fragColor;
vec3 fetchClamped(ivec2 p)
{
    p = clamp(p, ivec2(0), ivec2(uRes) - 1);
    return texelFetch(uTex, p, 0).rgb;
}
// r_rotblit adressiert den Framebuffer LINEAR: addr = (t>>16)*w + (s>>16);
// die Bilinear-Nachbarn sind addr+1 und addr+w — auch ueber Zeilengrenzen
// hinweg (AVS-Zeilen sind top-down, die Textur bottom-up -> Flip hier).
vec3 fetchLinear(int addr)
{
    int w = int(uRes.x);
    addr = clamp(addr, 0, w * int(uRes.y) - 1);
    int iy = addr / w;
    int ix = addr - iy * w;
    return texelFetch(uTex, ivec2(ix, int(uRes.y) - 1 - iy), 0).rgb;
}
void main()
{
    vec2 px = vTex * uRes - 0.5;
    vec3 moved;
    if (uAvsLinear)
    {
        // dest in AVS-Koordinaten (Zeile 0 = oben). r_rotblit wrappt s/t am
        // Zeilenanfang UND inkrementell je Pixel (DO_LOOPS: eine Korrektur
        // je Ueberlauf) — das ist mathematisch ein volles mod ueber
        // (w-1)<<16. Alles in 16.16-INT-Arithmetik wie das Original (float
        // kippte Gewichtsstufen und driftete ueber die Feedback-Kaskade).
        int w = int(uRes.x);
        int h = int(uRes.y);
        int dx = int(px.x + 0.5);
        int dy = (h - 1) - int(px.y + 0.5);
        int rw = (w - 1) << 16;
        int rh = (h - 1) << 16;
        int s = uSStart + dy * uDsDy;
        int t = uTStart + dy * uDtDy;
        if (rw != 0) { s %= rw; if (s < 0) s += rw; }
        if (rh != 0) { t %= rh; if (t < 0) t += rh; }
        s += dx * uDsDx;
        t += dx * uDtDx;
        if (rw != 0) { s %= rw; if (s < 0) s += rw; }
        if (rh != 0) { t %= rh; if (t < 0) t += rh; }
        int addr = (t >> 16) * w + (s >> 16);
        if (uSubpixel)
        {
            // BLEND4_16 (r_defs.h): Gewichte (s>>8)&0xff GEGEN 255-w, jede
            // Mischstufe trunkiert (>>8). Die 255 ist kein Tippfehler des
            // Originals — mmx_blend4_revn = 0x00ff00ff (render.cpp:64, „more
            // correct" waere 0x0100, aber sie wollten Gleichstand mit dem
            // Nicht-MMX-Zweig). Jede Stufe verliert dadurch ~0,4 %, was sich
            // ueber die Feedback-Kaskade zu sichtbarem Abdunkeln summiert (S49).
            int xw = (s >> 8) & 0xff;
            int yw = (t >> 8) & 0xff;
            ivec3 c00 = ivec3(fetchLinear(addr) * 255.0 + 0.5);
            ivec3 c10 = ivec3(fetchLinear(addr + 1) * 255.0 + 0.5);
            ivec3 c01 = ivec3(fetchLinear(addr + w) * 255.0 + 0.5);
            ivec3 c11 = ivec3(fetchLinear(addr + w + 1) * 255.0 + 0.5);
            ivec3 a = (c00 * (255 - xw) + c10 * xw) >> 8;
            ivec3 b = (c01 * (255 - xw) + c11 * xw) >> 8;
            moved = vec3((a * (255 - yw) + b * yw) >> 8) / 255.0;
        }
        else
        {
            moved = fetchLinear(addr);
        }
    }
    else
    {
        vec2 sp = uMap * px + uOff;
        sp = clamp(sp, vec2(0.0), uRes - 1.0);
        if (uSubpixel)
        {
            vec2 fl = floor(sp);
            vec2 fr = sp - fl;
            ivec2 i0 = ivec2(fl);
            vec3 c00 = fetchClamped(i0);
            vec3 c10 = fetchClamped(i0 + ivec2(1, 0));
            vec3 c01 = fetchClamped(i0 + ivec2(0, 1));
            vec3 c11 = fetchClamped(i0 + ivec2(1, 1));
            moved = mix(mix(c00, c10, fr.x), mix(c01, c11, fr.x), fr.y);
        }
        else
        {
            moved = fetchClamped(ivec2(sp));
        }
    }
    vec3 o = texture(uTex, vTex).rgb;
    fragColor = vec4(uBlend ? mix(o, moved, 0.5) : moved, 1.0);
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

// Der n-te Wert der AVS-rand()-Folge, aus `uSeed` als Startzustand. Auf der GPU
// gibt es keine Zug-Reihenfolge — also wird der Strom gesprungen: die Abbildung
// x -> a*x+c ist affin, ihre n-te Iteration laesst sich per
// Binaerexponentiation in O(log n) zusammensetzen (S49).
//
// Als MAKRO, damit die Shader es zur Compile-Zeit einsetzen (angrenzende
// String-Literale werden verkettet): Scatter und Grain brauchen beide dieselbe
// Arithmetik, und zwei Kopien derselben Konstanten waeren zwei Wahrheiten.
// Jeder einsetzende Shader deklariert `uniform int uSeed`.
#define LUMI_GLSL_AVS_RAND R"(
uint randAt(uint n)
{
    uint a = 214013u, c = 2531011u, ra = 1u, rc = 0u;
    while (n != 0u)
    {
        if ((n & 1u) != 0u) { rc = a * rc + c; ra = a * ra; }
        c = a * c + c;
        a = a * a;
        n >>= 1u;
    }
    return ((ra * uint(uSeed) + rc) >> 16) & 0x7fffu;
}
)"

// AVS "Trans / Grain" (ID 24): darken a random subset of pixels by a random
// factor (r_grain.cpp). uStatic 1 = stehendes Muster, 0 = je Frame neu.
const char* kGrainFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uDepth;  // r_grain-depthBuffer: .r = Faktor, .g = Schwelle
uniform int uResX;
uniform int uResY;
uniform int uSmax;         // (smax*255)/100 wie im Original
uniform int uBlend;        // 0 replace, 1 additiv, 2 50/50
uniform int uStatic;       // 1 = stehendes Muster, 0 = je Frame neu
uniform int uSeed;         // Zustand des geteilten rand()-Stroms (nur uStatic=0)
out vec4 fragColor;
)" LUMI_GLSL_AVS_RAND R"(
void main()
{
    int dx = int(gl_FragCoord.x);
    int dy = uResY - 1 - int(gl_FragCoord.y);  // depthBuffer laeuft AVS-Zeilen
    ivec3 o = ivec3(texelFetch(uTex, ivec2(dx, uResY - 1 - dy), 0).rgb * 255.0 + 0.5);

    // Die zwei Zufallswerte je Pixel. `staticgrain` an: aus dem einmal
    // gefuellten depthBuffer, also jeden Frame dieselben (Faktor 0..254,
    // Schwelle 0..99 — r_grain.cpp:136-141). Aus: die Referenz zieht sie je
    // Pixel frisch mit `fastrandbyte()`, beide als BYTE 0..255 — die Schwelle
    // hat dort also einen anderen Wertebereich als im statischen Fall, und die
    // Kornmenge unterscheidet sich zwischen den Betriebsarten systematisch.
    //
    // Bis S57 gab es diesen Zweig nicht: der Shader las immer den stehenden
    // Puffer, `staticGrain` wurde nirgends gelesen (Feld-Sonde stumm) und die
    // Vorgabe `false` versprach das Gegenteil von dem, was zu sehen war.
    // NICHT bitgleich zur Referenz ist die Zug-REIHENFOLGE: dort laeuft eine
    // Position sequentiell durch eine 491-Byte-Tabelle und springt alle 16
    // Zuege um `rand()%73`, und der Faktor wird nur gezogen, wenn die Schwelle
    // trifft — also datenabhaengig, was sich pro Pixel parallel nicht
    // berechnen laesst. Gleich sind Verteilung, Wertebereich und die
    // Eigenschaft, je Frame neu zu sein.
    ivec2 q;
    if (uStatic != 0)
    {
        q = ivec2(texelFetch(uDepth, ivec2(dx, dy), 0).rg * 255.0 + 0.5);
    }
    else
    {
        uint p = uint(dy * uResX + dx);
        q = ivec2(int(randAt(p * 2u + 2u) & 255u),   // Faktor
                  int(randAt(p * 2u + 1u) & 255u));  // Schwelle
    }

    // r_grain.cpp:227-249: schwarze Pixel bleiben unberuehrt; sonst wird der
    // Pixel mit q[0] skaliert ((c*s)>>8, je Kanal geklemmt) — und zwar NUR
    // wenn q[1] unter der Schwelle liegt, sonst wird er schwarz.
    ivec3 c = ivec3(0);
    if (q.y < uSmax) c = min((o * q.x) >> 8, ivec3(255));
    ivec3 r = c;
    if (uBlend == 1)      r = min(o + c, ivec3(255));      // BLEND
    else if (uBlend == 2) r = (o >> 1) + (c >> 1);         // BLEND_AVG
    if (o == ivec3(0)) r = o;                              // if (*p)
    fragColor = vec4(vec3(r) / 255.0, 1.0);
}
)";

// AVS "Trans / Scatter" (ID 16): per-pixel random displacement in a ~4px window
// (r_scat.cpp), refreshed each frame via uSeed.
const char* kScatterFragmentShader = R"(
#version 330 core
uniform sampler2D uTex;
uniform int uResX;
uniform int uResY;
// Merkregel S48/S49: Qt setzt Uniforms ueber glUniform1i — ein `uint`-
// Uniform bliebe dabei 0. Der Zustand kommt deshalb als int und wird
// hier bitgleich nach uint zurueckgedeutet.
uniform int uSeed;
out vec4 fragColor;
)" LUMI_GLSL_AVS_RAND R"(
// r_scat zieht je Pixel EINEN Wert aus dem globalen rand()-Strom.
vec3 fetchLinear(int addr)
{
    addr = clamp(addr, 0, uResX * uResY - 1);
    int ay = addr / uResX;
    return texelFetch(uTex, ivec2(addr - ay * uResX, uResY - 1 - ay), 0).rgb;
}

void main()
{
    int dx = int(gl_FragCoord.x);
    int dy = uResY - 1 - int(gl_FragCoord.y);  // AVS-Zeile
    int p = dy * uResX + dx;
    // r_scat.cpp:107-116: die obersten und untersten vier Zeilen bleiben stehen
    // (sie sind der Rand, den die Verschiebung braucht).
    if (dy < 4 || dy >= uResY - 4)
    {
        fragColor = vec4(fetchLinear(p), 1.0);
        return;
    }
    uint r = randAt(uint(p - 4 * uResX) + 1u) & 511u;
    // fudgetable (r_scat.cpp:92-101): +-3 Pixel in x und y, 0 eingeschlossen
    int xp = int(r % 8u) - 4;      if (xp < 0) xp++;
    int yp = int((r / 8u) % 8u) - 4; if (yp < 0) yp++;
    fragColor = vec4(fetchLinear(p + yp * uResX + xp), 1.0);
}
)";

// AVS "Trans / Interferences" (ID 41): accumulate uPoints rotated copies of the
// image, each weighted by uAlpha (r_interf.cpp). uRgb splits copies across the
// R/G/B channels; uBlend combines with the original.
// Die Gewichtung der Kopien laeuft in der Referenz ueber die BLEND-TABELLE
// `g_blendtable[_alpha][wert]` (r_interf.cpp:216), also `(alpha*wert)/255` als
// GANZZAHL — jede Kopie verliert dabei bis zu ein 255stel. Bei vier Kopien
// summiert sich das, und genau das war der Matrix-Rest `41_interferences`: die
// Struktur stimmte, im 4x-Diff standen nur Kantenkonturen (S57).
// `uAlpha255` ist deshalb der ROHE Alphawert 0..255, nicht der Bruch.
const char* kInterfFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform int uPoints;
uniform vec2 uOffsets[8];
uniform float uAlpha255;
uniform int uRgb;
uniform int uBlend;
out vec4 fragColor;
vec3 blendtab(vec3 wert)
{
    // g_blendtable: floor(i*j/255) je Kanal, in 8-Bit-Einheiten gerechnet.
    return floor(floor(wert * 255.0 + 0.5) * uAlpha255 / 255.0);
}
void main()
{
    vec3 orig = texture(uTex, vTex).rgb;
    vec3 acc = vec3(0.0);          // in 8-Bit-Einheiten (0..255)
    for (int i = 0; i < uPoints && i < 8; ++i)
    {
        // Ausserhalb des Bildes traegt eine Kopie NICHTS bei — die Referenz
        // prueft den Pixel-Index (`if (xp >= 0 && xp < w && yoffs[i] != -1)`,
        // r_interf.cpp:236) und laesst den Beitrag sonst auf 0. Unser Sampler
        // klemmte stattdessen auf den Randpixel und schmierte ihn nach innen.
        // Die Texturkoordinaten treffen Texelmitten, `>= 0 && < 1` ist also
        // genau dieselbe Bedingung.
        vec2 uv = vTex - uOffsets[i];
        vec3 s = vec3(0.0);
        if (uv.x >= 0.0 && uv.x < 1.0 && uv.y >= 0.0 && uv.y < 1.0)
            s = blendtab(texture(uTex, uv).rgb);
        if (uRgb == 1) { int ch = i - (i / 3) * 3; acc[ch] += s[ch]; }
        else acc += s;
    }
    acc = min(acc, vec3(255.0)) / 255.0;
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
// r_water.cpp zeilengenau. Drei Dinge, die eine float-Mittelung NICHT trifft:
//
//  1. Nachbarn ausserhalb des Bildes werden WEGGELASSEN, nicht geklemmt — die
//     Referenz hat je Rand einen eigenen Zweig (Ecke 2, Kante 3, Mitte 4
//     Nachbarn). Bei einem RUECKGEKOPPELTEN Effekt bleibt so ein Randfehler
//     nicht am Rand: er wandert ueber die Frames nach innen.
//  2. Die Summe wird GANZZAHLIG halbiert (`r/=2`, :274).
//  3. Die oberste und unterste Zeile werden GAR NICHT halbiert (:168-188) —
//     eine Eigenart des Originals, die dort einen hellen Saum erzeugt.
//
// Das war der Matrix-Rest `20_water` (S57).
const char* kWaterFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uCur;
uniform sampler2D uLast;
uniform vec2 uTexel;
uniform int uResX;
uniform int uResY;
out vec4 fragColor;
ivec3 hole(int ax, int ay)      // ay = AVS-Zeile (0 = oben)
{
    return ivec3(texelFetch(uCur, ivec2(ax, uResY - 1 - ay), 0).rgb * 255.0 + 0.5);
}
void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);
    int ax = pix.x;
    int ay = uResY - 1 - pix.y;

    ivec3 sum = ivec3(0);
    if (ax + 1 < uResX) sum += hole(ax + 1, ay);
    if (ax - 1 >= 0)    sum += hole(ax - 1, ay);
    if (ay + 1 < uResY) sum += hole(ax, ay + 1);   // f[w]  — eine Zeile tiefer
    if (ay - 1 >= 0)    sum += hole(ax, ay - 1);   // f[-w]
    sum /= 2;

    ivec3 last = ivec3(texelFetch(uLast, pix, 0).rgb * 255.0 + 0.5);
    fragColor = vec4(vec3(clamp(sum - last, ivec3(0), ivec3(255))) / 255.0, 1.0);
}
)";

// AVS "Trans / Bump" (ID 29): per-pixel bump lighting from the luminance
// gradient, lit by a movable source at uLight (r_bump.cpp). Bright near the
// light where the surface is flat; falls off with distance. The depth source
// is uDepthTex: the current frame, or a global buffer when `buffern` is set
// (r_bump.cpp:249). uCurbuf mirrors the original's curbuf flag: only when the
// depth source IS the framebuffer, pixels whose four depth neighbours are all
// zero are skipped and stay black (fbout is memset-0, r_bump.cpp:279/321);
// a global-buffer depth source always writes. The 1px border also stays black
// (the original only iterates the interior, r_bump.cpp:302-310).
const char* kBumpFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uDepthTex;
uniform vec2 uRes;
uniform vec2 uLight;
uniform float uDepth;
uniform int uInvert;
uniform int uBlend;
uniform int uCurbuf;
out vec4 fragColor;
float dpth(vec3 c)
{
    float m = max(max(c.r, c.g), c.b) * 255.0;
    return uInvert == 1 ? 255.0 - m : m;
}
void main()
{
    ivec2 p = ivec2(vTex * uRes);
    ivec2 sz = ivec2(uRes);
    if (p.x < 1 || p.y < 1 || p.x >= sz.x - 1 || p.y >= sz.y - 1)
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    vec3 orig = texelFetch(uTex, p, 0).rgb;
    vec3 m1 = texelFetch(uDepthTex, p + ivec2(-1, 0), 0).rgb;
    vec3 p1 = texelFetch(uDepthTex, p + ivec2(1, 0), 0).rgb;
    vec3 mw = texelFetch(uDepthTex, p + ivec2(0, -1), 0).rgb;
    vec3 pw = texelFetch(uDepthTex, p + ivec2(0, 1), 0).rgb;
    if (uCurbuf == 1 && m1 == vec3(0.0) && p1 == vec3(0.0) &&
        mw == vec3(0.0) && pw == vec3(0.0))
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    // Skript-Licht (x,y) und Gradient leben im AVS-Raum (y+ nach unten); die
    // Textur ist GL-y-up — Uebersetzung NUR hier am Modulrand (Konvention S46).
    float ay = uRes.y - 1.0 - float(p.y);
    float lx = float(p.x) - uLight.x * uRes.x;
    float ly = ay - uLight.y * uRes.y;
    float c1 = 127.0 - abs((dpth(p1) - dpth(m1)) - lx);
    float c2 = 127.0 - abs((dpth(mw) - dpth(pw)) - ly);
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

// AVS "Trans / Dynamic Shift" (ID 42), bit-treu nach r_shift.cpp:190-310 (S58).
//
// Die Referenz verschiebt NICHT stufenlos, sondern in ganzen Pixeln plus einem
// 8-Bit-Bruchteil, und sie schreibt die freigewordenen Zeilen und Spalten HART
// auf Schwarz. Der Bruchteil kippt dabei asymmetrisch: bei negativem Anteil
// bleibt `xa` stehen und `xpart` ist der Betrag, bei positivem wird `xa` um eins
// erhoeht und `xpart` gespiegelt (`255-xpart`). Genau daraus folgt, dass schon
// eine Verschiebung von NULL in y die erste und die letzte Zeile schwarz macht
// (`ya++`, `endy = h-1+ya`) — und eine Verschiebung um -0,2 in x die letzte
// SPALTE. Wir hatten stattdessen eine normierte uv-Verschiebung mit weichem
// Rand: der Saum blieb stehen, statt zu verschwinden. Bei "The Real
// Impressionist" faengt ein Movement genau diese Spalte ein.
//
// Die Grenzen (uXa/uYa/uEndX/uEndY) rechnet die Laufzeit, weil sie sich je
// Betriebsart (subpixel/nicht) unterscheiden; hier steht nur die Auswertung.
const char* kDynamicShiftFragmentShader = R"(
#version 330 core
uniform sampler2D uTex;
uniform int uXa;
uniform int uYa;
uniform int uXpart;
uniform int uYpart;
uniform int uEndX;
uniform int uEndY;
uniform int uW;
uniform int uH;
uniform int uBlend;
uniform int uIAlpha;
uniform int uSubpixel;
out vec4 fragColor;

// AVS-Zeile 0 ist oben, unsere Textur ist bottom-up.
ivec3 texAvs(int x, int y)
{
    x = clamp(x, 0, uW - 1);
    y = clamp(y, 0, uH - 1);
    return ivec3(round(texelFetch(uTex, ivec2(x, uH - 1 - y), 0).rgb * 255.0));
}

void main()
{
    int x = int(gl_FragCoord.x);
    int y = uH - 1 - int(gl_FragCoord.y);
    ivec3 orig = texAvs(x, y);
    ivec3 res = ivec3(0);
    if (y >= uYa && y < uEndY && x >= uXa && x < uEndX)
    {
        int sx = x - uXa;
        int sy = y - uYa;
        if (uSubpixel == 1)
        {
            // BLEND4 (r_defs.h): zwei Stufen, je `>>8` mit Gewichtssumme 255.
            ivec3 c00 = texAvs(sx, sy);
            ivec3 c10 = texAvs(sx + 1, sy);
            ivec3 c01 = texAvs(sx, sy + 1);
            ivec3 c11 = texAvs(sx + 1, sy + 1);
            ivec3 top = (c00 * (255 - uXpart) + c10 * uXpart) >> 8;
            ivec3 bot = (c01 * (255 - uXpart) + c11 * uXpart) >> 8;
            res = (top * (255 - uYpart) + bot * uYpart) >> 8;
        }
        else
        {
            res = texAvs(sx, sy);
        }
    }
    // BLEND_ADJ(verschoben, framebuffer, ialpha) — das VERSCHOBENE Bild traegt
    // `ialpha`, nicht das Original (r_shift.cpp:228; gleiche Richtung wie der
    // Adjustable-Blend-Befund aus S52).
    if (uBlend == 1) res = (res * uIAlpha + orig * (255 - uIAlpha)) >> 8;
    fragColor = vec4(vec3(res) / 255.0, 1.0);
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
uniform int uSubpixel;
out vec4 fragColor;

// AVS BLEND4 (r_defs.h): die vier Nachbarn mit 8-Bit-Anteilen mischen, in
// GANZZAHLEN und mit Abschneiden — nicht GL-bilinear. Der Unterschied ist
// belegt (s. Kommentar oben bei kFeedbackFragmentShader): GL-bilinear ist
// verlustfrei und laesst Trails ueber Rueckkopplungsketten saettigen.
vec3 avsBlend4(vec2 px)
{
    ivec2 res = ivec2(uRes);
    vec2 base = floor(px);
    ivec2 i0 = clamp(ivec2(base), ivec2(0), res - ivec2(2));
    int xw = int((px.x - base.x) * 255.0);
    int yw = int((px.y - base.y) * 255.0);
    ivec3 c00 = ivec3(texelFetch(uTex, i0, 0).rgb * 255.0);
    ivec3 c10 = ivec3(texelFetch(uTex, i0 + ivec2(1, 0), 0).rgb * 255.0);
    ivec3 c01 = ivec3(texelFetch(uTex, i0 + ivec2(0, 1), 0).rgb * 255.0);
    ivec3 c11 = ivec3(texelFetch(uTex, i0 + ivec2(1, 1), 0).rgb * 255.0);
    ivec3 top = (c00 * (255 - xw) + c10 * xw) >> 8;
    ivec3 bot = (c01 * (255 - xw) + c11 * xw) >> 8;
    return vec3((top * (255 - yw) + bot * yw) >> 8) / 255.0;
}

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
        // `subpixel` aus = naechster Bildpunkt (die Surface filtert NEAREST),
        // an = die vier Nachbarn wie im Original (r_ddm.cpp:313).
        res = uSubpixel == 1 ? avsBlend4(uv * uRes - vec2(0.5))
                             : texture(uTex, uv).rgb;
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
uniform sampler2D uTex;   // Eingabe (Puffer VOR dem Effekt)
uniform sampler2D uProc;  // BEARBEITETER Puffer (Pass 1); in Pass 1 == uTex
uniform vec2 uRes;
uniform float uKernel[49];
uniform float uScale;     // Teiler; 0 ist in der Laufzeit schon durch 1 ersetzt
uniform float uBias;      // Vorzeichen-BYTE des Konfigurationswerts (s. unten)
uniform int uAbsolute;
uniform int uTwoPass;
uniform int uEdge;
out vec4 fragColor;
void main()
{
    // Rand-Semantik der APE, aus dem JIT-Quelltext (tom holden, 2002) und an
    // Gradient-Sonden gemessen (S59):
    //  - Reads KLEMMEN auf Zeile h-2 und Spalte w-2 — die letzte Eingabe-
    //    Zeile/Spalte wird NIE gelesen (c[6] laedt beim Vorruecken den alten
    //    Zeiger zurueck, die Spaltenvarianten emittieren min(.., width-2)).
    //  - Geschrieben wird JEDES Pixel; ein separater Rand-Pass existiert nicht.
    //  - Liegt der erste Kernel-Eintrag ab dem ZENTRUM (zerostringl >= 24),
    //    schreibt die APE IN-PLACE: am unteren/rechten Rand lesen die
    //    zurueckgeklemmten Taps dann schon BEARBEITETE Pixel (row-major vor
    //    dem Schreibkopf) — die "doppelt gefaltete" Kante der Sonden.
    //    Der Host bildet das als zweiten Pass ab; `uProc` liefert Pass 1.
    // AVS-Pixelkoordinaten des Ziels (Zeilen von OBEN gezaehlt)
    float curCol = floor(gl_FragCoord.x);
    float curRow = uRes.y - 1.0 - floor(gl_FragCoord.y);
    vec3 sum = vec3(0.0);
    // Fuer den wrap-Pfad laufen die positiven und negativen Tap-Summen
    // GETRENNT mit (die MMX-Register mm0/mm1 des Originals); `sum` bleibt in
    // der alten Reihenfolge akkumuliert, damit der Normal-Pfad byte-identisch
    // zu den 33 kalibrierten S58-Sonden bleibt.
    vec3 possum = vec3(0.0);
    vec3 negsum = vec3(0.0);
    for (int j = 0; j < 7; j++)
        for (int i = 0; i < 7; i++)
        {
            float k = uKernel[j * 7 + i];
            // Kernzeile j zaehlt wie im Dialog von OBEN nach unten (Befund
            // S50: vertikal seitenverkehrt war messbar, horizontal nicht).
            float tc = curCol + float(i - 3);
            float tr = curRow + float(j - 3);
            // `wrap` ist KEIN Koordinaten-Wrap (Fehldeutung S57): der
            // JIT-Quelltext waehlt damit nur psubw statt psubusw — die
            // NEGATIV-Verrechnung laeuft ueber statt zu saettigen (deshalb
            // exklusiv mit `absolute`). Die Read-Koordinaten klemmen IMMER.
            float cc = clamp(tc, 0.0, uRes.x - 2.0);
            float cr = clamp(tr, 0.0, uRes.y - 2.0);
            bool bearbeitet = (cr < curRow) ||
                              (cr == curRow && cc < curCol);
            vec2 uv = vec2((cc + 0.5) / uRes.x, 1.0 - (cr + 0.5) / uRes.y);
            vec3 tap = (bearbeitet ? texture(uProc, uv).rgb
                                   : texture(uTex, uv).rgb);
            sum += tap * k;
            if (k > 0.0) possum += tap * k;
            else if (k < 0.0) negsum += tap * (-k);
        }
    // wrap-Arithmetik, an der ORIGINAL-APE vermessen (S60, Sonden
    // convolution_wrap_neg/_scale + scale-2/4/128-Grenztest):
    //  - scale == 1: identisch zur Saettigung (Negative -> 0) — der JIT
    //    emittiert dort keinen Divisionspfad, wrap ist wirkungslos.
    //  - scale >= 2: lane16 = (pos - neg) mod 65536, dann UNSIGNED-Division
    //    durch scale (Belege: Untergrund 16: -16 -> 65520/256 = 255;
    //    Linienpixel -494 -> 65042/256 = 254 — beide exakt getroffen).
    //  - exklusiv mit `absolute` (Dialog-Regel der APE).
    //  - bias mit wrap ist UNVERMESSEN; er wird wie im Normal-Pfad vor der
    //    Division addiert (bias=0 in allen Messfaellen).
    if (uEdge == 1 && uAbsolute == 0 && uScale > 1.5)
    {
        ivec3 p16 = min(ivec3(round(possum * 255.0)), ivec3(65535));
        ivec3 n16 = min(ivec3(round(negsum * 255.0)), ivec3(65535));
        ivec3 wr = (p16 - n16) & ivec3(65535);
        ivec3 qi = (wr + ivec3(int(uBias * 256.0))) / ivec3(int(uScale));
        if (uTwoPass == 1) qi *= 2;
        fragColor = vec4(vec3(clamp(qi, ivec3(0), ivec3(255))) / 255.0, 1.0);
        return;
    }
    // Die APE rechnet in BYTES und ganzzahlig (gemessen S58, Sonden
    // `2_trans/convolution_*` gegen AvsRef):
    //     x = (summe + bias * 256) / scale      <- Ganzzahl, Richtung Null
    // `bias` ist also KEIN Offset von wenigen Stufen, sondern zaehlt in ganzen
    // 256ern: schon bias=1 hebt jeden Kanal ueber 255 (weiss), bias=-1 drueckt
    // ihn unter 0. Belegt an (200,100,40): bias=1/scale=2 -> (228,178,148) =
    // (v+256)/2, bias=1/scale=256 -> (1,1,1). Wir addierten bias/255 — bei
    // Alternate Reality war das der Unterschied zwischen Weiss und Schwarz.
    vec3 t = (round(sum * 255.0) + vec3(uBias * 256.0)) / uScale;
    vec3 q = sign(t) * floor(abs(t) + 0.001);
    // `twoPass` VERDOPPELT das Ergebnis, es faltet nicht zweimal: mit Kern 1
    // und scale 2 misst die Referenz v (nicht v/4), mit scale 4 misst sie v/2.
    if (uTwoPass == 1) q *= 2.0;
    // Negativ heisst NICHT Betrag: mit `absolute` misst die Referenz 255
    // (Kern -1, scale 1 -> weiss; Kern -4, scale 2 -> weiss statt |-4v/2|),
    // ohne `absolute` schwarz.
    vec3 neg = step(q, vec3(-0.5));
    vec3 negOut = uAbsolute == 1 ? vec3(255.0) : vec3(0.0);
    fragColor = vec4(mix(min(q, vec3(255.0)), negOut, neg) / 255.0, 1.0);
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
// uUv0/uUv1 sind die Texturkoordinaten der Quad-Ecken aPos=(-1,-1) bzw. (1,1) —
// Texer II braucht ein EXAKTES, asymmetrisches UV-Fenster (Subpixel-Phase des
// Original-Resamplers, S59); Texer I setzt (0,1)/(1,0) und verhaelt sich wie
// vorher (der y-Flip steckt jetzt in den Ecken, nicht mehr im Fragment-Shader).
const char* kSpriteVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform vec2 uCenter;
uniform vec2 uHalf;
uniform vec2 uUv0;
uniform vec2 uUv1;
out vec2 vTex;
void main()
{
    vTex = mix(uUv0, uUv1, aPos * 0.5 + 0.5);
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
    vec4 c = texture(uImg, vTex);
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
    // Betriebsarten 0..2 wie bisher (Picture, ID 34), 3..5 fuer Picture II —
    // dessen APE hat SECHS, an der Referenz gemessen (S58, Sonden `p2_bm*`):
    // 0 ersetzen · 1 additiv · 2 Maximum · 3 Minimum · 4 50/50 · 5 Subtraktion
    // (Framebuffer minus Bild). Der Import bildet darauf ab; wir hatten alles
    // ab 2 pauschal als 50/50 genommen.
    vec3 r;
    if (uBlend == 1)      r = min(fb + img, vec3(1.0));
    else if (uBlend == 2)                                  // 50/50 = BLEND_AVG
        r = (floor(round(fb * 255.0) * 0.5) + floor(round(img * 255.0) * 0.5)) / 255.0;
    else if (uBlend == 3) r = max(fb, img);
    else if (uBlend == 4) r = min(fb, img);
    else if (uBlend == 5) r = max(fb - img, vec3(0.0));
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
// vec2 statt ivec2: Qt uebertraegt QPoint-Uniforms als FLOATS (glUniform2fv)
// — auf ein ivec2 gesetzt blieb das Uniform (0,0) und der Effekt war ein
// Passthrough (S48-Matrix-Befund 23).
uniform vec2 uSpacing;
uniform vec3 uColor;
uniform int uBlend;
out vec4 fragColor;
void main()
{
    vec3 c = texture(uTex, vTex).rgb;
    int px = int(vTex.x * uRes.x);
    int py = int(vTex.y * uRes.y);
    int tx = int(uSpacing.x + 0.5);
    int ty = int(uSpacing.y + 0.5);
    bool colored = false;
    if (ty > 0)
    {
        // r_interleave: yp startet bei (h%ty)/2 und wird VOR dem Vergleich
        // inkrementiert — der erste Farbblock ist eine Zeile KUERZER (+1).
        // py zaehlt hier in AVS-Richtung (Zeile 0 = oben): Texturzeilen sind
        // bottom-up, daher h-1-py_gl.
        int row = int(uRes.y) - 1 - py;
        int yy = row + (int(uRes.y) % ty) / 2 + 1;
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
    // Key + Tabellen-Abgriff in GANZZAHLEN (S49, gegen colormap.ape gemessen):
    // die APE indiziert tab[key] mit key aus den 8-Bit-Kanaelen. Float-Rechnung
    // plus GL_LINEAR-Abgriff verschob den Index um ein halbes Texel und
    // verschliff die Tabelle — beides sichtbar in Rueckkopplungsketten.
    ivec3 ci = ivec3(c * 255.0 + 0.5);
    int idx;
    if (uKey == 0)      idx = ci.r;
    else if (uKey == 1) idx = ci.g;
    else if (uKey == 2) idx = ci.b;
    else if (uKey == 3) idx = (ci.r + ci.g + ci.b) / 2;
    else if (uKey == 4) idx = max(max(ci.r, ci.g), ci.b);
    else                idx = (ci.r + ci.g + ci.b) / 3;
    ivec3 mi = ivec3(texelFetch(uLut, ivec2(clamp(idx, 0, 255), 0), 0).rgb * 255.0 + 0.5);
    // Mischung ebenfalls in Ganzzahlen, nach den AVS-Makros (r_defs.h):
    // BLEND_AVG = (a>>1)+(b>>1) (verliert Bit 0), BLEND_ADJ = (a*v+b*(255-v))>>8.
    ivec3 ri;
    if (uBlend == 0)      ri = mi;
    else if (uBlend == 1) ri = min(ci + mi, ivec3(255));
    else if (uBlend == 2) ri = max(ci, mi);
    else if (uBlend == 3) ri = min(ci, mi);
    else if (uBlend == 4) ri = (ci + mi) >> 1;  // echter Mittelwert, NICHT BLEND_AVG
    else if (uBlend == 5) ri = max(ci - mi, ivec3(0));
    else if (uBlend == 6) ri = max(mi - ci, ivec3(0));
    else if (uBlend == 7) ri = (ci * mi) >> 8;
    else if (uBlend == 8) ri = ci ^ mi;
    else
    {
        // gemessen: Gewichte a und 255-a, aber GERUNDET (+128) — anders als
        // BLEND_ADJ im Kern, das abschneidet.
        int a = clamp(int(uAdjust * 255.0 + 0.5), 0, 255);
        ri = (mi * a + ci * (255 - a) + 128) >> 8;
    }
    fragColor = vec4(vec3(clamp(ri, ivec3(0), ivec3(255))) / 255.0, 1.0);
}
)";

// AVS "Trans / Water Bump" (ID 31): height-field wave propagation — seit S59
// in den GANZZAHL-EinHEITEN der Referenz (r_waterbump.cpp). Der Hoehenpuffer
// packt .r = aktuelle Seite (buffers[page]), .g = vorige Seite; beide Shader
// teilen sich den SineBlob-Beitrag als Funktion (statt eines eigenen Passes),
// weil die Referenz den Tropfen VOR Displacement UND CalcWater in die
// aktuelle Seite addiert. Alle Werte sind ints in float32 (exakt bis 2^24;
// der FBO ist deshalb RGBA32F, nicht 16F).
//
// SineBlob (r_waterbump.cpp:128-165): innerhalb des (CPU-seitig auf den Rand
// geclippten, ENDE-EXKLUSIVEN) Rechtecks und des Radius addiert er
//   (int)((cos(dist)+0xffff)*hoehe) >> 19   [hoehe = -depth, arithm. Shift]
// mit dist = sqrt(square*(1024/radius)^2).
#define WB_BLOB_GLSL \
    "uniform int uDrop;\n" \
    "uniform ivec2 uDropPos;\n"  /* AVS-Pixel des Zentrums */ \
    "uniform ivec4 uDropClip;\n" /* left, top, right, bottom (relativ) */ \
    "uniform int uDropRad2;\n" \
    "uniform int uDropHeight;\n" \
    "uniform float uDropLen;\n" \
    "float blobAt(ivec2 avsP)\n" \
    "{\n" \
    "    if (uDrop == 0) return 0.0;\n" \
    "    ivec2 rel = avsP - uDropPos;\n" \
    "    if (rel.x < uDropClip.x || rel.x >= uDropClip.z ||\n" \
    "        rel.y < uDropClip.y || rel.y >= uDropClip.w) return 0.0;\n" \
    "    int square = rel.x * rel.x + rel.y * rel.y;\n" \
    "    if (square >= uDropRad2) return 0.0;\n" \
    "    float dist = sqrt(float(square) * uDropLen);\n" \
    "    float t = trunc((cos(dist) + 65535.0) * float(uDropHeight));\n" \
    "    return floor(t / 524288.0);\n" /* >> 19, arithmetisch */ \
    "}\n"

// CalcWater (r_waterbump.cpp:202-233): nur der INNENBEREICH [1..w-2]x[1..h-2]
// wird geschrieben — der Rand bleibt fuer immer 0 (feste Wand, S57):
//   newh = (sum8(alt) >> 2) - vorher;  neu = newh - (newh >> density)
const char* kWaterBumpPropShader =
    "#version 330 core\n"
    "in vec2 vTex;\n"
    "uniform sampler2D uH;\n"
    "uniform ivec2 uResI;\n"
    "uniform int uDensity;\n"
    WB_BLOB_GLSL
    "out vec4 fragColor;\n"
    "float curAt(ivec2 avsP)\n"
    "{\n"
    "    ivec2 tp = ivec2(avsP.x, uResI.y - 1 - avsP.y);\n"
    "    return texelFetch(uH, tp, 0).r + blobAt(avsP);\n"
    "}\n"
    "void main()\n"
    "{\n"
    "    ivec2 tp = ivec2(gl_FragCoord.xy);\n"
    "    ivec2 p = ivec2(tp.x, uResI.y - 1 - tp.y);\n"  // AVS: Zeilen von oben
    "    float cur = curAt(p);\n"
    "    if (p.x < 1 || p.y < 1 || p.x > uResI.x - 2 || p.y > uResI.y - 2)\n"
    "    {\n"
    "        fragColor = vec4(0.0, cur, 0.0, 1.0);\n"
    "        return;\n"
    "    }\n"
    "    float s = curAt(p + ivec2( 1, 0)) + curAt(p + ivec2(-1, 0))\n"
    "            + curAt(p + ivec2( 0, 1)) + curAt(p + ivec2( 0,-1))\n"
    "            + curAt(p + ivec2( 1, 1)) + curAt(p + ivec2(-1, 1))\n"
    "            + curAt(p + ivec2( 1,-1)) + curAt(p + ivec2(-1,-1));\n"
    "    float prev = texelFetch(uH, tp, 0).g;\n"
    "    float nh = floor(s / 4.0) - prev;\n"        // sum8 >> 2 (floor = arithm.)
    "    nh = nh - floor(nh / float(1 << uDensity));\n"  // newh - (newh >> density)
    "    fragColor = vec4(nh, cur, 0.0, 1.0);\n"
    "}\n";

// Water-Bump-Displacement (r_waterbump.cpp:315-347): GANZZAHLIGER, LINEARER
// Pufferversatz — `ofs = offset + w*(dy>>3) + (dx>>3)`; ein grosser x-Versatz
// laeuft dabei in die NACHBARZEILE ueber (linearer Puffer, kein 2D-Clamp).
// Nur [1..w-2]x[1..h-2] wird versetzt; ausserhalb der Grenzen ([0,len)) und
// am Rand kopiert die Referenz die Eingabe.
const char* kWaterBumpDispShader =
    "#version 330 core\n"
    "in vec2 vTex;\n"
    "uniform sampler2D uImg;\n"
    "uniform sampler2D uH;\n"
    "uniform ivec2 uResI;\n"
    "uniform float uScale;\n"
    WB_BLOB_GLSL
    "out vec4 fragColor;\n"
    "float curAt(ivec2 avsP)\n"
    "{\n"
    "    ivec2 tp = ivec2(avsP.x, uResI.y - 1 - avsP.y);\n"
    "    return texelFetch(uH, tp, 0).r + blobAt(avsP);\n"
    "}\n"
    "vec3 imgAt(ivec2 avsP)\n"
    "{\n"
    "    return texelFetch(uImg, ivec2(avsP.x, uResI.y - 1 - avsP.y), 0).rgb;\n"
    "}\n"
    "void main()\n"
    "{\n"
    "    ivec2 tp = ivec2(gl_FragCoord.xy);\n"
    "    ivec2 p = ivec2(tp.x, uResI.y - 1 - tp.y);\n"
    "    if (p.x < 1 || p.y < 1 || p.x > uResI.x - 2 || p.y > uResI.y - 2)\n"
    "    {\n"
    "        // Rand: die Referenz beschreibt fbout hier NIE — dort steht der\n"
    "        // Eingabepuffer des VORframes (1-Frame-Lag). Wir kopieren die\n"
    "        // AKTUELLE Eingabe: der Lag-Nachbau hat in S59 den ersten Frame\n"
    "        // verschluckt und die Trail-Messung verschlechtert — bewusste\n"
    "        // Naeherung, s. Offene_Punkte.\n"
    "        fragColor = vec4(imgAt(p), 1.0);\n"
    "        return;\n"
    "    }\n"
    "    float dx = trunc((curAt(p) - curAt(p + ivec2(1, 0))) * uScale);\n"
    "    float dy = trunc((curAt(p) - curAt(p + ivec2(0, 1))) * uScale);\n"
    "    float ofs = float(p.y * uResI.x + p.x)\n"
    "              + float(uResI.x) * floor(dy / 8.0) + floor(dx / 8.0);\n"
    "    vec3 c;\n"
    "    if (ofs >= 0.0 && ofs < float(uResI.x * uResI.y))\n"
    "    {\n"
    "        float row = floor(ofs / float(uResI.x));\n"
    "        float col = ofs - row * float(uResI.x);\n"
    "        c = imgAt(ivec2(int(col), int(row)));\n"
    "    }\n"
    "    else c = imgAt(p);\n"
    "    fragColor = vec4(c, 1.0);\n"
    "}\n";

// AVS "Render / Timescope" (ID 39): one spectrum column, tinted by uColor.
// Drawn scissored to a single x each frame (r_timescope.cpp).
// AVS Timescope (r_timescope.cpp:143): c = visdata[0][0][(i*nbands)/h] —
// das ROHE Spektrum-Byte (0..255) des LINKEN Kanals, Farbe = color*c/256.
// uSpec ist die 576-Byte-R8-Textur der visdata (S48-Matrix-Befund 39: das
// normalisierte App-Spektrum war viel zu dunkel + falsch gemappt). i ist
// die AVS-Zeile (0 = oben) — vTex.y laeuft bottom-up, daher 1-vTex.y.
const char* kTimescopeFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uSpec;
uniform vec3 uColor;
uniform float uBands;
uniform float uH;
out vec4 fragColor;
void main()
{
    float i = floor((1.0 - vTex.y) * uH);
    float idx = floor(i * uBands / uH);
    float c = texelFetch(uSpec, ivec2(int(idx), 0), 0).r;  // Byte/255
    fragColor = vec4(uColor * c * (255.0 / 256.0), 1.0);
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
    else                     // multiplier (r_multiplier.cpp: MD_XI..MD_XS)
    {
        if (uMode == 1)      r = c * 8.0;
        else if (uMode == 2) r = c * 4.0;
        else if (uMode == 3) r = c * 2.0;
        else if (uMode == 4) r = c * 0.5;
        else if (uMode == 5) r = c * 0.25;
        else if (uMode == 6) r = c * 0.125;
        // S46 (Anemone): MD_XI (0) = jeder Pixel != 0 wird VOLL WEISS —
        // das AVS-Idiom "fast-schwarz zeichnen, XI macht sichtbar";
        // MD_XS (7) = nur exakt 0xFFFFFF ueberlebt, Rest -> schwarz.
        else if (uMode == 0)
            r = any(greaterThan(c, vec3(0.0))) ? vec3(1.0) : vec3(0.0);
        else
            r = all(greaterThanEqual(c, vec3(254.5 / 255.0))) ? vec3(1.0)
                                                              : vec3(0.0);
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
/// Gewicht des ALTEN Bildes in der Betriebsart 2 (0 = nur das Fraktal,
/// 1 = nur der Framebuffer). 0,5 ist das benannte 50/50 — bis S57 rechnete
/// dieser Zweig fest damit, weshalb `fractalZoomer.feedback` („trail
/// persistence 0..1") nur ein SCHALTER war: 0,3 und 1,0 ergaben dasselbe Bild.
uniform float uFeedback;
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
        else             fragColor = vec4(mix(col, d, uFeedback), 1.0);
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
    const QString bare = QFileInfo(fname).fileName();

    // Nicht nur neben dem Preset suchen, sondern auch in den Elternordnern:
    // AVS legt seine Bilder im AVS-WURZELVERZEICHNIS ab, die Presets aber in
    // Unterordnern. In der Sammlung liegen z. B. "j10_2.bmp" (Texer II) und
    // "whackorev-soleillevant.jpg" (Picture II) in `avs/`, waehrend das Preset
    // in `avs/Whacko Revisited/` steht — mit nur einer Ebene fand der Import
    // sie nie und meldete "image not found" (Befund S50).
    QDir dir(baseDir);
    for (int level = 0; level <= 3; ++level)
    {
        for (const QString& cand : {dir.filePath(fname), dir.filePath(bare)})
        {
            if (!QFileInfo::exists(cand)) continue;
            QFile file(cand);
            if (file.open(QIODevice::ReadOnly))
            {
                imageData = file.readAll().toBase64().toStdString();
                break;
            }
        }
        if (!imageData.empty() || !dir.cdUp()) break;
    }

    // Zuletzt der in den Einstellungen gesetzte Bilder-Suchordner (S50-Vorgabe,
    // umgesetzt S53). Er greift NUR, wenn neben dem Preset nichts gefunden wurde
    // — ein Bild beim Preset bleibt also immer das massgebliche.
    if (imageData.empty())
    {
        QSettings settings;
        const QString extra =
            settings.value(QStringLiteral("import/imageSearchDir")).toString();
        if (!extra.isEmpty())
        {
            QDir sd(extra);
            for (const QString& cand : {sd.filePath(fname), sd.filePath(bare)})
            {
                if (!QFileInfo::exists(cand)) continue;
                QFile file(cand);
                if (file.open(QIODevice::ReadOnly))
                {
                    imageData = file.readAll().toBase64().toStdString();
                    break;
                }
            }
        }
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

/// AVI: videos are NOT embedded (size); resolve the bare preset name to an
/// absolute path — preset folder first, then up to 4 parent levels (asset
/// packs keep the .avi next to or above the preset folders, S43 pattern).
void resolveAviPaths(ChainNode& node, const QString& baseDir, QStringList* report)
{
    if (auto* p = std::get_if<AviParams>(&node.params))
    {
        if (p->resolvedPath.empty() && !p->filename.empty())
        {
            const QString fname =
                QFileInfo(QString::fromStdString(p->filename)).fileName();
            QDir dir(baseDir);
            for (int level = 0; level <= 4; ++level)
            {
                const QString candidate = dir.filePath(fname);
                if (QFileInfo::exists(candidate))
                {
                    p->resolvedPath = candidate.toStdString();
                    break;
                }
                if (!dir.cdUp()) break;
            }
            if (p->resolvedPath.empty() && report != nullptr)
                report->append(QStringLiteral("AVI: video not found: %1")
                                   .arg(QString::fromStdString(p->filename)));
        }
    }
    for (ChainNode& child : node.children) resolveAviPaths(child, baseDir, report);
}
}  // namespace

bool MultiEffectVisualizer::loadAvsFile(const QString& path, QStringList* outReport)
{
    const lumi::avs::ParseResult parsed =
        lumi::avs::parseFile(std::filesystem::path(path.toStdWString()));
    TranslationResult translated = translateAvsTree(parsed);

    // PROBLEME sammeln (Parser, Passthrough, fehlende Dateien). Die planmaessigen
    // HINWEISE (translated.notes) bleiben bewusst draussen: sie gehen in den
    // Import-Notes-Knoten, nicht in den Dialog (Entscheid Patrik S51).
    QStringList problems;
    for (const std::string& line : translated.report)
    {
        problems.append(QString::fromStdString(line));
    }
    // Resolve + embed Picture images relative to the .avs directory (appends any
    // "image not found" notes after the translation report).
    embedPictureImages(translated.root, QFileInfo(path).absolutePath(), &problems);
    resolveAviPaths(translated.root, QFileInfo(path).absolutePath(), &problems);
    if (outReport != nullptr) *outReport = problems;

    // Das vollstaendige Protokoll als Knoten IN der Kette — Zusammenfassung,
    // dann Probleme, dann Hinweise. Bewusst OHNE Zeitstempel: das JSON eines
    // Imports muss reproduzierbar bleiben (die eingefrorenen .lvfx-Zwillinge
    // wuerden sich sonst bei jedem Lauf unterscheiden).
    //
    // Nur wenn es etwas zu protokollieren gibt: ein sauberer Import soll keine
    // leere Notiz in jede Kette haengen. Die ANWESENHEIT des Knotens ist damit
    // selbst das Signal.
    if (!problems.isEmpty() || !translated.notes.empty())
    {
        QStringList lines;
        lines << QStringLiteral("Import: %1").arg(QFileInfo(path).fileName());
        lines << QStringLiteral("%1 Effekte, %2 Passthrough, %3 Problem(e), "
                                "%4 Hinweis(e)")
                     .arg(translated.effectCount)
                     .arg(translated.passthroughCount)
                     .arg(problems.size())
                     .arg(static_cast<int>(translated.notes.size()));
        if (!problems.isEmpty())
        {
            lines << QString() << QStringLiteral("--- Probleme ---");
            lines << problems;
        }
        if (!translated.notes.empty())
        {
            lines << QString() << QStringLiteral("--- Hinweise ---");
            for (const std::string& note : translated.notes)
            {
                lines << QString::fromStdString(note);
            }
        }
        ChainNode notes;
        notes.params = ImportNotesParams{lines.join('\n').toStdString()};
        translated.root.children.insert(translated.root.children.begin(),
                                        std::move(notes));
    }
    // Entscheid S47 (Variante 2): jeder AVS-Import bekommt einen Render-Scale-
    // Knoten als erstes Kind — Wert aus der App-Einstellung (1 = neutral, wie
    // ohne). Danach ist der Knoten im Preset die einzige Wahrheit.
    {
        ChainNode scale;
        scale.params = RenderScaleParams{m_importRenderScaleDivisor, 0};
        translated.root.children.insert(translated.root.children.begin(),
                                        std::move(scale));
    }
    // The new tree reuses node ids 1..N, colliding with the old runtimes; flag
    // a reset so the render thread frees their GL objects and starts fresh.
    m_pendingRuntimeReset = true;
    m_beatPeriodFrame = 0;  // --beat-period zaehlt je Preset ab 0 (wie AvsRef)
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

QImage MultiEffectVisualizer::debugGrabRootSurface() const
{
    return m_rootSurface.ready() ? m_rootSurface.current()->toImage() : QImage();
}

bool MultiEffectVisualizer::loadChainFile(const QString& path, QStringList* outReport)
{
    ChainNode loaded;
    if (!loadChainFromFile(path, loaded, outReport)) return false;
    m_pendingRuntimeReset = true;  // new node ids — free old GL runtimes (render thread)
    m_beatPeriodFrame = 0;  // --beat-period zaehlt je Preset ab 0 (wie AvsRef)
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
    m_scriptClock = 0.0;
    m_scriptClockAccum = 0.0;
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
    m_warpFxShader.reset();
    m_moveTabShader.reset();
    if (m_warpTabTex != 0)
    {
        QOpenGLContext::currentContext()->functions()->glDeleteTextures(1, &m_warpTabTex);
        m_warpTabTex = 0;
    }
    m_feedbackShader.reset();
    m_mosaicShader.reset();
    m_grainShader.reset();
    m_scatterShader.reset();
    m_interfShader.reset();
    m_waterShader.reset();
    m_bumpShader.reset();
    m_presentShader.reset();
    m_bloomDownShader.reset();
    m_bloomGaussShader.reset();
    m_bloomCompShader.reset();
    m_sprite3dShader.reset();
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
    m_convScratch.reset();
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
    m_sprite3dVao.reset();
    m_sprite3dVbo.reset();
    m_terrain3dShader.reset();
    m_orb3dShader.reset();
    m_flatShader.reset();
    m_orbVao.reset();
    m_orbVbo.reset();
    m_orbIbo.reset();
    m_triVao.reset();
    m_triVbo.reset();
    if (m_depth3dTex != 0)
    {
        if (auto* ctx = QOpenGLContext::currentContext())
            ctx->functions()->glDeleteTextures(1, &m_depth3dTex);
        m_depth3dTex = 0;
        m_depth3dW = 0;
        m_depth3dH = 0;
    }
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
    m_metaballShader = makeProgram(kQuadVertexShader, kMetaballFragmentShader);
    m_colorfadeShader = makeProgram(kQuadVertexShader, kColorfadeFragmentShader);
    m_lutShader = makeProgram(kQuadVertexShader, kLutFragmentShader);
    m_warpShader = makeProgram(kWarpVertexShader, kWarpFragmentShader);
    m_warpFxShader = makeProgram(kQuadVertexShader, kWarpFxFragmentShader);
    m_moveTabShader = makeProgram(kQuadVertexShader, kMoveTabFragmentShader);
    m_moveRemapShader = makeProgram(kQuadVertexShader, kMoveRemapFragmentShader);
    m_textShader = makeProgram(kQuadVertexShader, kTextFragmentShader);
    m_feedbackShader = makeProgram(kQuadVertexShader, kFeedbackFragmentShader);
    m_mosaicShader = makeProgram(kQuadVertexShader, kMosaicFragmentShader);
    m_grainShader = makeProgram(kQuadVertexShader, kGrainFragmentShader);
    m_scatterShader = makeProgram(kQuadVertexShader, kScatterFragmentShader);
    m_interfShader = makeProgram(kQuadVertexShader, kInterfFragmentShader);
    m_waterShader = makeProgram(kQuadVertexShader, kWaterFragmentShader);
    m_bumpShader = makeProgram(kQuadVertexShader, kBumpFragmentShader);
    m_presentShader = makeProgram(kQuadVertexShader, kPresentFragmentShader);
    m_bloomDownShader = makeProgram(kQuadVertexShader, kBloomDownFragmentShader);
    m_bloomGaussShader = makeProgram(kQuadVertexShader, kBloomGaussFragmentShader);
    m_bloomCompShader = makeProgram(kQuadVertexShader, kBloomCompFragmentShader);
    m_sprite3dShader = makeProgram(kSprite3DVertexShader, kSprite3DFragmentShader);
    m_terrain3dShader = makeProgram(kTerrain3DVertexShader, kTerrain3DFragmentShader);
    m_orb3dShader = makeProgram(kOrb3DVertexShader, kOrb3DFragmentShader);
    m_flatShader = makeProgram(kFlatVertexShader, kFlatFragmentShader);
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
        m_warpFxShader == nullptr || m_moveTabShader == nullptr ||
        m_feedbackShader == nullptr || m_mosaicShader == nullptr ||
        m_grainShader == nullptr || m_scatterShader == nullptr ||
        m_interfShader == nullptr || m_waterShader == nullptr ||
        m_bumpShader == nullptr || m_presentShader == nullptr ||
        m_bloomDownShader == nullptr || m_bloomGaussShader == nullptr ||
        m_bloomCompShader == nullptr || m_sprite3dShader == nullptr ||
        m_terrain3dShader == nullptr || m_orb3dShader == nullptr ||
        m_flatShader == nullptr ||
        m_shiftShader == nullptr ||
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
        m_presentShader.reset();
        m_bloomDownShader.reset();
        m_bloomGaussShader.reset();
        m_bloomCompShader.reset();
        m_sprite3dShader.reset();
        m_terrain3dShader.reset();
        m_orb3dShader.reset();
        m_flatShader.reset();
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

    // SuperScope 3D: dynamisches Sprite-Mesh (center.xy + corner.xy + half.xy
    // + rgb = 9 Floats je Vertex, 6 Vertices je Punkt; je Frame neu befuellt).
    m_sprite3dVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_sprite3dVao->create();
    m_sprite3dVao->bind();
    m_sprite3dVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_sprite3dVbo->create();
    m_sprite3dVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_sprite3dVbo->bind();
    m_sprite3dVbo->allocate(nullptr, 0);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                             reinterpret_cast<void*>(2 * sizeof(float)));
    f->glEnableVertexAttribArray(2);
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                             reinterpret_cast<void*>(4 * sizeof(float)));
    f->glEnableVertexAttribArray(3);
    f->glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                             reinterpret_cast<void*>(6 * sizeof(float)));
    m_sprite3dVao->release();
    m_sprite3dVbo->release();

    // Glow Orbs: geteilte Einheitskugel (16x12 lat-long, Position = Normale;
    // der Shader skaliert sie per Uniform zum Ellipsoid).
    {
        constexpr int kSeg = 16;   // Laengengrade
        constexpr int kRings = 12; // Breitengrade
        constexpr float kPi = 3.14159265358979f;
        std::vector<float> verts;
        verts.reserve((kRings + 1) * (kSeg + 1) * 3);
        for (int r = 0; r <= kRings; ++r)
        {
            const float phi = kPi * static_cast<float>(r) /
                              static_cast<float>(kRings);
            for (int s = 0; s <= kSeg; ++s)
            {
                const float theta = 2.0f * kPi *
                                    static_cast<float>(s) / static_cast<float>(kSeg);
                verts.push_back(std::sin(phi) * std::cos(theta));
                verts.push_back(std::cos(phi));
                verts.push_back(std::sin(phi) * std::sin(theta));
            }
        }
        std::vector<unsigned short> idx;
        idx.reserve(kRings * kSeg * 6);
        for (int r = 0; r < kRings; ++r)
        {
            for (int s = 0; s < kSeg; ++s)
            {
                const unsigned short a =
                    static_cast<unsigned short>(r * (kSeg + 1) + s);
                const unsigned short b = static_cast<unsigned short>(a + kSeg + 1);
                idx.insert(idx.end(), {a, b, static_cast<unsigned short>(a + 1),
                                       static_cast<unsigned short>(a + 1), b,
                                       static_cast<unsigned short>(b + 1)});
            }
        }
        m_orbIndexCount = static_cast<int>(idx.size());

        m_orbVao = std::make_unique<QOpenGLVertexArrayObject>();
        m_orbVao->create();
        m_orbVao->bind();
        m_orbVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
        m_orbVbo->create();
        m_orbVbo->bind();
        m_orbVbo->allocate(verts.data(),
                           static_cast<int>(verts.size() * sizeof(float)));
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                                 nullptr);
        m_orbIbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
        m_orbIbo->create();
        m_orbIbo->bind();
        m_orbIbo->allocate(idx.data(),
                           static_cast<int>(idx.size() * sizeof(unsigned short)));
        m_orbVao->release();
        m_orbVbo->release();
        m_orbIbo->release();
    }

    // Flat-Fill-Dreiecke (my_triangle-Ersatz): dynamisches pos.xy-Mesh.
    m_triVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_triVao->create();
    m_triVao->bind();
    m_triVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_triVbo->create();
    m_triVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_triVbo->bind();
    m_triVbo->allocate(nullptr, 0);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    m_triVao->release();
    m_triVbo->release();

    m_scopeRenderer.ensure();  // shared scope draw (SuperScope effect, E6)
    return true;
}

void MultiEffectVisualizer::drawFlatTriangles(const std::vector<float>& xyNdc,
                                              const QVector3D& color)
{
    if (m_flatShader == nullptr || xyNdc.size() < 6) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glDisable(GL_BLEND);  // my_triangle zeichnet REPLACE
    m_flatShader->bind();
    m_flatShader->setUniformValue("uColor", color);
    m_triVao->bind();
    m_triVbo->bind();
    m_triVbo->allocate(xyNdc.data(),
                       static_cast<int>(xyNdc.size() * sizeof(float)));
    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(xyNdc.size() / 2));
    m_triVbo->release();
    m_triVao->release();
    m_flatShader->release();
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
            if (rt.textTexture != 0) f->glDeleteTextures(1, &rt.textTexture);
            if (rt.aviTexture != 0) f->glDeleteTextures(1, &rt.aviTexture);
            if (rt.moveTabTex != 0) f->glDeleteTextures(1, &rt.moveTabTex);
            if (rt.grainTex != 0) f->glDeleteTextures(1, &rt.grainTex);
            closeAviRuntime(rt);  // VfW handles (no GL, but lifecycle-coupled)
            // Meganode: der Milkdrop-Kern gibt seine GL-Objekte selbst frei
            // (braucht den current Context — deshalb hier, nicht im Dtor)
            if (rt.milk != nullptr) rt.milk->cleanup();
        }
    }
    // Preset-Wechsel: der rand()-Strom faengt wieder bei Seed 1 an — AvsRef
    // startet je Preset einen frischen Prozess und ruft nie srand() (S49).
    if (m_scriptContext != nullptr) m_scriptContext->resetRandom();
    m_listRuntimes.clear();  // slot hosts / FBOs die with their GL-frame owner
    m_leafRuntimes.clear();
    m_groupRuntimes.clear();  // HG1: Gruppen-Surfaces/-Pools sterben mit
    m_bufferPool.clear();
    for (auto& ring : m_mdRing) ring.clear();  // Multi Delay shared rings
    for (int& head : m_mdHead) head = 0;
    m_mdW = 0;
    m_mdH = 0;
}

namespace
{
// HG2-Kurven-Hook (Definition bei renderHostGroup): render() braucht ihn
// schon fuer die Vorab-Summe der blendenden Gruppen-Gewichte.
double applyBlendCurve(int curve, double t);
}  // namespace

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

    // gettime()-Uhr: Stand VOR diesem Frame, double-akkumuliert (S59)
    m_scriptClock = m_scriptClockAccum;
    m_scriptClockAccum += static_cast<double>(deltaTime);
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
    if (m_beatPeriodOverride > 0)
    {
        // Deterministischer Beat wie AvsRef --beat-period: Frame 0, N, 2N …
        m_frameBeat = (m_beatPeriodFrame % m_beatPeriodOverride) == 0;
        ++m_beatPeriodFrame;
    }
    else
    {
        const float level =
            std::max(meanAbs(getWaveformChannel(0)), meanAbs(getWaveformChannel(1)));
        const bool onset = m_beat.updateAvsOnset(level);
        m_frameBeat =
            m_beatEstimator.refine(onset, lumi::modules::BeatEstimator::steadyNowMs());
    }

    // Working surface in physical pixels (the GL viewport is authoritative).
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);

    // Render Scale (LumiViz-Modul): erster aktivierter Knoten bestimmt die
    // interne Aufloesung (Fenster / divisor) + den Upscale-Filter des Presents.
    int scaleDivisor = 1;
    GLenum presentFilter = GL_NEAREST;
    {
        std::function<const RenderScaleParams*(const ChainNode&)> findScale =
            [&](const ChainNode& n) -> const RenderScaleParams* {
                if (!n.enabled) return nullptr;
                if (const auto* rs = std::get_if<RenderScaleParams>(&n.params))
                    return rs;
                for (const ChainNode& child : n.children)
                    if (const auto* rs = findScale(child)) return rs;
                return nullptr;
            };
        if (const RenderScaleParams* rs = findScale(m_root))
        {
            scaleDivisor = std::clamp(rs->divisor, 1, 8);
            presentFilter = rs->filter == 1 ? GL_LINEAR : GL_NEAREST;
        }
    }
    const int internalW = std::max(64, viewport[2] / scaleDivisor);
    const int internalH = std::max(64, viewport[3] / scaleDivisor);

    bool resized = false;
    if (!ensurePipelines() ||
        !ensureSurfacePair(m_rootSurface, internalW, internalH, &resized))
    {
        return;
    }
    m_surfaceWidth = internalW;
    m_surfaceHeight = internalH;
    if (resized) m_firstFrame = true;

    const GLboolean blendWasEnabled = f->glIsEnabled(GL_BLEND);
    f->glDisable(GL_BLEND);

    m_surfaceStack.clear();
    m_surfaceStack.push_back(&m_rootSurface);
    for (auto& [id, runtime] : m_listRuntimes) runtime.seenThisFrame = false;
    for (auto& [id, runtime] : m_groupRuntimes) runtime.seenThisFrame = false;

    // HG2: Vorab-Summe der BLENDENDEN Gruppen-Gewichte dieses Frames. Der
    // Gruppen-Mix normalisiert damit das sequentielle Adjustable-Compositing
    // auf den exakten paarweisen Mix (B*t + A*(1-t)) — unabhaengig von der
    // Ketten-Reihenfolge; Solo-Einblenden mischt gegen den Hintergrund.
    {
        double total = 0.0;
        std::function<void(const ChainNode&)> scanGroups =
            [&](const ChainNode& n) {
                if (const auto* hg = std::get_if<HostGroupParams>(&n.params))
                {
                    const auto it = m_groupRuntimes.find(n.nodeId);
                    if (it != m_groupRuntimes.end())
                    {
                        const double c = applyBlendCurve(
                            n.enabled ? hg->curveIn : hg->curveOut,
                            it->second.blendWeight);
                        if (c > 0.0 && c < 0.999) total += c;
                    }
                }
                for (const ChainNode& child : n.children) scanGroups(child);
            };
        scanGroups(m_root);
        m_blendRunningSum = std::max(0.0, 1.0 - total);
    }
    m_renderMode = RenderMode{};  // Set Render Mode is per-frame, reset before the walk
    m_camera3d = Camera3D{};      // 3D-Kamera je Frame auf die Fallback-Kamera
    m_depth3dCleared = false;     // gemeinsames Depth-RT je Frame frisch loeschen
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

    // Post-Bloom (S48, Lights-Etappe 1): der erste aktivierte Bloom-Knoten
    // mit post=true wirkt beim Present — der Glow wird JETZT aus der fertigen
    // Root-Surface erzeugt und nur in die Anzeige gemischt, die Surface (und
    // damit das Feedback des naechsten Frames) bleibt unberuehrt. Referenz
    // Lights: Bloom ist Post-Processing NACH dem Szenen-Render; in-chain
    // akkumulierte der additive Glow in Fadeout-Ketten bis Weiss.
    const BloomParams* postBloom = nullptr;
    /// Die Runtime des post-Bloom-Knotens — sie traegt die vom Parameter-Skript
    /// gerechneten Werte (S54); ohne Skript stehen dort die Preset-Werte.
    const LeafRuntime* postBloomRt = nullptr;
    unsigned int postGlowTex = 0;
    {
        std::function<const ChainNode*(const ChainNode&)> findBloom =
            [&](const ChainNode& n) -> const ChainNode* {
                if (!n.enabled) return nullptr;
                if (const auto* b = std::get_if<BloomParams>(&n.params))
                    return b->post ? &n : nullptr;
                for (const ChainNode& child : n.children)
                    if (const ChainNode* hit = findBloom(child)) return hit;
                return nullptr;
            };
        if (const ChainNode* hit = findBloom(m_root))
        {
            postBloom = std::get_if<BloomParams>(&hit->params);
            // Die vom Parameter-Skript gerechneten Werte, die `runBloom` vor
            // dem post-Ausstieg hinterlegt hat (S54) — ohne Skript sind es
            // die Preset-Werte.
            const LeafRuntime& brt = m_leafRuntimes[hit->nodeId];
            postBloomRt = &brt;
            postGlowTex =
                ensureBloomGlow(m_leafRuntimes[hit->nodeId], *postBloom,
                                m_rootSurface.current()->texture(),
                                brt.bloomThreshold, brt.bloomRadius);
        }
    }

    // Present: die Root-Surface als texturierter Quad aufs Fenster. KEIN
    // glBlitFramebuffer: das App-Fenster ist multisampled (samples=4) und ein
    // skalierender Blit in ein MS-Ziel wirft GL_INVALID_OPERATION — mit
    // Render-Scale fror das Bild ein (Befund S47). presentFilter setzt den
    // Upscale-Look (nearest = grob, linear = weich); danach Parameter zurueck.
    // Mit Post-Bloom uebernimmt der Composite-Shader den Present-Draw
    // (Base + Glow additiv + optionale Vignette in Fenster-Aufloesung).
    m_rootSurface.current()->release();
    f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    f->glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    if (m_presentShader != nullptr)
    {
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, m_rootSurface.current()->texture());
        GLint prevMin = GL_NEAREST;
        GLint prevMag = GL_NEAREST;
        f->glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &prevMin);
        f->glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &prevMag);
        const GLint filt = static_cast<GLint>(presentFilter);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt);

        if (postBloom != nullptr && postGlowTex != 0)
        {
            m_bloomCompShader->bind();
            m_bloomCompShader->setUniformValue("uBase", 0);
            f->glActiveTexture(GL_TEXTURE1);
            f->glBindTexture(GL_TEXTURE_2D, postGlowTex);
            m_bloomCompShader->setUniformValue("uGlow", 1);
            f->glActiveTexture(GL_TEXTURE0);
            // Intensitaet und Vignetten-Staerke aus der Runtime (Skriptwerte,
            // sonst Preset-Werte); der Vignetten-SCHALTER ist nicht skriptbar.
            m_bloomCompShader->setUniformValue(
                "uIntensity", std::max(0.0f, postBloomRt->bloomIntensity));
            m_bloomCompShader->setUniformValue("uVignette", postBloom->vignette);
            m_bloomCompShader->setUniformValue(
                "uVigStrength",
                std::clamp(postBloomRt->bloomVigStrength, 0.0f, 1.0f));
            m_quadVao->bind();
            f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_quadVao->release();
            m_bloomCompShader->release();
        }
        else
        {
            m_presentShader->bind();
            m_presentShader->setUniformValue("uTex", 0);
            m_quadVao->bind();
            f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_quadVao->release();
            m_presentShader->release();
        }

        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, prevMin);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, prevMag);
        f->glBindTexture(GL_TEXTURE_2D, 0);
    }

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
        //
        // Ebenso darf eine deaktivierte Liste MIT EEL-Code nicht hier schon
        // wegfallen: r_list.cpp:399 belegt `enabled` nur mit dem gespeicherten
        // Schalter VOR, danach gewinnt das Skript (r_list.cpp:419). Das ist ein
        // gaengiges Idiom — die Liste liegt ausgeschaltet im Preset und schaltet
        // sich per "enabled=bnot(equal(lw,w)*equal(lh,h))" im ersten Frame und
        // nach jedem Resize selbst ein, um einen Puffer einmalig zu fuellen
        // (Whacko Revisited, Befund S50). Wer hier abbricht, fuehrt den Code nie
        // aus und die Liste bleibt fuer immer aus.
        //
        // HG2: eine deaktivierte Host-Gruppe rendert weiter, solange ihr
        // Blend-Gewicht > 0 ist (Ausblend-Phase des Crossfades).
        const auto* list = std::get_if<ListParams>(&node.params);
        if (list == nullptr || (!list->onBeatRender && !list->useCode))
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
        void operator()(const ClearParams& params) const { self.runClear(node, params); }
        void operator()(const FadeoutParams& params) const { self.runFadeout(node, params); }
        void operator()(const InvertParams&) const { self.runInvert(); }
        void operator()(const BrightnessParams& params) const { self.runBrightness(node, params); }
        void operator()(const FastBrightnessParams& params) const { self.runFastBrightness(node, params); }
        void operator()(const BlurParams& params) const { self.runBlur(node, params); }
        void operator()(const MirrorParams& params) const { self.runMirror(node, params); }
        void operator()(const OnBeatClearParams& params) const { self.runOnBeatClear(node, params); }
        void operator()(const ColorfadeParams& params) const { self.runColorfade(node, params); }
        void operator()(const ColorModifierParams& params) const { self.runColorModifier(node, params); }
        void operator()(const MovementParams& params) const { self.runMovement(node, params); }
        void operator()(const DynamicMovementParams& params) const { self.runDynamicMovement(node, params); }
        void operator()(const DynamicShiftParams& params) const { self.runDynamicShift(node, params); }
        void operator()(const DynamicDistanceModifierParams& params) const { self.runDynamicDistanceModifier(node, params); }
        void operator()(const MovingParticleParams& params) const { self.runMovingParticle(node, params); }
        void operator()(const BlitterFeedbackParams& params) const { self.runBlitterFeedback(node, params); }
        void operator()(const RotoBlitterParams& params) const { self.runRotoBlitter(node, params); }
        void operator()(const BufferSaveParams& params) const { self.runBufferSave(node, params); }
        void operator()(const CustomBpmParams& params) const { self.runCustomBpm(node, params); }
        void operator()(const SetRenderModeParams& params) const { self.runSetRenderMode(params); }
        void operator()(const SuperScopeParams& params) const { self.runSuperScope(node, params); }
        void operator()(const MosaicParams& params) const { self.runMosaic(node, params); }
        void operator()(const GrainParams& params) const { self.runGrain(node, params); }
        void operator()(const ScatterParams& params) const { self.runScatter(params); }
        void operator()(const InterferencesParams& params) const { self.runInterferences(node, params); }
        void operator()(const WaterParams& params) const { self.runWater(node, params); }
        void operator()(const BumpParams& params) const { self.runBump(node, params); }
        void operator()(const WaterBumpParams& params) const { self.runWaterBump(node, params); }
        void operator()(const FyrewurXParams& params) const { self.runFyrewurX(node, params); }
        void operator()(const Metaballs3DParams& params) const { self.runMetaballs3D(node, params); }
        void operator()(const Tentacles3DParams& params) const { self.runTentacles3D(node, params); }
        void operator()(const StarfieldParams& params) const { self.runStarfield(node, params); }
        void operator()(const TimescopeParams& params) const { self.runTimescope(node, params); }
        void operator()(const DotGridParams& params) const { self.runDotGrid(node, params); }
        void operator()(const DotPlaneParams& params) const { self.runDotPlane(node, params); }
        void operator()(const DotFountainParams& params) const { self.runDotFountain(node, params); }
        void operator()(const ColorMapParams& params) const { self.runColorMap(node, params); }
        void operator()(const BufferBlendParams& params) const { self.runBufferBlend(node, params); }
        void operator()(const JherikoGlobalParams& params) const { self.runJherikoGlobal(node, params); }
        void operator()(const ColorClipParams& params) const { self.runColorClip(node, params); }
        void operator()(const UniqueToneParams& params) const { self.runUniqueTone(node, params); }
        void operator()(const InterleaveParams& params) const { self.runInterleave(node, params); }
        void operator()(const ConvolutionParams& params) const { self.runConvolution(node, params); }
        void operator()(const NormaliseParams&) const { self.runNormalise(); }
        void operator()(const MultiFilterParams& params) const { self.runMultiFilter(node, params); }
        void operator()(const AddBordersParams& params) const { self.runAddBorders(node, params); }
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
        void operator()(const ColorReductionParams& params) const { self.runColorReduction(node, params); }
        void operator()(const MultiplierParams& params) const { self.runMultiplier(node, params); }
        void operator()(const VideoDelayParams& params) const { self.runVideoDelay(node, params); }
        void operator()(const MultiDelayParams& params) const { self.runMultiDelay(node, params); }
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
        void operator()(const TextParams& params) const { self.runText(node, params); }
        void operator()(const AviParams& params) const { self.runAvi(node, params); }
        void operator()(const CommentParams&) const { /* annotation, no-op */ }
        void operator()(const ImportNotesParams&) const { /* Import-Protokoll, no-op */ }
        void operator()(const RenderScaleParams&) const { /* pre-frame state, no-op */ }
        void operator()(const BloomParams& params) const { self.runBloom(node, params); }
        void operator()(const Camera3DParams& params) const { self.runCamera3D(node, params); }
        void operator()(const SuperScope3DParams& params) const { self.runSuperScope3D(node, params); }
        void operator()(const Terrain3DParams& params) const { self.runTerrain3D(node, params); }
        void operator()(const GlowOrbsParams& params) const { self.runGlowOrbs(node, params); }
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

    // Der Beat gehoert der LISTE, nicht dem Frame. In r_list.cpp:747-751 wirken
    // die Rueckgaben SET_BEAT/CLR_BEAT eines Kindes auf die lokale Variable
    // `isBeat` des Listen-render() — also nur auf die NACHFOLGENDEN Kinder
    // DIESER Liste; der Elternteil sieht davon nichts. Dasselbe gilt fuer das
    // `beat` des Listen-Skripts. Wir haben bisher `m_frameBeat` global
    // umgestellt: bei "Alternate Reality" filtert ein Custom BPM in einer
    // Unterliste jeden vierten Beat heraus, und dieser gefilterte Beat lief
    // anschliessend durch die GANZE Kette weiter. Die Dynamic Movements
    // dahinter zogen dadurch nur bei jedem vierten Beat ihre acht `rand(4)`,
    // die Referenz bei jedem — der geteilte Zufallsstrom lief auseinander
    // (Befund S58).
    struct BeatBereich
    {
        bool* ziel;
        bool alt;
        ~BeatBereich() { *ziel = alt; }
    } beatBereich{&m_frameBeat, m_frameBeat};

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
            // r_list.cpp:399-402: `enabled` und `clear` werden JE FRAME aus dem
            // gespeicherten Schalter vorbelegt — nicht einmalig beim Compile.
            // Erst danach darf das Skript sie umschreiben (:419-420).
            engine.setNumber("enabled", node.enabled ? 1.0 : 0.0);
            engine.setNumber("clear", params.clearEveryFrame ? 1.0 : 0.0);
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
    // S3 (r_list.cpp:693-694/744): jede Liste rettet den Linien-Blend-Modus,
    // setzt ihn beim Eintritt auf REPLACE zurueck und stellt ihn am Ende
    // wieder her — ein Set Render Mode wirkt nur bis zum Ende SEINER Liste.
    const RenderMode savedRenderMode = m_renderMode;
    m_renderMode = RenderMode{};
    m_surfaceStack.push_back(&runtime.surface);
    bindActive();
    for (const ChainNode& child : node.children)
    {
        renderNode(child);
    }
    active().current()->release();
    m_surfaceStack.pop_back();
    m_renderMode = savedRenderMode;

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
/// HG2-Kurven-Hook — Ein-/Ausgangskurve bewusst je Gruppe individuell
/// (Entwurf §2.4). Indizes muessen zur curveNames-Liste im Panel passen.
double applyBlendCurve(int curve, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    switch (curve)
    {
        case 1: return t * t * (3.0 - 2.0 * t);      // S-Kurve (smoothstep)
        case 2: return t * t;                        // Ease-In (sanfter Start)
        case 3: return 1.0 - (1.0 - t) * (1.0 - t);  // Ease-Out (sanftes Ende)
        case 4: return t * t * t;                    // Exponentiell (spaet)
        default: return t;                           // 0 = linear
    }
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
            runtime.activeSeconds = 0.0;  // HG3: progress startet neu
            return;
        }
        if (node.enabled)
        {
            runtime.activeSeconds += static_cast<double>(m_deltaTime);
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
    const double prevGroupSeconds = m_groupActiveSeconds;
    m_activePool = runtime.pool.get();
    m_activeContext = runtime.context;
    m_groupActiveSeconds = runtime.activeSeconds;

    // HG3: Freeze-Frame — NUR automatischer Performance-Fallback (Entscheid
    // E5): bricht die Framerate waehrend eines Blends ein, friert die
    // AUSBLENDENDE Gruppe auf ihrem letzten Bild ein (kein Kinder-Rendering)
    // statt beide Visuals voll zu rechnen. Schwelle ~30 fps.
    constexpr double kFreezeDeltaSeconds = 1.0 / 30.0;
    const bool freezeFrame =
        !node.enabled && static_cast<double>(m_deltaTime) > kFreezeDeltaSeconds;

    if (!freezeFrame)
    {
        // S3-Konsistenz: Gruppen sind eigenstaendige Sub-Visuals — der
        // Linien-Blend-Modus startet innen auf REPLACE und leckt nicht heraus
        // (gleiches Save/Reset/Restore wie renderList).
        const RenderMode savedRenderMode = m_renderMode;
        m_renderMode = RenderMode{};
        m_surfaceStack.push_back(&runtime.surface);
        bindActive();
        for (const ChainNode& child : node.children)
        {
            renderNode(child);
        }
        active().current()->release();
        m_surfaceStack.pop_back();
        m_renderMode = savedRenderMode;
    }

    m_activePool = prevPool;
    m_activeContext = prevCtx;
    m_groupActiveSeconds = prevGroupSeconds;

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
            // Normalisiertes Over-Compositing: alpha_i = w_i/(sum_so_far+w_i)
            // macht den sequentiellen Mix EXAKT zum gewichteten 2er-Mix —
            // die Ketten-Reihenfolge der blendenden Gruppen ist damit egal
            // (Befund Sichttest S42: Eingangs-Gruppe ging sonst quadratisch ein)
            const double denom = m_blendRunningSum + curved;
            const double alphaNorm = denom > 1e-9 ? curved / denom : 1.0;
            m_blendRunningSum += curved;
            blendPass(active(), runtime.surface.current()->texture(),
                      BlendMode::Adjustable,
                      static_cast<int>(std::lround(
                          std::clamp(alphaNorm, 0.0, 1.0) * 255.0)),
                      0, false);
        }
    }
    bindActive();
}

// =============================================================================
// Leaves
// =============================================================================

void MultiEffectVisualizer::runClear(const ChainNode& node,
                                     const ClearParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vBlend = params.blend;
    runParamScript(rt, "clear", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"blend", &vBlend}});

    if (params.onlyFirst && !m_firstFrame) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    const QVector3D color = colorToVec(params.color);

    // ACHTUNG: `ClearParams::blend` ist eine EIGENE Aufzaehlung (0 replace,
    // 1 additiv, 2 50/50, 3 Line-Blend) und NICHT die BLEND_LINE-Tabelle.
    // `applyLineBlend` erwartet letztere — dort ist 2 = MAX und 50/50 = 3.
    // Vorher lief unser Modus roh hinein: aus "50/50 gegen Schwarz" wurde
    // "MAX gegen Schwarz", also ein No-op (max(x,0)=x). Das Bild klang damit
    // NIE ab; "Deep Red Sea" (50 Scopes auf einem Puffer, der sich je Frame
    // halbieren sollte) sammelte alles an und saettigte (Befund S52).
    const int blend = static_cast<int>(vBlend);  // Strang D: die Frame-Kopie
    int mode = blend == 3 ? m_renderMode.lineBlend
             : blend == 2 ? 3   // 50/50 = BLEND_AVG
                          : blend;

    if (mode == 0)
    {
        f->glClearColor(color.x(), color.y(), color.z(), 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    // Blended clear: full-screen color quad, full BLEND_LINE table (S9).
    applyLineBlend(mode, m_renderMode.alpha);
    if (mode == 1) f->glBlendFunc(GL_ONE, GL_ONE);  // Clear-Quad hat alpha=1
    m_barsShader->bind();
    m_quadVao->bind();
    m_barsShader->setUniformValue("uCenter", QVector2D(0.0f, 0.0f));
    m_barsShader->setUniformValue("uSize", QVector2D(1.0f, 1.0f));
    m_barsShader->setUniformValue("uColor", color);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_barsShader->release();
    resetLineBlend();
}

void MultiEffectVisualizer::runFadeout(const ChainNode& node,
                                       const FadeoutParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vFadeLen = params.fadeLen;
    runParamScript(rt, "fadeout", params.initCode, params.frameCode,
                   params.beatCode, {{"fadelen", &vFadeLen}});

    m_fadeShader->bind();
    m_fadeShader->setUniformValue("uTarget", colorToVec(params.color));
    m_fadeShader->setUniformValue("uStep",
                                  static_cast<float>(vFadeLen) / 255.0f);
    m_fadeShader->release();
    transformPass(*m_fadeShader);
}

void MultiEffectVisualizer::runInvert()
{
    transformPass(*m_invertShader);
}

void MultiEffectVisualizer::runBrightness(const ChainNode& node,
                                          const BrightnessParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vRed = params.red, vGreen = params.green, vBlue = params.blue;
    runParamScript(rt, "brightness", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"red", &vRed},
                    {"green", &vGreen},
                    {"blue", &vBlue}});

    // Slider -> multiplier (r_bright.cpp:188): negative darkens to 0, positive
    // brightens up to 17x.
    auto factor = [](int p) {
        return p < 0 ? 1.0f + static_cast<float>(p) / 4096.0f
                     : 1.0f + static_cast<float>(p) / 256.0f;
    };
    m_brightShader->bind();
    m_brightShader->setUniformValue(
        "uFactor", QVector3D(factor(static_cast<int>(vRed)),      // Strang D:
                             factor(static_cast<int>(vGreen)),    // Frame-Kopien
                             factor(static_cast<int>(vBlue))));
    m_brightShader->setUniformValue("uExclude", params.exclude);
    m_brightShader->setUniformValue("uExColor", colorToVec(params.color));
    m_brightShader->setUniformValue("uDistance",
                                    static_cast<float>(params.distance) / 255.0f);
    m_brightShader->release();
    transformPass(*m_brightShader);
}

void MultiEffectVisualizer::runFastBrightness(const ChainNode& node,
                                              const FastBrightnessParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vDir = params.dir;
    runParamScript(rt, "fastbright", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"dir", &vDir}});

    const int dir = static_cast<int>(vDir);  // Strang D: die Frame-Kopie
    if (dir == 2) return;  // identity — no work (r_fastbright dir==2)
    const float scale = dir == 0 ? 2.0f : 0.5f;  // x2 (clamped) / x0.5
    m_brightShader->bind();
    m_brightShader->setUniformValue("uFactor", QVector3D(scale, scale, scale));
    m_brightShader->setUniformValue("uExclude", false);
    m_brightShader->release();
    transformPass(*m_brightShader);
}

void MultiEffectVisualizer::runBlur(const ChainNode& node,
                                    const BlurParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vStrength = params.strength;
    runParamScript(rt, "blur", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"strength", &vStrength}});

    // Gewichte je Staerke, zeilengenau nach r_blur.cpp — als einzelne
    // Shift-Terme, weil die Referenz jeden getrennt abschneidet (s. Shader).
    // Der Ausgleich `roundAdj` ist der Wert, den „round mode" dort je Kernel
    // addiert; ohne den Schalter ist er 0 und das Bild klingt ab wie im
    // Original. AVS-Vorgabe ist AUS (`roundmode = 0`, r_blur.cpp:75/90).
    //
    //   Staerke 1 (enabled 1): DIV_2 + 4x DIV_8              -> +4
    //   Staerke 2 (enabled 2): DIV_2 + DIV_4 + 4x DIV_16     -> +5
    //   Staerke 3 (enabled 3):           4x DIV_4            -> +3
    const int strength = static_cast<int>(vStrength);  // Strang D: die Frame-Kopie
    float centerA = 0.5f;
    float centerB = 0.0f;
    float neighbor = 0.125f;
    float roundAdj = 4.0f;
    if (strength == 2)
    {
        centerB = 0.25f;
        neighbor = 0.0625f;
        roundAdj = 5.0f;
    }
    else if (strength == 3)
    {
        centerA = 0.0f;
        neighbor = 0.25f;
        roundAdj = 3.0f;
    }

    m_blurShader->bind();
    m_blurShader->setUniformValue(
        "uTexel", QVector2D(1.0f / static_cast<float>(m_surfaceWidth),
                            1.0f / static_cast<float>(m_surfaceHeight)));
    m_blurShader->setUniformValue("uCenterA", centerA);
    m_blurShader->setUniformValue("uCenterB", centerB);
    m_blurShader->setUniformValue("uNeighbor", neighbor);
    m_blurShader->setUniformValue("uRoundAdj", params.roundUp ? roundAdj : 0.0f);
    m_blurShader->release();
    transformPass(*m_blurShader);
}

void MultiEffectVisualizer::runMirror(const ChainNode& node,
                                      const MirrorParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vMode = params.mode, vSlower = params.slower;
    runParamScript(rt, "mirror", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"mode", &vMode},
                    {"slower", &vSlower}});


    // Active direction bits this frame (r_mirror.cpp:146 rbeat=(rand()%16)&mode).
    const int mode = static_cast<int>(vMode);  // Strang D: die Frame-Kopie
    int target = mode & 15;
    if (params.onBeatRandom)
    {
        if (m_frameBeat)
        {
            // EIN Zug aus dem Preset-Strom, wie r_mirror.cpp:148 (S49)
            rt.mirrorRBeat = (m_scriptContext->nextRand() % 16) & mode;
        }
        target = rt.mirrorRBeat;
    }

    // Per-direction factors: hard switch, or a 16-step ramp advancing every
    // `slower` frames (BLEND_ADAPT divisors, r_mirror.cpp:249-257).
    // Strang D: die Frame-Kopie. `slower` ist int — std::max(int, double) findet
    // keine Ueberladung (Stolperstein S53), deshalb erst casten.
    const bool step = params.smooth &&
                      (++rt.mirrorFrames % std::max(1, static_cast<int>(vSlower))) == 0;
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

    // Je Richtung ein eigener Durchgang, in der Reihenfolge der Referenz:
    // VERTICAL1, VERTICAL2, HORIZONTAL1, HORIZONTAL2 (r_mirror.cpp:167/188/
    // 210/230). Jeder sieht das Ergebnis des vorigen — genau darauf beruht,
    // dass zwei aktive Achsen ein SYMMETRISCHES Bild ergeben: der zweite
    // Durchgang spiegelt die bereits gespiegelte Haelfte zurueck. Ein einziger
    // Durchgang aus der Originaltextur kann das nicht leisten.
    static constexpr int kOrder[4] = {2, 3, 0, 1};
    for (const int dir : kOrder)
    {
        const float fac = rt.mirrorF[dir];
        if (fac <= 0.0f) continue;
        m_mirrorShader->bind();
        m_mirrorShader->setUniformValue("uDir", dir);
        m_mirrorShader->setUniformValue("uFac", fac);
        m_mirrorShader->release();
        transformPass(*m_mirrorShader);
    }
}

void MultiEffectVisualizer::runOnBeatClear(const ChainNode& node,
                                           const OnBeatClearParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vEvery = params.everyNBeats;
    runParamScript(rt, "onbeatclear", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"everynbeats", &vEvery}});

    // Beat counter -> clear every Nth beat (r_nfclr.cpp:97).
    if (!m_frameBeat) return;
    if (++rt.beatCounter < static_cast<int>(vEvery)) return;  // Strang D: Frame-Kopie
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
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vFadeR = params.faderR, vFadeG = params.faderG, vFadeB = params.faderB;
    double vBeatFrames = params.onBeatFrames;
    // Auch die BEAT-Fader sind skriptbar (S54). Legacy bleibt unberuehrt: die
    // Vorbelegung IST der Preset-Wert, ein Preset ohne Skript rechnet also
    // Schritt fuer Schritt dasselbe wie vorher.
    double vBeatR = params.beatFaderR, vBeatG = params.beatFaderG,
           vBeatB = params.beatFaderB;
    runParamScript(rt, "colorfade", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"faderr", &vFadeR},
                    {"faderg", &vFadeG},
                    {"faderb", &vFadeB},
                    {"beatfaderr", &vBeatR},
                    {"beatfaderg", &vBeatG},
                    {"beatfaderb", &vBeatB},
                    {"onbeatframes", &vBeatFrames}});

    // r_colorfade.cpp:139-168 zeilengenau. Der Effekt hat einen LAUFENDEN
    // Zustand (`faderpos`), und `enabled` ist dort ein Bitfeld: Bit 1 waehlt
    // die Fader im Beat zufaellig, Bit 2 laesst sie langsam nachziehen. Bis S57
    // fehlten beide Bits (der Import verwarf sie) und der Zustand ebenso — wir
    // setzten die Fader je Frame direkt.
    const int ziel[3] = {static_cast<int>(vFadeR), static_cast<int>(vFadeG),
                         static_cast<int>(vFadeB)};
    if (!rt.fadeSeeded)
    {
        rt.fadePos[0] = ziel[0];
        rt.fadePos[1] = ziel[1];
        rt.fadePos[2] = ziel[2];
        rt.fadeSeeded = true;
    }

    // Nachziehen um EINEN Schritt je Frame — und zwar mit VERTAUSCHTEM Ziel
    // fuer Gruen und Blau (`faderpos[1]` folgt `faders[2]`, `faderpos[2]`
    // folgt `faders[1]`; r_colorfade.cpp:139-147). Beim direkten Setzen unten
    // gilt die Vertauschung NICHT — eine Eigenart des Originals, keine
    // Verwechslung.
    const int nachziehZiel[3] = {ziel[0], ziel[2], ziel[1]};
    for (int k = 0; k < 3; ++k)
    {
        if (rt.fadePos[k] < nachziehZiel[k]) ++rt.fadePos[k];
        if (rt.fadePos[k] > nachziehZiel[k]) --rt.fadePos[k];
    }

    // Die drei Zweige in der Reihenfolge des Originals (r_colorfade.cpp:149-168).
    // Sie sind EXKLUSIV, und das ist der Grund, warum die Beat-Fader ohne
    // `slowFade` nichts tun: der erste Zweig fängt dann jeden Frame ab.
    //
    // `onBeatFrames` ist die LumiViz-Erweiterung darauf (s. Struct): das Fenster
    // wird ABGEFRAGT, bevor es herunterzaehlt — andernfalls waere es immer einen
    // Frame kuerzer als eingestellt (Fehler in der ersten Fassung, S57).
    const bool imBeatFenster = rt.beatFramesLeft > 0;
    if (rt.beatFramesLeft > 0) --rt.beatFramesLeft;

    if (!params.slowFade)
    {
        rt.fadePos[0] = ziel[0];
        rt.fadePos[1] = ziel[1];
        rt.fadePos[2] = ziel[2];
    }
    else if (m_frameBeat && params.onBeatRandom)
    {
        // r_colorfade.cpp:157-161 — Gruen wird aus dem Mittelfeld
        // herausgerissen, damit der Beat sichtbar bleibt.
        auto& rnd = *m_scriptContext;  // EIN rand()-Strom je Preset (S49)
        rt.fadePos[0] = static_cast<int>(rnd.nextRand() % 32) - 6;
        int g = static_cast<int>(rnd.nextRand() % 64) - 32;
        if (g < 0 && g > -16) g = -32;
        if (g >= 0 && g < 16) g = 32;
        rt.fadePos[1] = g;
        rt.fadePos[2] = static_cast<int>(rnd.nextRand() % 32) - 6;
        rt.beatFramesLeft = std::max(0, static_cast<int>(vBeatFrames) - 1);
    }
    else if (m_frameBeat || imBeatFenster)
    {
        rt.fadePos[0] = static_cast<int>(vBeatR);
        rt.fadePos[1] = static_cast<int>(vBeatG);
        rt.fadePos[2] = static_cast<int>(vBeatB);
        // Vorgabe 1 heisst: NUR der Beat-Frame, also das Referenzverhalten.
        if (m_frameBeat)
            rt.beatFramesLeft = std::max(0, static_cast<int>(vBeatFrames) - 1);
    }

    const int f1 = rt.fadePos[0];
    const int f2 = rt.fadePos[1];
    const int f3 = rt.fadePos[2];

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
    rt.lut->setVisData(m_visdata.data(), m_scriptClock);  // getspec/getosc for the level code
    {
        float bass, mid, treble;
        computeAudioBands(getSpectrum(), bass, mid, treble);
        feedAudioInputSet(*rt.lut, bass, mid, treble, m_audioLevel, m_frameBeat);
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
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    if (params.builtinRemap != 0)
    {
        // per-pixel remap builtins (1 fuzzify / 7 blocky partial out, S44)
        if (m_moveRemapShader == nullptr) return;
        m_moveRemapShader->bind();
        m_moveRemapShader->setUniformValue("uMode", params.builtinRemap);
        m_moveRemapShader->setUniformValue("uBlend", params.blend);
        m_moveRemapShader->release();
        transformPass(*m_moveRemapShader);
        return;
    }
    if (params.code.empty()) return;  // formula "none" -> no-op

    // r_trans wertet das Skript je PIXEL aus und legt eine Tabelle an, die nur
    // bei Groessen-/Skriptwechsel neu entsteht — kein Gitter, keine
    // Interpolation. Das Gitter unten dient nur noch dem sourcemapped-Streuer.
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
    //
    // Der Laufzeitwert kippt bei jedem Beat, darf also NICHT je Frame aus dem
    // Preset ueberschrieben werden — sonst gaebe es kein Kippen. Bis S55 wurde
    // er dafuer nur EINMAL uebernommen (`< 0`), und das hiess: nach dem Laden
    // stimmte er, nach einem Reglerdreh im Panel nie. Das Feld liess sich
    // verstellen, ohne dass etwas geschah, und wirkte erst nach Speichern +
    // Laden (Befund Patrik S55, gemessen mit den Edit-Sonden: MAE 0,081).
    //
    // Jetzt wie bei den Parameter-Skripten seit S54: der zuletzt uebernommene
    // PRESET-Wert wird mitgefuehrt; aendert er sich, gewinnt er. Das Kippen
    // zwischen zwei Aenderungen bleibt erhalten.
    if (rt.moveSourceMapped < 0 || rt.moveSourceMappedSeen != params.sourceMapped)
    {
        rt.moveSourceMapped = params.sourceMapped & 3;
        rt.moveSourceMappedSeen = params.sourceMapped;
    }
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
    if (applyMovementTable(rt, params)) return;
    applyGridWarp(rt, kXres, kYres, opt);
}

bool MultiEffectVisualizer::applyMovementTable(LeafRuntime& rt,
                                               const MovementParams& params)
{
    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    if (rt.grid == nullptr || w < 2 || h < 2) return false;

    // r_trans.cpp:306-309: Subpixel faellt bei sehr grossen Flaechen weg — der
    // Ziel-Offset belegt nur 22 Bit der Tabelle.
    const bool subpixel = params.subpixel && w * h < (1 << 22);
    const std::string key = (params.rectCoords ? "R" : "P") +
                            std::string(params.wrap ? "W" : "-") +
                            (subpixel ? "S" : "-") + params.code;
    if (rt.moveTabTex == 0 || rt.moveTabW != w || rt.moveTabH != h ||
        rt.moveTabKey != key)
    {
        std::vector<int> tab;
        if (!rt.grid->buildTransTable(w, h, params.wrap, subpixel, tab)) return false;

        auto* f = QOpenGLContext::currentContext()->functions();
        f->glActiveTexture(GL_TEXTURE1);
        if (rt.moveTabTex == 0)
        {
            f->glGenTextures(1, &rt.moveTabTex);
            f->glBindTexture(GL_TEXTURE_2D, rt.moveTabTex);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
            f->glBindTexture(GL_TEXTURE_2D, rt.moveTabTex);
        }
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, w, h, 0, GL_RED_INTEGER, GL_INT,
                        tab.data());
        f->glActiveTexture(GL_TEXTURE0);
        rt.moveTabW = w;
        rt.moveTabH = h;
        rt.moveTabKey = key;
    }

    auto* f = QOpenGLContext::currentContext()->functions();
    m_moveTabShader->bind();
    m_moveTabShader->setUniformValue("uResX", w);
    m_moveTabShader->setUniformValue("uResY", h);
    m_moveTabShader->setUniformValue("uBlend", params.blend);
    m_moveTabShader->setUniformValue("uSubpixel", subpixel);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.moveTabTex);
    m_moveTabShader->setUniformValue("uTab", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_moveTabShader->release();
    transformPass(*m_moveTabShader);
    return true;
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
    m_warpShader->setUniformValue("uTrunc", false);  // Scatter: kein Resample
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
        rt.grid->setAvsGridPositions(true);  // r_dmove-Stuetzstellen (trunkiert)
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
    if (applyGridWarpFx(rt, params.xres, params.yres, opt)) return;
    applyGridWarp(rt, params.xres, params.yres, opt);
}

bool MultiEffectVisualizer::applyGridWarpFx(LeafRuntime& rt, int xres, int yres,
                                            const GridWarpOptions& opt)
{
    if (rt.grid == nullptr || xres < 2 || yres < 2) return false;
    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    // r_dmove.cpp:407-413 / 455-461: ein Gitterband der Breite 0 laesst das
    // Original MITTEN im Bild abbrechen (der Rest bleibt stehen). Statt diesen
    // Zufall nachzubauen, faellt der Fall auf den Netz-Pfad zurueck.
    if (xres - 1 > w || yres - 1 > h) return false;

    const bool needExecute = !opt.staticField || rt.grid->fieldFx().empty() ||
                             rt.gridFieldW != w || rt.gridFieldH != h;
    if (needExecute)
    {
        rt.grid->setVisData(m_visdata.data(), m_scriptClock);  // getspec/getosc backing
        {
            float bass, mid, treble;
            computeAudioBands(getSpectrum(), bass, mid, treble);
            feedAudioInputSet(*rt.grid, bass, mid, treble, m_audioLevel,
                              m_frameBeat);
        }
        rt.grid->execute(static_cast<float>(w), static_cast<float>(h), m_frameBeat,
                         m_deltaTime);
        rt.gridFieldW = w;
        rt.gridFieldH = h;
    }
    const auto& fx = rt.grid->fieldFx();
    if (static_cast<int>(fx.size()) < xres * yres) return false;

    // r_dmove.cpp:230-231/256-260: der Wrap-/Klemm-Rand haengt an subpixel — mit
    // Subpixel bleibt eine Reserve-Spalte/-Zeile fuer den vierten BLEND4-Nachbarn.
    const int wAdj = opt.subpixel ? (w - 2) << 16 : (w - 1) << 16;
    const int hAdj = opt.subpixel ? (h - 2) << 16 : (h - 1) << 16;
    if (wAdj <= 0 || hAdj <= 0) return false;

    m_warpTab.resize(static_cast<std::size_t>(xres) * yres * 4);
    for (std::size_t i = 0, n = static_cast<std::size_t>(xres) * yres; i < n; ++i)
    {
        int tx = fx[i].x;
        int ty = fx[i].y;
        if (!opt.wrap)  // r_dmove.cpp:349-355: geklemmt wird schon in der Tabelle
        {
            tx = std::clamp(tx, 0, wAdj);
            ty = std::clamp(ty, 0, hAdj);
        }
        m_warpTab[i * 4 + 0] = tx;
        m_warpTab[i * 4 + 1] = ty;
        m_warpTab[i * 4 + 2] = fx[i].a;
        m_warpTab[i * 4 + 3] = 0;
    }

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glActiveTexture(GL_TEXTURE2);  // Tabelle lebt auf Einheit 2 (0/1 = Bilder)
    if (m_warpTabTex == 0)
    {
        f->glGenTextures(1, &m_warpTabTex);
        f->glBindTexture(GL_TEXTURE_2D, m_warpTabTex);
        // Integer-Texturen koennen nur NEAREST — gewollt, wir fetchen texelweise.
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        f->glBindTexture(GL_TEXTURE_2D, m_warpTabTex);
    }
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, xres, yres, 0, GL_RGBA_INTEGER,
                    GL_INT, m_warpTab.data());

    const unsigned int srcTex =
        opt.srcTexture != 0 ? opt.srcTexture : active().current()->texture();

    m_warpFxShader->bind();
    m_warpFxShader->setUniformValue("uResX", w);
    m_warpFxShader->setUniformValue("uResY", h);
    m_warpFxShader->setUniformValue("uGridX", xres);
    m_warpFxShader->setUniformValue("uGridY", yres);
    m_warpFxShader->setUniformValue("uXcDpos", (w << 16) / (xres - 1));
    m_warpFxShader->setUniformValue("uYcDpos", (h << 16) / (yres - 1));
    m_warpFxShader->setUniformValue("uWAdj", wAdj);
    m_warpFxShader->setUniformValue("uHAdj", hAdj);
    m_warpFxShader->setUniformValue("uWrap", opt.wrap);
    m_warpFxShader->setUniformValue("uBlend", opt.blend);
    m_warpFxShader->setUniformValue("uNomove", opt.nomove);
    m_warpFxShader->setUniformValue("uBufSrc", opt.srcTexture != 0);
    m_warpFxShader->setUniformValue("uSubpixel", opt.subpixel);
    m_warpFxShader->setUniformValue("uTab", 2);  // schon gebunden (s. oben)
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, srcTex);
    m_warpFxShader->setUniformValue("uSrcTex", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_warpFxShader->release();
    transformPass(*m_warpFxShader);  // bindet uTex = aktuelle Surface
    return true;
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
        rt.grid->setVisData(m_visdata.data(), m_scriptClock);  // getspec/getosc backing
        {
            float bass, mid, treble;
            computeAudioBands(getSpectrum(), bass, mid, treble);
            feedAudioInputSet(*rt.grid, bass, mid, treble, m_audioLevel,
                              m_frameBeat);
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
    m_warpShader->setUniformValue("uTrunc", opt.subpixel);  // Befund B (S46)
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

void MultiEffectVisualizer::runBlitterFeedback(const ChainNode& node,
                                               const BlitterFeedbackParams& params)
{
    // r_blit.cpp zeilengenau (S48-Matrix-Befund 04): fpos eased +-3/Frame
    // Richtung scale (Beat -> scale2); f_val < 32 zoomt zentrisch REIN
    // (Faktor 64/(f_val+32)), f_val > 32 schrumpft das GANZE Bild in ein
    // zentriertes Fenster (blitter_out), 32 ist neutral.
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vScale = params.scale, vScale2 = params.scale2;
    runParamScript(rt, "blitterfb", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"scale", &vScale}, {"scale2", &vScale2}});

    if (!rt.bfSeeded)
    {
        rt.bfFpos = static_cast<int>(vScale);
        rt.bfSeeded = true;
    }
    if (m_frameBeat && params.onBeat) rt.bfFpos = static_cast<int>(vScale2);
    int fVal;
    if (vScale < vScale2)
    {
        fVal = std::max(rt.bfFpos, static_cast<int>(vScale));
        rt.bfFpos -= 3;
    }
    else
    {
        fVal = std::min(rt.bfFpos, static_cast<int>(vScale));
        rt.bfFpos += 3;
    }
    if (fVal < 0) fVal = 0;
    if (fVal == 32) return;  // neutral

    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    if (fVal < 32)  // blitter_normal: zentrischer Zoom-IN
    {
        // src_avs = ds*dest_avs + h(1-ds)/2; die GL-Textur ist bottom-up:
        // y-Offset nach dem Flip = (1-ds)*(h/2 - 1) (x bleibt ungespiegelt).
        const float ds = static_cast<float>(fVal + 32) / 64.0f;
        const float m[4] = {ds, 0.0f, 0.0f, ds};
        const QVector2D off(static_cast<float>(w) * (1.0f - ds) * 0.5f,
                            (1.0f - ds) * (static_cast<float>(h) * 0.5f - 1.0f));
        feedbackPass(QMatrix2x2(m), off, false, params.blend, params.subpixel);
        return;
    }

    // blitter_out (f_val > 32): das ganze Bild in ein zentriertes Fenster
    // x_len*y_len skaliert (x_len 4er-aligned wie das SIMD-Original), der
    // Rand bleibt unveraendert stehen.
    const float dsX = static_cast<float>(fVal + 96) / 128.0f;
    const int xLen = static_cast<int>(static_cast<float>(w) / dsX) & ~3;
    const int yLen = static_cast<int>(static_cast<float>(h) / dsX);
    if (xLen >= w || yLen >= h || xLen < 4 || yLen < 1) return;
    const int startX = (w - xLen) / 2;
    const int startY = (h - yLen) / 2;  // AVS-Zeile (top-down)

    auto* f = QOpenGLContext::currentContext()->functions();
    SurfacePair& pair = active();
    pair.partner()->bind();
    f->glViewport(0, 0, w, h);

    // 1:1-Basis (der Rand bleibt stehen), dann das Fenster skaliert fuellen.
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_presentShader->bind();
    m_presentShader->setUniformValue("uTex", 0);
    m_quadVao->bind();
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_presentShader->release();

    f->glEnable(GL_SCISSOR_TEST);
    f->glScissor(startX, h - startY - yLen, xLen, yLen);  // GL zaehlt von unten
    m_feedbackShader->bind();
    m_feedbackShader->setUniformValue("uRes", QVector2D(static_cast<float>(w),
                                                        static_cast<float>(h)));
    // src laeuft ab 0.5 px ueber das GANZE Bild: sp = dsX*dest + off; die
    // y-Achse ist im GL-Raum gespiegelt (AVS top-down). Der y-Anker ist
    // h-0.5 (NICHT h-1.5): gesampelt werden soll GL-Zeile
    // h-1-floor(0.5+dsX*k), und floor((h-0.5)-dsX*k) liefert genau das —
    // mit h-1.5 lag JEDE Quellzeile eine AVS-Zeile zu tief, und der Fehler
    // stapelte sich ueber die Feedback-Kaskade (S60, Deckung 0,72).
    const float m[4] = {dsX, 0.0f, 0.0f, dsX};
    m_feedbackShader->setUniformValue("uMap", QMatrix2x2(m));
    m_feedbackShader->setUniformValue(
        "uOff",
        QVector2D(0.5f - dsX * static_cast<float>(startX),
                  (static_cast<float>(h) - 0.5f) -
                      dsX * static_cast<float>(h - 1 - startY)));
    m_feedbackShader->setUniformValue("uWrap", false);
    m_feedbackShader->setUniformValue("uBlend", params.blend);
    // blitter_out sampelt IMMER nearest (r_blit.cpp:143 — `src[s_x>>16]`,
    // kein BLEND4-Pfad): der `subpixel`-Schalter wirkt nur im Zoom-IN
    // (blitter_normal). Unser Bilinear hier war die konstante Kanten-
    // Differenz der Matrix-Zeile 04 (S60, MAE 0,023 ab Frame 2).
    m_feedbackShader->setUniformValue("uSubpixel", false);
    m_feedbackShader->setUniformValue("uTex", 0);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_feedbackShader->release();
    f->glDisable(GL_SCISSOR_TEST);
    m_quadVao->release();
    pair.partner()->release();
    pair.swap();
    bindActive();
}

void MultiEffectVisualizer::runRotoBlitter(const ChainNode& node,
                                           const RotoBlitterParams& params)
{
    // r_rotblit.cpp zeilengenau (S48-Matrix-Befund 09): das Bild wird JEDE
    // Frame um das konstante theta gedreht/gezoomt — die sichtbare Rotation
    // akkumuliert uebers Feedback (die alte Winkel-Akkumulation hier war das
    // Pixel-Rauschen). Sampling kachelt (s %= ds) und ist bilinear (BLEND4).
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vZoom = params.zoomScale, vZoom2 = params.zoomScale2;
    double vRotDir = params.rotDir;
    runParamScript(rt, "rotoblitter", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"zoomscale", &vZoom},
                    {"zoomscale2", &vZoom2},
                    {"rotdir", &vRotDir}});

    if (!rt.rotoSeeded)
    {
        rt.rotoFpos = static_cast<int>(vZoom);
        rt.rotoSeeded = true;
    }
    if (m_frameBeat && params.beatReverse) rt.rotoRev = -rt.rotoRev;
    if (!params.beatReverse) rt.rotoRev = 1.0f;
    rt.rotoRevPos += (1.0f / (1.0f + static_cast<float>(params.beatReverseSpeed) *
                                         4.0f)) *
                     (rt.rotoRev - rt.rotoRevPos);
    if (rt.rotoRevPos > rt.rotoRev && rt.rotoRev > 0.0f) rt.rotoRevPos = rt.rotoRev;
    if (rt.rotoRevPos < rt.rotoRev && rt.rotoRev < 0.0f) rt.rotoRevPos = rt.rotoRev;
    if (m_frameBeat && params.beatZoomJump) rt.rotoFpos = static_cast<int>(vZoom2);

    int fVal;
    if (vZoom < vZoom2)
    {
        fVal = std::max(rt.rotoFpos, static_cast<int>(vZoom));
        if (rt.rotoFpos > vZoom) rt.rotoFpos -= 3;
    }
    else
    {
        fVal = std::min(rt.rotoFpos, static_cast<int>(vZoom));
        if (rt.rotoFpos < vZoom) rt.rotoFpos += 3;
    }
    const double zoom = 1.0 + static_cast<double>(fVal - 31) / 31.0;
    const double thetaDeg =
        (vRotDir - 32.0) * static_cast<double>(rt.rotoRevPos);

    // Abbildung exakt wie r_rotblit (:159-171): 16.16-Fixpunkt-Ints (Cast =
    // trunc), Zentrum bei INTEGER (w-1)/2, sstart/tstart inkl. des (1<<20)-
    // Positiv-Offsets (macht das C-% zum mathematischen mod) — der top-down/
    // bottom-up-Flip passiert im Shader (uAvsLinear).
    const double th = thetaDeg * 3.14159265358979 / 180.0;
    const int dsdx = static_cast<int>(std::cos(th) * zoom * 65536.0);
    const int sinFx = static_cast<int>(std::sin(th) * zoom * 65536.0);
    const int dsdy = -sinFx;
    const int dtdx = sinFx;
    const int dtdy = dsdx;
    const int w1 = m_surfaceWidth - 1;
    const int h1 = m_surfaceHeight - 1;
    const int ds = w1 << 16;
    const int dt = h1 << 16;
    // Guard wie das Original: Schrittweite >= Bildgroesse -> no-op.
    if (dsdx <= -ds || dsdx >= ds || dtdx <= -dt || dtdx >= dt) return;
    const int cxi = w1 / 2;  // Integer-Division wie das Original
    const int cyi = h1 / 2;
    const int sstart = -(cxi * dsdx + cyi * dsdy) + w1 * (32768 + (1 << 20));
    const int tstart = -(cxi * dtdx + cyi * dtdy) + h1 * (32768 + (1 << 20));

    m_feedbackShader->bind();
    m_feedbackShader->setUniformValue(
        "uRes", QVector2D(static_cast<float>(m_surfaceWidth),
                          static_cast<float>(m_surfaceHeight)));
    m_feedbackShader->setUniformValue("uAvsLinear", true);
    m_feedbackShader->setUniformValue("uDsDx", dsdx);
    m_feedbackShader->setUniformValue("uDsDy", dsdy);
    m_feedbackShader->setUniformValue("uDtDx", dtdx);
    m_feedbackShader->setUniformValue("uDtDy", dtdy);
    m_feedbackShader->setUniformValue("uSStart", sstart);
    m_feedbackShader->setUniformValue("uTStart", tstart);
    m_feedbackShader->setUniformValue("uBlend", params.blend);
    m_feedbackShader->setUniformValue("uSubpixel", params.subpixel);
    m_feedbackShader->release();
    transformPass(*m_feedbackShader);
}

void MultiEffectVisualizer::feedbackPass(const QMatrix2x2& map, const QVector2D& off,
                                         bool /*wrap*/, bool blend, bool subpixel)
{
    m_feedbackShader->bind();
    m_feedbackShader->setUniformValue(
        "uRes", QVector2D(static_cast<float>(m_surfaceWidth),
                          static_cast<float>(m_surfaceHeight)));
    m_feedbackShader->setUniformValue("uAvsLinear", false);
    m_feedbackShader->setUniformValue("uMap", map);
    m_feedbackShader->setUniformValue("uOff", off);
    m_feedbackShader->setUniformValue("uBlend", blend);
    m_feedbackShader->setUniformValue("uSubpixel", subpixel);
    m_feedbackShader->release();
    transformPass(*m_feedbackShader);
}

void MultiEffectVisualizer::runBufferSave(const ChainNode& node,
                                          const BufferSaveParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vSlot = params.slot, vDir = params.dir, vAlpha = params.adjustAlpha;
    runParamScript(rt, "buffersave", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"slot", &vSlot},
                    {"dir", &vDir},
                    {"adjustalpha", &vAlpha}});

    // Direction per frame (r_stack.cpp:125-126): dir 0/1 are fixed, dir 2/3
    // alternate save/restore every render via the per-node toggle.
    const int dir = static_cast<int>(vDir);      // Strang D: die Frame-Kopien
    const int slot = static_cast<int>(vSlot);
    const int alpha = static_cast<int>(vAlpha);
    int tDir = dir;
    if (dir >= 2)
    {
        tDir = (dir & 1) ^ (rt.bufDirCh ? 1 : 0);
        rt.bufDirCh = !rt.bufDirCh;
    }

    if (tDir == 0)
    {
        // Write the current working buffer into global buffer `slot`. The blend
        // applies in BOTH directions in AVS (fbin/fbout swap, r_stack.cpp:127).
        QOpenGLFramebufferObject* pool =
            activePool().get(slot, m_surfaceWidth, m_surfaceHeight, true);
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
            blendPass(m_bufferScratch, active().current()->texture(), params.blend, alpha);
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
            activePool().get(slot, m_surfaceWidth, m_surfaceHeight, false);
        if (pool == nullptr) return;  // nothing saved yet
        blendPass(active(), pool->texture(), params.blend, alpha);
        bindActive();
    }
}

void MultiEffectVisualizer::runCustomBpm(const ChainNode& node,
                                         const CustomBpmParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vMs = params.arbitraryMs, vSkip = params.skipCount;
    runParamScript(rt, "custombpm", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"arbitraryms", &vMs},
                    {"skipcount", &vSkip}});

    const bool inBeat = m_frameBeat;

    // Aufbau 1:1 nach r_bpm.cpp:137-185. Die drei Betriebsarten kehren dort
    // JEWEILS SOFORT zurueck, sind also exklusiv — vorher liefen sie hier
    // hintereinander und konnten sich kombinieren (Befund S52).
    if (inBeat) ++rt.customBeatCount;

    // skipfirst: die ersten N Beats verschlucken. Wichtig — der Zaehler des
    // Skip-Zweigs laeuft dabei NICHT mit (die Referenz kehrt vor ihm zurueck).
    if (params.skipFirst != 0 && rt.customBeatCount <= params.skipFirst)
    {
        if (inBeat) m_frameBeat = false;
        return;
    }

    if (params.arbitrary)
    {
        const std::int64_t now = lumi::modules::BeatEstimator::steadyNowMs();
        // `arbLastTC` startet in der Referenz bei 0, der erste Vergleich
        // schlaegt also sofort an; das Seeding auf "jetzt" verzoegerte den
        // ersten Beat um ein volles Intervall.
        m_frameBeat = now > rt.customLastMs + static_cast<int>(vMs);  // Strang D
        if (m_frameBeat) rt.customLastMs = now;
        return;
    }

    if (params.skip)
    {
        // `++skipCount >= skipVal + 1` — durchgelassen wird jeder
        // (skipVal+1)-te Beat. Unsere alte Bedingung verglich gegen skipVal
        // selbst und liess damit jeden DRITTEN statt jeden VIERTEN durch
        // (Sonde 7_rand/bpm_zaehler_skip3: Zeile 9 Positionen daneben).
        m_frameBeat = inBeat && ++rt.customSkipCount >= static_cast<int>(vSkip) + 1;
        if (m_frameBeat) rt.customSkipCount = 0;
        return;
    }

    if (params.invert)
    {
        m_frameBeat = !inBeat;
        return;
    }
    // Keine Betriebsart aktiv: der Beat laeuft unveraendert weiter (return 0).
}

void MultiEffectVisualizer::runSetRenderMode(const SetRenderModeParams& params)
{
    // No visual output — sets the host render mode for the following render
    // effects (AVS r_linemode). A disabled node leaves the current blend
    // UNCHANGED (r_linemode.cpp:96-104 writes only when bit31 is set); a width
    // of 0 leaves each effect's own line width.
    m_renderMode.set = true;
    if (params.lineWidth > 0) m_renderMode.lineWidth = params.lineWidth;
    if (params.enabled) m_renderMode.lineBlend = std::clamp(params.lineBlend, 0, 9);
    m_renderMode.alpha = std::clamp(params.adjustAlpha, 0, 255);
}

void MultiEffectVisualizer::applyLineBlend(int mode, int adjustAlpha)
{
    // GL-Pendants der AVS BLEND_LINE-Modi (r_defs.h:267-283, S9). Der
    // Alpha-Kanal wird IMMER auf dst gehalten (Separate-Blend: ADD, 0*src +
    // 1*dst): AVS kennt kein Alpha, unsere Surfaces fuehren kanonisch 1.0 —
    // die Subtract-Modi hatten sonst Alpha-0-Loecher hinterlassen (Befund
    // Session 45: "transparente Diagonale" im Screenshot).
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    const auto funcRgb = [f](GLenum src, GLenum dst) {
        f->glBlendFuncSeparate(src, dst, GL_ZERO, GL_ONE);
    };
    switch (mode)
    {
        case 0:  funcRgb(GL_ONE, GL_ZERO); break;                      // replace
        case 2:                                                         // maximum
            f->glBlendEquationSeparate(GL_MAX, GL_FUNC_ADD);
            funcRgb(GL_ONE, GL_ONE);
            break;
        case 3:                                                         // 50/50
            funcRgb(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
            f->glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);
            break;
        case 4:                                                         // fb - c
            f->glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
            funcRgb(GL_SRC_ALPHA, GL_ONE);
            break;
        case 5:                                                         // c - fb
            f->glBlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_ADD);
            funcRgb(GL_SRC_ALPHA, GL_ONE);
            break;
        case 6:  funcRgb(GL_DST_COLOR, GL_ZERO); break;                // multiply
        case 7:                                                         // adjustable
            // `BLEND_ADJ(*fb, color, v)` = `blendtable[fb][v] +
            // blendtable[color][255-v]` (r_defs.h:250-257, Tabelle i*j/255):
            // **v gewichtet den FRAMEBUFFER, 255-v die neue Farbe.** Unsere
            // Faktoren standen vertauscht — bei "Deep Red Sea" (v=181) nahmen
            // wir 71 % neue Farbe statt 71 % Bild, jeder der 50 Scopes legte
            // nach und das Bild sättigte zu blassem Gelb (dMean 0,943).
            funcRgb(GL_ONE_MINUS_CONSTANT_ALPHA, GL_CONSTANT_ALPHA);
            f->glBlendColor(0.0f, 0.0f, 0.0f,
                            static_cast<float>(std::clamp(adjustAlpha, 0, 255)) /
                                255.0f);
            break;
        case 9:                                                         // minimum
            f->glBlendEquationSeparate(GL_MIN, GL_FUNC_ADD);
            funcRgb(GL_ONE, GL_ONE);
            break;
        // 8 (xor) ist mit Fixed-Function-Blending nicht darstellbar — bewusster
        // Additiv-Fallback (Notiz S9); 1 = additiv = Default.
        default: funcRgb(GL_SRC_ALPHA, GL_ONE); break;
    }
}

void MultiEffectVisualizer::resetLineBlend()
{
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glBlendEquation(GL_FUNC_ADD);
    f->glDisable(GL_BLEND);
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
        // Geteilter Kontext wie bei Grid/Texer/Liste: AVS haelt reg00..reg99
        // GLOBAL, und dieses Pack koppelt Kamera und Scopes ausschliesslich
        // darueber (Befund S50).
        rt.scope = std::make_unique<lumi::modules::SuperscopeModule>(activeContext());
        rt.scope->setLuaMode(true);  // EEL quartet -> Lua (import path)
        // S13 (Urteil per AvsRef, S46): AVS skaliert je ACHSE (r_sscope:
        // x*(w/2)+w/2, y*(h/2)+h/2) — x=+-1 fuellt die Breite, ein Kreis-
        // Skript wird auf 4:3 zur Ellipse. Keine Aspekt-Korrektur im
        // Import-Pfad (eigene LumiViz-Scopes behalten ihren Default).
        rt.scope->setAspectCorrection(false);
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
    rt.scope->setAudioSource(params.spectrumSource
                                 ? lumi::modules::SuperscopeAudioSource::Spectrum
                                 : lumi::modules::SuperscopeAudioSource::Waveform);
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
    rt.scope->setVisData(m_visdata.data(), m_scriptClock);
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

    // Blend onto the working buffer: full AVS BLEND_LINE table (S9); a
    // preceding Set Render Mode overrides the mode (params.lineBlend).
    if (!m_scopeRenderer.ready()) return;
    applyLineBlend(effLineBlend, m_renderMode.alpha);
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
    // dotSize 1 = AVS-Semantik (der Translator setzt sie fuer jeden Import,
    // r_sscope zeichnet immer genau ein Pixel). Groessere Werte kommen aus dem
    // Panel und meinen bewusst runde Punkte — die bleiben, wie sie waren.
    rp.avsPixelDots = params.dotSize <= 1.0f;

    const bool perPointMode = rt.scope->pointDrawModeActive();
    const bool perPointSize = rt.scope->pointLineSizeActive();
    if (perPointMode || perPointSize)
    {
        // Point code switches drawmode und/oder linesize mid-scope (r_sscope
        // wertet beide JE PUNKT aus): in Laeufe zerlegen, in denen beides
        // konstant ist. Ein Linien-Lauf nimmt den Vorgaengerpunkt mit, damit
        // das verbindende Segment erhalten bleibt.
        const auto sameRun = [&](const lumi::modules::SuperscopePoint& a,
                                 const lumi::modules::SuperscopePoint& b) {
            return a.drawLines == b.drawLines && a.lineSize == b.lineSize;
        };
        size_t start = 0;
        while (start < points.size())
        {
            size_t end = start + 1;
            while (end < points.size() && sameRun(points[end], points[start])) ++end;
            std::vector<lumi::modules::SuperscopePoint> run;
            if (points[start].drawLines && start > 0)
            {
                run.push_back(points[start - 1]);
            }
            run.insert(run.end(), points.begin() + static_cast<long long>(start),
                       points.begin() + static_cast<long long>(end));
            if (perPointMode)
            {
                rp.mode = points[start].drawLines
                              ? lumi::modules::SuperscopeRenderMode::Lines
                              : lumi::modules::SuperscopeRenderMode::Dots;
            }
            if (perPointSize && points[start].lineSize > 0.0f)
            {
                rp.lineWidth = points[start].lineSize;
            }
            m_scopeRenderer.draw(run, rp);
            start = end;
        }
    }
    else
    {
        m_scopeRenderer.draw(points, rp);
    }
    resetLineBlend();
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
        // Frame lazy ueber die Custom-Rev). Eingebettete .lvfx-Bilder VOR dem
        // Apply setzen (Fallback, wenn die Asset-Dateien fehlen — S43).
        rt.milk->setEmbeddedImages(params.embeddedImages);
        rt.milk->applyPresetState(params.preset,
                                  QString::fromStdString(params.presetDir), nullptr);
    }

    // Die drei Host-Regler standen bis S56 IM Revisions-Block — ein Panel-Edit
    // bumpt die Revision aber nicht (die zaehlt Preset-/Skript-/Shader-Edits),
    // also kamen sie nur beim Aufbau an. Gemessen mit den Edit-Sonden:
    // `milkdrop.meshX`/`meshY`/`debugGrid` WIRKUNGSLOS. Sie sind reine
    // Zuweisungen im Kern (`setParam`, kein Neuaufbau) und stehen deshalb
    // jetzt je Frame hier.
    rt.milk->setParam("render.meshX", lumi::modules::ParamValue{params.meshX});
    rt.milk->setParam("render.meshY", lumi::modules::ParamValue{params.meshY});
    rt.milk->setParam("render.debugGrid", lumi::modules::ParamValue{params.debugGrid});

    feedMilkAudio(*rt.milk);
    // Chain-Buffer-Groesse (physische Pixel) — width()/height() des Kerns
    // steuern dessen Composite-Viewport und die Feedback-Buffer-Groesse
    rt.milk->resize(QSize(m_surfaceWidth, m_surfaceHeight));

    // HG3: In einer Host-Gruppe speist deren Laufzeit die progress-Variable
    // (60-s-Zyklus ab Gruppen-Aktivierung; die Playlist liefert spaeter die
    // echte Slot-Dauer). Ausserhalb bleibt der interne Zyklus des Kerns.
    rt.milk->setProgressOverride(
        m_groupActiveSeconds >= 0.0
            ? std::fmod(m_groupActiveSeconds, 60.0) / 60.0
            : -1.0);

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

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vQuality = params.quality, vQuality2 = params.quality2;
    runParamScript(rt, "mosaic", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"quality", &vQuality}, {"quality2", &vQuality2}});

    if (rt.mosaicQuality <= 0.0f) rt.mosaicQuality = static_cast<float>(vQuality);

    // Pick this frame's block count, then step the ease-back (r_mosaic.cpp).
    if (params.onBeat && m_frameBeat)
    {
        rt.mosaicQuality = static_cast<float>(vQuality2);
        rt.mosaicFramesLeft = std::max(1, params.durationFrames);
    }
    else if (rt.mosaicFramesLeft == 0)
    {
        rt.mosaicQuality = static_cast<float>(vQuality);
    }
    const float thisQuality = rt.mosaicQuality;
    if (rt.mosaicFramesLeft > 0)
    {
        if (--rt.mosaicFramesLeft > 0)
        {
            const float step = std::abs(static_cast<float>(vQuality - vQuality2)) /
                               static_cast<float>(std::max(1, params.durationFrames));
            rt.mosaicQuality += vQuality2 > vQuality ? -step : step;
        }
        else
        {
            rt.mosaicQuality = static_cast<float>(vQuality);
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

void MultiEffectVisualizer::runGrain(const ChainNode& node, const GrainParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vAmount = params.amount;
    runParamScript(rt, "grain", params.initCode, params.frameCode, params.beatCode,
                   {{"amount", &vAmount}});

    auto* f = QOpenGLContext::currentContext()->functions();
    auto& rnd = *m_scriptContext;  // EIN rand()-Strom je Preset (S49)

    // r_grain.cpp:87-89: der Konstruktor zieht 491 Tabellenbytes + 1 Position.
    // Auch wenn wir die Tabelle (nur der NICHT-statische Pfad braucht sie) nicht
    // nachbilden: die Zuege muessen passieren, sonst laeuft der geteilte Strom
    // fuer alle anderen Effekte des Presets aus dem Takt.
    if (!rt.grainSeeded)
    {
        for (int i = 0; i < 491; ++i) rnd.nextRand();
        rnd.nextRand();
        rt.grainSeeded = true;
    }
    if (rt.grainTex == 0 || rt.grainW != m_surfaceWidth || rt.grainH != m_surfaceHeight)
    {
        // r_grain.cpp:133-141 (reinit): je Pixel zwei Zuege, Zeilen von oben.
        std::vector<unsigned char> depth(
            static_cast<std::size_t>(m_surfaceWidth) * m_surfaceHeight * 2);
        for (std::size_t i = 0; i < depth.size(); i += 2)
        {
            depth[i] = static_cast<unsigned char>(rnd.nextRand() % 255);
            depth[i + 1] = static_cast<unsigned char>(rnd.nextRand() % 100);
        }
        f->glActiveTexture(GL_TEXTURE1);
        if (rt.grainTex == 0)
        {
            f->glGenTextures(1, &rt.grainTex);
            f->glBindTexture(GL_TEXTURE_2D, rt.grainTex);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
            f->glBindTexture(GL_TEXTURE_2D, rt.grainTex);
        }
        f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, m_surfaceWidth, m_surfaceHeight, 0,
                        GL_RG, GL_UNSIGNED_BYTE, depth.data());
        f->glActiveTexture(GL_TEXTURE0);
        rt.grainW = m_surfaceWidth;
        rt.grainH = m_surfaceHeight;
    }
    rnd.nextRand();  // r_grain.cpp:168: ein Zug je Frame (randtab_pos-Vorschub)

    // Der Zustand VOR den Zuegen dieses Frames ist der Startwert des Shaders
    // (nur im nicht-statischen Zweig gelesen).
    const int seed = static_cast<int>(rnd.randState());

    m_grainShader->bind();
    m_grainShader->setUniformValue("uResX", m_surfaceWidth);
    m_grainShader->setUniformValue("uResY", m_surfaceHeight);
    m_grainShader->setUniformValue("uSmax",
                                   (std::clamp(static_cast<int>(vAmount), 0, 100) *
                                    255) / 100);
    m_grainShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_grainShader->setUniformValue("uDepth", 1);
    m_grainShader->setUniformValue("uStatic", params.staticGrain ? 1 : 0);
    m_grainShader->setUniformValue("uSeed", seed);
    m_grainShader->release();
    // Der nicht-statische Pfad zieht in der Referenz je 16 `fastrandbyte()` ein
    // `rand()%73` — bei w*h*2 Bytes also (w*h*2)/16 Zuege aus dem GETEILTEN
    // Strom, damit die folgenden Effekte des Presets an derselben Stelle
    // weitermachen. Das ist die OBERGRENZE: die Referenz zieht den Faktor nur,
    // wenn die Schwelle trifft, und schwarze Pixel ueberspringt sie ganz — die
    // wirkliche Zahl haengt am Bildinhalt und ist parallel nicht bestimmbar.
    // Eine deterministische Obergrenze ist hier die bessere Wahl als gar kein
    // Zug (der waere sicher falsch, sobald ein Pixel nicht schwarz ist).
    if (!params.staticGrain)
    {
        rnd.skipRandom(static_cast<std::uint32_t>(m_surfaceWidth) *
                       static_cast<std::uint32_t>(m_surfaceHeight) * 2u / 16u);
    }
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.grainTex);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_grainShader);
}

void MultiEffectVisualizer::runScatter(const ScatterParams&)
{
    // Der Shader liest den Strom selbst (ein Zug je Pixel der Mittelzone);
    // hier wird er anschliessend um genau diese Anzahl weitergestellt, damit
    // die naechsten Effekte des Presets an derselben Stelle weitermachen.
    m_scatterShader->bind();
    m_scatterShader->setUniformValue("uResX", m_surfaceWidth);
    m_scatterShader->setUniformValue("uResY", m_surfaceHeight);
    m_scatterShader->setUniformValue(
        "uSeed", static_cast<int>(m_scriptContext->randState()));
    m_scatterShader->release();
    if (m_surfaceHeight > 8)
    {
        m_scriptContext->skipRandom(
            static_cast<std::uint32_t>(m_surfaceWidth) *
            static_cast<std::uint32_t>(m_surfaceHeight - 8));
    }
    transformPass(*m_scatterShader);
}

void MultiEffectVisualizer::runInterferences(const ChainNode& node,
                                             const InterferencesParams& params)
{
    constexpr float kPi = 3.14159265358979323846f;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vPoints = params.points, vDist = params.distance;
    double vAlpha = params.alpha, vRotation = params.rotation;
    double vRotInc = params.rotationInc, vSpeed = params.speed;
    runParamScript(rt, "interferences", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"points", &vPoints},
                    {"distance", &vDist},
                    {"alpha", &vAlpha},
                    {"rotation", &vRotation},
                    {"rotationinc", &vRotInc},
                    {"speed", &vSpeed}});

    const int points = std::clamp(static_cast<int>(vPoints), 1, 8);
    // `rotation` ist ein STARTWERT: danach laeuft `interfRotation` selbstaendig
    // weiter (unten `+= rotInc`). Ein Panel-Edit ruft nur `recompileChain()`,
    // nicht `resetRuntimes()` — mit `interfSeeded` allein blieb der Zaehler
    // deshalb ewig auf dem Wert des ERSTEN Aufbaus, und ein neu eingestellter
    // Startwinkel kam nie an (Strang F, S57: das einzige WIRKUNGSLOS des
    // Vollaufs). Der Vergleich laeuft gegen den PRESET-Wert, nicht gegen die
    // Frame-Kopie: sonst setzt ein Skript, das `rotation` je Frame schreibt,
    // den Zaehler in jedem Frame zurueck.
    if (!rt.interfSeeded || rt.interfRotationSeed != params.rotation)
    {
        rt.interfRotation = static_cast<float>(vRotation);
        rt.interfStatus = kPi;
        rt.interfRotationSeed = params.rotation;
        rt.interfSeeded = true;
    }

    // Beat morph between the two parameter sets via sin(status) (r_interf).
    if (params.onBeat && m_frameBeat && rt.interfStatus >= kPi) rt.interfStatus = 0.0f;
    const float s = std::sin(rt.interfStatus);
    // Die drei Uebergangswerte sind in der Referenz GANZZAHLIG — der `(int)`
    // steht dort um die Interpolation, nicht um das Ergebnis
    // (`_distance = distance + (int)((float)(distance2-distance) * s)`,
    // r_interf.cpp:194-196). Wir rechneten float und lagen dadurch je Frame bis
    // zu einen Schritt daneben.
    const int rotIncBasis = static_cast<int>(vRotInc);
    const int rotInc = rotIncBasis +
                       static_cast<int>(static_cast<float>(params.rotationInc2 -
                                                           rotIncBasis) * s);
    const int alphaBasis = static_cast<int>(vAlpha);
    const int alpha = alphaBasis +
                      static_cast<int>(static_cast<float>(params.alpha2 -
                                                          alphaBasis) * s);
    const int distBasis = static_cast<int>(vDist);
    const int dist = distBasis +
                     static_cast<int>(static_cast<float>(params.distance2 -
                                                         distBasis) * s);

    // Copy offsets, evenly spaced around the accumulating rotation `a`.
    //
    // Die Versaetze sind GANZE PIXEL: `xpoints[i] = (int)(cos(a)*_distance)`
    // (r_interf.cpp:205-206). Bis S57 gaben wir den Bruchteil an den Shader
    // weiter, und der interpoliert dazwischen — aus scharfen Kopien wurde eine
    // Weichzeichnung. Das war der Matrix-Rest `41_interferences`.
    float a = static_cast<float>(rt.interfRotation) / 255.0f * 2.0f * kPi;
    const float angle = 2.0f * kPi / static_cast<float>(points);
    QVector2D offsets[8];
    for (int i = 0; i < points; ++i)
    {
        const int px = static_cast<int>(std::cos(a) * static_cast<float>(dist));
        const int py = static_cast<int>(std::sin(a) * static_cast<float>(dist));
        offsets[i] = QVector2D(static_cast<float>(px) /
                                   static_cast<float>(m_surfaceWidth),
                               static_cast<float>(py) /
                                   static_cast<float>(m_surfaceHeight));
        a += angle;
    }

    m_interfShader->bind();
    m_interfShader->setUniformValue("uPoints", points);
    m_interfShader->setUniformValueArray("uOffsets", offsets, 8);
    m_interfShader->setUniformValue(
        "uAlpha255", static_cast<float>(std::clamp(alpha, 0, 255)));
    m_interfShader->setUniformValue("uRgb", params.rgb ? 1 : 0);
    m_interfShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_interfShader->release();
    transformPass(*m_interfShader);

    // Advance rotation + morph phase for the next frame. `rotation` ist in der
    // Referenz ein INT und wird ganzzahlig akkumuliert (r_interf.cpp:384-386) —
    // eine float-Summe laeuft anders auf und dreht die Kopien mit der Zeit an
    // eine andere Stelle.
    rt.interfRotation += rotInc;
    if (rt.interfRotation > 255) rt.interfRotation -= 255;
    if (rt.interfRotation < -255) rt.interfRotation += 255;
    rt.interfStatus = std::min(rt.interfStatus + static_cast<float>(vSpeed), kPi);
    if (rt.interfStatus < -kPi) rt.interfStatus = kPi;  // r_interf.cpp:391
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
    m_waterShader->setUniformValue("uResX", m_surfaceWidth);
    m_waterShader->setUniformValue("uResY", m_surfaceHeight);
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
    double scriptBi = 1.0;
    {
        auto& engine = rt.bumpHost->engine();
        // r_bump.cpp:263-270: isbeat/islbeat sind -1 BEI Beat, +1 sonst
        engine.setNumber("isbeat", m_frameBeat ? -1.0 : 1.0);
        engine.setNumber("islbeat", rt.bumpFramesLeft > 0 ? -1.0 : 1.0);
        feedAudio(rt.bumpHost->engine());
        if (rt.bumpHost->has(Slot::Frame)) rt.bumpHost->run(Slot::Frame);
        if (m_frameBeat && rt.bumpHost->has(Slot::Beat)) rt.bumpHost->run(Slot::Beat);
        double lx = engine.number("x");
        double ly = engine.number("y");
        if (params.oldStyle) { lx /= 100.0; ly /= 100.0; }
        rt.bumpX = static_cast<float>(lx);
        rt.bumpY = static_cast<float>(ly);
        scriptBi = std::clamp(engine.number("bi"), 0.0, 1.0);
    }

    // r_bump.cpp:272-277: beim Beat springt die Tiefe auf depth2 (nF=durFrames);
    // ohne laufenden Burst gilt depth. Nach dem Rendern (402-410) faellt die
    // Tiefe LINEAR um a=|depth-depth2|/durFrames je Frame zurueck — a ist eine
    // INTEGER-Division; bei |depth-depth2| < durFrames ist a=0 und die Tiefe
    // haelt hart bis nF abgelaufen ist (der S46-Sonderfall).
    if (params.onBeat && m_frameBeat)
    {
        rt.bumpDepth = static_cast<float>(params.depth2);
        rt.bumpFramesLeft = std::max(1, params.durationFrames);
    }
    else if (rt.bumpFramesLeft == 0)
    {
        rt.bumpDepth = static_cast<float>(params.depth);
    }
    // r_bump.cpp:295-299: die Skript-Variable bi (0..1) skaliert die Tiefe
    const float thisDepth = rt.bumpDepth * static_cast<float>(scriptBi);
    if (rt.bumpFramesLeft > 0 && --rt.bumpFramesLeft > 0)
    {
        const int a = std::abs(params.depth - params.depth2) /
                      std::max(1, params.durationFrames);
        rt.bumpDepth += static_cast<float>(a) * (params.depth2 > params.depth
                                                     ? -1.0f : 1.0f);
    }

    // r_bump.cpp:249: depth source is the framebuffer, or global buffer N-1
    // when buffern is set; a missing buffer makes the effect a no-op.
    unsigned int depthTex = active().current()->texture();
    bool curbuf = true;
    if (params.buffern > 0)
    {
        QOpenGLFramebufferObject* pool = activePool().get(
            params.buffern - 1, m_surfaceWidth, m_surfaceHeight, false);
        if (pool == nullptr) return;
        depthTex = pool->texture();
        curbuf = false;
    }

    m_bumpShader->bind();
    m_bumpShader->setUniformValue("uRes",
                                  QVector2D(static_cast<float>(m_surfaceWidth),
                                            static_cast<float>(m_surfaceHeight)));
    m_bumpShader->setUniformValue("uLight", QVector2D(rt.bumpX, rt.bumpY));
    m_bumpShader->setUniformValue("uDepth", thisDepth * 256.0f / 100.0f);
    m_bumpShader->setUniformValue("uInvert", params.invert ? 1 : 0);
    m_bumpShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    m_bumpShader->setUniformValue("uDepthTex", 1);
    m_bumpShader->setUniformValue("uCurbuf", curbuf ? 1 : 0);
    m_bumpShader->release();
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, depthTex);
    f->glActiveTexture(GL_TEXTURE0);
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

    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    if (w < 2 || h < 2) return;

    // r_shift.cpp:191-197 — der Blend ist ein Vorzeichen, kein Faktor: bei
    // ialpha <= 0 kehrt der Effekt SOFORT zurueck (das Bild bleibt stehen), bei
    // >= 255 ist er abgeschaltet und das verschobene Bild gilt allein.
    int doBlend = params.blend ? 1 : 0;
    int iAlpha = 255;
    if (doBlend != 0)
    {
        iAlpha = static_cast<int>(alpha * 255.0);
        if (iAlpha <= 0) return;
        if (iAlpha >= 255) doBlend = 0;
    }

    // Ganzzahliger Anteil + 8-Bit-Bruchteil (r_shift.cpp:229-244). Der Zweig
    // `else { ++a; part = 255 - part; }` ist der Grund, warum eine Verschiebung
    // von 0 die erste und letzte Reihe kostet.
    auto teile = [](double v, int& a, int& part) {
        a = static_cast<int>(v);
        part = static_cast<int>((v - static_cast<int>(v)) * 255.0);
        if (part < 0)
        {
            part = -part;
        }
        else
        {
            ++a;
            part = 255 - part;
        }
        part = std::clamp(part, 0, 255);
    };

    int xa = 0, ya = 0, xpart = 0, ypart = 0, endX = 0, endY = 0;
    if (params.subpixel)
    {
        teile(ox, xa, xpart);
        teile(oy, ya, ypart);
        ya = std::clamp(ya, 1 - h, h - 1);
        xa = std::clamp(xa, 1 - w, w - 1);
        endX = std::clamp(w - 1 + xa, 0, w - 1);
        endY = std::clamp(h - 1 + ya, 0, h - 1);
    }
    else
    {
        // Ohne Subpixel gibt es keinen Bruchteil und die Grenzen laufen bis w/h
        // (r_shift.cpp:205-224).
        xa = std::min(static_cast<int>(ox), w);
        ya = std::min(static_cast<int>(oy), h);
        endX = std::min(w + xa, w);
        endY = std::min(h + ya, h);
    }

    m_shiftShader->bind();
    m_shiftShader->setUniformValue("uXa", xa);
    m_shiftShader->setUniformValue("uYa", ya);
    m_shiftShader->setUniformValue("uXpart", xpart);
    m_shiftShader->setUniformValue("uYpart", ypart);
    m_shiftShader->setUniformValue("uEndX", endX);
    m_shiftShader->setUniformValue("uEndY", endY);
    m_shiftShader->setUniformValue("uW", w);
    m_shiftShader->setUniformValue("uH", h);
    m_shiftShader->setUniformValue("uBlend", doBlend);
    m_shiftShader->setUniformValue("uIAlpha", iAlpha);
    m_shiftShader->setUniformValue("uSubpixel", params.subpixel ? 1 : 0);
    m_shiftShader->release();
    transformPass(*m_shiftShader);
}

namespace {
/// AVS scope colour-table cycling (r_simple/oscstar/oscring/rotstar): advances
/// colorPos and returns the interpolated 0x00RRGGBB colour.
// ---------------------------------------------------------- matrix.cpp 1:1
// Die 3D-Helfer des Originals (`matrixRotate`/`Translate`/`Multiply`/`Apply`).
// Dot Plane und Dot Fountain bauen exakt dieselbe Matrix — deshalb stehen sie
// hier einmal und nicht als Lambda-Paar in jedem Renderer.
void avsMatRotate(float* m, int axis, float deg)
{
    const float rad = deg * 3.141592653589f / 180.0f;
    std::memset(m, 0, sizeof(float) * 16);
    m[((axis - 1) << 2) + axis - 1] = m[15] = 1.0f;
    const int m1 = axis % 3;
    const int m2 = (m1 + 1) % 3;
    const float c = std::cos(rad);
    const float s = std::sin(rad);
    m[(m1 << 2) + m1] = c;
    m[(m1 << 2) + m2] = s;
    m[(m2 << 2) + m2] = c;
    m[(m2 << 2) + m1] = -s;
}

void avsMatTranslate(float* m, float x, float y, float z)
{
    std::memset(m, 0, sizeof(float) * 16);
    m[0] = m[4 + 1] = m[8 + 2] = m[12 + 3] = 1.0f;
    m[3] = x;
    m[4 + 3] = y;
    m[8 + 3] = z;
}

void avsMatMultiply(float* dest, const float* src)
{
    float temp[16];
    std::memcpy(temp, dest, sizeof(temp));
    for (int i = 0; i < 16; i += 4)
    {
        for (int col = 0; col < 4; ++col)
        {
            dest[i + col] = src[i + 0] * temp[0 + col] + src[i + 1] * temp[4 + col] +
                            src[i + 2] * temp[8 + col] + src[i + 3] * temp[12 + col];
        }
    }
}

void avsMatApply(const float* m, float x, float y, float z, float& ox, float& oy,
                 float& oz)
{
    ox = x * m[0] + y * m[1] + z * m[2] + m[3];
    oy = x * m[4] + y * m[5] + z * m[6] + m[7];
    oz = x * m[8] + y * m[9] + z * m[10] + m[11];
}

/// Die 64-Eintrag-Farbtabelle aus fuenf Stuetzstellen — `initcolortab` steht in
/// r_dotpln.cpp und r_dotfnt.cpp WORTGLEICH: vier Segmente zu je 16 Schritten,
/// exakte 16.16-Ganzzahlarithmetik.
void avsInitColorTab(const uint32_t (&colors)[5], uint32_t (&tab)[64])
{
    for (int t = 0; t < 4; ++t)
    {
        const int c0 = static_cast<int>(colors[t]);
        const int c1 = static_cast<int>(colors[t + 1]);
        int r = (c0 & 255) << 16;
        int g = ((c0 >> 8) & 255) << 16;
        int b = ((c0 >> 16) & 255) << 16;
        const int dr = (((c1 & 255) - (c0 & 255)) << 16) / 16;
        const int dg = ((((c1 >> 8) & 255) - ((c0 >> 8) & 255)) << 16) / 16;
        const int db = ((((c1 >> 16) & 255) - ((c0 >> 16) & 255)) << 16) / 16;
        for (int x = 0; x < 16; ++x)
        {
            tab[t * 16 + x] = static_cast<uint32_t>((r >> 16) | ((g >> 16) << 8) |
                                                    ((b >> 16) << 16));
            r += dr;
            g += dg;
            b += db;
        }
    }
}

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
        // Gemessene APE-Kennlinie (colormap_probe, S57): DREI ganzzahlige
        // Schritte, jeder schneidet ab. Die APE rechnet die Schrittweite je
        // Segment einmal als 16.16-Festkommazahl aus und akkumuliert damit:
        //
        //   step = 65536 / span          <- der eigentliche Verlust
        //   t    = (d * step) >> 8       <- Mischfaktor in 1/256
        //   out  = (a*(256-t) + b*t) >> 8
        //
        // Das erklaert, warum Zweierpotenzen exakt sind und 200 nicht:
        // 65536/16 = 4096 geht auf, 65536/200 = 327,68 -> 327 verliert.
        // Gemessen ueber die vollen Segmente von sechs Spannweiten:
        // **920 von 922 Punkten**, und die zwei Ausreisser sind die
        // Segment-Endpunkte, an denen der Zweig oben die Stuetzstelle ohnehin
        // direkt setzt.
        //
        // Bis S57 stand hier `a + (b-a)*d/span`. Das trifft nur bei
        // Zweierpotenzen (864/922) — auf dem Graukeil war JEDER Punkt 1 zu
        // hoch (die Referenz gibt `i-1`), und genau das war der „±1"-Befund
        // seit S49.
        const int span = p1 - p0;
        const int step = 65536 / span;
        const int t = std::min(((i - p0) * step) >> 8, 256);
        auto lerp8 = [&](int shift) {
            const int a = static_cast<int>(rgb(c0, shift));
            const int b = static_cast<int>(rgb(c1, shift));
            return static_cast<float>((a * (256 - t) + b * t) >> 8);
        };
        put(i, lerp8(16), lerp8(8), lerp8(0));
    }
}
}  // namespace

void MultiEffectVisualizer::runColorMap(const ChainNode& node,
                                        const ColorMapParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vKey = params.key, vBlendMode = params.blendMode, vAdjust = params.adjustBlend;
    runParamScript(rt, "colormap", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"key", &vKey},
                    {"blendmode", &vBlendMode},
                    {"adjustblend", &vAdjust}});

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
            // NEAREST: der Shader greift die Tabelle mit texelFetch ab (s. o.)
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
    // Strang D: die Frame-Kopien
    m_colorMapShader->setUniformValue("uKey",
                                      std::clamp(static_cast<int>(vKey), 0, 5));
    m_colorMapShader->setUniformValue("uBlend",
                                      std::clamp(static_cast<int>(vBlendMode), 0, 9));
    m_colorMapShader->setUniformValue(
        "uAdjust", static_cast<float>(vAdjust) / 255.0f);  // Strang D
    m_colorMapShader->setUniformValue("uLut", 1);
    m_colorMapShader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.cmTexture);
    f->glActiveTexture(GL_TEXTURE0);
    transformPass(*m_colorMapShader);
}

void MultiEffectVisualizer::runBufferBlend(const ChainNode& node,
                                           const BufferBlendParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vBufA = params.bufferA, vBufB = params.bufferB, vMode = params.mode;
    runParamScript(rt, "bufferblend", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"buffera", &vBufA},
                    {"bufferb", &vBufB},
                    {"mode", &vMode}});

    auto* f = QOpenGLContext::currentContext()->functions();
    SurfacePair& pair = active();
    const unsigned int cur = pair.current()->texture();
    const int bufA = static_cast<int>(vBufA);  // Strang D: die Frame-Kopien
    const int bufB = static_cast<int>(vBufB);
    unsigned int texA = bufA >= 8 ? cur : poolTexture(bufA);
    unsigned int texB = bufB >= 8 ? cur : poolTexture(bufB);
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
    m_bufferBlendShader->setUniformValue(
        "uMode", std::clamp(static_cast<int>(vMode), 0, 10));  // Strang D
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
    // S3/S9: die Referenz-Scopes zeichnen IMMER ueber BLEND_LINE
    // (r_simple/r_oscstar/r_oscring/r_rotstar/r_bspin via linedraw.cpp) —
    // Default ist REPLACE, additiv nur wenn ein Set Render Mode es sagt.
    applyLineBlend(m_renderMode.lineBlend, m_renderMode.alpha);
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = dots ? lumi::modules::SuperscopeRenderMode::Dots
                   : lumi::modules::SuperscopeRenderMode::Lines;
    rp.lineWidth = (m_renderMode.set && m_renderMode.lineWidth > 0)
                       ? static_cast<float>(m_renderMode.lineWidth)
                       : 1.0f;
    rp.dotSize = 1.0f;  // AVS-Dots sind 1 px (linedraw.cpp), Befund B (S46)
    rp.glowEnabled = false;
    m_scopeRenderer.draw(pts, rp);
    resetLineBlend();
}

void MultiEffectVisualizer::runSimpleScope(const ChainNode& node,
                                           const SimpleScopeParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vMode = params.mode, vChannel = params.channel, vPosition = params.position;
    runParamScript(rt, "simplescope", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"mode", &vMode},
                    {"channel", &vChannel},
                    {"position", &vPosition}});

    // Strang D: ab hier gilt die Frame-Kopie, nicht mehr der Regler.
    const int mode = static_cast<int>(vMode);
    const int channel = static_cast<int>(vChannel);
    const int position = static_cast<int>(vPosition);

    // r_simple.cpp zeilengenau (S48-Matrix-Befund 00): Analyzer lesen das
    // SPEKTRUM (200 Baender, xs=200/w), Scopes die Waveform (Byte^128);
    // solid = eine vertikale Linie je Bildspalte. Pixel-Mathematik auf der
    // Surface, dann Pixelzentren -> NDC fuer den ScopeRenderer (AVS y+ unten).
    const QVector3D c = colorToVec(cycleScopeColor(params.colors, rt.scopeColorPos));
    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    if (w < 2 || h < 2) return;

    const bool analyzer =
        mode == 0 || mode == 1 || mode == 4;
    unsigned char center[577];
    const unsigned char* fa;
    if (channel >= 2)
    {
        // Original: char-Arithmetik (visdata ist char[]) — signed halbieren,
        // Ueberlauf wrappt wie der char-Store (Spektrum-Bytes > 127!).
        const unsigned char* l = analyzer ? visSpectrum(0) : visWaveform(0);
        const unsigned char* r = analyzer ? visSpectrum(1) : visWaveform(1);
        for (int i = 0; i < 576; ++i)
        {
            const int v = static_cast<signed char>(l[i]) / 2 +
                          static_cast<signed char>(r[i]) / 2;
            center[i] = static_cast<unsigned char>(v);
        }
        center[576] = center[575];
        fa = center;
    }
    else
    {
        fa = analyzer ? visSpectrum(channel) : visWaveform(channel);
    }

    const float yscale = static_cast<float>(h) / 2.0f / 256.0f;
    const float xscale = 288.0f / static_cast<float>(w);
    std::vector<lumi::modules::SuperscopePoint> pts;
    const auto push = [&](float px, float py) {
        lumi::modules::SuperscopePoint p;
        p.x = (px + 0.5f) / static_cast<float>(w) * 2.0f - 1.0f;
        p.y = -((py + 0.5f) / static_cast<float>(h) * 2.0f - 1.0f);  // AVS y+ unten
        p.r = c.x();
        p.g = c.y();
        p.b = c.z();
        p.a = 1.0f;
        pts.push_back(p);
    };
    // ScopeRenderer: skip-Punkte werden VERWORFEN und trennen Segmente —
    // je Spalte also A, B und dann ein Trennpunkt (Koordinaten egal).
    const auto pushBreak = [&] {
        lumi::modules::SuperscopePoint p;
        p.skip = true;
        pts.push_back(p);
    };
    // Interpoliertes Analyzer-/Scope-Sample an Position r (Original s1-Mix).
    const auto sampleAt = [&](float r, bool xorWave) {
        const int i0 = std::clamp(static_cast<int>(r), 0, 575);
        const int i1 = std::min(i0 + 1, 576 - (fa == center ? 0 : 1));
        const float s1 = r - static_cast<float>(i0);
        const float a = static_cast<float>(xorWave ? (fa[i0] ^ 128) : fa[i0]);
        const float b = static_cast<float>(xorWave ? (fa[i1] ^ 128) : fa[i1]);
        return a * (1.0f - s1) + b * s1;
    };

    switch (mode)
    {
        case 0:  // solid analyzer (r_simple case 0)
        case 4:  // dot analyzer (Bit 6, case 0)
        {
            int h2 = h / 2;
            float ys = yscale;
            const float xs = 200.0f / static_cast<float>(w);
            int adj = 1;
            if (position != 1)
            {
                ys = -ys;
                adj = 0;
            }
            if (position == 2) h2 -= static_cast<int>(ys * 256.0f / 2.0f);
            for (int x = 0; x < w; ++x)
            {
                const float yr = sampleAt(static_cast<float>(x) * xs, false);
                const int yTip =
                    h2 + adj + static_cast<int>(yr * ys - 1.0f);
                if (mode == 4)
                {
                    if (yTip >= 0 && yTip < h)
                        push(static_cast<float>(x), static_cast<float>(yTip));
                }
                else
                {
                    push(static_cast<float>(x), static_cast<float>(h2 - adj));
                    push(static_cast<float>(x), static_cast<float>(yTip));
                    pushBreak();
                }
            }
            break;
        }
        case 3:  // solid scope (r_simple case 3)
        case 5:  // dot scope (Bit 6, case 2)
        {
            int yh = position * h / 2;
            if (position == 2) yh = h / 4;
            const int ysBase = yh + static_cast<int>(yscale * 128.0f);
            for (int x = 0; x < w; ++x)
            {
                const float yr = sampleAt(static_cast<float>(x) * xscale, true);
                const int yTip = yh + static_cast<int>(yr * yscale);
                if (mode == 5)
                {
                    if (yTip >= 0 && yTip < h)
                        push(static_cast<float>(x), static_cast<float>(yTip));
                }
                else
                {
                    push(static_cast<float>(x), static_cast<float>(ysBase - 1));
                    push(static_cast<float>(x), static_cast<float>(yTip));
                    pushBreak();
                }
            }
            break;
        }
        case 1:  // line analyzer (200 Punkte, xs = w/200)
        {
            int h2 = h / 2;
            float ys = yscale;
            if (position != 1) ys = -ys;
            if (position == 2) h2 -= static_cast<int>(ys * 256.0f / 2.0f);
            const float xs = 1.0f / xscale * (288.0f / 200.0f);
            push(0.0f, static_cast<float>(h2 + static_cast<int>(fa[0] * ys)));
            for (int x = 1; x < 200; ++x)
            {
                push(static_cast<float>(static_cast<int>(x * xs)),
                     static_cast<float>(h2 + static_cast<int>(fa[x] * ys)));
            }
            break;
        }
        case 2:  // line scope (288 Punkte, xs = w/288)
        default:
        {
            int yh = position * h / 2;
            if (position == 2) yh = h / 4;
            const float xs = 1.0f / xscale;
            push(0.0f,
                 static_cast<float>(yh + static_cast<int>(
                                        static_cast<int>(fa[0] ^ 128) * yscale)));
            for (int x = 1; x < 288; ++x)
            {
                push(static_cast<float>(static_cast<int>(x * xs)),
                     static_cast<float>(yh + static_cast<int>(
                                            static_cast<int>(fa[x] ^ 128) * yscale)));
            }
            break;
        }
    }

    drawScopeShape(pts, mode >= 4);
}

void MultiEffectVisualizer::runBassSpin(const ChainNode& node,
                                        const BassSpinParams& params)
{
    // r_bspin.cpp zeilengenau (S48-Matrix-Befund 07): rohe SPEKTRUM-Bytes je
    // Kanal (visdata[0][y]), last_a ist EIN Member ueber beide Kanaele,
    // Geometrie in Pixeln (ss = min(h/2, 3w/8)), Modus 1 zeichnet GEFUELLTE
    // Dreiecke zwischen Zentrum, letzter und neuer Speichenspitze.
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    if (w < 2 || h < 2) return;

    // Strang D auf einer Frame-Kopie. KLASSE A — s. Hinweis bei Moving Particle.
    double vMode = params.mode, vSmooth = params.smoothing, vStep = params.spinStep;
    runParamScript(rt, "bassspin", params.initCode, params.frameCode, params.beatCode,
                   {{"mode", &vMode}, {"smoothing", &vSmooth}, {"spinstep", &vStep}});

    const auto ndcX = [&](int px) {
        return (static_cast<float>(px) + 0.5f) / static_cast<float>(w) * 2.0f - 1.0f;
    };
    const auto ndcY = [&](int py) {
        return -((static_cast<float>(py) + 0.5f) / static_cast<float>(h) * 2.0f -
                 1.0f);  // AVS y+ unten
    };

    for (int chn = 0; chn < 2; ++chn)
    {
        if (chn == 0 ? !params.left : !params.right) continue;

        const unsigned char* fa = visSpectrum(chn);
        const int ss = std::min(h / 2, (w * 3) / 8);
        const int cx = chn == 0 ? w / 2 - ss / 2 : w / 2 + ss / 2;
        int d = 0;
        for (int x = 0; x < 44; ++x) d += fa[x];
        int a = (d * 512) / (rt.bsLastA + 30 * 256);
        rt.bsLastA = d;
        if (a > 255) a = 255;
        // S53 freigemacht (Klasse A) — Vorgaben sind die r_bspin-Werte 0,7/pi 6.
        const float sm = std::clamp(static_cast<float>(vSmooth), 0.0f, 1.0f);
        rt.bsV[chn] = sm * (std::max(a - 104, 12) / 96.0f) + (1.0f - sm) * rt.bsV[chn];
        rt.bsRv[chn] +=
            static_cast<float>(vStep) * rt.bsV[chn] * (chn == 0 ? -1.0f : 1.0f);

        const double s = static_cast<double>(ss) * a / 256.0;
        const int xp = static_cast<int>(std::cos(rt.bsRv[chn]) * s);
        const int yp = static_cast<int>(std::sin(rt.bsRv[chn]) * s);
        const QVector3D col =
            colorToVec(chn == 0 ? params.colorLeft : params.colorRight);

        if (static_cast<int>(vMode) == 0)  // Linien: Speichen + Trail zur Spitze
        {
            std::vector<lumi::modules::SuperscopePoint> pts;
            const auto push = [&](int x, int y) {
                lumi::modules::SuperscopePoint p;
                p.x = ndcX(x);
                p.y = ndcY(y);
                p.r = col.x();
                p.g = col.y();
                p.b = col.z();
                p.a = 1.0f;
                pts.push_back(p);
            };
            const auto pushBreak = [&] {
                lumi::modules::SuperscopePoint p;
                p.skip = true;
                pts.push_back(p);
            };
            if (rt.bsLx[0][chn] != 0 || rt.bsLy[0][chn] != 0)
            {
                push(rt.bsLx[0][chn], rt.bsLy[0][chn]);
                push(cx + xp, h / 2 + yp);
                pushBreak();
            }
            rt.bsLx[0][chn] = cx + xp;
            rt.bsLy[0][chn] = h / 2 + yp;
            push(cx, h / 2);
            push(cx + xp, h / 2 + yp);
            pushBreak();
            if (rt.bsLx[1][chn] != 0 || rt.bsLy[1][chn] != 0)
            {
                push(rt.bsLx[1][chn], rt.bsLy[1][chn]);
                push(cx - xp, h / 2 - yp);
                pushBreak();
            }
            rt.bsLx[1][chn] = cx - xp;
            rt.bsLy[1][chn] = h / 2 - yp;
            push(cx, h / 2);
            push(cx - xp, h / 2 - yp);
            drawScopeShape(pts, false);
        }
        else  // Modus 1: gefuellte Dreiecke (my_triangle, Replace)
        {
            std::vector<float> tris;
            const auto pushTri = [&](int x1, int y1, int x2, int y2, int x3,
                                     int y3) {
                tris.insert(tris.end(), {ndcX(x1), ndcY(y1), ndcX(x2), ndcY(y2),
                                         ndcX(x3), ndcY(y3)});
            };
            if (rt.bsLx[0][chn] != 0 || rt.bsLy[0][chn] != 0)
            {
                pushTri(cx, h / 2, rt.bsLx[0][chn], rt.bsLy[0][chn], cx + xp,
                        h / 2 + yp);
            }
            rt.bsLx[0][chn] = cx + xp;
            rt.bsLy[0][chn] = h / 2 + yp;
            if (rt.bsLx[1][chn] != 0 || rt.bsLy[1][chn] != 0)
            {
                pushTri(cx, h / 2, rt.bsLx[1][chn], rt.bsLy[1][chn], cx - xp,
                        h / 2 - yp);
            }
            rt.bsLx[1][chn] = cx - xp;
            rt.bsLy[1][chn] = h / 2 - yp;
            drawFlatTriangles(tris, col);
        }
    }
}

bool MultiEffectVisualizer::ensureEmbeddedTexture(LeafRuntime& rt,
                                                  const std::string& imageData,
                                                  bool fallbackDot)
{
    // Ein Bildwechsel im Panel muss ankommen. Bis S56 stand hier nur
    // `picTexture != 0` — die Textur wurde einmal beim Aufbau erzeugt und blieb
    // danach stehen; neue `imageData` wirkten erst nach Speichern + Laden (dort
    // baut `resetRuntimes()` die Runtime neu). Gemessen mit den Edit-Sonden:
    // `texer.imageData` WIRKUNGSLOS. Gleiche Bauart wie `cmSnapshot`.
    if (rt.picTexture != 0 && rt.picSnapshot == imageData) return true;
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
        // missing (Acko default texture). Picture effects keep the hard fail.
        //
        // Das Default-Sprite ist GEMESSEN, nicht geraten: die 21x21-Matrix
        // unten ist der 1:1-Blit der Original-texer2.ape (AvsRef, resize aus,
        // weiss auf Schwarz — der no-resize-Pfad kopiert die interne Textur
        // unfiltriert zurueck; Community-Quelle e_texer2.cpp bestaetigt
        // iw=ih=21). Ersetzt die radial gemittelte 20x20-Rekonstruktion aus
        // S50 — deren Halbpixel-"Asymmetrie" war in Wahrheit die Subpixel-
        // Phase des RESIZE-Pfads, nicht die Textur (Befund S59).
        //
        // Alpha ist bewusst 255: der Sprite-Pass blendet GL_SRC_ALPHA/GL_ONE,
        // ein Alpha=v haette das Profil QUADRIERT (der alte 16er-Kegel tat das
        // und kam deshalb auf nur 14 px sichtbare Breite).
        if (!fallbackDot) return false;
        constexpr int kDot = 21;
        static constexpr std::array<unsigned char, kDot * kDot> kDefaultSprite = {
              0,   0,   0,   0,   0,   0,   2,   7,  12,  15,  17,  15,  12,   7,   2,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   2,  10,  21,  32,  41,  46,  49,  46,  41,  32,  21,  10,   2,   0,   0,   0,   0,
              0,   0,   0,   4,  17,  33,  49,  64,  76,  83,  86,  83,  76,  64,  49,  33,  17,   4,   0,   0,   0,
              0,   0,   4,  19,  39,  61,  82, 100, 115, 124, 127, 124, 115, 101,  82,  61,  39,  19,   4,   0,   0,
              0,   2,  16,  39,  65,  92, 118, 140, 155, 164, 167, 164, 155, 140, 118,  92,  65,  39,  16,   2,   0,
              0,   9,  32,  61,  92, 124, 153, 174, 191, 201, 204, 201, 191, 174, 153, 124,  92,  61,  32,   9,   0,
              2,  20,  48,  82, 117, 152, 181, 205, 221, 231, 234, 231, 221, 204, 181, 152, 117,  82,  48,  20,   2,
              6,  30,  63, 100, 139, 174, 204, 228, 241, 247, 249, 247, 241, 228, 204, 174, 139, 100,  63,  30,   6,
             11,  39,  75, 114, 154, 190, 220, 241, 249, 252, 253, 252, 249, 241, 220, 190, 154, 114,  75,  39,  10,
             14,  45,  82, 123, 163, 199, 230, 247, 252, 254, 254, 254, 252, 247, 230, 199, 163, 123,  82,  45,  14,
             15,  47,  84, 125, 165, 202, 232, 248, 253, 254, 254, 254, 253, 248, 232, 202, 165, 125,  84,  47,  15,
             14,  44,  81, 122, 162, 198, 229, 246, 252, 253, 254, 253, 252, 246, 229, 198, 162, 122,  81,  44,  14,
             10,  39,  73, 112, 153, 188, 219, 240, 249, 252, 253, 252, 249, 240, 219, 188, 153, 112,  73,  39,  10,
              5,  30,  62,  98, 137, 172, 202, 226, 240, 246, 248, 246, 240, 226, 202, 172, 137,  98,  62,  30,   5,
              2,  19,  47,  79, 115, 149, 178, 201, 218, 228, 231, 228, 218, 202, 178, 149, 115,  79,  47,  19,   1,
              0,   8,  31,  59,  89, 121, 149, 171, 187, 197, 200, 197, 187, 171, 149, 121,  89,  59,  31,   8,   0,
              0,   1,  15,  37,  62,  89, 114, 136, 152, 160, 163, 160, 152, 136, 114,  89,  63,  37,  15,   1,   0,
              0,   0,   3,  17,  37,  58,  79,  97, 111, 120, 123, 120, 111,  97,  79,  58,  37,  17,   3,   0,   0,
              0,   0,   0,   3,  14,  30,  46,  61,  72,  79,  82,  79,  72,  61,  46,  30,  14,   3,   0,   0,   0,
              0,   0,   0,   0,   1,   8,  18,  29,  37,  43,  45,  43,  37,  28,  18,   8,   1,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   1,   5,   9,  13,  14,  13,   9,   5,   2,   0,   0,   0,   0,   0,   0};
        img = QImage(kDot, kDot, QImage::Format_RGBA8888);
        for (int y = 0; y < kDot; ++y)
        {
            for (int x = 0; x < kDot; ++x)
            {
                const int b = kDefaultSprite[static_cast<std::size_t>(y) * kDot + x];
                img.setPixel(x, y, qRgba(b, b, b, 255));
            }
        }
    }
    img = img.convertToFormat(QImage::Format_RGBA8888);
    if (rt.picTexture == 0) f->glGenTextures(1, &rt.picTexture);
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
    rt.picSnapshot = imageData;
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
    m_pictureShader->setUniformValue("uBlend", std::clamp(blend, 0, 5));
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
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vBlend = params.blend;
    runParamScript(rt, "picture", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"blend", &vBlend}});

    drawEmbeddedImage(m_leafRuntimes[node.nodeId], params.imageData,
                      static_cast<int>(vBlend), params.keepAspect);  // Strang D
}

void MultiEffectVisualizer::runPictureII(const ChainNode& node,
                                         const PictureIIParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vBlend = params.blend;
    runParamScript(rt, "pictureii", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"blend", &vBlend}});

    // Picture II stretches to fill (no aspect lock); bilinear is already on.
    drawEmbeddedImage(m_leafRuntimes[node.nodeId], params.imageData,
                      static_cast<int>(vBlend), false);  // Strang D
}

namespace
{
/// r_text getWord: the n-th ';'-separated entry ("" when past the end).
std::string textWord(const std::string& text, int n)
{
    std::size_t start = 0;
    for (int w = 0; w < n; ++w)
    {
        const std::size_t sep = text.find(';', start);
        if (sep == std::string::npos) return {};
        start = sep + 1;
    }
    const std::size_t end = text.find(';', start);
    return text.substr(start, end == std::string::npos ? std::string::npos
                                                       : end - start);
}
/// r_text getNWords: number of ';' — word count is this + 1.
int textSeparators(const std::string& text)
{
    return static_cast<int>(std::count(text.begin(), text.end(), ';'));
}
}  // namespace

void MultiEffectVisualizer::runText(const ChainNode& node, const TextParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const int words = textSeparators(params.text) + 1;

    // --- word cycling (r_text render:544-600)
    const bool switchNow =
        (!params.onBeat && rt.textNf >= params.normSpeed) ||
        (params.onBeat && m_frameBeat && rt.textNb <= 0);
    if (switchNow)
    {
        rt.textNf = 0;
        if (!(params.insertBlank && (rt.textOddEven % 2) == 0))
        {
            if (params.randomWord)
                rt.textCurWord = static_cast<int>(nextRandom() %
                                                  static_cast<unsigned>(words));
            else
                rt.textCurWord = (rt.textCurWord + 1) % words;
        }
        rt.textOddEven = (rt.textOddEven + 1) % 2;
        if (params.onBeat) rt.textNb = params.onBeatSpeed;
        if (params.randomPos)
        {
            rt.textRandX = static_cast<float>(nextRandom() & 0xffff) / 65535.0f;
            rt.textRandY = static_cast<float>(nextRandom() & 0xffff) / 65535.0f;
        }
    }
    else if (params.onBeat && rt.textNb > 0)
    {
        --rt.textNb;  // beat lockout window (r_text nb)
    }
    ++rt.textNf;

    std::string word = textWord(params.text, rt.textCurWord);
    if (params.insertBlank && rt.textOddEven == 0) word.clear();
    if (word.empty()) return;  // nothing to draw this phase

    // --- glyph layer: redraw only when the draw state changed
    auto* f = QOpenGLContext::currentContext()->functions();
    const std::string snapshot =
        word + '\x1f' + params.fontFace + '\x1f' + std::to_string(params.fontHeight) +
        ':' + std::to_string(params.fontWeight) + ':' +
        std::to_string(params.italic) + std::to_string(params.underline) +
        std::to_string(params.color) + ':' + std::to_string(params.hAlign) +
        std::to_string(params.vAlign) + ':' + std::to_string(params.xShift) + ',' +
        std::to_string(params.yShift) + ':' + std::to_string(params.outline) +
        std::to_string(params.outlineColor) + std::to_string(params.outlineSize) +
        std::to_string(params.shadow) + ':' +
        std::to_string(params.randomPos ? rt.textRandX : -1.0f) + ',' +
        std::to_string(params.randomPos ? rt.textRandY : -1.0f) + ':' +
        std::to_string(m_surfaceWidth) + 'x' + std::to_string(m_surfaceHeight);
    if (rt.textTexture == 0 || rt.textSnapshot != snapshot)
    {
        rt.textSnapshot = snapshot;
        QImage img(m_surfaceWidth, m_surfaceHeight, QImage::Format_RGBA8888);
        img.fill(Qt::transparent);
        {
            QPainter painter(&img);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            QFont font;
            if (!params.fontFace.empty())
                font.setFamily(QString::fromStdString(params.fontFace));
            // LOGFONT: negative height = character pixel height
            font.setPixelSize(std::max(4, std::abs(params.fontHeight)));
            font.setWeight(params.fontWeight >= 600 ? QFont::Bold : QFont::Normal);
            font.setItalic(params.italic);
            font.setUnderline(params.underline);
            const QFontMetrics fm(font);
            const QString qword = QString::fromStdString(word);
            const int tw = fm.horizontalAdvance(qword);
            const int th = fm.height();
            int x = 0;
            int y = 0;
            if (params.randomPos)
            {
                // r_text: random offset, left/top-aligned (render:581-592)
                x = static_cast<int>(rt.textRandX *
                                     static_cast<float>(std::max(0, m_surfaceWidth - tw)));
                y = static_cast<int>(rt.textRandY *
                                     static_cast<float>(std::max(0, m_surfaceHeight - th)));
            }
            else
            {
                if (params.hAlign == 1) x = (m_surfaceWidth - tw) / 2;
                else if (params.hAlign == 2) x = m_surfaceWidth - tw;
                if (params.vAlign == 1) y = (m_surfaceHeight - th) / 2;
                else if (params.vAlign == 2) y = m_surfaceHeight - th;
                x += params.xShift * m_surfaceWidth / 100;   // shifts are percent
                y += params.yShift * m_surfaceHeight / 100;
            }
            const QPointF baseline(x, y + fm.ascent());
            auto toColor = [](std::uint32_t c) {
                return QColor(static_cast<int>((c >> 16) & 0xFF),
                              static_cast<int>((c >> 8) & 0xFF),
                              static_cast<int>(c & 0xFF));
            };
            QPainterPath glyphs;
            glyphs.addText(baseline, font, qword);
            if (params.shadow && !params.outline)
            {
                QPainterPath sh;
                sh.addText(baseline + QPointF(params.outlineSize, params.outlineSize),
                           font, qword);
                painter.fillPath(sh, toColor(params.outlineColor));
            }
            if (params.outline)
            {
                painter.setPen(QPen(toColor(params.outlineColor),
                                    params.outlineSize * 2.0));
                painter.setBrush(Qt::NoBrush);
                painter.drawPath(glyphs);
            }
            painter.fillPath(glyphs, toColor(params.color));
            if (params.underline)  // fillPath misses QFont underline: draw text too
            {
                painter.setPen(toColor(params.color));
                painter.setFont(font);
                painter.drawText(baseline, qword);
            }
        }
        if (rt.textTexture == 0)
        {
            f->glGenTextures(1, &rt.textTexture);
            f->glBindTexture(GL_TEXTURE_2D, rt.textTexture);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        f->glBindTexture(GL_TEXTURE_2D, rt.textTexture);
        f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
    }

    // --- compose over the base (blend on glyph pixels only)
    if (m_textShader == nullptr) return;
    SurfacePair& pair = active();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_textShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_textShader->setUniformValue("uTex", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.textTexture);
    m_textShader->setUniformValue("uImg", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_textShader->setUniformValue("uBlend", std::clamp(params.blend, 0, 2));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_textShader->release();
    pair.partner()->release();
    pair.swap();
    bindActive();
}

void MultiEffectVisualizer::closeAviRuntime(LeafRuntime& rt)
{
#ifdef _WIN32
    if (rt.aviGetFrame != nullptr)
        AVIStreamGetFrameClose(static_cast<PGETFRAME>(rt.aviGetFrame));
    if (rt.aviStream != nullptr)
        AVIStreamRelease(static_cast<PAVISTREAM>(rt.aviStream));
    if (rt.aviFile != nullptr) AVIFileRelease(static_cast<PAVIFILE>(rt.aviFile));
#endif
    rt.aviGetFrame = nullptr;
    rt.aviStream = nullptr;
    rt.aviFile = nullptr;
    rt.aviLength = 0;
    rt.aviClip.reset();  // Cache-Clip bleibt im VideoFrameCache erhalten
}

#ifdef _WIN32
namespace
{
/// Open the video stream via VfW with a 32bpp decompression target.
bool openAvi(void*& outFile, void*& outStream, void*& outGetFrame, int& outLength,
             const std::string& path)
{
    static bool vfwInit = false;
    if (!vfwInit)
    {
        AVIFileInit();
        vfwInit = true;
    }
    PAVIFILE file = nullptr;
    if (AVIFileOpenA(&file, path.c_str(), OF_READ, nullptr) != AVIERR_OK)
        return false;
    PAVISTREAM stream = nullptr;
    if (AVIFileGetStream(file, &stream, streamtypeVIDEO, 0) != AVIERR_OK)
    {
        AVIFileRelease(file);
        return false;
    }
    BITMAPINFOHEADER want{};
    want.biSize = sizeof(want);
    want.biPlanes = 1;
    want.biBitCount = 32;
    want.biCompression = BI_RGB;
    PGETFRAME gf = AVIStreamGetFrameOpen(stream, &want);
    if (gf == nullptr)
    {
        // Kann VfW nicht auf 32 Bit bringen, dann wenigstens auf 24 — beides
        // zeichnet der Renderer. Erst danach das Quellformat, das alles sein
        // darf (und dann in der Meldung landet).
        want.biBitCount = 24;
        gf = AVIStreamGetFrameOpen(stream, &want);
    }
    if (gf == nullptr) gf = AVIStreamGetFrameOpen(stream, nullptr);
    const int length = static_cast<int>(AVIStreamLength(stream));
    if (gf == nullptr || length <= 0)
    {
        if (gf != nullptr) AVIStreamGetFrameClose(gf);
        AVIStreamRelease(stream);
        AVIFileRelease(file);
        return false;
    }
    outFile = file;
    outStream = stream;
    outGetFrame = gf;
    outLength = length;
    return true;
}
}  // namespace
#endif

void MultiEffectVisualizer::runAvi(const ChainNode& node, const AviParams& params)
{
#ifndef _WIN32
    Q_UNUSED(node);
    Q_UNUSED(params);
#else
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const std::string& path =
        params.resolvedPath.empty() ? params.filename : params.resolvedPath;

    // Ein Pfadwechsel im Panel muss ankommen. Bis S55 merkte sich `aviTried`
    // nur DASS einmal geoeffnet wurde — danach blieb der Knoten auf der ersten
    // Datei stehen, und ein neuer Pfad wirkte erst nach Speichern + Laden
    // (dort baut `resetRuntimes()` die Runtime neu). Gemessen mit den
    // Edit-Sonden: `avi.filename` WIRKUNGSLOS, MAE 0,234.
    if (rt.aviPath != path)
    {
        closeAviRuntime(rt);
        if (rt.aviTexture != 0)
        {
            QOpenGLContext::currentContext()->functions()->glDeleteTextures(
                1, &rt.aviTexture);
            rt.aviTexture = 0;  // sonst zeigt der neue Pfad das alte Bild
        }
        rt.aviPath = path;
        rt.aviTried = false;
        rt.aviFrameIndex = 0;
        rt.aviWarnedBpp = 0;
        rt.aviClip.reset();
    }
    if (rt.aviGetFrame == nullptr && !rt.aviTried)
    {
        rt.aviTried = true;
        if (!path.empty() &&
            !openAvi(rt.aviFile, rt.aviStream, rt.aviGetFrame, rt.aviLength, path))
        {
            // Qt-Multimedia-Fallback (Entscheid S59, Stufe 1 des Video-Wegs):
            // das 64-Bit-VfW kennt Alt-Codecs wie Indeo 3.2 (el-visVR09) nicht
            // mehr — der FFmpeg-Backend-Decoder schon. Der Cache dekodiert die
            // Datei EINMAL komplett; gezeichnet wird deterministisch nach
            // Frame-INDEX, nicht uhrzeitgetrieben (Frame-Schritt-Pflicht).
            rt.aviClip = lumi::services::VideoFrameCache::instance().hole(
                QString::fromStdString(path));
        }
    }
    if (rt.aviGetFrame == nullptr &&
        (rt.aviClip == nullptr ||
         rt.aviClip->status.load(std::memory_order_acquire) !=
             lumi::services::VideoFrameCache::FERTIG))
        return;
    auto* f = QOpenGLContext::currentContext()->functions();

    // speed = min. milliseconds between frame advances (r_avi render:236-239).
    // Frame-Uhr statt Wanduhr (S59): die Referenz tickt zwar per GetTickCount,
    // aber nur mit der Skript-Uhr sind zwei Laeufe bit-identisch — dieselbe
    // Entscheidung wie bei gettime() (Feld-Sonden-Grundlage).
    const std::int64_t now = static_cast<std::int64_t>(m_scriptClock * 1000.0);
    // KEIN Sofort-Start bei fehlender Textur: die Referenz zeigt bis zum
    // ersten Gate-Ablauf (tick >= lastspeed+speed, lastspeed startet 0) ihr
    // leeres old_image — wir zuendeten Frame 0 sofort und liefen dauerhaft
    // EINEN Video-Frame vor (S60, Gate-Sonden: speed=0 exakt, 100/400 je
    // MAE 0,18 konstant). Vor dem ersten Advance zeichnet der Knoten nichts
    // (bewusste Naeherung: ref memcpy't dort ein nie beschriebenes Bild).
    if (rt.aviGetFrame == nullptr)
    {
        // Frames aus dem Qt-Decode-Cache (RGBX8888, top-down)
        const auto& frames = rt.aviClip->frames;
        if (now - rt.aviLastMs >= static_cast<std::int64_t>(params.speedMs))
        {
            rt.aviLastMs = now;
            rt.aviFrameIndex %= static_cast<int>(frames.size());
            const QImage& img = frames[static_cast<std::size_t>(rt.aviFrameIndex++)];
            if (rt.aviTexture == 0)
            {
                f->glGenTextures(1, &rt.aviTexture);
                f->glBindTexture(GL_TEXTURE_2D, rt.aviTexture);
                f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            f->glBindTexture(GL_TEXTURE_2D, rt.aviTexture);
            f->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(),
                            0, GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
        }
    }
    else if (now - rt.aviLastMs >= static_cast<std::int64_t>(params.speedMs))
    {
        rt.aviLastMs = now;
        rt.aviFrameIndex %= std::max(1, rt.aviLength);
        const void* frame = AVIStreamGetFrame(
            static_cast<PGETFRAME>(rt.aviGetFrame), rt.aviFrameIndex++);
        if (frame != nullptr)
        {
            const auto* bih = static_cast<const BITMAPINFOHEADER*>(frame);
            const auto* bits = reinterpret_cast<const unsigned char*>(bih) +
                               bih->biSize + bih->biClrUsed * 4u;
            const int fw = static_cast<int>(bih->biWidth);
            const int fh = std::abs(static_cast<int>(bih->biHeight));
            const int bpp = static_cast<int>(bih->biBitCount);
            if ((bpp == 32 || bpp == 24) && fw > 0 && fh > 0)
            {
                // 32bpp DIB = BGRX little-endian = QImage Format_RGB32, 24bpp
                // = BGR mit auf 4 Bytes aufgerundeter Zeilenlaenge. 24 Bit ist
                // bei unkomprimierten AVIs der Normalfall — bis S55 verwarf
                // dieser Zweig sie stillschweigend, der Knoten oeffnete die
                // Datei und zeichnete nichts. DIBs mit positiver Hoehe sind
                // bottom-up -> fuer den Overlay-Shader auf top-down drehen.
                const int stride = (bpp == 32) ? fw * 4 : ((fw * 3 + 3) & ~3);
                QImage wrap(bits, fw, fh, stride,
                            bpp == 32 ? QImage::Format_RGB32
                                      : QImage::Format_BGR888);
                QImage img = (bih->biHeight > 0 ? wrap.flipped(Qt::Vertical)
                                                : wrap.copy())
                                 .convertToFormat(QImage::Format_RGBX8888);
                if (rt.aviTexture == 0)
                {
                    f->glGenTextures(1, &rt.aviTexture);
                    f->glBindTexture(GL_TEXTURE_2D, rt.aviTexture);
                    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                       GL_LINEAR);
                    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                       GL_LINEAR);
                    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                       GL_CLAMP_TO_EDGE);
                    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                       GL_CLAMP_TO_EDGE);
                }
                f->glBindTexture(GL_TEXTURE_2D, rt.aviTexture);
                f->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(),
                                img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                                img.constBits());
            }
            else if (rt.aviWarnedBpp != bpp)
            {
                // Einmal je Tiefe, nicht je Frame. Ohne diese Meldung sieht ein
                // nicht darstellbares Video exakt aus wie eine fehlende Datei.
                rt.aviWarnedBpp = bpp;
                BasicLogger::logWarning(
                    "MultiEffect: AVI-Video mit " + std::to_string(bpp) +
                    " Bit je Punkt wird nicht gezeichnet (unterstuetzt: 24, 32)");
            }
        }
    }
    if (rt.aviTexture == 0 || m_textShader == nullptr) return;

    // beat persist window (r_avi render:262-265) + adapt blend selection
    if (m_frameBeat) rt.aviPersistLeft = params.persist;
    else if (rt.aviPersistLeft > 0) --rt.aviPersistLeft;
    int blend = params.blend;
    if (params.adapt) blend = (m_frameBeat || rt.aviPersistLeft > 0) ? 1 : 2;

    // full-frame stretch through the shared overlay shader (alpha = 1)
    SurfacePair& pair = active();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_textShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_textShader->setUniformValue("uTex", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.aviTexture);
    m_textShader->setUniformValue("uImg", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_textShader->setUniformValue("uBlend", std::clamp(blend, 0, 2));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_textShader->release();
    pair.partner()->release();
    pair.swap();
    bindActive();
#endif
}

void MultiEffectVisualizer::runTexer(const ChainNode& node, const TexerParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vParticles = params.particles, vBlend = params.blend;
    runParamScript(rt, "texer", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"particles", &vParticles},
                    {"blend", &vBlend}});

    if (!ensureEmbeddedTexture(rt, params.imageData, /*fallbackDot=*/true)) return;
    const std::vector<float> wave = getWaveform();
    const int wn = static_cast<int>(wave.size());
    const int n = std::clamp(static_cast<int>(vParticles), 1, 4096);  // Strang D

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    // Die drei Betriebsarten des Feldes (0 ersetzen, 1 additiv, 2 50/50). Bis
    // S57 kannte diese Stelle nur "0" und "alles andere": 1 und 2 waren
    // derselbe GL-Zustand, 50/50 gab es also gar nicht, und "ersetzen" war
    // additiv ohne Alpha-Gewichtung. Die Feld-Sonde konnte das nicht zeigen —
    // ihr Gegenwert 2 traf denselben Zustand wie die Vorgabe 1 (MAE 0,0000).
    // Dieselben Faktoren wie `applyLineBlend` (Referenz r_defs.h BLEND_LINE),
    // damit ein Sprite genauso mischt wie eine Linie.
    switch (std::clamp(static_cast<int>(vBlend), 0, 2))  // Strang D
    {
        case 0:  f->glBlendFunc(GL_ONE, GL_ZERO); break;             // ersetzen
        case 2:                                                       // 50/50
            f->glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
            f->glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);
            break;
        default: f->glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;         // additiv
    }
    m_spriteShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, rt.picTexture);
    m_spriteShader->setUniformValue("uImg", 0);
    m_spriteShader->setUniformValue("uTint", QVector3D(1.0f, 1.0f, 1.0f));
    m_spriteShader->setUniformValue("uColorFilter", 0);
    // Altes Verhalten: volle Textur, y gespiegelt (s. kSpriteVertexShader)
    m_spriteShader->setUniformValue("uUv0", QVector2D(0.0f, 1.0f));
    m_spriteShader->setUniformValue("uUv1", QVector2D(1.0f, 0.0f));
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
        // Vorbelegung 0, NICHT 100: setzt das Skript kein `n`, zeichnet Texer II
        // gar nichts. Gemessen (S50): ein Knoten mit leerem Init liefert in
        // AvsRef 0 Pixel, unsere 100er-Vorbelegung 4124 — mit explizitem n=100
        // stimmen beide ueberein.
        rt.texerHost->engine().setNumber("n", 0.0);
        // w/h muessen schon den INIT erreichen: die Paar-Sonde
        // 5_vars/texer2_n_aus_w_init (n=w*0.1 im Init) zeichnet in AvsRef
        // dieselben 6526 Pixel wie das Literal n=32 (S51).
        rt.texerHost->engine().setNumber("w", static_cast<double>(m_surfaceWidth));
        rt.texerHost->engine().setNumber("h", static_cast<double>(m_surfaceHeight));
        rt.texerHost->run(Slot::Init);
    }
    auto& engine = rt.texerHost->engine();
    engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
    // Bildschirmmasse: Texer-II-Skripte rechnen Sprite-Zahl UND -Groesse daraus.
    // "Alien Alloy" setzt im Frame-Slot n=w*0.1 und reg00=h/280 — ohne w/h war
    // n=0, alle vier Texer zeichneten NICHTS und das Preset blieb schwarz
    // (Befund S51; die S50-Notiz "leerer Init" war die falsche Spur, `n` steht
    // im FRAME-Slot). Werte per Paar-Sonde gepinnt: n=w*0.1 und n=32 ergeben
    // bei 320x240 dasselbe Referenzbild, ebenso sizex=h/120 und sizex=2.
    engine.setNumber("w", static_cast<double>(m_surfaceWidth));
    engine.setNumber("h", static_cast<double>(m_surfaceHeight));
    // Neutrale Groesse VOR dem Frame-Slot, nicht danach: die Sonde
    // texer2_size_aus_init (sizex=2 nur im Init) zeichnet in der Referenz KLEIN
    // (2247 px), texer2_size_literal mit demselben sizex=2 im Frame-Slot GROSS
    // (8945) — AVS belegt sizex/sizey je Frame neu vor, der Init-Wert ueberlebt
    // nicht. Vorher stand diese Vorbelegung nach dem Frame-Slot und loeschte
    // genau das, was das Skript gerade gesetzt hatte.
    engine.setNumber("sizex", 1.0);
    engine.setNumber("sizey", 1.0);
    // Farbe genauso: EINMAL je Frame vorbelegen, NICHT je Punkt. "Alien Alloy"
    // faerbt seine Sprites im FRAME-Slot (red=sin(ct+2.07)*0.5+0.5 …) — unsere
    // Vorbelegung stand in der Punktschleife und loeschte das bei jedem Sprite,
    // wir zeichneten durchgehend WEISS. Paar-Sonde 6_alloy/texer_farbe_*:
    // dieselbe Farbe im Point-Slot ist referenzgleich (MAE 0.001), im
    // Frame-Slot wich sie ab (0.042) — die Referenz laesst den Frame-Wert die
    // Punktschleife also ueberleben. Ein Point-Slot, der die Farbe setzt,
    // ueberschreibt sie weiterhin je Punkt.
    engine.setNumber("red", 1.0);
    engine.setNumber("green", 1.0);
    engine.setNumber("blue", 1.0);
    feedAudio(rt.texerHost->engine());
    if (rt.texerHost->has(Slot::Frame)) rt.texerHost->run(Slot::Frame);
    if (m_frameBeat && rt.texerHost->has(Slot::Beat)) rt.texerHost->run(Slot::Beat);
    // n=0 heisst NULL Sprites, nicht eines: die Sprite-Schleife laeuft
    // "for (i=0; i<n; i++)" und faellt dann sofort durch. Unsere alte
    // Untergrenze 1 zeichnete ein zusaetzliches Sprite in der Bildmitte —
    // gemessen 3356 Pixel, wo AvsRef 0 liefert (Befund S50; in "Mister Santa"
    // ist der Beard-Texer genau so abgeschaltet).
    const int n = std::clamp(static_cast<int>(engine.number("n")), 0, 4096);

    auto* f = QOpenGLContext::currentContext()->functions();
    // Texer II folgt dem aktuellen BLEND_LINE-Modus wie die uebrigen Renderer,
    // Default ist REPLACE — NICHT fest additiv (Befund S50). Gemessen: derselbe
    // Sprite ueber 6 Frames auf dieselbe Stelle laesst die Bildenergie in AvsRef
    // unveraendert (176901), unsere fest additive Fassung wuchs auf das 1,86-
    // fache. Bei EINEM Frame auf Schwarz sind Replace und Additiv ununterscheid-
    // bar — deshalb faellt das nur in Ketten mit ueberlappenden Sprites auf,
    // also genau in den Presets, die davon leben.
    applyLineBlend(m_renderMode.set ? m_renderMode.lineBlend : 0, m_renderMode.alpha);
    m_spriteShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, rt.picTexture);
    m_spriteShader->setUniformValue("uImg", 0);
    m_spriteShader->setUniformValue("uColorFilter", params.colorFiltering ? 1 : 0);
    // Ausserhalb der Textur ist NULL (die Referenz liest jenseits des Bildes
    // nichts): CLAMP_TO_BORDER mit schwarzem, durchsichtigem Rand — nur fuer
    // die Texer-II-Zeichnung; Erzeugung/Texer I bleiben auf CLAMP_TO_EDGE.
    {
        const float kRand0[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        f->glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, kRand0);
    }

    // Punkt-Vertrag wie r_sscope (SuperscopeModule::executePointLua): je Punkt
    // werden NUR i, v, skip und die Farb-Vorbelegung gesetzt. x/y/sizex/sizey
    // sind persistente EEL-Variablen und dürfen NICHT zurückgesetzt werden —
    // die Skripte dieses Packs lesen den Vorgängerwert ("x=if(dt,x3*dt*psf,x)"
    // hält den letzten Punkt, wenn er hinter der Kamera liegt). Die neutrale
    // Vorbelegung von sizex/sizey passiert deshalb genau einmal je Frame — oben,
    // VOR dem Frame-Slot.

    // v AVS-treu aus den rohen visdata-Bytes (Vertrag S44/S48: v = Byte/128-1,
    // Mittenkanal per CHAR-Arithmetik, Interpolation r_sscope.cpp:284-289) —
    // Bytes über die SSOT-Accessoren, kein zweites Layout-Wissen.
    const unsigned char* waveL = visWaveform(0);
    const unsigned char* waveR = visWaveform(1);
    const auto waveValue = [waveL, waveR](int point, int count) -> double {
        if (count < 1 || waveL == nullptr || waveR == nullptr) return 0.0;
        const auto centerByte = [waveL, waveR](int idx) -> int {
            idx = std::clamp(idx, 0, 575);
            const char cl = static_cast<char>(waveL[idx]);
            const char cr = static_cast<char>(waveR[idx]);
            return static_cast<unsigned char>(static_cast<char>(cl / 2 + cr / 2));
        };
        const double r = (static_cast<double>(point) * 576.0) / count;
        const int i0 = static_cast<int>(r);
        const double s1 = r - i0;
        const double b0 = centerByte(i0) ^ 128;
        const double b1 = centerByte(i0 + 1) ^ 128;
        return (b0 * (1.0 - s1) + b1 * s1) / 128.0 - 1.0;
    };

    for (int pt = 0; pt < n; ++pt)
    {
        const double iNorm = n > 1 ? static_cast<double>(pt) / (n - 1) : 0.0;
        engine.setNumber("i", iNorm);
        engine.setNumber("v", waveValue(pt, n));
        engine.setNumber("skip", 0.0);
        if (rt.texerHost->has(Slot::Point)) rt.texerHost->run(Slot::Point);
        if (engine.number("skip") > 0.5) continue;
        const double x = engine.number("x");
        const double y = engine.number("y");
        // "Resize" aus = Sprite in Originalgröße, sizex/sizey wirken nicht.
        const double sx = params.resizing ? engine.number("sizex") : 1.0;
        const double sy = params.resizing ? engine.number("sizey") : 1.0;
        m_spriteShader->setUniformValue(
            "uTint", QVector3D(static_cast<float>(engine.number("red")),
                               static_cast<float>(engine.number("green")),
                               static_cast<float>(engine.number("blue"))));

        // Ziel-Rechteck + Sampling-Fenster EXAKT wie die Original-APE (S59,
        // an Impuls-Sonden bit-genau gemessen, Herleitung im Session-Report):
        //   c       = (pos/2+0.5)·(dim−1)          [Pixelraster dim−1!]
        //   r       = c ± iw·s/2 ∓ 0.5             [iw = Bildbreite−1]
        //   r2      = fistp-Rundung (round-half-even)
        //   Phase   = (r2.left − r.left)/Spanne, als 16.16 ab Startspalte
        //   Schritt = (iw−1)/(Spanne+1) in 16.16   [Spanne+1 = iw·s]
        //   gemalt  = r2.left..(r2.right|1)        [qword-Ende: IMMER ungerade]
        // Die 8-Bit-Gewichtskaskade der Referenz uebernimmt hier der
        // GL-Bilinearfilter — gleiche Stuetzstellen, gleiche Subpixel-Phase.
        struct Achse
        {
            int p0 = 0, p1 = 0;   // gemalte Pixel inklusive
            double t0 = 0.0, t1 = 0.0;  // Texcoords an den Quad-KANTEN
            bool ok = false;
        };
        const auto achse = [](double pos, double groesse, int dim, int texDim) {
            Achse a;
            const int iw = texDim - 1;
            const int w = dim - 1;
            if (iw <= 1 || w <= 0) return a;
            const double c = (pos * 0.5 + 0.5) * w;
            const double rl = c - iw * groesse * 0.5 + 0.5;
            const double rr = c + iw * groesse * 0.5 - 0.5;
            const auto fistp = [](double v) {
                return static_cast<int>(std::nearbyint(v));  // half-even
            };
            int r2l = fistp(rl);
            int r2r = fistp(rr);
            if (r2r < 0 || r2l > w || rr <= rl) return a;
            double x0 = (r2l - rl) / (rr - rl);
            if (rl < 0.0)
            {
                x0 = -rl / (rr - rl);
                r2l = 0;
            }
            if (rr > w) r2r = w;
            const double fx0 = x0 * iw;
            const int cxi = fistp(fx0);
            const int dxf =
                65535 - static_cast<int>((0.5 - (fx0 - cxi)) * 65536.0);
            const int sdx =
                static_cast<int>((iw - 1) / (rr - rl + 1.0) * 65536.0);
            long long cx0 = (static_cast<long long>(cxi) << 16) + dxf;
            if (cx0 < 0)
            {
                cx0 += sdx;
                ++r2l;
            }
            if (r2r <= r2l) return a;
            a.p0 = r2l;
            // Rechte/untere Malgrenze der Referenz (12/12 Messfaelle):
            // ceil(r.right), mindestens aber auf ungerade erweitert —
            // das qword-Ende der MMX-Schleife malt ueber das gerundete
            // Rechteck hinaus.
            a.p1 = std::min(
                std::max(static_cast<int>(std::ceil(rr)), r2r | 1), w);
            // Texcoord an der linken Quad-Kante (Fenster-x = p0) und rechten
            // (p1+1): Fragmentmitte px+0.5 sampelt u = (cx0+(px−p0)·sdx)/2^16.
            const double u0 = (static_cast<double>(cx0) - 0.5 * sdx) / 65536.0;
            const double u1 =
                (static_cast<double>(cx0) +
                 (a.p1 + 0.5 - a.p0) * static_cast<double>(sdx)) / 65536.0;
            a.t0 = (u0 + 0.5) / texDim;
            a.t1 = (u1 + 0.5) / texDim;
            a.ok = true;
            return a;
        };
        // Resize AUS = 1:1-Blit: linke Kante rint(c) − iw/2, IW Spalten,
        // Texcoords 0..1 (Texelmitten treffen Pixelmitten — kein Filtern).
        const auto achseBlit = [](double pos, int dim, int texDim) {
            Achse a;
            const int iw = texDim - 1;
            const int w = dim - 1;
            if (texDim <= 0 || w <= 0) return a;
            const double c = (pos * 0.5 + 0.5) * w;
            const int links = static_cast<int>(std::nearbyint(c)) - iw / 2;
            const int rechts = links + iw;
            if (rechts < 0 || links > w) return a;
            a.p0 = std::max(links, 0);
            a.p1 = std::min(rechts, w);
            a.t0 = static_cast<double>(a.p0 - links) / texDim;
            a.t1 = static_cast<double>(a.p1 - links + 1) / texDim;
            a.ok = a.p1 >= a.p0;
            return a;
        };
        const int W = m_surfaceWidth;
        const int H = m_surfaceHeight;
        const auto zeichne = [&](double px, double py) {
            const Achse ax = params.resizing ? achse(px, sx, W, rt.picW)
                                             : achseBlit(px, W, rt.picW);
            const Achse ay = params.resizing ? achse(py, sy, H, rt.picH)
                                             : achseBlit(py, H, rt.picH);
            if (!ax.ok || !ay.ok) return;
            // Pixelrechteck -> NDC-Kanten (y: AVS zaehlt von oben, GL von
            // unten; Zeile r beginnt bei NDC 1 − r·2/H)
            const float x0n = static_cast<float>(ax.p0) * 2.0f / W - 1.0f;
            const float x1n = static_cast<float>(ax.p1 + 1) * 2.0f / W - 1.0f;
            const float yTn = 1.0f - static_cast<float>(ay.p0) * 2.0f / H;
            const float yBn = 1.0f - static_cast<float>(ay.p1 + 1) * 2.0f / H;
            m_spriteShader->setUniformValue(
                "uCenter", QVector2D((x0n + x1n) * 0.5f, (yBn + yTn) * 0.5f));
            m_spriteShader->setUniformValue(
                "uHalf", QVector2D((x1n - x0n) * 0.5f, (yTn - yBn) * 0.5f));
            // aPos=(-1,-1) = NDC unten links = AVS-Zeile p1/Spalte p0
            m_spriteShader->setUniformValue(
                "uUv0", QVector2D(static_cast<float>(ax.t0),
                                  static_cast<float>(ay.t1)));
            m_spriteShader->setUniformValue(
                "uUv1", QVector2D(static_cast<float>(ax.t1),
                                  static_cast<float>(ay.t0)));
            f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        };
        zeichne(x, y);
        // `wrapAround`: das Bild ist ein Torus — was rechts hinausragt, kommt
        // links wieder herein. Der EEL-Raum ist 2 breit, die Gegenseite liegt
        // also 2,0 entfernt. Bis S57 war dieses Feld im Panel verstellbar und
        // wurde von KEINEM Renderer gelesen (Feld-Sonde stumm, MAE 0,0000).
        if (params.wrapAround)
        {
            const double hxN = rt.picW * sx / W;
            const double hyN = rt.picH * sy / H;
            for (int ox = -1; ox <= 1; ++ox)
            {
                for (int oy = -1; oy <= 1; ++oy)
                {
                    if (ox == 0 && oy == 0) continue;
                    const double mx = x + ox * 2.0;
                    const double my = y + oy * 2.0;
                    if (std::abs(mx) - hxN > 1.0 || std::abs(my) - hyN > 1.0)
                        continue;
                    zeichne(mx, my);
                }
            }
        }
    }
    m_quadVao->release();
    m_spriteShader->release();
    resetLineBlend();
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
        rt.triHost->engine().setNumber("w", static_cast<double>(m_surfaceWidth));
        rt.triHost->engine().setNumber("h", static_cast<double>(m_surfaceHeight));
        rt.triHost->run(Slot::Init);
    }
    auto& engine = rt.triHost->engine();
    engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
    // Wie Texer II: die Bildschirmmasse fehlten, "n=w*0.05" ergab 0 Dreiecke.
    // Sonde 5_vars/triangle_n_aus_w — Referenz 23424 px, wir 0 (S51).
    engine.setNumber("w", static_cast<double>(m_surfaceWidth));
    engine.setNumber("h", static_cast<double>(m_surfaceHeight));
    feedAudio(rt.triHost->engine());
    if (rt.triHost->has(Slot::Frame)) rt.triHost->run(Slot::Frame);
    if (m_frameBeat && rt.triHost->has(Slot::Beat)) rt.triHost->run(Slot::Beat);
    const int n = std::clamp(static_cast<int>(engine.number("n")), 0, 4096);

    // Triangle zeichnet GEFUELLTE Dreiecke, nicht Umrisse: die Sonde
    // triangle_n_literal (16 Dreiecke) liefert in AvsRef 23424 Pixel, unser
    // Drahtgitter nur 4009 und den Schwerpunkt 12 Zeilen daneben (S51). Die
    // Flat-Fill-Stufe stammt aus dem Bass-Spin-Umbau (S48).
    std::vector<float> tri(6);
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
        const auto ndc = [&engine](const char* xn, const char* yn) {
            return std::pair{static_cast<float>(engine.number(xn)),
                             -static_cast<float>(engine.number(yn))};  // AVS y is down
        };
        const auto [x1, y1] = ndc("x1", "y1");
        const auto [x2, y2] = ndc("x2", "y2");
        const auto [x3, y3] = ndc("x3", "y3");
        if (params.filled)
        {
            tri = {x1, y1, x2, y2, x3, y3};
            drawFlatTriangles(tri, col);
            continue;
        }
        // Drahtgitter (S53, opt-in): geschlossener Kantenzug statt Fuellung.
        // Das ist eine bewusste ABWEICHUNG von AVS — die Referenz fuellt.
        if (!m_scopeRenderer.ready()) continue;
        std::vector<lumi::modules::SuperscopePoint> edge;
        edge.reserve(4);
        for (const auto& [ex, ey] :
             {std::pair{x1, y1}, std::pair{x2, y2}, std::pair{x3, y3},
              std::pair{x1, y1}})
        {
            lumi::modules::SuperscopePoint p;
            p.x = ex;
            p.y = ey;
            p.r = col.x();
            p.g = col.y();
            p.b = col.z();
            p.a = 1.0f;
            edge.push_back(p);
        }
        lumi::render::ScopeRenderer::Params rp;
        rp.mode = lumi::modules::SuperscopeRenderMode::Lines;
        rp.lineWidth = std::max(1.0f, params.lineWidth);
        m_scopeRenderer.draw(edge, rp);
    }
}

void MultiEffectVisualizer::runOscStar(const ChainNode& node, const OscStarParams& params)
{
    // S60: exakter r_oscstar-Port (Flaechen-Befund S59 — Deckung 0,02).
    // Vorher malte hier eine freie Naeherung 576 Punkte je Arm; die Referenz
    // laeuft 64 Schritte je Arm mit FORTLAUFENDEM Byte-Index ueber die Arme,
    // der dfactor-Huellkurve 1/1024 → ~1/128 und getrennten (int)-Casts.
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const QVector3D c = colorToVec(cycleScopeColor(params.colors, rt.scopeColorPos));

    // S53: `rotScale`, `spokes` und `amplitude` waren Literale; Strang D
    // rechnet sie optional je Frame — auf einer Frame-Kopie.
    double vSize = params.size, vRot = params.rot, vSpokes = params.spokes;
    double vRotScale = params.rotScale, vAmp = params.amplitude;
    runParamScript(rt, "oscstar", params.initCode, params.frameCode, params.beatCode,
                   {{"size", &vSize},
                    {"rot", &vRot},
                    {"spokes", &vSpokes},
                    {"rotscale", &vRotScale},
                    {"amplitude", &vAmp}});

    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    if (w < 2 || h < 2) return;

    // Waveform-Bytes wie die Referenz (visdata[1][ch]); "Mitte" ist die
    // SIGNED-halbierte Summe beider Kanaele (char-Arithmetik, wie r_simple).
    const int which = std::clamp(params.channel, 0, 2);
    unsigned char center[576];
    const unsigned char* fa;
    if (which >= 2)
    {
        const unsigned char* lc = visWaveform(0);
        const unsigned char* rc = visWaveform(1);
        for (int i = 0; i < 576; ++i)
            center[i] = static_cast<unsigned char>(static_cast<char>(
                static_cast<char>(lc[i]) / 2 + static_cast<char>(rc[i]) / 2));
        fa = center;
    }
    else
    {
        fa = visWaveform(which);
    }

    // r_oscstar:148-160: s = size/32, is = min(h*s, w*s) — GANZZAHLIG.
    const double sz = std::clamp(static_cast<int>(vSize), 0, 16) / 32.0;
    const int is = std::min(static_cast<int>(h * sz), static_cast<int>(w * sz));
    int cX = w / 2;
    if (params.position == 0) cX = w / 4;
    else if (params.position == 1) cX = w / 2 + w / 4;

    const int spokes = std::clamp(static_cast<int>(vSpokes), 1, 64);
    int ii = 0;
    for (int q = 0; q < spokes; ++q)
    {
        const double ang = static_cast<double>(rt.scopeRot) +
                           static_cast<double>(q) *
                               (3.14159 * 2.0 / static_cast<double>(spokes));
        const double sa = std::sin(ang);
        const double ca = std::cos(ang);
        double p = 0.0;
        int lx = cX;
        int ly = h / 2;
        const double dp = static_cast<double>(is) / 64.0;
        const double hw = is;
        double dfactor = 1.0 / 1024.0;
        std::vector<lumi::modules::SuperscopePoint> pts;
        pts.reserve(65);
        const auto push = [&](int px, int py, bool skip) {
            lumi::modules::SuperscopePoint pt;
            pt.x = (static_cast<double>(px) + 0.5) /
                       static_cast<double>(w) * 2.0 - 1.0;
            pt.y = -((static_cast<double>(py) + 0.5) /
                         static_cast<double>(h) * 2.0 - 1.0);
            pt.r = c.x();
            pt.g = c.y();
            pt.b = c.z();
            pt.a = 1.0f;
            pt.skip = skip;
            pts.push_back(pt);
        };
        push(lx, ly, false);
        for (int t = 0; t < 64; ++t)
        {
            const double ale =
                ((fa[ii % 576] ^ 128) - 128) * dfactor * hw * vAmp;
            ++ii;
            const int x = cX + static_cast<int>(ca * p) -
                          static_cast<int>(sa * ale);
            const int y = h / 2 + static_cast<int>(sa * p) +
                          static_cast<int>(ca * ale);
            // r_oscstar:209-213 — gezeichnet wird nur, wenn EIN Endpunkt im
            // Bild liegt; die Kette laeuft trotzdem weiter (skip = Segment aus).
            const bool draw = (x >= 0 && x < w && y >= 0 && y < h) ||
                              (lx >= 0 && lx < w && ly >= 0 && ly < h);
            push(x, y, !draw);
            lx = x;
            ly = y;
            p += dp;
            dfactor -= ((1.0 / 1024.0) - (1.0 / 128.0)) / 64.0;
        }
        // Exakter linedraw-Weg auch bei Breite 1 (wie Rotating Stars, S60).
        if (m_scopeRenderer.ready() && pts.size() >= 2)
        {
            auto* f = QOpenGLContext::currentContext()->functions();
            f->glEnable(GL_BLEND);
            applyLineBlend(m_renderMode.lineBlend, m_renderMode.alpha);
            lumi::render::ScopeRenderer::Params rp;
            rp.mode = lumi::modules::SuperscopeRenderMode::ThickLines;
            rp.lineWidth = (m_renderMode.set && m_renderMode.lineWidth > 0)
                               ? static_cast<float>(m_renderMode.lineWidth)
                               : 1.0f;
            rp.dotSize = 1.0f;
            rp.glowEnabled = false;
            m_scopeRenderer.draw(pts, rp);
            resetLineBlend();
        }
    }

    // r_oscstar:222-224 — NACH dem Zeichnen, mit 2π-Umbruch (Literal 3.14159).
    rt.scopeRot += static_cast<float>(vRot * vRotScale);
    if (rt.scopeRot >= static_cast<float>(3.14159 * 2.0))
        rt.scopeRot -= static_cast<float>(3.14159 * 2.0);
}

void MultiEffectVisualizer::runOscRing(const ChainNode& node, const OscRingParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const QVector3D c = colorToVec(cycleScopeColor(params.colors, rt.scopeColorPos));

    // Strang D auf einer Frame-Kopie (s. Osc Star).
    double vSize = params.size, vSeg = params.segments;
    double vBase = params.baseScale, vAudio = params.audioScale;
    runParamScript(rt, "oscring", params.initCode, params.frameCode, params.beatCode,
                   {{"size", &vSize},
                    {"segments", &vSeg},
                    {"basescale", &vBase},
                    {"audioscale", &vAudio}});

    // S60: exakter r_oscring-Port (Flaechen-Befund S59 — Deckung 0).
    // Quelle: source 0 = Waveform (visdata[1]), 1 = Spektrum (visdata[0]);
    // "Mitte" ist die SIGNED-halbierte Summe beider Kanaele.
    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    if (w < 2 || h < 2) return;
    const bool spectrum = params.source != 0;
    const auto bank = [&](int ch) {
        return spectrum ? visSpectrum(ch) : visWaveform(ch);
    };
    const int which = std::clamp(params.channel, 0, 2);
    unsigned char center[576];
    const unsigned char* fa;
    if (which >= 2)
    {
        const unsigned char* lc = bank(0);
        const unsigned char* rc = bank(1);
        for (int i = 0; i < 576; ++i)
            center[i] = static_cast<unsigned char>(static_cast<char>(
                static_cast<char>(lc[i]) / 2 + static_cast<char>(rc[i]) / 2));
        fa = center;
    }
    else
    {
        fa = bank(which);
    }

    // r_oscring:147-163: s = size/32, is = min(h*s, w*s) als DOUBLE (kein
    // Zwischencast — anders als Osc Star!), Zentrum (c_x, h/2).
    const double sz = std::clamp(static_cast<int>(vSize), 0, 16) / 32.0;
    const double is = std::min(static_cast<double>(h) * sz,
                               static_cast<double>(w) * sz);
    int cX = w / 2;
    if (params.position == 0) cX = w / 4;
    else if (params.position == 1) cX = w / 2 + w / 4;
    const int cY = h / 2;

    // Radius-Modulation (r_oscring:169/187): Spiegel-Index q>seg/2 → seg-q;
    // Waveform: 0,1 + ((byte^128)/255)*0,9 · Spektrum: 0,1 + ((b0/2+b1/2)/255)*0,9
    // (UNSIGNED Halbierung). Die Anteile 0,1/0,9 sind die S53-Parameter.
    const int seg = std::clamp(static_cast<int>(vSeg), 4, 512);
    const auto scaAt = [&](int q) {
        const int idx = q > seg / 2 ? seg - q : q;
        int v;
        if (!spectrum)
        {
            v = fa[(idx % 576)] ^ 128;
        }
        else
        {
            const int i2 = (idx * 2) % 575;
            v = fa[i2] / 2 + fa[i2 + 1] / 2;
        }
        return vBase + (static_cast<double>(v) / 255.0) * vAudio;
    };

    // Vertizes: EIN impliziter (int)-Cast ueber die ganze Summe
    // (`lx=c_x+(cos(a)*is*sca)` — r_oscring:176/189), Winkel laeuft NEGATIV.
    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(static_cast<std::size_t>(seg) + 1);
    const auto push = [&](int px, int py, bool skip) {
        lumi::modules::SuperscopePoint p;
        p.x = (static_cast<double>(px) + 0.5) / static_cast<double>(w) * 2.0 - 1.0;
        p.y = -((static_cast<double>(py) + 0.5) / static_cast<double>(h) * 2.0 - 1.0);
        p.r = c.x();
        p.g = c.y();
        p.b = c.z();
        p.a = 1.0f;
        p.skip = skip;
        pts.push_back(p);
    };
    double a = 0.0;
    double sca = scaAt(0);
    int lx = static_cast<int>(static_cast<double>(cX) + std::cos(a) * is * sca);
    int ly = static_cast<int>(static_cast<double>(cY) + std::sin(a) * is * sca);
    push(lx, ly, false);
    for (int q = 1; q <= seg; ++q)
    {
        a -= 3.14159 * 2.0 / static_cast<double>(seg);
        sca = scaAt(q);
        const int tx =
            static_cast<int>(static_cast<double>(cX) + std::cos(a) * is * sca);
        const int ty =
            static_cast<int>(static_cast<double>(cY) + std::sin(a) * is * sca);
        // r_oscring:206-210 — Segment nur, wenn EIN Endpunkt im Bild liegt.
        const bool draw = (tx >= 0 && tx < w && ty >= 0 && ty < h) ||
                          (lx >= 0 && lx < w && ly >= 0 && ly < h);
        push(tx, ty, !draw);
        lx = tx;
        ly = ty;
    }
    // Exakter linedraw-Weg auch bei Breite 1 (wie Rotating Stars, S60).
    if (m_scopeRenderer.ready() && pts.size() >= 2)
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        f->glEnable(GL_BLEND);
        applyLineBlend(m_renderMode.lineBlend, m_renderMode.alpha);
        lumi::render::ScopeRenderer::Params rp;
        rp.mode = lumi::modules::SuperscopeRenderMode::ThickLines;
        rp.lineWidth = (m_renderMode.set && m_renderMode.lineWidth > 0)
                           ? static_cast<float>(m_renderMode.lineWidth)
                           : 1.0f;
        rp.dotSize = 1.0f;
        rp.glowEnabled = false;
        m_scopeRenderer.draw(pts, rp);
        resetLineBlend();
    }
}

void MultiEffectVisualizer::runParamScript(LeafRuntime& rt, const char* prefix,
                                           const std::string& init,
                                           const std::string& frame,
                                           const std::string& beat,
                                           const std::vector<ParamVar>& vars)
{
    // Opt-in: ein Knoten ohne Skript darf keinen Cent Rechenzeit kosten.
    if (init.empty() && frame.empty() && beat.empty()) return;

    const std::string combined = init + '\n' + frame + '\n' + beat;
    const bool fresh = (rt.paramHost == nullptr || rt.paramCompiled != combined);
    if (fresh)
    {
        rt.paramHost = std::make_unique<ScriptSlotHost>(prefix, activeContext(),
                                                        ScriptSlotHost::Dialect::Avs);
        rt.paramHost->setSource(Slot::Init, init);
        rt.paramHost->setSource(Slot::Frame, frame);
        rt.paramHost->setSource(Slot::Beat, beat);
        // Ein Uebersetzungsfehler darf NICHT still bleiben: bis S54 fragte
        // niemand `lastError()` ab, ein Skript mit Tippfehler verhielt sich
        // damit exakt wie ein leeres — die Probe mit absichtlichem Unsinn kam
        // ohne eine einzige Meldung durch. Gemeldet wird nur HIER, also einmal
        // je Skriptaenderung, nicht je Frame.
        if (!rt.paramHost->compileAll())
        {
            rt.paramError = rt.paramHost->lastError();
            BasicLogger::logWarning("MultiEffect: Parameter-Skript '" +
                                    std::string(prefix) + "': " + rt.paramError);
        }
        else
        {
            rt.paramError.clear();
        }
        rt.paramCompiled = combined;
    }

    auto& engine = rt.paramHost->engine();
    // Merkregel S52: ein Skript-Traeger braucht ALLE Variablen, die seine Slots
    // lesen — und die Vorbelegung gehoert VOR den Frame-Slot, nicht danach.
    //
    // S54: die Vorbelegung kommt NUR beim ersten Frame (und nach einer
    // Reglerbewegung) aus den Params. Sonst haette der Init-Slot nie eine
    // Wirkung — sein Ergebnis wurde eine Zeile weiter jeden Frame wieder
    // ueberschrieben (Entscheid Patrik: Init setzt eine einmalige
    // Startbelegung, die Frames schreiben sie fort).
    for (const ParamVar& v : vars)
    {
        const auto seen = rt.paramSeen.find(v.name);
        if (fresh || seen == rt.paramSeen.end() || seen->second != *v.value)
        {
            rt.paramState[v.name] = *v.value;
            rt.paramSeen[v.name] = *v.value;
        }
        engine.setNumber(v.name, rt.paramState[v.name]);
    }
    engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
    engine.setNumber("w", static_cast<double>(m_surfaceWidth));
    engine.setNumber("h", static_cast<double>(m_surfaceHeight));
    feedAudio(engine);

    if (fresh && rt.paramHost->has(Slot::Init)) rt.paramHost->run(Slot::Init);
    if (rt.paramHost->has(Slot::Frame)) rt.paramHost->run(Slot::Frame);
    if (m_frameBeat && rt.paramHost->has(Slot::Beat)) rt.paramHost->run(Slot::Beat);

    // Laufzeitfehler: der Slot schaltet sich selbst ab (LuaScriptEngine), die
    // Meldung waere sonst verloren. Einmal je NEUEM Fehlertext, nicht je Frame.
    if (const std::string& err = rt.paramHost->lastError();
        !err.empty() && err != rt.paramError)
    {
        rt.paramError = err;
        BasicLogger::logWarning("MultiEffect: Parameter-Skript '" +
                                std::string(prefix) + "': " + err);
    }

    // Ergebnis zurueck in die Frame-Kopie UND in die Fortschreibung: was der
    // Frame-Slot gerechnet hat, ist die Vorbelegung des naechsten Frames.
    for (const ParamVar& v : vars)
    {
        *v.value = engine.number(v.name);
        rt.paramState[v.name] = *v.value;
    }
}

void MultiEffectVisualizer::runRotatingStars(const ChainNode& node,
                                             const RotatingStarsParams& params)
{
    // S53: die frueheren Literale sind Parameter geworden. S60: der Rechenweg
    // ist der EXAKTE r_rotstar-Port (Flaechen-Befund S59 — die Referenz malt
    // WINZIGE Sterne): Groesse aus dem groessten LOKALEN Peak der rohen
    // visdata-Bytes je Kanal, Vertizes ueber die (int)-Kaskade der Referenz,
    // gezeichnet mit der aktuellen Rotation, Inkrement DANACH (r1 startet 0).
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    const QVector3D c = colorToVec(cycleScopeColor(params.colors, rt.scopeColorPos));

    // Strang D: das Skript rechnet auf einer FRAME-KOPIE — das Preset selbst
    // bleibt unberuehrt, sonst waeren die Regler nach einem Frame verstellt.
    double vPoints = params.points, vSkip = params.skip, vStars = params.stars;
    double vRotSpeed = params.rotSpeed, vOrbit = params.orbit;
    double vBase = params.baseRadius, vGain = params.audioGain;
    runParamScript(rt, "rotstar", params.initCode, params.frameCode, params.beatCode,
                   {{"points", &vPoints},
                    {"skip", &vSkip},
                    {"stars", &vStars},
                    {"rotspeed", &vRotSpeed},
                    {"orbit", &vOrbit},
                    {"baseradius", &vBase},
                    {"audiogain", &vGain}});

    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    if (w < 2 || h < 2) return;

    const int points = std::clamp(static_cast<int>(vPoints), 2, 64);
    const int skip = std::clamp(static_cast<int>(vSkip), 1, std::max(1, points - 1));
    const int stars = std::clamp(static_cast<int>(vStars), 1, 16);
    // Lokal-Peak-Suche braucht die Nachbarn l-1/l+1 (r_rotstar liest 2..14).
    const int lo = std::clamp(params.bandLo, 1, 574);
    const int hi = std::clamp(params.bandHi, lo + 1, 575);

    int aPx0 = 0, bPx0 = 0;
    for (int ch = 0; ch < stars; ++ch)
    {
        // r_rotstar:136-140 — visdata ist char[]: die Bytes werden SIGNED
        // verglichen, Werte ueber 127 sind negativ und heben s nie
        // (Original-Quirk). Ein Peak zaehlt nur, wenn er BEIDE Nachbarn um
        // mehr als 4 uebersteigt. Stern 0 = linker, Stern 1 = rechter Kanal.
        const unsigned char* raw = visSpectrum(ch & 1);
        int s = 0;
        for (int l = lo; l < hi; ++l)
        {
            const int v = static_cast<signed char>(raw[l]);
            if (v > s && v > static_cast<signed char>(raw[l + 1]) + 4 &&
                v > static_cast<signed char>(raw[l - 1]) + 4)
                s = v;
        }
        // Groesse je Achse (r_rotstar:145-146): w/8*(s+9)/88 bzw. h/8*…,
        // in NDC (s+9)/352 — bei den Vorgaben 9/352 + 255/352*(s/255) exakt.
        const double size = vBase + vGain * (static_cast<double>(s) / 255.0);
        const double vwPx = size * (static_cast<double>(w) * 0.5);
        const double vhPx = size * (static_cast<double>(h) * 0.5);

        // Bahnposition: a=(int)(cos(r1)*w/4), b=(int)(sin(r1)*h/4); Stern 2
        // negiert die fertigen INTS (r_rotstar:142) — nicht den Winkel, sonst
        // kann die Truncation um 1 px auseinanderlaufen. Ab drei Sternen
        // (LumiViz-Erweiterung) verteilen sich die Bahnwinkel gleichmaessig.
        int aPx, bPx;
        if (stars == 2 && ch == 1)
        {
            aPx = -aPx0;
            bPx = -bPx0;
        }
        else
        {
            const double phase = static_cast<double>(rt.scopeRot) +
                                 static_cast<double>(ch) * 6.283185307179586 /
                                     static_cast<double>(stars);
            aPx = static_cast<int>(std::cos(phase) * vOrbit *
                                   (static_cast<double>(w) * 0.5));
            bPx = static_cast<int>(std::sin(phase) * vOrbit *
                                   (static_cast<double>(h) * 0.5));
            if (ch == 0) { aPx0 = aPx; bPx0 = bPx; }
        }

        // Pentagramm-Lauf (r_rotstar:148-164): Start bei -r1, Schritt
        // 3.14159*2*skip/points (Original-Literal, skip=2/points=5 → 4π/5).
        // RUNDUNGS-EIGENHEIT der Referenz: der ERSTE Eckpunkt entsteht als
        // w/2+a+(int)(cos*vw) — getrennte Casts, trunc Richtung 0 —, die
        // SCHLEIFEN-Eckpunkte als (int)(cos*vw + w/2 + a) — EIN Cast ueber
        // die positive Summe, also floor. Bei negativem cos*vw liegen beide
        // um 1 px auseinander; der Stern ist so klein, dass das die halbe
        // Silhouette verschiebt (S60, Pixelvergleich).
        const double step = 3.14159 * 2.0 * static_cast<double>(skip) /
                            static_cast<double>(points);
        double r2 = -static_cast<double>(rt.scopeRot);
        std::vector<lumi::modules::SuperscopePoint> pts;
        pts.reserve(static_cast<std::size_t>(points) + 1);
        const auto push = [&](int px, int py) {
            lumi::modules::SuperscopePoint p;
            p.x = (static_cast<double>(px) + 0.5) /
                      static_cast<double>(w) * 2.0 - 1.0;
            p.y = -((static_cast<double>(py) + 0.5) /
                        static_cast<double>(h) * 2.0 - 1.0);
            p.r = c.x();
            p.g = c.y();
            p.b = c.z();
            p.a = 1.0f;
            pts.push_back(p);
        };
        push(w / 2 + aPx + static_cast<int>(std::cos(r2) * vwPx),
             h / 2 + bPx + static_cast<int>(std::sin(r2) * vhPx));
        r2 += step;
        for (int t = 0; t < points; ++t)
        {
            push(static_cast<int>(std::cos(r2) * vwPx +
                                  static_cast<double>(w / 2 + aPx)),
                 static_cast<int>(std::sin(r2) * vhPx +
                                  static_cast<double>(h / 2 + bPx)));
            r2 += step;
        }
        // NICHT drawScopeShape (duenne GL-Linien): die 2-7-px-Segmente der
        // Sternchen rastern per GL-Linie anders als linedraw.cpp — nur der
        // renderThickLines-Bresenham-Port trifft die Referenz auch bei
        // Breite 1 (S60; Breite kommt wie bei line() aus dem SRM).
        if (m_scopeRenderer.ready() && pts.size() >= 2)
        {
            auto* f = QOpenGLContext::currentContext()->functions();
            f->glEnable(GL_BLEND);
            applyLineBlend(m_renderMode.lineBlend, m_renderMode.alpha);
            lumi::render::ScopeRenderer::Params rp;
            rp.mode = lumi::modules::SuperscopeRenderMode::ThickLines;
            rp.lineWidth = (m_renderMode.set && m_renderMode.lineWidth > 0)
                               ? static_cast<float>(m_renderMode.lineWidth)
                               : 1.0f;
            rp.dotSize = 1.0f;
            rp.glowEnabled = false;
            m_scopeRenderer.draw(pts, rp);
            resetLineBlend();
        }
    }

    // r_rotstar:166 — NACH dem Zeichnen; der erste Frame malt mit r1 = 0.
    rt.scopeRot += static_cast<float>(vRotSpeed);
}

void MultiEffectVisualizer::runColorClip(const ChainNode& node,
                                         const ColorClipParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vDistance = params.distance;
    runParamScript(rt, "colorclip", params.initCode, params.frameCode,
                   params.beatCode, {{"distance", &vDistance}});

    const QVector3D clip = colorToVec(params.clipColor);
    const QVector3D outc = colorToVec(params.outColor);
    m_colorClipShader->bind();
    m_colorClipShader->setUniformValue("uMode", std::clamp(params.mode, 1, 3));
    m_colorClipShader->setUniformValue("uClip", clip);
    m_colorClipShader->setUniformValue("uOut", outc);
    m_colorClipShader->setUniformValue(
        "uDist", static_cast<float>(vDistance) * 2.0f / 255.0f);
    m_colorClipShader->release();
    transformPass(*m_colorClipShader);
}

void MultiEffectVisualizer::runUniqueTone(const ChainNode& node,
                                          const UniqueToneParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vBlend = params.blend;
    runParamScript(rt, "uniquetone", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"blend", &vBlend}});

    m_uniqueToneShader->bind();
    m_uniqueToneShader->setUniformValue("uColor", colorToVec(params.color));
    m_uniqueToneShader->setUniformValue("uInvert", params.invert ? 1 : 0);
    m_uniqueToneShader->setUniformValue(
        "uBlend", std::clamp(static_cast<int>(vBlend), 0, 2));  // Strang D
    m_uniqueToneShader->release();
    transformPass(*m_uniqueToneShader);
}

void MultiEffectVisualizer::runInterleave(const ChainNode& node,
                                          const InterleaveParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vX = params.x, vY = params.y;
    runParamScript(rt, "interleave", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"x", &vX},
                    {"y", &vY}});

    if (!rt.interSeeded)
    {
        rt.interCurX = static_cast<float>(params.x);
        rt.interCurY = static_cast<float>(params.y);
        rt.interSeeded = true;
    }
    // Ease towards x/y (r_interleave sc1), jump to x2/y2 on beat.
    const float sc1 = (static_cast<float>(params.beatDuration) + 512.0f - 64.0f) / 512.0f;
    // Strang D: die Frame-Kopien. `x2`/`y2` (der Beat-Sprung) haben keine
    // Skriptvariable und bleiben deshalb am Preset-Wert.
    rt.interCurX = rt.interCurX * sc1 + static_cast<float>(vX) * (1.0f - sc1);
    rt.interCurY = rt.interCurY * sc1 + static_cast<float>(vY) * (1.0f - sc1);
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

void MultiEffectVisualizer::runConvolution(const ChainNode& node,
                                           const ConvolutionParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vBias = params.bias, vScale = params.scale;
    runParamScript(rt, "convolution", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"bias", &vBias},
                    {"scale", &vScale}});

    std::array<float, 49> kf{};
    for (int i = 0; i < 49; ++i)
        kf[static_cast<std::size_t>(i)] = static_cast<float>(params.kernel[static_cast<std::size_t>(i)]);
    // Strang D: die Frame-Kopien
    const float scale = vScale != 0 ? static_cast<float>(vScale) : 1.0f;
    // Der Bias wirkt als vorzeichenbehaftetes BYTE des Konfigurationswerts:
    // +128 misst wie -128 (schwarz), +256 ist wirkungslos, +257 wirkt wie +1.
    // Unterhalb von -128 misst die Referenz schwarz, wo das reine Bytemodell
    // wieder Weiss saehe — dort klemmen wir statt zu schneiden (S58).
    const int biasCfg = static_cast<int>(vBias);
    const float bias = biasCfg < -128
                           ? -128.0f
                           : static_cast<float>(static_cast<std::int8_t>(biasCfg & 0xFF));

    auto pass = [&] {
        m_convolutionShader->bind();
        m_convolutionShader->setUniformValue(
            "uRes", QVector2D(static_cast<float>(m_surfaceWidth),
                              static_cast<float>(m_surfaceHeight)));
        m_convolutionShader->setUniformValueArray("uKernel", kf.data(), 49, 1);
        m_convolutionShader->setUniformValue("uScale", scale);
        m_convolutionShader->setUniformValue("uBias", bias);
        m_convolutionShader->setUniformValue("uAbsolute", params.absolute ? 1 : 0);
        m_convolutionShader->setUniformValue("uTwoPass", params.twoPass ? 1 : 0);
        m_convolutionShader->setUniformValue("uEdge", std::clamp(params.edgeMode, 0, 1));
        // Pass 1 kennt keinen bearbeiteten Puffer: uProc = uTex (Unit 0).
        // Die "bearbeitet"-Zweige des Shaders lesen dann schlicht die Eingabe
        // — exakt der fbout-Weg der APE.
        m_convolutionShader->setUniformValue("uProc", 0);
        m_convolutionShader->release();
        transformPass(*m_convolutionShader);
    };
    // `twoPass` ist eine Verdopplung IM Durchgang (s. Shader) — zweimal falten
    // ist es nicht: das haetten die Sonden `two1_k1_s2/s4` als v/4 bzw. v/8
    // gezeigt, gemessen sind v und v/2.
    pass();

    // IN-PLACE-Kante der APE (S59, s. Shader-Kommentar): liegt der erste
    // Kernel-Eintrag ab dem Zentrum (Index >= 24 row-major), schreibt die
    // Referenz in den EINGABEPUFFER — die auf h-2/w-2 zurueckgeklemmten Taps
    // der letzten Zeile/Spalte lesen dann schon bearbeitete Pixel. Pass 2
    // rechnet genau diese Pixel mit uProc = Pass-1-Ergebnis nach; alle
    // betroffenen Taps zeigen in die UNBETROFFENE Innenflaeche, deshalb
    // reichen exakt zwei Paesse.
    int ersterEintrag = -1;
    for (int i = 0; i < 49; ++i)
    {
        if (params.kernel[static_cast<std::size_t>(i)] != 0)
        {
            ersterEintrag = i;
            break;
        }
    }
    const bool inPlace = ersterEintrag < 0 || ersterEintrag >= 24;
    if (inPlace)
    {
        auto* f = QOpenGLContext::currentContext()->functions();
        auto* extra = QOpenGLContext::currentContext()->extraFunctions();
        if (m_convScratch == nullptr ||
            m_convScratch->width() != m_surfaceWidth ||
            m_convScratch->height() != m_surfaceHeight)
        {
            m_convScratch = std::make_unique<QOpenGLFramebufferObject>(
                m_surfaceWidth, m_surfaceHeight);
        }
        // Nach transformPass: current = Pass 1, partner = Eingabe (intakt)
        SurfacePair& pair = active();
        pair.current()->release();
        m_convScratch->bind();
        f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
        m_convolutionShader->bind();
        m_quadVao->bind();
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, pair.partner()->texture());
        m_convolutionShader->setUniformValue("uTex", 0);
        f->glActiveTexture(GL_TEXTURE1);
        f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
        m_convolutionShader->setUniformValue("uProc", 1);
        f->glActiveTexture(GL_TEXTURE0);
        f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_quadVao->release();
        m_convolutionShader->release();
        m_convScratch->release();
        extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_convScratch->handle());
        extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pair.current()->handle());
        extra->glBlitFramebuffer(0, 0, m_surfaceWidth, m_surfaceHeight, 0, 0,
                                 m_surfaceWidth, m_surfaceHeight,
                                 GL_COLOR_BUFFER_BIT, GL_NEAREST);
        bindActive();
    }
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

void MultiEffectVisualizer::runMultiFilter(const ChainNode& node,
                                           const MultiFilterParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vEffect = params.effect;
    runParamScript(rt, "multifilter", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"effect", &vEffect}});

    if (params.onBeat && !m_frameBeat) return;  // only on beat frames
    m_multiFilterShader->bind();
    m_multiFilterShader->setUniformValue(
        "uEffect", std::clamp(static_cast<int>(vEffect), 0, 3));  // Strang D
    m_multiFilterShader->release();
    transformPass(*m_multiFilterShader);
}

void MultiEffectVisualizer::runAddBorders(const ChainNode& node,
                                          const AddBordersParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vSize = params.size;
    runParamScript(rt, "addborders", params.initCode, params.frameCode,
                   params.beatCode, {{"size", &vSize}});

    m_addBordersShader->bind();
    m_addBordersShader->setUniformValue("uRes",
                                        QVector2D(static_cast<float>(m_surfaceWidth),
                                                  static_cast<float>(m_surfaceHeight)));
    m_addBordersShader->setUniformValue("uColor", colorToVec(params.color));
    m_addBordersShader->setUniformValue("uSize",
                                        std::max(0, static_cast<int>(vSize)));
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
    m_ddmShader->setUniformValue("uSubpixel", params.subpixel ? 1 : 0);
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

    // Strang D auf einer Frame-Kopie. KLASSE A: ein Skript hier verlaesst die
    // Referenz — der Editor weist darauf hin (die ⚠ an den Reglern kann nur
    // feste Werte pruefen, nicht was das Skript je Frame rechnet).
    double vSize = params.size, vSize2 = params.size2;
    double vMaxDist = params.maxDistance;
    double vSpring = params.spring, vDamping = params.damping;
    runParamScript(rt, "movingparticle", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"size", &vSize},
                    {"size2", &vSize2},
                    {"maxdistance", &vMaxDist},
                    {"spring", &vSpring},
                    {"damping", &vDamping}});

    // r_parts.cpp:124-125 — zwei Zuege aus dem Preset-Strom je Beat
    auto rnd = [this] {
        return (m_scriptContext->nextRand() % 33 - 16) / 48.0f;
    };
    if (m_frameBeat)
    {
        rt.mpCx = rnd();
        rt.mpCy = rnd();
    }
    // S53 freigemacht (Klasse A) — Vorgaben sind die r_parts-Werte.
    const float spring = static_cast<float>(vSpring);
    const float damping = static_cast<float>(vDamping);
    rt.mpVx -= spring * (rt.mpPx - rt.mpCx);
    rt.mpVy -= spring * (rt.mpPy - rt.mpCy);
    rt.mpPx += rt.mpVx;
    rt.mpPy += rt.mpVy;
    rt.mpVx *= damping;
    rt.mpVy *= damping;

    // Size ease (r_parts s_pos): jump to size2 on beat, then relax to size.
    if (m_frameBeat && params.onBeatSize) rt.mpSize = static_cast<float>(vSize2);
    const float sz = rt.mpSize;
    rt.mpSize = (rt.mpSize + static_cast<float>(vSize)) * 0.5f;

    const float ss = std::min(static_cast<float>(m_surfaceHeight) * 0.5f,
                              static_cast<float>(m_surfaceWidth) * 3.0f / 8.0f);
    const float scale = ss * static_cast<float>(vMaxDist) / 32.0f;
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
        case 3:                                                       // BLEND_LINE (r_parts.cpp:153/186 — folgt dem SRM-Zustand, S3)
            applyLineBlend(m_renderMode.lineBlend, m_renderMode.alpha);
            break;
        default: f->glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;        // additive
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
    resetLineBlend();
}

void MultiEffectVisualizer::runWaterBump(const ChainNode& node,
                                         const WaterBumpParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vDensity = params.density, vDepth = params.depth;
    double vDropR = params.dropRadius, vDisp = params.displaceScale;
    runParamScript(rt, "waterbump", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"density", &vDensity},
                    {"depth", &vDepth},
                    {"dropradius", &vDropR},
                    {"displacescale", &vDisp}});

    auto* f = QOpenGLContext::currentContext()->functions();

    // RGBA32F height ping-pong (.r aktuelle Seite, .g vorige Seite; Werte sind
    // GANZE Zahlen in Referenz-Einheiten — 16F traegt nur bis 2048 exakt).
    if (rt.wbHeight[0] == nullptr || rt.wbW != m_surfaceWidth ||
        rt.wbH != m_surfaceHeight)
    {
        QOpenGLFramebufferObjectFormat fmt;
        fmt.setInternalTextureFormat(GL_RGBA32F);
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

    // Tropfen auf Beat (SineBlob, r_waterbump.cpp:128-165 + 293-313): Zentrum,
    // Radius und das ENDE-EXKLUSIVE Clip-Rechteck rechnet der Host; der
    // cos-Beitrag laeuft als geteilte Funktion in BEIDEN Shadern, weil die
    // Referenz den Blob VOR Displacement UND CalcWater in die aktuelle Seite
    // addiert (ein eigener Pass wuerde die Reihenfolge brechen).
    int drop = 0;
    int dropX = 0, dropY = 0, radius = 1;
    if (m_frameBeat)
    {
        drop = 1;
        if (params.randomDrop)
        {
            // r_waterbump.cpp:295-297: radius = drop_radius * max(w,h) / 100;
            // Position 1+radius+rand()%(dim-2*radius-1) — Reihenfolge x, y.
            radius = std::max(1, static_cast<int>(vDropR) *
                                     std::max(m_surfaceWidth, m_surfaceHeight) / 100);
            const int spanX = std::max(1, m_surfaceWidth - 2 * radius - 1);
            const int spanY = std::max(1, m_surfaceHeight - 2 * radius - 1);
            dropX = 1 + radius + static_cast<int>(m_scriptContext->nextRand() % spanX);
            dropY = 1 + radius + static_cast<int>(m_scriptContext->nextRand() % spanY);
        }
        else
        {
            radius = std::max(1, static_cast<int>(vDropR));
            auto code = [](int c, int dim) {
                return c <= 0 ? dim / 4 : (c >= 2 ? dim * 3 / 4 : dim / 2);
            };
            dropX = code(params.dropX, m_surfaceWidth);
            dropY = code(params.dropY, m_surfaceHeight);
        }
    }
    // Clip wie SineBlob: left/top/right/bottom relativ, Ende exklusiv
    int cl = -radius, ct = -radius, cr = radius, cb = radius;
    if (dropX - radius < 1) cl -= (dropX - radius - 1);
    if (dropY - radius < 1) ct -= (dropY - radius - 1);
    if (dropX + radius > m_surfaceWidth - 1)
        cr -= (dropX + radius - m_surfaceWidth + 1);
    if (dropY + radius > m_surfaceHeight - 1)
        cb -= (dropY + radius - m_surfaceHeight + 1);
    const float dropLen = (1024.0f / static_cast<float>(radius)) *
                          (1024.0f / static_cast<float>(radius));
    const auto setzeBlob = [&](QOpenGLShaderProgram& sh) {
        sh.setUniformValue("uDrop", drop);
        // ivec2/ivec4 IMMER ueber glUniform*i setzen: QPoint laedt Qt als
        // FLOAT-vec2 (glUniform2fv) hoch — auf einem ivec2 ist das ein stiller
        // GL-Fehler und die Uniform bleibt (0,0). Genau so war der ganze
        // Knoten ein bit-exakter Passthrough (Feld-Vollauf S60, 10x STUMM).
        f->glUniform2i(sh.uniformLocation("uDropPos"), dropX, dropY);
        f->glUniform4i(sh.uniformLocation("uDropClip"), cl, ct, cr, cb);
        sh.setUniformValue("uDropRad2", radius * radius);
        sh.setUniformValue("uDropHeight", -static_cast<int>(vDepth));
        sh.setUniformValue("uDropLen", dropLen);
        f->glUniform2i(sh.uniformLocation("uResI"), m_surfaceWidth,
                       m_surfaceHeight);
    };

    // --- 1. Displacement mit der AKTUELLEN Seite (Reihenfolge der Referenz:
    //        erst versetzen, dann CalcWater) --------------------------------
    const int src = rt.wbCur;
    const int dst = 1 - rt.wbCur;
    SurfacePair& pair = active();
    pair.current()->release();
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_wbDispShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_wbDispShader->setUniformValue("uImg", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, rt.wbHeight[src]->texture());
    m_wbDispShader->setUniformValue("uH", 1);
    f->glActiveTexture(GL_TEXTURE0);
    setzeBlob(*m_wbDispShader);
    m_wbDispShader->setUniformValue("uScale", static_cast<float>(vDisp));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_wbDispShader->release();
    pair.partner()->release();
    pair.swap();

    // --- 2. CalcWater: aktuelle Seite (+Blob) -> andere Seite ---------------
    rt.wbHeight[dst]->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_wbPropShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, rt.wbHeight[src]->texture());
    m_wbPropShader->setUniformValue("uH", 0);
    setzeBlob(*m_wbPropShader);
    m_wbPropShader->setUniformValue(
        "uDensity", std::clamp(static_cast<int>(vDensity), 1, 30));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_wbPropShader->release();
    rt.wbHeight[dst]->release();
    rt.wbCur = dst;
    bindActive();
}

unsigned int MultiEffectVisualizer::ensureBloomGlow(LeafRuntime& rt,
                                                    const BloomParams& params,
                                                    unsigned int srcTexture,
                                                    float threshold, int radius)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    // Glow-RTs: Surface / 2^downsample (Referenz: fix 512^2), neu bei Resize
    // oder geaendertem Divisor. Linear gefiltert — das Composite skaliert das
    // kleine RT weich auf die volle Surface hoch (Soft-Glow, kein Pixelraster).
    const int shift = std::clamp(params.downsample, 0, 4);
    const int glowW = std::max(8, m_surfaceWidth >> shift);
    const int glowH = std::max(8, m_surfaceHeight >> shift);
    if (rt.bloomRt[0] == nullptr || rt.bloomW != glowW || rt.bloomH != glowH)
    {
        for (auto& fbo : rt.bloomRt)
        {
            fbo = std::make_unique<QOpenGLFramebufferObject>(glowW, glowH);
            f->glBindTexture(GL_TEXTURE_2D, fbo->texture());
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        f->glBindTexture(GL_TEXTURE_2D, 0);
        rt.bloomW = glowW;
        rt.bloomH = glowH;
    }
    if (!rt.bloomRt[0]->isValid() || !rt.bloomRt[1]->isValid()) return 0;

    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);

    // --- Pass 1: Downsample (+ Threshold) Quelle -> RT0 ----------------------
    rt.bloomRt[0]->bind();
    f->glViewport(0, 0, glowW, glowH);
    m_bloomDownShader->bind();
    f->glBindTexture(GL_TEXTURE_2D, srcTexture);
    // Quelle fuer den Downsample linear mitteln; danach zurueck auf nearest
    // (der Vertrag der Surface-Texturen ueberall sonst, inkl. Present).
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_bloomDownShader->setUniformValue("uTex", 0);
    m_bloomDownShader->setUniformValue("uThreshold",
                                       std::clamp(threshold, 0.0f, 1.0f));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_bloomDownShader->release();
    rt.bloomRt[0]->release();

    // --- Pass 2+3: separierbarer Gauss H (RT0 -> RT1), V (RT1 -> RT0) --------
    const float sigma = static_cast<float>(std::clamp(radius, 1, 32));
    const auto gaussPass = [&](QOpenGLFramebufferObject& src,
                               QOpenGLFramebufferObject& dst,
                               const QVector2D& dir) {
        dst.bind();
        f->glViewport(0, 0, glowW, glowH);
        m_bloomGaussShader->bind();
        f->glBindTexture(GL_TEXTURE_2D, src.texture());
        m_bloomGaussShader->setUniformValue("uTex", 0);
        m_bloomGaussShader->setUniformValue("uDir", dir);
        m_bloomGaussShader->setUniformValue("uSigma", sigma);
        f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_bloomGaussShader->release();
        dst.release();
    };
    gaussPass(*rt.bloomRt[0], *rt.bloomRt[1],
              QVector2D(1.0f / static_cast<float>(glowW), 0.0f));
    gaussPass(*rt.bloomRt[1], *rt.bloomRt[0],
              QVector2D(0.0f, 1.0f / static_cast<float>(glowH)));
    m_quadVao->release();
    return rt.bloomRt[0]->texture();
}

void MultiEffectVisualizer::runBloom(const ChainNode& node,
                                     const BloomParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript). Das Skript laeuft
    // VOR dem post-Ausstieg: `post` ist die Vorgabe, ein Ausstieg davor haette
    // die drei Skriptfelder im Normalfall wirkungslos gemacht (S54).
    double vIntensity = params.intensity, vThreshold = params.threshold;
    double vRadius = params.radius, vVigStrength = params.vignetteStrength;
    runParamScript(rt, "bloom", params.initCode, params.frameCode, params.beatCode,
                   {{"intensity", &vIntensity},
                    {"threshold", &vThreshold},
                    {"radius", &vRadius},
                    {"vignettestrength", &vVigStrength}});

    // Ergebnis fuer den Present-Pfad hinterlegen — er sieht die Frame-Kopien
    // sonst nicht, weil er ausserhalb dieser Funktion laeuft.
    rt.bloomIntensity = static_cast<float>(vIntensity);
    rt.bloomThreshold = static_cast<float>(vThreshold);
    rt.bloomRadius = static_cast<int>(vRadius);
    rt.bloomVigStrength = static_cast<float>(vVigStrength);

    // post=true: Anzeige-only — der Glow entsteht beim Present (kein
    // Feedback in die Chain; S48-Befund: additiver Glow akkumulierte in
    // Fadeout-Ketten ueber die Rueckkopplung bis Weiss).
    if (params.post) return;

    auto* f = QOpenGLContext::currentContext()->functions();

    SurfacePair& pair = active();
    const unsigned int glowTex =
        ensureBloomGlow(rt, params, pair.current()->texture(),
                        static_cast<float>(vThreshold),      // Strang D
                        static_cast<int>(vRadius));
    if (glowTex == 0) return;

    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);

    // --- Composite: Surface + Glow additiv (+ Vignette) -> Partner, swap -----
    pair.partner()->bind();
    f->glViewport(0, 0, m_surfaceWidth, m_surfaceHeight);
    m_bloomCompShader->bind();
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_bloomCompShader->setUniformValue("uBase", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, glowTex);
    m_bloomCompShader->setUniformValue("uGlow", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_bloomCompShader->setUniformValue("uIntensity",
                                       std::max(0.0f, static_cast<float>(vIntensity)));
    m_bloomCompShader->setUniformValue("uVignette", params.vignette);
    m_bloomCompShader->setUniformValue(
        "uVigStrength",
        std::clamp(static_cast<float>(vVigStrength), 0.0f, 1.0f));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_bloomCompShader->release();
    pair.partner()->release();
    m_quadVao->release();
    pair.swap();
    bindActive();
}

void MultiEffectVisualizer::computeCamera3D(QMatrix4x4& view, QMatrix4x4& proj) const
{
    const Camera3D& cam = m_camera3d;
    QVector3D dir = cam.target - cam.pos;
    if (dir.lengthSquared() < 1e-12f) dir = QVector3D(0.0f, 0.0f, -1.0f);
    dir.normalize();
    QVector3D up(0.0f, 1.0f, 0.0f);  // y+ = oben (LumiViz-Weltkonvention)
    if (std::abs(QVector3D::dotProduct(dir, up)) > 0.999f)
        up = QVector3D(0.0f, 0.0f, 1.0f);  // Blick senkrecht: Ersatz-Up
    if (cam.rollDeg != 0.0f)
        up = QQuaternion::fromAxisAndAngle(dir, cam.rollDeg).rotatedVector(up);
    view.setToIdentity();
    view.lookAt(cam.pos, cam.pos + dir, up);
    proj.setToIdentity();
    const float aspect = static_cast<float>(m_surfaceWidth) /
                         static_cast<float>(std::max(1, m_surfaceHeight));
    proj.perspective(cam.fovDeg, aspect, 0.05f, 1000.0f);
}

void MultiEffectVisualizer::begin3DDepth()
{
    auto* f = QOpenGLContext::currentContext()->functions();
    if (m_depth3dTex == 0 || m_depth3dW != m_surfaceWidth ||
        m_depth3dH != m_surfaceHeight)
    {
        if (m_depth3dTex != 0) f->glDeleteTextures(1, &m_depth3dTex);
        f->glGenTextures(1, &m_depth3dTex);
        f->glBindTexture(GL_TEXTURE_2D, m_depth3dTex);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_surfaceWidth,
                        m_surfaceHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                        nullptr);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        f->glBindTexture(GL_TEXTURE_2D, 0);
        m_depth3dW = m_surfaceWidth;
        m_depth3dH = m_surfaceHeight;
        m_depth3dCleared = false;  // frische Textur -> Inhalt undefiniert
    }
    // An das AKTUELL gebundene Draw-FBO haengen — die eigene Textur ueberlebt
    // so das Farb-Ping-Pong der Surfaces (Etappe-2-Entscheid 1).
    f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                              m_depth3dTex, 0);
    f->glEnable(GL_DEPTH_TEST);
    f->glDepthFunc(GL_LESS);
    if (!m_depth3dCleared)
    {
        f->glDepthMask(GL_TRUE);
        f->glClear(GL_DEPTH_BUFFER_BIT);
        m_depth3dCleared = true;
    }
}

void MultiEffectVisualizer::end3DDepth()
{
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glDepthMask(GL_TRUE);
    f->glDisable(GL_DEPTH_TEST);
    f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                              0, 0);
}

void MultiEffectVisualizer::runCamera3D(const ChainNode& node,
                                        const Camera3DParams& params)
{
    // Startwerte aus den Params; die EEL-Slots duerfen sie ueberschreiben
    // (erster Fall der dynamischen Modulparameter, Entwurf Modul 2). Die
    // Parameterwerte stecken im Compile-Snapshot: eine Panel-Aenderung
    // kompiliert neu und seedet die Skript-Variablen frisch (wie Fractal 3D).
    double px = params.px, py = params.py, pz = params.pz;
    double tx = params.tx, ty = params.ty, tz = params.tz;
    double fov = params.fov, roll = params.roll;
    double fogStart = params.fogStart, fogEnd = params.fogEnd;

    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty();
    if (scripted)
    {
        LeafRuntime& rt = m_leafRuntimes[node.nodeId];
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + "\n#" + std::to_string(px) + ',' +
                                 std::to_string(py) + ',' + std::to_string(pz) + ',' +
                                 std::to_string(tx) + ',' + std::to_string(ty) + ',' +
                                 std::to_string(tz) + ',' + std::to_string(fov) + ',' +
                                 std::to_string(roll) + ',' + std::to_string(fogStart) +
                                 ',' + std::to_string(fogEnd);
        if (rt.cam3dHost == nullptr || rt.cam3dCompiled != snap)
        {
            rt.cam3dHost = std::make_unique<ScriptSlotHost>("camera3d", activeContext(),
                                                            ScriptSlotHost::Dialect::Avs);
            rt.cam3dHost->setSource(Slot::Init, params.initCode);
            rt.cam3dHost->setSource(Slot::Frame, params.frameCode);
            rt.cam3dHost->setSource(Slot::Beat, params.beatCode);
            rt.cam3dHost->compileAll();
            rt.cam3dCompiled = snap;
            auto& engine = rt.cam3dHost->engine();
            feedAudio(engine);  // S47-Regel: visdata muss den Erst-Lauf erreichen
            engine.setNumber("px", px);
            engine.setNumber("py", py);
            engine.setNumber("pz", pz);
            engine.setNumber("tx", tx);
            engine.setNumber("ty", ty);
            engine.setNumber("tz", tz);
            engine.setNumber("fov", fov);
            engine.setNumber("roll", roll);
            engine.setNumber("fogstart", fogStart);
            engine.setNumber("fogend", fogEnd);
            rt.cam3dHost->run(Slot::Init);
        }
        auto& engine = rt.cam3dHost->engine();
        engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
        feedAudio(engine);
        // Slot-Ordnung Frame VOR Beat (S47-Konvention, r_sscope:272)
        if (rt.cam3dHost->has(Slot::Frame)) rt.cam3dHost->run(Slot::Frame);
        if (m_frameBeat && rt.cam3dHost->has(Slot::Beat)) rt.cam3dHost->run(Slot::Beat);
        px = engine.number("px");
        py = engine.number("py");
        pz = engine.number("pz");
        tx = engine.number("tx");
        ty = engine.number("ty");
        tz = engine.number("tz");
        fov = engine.number("fov");
        roll = engine.number("roll");
        fogStart = engine.number("fogstart");
        fogEnd = engine.number("fogend");
    }

    m_camera3d.pos = QVector3D(static_cast<float>(px), static_cast<float>(py),
                               static_cast<float>(pz));
    m_camera3d.target = QVector3D(static_cast<float>(tx), static_cast<float>(ty),
                                  static_cast<float>(tz));
    m_camera3d.fovDeg = std::clamp(static_cast<float>(fov), 1.0f, 179.0f);
    m_camera3d.rollDeg = static_cast<float>(roll);
    m_camera3d.fogStart = static_cast<float>(fogStart);
    m_camera3d.fogEnd = static_cast<float>(fogEnd);
    m_camera3d.fogColor = colorToVec(params.fogColor);
}

void MultiEffectVisualizer::runSuperScope3D(const ChainNode& node,
                                            const SuperScope3DParams& params)
{
    if (m_sprite3dShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // EEL-Quartett; Parameterwerte im Snapshot (Panel-Aenderung -> Re-Seed).
    const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                             params.beatCode + '\n' + params.pointCode + "\n#" +
                             std::to_string(params.pointCount);
    if (rt.scope3dHost == nullptr || rt.scope3dCompiled != snap)
    {
        rt.scope3dHost = std::make_unique<ScriptSlotHost>(
            "superscope3d", activeContext(), ScriptSlotHost::Dialect::Avs);
        rt.scope3dHost->setSource(Slot::Init, params.initCode);
        rt.scope3dHost->setSource(Slot::Frame, params.frameCode);
        rt.scope3dHost->setSource(Slot::Beat, params.beatCode);
        rt.scope3dHost->setSource(Slot::Point, params.pointCode);
        rt.scope3dHost->compileAll();
        rt.scope3dCompiled = snap;
        auto& engine = rt.scope3dHost->engine();
        feedAudio(engine);  // S47-Regel: visdata muss den Erst-Lauf erreichen
        engine.setNumber("n", params.pointCount);
        rt.scope3dHost->run(Slot::Init);
    }
    auto& engine = rt.scope3dHost->engine();
    engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
    engine.setNumber("w", m_surfaceWidth);
    engine.setNumber("h", m_surfaceHeight);
    feedAudio(engine);
    // Slot-Ordnung Frame VOR Beat (S47-Konvention)
    if (rt.scope3dHost->has(Slot::Frame)) rt.scope3dHost->run(Slot::Frame);
    if (m_frameBeat && rt.scope3dHost->has(Slot::Beat)) rt.scope3dHost->run(Slot::Beat);
    const int n = std::clamp(static_cast<int>(engine.number("n")), 1, 4096);

    // Kamera (camera3d-Knoten oder Fallback) -> View/Proj einmal je Knoten.
    const Camera3D& cam = m_camera3d;
    constexpr float kNear = 0.05f;
    QMatrix4x4 view;
    QMatrix4x4 proj;
    computeCamera3D(view, proj);
    const QMatrix4x4 vp = proj * view;
    const float projX = proj(0, 0);  // NDC-Halbgroesse = size * proj / w
    const float projY = proj(1, 1);

    // v aus den visdata-Bytes (Byte/128-1, linear interpoliert; S45-Vertrag).
    const auto visValue = [&](double t01) -> double {
        const double fpos = std::clamp(t01, 0.0, 1.0) * 575.0;
        const int i0 = static_cast<int>(fpos);
        const int i1 = std::min(i0 + 1, 575);
        const double frac = fpos - i0;
        const auto sample = [&](int ch) {
            const unsigned char* d =
                params.spectrumSource ? visSpectrum(ch) : visWaveform(ch);
            return d[i0] * (1.0 - frac) + d[i1] * frac;
        };
        double byteVal = 0.0;
        if (params.audioChannel == 0)      byteVal = sample(0);
        else if (params.audioChannel == 1) byteVal = sample(1);
        else                               byteVal = 0.5 * (sample(0) + sample(1));
        return byteVal / 128.0 - 1.0;
    };

    const bool lines = params.renderMode == 1;
    const bool fogActive = cam.fogEnd > cam.fogStart;
    m_sprite3dVertices.clear();
    std::vector<lumi::modules::SuperscopePoint> linePts;
    if (lines) linePts.reserve(static_cast<std::size_t>(n));

    for (int pt = 0; pt < n; ++pt)
    {
        const double idx01 = n > 1 ? static_cast<double>(pt) / (n - 1) : 0.0;
        engine.setNumber("i", idx01);
        engine.setNumber("v", visValue(idx01));
        engine.setNumber("x", 0.0);
        engine.setNumber("y", 0.0);
        engine.setNumber("z", 0.0);
        engine.setNumber("size", params.size);
        engine.setNumber("red", 1.0);
        engine.setNumber("green", 1.0);
        engine.setNumber("blue", 1.0);
        engine.setNumber("skip", 0.0);
        if (rt.scope3dHost->has(Slot::Point)) rt.scope3dHost->run(Slot::Point);

        const bool skipped = engine.number("skip") != 0.0;
        const QVector3D world(static_cast<float>(engine.number("x")),
                              static_cast<float>(engine.number("y")),
                              static_cast<float>(engine.number("z")));
        const QVector4D clip = vp * QVector4D(world, 1.0f);
        const float w = clip.w();  // Tiefe entlang der Blickachse
        const bool clipped = w < kNear;

        if (lines)
        {
            // Geclippte/uebersprungene Punkte brechen den Linienzug (skip).
            lumi::modules::SuperscopePoint p;
            if (!clipped)
            {
                p.x = clip.x() / w;
                p.y = clip.y() / w;
            }
            p.skip = skipped || clipped;
            float cr = static_cast<float>(engine.number("red"));
            float cg = static_cast<float>(engine.number("green"));
            float cb = static_cast<float>(engine.number("blue"));
            if (fogActive && !clipped)
            {
                const float fogF = std::clamp(
                    (w - cam.fogStart) / (cam.fogEnd - cam.fogStart), 0.0f, 1.0f);
                cr *= 1.0f - fogF;
                cg *= 1.0f - fogF;
                cb *= 1.0f - fogF;
            }
            p.r = cr;
            p.g = cg;
            p.b = cb;
            linePts.push_back(p);
            continue;
        }

        if (skipped || clipped) continue;
        const float ndcX = clip.x() / w;
        const float ndcY = clip.y() / w;
        const float size = std::max(0.0f, static_cast<float>(engine.number("size")));
        const float halfX = size * projX / w;  // Groessen-Attenuation ~1/Tiefe
        const float halfY = size * projY / w;
        if (ndcX + halfX < -1.0f || ndcX - halfX > 1.0f ||
            ndcY + halfY < -1.0f || ndcY - halfY > 1.0f)
        {
            continue;  // Sprite komplett ausserhalb
        }
        float cr = static_cast<float>(engine.number("red"));
        float cg = static_cast<float>(engine.number("green"));
        float cb = static_cast<float>(engine.number("blue"));
        if (fogActive)
        {
            const float fogF = std::clamp(
                (w - cam.fogStart) / (cam.fogEnd - cam.fogStart), 0.0f, 1.0f);
            // Additive Sprites: Fog daempft nur (fogColor beizumischen wuerde
            // je Sprite Nebel ADDIEREN — falsch bei additivem Blend).
            cr *= 1.0f - fogF;
            cg *= 1.0f - fogF;
            cb *= 1.0f - fogF;
        }
        const float corners[6][2] = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f},
                                     {1.0f, -1.0f},  {1.0f, 1.0f}, {-1.0f, 1.0f}};
        for (const auto& c : corners)
        {
            m_sprite3dVertices.insert(m_sprite3dVertices.end(),
                                      {ndcX, ndcY, c[0], c[1], halfX, halfY,
                                       cr, cg, cb});
        }
    }

    if (lines)
    {
        drawScopeShape(linePts, false);
        return;
    }
    if (m_sprite3dVertices.empty()) return;

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_ONE, GL_ONE);  // additiv (Lights-Punkt-Sprites)
    m_sprite3dShader->bind();
    m_sprite3dShader->setUniformValue("uFalloff",
                                      std::clamp(params.falloff, 0.5f, 32.0f));
    m_sprite3dVao->bind();
    m_sprite3dVbo->bind();
    m_sprite3dVbo->allocate(m_sprite3dVertices.data(),
                            static_cast<int>(m_sprite3dVertices.size() *
                                             sizeof(float)));
    f->glDrawArrays(GL_TRIANGLES, 0,
                    static_cast<GLsizei>(m_sprite3dVertices.size() / 9));
    m_sprite3dVbo->release();
    m_sprite3dVao->release();
    m_sprite3dShader->release();
    f->glDisable(GL_BLEND);
}

void MultiEffectVisualizer::runTerrain3D(const ChainNode& node,
                                         const Terrain3DParams& params)
{
    if (m_terrain3dShader == nullptr || m_sprite3dShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();

    // --- Grid-Puffer (Reset bei resolution-Wechsel) ---------------------------
    const int res = std::clamp(params.resolution, 8, 128);
    const int cells = res * res;
    if (rt.terrainRes != res)
    {
        rt.terrainRes = res;
        rt.terrainBase.resize(static_cast<std::size_t>(cells));
        rt.terrainH.assign(static_cast<std::size_t>(cells), 0.0f);
        rt.terrainV.assign(static_cast<std::size_t>(cells), 0.0f);
        // Prozedurale Basis: zwei Sinus-Oktaven mit festen Phasen —
        // deterministisch (kein Bild-Asset, kein rand()).
        for (int gy = 0; gy < res; ++gy)
        {
            for (int gx = 0; gx < res; ++gx)
            {
                const float x = static_cast<float>(gx);
                const float y = static_cast<float>(gy);
                rt.terrainBase[static_cast<std::size_t>(gy * res + gx)] =
                    std::sin(x * 0.37f) * std::cos(y * 0.29f) * 0.6f +
                    std::sin(x * 0.83f + 1.7f) * std::cos(y * 0.71f + 0.3f) * 0.4f;
            }
        }
        for (int i = 0; i < cells; ++i)
            rt.terrainH[static_cast<std::size_t>(i)] =
                rt.terrainBase[static_cast<std::size_t>(i)] * params.baseAmp;
        rt.terrainVao.reset();  // Mesh-Objekte passen nicht mehr (IBO je res)
        rt.terrainVbo.reset();
        rt.terrainIbo.reset();
    }

    // --- EEL-Slots VOR der Sim (Director-Muster des Originals): die Slots
    // duerfen die MODI des Frames setzen (rings/relax/flatten — dynamische
    // Modulparameter) und via megabuf(gy*res+gx) die Hoehen frei formen.
    float ringsF = params.ringAmp;
    float relaxF = params.relax;
    float flattenF = params.flatten;
    const bool scripted = !params.initCode.empty() || !params.frameCode.empty() ||
                          !params.beatCode.empty() || !params.pointCode.empty();
    bool pointColors = false;
    if (scripted)
    {
        const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                                 params.beatCode + '\n' + params.pointCode + "\n#" +
                                 std::to_string(res) + ',' +
                                 std::to_string(params.ringAmp) + ',' +
                                 std::to_string(params.relax) + ',' +
                                 std::to_string(params.flatten);
        if (rt.terrainHost == nullptr || rt.terrainCompiled != snap)
        {
            rt.terrainHost = std::make_unique<ScriptSlotHost>(
                "terrain3d", activeContext(), ScriptSlotHost::Dialect::Avs);
            rt.terrainHost->setSource(Slot::Init, params.initCode);
            rt.terrainHost->setSource(Slot::Frame, params.frameCode);
            rt.terrainHost->setSource(Slot::Beat, params.beatCode);
            rt.terrainHost->setSource(Slot::Point, params.pointCode);
            rt.terrainHost->compileAll();
            rt.terrainCompiled = snap;
            auto& engine = rt.terrainHost->engine();
            feedAudio(engine);  // S47-Regel: visdata vor dem Erst-Lauf
            engine.setNumber("res", res);
            engine.setNumber("n", cells);
            engine.setNumber("rings", params.ringAmp);
            engine.setNumber("relax", params.relax);
            engine.setNumber("flatten", params.flatten);
            rt.terrainHost->run(Slot::Init);
        }
        auto& engine = rt.terrainHost->engine();
        engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
        feedAudio(engine);
        // Hoehen nur spiegeln, wenn die Slots megabuf ueberhaupt anfassen.
        const bool wantsBuf =
            rt.terrainHost->sourceMentions(Slot::Init, "megabuf") ||
            rt.terrainHost->sourceMentions(Slot::Frame, "megabuf") ||
            rt.terrainHost->sourceMentions(Slot::Beat, "megabuf");
        if (wantsBuf)
        {
            for (int i = 0; i < cells; ++i)
                engine.setMegabufValue(i, rt.terrainH[static_cast<std::size_t>(i)]);
        }
        // Slot-Ordnung Frame VOR Beat (S47-Konvention)
        if (rt.terrainHost->has(Slot::Frame)) rt.terrainHost->run(Slot::Frame);
        if (m_frameBeat && rt.terrainHost->has(Slot::Beat))
            rt.terrainHost->run(Slot::Beat);
        if (wantsBuf)
        {
            for (int i = 0; i < cells; ++i)
                rt.terrainH[static_cast<std::size_t>(i)] = static_cast<float>(
                    engine.megabufValue(i));
        }
        ringsF = std::clamp(static_cast<float>(engine.number("rings")), 0.0f, 10.0f);
        relaxF = std::clamp(static_cast<float>(engine.number("relax")), 0.0f, 1.0f);
        flattenF =
            std::clamp(static_cast<float>(engine.number("flatten")), 0.0f, 1.0f);
        pointColors = rt.terrainHost->has(Slot::Point);
    }

    // --- Simulation (Lights-Terrain-Rezept, TerrainDisplacement.js) ----------
    // updateSpectrum SETZT: h = h0 + spectrum[floor(dist)] (radiale Ringe);
    // rings < 1 blendet weich dahin. updateTerrain federt (drag = 1-dt*5,
    // v += (h0-h)*|h|*dt), updateFlat zieht zur Basis — im Original laufen
    // die Modi PHASEN-exklusiv (Director), hier als skriptbarer Mix.
    const float dtc = std::clamp(m_deltaTime, 0.0f, 1.0f / 30.0f);
    const float half = static_cast<float>(res - 1) * 0.5f;
    const float ringMix = std::min(1.0f, ringsF);
    for (int gy = 0; gy < res; ++gy)
    {
        for (int gx = 0; gx < res; ++gx)
        {
            const std::size_t i = static_cast<std::size_t>(gy * res + gx);
            const float h0 = rt.terrainBase[i] * params.baseAmp;
            float h = rt.terrainH[i];
            if (ringsF > 0.0f)
            {
                const float dx = static_cast<float>(gx) - half;
                const float dy = static_cast<float>(gy) - half;
                const float dist01 =
                    std::min(1.0f, std::sqrt(dx * dx + dy * dy) / half);
                // Spektrum mono (Bloecke 0/1 der visdata), 0..1
                const int k = static_cast<int>(dist01 * 0.7f * 575.0f);
                const float s =
                    (static_cast<float>(m_visdata[static_cast<std::size_t>(k)]) +
                     static_cast<float>(m_visdata[static_cast<std::size_t>(576 + k)])) /
                    510.0f;
                const float target = h0 + s * ringsF * 0.5f;
                h += (target - h) * ringMix;  // rings=1: hart gesetzt (Original)
            }
            // Feder-Relaxation zur Basis (updateTerrain: v = v*drag + (h0-h)*|h|*dt)
            float& v = rt.terrainV[i];
            v = v * 0.92f + (h0 - h) * (std::abs(h) + 0.1f) * dtc * 30.0f * relaxF;
            h += v * dtc * 30.0f * 0.1f;
            if (flattenF > 0.0f) h += (h0 - h) * flattenF;
            rt.terrainH[i] = h;
        }
    }

    // --- Welt-Positionen + Kamera ---------------------------------------------
    QMatrix4x4 view;
    QMatrix4x4 proj;
    computeCamera3D(view, proj);
    const QMatrix4x4 vp = proj * view;
    const Camera3D& cam = m_camera3d;
    const bool fogActive = cam.fogEnd > cam.fogStart;
    const auto worldAt = [&](int gx, int gy) {
        return QVector3D(
            (static_cast<float>(gx) / static_cast<float>(res - 1) - 0.5f) *
                params.extent,
            params.yOffset + rt.terrainH[static_cast<std::size_t>(gy * res + gx)],
            (static_cast<float>(gy) / static_cast<float>(res - 1) - 0.5f) *
                params.extent);
    };

    // --- (a) dunkles Mesh: opak, schreibt ins gemeinsame Depth-RT -------------
    if (params.drawMesh)
    {
        if (rt.terrainVao == nullptr)
        {
            rt.terrainVao = std::make_unique<QOpenGLVertexArrayObject>();
            rt.terrainVao->create();
            rt.terrainVao->bind();
            rt.terrainVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
            rt.terrainVbo->create();
            rt.terrainVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
            rt.terrainVbo->bind();
            rt.terrainVbo->allocate(nullptr, 0);
            f->glEnableVertexAttribArray(0);
            f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                                     nullptr);
            rt.terrainIbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
            rt.terrainIbo->create();
            rt.terrainIbo->bind();
            std::vector<unsigned int> idx;
            idx.reserve(static_cast<std::size_t>((res - 1) * (res - 1) * 6));
            for (int gy = 0; gy < res - 1; ++gy)
            {
                for (int gx = 0; gx < res - 1; ++gx)
                {
                    const unsigned int a = static_cast<unsigned int>(gy * res + gx);
                    const unsigned int b = a + static_cast<unsigned int>(res);
                    idx.insert(idx.end(), {a, b, a + 1, a + 1, b, b + 1});
                }
            }
            rt.terrainIndexCount = static_cast<int>(idx.size());
            rt.terrainIbo->allocate(idx.data(),
                                    static_cast<int>(idx.size() * sizeof(unsigned int)));
            rt.terrainVao->release();
        }
        m_warpVertices.clear();  // CPU-Scratch mitbenutzen (pos.xyz)
        m_warpVertices.reserve(static_cast<std::size_t>(cells) * 3);
        for (int gy = 0; gy < res; ++gy)
        {
            for (int gx = 0; gx < res; ++gx)
            {
                const QVector3D w = worldAt(gx, gy);
                m_warpVertices.insert(m_warpVertices.end(), {w.x(), w.y(), w.z()});
            }
        }
        begin3DDepth();
        f->glDepthMask(GL_TRUE);
        m_terrain3dShader->bind();
        m_terrain3dShader->setUniformValue("uVp", vp);
        m_terrain3dShader->setUniformValue("uCamPos", cam.pos);
        m_terrain3dShader->setUniformValue("uColor", colorToVec(params.meshColor));
        m_terrain3dShader->setUniformValue("uFogStart", cam.fogStart);
        m_terrain3dShader->setUniformValue("uFogEnd", cam.fogEnd);
        m_terrain3dShader->setUniformValue("uFogColor", cam.fogColor);
        rt.terrainVao->bind();
        rt.terrainVbo->bind();
        rt.terrainVbo->allocate(m_warpVertices.data(),
                                static_cast<int>(m_warpVertices.size() * sizeof(float)));
        f->glDrawElements(GL_TRIANGLES, rt.terrainIndexCount, GL_UNSIGNED_INT,
                          nullptr);
        rt.terrainVbo->release();
        rt.terrainVao->release();
        m_terrain3dShader->release();
        f->glDepthMask(GL_FALSE);  // Sprites testen nur noch
    }

    // --- (b) additive Soft-Sprites an den Gitterpunkten -----------------------
    if (params.drawDots)
    {
        if (!params.drawMesh) begin3DDepth();  // testen gegen fremde 3D-Opaque
        f->glDepthMask(GL_FALSE);
        const float projX = proj(0, 0);
        const float projY = proj(1, 1);
        const QVector3D cLow = colorToVec(params.colorLow);
        const QVector3D cHigh = colorToVec(params.colorHigh);
        const float hRange = std::max(0.05f, params.baseAmp * 2.0f);
        lumi::scripting::LuaScriptEngine* pointEngine =
            pointColors ? &rt.terrainHost->engine() : nullptr;
        m_sprite3dVertices.clear();
        for (int gy = 0; gy < res; ++gy)
        {
            for (int gx = 0; gx < res; ++gx)
            {
                const int i = gy * res + gx;
                const QVector3D w3 = worldAt(gx, gy);
                const QVector4D clip = vp * QVector4D(w3, 1.0f);
                const float w = clip.w();
                if (w < 0.05f) continue;
                const float ndcX = clip.x() / w;
                const float ndcY = clip.y() / w;
                const float halfX = params.dotSize * projX / w;
                const float halfY = params.dotSize * projY / w;
                if (ndcX + halfX < -1.0f || ndcX - halfX > 1.0f ||
                    ndcY + halfY < -1.0f || ndcY - halfY > 1.0f)
                {
                    continue;
                }
                const float h = rt.terrainH[static_cast<std::size_t>(i)];
                float t = std::clamp(0.5f + h / hRange, 0.0f, 1.0f);
                float cr = cLow.x() + (cHigh.x() - cLow.x()) * t;
                float cg = cLow.y() + (cHigh.y() - cLow.y()) * t;
                float cb = cLow.z() + (cHigh.z() - cLow.z()) * t;
                if (pointEngine != nullptr)
                {
                    pointEngine->setNumber("i", i);
                    pointEngine->setNumber("gx", gx);
                    pointEngine->setNumber("gy", gy);
                    pointEngine->setNumber("h", h);
                    pointEngine->setNumber("red", cr);
                    pointEngine->setNumber("green", cg);
                    pointEngine->setNumber("blue", cb);
                    rt.terrainHost->run(Slot::Point);
                    cr = static_cast<float>(pointEngine->number("red"));
                    cg = static_cast<float>(pointEngine->number("green"));
                    cb = static_cast<float>(pointEngine->number("blue"));
                }
                if (fogActive)
                {
                    const float fogF = std::clamp(
                        (w - cam.fogStart) / (cam.fogEnd - cam.fogStart), 0.0f,
                        1.0f);
                    cr *= 1.0f - fogF;
                    cg *= 1.0f - fogF;
                    cb *= 1.0f - fogF;
                }
                const float corners[6][2] = {{-1.0f, -1.0f}, {1.0f, -1.0f},
                                             {-1.0f, 1.0f},  {1.0f, -1.0f},
                                             {1.0f, 1.0f},   {-1.0f, 1.0f}};
                for (const auto& c : corners)
                {
                    m_sprite3dVertices.insert(m_sprite3dVertices.end(),
                                              {ndcX, ndcY, c[0], c[1], halfX,
                                               halfY, cr, cg, cb});
                }
            }
        }
        if (!m_sprite3dVertices.empty())
        {
            f->glEnable(GL_BLEND);
            f->glBlendFunc(GL_ONE, GL_ONE);
            m_sprite3dShader->bind();
            m_sprite3dShader->setUniformValue(
                "uFalloff", std::clamp(params.falloff, 0.5f, 32.0f));
            m_sprite3dVao->bind();
            m_sprite3dVbo->bind();
            m_sprite3dVbo->allocate(
                m_sprite3dVertices.data(),
                static_cast<int>(m_sprite3dVertices.size() * sizeof(float)));
            f->glDrawArrays(GL_TRIANGLES, 0,
                            static_cast<GLsizei>(m_sprite3dVertices.size() / 9));
            m_sprite3dVbo->release();
            m_sprite3dVao->release();
            m_sprite3dShader->release();
            f->glDisable(GL_BLEND);
        }
    }
    if (params.drawMesh || params.drawDots) end3DDepth();
}

void MultiEffectVisualizer::runGlowOrbs(const ChainNode& node,
                                        const GlowOrbsParams& params)
{
    if (m_orb3dShader == nullptr || m_sprite3dShader == nullptr) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto* f = QOpenGLContext::currentContext()->functions();

    const std::string snap = params.initCode + '\n' + params.frameCode + '\n' +
                             params.beatCode + '\n' + params.pointCode + "\n#" +
                             std::to_string(params.orbCount);
    if (rt.orbsHost == nullptr || rt.orbsCompiled != snap)
    {
        rt.orbsHost = std::make_unique<ScriptSlotHost>("glowOrbs", activeContext(),
                                                       ScriptSlotHost::Dialect::Avs);
        rt.orbsHost->setSource(Slot::Init, params.initCode);
        rt.orbsHost->setSource(Slot::Frame, params.frameCode);
        rt.orbsHost->setSource(Slot::Beat, params.beatCode);
        rt.orbsHost->setSource(Slot::Point, params.pointCode);
        rt.orbsHost->compileAll();
        rt.orbsCompiled = snap;
        auto& engine = rt.orbsHost->engine();
        feedAudio(engine);  // S47-Regel: visdata vor dem Erst-Lauf
        engine.setNumber("n", params.orbCount);
        rt.orbsHost->run(Slot::Init);
    }
    auto& engine = rt.orbsHost->engine();
    engine.setNumber("b", m_frameBeat ? 1.0 : 0.0);
    feedAudio(engine);
    // Slot-Ordnung Frame VOR Beat (S47-Konvention)
    if (rt.orbsHost->has(Slot::Frame)) rt.orbsHost->run(Slot::Frame);
    if (m_frameBeat && rt.orbsHost->has(Slot::Beat)) rt.orbsHost->run(Slot::Beat);
    const int n = std::clamp(static_cast<int>(engine.number("n")), 1, 64);

    QMatrix4x4 view;
    QMatrix4x4 proj;
    computeCamera3D(view, proj);
    const QMatrix4x4 vp = proj * view;
    const Camera3D& cam = m_camera3d;
    const bool fogActive = cam.fogEnd > cam.fogStart;

    struct OrbState
    {
        QVector3D center;
        QVector3D scale;
        QVector3D color;
        QVector3D color2;
        float flash = 0.0f;
    };
    std::vector<OrbState> orbs;
    orbs.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
    {
        // Defaults: Reihe entlang x, warmer Verlauf; der Point-Slot (je Orb,
        // i = Orb-INDEX) darf alles ueberschreiben.
        engine.setNumber("i", i);
        engine.setNumber("x", (static_cast<double>(i) - (n - 1) * 0.5) * 1.2);
        engine.setNumber("y", 0.0);
        engine.setNumber("z", 0.0);
        engine.setNumber("radius", 0.4);
        engine.setNumber("sx", 1.0);
        engine.setNumber("sy", 1.0);
        engine.setNumber("sz", 1.0);
        engine.setNumber("red", 1.0);
        engine.setNumber("green", 0.55);
        engine.setNumber("blue", 0.2);
        engine.setNumber("red2", 0.25);
        engine.setNumber("green2", 0.05);
        engine.setNumber("blue2", 0.35);
        engine.setNumber("flash", 0.0);
        if (rt.orbsHost->has(Slot::Point)) rt.orbsHost->run(Slot::Point);
        OrbState o;
        o.center = QVector3D(static_cast<float>(engine.number("x")),
                             static_cast<float>(engine.number("y")),
                             static_cast<float>(engine.number("z")));
        const float radius =
            std::max(0.0001f, static_cast<float>(engine.number("radius")));
        o.scale = QVector3D(
            radius * std::max(0.01f, static_cast<float>(engine.number("sx"))),
            radius * std::max(0.01f, static_cast<float>(engine.number("sy"))),
            radius * std::max(0.01f, static_cast<float>(engine.number("sz"))));
        o.color = QVector3D(static_cast<float>(engine.number("red")),
                            static_cast<float>(engine.number("green")),
                            static_cast<float>(engine.number("blue")));
        o.color2 = QVector3D(static_cast<float>(engine.number("red2")),
                             static_cast<float>(engine.number("green2")),
                             static_cast<float>(engine.number("blue2")));
        o.flash = std::clamp(static_cast<float>(engine.number("flash")), 0.0f, 1.0f);
        orbs.push_back(o);
    }

    // --- (a) Ellipsoide: opak ins gemeinsame Depth-RT -------------------------
    begin3DDepth();
    f->glDepthMask(GL_TRUE);
    m_orb3dShader->bind();
    m_orb3dShader->setUniformValue("uVp", vp);
    m_orb3dShader->setUniformValue("uCamPos", cam.pos);
    m_orb3dShader->setUniformValue("uFogStart", cam.fogStart);
    m_orb3dShader->setUniformValue("uFogEnd", cam.fogEnd);
    m_orb3dShader->setUniformValue("uFogColor", cam.fogColor);
    m_orbVao->bind();
    for (const OrbState& o : orbs)
    {
        m_orb3dShader->setUniformValue("uCenter", o.center);
        m_orb3dShader->setUniformValue("uScale", o.scale);
        m_orb3dShader->setUniformValue("uColor", o.color);
        m_orb3dShader->setUniformValue("uColor2", o.color2);
        m_orb3dShader->setUniformValue("uFlash", o.flash);
        f->glDrawElements(GL_TRIANGLES, m_orbIndexCount, GL_UNSIGNED_SHORT,
                          nullptr);
    }
    m_orbVao->release();
    m_orb3dShader->release();

    // --- (b) Halo-Billboards: additiv, Depth-Test OHNE Write => Rim-Glow ------
    if (params.haloIntensity > 0.0f)
    {
        f->glDepthMask(GL_FALSE);
        const float projX = proj(0, 0);
        const float projY = proj(1, 1);
        m_sprite3dVertices.clear();
        for (const OrbState& o : orbs)
        {
            const QVector4D clip = vp * QVector4D(o.center, 1.0f);
            const float w = clip.w();
            if (w < 0.05f) continue;
            const float ndcX = clip.x() / w;
            const float ndcY = clip.y() / w;
            const float r = std::max(o.scale.x(), o.scale.y()) * params.haloScale;
            const float halfX = r * projX / w;
            const float halfY = r * projY / w;
            if (ndcX + halfX < -1.0f || ndcX - halfX > 1.0f ||
                ndcY + halfY < -1.0f || ndcY - halfY > 1.0f)
            {
                continue;
            }
            QVector3D c = (o.color * 0.7f + o.color2 * 0.3f) *
                              (params.haloIntensity + o.flash) ;
            if (fogActive)
            {
                const float fogF = std::clamp(
                    (w - cam.fogStart) / (cam.fogEnd - cam.fogStart), 0.0f, 1.0f);
                c *= 1.0f - fogF;
            }
            const float corners[6][2] = {{-1.0f, -1.0f}, {1.0f, -1.0f},
                                         {-1.0f, 1.0f},  {1.0f, -1.0f},
                                         {1.0f, 1.0f},   {-1.0f, 1.0f}};
            for (const auto& cn : corners)
            {
                m_sprite3dVertices.insert(m_sprite3dVertices.end(),
                                          {ndcX, ndcY, cn[0], cn[1], halfX, halfY,
                                           c.x(), c.y(), c.z()});
            }
        }
        if (!m_sprite3dVertices.empty())
        {
            f->glEnable(GL_BLEND);
            f->glBlendFunc(GL_ONE, GL_ONE);
            m_sprite3dShader->bind();
            m_sprite3dShader->setUniformValue(
                "uFalloff", std::clamp(params.falloff, 0.5f, 32.0f));
            m_sprite3dVao->bind();
            m_sprite3dVbo->bind();
            m_sprite3dVbo->allocate(
                m_sprite3dVertices.data(),
                static_cast<int>(m_sprite3dVertices.size() * sizeof(float)));
            f->glDrawArrays(GL_TRIANGLES, 0,
                            static_cast<GLsizei>(m_sprite3dVertices.size() / 9));
            m_sprite3dVbo->release();
            m_sprite3dVao->release();
            m_sprite3dShader->release();
            f->glDisable(GL_BLEND);
        }
    }
    end3DDepth();
}

void MultiEffectVisualizer::runStarfield(const ChainNode& node,
                                         const StarfieldParams& params)
{
    // Zeilengetreu nach r_stars.cpp:198-265 (S49) — inklusive der Zuege aus dem
    // geteilten rand()-Strom des Presets: 4 je Stern beim Anlegen, 2 je
    // Neugeburt. Sterne leben in PIXELN um die Bildmitte, nicht in NDC.
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    auto& rnd = *m_scriptContext;
    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    const int xOff = w / 2;
    const int yOff = h / 2;

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vStars = params.maxStars, vWarp = params.warpSpeed;
    double vBeatSpeed = params.beatSpeed, vDuration = params.durationFrames;
    runParamScript(rt, "starfield", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"maxstars", &vStars},
                    {"warpspeed", &vWarp},
                    {"beatspeed", &vBeatSpeed},
                    {"durationframes", &vDuration}});

    if (params.onBeat && m_frameBeat)
    {
        rt.starSpeed = static_cast<float>(vBeatSpeed);
        rt.starBeatFrames = std::max(1, static_cast<int>(vDuration));
        rt.starIncBeat = (static_cast<float>(vWarp) - rt.starSpeed) /
                         static_cast<float>(rt.starBeatFrames);
    }
    if (rt.starSpeed <= 0.0f && rt.starBeatFrames == 0)
        rt.starSpeed = static_cast<float>(vWarp);

    // MaxStars = MulDiv(gesetzt, w*h, 512*384), gedeckelt auf 4095
    const long long scaled = (static_cast<long long>(vStars) * w * h +
                              (512LL * 384) / 2) / (512LL * 384);
    const int maxStars = std::min(4095, std::max(1, static_cast<int>(scaled)));
    if (rt.starW != w || rt.starH != h ||
        static_cast<int>(rt.stars.size()) != maxStars)
    {
        rt.stars.assign(static_cast<std::size_t>(maxStars), Star{});
        for (Star& st : rt.stars)
        {
            st.x = rnd.nextRand() % std::max(1, w) - xOff;
            st.y = rnd.nextRand() % std::max(1, h) - yOff;
            st.z = static_cast<float>(rnd.nextRand() % 255);
            st.speed = static_cast<float>(rnd.nextRand() % 9 + 1) / 10.0f;
        }
        rt.starW = w;
        rt.starH = h;
    }

    // CreateStar (r_stars.cpp:180-185): nur X und Y werden gezogen, Z = Zoff
    auto createStar = [&](Star& st) {
        st.x = rnd.nextRand() % std::max(1, w) - xOff;
        st.y = rnd.nextRand() % std::max(1, h) - yOff;
        st.z = 255.0f;  // Zoff
    };
    const int colR = static_cast<int>((params.color >> 16) & 0xFF);
    const int colG = static_cast<int>((params.color >> 8) & 0xFF);
    const int colB = static_cast<int>(params.color & 0xFF);
    const bool tinted = (params.color & 0xFFFFFF) != 0xFFFFFF;

    std::vector<lumi::modules::SuperscopePoint> points;
    points.reserve(rt.stars.size());
    for (Star& st : rt.stars)
    {
        if (static_cast<int>(st.z) <= 0)
        {
            createStar(st);
            continue;
        }
        const int nx = ((st.x << 7) / static_cast<int>(st.z)) + xOff;
        const int ny = ((st.y << 7) / static_cast<int>(st.z)) + yOff;
        if (nx <= 0 || nx >= w || ny <= 0 || ny >= h)
        {
            createStar(st);
            continue;
        }
        const int c = static_cast<int>((255 - static_cast<int>(st.z)) * st.speed);
        int r = c, g = c, b = c;
        if (tinted)
        {
            // BLEND_ADAPT (r_stars.cpp:186-189): je Kanal ((a>>4)*(16-d) +
            // (b>>4)*d) mit d = c>>4 — die Nibble-Rechnung haelt das Ergebnis
            // ohne Endschieben im 8-Bit-Feld.
            const int d = c >> 4;
            r = (c >> 4) * (16 - d) + (colR >> 4) * d;
            g = (c >> 4) * (16 - d) + (colG >> 4) * d;
            b = (c >> 4) * (16 - d) + (colB >> 4) * d;
        }
        lumi::modules::SuperscopePoint p;
        // Pixelmitte -> NDC, damit der Punkt genau auf (nx, ny) landet
        p.x = (static_cast<float>(nx) + 0.5f) / static_cast<float>(w) * 2.0f - 1.0f;
        p.y = 1.0f - (static_cast<float>(ny) + 0.5f) / static_cast<float>(h) * 2.0f;
        p.r = static_cast<float>(std::clamp(r, 0, 255)) / 255.0f;
        p.g = static_cast<float>(std::clamp(g, 0, 255)) / 255.0f;
        p.b = static_cast<float>(std::clamp(b, 0, 255)) / 255.0f;
        p.a = 1.0f;
        points.push_back(p);
        st.z -= st.speed * rt.starSpeed;
    }

    // r_stars.cpp:258-264: Rueckkehr zur Grundgeschwindigkeit
    if (rt.starBeatFrames == 0)
    {
        rt.starSpeed = static_cast<float>(vWarp);
    }
    else
    {
        rt.starSpeed = std::max(0.0f, rt.starSpeed + rt.starIncBeat);
        --rt.starBeatFrames;
    }

    if (points.empty() || !m_scopeRenderer.ready()) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    if (params.blend == 1)  // BLEND: saettigende Addition
    {
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_ONE, GL_ONE);
    }
    else if (params.blend == 2)  // BLEND_AVG (GL rundet, das Original schneidet ab)
    {
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
        f->glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);
    }
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = lumi::modules::SuperscopeRenderMode::Dots;
    rp.dotSize = 1.0f;
    rp.glowEnabled = false;
    m_scopeRenderer.draw(points, rp);
    if (params.blend != 0) f->glDisable(GL_BLEND);
}

namespace {

/// Palette-Farbe i als RGB 0..1 (0x00RRGGBB), mit Umlauf.
void paletteRgb(const std::vector<uint32_t>& colors, int i,
                float& r, float& g, float& b)
{
    const uint32_t c = colors.empty()
                           ? 0xFFFFFFu
                           : colors[static_cast<std::size_t>(i) % colors.size()];
    r = static_cast<float>((c >> 16) & 0xFF) / 255.0f;
    g = static_cast<float>((c >> 8) & 0xFF) / 255.0f;
    b = static_cast<float>(c & 0xFF) / 255.0f;
}

}  // namespace

void MultiEffectVisualizer::runMetaballs3D(const ChainNode& node,
                                           const Metaballs3DParams& params)
{
    // Verhaltens-Nachbau der closed-source-APE "Metaballs 3D" (UnConeD, S52) —
    // wie FyrewurX (S38): aus dem Preset kommt NUR die Farbtafel, die Geometrie
    // ist host-eigen. Nachgebaut wird, was der Effekt tut: mehrere Kugeln
    // wandern auf Lissajous-Bahnen durch einen Raum; ihr summiertes 1/r²-Feld
    // wird geschwellt, die Oberflaeche traegt die Palette-Farbe der jeweils
    // naechsten Kugel. Kein Anspruch auf Pixelgleichheit — die Referenz kann
    // das gar nicht liefern (die APE-DLL ist nicht deterministisch, S22).
    if (!m_metaballShader) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vCount = params.count, vRadius = params.radius, vSpeed = params.speed;
    double vThresh = params.threshold, vSpread = params.spread;
    double vDepth = params.depth, vPhase = params.phase;
    runParamScript(rt, "metaballs", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"count", &vCount},
                    {"radius", &vRadius},
                    {"speed", &vSpeed},
                    {"threshold", &vThresh},
                    {"spread", &vSpread},
                    {"depth", &vDepth},
                    {"phase", &vPhase}});

    const int n = std::clamp(static_cast<int>(vCount), 1, 16);
    const float t = m_time * static_cast<float>(vSpeed);

    std::array<QVector4D, 16> spheres{};  // xyz + Radius
    std::array<QVector3D, 16> tints{};
    for (int i = 0; i < n; ++i)
    {
        // S53: Phasenversatz, Bahnweite und Tiefe sind Parameter geworden;
        // ihre Vorgaben (1,7 · 1,0 · 1,2) sind die frueheren Literale.
        const float p = static_cast<float>(i) * static_cast<float>(vPhase);
        const float sp = static_cast<float>(vSpread);
        // Teilerfremde Frequenzen: die Kugeln treffen sich nie periodisch.
        const float x = std::sin(t * 0.71f + p) * 0.62f * sp;
        const float y = std::sin(t * 0.53f + p * 1.3f) * 0.55f * sp;
        const float z = std::sin(t * 0.37f + p * 0.7f) * 0.5f + static_cast<float>(vDepth);
        // Perspektive: entferntere Kugeln werden kleiner und wandern zur Mitte.
        const float w = 1.0f / z;
        spheres[static_cast<std::size_t>(i)] =
            QVector4D(x * w, y * w, z, static_cast<float>(vRadius) * w);
        float r = 1.0f, g = 1.0f, b = 1.0f;
        paletteRgb(params.colors, i, r, g, b);
        tints[static_cast<std::size_t>(i)] = QVector3D(r, g, b);
    }

    applyLineBlend(params.blend == 0 ? 0 : (params.blend == 1 ? 1 : 3),
                   m_renderMode.alpha);
    m_metaballShader->bind();
    m_metaballShader->setUniformValue("uCount", n);
    m_metaballShader->setUniformValue("uThreshold", static_cast<float>(vThresh));
    m_metaballShader->setUniformValue(
        "uAspect", static_cast<float>(m_surfaceWidth) /
                       std::max(1.0f, static_cast<float>(m_surfaceHeight)));
    m_metaballShader->setUniformValueArray("uSphere", spheres.data(), 16);
    m_metaballShader->setUniformValueArray("uTint", tints.data(), 16);
    m_quadVao->bind();
    QOpenGLContext::currentContext()->functions()->glDrawArrays(GL_TRIANGLE_STRIP,
                                                                0, 4);
    m_quadVao->release();
    m_metaballShader->release();
    resetLineBlend();
}

void MultiEffectVisualizer::runTentacles3D(const ChainNode& node,
                                           const Tentacles3DParams& params)
{
    // Verhaltens-Nachbau der closed-source-APE "Tentacles 3D" (UnConeD, S52),
    // gleiche Lage wie Metaballs. Mehrere Tentakel wachsen aus der Bildmitte
    // nach aussen und schwingen; je Tentakel eine Palette-Farbe, die Dicke
    // nimmt zur Spitze hin ab (deshalb ein Zug je Segment statt eines Laufs).
    if (!m_scopeRenderer.ready()) return;
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vCount = params.count, vSeg = params.segments, vLen = params.length;
    double vThick = params.thickness, vSpeed = params.speed;
    double vSway = params.sway, vWaves = params.waves, vTaper = params.taper;
    runParamScript(rt, "tentacles", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"count", &vCount},
                    {"segments", &vSeg},
                    {"length", &vLen},
                    {"thickness", &vThick},
                    {"speed", &vSpeed},
                    {"sway", &vSway},
                    {"waves", &vWaves},
                    {"taper", &vTaper}});

    const int n = std::clamp(static_cast<int>(vCount), 1, 16);
    const int seg = std::clamp(static_cast<int>(vSeg), 2, 256);
    const float t = m_time * static_cast<float>(vSpeed);
    const float aspect = static_cast<float>(m_surfaceWidth) /
                         std::max(1.0f, static_cast<float>(m_surfaceHeight));

    applyLineBlend(params.blend == 0 ? 0 : (params.blend == 1 ? 1 : 3),
                   m_renderMode.alpha);
    for (int i = 0; i < n; ++i)
    {
        float r = 1.0f, g = 1.0f, b = 1.0f;
        paletteRgb(params.colors, i, r, g, b);
        const float base = static_cast<float>(i) * 6.2831853f / static_cast<float>(n);
        // Segmentweise zeichnen: nur so nimmt die Linienbreite zur Spitze ab.
        for (int s = 0; s + 1 < seg; ++s)
        {
            std::vector<lumi::modules::SuperscopePoint> run;
            run.reserve(2);
            for (int k = 0; k < 2; ++k)
            {
                const float u = static_cast<float>(s + k) / static_cast<float>(seg - 1);
                // Schwingung waechst zur Spitze — Wurzel steht ruhig.
                // S53: Wellenzahl und Schwingweite sind Parameter (3,1 · 0,9).
                const float ang = base + std::sin(t + u * static_cast<float>(vWaves) +
                                                  base) *
                                             static_cast<float>(vSway) * u;
                const float rad = u * static_cast<float>(vLen);
                lumi::modules::SuperscopePoint pt;
                pt.x = std::cos(ang) * rad / aspect;
                pt.y = std::sin(ang) * rad;
                pt.r = r;
                pt.g = g;
                pt.b = b;
                pt.a = 1.0f;
                run.push_back(pt);
            }
            lumi::render::ScopeRenderer::Params rp;
            rp.mode = lumi::modules::SuperscopeRenderMode::Lines;
            const float u = static_cast<float>(s) / static_cast<float>(seg - 1);
            // taper=1 → linear auf 0 (bisheriges Verhalten), 0 → gleich dick.
            rp.lineWidth = std::max(1.0f, static_cast<float>(vThick) *
                                              (1.0f - u * static_cast<float>(vTaper)));
            m_scopeRenderer.draw(run, rp);
        }
    }
    resetLineBlend();
}

void MultiEffectVisualizer::runFyrewurX(const ChainNode& node,
                                        const FyrewurXParams& params)
{
    // Behavioral rebuild of the closed-source "FunkyFX FyrewurX v1" APE:
    // every beat launches a firework burst; sparks fly out radially, gravity
    // pulls them down (AVS convention: +y = screen bottom), and they fade out
    // over their lifetime. Constants are sight-calibration points.
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vSparks = params.sparks, vSpeed = params.speed;
    double vGravity = params.gravity, vLife = params.lifeSeconds;
    double vDot = params.dotSize, vHue = params.hueDrift, vBurst = params.burstSpread;
    runParamScript(rt, "fyrewurx", params.initCode, params.frameCode, params.beatCode,
                   {{"sparks", &vSparks},
                    {"speed", &vSpeed},
                    {"gravity", &vGravity},
                    {"life", &vLife},
                    {"dotsize", &vDot},
                    {"huedrift", &vHue},
                    {"burstspread", &vBurst}});

    if (m_frameBeat && rt.fwSparks.size() < 4096)
    {
        auto frand = [this] { return (nextRandom() % 10000u) / 10000.0f; };
        // S53: `burstSpread` skaliert die Streuung (Vorgabe 1 = wie bisher).
        const float sp = static_cast<float>(vBurst);
        const float cx = (frand() * 1.6f - 0.8f) * sp;  // burst center
        const float cy = (frand() * 1.0f - 0.7f) * sp;  // upper screen area
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
        const int sparkCount = std::clamp(static_cast<int>(vSparks), 0, 4096);
        for (int i = 0; i < sparkCount; ++i)
        {
            FwSpark s;
            const float ang = frand() * 6.2831853f;
            const float spd = static_cast<float>(vSpeed) * (0.25f + 0.75f * frand());
            s.x = cx;
            s.y = cy;
            s.vx = std::cos(ang) * spd;
            s.vy = std::sin(ang) * spd;
            s.lifeMax = static_cast<float>(vLife) * (0.6f + 0.4f * frand());
            s.life = s.lifeMax;
            hueRgb(hue + (frand() - 0.5f) * static_cast<float>(vHue), s.r, s.g, s.b);
            rt.fwSparks.push_back(s);
        }
    }

    // Integrate + cull.
    const float dt = std::clamp(m_deltaTime, 0.0f, 0.1f);
    for (FwSpark& s : rt.fwSparks)
    {
        s.vy += static_cast<float>(vGravity) * dt;
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
    rp.dotSize = std::max(1.0f, static_cast<float>(vDot));
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

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vBands = params.bands, vChannel = params.channel;
    runParamScript(rt, "timescope", params.initCode, params.frameCode,
                   params.beatCode, {{"bands", &vBands}, {"channel", &vChannel}});

    rt.timescopeX = (rt.timescopeX + 1) % m_surfaceWidth;

    // Die ROHEN visdata-Spektrum-Bytes als R8-Textur. Vorgabe ist der LINKE
    // Kanal, weil r_timescope `visdata[0][0]` fest liest — das dort aus
    // `which_ch` gebaute `fa_data` wird nie benutzt (S48-Matrix-Befund 39).
    // Erst `useChannel` macht den Regler wirksam (S54): dann zaehlt `channel`,
    // bei 2 der Mittelwert beider Kanaele wie in r_timescope.cpp:131.
    const unsigned char* spec = visSpectrum(0);
    std::array<unsigned char, 576> mixed{};
    if (params.useChannel)
    {
        const int ch = std::clamp(static_cast<int>(vChannel), 0, 2);
        if (ch >= 2)
        {
            const unsigned char* l = visSpectrum(0);
            const unsigned char* r = visSpectrum(1);
            for (std::size_t i = 0; i < mixed.size(); ++i)
                mixed[i] = static_cast<unsigned char>((static_cast<int>(l[i]) +
                                                       static_cast<int>(r[i])) / 2);
            spec = mixed.data();
        }
        else
        {
            spec = visSpectrum(ch);
        }
    }

    if (m_specTex == 0) f->glGenTextures(1, &m_specTex);
    f->glBindTexture(GL_TEXTURE_2D, m_specTex);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 576, 1, 0, GL_RED,
                    GL_UNSIGNED_BYTE, spec);
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

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
    else if (params.blend == 3)
    {
        // BLEND_LINE (r_timescope.cpp:147-148, AVS-Default) — folgt SRM (S3)
        f->glEnable(GL_BLEND);
        applyLineBlend(m_renderMode.lineBlend, m_renderMode.alpha);
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
                                       static_cast<float>(std::clamp(
                                           static_cast<int>(vBands), 1, 576)));
    m_timescopeShader->setUniformValue("uH", static_cast<float>(m_surfaceHeight));
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_timescopeShader->release();

    f->glDisable(GL_SCISSOR_TEST);
    resetLineBlend();
    pair.current()->release();
    bindActive();
}

// Draw a batch of points via the scope renderer (Dot renderers). `blend`:
// 0 replace, 1 additive, 2 50/50, 3 BLEND_LINE = dem SRM-Zustand folgen
// (S3/S9; r_dotpln/r_dotfnt zeichnen immer BLEND_LINE, r_dotgrid waehlbar).
void MultiEffectVisualizer::drawDots(
    const std::vector<lumi::modules::SuperscopePoint>& points, float dotSize,
    int blend)
{
    if (points.empty() || !m_scopeRenderer.ready()) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    switch (blend)
    {
        case 0:  f->glBlendFunc(GL_ONE, GL_ZERO); break;
        case 1:  f->glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
        case 2:
            f->glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
            f->glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);
            break;
        default: applyLineBlend(m_renderMode.lineBlend, m_renderMode.alpha); break;
    }
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = lumi::modules::SuperscopeRenderMode::Dots;
    rp.dotSize = dotSize;
    rp.glowEnabled = false;
    m_scopeRenderer.draw(points, rp);
    resetLineBlend();
}

namespace {
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
    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vSpacing = params.spacing, vXMove = params.xMove, vYMove = params.yMove;
    runParamScript(rt, "dotgrid", params.initCode, params.frameCode, params.beatCode,
                   {{"spacing", &vSpacing}, {"xmove", &vXMove}, {"ymove", &vYMove}});

    const int spacing = std::max(2, static_cast<int>(vSpacing));
    const int nc = static_cast<int>(params.colors.size());
    if (nc == 0) return;

    // Farbzyklus zeilengenau nach r_dotgrid.cpp:71-86 — GANZZAHLIG und mit
    // `(63-r)` gegen `r`, geteilt durch 64. Die beiden Gewichte summieren sich
    // also zu 63/64, nicht zu 1: die Referenzfarbe ist durchgehend rund 1,6 %
    // dunkler als eine saubere Interpolation. Genau das rechneten wir vorher.
    rt.dotColorPos += 1.0f;
    const float span = static_cast<float>(nc) * 64.0f;
    if (rt.dotColorPos >= span) rt.dotColorPos -= span;
    const int cp = static_cast<int>(rt.dotColorPos);
    const int p0 = cp / 64;
    const int cr = cp & 63;
    const uint32_t c1 = params.colors[static_cast<size_t>(p0)];
    const uint32_t c2 = params.colors[
        static_cast<size_t>(p0 + 1 < nc ? p0 + 1 : 0)];
    auto misch = [&](int shift) {
        const int a = static_cast<int>((c1 >> shift) & 0xFF);
        const int b = static_cast<int>((c2 >> shift) & 0xFF);
        return static_cast<float>((a * (63 - cr) + b * cr) / 64) / 255.0f;
    };
    const QVector3D col(misch(16), misch(8), misch(0));

    // Position: `xp`/`yp` sind 8.8-FESTKOMMA-Ganzzahlen, und der Rasterversatz
    // ist `(xp>>8) % spacing` — also ein GANZER Pixel (r_dotgrid.cpp:88-94).
    // Wir fuehrten sie als float und setzten die Punkte auf Zwischenpositionen.
    int xp = static_cast<int>(rt.dotOffX);
    int yp = static_cast<int>(rt.dotOffY);
    while (yp < 0) yp += spacing * 256;
    while (xp < 0) xp += spacing * 256;
    const int sx = (xp >> 8) % spacing;
    const int sy = (yp >> 8) % spacing;

    std::vector<lumi::modules::SuperscopePoint> pts;
    for (int py = sy; py < m_surfaceHeight; py += spacing)
    {
        for (int px = sx; px < m_surfaceWidth; px += spacing)
        {
            lumi::modules::SuperscopePoint p;
            // Pixel-MITTE, damit der Punkt genau dieses eine Pixel trifft.
            // y NEGIERT: avsZeile() erwartet GL-Raum (y+ oben), py ist eine
            // AVS-Zeile von oben — ohne Spiegelung stand das Gitter auf
            // h-1-py und traf bei spacing 8 NIE die Referenzzeilen (die
            // dMean-Metrik sah das nicht, erst das Flaechen-Urteil; S60).
            p.x = (static_cast<float>(px) + 0.5f) /
                      static_cast<float>(m_surfaceWidth) * 2.0f - 1.0f;
            p.y = -((static_cast<float>(py) + 0.5f) /
                        static_cast<float>(m_surfaceHeight) * 2.0f - 1.0f);
            p.r = col.x(); p.g = col.y(); p.b = col.z(); p.a = 1.0f;
            pts.push_back(p);
        }
    }
    // EIN Pixel je Gitterpunkt (`framebuffer[x] = current_color`), nicht zwei —
    // mit Groesse 2 zeichneten wir die vierfache Flaeche.
    drawDots(pts, 1.0f, params.blend);

    rt.dotOffX = static_cast<float>(xp + static_cast<int>(vXMove));
    rt.dotOffY = static_cast<float>(yp + static_cast<int>(vYMove));
}

void MultiEffectVisualizer::runDotPlane(const ChainNode& node, const DotPlaneParams& params)
{
    // r_dotpln.cpp zeilengenau (S48, Tie-Tunnel-DM-Befund): 64x64-Grid
    // scrollt je Frame eine Zeile; die frische Zeile bekommt Hoehe UND Farbe
    // aus dem Spektrum (color_tab[Byte>>2]) und die Farbe WANDERT mit der
    // Zeile mit — die alte Hoehen-Palette faerbte das ganze Feld um. Physik
    // (Velocity - 0.15*h/255), 3D-Matrix (Rotation um y + angle um x,
    // Translation 0/-20/400) und Zeichenreihenfolge wie das Original.
    constexpr int kN = 64;  // NUM_WIDTH — fest: Struktur, kein Parameter
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie. KLASSE A — s. Hinweis bei Moving Particle.
    double vRotVel = params.rotVel, vAngle = params.angle;
    double vCam = params.camDistance, vSettle = params.settle;
    runParamScript(rt, "dotplane", params.initCode, params.frameCode, params.beatCode,
                   {{"rotvel", &vRotVel},
                    {"angle", &vAngle},
                    {"camdistance", &vCam},
                    {"settle", &vSettle}});
    if (!rt.dpSeeded || rt.dpHeights.size() != static_cast<std::size_t>(kN * kN))
    {
        rt.dpHeights.assign(static_cast<std::size_t>(kN * kN), 0.0f);
        rt.dpVel.assign(static_cast<std::size_t>(kN * kN), 0.0f);
        rt.dpColors.assign(static_cast<std::size_t>(kN * kN), 0u);
        rt.dpR = 0.0f;
        rt.dpSeeded = true;
    }

    // initcolortab: 4 Segmente je 16 Schritte, exakte int-Arithmetik.
    uint32_t colorTab[64];
    for (int t = 0; t < 4; ++t)
    {
        const int c0 = static_cast<int>(params.colors[static_cast<std::size_t>(t)]);
        const int c1 =
            static_cast<int>(params.colors[static_cast<std::size_t>(t + 1)]);
        int r = (c0 & 255) << 16;
        int g = ((c0 >> 8) & 255) << 16;
        int b = ((c0 >> 16) & 255) << 16;
        const int dr = (((c1 & 255) - (c0 & 255)) << 16) / 16;
        const int dg = ((((c1 >> 8) & 255) - ((c0 >> 8) & 255)) << 16) / 16;
        const int db = ((((c1 >> 16) & 255) - ((c0 >> 16) & 255)) << 16) / 16;
        for (int x = 0; x < 16; ++x)
        {
            colorTab[t * 16 + x] = static_cast<uint32_t>(
                (r >> 16) | ((g >> 16) << 8) | ((b >> 16) << 16));
            r += dr;
            g += dg;
            b += db;
        }
    }

    // matrix.cpp-Helfer: `avsMat*` im anonymen Namespace (Dot Plane UND Dot
    // Fountain bauen dieselbe Matrix — eine Kopie je Renderer waeren zwei
    // Wahrheiten).
    const auto& mRotate = avsMatRotate;
    const auto& mTranslate = avsMatTranslate;
    const auto& mMultiply = avsMatMultiply;
    const auto& mApply = avsMatApply;

    float matrix[16];
    float matrix2[16];
    mRotate(matrix, 2, rt.dpR);
    mRotate(matrix2, 1, static_cast<float>(vAngle));
    mMultiply(matrix, matrix2);
    mTranslate(matrix2, 0.0f, -20.0f, static_cast<float>(vCam));  // S53: war 400
    mMultiply(matrix, matrix2);

    // Scroll + Physik + Injektion (Spektrum links, 3er-Gruppen-Maximum).
    float btable[kN];
    std::memcpy(btable, rt.dpHeights.data(), sizeof(btable));
    for (int fo = 0; fo < kN; ++fo)
    {
        if (fo == kN - 1)  // Injektionszeile 0: frische Hoehe + Farbe
        {
            const unsigned char* sd = visSpectrum(0);
            const float* i = btable;
            float* o = rt.dpHeights.data();
            float* ov = rt.dpVel.data();
            uint32_t* oc = rt.dpColors.data();
            for (int p = 0; p < kN; ++p)
            {
                int tv = std::max(sd[0], std::max(sd[1], sd[2]));
                *o = static_cast<float>(tv);
                tv >>= 2;
                if (tv > 63) tv = 63;
                *oc++ = colorTab[tv];
                *ov++ = (*o - *i) / 90.0f;
                ++o;
                ++i;
                sd += 3;
            }
        }
        else  // Zeile t -> t+1 kopieren (mit Feder-Physik)
        {
            const int t = (kN - (fo + 2)) * kN;
            const float* i = &rt.dpHeights[static_cast<std::size_t>(t)];
            float* o = &rt.dpHeights[static_cast<std::size_t>(t + kN)];
            const float* v = &rt.dpVel[static_cast<std::size_t>(t)];
            float* ov = &rt.dpVel[static_cast<std::size_t>(t + kN)];
            const uint32_t* c = &rt.dpColors[static_cast<std::size_t>(t)];
            uint32_t* oc = &rt.dpColors[static_cast<std::size_t>(t + kN)];
            for (int p = 0; p < kN; ++p)
            {
                *o = *i++ + *v;
                if (*o < 0.0f) *o = 0.0f;
                *ov++ = *v++ - static_cast<float>(vSettle) * (*o++ / 255.0f);  // war 0,15
                *oc++ = *c++;
            }
        }
    }

    // Projektion + Zeichenreihenfolge (r-abhaengiges Back-to-Front-Flippen).
    const int w = m_surfaceWidth;
    const int h = m_surfaceHeight;
    float adj = static_cast<float>(w) * 440.0f / 640.0f;
    const float adj2 = static_cast<float>(h) * 440.0f / 480.0f;
    if (adj2 < adj) adj = adj2;

    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(static_cast<std::size_t>(kN * kN));
    for (int fo = 0; fo < kN; ++fo)
    {
        const int f =
            (rt.dpR < 90.0f || rt.dpR > 270.0f) ? kN - fo - 1 : fo;
        float dw = 350.0f / static_cast<float>(kN);
        float wpos = -(kN * 0.5f) * dw;
        const float q = (static_cast<float>(f) - kN * 0.5f) * dw;
        const uint32_t* ct = &rt.dpColors[static_cast<std::size_t>(f * kN)];
        const float* at = &rt.dpHeights[static_cast<std::size_t>(f * kN)];
        int da = 1;
        if (rt.dpR < 180.0f)
        {
            da = -1;
            dw = -dw;
            wpos = -wpos + dw;
            ct += kN - 1;
            at += kN - 1;
        }
        for (int p = 0; p < kN; ++p)
        {
            float x;
            float y;
            float z;
            mApply(matrix, wpos, 64.0f - *at, q, x, y, z);
            if (z > 0.0f)
            {
                z = adj / z;
                const int ix = static_cast<int>(x * z) + w / 2;
                const int iy = static_cast<int>(y * z) + h / 2;
                if (iy >= 0 && iy < h && ix >= 0 && ix < w)
                {
                    const QVector3D col = colorToVec(*ct);
                    lumi::modules::SuperscopePoint pt;
                    pt.x = (static_cast<float>(ix) + 0.5f) /
                               static_cast<float>(w) * 2.0f - 1.0f;
                    pt.y = -((static_cast<float>(iy) + 0.5f) /
                                 static_cast<float>(h) * 2.0f - 1.0f);  // AVS y+ unten
                    pt.r = col.x();
                    pt.g = col.y();
                    pt.b = col.z();
                    pt.a = 1.0f;
                    pts.push_back(pt);
                }
            }
            wpos += dw;
            ct += da;
            at += da;
        }
    }
    drawDots(pts, 1.0f, 3);  // 1px-Dots via BLEND_LINE (folgt SRM, Original)

    rt.dpR += static_cast<float>(vRotVel) / 5.0f;
    if (rt.dpR >= 360.0f) rt.dpR -= 360.0f;
    if (rt.dpR < 0.0f) rt.dpR += 360.0f;
}

void MultiEffectVisualizer::runDotFountain(const ChainNode& node,
                                           const DotFountainParams& params)
{
    // r_dotfnt.cpp zeilengenau (S57). Bis dahin waren das 400 freie Partikel
    // mit eigener Physik — der Header sagte es selbst („Simplified particle
    // model here"). Die Referenz ist etwas voellig anderes: ein **30x256-Gitter**
    // (7680 Punkte), eine rotierende HOEHENWAND. Je Frame rutscht jede der 256
    // Alterungsstufen eine weiter nach hinten und bekommt dabei ihre Physik;
    // Stufe 0 wird aus dem Spektrum neu gesetzt. Gezeichnet wird mit derselben
    // 3D-Matrix wie Dot Plane (Drehung, Neigung, `translate(0,-20,400)`).
    //
    // Die Matrix-Zeile `19_dot_fountain` mass trotz voellig anderem Bild 0,002:
    // beide Seiten sind ueberwiegend schwarz, und die Metrik luegt bei duennen
    // Inhalten (Befund S53).
    constexpr int kDiv = 30;     // NUM_ROT_DIV — Speichen
    constexpr int kHeight = 256; // NUM_ROT_HEIGHT — Alterungsstufen
    constexpr int kCount = kDiv * kHeight;

    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (static_cast<int>(rt.fountain.size()) != kCount)
        rt.fountain.assign(kCount, FountainP{});

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vRotVel = params.rotVel, vAngle = params.angle;
    runParamScript(rt, "dotfountain", params.initCode, params.frameCode,
                   params.beatCode, {{"rotvel", &vRotVel}, {"angle", &vAngle}});

    uint32_t colorTab[64];
    avsInitColorTab(params.colors, colorTab);

    auto at = [&rt](int stufe, int p) -> FountainP& {
        return rt.fountain[static_cast<std::size_t>(stufe) * kDiv + p];
    };

    // Die Stufe 0 VOR dem Schieben — die Referenz sichert sie in `pb` und liest
    // sie beim Erzeugen wieder (`in->dh`).
    FountainP pb[kDiv];
    for (int p = 0; p < kDiv; ++p) pb[p] = at(0, p);

    // Altern: Stufe fo wandert nach fo+1 und bekommt dabei ihre Physik
    // (r_dotfnt.cpp:166-181). Rueckwaerts, damit nichts ueberschrieben wird.
    for (int fo = kHeight - 2; fo >= 0; --fo)
    {
        const float booga = 1.3f / static_cast<float>(fo + 100);
        for (int p = 0; p < kDiv; ++p)
        {
            FountainP q = at(fo, p);
            q.r += q.dr;
            q.dh += 0.05f;
            q.dr += booga;
            q.h += q.dh;
            at(fo + 1, p) = q;
        }
    }

    // Neue Stufe 0 aus der WELLENFORM (`visdata[1][0]`, also Kanal links) —
    // Bytes ueber die SSOT-Accessoren, kein zweites Layout-Wissen.
    const unsigned char* wave = visWaveform(0);
    for (int p = 0; p < kDiv; ++p)
    {
        FountainP& out = at(0, p);
        int t = static_cast<int>(wave[p]) ^ 128;
        t = t * 5 / 4 - 64;
        if (m_frameBeat) t += 128;
        if (t > 255) t = 255;

        float dr = static_cast<float>(t) / 200.0f;
        if (dr < 0.0f) dr = -dr;
        dr += 1.0f;
        // `(out->dh - in->dh)` ist hier IMMER 0: `in` ist die Kopie von genau
        // dieser Stufe, und das Schieben oben laesst Stufe 0 unangetastet. Der
        // Term steht trotzdem so im Original — nachgebildet, nicht weggekuerzt.
        out.dh = -dr * (100.0f + (out.dh - pb[p].dh)) / 100.0f * 2.8f;
        out.r = 1.0f;
        out.h = 250.0f;
        int ci = t / 4;
        if (ci > 63) ci = 63;
        if (ci < 0) ci = 0;
        out.c = colorTab[ci];
        const float a = static_cast<float>(p) * 3.14159f * 2.0f /
                        static_cast<float>(kDiv);
        out.ax = std::sin(a);
        out.ay = std::cos(a);
        out.dr = 0.0f;
    }

    // 3D-Matrix wie Dot Plane: Drehung um die Hochachse, Neigung, Translation.
    float matrix[16];
    float matrix2[16];
    avsMatRotate(matrix, 2, rt.dotRot);
    avsMatRotate(matrix2, 1, static_cast<float>(vAngle));
    avsMatMultiply(matrix, matrix2);
    avsMatTranslate(matrix2, 0.0f, -20.0f, 400.0f);
    avsMatMultiply(matrix, matrix2);

    const float w = static_cast<float>(m_surfaceWidth);
    const float h = static_cast<float>(m_surfaceHeight);
    float adj = w * 440.0f / 640.0f;
    const float adj2 = h * 440.0f / 480.0f;
    if (adj2 < adj) adj = adj2;

    std::vector<lumi::modules::SuperscopePoint> pts;
    pts.reserve(static_cast<std::size_t>(kCount));
    for (int fo = 0; fo < kHeight; ++fo)
    {
        for (int p = 0; p < kDiv; ++p)
        {
            const FountainP& q = at(fo, p);
            float x, y, z;
            avsMatApply(matrix, q.ax * q.r, q.h, q.ay * q.r, x, y, z);
            if (z <= 0.0000001f) continue;
            z = adj / z;
            const int ix = static_cast<int>(x * z) + m_surfaceWidth / 2;
            const int iy = static_cast<int>(y * z) + m_surfaceHeight / 2;
            if (ix < 0 || ix >= m_surfaceWidth || iy < 0 || iy >= m_surfaceHeight)
                continue;
            lumi::modules::SuperscopePoint sp;
            // Pixel-MITTE; AVS zaehlt y nach unten, GL nach oben.
            sp.x = (static_cast<float>(ix) + 0.5f) / w * 2.0f - 1.0f;
            sp.y = 1.0f - (static_cast<float>(iy) + 0.5f) / h * 2.0f;
            // Dieselbe Kanalzuordnung wie Dot Plane (`colorToVec`) — die
            // Farbtabelle liegt in 0x00RRGGBB vor, nicht in AVS-Reihenfolge.
            const QVector3D col = colorToVec(q.c);
            sp.r = col.x();
            sp.g = col.y();
            sp.b = col.z();
            sp.a = 1.0f;
            pts.push_back(sp);
        }
    }
    drawDots(pts, 1.0f);   // EIN Pixel je Punkt (BLEND_LINE auf einem Pixel)

    rt.dotRot += static_cast<float>(vRotVel) / 5.0f;
    if (rt.dotRot >= 360.0f) rt.dotRot -= 360.0f;
    if (rt.dotRot < 0.0f) rt.dotRot += 360.0f;
}

void MultiEffectVisualizer::runChannelShift(const ChainNode& node,
                                            const ChannelShiftParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vMode = params.mode;
    runParamScript(rt, "channelshift", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"mode", &vMode}});

    int mode = std::clamp(static_cast<int>(vMode), 0, 5);  // Strang D
    if (params.onBeat)
    {
        // r_chanshift:124-126 zieht NUR auf dem Beat; bis zum ersten Beat gilt
        // die Vorgabe aus dem Preset. Wir zogen zusaetzlich im ersten Frame
        // (`apeChanMode < 0`) — ein Zug zuviel aus dem geteilten Strom, damit
        // war jede weitere Permutation um eine Ziehung versetzt (Befund S58,
        // Alternate Reality: Referenz BGR, wir BRG).
        // Der Preset-Wert ist der STARTWERT eines selbstlaufenden Zustands:
        // gegen ihn vergleichen, nicht gegen die Frame-Kopie — sonst setzt ein
        // Skript den Zustand in jedem Frame zurueck (Merkregel S57,
        // `interferences.rotation`).
        const int seed = std::clamp(params.mode, 0, 5);
        if (rt.apeChanSeed != seed)
        {
            rt.apeChanSeed = seed;
            rt.apeChanMode = seed;
        }
        if (m_frameBeat)
            rt.apeChanMode = m_scriptContext->nextRand() % 6;  // r_chanshift:125
        mode = rt.apeChanMode;
    }
    m_apeShader->bind();
    m_apeShader->setUniformValue("uType", 0);
    m_apeShader->setUniformValue("uMode", mode);
    m_apeShader->release();
    transformPass(*m_apeShader);
}

void MultiEffectVisualizer::runColorReduction(const ChainNode& node,
                                              const ColorReductionParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vLevels = params.levels;
    runParamScript(rt, "colorreduction", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"levels", &vLevels}});

    const int levels = std::clamp(static_cast<int>(vLevels), 1, 8);  // Strang D
    m_apeShader->bind();
    m_apeShader->setUniformValue("uType", 1);
    m_apeShader->setUniformValue("uLevels", static_cast<float>(1 << levels));
    m_apeShader->release();
    transformPass(*m_apeShader);
}

void MultiEffectVisualizer::runMultiplier(const ChainNode& node,
                                          const MultiplierParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vMode = params.mode;
    runParamScript(rt, "multiplier", params.initCode, params.frameCode,
                   params.beatCode,
                   {{"mode", &vMode}});

    m_apeShader->bind();
    m_apeShader->setUniformValue("uType", 2);
    m_apeShader->setUniformValue("uMode",
                                 std::clamp(static_cast<int>(vMode), 0, 7));  // Strang D
    m_apeShader->release();
    transformPass(*m_apeShader);
}

void MultiEffectVisualizer::runVideoDelay(const ChainNode& node,
                                          const VideoDelayParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];
    if (!QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()) return;

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vDelay = params.delay;
    runParamScript(rt, "videodelay", params.initCode, params.frameCode,
                   params.beatCode, {{"delay", &vDelay}});

    auto* extra = QOpenGLContext::currentContext()->extraFunctions();

    // ~30 frames/beat is a rough conversion (no per-node BPM here); capped for VRAM.
    const int delay =
        std::clamp(static_cast<int>(params.useBeats ? vDelay * 30 : vDelay),
                   1, 128);
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

void MultiEffectVisualizer::runMultiDelay(const ChainNode& node,
                                          const MultiDelayParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

    // Strang D auf einer Frame-Kopie (s. runParamScript).
    double vDelay = params.delay, vBuffer = params.buffer;
    runParamScript(rt, "multidelay", params.initCode, params.frameCode,
                   params.beatCode, {{"delay", &vDelay}, {"buffer", &vBuffer}});

    if (params.mode == 0) return;  // inactive
    if (!QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()) return;
    const int b = std::clamp(static_cast<int>(vBuffer), 0, 5);
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
        std::clamp(static_cast<int>(params.useBeats ? vDelay * 30 : vDelay),
                   1, 128);
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
        // Die Verzoegerung gehoert dem PUFFER, nicht dem Knoten (Entscheid
        // Patrik S55, nach dem Original): `delay[6]`/`usebeats[6]` sind in
        // r_multidelay.cpp GLOBALE Felder, ein Knoten speichert nur `mode` und
        // `activebuffer`, und der Ausgabe-Knoten liest schlicht `outpos[buffer]`
        // — den AELTESTEN Frame des Rings. `head` zeigt auf den Platz, der als
        // naechstes ueberschrieben wird, und das ist genau der aelteste.
        //
        // Bis S55 rechnete der Leser mit seinem EIGENEN `delay`. Damit aenderte
        // das `delay` des Schreibers nur die Ringgroesse und blieb im Bild
        // unsichtbar — die Feld-Sonde meldete es zu Recht als stumm. Ungleiche
        // Werte kann es im Original ohnehin nicht geben: JEDER Knoten speichert
        // alle sechs Puffer-Einstellungen und schreibt sie beim Laden in den
        // globalen Zustand (r_multidelay.cpp:387-401).
        blit(ring[static_cast<size_t>(head)]->handle(), cur->handle());
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
    // Bei diesem Knoten ist `blend = 2` die BENANNTE Betriebsart „50/50" — das
    // Gewicht ist hier also fest, anders als beim Fractal Zoomer.
    m_fractal2DShader->setUniformValue("uFeedback", 0.5f);
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
    // Winamp-Vertrag (ref VIS.cpp:719-745, S44/S11): 512er-FFT -> 256 Bins,
    // linear sqrt(re^2+im^2)/16, je Bin ZWEI Ausgabe-Bytes (Positionen 0..511),
    // die LETZTEN 64 der 576 sind nur Abkling-Fuellung (~0). Unsere BASS-FFT1024
    // liefert exakt die doppelte Bin-Aufloesung -> Position p == unser Bin p,
    // Positionen >= 512 sind 0 (vorher: 512 Bins ueber 576 gestreckt = ~12 %
    // gestauchte Frequenzachse + Phantomwerte in den Winamp-Fade-Baendern).
    // Danach die AVS-Log-Kurve (g_logtab, vis_avs main.cpp:242-249). kSpecGain=8
    // trifft Winamps Saettigungspunkt (Magnitude ~0.126 -> Byte 255) fast exakt
    // (S38-Sichtkalibrierung, jetzt gegen VIS.cpp/FFT.cpp hergeleitet).
    constexpr float kSpecGain = 8.0f;
    constexpr int kWinampRealBands = 512;  // danach nur Fade-Fuellung
    auto specByte = [](const std::vector<float>& v, int i) -> unsigned char {
        if (v.empty() || i >= kWinampRealBands) return 0;
        const float s = v[static_cast<size_t>(i) * v.size() / kWinampRealBands];
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
    engine.setScriptTime(m_scriptClock);
    float bass, mid, treble;
    computeAudioBands(getSpectrum(), bass, mid, treble);
    engine.setNumber("bass", bass);
    engine.setNumber("mid", mid);
    engine.setNumber("treb", treble);
    engine.setNumber("treble", treble);  // alias (MilkDrop uses treb)
    engine.setNumber("vol", m_audioLevel);
    engine.setNumber("beat", m_frameBeat ? 1.0 : 0.0);
    // `time` nur injizieren, wenn das Skript den Namen nicht selbst besitzt
    // (AVS-EEL: gewoehnlicher User-Name — ScriptSlotHost::compileAll scannt
    // die Quellen; Befund S59, el-vis_hypno07). Uhrstand VOR dem Frame wie
    // gettime().
    if (engine.timeInjectable()) engine.setNumber("time", m_scriptClock);
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

    // Persistent zoom (fracTime) + rotation (fracRot) advance every frame.
    if (rt.fracTime <= 0.0f) rt.fracTime = 1.0f;
    rt.fracTime *= (zoomSpeed > 1e-4f ? zoomSpeed : 1.0f);
    if (rt.fracTime > 1e12f) rt.fracTime = 1.0f;  // loop the trip
    rt.fracRot += rotSpeed;
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
    m_fractal2DShader->setUniformValue("uRot", rt.fracRot);
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
    // `feedback` ist eine STAERKE, kein Schalter: sie gewichtet, wie viel vom
    // vorigen Bild stehen bleibt (0 = keine Schleife, 1 = Standbild). Bis S57
    // wurde hier nur auf die 50/50-Betriebsart geschaltet, weshalb 0,3 und 1,0
    // dasselbe Bild ergaben. Die Vorgabe 0,5 IST das alte 50/50 — bestehende
    // Presets mit dem Vorgabewert sehen unverändert aus.
    const float trail = std::clamp(params.feedback, 0.0f, 1.0f);
    m_fractal2DShader->setUniformValue("uBlend", trail > 0.0f ? 2 : 0);
    m_fractal2DShader->setUniformValue("uFeedback", trail);
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
