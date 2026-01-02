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

#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>

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
    
    int sampleCount = m_waveform.sampleCount();
    m_displayLeft.resize(sampleCount, 0.0f);
    m_displayRight.resize(sampleCount, 0.0f);
    m_displayMono.resize(sampleCount, 0.0f);
    m_smoothedLeft.resize(sampleCount, 0.0f);
    m_smoothedRight.resize(sampleCount, 0.0f);
    m_smoothedMono.resize(sampleCount, 0.0f);
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

    // Waveform Parameters
    for (const auto& p : m_waveform.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "waveform." + p.id;
        prefixed.group = "2. Waveform";
        prefixed.order = 100 + p.order;
        
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
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }
    
    if (id.rfind("waveform.", 0) == 0)
    {
        return m_waveform.getParam(id.substr(9), out);
    }
    
    return false;
}

bool WaveformVisualizer::setParam(const std::string& id, const ParamValue& value)
{
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }
    
    if (id.rfind("waveform.", 0) == 0)
    {
        bool result = m_waveform.setParam(id.substr(9), value);
        
        if (result && id == "waveform.sampleCount")
        {
            int newCount = m_waveform.sampleCount();
            m_displayLeft.resize(newCount, 0.0f);
            m_displayRight.resize(newCount, 0.0f);
            m_displayMono.resize(newCount, 0.0f);
            m_smoothedLeft.resize(newCount, 0.0f);
            m_smoothedRight.resize(newCount, 0.0f);
            m_smoothedMono.resize(newCount, 0.0f);
        }
        
        return result;
    }
    
    return false;
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
    m_smoothedLeft.assign(sampleCount, 0.0f);
    m_smoothedRight.assign(sampleCount, 0.0f);
    m_smoothedMono.assign(sampleCount, 0.0f);
    
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
    
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    
    // Position (2 floats) + Amplitude (1 float)
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                               reinterpret_cast<void*>(2 * sizeof(float)));
    
    m_vertexBuffer->release();
    m_vao->release();
    
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

void WaveformVisualizer::splitStereoData(const std::vector<float>& interleaved,
                                          std::vector<float>& left,
                                          std::vector<float>& right)
{
    size_t samples = interleaved.size() / 2;
    left.resize(samples);
    right.resize(samples);
    
    for (size_t i = 0; i < samples; ++i)
    {
        left[i] = interleaved[i * 2];
        right[i] = interleaved[i * 2 + 1];
    }
}

void WaveformVisualizer::resampleWaveform(const std::vector<float>& source,
                                           std::vector<float>& target,
                                           std::vector<float>& smoothed,
                                           int targetSize,
                                           float smoothing,
                                           float gain)
{
    if (source.empty()) return;
    
    target.resize(targetSize);
    smoothed.resize(targetSize);
    
    for (int i = 0; i < targetSize; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(targetSize - 1);
        int srcIdx = static_cast<int>(t * (source.size() - 1));
        srcIdx = std::clamp(srcIdx, 0, static_cast<int>(source.size()) - 1);
        
        float value = source[srcIdx] * gain;
        smoothed[i] = smoothing * smoothed[i] + (1.0f - smoothing) * value;
        target[i] = smoothed[i];
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
    
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    GLint viewport[4];
    gl->glGetIntegerv(GL_VIEWPORT, viewport);
    float pixelHeight = viewport[3] > 0 ? 2.0f / static_cast<float>(viewport[3]) : 0.002f;
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
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    
    // Get gradient for the specific channel
    const auto& gradient = m_waveform.colorGradient(channelIndex);
    const auto& stops = gradient.stops();
    int stopCount = std::min(static_cast<int>(stops.size()), 8);
    
    // Upload colors
    int* colorUniforms = isLine ? m_lineUniColor : m_fillUniColor;
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
    
    // Upload stop positions
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
    gl->glUniform1i(modeLoc, static_cast<int>(gradient.mode()));
    gl->glUniform1f(angleLoc, gradient.angle());
}

// =============================================================================
// Hold/Fade
// =============================================================================

void WaveformVisualizer::updateHeldFrames(float deltaTime)
{
    float fadeTime = m_waveform.fadeTime();
    int maxFrames = m_waveform.maxHoldFrames();
    
    auto updateQueue = [deltaTime, fadeTime, maxFrames](std::deque<HeldWaveformFrame>& frames) {
        for (auto& frame : frames)
        {
            frame.age += deltaTime;
            frame.alpha = 1.0f - (frame.age / fadeTime);
            frame.alpha = std::max(0.0f, frame.alpha);
        }
        
        while (!frames.empty() && frames.front().alpha <= 0.0f)
        {
            frames.pop_front();
        }
        
        while (frames.size() > static_cast<size_t>(maxFrames))
        {
            frames.pop_front();
        }
    };
    
    updateQueue(m_heldFramesMono);
    updateQueue(m_heldFramesLeft);
    updateQueue(m_heldFramesRight);
}

// =============================================================================
// Channel Rendering
// =============================================================================

void WaveformVisualizer::renderChannel(int channelIndex,
                                        const std::vector<float>& samples,
                                        const WaveformChannelConfig& config,
                                        float alpha)
{
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!gl || samples.empty()) return;
    
    bool mirror = m_waveform.mirrorEnabled();
    
    // Render fill first (behind line)
    if (config.fillEnabled)
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
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!gl || !m_lineShader || !m_vao) return;
    
    m_totalTime += deltaTime;
    
    // Get settings
    int sampleCount = m_waveform.sampleCount();
    float smoothing = m_waveform.smoothing();
    float gain = m_audioSource.gain();
    WaveformChannelMode channelMode = m_waveform.channelMode();
    
    // Ensure buffers match
    if (static_cast<int>(m_displayLeft.size()) != sampleCount)
    {
        m_displayLeft.resize(sampleCount, 0.0f);
        m_displayRight.resize(sampleCount, 0.0f);
        m_displayMono.resize(sampleCount, 0.0f);
        m_smoothedLeft.resize(sampleCount, 0.0f);
        m_smoothedRight.resize(sampleCount, 0.0f);
        m_smoothedMono.resize(sampleCount, 0.0f);
    }
    
    // Get waveform data
    std::vector<float> rawWaveform = getWaveform();
    
    if (!rawWaveform.empty())
    {
        splitStereoData(rawWaveform, m_rawWaveformLeft, m_rawWaveformRight);
        
        resampleWaveform(m_rawWaveformLeft, m_displayLeft, m_smoothedLeft,
                         sampleCount, smoothing, gain);
        resampleWaveform(m_rawWaveformRight, m_displayRight, m_smoothedRight,
                         sampleCount, smoothing, gain);
        
        // Create mono mix
        for (int i = 0; i < sampleCount; ++i)
        {
            float monoVal = (m_displayLeft[i] + m_displayRight[i]) * 0.5f;
            m_smoothedMono[i] = smoothing * m_smoothedMono[i] + (1.0f - smoothing) * monoVal;
            m_displayMono[i] = m_smoothedMono[i];
        }
        
        // Add to hold buffers
        if (m_waveform.holdEnabled())
        {
            if (channelMode == WaveformChannelMode::Mono || channelMode == WaveformChannelMode::Both)
            {
                HeldWaveformFrame frame;
                frame.samples = m_displayMono;
                frame.age = 0.0f;
                frame.alpha = 1.0f;
                m_heldFramesMono.push_back(frame);
            }
            
            if (channelMode == WaveformChannelMode::Stereo || channelMode == WaveformChannelMode::Both)
            {
                HeldWaveformFrame frameL, frameR;
                frameL.samples = m_displayLeft;
                frameL.age = 0.0f;
                frameL.alpha = 1.0f;
                frameR.samples = m_displayRight;
                frameR.age = 0.0f;
                frameR.alpha = 1.0f;
                m_heldFramesLeft.push_back(frameL);
                m_heldFramesRight.push_back(frameR);
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
            for (const auto& frame : m_heldFramesMono)
            {
                if (frame.alpha > 0.01f)
                {
                    renderChannel(WaveformModule::CHANNEL_MONO, frame.samples, monoConfig, frame.alpha);
                }
            }
        }
        
        if (channelMode == WaveformChannelMode::Stereo || channelMode == WaveformChannelMode::Both)
        {
            for (const auto& frame : m_heldFramesLeft)
            {
                if (frame.alpha > 0.01f)
                {
                    renderChannel(WaveformModule::CHANNEL_LEFT, frame.samples, leftConfig, frame.alpha);
                }
            }
            for (const auto& frame : m_heldFramesRight)
            {
                if (frame.alpha > 0.01f)
                {
                    renderChannel(WaveformModule::CHANNEL_RIGHT, frame.samples, rightConfig, frame.alpha);
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
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
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
    VisualizerBase::updateSpectrum(spectrum, count);
}
