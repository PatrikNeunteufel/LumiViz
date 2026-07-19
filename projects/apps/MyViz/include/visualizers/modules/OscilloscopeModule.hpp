/**
 ****************************************************************************************
 * @file   OscilloscopeModule.hpp
 * @brief  Classic oscilloscope display module with trigger, timebase, and math channels
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 2.0.0
 *
 * @details
 * Professional oscilloscope with:
 * - 4 Signal channels (CH1-CH4) with selectable source (L/R/Mono/Mid/Side)
 * - 2 Math channels (M1-M2) with operations (A+B, A-B, |A|, Rectify, etc.)
 * - AC/DC coupling
 * - Trigger system with edge detection
 * - Timebase control
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
// Oscilloscope Enums
// =============================================================================

/**
 * @brief Signal source selection
 */
enum class SignalSource
{
    Left = 0,       ///< Left channel
    Right,          ///< Right channel
    Mono,           ///< (L+R)/2
    Mid,            ///< (L+R)/2 (same as Mono, named for Mid/Side context)
    Side            ///< (L-R)/2
};

/**
 * @brief Signal display mode
 */
enum class SignalMode
{
    Waveform = 0,   ///< Raw waveform
    Envelope        ///< Envelope/Level (rectified + smoothed)
};

/**
 * @brief Math channel operation
 */
enum class MathOperation
{
    Add = 0,        ///< A + B
    Subtract,       ///< A - B
    Multiply,       ///< A * B
    Absolute,       ///< |A|
    Rectify,        ///< max(A, 0) - half-wave rectify
    Invert,         ///< -A
    Difference      ///< |A - B|
};

/**
 * @brief Trigger edge type
 */
enum class TriggerEdge
{
    Rising = 0,     ///< Trigger on rising edge
    Falling,        ///< Trigger on falling edge
    Both            ///< Trigger on any edge
};

/**
 * @brief Trigger mode
 */
enum class TriggerMode
{
    Auto = 0,       ///< Always display, even without trigger
    Normal,         ///< Only display when trigger occurs
    Single          ///< Single shot, wait for next trigger
};

/**
 * @brief Trigger indicator display style
 */
enum class TriggerIndicatorStyle
{
    Arrows = 0,     ///< Arrows on edges (Y-axis left, X-axis bottom)
    Crosshair       ///< Crosshair lines through trigger point
};

/**
 * @brief Channel coupling mode
 */
enum class CouplingMode
{
    DC = 0,         ///< Direct coupling (show DC offset)
    AC              ///< AC coupling (remove DC offset via high-pass filter)
};

/**
 * @brief Grid display style
 */
enum class GridStyle
{
    None = 0,       ///< No grid
    Lines,          ///< Full grid lines
    Dots,           ///< Dot grid (9 subdivisions per division)
    Cross           ///< Cross markers at divisions
};

// =============================================================================
// Channel Configurations
// =============================================================================

/**
 * @brief Base settings for any oscilloscope channel
 */
struct ChannelConfigBase
{
    // Vertical settings
    float voltsPerDiv = 0.5f;       ///< Vertical scale (amplitude per division)
    float offset = 0.0f;            ///< Vertical offset [-4..4] divisions
    
    // Coupling
    CouplingMode coupling = CouplingMode::DC;
    
    // Display
    bool visible = false;           ///< Channel visibility
    float lineWidth = 2.0f;         ///< Line width in pixels

    // NOTE: The former phosphor fields were dead (never pushed/rendered, no
    // param keys — E7). A real phosphor effect comes via PostFxModule (5.6).
};

/**
 * @brief Settings for a signal channel (CH1-CH4)
 */
struct SignalChannelConfig : ChannelConfigBase
{
    SignalSource source = SignalSource::Left;
    SignalMode mode = SignalMode::Waveform;
    
    // Envelope mode settings
    float envelopeAttack = 5.0f;    ///< Attack time in ms
    float envelopeRelease = 50.0f;  ///< Release time in ms
};

/**
 * @brief Settings for a math channel (M1-M2)
 */
struct MathChannelConfig : ChannelConfigBase
{
    MathOperation operation = MathOperation::Add;
    int sourceA = 0;                ///< First source channel index (0-3 for CH1-CH4)
    int sourceB = 1;                ///< Second source channel index (for binary ops)
};

// =============================================================================
// AC Coupling Filter State
// =============================================================================

/**
 * @brief State for AC coupling high-pass filter (per channel)
 * 
 * Implements a classic first-order IIR high-pass filter:
 * y[n] = alpha * (y[n-1] + x[n] - x[n-1])
 * 
 * This removes DC offset and shows only signal changes.
 */
struct ACCouplingState
{
    float prevInput = 0.0f;          ///< Previous input sample x[n-1]
    float prevOutput = 0.0f;         ///< Smoothed DC offset
    float alpha = 0.9f;              ///< DC offset smoothing (lower = faster adaptation)
    
    void reset() 
    { 
        prevInput = 0.0f; 
        prevOutput = 0.0f; 
    }
};

// =============================================================================
// OscilloscopeModule
// =============================================================================

/**
 * @class OscilloscopeModule
 * @brief Professional oscilloscope with 4 signal + 2 math channels
 */
class OscilloscopeModule
{
public:
    // Channel counts
    static constexpr int SIGNAL_CHANNELS = 4;   ///< CH1-CH4
    static constexpr int MATH_CHANNELS = 2;     ///< M1-M2
    static constexpr int TOTAL_CHANNELS = SIGNAL_CHANNELS + MATH_CHANNELS;
    
    // Channel indices
    static constexpr int CH1 = 0;
    static constexpr int CH2 = 1;
    static constexpr int CH3 = 2;
    static constexpr int CH4 = 3;
    static constexpr int M1 = 4;
    static constexpr int M2 = 5;
    
    // Grid dimensions
    static constexpr int DIVISIONS_X = 10;
    static constexpr int DIVISIONS_Y = 8;

    // =========================================================================
    // Construction
    // =========================================================================

    OscilloscopeModule();
    ~OscilloscopeModule() = default;

    // =========================================================================
    // Module Interface
    // =========================================================================

    [[nodiscard]] static const char* moduleName() { return "Oscilloscope"; }
    [[nodiscard]] static const char* moduleDescription()
    {
        return "Professional oscilloscope with 4 signal + 2 math channels";
    }

    void reset();

    // =========================================================================
    // IModule-style Parameter Interface
    // =========================================================================

    [[nodiscard]] std::vector<ModuleParamDesc> paramDescs() const;
    [[nodiscard]] bool getParam(const std::string& id, ParamValue& out) const;
    bool setParam(const std::string& id, const ParamValue& value);

    // =========================================================================
    // Timebase Settings
    // =========================================================================

    void setTimePerDiv(float msPerDiv);
    [[nodiscard]] float timePerDiv() const { return m_timePerDiv; }

    void setSampleCount(int count);
    [[nodiscard]] int sampleCount() const { return m_sampleCount; }

    // =========================================================================
    // Trigger Settings
    // =========================================================================

    void setTriggerEnabled(bool enabled) { m_triggerEnabled = enabled; }
    [[nodiscard]] bool triggerEnabled() const { return m_triggerEnabled; }

    void setTriggerLevel(float level);
    [[nodiscard]] float triggerLevel() const { return m_triggerLevel; }

    void setTriggerTolerance(float tol) { m_triggerTolerance = std::clamp(tol, 0.0f, 2.0f); }
    [[nodiscard]] float triggerTolerance() const { return m_triggerTolerance; }

    void setTriggerPosition(float pos);
    [[nodiscard]] float triggerPosition() const { return m_triggerPosition; }

    void setTriggerEdge(TriggerEdge edge) { m_triggerEdge = edge; }
    [[nodiscard]] TriggerEdge triggerEdge() const { return m_triggerEdge; }

    void setTriggerMode(TriggerMode mode) { m_triggerMode = mode; }
    [[nodiscard]] TriggerMode triggerMode() const { return m_triggerMode; }

    void setTriggerIndicatorStyle(TriggerIndicatorStyle style) { m_triggerIndicatorStyle = style; }
    [[nodiscard]] TriggerIndicatorStyle triggerIndicatorStyle() const { return m_triggerIndicatorStyle; }

    void setTriggerChannel(int channel);
    [[nodiscard]] int triggerChannel() const { return m_triggerChannel; }

    void setTriggerHoldoff(float ms);
    [[nodiscard]] float triggerHoldoff() const { return m_triggerHoldoff; }

    void setTriggerFadeTime(float seconds);
    [[nodiscard]] float triggerFadeTime() const { return m_triggerFadeTime; }

    // =========================================================================
    // Signal Channel Access (CH1-CH4)
    // =========================================================================

    [[nodiscard]] SignalChannelConfig& signalChannel(int index);
    [[nodiscard]] const SignalChannelConfig& signalChannel(int index) const;

    // =========================================================================
    // Math Channel Access (M1-M2)
    // =========================================================================

    [[nodiscard]] MathChannelConfig& mathChannel(int index);
    [[nodiscard]] const MathChannelConfig& mathChannel(int index) const;

    // =========================================================================
    // Generic Channel Access (for rendering)
    // =========================================================================

    [[nodiscard]] const ChannelConfigBase& channelBase(int index) const;
    [[nodiscard]] bool isSignalChannel(int index) const { return index < SIGNAL_CHANNELS; }
    [[nodiscard]] bool isMathChannel(int index) const { return index >= SIGNAL_CHANNELS; }

    // =========================================================================
    // Grid Settings
    // =========================================================================

    void setGridStyle(GridStyle style) { m_gridStyle = style; }
    [[nodiscard]] GridStyle gridStyle() const { return m_gridStyle; }

    void setGridColor(float r, float g, float b, float a = 1.0f);
    [[nodiscard]] const Color4f& gridColor() const { return m_gridColor; }

    void setGridBrightness(float brightness);
    [[nodiscard]] float gridBrightness() const { return m_gridBrightness; }

    void setGridLineWidth(float width) { m_gridLineWidth = std::clamp(width, 0.5f, 3.0f); }
    [[nodiscard]] float gridLineWidth() const { return m_gridLineWidth; }

    void setGridDotSize(float size) { m_gridDotSize = std::clamp(size, 1.0f, 5.0f); }
    [[nodiscard]] float gridDotSize() const { return m_gridDotSize; }

    void setGridCrossSize(float size) { m_gridCrossSize = std::clamp(size, 2.0f, 10.0f); }
    [[nodiscard]] float gridCrossSize() const { return m_gridCrossSize; }

    // =========================================================================
    // Display Settings
    // =========================================================================

    void setBackgroundColor(float r, float g, float b);
    [[nodiscard]] float backgroundR() const { return m_bgColorR; }
    [[nodiscard]] float backgroundG() const { return m_bgColorG; }
    [[nodiscard]] float backgroundB() const { return m_bgColorB; }

    void setInterpolation(bool enabled) { m_interpolation = enabled; }
    [[nodiscard]] bool interpolation() const { return m_interpolation; }

    // =========================================================================
    // Color Gradient Access (per channel, 0-5)
    // =========================================================================

    [[nodiscard]] ColorGradientModule& colorGradient(int channel);
    [[nodiscard]] const ColorGradientModule& colorGradient(int channel) const;

    // =========================================================================
    // Signal Processing
    // =========================================================================

    /**
     * @brief Process raw stereo waveform into per-channel display data
     * @param left Left channel samples
     * @param right Right channel samples  
     * @param count Number of samples
     * @param output Output array [TOTAL_CHANNELS][sampleCount]
     */
    void processSignals(const float* left, const float* right, int count,
                        std::array<std::vector<float>, TOTAL_CHANNELS>& output);

    /**
     * @brief Apply AC coupling filter to samples
     * @param samples Input/output samples
     * @param count Number of samples
     * @param state Filter state (updated in-place)
     */
    static void applyACCoupling(float* samples, int count, ACCouplingState& state);

    /**
     * @brief Compute envelope from waveform
     * @param samples Input samples
     * @param count Number of samples
     * @param output Output envelope
     * @param attack Attack time in samples
     * @param release Release time in samples
     */
    static void computeEnvelope(const float* samples, int count, float* output,
                                float attack, float release);

    // =========================================================================
    // Trigger Detection
    // =========================================================================

    [[nodiscard]] int findTriggerPoint(const float* samples, int count) const;

    // =========================================================================
    // AC Coupling State Access
    // =========================================================================

    [[nodiscard]] ACCouplingState& acCouplingState(int channel);

    // =========================================================================
    // Utility
    // =========================================================================

    static const char* signalSourceName(SignalSource source);
    static const char* signalModeName(SignalMode mode);
    static const char* mathOperationName(MathOperation op);
    static const char* triggerEdgeName(TriggerEdge edge);
    static const char* triggerModeName(TriggerMode mode);
    static const char* couplingModeName(CouplingMode mode);
    static const char* gridStyleName(GridStyle style);

private:
    // =========================================================================
    // Timebase
    // =========================================================================

    float m_timePerDiv = 10.0f;     ///< ms per division
    int m_sampleCount = 512;        ///< Display samples (rendering resolution)

    // =========================================================================
    // Trigger
    // =========================================================================

    bool m_triggerEnabled = true;
    float m_triggerLevel = 0.0f;
    float m_triggerTolerance = 0.1f;  ///< Tolerance in divisions (0 = exact)
    float m_triggerPosition = 0.5f;   ///< Horizontal position (0=left, 1=right)
    TriggerEdge m_triggerEdge = TriggerEdge::Rising;
    TriggerMode m_triggerMode = TriggerMode::Auto;
    TriggerIndicatorStyle m_triggerIndicatorStyle = TriggerIndicatorStyle::Crosshair;
    int m_triggerChannel = CH1;
    float m_triggerHoldoff = 0.0f;
    float m_triggerFadeTime = 2.0f;   ///< Fade duration in seconds (for Normal/Single modes)

    // =========================================================================
    // Signal Channels (CH1-CH4)
    // =========================================================================

    std::array<SignalChannelConfig, SIGNAL_CHANNELS> m_signalChannels;

    // =========================================================================
    // Math Channels (M1-M2)
    // =========================================================================

    std::array<MathChannelConfig, MATH_CHANNELS> m_mathChannels;

    // =========================================================================
    // AC Coupling States (per signal channel)
    // =========================================================================

    std::array<ACCouplingState, SIGNAL_CHANNELS> m_acCouplingStates;

    // =========================================================================
    // Grid
    // =========================================================================

    GridStyle m_gridStyle = GridStyle::Lines;
    Color4f m_gridColor = {0.3f, 0.3f, 0.3f, 1.0f};
    float m_gridBrightness = 1.0f;
    float m_gridLineWidth = 1.0f;   ///< Line width for Lines mode (pixels)
    float m_gridDotSize = 2.0f;     ///< Dot size for Dots mode (pixels)
    float m_gridCrossSize = 5.0f;   ///< Cross size for Cross mode (pixels)

    // =========================================================================
    // Display
    // =========================================================================

    float m_bgColorR = 0.02f;
    float m_bgColorG = 0.05f;
    float m_bgColorB = 0.02f;
    bool m_interpolation = true;

    // =========================================================================
    // Color Gradients (per channel, 0-5)
    // =========================================================================

    std::array<ColorGradientModule, TOTAL_CHANNELS> m_colorGradients;
};

} // namespace lumi::modules
