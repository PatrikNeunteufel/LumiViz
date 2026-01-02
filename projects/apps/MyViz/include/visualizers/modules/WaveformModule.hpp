/**
 ****************************************************************************************
 * @file   WaveformModule.hpp
 * @brief  Advanced waveform display module with per-channel settings
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 3.0.0
 ****************************************************************************************
 */

#pragma once

#include "IModule.hpp"
#include "ColorGradientModule.hpp"

#include <string>
#include <vector>
#include <array>

namespace lumi::modules {

// =============================================================================
// Waveform Enums
// =============================================================================

/**
 * @brief Line drawing style
 */
enum class WaveformLineStyle
{
    Line = 0,       ///< Continuous connected line
    Dots,           ///< Individual dots at sample points
    Dashed          ///< Dashed line segments
};

/**
 * @brief Channel display mode
 */
enum class WaveformChannelMode
{
    Mono = 0,       ///< Single mono channel only
    Stereo,         ///< Left and right separate (no mono)
    Both            ///< Mono, Left, and Right (all three)
};

// =============================================================================
// Per-Channel Settings Structure
// =============================================================================

/**
 * @brief Complete settings for a single audio channel
 */
struct WaveformChannelConfig
{
    // Layout
    float lineOffset = 0.0f;        ///< Vertical offset [-1..1]
    float amplitude = 0.8f;         ///< Amplitude scale [0.1..2.0]
    
    // Line style
    float lineWidth = 2.0f;         ///< Line width in pixels
    
    // Fill
    bool fillEnabled = false;       ///< Fill to zero line
    float fillOpacity = 0.3f;       ///< Fill opacity [0..1]
    float fillBrightness = -0.3f;   ///< Brightness relative to line
    
    // Visibility
    bool visible = true;            ///< Channel visibility
};

// =============================================================================
// WaveformModule
// =============================================================================

/**
 * @class WaveformModule
 * @brief Advanced waveform display with per-channel configuration
 *
 * Each channel (Mono, Left, Right) has independent settings for:
 * - Offset and Amplitude
 * - Line Width
 * - Fill settings
 * - Color gradient
 */
class WaveformModule
{
public:
    // Channel indices
    static constexpr int CHANNEL_MONO = 0;
    static constexpr int CHANNEL_LEFT = 1;
    static constexpr int CHANNEL_RIGHT = 2;

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
        return "Advanced waveform display with per-channel settings";
    }

    void reset();

    // =========================================================================
    // IModule-style Parameter Interface
    // =========================================================================

    [[nodiscard]] std::vector<ModuleParamDesc> paramDescs() const;
    [[nodiscard]] bool getParam(const std::string& id, ParamValue& out) const;
    bool setParam(const std::string& id, const ParamValue& value);

    // =========================================================================
    // Channel Mode
    // =========================================================================

    void setChannelMode(WaveformChannelMode mode);
    [[nodiscard]] WaveformChannelMode channelMode() const { return m_channelMode; }

    // =========================================================================
    // Per-Channel Settings Access
    // =========================================================================

    /// @brief Get channel config (0=Mono, 1=Left, 2=Right)
    [[nodiscard]] WaveformChannelConfig& channelConfig(int channel);
    [[nodiscard]] const WaveformChannelConfig& channelConfig(int channel) const;

    // =========================================================================
    // Global Settings
    // =========================================================================

    void setLineStyle(WaveformLineStyle style) { m_lineStyle = style; }
    [[nodiscard]] WaveformLineStyle lineStyle() const { return m_lineStyle; }

    void setSampleCount(int count) { m_sampleCount = std::clamp(count, 64, 2048); }
    [[nodiscard]] int sampleCount() const { return m_sampleCount; }

    void setSmoothing(float smoothing) { m_smoothing = std::clamp(smoothing, 0.0f, 0.95f); }
    [[nodiscard]] float smoothing() const { return m_smoothing; }

    void setDisplayWidth(float width) { m_displayWidth = std::clamp(width, 0.1f, 1.0f); }
    [[nodiscard]] float displayWidth() const { return m_displayWidth; }

    // Dash settings (global)
    void setDashLength(float length) { m_dashLength = std::clamp(length, 2.0f, 50.0f); }
    [[nodiscard]] float dashLength() const { return m_dashLength; }

    void setDashGap(float gap) { m_dashGap = std::clamp(gap, 1.0f, 50.0f); }
    [[nodiscard]] float dashGap() const { return m_dashGap; }

    // =========================================================================
    // Effects (Global)
    // =========================================================================

    void setMirrorEnabled(bool enabled) { m_mirrorEnabled = enabled; }
    [[nodiscard]] bool mirrorEnabled() const { return m_mirrorEnabled; }

    void setHoldEnabled(bool enabled) { m_holdEnabled = enabled; }
    [[nodiscard]] bool holdEnabled() const { return m_holdEnabled; }

    void setFadeTime(float seconds) { m_fadeTime = std::clamp(seconds, 0.1f, 5.0f); }
    [[nodiscard]] float fadeTime() const { return m_fadeTime; }

    void setMaxHoldFrames(int frames) { m_maxHoldFrames = std::clamp(frames, 1, 120); }
    [[nodiscard]] int maxHoldFrames() const { return m_maxHoldFrames; }

    // =========================================================================
    // Color Gradient Access (per channel)
    // =========================================================================

    /// @brief Get color gradient for channel (0=Mono, 1=Left, 2=Right)
    [[nodiscard]] ColorGradientModule& colorGradient(int channel);
    [[nodiscard]] const ColorGradientModule& colorGradient(int channel) const;
    
    /// @brief Convenience: Mono gradient
    [[nodiscard]] ColorGradientModule& colorGradient() { return m_colorGradients[CHANNEL_MONO]; }
    [[nodiscard]] const ColorGradientModule& colorGradient() const { return m_colorGradients[CHANNEL_MONO]; }

    // =========================================================================
    // Utility
    // =========================================================================

    static const char* lineStyleName(WaveformLineStyle style);
    static const char* channelModeName(WaveformChannelMode mode);

private:
    // =========================================================================
    // Channel Mode
    // =========================================================================

    WaveformChannelMode m_channelMode = WaveformChannelMode::Mono;

    // =========================================================================
    // Per-Channel Configs: [0]=Mono, [1]=Left, [2]=Right
    // =========================================================================

    std::array<WaveformChannelConfig, 3> m_channelConfigs;

    // =========================================================================
    // Global Line Style
    // =========================================================================

    WaveformLineStyle m_lineStyle = WaveformLineStyle::Line;
    float m_dashLength = 10.0f;
    float m_dashGap = 5.0f;

    // =========================================================================
    // Global Layout
    // =========================================================================

    int m_sampleCount = 512;
    float m_smoothing = 0.3f;
    float m_displayWidth = 1.0f;

    // =========================================================================
    // Effects (Global)
    // =========================================================================

    bool m_mirrorEnabled = false;
    bool m_holdEnabled = false;
    float m_fadeTime = 1.0f;
    int m_maxHoldFrames = 60;

    // =========================================================================
    // Color (per channel): [0]=Mono, [1]=Left, [2]=Right
    // =========================================================================

    std::array<ColorGradientModule, 3> m_colorGradients;
};

} // namespace lumi::modules
