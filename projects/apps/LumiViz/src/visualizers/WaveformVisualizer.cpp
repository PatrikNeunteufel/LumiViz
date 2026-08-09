/**
 ****************************************************************************************
 * @file   WaveformVisualizer.cpp
 * @brief  Advanced audio waveform visualizer with 8-stop gradient
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 4.0.0
 ****************************************************************************************
 */

#include "visualizers/WaveformVisualizer.hpp"
#include "visualizers/PipelineKeys.hpp"
#include "visualizers/VisualizerPresetManager.hpp"
#include "visualizers/modules/AudioUtil.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>
#include <map>
#include <string>

using namespace lumi::modules;

namespace
{

// =============================================================================
// 8-Stop Gradient Shader (same as PulsingVisualizer)
// =============================================================================

const char* LINE_VERTEX_SHADER = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in float aAmplitude;

out float vXPosition;
out float vAmplitude;

void main()
{
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vXPosition = (aPosition.x + 1.0) * 0.5;  // Normalize to [0,1]
    vAmplitude = aAmplitude;
}
)";

const char* LINE_FRAGMENT_SHADER = R"(
#version 330 core

in float vXPosition;
in float vAmplitude;

out vec4 fragColor;

// Up to 8 color stops
uniform vec4 uColor0;
uniform vec4 uColor1;
uniform vec4 uColor2;
uniform vec4 uColor3;
uniform vec4 uColor4;
uniform vec4 uColor5;
uniform vec4 uColor6;
uniform vec4 uColor7;
uniform vec4 uStopPos;      // Positions of stops 0-3
uniform vec4 uStopPos2;     // Positions of stops 4-7
uniform int uStopCount;     // Number of active stops (2-8)
uniform int uGradientMode;  // 0=Solid, 1=Linear, 2=Radial
uniform float uGradientAngle;
uniform float uAlpha;

vec4 getColor(int idx)
{
    if (idx == 0) return uColor0;
    if (idx == 1) return uColor1;
    if (idx == 2) return uColor2;
    if (idx == 3) return uColor3;
    if (idx == 4) return uColor4;
    if (idx == 5) return uColor5;
    if (idx == 6) return uColor6;
    return uColor7;
}

float getStopPos(int idx)
{
    if (idx < 4) {
        if (idx == 0) return uStopPos.x;
        if (idx == 1) return uStopPos.y;
        if (idx == 2) return uStopPos.z;
        return uStopPos.w;
    } else {
        if (idx == 4) return uStopPos2.x;
        if (idx == 5) return uStopPos2.y;
        if (idx == 6) return uStopPos2.z;
        return uStopPos2.w;
    }
}

vec4 sampleGradient(float t)
{
    t = clamp(t, 0.0, 1.0);
    
    if (uStopCount <= 1 || t <= getStopPos(0)) return uColor0;
    if (t >= getStopPos(uStopCount - 1)) return getColor(uStopCount - 1);
    
    for (int i = 0; i < uStopCount - 1; i++) {
        float pos0 = getStopPos(i);
        float pos1 = getStopPos(i + 1);
        
        if (t >= pos0 && t < pos1) {
            float localT = (t - pos0) / max(pos1 - pos0, 0.001);
            return mix(getColor(i), getColor(i + 1), localT);
        }
    }
    
    return uColor0;
}

void main()
{
    vec4 color;
    
    if (uGradientMode == 0)
    {
        // Solid color
        color = uColor0;
    }
    else if (uGradientMode == 1)
    {
        // Linear gradient based on X position
        float t = vXPosition;
        // Angle 0 = left to right, 180 = right to left
        float radians = uGradientAngle * 3.14159 / 180.0;
        if (cos(radians) < 0.0) {
            t = 1.0 - t;
        }
        color = sampleGradient(t);
    }
    else
    {
        // Radial - based on amplitude
        color = sampleGradient(vAmplitude);
    }
    
    fragColor = color;
    fragColor.a *= uAlpha;
}
)";

const char* FILL_VERTEX_SHADER = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;

out float vXPosition;

void main()
{
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vXPosition = (aPosition.x + 1.0) * 0.5;
}
)";

const char* FILL_FRAGMENT_SHADER = R"(
#version 330 core

in float vXPosition;

out vec4 fragColor;

uniform vec4 uColor0;
uniform vec4 uColor1;
uniform vec4 uColor2;
uniform vec4 uColor3;
uniform vec4 uColor4;
uniform vec4 uColor5;
uniform vec4 uColor6;
uniform vec4 uColor7;
uniform vec4 uStopPos;
uniform vec4 uStopPos2;
uniform int uStopCount;
uniform int uGradientMode;
uniform float uGradientAngle;
uniform float uAlpha;
uniform float uBrightness;

vec4 getColor(int idx)
{
    if (idx == 0) return uColor0;
    if (idx == 1) return uColor1;
    if (idx == 2) return uColor2;
    if (idx == 3) return uColor3;
    if (idx == 4) return uColor4;
    if (idx == 5) return uColor5;
    if (idx == 6) return uColor6;
    return uColor7;
}

float getStopPos(int idx)
{
    if (idx < 4) {
        if (idx == 0) return uStopPos.x;
        if (idx == 1) return uStopPos.y;
        if (idx == 2) return uStopPos.z;
        return uStopPos.w;
    } else {
        if (idx == 4) return uStopPos2.x;
        if (idx == 5) return uStopPos2.y;
        if (idx == 6) return uStopPos2.z;
        return uStopPos2.w;
    }
}

vec4 sampleGradient(float t)
{
    t = clamp(t, 0.0, 1.0);
    
    if (uStopCount <= 1 || t <= getStopPos(0)) return uColor0;
    if (t >= getStopPos(uStopCount - 1)) return getColor(uStopCount - 1);
    
    for (int i = 0; i < uStopCount - 1; i++) {
        float pos0 = getStopPos(i);
        float pos1 = getStopPos(i + 1);
        
        if (t >= pos0 && t < pos1) {
            float localT = (t - pos0) / max(pos1 - pos0, 0.001);
            return mix(getColor(i), getColor(i + 1), localT);
        }
    }
    
    return uColor0;
}

void main()
{
    vec4 color;
    
    if (uGradientMode == 0)
    {
        color = uColor0;
    }
    else if (uGradientMode == 1)
    {
        float t = vXPosition;
        float radians = uGradientAngle * 3.14159 / 180.0;
        if (cos(radians) < 0.0) {
            t = 1.0 - t;
        }
        color = sampleGradient(t);
    }
    else
    {
        color = uColor0;
    }
    
    // Apply brightness adjustment
    if (uBrightness > 0.0)
    {
        color.rgb = color.rgb + (1.0 - color.rgb) * uBrightness;
    }
    else
    {
        color.rgb = color.rgb * (1.0 + uBrightness);
    }
    
    fragColor = color;
    fragColor.a *= uAlpha;
}
)";

// =============================================================================
// Key-Schema (Phase 4 Schritt 5.3) -- module sub-id <-> pipeline key
// =============================================================================

// Scalar sub-ids of WaveformModule -> new pipeline keys (Parameter_Key_Migration.md §3).
// Gradient blocks are handled by prefix (monoColor. -> color.mono. etc., below).
const std::map<std::string, std::string>& subIdKeyTable()
{
    static const std::map<std::string, std::string> table = {
        {"channelMode", "map.channelMode"},
        {"sampleCount", "map.sampleCount"},
        {"monoOffset", "render.mono.offset"},
        {"monoAmplitude", "render.mono.amplitude"},
        {"leftOffset", "render.left.offset"},
        {"leftAmplitude", "render.left.amplitude"},
        {"rightOffset", "render.right.offset"},
        {"rightAmplitude", "render.right.amplitude"},
        {"displayWidth", "render.displayWidth"},
        {"lineStyle", "render.lineStyle"},
        {"monoLineWidth", "render.mono.lineWidth"},
        {"leftLineWidth", "render.left.lineWidth"},
        {"rightLineWidth", "render.right.lineWidth"},
        {"dashLength", "render.dashLength"},
        {"dashGap", "render.dashGap"},
        {"monoFillEnabled", "render.mono.fillEnabled"},
        {"monoFillOpacity", "render.mono.fillOpacity"},
        {"monoFillBrightness", "render.mono.fillBrightness"},
        {"leftFillEnabled", "render.left.fillEnabled"},
        {"leftFillOpacity", "render.left.fillOpacity"},
        {"leftFillBrightness", "render.left.fillBrightness"},
        {"rightFillEnabled", "render.right.fillEnabled"},
        {"rightFillOpacity", "render.right.fillOpacity"},
        {"rightFillBrightness", "render.right.fillBrightness"},
        {"mirrorEnabled", "post.mirror.enabled"},
        {"holdEnabled", "post.hold.enabled"},
        {"fadeTime", "post.hold.fadeTime"},
        {"maxHoldFrames", "post.hold.maxFrames"},
    };
    return table;
}

struct PrefixPair
{
    const char* subPrefix;  ///< module-internal gradient prefix
    const char* keyPrefix;  ///< pipeline color-handle prefix
};

constexpr PrefixPair kGradientPrefixes[] = {
    {"monoColor.", "color.mono."},
    {"leftColor.", "color.left."},
    {"rightColor.", "color.right."},
};

/// Module sub-id -> pipeline key ("" if unknown)
std::string subIdToKey(const std::string& subId)
{
    for (const auto& [subPrefix, keyPrefix] : kGradientPrefixes)
    {
        if (subId.rfind(subPrefix, 0) == 0)
        {
            return keyPrefix + subId.substr(std::string(subPrefix).size());
        }
    }
    auto it = subIdKeyTable().find(subId);
    return it == subIdKeyTable().end() ? std::string{} : it->second;
}

/// Pipeline key -> module sub-id ("" if unknown)
std::string keyToSubId(const std::string& key)
{
    for (const auto& [subPrefix, keyPrefix] : kGradientPrefixes)
    {
        if (key.rfind(keyPrefix, 0) == 0)
        {
            return subPrefix + key.substr(std::string(keyPrefix).size());
        }
    }
    static const std::map<std::string, std::string> reverse = [] {
        std::map<std::string, std::string> r;
        for (const auto& [subId, newKey] : subIdKeyTable())
        {
            r.emplace(newKey, subId);
        }
        return r;
    }();
    auto it = reverse.find(key);
    return it == reverse.end() ? std::string{} : it->second;
}

// stageForKey/groupForStage: shared helpers from visualizers/PipelineKeys.hpp

// =============================================================================
// Legacy-Key-Migration (Phase 4 Schritt 5.3)
// =============================================================================

/**
 * @brief Alias map old->new schema (Parameter_Key_Migration.md §3)
 *
 * Old keys were "waveform." + module sub-id -- generated from the same tables
 * that drive the live schema. Includes the audio.* identity whitelist, the
 * waveform.color.* legacy alias (§7.4) and the E3 value converter.
 */
void registerLegacyKeyAliases()
{
    std::map<std::string, std::string> aliases;

    // Stage 1: audio.* unchanged (identity) -- incl. audio.bands (E2: Equalizer only)
    for (const char* key : {"preset", "scale", "bands", "floorDb", "ceilDb", "clamp01",
                            "gain", "smooth.preset", "smooth.algorithm", "smooth.timeMs",
                            "smooth.windowSize", "smooth.primeFirstFrame"})
    {
        aliases.emplace(std::string("audio.") + key, std::string("audio.") + key);
    }

    // Scalars: waveform.<subId> -> new key
    for (const auto& [subId, newKey] : subIdKeyTable())
    {
        aliases.emplace("waveform." + subId, newKey);
    }

    // Gradient blocks: waveform.monoColor.* -> color.mono.* etc.; plus the
    // waveform.color.* legacy alias (mono, §7.4)
    for (const char* sub : {"mode", "solidColor", "angle", "preset", "editGradient",
                            "outlineWidth", "gradientPresetName", "gradientData"})
    {
        for (const auto& [subPrefix, keyPrefix] : kGradientPrefixes)
        {
            aliases.emplace(std::string("waveform.") + subPrefix + sub,
                            std::string(keyPrefix) + sub);
        }
        aliases.emplace(std::string("waveform.color.") + sub,
                        std::string("color.mono.") + sub);
    }

    lumi::VisualizerPresetManager::registerKeyAliases(QStringLiteral("waveform"),
                                                      std::move(aliases));

    // E3 (Hybrid): waveform.smoothing (EMA factor s) -> audio.smooth.timeMs,
    // timeMs ~ -16.67/ln(s) (60-FPS assumption); s <= 0 -> 0 ms, s >= 1 clamped
    lumi::VisualizerPresetManager::registerKeyConverter(
        QStringLiteral("waveform"), "waveform.smoothing", "audio.smooth.timeMs",
        [](const lumi::modules::ParamValue& value) -> lumi::modules::ParamValue {
            float s = 0.0f;
            if (auto* f = std::get_if<float>(&value)) s = *f;
            else if (auto* i = std::get_if<int>(&value)) s = static_cast<float>(*i);
            if (s <= 0.0f)
            {
                return 0.0f;  // no smoothing
            }
            const float clamped = std::min(s, 0.999f);
            return -16.67f / std::log(clamped);
        });
}

} // anonymous namespace

// =============================================================================
// Constructor / Destructor
// =============================================================================

WaveformVisualizer::WaveformVisualizer()
    : VisualizerBase(
          QStringLiteral("waveform"),
          QObject::tr("Waveform"),
          QObject::tr("Advanced audio waveform oscilloscope"))
{
    BasicLogger::logDebug("WaveformVisualizer: Constructor called");

    // Idempotent on every construction — a magic static would not survive
    // clearKeyAliases() (tests) since the registration would never re-fire
    registerLegacyKeyAliases();

    int sampleCount = m_waveform.sampleCount();
    m_displayLeft.resize(sampleCount, 0.0f);
    m_displayRight.resize(sampleCount, 0.0f);
    m_displayMono.resize(sampleCount, 0.0f);

    syncDisplaySmoothing();
}

WaveformVisualizer::~WaveformVisualizer()
{
    if (isInitialized())
    {
        cleanup();
    }
}

// =============================================================================
// Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> WaveformVisualizer::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    // Stage 1: Audio Source
    for (const auto& p : m_audioSource.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "Audio";
        prefixed.stage = PipelineStage::AudioSource;

        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "audio." + prefixed.dependsOn;
        }

        params.push_back(prefixed);
    }

    // Stages 2/3/4/6: module schema translated to the pipeline keys (5.3)
    for (const auto& p : m_waveform.paramDescs())
    {
        const std::string newKey = subIdToKey(p.id);
        if (newKey.empty())
        {
            BasicLogger::logWarning("WaveformVisualizer: No pipeline key for '" + p.id + "'");
            continue;
        }

        ModuleParamDesc prefixed = p;
        prefixed.id = newKey;
        prefixed.stage = lumi::stageForKey(newKey);
        prefixed.group = lumi::groupForStage(prefixed.stage);

        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = subIdToKey(prefixed.dependsOn);
        }

        params.push_back(prefixed);
    }

    return params;
}

bool WaveformVisualizer::getParam(const std::string& id, ParamValue& out) const
{
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }

    const std::string subId = keyToSubId(id);
    return subId.empty() ? false : m_waveform.getParam(subId, out);
}

bool WaveformVisualizer::setParam(const std::string& id, const ParamValue& value)
{
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }

    const std::string subId = keyToSubId(id);
    if (subId.empty())
    {
        return false;
    }

    bool result = m_waveform.setParam(subId, value);

    // Buffer-resize coupling (§7.2)
    if (result && id == "map.sampleCount")
    {
        int newCount = m_waveform.sampleCount();
        m_displayLeft.resize(newCount, 0.0f);
        m_displayRight.resize(newCount, 0.0f);
        m_displayMono.resize(newCount, 0.0f);
    }

    return result;
}

void WaveformVisualizer::resetToDefaults()
{
    m_audioSource.resetToDefaults();
    m_waveform.reset();
    
    m_heldFramesMono.clear();
    m_heldFramesLeft.clear();
    m_heldFramesRight.clear();
    
    int sampleCount = m_waveform.sampleCount();
    m_displayLeft.assign(sampleCount, 0.0f);
    m_displayRight.assign(sampleCount, 0.0f);
    m_displayMono.assign(sampleCount, 0.0f);

    m_displaySmoothMono.reset();
    m_displaySmoothLeft.reset();
    m_displaySmoothRight.reset();
    syncDisplaySmoothing();

    BasicLogger::logInfo("WaveformVisualizer: Reset to defaults");
}

// =============================================================================
// OpenGL Lifecycle
// =============================================================================

void WaveformVisualizer::onInitialize()
{
    BasicLogger::logInfo("WaveformVisualizer: Initializing...");
    
    if (!createShaders())
    {
        BasicLogger::logWarning("WaveformVisualizer: Failed to create shaders");
        return;
    }
    
    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    if (!m_vao->create())
    {
        BasicLogger::logWarning("WaveformVisualizer: Failed to create VAO");
        return;
    }
    
    m_vertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    if (!m_vertexBuffer->create())
    {
        BasicLogger::logWarning("WaveformVisualizer: Failed to create VBO");
        return;
    }
    
    m_vao->bind();
    m_vertexBuffer->bind();
    
    m_vertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vertexBuffer->allocate(131072 * sizeof(float));  // Large buffer
    
    // CRITICAL: Check context BEFORE calling functions() to prevent crash
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        BasicLogger::logWarning("WaveformVisualizer: No OpenGL context during initialization!");
        m_vertexBuffer->release();
        m_vao->release();
        return;
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (!gl)
    {
        BasicLogger::logWarning("WaveformVisualizer: No OpenGL functions during initialization!");
        m_vertexBuffer->release();
        m_vao->release();
        return;
    }
    
    // Position (2 floats) + Amplitude (1 float)
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                               reinterpret_cast<void*>(2 * sizeof(float)));
    
    m_vertexBuffer->release();
    m_vao->release();
    
    // Store context for change detection
    m_lastContext = ctx;
    
    BasicLogger::logInfo("WaveformVisualizer: Initialized successfully");
}

bool WaveformVisualizer::createShaders()
{
    // Line shader
    m_lineShader = std::make_unique<QOpenGLShaderProgram>();
    
    if (!m_lineShader->addShaderFromSourceCode(QOpenGLShader::Vertex, LINE_VERTEX_SHADER))
    {
        BasicLogger::logWarning("WaveformVisualizer: Line vertex shader failed: " +
                                m_lineShader->log().toStdString());
        return false;
    }
    
    if (!m_lineShader->addShaderFromSourceCode(QOpenGLShader::Fragment, LINE_FRAGMENT_SHADER))
    {
        BasicLogger::logWarning("WaveformVisualizer: Line fragment shader failed: " +
                                m_lineShader->log().toStdString());
        return false;
    }
    
    if (!m_lineShader->link())
    {
        BasicLogger::logWarning("WaveformVisualizer: Line shader linking failed: " +
                                m_lineShader->log().toStdString());
        return false;
    }
    
    // Cache line uniforms
    for (int i = 0; i < 8; ++i)
    {
        m_lineUniColor[i] = m_lineShader->uniformLocation(QString("uColor%1").arg(i));
    }
    m_lineUniStopPos = m_lineShader->uniformLocation("uStopPos");
    m_lineUniStopPos2 = m_lineShader->uniformLocation("uStopPos2");
    m_lineUniStopCount = m_lineShader->uniformLocation("uStopCount");
    m_lineUniGradientMode = m_lineShader->uniformLocation("uGradientMode");
    m_lineUniGradientAngle = m_lineShader->uniformLocation("uGradientAngle");
    m_lineUniAlpha = m_lineShader->uniformLocation("uAlpha");
    
    // Fill shader
    m_fillShader = std::make_unique<QOpenGLShaderProgram>();
    
    if (!m_fillShader->addShaderFromSourceCode(QOpenGLShader::Vertex, FILL_VERTEX_SHADER))
    {
        BasicLogger::logWarning("WaveformVisualizer: Fill vertex shader failed");
        return false;
    }
    
    if (!m_fillShader->addShaderFromSourceCode(QOpenGLShader::Fragment, FILL_FRAGMENT_SHADER))
    {
        BasicLogger::logWarning("WaveformVisualizer: Fill fragment shader failed");
        return false;
    }
    
    if (!m_fillShader->link())
    {
        BasicLogger::logWarning("WaveformVisualizer: Fill shader linking failed");
        return false;
    }
    
    // Cache fill uniforms
    for (int i = 0; i < 8; ++i)
    {
        m_fillUniColor[i] = m_fillShader->uniformLocation(QString("uColor%1").arg(i));
    }
    m_fillUniStopPos = m_fillShader->uniformLocation("uStopPos");
    m_fillUniStopPos2 = m_fillShader->uniformLocation("uStopPos2");
    m_fillUniStopCount = m_fillShader->uniformLocation("uStopCount");
    m_fillUniGradientMode = m_fillShader->uniformLocation("uGradientMode");
    m_fillUniGradientAngle = m_fillShader->uniformLocation("uGradientAngle");
    m_fillUniAlpha = m_fillShader->uniformLocation("uAlpha");
    m_fillUniBrightness = m_fillShader->uniformLocation("uBrightness");
    
    return true;
}

// =============================================================================
// Audio Processing
// =============================================================================

// splitStereoData/resampleNearest: shared helpers from modules/AudioUtil.hpp (5.6)

void WaveformVisualizer::syncDisplaySmoothing()
{
    // The stage-1 smoothing config (audio.smooth.*) is the single source of
    // truth (E3) -- mirror it into the per-channel display smoothers.
    const SmoothingModule& config = m_audioSource.smoothing();
    for (SmoothingModule* smoother :
         {&m_displaySmoothMono, &m_displaySmoothLeft, &m_displaySmoothRight})
    {
        smoother->setAlgorithm(config.algorithm());
        smoother->setTimeMs(config.timeMs());
    }
}

// =============================================================================
// Vertex Building
// =============================================================================

void WaveformVisualizer::buildThickLineVertices(const std::vector<float>& samples,
                                                 float offset,
                                                 float amplitude,
                                                 float lineWidth,
                                                 std::vector<float>& vertices)
{
    int count = static_cast<int>(samples.size());
    float displayWidth = m_waveform.displayWidth();
    
    // CRITICAL: Check context BEFORE calling functions() to prevent crash on undocking
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    float pixelHeight = 0.002f;  // Default fallback
    
    if (ctx)
    {
        QOpenGLFunctions* gl = ctx->functions();
        if (gl)
        {
            GLint viewport[4];
            gl->glGetIntegerv(GL_VIEWPORT, viewport);
            pixelHeight = viewport[3] > 0 ? 2.0f / static_cast<float>(viewport[3]) : 0.002f;
        }
    }
    
    float halfWidth = lineWidth * pixelHeight * 0.5f;
    
    for (int i = 0; i < count; ++i)
    {
        float x = -displayWidth + 2.0f * displayWidth * static_cast<float>(i) / static_cast<float>(count - 1);
        float y = samples[i] * amplitude + offset;
        float amp = std::abs(samples[i]);
        
        float nx = 0.0f;
        float ny = 1.0f;
        
        if (i < count - 1)
        {
            float nextX = -displayWidth + 2.0f * displayWidth * static_cast<float>(i + 1) / static_cast<float>(count - 1);
            float nextY = samples[i + 1] * amplitude + offset;
            
            float dx = nextX - x;
            float dy = nextY - y;
            float len = std::sqrt(dx * dx + dy * dy);
            
            if (len > 0.0001f)
            {
                nx = -dy / len;
                ny = dx / len;
            }
        }
        else if (i > 0)
        {
            float prevX = -displayWidth + 2.0f * displayWidth * static_cast<float>(i - 1) / static_cast<float>(count - 1);
            float prevY = samples[i - 1] * amplitude + offset;
            
            float dx = x - prevX;
            float dy = y - prevY;
            float len = std::sqrt(dx * dx + dy * dy);
            
            if (len > 0.0001f)
            {
                nx = -dy / len;
                ny = dx / len;
            }
        }
        
        // Top vertex
        vertices.push_back(x + nx * halfWidth);
        vertices.push_back(y + ny * halfWidth);
        vertices.push_back(amp);
        
        // Bottom vertex
        vertices.push_back(x - nx * halfWidth);
        vertices.push_back(y - ny * halfWidth);
        vertices.push_back(amp);
    }
}

void WaveformVisualizer::buildFillVertices(const std::vector<float>& samples,
                                            float offset,
                                            float amplitude,
                                            std::vector<float>& vertices)
{
    int count = static_cast<int>(samples.size());
    float displayWidth = m_waveform.displayWidth();
    
    for (int i = 0; i < count; ++i)
    {
        float x = -displayWidth + 2.0f * displayWidth * static_cast<float>(i) / static_cast<float>(count - 1);
        float y = samples[i] * amplitude + offset;
        
        // Point on waveform
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(std::abs(samples[i]));
        
        // Point on baseline (offset)
        vertices.push_back(x);
        vertices.push_back(offset);
        vertices.push_back(0.0f);
    }
}

// =============================================================================
// Gradient Uniforms
// =============================================================================

void WaveformVisualizer::uploadGradientUniforms(QOpenGLShaderProgram* /*shader*/, int channelIndex, bool isLine)
{
    // CRITICAL: Check context BEFORE calling functions() to prevent crash on undocking
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (!gl)
    {
        return;
    }
    
    // Get gradient for the specific channel
    const auto& gradient = m_waveform.colorGradient(channelIndex);
    auto mode = gradient.mode();
    
    // Upload colors - for Solid/Outline mode use solidColor, otherwise use gradient stops
    int* colorUniforms = isLine ? m_lineUniColor : m_fillUniColor;
    
    if (mode == lumi::modules::GradientMode::Solid || mode == lumi::modules::GradientMode::Outline)
    {
        // Use solid color for uColor0
        const auto& c = gradient.solidColor();
        gl->glUniform4f(colorUniforms[0], c[0], c[1], c[2], c[3]);
        
        // Fill remaining with same color (not really used but be safe)
        for (int i = 1; i < 8; ++i)
        {
            gl->glUniform4f(colorUniforms[i], c[0], c[1], c[2], c[3]);
        }
    }
    else
    {
        // Use gradient stops
        const auto& stops = gradient.stops();
        int stopCount = std::min(static_cast<int>(stops.size()), 8);
        
        for (int i = 0; i < 8; ++i)
        {
            if (i < stopCount)
            {
                const auto& c = stops[i].color;
                gl->glUniform4f(colorUniforms[i], c[0], c[1], c[2], c[3]);
            }
            else
            {
                gl->glUniform4f(colorUniforms[i], 0.0f, 0.0f, 0.0f, 1.0f);
            }
        }
    }
    
    // Upload stop positions (only relevant for gradient modes)
    const auto& stops = gradient.stops();
    int stopCount = std::min(static_cast<int>(stops.size()), 8);
    
    float stopPos[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float stopPos2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    
    for (int i = 0; i < stopCount && i < 4; ++i)
    {
        stopPos[i] = stops[i].position;
    }
    for (int i = 4; i < stopCount && i < 8; ++i)
    {
        stopPos2[i - 4] = stops[i].position;
    }
    
    int stopPosLoc = isLine ? m_lineUniStopPos : m_fillUniStopPos;
    int stopPos2Loc = isLine ? m_lineUniStopPos2 : m_fillUniStopPos2;
    int stopCountLoc = isLine ? m_lineUniStopCount : m_fillUniStopCount;
    int modeLoc = isLine ? m_lineUniGradientMode : m_fillUniGradientMode;
    int angleLoc = isLine ? m_lineUniGradientAngle : m_fillUniGradientAngle;
    
    gl->glUniform4f(stopPosLoc, stopPos[0], stopPos[1], stopPos[2], stopPos[3]);
    gl->glUniform4f(stopPos2Loc, stopPos2[0], stopPos2[1], stopPos2[2], stopPos2[3]);
    gl->glUniform1i(stopCountLoc, stopCount);
    gl->glUniform1i(modeLoc, static_cast<int>(mode));
    gl->glUniform1f(angleLoc, gradient.angle());
}

// =============================================================================
// Hold/Fade
// =============================================================================

void WaveformVisualizer::updateHeldFrames(float deltaTime)
{
    // Frame mechanics live in the shared HoldFadeEffect (PostFxModule, 5.6)
    const float fadeTime = m_waveform.fadeTime();
    m_heldFramesMono.update(deltaTime, fadeTime);
    m_heldFramesLeft.update(deltaTime, fadeTime);
    m_heldFramesRight.update(deltaTime, fadeTime);
}

// =============================================================================
// Channel Rendering
// =============================================================================

void WaveformVisualizer::renderChannel(int channelIndex,
                                        const std::vector<float>& samples,
                                        const WaveformChannelConfig& config,
                                        float alpha)
{
    // CRITICAL: Check context BEFORE calling functions() to prevent crash on undocking
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (!gl || samples.empty()) return;
    
    // Safety check for OpenGL resources
    if (!m_vao || !m_vertexBuffer || !m_lineShader)
    {
        return;
    }
    
    bool mirror = m_waveform.mirrorEnabled();
    
    // Render fill first (behind line)
    if (config.fillEnabled && m_fillShader)
    {
        std::vector<float> fillVertices;
        buildFillVertices(samples, config.lineOffset, config.amplitude, fillVertices);
        
        if (!fillVertices.empty())
        {
            m_fillShader->bind();
            
            // Use line gradient with brightness adjustment
            uploadGradientUniforms(m_fillShader.get(), channelIndex, false);
            gl->glUniform1f(m_fillUniBrightness, config.fillBrightness);
            gl->glUniform1f(m_fillUniAlpha, alpha * config.fillOpacity);
            
            m_vao->bind();
            m_vertexBuffer->bind();
            m_vertexBuffer->write(0, fillVertices.data(),
                                   static_cast<int>(fillVertices.size() * sizeof(float)));
            
            int vertCount = static_cast<int>(fillVertices.size() / 3);
            gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, vertCount);
            
            if (mirror)
            {
                for (size_t i = 1; i < fillVertices.size(); i += 3)
                {
                    fillVertices[i] = 2.0f * config.lineOffset - fillVertices[i];
                }
                m_vertexBuffer->write(0, fillVertices.data(),
                                       static_cast<int>(fillVertices.size() * sizeof(float)));
                gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, vertCount);
            }
            
            m_vertexBuffer->release();
            m_vao->release();
            m_fillShader->release();
        }
    }
    
    // Render line
    std::vector<float> lineVertices;
    buildThickLineVertices(samples, config.lineOffset, config.amplitude, config.lineWidth, lineVertices);
    
    if (lineVertices.empty()) return;
    
    m_lineShader->bind();
    uploadGradientUniforms(m_lineShader.get(), channelIndex, true);
    gl->glUniform1f(m_lineUniAlpha, alpha);
    
    m_vao->bind();
    m_vertexBuffer->bind();
    m_vertexBuffer->write(0, lineVertices.data(),
                           static_cast<int>(lineVertices.size() * sizeof(float)));
    
    int vertCount = static_cast<int>(lineVertices.size() / 3);
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, vertCount);
    
    if (mirror)
    {
        for (size_t i = 1; i < lineVertices.size(); i += 3)
        {
            lineVertices[i] = 2.0f * config.lineOffset - lineVertices[i];
        }
        m_vertexBuffer->write(0, lineVertices.data(),
                               static_cast<int>(lineVertices.size() * sizeof(float)));
        gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, vertCount);
    }
    
    m_vertexBuffer->release();
    m_vao->release();
    m_lineShader->release();
}

// =============================================================================
// Main Render Loop
// =============================================================================

void WaveformVisualizer::onRender(float deltaTime)
{
    // CRITICAL: Check context BEFORE calling functions() to prevent crash on undocking
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (!gl) return;
    
    // =========================================================================
    // Context Change Detection - Reinitialize if context changed
    // =========================================================================
    
    if (m_lastContext != ctx)
    {
        BasicLogger::logInfo("WaveformVisualizer: OpenGL context changed, reinitializing resources...");
        
        // Clean up old resources (they're invalid in new context anyway)
        m_lineShader.reset();
        m_fillShader.reset();
        m_vertexBuffer.reset();
        m_vao.reset();
        
        // Reinitialize in new context
        onInitialize();
        
        // Track new context
        m_lastContext = ctx;
        
        // If initialization failed, skip rendering
        if (!m_lineShader || !m_vao)
        {
            BasicLogger::logWarning("WaveformVisualizer: Reinitialization failed");
            return;
        }
    }
    
    if (!m_lineShader || !m_vao) return;
    
    m_totalTime += deltaTime;
    
    // Get settings
    int sampleCount = m_waveform.sampleCount();
    float gain = m_audioSource.gain();
    WaveformChannelMode channelMode = m_waveform.channelMode();

    // Ensure buffers match
    if (static_cast<int>(m_displayLeft.size()) != sampleCount)
    {
        m_displayLeft.resize(sampleCount, 0.0f);
        m_displayRight.resize(sampleCount, 0.0f);
        m_displayMono.resize(sampleCount, 0.0f);
    }

    // Get waveform data
    std::vector<float> rawWaveform = getWaveform();

    if (!rawWaveform.empty())
    {
        splitStereoData(rawWaveform, m_rawWaveformLeft, m_rawWaveformRight);

        // Display smoothing via the shared SmoothingModule (E3): config
        // mirrors audio.smooth.*, per-index state lives in the smoothers
        syncDisplaySmoothing();

        resampleNearest(m_rawWaveformLeft, m_displayLeft, sampleCount, gain);
        resampleNearest(m_rawWaveformRight, m_displayRight, sampleCount, gain);
        m_displaySmoothLeft.processArrayPerIndex(m_displayLeft.data(), sampleCount,
                                                 deltaTime, m_displayLeft.data());
        m_displaySmoothRight.processArrayPerIndex(m_displayRight.data(), sampleCount,
                                                  deltaTime, m_displayRight.data());

        // Mono mix of the smoothed channels, then its own smoothing pass
        // (parity with the previous double-EMA behavior)
        for (int i = 0; i < sampleCount; ++i)
        {
            m_displayMono[i] = (m_displayLeft[i] + m_displayRight[i]) * 0.5f;
        }
        m_displaySmoothMono.processArrayPerIndex(m_displayMono.data(), sampleCount,
                                                 deltaTime, m_displayMono.data());

        // Add to hold trails (shared HoldFadeEffect, 5.6)
        if (m_waveform.holdEnabled())
        {
            const int maxFrames = m_waveform.maxHoldFrames();
            if (channelMode == WaveformChannelMode::Mono || channelMode == WaveformChannelMode::Both)
            {
                m_heldFramesMono.push(m_displayMono, maxFrames);
            }

            if (channelMode == WaveformChannelMode::Stereo || channelMode == WaveformChannelMode::Both)
            {
                m_heldFramesLeft.push(m_displayLeft, maxFrames);
                m_heldFramesRight.push(m_displayRight, maxFrames);
            }
        }
    }
    
    if (m_waveform.holdEnabled())
    {
        updateHeldFrames(deltaTime);
    }
    
    // Clear background
    gl->glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT);
    
    gl->glDisable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Get channel configs
    const auto& monoConfig = m_waveform.channelConfig(WaveformModule::CHANNEL_MONO);
    const auto& leftConfig = m_waveform.channelConfig(WaveformModule::CHANNEL_LEFT);
    const auto& rightConfig = m_waveform.channelConfig(WaveformModule::CHANNEL_RIGHT);
    
    // Render held frames first
    if (m_waveform.holdEnabled())
    {
        if (channelMode == WaveformChannelMode::Mono || channelMode == WaveformChannelMode::Both)
        {
            for (const auto& frame : m_heldFramesMono.frames())
            {
                if (frame.alpha > 0.01f)
                {
                    renderChannel(WaveformModule::CHANNEL_MONO, frame.data, monoConfig, frame.alpha);
                }
            }
        }

        if (channelMode == WaveformChannelMode::Stereo || channelMode == WaveformChannelMode::Both)
        {
            for (const auto& frame : m_heldFramesLeft.frames())
            {
                if (frame.alpha > 0.01f)
                {
                    renderChannel(WaveformModule::CHANNEL_LEFT, frame.data, leftConfig, frame.alpha);
                }
            }
            for (const auto& frame : m_heldFramesRight.frames())
            {
                if (frame.alpha > 0.01f)
                {
                    renderChannel(WaveformModule::CHANNEL_RIGHT, frame.data, rightConfig, frame.alpha);
                }
            }
        }
    }
    
    // Render current waveforms based on channel mode
    switch (channelMode)
    {
        case WaveformChannelMode::Mono:
            // Only mono
            renderChannel(WaveformModule::CHANNEL_MONO, m_displayMono, monoConfig, 1.0f);
            break;
            
        case WaveformChannelMode::Stereo:
            // Only Left and Right (NO MONO!)
            renderChannel(WaveformModule::CHANNEL_LEFT, m_displayLeft, leftConfig, 1.0f);
            renderChannel(WaveformModule::CHANNEL_RIGHT, m_displayRight, rightConfig, 1.0f);
            break;
            
        case WaveformChannelMode::Both:
            // All three: Mono, Left, Right
            renderChannel(WaveformModule::CHANNEL_MONO, m_displayMono, monoConfig, 1.0f);
            renderChannel(WaveformModule::CHANNEL_LEFT, m_displayLeft, leftConfig, 1.0f);
            renderChannel(WaveformModule::CHANNEL_RIGHT, m_displayRight, rightConfig, 1.0f);
            break;
    }
}

void WaveformVisualizer::onResize(const QSize& size)
{
    // CRITICAL: Check context BEFORE calling functions() to prevent crash on undocking
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (gl)
    {
        gl->glViewport(0, 0, size.width(), size.height());
    }
}

void WaveformVisualizer::onCleanup()
{
    m_lineShader.reset();
    m_fillShader.reset();
    m_vertexBuffer.reset();
    m_vao.reset();
    
    m_heldFramesMono.clear();
    m_heldFramesLeft.clear();
    m_heldFramesRight.clear();
    
    BasicLogger::logInfo("WaveformVisualizer: Cleaned up");
}

// =============================================================================
// Audio Interface
// =============================================================================

void WaveformVisualizer::updateWaveform(const float* waveform, int count)
{
    VisualizerBase::updateWaveform(waveform, count);
}

void WaveformVisualizer::updateSpectrum(const float* spectrum, int count)
{
    if (!spectrum || count <= 0) return;
    
    // Store raw data in base class
    VisualizerBase::updateSpectrum(spectrum, count);
    
    // Process through AudioSourceModule pipeline
    // This applies: Frequency Mapping (Linear/Log/Mel), dB Normalization, Smoothing
    constexpr float deltaTime = 1.0f / 60.0f;  // Assume 60fps
    m_audioSource.update(spectrum, count, deltaTime);
}
