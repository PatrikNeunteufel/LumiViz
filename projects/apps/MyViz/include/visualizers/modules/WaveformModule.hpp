/**
 ****************************************************************************************
 * @file   WaveformModule.hpp
 * @brief  Waveform display module for audio visualizers
 *
 * Provides waveform rendering parameters and styles.
 * Embeds ColorGradientModule for consistent color handling.
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "IModule.hpp"
#include "ColorGradientModule.hpp"

#include <string>
#include <vector>

namespace lumi::modules {

// =============================================================================
// Waveform Style Types
// =============================================================================

/**
 * @brief Available waveform display styles
 */
enum class WaveformStyle
{
    Line = 0,       ///< Connected line through sample points
    Bars,           ///< Vertical bars for each sample
    Mirror,         ///< Mirrored waveform (top and bottom)
    Filled,         ///< Filled area under waveform
    Dots            ///< Individual dots at sample points
};

/**
 * @brief Waveform orientation
 */
enum class WaveformOrientation
{
    Horizontal = 0, ///< Left to right (standard oscilloscope)
    Vertical,       ///< Bottom to top
    Circular        ///< Arranged in a circle
};

// =============================================================================
// WaveformModule
// =============================================================================

/**
 * @class WaveformModule
 * @brief Module for waveform display configuration
 *
 * Manages waveform rendering parameters including style, size, and colors.
 * Embeds ColorGradientModule for consistent gradient handling.
 *
 * @par Example Usage
 * @code
 * WaveformModule waveform;
 * waveform.setStyle(WaveformStyle::Mirror);
 * waveform.setLineWidth(3.0f);
 * waveform.colorGradient().loadPreset("Neon");
 *
 * // Get params for ConfigPanel:
 * auto params = waveform.paramDescs();
 * @endcode
 */
class WaveformModule
{
public:
    // =========================================================================
    // Construction
    // =========================================================================

    WaveformModule();
    ~WaveformModule() = default;

    // =========================================================================
    // Module Interface
    // =========================================================================

    [[nodiscard]] static const char* moduleName() { return "Waveform"; }
    [[nodiscard]] static const char* moduleDescription()
    {
        return "Waveform display configuration";
    }

    /// @brief Reset to default state
    void reset();

    // =========================================================================
    // IModule-style Parameter Interface
    // =========================================================================

    /**
     * @brief Get all parameter descriptors
     * @return Vector of parameter descriptors (including color subgroup)
     */
    [[nodiscard]] std::vector<ModuleParamDesc> paramDescs() const;

    /**
     * @brief Get parameter value by ID
     * @param id Parameter ID (e.g., "style", "color.mode")
     * @param out Output value
     * @return true if parameter found
     */
    [[nodiscard]] bool getParam(const std::string& id, ParamValue& out) const;

    /**
     * @brief Set parameter value by ID
     * @param id Parameter ID
     * @param value New value
     * @return true if parameter set successfully
     */
    bool setParam(const std::string& id, const ParamValue& value);

    // =========================================================================
    // Style Configuration
    // =========================================================================

    /**
     * @brief Set waveform display style
     * @param style The display style
     */
    void setStyle(WaveformStyle style) { m_style = style; }

    /**
     * @brief Get current style
     */
    [[nodiscard]] WaveformStyle style() const { return m_style; }

    /**
     * @brief Set waveform orientation
     * @param orientation Display orientation
     */
    void setOrientation(WaveformOrientation orientation) { m_orientation = orientation; }

    /**
     * @brief Get current orientation
     */
    [[nodiscard]] WaveformOrientation orientation() const { return m_orientation; }

    // =========================================================================
    // Display Parameters
    // =========================================================================

    /**
     * @brief Set line width in pixels
     * @param width Width (1-10)
     */
    void setLineWidth(float width) { m_lineWidth = std::clamp(width, 1.0f, 10.0f); }

    /**
     * @brief Get line width
     */
    [[nodiscard]] float lineWidth() const { return m_lineWidth; }

    /**
     * @brief Set amplitude scale
     * @param amplitude Vertical scale (0.1-2.0)
     */
    void setAmplitude(float amplitude) { m_amplitude = std::clamp(amplitude, 0.1f, 2.0f); }

    /**
     * @brief Get amplitude scale
     */
    [[nodiscard]] float amplitude() const { return m_amplitude; }

    /**
     * @brief Set sample count for display
     * @param count Number of samples (64-1024)
     */
    void setSampleCount(int count) { m_sampleCount = std::clamp(count, 64, 1024); }

    /**
     * @brief Get sample count
     */
    [[nodiscard]] int sampleCount() const { return m_sampleCount; }

    /**
     * @brief Set temporal smoothing factor
     * @param smoothing Smoothing (0-0.95)
     */
    void setSmoothing(float smoothing) { m_smoothing = std::clamp(smoothing, 0.0f, 0.95f); }

    /**
     * @brief Get smoothing factor
     */
    [[nodiscard]] float smoothing() const { return m_smoothing; }

    // =========================================================================
    // Glow Effect
    // =========================================================================

    /**
     * @brief Enable/disable glow effect
     * @param enabled True to enable glow
     */
    void setGlowEnabled(bool enabled) { m_glowEnabled = enabled; }

    /**
     * @brief Check if glow is enabled
     */
    [[nodiscard]] bool glowEnabled() const { return m_glowEnabled; }

    /**
     * @brief Set glow intensity
     * @param intensity Glow intensity (0-1)
     */
    void setGlowIntensity(float intensity) { m_glowIntensity = std::clamp(intensity, 0.0f, 1.0f); }

    /**
     * @brief Get glow intensity
     */
    [[nodiscard]] float glowIntensity() const { return m_glowIntensity; }

    // =========================================================================
    // Mirror Mode Parameters
    // =========================================================================

    /**
     * @brief Set mirror gap (space between top/bottom halves)
     * @param gap Gap as fraction of height (0-0.5)
     */
    void setMirrorGap(float gap) { m_mirrorGap = std::clamp(gap, 0.0f, 0.5f); }

    /**
     * @brief Get mirror gap
     */
    [[nodiscard]] float mirrorGap() const { return m_mirrorGap; }

    // =========================================================================
    // Color Gradient Access
    // =========================================================================

    /**
     * @brief Get access to the color gradient module
     * @return Reference to the embedded color gradient
     */
    [[nodiscard]] ColorGradientModule& colorGradient() { return m_colorGradient; }

    /**
     * @brief Get const access to the color gradient module
     */
    [[nodiscard]] const ColorGradientModule& colorGradient() const { return m_colorGradient; }

    // =========================================================================
    // Utility
    // =========================================================================

    /**
     * @brief Get style name as string
     * @param style Style type
     * @return Human-readable name
     */
    static const char* styleName(WaveformStyle style);

    /**
     * @brief Get all available style names
     */
    static std::vector<const char*> availableStyles();

private:
    // =========================================================================
    // Embedded Modules
    // =========================================================================

    ColorGradientModule m_colorGradient;

    // =========================================================================
    // Style Configuration
    // =========================================================================

    WaveformStyle m_style = WaveformStyle::Line;
    WaveformOrientation m_orientation = WaveformOrientation::Horizontal;

    // =========================================================================
    // Display Parameters
    // =========================================================================

    float m_lineWidth = 2.0f;
    float m_amplitude = 0.8f;
    int m_sampleCount = 256;
    float m_smoothing = 0.3f;

    // =========================================================================
    // Glow Effect
    // =========================================================================

    bool m_glowEnabled = true;
    float m_glowIntensity = 0.5f;

    // =========================================================================
    // Mirror Mode
    // =========================================================================

    float m_mirrorGap = 0.05f;
};

} // namespace lumi::modules
