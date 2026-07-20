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

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QString>

#include <algorithm>
#include <cmath>
#include <filesystem>

using namespace lumi::multieffect;
using lumi::scripting::ScriptSlotHost;
using Slot = lumi::scripting::LuaScriptEngine::Slot;

namespace {

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
const char* kMirrorFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform bool uLeftToRight;  // right half mirrors the left
uniform bool uTopToBottom;  // bottom half mirrors the top
out vec4 fragColor;
void main()
{
    vec2 uv = vTex;
    if (uLeftToRight && uv.x > 0.5) uv.x = 1.0 - uv.x;
    if (uTopToBottom && uv.y < 0.5) uv.y = 1.0 - uv.y;
    fragColor = vec4(texture(uTex, uv).rgb, 1.0);
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
out vec2 vTex;
void main()
{
    vTex = aTex;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* kWarpFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform bool uWrap;
out vec4 fragColor;
void main()
{
    vec2 uv = uWrap ? fract(vTex) : clamp(vTex, 0.0, 1.0);
    fragColor = vec4(texture(uTex, uv).rgb, 1.0);
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
    // The new tree reuses node ids 1..N, colliding with the old runtimes; flag
    // a reset so the render thread frees their GL objects and starts fresh.
    m_pendingRuntimeReset = true;
    setChain(std::move(translated.root));
    return parsed.ok;
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
    if (m_fadeShader == nullptr || m_invertShader == nullptr ||
        m_barsShader == nullptr || m_blendShader == nullptr ||
        m_brightShader == nullptr || m_blurShader == nullptr ||
        m_mirrorShader == nullptr || m_colorfadeShader == nullptr ||
        m_lutShader == nullptr || m_warpShader == nullptr ||
        m_feedbackShader == nullptr || m_mosaicShader == nullptr ||
        m_grainShader == nullptr || m_scatterShader == nullptr ||
        m_interfShader == nullptr)
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

    // Dynamic mesh for the grid-warp effects (pos.xy + tex.xy, re-uploaded/frame).
    m_warpVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_warpVao->create();
    m_warpVao->bind();
    m_warpVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_warpVbo->create();
    m_warpVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_warpVbo->bind();
    m_warpVbo->allocate(nullptr, 0);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                             reinterpret_cast<void*>(2 * sizeof(float)));
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
        }
    }
    m_listRuntimes.clear();  // slot hosts / FBOs die with their GL-frame owner
    m_leafRuntimes.clear();
    m_bufferPool.clear();
}

void MultiEffectVisualizer::destroySurfaces()
{
    m_rootSurface.destroy();
    for (auto& [id, runtime] : m_listRuntimes)
    {
        runtime.surface.destroy();
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

    // Waveform RMS: smoothed level for DebugBars + onset energy for the beat.
    const std::vector<float> waveform = getWaveform();
    float rms = 0.0f;
    if (!waveform.empty())
    {
        float sum = 0.0f;
        for (float sample : waveform) sum += sample * sample;
        rms = std::sqrt(sum / static_cast<float>(waveform.size()));
        m_audioLevel += (rms - m_audioLevel) * 0.3f;
    }

    // Chain-scoped beat: onset feeds the estimator every frame (kept warm);
    // list scripts and later Custom BPM may mutate m_frameBeat mid-chain.
    const bool onset = m_beat.updateAdaptive(rms);
    m_beatEstimator.refine(onset, lumi::modules::BeatEstimator::steadyNowMs());
    m_frameBeat = onset;

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

    if (blendWasEnabled == GL_TRUE) f->glEnable(GL_BLEND);
    m_firstFrame = false;
}

void MultiEffectVisualizer::renderNode(const ChainNode& node)
{
    if (!node.enabled) return;

    struct Visitor
    {
        MultiEffectVisualizer& self;
        const ChainNode& node;

        void operator()(const ListParams& params) const { self.renderList(node, params); }
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
        void operator()(const BlitterFeedbackParams& params) const { self.runBlitterFeedback(params); }
        void operator()(const RotoBlitterParams& params) const { self.runRotoBlitter(node, params); }
        void operator()(const BufferSaveParams& params) const { self.runBufferSave(params); }
        void operator()(const CustomBpmParams& params) const { self.runCustomBpm(node, params); }
        void operator()(const SuperScopeParams& params) const { self.runSuperScope(node, params); }
        void operator()(const MosaicParams& params) const { self.runMosaic(node, params); }
        void operator()(const GrainParams& params) const { self.runGrain(params); }
        void operator()(const ScatterParams& params) const { self.runScatter(params); }
        void operator()(const InterferencesParams& params) const { self.runInterferences(node, params); }
        void operator()(const DebugBarsParams& params) const { self.runDebugBars(params); }
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
                "effectlist", m_scriptContext, ScriptSlotHost::Dialect::Avs);
            runtime.slotHost->setSource(Slot::Init, params.initCode);
            runtime.slotHost->setSource(Slot::Frame, params.frameCode);
            runtime.slotHost->compileAll();
            runtime.compiledInit = params.initCode;
            runtime.compiledFrame = params.frameCode;
            auto& engine = runtime.slotHost->engine();
            engine.setNumber("enabled", 1.0);
            engine.setNumber("clear", 0.0);
            engine.setNumber("alphain", static_cast<double>(alphaIn));
            engine.setNumber("alphaout", static_cast<double>(alphaOut));
            runtime.slotHost->run(Slot::Init);
        }
        if (runtime.slotHost->has(Slot::Frame))
        {
            auto& engine = runtime.slotHost->engine();
            engine.setNumber("beat", m_frameBeat ? 1.0 : 0.0);
            engine.setNumber("w", static_cast<double>(m_surfaceWidth));
            engine.setNumber("h", static_cast<double>(m_surfaceHeight));
            runtime.slotHost->run(Slot::Frame);
            scriptEnabled = engine.number("enabled") != 0.0;
            scriptClear = engine.number("clear") != 0.0;
            m_frameBeat = engine.number("beat") != 0.0;  // beat is mutable (§5.1)
            alphaIn = static_cast<int>(engine.number("alphain"));
            alphaOut = static_cast<int>(engine.number("alphaout"));
        }
    }
    if (!scriptEnabled) return;

    // --- OnBeat activation window
    if (params.onBeatRender)
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
// Leaves
// =============================================================================

void MultiEffectVisualizer::runClear(const ClearParams& params)
{
    if (params.onlyFirst && !m_firstFrame) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    const QVector3D color = colorToVec(params.color);
    f->glClearColor(color.x(), color.y(), color.z(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);
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
    bool left = params.leftToRight;
    bool top = params.topToBottom;

    if (params.onBeatRandom)
    {
        // On beat pick a random subset of the enabled axes (r_mirror.cpp:146).
        LeafRuntime& rt = m_leafRuntimes[node.nodeId];
        if (m_frameBeat)
        {
            const uint32_t bits = nextRandom();
            rt.mirrorV = params.leftToRight && (bits & 1u);
            rt.mirrorH = params.topToBottom && (bits & 2u);
        }
        left = rt.mirrorV;
        top = rt.mirrorH;
    }

    if (!left && !top) return;  // nothing to mirror this frame
    m_mirrorShader->bind();
    m_mirrorShader->setUniformValue("uLeftToRight", left);
    m_mirrorShader->setUniformValue("uTopToBottom", top);
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
        rt.lut = std::make_unique<lumi::modules::ScriptLutModule>(m_scriptContext);
        rt.lut->setInitCode(params.initCode);
        rt.lut->setFrameCode(params.frameCode);
        rt.lut->setBeatCode(params.beatCode);
        rt.lut->setLevelCode(params.levelCode);
        rt.lutCompiled = combined;
    }
    rt.lut->setRecompute(params.recompute);
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

    constexpr int kXres = 32, kYres = 24;  // AVS Movement default working grid
    const std::string combined = "M:" + params.code;
    if (rt.grid == nullptr || rt.gridCompiled != combined)
    {
        rt.grid = std::make_unique<lumi::modules::ScriptGridModule>(m_scriptContext);
        rt.grid->setPointCode(params.code);
        rt.grid->setGridSize(kXres, kYres);
        rt.gridCompiled = combined;
    }
    rt.grid->setRectCoords(params.rectCoords);
    applyGridWarp(rt, kXres, kYres, params.wrap);
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
        rt.grid = std::make_unique<lumi::modules::ScriptGridModule>(m_scriptContext);
        rt.grid->setInitCode(params.initCode);
        rt.grid->setFrameCode(params.frameCode);
        rt.grid->setBeatCode(params.beatCode);
        rt.grid->setPointCode(params.pointCode);
        rt.grid->setGridSize(params.xres, params.yres);
        rt.gridCompiled = combined;
    }
    rt.grid->setRectCoords(params.rectCoords);
    applyGridWarp(rt, params.xres, params.yres, params.wrap);
}

void MultiEffectVisualizer::applyGridWarp(LeafRuntime& rt, int xres, int yres,
                                          bool wrap)
{
    if (rt.grid == nullptr || xres < 2 || yres < 2) return;

    rt.grid->execute(static_cast<float>(m_surfaceWidth),
                     static_cast<float>(m_surfaceHeight), m_frameBeat, m_deltaTime);
    const auto& field = rt.grid->field();
    if (static_cast<int>(field.size()) < xres * yres) return;

    // Build the triangulated grid mesh: vertex at grid NDC, texcoord = (u,v).
    m_warpVertices.clear();
    m_warpVertices.reserve(static_cast<size_t>(xres - 1) * (yres - 1) * 6 * 4);
    auto pushVertex = [&](int gx, int gy) {  // NB: not "emit" — Qt reserves it
        const float px = -1.0f + 2.0f * static_cast<float>(gx) / (xres - 1);
        const float py = -1.0f + 2.0f * static_cast<float>(gy) / (yres - 1);
        const auto& n = field[static_cast<size_t>(gy) * xres + gx];
        m_warpVertices.push_back(px);
        m_warpVertices.push_back(py);
        m_warpVertices.push_back(n.u * 0.5f + 0.5f);
        m_warpVertices.push_back(n.v * 0.5f + 0.5f);
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

    m_warpShader->bind();
    m_warpShader->setUniformValue("uWrap", wrap);
    m_warpVao->bind();
    m_warpVbo->bind();
    m_warpVbo->allocate(m_warpVertices.data(),
                        static_cast<int>(m_warpVertices.size() * sizeof(float)));
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, pair.current()->texture());
    m_warpShader->setUniformValue("uTex", 0);
    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_warpVertices.size() / 4));
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

void MultiEffectVisualizer::runBufferSave(const BufferSaveParams& params)
{
    if (params.save)
    {
        // Copy the current working buffer into global buffer `slot`.
        QOpenGLFramebufferObject* pool =
            m_bufferPool.get(params.slot, m_surfaceWidth, m_surfaceHeight, true);
        if (pool == nullptr) return;
        if (QOpenGLFramebufferObject::hasOpenGLFramebufferBlit())
        {
            auto* extra = QOpenGLContext::currentContext()->extraFunctions();
            extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, active().current()->handle());
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
            m_bufferPool.get(params.slot, m_surfaceWidth, m_surfaceHeight, false);
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

void MultiEffectVisualizer::runSuperScope(const ChainNode& node,
                                          const SuperScopeParams& params)
{
    LeafRuntime& rt = m_leafRuntimes[node.nodeId];

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
    rt.scope->setLineWidth(params.lineWidth);
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

    const std::vector<lumi::modules::SuperscopePoint> points = rt.scope->execute(
        w, w, s, s, sampleCount, m_surfaceWidth, m_surfaceHeight, m_frameBeat,
        m_deltaTime);

    // Draw additively onto the current working buffer (AVS scope default).
    if (!m_scopeRenderer.ready()) return;
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    lumi::render::ScopeRenderer::Params rp;
    rp.mode = static_cast<lumi::modules::SuperscopeRenderMode>(params.renderMode);
    rp.lineWidth = params.lineWidth;
    rp.dotSize = params.dotSize;
    rp.glowEnabled = false;
    m_scopeRenderer.draw(points, rp);
    f->glDisable(GL_BLEND);
}

uint32_t MultiEffectVisualizer::nextRandom()
{
    // Numerical Recipes LCG — host-local, deterministic (no global rand()).
    m_rng = m_rng * 1664525u + 1013904223u;
    return m_rng;
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
        m_bufferPool.get(n, m_surfaceWidth, m_surfaceHeight, false);
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
