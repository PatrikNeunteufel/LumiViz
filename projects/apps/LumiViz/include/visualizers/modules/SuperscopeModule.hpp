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

namespace lumi::scripting { class ScriptSlotHost; class ScriptContext; }

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
    /// X/Y in NDC (-1..1) als DOUBLE — wie in AVS. Der Zielpixel entsteht dort
    /// aus `(int)((x+1)*w*0.5)`, und das ist ein Kantenfall: `x=1-2/w` ergibt in
    /// double exakt `w-1`, ueber float gerundet aber `w-1-eps` und damit `w-2`.
    /// Wir schrieben die Skriptwerte frueher als float weg und verloren die
    /// letzte Spalte (Befund S58: bei "The Real Impressionist" faengt ein
    /// Movement genau diese Spalte ein und zieht sie ueber das ganze Bild).
    /// Die Vertex-Puffer bekommen weiterhin float — dort ist es GL-Genauigkeit.
    double x = 0.0;         ///< X coordinate (-1 to 1)
    double y = 0.0;         ///< Y coordinate (-1 to 1)
    float r = 1.0f;         ///< Red (0 to 1)
    float g = 1.0f;         ///< Green (0 to 1)
    float b = 1.0f;         ///< Blue (0 to 1)
    float a = 1.0f;         ///< Alpha (0 to 1)
    bool skip = false;      ///< Skip this point (break line)
    /// Reiner Strip-Trenner OHNE Ankerfunktion (Motion Vectors, Stereo-Wave):
    /// `skip` folgt der AVS-Semantik — der Punkt unterdrueckt nur das in ihm
    /// ENDENDE Segment und bleibt Anker des naechsten (S58). Ein Dummy-Punkt
    /// mit skip allein zieht darum eine Geisterlinie aus seiner (0,0)-Position
    /// zum Folgepunkt (Befund S64: infinity-2-Weissflut, 3071 Center-Quads je
    /// Frame). breakStrip-Punkte werden nirgends gezeichnet UND ankern nicht.
    bool breakStrip = false;
    /// Per-point drawmode (r_sscope EEL var, only meaningful when the point
    /// code mentions `drawmode`): true = line segment to this point, false =
    /// dot. Hosts without per-point support ignore it.
    bool drawLines = true;
    /// Per-point Strichbreite (r_sscope EEL `linesize`, nur gesetzt wenn der
    /// PUNKT-Code sie erwaehnt; 0 = nicht geskriptet, Host-Breite gilt). AVS
    /// wertet sie je Punkt aus: "linesize=dt*ls" mit dt=1/z ergibt die
    /// perspektivische Strichstaerke, aus der etwa Santas Bart besteht
    /// (Befund S50 — wir lasen sie einmal je Frame und zeichneten deshalb
    /// alles in der Breite des LETZTEN Punktes).
    float lineSize = 0.0f;
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
    /**
     * @param context Geteilter Skript-Kontext (reg00..reg99, gmegabuf) — im
     *        Import-Pfad der ScriptContext der Kette, damit ein Scope die
     *        Register liest und schreibt, die andere Effekte setzen. AVS haelt
     *        diese Register GLOBAL; ohne den Kontext bekommt der Scope einen
     *        eigenen, isolierten Satz und liest ueberall 0 (Befund S50: in
     *        "Mister Santa" holen sich alle Scopes die Kamera-Matrix aus
     *        reg00..reg11 einer Dynamic Movement — ohne Kontext blieb der
     *        komplette Vordergrund unsichtbar). Leer = eigener Kontext, so
     *        bleiben die eigenstaendigen LumiViz-Scopes unter sich.
     */
    explicit SuperscopeModule(std::shared_ptr<scripting::ScriptContext> context = {});
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

    // AVS line() clamps linesize to 1..255 (linedraw.cpp:46-47); presets set it
    // via Set Render Mode width (e.g. wormhole = 255 for wall-thick "bar" scopes).
    void setLineWidth(float width) { m_lineWidth = std::clamp(width, 1.0f, 255.0f); }
    [[nodiscard]] float lineWidth() const { return m_lineWidth; }

    void setDotSize(float size) { m_dotSize = std::clamp(size, 1.0f, 50.0f); }
    [[nodiscard]] float dotSize() const { return m_dotSize; }

    void setBlendMode(SuperscopeBlendMode mode) { m_blendMode = mode; }
    [[nodiscard]] SuperscopeBlendMode blendMode() const { return m_blendMode; }

    /// Lua mode: the EEL vars drawmode/linesize are scriptable (r_sscope).
    /// Frame-level readback — true when any slot mentions the var; the host
    /// then applies the script value instead of the UI parameter.
    [[nodiscard]] bool scriptDrawModeActive() const { return m_scriptSetsDrawMode; }
    [[nodiscard]] bool scriptWantsLines() const { return m_scriptDrawMode >= 0.00001; }
    /// True when the POINT code itself switches drawmode — the host must then
    /// split the point list into per-mode runs (SuperscopePoint::drawLines).
    [[nodiscard]] bool pointDrawModeActive() const { return m_scriptSetsDrawModePoint; }
    /// True when the POINT code sets `linesize` — dann traegt jeder Punkt seine
    /// eigene Breite (SuperscopePoint::lineSize) und der Host muss in Laeufe
    /// gleicher Breite zerlegen, genau wie beim Punkt-drawmode.
    [[nodiscard]] bool pointLineSizeActive() const { return m_scriptSetsLineSizePoint; }
    [[nodiscard]] bool scriptLineSizeActive() const { return m_scriptSetsLineSize; }
    [[nodiscard]] float scriptLineSize() const
    {
        return std::clamp(static_cast<float>(m_scriptLineSize), 1.0f, 255.0f);
    }

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

    /// Feed AVS-layout visualisation data (576*4 bytes) + gettime clock to the
    /// scope's script engine, so its point/frame code can call getspec/getosc.
    void setVisData(const unsigned char* data576x4, double scriptTime);

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
     * @brief AVS-treues v aus den visdata-Bytes (r_sscope.cpp:284-289):
     *        Quelle/Kanal-Auswahl auf den 576er-Blöcken, linear interpoliert,
     *        Byte^xorv, /128-1 — Spektrum ist damit -1 bei Stille (Original).
     *        Nur im Lua-/Chain-Pfad aktiv (m_visBytes gesetzt).
     */
    [[nodiscard]] float visdataValue(int point, int count) const;

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

    // Scripted drawmode/linesize (r_sscope EEL vars, frame-level readback)
    bool m_scriptSetsDrawMode = false;
    bool m_scriptSetsDrawModePoint = false;  ///< point code switches drawmode
    bool m_scriptSetsLineSizePoint = false;  ///< point code sets linesize
    bool m_scriptSetsLineSize = false;
    /// Init/Frame/Beat erwaehnt red/green/blue — dann gehoert die Farbe dem
    /// Skript und der Punkt-Lauf setzt sie nicht neu (r_sscope:264-266).
    bool m_scriptSetsColorOutsidePoint = false;
    double m_scriptDrawMode = 0.0;
    double m_scriptLineSize = 1.0;

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
    /// AVS-visdata-Bytes des Hosts (nur Lua-/Chain-Pfad; Frame-Lebensdauer —
    /// der Host hält den Puffer über den Render-Aufruf am Leben)
    const unsigned char* m_visBytes = nullptr;
    /// gettime()-Uhr zum gepufferten visdata (Nachfüttern beim Erst-Compile)
    double m_visTime = 0.0;

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
    /// Geteilter reg/gmegabuf-Raum der Kette (leer = eigener) — s. Konstruktor.
    std::shared_ptr<scripting::ScriptContext> m_context;
    std::unique_ptr<scripting::ScriptSlotHost> m_script;  ///< EEL quartet + Engine
    std::string m_lastScriptError;
};

} // namespace lumi::modules
