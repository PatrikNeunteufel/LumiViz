/**
 ****************************************************************************************
 * @file   ColorGradientModule.cpp
 * @brief  Multi-stop color gradient implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/ColorGradientModule.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>

#include <BasicLogger.h>

namespace lumi::modules {

// Static member initialization
std::string ColorGradientModule::s_userPresetsDir;

// =============================================================================
// Construction
// =============================================================================

ColorGradientModule::ColorGradientModule()
{
    initBuiltinPresets();
    loadUserPresetsFromDisk();
    reset();
}

// =============================================================================
// Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> ColorGradientModule::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    // Mode selection (always visible)
    {
        ModuleParamDesc p;
        p.id = "mode";
        p.displayName = "Color Mode";
        p.tooltip = "Solid color or gradient";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(GradientMode::Solid);
        p.enumOptions = {"Solid", "Linear Gradient", "Radial Gradient", "Outline"};
        p.subGroup = "Color";
        p.order = 0;
        params.push_back(p);
    }

    // Solid color (visible when mode == Solid or Outline)
    {
        ModuleParamDesc p;
        p.id = "solidColor";
        p.displayName = "Color";
        p.tooltip = "Solid fill color (for Solid/Outline mode)";
        p.type = ParamType::Color;
        p.subGroup = "Color";
        p.order = 1;
        p.dependsOn = "mode";
        p.dependsValues = {0, 3};  // Solid=0, Outline=3
        params.push_back(p);
    }

    // Angle (visible when mode == Linear)
    {
        ModuleParamDesc p;
        p.id = "angle";
        p.displayName = "Gradient Angle";
        p.tooltip = "Angle for linear gradient (0=horizontal, 90=vertical)";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 360.0f;
        p.defaultValue = 0.0f;
        p.subGroup = "Color";
        p.order = 2;
        p.dependsOn = "mode";
        p.dependsValues = {1};  // Linear=1
        params.push_back(p);
    }

    // Preset selection (visible when mode == Linear or Radial)
    {
        ModuleParamDesc p;
        p.id = "preset";
        p.displayName = "Preset";
        p.tooltip = "Load a gradient preset";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = presetNames();
        p.subGroup = "Color";
        p.order = 3;
        p.dependsOn = "mode";
        p.dependsValues = {1, 2};  // Linear=1, Radial=2
        params.push_back(p);
    }
    
    // Edit Gradient button (visible when mode == Linear or Radial)
    {
        ModuleParamDesc p;
        p.id = "editGradient";
        p.displayName = "Edit Gradient...";
        p.tooltip = "Open gradient editor dialog";
        p.type = ParamType::String;
        p.subGroup = "Color";
        p.order = 4;
        p.dependsOn = "mode";
        p.dependsValues = {1, 2};  // Linear=1, Radial=2
        params.push_back(p);
    }
    
    // Outline width (visible when mode == Outline)
    {
        ModuleParamDesc p;
        p.id = "outlineWidth";
        p.displayName = "Outline Width";
        p.tooltip = "Width of the outline stroke in pixels";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 15.0f;
        p.defaultValue = 3.0f;
        p.subGroup = "Color";
        p.order = 5;
        p.dependsOn = "mode";
        p.dependsValues = {3};  // Outline=3
        params.push_back(p);
    }
    
    // =========================================================================
    // Hidden parameters for preset serialization
    // =========================================================================
    
    // Gradient preset name - used to reload preset by name if available
    {
        ModuleParamDesc p;
        p.id = "gradientPresetName";
        p.displayName = "Gradient Preset Name";
        p.tooltip = "Internal: Name of the loaded gradient preset";
        p.type = ParamType::String;
        p.hidden = true;
        p.order = 998;
        params.push_back(p);
    }
    
    // Gradient data - fallback if preset not found or is [Custom]
    {
        ModuleParamDesc p;
        p.id = "gradientData";
        p.displayName = "Gradient Data";
        p.tooltip = "Internal: Serialized gradient stop data";
        p.type = ParamType::String;
        p.hidden = true;
        p.order = 999;
        params.push_back(p);
    }

    return params;
}

bool ColorGradientModule::getParam(const std::string& id, ParamValue& out) const
{
    if (id == "mode")
    {
        out = static_cast<int>(m_mode);
        return true;
    }
    if (id == "solidColor")
    {
        out.emplace<7>(m_solidColor);  // Index 7 = Color4f
        return true;
    }
    if (id == "angle")
    {
        out = m_angle;
        return true;
    }
    if (id == "outlineWidth")
    {
        out = m_outlineWidth;
        return true;
    }
    if (id == "preset")
    {
        // Find current preset index
        auto names = presetNames();
        auto it = std::find(names.begin(), names.end(), m_currentPreset);
        out = (it != names.end()) ? static_cast<int>(it - names.begin()) : 0;
        return true;
    }
    if (id == "editGradient")
    {
        // Return empty string - this is a button trigger
        out = std::string("");
        return true;
    }
    if (id == "gradientPresetName")
    {
        // Return the current preset name for serialization
        out = m_currentPreset;
        return true;
    }
    if (id == "gradientData")
    {
        // Serialize gradient stops to string format: "pos,r,g,b,a;pos,r,g,b,a;..."
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        for (size_t i = 0; i < m_stops.size(); ++i)
        {
            if (i > 0) oss << ";";
            oss << m_stops[i].position << ","
                << m_stops[i].color[0] << ","
                << m_stops[i].color[1] << ","
                << m_stops[i].color[2] << ","
                << m_stops[i].color[3];
        }
        out = oss.str();
        return true;
    }
    return false;
}

bool ColorGradientModule::setParam(const std::string& id, const ParamValue& value)
{
    if (id == "mode")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            setMode(static_cast<GradientMode>(*v));
            m_currentPreset = "[Custom]";  // Manual change -> Custom
            return true;
        }
    }
    if (id == "solidColor")
    {
        if (value.index() == 7)
        {
            m_solidColor = std::get<7>(value);
            m_currentPreset = "[Custom]";  // Manual change -> Custom
            return true;
        }
    }
    if (id == "angle")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            setAngle(*v);
            m_currentPreset = "[Custom]";  // Manual change -> Custom
            return true;
        }
    }
    if (id == "preset")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            auto names = presetNames();
            if (*v >= 0 && *v < static_cast<int>(names.size()))
            {
                loadPreset(names[*v]);
                return true;
            }
        }
    }
    if (id == "outlineWidth")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_outlineWidth = std::clamp(*v, 1.0f, 15.0f);
            m_currentPreset = "[Custom]";  // Manual change -> Custom
            return true;
        }
    }
    if (id == "editGradient")
    {
        // Button was clicked - this is handled by the UI
        // The UI should open a GradientEditorDialog
        return true;
    }
    if (id == "gradientPresetName")
    {
        if (auto* v = std::get_if<std::string>(&value))
        {
            // Store the preset name
            // If it's a valid preset name (not [Custom]), try to load it
            if (!v->empty() && *v != "[Custom]")
            {
                auto names = presetNames();
                if (std::find(names.begin(), names.end(), *v) != names.end())
                {
                    // Preset exists on this system - load it
                    loadPreset(*v);
                    return true;
                }
            }
            // Preset not found or is [Custom] - just store the name
            // The gradientData parameter will provide the actual stops
            m_currentPreset = *v;
            return true;
        }
    }
    if (id == "gradientData")
    {
        if (auto* v = std::get_if<std::string>(&value))
        {
            // Only apply gradient data if preset is [Custom] or wasn't found
            // This is checked by seeing if m_currentPreset matches a known preset
            // If loadPreset() was called in gradientPresetName, stops are already set
            
            if (v->empty())
            {
                return true;  // Empty data is valid (keep current stops)
            }
            
            // Check if we should use this data
            // Use it if current preset is [Custom] or not in our preset list
            bool useGradientData = (m_currentPreset == "[Custom]");
            if (!useGradientData)
            {
                auto names = presetNames();
                useGradientData = (std::find(names.begin(), names.end(), m_currentPreset) == names.end());
            }
            
            if (!useGradientData)
            {
                // Preset was loaded successfully, ignore gradientData
                return true;
            }
            
            // Parse string format: "pos,r,g,b,a;pos,r,g,b,a;..."
            std::vector<ColorStop> newStops;
            std::istringstream iss(*v);
            std::string stopStr;
            
            while (std::getline(iss, stopStr, ';'))
            {
                if (stopStr.empty()) continue;
                
                std::istringstream stopStream(stopStr);
                std::string token;
                std::vector<float> values;
                
                while (std::getline(stopStream, token, ','))
                {
                    try
                    {
                        values.push_back(std::stof(token));
                    }
                    catch (...)
                    {
                        values.clear();
                        break;
                    }
                }
                
                if (values.size() == 5)
                {
                    ColorStop stop;
                    stop.position = std::clamp(values[0], 0.0f, 1.0f);
                    stop.color = {values[1], values[2], values[3], values[4]};
                    newStops.push_back(stop);
                }
            }
            
            if (newStops.size() >= 2)
            {
                m_stops = std::move(newStops);
                // Keep m_currentPreset as set by gradientPresetName
                return true;
            }
        }
    }
    return false;
}

// =============================================================================
// Mode Configuration
// =============================================================================

void ColorGradientModule::setMode(GradientMode mode)
{
    m_mode = mode;
}

void ColorGradientModule::setAngle(float degrees)
{
    m_angle = std::fmod(degrees, 360.0f);
    if (m_angle < 0.0f) m_angle += 360.0f;
}

void ColorGradientModule::setOutlineWidth(float width)
{
    m_outlineWidth = std::clamp(width, 1.0f, 15.0f);
}

// =============================================================================
// Solid Color
// =============================================================================

void ColorGradientModule::setSolidColor(float r, float g, float b, float a)
{
    m_solidColor = {r, g, b, a};
}

void ColorGradientModule::setSolidColor(const Color4f& color)
{
    m_solidColor = color;
}

// =============================================================================
// Color Stops Management
// =============================================================================

size_t ColorGradientModule::addStop(float position, const Color4f& color)
{
    position = std::clamp(position, 0.0f, 1.0f);
    
    ColorStop stop;
    stop.position = position;
    stop.color = color;
    
    m_stops.push_back(stop);
    sortStops();
    updateMidpointsCount();
    m_currentPreset = "[Custom]";  // Manual change -> Custom
    
    // Find index of the added stop
    for (size_t i = 0; i < m_stops.size(); ++i)
    {
        if (std::abs(m_stops[i].position - position) < 0.0001f)
        {
            return i;
        }
    }
    return m_stops.size() - 1;
}

bool ColorGradientModule::removeStop(size_t index)
{
    // Keep at least 2 stops
    if (m_stops.size() <= 2 || index >= m_stops.size())
    {
        return false;
    }
    
    m_stops.erase(m_stops.begin() + static_cast<ptrdiff_t>(index));
    updateMidpointsCount();
    m_currentPreset = "[Custom]";  // Manual change -> Custom
    return true;
}

void ColorGradientModule::updateStop(size_t index, float position, const Color4f& color)
{
    if (index >= m_stops.size())
    {
        return;
    }
    
    m_stops[index].position = std::clamp(position, 0.0f, 1.0f);
    m_stops[index].color = color;
    sortStops();
    m_currentPreset = "[Custom]";  // Manual change -> Custom
}

void ColorGradientModule::clearStops()
{
    m_stops.clear();
    m_midpoints.clear();
    ensureMinimumStops();
    m_currentPreset = "[Custom]";  // Manual change -> Custom
}

// =============================================================================
// Midpoints Management
// =============================================================================

void ColorGradientModule::setMidpoint(size_t index, float position)
{
    if (index < m_midpoints.size())
    {
        m_midpoints[index].position = std::clamp(position, 0.0f, 1.0f);
        m_currentPreset = "[Custom]";  // Manual change -> Custom
    }
}

float ColorGradientModule::midpoint(size_t index) const
{
    if (index < m_midpoints.size())
    {
        return m_midpoints[index].position;
    }
    return 0.5f;
}

// =============================================================================
// Color Sampling
// =============================================================================

Color4f ColorGradientModule::sample(float t) const
{
    // Solid mode: return solid color
    if (m_mode == GradientMode::Solid || m_stops.size() < 2)
    {
        return m_solidColor;
    }
    
    t = std::clamp(t, 0.0f, 1.0f);
    
    // Find surrounding stops
    size_t i = 0;
    while (i < m_stops.size() - 1 && m_stops[i + 1].position < t)
    {
        ++i;
    }
    
    // Clamp to range
    if (t <= m_stops.front().position)
    {
        return m_stops.front().color;
    }
    if (t >= m_stops.back().position)
    {
        return m_stops.back().color;
    }
    
    // Get surrounding stops
    const ColorStop& start = m_stops[i];
    const ColorStop& end = m_stops[i + 1];
    
    // Calculate local t within segment
    float segmentLength = end.position - start.position;
    if (segmentLength < 0.0001f)
    {
        return start.color;
    }
    
    float localT = (t - start.position) / segmentLength;
    
    // Apply midpoint adjustment
    localT = applyMidpoint(localT, i);
    
    // Linear interpolation
    Color4f result;
    for (int c = 0; c < 4; ++c)
    {
        result[c] = start.color[c] + localT * (end.color[c] - start.color[c]);
    }
    
    return result;
}

Color4f ColorGradientModule::sample2D(float x, float y) const
{
    if (m_mode == GradientMode::Solid)
    {
        return m_solidColor;
    }
    
    if (m_mode == GradientMode::Outline)
    {
        // Outline mode: only color at edges
        float dist = std::sqrt(x * x + y * y);
        if (dist > (1.0f - m_outlineWidth))
        {
            return m_solidColor;
        }
        return {0.0f, 0.0f, 0.0f, 0.0f};  // Transparent inside
    }
    
    float t = 0.0f;
    
    switch (m_mode)
    {
        case GradientMode::Linear:
        {
            float rad = m_angle * 3.14159265f / 180.0f;
            float c = std::cos(rad);
            float s = std::sin(rad);
            t = (x * c + y * s) * 0.5f + 0.5f;
            break;
        }
            
        case GradientMode::Radial:
            t = std::sqrt(x * x + y * y);
            t = std::clamp(t, 0.0f, 1.0f);
            break;
            
        default:
            break;
    }
    
    return sample(t);
}

// =============================================================================
// Presets
// =============================================================================

void ColorGradientModule::initBuiltinPresets()
{
    // Fire: Red -> Orange -> Yellow
    {
        GradientPreset p;
        p.name = "Fire";
        p.mode = GradientMode::Radial;
        p.stops = {
            {0.0f, {1.0f, 1.0f, 0.0f, 1.0f}},   // Yellow center
            {0.5f, {1.0f, 0.5f, 0.0f, 1.0f}},   // Orange
            {1.0f, {0.8f, 0.0f, 0.0f, 1.0f}}    // Dark red edge
        };
        p.midpoints = {{0.3f}, {0.7f}};  // Shift orange toward center, red toward edge
        m_builtinPresets["Fire"] = p;
    }
    
    // Ocean: Deep blue -> Cyan -> White
    {
        GradientPreset p;
        p.name = "Ocean";
        p.mode = GradientMode::Radial;
        p.stops = {
            {0.0f, {0.9f, 1.0f, 1.0f, 1.0f}},   // White center
            {0.4f, {0.0f, 0.8f, 1.0f, 1.0f}},   // Cyan
            {1.0f, {0.0f, 0.2f, 0.5f, 1.0f}}    // Deep blue edge
        };
        p.midpoints = {{0.7f}, {0.3f}};  // More cyan visible
        m_builtinPresets["Ocean"] = p;
    }
    
    // Neon: Magenta -> Cyan
    {
        GradientPreset p;
        p.name = "Neon";
        p.mode = GradientMode::Radial;
        p.stops = {
            {0.0f, {1.0f, 0.0f, 1.0f, 1.0f}},   // Magenta center
            {1.0f, {0.0f, 1.0f, 1.0f, 1.0f}}    // Cyan edge
        };
        p.midpoints = {{0.3f}};  // More magenta visible
        m_builtinPresets["Neon"] = p;
    }
    
    // Rainbow (full 7-color spectrum)
    {
        GradientPreset p;
        p.name = "Rainbow";
        p.mode = GradientMode::Linear;
        p.angle = 0.0f;  // Horizontal
        p.stops = {
            {0.000f, {1.0f, 0.0f, 0.0f, 1.0f}},  // Red
            {0.167f, {1.0f, 0.5f, 0.0f, 1.0f}},  // Orange
            {0.333f, {1.0f, 1.0f, 0.0f, 1.0f}},  // Yellow
            {0.500f, {0.0f, 1.0f, 0.0f, 1.0f}},  // Green
            {0.667f, {0.0f, 1.0f, 1.0f, 1.0f}},  // Cyan
            {0.833f, {0.0f, 0.0f, 1.0f, 1.0f}},  // Blue
            {1.000f, {0.5f, 0.0f, 1.0f, 1.0f}}   // Violet
        };
        p.midpoints = {{0.5f}, {0.5f}, {0.5f}, {0.5f}, {0.5f}, {0.5f}};
        m_builtinPresets["Rainbow"] = p;
    }
    
    // Sunset: Purple -> Pink -> Orange -> Yellow
    {
        GradientPreset p;
        p.name = "Sunset";
        p.mode = GradientMode::Linear;
        p.angle = 90.0f;  // Vertical
        p.stops = {
            {0.0f, {1.0f, 0.9f, 0.3f, 1.0f}},   // Yellow (top)
            {0.4f, {1.0f, 0.5f, 0.2f, 1.0f}},   // Orange
            {0.7f, {1.0f, 0.3f, 0.5f, 1.0f}},   // Pink
            {1.0f, {0.4f, 0.2f, 0.6f, 1.0f}}    // Purple (bottom)
        };
        p.midpoints = {{0.5f}, {0.5f}, {0.5f}};
        m_builtinPresets["Sunset"] = p;
    }
    
    // Forest: Deep green -> Bright green
    {
        GradientPreset p;
        p.name = "Forest";
        p.mode = GradientMode::Radial;
        p.stops = {
            {0.0f, {0.6f, 1.0f, 0.4f, 1.0f}},   // Bright green center
            {1.0f, {0.1f, 0.4f, 0.2f, 1.0f}}    // Dark green edge
        };
        p.midpoints = {{0.5f}};
        m_builtinPresets["Forest"] = p;
    }
    
    // Ice: White -> Light blue -> Dark blue
    {
        GradientPreset p;
        p.name = "Ice";
        p.mode = GradientMode::Radial;
        p.stops = {
            {0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},   // White center
            {0.5f, {0.7f, 0.9f, 1.0f, 1.0f}},   // Light blue
            {1.0f, {0.3f, 0.5f, 0.8f, 1.0f}}    // Dark blue edge
        };
        p.midpoints = {{0.5f}, {0.5f}};
        m_builtinPresets["Ice"] = p;
    }
    
    // Lava: Black -> Red -> Orange -> Yellow
    {
        GradientPreset p;
        p.name = "Lava";
        p.mode = GradientMode::Radial;
        p.stops = {
            {0.0f, {1.0f, 1.0f, 0.6f, 1.0f}},   // Yellow center
            {0.3f, {1.0f, 0.6f, 0.0f, 1.0f}},   // Orange
            {0.6f, {0.8f, 0.2f, 0.0f, 1.0f}},   // Red
            {1.0f, {0.2f, 0.0f, 0.0f, 1.0f}}    // Dark red/black edge
        };
        p.midpoints = {{0.5f}, {0.5f}, {0.5f}};
        m_builtinPresets["Lava"] = p;
    }
    
    // Galaxy: Purple -> Blue -> Pink
    {
        GradientPreset p;
        p.name = "Galaxy";
        p.mode = GradientMode::Radial;
        p.stops = {
            {0.0f, {1.0f, 0.8f, 1.0f, 1.0f}},   // Light pink center
            {0.4f, {0.6f, 0.4f, 1.0f, 1.0f}},   // Purple
            {0.7f, {0.3f, 0.3f, 0.8f, 1.0f}},   // Blue
            {1.0f, {0.1f, 0.1f, 0.3f, 1.0f}}    // Dark blue edge
        };
        p.midpoints = {{0.5f}, {0.5f}, {0.5f}};
        m_builtinPresets["Galaxy"] = p;
    }
    
    // Monochrome: White -> Black
    {
        GradientPreset p;
        p.name = "Monochrome";
        p.mode = GradientMode::Radial;
        p.stops = {
            {0.0f, {1.0f, 1.0f, 1.0f, 1.0f}},   // White center
            {1.0f, {0.0f, 0.0f, 0.0f, 1.0f}}    // Black edge
        };
        p.midpoints = {{0.5f}};
        m_builtinPresets["Monochrome"] = p;
    }
}

void ColorGradientModule::loadPreset(const std::string& name)
{
    // [Custom] is not a real preset - just keep current values
    if (name == "[Custom]")
    {
        m_currentPreset = "[Custom]";
        return;
    }
    
    // Skip separator
    if (name == "---")
    {
        return;
    }
    
    // Check builtin presets
    auto it = m_builtinPresets.find(name);
    if (it != m_builtinPresets.end())
    {
        const auto& preset = it->second;
        m_mode = preset.mode;
        m_angle = preset.angle;
        m_stops = preset.stops;
        m_midpoints = preset.midpoints;
        m_currentPreset = name;
        return;
    }
    
    // Check user presets
    it = m_userPresets.find(name);
    if (it != m_userPresets.end())
    {
        const auto& preset = it->second;
        m_mode = preset.mode;
        m_angle = preset.angle;
        m_stops = preset.stops;
        m_midpoints = preset.midpoints;
        m_currentPreset = name;
        return;
    }
}

std::vector<std::string> ColorGradientModule::presetNames() const
{
    // Lazy-load user presets on first access
    // (Directory might not be set during construction)
    if (!m_userPresetsLoaded && !s_userPresetsDir.empty())
    {
        const_cast<ColorGradientModule*>(this)->loadUserPresetsFromDisk();
    }
    
    std::vector<std::string> names;
    
    // Add [Custom] as first entry (shown when user modifies values)
    names.push_back("[Custom]");
    
    // Add builtin presets
    for (const auto& pair : m_builtinPresets)
    {
        names.push_back(pair.first);
    }
    
    // Separator if user presets exist
    if (!m_userPresets.empty())
    {
        names.push_back("---");  // Separator
        
        // User presets
        for (const auto& pair : m_userPresets)
        {
            names.push_back(pair.first);
        }
    }
    
    return names;
}

void ColorGradientModule::savePreset(const std::string& name)
{
    GradientPreset preset;
    preset.name = name;
    preset.mode = m_mode;
    preset.angle = m_angle;
    preset.stops = m_stops;
    preset.midpoints = m_midpoints;
    
    m_userPresets[name] = preset;
    m_currentPreset = name;
    
    // Save to individual .grad file
    if (s_userPresetsDir.empty())
    {
        BasicLogger::logWarning("ColorGradientModule: No user presets dir set, cannot save");
        return;
    }
    
    std::filesystem::create_directories(s_userPresetsDir);
    std::string filePath = s_userPresetsDir + "/" + name + ".grad";
    
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        BasicLogger::logError("ColorGradientModule: Cannot write to " + filePath);
        return;
    }
    
    // Write JSON
    file << "{\n";
    file << "  \"name\": \"" << name << "\",\n";
    file << "  \"mode\": " << static_cast<int>(preset.mode) << ",\n";
    file << "  \"angle\": " << preset.angle << ",\n";
    
    // Stops
    file << "  \"stops\": [";
    for (size_t i = 0; i < preset.stops.size(); ++i)
    {
        const auto& s = preset.stops[i];
        if (i > 0) file << ", ";
        file << "[" << s.position << "," 
             << s.color[0] << "," << s.color[1] << "," 
             << s.color[2] << "," << s.color[3] << "]";
    }
    file << "],\n";
    
    // Midpoints
    file << "  \"midpoints\": [";
    for (size_t i = 0; i < preset.midpoints.size(); ++i)
    {
        if (i > 0) file << ", ";
        file << preset.midpoints[i].position;
    }
    file << "]\n";
    
    file << "}\n";
    
    BasicLogger::logInfo("ColorGradientModule: Saved gradient preset '" + name + "' to " + filePath);
}

bool ColorGradientModule::deletePreset(const std::string& name)
{
    // Can't delete builtin presets
    if (m_builtinPresets.find(name) != m_builtinPresets.end())
    {
        return false;
    }
    
    auto it = m_userPresets.find(name);
    if (it != m_userPresets.end())
    {
        m_userPresets.erase(it);
        
        // Delete .grad file
        if (!s_userPresetsDir.empty())
        {
            std::string filePath = s_userPresetsDir + "/" + name + ".grad";
            std::filesystem::remove(filePath);
            BasicLogger::logInfo("ColorGradientModule: Deleted gradient preset '" + name + "'");
        }
        return true;
    }
    
    return false;
}

void ColorGradientModule::reset()
{
    m_mode = GradientMode::Solid;
    m_solidColor = {1.0f, 0.0f, 1.0f, 1.0f};  // Magenta
    m_angle = 0.0f;
    
    m_stops.clear();
    m_stops.push_back({0.0f, {1.0f, 0.0f, 1.0f, 1.0f}});  // Magenta
    m_stops.push_back({1.0f, {0.0f, 1.0f, 1.0f, 1.0f}});  // Cyan
    
    m_midpoints.clear();
    m_midpoints.push_back({0.5f});
    
    m_currentPreset = "[Custom]";  // Default to custom
}

// =============================================================================
// Serialization
// =============================================================================

std::string ColorGradientModule::toJson() const
{
    std::ostringstream ss;
    
    ss << "{\n";
    ss << "  \"mode\": " << static_cast<int>(m_mode) << ",\n";
    ss << "  \"angle\": " << m_angle << ",\n";
    ss << "  \"solidColor\": [" 
       << m_solidColor[0] << ", " << m_solidColor[1] << ", " 
       << m_solidColor[2] << ", " << m_solidColor[3] << "],\n";
    
    ss << "  \"stops\": [\n";
    for (size_t i = 0; i < m_stops.size(); ++i)
    {
        const auto& s = m_stops[i];
        ss << "    {\"position\": " << s.position 
           << ", \"color\": [" << s.color[0] << ", " << s.color[1] 
           << ", " << s.color[2] << ", " << s.color[3] << "]}";
        if (i < m_stops.size() - 1) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";
    
    ss << "  \"midpoints\": [";
    for (size_t i = 0; i < m_midpoints.size(); ++i)
    {
        ss << m_midpoints[i].position;
        if (i < m_midpoints.size() - 1) ss << ", ";
    }
    ss << "]\n";
    
    ss << "}";
    
    return ss.str();
}

bool ColorGradientModule::fromJson(const std::string& /*json*/)
{
    // TODO: Implement JSON parsing
    return false;
}

// =============================================================================
// Internal Helpers
// =============================================================================

void ColorGradientModule::sortStops()
{
    std::sort(m_stops.begin(), m_stops.end());
}

void ColorGradientModule::ensureMinimumStops()
{
    if (m_stops.empty())
    {
        m_stops.push_back({0.0f, {1.0f, 1.0f, 1.0f, 1.0f}});
        m_stops.push_back({1.0f, {0.0f, 0.0f, 0.0f, 1.0f}});
    }
    else if (m_stops.size() == 1)
    {
        if (m_stops[0].position < 0.5f)
        {
            m_stops.push_back({1.0f, {0.0f, 0.0f, 0.0f, 1.0f}});
        }
        else
        {
            m_stops.insert(m_stops.begin(), {0.0f, {1.0f, 1.0f, 1.0f, 1.0f}});
        }
    }
    updateMidpointsCount();
}

void ColorGradientModule::updateMidpointsCount()
{
    // One midpoint between each pair of stops
    size_t needed = m_stops.size() > 1 ? m_stops.size() - 1 : 0;
    
    while (m_midpoints.size() < needed)
    {
        m_midpoints.push_back({0.5f});
    }
    while (m_midpoints.size() > needed)
    {
        m_midpoints.pop_back();
    }
}

float ColorGradientModule::applyMidpoint(float t, size_t segmentIndex) const
{
    if (segmentIndex >= m_midpoints.size())
    {
        return t;
    }
    
    float mid = m_midpoints[segmentIndex].position;
    
    // The midpoint defines WHERE the 50/50 blend occurs
    // Remap t so that t=mid maps to 0.5 (50% blend)
    // t ∈ [0, mid] → [0, 0.5]
    // t ∈ [mid, 1] → [0.5, 1]
    
    // Handle edge cases
    if (mid <= 0.001f)
    {
        return t < 0.001f ? 0.5f : 0.5f + t * 0.5f;
    }
    if (mid >= 0.999f)
    {
        return t > 0.999f ? 0.5f : t * 0.5f;
    }
    
    // Piecewise linear remapping
    if (t <= mid)
    {
        // Map [0, mid] to [0, 0.5]
        return t * 0.5f / mid;
    }
    else
    {
        // Map [mid, 1] to [0.5, 1]
        return 0.5f + (t - mid) * 0.5f / (1.0f - mid);
    }
}

// =============================================================================
// User Preset Persistence
// =============================================================================

void ColorGradientModule::setUserPresetsDirectory(const std::string& path)
{
    s_userPresetsDir = path;
    BasicLogger::logInfo("ColorGradientModule: User presets dir set to " + path);
}

void ColorGradientModule::loadUserPresetsFromDisk()
{
    if (s_userPresetsDir.empty())
    {
        // Don't mark as loaded if directory not set yet
        // presetNames() will retry later
        return;
    }
    
    m_userPresetsLoaded = true;  // Mark as loaded (even if no files exist)
    
    // Scan directory for .grad files
    std::filesystem::path dirPath(s_userPresetsDir);
    if (!std::filesystem::exists(dirPath))
    {
        BasicLogger::logDebug("ColorGradientModule: User presets dir does not exist yet: " + s_userPresetsDir);
        return;
    }
    
    int loadedCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dirPath))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        
        std::string ext = entry.path().extension().string();
        if (ext != ".grad")
        {
            continue;
        }
        
        // Load single preset file
        std::ifstream file(entry.path());
        if (!file.is_open())
        {
            continue;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        GradientPreset preset;
        if (parseGradientFile(content, preset))
        {
            m_userPresets[preset.name] = preset;
            ++loadedCount;
            BasicLogger::logDebug("ColorGradientModule: Loaded '" + preset.name + "' from " + 
                                  entry.path().filename().string());
        }
    }
    
    BasicLogger::logInfo("ColorGradientModule: Loaded " + 
                         std::to_string(loadedCount) + " user gradient presets");
}

bool ColorGradientModule::parseGradientFile(const std::string& content, GradientPreset& preset)
{
    // Parse single .grad JSON file
    // Format: { "name": "...", "mode": 0, "angle": 0, 
    //           "stops": [[pos,r,g,b,a],...], "midpoints": [0.5,...] }
    
    // Name
    size_t namePos = content.find("\"name\"");
    if (namePos != std::string::npos)
    {
        size_t valStart = content.find('"', namePos + 6);
        size_t valEnd = content.find('"', valStart + 1);
        if (valStart != std::string::npos && valEnd != std::string::npos)
        {
            preset.name = content.substr(valStart + 1, valEnd - valStart - 1);
        }
    }
    
    // Mode
    size_t modePos = content.find("\"mode\"");
    if (modePos != std::string::npos)
    {
        size_t valStart = content.find(':', modePos) + 1;
        while (valStart < content.size() && std::isspace(content[valStart])) ++valStart;
        try {
            int mode = std::stoi(content.substr(valStart));
            preset.mode = static_cast<GradientMode>(mode);
        } catch (...) {}
    }
    
    // Angle
    size_t anglePos = content.find("\"angle\"");
    if (anglePos != std::string::npos)
    {
        size_t valStart = content.find(':', anglePos) + 1;
        while (valStart < content.size() && std::isspace(content[valStart])) ++valStart;
        try {
            preset.angle = std::stof(content.substr(valStart));
        } catch (...) {}
    }
    
    // Stops array - need to find matching bracket (nested arrays!)
    size_t stopsPos = content.find("\"stops\"");
    if (stopsPos != std::string::npos)
    {
        size_t arrStart = content.find('[', stopsPos);
        if (arrStart != std::string::npos)
        {
            // Find matching closing bracket by counting
            int bracketCount = 1;
            size_t arrEnd = arrStart + 1;
            while (arrEnd < content.size() && bracketCount > 0)
            {
                if (content[arrEnd] == '[') ++bracketCount;
                else if (content[arrEnd] == ']') --bracketCount;
                ++arrEnd;
            }
            
            if (bracketCount == 0)
            {
                std::string stopsStr = content.substr(arrStart + 1, arrEnd - arrStart - 2);
                size_t stopPos = 0;
                while ((stopPos = stopsStr.find('[', stopPos)) != std::string::npos)
                {
                    size_t stopEnd = stopsStr.find(']', stopPos);
                    if (stopEnd == std::string::npos) break;
                    
                    std::string stopStr = stopsStr.substr(stopPos + 1, stopEnd - stopPos - 1);
                    std::istringstream ss(stopStr);
                    float p, r, g, b, a;
                    char comma;
                    if (ss >> p >> comma >> r >> comma >> g >> comma >> b >> comma >> a)
                    {
                        preset.stops.push_back({p, {r, g, b, a}});
                    }
                    stopPos = stopEnd + 1;
                }
            }
        }
    }
    
    // Midpoints array
    size_t midPos = content.find("\"midpoints\"");
    if (midPos != std::string::npos)
    {
        size_t arrStart = content.find('[', midPos);
        size_t arrEnd = content.find(']', arrStart);
        if (arrStart != std::string::npos && arrEnd != std::string::npos)
        {
            std::string midsStr = content.substr(arrStart + 1, arrEnd - arrStart - 1);
            std::istringstream ss(midsStr);
            float val;
            while (ss >> val)
            {
                preset.midpoints.push_back({val});
                char c;
                ss >> c;  // Skip comma
            }
        }
    }
    
    return !preset.name.empty() && !preset.stops.empty();
}

void ColorGradientModule::saveUserPresetsToDisk() const
{
    // This is now a no-op - individual presets are saved via savePreset()
    // Kept for API compatibility
}

} // namespace lumi::modules
