/**
 ****************************************************************************************
 * @file   OscilloscopeVisualizer.cpp
 * @brief  Classic oscilloscope-style audio visualizer implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/OscilloscopeVisualizer.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>
#include <sstream>

using namespace lumi::modules;

namespace
{

// =============================================================================
// Shader Sources
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

const char* GRID_VERTEX_SHADER = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;

void main()
{
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)";

const char* GRID_FRAGMENT_SHADER = R"(
#version 330 core

out vec4 fragColor;

uniform vec4 uColor;

void main()
{
    fragColor = uColor;
}
)";

} // anonymous namespace

// =============================================================================
// Construction / Destruction
// =============================================================================

OscilloscopeVisualizer::OscilloscopeVisualizer()
    : VisualizerBase(
          QStringLiteral("oscilloscope"),
          QObject::tr("Oscilloscope"),
          QObject::tr("Classic oscilloscope display with trigger"))
{
    BasicLogger::logDebug("OscilloscopeVisualizer: Constructor called");

    // Initialize buffers for all channels
    int sampleCount = m_oscilloscope.sampleCount();
    for (int c = 0; c < OscilloscopeModule::TOTAL_CHANNELS; ++c)
    {
        m_displayChannels[c].resize(sampleCount, 0.0f);
        m_processedChannels[c].resize(sampleCount, 0.0f);
    }
}

OscilloscopeVisualizer::~OscilloscopeVisualizer()
{
    if (isInitialized())
    {
        cleanup();
    }
}

// =============================================================================
// Audio Interface
// =============================================================================

void OscilloscopeVisualizer::updateWaveform(const float* waveform, int count)
{
    if (!waveform || count <= 0)
    {
        return;
    }

    // Store raw waveform for later processing
    m_rawWaveformLeft.assign(waveform, waveform + count);

    // If stereo (interleaved), split into left/right
    if (count >= 2)
    {
        m_rawWaveformRight.resize(count / 2);
        m_rawWaveformLeft.resize(count / 2);

        for (int i = 0; i < count / 2; ++i)
        {
            m_rawWaveformLeft[i] = waveform[i * 2];
            m_rawWaveformRight[i] = waveform[i * 2 + 1];
        }
    }
}

void OscilloscopeVisualizer::updateSpectrum(const float* /*spectrum*/, int /*count*/)
{
    // Oscilloscope doesn't use spectrum data
}

// =============================================================================
// Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> OscilloscopeVisualizer::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    // Audio Source Parameters
    for (const auto& p : m_audioSource.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "1. Audio";

        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "audio." + prefixed.dependsOn;
        }

        params.push_back(prefixed);
    }

    // Oscilloscope Parameters
    for (const auto& p : m_oscilloscope.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "scope." + p.id;
        prefixed.group = "2. Oscilloscope";
        prefixed.order = 100 + p.order;

        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "scope." + prefixed.dependsOn;
        }

        params.push_back(prefixed);
    }

    return params;
}

bool OscilloscopeVisualizer::getParam(const std::string& id, ParamValue& out) const
{
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }

    if (id.rfind("scope.", 0) == 0)
    {
        return m_oscilloscope.getParam(id.substr(6), out);
    }

    return false;
}

bool OscilloscopeVisualizer::setParam(const std::string& id, const ParamValue& value)
{
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }

    if (id.rfind("scope.", 0) == 0)
    {
        bool result = m_oscilloscope.setParam(id.substr(6), value);

        if (result && id == "scope.sampleCount")
        {
            int newCount = m_oscilloscope.sampleCount();
            for (int c = 0; c < OscilloscopeModule::TOTAL_CHANNELS; ++c)
            {
                m_displayChannels[c].resize(newCount, 0.0f);
                m_processedChannels[c].resize(newCount, 0.0f);
            }
        }

        return result;
    }

    return false;
}

void OscilloscopeVisualizer::resetToDefaults()
{
    m_audioSource.resetToDefaults();
    m_oscilloscope.reset();

    // Clear phosphor buffers for all channels
    for (int c = 0; c < OscilloscopeModule::TOTAL_CHANNELS; ++c)
    {
        m_phosphorBuffers[c].clear();
    }

    // Reset display buffers
    int sampleCount = m_oscilloscope.sampleCount();
    for (int c = 0; c < OscilloscopeModule::TOTAL_CHANNELS; ++c)
    {
        m_displayChannels[c].assign(sampleCount, 0.0f);
        m_processedChannels[c].assign(sampleCount, 0.0f);
    }

    m_lastTriggerPoint = 0;
    m_triggered = false;
    m_holdoffTimer = 0.0f;

    BasicLogger::logInfo("OscilloscopeVisualizer: Reset to defaults");
}

// =============================================================================
// OpenGL Lifecycle
// =============================================================================

void OscilloscopeVisualizer::onInitialize()
{
    BasicLogger::logInfo("OscilloscopeVisualizer: Initializing...");

    // CRITICAL: Check context BEFORE any OpenGL calls
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: No OpenGL context during initialization!");
        return;
    }

    QOpenGLFunctions* gl = ctx->functions();
    if (!gl)
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: No OpenGL functions during initialization!");
        return;
    }

    if (!createShaders())
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Failed to create shaders");
        return;
    }

    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    if (!m_vao->create())
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Failed to create VAO");
        return;
    }

    m_vertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    if (!m_vertexBuffer->create())
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Failed to create VBO");
        return;
    }

    m_vao->bind();
    m_vertexBuffer->bind();

    m_vertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vertexBuffer->allocate(131072 * sizeof(float));  // Large buffer

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

    BasicLogger::logInfo("OscilloscopeVisualizer: Initialized successfully");
}

bool OscilloscopeVisualizer::createShaders()
{
    // Line shader
    m_lineShader = std::make_unique<QOpenGLShaderProgram>();

    if (!m_lineShader->addShaderFromSourceCode(QOpenGLShader::Vertex, LINE_VERTEX_SHADER))
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Line vertex shader failed: " +
                                m_lineShader->log().toStdString());
        return false;
    }

    if (!m_lineShader->addShaderFromSourceCode(QOpenGLShader::Fragment, LINE_FRAGMENT_SHADER))
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Line fragment shader failed: " +
                                m_lineShader->log().toStdString());
        return false;
    }

    if (!m_lineShader->link())
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Line shader linking failed: " +
                                m_lineShader->log().toStdString());
        return false;
    }

    // Get line shader uniforms
    m_lineUniColor[0] = m_lineShader->uniformLocation("uColor0");
    m_lineUniColor[1] = m_lineShader->uniformLocation("uColor1");
    m_lineUniColor[2] = m_lineShader->uniformLocation("uColor2");
    m_lineUniColor[3] = m_lineShader->uniformLocation("uColor3");
    m_lineUniColor[4] = m_lineShader->uniformLocation("uColor4");
    m_lineUniColor[5] = m_lineShader->uniformLocation("uColor5");
    m_lineUniColor[6] = m_lineShader->uniformLocation("uColor6");
    m_lineUniColor[7] = m_lineShader->uniformLocation("uColor7");
    m_lineUniStopPos = m_lineShader->uniformLocation("uStopPos");
    m_lineUniStopPos2 = m_lineShader->uniformLocation("uStopPos2");
    m_lineUniStopCount = m_lineShader->uniformLocation("uStopCount");
    m_lineUniGradientMode = m_lineShader->uniformLocation("uGradientMode");
    m_lineUniGradientAngle = m_lineShader->uniformLocation("uGradientAngle");
    m_lineUniAlpha = m_lineShader->uniformLocation("uAlpha");

    // Grid shader
    m_gridShader = std::make_unique<QOpenGLShaderProgram>();

    if (!m_gridShader->addShaderFromSourceCode(QOpenGLShader::Vertex, GRID_VERTEX_SHADER))
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Grid vertex shader failed: " +
                                m_gridShader->log().toStdString());
        return false;
    }

    if (!m_gridShader->addShaderFromSourceCode(QOpenGLShader::Fragment, GRID_FRAGMENT_SHADER))
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Grid fragment shader failed: " +
                                m_gridShader->log().toStdString());
        return false;
    }

    if (!m_gridShader->link())
    {
        BasicLogger::logWarning("OscilloscopeVisualizer: Grid shader linking failed: " +
                                m_gridShader->log().toStdString());
        return false;
    }

    m_gridUniColor = m_gridShader->uniformLocation("uColor");

    return true;
}

void OscilloscopeVisualizer::onRender(float deltaTime)
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

    // =========================================================================
    // Context Change Detection - Reinitialize if context changed
    // =========================================================================

    if (m_lastContext != ctx)
    {
        BasicLogger::logInfo("OscilloscopeVisualizer: OpenGL context changed, reinitializing...");

        // Clean up old resources (they're invalid in new context anyway)
        m_lineShader.reset();
        m_gridShader.reset();
        m_vertexBuffer.reset();
        m_vao.reset();

        // Reinitialize in new context
        onInitialize();

        // Track new context
        m_lastContext = ctx;

        // If initialization failed, skip rendering
        if (!m_lineShader || !m_vao)
        {
            BasicLogger::logWarning("OscilloscopeVisualizer: Reinitialization failed");
            return;
        }
    }

    if (!m_lineShader || !m_vao)
    {
        return;
    }

    m_totalTime += deltaTime;

    // Get settings
    int displaySamples = m_oscilloscope.sampleCount();  // Display resolution (rendering quality)
    float timePerDiv = m_oscilloscope.timePerDiv();     // ms per division
    float gain = m_audioSource.gain();
    
    // Log display samples change (rate-limited)
    static int lastDisplaySamples = -1;
    if (displaySamples != lastDisplaySamples)
    {
        std::ostringstream oss;
        oss << "Display resolution changed: " << lastDisplaySamples << " -> " << displaySamples;
        BasicLogger::logDebug(oss.str());
        lastDisplaySamples = displaySamples;
    }
    
    // Timebase controls how much of the available waveform we show
    // Range: 0.1ms/div (zoomed in) to 100ms/div (zoomed out)
    // We map this to a fraction of available samples
    // At 100ms/div: show all samples (zoom = 1.0)
    // At 0.1ms/div: show 1/1000 of samples (zoom = 0.001)
    constexpr float MAX_TIME_PER_DIV = 100.0f;
    
    // Calculate zoom factor (0.001 to 1.0)
    float zoomFactor = timePerDiv / MAX_TIME_PER_DIV;
    zoomFactor = std::clamp(zoomFactor, 0.001f, 1.0f);

    // Ensure raw buffers have data
    if (m_rawWaveformLeft.empty())
    {
        m_rawWaveformLeft.resize(displaySamples, 0.0f);
    }
    if (m_rawWaveformRight.empty())
    {
        m_rawWaveformRight.resize(displaySamples, 0.0f);
    }

    // Process all channels using OscilloscopeModule::processSignals
    // This handles signal source selection, AC coupling, envelope, and math operations
    int rawCount = static_cast<int>(m_rawWaveformLeft.size());
    m_oscilloscope.processSignals(m_rawWaveformLeft.data(),
                                   m_rawWaveformRight.data(),
                                   rawCount,
                                   m_processedChannels);

    // Find trigger point first (on processed data)
    int triggerPoint = 0;
    if (m_oscilloscope.triggerEnabled())
    {
        int triggerChannel = m_oscilloscope.triggerChannel();
        
        if (triggerChannel < OscilloscopeModule::TOTAL_CHANNELS &&
            !m_processedChannels[triggerChannel].empty())
        {
            int procSize = static_cast<int>(m_processedChannels[triggerChannel].size());
            
            // Log once per second
            static int logCounter = 0;
            if (++logCounter % 60 == 0)
            {
                std::ostringstream oss;
                oss << "Trigger search: ch=" << triggerChannel 
                    << " procSize=" << procSize 
                    << " displaySamples=" << displaySamples
                    << " mode=" << static_cast<int>(m_oscilloscope.triggerMode());
                BasicLogger::logDebug(oss.str());
            }
            
            // Find trigger in processed data
            triggerPoint = m_oscilloscope.findTriggerPoint(
                m_processedChannels[triggerChannel].data(),
                procSize);

            if (triggerPoint < 0)
            {
                if (m_oscilloscope.triggerMode() != TriggerMode::Auto)
                {
                    // Normal/Single mode without trigger: 
                    // DON'T update display channels - keep showing last triggered frame
                    // Just skip to rendering (glClear will be called, old data shown)
                    
                    // Skip the resampling loop by going directly to rendering
                    goto render_section;
                }
                else
                {
                    triggerPoint = 0;
                }
            }
            else
            {
                // Trigger was found! Update the bump effect
                bool shouldActivateBump = false;
                
                if (m_oscilloscope.triggerMode() == TriggerMode::Normal)
                {
                    // Normal mode: Always activate bump on trigger
                    shouldActivateBump = true;
                }
                else if (m_oscilloscope.triggerMode() == TriggerMode::Single)
                {
                    // Single mode: Only activate if bump has faded below 5%
                    shouldActivateBump = (m_triggerBumpAlpha < 0.05f);
                }
                else // Auto mode
                {
                    // Auto mode: Always activate bump
                    shouldActivateBump = true;
                }
                
                if (shouldActivateBump)
                {
                    m_triggerBumpAlpha = 1.0f;
                    
                    // Calculate bump position based on trigger position setting
                    // triggerPosition = 0.0 → left edge (X = -1)
                    // triggerPosition = 0.5 → center (X = 0)
                    // triggerPosition = 1.0 → right edge (X = 1)
                    float triggerPosX = m_oscilloscope.triggerPosition();
                    m_triggerBumpX = (triggerPosX * 2.0f) - 1.0f;
                    
                    // Y position is the trigger level
                    m_triggerBumpY = m_oscilloscope.triggerLevel();
                }
            }
        }
    }

    // Extract timebase window and resample to display resolution
    for (int c = 0; c < OscilloscopeModule::TOTAL_CHANNELS; ++c)
    {
        if (m_displayChannels[c].size() != static_cast<size_t>(displaySamples))
        {
            m_displayChannels[c].resize(displaySamples, 0.0f);
        }
        
        if (!m_processedChannels[c].empty())
        {
            int srcCount = static_cast<int>(m_processedChannels[c].size());
            
            // Window size based on zoom factor
            int windowSize = std::max(64, static_cast<int>(srcCount * zoomFactor));
            
            // Trigger position determines where in the display the trigger point appears
            // triggerPosition = 0.0 → trigger at left edge
            // triggerPosition = 0.5 → trigger at center
            // triggerPosition = 1.0 → trigger at right edge
            float triggerPos = m_oscilloscope.triggerPosition();
            int samplesBeforeTrigger = static_cast<int>(windowSize * triggerPos);
            
            // Window starts before trigger point to position trigger correctly
            int windowStart = triggerPoint - samplesBeforeTrigger;
            
            // Clamp to valid range
            if (windowStart < 0)
            {
                windowStart = 0;
            }
            if (windowStart + windowSize > srcCount)
            {
                windowStart = std::max(0, srcCount - windowSize);
            }
            
            int actualWindow = std::min(windowSize, srcCount - windowStart);
            
            // Resample from window to display buffer
            if (actualWindow > 0)
            {
                float step = static_cast<float>(actualWindow) / static_cast<float>(displaySamples);
                
                for (int i = 0; i < displaySamples; ++i)
                {
                    float srcIdx = i * step;
                    int idx0 = windowStart + static_cast<int>(srcIdx);
                    int idx1 = std::min(idx0 + 1, srcCount - 1);
                    float frac = srcIdx - static_cast<int>(srcIdx);
                    
                    float value = m_processedChannels[c][idx0] * (1.0f - frac);
                    if (idx1 < srcCount)
                    {
                        value += m_processedChannels[c][idx1] * frac;
                    }
                    
                    m_displayChannels[c][i] = std::clamp(value * gain, -1.0f, 1.0f);
                }
            }
            else
            {
                std::fill(m_displayChannels[c].begin(), m_displayChannels[c].end(), 0.0f);
            }
        }
    }

render_section:
    // Update phosphor frames
    updatePhosphorFrames(deltaTime);
    
    // Update trigger bump fade
    if (m_triggerBumpAlpha > 0.0f)
    {
        float fadeTime = m_oscilloscope.triggerFadeTime();
        if (fadeTime > 0.001f)
        {
            // Exponential decay based on fade time
            float decayRate = 3.0f / fadeTime;  // ~95% fade in fadeTime seconds
            m_triggerBumpAlpha *= std::exp(-decayRate * deltaTime);
            
            if (m_triggerBumpAlpha < 0.001f)
            {
                m_triggerBumpAlpha = 0.0f;
            }
        }
        else
        {
            // Instant fade if fadeTime is 0
            m_triggerBumpAlpha = 0.0f;
        }
    }

    // =========================================================================
    // Render
    // =========================================================================

    // Get background color
    float bgR = m_oscilloscope.backgroundR();
    float bgG = m_oscilloscope.backgroundG();
    float bgB = m_oscilloscope.backgroundB();

    gl->glClearColor(bgR, bgG, bgB, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gl->glDisable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render grid
    renderGrid();

    // Render trigger level indicator
    if (m_oscilloscope.triggerEnabled())
    {
        renderTriggerLevel();
    }

    // Render all visible channels (Signal channels CH1-CH4, then Math channels M1-M2)
    for (int c = 0; c < OscilloscopeModule::TOTAL_CHANNELS; ++c)
    {
        const auto& config = m_oscilloscope.channelBase(c);
        
        if (config.visible && !m_displayChannels[c].empty())
        {
            renderChannel(c, m_displayChannels[c], config);
        }
    }
    
    // Render trigger bump effect (on top of everything)
    if (m_oscilloscope.triggerEnabled() && m_triggerBumpAlpha > 0.01f)
    {
        renderTriggerBump();
    }
}

void OscilloscopeVisualizer::onResize(const QSize& size)
{
    // CRITICAL: Check context
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

void OscilloscopeVisualizer::onCleanup()
{
    m_lineShader.reset();
    m_gridShader.reset();
    m_vertexBuffer.reset();
    m_vao.reset();
    m_lastContext = nullptr;
}

// =============================================================================
// Private Methods
// =============================================================================

void OscilloscopeVisualizer::splitStereoData(const std::vector<float>& interleaved,
                                              std::vector<float>& left,
                                              std::vector<float>& right)
{
    int stereoSamples = static_cast<int>(interleaved.size()) / 2;
    left.resize(stereoSamples);
    right.resize(stereoSamples);

    for (int i = 0; i < stereoSamples; ++i)
    {
        left[i] = interleaved[i * 2];
        right[i] = interleaved[i * 2 + 1];
    }
}

void OscilloscopeVisualizer::resampleWaveform(const std::vector<float>& source,
                                               std::vector<float>& target,
                                               int targetSize,
                                               float gain)
{
    if (source.empty())
    {
        std::fill(target.begin(), target.end(), 0.0f);
        return;
    }

    target.resize(targetSize);

    float step = static_cast<float>(source.size()) / static_cast<float>(targetSize);

    for (int i = 0; i < targetSize; ++i)
    {
        float srcIndex = i * step;
        int idx0 = static_cast<int>(srcIndex);
        int idx1 = std::min(idx0 + 1, static_cast<int>(source.size()) - 1);
        float frac = srcIndex - idx0;

        // Linear interpolation
        float value = source[idx0] * (1.0f - frac) + source[idx1] * frac;
        value *= gain;

        target[i] = std::clamp(value, -1.0f, 1.0f);
    }
}

void OscilloscopeVisualizer::buildLineVertices(const std::vector<float>& samples,
                                                float yOffset,
                                                float yScale,
                                                float lineWidth,
                                                std::vector<float>& vertices)
{
    if (samples.size() < 2)
    {
        return;
    }

    vertices.clear();

    // Get pixel size for line width calculation
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    float pixelHeight = 0.002f;  // Default fallback

    if (ctx)
    {
        QOpenGLFunctions* gl = ctx->functions();
        if (gl)
        {
            GLint viewport[4];
            gl->glGetIntegerv(GL_VIEWPORT, viewport);
            if (viewport[3] > 0)
            {
                pixelHeight = 2.0f / static_cast<float>(viewport[3]);
            }
        }
    }

    float halfWidth = lineWidth * pixelHeight * 0.5f;
    int sampleCount = static_cast<int>(samples.size());

    // Build thick line as triangle strip
    for (int i = 0; i < sampleCount - 1; ++i)
    {
        float x0 = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(sampleCount - 1);
        float x1 = -1.0f + 2.0f * static_cast<float>(i + 1) / static_cast<float>(sampleCount - 1);

        float y0 = yOffset + samples[i] * yScale;
        float y1 = yOffset + samples[i + 1] * yScale;

        // Calculate perpendicular direction for line width
        float dx = x1 - x0;
        float dy = y1 - y0;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.0001f) len = 0.0001f;

        float nx = -dy / len * halfWidth;
        float ny = dx / len * halfWidth;

        float amp0 = std::abs(samples[i]);
        float amp1 = std::abs(samples[i + 1]);

        // Two triangles per segment
        // Triangle 1
        vertices.push_back(x0 - nx); vertices.push_back(y0 - ny); vertices.push_back(amp0);
        vertices.push_back(x0 + nx); vertices.push_back(y0 + ny); vertices.push_back(amp0);
        vertices.push_back(x1 - nx); vertices.push_back(y1 - ny); vertices.push_back(amp1);

        // Triangle 2
        vertices.push_back(x0 + nx); vertices.push_back(y0 + ny); vertices.push_back(amp0);
        vertices.push_back(x1 + nx); vertices.push_back(y1 + ny); vertices.push_back(amp1);
        vertices.push_back(x1 - nx); vertices.push_back(y1 - ny); vertices.push_back(amp1);
    }
}

void OscilloscopeVisualizer::renderGrid()
{
    if (m_oscilloscope.gridStyle() == GridStyle::None)
    {
        return;
    }

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }

    QOpenGLFunctions* gl = ctx->functions();
    if (!gl || !m_gridShader || !m_vao || !m_vertexBuffer)
    {
        return;
    }

    std::vector<float> gridVertices;
    float brightness = m_oscilloscope.gridBrightness();
    const auto& gridColor = m_oscilloscope.gridColor();

    // Build grid lines (10 vertical, 8 horizontal divisions)
    if (m_oscilloscope.gridStyle() == GridStyle::Lines)
    {
        // Vertical lines
        for (int i = 0; i <= OscilloscopeModule::DIVISIONS_X; ++i)
        {
            float x = -1.0f + 2.0f * static_cast<float>(i) / OscilloscopeModule::DIVISIONS_X;
            gridVertices.push_back(x);
            gridVertices.push_back(-1.0f);
            gridVertices.push_back(x);
            gridVertices.push_back(1.0f);
        }

        // Horizontal lines
        for (int i = 0; i <= OscilloscopeModule::DIVISIONS_Y; ++i)
        {
            float y = -1.0f + 2.0f * static_cast<float>(i) / OscilloscopeModule::DIVISIONS_Y;
            gridVertices.push_back(-1.0f);
            gridVertices.push_back(y);
            gridVertices.push_back(1.0f);
            gridVertices.push_back(y);
        }
    }
    else if (m_oscilloscope.gridStyle() == GridStyle::Cross)
    {
        // Crosses only at division boundaries
        float crossSizeNorm = m_oscilloscope.gridCrossSize() / 500.0f;
        
        for (int i = 0; i <= OscilloscopeModule::DIVISIONS_X; ++i)
        {
            for (int j = 0; j <= OscilloscopeModule::DIVISIONS_Y; ++j)
            {
                float x = -1.0f + 2.0f * static_cast<float>(i) / OscilloscopeModule::DIVISIONS_X;
                float y = -1.0f + 2.0f * static_cast<float>(j) / OscilloscopeModule::DIVISIONS_Y;

                // Horizontal part of cross
                gridVertices.push_back(x - crossSizeNorm); gridVertices.push_back(y);
                gridVertices.push_back(x + crossSizeNorm); gridVertices.push_back(y);
                // Vertical part of cross
                gridVertices.push_back(x); gridVertices.push_back(y - crossSizeNorm);
                gridVertices.push_back(x); gridVertices.push_back(y + crossSizeNorm);
            }
        }
    }
    else if (m_oscilloscope.gridStyle() == GridStyle::Dots)
    {
        // Dots ONLY along division lines (like real oscilloscopes)
        // 10 subdivisions per division along each axis
        constexpr int SUBDIVS = 10;
        float dotSizeNorm = m_oscilloscope.gridDotSize() / 1000.0f;
        float bigDotSize = dotSizeNorm * 2.0f;  // Division crossings are larger
        
        // Dots along horizontal division lines (constant Y, varying X)
        for (int divY = 0; divY <= OscilloscopeModule::DIVISIONS_Y; ++divY)
        {
            float y = -1.0f + 2.0f * static_cast<float>(divY) / OscilloscopeModule::DIVISIONS_Y;
            
            // Subdivisions along this horizontal line
            int totalX = OscilloscopeModule::DIVISIONS_X * SUBDIVS;
            for (int subX = 0; subX <= totalX; ++subX)
            {
                float x = -1.0f + 2.0f * static_cast<float>(subX) / totalX;
                bool isDivCrossing = (subX % SUBDIVS == 0);
                float size = isDivCrossing ? bigDotSize : dotSizeNorm;
                
                // Single point (tiny cross to simulate)
                gridVertices.push_back(x - size); gridVertices.push_back(y);
                gridVertices.push_back(x + size); gridVertices.push_back(y);
                gridVertices.push_back(x); gridVertices.push_back(y - size);
                gridVertices.push_back(x); gridVertices.push_back(y + size);
            }
        }
        
        // Dots along vertical division lines (constant X, varying Y)
        // Skip the horizontal line crossings (already drawn above)
        for (int divX = 0; divX <= OscilloscopeModule::DIVISIONS_X; ++divX)
        {
            float x = -1.0f + 2.0f * static_cast<float>(divX) / OscilloscopeModule::DIVISIONS_X;
            
            int totalY = OscilloscopeModule::DIVISIONS_Y * SUBDIVS;
            for (int subY = 0; subY <= totalY; ++subY)
            {
                // Skip division crossings (already drawn in horizontal pass)
                if (subY % SUBDIVS == 0) continue;
                
                float y = -1.0f + 2.0f * static_cast<float>(subY) / totalY;
                
                gridVertices.push_back(x - dotSizeNorm); gridVertices.push_back(y);
                gridVertices.push_back(x + dotSizeNorm); gridVertices.push_back(y);
                gridVertices.push_back(x); gridVertices.push_back(y - dotSizeNorm);
                gridVertices.push_back(x); gridVertices.push_back(y + dotSizeNorm);
            }
        }
    }

    if (gridVertices.empty())
    {
        return;
    }

    m_gridShader->bind();
    m_vao->bind();
    m_vertexBuffer->bind();

    // Set line width for Lines mode
    if (m_oscilloscope.gridStyle() == GridStyle::Lines)
    {
        gl->glLineWidth(m_oscilloscope.gridLineWidth());
    }
    else
    {
        gl->glLineWidth(1.0f);
    }

    // Upload grid vertices (only position, no amplitude)
    // Need to pad to 3 floats per vertex for compatibility
    std::vector<float> paddedVertices;
    for (size_t i = 0; i < gridVertices.size(); i += 2)
    {
        paddedVertices.push_back(gridVertices[i]);
        paddedVertices.push_back(gridVertices[i + 1]);
        paddedVertices.push_back(0.0f);  // Amplitude padding
    }

    m_vertexBuffer->write(0, paddedVertices.data(),
                          static_cast<int>(paddedVertices.size() * sizeof(float)));

    // gridColor is std::array<float, 4> - use [0], [1], [2], [3]
    gl->glUniform4f(m_gridUniColor,
                    gridColor[0] * brightness,
                    gridColor[1] * brightness,
                    gridColor[2] * brightness,
                    gridColor[3]);

    gl->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(paddedVertices.size() / 3));

    m_vertexBuffer->release();
    m_vao->release();
    m_gridShader->release();
}

void OscilloscopeVisualizer::renderTriggerLevel()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }

    QOpenGLFunctions* gl = ctx->functions();
    if (!gl || !m_gridShader || !m_vao || !m_vertexBuffer)
    {
        return;
    }

    float triggerLevel = m_oscilloscope.triggerLevel();
    float triggerPosition = m_oscilloscope.triggerPosition();
    float triggerX = -1.0f + 2.0f * triggerPosition;  // Map 0..1 to -1..1
    
    TriggerIndicatorStyle style = m_oscilloscope.triggerIndicatorStyle();
    
    std::vector<float> vertices;
    
    if (style == TriggerIndicatorStyle::Crosshair)
    {
        // Horizontal line through trigger level (full width, subtle)
        vertices.push_back(-1.0f); vertices.push_back(triggerLevel); vertices.push_back(0.0f);
        vertices.push_back( 1.0f); vertices.push_back(triggerLevel); vertices.push_back(0.0f);
        
        // Vertical line through trigger position (full height, subtle)
        vertices.push_back(triggerX); vertices.push_back(-1.0f); vertices.push_back(0.0f);
        vertices.push_back(triggerX); vertices.push_back( 1.0f); vertices.push_back(0.0f);
    }
    else // Arrows
    {
        // Arrow size
        constexpr float arrowSize = 0.05f;
        constexpr float arrowWidth = 0.03f;
        
        // Y-axis arrow (left edge, pointing right at trigger level)
        float yBase = triggerLevel;
        vertices.push_back(-1.0f);             vertices.push_back(yBase); vertices.push_back(0.0f);
        vertices.push_back(-1.0f + arrowSize); vertices.push_back(yBase); vertices.push_back(0.0f);
        // Arrow head
        vertices.push_back(-1.0f + arrowSize); vertices.push_back(yBase - arrowWidth); vertices.push_back(0.0f);
        vertices.push_back(-1.0f + arrowSize * 1.5f); vertices.push_back(yBase); vertices.push_back(0.0f);
        vertices.push_back(-1.0f + arrowSize); vertices.push_back(yBase + arrowWidth); vertices.push_back(0.0f);
        vertices.push_back(-1.0f + arrowSize * 1.5f); vertices.push_back(yBase); vertices.push_back(0.0f);
        
        // X-axis arrow (bottom edge, pointing up at trigger position)
        float xBase = triggerX;
        vertices.push_back(xBase); vertices.push_back(-1.0f);             vertices.push_back(0.0f);
        vertices.push_back(xBase); vertices.push_back(-1.0f + arrowSize); vertices.push_back(0.0f);
        // Arrow head
        vertices.push_back(xBase - arrowWidth); vertices.push_back(-1.0f + arrowSize); vertices.push_back(0.0f);
        vertices.push_back(xBase);              vertices.push_back(-1.0f + arrowSize * 1.5f); vertices.push_back(0.0f);
        vertices.push_back(xBase + arrowWidth); vertices.push_back(-1.0f + arrowSize); vertices.push_back(0.0f);
        vertices.push_back(xBase);              vertices.push_back(-1.0f + arrowSize * 1.5f); vertices.push_back(0.0f);
    }

    m_gridShader->bind();
    m_vao->bind();
    m_vertexBuffer->bind();

    m_vertexBuffer->write(0, vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    // Red trigger indicator
    gl->glUniform4f(m_gridUniColor, 1.0f, 0.3f, 0.3f, 0.6f);

    gl->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size() / 3));

    m_vertexBuffer->release();
    m_vao->release();
    m_gridShader->release();
}

void OscilloscopeVisualizer::renderChannel(int channelIndex,
                                            const std::vector<float>& samples,
                                            const ChannelConfigBase& config)
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }

    QOpenGLFunctions* gl = ctx->functions();
    if (!gl || !m_lineShader || !m_vao || !m_vertexBuffer)
    {
        return;
    }

    std::vector<float> vertices;
    buildLineVertices(samples,
                      config.offset,
                      config.voltsPerDiv * 2.0f,  // Scale factor
                      config.lineWidth,
                      vertices);

    if (vertices.empty())
    {
        return;
    }

    m_lineShader->bind();
    m_vao->bind();
    m_vertexBuffer->bind();

    m_vertexBuffer->write(0, vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    uploadGradientUniforms(channelIndex);
    gl->glUniform1f(m_lineUniAlpha, 1.0f);

    gl->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 3));

    m_vertexBuffer->release();
    m_vao->release();
    m_lineShader->release();
}

void OscilloscopeVisualizer::uploadGradientUniforms(int channelIndex)
{
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

    const auto& gradient = m_oscilloscope.colorGradient(channelIndex);
    const auto gradientMode = gradient.mode();

    // For Solid mode, use solidColor instead of gradient stops
    if (gradientMode == lumi::modules::GradientMode::Solid ||
        gradientMode == lumi::modules::GradientMode::Outline)
    {
        const auto& solid = gradient.solidColor();
        gl->glUniform4f(m_lineUniColor[0], solid[0], solid[1], solid[2], solid[3]);

        // Set remaining colors to white (unused)
        for (int i = 1; i < 8; ++i)
        {
            gl->glUniform4f(m_lineUniColor[i], 1.0f, 1.0f, 1.0f, 1.0f);
        }

        gl->glUniform4f(m_lineUniStopPos, 0.0f, 1.0f, 1.0f, 1.0f);
        gl->glUniform4f(m_lineUniStopPos2, 1.0f, 1.0f, 1.0f, 1.0f);
        gl->glUniform1i(m_lineUniStopCount, 1);
    }
    else
    {
        // Linear or Radial gradient - use stops
        const auto& stops = gradient.stops();
        int stopCount = std::min(static_cast<int>(stops.size()), 8);

        // Upload colors - stops[i].color is std::array<float, 4>
        for (int i = 0; i < 8; ++i)
        {
            if (i < stopCount)
            {
                gl->glUniform4f(m_lineUniColor[i],
                                stops[i].color[0],
                                stops[i].color[1],
                                stops[i].color[2],
                                stops[i].color[3]);
            }
            else
            {
                gl->glUniform4f(m_lineUniColor[i], 1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        // Upload stop positions
        float positions[8] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        for (int i = 0; i < stopCount; ++i)
        {
            positions[i] = stops[i].position;
        }
        gl->glUniform4f(m_lineUniStopPos, positions[0], positions[1], positions[2], positions[3]);
        gl->glUniform4f(m_lineUniStopPos2, positions[4], positions[5], positions[6], positions[7]);

        gl->glUniform1i(m_lineUniStopCount, stopCount);
    }

    gl->glUniform1i(m_lineUniGradientMode, static_cast<int>(gradientMode));
    gl->glUniform1f(m_lineUniGradientAngle, gradient.angle() * 3.14159f / 180.0f);
}

void OscilloscopeVisualizer::updatePhosphorFrames(float deltaTime)
{
    auto updateQueue = [deltaTime](std::deque<PhosphorFrame>& queue, float decay)
    {
        for (auto& frame : queue)
        {
            frame.age += deltaTime;
            frame.alpha *= decay;
        }

        // Remove faded frames
        while (!queue.empty() && queue.front().alpha < 0.01f)
        {
            queue.pop_front();
        }
    };

    // Update phosphor for all channels
    for (int c = 0; c < OscilloscopeModule::TOTAL_CHANNELS; ++c)
    {
        const auto& config = m_oscilloscope.channelBase(c);
        
        if (config.phosphorEnabled)
        {
            updateQueue(m_phosphorBuffers[c], config.phosphorDecay);
        }
    }
}

void OscilloscopeVisualizer::renderTriggerBump()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }

    QOpenGLFunctions* gl = ctx->functions();
    if (!gl || !m_gridShader || !m_vao || !m_vertexBuffer)
    {
        return;
    }

    // Use additive blending for glow effect
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    m_gridShader->bind();
    m_vao->bind();
    m_vertexBuffer->bind();

    // Build a "bump" shape - a filled circle with radial fade
    // We'll use triangles to approximate a circle
    constexpr int SEGMENTS = 32;
    constexpr float BUMP_RADIUS = 0.08f;  // Size of the bump in NDC
    
    std::vector<float> vertices;
    vertices.reserve(SEGMENTS * 9);  // SEGMENTS triangles, 3 vertices each, 3 floats per vertex
    
    float centerX = m_triggerBumpX;
    float centerY = m_triggerBumpY;
    
    // Create triangle fan (as individual triangles for GL_TRIANGLES)
    for (int i = 0; i < SEGMENTS; ++i)
    {
        float angle1 = (2.0f * 3.14159f * i) / SEGMENTS;
        float angle2 = (2.0f * 3.14159f * (i + 1)) / SEGMENTS;
        
        // Center vertex
        vertices.push_back(centerX);
        vertices.push_back(centerY);
        vertices.push_back(0.0f);
        
        // First edge vertex
        vertices.push_back(centerX + BUMP_RADIUS * std::cos(angle1));
        vertices.push_back(centerY + BUMP_RADIUS * std::sin(angle1));
        vertices.push_back(0.0f);
        
        // Second edge vertex
        vertices.push_back(centerX + BUMP_RADIUS * std::cos(angle2));
        vertices.push_back(centerY + BUMP_RADIUS * std::sin(angle2));
        vertices.push_back(0.0f);
    }

    m_vertexBuffer->write(0, vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    // Set color: bright cyan/white with alpha based on bump intensity
    float intensity = m_triggerBumpAlpha;
    gl->glUniform4f(m_gridUniColor, 
                    0.3f + 0.7f * intensity,   // R: white when bright
                    0.8f + 0.2f * intensity,   // G: cyan-ish
                    1.0f,                       // B: full blue
                    intensity * 0.8f);          // A: fade with intensity

    gl->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 3));

    // Draw outer glow ring
    std::vector<float> ringVertices;
    constexpr float RING_INNER = BUMP_RADIUS;
    constexpr float RING_OUTER = BUMP_RADIUS * 2.0f;
    
    for (int i = 0; i < SEGMENTS; ++i)
    {
        float angle1 = (2.0f * 3.14159f * i) / SEGMENTS;
        float angle2 = (2.0f * 3.14159f * (i + 1)) / SEGMENTS;
        
        // Two triangles per segment to form a quad
        // Triangle 1
        ringVertices.push_back(centerX + RING_INNER * std::cos(angle1));
        ringVertices.push_back(centerY + RING_INNER * std::sin(angle1));
        ringVertices.push_back(0.0f);
        
        ringVertices.push_back(centerX + RING_OUTER * std::cos(angle1));
        ringVertices.push_back(centerY + RING_OUTER * std::sin(angle1));
        ringVertices.push_back(0.0f);
        
        ringVertices.push_back(centerX + RING_OUTER * std::cos(angle2));
        ringVertices.push_back(centerY + RING_OUTER * std::sin(angle2));
        ringVertices.push_back(0.0f);
        
        // Triangle 2
        ringVertices.push_back(centerX + RING_INNER * std::cos(angle1));
        ringVertices.push_back(centerY + RING_INNER * std::sin(angle1));
        ringVertices.push_back(0.0f);
        
        ringVertices.push_back(centerX + RING_OUTER * std::cos(angle2));
        ringVertices.push_back(centerY + RING_OUTER * std::sin(angle2));
        ringVertices.push_back(0.0f);
        
        ringVertices.push_back(centerX + RING_INNER * std::cos(angle2));
        ringVertices.push_back(centerY + RING_INNER * std::sin(angle2));
        ringVertices.push_back(0.0f);
    }
    
    m_vertexBuffer->write(0, ringVertices.data(), static_cast<int>(ringVertices.size() * sizeof(float)));
    
    // Outer glow is more transparent
    gl->glUniform4f(m_gridUniColor,
                    0.2f + 0.5f * intensity,
                    0.6f + 0.2f * intensity,
                    1.0f,
                    intensity * 0.3f);
    
    gl->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(ringVertices.size() / 3));

    m_vertexBuffer->release();
    m_vao->release();
    m_gridShader->release();

    // Restore normal blending
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
