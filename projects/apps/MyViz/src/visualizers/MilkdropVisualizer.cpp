/**
 ****************************************************************************************
 * @file   MilkdropVisualizer.cpp
 * @brief  MD1 MilkDrop render core (Import-Phase Roadmap 6, M3)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * All render math is a 1:1 port of the MilkDrop3 reference (BSD, Nullsoft):
 * warp mesh ComputeGridAlphaValues (milkdropfs.cpp:1681-1857), basic waveform
 * DrawWave (:2682-3298) incl. SmoothWave (:2466), MD1 composite
 * ShowToUser_NoShaders (:3967-4287), borders/darken-center DrawSprites
 * (:3300-3405). Deliberate deviations are marked "PORT:" inline.
 *
 * Y convention: ONE internal math space (reference formulas verbatim, no
 * per-draw flips) — the single vertical flip happens in the composite pass.
 ****************************************************************************************
 */

#include "visualizers/MilkdropVisualizer.hpp"

#include "visualizers/milkdrop/MilkdropBlur.hpp"
#include "visualizers/milkdrop/MilkdropSerializer.hpp"

#include <HlslTranspiler.hpp>

#include <QFileInfo>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include <algorithm>
#include <cmath>
#include <filesystem>

using lumi::modules::ModuleParamDesc;
using lumi::modules::ParamType;
using lumi::modules::ParamValue;
using lumi::modules::PipelineStage;
using lumi::modules::SuperscopePoint;
using lumi::modules::SuperscopeRenderMode;
using lumi::scripting::ScriptContext;
using lumi::scripting::ScriptSlotHost;
using Slot = lumi::scripting::LuaScriptEngine::Slot;

namespace
{

// -----------------------------------------------------------------------------------------
// Shaders (GLSL 330 core, matching the project's other visualizers)
// -----------------------------------------------------------------------------------------

const char* kTexVertexShader = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
out vec2 vTex;
void main() { vTex = aTex; gl_Position = vec4(aPos, 0.0, 1.0); }
)";

// warp pass: previous frame sampled at the mesh UVs, dimmed by decay; uDecaySub
// is the file-default warp shader's `ret -= k` (baked, usually 0)
const char* kWarpFragmentShader = R"(#version 330 core
uniform sampler2D uTex;
uniform float uDecay;
uniform float uDecaySub;
in vec2 vTex;
out vec4 frag;
void main() { frag = vec4(texture(uTex, vTex).rgb * uDecay - vec3(uDecaySub), 1.0); }
)";

// blur pass 1: long horizontal kernel + progressive range compression
// (GLSL port of blur1_ps.fx; constants come from MilkdropBlur.hpp)
const char* kBlurHFragmentShader = R"(#version 330 core
uniform sampler2D uTex;
uniform vec4 uTexSize;    // w, h, 1/w, 1/h of the SOURCE
uniform vec4 uW;          // w1..w4
uniform vec4 uD;          // d1..d4
uniform vec4 uScaleBias;  // fscale, fbias, wDiv, 0
in vec2 vTex;
out vec4 frag;
void main()
{
    vec2 uv2 = vTex + uTexSize.zw;      // + moves blur up/left by 1 px (reference)
    vec3 blur =
        (texture(uTex, uv2 + vec2( uD.x * uTexSize.z, 0.0)).rgb
       + texture(uTex, uv2 + vec2(-uD.x * uTexSize.z, 0.0)).rgb) * uW.x +
        (texture(uTex, uv2 + vec2( uD.y * uTexSize.z, 0.0)).rgb
       + texture(uTex, uv2 + vec2(-uD.y * uTexSize.z, 0.0)).rgb) * uW.y +
        (texture(uTex, uv2 + vec2( uD.z * uTexSize.z, 0.0)).rgb
       + texture(uTex, uv2 + vec2(-uD.z * uTexSize.z, 0.0)).rgb) * uW.z +
        (texture(uTex, uv2 + vec2( uD.w * uTexSize.z, 0.0)).rgb
       + texture(uTex, uv2 + vec2(-uD.w * uTexSize.z, 0.0)).rgb) * uW.w;
    blur *= uScaleBias.z;
    blur = blur * uScaleBias.x + vec3(uScaleBias.y);
    frag = vec4(blur, 1.0);
}
)";

// blur pass 2: short vertical kernel + edge darkening (blur2_ps.fx port)
const char* kBlurVFragmentShader = R"(#version 330 core
uniform sampler2D uTex;
uniform vec4 uTexSize;    // w, h, 1/w, 1/h of the SOURCE
uniform vec4 uWD;         // w1, w2, d1, d2
uniform vec4 uEdge;       // wDiv, edge_c1, edge_c2, edge_c3
in vec2 vTex;
out vec4 frag;
void main()
{
    vec2 uv2 = vTex + vec2(uTexSize.z, 0.0);
    vec3 blur =
        (texture(uTex, uv2 + vec2(0.0,  uWD.z * uTexSize.w)).rgb
       + texture(uTex, uv2 + vec2(0.0, -uWD.z * uTexSize.w)).rgb) * uWD.x +
        (texture(uTex, uv2 + vec2(0.0,  uWD.w * uTexSize.w)).rgb
       + texture(uTex, uv2 + vec2(0.0, -uWD.w * uTexSize.w)).rgb) * uWD.y;
    blur *= uEdge.x;
    float t = min(min(vTex.x, vTex.y), 1.0 - max(vTex.x, vTex.y));
    t = sqrt(t);
    t = uEdge.y + uEdge.z * clamp(t * uEdge.w, 0.0, 1.0);
    frag = vec4(blur * t, 1.0);
}
)";

// composite blur term: coeff*GetBlurN(uv) = tex*(coeff*(max-min)) + coeff*min,
// drawn additively over the base layers (stage B blur mixes)
const char* kBlurLayerFragmentShader = R"(#version 330 core
uniform sampler2D uTex;
uniform float uScale;
uniform float uBias;
in vec2 vTex;
out vec4 frag;
void main() { frag = vec4(texture(uTex, vTex).rgb * uScale + vec3(uBias), 1.0); }
)";

// vertex shader of the transpiled custom shaders (C1): uv from the mesh/quad,
// uv_orig from the (undistorted) position for the warp pass
const char* kCustomVertexShader = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
out vec2 vTex;
out vec2 vUvOrig;
uniform bool uOrigFromPos;
void main()
{
    vTex = aTex;
    vUvOrig = uOrigFromPos ? (aPos * 0.5 + 0.5) : aTex;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// include.fx-Gegenstueck (C1): every uniform/macro the transpiled preset code
// may reference. Inactive sampler uniforms are optimised away by the linker,
// so declaring the full set costs nothing.
[[nodiscard]] std::string milkCustomPreamble()
{
    std::string s = R"(#version 330 core
in vec2 vTex;
in vec2 vUvOrig;
out vec4 fragOut;
uniform sampler2D sampler_main;
uniform sampler2D sampler_fc_main;
uniform sampler2D sampler_pc_main;
uniform sampler2D sampler_fw_main;
uniform sampler2D sampler_pw_main;
uniform sampler2D sampler_blur1;
uniform sampler2D sampler_blur2;
uniform sampler2D sampler_blur3;
uniform vec4 texsize;
uniform vec4 aspect;
uniform float time;
uniform float fps;
uniform float frame;
uniform float progress;
uniform float bass;
uniform float mid;
uniform float treb;
uniform float vol;
uniform float bass_att;
uniform float mid_att;
uniform float treb_att;
uniform float vol_att;
uniform vec4 rand_preset;
uniform vec4 rand_frame;
uniform vec4 roam_cos;
uniform vec4 roam_sin;
uniform vec4 slow_roam_cos;
uniform vec4 slow_roam_sin;
uniform float blur1_min;
uniform float blur1_max;
uniform float blur2_min;
uniform float blur2_max;
uniform float blur3_min;
uniform float blur3_max;
uniform vec4 _blur_scale;
uniform vec2 _blur_sb3;
uniform vec3 hue_shader;
vec3 GetMain(vec2 p) { return texture(sampler_main, p).rgb; }
vec3 GetPixel(vec2 p) { return texture(sampler_main, p).rgb; }
vec3 GetBlur1(vec2 p) { return texture(sampler_blur1, p).rgb * _blur_scale.x + _blur_scale.y; }
vec3 GetBlur2(vec2 p) { return texture(sampler_blur2, p).rgb * _blur_scale.z + _blur_scale.w; }
vec3 GetBlur3(vec2 p) { return texture(sampler_blur3, p).rgb * _blur_sb3.x + _blur_sb3.y; }
float lum(vec3 v) { return dot(v, vec3(0.32, 0.49, 0.29)); }
)";
    // q1..q32 + noise samplers (base + fc/pc/fw/pw prefixes) + texsize_noise_*
    for (int i = 1; i <= 32; ++i) s += "uniform float q" + std::to_string(i) + ";\n";
    for (const char* base : {"noise_lq", "noise_lq_lite", "noise_mq", "noise_hq"})
    {
        s += std::string("uniform sampler2D sampler_") + base + ";\n";
        for (const char* prefix : {"fc_", "pc_", "fw_", "pw_"})
            s += std::string("uniform sampler2D sampler_") + prefix + base + ";\n";
        s += std::string("uniform vec4 texsize_") + base + ";\n";
    }
    return s;
}

/// Sampler names the preamble already declares (user re-declarations skip these)
[[nodiscard]] bool preambleDeclares(const std::string& name)
{
    if (name.rfind("sampler_", 0) == 0)
    {
        std::string base = name.substr(8);
        for (const char* prefix : {"fc_", "pc_", "fw_", "pw_"})
        {
            if (base.rfind(prefix, 0) == 0)
            {
                base = base.substr(3);
                break;
            }
        }
        return base == "main" || base == "blur1" || base == "blur2" || base == "blur3" ||
               base == "noise_lq" || base == "noise_lq_lite" || base == "noise_mq" ||
               base == "noise_hq";
    }
    if (name.rfind("texsize_", 0) == 0)
    {
        const std::string base = name.substr(8);
        return base == "noise_lq" || base == "noise_lq_lite" || base == "noise_mq" ||
               base == "noise_hq";
    }
    return false;
}

/// Full fragment source from a transpile result (preamble + globals + main)
[[nodiscard]] std::string assembleCustomFragment(const lumi::hlsl::HlslResult& r)
{
    std::string s = milkCustomPreamble();
    for (const std::string& name : r.customSamplers)
    {
        if (!preambleDeclares(name)) s += "uniform sampler2D " + name + ";\n";
    }
    for (const std::string& name : r.customTexsizes)
    {
        if (!preambleDeclares(name)) s += "uniform vec4 " + name + ";\n";
    }
    s += r.glslGlobals;
    s += R"(void main()
{
    vec2 uv = vTex;
    vec2 uv_orig = vUvOrig;
    float rad = length((uv_orig - 0.5) * aspect.xy);
    float ang = atan((uv_orig.y - 0.5) * aspect.y, (uv_orig.x - 0.5) * aspect.x);
    vec3 ret = vec3(0.0);
)";
    s += r.glslBody;
    s += "    fragOut = vec4(ret, 1.0);\n}\n";
    return s;
}

// composite pass: texture x uniform colour (gamma/echo passes drive uColor)
const char* kTexFragmentShader = R"(#version 330 core
uniform sampler2D uTex;
uniform vec4 uColor;
in vec2 vTex;
out vec4 frag;
void main() { frag = vec4(texture(uTex, vTex).rgb * uColor.rgb, uColor.a); }
)";

// textured custom shapes: pos + uv + per-vertex colour (centre/edge gradient)
const char* kShapeVertexShader = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;
out vec2 vTex;
out vec4 vColor;
void main() { vTex = aTex; vColor = aColor; gl_Position = vec4(aPos, 0.0, 1.0); }
)";

const char* kShapeFragmentShader = R"(#version 330 core
uniform sampler2D uTex;
in vec2 vTex;
in vec4 vColor;
out vec4 frag;
void main() { frag = vec4(texture(uTex, vTex).rgb * vColor.rgb, vColor.a); }
)";

// untextured geometry: borders, darken-center, post filters
const char* kColorVertexShader = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
out vec4 vColor;
void main() { vColor = aColor; gl_Position = vec4(aPos, 0.0, 1.0); }
)";

const char* kColorFragmentShader = R"(#version 330 core
in vec4 vColor;
out vec4 frag;
void main() { frag = vColor; }
)";

[[nodiscard]] double clampd(double v, double lo, double hi)
{
    return std::max(lo, std::min(hi, v));
}

/// texture-size dependent alpha buckets used by wave modes 2/3/5 (DrawWave)
[[nodiscard]] double sizeBucketAlpha(int width, double a256, double a512, double a1024,
                                     double a2048)
{
    if (width < 384) return a256;
    if (width < 768) return a512;
    if (width < 1536) return a1024;
    return a2048;
}

/// SmoothWave (milkdropfs.cpp:2466-2494): doubles the vertex count with a
/// 4-tap smoothed in-between point; segments (nBreak) are smoothed separately.
void smoothWaveSegment(const std::vector<SuperscopePoint>& in, int start, int count,
                       std::vector<SuperscopePoint>& out)
{
    constexpr float c1 = -0.15f;
    constexpr float c2 = 1.15f;
    constexpr float c3 = 1.15f;
    constexpr float c4 = -0.15f;
    constexpr float invSum = 1.0f / (c1 + c2 + c3 + c4);

    for (int i = 0; i < count; ++i)
    {
        const int iBelow = start + std::max(0, i - 1);
        const int iAbove = start + std::min(count - 1, i + 1);
        const int iAbove2 = start + std::min(count - 1, i + 2);

        out.push_back(in[static_cast<std::size_t>(start + i)]);
        SuperscopePoint mid = in[static_cast<std::size_t>(start + i)];
        mid.x = (c1 * in[static_cast<std::size_t>(iBelow)].x +
                 c2 * in[static_cast<std::size_t>(start + i)].x +
                 c3 * in[static_cast<std::size_t>(iAbove)].x +
                 c4 * in[static_cast<std::size_t>(iAbove2)].x) *
                invSum;
        mid.y = (c1 * in[static_cast<std::size_t>(iBelow)].y +
                 c2 * in[static_cast<std::size_t>(start + i)].y +
                 c3 * in[static_cast<std::size_t>(iAbove)].y +
                 c4 * in[static_cast<std::size_t>(iAbove2)].y) *
                invSum;
        mid.skip = false;
        out.push_back(mid);
    }
}

/// Liang-Barsky style clip of a segment endpoint pair against |x|,|y| <= 1.1
/// (DrawWave modes 6/7/8 clip their line ends, milkdropfs.cpp:3062-3112)
void clipEdgeToRect(float& x0, float& y0, float& x1, float& y1)
{
    constexpr float kLim = 1.1f;
    float t0 = 0.0f;
    float t1 = 1.0f;
    const float dx = x1 - x0;
    const float dy = y1 - y0;
    const float p[4] = {-dx, dx, -dy, dy};
    const float q[4] = {x0 + kLim, kLim - x0, y0 + kLim, kLim - y0};
    for (int i = 0; i < 4; ++i)
    {
        if (std::fabs(p[i]) < 1e-12f)
        {
            if (q[i] < 0.0f) return;  // fully outside, keep as-is (degenerate)
            continue;
        }
        const float r = q[i] / p[i];
        if (p[i] < 0.0f)
        {
            t0 = std::max(t0, r);
        }
        else
        {
            t1 = std::min(t1, r);
        }
    }
    if (t0 > t1) return;
    const float nx0 = x0 + dx * t0;
    const float ny0 = y0 + dy * t0;
    x1 = x0 + dx * t1;
    y1 = y0 + dy * t1;
    x0 = nx0;
    y0 = ny0;
}

} // namespace

// =============================================================================================
// Construction / preset loading
// =============================================================================================

MilkdropVisualizer::MilkdropVisualizer()
    : VisualizerBase(QStringLiteral("milkdrop"), QStringLiteral("Milkdrop"),
                     QStringLiteral("MilkDrop preset host (MD1 core, import target)"))
{
}

bool MilkdropVisualizer::loadMilkFile(const QString& path, QStringList* report)
{
    const lumi::milk::ParseResult parsed =
        lumi::milk::parseFile(std::filesystem::path(path.toStdWString()));
    if (!parsed.ok)
    {
        if (report != nullptr)
        {
            report->append(QStringLiteral("Parser: %1")
                               .arg(QString::fromStdString(parsed.error)));
        }
        return false;
    }
    if (report != nullptr)
    {
        for (const std::string& w : parsed.warnings)
        {
            report->append(QStringLiteral("Parser: %1").arg(QString::fromStdString(w)));
        }
        if (!parsed.sprites.empty())
        {
            report->append(QStringLiteral("%1 Sprite-Sektion(en) geparst — "
                                          "Rendering folgt später")
                               .arg(parsed.sprites.size()));
        }
    }

    lumi::milkdrop::PresetState state = lumi::milkdrop::translate(parsed);
    state.name = QFileInfo(path).completeBaseName().toStdString();
    applyState(std::move(state), report);
    return true;
}

bool MilkdropVisualizer::loadPresetDocument(const QString& path, QStringList* report)
{
    lumi::milkdrop::PresetState state;
    if (!lumi::milkdrop::loadPresetFromFile(path, state, report)) return false;
    if (state.name.empty())
    {
        state.name = QFileInfo(path).completeBaseName().toStdString();
    }
    applyState(std::move(state), report);
    return true;
}

bool MilkdropVisualizer::savePresetDocument(const QString& path) const
{
    return lumi::milkdrop::savePresetToFile(m_state, path);
}

void MilkdropVisualizer::applyState(lumi::milkdrop::PresetState state, QStringList* report)
{
    m_state = std::move(state);

    if (report != nullptr)
    {
        // stage-B classification (M5): default-family shaders are exact, custom
        // shaders fall back to the MD1 path with a feature summary
        using lumi::milk::ShaderClass;
        const auto featureSummary = [](const lumi::milk::ShaderInfo& info) {
            QStringList features;
            if (info.usesBlur[0] || info.usesBlur[1] || info.usesBlur[2])
                features << QStringLiteral("Blur");
            if (info.usesNoise) features << QStringLiteral("Noise");
            if (info.usesTexture) features << QStringLiteral("Texturen");
            if (info.usesRand) features << QStringLiteral("Zufall");
            return features.isEmpty() ? QStringLiteral("einfach") : features.join(QStringLiteral("/"));
        };
        const auto describe = [&](const char* which, const lumi::milk::ShaderInfo& info) {
            switch (info.shaderClass)
            {
            case ShaderClass::None:
                break;
            case ShaderClass::Md1Default:
                report->append(QStringLiteral("%1-Shader = generierter MD1-Default → exakt "
                                              "(eingebackene Konstanten)")
                                   .arg(QLatin1String(which)));
                break;
            case ShaderClass::Md1Plus:
                report->append(QStringLiteral("%1-Shader = MD1-Default + Blur-/Gain-Mix → "
                                              "exakt übersetzt (Stufe B)")
                                   .arg(QLatin1String(which)));
                break;
            case ShaderClass::Custom:
                // Ergebnis (C1-uebersetzt oder MD1-Fallback) meldet
                // prepareCustomShaders direkt im Anschluss
                report->append(QStringLiteral("Custom-%1-Shader (PS%2, %3 Zeilen, %4)")
                                   .arg(QLatin1String(which))
                                   .arg(m_state.psVersion)
                                   .arg(info.codeLines)
                                   .arg(featureSummary(info)));
                break;
            }
        };
        describe("Warp", m_state.warpInfo);
        describe("Comp", m_state.compInfo);
        if (m_state.compInfo.hueMix > 0.001)
        {
            report->append(QStringLiteral(
                "Comp-Shader nutzt hue_shader (fShader-Farbwash) — noch nicht gerendert"));
        }
    }

    prepareCustomShaders(report);
    rebuildScripts(report);
    m_time = 0.0;
    m_frame = 0;
    m_monitor = 0.0;
    m_initRan = false;
}

void MilkdropVisualizer::rebuildScripts(QStringList* report)
{
    m_context = std::make_shared<ScriptContext>();
    m_script = std::make_unique<ScriptSlotHost>("milkdrop", m_context,
                                                ScriptSlotHost::Dialect::Milkdrop);
    m_script->setSource(Slot::Init, m_state.perFrameInit);
    m_script->setSource(Slot::Frame, m_state.perFrame);
    m_script->setSource(Slot::Point, m_state.perPixel);
    if (!m_script->compileAll() && report != nullptr)
    {
        report->append(QStringLiteral("Skript: %1")
                           .arg(QString::fromStdString(m_script->lastError())));
    }

    // custom waves/shapes: one host each (own engine env, SHARED preset context)
    m_waveRt.clear();
    for (const lumi::milkdrop::WaveState& ws : m_state.waves)
    {
        if (!ws.enabled) continue;
        WaveRuntime rt;
        rt.def = ws;
        rt.script = std::make_unique<ScriptSlotHost>(
            "milkwave" + std::to_string(ws.index), m_context,
            ScriptSlotHost::Dialect::Milkdrop);
        rt.script->setSource(Slot::Init, ws.initCode);
        rt.script->setSource(Slot::Frame, ws.frameCode);
        rt.script->setSource(Slot::Point, ws.pointCode);
        if (!rt.script->compileAll() && report != nullptr)
        {
            report->append(QStringLiteral("Wave %1: %2")
                               .arg(ws.index)
                               .arg(QString::fromStdString(rt.script->lastError())));
        }
        m_waveRt.push_back(std::move(rt));
    }
    m_shapeRt.clear();
    for (const lumi::milkdrop::ShapeState& ss : m_state.shapes)
    {
        if (!ss.enabled) continue;
        ShapeRuntime rt;
        rt.def = ss;
        rt.script = std::make_unique<ScriptSlotHost>(
            "milkshape" + std::to_string(ss.index), m_context,
            ScriptSlotHost::Dialect::Milkdrop);
        rt.script->setSource(Slot::Init, ss.initCode);
        rt.script->setSource(Slot::Frame, ss.frameCode);
        if (!rt.script->compileAll() && report != nullptr)
        {
            report->append(QStringLiteral("Shape %1: %2")
                               .arg(ss.index)
                               .arg(QString::fromStdString(rt.script->lastError())));
        }
        m_shapeRt.push_back(std::move(rt));
    }
}

// =============================================================================================
// Audio / per-frame script plumbing
// =============================================================================================

void MilkdropVisualizer::updateAudio(float deltaTime)
{
    m_time += deltaTime;
    if (deltaTime > 1e-4f)
    {
        m_fps = m_fps * 0.95 + (1.0 / deltaTime) * 0.05;
        m_fps = clampd(m_fps, 1.0, 240.0);
    }

    // --- band loudness (MilkLoudness, M2): equal-octave thirds 200..11025 Hz --------------
    const std::vector<float> spec = getSpectrum();
    double imm[3] = {0.0, 0.0, 0.0};
    if (!spec.empty())
    {
        // PORT: bin mapping assumes the spectrum spans 0..22050 Hz linearly;
        // only the ratios matter for the relative loudness model.
        const double edges[4] = {200.0, 761.2, 2897.1, 11025.0};
        const int n = static_cast<int>(spec.size());
        for (int band = 0; band < 3; ++band)
        {
            int start = static_cast<int>(edges[band] / 22050.0 * n);
            int end = static_cast<int>(edges[band + 1] / 22050.0 * n);
            start = std::clamp(start, 0, n - 1);
            end = std::clamp(end, start + 1, n);
            double sum = 0.0;
            for (int i = start; i < end; ++i) sum += spec[static_cast<std::size_t>(i)];
            imm[band] = sum / (end - start);
        }
    }
    m_loudness.update(imm[0], imm[1], imm[2], m_fps);

    // --- waveform: resample both channels to 576 (raw) + smoothed copy (spec §0) -----------
    for (int ch = 0; ch < 2; ++ch)
    {
        const std::vector<float> raw = getWaveformChannel(ch);
        std::array<float, kWaveBuffer>& rawDst = (ch == 0) ? m_waveRawL : m_waveRawR;
        std::array<float, kWaveBuffer>& dst = (ch == 0) ? m_waveL : m_waveR;
        if (raw.size() < 2)
        {
            rawDst.fill(0.0f);
            dst.fill(0.0f);
            continue;
        }
        const int n = static_cast<int>(raw.size());
        for (int i = 0; i < kWaveBuffer; ++i)
        {
            const double pos = static_cast<double>(i) * (n - 1) / (kWaveBuffer - 1);
            const int i0 = static_cast<int>(pos);
            const int i1 = std::min(i0 + 1, n - 1);
            const double frac = pos - i0;
            rawDst[static_cast<std::size_t>(i)] = static_cast<float>(
                raw[static_cast<std::size_t>(i0)] * (1.0 - frac) +
                raw[static_cast<std::size_t>(i1)] * frac);
        }
        // basic wave gets the §0-smoothed copy; custom waves filter the RAW data
        // themselves (own smoothing coefficients, spec 1.4).
        // PORT: our samples are already +-1 (reference: +-128, scale=fWaveScale/128)
        const double mix2 = clampd(m_state.waveSmoothing, 0.0, 0.98);
        const double mix1 = m_state.waveScale * (1.0 - mix2);
        dst[0] = static_cast<float>(rawDst[0] * m_state.waveScale);
        for (int i = 1; i < kWaveBuffer; ++i)
        {
            dst[static_cast<std::size_t>(i)] = static_cast<float>(
                rawDst[static_cast<std::size_t>(i)] * mix1 +
                dst[static_cast<std::size_t>(i - 1)] * mix2);
        }
    }

    // spectrum frame copies for spectrum-sourced custom waves
    m_spectrumL = getSpectrumChannel(0);
    m_spectrumR = getSpectrumChannel(1);
}

void MilkdropVisualizer::pushCommonInputs(lumi::scripting::LuaScriptEngine& e)
{
    // the shared read-only input block every milk slot sees (waves/shapes too)
    e.setNumber("time", m_time);
    e.setNumber("fps", m_fps);
    e.setNumber("frame", static_cast<double>(m_frame));
    e.setNumber("progress", std::fmod(m_time, 60.0) / 60.0);
    e.setNumber("bass", m_loudness.bass());
    e.setNumber("mid", m_loudness.mid());
    e.setNumber("treb", m_loudness.treb());
    e.setNumber("treble", m_loudness.treb());  // E1 alias
    e.setNumber("bass_att", m_loudness.bassAtt());
    e.setNumber("mid_att", m_loudness.midAtt());
    e.setNumber("treb_att", m_loudness.trebAtt());
    e.setNumber("vol", (m_loudness.bass() + m_loudness.mid() + m_loudness.treb()) / 3.0);
    e.setNumber("meshx", static_cast<double>(m_meshX));
    e.setNumber("meshy", static_cast<double>(m_meshY));
    e.setNumber("pixelsx", static_cast<double>(width()));
    e.setNumber("pixelsy", static_cast<double>(height()));
    e.setNumber("aspectx", 1.0 / m_aspectX);
    e.setNumber("aspecty", 1.0 / m_aspectY);
}

void MilkdropVisualizer::pushFrameInputs()
{
    if (m_script == nullptr) return;
    auto& e = m_script->engine();
    const lumi::milkdrop::PresetState& s = m_state;

    // motion (pixel-affecting, LoadPerFrameEvallibVars:470-479)
    e.setNumber("zoom", s.zoom);
    e.setNumber("zoomexp", s.zoomExponent);
    e.setNumber("rot", s.rot);
    e.setNumber("warp", s.warp);
    e.setNumber("cx", s.cx);
    e.setNumber("cy", s.cy);
    e.setNumber("dx", s.dx);
    e.setNumber("dy", s.dy);
    e.setNumber("sx", s.sx);
    e.setNumber("sy", s.sy);

    // read-only inputs (:481-494; PORT: progress cycles over 60 s, no playlist)
    pushCommonInputs(e);
    e.setNumber("monitor", m_monitor);

    // non-motion state (:497-553)
    e.setNumber("decay", s.decay);
    e.setNumber("gamma", s.gammaAdj);
    e.setNumber("echo_zoom", s.videoEchoZoom);
    e.setNumber("echo_alpha", s.videoEchoAlpha);
    e.setNumber("echo_orient", static_cast<double>(s.videoEchoOrientation));
    e.setNumber("wave_a", s.waveAlpha);
    e.setNumber("wave_r", s.waveR);
    e.setNumber("wave_g", s.waveG);
    e.setNumber("wave_b", s.waveB);
    e.setNumber("wave_x", s.waveX);
    e.setNumber("wave_y", s.waveY);
    e.setNumber("wave_mystery", s.waveMystery);
    e.setNumber("wave_mode", static_cast<double>(s.waveMode));
    e.setNumber("wave_usedots", s.waveDots ? 1.0 : 0.0);
    e.setNumber("wave_thick", s.waveThick ? 1.0 : 0.0);
    e.setNumber("wave_additive", s.additiveWaves ? 1.0 : 0.0);
    e.setNumber("wave_brighten", s.maximizeWaveColor ? 1.0 : 0.0);
    e.setNumber("darken_center", s.darkenCenter ? 1.0 : 0.0);
    e.setNumber("wrap", s.texWrap ? 1.0 : 0.0);
    e.setNumber("invert", s.invert ? 1.0 : 0.0);
    e.setNumber("brighten", s.brighten ? 1.0 : 0.0);
    e.setNumber("darken", s.darken ? 1.0 : 0.0);
    e.setNumber("solarize", s.solarize ? 1.0 : 0.0);
    e.setNumber("ob_size", s.obSize);
    e.setNumber("ob_r", s.obR);
    e.setNumber("ob_g", s.obG);
    e.setNumber("ob_b", s.obB);
    e.setNumber("ob_a", s.obA);
    e.setNumber("ib_size", s.ibSize);
    e.setNumber("ib_r", s.ibR);
    e.setNumber("ib_g", s.ibG);
    e.setNumber("ib_b", s.ibB);
    e.setNumber("ib_a", s.ibA);
    e.setNumber("mv_x", s.mvX);
    e.setNumber("mv_y", s.mvY);
    e.setNumber("mv_dx", s.mvDX);
    e.setNumber("mv_dy", s.mvDY);
    e.setNumber("mv_l", s.mvL);
    e.setNumber("mv_r", s.mvR);
    e.setNumber("mv_g", s.mvG);
    e.setNumber("mv_b", s.mvB);
    e.setNumber("mv_a", s.mvA);
    // blur pyramid controls (M5): preset values in, per_frame may re-write them
    e.setNumber("blur1_min", s.blur1Min);
    e.setNumber("blur2_min", s.blur2Min);
    e.setNumber("blur3_min", s.blur3Min);
    e.setNumber("blur1_max", s.blur1Max);
    e.setNumber("blur2_max", s.blur2Max);
    e.setNumber("blur3_max", s.blur3Max);
    e.setNumber("blur1_edge_darken", s.blur1EdgeDarken);
}

void MilkdropVisualizer::pullFrameOutputs(FrameVars& fv)
{
    // defaults straight from the preset (also the no-script path)
    const lumi::milkdrop::PresetState& s = m_state;
    fv.zoom = s.zoom;
    fv.zoomExp = s.zoomExponent;
    fv.rot = s.rot;
    fv.warp = s.warp;
    fv.cx = s.cx;
    fv.cy = s.cy;
    fv.dx = s.dx;
    fv.dy = s.dy;
    fv.sx = s.sx;
    fv.sy = s.sy;
    fv.decay = s.decay;
    fv.gamma = s.gammaAdj;
    fv.echoZoom = s.videoEchoZoom;
    fv.echoAlpha = s.videoEchoAlpha;
    fv.echoOrient = s.videoEchoOrientation;
    fv.waveA = s.waveAlpha;
    fv.waveR = s.waveR;
    fv.waveG = s.waveG;
    fv.waveB = s.waveB;
    fv.waveX = s.waveX;
    fv.waveY = s.waveY;
    fv.waveMystery = s.waveMystery;
    fv.waveMode = s.waveMode;
    fv.waveDots = s.waveDots;
    fv.waveThick = s.waveThick;
    fv.waveAdditive = s.additiveWaves;
    fv.waveBrighten = s.maximizeWaveColor;
    fv.darkenCenter = s.darkenCenter;
    fv.wrap = s.texWrap;
    fv.invert = s.invert;
    fv.brighten = s.brighten;
    fv.darken = s.darken;
    fv.solarize = s.solarize;
    fv.obSize = s.obSize;
    fv.obR = s.obR;
    fv.obG = s.obG;
    fv.obB = s.obB;
    fv.obA = s.obA;
    fv.ibSize = s.ibSize;
    fv.ibR = s.ibR;
    fv.ibG = s.ibG;
    fv.ibB = s.ibB;
    fv.ibA = s.ibA;
    fv.mvX = s.mvX;
    fv.mvY = s.mvY;
    fv.mvDX = s.mvDX;
    fv.mvDY = s.mvDY;
    fv.mvL = s.mvL;
    fv.mvR = s.mvR;
    fv.mvG = s.mvG;
    fv.mvB = s.mvB;
    fv.mvA = s.mvA;
    fv.blurMin = {s.blur1Min, s.blur2Min, s.blur3Min};
    fv.blurMax = {s.blur1Max, s.blur2Max, s.blur3Max};
    fv.blurEdgeDarken = s.blur1EdgeDarken;

    if (m_script == nullptr || !m_script->has(Slot::Frame)) return;

    auto& e = m_script->engine();
    fv.zoom = e.number("zoom");
    fv.zoomExp = e.number("zoomexp");
    fv.rot = e.number("rot");
    fv.warp = e.number("warp");
    fv.cx = e.number("cx");
    fv.cy = e.number("cy");
    fv.dx = e.number("dx");
    fv.dy = e.number("dy");
    fv.sx = e.number("sx");
    fv.sy = e.number("sy");
    fv.decay = e.number("decay");
    // range checks after per_frame (milkdropfs.cpp:677-678) — the ONLY clamps
    fv.gamma = clampd(e.number("gamma"), 0.0, 8.0);
    fv.echoZoom = clampd(e.number("echo_zoom"), 0.001, 1000.0);
    fv.echoAlpha = e.number("echo_alpha");
    fv.echoOrient = static_cast<int>(e.number("echo_orient"));
    fv.waveA = e.number("wave_a");
    fv.waveR = e.number("wave_r");
    fv.waveG = e.number("wave_g");
    fv.waveB = e.number("wave_b");
    fv.waveX = e.number("wave_x");
    fv.waveY = e.number("wave_y");
    fv.waveMystery = e.number("wave_mystery");
    fv.waveMode = static_cast<int>(e.number("wave_mode"));
    fv.waveDots = e.number("wave_usedots") > 0.5;
    fv.waveThick = e.number("wave_thick") > 0.5;
    fv.waveAdditive = e.number("wave_additive") > 0.5;
    fv.waveBrighten = e.number("wave_brighten") > 0.5;
    fv.darkenCenter = e.number("darken_center") > 0.5;
    fv.wrap = e.number("wrap") > 0.5;
    fv.invert = e.number("invert") > 0.5;
    fv.brighten = e.number("brighten") > 0.5;
    fv.darken = e.number("darken") > 0.5;
    fv.solarize = e.number("solarize") > 0.5;
    fv.obSize = e.number("ob_size");
    fv.obR = e.number("ob_r");
    fv.obG = e.number("ob_g");
    fv.obB = e.number("ob_b");
    fv.obA = e.number("ob_a");
    fv.ibSize = e.number("ib_size");
    fv.ibR = e.number("ib_r");
    fv.ibG = e.number("ib_g");
    fv.ibB = e.number("ib_b");
    fv.ibA = e.number("ib_a");
    fv.mvX = e.number("mv_x");
    fv.mvY = e.number("mv_y");
    fv.mvDX = e.number("mv_dx");
    fv.mvDY = e.number("mv_dy");
    fv.mvL = e.number("mv_l");
    fv.mvR = e.number("mv_r");
    fv.mvG = e.number("mv_g");
    fv.mvB = e.number("mv_b");
    fv.mvA = e.number("mv_a");
    fv.blurMin = {e.number("blur1_min"), e.number("blur2_min"), e.number("blur3_min")};
    fv.blurMax = {e.number("blur1_max"), e.number("blur2_max"), e.number("blur3_max")};
    fv.blurEdgeDarken = e.number("blur1_edge_darken");
    if (!m_warpCustomSrc.empty() || !m_compCustomSrc.empty())
    {
        for (int i = 0; i < 32; ++i)
        {
            fv.qVals[static_cast<std::size_t>(i)] =
                e.number(("q" + std::to_string(i + 1)).c_str());
        }
    }
    m_monitor = e.number("monitor");
}

void MilkdropVisualizer::runPerFrameInit()
{
    m_initRan = true;
    if (m_script == nullptr || m_context == nullptr) return;
    pushFrameInputs();
    if (m_script->has(Slot::Init))
    {
        m_script->run(Slot::Init);
        m_monitor = m_script->engine().number("monitor");
    }
    // freeze q_values_after_init_code (M2 contract, state.cpp:1689)
    m_context->captureInitSnapshot();

    // wave/shape init codes run ONCE with the post-init q; their t1-t8 are
    // frozen as t_values_after_init_code (state.cpp:1755/1826). Their q writes
    // must NOT leak into each other (own VM copies in the original).
    const auto runInit = [&](ScriptSlotHost& script, std::array<double, 8>& tInit) {
        auto& e = script.engine();
        pushCommonInputs(e);
        for (int k = 0; k < 8; ++k)
        {
            e.setNumber(("t" + std::to_string(k + 1)).c_str(), 0.0);
        }
        if (script.has(Slot::Init)) script.run(Slot::Init);
        for (int k = 0; k < 8; ++k)
        {
            tInit[static_cast<std::size_t>(k)] =
                e.number(("t" + std::to_string(k + 1)).c_str());
        }
        m_context->restoreInitSnapshot();  // q-Pollution des Inits zuruecksetzen
    };
    for (WaveRuntime& rt : m_waveRt) runInit(*rt.script, rt.tInit);
    for (ShapeRuntime& rt : m_shapeRt) runInit(*rt.script, rt.tInit);
}

// =============================================================================================
// GL lifecycle
// =============================================================================================

void MilkdropVisualizer::onInitialize()
{
    m_lastContext = QOpenGLContext::currentContext();
}

void MilkdropVisualizer::onResize(const QSize& size)
{
    Q_UNUSED(size)
    // FeedbackBuffer::ensure handles resize (blit-preserve) on the next frame.
}

void MilkdropVisualizer::onCleanup()
{
    releaseGlResources();
}

void MilkdropVisualizer::releaseGlResources()
{
    m_feedback.destroy();
    m_scope.destroy();
    releaseBlurTargets();
    releaseCustomGl();
    m_warpProgram.reset();
    m_textureProgram.reset();
    m_colorProgram.reset();
    m_shapeProgram.reset();
    m_blurHProgram.reset();
    m_blurVProgram.reset();
    m_blurLayerProgram.reset();
    m_meshVao.reset();
    m_meshVbo.reset();
    m_quadVao.reset();
    m_quadVbo.reset();
    m_shapeVao.reset();
    m_shapeVbo.reset();
}

bool MilkdropVisualizer::ensureGlResources()
{
    if (m_warpProgram != nullptr) return true;

    const auto makeProgram = [](const char* vs, const char* fs) {
        auto program = std::make_unique<QOpenGLShaderProgram>();
        if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vs) ||
            !program->addShaderFromSourceCode(QOpenGLShader::Fragment, fs) ||
            !program->link())
        {
            return std::unique_ptr<QOpenGLShaderProgram>();
        }
        return program;
    };

    m_warpProgram = makeProgram(kTexVertexShader, kWarpFragmentShader);
    m_textureProgram = makeProgram(kTexVertexShader, kTexFragmentShader);
    m_colorProgram = makeProgram(kColorVertexShader, kColorFragmentShader);
    m_shapeProgram = makeProgram(kShapeVertexShader, kShapeFragmentShader);
    m_blurHProgram = makeProgram(kTexVertexShader, kBlurHFragmentShader);
    m_blurVProgram = makeProgram(kTexVertexShader, kBlurVFragmentShader);
    m_blurLayerProgram = makeProgram(kTexVertexShader, kBlurLayerFragmentShader);
    if (m_warpProgram == nullptr || m_textureProgram == nullptr ||
        m_colorProgram == nullptr || m_shapeProgram == nullptr ||
        m_blurHProgram == nullptr || m_blurVProgram == nullptr ||
        m_blurLayerProgram == nullptr)
    {
        releaseGlResources();
        return false;
    }

    // pos2+uv2 buffer (warp mesh AND composite quads — re-uploaded per draw)
    m_meshVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_meshVao->create();
    m_meshVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_meshVbo->create();
    m_meshVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        m_meshVao->bind();
        m_meshVbo->bind();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                                 reinterpret_cast<void*>(2 * sizeof(float)));
        m_meshVao->release();
        m_meshVbo->release();
    }

    // pos2+uv2+rgba4 buffer (textured custom shapes)
    m_shapeVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_shapeVao->create();
    m_shapeVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_shapeVbo->create();
    m_shapeVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        m_shapeVao->bind();
        m_shapeVbo->bind();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                                 reinterpret_cast<void*>(2 * sizeof(float)));
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                                 reinterpret_cast<void*>(4 * sizeof(float)));
        m_shapeVao->release();
        m_shapeVbo->release();
    }

    // pos2+rgba4 buffer (borders, darken center, post filters)
    m_quadVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_quadVao->create();
    m_quadVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_quadVbo->create();
    m_quadVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        m_quadVao->bind();
        m_quadVbo->bind();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                                 reinterpret_cast<void*>(2 * sizeof(float)));
        m_quadVao->release();
        m_quadVbo->release();
    }

    return m_scope.ensure();
}

// =============================================================================================
// Blur pyramid (BlurPasses port, milkdropfs.cpp:1501-1679 — M5)
// =============================================================================================

int MilkdropVisualizer::activeBlurLevels() const
{
    // stage B: baked composite blur mixes; stage C1: transpiled custom shaders
    // that sample blurN (usesBlur flags from the classifier feature scan)
    int levels = 0;
    const lumi::milk::ShaderInfo& ci = m_state.compInfo;
    if (ci.shaderClass == lumi::milk::ShaderClass::Md1Plus) levels = ci.highestBlurLevel();
    const auto usageLevel = [](const lumi::milk::ShaderInfo& info) {
        for (int n = 3; n >= 1; --n)
        {
            if (info.usesBlur[static_cast<std::size_t>(n - 1)]) return n;
        }
        return 0;
    };
    if (m_compCustomProgram != nullptr) levels = std::max(levels, usageLevel(ci));
    if (m_warpCustomProgram != nullptr)
        levels = std::max(levels, usageLevel(m_state.warpInfo));
    return levels;
}

bool MilkdropVisualizer::ensureBlurTargets(int sourceW, int sourceH)
{
    if (m_blurTex[0] != 0 && sourceW == m_blurSrcW && sourceH == m_blurSrcH) return true;
    releaseBlurTargets();

    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    m_blurSizes = lumi::milkdrop::blurTextureSizes(sourceW, sourceH);
    f->glGenTextures(lumi::milkdrop::kBlurTexCount, m_blurTex.data());
    f->glGenFramebuffers(lumi::milkdrop::kBlurTexCount, m_blurFbo.data());
    for (int i = 0; i < lumi::milkdrop::kBlurTexCount; ++i)
    {
        const auto idx = static_cast<std::size_t>(i);
        f->glBindTexture(GL_TEXTURE_2D, m_blurTex[idx]);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_blurSizes[idx][0], m_blurSizes[idx][1],
                        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        f->glBindFramebuffer(GL_FRAMEBUFFER, m_blurFbo[idx]);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                  m_blurTex[idx], 0);
        if (f->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
            releaseBlurTargets();
            return false;
        }
    }
    f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    f->glBindTexture(GL_TEXTURE_2D, 0);
    m_blurSrcW = sourceW;
    m_blurSrcH = sourceH;
    return true;
}

void MilkdropVisualizer::releaseBlurTargets()
{
    if (m_blurTex[0] == 0 && m_blurFbo[0] == 0) return;
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (ctx != nullptr)
    {
        QOpenGLFunctions* f = ctx->functions();
        f->glDeleteFramebuffers(lumi::milkdrop::kBlurTexCount, m_blurFbo.data());
        f->glDeleteTextures(lumi::milkdrop::kBlurTexCount, m_blurTex.data());
    }
    m_blurTex.fill(0);
    m_blurFbo.fill(0);
    m_blurSrcW = 0;
    m_blurSrcH = 0;
}

void MilkdropVisualizer::runBlurPasses(const FrameVars& fv)
{
    const int levels = activeBlurLevels();
    if (levels <= 0) return;

    const int w = std::max(16, width());
    const int h = std::max(16, height());
    if (!ensureBlurTargets(w, h)) return;

    using namespace lumi::milkdrop;
    const BlurRanges ranges = computeSafeBlurRanges(
        {static_cast<float>(fv.blurMin[0]), static_cast<float>(fv.blurMin[1]),
         static_cast<float>(fv.blurMin[2])},
        {static_cast<float>(fv.blurMax[0]), static_cast<float>(fv.blurMax[1]),
         static_cast<float>(fv.blurMax[2])});
    const BlurPassScales scales = computeBlurPassScales(ranges);
    const BlurKernelH kh = blurKernelH();
    const BlurKernelV kv = blurKernelV();

    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glDisable(GL_BLEND);
    f->glDisable(GL_DEPTH_TEST);

    // fullscreen quad, identity UVs (source and target share the orientation)
    const float quad[6][4] = {{-1.0f, -1.0f, 0.0f, 0.0f}, {1.0f, -1.0f, 1.0f, 0.0f},
                              {-1.0f, 1.0f, 0.0f, 1.0f},  {1.0f, -1.0f, 1.0f, 0.0f},
                              {1.0f, 1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, 0.0f, 1.0f}};

    const int passes = 2 * levels;
    for (int i = 0; i < passes; ++i)
    {
        const auto idx = static_cast<std::size_t>(i);
        f->glBindFramebuffer(GL_FRAMEBUFFER, m_blurFbo[idx]);
        f->glViewport(0, 0, m_blurSizes[idx][0], m_blurSizes[idx][1]);

        const int srcW = (i == 0) ? w : m_blurSizes[idx - 1][0];
        const int srcH = (i == 0) ? h : m_blurSizes[idx - 1][1];
        const unsigned int srcTex =
            (i == 0) ? m_feedback.previousTexture() : m_blurTex[idx - 1];
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, srcTex);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        QOpenGLShaderProgram& program = ((i % 2) == 0) ? *m_blurHProgram : *m_blurVProgram;
        program.bind();
        program.setUniformValue("uTex", 0);
        program.setUniformValue("uTexSize",
                                QVector4D(static_cast<float>(srcW), static_cast<float>(srcH),
                                          1.0f / static_cast<float>(srcW),
                                          1.0f / static_cast<float>(srcH)));
        if ((i % 2) == 0)
        {
            program.setUniformValue("uW", QVector4D(kh.w1, kh.w2, kh.w3, kh.w4));
            program.setUniformValue("uD", QVector4D(kh.d1, kh.d2, kh.d3, kh.d4));
            const auto level = static_cast<std::size_t>(i / 2);
            program.setUniformValue(
                "uScaleBias", QVector4D(scales.scale[level], scales.bias[level], kh.wDiv, 0.0f));
        }
        else
        {
            program.setUniformValue("uWD", QVector4D(kv.w1, kv.w2, kv.d1, kv.d2));
            // edge darkening only on the FIRST vertical pass (reference :1656)
            const float edge = static_cast<float>(fv.blurEdgeDarken);
            const QVector4D edgeVec = (i == 1)
                                          ? QVector4D(kv.wDiv, 1.0f - edge, edge, 5.0f)
                                          : QVector4D(kv.wDiv, 1.0f, 0.0f, 5.0f);
            program.setUniformValue("uEdge", edgeVec);
        }

        m_meshVao->bind();
        m_meshVbo->bind();
        m_meshVbo->allocate(quad, sizeof(quad));
        f->glDrawArrays(GL_TRIANGLES, 0, 6);
        m_meshVbo->release();
        m_meshVao->release();
        program.release();
    }
    f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// =============================================================================================
// Stufe C1: transpiled custom shaders (HLSL -> GLSL, MilkDrop_Import_Konzept §6b)
// =============================================================================================

void MilkdropVisualizer::prepareCustomShaders(QStringList* report)
{
    m_warpCustomSrc.clear();
    m_compCustomSrc.clear();
    m_customGlError.clear();
    ++m_customRev;

    const auto tryTranspile = [&](const std::string& text, lumi::hlsl::ShaderKind kind,
                                  const lumi::milk::ShaderInfo& info, std::string& outSrc,
                                  const char* which) {
        if (info.shaderClass != lumi::milk::ShaderClass::Custom || text.empty()) return;
        const lumi::hlsl::HlslResult r = lumi::hlsl::transpile(text, kind);
        if (r.ok)
        {
            outSrc = assembleCustomFragment(r);
            if (report != nullptr)
            {
                QString note = QStringLiteral(
                                   "%1-Shader → GLSL übersetzt (Stufe C1, GL-Kompilierung "
                                   "zur Laufzeit)")
                                   .arg(QLatin1String(which));
                if (!r.customSamplers.empty())
                {
                    note += QStringLiteral(" — %1 Custom-Textur(en), Platzhalter bis C2")
                                .arg(r.customSamplers.size());
                }
                report->append(note);
            }
        }
        else if (report != nullptr)
        {
            report->append(QStringLiteral("%1-Shader nicht uebersetzbar (%2) → MD1-Fallback")
                               .arg(QLatin1String(which))
                               .arg(QString::fromStdString(r.error)));
        }
    };
    tryTranspile(m_state.warpShaderText, lumi::hlsl::ShaderKind::Warp, m_state.warpInfo,
                 m_warpCustomSrc, "Warp");
    tryTranspile(m_state.compShaderText, lumi::hlsl::ShaderKind::Comp, m_state.compInfo,
                 m_compCustomSrc, "Comp");

    // rand_preset: einmal je Preset-Ladung (engine-lokaler PRNG, Entscheid §10)
    m_randSeed = m_randSeed * 1664525u + 1013904223u;
}

void MilkdropVisualizer::releaseCustomGl()
{
    m_warpCustomProgram.reset();
    m_compCustomProgram.reset();
    m_customBuiltRev = -1;
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (ctx != nullptr)
    {
        QOpenGLFunctions* f = ctx->functions();
        if (m_noiseTex[0] != 0)
            f->glDeleteTextures(static_cast<GLsizei>(m_noiseTex.size()), m_noiseTex.data());
        if (m_placeholderTex != 0) f->glDeleteTextures(1, &m_placeholderTex);
        QOpenGLExtraFunctions* ef = ctx->extraFunctions();
        if (m_samplerObj[0] != 0)
            ef->glDeleteSamplers(static_cast<GLsizei>(m_samplerObj.size()),
                                 m_samplerObj.data());
    }
    m_noiseTex.fill(0);
    m_placeholderTex = 0;
    m_samplerObj.fill(0);
}

bool MilkdropVisualizer::ensureCustomPrograms()
{
    if (m_customBuiltRev == m_customRev)
        return m_warpCustomProgram != nullptr || m_compCustomProgram != nullptr;
    m_customBuiltRev = m_customRev;
    m_warpCustomProgram.reset();
    m_compCustomProgram.reset();

    const auto build = [&](const std::string& src) {
        if (src.empty()) return std::unique_ptr<QOpenGLShaderProgram>();
        auto program = std::make_unique<QOpenGLShaderProgram>();
        if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, kCustomVertexShader) ||
            !program->addShaderFromSourceCode(QOpenGLShader::Fragment, src.c_str()) ||
            !program->link())
        {
            // Fallback auf MD1; Fehler fuer Diagnose merken (nie aus dem
            // Render-Thread loggen — BasicLogger ist nicht thread-safe)
            if (m_customGlError.empty()) m_customGlError = program->log().toStdString();
            return std::unique_ptr<QOpenGLShaderProgram>();
        }
        return program;
    };
    m_warpCustomProgram = build(m_warpCustomSrc);
    m_compCustomProgram = build(m_compCustomSrc);
    return m_warpCustomProgram != nullptr || m_compCustomProgram != nullptr;
}

bool MilkdropVisualizer::ensureNoiseTextures()
{
    if (m_noiseTex[0] != 0) return true;
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions* f = ctx->functions();
    QOpenGLExtraFunctions* ef = ctx->extraFunctions();

    // C1-Platzhalter: gleichverteiltes RGBA-Rauschen (exakter AddNoiseTex-Port
    // mit Zoom/Interpolation folgt in C2); deterministisch fuer Sichttests
    f->glGenTextures(static_cast<GLsizei>(m_noiseTex.size()), m_noiseTex.data());
    unsigned int seed = 0x1234567u;
    const auto nextByte = [&seed]() {
        seed = seed * 1664525u + 1013904223u;
        return static_cast<unsigned char>(seed >> 24);
    };
    const std::array<int, 4> sizes = {256, 32, 256, 256};  // lq, lq_lite, mq, hq
    std::vector<unsigned char> pixels;
    for (std::size_t i = 0; i < m_noiseTex.size(); ++i)
    {
        const int n = sizes[i];
        pixels.resize(static_cast<std::size_t>(n) * n * 4);
        for (unsigned char& p : pixels) p = nextByte();
        f->glBindTexture(GL_TEXTURE_2D, m_noiseTex[i]);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, n, n, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                        pixels.data());
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    f->glGenTextures(1, &m_placeholderTex);
    const unsigned char grey[4] = {128, 128, 128, 255};
    f->glBindTexture(GL_TEXTURE_2D, m_placeholderTex);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, grey);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 4 Sampler-Objekte: wrap/clamp x linear/point (fc/pc/fw/pw-Semantik)
    ef->glGenSamplers(static_cast<GLsizei>(m_samplerObj.size()), m_samplerObj.data());
    const auto setup = [&](unsigned int obj, GLint wrap, GLint filter) {
        ef->glSamplerParameteri(obj, GL_TEXTURE_WRAP_S, wrap);
        ef->glSamplerParameteri(obj, GL_TEXTURE_WRAP_T, wrap);
        ef->glSamplerParameteri(obj, GL_TEXTURE_MIN_FILTER, filter);
        ef->glSamplerParameteri(obj, GL_TEXTURE_MAG_FILTER, filter);
    };
    setup(m_samplerObj[0], GL_REPEAT, GL_LINEAR);
    setup(m_samplerObj[1], GL_CLAMP_TO_EDGE, GL_LINEAR);
    setup(m_samplerObj[2], GL_REPEAT, GL_NEAREST);
    setup(m_samplerObj[3], GL_CLAMP_TO_EDGE, GL_NEAREST);
    return true;
}

void MilkdropVisualizer::feedCustomUniforms(QOpenGLShaderProgram& program,
                                            const FrameVars& fv, unsigned int mainTexture)
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions* ef = QOpenGLContext::currentContext()->extraFunctions();
    const int w = std::max(16, width());
    const int h = std::max(16, height());

    // --- Sampler dynamisch auf Units verteilen (nur aktive Uniforms) ------------------
    int unit = 0;
    const auto bindSampler = [&](const char* name, unsigned int tex, unsigned int samplerObj) {
        const int loc = program.uniformLocation(name);
        if (loc < 0 || tex == 0) return;
        f->glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
        f->glBindTexture(GL_TEXTURE_2D, tex);
        ef->glBindSampler(static_cast<GLuint>(unit), samplerObj);
        program.setUniformValue(name, unit);
        ++unit;
    };
    const unsigned int wrapLin = m_samplerObj[0];
    const unsigned int clampLin = m_samplerObj[1];
    const unsigned int wrapPoint = m_samplerObj[2];
    const unsigned int clampPoint = m_samplerObj[3];

    bindSampler("sampler_main", mainTexture, fv.wrap ? wrapLin : clampLin);
    bindSampler("sampler_fc_main", mainTexture, clampLin);
    bindSampler("sampler_pc_main", mainTexture, clampPoint);
    bindSampler("sampler_fw_main", mainTexture, wrapLin);
    bindSampler("sampler_pw_main", mainTexture, wrapPoint);
    bindSampler("sampler_blur1", m_blurTex[1], clampLin);
    bindSampler("sampler_blur2", m_blurTex[3], clampLin);
    bindSampler("sampler_blur3", m_blurTex[5], clampLin);
    const std::array<const char*, 4> noiseNames = {"noise_lq", "noise_lq_lite", "noise_mq",
                                                   "noise_hq"};
    for (std::size_t i = 0; i < noiseNames.size(); ++i)
    {
        const std::string base = noiseNames[i];
        bindSampler(("sampler_" + base).c_str(), m_noiseTex[i], wrapLin);
        bindSampler(("sampler_fc_" + base).c_str(), m_noiseTex[i], clampLin);
        bindSampler(("sampler_pc_" + base).c_str(), m_noiseTex[i], clampPoint);
        bindSampler(("sampler_fw_" + base).c_str(), m_noiseTex[i], wrapLin);
        bindSampler(("sampler_pw_" + base).c_str(), m_noiseTex[i], wrapPoint);
    }
    // Custom-Texturen (C2): alle uebrigen aktiven sampler-Uniforms auf den
    // Platzhalter legen, damit das Programm definiert laeuft
    {
        GLint count = 0;
        GLuint prog = program.programId();
        ef->glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &count);
        for (GLint u = 0; u < count; ++u)
        {
            char nameBuf[128];
            GLsizei len = 0;
            GLint size = 0;
            GLenum type = 0;
            ef->glGetActiveUniform(prog, static_cast<GLuint>(u), sizeof(nameBuf), &len,
                                   &size, &type, nameBuf);
            if (type != GL_SAMPLER_2D) continue;
            const std::string name(nameBuf, static_cast<std::size_t>(len));
            if (preambleDeclares(name) || name.rfind("sampler_", 0) != 0) continue;
            bindSampler(name.c_str(), m_placeholderTex, wrapLin);
        }
    }

    // --- Skalar-/Vektor-Uniforms --------------------------------------------------------
    program.setUniformValue("uOrigFromPos", false);  // Aufrufer ueberschreibt fuer Warp
    program.setUniformValue("texsize", QVector4D(static_cast<float>(w),
                                                 static_cast<float>(h), 1.0f / w, 1.0f / h));
    program.setUniformValue(
        "aspect", QVector4D(static_cast<float>(m_aspectX), static_cast<float>(m_aspectY),
                            1.0f / static_cast<float>(m_aspectX),
                            1.0f / static_cast<float>(m_aspectY)));
    const float t = static_cast<float>(m_time);
    program.setUniformValue("time", t);
    program.setUniformValue("fps", static_cast<float>(m_fps));
    program.setUniformValue("frame", static_cast<float>(m_frame));
    program.setUniformValue("progress", static_cast<float>(std::fmod(m_time, 60.0) / 60.0));

    auto& engine = m_script->engine();
    program.setUniformValue("bass", static_cast<float>(engine.number("bass")));
    program.setUniformValue("mid", static_cast<float>(engine.number("mid")));
    program.setUniformValue("treb", static_cast<float>(engine.number("treb")));
    program.setUniformValue("vol", static_cast<float>(engine.number("vol")));
    program.setUniformValue("bass_att", static_cast<float>(engine.number("bass_att")));
    program.setUniformValue("mid_att", static_cast<float>(engine.number("mid_att")));
    program.setUniformValue("treb_att", static_cast<float>(engine.number("treb_att")));
    program.setUniformValue("vol_att", static_cast<float>(engine.number("vol")));

    // rand_frame je Frame, rand_preset je Ladung (LCG, deterministisch genug)
    unsigned int rs = m_randSeed;
    const auto rand01 = [](unsigned int& s) {
        s = s * 1664525u + 1013904223u;
        return static_cast<float>(s >> 8) / 16777216.0f;
    };
    program.setUniformValue("rand_preset",
                            QVector4D(rand01(rs), rand01(rs), rand01(rs), rand01(rs)));
    unsigned int fs = m_randSeed ^ (static_cast<unsigned int>(m_frame) * 2654435761u);
    program.setUniformValue("rand_frame",
                            QVector4D(rand01(fs), rand01(fs), rand01(fs), rand01(fs)));

    // roam-Vektoren (plugin.cpp:3892-3911)
    const auto roam = [t](float mul, float phase, bool sine) {
        return 0.5f + 0.5f * (sine ? std::sin(t * mul + phase) : std::cos(t * mul + phase));
    };
    program.setUniformValue("roam_cos", QVector4D(roam(0.329f, 1.2f, false),
                                                  roam(1.293f, 3.9f, false),
                                                  roam(5.070f, 2.5f, false),
                                                  roam(20.051f, 5.4f, false)));
    program.setUniformValue("roam_sin", QVector4D(roam(0.329f, 1.2f, true),
                                                  roam(1.293f, 3.9f, true),
                                                  roam(5.070f, 2.5f, true),
                                                  roam(20.051f, 5.4f, true)));
    program.setUniformValue("slow_roam_cos", QVector4D(roam(0.0050f, 2.7f, false),
                                                       roam(0.0085f, 5.3f, false),
                                                       roam(0.0133f, 4.5f, false),
                                                       roam(0.0217f, 3.8f, false)));
    program.setUniformValue("slow_roam_sin", QVector4D(roam(0.0050f, 2.7f, true),
                                                       roam(0.0085f, 5.3f, true),
                                                       roam(0.0133f, 4.5f, true),
                                                       roam(0.0217f, 3.8f, true)));

    for (int i = 0; i < 32; ++i)
    {
        program.setUniformValue(("q" + std::to_string(i + 1)).c_str(),
                                static_cast<float>(fv.qVals[static_cast<std::size_t>(i)]));
    }

    // Blur-Ranges + Un-Bias-Konstanten (GetBlurN)
    const lumi::milkdrop::BlurRanges ranges = lumi::milkdrop::computeSafeBlurRanges(
        {static_cast<float>(fv.blurMin[0]), static_cast<float>(fv.blurMin[1]),
         static_cast<float>(fv.blurMin[2])},
        {static_cast<float>(fv.blurMax[0]), static_cast<float>(fv.blurMax[1]),
         static_cast<float>(fv.blurMax[2])});
    program.setUniformValue("blur1_min", ranges.min[0]);
    program.setUniformValue("blur1_max", ranges.max[0]);
    program.setUniformValue("blur2_min", ranges.min[1]);
    program.setUniformValue("blur2_max", ranges.max[1]);
    program.setUniformValue("blur3_min", ranges.min[2]);
    program.setUniformValue("blur3_max", ranges.max[2]);
    program.setUniformValue("_blur_scale",
                            QVector4D(ranges.max[0] - ranges.min[0], ranges.min[0],
                                      ranges.max[1] - ranges.min[1], ranges.min[1]));
    program.setUniformValue(
        "_blur_sb3", QVector2D(ranges.max[2] - ranges.min[2], ranges.min[2]));

    // texsize_noise_* + hue_shader (C1: neutral, fShader-Wash folgt)
    const auto texsizeVec = [](int n) {
        return QVector4D(static_cast<float>(n), static_cast<float>(n), 1.0f / n, 1.0f / n);
    };
    program.setUniformValue("texsize_noise_lq", texsizeVec(256));
    program.setUniformValue("texsize_noise_lq_lite", texsizeVec(32));
    program.setUniformValue("texsize_noise_mq", texsizeVec(256));
    program.setUniformValue("texsize_noise_hq", texsizeVec(256));
    program.setUniformValue("hue_shader", QVector3D(1.0f, 1.0f, 1.0f));
    f->glActiveTexture(GL_TEXTURE0);
}

// =============================================================================================
// Frame
// =============================================================================================

void MilkdropVisualizer::onRender(float deltaTime)
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (ctx == nullptr) return;
    if (m_lastContext != nullptr && ctx != m_lastContext)
    {
        // context changed (undock) — old GL objects died with it; release the
        // wrappers in the new context (PulsingVisualizer pattern)
        releaseGlResources();
    }
    m_lastContext = ctx;

    if (!ensureGlResources()) return;

    const int w = std::max(16, width());
    const int h = std::max(16, height());
    m_aspectX = (h > w) ? static_cast<double>(w) / h : 1.0;   // plugin.cpp:2035
    m_aspectY = (w > h) ? static_cast<double>(h) / w : 1.0;

    const bool wasReady = m_feedback.ready();
    if (!m_feedback.ensure(w, h)) return;
    QOpenGLFunctions* f = ctx->functions();
    if (!wasReady)
    {
        // fresh buffers hold garbage — start from black on both sides
        for (int i = 0; i < 2; ++i)
        {
            m_feedback.beginFrame();
            f->glViewport(0, 0, w, h);
            f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            f->glClear(GL_COLOR_BUFFER_BIT);
            m_feedback.swapOnly();
        }
    }

    updateAudio(deltaTime);
    if (!m_initRan) runPerFrameInit();

    // --- per_frame (M2 q contract: init snapshot -> run -> frame snapshot) ----------------
    FrameVars fv;
    if (m_context != nullptr) m_context->restoreInitSnapshot();
    pushFrameInputs();
    if (m_script != nullptr && m_script->has(Slot::Frame)) m_script->run(Slot::Frame);
    pullFrameOutputs(fv);
    if (m_context != nullptr) m_context->captureFrameSnapshot();

    // --- warp + drawing into the current buffer -------------------------------------------
    // order per RenderFrame (milkdropfs.cpp:1054-1057): shapes first ("better
    // for feedback if the waves draw over them"), then custom waves, then the
    // basic wave. PORT: motion vectors are drawn after the warp instead of
    // into the previous image (one frame later into the feedback loop).
    // Stufe C1: transpilierte Programme + Noise-Texturen bereitstellen (lazy)
    if (!m_warpCustomSrc.empty() || !m_compCustomSrc.empty())
    {
        ensureNoiseTextures();
        ensureCustomPrograms();
    }

    computeWarpMesh(fv);
    runBlurPasses(fv);  // sources the previous frame; own FBOs (before beginFrame)
    m_feedback.beginFrame();
    f->glViewport(0, 0, w, h);
    f->glDisable(GL_DEPTH_TEST);
    drawWarpPass(fv);
    drawMotionVectors(fv);
    drawCustomShapes();
    drawCustomWaves();
    drawBasicWave(fv);
    drawBorders(fv);
    if (fv.darkenCenter) drawDarkenCenter();

    // --- present (single vertical flip lives here) -----------------------------------------
    compositeToScreen(fv);
    if (m_debugGrid) drawDebugGrid();
    m_feedback.swapOnly();

    ++m_frame;
}

// =============================================================================================
// Warp mesh (ComputeGridAlphaValues port, milkdropfs.cpp:1698-1857)
// =============================================================================================

void MilkdropVisualizer::computeWarpMesh(const FrameVars& fv)
{
    const int gx = m_meshX;
    const int gy = m_meshY;
    const int vertsX = gx + 1;
    const int vertsY = gy + 1;

    const double warpTime = m_time * m_state.warpAnimSpeed;
    const double warpScaleInv = 1.0 / std::max(0.001, m_state.warpScale);
    const double f0 = 11.68 + 4.0 * std::cos(warpTime * 1.413 + 10.0);
    const double f1 = 8.77 + 3.0 * std::cos(warpTime * 1.113 + 7.0);
    const double f2 = 10.54 + 3.0 * std::cos(warpTime * 1.233 + 3.0);
    const double f3 = 11.49 + 4.0 * std::cos(warpTime * 0.933 + 5.0);

    const double texelX = 0.5 / std::max(1, width());
    const double texelY = 0.5 / std::max(1, height());

    m_vertexUv.assign(static_cast<std::size_t>(vertsX * vertsY) * 2, 0.0);

    const bool perVertex = m_script != nullptr && m_script->has(Slot::Point);
    lumi::scripting::LuaScriptEngine* eng =
        (m_script != nullptr) ? &m_script->engine() : nullptr;

    int n = 0;
    for (int iy = 0; iy < vertsY; ++iy)
    {
        for (int ix = 0; ix < vertsX; ++ix, ++n)
        {
            const double vx = static_cast<double>(ix) / gx * 2.0 - 1.0;
            const double vy = static_cast<double>(iy) / gy * 2.0 - 1.0;
            const double rad = std::sqrt(vx * vx * m_aspectX * m_aspectX +
                                         vy * vy * m_aspectY * m_aspectY);
            const double ang = (ix == gx / 2 && iy == gy / 2)
                                   ? 0.0
                                   : std::atan2(vy * m_aspectY, vx * m_aspectX);

            double zoom = fv.zoom;
            double zoomExp = fv.zoomExp;
            double rot = fv.rot;
            double warpAmt = fv.warp;
            double cx = fv.cx;
            double cy = fv.cy;
            double dx = fv.dx;
            double dy = fv.dy;
            double sx = fv.sx;
            double sy = fv.sy;

            if (perVertex)
            {
                eng->setNumber("x", vx * 0.5 * m_aspectX + 0.5);
                eng->setNumber("y", vy * -0.5 * m_aspectY + 0.5);
                eng->setNumber("rad", rad);
                eng->setNumber("ang", ang);
                eng->setNumber("zoom", zoom);
                eng->setNumber("zoomexp", zoomExp);
                eng->setNumber("rot", rot);
                eng->setNumber("warp", warpAmt);
                eng->setNumber("cx", cx);
                eng->setNumber("cy", cy);
                eng->setNumber("dx", dx);
                eng->setNumber("dy", dy);
                eng->setNumber("sx", sx);
                eng->setNumber("sy", sy);
                m_script->run(Slot::Point);
                zoom = eng->number("zoom");
                zoomExp = eng->number("zoomexp");
                rot = eng->number("rot");
                warpAmt = eng->number("warp");
                cx = eng->number("cx");
                cy = eng->number("cy");
                dx = eng->number("dx");
                dy = eng->number("dy");
                sx = eng->number("sx");
                sy = eng->number("sy");
            }

            const double zoom2 = std::pow(zoom, std::pow(zoomExp, rad * 2.0 - 1.0));
            const double zoom2Inv = 1.0 / ((std::fabs(zoom2) < 1e-9) ? 1e-9 : zoom2);

            double u = vx * m_aspectX * 0.5 * zoom2Inv + 0.5;
            double v = -vy * m_aspectY * 0.5 * zoom2Inv + 0.5;

            // stretch
            u = (u - cx) / ((std::fabs(sx) < 1e-9) ? 1e-9 : sx) + cx;
            v = (v - cy) / ((std::fabs(sy) < 1e-9) ? 1e-9 : sy) + cy;

            // warp ripple (4 terms, constants verbatim)
            u += warpAmt * 0.0035 * std::sin(warpTime * 0.333 + warpScaleInv * (vx * f0 - vy * f3));
            v += warpAmt * 0.0035 * std::cos(warpTime * 0.375 - warpScaleInv * (vx * f2 + vy * f1));
            u += warpAmt * 0.0035 * std::cos(warpTime * 0.753 - warpScaleInv * (vx * f1 - vy * f2));
            v += warpAmt * 0.0035 * std::sin(warpTime * 0.825 + warpScaleInv * (vx * f0 + vy * f3));

            // rotation about (cx, cy)
            const double u2 = u - cx;
            const double v2 = v - cy;
            const double cosRot = std::cos(rot);
            const double sinRot = std::sin(rot);
            u = u2 * cosRot - v2 * sinRot + cx;
            v = u2 * sinRot + v2 * cosRot + cy;

            // translation + aspect undo + half-texel alignment
            u -= dx;
            v -= dy;
            u = (u - 0.5) / m_aspectX + 0.5;
            v = (v - 0.5) / m_aspectY + 0.5;
            u += texelX;
            v += texelY;

            m_vertexUv[static_cast<std::size_t>(n) * 2] = u;
            m_vertexUv[static_cast<std::size_t>(n) * 2 + 1] = v;
        }
    }

    // triangulate: pos = grid NDC, uv = (u, 1-v) — reference v is D3D-style (0 = top)
    m_meshData.clear();
    m_meshData.reserve(static_cast<std::size_t>(gx * gy) * 6 * 4);
    // NB: nicht "emit" nennen — Qt-Makro (Merkregel Session 38)
    const auto pushVertex = [&](int ix, int iy) {
        const int idx = iy * vertsX + ix;
        m_meshData.push_back(static_cast<float>(static_cast<double>(ix) / gx * 2.0 - 1.0));
        m_meshData.push_back(static_cast<float>(static_cast<double>(iy) / gy * 2.0 - 1.0));
        m_meshData.push_back(static_cast<float>(m_vertexUv[static_cast<std::size_t>(idx) * 2]));
        m_meshData.push_back(
            static_cast<float>(1.0 - m_vertexUv[static_cast<std::size_t>(idx) * 2 + 1]));
    };
    for (int iy = 0; iy < gy; ++iy)
    {
        for (int ix = 0; ix < gx; ++ix)
        {
            pushVertex(ix, iy);
            pushVertex(ix + 1, iy);
            pushVertex(ix, iy + 1);
            pushVertex(ix + 1, iy);
            pushVertex(ix + 1, iy + 1);
            pushVertex(ix, iy + 1);
        }
    }
}

void MilkdropVisualizer::drawWarpPass(const FrameVars& fv)
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glDisable(GL_BLEND);

    // Stufe C1: transpilierter Warp-Shader ersetzt den MD1-Decay-Blit komplett
    // (Decay/Effekte stecken im Shader-Text; uv kommt weiter aus dem per_pixel-Mesh)
    if (m_warpCustomProgram != nullptr)
    {
        m_warpCustomProgram->bind();
        feedCustomUniforms(*m_warpCustomProgram, fv, m_feedback.previousTexture());
        m_warpCustomProgram->setUniformValue("uOrigFromPos", true);
        m_meshVao->bind();
        m_meshVbo->bind();
        m_meshVbo->allocate(m_meshData.data(),
                            static_cast<int>(m_meshData.size() * sizeof(float)));
        f->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_meshData.size() / 4));
        m_meshVbo->release();
        m_meshVao->release();
        m_warpCustomProgram->release();
        return;
    }

    // MD2 semantics: with a recognized warp shader the decay constants are BAKED
    // into the shader text (per_frame `decay` has no effect); MD1/custom use the
    // live value (custom = fallback, report said so)
    double decayMul = clampd(fv.decay, 0.0, 1.0);
    double decaySub = 0.0;
    bool wrap = fv.wrap;
    if (m_state.warpInfo.shaderClass == lumi::milk::ShaderClass::Md1Default)
    {
        decayMul = (m_state.warpInfo.decayMul >= 0.0) ? m_state.warpInfo.decayMul : 1.0;
        decaySub = m_state.warpInfo.decaySub;
        wrap = m_state.warpInfo.wrapSampler;
    }

    m_warpProgram->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, m_feedback.previousTexture());
    const GLint wrapMode = wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_warpProgram->setUniformValue("uTex", 0);
    m_warpProgram->setUniformValue("uDecay", static_cast<float>(decayMul));
    m_warpProgram->setUniformValue("uDecaySub", static_cast<float>(decaySub));

    m_meshVao->bind();
    m_meshVbo->bind();
    m_meshVbo->allocate(m_meshData.data(),
                        static_cast<int>(m_meshData.size() * sizeof(float)));
    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_meshData.size() / 4));
    m_meshVbo->release();
    m_meshVao->release();
    m_warpProgram->release();
}

// =============================================================================================
// Motion vectors (DrawMotionVectors port, milkdropfs.cpp:1156-1308)
// =============================================================================================

bool MilkdropVisualizer::reversePropagate(double fx, double fy, double& outX,
                                          double& outY) const
{
    // bilinear lookup in the warp-mesh UVs (ReversePropagatePoint, :1431)
    const int gx = m_meshX;
    const int gy = m_meshY;
    if (m_vertexUv.size() < static_cast<std::size_t>((gx + 1) * (gy + 1)) * 2) return false;

    const double gxPos = fx * gx;
    const double gyPos = fy * gy;
    const int x0 = static_cast<int>(gxPos);
    const int y0 = static_cast<int>(gyPos);
    const double dx = gxPos - x0;
    const double dy = gyPos - y0;
    const int x1 = x0 + 1;
    const int y1 = y0 + 1;
    if (x0 < 0 || y0 < 0 || x1 > gx || y1 > gy) return false;

    const auto uv = [&](int ix, int iy, int comp) {
        return m_vertexUv[static_cast<std::size_t>(iy * (gx + 1) + ix) * 2 +
                          static_cast<std::size_t>(comp)];
    };
    const double tu = uv(x0, y0, 0) * (1 - dx) * (1 - dy) + uv(x1, y0, 0) * dx * (1 - dy) +
                      uv(x0, y1, 0) * (1 - dx) * dy + uv(x1, y1, 0) * dx * dy;
    const double tv = uv(x0, y0, 1) * (1 - dx) * (1 - dy) + uv(x1, y0, 1) * dx * (1 - dy) +
                      uv(x0, y1, 1) * (1 - dx) * dy + uv(x1, y1, 1) * dx * dy;
    outX = tu;
    outY = 1.0 - tv;  // reference flips v on return
    return true;
}

void MilkdropVisualizer::drawMotionVectors(const FrameVars& fv)
{
    if (fv.mvA < 0.001) return;

    int nX = static_cast<int>(fv.mvX);
    int nY = static_cast<int>(fv.mvY);
    double fracX = fv.mvX - nX;
    double fracY = fv.mvY - nY;
    if (nX > 64) { nX = 64; fracX = 0.0; }
    if (nY > 48) { nY = 48; fracY = 0.0; }
    if (nX <= 0 || nY <= 0) return;
    fracX = clampd(fracX, 0.0, 1.0);
    fracY = clampd(fracY, 0.0, 1.0);

    const double minLen = 1.0 / std::max(1, width());

    std::vector<SuperscopePoint> pts;
    pts.reserve(static_cast<std::size_t>(nX * nY) * 3);
    const auto pushSegment = [&](double x0, double y0, double x1, double y1) {
        // ScopeRenderer VERWIRFT skip-Punkte (Strip-Trenner) — deshalb ein
        // Dummy-Trennpunkt ZWISCHEN den Segmenten, nie auf einem echten Punkt
        // (Sichttest-Befund Session 39: sonst ueberlebt nur der erste Vektor)
        if (!pts.empty())
        {
            SuperscopePoint separator;
            separator.skip = true;
            pts.push_back(separator);
        }
        SuperscopePoint a;
        a.x = static_cast<float>(x0);
        a.y = static_cast<float>(y0);
        SuperscopePoint b;
        b.x = static_cast<float>(x1);
        b.y = static_cast<float>(y1);
        pts.push_back(a);
        pts.push_back(b);
    };

    for (int y = 0; y < nY; ++y)
    {
        double fy = (y + 0.25) / (nY + fracY + 0.25 - 1.0);
        fy -= fv.mvDY;
        if (fy <= 0.0001 || fy >= 0.9999) continue;
        for (int x = 0; x < nX; ++x)
        {
            double fx = (x + 0.25) / (nX + fracX + 0.25 - 1.0);
            fx += fv.mvDX;
            if (fx <= 0.0001 || fx >= 0.9999) continue;

            double fx2 = fx;
            double fy2 = fy;
            if (!reversePropagate(fx, fy, fx2, fy2)) continue;

            // enforce minimum trail lengths (:1256-1282)
            double dx = (fx2 - fx) * fv.mvL;
            double dy = (fy2 - fy) * fv.mvL;
            const double len = std::sqrt(dx * dx + dy * dy);
            if (len <= minLen)
            {
                if (len > 1e-8)
                {
                    const double scale = minLen / len;
                    dx *= scale;
                    dy *= scale;
                }
                else
                {
                    dx = minLen;
                    dy = minLen;
                }
            }
            pushSegment(fx * 2.0 - 1.0, fy * 2.0 - 1.0, (fx + dx) * 2.0 - 1.0,
                        (fy + dy) * 2.0 - 1.0);
        }
    }
    if (pts.empty()) return;

    for (SuperscopePoint& pt : pts)
    {
        if (pt.skip) continue;
        pt.r = static_cast<float>(clampd(fv.mvR, 0.0, 1.0));
        pt.g = static_cast<float>(clampd(fv.mvG, 0.0, 1.0));
        pt.b = static_cast<float>(clampd(fv.mvB, 0.0, 1.0));
        pt.a = static_cast<float>(clampd(fv.mvA, 0.0, 1.0));
    }

    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    lumi::render::ScopeRenderer::Params params;
    params.mode = SuperscopeRenderMode::Lines;
    m_scope.draw(pts, params);
    f->glDisable(GL_BLEND);
}

// =============================================================================================
// Custom shapes (DrawCustomShapes port, milkdropfs.cpp:2215-2397)
// =============================================================================================

void MilkdropVisualizer::drawCustomShapes()
{
    if (m_shapeRt.empty() || m_context == nullptr) return;
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

    for (ShapeRuntime& rt : m_shapeRt)
    {
        const int instances = std::clamp(rt.def.instances, 1, 1024);
        for (int inst = 0; inst < instances; ++inst)
        {
            // q = post-per_frame values; t1-t8 = post-init (NOT persistent)
            m_context->restoreFrameSnapshot();
            auto& e = rt.script->engine();
            pushCommonInputs(e);
            for (int k = 0; k < 8; ++k)
            {
                e.setNumber(("t" + std::to_string(k + 1)).c_str(),
                            rt.tInit[static_cast<std::size_t>(k)]);
            }
            const lumi::milkdrop::ShapeState& d = rt.def;
            e.setNumber("x", d.x);
            e.setNumber("y", d.y);
            e.setNumber("rad", d.rad);
            e.setNumber("ang", d.ang);
            e.setNumber("tex_zoom", d.texZoom);
            e.setNumber("tex_ang", d.texAng);
            e.setNumber("sides", static_cast<double>(d.sides));
            e.setNumber("additive", d.additive ? 1.0 : 0.0);
            e.setNumber("textured", d.textured ? 1.0 : 0.0);
            e.setNumber("num_inst", static_cast<double>(instances));
            e.setNumber("instance", static_cast<double>(inst));
            e.setNumber("thick", d.thickOutline ? 1.0 : 0.0);
            e.setNumber("r", d.r);
            e.setNumber("g", d.g);
            e.setNumber("b", d.b);
            e.setNumber("a", d.a);
            e.setNumber("r2", d.r2);
            e.setNumber("g2", d.g2);
            e.setNumber("b2", d.b2);
            e.setNumber("a2", d.a2);
            e.setNumber("border_r", d.borderR);
            e.setNumber("border_g", d.borderG);
            e.setNumber("border_b", d.borderB);
            e.setNumber("border_a", d.borderA);

            if (rt.script->has(Slot::Frame)) rt.script->run(Slot::Frame);

            const int sides = std::clamp(static_cast<int>(e.number("sides")), 3, 100);
            const double cxN = e.number("x") * 2.0 - 1.0;
            const double cyN = e.number("y") * -2.0 + 1.0;
            const double rad = e.number("rad");
            const double ang = e.number("ang");
            const double texZoom = std::max(0.001, e.number("tex_zoom"));
            const double texAng = e.number("tex_ang");
            const bool textured = e.number("textured") > 0.5;
            const bool additive = e.number("additive") > 0.5;
            const bool thick = e.number("thick") > 0.5;
            const float cr = static_cast<float>(clampd(e.number("r"), 0.0, 1.0));
            const float cg = static_cast<float>(clampd(e.number("g"), 0.0, 1.0));
            const float cb = static_cast<float>(clampd(e.number("b"), 0.0, 1.0));
            const float ca = static_cast<float>(clampd(e.number("a"), 0.0, 1.0));
            const float er = static_cast<float>(clampd(e.number("r2"), 0.0, 1.0));
            const float eg = static_cast<float>(clampd(e.number("g2"), 0.0, 1.0));
            const float eb = static_cast<float>(clampd(e.number("b2"), 0.0, 1.0));
            const float ea = static_cast<float>(clampd(e.number("a2"), 0.0, 1.0));

            f->glEnable(GL_BLEND);
            f->glBlendFunc(GL_SRC_ALPHA, additive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);

            // triangle fan: center (rgba) -> ring (r2g2b2a2); ring angle base +45°
            std::vector<float> fan;  // textured: pos2+uv2+col4; untextured: pos2+col4
            const int stride = textured ? 8 : 6;
            fan.reserve(static_cast<std::size_t>((sides + 2) * stride));
            const auto pushVert = [&](double px, double py, double tu, double tv, float r,
                                      float g, float b, float a) {
                fan.push_back(static_cast<float>(px));
                fan.push_back(static_cast<float>(py));
                if (textured)
                {
                    fan.push_back(static_cast<float>(tu));
                    fan.push_back(static_cast<float>(1.0 - tv));  // D3D v -> GL v
                }
                fan.push_back(r);
                fan.push_back(g);
                fan.push_back(b);
                fan.push_back(a);
            };
            pushVert(cxN, cyN, 0.5, 0.5, cr, cg, cb, ca);
            std::vector<std::pair<double, double>> ring(static_cast<std::size_t>(sides + 1));
            for (int j = 0; j <= sides; ++j)
            {
                const double t = static_cast<double>(j % sides) / sides;
                const double theta = t * 6.283185307 + ang + 0.785398163;  // +pi/4
                const double px = cxN + rad * std::cos(theta) * m_aspectY;
                const double py = cyN + rad * std::sin(theta);
                ring[static_cast<std::size_t>(j)] = {px, py};
                const double tu =
                    0.5 + 0.5 * std::cos(t * 6.283185307 + texAng + 0.785398163) / texZoom *
                              m_aspectY;
                const double tv =
                    0.5 + 0.5 * std::sin(t * 6.283185307 + texAng + 0.785398163) / texZoom;
                pushVert(px, py, tu, tv, er, eg, eb, ea);
            }

            if (textured)
            {
                m_shapeProgram->bind();
                f->glActiveTexture(GL_TEXTURE0);
                f->glBindTexture(GL_TEXTURE_2D, m_feedback.previousTexture());
                // PORT: samples the PREVIOUS frame (original: current source
                // surface) — one frame older, visually equivalent in feedback
                f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                m_shapeProgram->setUniformValue("uTex", 0);
                m_shapeVao->bind();
                m_shapeVbo->bind();
                m_shapeVbo->allocate(fan.data(),
                                     static_cast<int>(fan.size() * sizeof(float)));
                f->glDrawArrays(GL_TRIANGLE_FAN, 0, sides + 2);
                m_shapeVbo->release();
                m_shapeVao->release();
                m_shapeProgram->release();
            }
            else
            {
                drawColorQuads(fan.data(), sides + 2, GL_TRIANGLE_FAN);
            }

            // border outline (line strip around the ring, thickOutline = 4x)
            const float borderAlpha =
                static_cast<float>(clampd(e.number("border_a"), 0.0, 1.0));
            if (borderAlpha > 0.001f)
            {
                std::vector<SuperscopePoint> outline(ring.size());
                for (std::size_t j = 0; j < ring.size(); ++j)
                {
                    outline[j].x = static_cast<float>(ring[j].first);
                    outline[j].y = static_cast<float>(ring[j].second);
                    outline[j].r = static_cast<float>(clampd(e.number("border_r"), 0.0, 1.0));
                    outline[j].g = static_cast<float>(clampd(e.number("border_g"), 0.0, 1.0));
                    outline[j].b = static_cast<float>(clampd(e.number("border_b"), 0.0, 1.0));
                    outline[j].a = borderAlpha;
                }
                lumi::render::ScopeRenderer::Params params;
                params.mode = SuperscopeRenderMode::Lines;
                const int its = thick ? 4 : 1;
                const float xInc = 2.0f / std::max(1, width());
                const float yInc = 2.0f / std::max(1, height());
                for (int it = 0; it < its; ++it)
                {
                    for (SuperscopePoint& pt : outline)
                    {
                        if (it == 1) pt.x += xInc;
                        if (it == 2) pt.y += yInc;
                        if (it == 3) pt.x -= xInc;
                    }
                    m_scope.draw(outline, params);
                }
            }
            f->glDisable(GL_BLEND);
        }
    }
}

// =============================================================================================
// Custom waves (DrawCustomWaves port, milkdropfs.cpp:2496-2680)
// =============================================================================================

void MilkdropVisualizer::drawCustomWaves()
{
    if (m_waveRt.empty() || m_context == nullptr) return;
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    const int w = std::max(16, width());
    const int h = std::max(16, height());

    // PORT scale factors: reference waveform is +-128 (ours +-1) and its
    // spectrum magnitudes are unnormalised FFT sums (ours 0..~1) — both are
    // Sichttest calibration points.
    // Kalibrier-Runde 1 (Session 39, Presets m4/02+03): Wave 128 -> 192
    // (nach Aspect-Fix ~±0.16 statt ±0.25), Spektrum 32 -> 8 (Bass lief oben raus)
    constexpr double kWavePortScale = 192.0;
    constexpr double kSpecPortScale = 8.0;

    for (WaveRuntime& rt : m_waveRt)
    {
        const lumi::milkdrop::WaveState& d = rt.def;

        m_context->restoreFrameSnapshot();
        auto& e = rt.script->engine();
        pushCommonInputs(e);
        for (int k = 0; k < 8; ++k)
        {
            e.setNumber(("t" + std::to_string(k + 1)).c_str(),
                        rt.tInit[static_cast<std::size_t>(k)]);
        }
        e.setNumber("r", d.r);
        e.setNumber("g", d.g);
        e.setNumber("b", d.b);
        e.setNumber("a", d.a);
        e.setNumber("samples", static_cast<double>(d.samples));

        if (rt.script->has(Slot::Frame)) rt.script->run(Slot::Frame);

        const int maxSamples = d.spectrum ? 512 : kWaveSamples;
        const int sep = std::clamp(d.sep, 0, maxSamples - 2);
        int nSamples = std::clamp(static_cast<int>(e.number("samples")), 1, 512);
        nSamples = std::min(nSamples, maxSamples - sep);
        const bool dots = d.useDots;
        if (nSamples < 2 && !(dots && nSamples >= 1)) continue;

        const double frameR = e.number("r");
        const double frameG = e.number("g");
        const double frameB = e.number("b");
        const double frameA = e.number("a");

        // --- sample data (spec 1.4): source, sep shift, forward+backward IIR ---------
        const double mult = (d.spectrum ? 0.15 * kSpecPortScale
                                        : 0.004 * kWavePortScale) *
                            d.scaling * m_state.waveScale;
        const double mix1 = std::sqrt(clampd(d.smoothing, 0.0, 1.0) * 0.98);
        const double mix2 = 1.0 - mix1;

        std::array<std::array<double, 512>, 2> data{};
        const int specN0 = static_cast<int>(m_spectrumL.size());
        const int specN1 = static_cast<int>(m_spectrumR.size());
        const auto sourceSample = [&](int channel, int refIndex) -> double {
            if (d.spectrum)
            {
                const int n = (channel == 0) ? specN0 : specN1;
                if (n < 1) return 0.0;
                // PORT: Referenz-Spektrum deckt nur ~0..11 kHz ab, unseres
                // 0..22 kHz — auf die UNTERE Haelfte mappen, sonst zeigt die
                // rechte Bildhaelfte 11-22 kHz und der oberste Bin spikt
                // (Sichttest-Befund Session 39, Preset m4/03)
                const int idx = std::clamp(refIndex * (n / 2) / 512, 0, n - 1);
                const std::vector<float>& spec = (channel == 0) ? m_spectrumL : m_spectrumR;
                return spec[static_cast<std::size_t>(idx)];
            }
            const std::array<float, kWaveBuffer>& wave =
                (channel == 0) ? m_waveRawL : m_waveRawR;
            return wave[static_cast<std::size_t>(std::clamp(refIndex, 0, kWaveBuffer - 1))];
        };

        int j0 = 0;
        int j1 = 0;
        double step = 1.0;
        if (d.spectrum)
        {
            step = static_cast<double>(maxSamples - sep) / nSamples;
        }
        else
        {
            j0 = (maxSamples - nSamples) / 2 - sep / 2;
            j1 = (maxSamples - nSamples) / 2 + sep / 2;
        }
        data[0][0] = sourceSample(0, j0);
        data[1][0] = sourceSample(1, j1);
        for (int j = 1; j < nSamples; ++j)
        {
            data[0][static_cast<std::size_t>(j)] =
                sourceSample(0, static_cast<int>(j * step) + j0) * mix2 +
                data[0][static_cast<std::size_t>(j - 1)] * mix1;
            data[1][static_cast<std::size_t>(j)] =
                sourceSample(1, static_cast<int>(j * step) + j1) * mix2 +
                data[1][static_cast<std::size_t>(j - 1)] * mix1;
        }
        for (int j = nSamples - 2; j >= 0; --j)
        {
            data[0][static_cast<std::size_t>(j)] =
                data[0][static_cast<std::size_t>(j)] * mix2 +
                data[0][static_cast<std::size_t>(j + 1)] * mix1;
            data[1][static_cast<std::size_t>(j)] =
                data[1][static_cast<std::size_t>(j)] * mix2 +
                data[1][static_cast<std::size_t>(j + 1)] * mix1;
        }
        for (int j = 0; j < nSamples; ++j)
        {
            data[0][static_cast<std::size_t>(j)] *= mult;
            data[1][static_cast<std::size_t>(j)] *= mult;
        }

        // --- per_point loop (spec 1.5) --------------------------------------------------
        const bool hasPoint = rt.script->has(Slot::Point);
        const double jMult = (nSamples > 1) ? 1.0 / (nSamples - 1) : 0.0;
        std::vector<SuperscopePoint> pts(static_cast<std::size_t>(nSamples));
        for (int j = 0; j < nSamples; ++j)
        {
            const double value1 = data[0][static_cast<std::size_t>(j)];
            const double value2 = data[1][static_cast<std::size_t>(j)];
            double px = 0.5 + value1;
            double py = 0.5 + value2;
            double pr = frameR;
            double pg = frameG;
            double pb = frameB;
            double pa = frameA;
            if (hasPoint)
            {
                e.setNumber("sample", j * jMult);
                e.setNumber("value1", value1);
                e.setNumber("value2", value2);
                e.setNumber("x", px);
                e.setNumber("y", py);
                e.setNumber("r", pr);
                e.setNumber("g", pg);
                e.setNumber("b", pb);
                e.setNumber("a", pa);
                rt.script->run(Slot::Point);
                px = e.number("x");
                py = e.number("y");
                pr = e.number("r");
                pg = e.number("g");
                pb = e.number("b");
                pa = e.number("a");
            }
            SuperscopePoint& pt = pts[static_cast<std::size_t>(j)];
            // PORT: KEINE InvAspect-Faktoren — die Referenz rechnet auf einer
            // QUADRATISCHEN Leinwand (Faktoren dort = 1); unser Buffer ist
            // fenstergross, Skript-x/y sind Bildschirm-Anteile 0..1
            // (Sichttest-Befund Session 39: 1.73x-Streckung ohne diesen Fix)
            pt.x = static_cast<float>(px * 2.0 - 1.0);
            pt.y = static_cast<float>(py * -2.0 + 1.0);
            pt.r = static_cast<float>(clampd(pr, 0.0, 1.0));
            pt.g = static_cast<float>(clampd(pg, 0.0, 1.0));
            pt.b = static_cast<float>(clampd(pb, 0.0, 1.0));
            // PORT: clamp statt Original-Byte-Wrap (&0xFF) — Wrap waere ein Artefakt
            pt.a = static_cast<float>(clampd(pa, 0.0, 1.0));
        }

        // one smoothing pass unless dots (same coefficients as the basic wave)
        std::vector<SuperscopePoint> drawPts;
        if (!dots && pts.size() >= 2)
        {
            drawPts.reserve(pts.size() * 2);
            smoothWaveSegment(pts, 0, static_cast<int>(pts.size()), drawPts);
        }
        else
        {
            drawPts = pts;
        }

        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_SRC_ALPHA, d.additive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
        lumi::render::ScopeRenderer::Params params;
        params.mode = dots ? SuperscopeRenderMode::Dots : SuperscopeRenderMode::Lines;
        params.dotSize =
            static_cast<float>((w >= 1024 ? 2 : 1) + (d.drawThick ? 1 : 0));
        const int its = (d.drawThick && !dots) ? 4 : 1;
        const float xInc = 2.0f / w;
        const float yInc = 2.0f / h;
        for (int it = 0; it < its; ++it)
        {
            for (SuperscopePoint& pt : drawPts)
            {
                if (it == 1) pt.x += xInc;
                if (it == 2) pt.y += yInc;
                if (it == 3) pt.x -= xInc;
            }
            m_scope.draw(drawPts, params);
        }
        f->glDisable(GL_BLEND);
    }
}

// =============================================================================================
// Basic waveform (DrawWave port, milkdropfs.cpp:2682-3298)
// =============================================================================================

void MilkdropVisualizer::drawBasicWave(const FrameVars& fv)
{
    const int w = std::max(16, width());
    const int h = std::max(16, height());
    const float* fL = m_waveL.data();
    const float* fR = m_waveR.data();

    // colour (clamped; wave_brighten normalises to full brightness)
    double cr = clampd(fv.waveR, 0.0, 1.0);
    double cg = clampd(fv.waveG, 0.0, 1.0);
    double cb = clampd(fv.waveB, 0.0, 1.0);
    if (fv.waveBrighten)
    {
        const double maxc = std::max(cr, std::max(cg, cb));
        if (maxc > 0.01)
        {
            cr /= maxc;
            cg /= maxc;
            cb /= maxc;
        }
    }

    const double wavePosX = fv.waveX * 2.0 - 1.0;
    const double wavePosY = fv.waveY * 2.0 - 1.0;
    const int mode = ((fv.waveMode % 8) + 8) % 8;

    // fWaveParam2 folding (modes 0/1/4, :2786-2794)
    double waveParam2 = fv.waveMystery;
    if ((mode == 0 || mode == 1 || mode == 4) &&
        (waveParam2 < -1.0 || waveParam2 > 1.0))
    {
        double p = waveParam2 * 0.5 + 0.5;
        p -= std::floor(p);
        p = std::fabs(p);
        waveParam2 = p * 2.0 - 1.0;
    }

    double alpha = fv.waveA;
    const auto modAlphaByVolume = [&]() {
        if (m_state.modWaveAlphaByVolume)
        {
            const double vol =
                (m_loudness.bass() + m_loudness.mid() + m_loudness.treb()) * 0.333;
            const double den = m_state.modWaveAlphaEnd - m_state.modWaveAlphaStart;
            alpha *= (vol - m_state.modWaveAlphaStart) / ((std::fabs(den) < 1e-9) ? 1e-9 : den);
        }
        alpha = clampd(alpha, 0.0, 1.0);
    };

    std::vector<SuperscopePoint> pts;
    int nBreak = -1;

    switch (mode)
    {
        case 0:  // circular wave
        {
            const int nVerts = kWaveSamples / 2;
            const int off = (kWaveSamples - nVerts) / 2;
            const double inv = 1.0 / (nVerts - 1);
            modAlphaByVolume();
            pts.resize(static_cast<std::size_t>(nVerts));
            for (int i = 0; i < nVerts; ++i)
            {
                double rad = 0.5 + 0.4 * fR[i + off] + waveParam2;
                const double ang = i * inv * 6.28 + m_time * 0.2;
                if (i < nVerts / 10)
                {
                    double mix = i / (nVerts * 0.1);
                    mix = 0.5 - 0.5 * std::cos(mix * 3.1416);
                    const double rad2 = 0.5 + 0.4 * fR[i + nVerts + off] + waveParam2;
                    rad = rad2 * (1.0 - mix) + rad * mix;
                }
                pts[static_cast<std::size_t>(i)].x =
                    static_cast<float>(rad * std::cos(ang) * m_aspectY + wavePosX);
                pts[static_cast<std::size_t>(i)].y =
                    static_cast<float>(rad * std::sin(ang) * m_aspectX + wavePosY);
            }
            pts.push_back(pts.front());  // close the circle (non-blending path)
            break;
        }
        case 1:  // x-y osc that goes around in a spiral, in time
        {
            alpha *= 1.25;
            modAlphaByVolume();
            const int nVerts = kWaveSamples / 2;
            pts.resize(static_cast<std::size_t>(nVerts));
            for (int i = 0; i < nVerts; ++i)
            {
                const double rad = 0.53 + 0.43 * fR[i] + waveParam2;
                const double ang = fL[i + 32] * 1.57 + m_time * 2.3;
                pts[static_cast<std::size_t>(i)].x =
                    static_cast<float>(rad * std::cos(ang) * m_aspectY + wavePosX);
                pts[static_cast<std::size_t>(i)].y =
                    static_cast<float>(rad * std::sin(ang) * m_aspectX + wavePosY);
            }
            break;
        }
        case 2:  // centered spiro (alpha constant)
        case 3:  // centered spiro (alpha tied to volume)
        {
            if (mode == 2)
            {
                alpha *= sizeBucketAlpha(w, 0.07, 0.09, 0.11, 0.13);
            }
            else
            {
                alpha = sizeBucketAlpha(w, 0.075, 0.150, 0.220, 0.330);
                alpha *= 1.3;
                alpha *= m_loudness.treb() * m_loudness.treb();
            }
            modAlphaByVolume();
            pts.resize(static_cast<std::size_t>(kWaveSamples));
            for (int i = 0; i < kWaveSamples; ++i)
            {
                pts[static_cast<std::size_t>(i)].x =
                    static_cast<float>(fR[i] * m_aspectY + wavePosX);
                pts[static_cast<std::size_t>(i)].y =
                    static_cast<float>(fL[i + 32] * m_aspectX + wavePosY);
            }
            break;
        }
        case 4:  // horizontal "script", left channel
        {
            modAlphaByVolume();
            const int nVerts = std::min(kWaveSamples, w / 3);
            const int off = (kWaveSamples - nVerts) / 2;
            const double w1 = 0.45 + 0.5 * (waveParam2 * 0.5 + 0.5);
            const double w2 = 1.0 - w1;
            const double inv = 1.0 / nVerts;
            pts.resize(static_cast<std::size_t>(nVerts));
            for (int i = 0; i < nVerts; ++i)
            {
                double x = -1.0 + 2.0 * (i * inv) + wavePosX;
                double y = fL[i + off] * 0.47 + wavePosY;
                x += fR[i + 25 + off] * 0.44;
                if (i > 1)
                {
                    x = x * w2 + w1 * (pts[static_cast<std::size_t>(i - 1)].x * 2.0 -
                                       pts[static_cast<std::size_t>(i - 2)].x);
                    y = y * w2 + w1 * (pts[static_cast<std::size_t>(i - 1)].y * 2.0 -
                                       pts[static_cast<std::size_t>(i - 2)].y);
                }
                pts[static_cast<std::size_t>(i)].x = static_cast<float>(x);
                pts[static_cast<std::size_t>(i)].y = static_cast<float>(y);
            }
            break;
        }
        case 5:  // explosive complex thing, rotating
        {
            alpha *= sizeBucketAlpha(w, 0.07, 0.09, 0.11, 0.13);
            modAlphaByVolume();
            const double cs = std::cos(m_time * 0.3);
            const double sn = std::sin(m_time * 0.3);
            pts.resize(static_cast<std::size_t>(kWaveSamples));
            for (int i = 0; i < kWaveSamples; ++i)
            {
                const double x0 = fR[i] * fL[i + 32] + fL[i] * fR[i + 32];
                const double y0 = fR[i] * fR[i] - fL[i + 32] * fL[i + 32];
                pts[static_cast<std::size_t>(i)].x =
                    static_cast<float>((x0 * cs - y0 * sn) * m_aspectY + wavePosX);
                pts[static_cast<std::size_t>(i)].y =
                    static_cast<float>((x0 * sn + y0 * cs) * m_aspectX + wavePosY);
            }
            break;
        }
        default:  // 6 / 7: angle-adjustable line(s)
        {
            modAlphaByVolume();
            int nVerts = std::min(kWaveSamples / 2, w / 3);
            const int off = (kWaveSamples - nVerts) / 2;
            const double ang = 1.57 * waveParam2;
            double dxDir = std::cos(ang);
            double dyDir = std::sin(ang);
            float ex0 = static_cast<float>(wavePosX * std::cos(ang + 1.57) - dxDir * 3.0);
            float ey0 = static_cast<float>(wavePosX * std::sin(ang + 1.57) - dyDir * 3.0);
            float ex1 = static_cast<float>(wavePosX * std::cos(ang + 1.57) + dxDir * 3.0);
            float ey1 = static_cast<float>(wavePosX * std::sin(ang + 1.57) + dyDir * 3.0);
            clipEdgeToRect(ex0, ey0, ex1, ey1);
            const double stepX = (ex1 - ex0) / nVerts;
            const double stepY = (ey1 - ey0) / nVerts;
            const double ang2 = std::atan2(stepY, stepX);
            const double perpX = std::cos(ang2 + 1.57);
            const double perpY = std::sin(ang2 + 1.57);

            if (mode == 6)
            {
                pts.resize(static_cast<std::size_t>(nVerts));
                for (int i = 0; i < nVerts; ++i)
                {
                    pts[static_cast<std::size_t>(i)].x = static_cast<float>(
                        ex0 + stepX * i + perpX * 0.25 * fL[i + off]);
                    pts[static_cast<std::size_t>(i)].y = static_cast<float>(
                        ey0 + stepY * i + perpY * 0.25 * fL[i + off]);
                }
            }
            else  // mode 7: two lines (stereo) with separation
            {
                const double sep = std::pow(wavePosY * 0.5 + 0.5, 2.0);
                pts.resize(static_cast<std::size_t>(nVerts) * 2);
                for (int i = 0; i < nVerts; ++i)
                {
                    pts[static_cast<std::size_t>(i)].x = static_cast<float>(
                        ex0 + stepX * i + perpX * (0.25 * fL[i + off] + sep));
                    pts[static_cast<std::size_t>(i)].y = static_cast<float>(
                        ey0 + stepY * i + perpY * (0.25 * fL[i + off] + sep));
                    pts[static_cast<std::size_t>(i + nVerts)].x = static_cast<float>(
                        ex0 + stepX * i + perpX * (0.25 * fR[i + off] - sep));
                    pts[static_cast<std::size_t>(i + nVerts)].y = static_cast<float>(
                        ey0 + stepY * i + perpY * (0.25 * fR[i + off] - sep));
                }
                nBreak = nVerts;
            }
            break;
        }
    }

    if (alpha < 0.004 || pts.size() < 2) return;

    // one smoothing pass (SmoothWave), segment-aware
    std::vector<SuperscopePoint> smoothed;
    smoothed.reserve(pts.size() * 2);
    if (nBreak < 0)
    {
        smoothWaveSegment(pts, 0, static_cast<int>(pts.size()), smoothed);
    }
    else
    {
        smoothWaveSegment(pts, 0, nBreak, smoothed);
        // Dummy-Trennpunkt: der ScopeRenderer verwirft skip-Punkte — ein
        // markierter ECHTER Punkt wuerde das erste Sample von Linie B kosten
        SuperscopePoint separator;
        separator.skip = true;
        smoothed.push_back(separator);
        smoothWaveSegment(pts, nBreak, static_cast<int>(pts.size()) - nBreak, smoothed);
    }

    for (SuperscopePoint& pt : smoothed)
    {
        pt.r = static_cast<float>(cr);
        pt.g = static_cast<float>(cg);
        pt.b = static_cast<float>(cb);
        pt.a = static_cast<float>(alpha);
    }

    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, fv.waveAdditive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);

    lumi::render::ScopeRenderer::Params params;
    params.mode = fv.waveDots ? SuperscopeRenderMode::Dots : SuperscopeRenderMode::Lines;
    params.dotSize = 1.0f;

    // thick/dots: 4 draws shifted by one texel (:3254-3294)
    const int drawIts = ((fv.waveThick || fv.waveDots) && w >= 512) ? 4 : 1;
    const float xInc = 2.0f / w;
    const float yInc = 2.0f / h;
    for (int it = 0; it < drawIts; ++it)
    {
        for (SuperscopePoint& pt : smoothed)
        {
            if (it == 1) pt.x += xInc;
            if (it == 2) pt.y += yInc;
            if (it == 3) pt.x -= xInc;
        }
        m_scope.draw(smoothed, params);
    }

    f->glDisable(GL_BLEND);
}

// =============================================================================================
// Borders + darken center (DrawSprites port, milkdropfs.cpp:3300-3405)
// =============================================================================================

void MilkdropVisualizer::drawColorQuads(const float* vertexData, int vertexCount,
                                        unsigned int glMode)
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    m_colorProgram->bind();
    m_quadVao->bind();
    m_quadVbo->bind();
    m_quadVbo->allocate(vertexData, vertexCount * 6 * static_cast<int>(sizeof(float)));
    f->glDrawArrays(static_cast<GLenum>(glMode), 0, vertexCount);
    m_quadVbo->release();
    m_quadVao->release();
    m_colorProgram->release();
}

void MilkdropVisualizer::drawBorders(const FrameVars& fv)
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

    const auto ring = [&](double outerRad, double innerRad, double r, double g, double b,
                          double a) {
        if (a <= 0.001) return;
        const float o = static_cast<float>(outerRad);
        const float in = static_cast<float>(innerRad);
        const float cr = static_cast<float>(r);
        const float cg = static_cast<float>(g);
        const float cb = static_cast<float>(b);
        const float ca = static_cast<float>(a);
        // four rectangles (top/bottom strips full width, left/right between)
        const float quads[4][4] = {
            {-o, in, o, o},    // top
            {-o, -o, o, -in},  // bottom
            {-o, -in, -in, in},
            {in, -in, o, in},
        };
        std::vector<float> data;
        data.reserve(4 * 6 * 6);
        for (const auto& q : quads)
        {
            const float x0 = q[0];
            const float y0 = q[1];
            const float x1 = q[2];
            const float y1 = q[3];
            const float corners[6][2] = {{x0, y0}, {x1, y0}, {x0, y1},
                                         {x1, y0}, {x1, y1}, {x0, y1}};
            for (const auto& c : corners)
            {
                data.insert(data.end(), {c[0], c[1], cr, cg, cb, ca});
            }
        }
        drawColorQuads(data.data(), static_cast<int>(data.size() / 6), GL_TRIANGLES);
    };

    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ring(1.0, 1.0 - fv.obSize, fv.obR, fv.obG, fv.obB, fv.obA);
    ring(1.0 - fv.obSize, 1.0 - fv.obSize - fv.ibSize, fv.ibR, fv.ibG, fv.ibB, fv.ibA);
    f->glDisable(GL_BLEND);
}

void MilkdropVisualizer::drawDarkenCenter()
{
    // small alpha-gradient diamond in the middle (:3310-3347)
    constexpr float kHalfSize = 0.05f;
    const float ax = kHalfSize * static_cast<float>(m_aspectY);
    const float centerAlpha = 3.0f / 32.0f;
    const float fan[4][2] = {{0.0f, -kHalfSize}, {ax, 0.0f}, {0.0f, kHalfSize}, {-ax, 0.0f}};
    std::vector<float> data;
    for (int tri = 0; tri < 4; ++tri)
    {
        const float(&p1)[2] = fan[tri];
        const float(&p2)[2] = fan[(tri + 1) % 4];
        data.insert(data.end(), {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, centerAlpha});
        data.insert(data.end(), {p1[0], p1[1], 0.0f, 0.0f, 0.0f, 0.0f});
        data.insert(data.end(), {p2[0], p2[1], 0.0f, 0.0f, 0.0f, 0.0f});
    }
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawColorQuads(data.data(), static_cast<int>(data.size() / 6), GL_TRIANGLES);
    f->glDisable(GL_BLEND);
}

// =============================================================================================
// MD1 composite (ShowToUser_NoShaders port, milkdropfs.cpp:3967-4287)
// =============================================================================================

void MilkdropVisualizer::compositeToScreen(const FrameVars& fv)
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions* f = ctx->functions();

    f->glBindFramebuffer(GL_FRAMEBUFFER, ctx->defaultFramebufferObject());
    f->glViewport(0, 0, std::max(1, width()), std::max(1, height()));

    // Stufe C1: transpilierter Comp-Shader ersetzt den gesamten MD1-Composite
    if (m_compCustomProgram != nullptr)
    {
        f->glDisable(GL_BLEND);
        m_compCustomProgram->bind();
        feedCustomUniforms(*m_compCustomProgram, fv, m_feedback.currentTexture());
        // Praesentations-Quad wie der MD1-Basis-Layer (Identitaets-Mapping)
        const float quad[6][4] = {{-1.0f, -1.0f, 0.0f, 0.0f}, {1.0f, -1.0f, 1.0f, 0.0f},
                                  {-1.0f, 1.0f, 0.0f, 1.0f},  {1.0f, -1.0f, 1.0f, 0.0f},
                                  {1.0f, 1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, 0.0f, 1.0f}};
        m_meshVao->bind();
        m_meshVbo->bind();
        m_meshVbo->allocate(quad, sizeof(quad));
        f->glDrawArrays(GL_TRIANGLES, 0, 6);
        m_meshVbo->release();
        m_meshVao->release();
        m_compCustomProgram->release();
        return;
    }

    m_textureProgram->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, m_feedback.currentTexture());
    const GLint wrapMode = fv.wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_textureProgram->setUniformValue("uTex", 0);

    // fullscreen quad with per-layer UVs; the SINGLE vertical flip lives in vGl
    const auto drawLayer = [&](double zoomAmount, bool echoFlips, int orient, double colorMul) {
        const double lo = 0.5 - 0.5 / zoomAmount;
        const double hi = 0.5 + 0.5 / zoomAmount;
        float quad[6][4];
        const float pos[6][2] = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f},
                                 {1.0f, -1.0f},  {1.0f, 1.0f}, {-1.0f, 1.0f}};
        for (int i = 0; i < 6; ++i)
        {
            const double px = pos[i][0];
            const double py = pos[i][1];
            double u = (px < 0.0) ? lo : hi;
            double vRef = (py < 0.0) ? hi : lo;  // presentation flip (math space -> screen)
            if (echoFlips)
            {
                if ((orient & 1) != 0) u = 1.0 - u;
                if (orient >= 2) vRef = 1.0 - vRef;
            }
            quad[i][0] = pos[i][0];
            quad[i][1] = pos[i][1];
            quad[i][2] = static_cast<float>(u);
            quad[i][3] = static_cast<float>(1.0 - vRef);  // D3D v -> GL v
        }
        m_textureProgram->setUniformValue(
            "uColor", QVector4D(static_cast<float>(colorMul), static_cast<float>(colorMul),
                                static_cast<float>(colorMul), 1.0f));
        m_meshVao->bind();
        m_meshVbo->bind();
        m_meshVbo->allocate(quad, sizeof(quad));
        f->glDrawArrays(GL_TRIANGLES, 0, 6);
        m_meshVbo->release();
        m_meshVao->release();
    };

    f->glDisable(GL_BLEND);

    // stage B (M5): recognized comp shaders render with their BAKED constants —
    // in MD2 the composite constants live in the shader text, so per_frame
    // animation of gamma/echo has no effect there; MD1 presets stay live
    const lumi::milk::ShaderInfo& ci = m_state.compInfo;
    const bool baked = ci.shaderClass == lumi::milk::ShaderClass::Md1Default ||
                       ci.shaderClass == lumi::milk::ShaderClass::Md1Plus;
    const double echoAlpha = baked ? ci.echoAlpha : fv.echoAlpha;
    const double echoZoom = baked ? ci.echoZoom : fv.echoZoom;
    const int echoOrient = baked ? ci.echoOrient : fv.echoOrient;
    const double gamma = baked ? ci.gain : fv.gamma;
    const bool brighten = baked ? ci.brighten : fv.brighten;
    const bool darken = baked ? ci.darken : fv.darken;
    const bool solarize = baked ? ci.solarize : fv.solarize;
    const bool invert = baked ? ci.invert : fv.invert;

    const bool echo = echoAlpha > 0.001;
    if (echo)
    {
        // two layers: base (1-alpha) + zoomed/flipped echo (alpha); gamma via redraws
        for (int layer = 0; layer < 2; ++layer)
        {
            const double zoomAmount = (layer == 0) ? 1.0 : echoZoom;
            const double mix = (layer == 1) ? echoAlpha : 1.0 - echoAlpha;
            drawLayer(zoomAmount, layer == 1, echoOrient % 4, mix);
            if (layer == 0)
            {
                f->glEnable(GL_BLEND);
                f->glBlendFunc(GL_ONE, GL_ONE);
            }
            const int nRedraws = static_cast<int>(gamma - 0.0001);
            for (int r = 0; r < nRedraws; ++r)
            {
                const double g = (r == nRedraws - 1)
                                     ? gamma - static_cast<int>(gamma - 0.0001)
                                     : 1.0;
                drawLayer(zoomAmount, layer == 1, echoOrient % 4, g * mix);
            }
        }
    }
    else
    {
        const int nPasses = static_cast<int>(gamma - 0.001) + 1;
        for (int pass = 0; pass < nPasses; ++pass)
        {
            const double g = (pass == nPasses - 1) ? gamma - pass : 1.0;
            drawLayer(1.0, false, 0, g);
            if (pass == 0)
            {
                f->glEnable(GL_BLEND);
                f->glBlendFunc(GL_ONE, GL_ONE);
            }
        }
    }
    m_textureProgram->release();

    // stage B blur terms: coeff*GetBlurN added over the base (before the filters,
    // matching the generated statement order); GetBlurN un-biases the range
    // compression with (max-min)/min of THIS frame's safe ranges
    const int blurLevels = activeBlurLevels();
    if (blurLevels > 0 && m_blurTex[0] != 0)
    {
        const lumi::milkdrop::BlurRanges ranges = lumi::milkdrop::computeSafeBlurRanges(
            {static_cast<float>(fv.blurMin[0]), static_cast<float>(fv.blurMin[1]),
             static_cast<float>(fv.blurMin[2])},
            {static_cast<float>(fv.blurMax[0]), static_cast<float>(fv.blurMax[1]),
             static_cast<float>(fv.blurMax[2])});
        f->glEnable(GL_BLEND);
        f->glBlendFunc(GL_ONE, GL_ONE);
        m_blurLayerProgram->bind();
        m_blurLayerProgram->setUniformValue("uTex", 0);
        const float quad[6][4] = {{-1.0f, -1.0f, 0.0f, 0.0f}, {1.0f, -1.0f, 1.0f, 0.0f},
                                  {-1.0f, 1.0f, 0.0f, 1.0f},  {1.0f, -1.0f, 1.0f, 0.0f},
                                  {1.0f, 1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, 0.0f, 1.0f}};
        for (int n = 0; n < 3; ++n)
        {
            const auto idx = static_cast<std::size_t>(n);
            const double coeff = ci.blurAdd[idx];
            if (coeff <= 1e-9) continue;
            f->glActiveTexture(GL_TEXTURE0);
            f->glBindTexture(GL_TEXTURE_2D, m_blurTex[static_cast<std::size_t>(n * 2 + 1)]);
            m_blurLayerProgram->setUniformValue(
                "uScale", static_cast<float>(coeff * (ranges.max[idx] - ranges.min[idx])));
            m_blurLayerProgram->setUniformValue(
                "uBias", static_cast<float>(coeff * ranges.min[idx]));
            m_meshVao->bind();
            m_meshVbo->bind();
            m_meshVbo->allocate(quad, sizeof(quad));
            f->glDrawArrays(GL_TRIANGLES, 0, 6);
            m_meshVbo->release();
            m_meshVao->release();
        }
        m_blurLayerProgram->release();
    }

    // post filters: fullscreen white quads with destination-blend tricks (:4185-4283)
    const bool anyFilter = brighten || darken || solarize || invert;
    if (anyFilter)
    {
        std::vector<float> quad;
        const float pos[6][2] = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f},
                                 {1.0f, -1.0f},  {1.0f, 1.0f}, {-1.0f, 1.0f}};
        for (const auto& p : pos) quad.insert(quad.end(), {p[0], p[1], 1.0f, 1.0f, 1.0f, 1.0f});

        f->glEnable(GL_BLEND);
        const auto pass = [&](GLenum src, GLenum dst) {
            f->glBlendFunc(src, dst);
            drawColorQuads(quad.data(), 6, GL_TRIANGLES);
        };
        if (brighten)  // ~sqrt(colour): invert, square, invert
        {
            pass(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
            pass(GL_ZERO, GL_DST_COLOR);
            pass(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        }
        if (darken) pass(GL_ZERO, GL_DST_COLOR);  // colour^2
        if (solarize)
        {
            pass(GL_ZERO, GL_ONE_MINUS_DST_COLOR);
            pass(GL_DST_COLOR, GL_ONE);
        }
        if (invert) pass(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    }
    f->glDisable(GL_BLEND);
}

void MilkdropVisualizer::drawDebugGrid()
{
    // Reference grid for calibration (Session 40, Patrik's request): drawn on
    // the screen AFTER the composite, so it never enters the feedback loop and
    // never distorts — content moves, the grid stands still. 8x6 divisions in
    // the 0..1 preset space + a brighter centre cross.
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::vector<float> lines;
    const auto addLine = [&lines](float x0, float y0, float x1, float y1, float a) {
        const float rgba[4] = {1.0f, 1.0f, 1.0f, a};
        lines.insert(lines.end(), {x0, y0, rgba[0], rgba[1], rgba[2], rgba[3]});
        lines.insert(lines.end(), {x1, y1, rgba[0], rgba[1], rgba[2], rgba[3]});
    };

    constexpr int kCols = 8;
    constexpr int kRows = 6;
    constexpr float kMinorAlpha = 0.18f;
    constexpr float kCenterAlpha = 0.45f;
    for (int i = 0; i <= kCols; ++i)
    {
        const float x = -1.0f + 2.0f * static_cast<float>(i) / kCols;
        addLine(x, -1.0f, x, 1.0f, (i * 2 == kCols) ? kCenterAlpha : kMinorAlpha);
    }
    for (int j = 0; j <= kRows; ++j)
    {
        const float y = -1.0f + 2.0f * static_cast<float>(j) / kRows;
        addLine(-1.0f, y, 1.0f, y, (j * 2 == kRows) ? kCenterAlpha : kMinorAlpha);
    }

    drawColorQuads(lines.data(), static_cast<int>(lines.size() / 6), GL_LINES);
    f->glDisable(GL_BLEND);
}

// =============================================================================================
// Parameters
// =============================================================================================

std::vector<ModuleParamDesc> MilkdropVisualizer::paramDescs() const
{
    std::vector<ModuleParamDesc> descs;

    ModuleParamDesc meshX;
    meshX.id = "render.meshX";
    meshX.displayName = "Mesh X";
    meshX.group = "Warp Mesh";
    meshX.tooltip = "Warp-Gitter horizontal (Original-Default 32; hoeher = feinerer Warp)";
    meshX.stage = PipelineStage::Render;
    meshX.type = ParamType::Int;
    meshX.defaultValue = kDefaultMeshX;
    meshX.minValue = 8.0f;
    meshX.maxValue = static_cast<float>(kMaxMeshX);
    meshX.step = 1.0f;
    meshX.order = 0;
    descs.push_back(meshX);

    ModuleParamDesc meshY;
    meshY.id = "render.meshY";
    meshY.displayName = "Mesh Y";
    meshY.group = "Warp Mesh";
    meshY.tooltip = "Warp-Gitter vertikal (Original-Default 24)";
    meshY.stage = PipelineStage::Render;
    meshY.type = ParamType::Int;
    meshY.defaultValue = kDefaultMeshY;
    meshY.minValue = 6.0f;
    meshY.maxValue = static_cast<float>(kMaxMeshY);
    meshY.step = 1.0f;
    meshY.order = 1;
    descs.push_back(meshY);

    ModuleParamDesc grid;
    grid.id = "render.debugGrid";
    grid.displayName = "Kalibrier-Raster";
    grid.group = "Debug";
    grid.tooltip = "Referenz-Raster (8x6 + Mittelkreuz) als Overlay ueber dem Bild — "
                   "fuer Sichttests/Kalibrierung; geht NICHT in den Feedback-Loop ein";
    grid.stage = PipelineStage::Render;
    grid.type = ParamType::Bool;
    grid.defaultValue = false;
    grid.order = 2;
    descs.push_back(grid);

    return descs;
}

bool MilkdropVisualizer::getParam(const std::string& id, ParamValue& out) const
{
    if (id == "render.meshX")
    {
        out = m_meshX;
        return true;
    }
    if (id == "render.meshY")
    {
        out = m_meshY;
        return true;
    }
    if (id == "render.debugGrid")
    {
        out = m_debugGrid;
        return true;
    }
    return false;
}

bool MilkdropVisualizer::setParam(const std::string& id, const ParamValue& value)
{
    if (id == "render.debugGrid")
    {
        const bool* asBool = std::get_if<bool>(&value);
        if (asBool == nullptr) return false;
        m_debugGrid = *asBool;
        return true;
    }
    const int* asInt = std::get_if<int>(&value);
    if (asInt == nullptr) return false;
    if (id == "render.meshX")
    {
        m_meshX = std::clamp(*asInt, 8, kMaxMeshX);
        return true;
    }
    if (id == "render.meshY")
    {
        m_meshY = std::clamp(*asInt, 6, kMaxMeshY);
        return true;
    }
    return false;
}

void MilkdropVisualizer::resetToDefaults()
{
    m_meshX = kDefaultMeshX;
    m_meshY = kDefaultMeshY;
    m_debugGrid = false;
}
