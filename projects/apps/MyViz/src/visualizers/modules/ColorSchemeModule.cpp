/**
 ****************************************************************************************
 * @file   ColorSchemeModule.cpp
 * @brief  ColorSchemeModule implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/ColorSchemeModule.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::modules {

// =============================================================================
// Color Static Methods
// =============================================================================

Color Color::fromHex(uint32_t hex, bool hasAlpha)
{
    if (hasAlpha)
    {
        return {
            static_cast<float>((hex >> 24) & 0xFF) / 255.0f,
            static_cast<float>((hex >> 16) & 0xFF) / 255.0f,
            static_cast<float>((hex >> 8) & 0xFF) / 255.0f,
            static_cast<float>(hex & 0xFF) / 255.0f
        };
    }
    else
    {
        return {
            static_cast<float>((hex >> 16) & 0xFF) / 255.0f,
            static_cast<float>((hex >> 8) & 0xFF) / 255.0f,
            static_cast<float>(hex & 0xFF) / 255.0f,
            1.0f
        };
    }
}

Color Color::fromHsv(float h, float s, float v, float a)
{
    // Normalize hue to [0, 360)
    h = std::fmod(h, 360.0f);
    if (h < 0.0f)
    {
        h += 360.0f;
    }

    float c = v * s;
    float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    if (h < 60.0f)
    {
        r = c;
        g = x;
        b = 0.0f;
    }
    else if (h < 120.0f)
    {
        r = x;
        g = c;
        b = 0.0f;
    }
    else if (h < 180.0f)
    {
        r = 0.0f;
        g = c;
        b = x;
    }
    else if (h < 240.0f)
    {
        r = 0.0f;
        g = x;
        b = c;
    }
    else if (h < 300.0f)
    {
        r = x;
        g = 0.0f;
        b = c;
    }
    else
    {
        r = c;
        g = 0.0f;
        b = x;
    }

    return {r + m, g + m, b + m, a};
}

Color Color::lerp(const Color& a, const Color& b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return {
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    };
}

// =============================================================================
// ColorSchemeModule Construction
// =============================================================================

ColorSchemeModule::ColorSchemeModule()
{
    buildScheme(m_scheme);
}

void ColorSchemeModule::reset()
{
    m_animationPhase = 0.0f;
    m_beatIntensity = 0.0f;
}

// =============================================================================
// Scheme Selection
// =============================================================================

void ColorSchemeModule::setScheme(ColorSchemeType scheme)
{
    if (scheme != ColorSchemeType::Custom)
    {
        m_scheme = scheme;
        buildScheme(scheme);
    }
}

void ColorSchemeModule::setCustomGradient(std::vector<GradientStop> stops)
{
    m_scheme = ColorSchemeType::Custom;
    m_stops = std::move(stops);

    // Sort by position
    std::sort(m_stops.begin(), m_stops.end(),
              [](const GradientStop& a, const GradientStop& b) {
                  return a.position < b.position;
              });

    // Ensure we have at least 2 stops
    if (m_stops.empty())
    {
        m_stops.push_back({0.0f, Color::rgb(0.0f, 0.0f, 0.0f)});
        m_stops.push_back({1.0f, Color::rgb(1.0f, 1.0f, 1.0f)});
    }
    else if (m_stops.size() == 1)
    {
        m_stops.push_back({1.0f, m_stops[0].color});
    }
}

// =============================================================================
// Animation
// =============================================================================

void ColorSchemeModule::update(float deltaTime)
{
    if (m_animationSpeed > 0.0f)
    {
        m_animationPhase += deltaTime * m_animationSpeed;
        m_animationPhase = std::fmod(m_animationPhase, 1.0f);
    }
}

// =============================================================================
// Color Sampling
// =============================================================================

Color ColorSchemeModule::sample(float t) const
{
    if (m_stops.empty())
    {
        return Color::rgb(1.0f, 1.0f, 1.0f);
    }

    // Clamp t to [0, 1]
    t = std::clamp(t, 0.0f, 1.0f);

    // Find the two stops to interpolate between
    const GradientStop* lower = &m_stops.front();
    const GradientStop* upper = &m_stops.back();

    for (size_t i = 0; i < m_stops.size() - 1; ++i)
    {
        if (t >= m_stops[i].position && t <= m_stops[i + 1].position)
        {
            lower = &m_stops[i];
            upper = &m_stops[i + 1];
            break;
        }
    }

    // Handle edge cases
    if (t <= m_stops.front().position)
    {
        return applyBeatModulation(m_stops.front().color);
    }
    if (t >= m_stops.back().position)
    {
        return applyBeatModulation(m_stops.back().color);
    }

    // Interpolate
    float range = upper->position - lower->position;
    float localT = (range > 0.0001f) ? (t - lower->position) / range : 0.0f;

    Color result = Color::lerp(lower->color, upper->color, localT);
    return applyBeatModulation(result);
}

Color ColorSchemeModule::sampleAnimated(float t) const
{
    float animatedT = std::fmod(t + m_animationPhase + m_animationOffset, 1.0f);
    return sample(animatedT);
}

Color ColorSchemeModule::sampleAmplitude(float amplitude) const
{
    // Map amplitude [0..1] directly to gradient position
    return sample(amplitude);
}

Color ColorSchemeModule::primaryColor() const
{
    return m_stops.empty() ? Color::rgb(1.0f, 1.0f, 1.0f) : m_stops.front().color;
}

Color ColorSchemeModule::secondaryColor() const
{
    return m_stops.empty() ? Color::rgb(1.0f, 1.0f, 1.0f) : m_stops.back().color;
}

// =============================================================================
// Utility
// =============================================================================

const char* ColorSchemeModule::schemeName(ColorSchemeType scheme)
{
    switch (scheme)
    {
    case ColorSchemeType::Fire:
        return "Fire";
    case ColorSchemeType::Ocean:
        return "Ocean";
    case ColorSchemeType::Neon:
        return "Neon";
    case ColorSchemeType::Rainbow:
        return "Rainbow";
    case ColorSchemeType::Sunset:
        return "Sunset";
    case ColorSchemeType::Forest:
        return "Forest";
    case ColorSchemeType::Ice:
        return "Ice";
    case ColorSchemeType::Lava:
        return "Lava";
    case ColorSchemeType::Galaxy:
        return "Galaxy";
    case ColorSchemeType::Monochrome:
        return "Monochrome";
    case ColorSchemeType::Custom:
        return "Custom";
    }
    return "Unknown";
}

std::vector<const char*> ColorSchemeModule::availableSchemes()
{
    return {
        "Fire",
        "Ocean",
        "Neon",
        "Rainbow",
        "Sunset",
        "Forest",
        "Ice",
        "Lava",
        "Galaxy",
        "Monochrome"
    };
}

// =============================================================================
// Private Methods
// =============================================================================

void ColorSchemeModule::buildScheme(ColorSchemeType scheme)
{
    m_stops.clear();

    switch (scheme)
    {
    case ColorSchemeType::Fire:
        // Black → Dark Red → Red → Orange → Yellow → White
        m_stops = {
            {0.00f, Color::fromHex(0x000000)},
            {0.20f, Color::fromHex(0x8B0000)},  // Dark Red
            {0.40f, Color::fromHex(0xFF0000)},  // Red
            {0.60f, Color::fromHex(0xFF4500)},  // Orange Red
            {0.80f, Color::fromHex(0xFFA500)},  // Orange
            {0.95f, Color::fromHex(0xFFFF00)},  // Yellow
            {1.00f, Color::fromHex(0xFFFFFF)}   // White
        };
        break;

    case ColorSchemeType::Ocean:
        // Dark Blue → Blue → Cyan → Light Cyan → White
        m_stops = {
            {0.00f, Color::fromHex(0x000033)},  // Dark Navy
            {0.25f, Color::fromHex(0x000080)},  // Navy
            {0.50f, Color::fromHex(0x0080FF)},  // Bright Blue
            {0.75f, Color::fromHex(0x00FFFF)},  // Cyan
            {1.00f, Color::fromHex(0xE0FFFF)}   // Light Cyan
        };
        break;

    case ColorSchemeType::Neon:
        // Magenta → Pink → Cyan → Green
        m_stops = {
            {0.00f, Color::fromHex(0xFF00FF)},  // Magenta
            {0.33f, Color::fromHex(0xFF1493)},  // Deep Pink
            {0.66f, Color::fromHex(0x00FFFF)},  // Cyan
            {1.00f, Color::fromHex(0x00FF00)}   // Lime Green
        };
        break;

    case ColorSchemeType::Rainbow:
        // Full HSV spectrum
        m_stops = {
            {0.000f, Color::fromHex(0xFF0000)},  // Red
            {0.167f, Color::fromHex(0xFFFF00)},  // Yellow
            {0.333f, Color::fromHex(0x00FF00)},  // Green
            {0.500f, Color::fromHex(0x00FFFF)},  // Cyan
            {0.667f, Color::fromHex(0x0000FF)},  // Blue
            {0.833f, Color::fromHex(0xFF00FF)},  // Magenta
            {1.000f, Color::fromHex(0xFF0000)}   // Back to Red
        };
        break;

    case ColorSchemeType::Sunset:
        // Purple → Red → Orange → Yellow
        m_stops = {
            {0.00f, Color::fromHex(0x2E0854)},  // Dark Purple
            {0.25f, Color::fromHex(0x8B008B)},  // Dark Magenta
            {0.50f, Color::fromHex(0xFF4500)},  // Orange Red
            {0.75f, Color::fromHex(0xFFA500)},  // Orange
            {1.00f, Color::fromHex(0xFFD700)}   // Gold
        };
        break;

    case ColorSchemeType::Forest:
        // Dark Green → Green → Lime → Yellow
        m_stops = {
            {0.00f, Color::fromHex(0x003300)},  // Very Dark Green
            {0.33f, Color::fromHex(0x006400)},  // Dark Green
            {0.66f, Color::fromHex(0x32CD32)},  // Lime Green
            {1.00f, Color::fromHex(0xADFF2F)}   // Green Yellow
        };
        break;

    case ColorSchemeType::Ice:
        // White → Light Blue → Blue → Dark Blue
        m_stops = {
            {0.00f, Color::fromHex(0xFFFFFF)},  // White
            {0.33f, Color::fromHex(0xE0FFFF)},  // Light Cyan
            {0.66f, Color::fromHex(0x87CEEB)},  // Sky Blue
            {1.00f, Color::fromHex(0x4169E1)}   // Royal Blue
        };
        break;

    case ColorSchemeType::Lava:
        // Black → Dark Red → Red → Orange → Bright Yellow
        m_stops = {
            {0.00f, Color::fromHex(0x000000)},  // Black
            {0.30f, Color::fromHex(0x8B0000)},  // Dark Red
            {0.50f, Color::fromHex(0xFF0000)},  // Red
            {0.70f, Color::fromHex(0xFF4500)},  // Orange Red
            {0.85f, Color::fromHex(0xFFA500)},  // Orange
            {1.00f, Color::fromHex(0xFFFF00)}   // Yellow
        };
        break;

    case ColorSchemeType::Galaxy:
        // Deep Purple → Blue → Pink → White
        m_stops = {
            {0.00f, Color::fromHex(0x0D0221)},  // Very Dark Purple
            {0.33f, Color::fromHex(0x4B0082)},  // Indigo
            {0.50f, Color::fromHex(0x8A2BE2)},  // Blue Violet
            {0.66f, Color::fromHex(0xFF69B4)},  // Hot Pink
            {0.85f, Color::fromHex(0xE6E6FA)},  // Lavender
            {1.00f, Color::fromHex(0xFFFFFF)}   // White
        };
        break;

    case ColorSchemeType::Monochrome:
        // Black → White
        m_stops = {
            {0.00f, Color::fromHex(0x000000)},
            {1.00f, Color::fromHex(0xFFFFFF)}
        };
        break;

    case ColorSchemeType::Custom:
        // Keep existing custom stops, or create default if empty
        if (m_stops.empty())
        {
            m_stops = {
                {0.00f, Color::rgb(0.0f, 0.0f, 0.0f)},
                {1.00f, Color::rgb(1.0f, 1.0f, 1.0f)}
            };
        }
        break;
    }
}

Color ColorSchemeModule::applyBeatModulation(const Color& color) const
{
    if (!m_beatBrightnessEnabled || m_beatIntensity <= 0.0f)
    {
        return color;
    }

    // Calculate brightness multiplier based on beat intensity
    float brightness = m_beatBrightnessMin +
                       (m_beatBrightnessMax - m_beatBrightnessMin) * m_beatIntensity;

    return {
        std::clamp(color.r * brightness, 0.0f, 1.0f),
        std::clamp(color.g * brightness, 0.0f, 1.0f),
        std::clamp(color.b * brightness, 0.0f, 1.0f),
        color.a
    };
}

} // namespace lumi::modules
