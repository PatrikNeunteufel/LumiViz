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

#include <QFileInfo>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
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

// warp pass: previous frame sampled at the mesh UVs, dimmed by decay
const char* kWarpFragmentShader = R"(#version 330 core
uniform sampler2D uTex;
uniform float uDecay;
in vec2 vTex;
out vec4 frag;
void main() { frag = vec4(texture(uTex, vTex).rgb * uDecay, 1.0); }
)";

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
        if (!parsed.warpShader.empty() || !parsed.compShader.empty())
        {
            report->append(QStringLiteral(
                "Preset nutzt HLSL-Shader (PS%1) — MD1-Fallback aktiv (Stufe B folgt in M5)")
                               .arg(parsed.psVersion));
        }
    }

    m_state = lumi::milkdrop::translate(parsed);
    m_state.name = QFileInfo(path).completeBaseName().toStdString();

    rebuildScripts(report);
    m_time = 0.0;
    m_frame = 0;
    m_monitor = 0.0;
    m_initRan = false;
    return true;
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
    // blur ranges: defaults so reads are sane; rendering follows in M5
    e.setNumber("blur1_min", 0.0);
    e.setNumber("blur2_min", 0.0);
    e.setNumber("blur3_min", 0.0);
    e.setNumber("blur1_max", 1.0);
    e.setNumber("blur2_max", 1.0);
    e.setNumber("blur3_max", 1.0);
    e.setNumber("blur1_edge_darken", 0.25);
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
    m_warpProgram.reset();
    m_textureProgram.reset();
    m_colorProgram.reset();
    m_shapeProgram.reset();
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
    if (m_warpProgram == nullptr || m_textureProgram == nullptr ||
        m_colorProgram == nullptr || m_shapeProgram == nullptr)
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
    computeWarpMesh(fv);
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

    m_warpProgram->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, m_feedback.previousTexture());
    const GLint wrapMode = fv.wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_warpProgram->setUniformValue("uTex", 0);
    m_warpProgram->setUniformValue("uDecay", static_cast<float>(clampd(fv.decay, 0.0, 1.0)));

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

    const bool echo = fv.echoAlpha > 0.001;
    if (echo)
    {
        // two layers: base (1-alpha) + zoomed/flipped echo (alpha); gamma via redraws
        for (int layer = 0; layer < 2; ++layer)
        {
            const double zoomAmount = (layer == 0) ? 1.0 : fv.echoZoom;
            const double mix = (layer == 1) ? fv.echoAlpha : 1.0 - fv.echoAlpha;
            drawLayer(zoomAmount, layer == 1, fv.echoOrient % 4, mix);
            if (layer == 0)
            {
                f->glEnable(GL_BLEND);
                f->glBlendFunc(GL_ONE, GL_ONE);
            }
            const int nRedraws = static_cast<int>(fv.gamma - 0.0001);
            for (int r = 0; r < nRedraws; ++r)
            {
                const double g = (r == nRedraws - 1)
                                     ? fv.gamma - static_cast<int>(fv.gamma - 0.0001)
                                     : 1.0;
                drawLayer(zoomAmount, layer == 1, fv.echoOrient % 4, g * mix);
            }
        }
    }
    else
    {
        const int nPasses = static_cast<int>(fv.gamma - 0.001) + 1;
        for (int pass = 0; pass < nPasses; ++pass)
        {
            const double g = (pass == nPasses - 1) ? fv.gamma - pass : 1.0;
            drawLayer(1.0, false, 0, g);
            if (pass == 0)
            {
                f->glEnable(GL_BLEND);
                f->glBlendFunc(GL_ONE, GL_ONE);
            }
        }
    }
    m_textureProgram->release();

    // post filters: fullscreen white quads with destination-blend tricks (:4185-4283)
    const bool anyFilter = fv.brighten || fv.darken || fv.solarize || fv.invert;
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
        if (fv.brighten)  // ~sqrt(colour): invert, square, invert
        {
            pass(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
            pass(GL_ZERO, GL_DST_COLOR);
            pass(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        }
        if (fv.darken) pass(GL_ZERO, GL_DST_COLOR);  // colour^2
        if (fv.solarize)
        {
            pass(GL_ZERO, GL_ONE_MINUS_DST_COLOR);
            pass(GL_DST_COLOR, GL_ONE);
        }
        if (fv.invert) pass(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    }
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
    return false;
}

bool MilkdropVisualizer::setParam(const std::string& id, const ParamValue& value)
{
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
}
