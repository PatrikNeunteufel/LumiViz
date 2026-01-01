/**
 ****************************************************************************************
 * @file   PulsingVisualizer.cpp
 * @brief  DEBUG VERSION with centered circle and event support
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version DEBUG-5.0 - Centered + Events
 ****************************************************************************************
 */

#include "visualizers/PulsingVisualizer.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>

// =============================================================================
// Shader Source - CENTERED VERSION
// =============================================================================

namespace {

const char* VERTEX_SHADER_SOURCE = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;

uniform float uAspect;
uniform float uSize;

void main()
{
    // Scale by size
    vec2 pos = aPosition * uSize;
    
    // Correct for aspect ratio - keep circle circular
    // Divide X by aspect ratio to prevent horizontal stretching
    pos.x /= uAspect;
    
    // Position is already centered (vertices are -1 to 1, center at 0,0)
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

const char* FRAGMENT_SHADER_SOURCE = R"(
#version 330 core

out vec4 fragColor;

uniform vec3 uColor;

void main()
{
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
    
    // Defaults
    m_colorScheme.setScheme(lumi::modules::ColorSchemeType::Neon);
    m_pulseShape.setShape(lumi::modules::PulseShape::Circle);
    m_pulseShape.setBaseSize(0.6f);
}

PulsingVisualizer::~PulsingVisualizer() = default;

// =============================================================================
// IModule-style Parameter Access
// =============================================================================

std::vector<lumi::modules::ModuleParamDesc> PulsingVisualizer::paramDescs() const
{
    using namespace lumi::modules;
    std::vector<ModuleParamDesc> params;

    // =========================================================================
    // Color Scheme Parameters
    // =========================================================================
    
    {
        ModuleParamDesc p;
        p.id = "color.scheme";
        p.displayName = "Color Scheme";
        p.tooltip = "Select the color scheme for the visualization";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(ColorSchemeType::Neon);
        p.enumOptions = {"Fire", "Ocean", "Neon", "Rainbow", "Sunset", 
                         "Forest", "Ice", "Lava", "Galaxy", "Monochrome"};
        p.group = "Color";
        p.order = 0;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "color.animSpeed";
        p.displayName = "Animation Speed";
        p.tooltip = "Color cycling speed (cycles per second)";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 5.0f;
        p.defaultValue = 0.5f;
        p.group = "Color";
        p.order = 1;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "color.beatBrightness";
        p.displayName = "Beat Brightness";
        p.tooltip = "Enable brightness response to beats";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.group = "Color";
        p.order = 2;
        params.push_back(p);
    }

    // =========================================================================
    // Shape Parameters
    // =========================================================================
    
    {
        ModuleParamDesc p;
        p.id = "shape.type";
        p.displayName = "Shape";
        p.tooltip = "Select the pulse shape";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(PulseShape::Circle);
        p.enumOptions = {"Circle", "Ring", "NGon", "Star", "Flash", "Wave"};
        p.group = "Shape";
        p.order = 0;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.sides";
        p.displayName = "Sides";
        p.tooltip = "Number of sides for N-gon and Star shapes";
        p.type = ParamType::Int;
        p.minValue = 3.0f;
        p.maxValue = 64.0f;
        p.defaultValue = 6;
        p.group = "Shape";
        p.order = 1;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.baseSize";
        p.displayName = "Base Size";
        p.tooltip = "Base size of the shape (0-2)";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 2.0f;
        p.defaultValue = 0.6f;
        p.group = "Shape";
        p.order = 2;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.innerRadius";
        p.displayName = "Inner Radius";
        p.tooltip = "Inner radius ratio for Ring shape (0-1)";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 0.99f;
        p.defaultValue = 0.5f;
        p.group = "Shape";
        p.order = 3;
        params.push_back(p);
    }

    // =========================================================================
    // Animation Parameters
    // =========================================================================
    
    {
        ModuleParamDesc p;
        p.id = "anim.trigger";
        p.displayName = "Trigger Mode";
        p.tooltip = "What triggers the pulse animation";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(PulseTrigger::Continuous);
        p.enumOptions = {"Continuous", "Beat", "Threshold"};
        p.group = "Animation";
        p.order = 0;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "anim.decay";
        p.displayName = "Decay Mode";
        p.tooltip = "How the pulse fades out";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(PulseDecay::Linear);
        p.enumOptions = {"Linear", "Exponential", "Instant"};
        p.group = "Animation";
        p.order = 1;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "anim.decayTime";
        p.displayName = "Decay Time";
        p.tooltip = "Time for pulse to fade (seconds)";
        p.type = ParamType::Float;
        p.minValue = 0.01f;
        p.maxValue = 5.0f;
        p.defaultValue = 0.5f;
        p.group = "Animation";
        p.order = 2;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "anim.rotationSpeed";
        p.displayName = "Rotation Speed";
        p.tooltip = "Shape rotation speed (degrees/second)";
        p.type = ParamType::Float;
        p.minValue = -360.0f;
        p.maxValue = 360.0f;
        p.defaultValue = 0.0f;
        p.group = "Animation";
        p.order = 3;
        params.push_back(p);
    }

    return params;
}

bool PulsingVisualizer::getParam(const std::string& id, 
                                  lumi::modules::ParamValue& out) const
{
    using namespace lumi::modules;
    
    // Color parameters
    if (id == "color.scheme")
    {
        out = static_cast<int>(m_colorScheme.scheme());
        return true;
    }
    if (id == "color.animSpeed")
    {
        out = m_colorScheme.animationSpeed();
        return true;
    }
    if (id == "color.beatBrightness")
    {
        out = m_colorScheme.beatBrightnessEnabled();
        return true;
    }
    
    // Shape parameters
    if (id == "shape.type")
    {
        out = static_cast<int>(m_pulseShape.shape());
        return true;
    }
    if (id == "shape.sides")
    {
        out = m_pulseShape.sides();
        return true;
    }
    if (id == "shape.baseSize")
    {
        out = m_pulseShape.baseSize();
        return true;
    }
    if (id == "shape.innerRadius")
    {
        out = m_pulseShape.innerRadiusRatio();
        return true;
    }
    
    // Animation parameters
    if (id == "anim.trigger")
    {
        out = static_cast<int>(m_pulseShape.triggerMode());
        return true;
    }
    if (id == "anim.decay")
    {
        out = static_cast<int>(m_pulseShape.decay());
        return true;
    }
    if (id == "anim.decayTime")
    {
        out = m_pulseShape.decayTime();
        return true;
    }
    if (id == "anim.rotationSpeed")
    {
        out = m_pulseShape.rotationSpeed();
        return true;
    }
    
    return false;
}

bool PulsingVisualizer::setParam(const std::string& id, 
                                  const lumi::modules::ParamValue& value)
{
    using namespace lumi::modules;
    
    // Color parameters
    if (id == "color.scheme")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_colorScheme.setScheme(static_cast<ColorSchemeType>(*v));
            return true;
        }
    }
    if (id == "color.animSpeed")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_colorScheme.setAnimationSpeed(*v);
            return true;
        }
    }
    if (id == "color.beatBrightness")
    {
        if (auto* v = std::get_if<bool>(&value))
        {
            m_colorScheme.setBeatBrightnessEnabled(*v);
            return true;
        }
    }
    
    // Shape parameters
    if (id == "shape.type")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_pulseShape.setShape(static_cast<PulseShape>(*v));
            return true;
        }
    }
    if (id == "shape.sides")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_pulseShape.setSides(*v);
            return true;
        }
    }
    if (id == "shape.baseSize")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_pulseShape.setBaseSize(*v);
            return true;
        }
    }
    if (id == "shape.innerRadius")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_pulseShape.setInnerRadiusRatio(*v);
            return true;
        }
    }
    
    // Animation parameters
    if (id == "anim.trigger")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_pulseShape.setTrigger(static_cast<PulseTrigger>(*v));
            return true;
        }
    }
    if (id == "anim.decay")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_pulseShape.setDecay(static_cast<PulseDecay>(*v));
            return true;
        }
    }
    if (id == "anim.decayTime")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_pulseShape.setDecayTime(*v);
            return true;
        }
    }
    if (id == "anim.rotationSpeed")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_pulseShape.setRotationSpeed(*v);
            return true;
        }
    }
    
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
    BasicLogger::logDebug("PulsingVisualizer: Color scheme set to " + 
                          std::to_string(static_cast<int>(scheme)));
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
    return false;
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
    BasicLogger::logDebug("PulsingVisualizer: Smoothing set to " + 
                          std::to_string(milliseconds) + "ms");
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
    
    // Generate circle vertices
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
    
    // Center vertex at origin (0,0)
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    
    // Outer vertices - unit circle (will be scaled by uSize in shader)
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
    // Get viewport from OpenGL state (most reliable)
    // =========================================================================
    GLint viewport[4];
    gl->glGetIntegerv(GL_VIEWPORT, viewport);
    int vpWidth = viewport[2];
    int vpHeight = viewport[3];
    
    // Calculate aspect ratio from actual viewport
    float aspect = 1.0f;
    if (vpHeight > 0)
    {
        aspect = static_cast<float>(vpWidth) / static_cast<float>(vpHeight);
    }
    
    // =========================================================================
    // Animation
    // =========================================================================
    
    // Size pulses between 0.4 and 0.9
    float pulseSize = 0.65f + 0.25f * std::sin(m_totalTime * 2.0f);
    
    // Get audio level if available
    auto spectrum = getSpectrum();
    float audioLevel = 0.0f;
    if (!spectrum.empty())
    {
        float sum = 0.0f;
        int count = std::min(static_cast<int>(spectrum.size()), 32);
        for (int i = 0; i < count; ++i)
        {
            sum += spectrum[i];
        }
        audioLevel = sum / count;
        
        // Modulate size with audio
        pulseSize = 0.4f + 0.5f * audioLevel + 0.1f * std::sin(m_totalTime * 2.0f);
        pulseSize = std::clamp(pulseSize, 0.3f, 1.0f);
    }
    
    // Get color from color scheme module
    auto color = m_colorScheme.sampleAnimated(audioLevel);
    float r = color.r;
    float g = color.g;
    float b = color.b;
    
    // Ensure color is bright enough
    float maxC = std::max({r, g, b});
    if (maxC < 0.3f)
    {
        float boost = 0.5f / (maxC + 0.01f);
        r = std::min(1.0f, r * boost);
        g = std::min(1.0f, g * boost);
        b = std::min(1.0f, b * boost);
    }
    
    // =========================================================================
    // Render
    // =========================================================================
    
    gl->glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    gl->glDisable(GL_DEPTH_TEST);
    gl->glDisable(GL_BLEND);
    
    if (!m_shader || !m_vao || m_vertexCount == 0)
    {
        static int warnCount = 0;
        if (warnCount++ < 5)
        {
            BasicLogger::logWarning("renderPulse: not ready");
        }
        return;
    }
    
    m_shader->bind();
    m_vao->bind();
    
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
    
    // Set uniforms
    gl->glUniform3f(m_uniformColor, r, g, b);
    gl->glUniform1f(m_uniformIntensity, pulseSize);
    gl->glUniform1f(m_uniformAspect, aspect);
    
    // Draw
    gl->glDrawArrays(GL_TRIANGLE_FAN, 0, m_vertexCount);
    
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
// Unused stubs
// =============================================================================

bool PulsingVisualizer::createShaders() { return true; }
void PulsingVisualizer::updateVertexBuffer() {}
void PulsingVisualizer::renderPulse(float, float) {}
float PulsingVisualizer::detectBeat(float) { return 0.0f; }
