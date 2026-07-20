/**
 ****************************************************************************************
 * @file   SuperscopeModule.hpp
 * @brief  Programmable point/line visualizer inspired by Winamp AVS Superscope
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * Superscope is a programmable visualization with:
 * - 4-phase expression system (Init, Beat, Frame, Point)
 * - Mathematical expressions for x, y coordinates
 * - Per-point color control (red, green, blue)
 * - Multiple render modes (Dots, Lines, Thick Lines)
 * - Audio-reactive variables (v, b)
 * - Builtin preset library
 ****************************************************************************************
 */

#pragma once

#include "IModule.hpp"
#include "ColorGradientModule.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cmath>

namespace lumi::scripting { class ScriptSlotHost; }

namespace lumi::modules {

// =============================================================================
// Superscope Enums
// =============================================================================

/**
 * @brief Render mode for superscope output
 */
enum class SuperscopeRenderMode
{
    Dots = 0,       ///< Individual points
    Lines,          ///< Connected line strip
    ThickLines      ///< Thick lines with optional glow
};

/**
 * @brief Audio source for the v variable
 */
enum class SuperscopeAudioSource
{
    Waveform = 0,   ///< Oscilloscope data (-1 to 1)
    Spectrum        ///< Spectrum data (0 to 1)
};

/**
 * @brief Audio channel selection
 */
enum class SuperscopeAudioChannel
{
    Left = 0,       ///< Left channel only
    Right,          ///< Right channel only
    Mono,           ///< (L+R)/2
    Mid,            ///< Same as Mono
    Side            ///< (L-R)/2
};

/**
 * @brief Blending mode for rendering
 */
enum class SuperscopeBlendMode
{
    Replace = 0,    ///< Overwrite existing pixels
    Additive,       ///< Add to existing pixels
    Alpha           ///< Alpha blend
};

/**
 * @brief Builtin preset identifiers
 */
enum class SuperscopePreset
{
    Custom = 0,             ///< User-defined expressions
    HorizontalScope,        ///< Classic horizontal waveform
    VerticalScope,          ///< Vertical waveform
    Circle,                 ///< Audio-reactive circle
    Spiral,                 ///< Rotating spiral
    Lissajous,              ///< Lissajous figures
    Flower,                 ///< Rose curve / flower shape
    Star,                   ///< Classic 5-pointed star
    Starburst,              ///< Radiating burst pattern (was "Star")
    Heart,                  ///< Heart shape
    DNA,                    ///< Double helix
    SpectrumBars,           ///< Vertical spectrum bars
    CircularSpectrum,       ///< Radial spectrum display
    Butterfly,              ///< Butterfly curve
    Hypocycloid             ///< Hypocycloid pattern
};

// =============================================================================
// Point Data Structure
// =============================================================================

/**
 * @brief Output data for a single superscope point
 */
struct SuperscopePoint
{
    float x = 0.0f;         ///< X coordinate (-1 to 1)
    float y = 0.0f;         ///< Y coordinate (-1 to 1)
    float r = 1.0f;         ///< Red (0 to 1)
    float g = 1.0f;         ///< Green (0 to 1)
    float b = 1.0f;         ///< Blue (0 to 1)
    float a = 1.0f;         ///< Alpha (0 to 1)
    bool skip = false;      ///< Skip this point (break line)
};

// =============================================================================
// SuperscopeModule
// =============================================================================

/**
 * @class SuperscopeModule
 * @brief Programmable point/line visualizer with expression support
 *
 * The Superscope module executes mathematical expressions in four phases:
 * 1. Init: Once at startup or when code changes
 * 2. Beat: When a beat is detected
 * 3. Frame: Once per frame
 * 4. Point: For each of the n points
 *
 * Variables available:
 * - Input: n, i, v, b, w, h
 * - Output: x, y, red, green, blue, skip
 * - User-defined: Any other variable names
 */
class SuperscopeModule
{
public:
    SuperscopeModule();
    ~SuperscopeModule();  // out-of-line: unique_ptr<ScriptSlotHost> member

    // =========================================================================
    // Expression Code
    // =========================================================================

    void setInitCode(const std::string& code);
    [[nodiscard]] const std::string& initCode() const { return m_initCode; }

    void setBeatCode(const std::string& code);
    [[nodiscard]] const std::string& beatCode() const { return m_beatCode; }

    void setFrameCode(const std::string& code);
    [[nodiscard]] const std::string& frameCode() const { return m_frameCode; }

    void setPointCode(const std::string& code);
    [[nodiscard]] const std::string& pointCode() const { return m_pointCode; }

    // =========================================================================
    // Lua Script Mode (Import-Phase Roadmap 1 — Keimzelle)
    // =========================================================================

    /**
     * @brief Enable/disable script execution of the four code slots
     *
     * When enabled, Init/Beat/Frame/Point are transpiled from EEL (AVS dialect,
     * EelTranspiler) and run as sandboxed Lua chunks (LuaScriptEngine).
     * Contract: host inputs i, v, b, n, w, h, time, dt — `t` is NEVER written
     * by the host, scripts own and accumulate it (AVS style: "t=t+0.02").
     * Outputs: x, y, skip — and red/green/blue [0..1] if the point code
     * mentions them (otherwise the color gradient applies as before).
     * Without a compiled point script the hardcoded preset math is used.
     */
    void setLuaMode(bool enabled);
    [[nodiscard]] bool luaMode() const { return m_luaMode; }

    /// @brief Last compile/runtime error of the script slots (empty = none)
    [[nodiscard]] const std::string& lastScriptError() const { return m_lastScriptError; }

    // =========================================================================
    // Preset System
    // =========================================================================

    void setPreset(SuperscopePreset preset);
    [[nodiscard]] SuperscopePreset preset() const { return m_preset; }

    void loadPresetCode(SuperscopePreset preset);

    // =========================================================================
    // Base color (AVS r_sscope): red/green/blue are pre-seeded with this before
    // the point script runs, so the script reads/modifies/overrides it. Base =
    // gradient(i) x cycled color table, combined per `colorBlend`. Empty table +
    // colorBlend 0 = gradient only (historical behaviour).
    // =========================================================================

    void setColorTable(const std::vector<uint32_t>& colors) { m_colorTable = colors; }
    void setColorBlend(int mode) { m_colorBlend = mode; }  ///< 0 grad 1 table 2 add 3 mul 4 avg
    void setColorCycleFrames(int frames) { m_colorCycleFrames = frames < 1 ? 1 : frames; }

    // =========================================================================
    // Render Settings
    // =========================================================================

    void setPointCount(int count);
    [[nodiscard]] int pointCount() const { return m_pointCount; }

    void setRenderMode(SuperscopeRenderMode mode) { m_renderMode = mode; }
    [[nodiscard]] SuperscopeRenderMode renderMode() const { return m_renderMode; }

    void setLineWidth(float width) { m_lineWidth = std::clamp(width, 1.0f, 20.0f); }
    [[nodiscard]] float lineWidth() const { return m_lineWidth; }

    void setDotSize(float size) { m_dotSize = std::clamp(size, 1.0f, 50.0f); }
    [[nodiscard]] float dotSize() const { return m_dotSize; }

    void setBlendMode(SuperscopeBlendMode mode) { m_blendMode = mode; }
    [[nodiscard]] SuperscopeBlendMode blendMode() const { return m_blendMode; }

    // =========================================================================
    // Audio Settings
    // =========================================================================

    void setAudioSource(SuperscopeAudioSource source) { m_audioSource = source; }
    [[nodiscard]] SuperscopeAudioSource audioSource() const { return m_audioSource; }

    void setAudioChannel(SuperscopeAudioChannel channel) { m_audioChannel = channel; }
    [[nodiscard]] SuperscopeAudioChannel audioChannel() const { return m_audioChannel; }

    // =========================================================================
    // Color Gradient Access
    // =========================================================================

    [[nodiscard]] ColorGradientModule& colorGradient() { return m_colorGradient; }
    [[nodiscard]] const ColorGradientModule& colorGradient() const { return m_colorGradient; }

    // =========================================================================
    // Glow Settings
    // =========================================================================

    void setGlowEnabled(bool enabled) { m_glowEnabled = enabled; }
    [[nodiscard]] bool glowEnabled() const { return m_glowEnabled; }

    void setGlowIntensity(float intensity) { m_glowIntensity = std::clamp(intensity, 0.0f, 2.0f); }
    [[nodiscard]] float glowIntensity() const { return m_glowIntensity; }

    void setGlowSize(float size) { m_glowSize = std::clamp(size, 1.0f, 10.0f); }
    [[nodiscard]] float glowSize() const { return m_glowSize; }

    // =========================================================================
    // Hold/Fade Settings
    // =========================================================================

    void setHoldEnabled(bool enabled) { m_holdEnabled = enabled; }
    [[nodiscard]] bool holdEnabled() const { return m_holdEnabled; }

    void setFadeTime(float seconds) { m_fadeTime = std::clamp(seconds, 0.1f, 10.0f); }
    [[nodiscard]] float fadeTime() const { return m_fadeTime; }

    void setMaxHoldFrames(int frames) { m_maxHoldFrames = std::clamp(frames, 1, 60); }
    [[nodiscard]] int maxHoldFrames() const { return m_maxHoldFrames; }

    // =========================================================================
    // Display Settings
    // =========================================================================

    void setAspectCorrection(bool enabled) { m_aspectCorrection = enabled; }
    [[nodiscard]] bool aspectCorrection() const { return m_aspectCorrection; }

    void setStretchX(float stretch) { m_stretchX = std::clamp(stretch, 0.1f, 4.0f); }
    [[nodiscard]] float stretchX() const { return m_stretchX; }

    void setStretchY(float stretch) { m_stretchY = std::clamp(stretch, 0.1f, 4.0f); }
    [[nodiscard]] float stretchY() const { return m_stretchY; }

    // =========================================================================
    // Execution
    // =========================================================================

    /**
     * @brief Execute Init code (call once or when code changes)
     */
    void executeInit();

    /**
     * @brief Execute Beat code (call when beat detected)
     */
    void executeBeat();

    /**
     * @brief Execute Frame code and generate all points
     * @param waveformL Left channel waveform data
     * @param waveformR Right channel waveform data
     * @param spectrumL Left channel spectrum data
     * @param spectrumR Right channel spectrum data
     * @param sampleCount Number of audio samples
     * @param width Viewport width
     * @param height Viewport height
     * @param isBeat True if current frame is on beat
     * @param deltaTime Time since last frame
     * @return Vector of generated points
     */
    std::vector<SuperscopePoint> execute(
        const float* waveformL,
        const float* waveformR,
        const float* spectrumL,
        const float* spectrumR,
        int sampleCount,
        int width,
        int height,
        bool isBeat,
        float deltaTime
    );

    // =========================================================================
    // Variable Access (for debugging/UI)
    // =========================================================================

    [[nodiscard]] double getVariable(const std::string& name) const;
    void setVariable(const std::string& name, double value);

    // =========================================================================
    // IModule-style Parameter Interface
    // =========================================================================

    [[nodiscard]] std::vector<ModuleParamDesc> paramDescs(const std::string& prefix) const;
    [[nodiscard]] bool getParam(const std::string& id, ParamValue& out) const;
    bool setParam(const std::string& id, const ParamValue& value);

    // =========================================================================
    // Utility
    // =========================================================================

    static const char* renderModeName(SuperscopeRenderMode mode);
    static const char* audioSourceName(SuperscopeAudioSource source);
    static const char* audioChannelName(SuperscopeAudioChannel channel);
    static const char* blendModeName(SuperscopeBlendMode mode);
    static const char* presetName(SuperscopePreset preset);

    /**
     * @brief Reset execution state (call when starting fresh)
     */
    void resetState();

private:
    // =========================================================================
    // Internal Helpers
    // =========================================================================

    /**
     * @brief Get audio value at position i (0-1)
     */
    float getAudioValue(float i, const float* waveformL, const float* waveformR,
                        const float* spectrumL, const float* spectrumR, int sampleCount) const;

    /**
     * @brief Execute point code for a single point (hardcoded for Phase 1)
     */
    SuperscopePoint executePoint(float i, float v, bool isBeat);

    /**
     * @brief Execute the Lua point script for a single point
     */
    SuperscopePoint executePointLua(float i, float v);

    /// @brief Create the engine and (re)compile all four slots; runs Init
    void initializeLuaScripts();

    /**
     * @brief Apply hardcoded preset function
     */
    void executeHardcodedPreset(float i, float v);

    // =========================================================================
    // Expression Code
    // =========================================================================

    std::string m_initCode;
    std::string m_beatCode;
    std::string m_frameCode;
    std::string m_pointCode;

    // =========================================================================
    // Preset
    // =========================================================================

    SuperscopePreset m_preset = SuperscopePreset::Spiral;

    // Base color table (cycled over time; combined with the gradient by m_colorBlend)
    std::vector<uint32_t> m_colorTable;
    int m_colorBlend = 0;         ///< 0 gradient, 1 table, 2 add, 3 multiply, 4 average
    int m_colorCycleFrames = 60;  ///< frames per table step
    float m_colorPos = 0.0f;      ///< current position in the color table
    float m_frameTableR = 1.0f;   ///< this frame's cycled table color
    float m_frameTableG = 1.0f;
    float m_frameTableB = 1.0f;

    // =========================================================================
    // Render Settings
    // =========================================================================

    int m_pointCount = 256;
    SuperscopeRenderMode m_renderMode = SuperscopeRenderMode::Lines;
    float m_lineWidth = 2.0f;
    float m_dotSize = 4.0f;
    SuperscopeBlendMode m_blendMode = SuperscopeBlendMode::Additive;

    // =========================================================================
    // Audio Settings
    // =========================================================================

    SuperscopeAudioSource m_audioSource = SuperscopeAudioSource::Waveform;
    SuperscopeAudioChannel m_audioChannel = SuperscopeAudioChannel::Mono;

    // =========================================================================
    // Color Gradient
    // =========================================================================

    ColorGradientModule m_colorGradient;

    // =========================================================================
    // Glow Settings
    // =========================================================================

    bool m_glowEnabled = true;
    float m_glowIntensity = 0.5f;
    float m_glowSize = 2.0f;

    // =========================================================================
    // Hold/Fade Settings
    // =========================================================================

    bool m_holdEnabled = false;
    float m_fadeTime = 2.0f;
    int m_maxHoldFrames = 20;

    // =========================================================================
    // Display Settings
    // =========================================================================

    bool m_aspectCorrection = true;
    float m_stretchX = 1.0f;
    float m_stretchY = 1.0f;

    // =========================================================================
    // Runtime State (variables for expressions)
    // =========================================================================

    std::unordered_map<std::string, double> m_variables;

    // Standard variable shortcuts
    double m_n = 256.0;     ///< Point count
    double m_i = 0.0;       ///< Current point index (0-1)
    double m_v = 0.0;       ///< Audio value at current point
    double m_b = 0.0;       ///< Beat indicator (0 or 1)
    double m_w = 800.0;     ///< Viewport width
    double m_h = 600.0;     ///< Viewport height
    double m_t = 0.0;       ///< Time accumulator
    double m_x = 0.0;       ///< Output X
    double m_y = 0.0;       ///< Output Y
    double m_skip = 0.0;    ///< Skip this point
    // Note: red/green/blue output variables were removed (never wired into the
    // expression engine). They return with the Lua color expressions (import phase).

    // Convenience constants
    double m_pi = 3.14159265358979323846;
    double m_pi2 = 6.28318530717958647692;  // 2*pi

    // =========================================================================
    // Execution State
    // =========================================================================

    bool m_initExecuted = false;
    float m_totalTime = 0.0f;

    // =========================================================================
    // Lua Script State
    // =========================================================================

    bool m_luaMode = false;
    std::unique_ptr<scripting::ScriptSlotHost> m_script;  ///< EEL quartet + Engine
    std::string m_lastScriptError;
};

} // namespace lumi::modules
