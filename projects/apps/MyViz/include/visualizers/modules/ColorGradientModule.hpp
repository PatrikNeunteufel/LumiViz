/**
 ****************************************************************************************
 * @file   ColorGradientModule.hpp
 * @brief  Multi-stop color gradient with midpoints and presets
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * Professional gradient editor supporting:
 *   - Multiple color stops at arbitrary positions
 *   - Adjustable midpoints between stops for non-linear interpolation
 *   - Preset save/load/reset functionality
 *   - Solid color mode (single color)
 *   - Gradient modes: Radial, Linear (horizontal/vertical/angle)
 * 
 * Note: This is a helper module, NOT derived from IModule.
 * It provides gradient functionality to visualizers.
 ****************************************************************************************
 */

#pragma once

#include "IModule.hpp"  // For ParamValue, ModuleParamDesc, Color4f

#include <vector>
#include <string>
#include <map>
#include <array>

namespace lumi::modules {

// =============================================================================
// Color Stop Structure
// =============================================================================

/**
 * @brief A single color stop in the gradient
 */
struct ColorStop
{
    float position = 0.0f;           ///< Position on gradient (0-1)
    std::array<float, 4> color = {1.0f, 1.0f, 1.0f, 1.0f};  ///< RGBA color
    
    bool operator<(const ColorStop& other) const 
    { 
        return position < other.position; 
    }
};

/**
 * @brief Midpoint between two color stops
 */
struct GradientMidpoint
{
    float position = 0.5f;  ///< Relative position between stops (0-1)
};

// =============================================================================
// Gradient Mode
// =============================================================================

/**
 * @brief How the gradient is applied
 */
enum class GradientMode
{
    Solid,      ///< Single color (no gradient)
    Linear,     ///< Linear gradient with configurable angle
    Radial,     ///< Center to edge gradient
    Outline     ///< Outline/stroke only (fill transparent)
};

// =============================================================================
// Gradient Preset
// =============================================================================

/**
 * @brief Saved gradient configuration
 */
struct GradientPreset
{
    std::string name;
    std::vector<ColorStop> stops;
    std::vector<GradientMidpoint> midpoints;
    GradientMode mode = GradientMode::Radial;
    float angle = 0.0f;
};

// =============================================================================
// ColorGradientModule
// =============================================================================

/**
 * @class ColorGradientModule
 * @brief Professional gradient helper module (NOT derived from IModule)
 */
class ColorGradientModule
{
public:
    ColorGradientModule();
    ~ColorGradientModule() = default;

    // =========================================================================
    // Parameter Interface (IModule-style but not virtual)
    // =========================================================================

    [[nodiscard]] std::vector<ModuleParamDesc> paramDescs() const;
    [[nodiscard]] bool getParam(const std::string& id, ParamValue& out) const;
    bool setParam(const std::string& id, const ParamValue& value);

    // =========================================================================
    // Gradient Mode
    // =========================================================================

    void setMode(GradientMode mode);
    [[nodiscard]] GradientMode mode() const { return m_mode; }

    void setAngle(float degrees);
    [[nodiscard]] float angle() const { return m_angle; }
    
    void setOutlineWidth(float width);
    [[nodiscard]] float outlineWidth() const { return m_outlineWidth; }

    // =========================================================================
    // Solid Color (when mode == Solid)
    // =========================================================================

    void setSolidColor(float r, float g, float b, float a = 1.0f);
    void setSolidColor(const Color4f& color);
    [[nodiscard]] Color4f solidColor() const { return m_solidColor; }

    // =========================================================================
    // Color Stops Management
    // =========================================================================

    size_t addStop(float position, const Color4f& color);
    bool removeStop(size_t index);
    void updateStop(size_t index, float position, const Color4f& color);
    [[nodiscard]] const std::vector<ColorStop>& stops() const { return m_stops; }
    [[nodiscard]] size_t stopCount() const { return m_stops.size(); }
    void clearStops();

    // =========================================================================
    // Midpoints Management
    // =========================================================================

    void setMidpoint(size_t index, float position);
    [[nodiscard]] float midpoint(size_t index) const;
    [[nodiscard]] const std::vector<GradientMidpoint>& midpoints() const { return m_midpoints; }

    // =========================================================================
    // Color Sampling
    // =========================================================================

    [[nodiscard]] Color4f sample(float t) const;
    [[nodiscard]] Color4f sample2D(float x, float y) const;

    // =========================================================================
    // Presets
    // =========================================================================

    void loadPreset(const std::string& name);
    [[nodiscard]] std::vector<std::string> presetNames() const;
    void savePreset(const std::string& name);
    bool deletePreset(const std::string& name);
    void reset();
    
    /// Set directory for user presets (call before using presets)
    static void setUserPresetsDirectory(const std::string& path);
    
    /// Load user presets from disk
    void loadUserPresetsFromDisk();
    
    /// Save user presets to disk
    void saveUserPresetsToDisk() const;

    // =========================================================================
    // Serialization
    // =========================================================================

    [[nodiscard]] std::string toJson() const;
    bool fromJson(const std::string& json);

private:
    void sortStops();
    void ensureMinimumStops();
    void updateMidpointsCount();
    [[nodiscard]] float applyMidpoint(float t, size_t segmentIndex) const;
    void initBuiltinPresets();
    [[nodiscard]] bool parseGradientFile(const std::string& content, GradientPreset& preset);

    GradientMode m_mode = GradientMode::Solid;
    float m_angle = 0.0f;
    float m_outlineWidth = 3.0f;  ///< Width for Outline mode (pixels)
    Color4f m_solidColor = {1.0f, 0.0f, 1.0f, 1.0f};

    std::vector<ColorStop> m_stops;
    std::vector<GradientMidpoint> m_midpoints;

    std::map<std::string, GradientPreset> m_builtinPresets;
    std::map<std::string, GradientPreset> m_userPresets;
    std::string m_currentPreset;
    bool m_userPresetsLoaded = false;  ///< Flag to track if user presets were loaded
    
    static std::string s_userPresetsDir;
};

} // namespace lumi::modules
