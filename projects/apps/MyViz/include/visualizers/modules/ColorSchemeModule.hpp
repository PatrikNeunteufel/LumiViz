/**
 ****************************************************************************************
 * @file   ColorSchemeModule.hpp
 * @brief  Color scheme module for audio-reactive visualizers
 *
 * Provides various color schemes and gradients for visualization effects.
 * Supports both predefined schemes (Fire, Ocean, Neon, Rainbow) and custom gradients.
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstdint>

namespace lumi::modules {

// =============================================================================
// Color Types
// =============================================================================

/**
 * @brief RGBA color with normalized float components [0..1]
 */
struct Color
{
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    /// @brief Create from RGB (alpha defaults to 1.0)
    static constexpr Color rgb(float r, float g, float b)
    {
        return {r, g, b, 1.0f};
    }

    /// @brief Create from RGBA
    static constexpr Color rgba(float r, float g, float b, float a)
    {
        return {r, g, b, a};
    }

    /// @brief Create from hex color (0xRRGGBB or 0xRRGGBBAA)
    static Color fromHex(uint32_t hex, bool hasAlpha = false);

    /// @brief Create from HSV (hue: 0-360, sat/val: 0-1)
    static Color fromHsv(float h, float s, float v, float a = 1.0f);

    /// @brief Linear interpolation between two colors
    static Color lerp(const Color& a, const Color& b, float t);
};

// =============================================================================
// Color Scheme Presets
// =============================================================================

/**
 * @brief Predefined color scheme types
 */
enum class ColorSchemeType
{
    Fire,       ///< Red → Orange → Yellow
    Ocean,      ///< Dark Blue → Cyan → White
    Neon,       ///< Magenta → Cyan → Green
    Rainbow,    ///< Full spectrum HSV cycle
    Sunset,     ///< Purple → Red → Orange → Yellow
    Forest,     ///< Dark Green → Lime → Yellow
    Ice,        ///< White → Light Blue → Dark Blue
    Lava,       ///< Black → Red → Orange → Yellow
    Galaxy,     ///< Deep Purple → Blue → Pink
    Monochrome, ///< Black → White
    Custom      ///< User-defined gradient stops
};

/**
 * @brief Gradient domain - what controls the color
 */
enum class GradientDomain
{
    Position,   ///< Color based on position (e.g., band index)
    Amplitude,  ///< Color based on audio amplitude
    Time,       ///< Color cycles over time
    Beat        ///< Color pulses with beat
};

// =============================================================================
// Gradient Stop
// =============================================================================

/**
 * @brief A single color stop in a gradient
 */
struct GradientStop
{
    float position = 0.0f;  ///< Position in gradient [0..1]
    Color color;            ///< Color at this position
};

// =============================================================================
// ColorSchemeModule
// =============================================================================

/**
 * @class ColorSchemeModule
 * @brief Module for color scheme management and gradient sampling
 *
 * Provides color values based on position, amplitude, or time.
 * Supports both predefined schemes and custom gradients.
 *
 * @par Example Usage
 * @code
 * ColorSchemeModule colorScheme;
 * colorScheme.setScheme(ColorSchemeType::Fire);
 * colorScheme.setDomain(GradientDomain::Amplitude);
 *
 * // Get color for amplitude value
 * Color c = colorScheme.sample(0.75f);  // 75% amplitude
 *
 * // Or sample with time modulation
 * colorScheme.update(deltaTime);
 * Color c = colorScheme.sampleWithTime(amplitude, currentTime);
 * @endcode
 */
class ColorSchemeModule
{
public:
    // =========================================================================
    // Construction
    // =========================================================================

    ColorSchemeModule();
    ~ColorSchemeModule() = default;

    // =========================================================================
    // Module Interface
    // =========================================================================

    [[nodiscard]] static const char* moduleName() { return "ColorScheme"; }
    [[nodiscard]] static const char* moduleDescription()
    {
        return "Color gradient schemes for visualization";
    }

    /// @brief Reset to default state
    void reset();

    // =========================================================================
    // Scheme Selection
    // =========================================================================

    /**
     * @brief Set predefined color scheme
     * @param scheme The scheme type to use
     */
    void setScheme(ColorSchemeType scheme);

    /**
     * @brief Get current scheme type
     */
    [[nodiscard]] ColorSchemeType scheme() const { return m_scheme; }

    /**
     * @brief Set custom gradient stops
     * @param stops Vector of gradient stops (sorted by position)
     *
     * Automatically sets scheme to Custom.
     */
    void setCustomGradient(std::vector<GradientStop> stops);

    /**
     * @brief Get current gradient stops
     */
    [[nodiscard]] const std::vector<GradientStop>& gradientStops() const
    {
        return m_stops;
    }

    // =========================================================================
    // Domain Configuration
    // =========================================================================

    /**
     * @brief Set gradient domain
     * @param domain What controls the color output
     */
    void setDomain(GradientDomain domain) { m_domain = domain; }

    /**
     * @brief Get current domain
     */
    [[nodiscard]] GradientDomain domain() const { return m_domain; }

    // =========================================================================
    // Time Animation
    // =========================================================================

    /**
     * @brief Set time-based animation speed
     * @param cyclesPerSecond How fast the gradient cycles (0 = no animation)
     */
    void setAnimationSpeed(float cyclesPerSecond)
    {
        m_animationSpeed = cyclesPerSecond;
    }

    /**
     * @brief Get animation speed
     */
    [[nodiscard]] float animationSpeed() const { return m_animationSpeed; }

    /**
     * @brief Set animation offset
     * @param offset Phase offset [0..1]
     */
    void setAnimationOffset(float offset) { m_animationOffset = offset; }

    /**
     * @brief Update animation state
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime);

    // =========================================================================
    // Color Sampling
    // =========================================================================

    /**
     * @brief Sample color at position
     * @param t Position in gradient [0..1]
     * @return Interpolated color
     */
    [[nodiscard]] Color sample(float t) const;

    /**
     * @brief Sample with time-based animation
     * @param t Base position [0..1]
     * @param timeOffset Additional time offset
     * @return Animated color
     */
    [[nodiscard]] Color sampleAnimated(float t) const;

    /**
     * @brief Sample color for amplitude value
     * @param amplitude Audio amplitude [0..1]
     * @return Color mapped from amplitude
     */
    [[nodiscard]] Color sampleAmplitude(float amplitude) const;

    /**
     * @brief Get solid color (for single-color modes)
     * @return Primary color of scheme
     */
    [[nodiscard]] Color primaryColor() const;

    /**
     * @brief Get secondary color (for two-color modes)
     * @return Secondary color of scheme
     */
    [[nodiscard]] Color secondaryColor() const;

    // =========================================================================
    // Beat Reactivity
    // =========================================================================

    /**
     * @brief Set beat intensity for color modulation
     * @param intensity Beat intensity [0..1]
     */
    void setBeatIntensity(float intensity) { m_beatIntensity = intensity; }

    /**
     * @brief Enable/disable beat-reactive brightness
     * @param enabled True to modulate brightness with beat
     */
    void setBeatBrightnessEnabled(bool enabled)
    {
        m_beatBrightnessEnabled = enabled;
    }

    /**
     * @brief Check if beat brightness is enabled
     */
    [[nodiscard]] bool beatBrightnessEnabled() const { return m_beatBrightnessEnabled; }

    /**
     * @brief Set brightness range for beat modulation
     * @param min Minimum brightness (0..1)
     * @param max Maximum brightness (0..1)
     */
    void setBeatBrightnessRange(float min, float max)
    {
        m_beatBrightnessMin = min;
        m_beatBrightnessMax = max;
    }

    // =========================================================================
    // Utility
    // =========================================================================

    /**
     * @brief Get scheme name as string
     * @param scheme Scheme type
     * @return Human-readable name
     */
    static const char* schemeName(ColorSchemeType scheme);

    /**
     * @brief Get all available scheme names
     * @return Vector of scheme names
     */
    static std::vector<const char*> availableSchemes();

private:
    /**
     * @brief Build gradient stops for predefined scheme
     */
    void buildScheme(ColorSchemeType scheme);

    /**
     * @brief Apply beat modulation to color
     */
    [[nodiscard]] Color applyBeatModulation(const Color& color) const;

    // Configuration
    ColorSchemeType m_scheme = ColorSchemeType::Rainbow;
    GradientDomain m_domain = GradientDomain::Position;
    std::vector<GradientStop> m_stops;

    // Animation
    float m_animationSpeed = 0.0f;
    float m_animationOffset = 0.0f;
    float m_animationPhase = 0.0f;

    // Beat reactivity
    float m_beatIntensity = 0.0f;
    bool m_beatBrightnessEnabled = false;
    float m_beatBrightnessMin = 0.5f;
    float m_beatBrightnessMax = 1.0f;
};

} // namespace lumi::modules
