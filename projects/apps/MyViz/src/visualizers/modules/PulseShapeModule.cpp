/**
 ****************************************************************************************
 * @file   PulseShapeModule.cpp
 * @brief  PulseShapeModule implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/PulseShapeModule.hpp"

#include <cmath>
#include <algorithm>

namespace lumi::modules {

namespace {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 2.0f * PI;
    constexpr float DEG_TO_RAD = PI / 180.0f;
}

// =============================================================================
// Construction
// =============================================================================

PulseShapeModule::PulseShapeModule()
{
    reset();
}

void PulseShapeModule::reset()
{
    m_state = PulseState{};
    m_currentAudioLevel = 0.0f;
    m_accumulatedRotation = 0.0f;
}

// =============================================================================
// Update & Trigger
// =============================================================================

void PulseShapeModule::update(float deltaTime, float audioLevel)
{
    m_currentAudioLevel = audioLevel;

    // Update rotation
    m_accumulatedRotation += m_rotationSpeed * DEG_TO_RAD * deltaTime;

    // Wrap rotation
    if (m_accumulatedRotation > TWO_PI)
    {
        m_accumulatedRotation -= TWO_PI;
    }
    else if (m_accumulatedRotation < 0.0f)
    {
        m_accumulatedRotation += TWO_PI;
    }

    switch (m_trigger)
    {
    case PulseTrigger::Continuous:
    {
        // Smooth follow of audio level
        float target = audioLevel;
        if (m_attackTime > 0.0f && target > m_state.intensity)
        {
            // Attack
            float attackRate = 1.0f / m_attackTime;
            m_state.intensity += attackRate * deltaTime;
            m_state.intensity = std::min(m_state.intensity, target);
        }
        else if (m_decayTime > 0.0f && target < m_state.intensity)
        {
            // Decay
            float decayProgress = deltaTime / m_decayTime;
            m_state.intensity = std::max(target, m_state.intensity - decayProgress);
        }
        else
        {
            m_state.intensity = target;
        }
        m_state.active = m_state.intensity > 0.01f;
        m_state.size = m_state.intensity;
        break;
    }

    case PulseTrigger::OnBeat:
    case PulseTrigger::OnThreshold:
    {
        // Threshold trigger check
        if (m_trigger == PulseTrigger::OnThreshold && audioLevel >= m_threshold)
        {
            if (!m_state.active)
            {
                trigger(audioLevel);
            }
        }

        // Decay active pulse
        if (m_state.active)
        {
            m_state.timeSinceTrigger += deltaTime;
            float decayT = m_state.timeSinceTrigger / m_decayTime;

            if (decayT >= 1.0f)
            {
                // Fully decayed
                m_state.active = false;
                m_state.intensity = 0.0f;
                m_state.size = 0.0f;
            }
            else
            {
                m_state.intensity = calculateDecay(decayT);
                m_state.size = m_state.intensity;
            }
        }
        break;
    }
    }

    // Update phase for animated shapes
    m_state.phase += deltaTime * m_waveSpeed;
    if (m_state.phase > 1.0f)
    {
        m_state.phase -= 1.0f;
    }

    // Update rotation
    m_state.rotation = effectiveRotation();
}

void PulseShapeModule::trigger(float intensity)
{
    m_state.active = true;
    m_state.intensity = std::clamp(intensity, 0.0f, 1.0f);
    m_state.size = m_state.intensity;
    m_state.timeSinceTrigger = 0.0f;
}

float PulseShapeModule::effectiveSize() const
{
    float sizeRange = m_sizeMax - m_sizeMin;
    float sizeMultiplier = m_sizeMin + sizeRange * m_state.intensity;
    return m_baseSize * sizeMultiplier;
}

float PulseShapeModule::effectiveRotation() const
{
    float rotation = m_baseRotation * DEG_TO_RAD + m_accumulatedRotation;

    if (m_audioRotationEnabled)
    {
        rotation += m_currentAudioLevel * m_audioRotationScale * DEG_TO_RAD;
    }

    return rotation;
}

// =============================================================================
// Decay Calculation
// =============================================================================

float PulseShapeModule::calculateDecay(float t) const
{
    t = std::clamp(t, 0.0f, 1.0f);

    switch (m_decay)
    {
    case PulseDecay::Linear:
        return 1.0f - t;

    case PulseDecay::Exponential:
        // Smooth exponential decay
        return std::exp(-5.0f * t);

    case PulseDecay::Hold:
    {
        // Hold at full intensity for first half, then drop
        if (t < 0.5f)
        {
            return 1.0f;
        }
        float localT = (t - 0.5f) * 2.0f;  // Normalize remaining half
        return 1.0f - localT;
    }

    case PulseDecay::Bounce:
    {
        // Bounce effect using damped sine
        float dampening = std::exp(-3.0f * t);
        float bounce = std::abs(std::sin(t * PI * 3.0f));
        return dampening * bounce;
    }
    }

    return 1.0f - t;  // Default: linear
}

// =============================================================================
// Vertex Generation
// =============================================================================

std::vector<ShapeVertex> PulseShapeModule::generateVertices(int segments) const
{
    switch (m_shape)
    {
    case PulseShape::Circle:
        return generateCircle(segments);

    case PulseShape::Ring:
        return generateRing(segments);

    case PulseShape::Flash:
        // Flash is fullscreen quad, handled separately
        return {
            {-1.0f, -1.0f, 0.0f, 0.0f},
            { 1.0f, -1.0f, 1.0f, 0.0f},
            { 1.0f,  1.0f, 1.0f, 1.0f},
            {-1.0f,  1.0f, 0.0f, 1.0f}
        };

    case PulseShape::Ngon:
        return generateNgon();

    case PulseShape::Star:
        return generateStar();

    case PulseShape::Wave:
        // Multiple rings with phase offset
        {
            std::vector<ShapeVertex> allVertices;
            for (int w = 0; w < m_waveCount; ++w)
            {
                float phaseOffset = static_cast<float>(w) / m_waveCount;
                float wavePhase = std::fmod(m_state.phase + phaseOffset, 1.0f);

                // Create ring at wave position
                float innerR = wavePhase * 0.9f;
                float outerR = innerR + 0.1f;

                // Store temporarily, would need modification for multi-ring
                auto ring = generateRing(segments);
                allVertices.insert(allVertices.end(), ring.begin(), ring.end());
            }
            return allVertices;
        }

    case PulseShape::RadialBars:
    case PulseShape::Blob:
    case PulseShape::Tunnel:
    case PulseShape::Grid:
        // Complex shapes - return circle as fallback
        return generateCircle(segments);
    }

    return generateCircle(segments);
}

std::vector<ShapeVertex> PulseShapeModule::generateCircle(int segments) const
{
    std::vector<ShapeVertex> vertices;
    vertices.reserve(segments + 2);

    float size = effectiveSize();
    float rotation = effectiveRotation();

    // Center vertex
    vertices.push_back({m_centerX, m_centerY, 0.5f, 0.0f});

    // Outer vertices
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (static_cast<float>(i) / segments) * TWO_PI + rotation;
        float u = static_cast<float>(i) / segments;

        float x = m_centerX + std::cos(angle) * size;
        float y = m_centerY + std::sin(angle) * size;

        vertices.push_back({x, y, u, 1.0f});
    }

    return vertices;
}

std::vector<ShapeVertex> PulseShapeModule::generateRing(int segments) const
{
    std::vector<ShapeVertex> vertices;
    vertices.reserve((segments + 1) * 2);

    float size = effectiveSize();
    float innerSize = size * m_innerRadiusRatio;
    float rotation = effectiveRotation();

    // Generate triangle strip for ring
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (static_cast<float>(i) / segments) * TWO_PI + rotation;
        float u = static_cast<float>(i) / segments;

        float cosA = std::cos(angle);
        float sinA = std::sin(angle);

        // Inner vertex
        float xInner = m_centerX + cosA * innerSize;
        float yInner = m_centerY + sinA * innerSize;
        vertices.push_back({xInner, yInner, u, 0.0f});

        // Outer vertex
        float xOuter = m_centerX + cosA * size;
        float yOuter = m_centerY + sinA * size;
        vertices.push_back({xOuter, yOuter, u, 1.0f});
    }

    return vertices;
}

std::vector<ShapeVertex> PulseShapeModule::generateNgon() const
{
    std::vector<ShapeVertex> vertices;
    vertices.reserve(m_sides + 2);

    float size = effectiveSize();
    float rotation = effectiveRotation();

    // Center vertex
    vertices.push_back({m_centerX, m_centerY, 0.5f, 0.0f});

    // Corner vertices
    for (int i = 0; i <= m_sides; ++i)
    {
        float angle = (static_cast<float>(i) / m_sides) * TWO_PI + rotation;
        float u = static_cast<float>(i) / m_sides;

        float x = m_centerX + std::cos(angle) * size;
        float y = m_centerY + std::sin(angle) * size;

        vertices.push_back({x, y, u, 1.0f});
    }

    return vertices;
}

std::vector<ShapeVertex> PulseShapeModule::generateStar() const
{
    std::vector<ShapeVertex> vertices;
    vertices.reserve(m_sides * 2 + 2);

    float outerSize = effectiveSize();
    float innerSize = outerSize * 0.4f;  // Inner radius for star points
    float rotation = effectiveRotation();

    // Center vertex
    vertices.push_back({m_centerX, m_centerY, 0.5f, 0.0f});

    // Alternate between outer points and inner valleys
    int totalPoints = m_sides * 2;
    for (int i = 0; i <= totalPoints; ++i)
    {
        float angle = (static_cast<float>(i) / totalPoints) * TWO_PI + rotation;
        float u = static_cast<float>(i) / totalPoints;

        // Alternate between outer and inner radius
        float radius = (i % 2 == 0) ? outerSize : innerSize;
        float v = (i % 2 == 0) ? 1.0f : 0.4f;

        float x = m_centerX + std::cos(angle) * radius;
        float y = m_centerY + std::sin(angle) * radius;

        vertices.push_back({x, y, u, v});
    }

    return vertices;
}

// =============================================================================
// Utility
// =============================================================================

const char* PulseShapeModule::shapeName(PulseShape shape)
{
    switch (shape)
    {
    case PulseShape::Circle:
        return "Circle";
    case PulseShape::Ring:
        return "Ring";
    case PulseShape::Flash:
        return "Flash";
    case PulseShape::Ngon:
        return "N-gon";
    case PulseShape::Star:
        return "Star";
    case PulseShape::Wave:
        return "Wave";
    case PulseShape::RadialBars:
        return "Radial Bars";
    case PulseShape::Blob:
        return "Blob";
    case PulseShape::Tunnel:
        return "Tunnel";
    case PulseShape::Grid:
        return "Grid";
    }
    return "Unknown";
}

std::vector<const char*> PulseShapeModule::availableShapes()
{
    return {
        "Circle",
        "Ring",
        "Flash",
        "N-gon",
        "Star",
        "Wave",
        "Radial Bars",
        "Blob",
        "Tunnel",
        "Grid"
    };
}

} // namespace lumi::modules
