/**
 ****************************************************************************************
 * @file   PulsingVisualizer.cpp
 * @brief  MINIMAL DEBUG VERSION - Guaranteed visible rendering
 *
 * This version removes all complexity and guarantees a visible pulsing circle.
 * Uses BasicLogger for all debug output (not qDebug).
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version DEBUG-4.1
 ****************************************************************************************
 */

#include "visualizers/PulsingVisualizer.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>

// =============================================================================
// Simple Shader Source - NO COMPLEXITY
// =============================================================================

namespace {

const char* VERTEX_SHADER_SOURCE = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;

uniform float uAspect;
uniform float uSize;

void main()
{
    vec2 pos = aPosition * uSize;
    
    // Correct for aspect ratio
    if (uAspect > 1.0) {
        pos.x /= uAspect;
    } else {
        pos.y *= uAspect;
    }
    
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

const char* FRAGMENT_SHADER_SOURCE = R"(
#version 330 core

out vec4 fragColor;

uniform vec3 uColor;

void main()
{
    // ALWAYS output the color at full brightness
    fragColor = vec4(uColor, 1.0);
}
)";

} // anonymous namespace

// =============================================================================
// Construction
// =============================================================================

PulsingVisualizer::PulsingVisualizer()
    : VisualizerBase(
          QStringLiteral("pulsing"),
          QObject::tr("Pulsing"),
          QObject::tr("Audio-reactive pulsing effect"))
    , m_startTime(std::chrono::steady_clock::now())
{
    BasicLogger::logDebug("PulsingVisualizer: Constructor called");
    
    // Simple defaults
    m_colorScheme.setScheme(lumi::modules::ColorSchemeType::Neon);
    m_pulseShape.setShape(lumi::modules::PulseShape::Circle);
    m_pulseShape.setBaseSize(0.6f);
}

PulsingVisualizer::~PulsingVisualizer() = default;

// =============================================================================
// IModule-style Parameter Access (NEW in v3.0)
// =============================================================================

std::vector<lumi::modules::ModuleParamDesc> PulsingVisualizer::paramDescs() const
{
    return {}; // Simplified for debug
}

bool PulsingVisualizer::getParam(const std::string& id, 
                                  lumi::modules::ParamValue& out) const
{
    (void)id;
    (void)out;
    return false;
}

bool PulsingVisualizer::setParam(const std::string& id, 
                                  const lumi::modules::ParamValue& value)
{
    (void)id;
    (void)value;
    return false;
}

// =============================================================================
// Legacy API - Shape Configuration
// =============================================================================

void PulsingVisualizer::setShape(lumi::modules::PulseShape shape)
{
    m_pulseShape.setShape(shape);
}

lumi::modules::PulseShape PulsingVisualizer::shape() const
{
    return m_pulseShape.shape();
}

void PulsingVisualizer::setSides(int sides)
{
    m_pulseShape.setSides(sides);
}

int PulsingVisualizer::sides() const
{
    return m_pulseShape.sides();
}

// =============================================================================
// Legacy API - Color Configuration
// =============================================================================

void PulsingVisualizer::setColorScheme(lumi::modules::ColorSchemeType scheme)
{
    m_colorScheme.setScheme(scheme);
}

lumi::modules::ColorSchemeType PulsingVisualizer::colorScheme() const
{
    return m_colorScheme.scheme();
}

void PulsingVisualizer::setColorAnimationSpeed(float cyclesPerSecond)
{
    m_colorScheme.setAnimationSpeed(cyclesPerSecond);
}

float PulsingVisualizer::colorAnimationSpeed() const
{
    return m_colorScheme.animationSpeed();
}

void PulsingVisualizer::setBeatBrightnessEnabled(bool enabled)
{
    m_colorScheme.setBeatBrightnessEnabled(enabled);
}

bool PulsingVisualizer::beatBrightnessEnabled() const
{
    return m_colorScheme.beatBrightnessEnabled();
}

// =============================================================================
// Legacy API - Animation Configuration
// =============================================================================

void PulsingVisualizer::setTrigger(lumi::modules::PulseTrigger trigger)
{
    m_pulseShape.setTrigger(trigger);
}

lumi::modules::PulseTrigger PulsingVisualizer::trigger() const
{
    return m_pulseShape.triggerMode();
}

void PulsingVisualizer::setDecay(lumi::modules::PulseDecay decay)
{
    m_pulseShape.setDecay(decay);
}

lumi::modules::PulseDecay PulsingVisualizer::decay() const
{
    return m_pulseShape.decay();
}

void PulsingVisualizer::setDecayTime(float seconds)
{
    m_pulseShape.setDecayTime(seconds);
}

float PulsingVisualizer::decayTime() const
{
    return m_pulseShape.decayTime();
}

// =============================================================================
// Legacy API - Size Configuration
// =============================================================================

void PulsingVisualizer::setBaseSize(float size)
{
    m_pulseShape.setBaseSize(size);
}

float PulsingVisualizer::baseSize() const
{
    return m_pulseShape.baseSize();
}

void PulsingVisualizer::setSizeRange(float min, float max)
{
    m_pulseShape.setSizeRange(min, max);
}

// =============================================================================
// Legacy API - Rotation Configuration
// =============================================================================

void PulsingVisualizer::setRotationSpeed(float degreesPerSecond)
{
    m_pulseShape.setRotationSpeed(degreesPerSecond);
}

float PulsingVisualizer::rotationSpeed() const
{
    return m_pulseShape.rotationSpeed();
}

void PulsingVisualizer::setAudioRotationEnabled(bool enabled)
{
    m_pulseShape.setAudioRotation(enabled);
}

bool PulsingVisualizer::audioRotationEnabled() const
{
    return false; // Simplified
}

// =============================================================================
// Legacy API - Audio Configuration
// =============================================================================

void PulsingVisualizer::setAudioRange(int lowBand, int highBand)
{
    m_lowBand = lowBand;
    m_highBand = highBand;
}

void PulsingVisualizer::setBeatSensitivity(float sensitivity)
{
    m_beatSensitivity = sensitivity;
}

float PulsingVisualizer::beatSensitivity() const
{
    return m_beatSensitivity;
}

void PulsingVisualizer::setSmoothingTime(float milliseconds)
{
    m_audioSource.smoothing().setTimeMs(milliseconds);
}

float PulsingVisualizer::smoothingTime() const
{
    return m_audioSource.smoothing().timeMs();
}

// =============================================================================
// Legacy API - Background Configuration
// =============================================================================

void PulsingVisualizer::setBackgroundSolid(bool solid)
{
    m_backgroundSolid = solid;
}

bool PulsingVisualizer::backgroundSolid() const
{
    return m_backgroundSolid;
}

void PulsingVisualizer::setBackgroundColor(float r, float g, float b)
{
    m_bgColorR = r;
    m_bgColorG = g;
    m_bgColorB = b;
}

// =============================================================================
// OpenGL Lifecycle
// =============================================================================

void PulsingVisualizer::onInitialize()
{
    BasicLogger::logInfo("PulsingVisualizer::onInitialize() - START");
    
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!gl)
    {
        BasicLogger::logError("PulsingVisualizer::onInitialize() - No OpenGL context!");
        return;
    }
    
    // Create shader program
    m_shader = std::make_unique<QOpenGLShaderProgram>();
    
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER_SOURCE))
    {
        BasicLogger::logError("Vertex shader failed: " + m_shader->log().toStdString());
        return;
    }
    BasicLogger::logDebug("  Vertex shader compiled OK");
    
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAGMENT_SHADER_SOURCE))
    {
        BasicLogger::logError("Fragment shader failed: " + m_shader->log().toStdString());
        return;
    }
    BasicLogger::logDebug("  Fragment shader compiled OK");
    
    if (!m_shader->link())
    {
        BasicLogger::logError("Shader link failed: " + m_shader->log().toStdString());
        return;
    }
    BasicLogger::logDebug("  Shader linked OK");
    
    // Get uniform locations
    m_uniformColor = m_shader->uniformLocation("uColor");
    m_uniformAspect = m_shader->uniformLocation("uAspect");
    m_uniformIntensity = m_shader->uniformLocation("uSize");
    
    BasicLogger::logDebug("  Uniforms: color=" + std::to_string(m_uniformColor) + 
                          " aspect=" + std::to_string(m_uniformAspect) +
                          " size=" + std::to_string(m_uniformIntensity));
    
    // Create VAO
    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    if (!m_vao->create())
    {
        BasicLogger::logError("Failed to create VAO!");
        return;
    }
    BasicLogger::logDebug("  VAO created OK");
    
    // Create VBO
    m_vertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    if (!m_vertexBuffer->create())
    {
        BasicLogger::logError("Failed to create VBO!");
        return;
    }
    m_vertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    BasicLogger::logDebug("  VBO created OK");
    
    // Generate circle vertices - SIMPLE VERSION
    generateCircleVertices(64);
    
    BasicLogger::logInfo("PulsingVisualizer::onInitialize() - COMPLETE, vertexCount=" + 
                         std::to_string(m_vertexCount));
}

void PulsingVisualizer::generateCircleVertices(int segments)
{
    std::vector<float> vertices;
    vertices.reserve((segments + 2) * 2);
    
    const float PI = 3.14159265358979323846f;
    const float TWO_PI = 2.0f * PI;
    
    // Center vertex
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    
    // Outer vertices
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (static_cast<float>(i) / segments) * TWO_PI;
        vertices.push_back(std::cos(angle));
        vertices.push_back(std::sin(angle));
    }
    
    m_vertexCount = static_cast<int>(vertices.size() / 2);
    
    // Upload to GPU
    m_vao->bind();
    m_vertexBuffer->bind();
    m_vertexBuffer->allocate(vertices.data(), 
                              static_cast<int>(vertices.size() * sizeof(float)));
    
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (gl)
    {
        gl->glEnableVertexAttribArray(0);
        gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    }
    
    m_vertexBuffer->release();
    m_vao->release();
    
    BasicLogger::logDebug("  Circle vertices generated: " + std::to_string(m_vertexCount));
}

void PulsingVisualizer::onRender(float deltaTime)
{
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!gl)
    {
        return;
    }
    
    m_totalTime += deltaTime;
    
    // =========================================================================
    // CRITICAL: Ensure viewport is set correctly
    // =========================================================================
    int vpWidth = width();
    int vpHeight = height();
    if (vpWidth > 0 && vpHeight > 0)
    {
        gl->glViewport(0, 0, vpWidth, vpHeight);
    }
    
    // =========================================================================
    // Animation: Time-based pulsing (IGNORE audio for now to debug)
    // =========================================================================
    
    // Size pulses between 0.4 and 0.9 - LARGE and visible!
    float pulseSize = 0.65f + 0.25f * std::sin(m_totalTime * 2.0f);
    
    // Bright cycling colors - FULL SATURATION
    float hue = std::fmod(m_totalTime * 0.2f, 1.0f);  // Slow color cycle
    
    // HSV to RGB (simplified, hue only)
    float r, g, b;
    float h = hue * 6.0f;
    int i = static_cast<int>(h);
    float f = h - i;
    switch (i % 6) {
        case 0: r = 1.0f; g = f;    b = 0.0f; break;
        case 1: r = 1-f;  g = 1.0f; b = 0.0f; break;
        case 2: r = 0.0f; g = 1.0f; b = f;    break;
        case 3: r = 0.0f; g = 1-f;  b = 1.0f; break;
        case 4: r = f;    g = 0.0f; b = 1.0f; break;
        case 5: r = 1.0f; g = 0.0f; b = 1-f;  break;
        default: r = 1.0f; g = 0.0f; b = 1.0f; break;
    }
    
    // =========================================================================
    // Clear with VERY DARK background (almost black)
    // =========================================================================
    gl->glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Disable depth test - we're doing 2D
    gl->glDisable(GL_DEPTH_TEST);
    
    // Disable blending - we want solid colors
    gl->glDisable(GL_BLEND);
    
    if (!m_shader || !m_vao || m_vertexCount == 0)
    {
        static int warnCount = 0;
        if (warnCount++ < 5)
        {
            BasicLogger::logWarning("renderPulse: shader=" + 
                std::to_string(m_shader != nullptr) + 
                " vao=" + std::to_string(m_vao != nullptr) + 
                " vertexCount=" + std::to_string(m_vertexCount));
        }
        return;
    }
    
    m_shader->bind();
    m_vao->bind();
    
    // Get aspect ratio with safety check
    float aspect = aspectRatio();
    if (aspect <= 0.0f || std::isnan(aspect) || std::isinf(aspect))
    {
        aspect = 1.0f;
    }
    
    // Debug log first 10 frames
    static int frameCount = 0;
    if (frameCount++ < 10)
    {
        BasicLogger::logDebug("Frame " + std::to_string(frameCount) + 
                              ": size=" + std::to_string(pulseSize) +
                              " aspect=" + std::to_string(aspect) +
                              " viewport=" + std::to_string(vpWidth) + "x" + std::to_string(vpHeight) +
                              " color=(" + std::to_string(r) + "," + 
                              std::to_string(g) + "," + std::to_string(b) + ")");
    }
    
    // Set uniforms - use explicit uniform locations
    gl->glUniform3f(m_uniformColor, r, g, b);
    gl->glUniform1f(m_uniformIntensity, pulseSize);  // This is "uSize"
    gl->glUniform1f(m_uniformAspect, aspect);
    
    // Draw circle as triangle fan
    gl->glDrawArrays(GL_TRIANGLE_FAN, 0, m_vertexCount);
    
    // Check for OpenGL errors
    GLenum err = gl->glGetError();
    if (err != GL_NO_ERROR && frameCount <= 5)
    {
        BasicLogger::logError("OpenGL error after draw: " + std::to_string(err));
    }
    
    m_vao->release();
    m_shader->release();
}

void PulsingVisualizer::onResize(const QSize& size)
{
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (gl)
    {
        gl->glViewport(0, 0, size.width(), size.height());
    }
}

void PulsingVisualizer::onCleanup()
{
    m_shader.reset();
    m_vertexBuffer.reset();
    m_vao.reset();
}

// =============================================================================
// Unused methods (stubs for now)
// =============================================================================

bool PulsingVisualizer::createShaders() 
{ 
    return true; 
}

void PulsingVisualizer::updateVertexBuffer() 
{
}

void PulsingVisualizer::renderPulse(float, float) 
{
}

float PulsingVisualizer::detectBeat(float bassLevel) 
{ 
    (void)bassLevel;
    return 0.0f; 
}
