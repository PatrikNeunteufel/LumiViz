/**
 ****************************************************************************************
 * @file   PulsingVisualizer.cpp
 * @brief  Audio-reactive pulsing visualizer with shapes and gradients
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 4.0.0
 ****************************************************************************************
 */

#include "visualizers/PulsingVisualizer.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>
#include <vector>

// =============================================================================
// Shape Enum Mapping
// =============================================================================
// PulseShape enum has Flash at index 2, but we don't expose it in UI.
// UI: Circle=0, Ring=1, NGon=2, Star=3
// Enum: Circle=0, Ring=1, Flash=2, Ngon=3, Star=4

namespace {

// Map from UI dropdown index to PulseShape enum
lumi::modules::PulseShape uiIndexToShape(int index)
{
    switch (index)
    {
        case 0: return lumi::modules::PulseShape::Circle;
        case 1: return lumi::modules::PulseShape::Ring;
        case 2: return lumi::modules::PulseShape::Ngon;
        case 3: return lumi::modules::PulseShape::Star;
        default: return lumi::modules::PulseShape::Circle;
    }
}

// Map from PulseShape enum to UI dropdown index
int shapeToUiIndex(lumi::modules::PulseShape shape)
{
    switch (shape)
    {
        case lumi::modules::PulseShape::Circle: return 0;
        case lumi::modules::PulseShape::Ring: return 1;
        case lumi::modules::PulseShape::Ngon: return 2;
        case lumi::modules::PulseShape::Star: return 3;
        default: return 0;
    }
}

// =============================================================================
// Shader Source - With Multi-Stop Gradient Support
// =============================================================================

const char* VERTEX_SHADER_SOURCE = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;

out vec2 vUV;
out vec2 vPosition;

uniform float uAspect;
uniform float uSize;
uniform float uRotation;

void main()
{
    // Apply rotation
    float c = cos(uRotation);
    float s = sin(uRotation);
    vec2 rotated = vec2(
        aPosition.x * c - aPosition.y * s,
        aPosition.x * s + aPosition.y * c
    );
    
    // Scale by size
    vec2 pos = rotated * uSize;
    
    // Correct for aspect ratio
    pos.x /= uAspect;
    
    gl_Position = vec4(pos, 0.0, 1.0);
    vUV = aUV;
    vPosition = aPosition;
}
)";

const char* FRAGMENT_SHADER_SOURCE = R"(
#version 330 core

in vec2 vUV;
in vec2 vPosition;

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
uniform vec4 uMidpoints;    // Midpoints for segments 0-3
uniform vec4 uMidpoints2;   // Midpoints for segments 4-6
uniform int uStopCount;     // Number of active stops (2-8)
uniform int uColorMode;     // 0=Solid, 1=Linear, 2=Radial, 3=Outline
uniform float uGradientAngle;
uniform float uInnerRadius;
uniform float uOutlineWidth;

// Apply midpoint adjustment to local t
// The midpoint defines WHERE the 50/50 blend occurs
float applyMidpoint(float t, float mid)
{
    // Handle edge cases
    if (mid <= 0.001) {
        return t < 0.001 ? 0.5 : 0.5 + t * 0.5;
    }
    if (mid >= 0.999) {
        return t > 0.999 ? 0.5 : t * 0.5;
    }
    
    // Piecewise linear remapping
    // t in [0, mid] -> [0, 0.5]
    // t in [mid, 1] -> [0.5, 1]
    if (t <= mid) {
        return t * 0.5 / mid;
    } else {
        return 0.5 + (t - mid) * 0.5 / (1.0 - mid);
    }
}

// Get color at index (0-7)
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

// Get stop position at index (0-7)
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

// Get midpoint at index (0-6, for segments between stops)
float getMidpoint(int idx)
{
    if (idx < 4) {
        if (idx == 0) return uMidpoints.x;
        if (idx == 1) return uMidpoints.y;
        if (idx == 2) return uMidpoints.z;
        return uMidpoints.w;
    } else {
        if (idx == 4) return uMidpoints2.x;
        if (idx == 5) return uMidpoints2.y;
        return uMidpoints2.z;
    }
}

vec4 sampleGradient(float t)
{
    t = clamp(t, 0.0, 1.0);
    
    // Handle edge cases
    if (uStopCount <= 1 || t <= getStopPos(0)) return uColor0;
    if (t >= getStopPos(uStopCount - 1)) return getColor(uStopCount - 1);
    
    // Find segment and interpolate
    for (int i = 0; i < uStopCount - 1; i++) {
        float pos0 = getStopPos(i);
        float pos1 = getStopPos(i + 1);
        
        if (t >= pos0 && t < pos1) {
            float localT = (t - pos0) / max(pos1 - pos0, 0.001);
            localT = applyMidpoint(localT, getMidpoint(i));
            return mix(getColor(i), getColor(i + 1), localT);
        }
    }
    
    // Fallback
    return uColor0;
}

void main()
{
    vec4 color;
    float dist = length(vPosition);
    
    // Mode 0: Solid color
    if (uColorMode == 0)
    {
        color = uColor0;
    }
    // Mode 1: Linear gradient with angle
    else if (uColorMode == 1)
    {
        float c = cos(uGradientAngle);
        float s = sin(uGradientAngle);
        float t = dot(vPosition, vec2(c, s)) * 0.5 + 0.5;
        color = sampleGradient(t);
    }
    // Mode 2: Radial gradient
    else if (uColorMode == 2)
    {
        float t = dist;
        color = sampleGradient(t);
    }
    // Mode 3: Outline - handled separately via GL_LINE_LOOP
    // This branch shouldn't be reached for outline mode
    else if (uColorMode == 3)
    {
        color = uColor0;
    }
    else
    {
        color = uColor0;
    }
    
    // Ring cutout (for Ring shape)
    if (uInnerRadius > 0.0 && uColorMode != 3)
    {
        if (dist < uInnerRadius)
        {
            discard;
        }
    }
    
    fragColor = color;
}
)";

// Vertex structure with position and UV
struct Vertex
{
    float x, y;
    float u, v;
};

constexpr float PI = 3.14159265358979323846f;

// =============================================================================
// Local Vertex Generation Functions
// =============================================================================

void generateCircleVerts(std::vector<Vertex>& vertices, int segments)
{
    vertices.clear();
    vertices.reserve(segments + 2);
    
    vertices.push_back({0.0f, 0.0f, 0.5f, 0.5f});
    
    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
        float x = std::cos(angle);
        float y = std::sin(angle);
        vertices.push_back({x, y, x * 0.5f + 0.5f, y * 0.5f + 0.5f});
    }
}

void generateNGonVerts(std::vector<Vertex>& vertices, int sides)
{
    vertices.clear();
    vertices.reserve(sides + 2);
    
    vertices.push_back({0.0f, 0.0f, 0.5f, 0.5f});
    
    for (int i = 0; i <= sides; ++i)
    {
        float angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(sides);
        angle -= PI / 2.0f;  // Start at top
        float x = std::cos(angle);
        float y = std::sin(angle);
        vertices.push_back({x, y, x * 0.5f + 0.5f, y * 0.5f + 0.5f});
    }
}

void generateStarVerts(std::vector<Vertex>& vertices, int points)
{
    vertices.clear();
    int totalVertices = points * 2;
    vertices.reserve(totalVertices + 2);
    
    vertices.push_back({0.0f, 0.0f, 0.5f, 0.5f});
    
    float innerRadiusRatio = 0.4f;
    
    for (int i = 0; i <= totalVertices; ++i)
    {
        float angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(totalVertices);
        angle -= PI / 2.0f;  // Start at top
        
        float r = (i % 2 == 0) ? 1.0f : innerRadiusRatio;
        float x = r * std::cos(angle);
        float y = r * std::sin(angle);
        vertices.push_back({x, y, x * 0.5f + 0.5f, y * 0.5f + 0.5f});
    }
}

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
    
    m_pulseShape.setShape(lumi::modules::PulseShape::Circle);
    m_colorGradient.loadPreset("Neon");
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
    // 1. Audio Source Parameters
    // =========================================================================
    
    for (const auto& p : m_audioSource.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "1. Audio";
        
        // Also prefix the dependsOn reference if set
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "audio." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }

    // =========================================================================
    // 2. Shape Parameters (including Color as sub-section)
    // =========================================================================
    
    {
        ModuleParamDesc p;
        p.id = "shape.type";
        p.displayName = "Shape";
        p.tooltip = "Select the pulse shape";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Circle", "Ring", "NGon", "Star"};
        p.group = "2. Shape";
        p.order = 0;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.sides";
        p.displayName = "Sides/Points";
        p.tooltip = "Number of sides (NGon) or points (Star)";
        p.type = ParamType::Int;
        p.minValue = 3.0f;
        p.maxValue = 32.0f;
        p.defaultValue = 6;
        p.group = "2. Shape";
        p.order = 1;
        p.dependsOn = "shape.type";
        p.dependsValues = {2, 3};  // NGon=2, Star=3
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.innerRadius";
        p.displayName = "Inner Radius";
        p.tooltip = "Inner radius for Ring (0 = filled)";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 0.95f;
        p.defaultValue = 0.5f;
        p.group = "2. Shape";
        p.order = 2;
        p.dependsOn = "shape.type";
        p.dependsValues = {1};  // Ring=1
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.minSize";
        p.displayName = "Min Size";
        p.tooltip = "Minimum size at silence";
        p.type = ParamType::Float;
        p.minValue = 0.05f;
        p.maxValue = 1.5f;
        p.defaultValue = 0.3f;
        p.group = "2. Shape";
        p.order = 3;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.maxSize";
        p.displayName = "Max Size";
        p.tooltip = "Maximum size at peak audio";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 2.0f;
        p.defaultValue = 0.9f;
        p.group = "2. Shape";
        p.order = 4;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.rotation";
        p.displayName = "Rotation Speed";
        p.tooltip = "Degrees per second";
        p.type = ParamType::Float;
        p.minValue = -360.0f;
        p.maxValue = 360.0f;
        p.defaultValue = 0.0f;
        p.group = "2. Shape";
        p.order = 5;
        params.push_back(p);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.beatReverse";
        p.displayName = "Beat Reverse";
        p.tooltip = "Reverse rotation direction on beat";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.group = "2. Shape";
        p.order = 6;
        params.push_back(p);
    }

    // =========================================================================
    // Color Sub-Section (under Shape group)
    // =========================================================================
    
    // Delegate to ColorGradientModule params
    for (const auto& p : m_colorGradient.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "shape.color." + p.id;
        prefixed.group = "2. Shape";
        prefixed.order = 10 + p.order;  // After shape params
        
        // Also prefix the dependsOn reference if set
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "shape.color." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }
    
    {
        ModuleParamDesc p;
        p.id = "shape.color.beatBrightness";
        p.displayName = "Beat Brightness";
        p.tooltip = "Modulate brightness with audio";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.group = "2. Shape";
        p.order = 20;
        params.push_back(p);
    }

    return params;
}

bool PulsingVisualizer::getParam(const std::string& id, 
                                  lumi::modules::ParamValue& out) const
{
    using namespace lumi::modules;
    
    // Audio parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }
    
    // Color gradient parameters (under shape.color.*)
    if (id.rfind("shape.color.", 0) == 0)
    {
        std::string gradientId = id.substr(12);  // Remove "shape.color."
        
        if (gradientId == "beatBrightness")
        {
            out = m_beatBrightnessEnabled;
            return true;
        }
        
        return m_colorGradient.getParam(gradientId, out);
    }
    
    // Shape parameters
    if (id == "shape.type")
    {
        out = shapeToUiIndex(m_pulseShape.shape());
        return true;
    }
    if (id == "shape.sides")
    {
        out = m_pulseShape.sides();
        return true;
    }
    if (id == "shape.innerRadius")
    {
        out = m_innerRadius;
        return true;
    }
    if (id == "shape.minSize")
    {
        out = m_minSize;
        return true;
    }
    if (id == "shape.maxSize")
    {
        out = m_maxSize;
        return true;
    }
    if (id == "shape.rotation")
    {
        out = m_rotationSpeed;
        return true;
    }
    if (id == "shape.beatReverse")
    {
        out = m_beatReverseRotation;
        return true;
    }
    
    return false;
}

bool PulsingVisualizer::setParam(const std::string& id, 
                                  const lumi::modules::ParamValue& value)
{
    using namespace lumi::modules;
    
    // Audio parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }
    
    // Color gradient parameters
    if (id.rfind("shape.color.", 0) == 0)
    {
        std::string gradientId = id.substr(12);
        
        if (gradientId == "beatBrightness")
        {
            if (auto* v = std::get_if<bool>(&value))
            {
                m_beatBrightnessEnabled = *v;
                return true;
            }
        }
        
        return m_colorGradient.setParam(gradientId, value);
    }
    
    // Shape parameters
    if (id == "shape.type")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_pulseShape.setShape(uiIndexToShape(*v));
            m_needsRebuild = true;
            return true;
        }
    }
    if (id == "shape.sides")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_pulseShape.setSides(*v);
            m_needsRebuild = true;
            return true;
        }
    }
    if (id == "shape.innerRadius")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_innerRadius = std::clamp(*v, 0.0f, 0.95f);
            return true;
        }
    }
    if (id == "shape.minSize")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_minSize = std::clamp(*v, 0.05f, 1.5f);
            return true;
        }
    }
    if (id == "shape.maxSize")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_maxSize = std::clamp(*v, 0.1f, 2.0f);
            return true;
        }
    }
    if (id == "shape.rotation")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_rotationSpeed = *v;
            return true;
        }
    }
    if (id == "shape.beatReverse")
    {
        if (auto* v = std::get_if<bool>(&value))
        {
            m_beatReverseRotation = *v;
            return true;
        }
    }
    
    return false;
}

// =============================================================================
// Legacy API
// =============================================================================

void PulsingVisualizer::setShape(lumi::modules::PulseShape shape)
{
    m_pulseShape.setShape(shape);
    m_needsRebuild = true;
}

lumi::modules::PulseShape PulsingVisualizer::shape() const
{
    return m_pulseShape.shape();
}

void PulsingVisualizer::setSides(int sides)
{
    m_pulseShape.setSides(sides);
    m_needsRebuild = true;
}

int PulsingVisualizer::sides() const
{
    return m_pulseShape.sides();
}

void PulsingVisualizer::loadGradientPreset(const std::string& name)
{
    m_colorGradient.loadPreset(name);
}

void PulsingVisualizer::setRotationSpeed(float degreesPerSecond)
{
    m_rotationSpeed = degreesPerSecond;
}

float PulsingVisualizer::rotationSpeed() const
{
    return m_rotationSpeed;
}

void PulsingVisualizer::setBeatReverseRotation(bool enabled)
{
    m_beatReverseRotation = enabled;
}

bool PulsingVisualizer::beatReverseRotation() const
{
    return m_beatReverseRotation;
}

void PulsingVisualizer::setBeatBrightnessEnabled(bool enabled)
{
    m_beatBrightnessEnabled = enabled;
}

bool PulsingVisualizer::beatBrightnessEnabled() const
{
    return m_beatBrightnessEnabled;
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
    
    m_shader = std::make_unique<QOpenGLShaderProgram>();
    
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER_SOURCE))
    {
        BasicLogger::logError("Vertex shader failed: " + m_shader->log().toStdString());
        return;
    }
    
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAGMENT_SHADER_SOURCE))
    {
        BasicLogger::logError("Fragment shader failed: " + m_shader->log().toStdString());
        return;
    }
    
    if (!m_shader->link())
    {
        BasicLogger::logError("Shader link failed: " + m_shader->log().toStdString());
        return;
    }
    
    // Get uniform locations
    m_uniformAspect = m_shader->uniformLocation("uAspect");
    m_uniformSize = m_shader->uniformLocation("uSize");
    m_uniformRotation = m_shader->uniformLocation("uRotation");
    m_uniformInnerRadius = m_shader->uniformLocation("uInnerRadius");
    m_uniformOutlineWidth = m_shader->uniformLocation("uOutlineWidth");
    m_uniformColorMode = m_shader->uniformLocation("uColorMode");
    m_uniformGradientAngle = m_shader->uniformLocation("uGradientAngle");
    m_uniformColor0 = m_shader->uniformLocation("uColor0");
    m_uniformColor1 = m_shader->uniformLocation("uColor1");
    m_uniformColor2 = m_shader->uniformLocation("uColor2");
    m_uniformColor3 = m_shader->uniformLocation("uColor3");
    m_uniformColor4 = m_shader->uniformLocation("uColor4");
    m_uniformColor5 = m_shader->uniformLocation("uColor5");
    m_uniformColor6 = m_shader->uniformLocation("uColor6");
    m_uniformColor7 = m_shader->uniformLocation("uColor7");
    m_uniformStopPos = m_shader->uniformLocation("uStopPos");
    m_uniformStopPos2 = m_shader->uniformLocation("uStopPos2");
    m_uniformMidpoints = m_shader->uniformLocation("uMidpoints");
    m_uniformMidpoints2 = m_shader->uniformLocation("uMidpoints2");
    m_uniformStopCount = m_shader->uniformLocation("uStopCount");
    
    // Create VAO
    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    if (!m_vao->create())
    {
        BasicLogger::logError("VAO creation failed");
        return;
    }
    
    // Create VBO
    m_vertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    if (!m_vertexBuffer->create())
    {
        BasicLogger::logError("VBO creation failed");
        return;
    }
    
    rebuildShape();
    
    BasicLogger::logInfo("PulsingVisualizer::onInitialize() - SUCCESS");
}

void PulsingVisualizer::rebuildShape()
{
    using namespace lumi::modules;
    
    std::vector<Vertex> vertices;
    
    PulseShape shape = m_pulseShape.shape();
    int sides = m_pulseShape.sides();
    
    switch (shape)
    {
        case PulseShape::Circle:
        case PulseShape::Ring:
            generateCircleVerts(vertices, 64);
            break;
            
        case PulseShape::Ngon:
            generateNGonVerts(vertices, sides);
            break;
            
        case PulseShape::Star:
            generateStarVerts(vertices, sides);
            break;
            
        default:
            generateCircleVerts(vertices, 64);
            break;
    }
    
    m_shader->bind();
    m_vao->bind();
    m_vertexBuffer->bind();
    m_vertexBuffer->allocate(vertices.data(), 
                             static_cast<int>(vertices.size() * sizeof(Vertex)));
    
    m_shader->enableAttributeArray(0);
    m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(Vertex));
    
    m_shader->enableAttributeArray(1);
    m_shader->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 2, sizeof(Vertex));
    
    m_vertexBuffer->release();
    m_vao->release();
    m_shader->release();
    
    m_vertexCount = static_cast<int>(vertices.size());
    m_needsRebuild = false;
    
    BasicLogger::logDebug("PulsingVisualizer: Shape rebuilt (" + 
                          std::to_string(static_cast<int>(shape)) + 
                          ") with " + std::to_string(m_vertexCount) + " vertices");
}

void PulsingVisualizer::onRender(float deltaTime)
{
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!gl)
    {
        return;
    }
    
    m_totalTime += deltaTime;
    
    // Rebuild shape if needed
    if (m_needsRebuild)
    {
        rebuildShape();
    }
    
    // =========================================================================
    // Get Audio Data
    // =========================================================================
    
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
        audioLevel = sum / static_cast<float>(count);
        audioLevel *= m_audioSource.gain();
        audioLevel = std::clamp(audioLevel, 0.0f, 1.0f);
    }
    
    // =========================================================================
    // Beat Detection for Rotation Reversal
    // =========================================================================
    
    if (m_beatReverseRotation)
    {
        // Simple beat detection
        float beatLevel = audioLevel * m_beatSensitivity;
        if (beatLevel > m_beatThreshold && m_lastBassLevel <= m_beatThreshold)
        {
            // Beat detected - reverse direction
            m_rotationDirection *= -1.0f;
        }
        m_lastBassLevel = beatLevel;
    }
    
    // Update rotation
    m_currentRotation += m_rotationSpeed * m_rotationDirection * deltaTime;
    
    // =========================================================================
    // Calculate Size
    // =========================================================================
    
    float size = m_minSize + (m_maxSize - m_minSize) * audioLevel;
    size = std::clamp(size, m_minSize, m_maxSize);
    
    // =========================================================================
    // Get Colors from Gradient (up to 8 stops)
    // =========================================================================
    
    const auto& stops = m_colorGradient.stops();
    size_t stopCount = std::min(stops.size(), static_cast<size_t>(8));
    
    // Prepare colors and positions for shader
    lumi::modules::Color4f colors[8];
    float positions[8] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    
    for (size_t i = 0; i < stopCount; ++i)
    {
        colors[i] = stops[i].color;
        positions[i] = stops[i].position;
    }
    
    // Fill remaining with last color
    for (size_t i = stopCount; i < 8; ++i)
    {
        colors[i] = stopCount > 0 ? colors[stopCount - 1] : lumi::modules::Color4f{1, 1, 1, 1};
    }
    
    // Apply beat brightness
    if (m_beatBrightnessEnabled)
    {
        float brightnessMod = 0.5f + 0.5f * audioLevel;
        for (size_t i = 0; i < 8; ++i)
        {
            colors[i][0] *= brightnessMod;
            colors[i][1] *= brightnessMod;
            colors[i][2] *= brightnessMod;
        }
    }
    
    // =========================================================================
    // Get Viewport
    // =========================================================================
    
    GLint viewport[4];
    gl->glGetIntegerv(GL_VIEWPORT, viewport);
    float aspect = 1.0f;
    if (viewport[3] > 0)
    {
        aspect = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);
    }
    
    // =========================================================================
    // Render
    // =========================================================================
    
    gl->glClearColor(m_bgColorR, m_bgColorG, m_bgColorB, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    gl->glDisable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if (!m_shader || !m_vao || m_vertexCount == 0)
    {
        return;
    }
    
    m_shader->bind();
    m_vao->bind();
    
    // Set uniforms
    gl->glUniform1f(m_uniformAspect, aspect);
    gl->glUniform1f(m_uniformSize, size);
    gl->glUniform1f(m_uniformRotation, m_currentRotation * PI / 180.0f);
    
    // Color mode
    int colorMode = static_cast<int>(m_colorGradient.mode());
    gl->glUniform1i(m_uniformColorMode, colorMode);
    gl->glUniform1f(m_uniformGradientAngle, m_colorGradient.angle() * PI / 180.0f);
    
    // Colors (8 stops)
    gl->glUniform4f(m_uniformColor0, colors[0][0], colors[0][1], colors[0][2], colors[0][3]);
    gl->glUniform4f(m_uniformColor1, colors[1][0], colors[1][1], colors[1][2], colors[1][3]);
    gl->glUniform4f(m_uniformColor2, colors[2][0], colors[2][1], colors[2][2], colors[2][3]);
    gl->glUniform4f(m_uniformColor3, colors[3][0], colors[3][1], colors[3][2], colors[3][3]);
    gl->glUniform4f(m_uniformColor4, colors[4][0], colors[4][1], colors[4][2], colors[4][3]);
    gl->glUniform4f(m_uniformColor5, colors[5][0], colors[5][1], colors[5][2], colors[5][3]);
    gl->glUniform4f(m_uniformColor6, colors[6][0], colors[6][1], colors[6][2], colors[6][3]);
    gl->glUniform4f(m_uniformColor7, colors[7][0], colors[7][1], colors[7][2], colors[7][3]);
    
    // Stop positions (2x vec4)
    gl->glUniform4f(m_uniformStopPos, positions[0], positions[1], positions[2], positions[3]);
    gl->glUniform4f(m_uniformStopPos2, positions[4], positions[5], positions[6], positions[7]);
    
    // Midpoints - get from gradient module (7 midpoints for 8 stops)
    const auto& midpoints = m_colorGradient.midpoints();
    float mid0 = midpoints.size() > 0 ? midpoints[0].position : 0.5f;
    float mid1 = midpoints.size() > 1 ? midpoints[1].position : 0.5f;
    float mid2 = midpoints.size() > 2 ? midpoints[2].position : 0.5f;
    float mid3 = midpoints.size() > 3 ? midpoints[3].position : 0.5f;
    float mid4 = midpoints.size() > 4 ? midpoints[4].position : 0.5f;
    float mid5 = midpoints.size() > 5 ? midpoints[5].position : 0.5f;
    float mid6 = midpoints.size() > 6 ? midpoints[6].position : 0.5f;
    gl->glUniform4f(m_uniformMidpoints, mid0, mid1, mid2, mid3);
    gl->glUniform4f(m_uniformMidpoints2, mid4, mid5, mid6, 0.5f);
    
    gl->glUniform1i(m_uniformStopCount, static_cast<int>(stopCount));
    
    // Inner radius only for Ring
    float innerR = (m_pulseShape.shape() == lumi::modules::PulseShape::Ring) 
                   ? m_innerRadius : 0.0f;
    gl->glUniform1f(m_uniformInnerRadius, innerR);
    
    // Outline width (convert pixels to normalized units)
    float outlinePixels = m_colorGradient.outlineWidth();
    float outlineNorm = outlinePixels / static_cast<float>(std::min(viewport[2], viewport[3]));
    gl->glUniform1f(m_uniformOutlineWidth, outlineNorm);
    
    // Draw
    if (colorMode == 3)  // Outline mode - use double-draw technique
    {
        // glLineWidth is often capped at 1.0 on modern GPUs
        // Instead: draw shape larger, then draw smaller shape with background color
        
        // Get the solid color for outline
        auto solidColor = m_colorGradient.solidColor();
        
        // Apply beat brightness to outline color too
        if (m_beatBrightnessEnabled)
        {
            float brightnessMod = 0.5f + 0.5f * audioLevel;
            solidColor[0] *= brightnessMod;
            solidColor[1] *= brightnessMod;
            solidColor[2] *= brightnessMod;
        }
        
        // 1. Draw outer shape (larger) with solid color
        float outerSize = size + outlineNorm * 2.0f;
        gl->glUniform1f(m_uniformSize, outerSize);
        gl->glUniform1i(m_uniformColorMode, 0);  // Solid color mode
        gl->glUniform4f(m_uniformColor0, solidColor[0], solidColor[1], solidColor[2], solidColor[3]);
        gl->glDrawArrays(GL_TRIANGLE_FAN, 0, m_vertexCount);
        
        // 2. Draw inner shape with background color (punch out center)
        gl->glUniform1f(m_uniformSize, size);
        gl->glUniform4f(m_uniformColor0, m_bgColorR, m_bgColorG, m_bgColorB, 1.0f);
        gl->glDrawArrays(GL_TRIANGLE_FAN, 0, m_vertexCount);
    }
    else
    {
        gl->glDrawArrays(GL_TRIANGLE_FAN, 0, m_vertexCount);
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
// Stubs
// =============================================================================

bool PulsingVisualizer::createShaders() { return true; }
void PulsingVisualizer::updateVertexBuffer() { rebuildShape(); }
void PulsingVisualizer::renderPulse(float, float) {}
float PulsingVisualizer::detectBeat(float) { return 0.0f; }
