/**
 ****************************************************************************************
 * @file   WaveformVisualizer.cpp
 * @brief  Audio waveform visualizer implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 2.0.0
 ****************************************************************************************
 */

#include "visualizers/WaveformVisualizer.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>
#include <numeric>

using namespace lumi::modules;

namespace
{

constexpr float PI = 3.14159265358979323846f;

// =============================================================================
// Shader Source
// =============================================================================

const char* WAVEFORM_VERTEX_SHADER = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in float aAmplitude;

out float vAmplitude;
out vec2 vPosition;

uniform float uAspect;
uniform float uAmplitudeScale;

void main()
{
    vec2 pos = aPosition;
    pos.y *= uAmplitudeScale;
    pos.x /= uAspect;
    
    gl_Position = vec4(pos, 0.0, 1.0);
    vAmplitude = abs(aAmplitude);
    vPosition = aPosition;
}
)";

const char* WAVEFORM_FRAGMENT_SHADER = R"(
#version 330 core

in float vAmplitude;
in vec2 vPosition;

out vec4 fragColor;

uniform vec4 uColor0;
uniform vec4 uColor1;
uniform int uColorMode;
uniform float uTime;

void main()
{
    vec4 color;
    
    // Mode 0: Solid color
    if (uColorMode == 0)
    {
        color = uColor0;
    }
    // Mode 1: Linear gradient based on x position
    else if (uColorMode == 1)
    {
        float t = vPosition.x * 0.5 + 0.5;
        color = mix(uColor0, uColor1, t);
    }
    // Mode 2: Radial - gradient based on amplitude
    else
    {
        float t = vAmplitude;
        color = mix(uColor0, uColor1, t);
    }
    
    // Subtle pulse
    float pulse = 1.0 + 0.05 * sin(uTime * 3.0);
    color.rgb *= pulse;
    
    fragColor = color;
}
)";

} // anonymous namespace

// =============================================================================
// Constructor / Destructor
// =============================================================================

WaveformVisualizer::WaveformVisualizer()
    : VisualizerBase(
          QStringLiteral("waveform"),
          QObject::tr("Waveform"),
          QObject::tr("Audio waveform oscilloscope display"))
    , m_startTime(std::chrono::steady_clock::now())
{
    BasicLogger::logDebug("WaveformVisualizer: Constructor called");
    
    // Initialize display buffers
    int sampleCount = m_waveform.sampleCount();
    m_displayWaveform.resize(sampleCount, 0.0f);
    m_smoothedWaveform.resize(sampleCount, 0.0f);
}

WaveformVisualizer::~WaveformVisualizer()
{
    if (isInitialized())
    {
        cleanup();
    }
}

// =============================================================================
// Parameter Interface - Delegates to Modules
// =============================================================================

std::vector<ModuleParamDesc> WaveformVisualizer::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    // =========================================================================
    // 1. Audio Source Parameters (from AudioSourceModule)
    // =========================================================================
    
    for (const auto& p : m_audioSource.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "1. Audio";
        
        // Prefix dependsOn reference
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "audio." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }

    // =========================================================================
    // 2. Waveform Display Parameters (from WaveformModule)
    // =========================================================================
    
    for (const auto& p : m_waveform.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "waveform." + p.id;
        prefixed.group = "2. Waveform";
        prefixed.order = 100 + p.order;
        
        // Prefix dependsOn reference
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "waveform." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }
    
    return params;
}

bool WaveformVisualizer::getParam(const std::string& id, ParamValue& out) const
{
    // Audio module parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }
    
    // Waveform module parameters
    if (id.rfind("waveform.", 0) == 0)
    {
        return m_waveform.getParam(id.substr(9), out);
    }
    
    return false;
}

bool WaveformVisualizer::setParam(const std::string& id, const ParamValue& value)
{
    // Audio module parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }
    
    // Waveform module parameters
    if (id.rfind("waveform.", 0) == 0)
    {
        bool result = m_waveform.setParam(id.substr(9), value);
        
        // Handle sample count change
        if (result && id == "waveform.sampleCount")
        {
            int newCount = m_waveform.sampleCount();
            m_displayWaveform.resize(newCount, 0.0f);
            m_smoothedWaveform.resize(newCount, 0.0f);
        }
        
        return result;
    }
    
    return false;
}

void WaveformVisualizer::resetToDefaults()
{
    // Reset modules
    m_audioSource.resetToDefaults();
    m_waveform.reset();
    
    // Background
    m_bgColorR = 0.02f;
    m_bgColorG = 0.02f;
    m_bgColorB = 0.05f;
    
    // Resize buffers
    int sampleCount = m_waveform.sampleCount();
    m_displayWaveform.assign(sampleCount, 0.0f);
    m_smoothedWaveform.assign(sampleCount, 0.0f);
    
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
    
    // Create VAO
    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    if (!m_vao->create())
    {
        BasicLogger::logWarning("WaveformVisualizer: Failed to create VAO");
        return;
    }
    
    // Create VBO
    m_vertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    if (!m_vertexBuffer->create())
    {
        BasicLogger::logWarning("WaveformVisualizer: Failed to create VBO");
        return;
    }
    
    // Setup vertex attributes
    m_vao->bind();
    m_vertexBuffer->bind();
    
    // Reserve space for max vertices (position + amplitude per vertex)
    m_vertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vertexBuffer->allocate(2048 * 3 * sizeof(float));
    
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    
    // Position (2 floats)
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    
    // Amplitude (1 float)
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                               reinterpret_cast<void*>(2 * sizeof(float)));
    
    m_vertexBuffer->release();
    m_vao->release();
    
    BasicLogger::logInfo("WaveformVisualizer: Initialized successfully");
}

bool WaveformVisualizer::createShaders()
{
    m_shader = std::make_unique<QOpenGLShaderProgram>();
    
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, WAVEFORM_VERTEX_SHADER))
    {
        BasicLogger::logWarning("WaveformVisualizer: Vertex shader failed: " +
                                m_shader->log().toStdString());
        return false;
    }
    
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, WAVEFORM_FRAGMENT_SHADER))
    {
        BasicLogger::logWarning("WaveformVisualizer: Fragment shader failed: " +
                                m_shader->log().toStdString());
        return false;
    }
    
    if (!m_shader->link())
    {
        BasicLogger::logWarning("WaveformVisualizer: Shader linking failed: " +
                                m_shader->log().toStdString());
        return false;
    }
    
    // Cache uniform locations
    m_uniformAspect = m_shader->uniformLocation("uAspect");
    m_uniformAmplitude = m_shader->uniformLocation("uAmplitudeScale");
    m_uniformColorMode = m_shader->uniformLocation("uColorMode");
    m_uniformColor0 = m_shader->uniformLocation("uColor0");
    m_uniformColor1 = m_shader->uniformLocation("uColor1");
    m_uniformTime = m_shader->uniformLocation("uTime");
    
    return true;
}

void WaveformVisualizer::onRender(float deltaTime)
{
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!gl || !m_shader || !m_vao)
    {
        return;
    }
    
    m_totalTime += deltaTime;
    
    // =========================================================================
    // Get Parameters from Modules
    // =========================================================================
    
    int sampleCount = m_waveform.sampleCount();
    float smoothing = m_waveform.smoothing();
    float amplitude = m_waveform.amplitude();
    float lineWidth = m_waveform.lineWidth();
    WaveformStyle style = m_waveform.style();
    float gain = m_audioSource.gain();
    
    // Ensure buffers match sample count
    if (static_cast<int>(m_displayWaveform.size()) != sampleCount)
    {
        m_displayWaveform.resize(sampleCount, 0.0f);
        m_smoothedWaveform.resize(sampleCount, 0.0f);
    }
    
    // =========================================================================
    // Get Waveform Data from VisualizerBase (thread-safe)
    // =========================================================================
    
    std::vector<float> rawWaveform = getWaveform();
    
    if (!rawWaveform.empty())
    {
        // Resample waveform data to display size
        for (int i = 0; i < sampleCount; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(sampleCount - 1);
            int srcIdx = static_cast<int>(t * (rawWaveform.size() - 1));
            srcIdx = std::clamp(srcIdx, 0, static_cast<int>(rawWaveform.size()) - 1);
            
            float target = rawWaveform[srcIdx];
            
            // Apply gain from audio module
            target *= gain;
            
            // Apply smoothing
            m_smoothedWaveform[i] = smoothing * m_smoothedWaveform[i] +
                                    (1.0f - smoothing) * target;
        }
        
        m_displayWaveform = m_smoothedWaveform;
    }
    
    // =========================================================================
    // Build Vertex Data
    // =========================================================================
    
    std::vector<float> vertices;
    vertices.reserve(sampleCount * 3 * 2);  // *2 for mirror mode
    
    bool mirror = (style == WaveformStyle::Mirror);
    float mirrorGap = m_waveform.mirrorGap();
    
    for (int i = 0; i < sampleCount; ++i)
    {
        float x = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(sampleCount - 1);
        float y = m_displayWaveform[i];
        float amp = std::abs(y);
        
        if (mirror)
        {
            // Positive half (above center + gap)
            vertices.push_back(x);
            vertices.push_back(std::abs(y) + mirrorGap);
            vertices.push_back(amp);
        }
        else
        {
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(amp);
        }
    }
    
    // For mirror mode, add negative half
    if (mirror)
    {
        for (int i = 0; i < sampleCount; ++i)
        {
            float x = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(sampleCount - 1);
            float y = m_displayWaveform[i];
            float amp = std::abs(y);
            
            vertices.push_back(x);
            vertices.push_back(-std::abs(y) - mirrorGap);
            vertices.push_back(amp);
        }
    }
    
    // Upload vertex data
    m_vao->bind();
    m_vertexBuffer->bind();
    m_vertexBuffer->write(0, vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));
    m_vertexBuffer->release();
    
    // =========================================================================
    // Get Viewport
    // =========================================================================
    
    GLint viewport[4];
    gl->glGetIntegerv(GL_VIEWPORT, viewport);
    float aspect = viewport[3] > 0 ?
        static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]) : 1.0f;
    
    // =========================================================================
    // Clear Background
    // =========================================================================
    
    gl->glClearColor(m_bgColorR, m_bgColorG, m_bgColorB, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT);
    
    gl->glDisable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // =========================================================================
    // Get Colors from ColorGradientModule (via WaveformModule)
    // =========================================================================
    
    const auto& colorGradient = m_waveform.colorGradient();
    const auto& stops = colorGradient.stops();
    Color4f color0 = stops.size() > 0 ? stops[0].color : Color4f{0, 1, 1, 1};
    Color4f color1 = stops.size() > 1 ? stops[1].color : color0;
    
    // =========================================================================
    // Render Waveform
    // =========================================================================
    
    m_shader->bind();
    
    // Set uniforms
    gl->glUniform1f(m_uniformAspect, aspect);
    gl->glUniform1f(m_uniformAmplitude, amplitude);
    gl->glUniform1i(m_uniformColorMode, static_cast<int>(colorGradient.mode()));
    gl->glUniform4f(m_uniformColor0, color0[0], color0[1], color0[2], color0[3]);
    gl->glUniform4f(m_uniformColor1, color1[0], color1[1], color1[2], color1[3]);
    gl->glUniform1f(m_uniformTime, m_totalTime);
    
    // Set line width
    gl->glLineWidth(lineWidth);
    
    int vertexCount = sampleCount;
    
    // Draw based on style
    switch (style)
    {
        case WaveformStyle::Line:
            gl->glDrawArrays(GL_LINE_STRIP, 0, vertexCount);
            break;
            
        case WaveformStyle::Bars:
            gl->glEnable(GL_PROGRAM_POINT_SIZE);
            gl->glDrawArrays(GL_POINTS, 0, vertexCount);
            break;
        
        case WaveformStyle::Mirror:
            // Draw top and bottom halves
            gl->glDrawArrays(GL_LINE_STRIP, 0, vertexCount);
            gl->glDrawArrays(GL_LINE_STRIP, vertexCount, vertexCount);
            break;
        
        case WaveformStyle::Filled:
            gl->glDrawArrays(GL_LINE_STRIP, 0, vertexCount);
            break;
        
        case WaveformStyle::Dots:
            gl->glEnable(GL_PROGRAM_POINT_SIZE);
            gl->glDrawArrays(GL_POINTS, 0, vertexCount);
            break;
    }
    
    m_vao->release();
    m_shader->release();
}

void WaveformVisualizer::onResize(const QSize& size)
{
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (gl)
    {
        gl->glViewport(0, 0, size.width(), size.height());
    }
}

void WaveformVisualizer::onCleanup()
{
    m_shader.reset();
    m_glowShader.reset();
    m_vertexBuffer.reset();
    m_vao.reset();
    
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
    VisualizerBase::updateSpectrum(spectrum, count);
}
