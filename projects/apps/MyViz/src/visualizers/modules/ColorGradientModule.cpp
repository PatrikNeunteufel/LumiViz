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

namespace lumi::modules {

// =============================================================================
// Construction
// =============================================================================

ColorGradientModule::ColorGradientModule()
{
    initBuiltinPresets();
    reset();
}

// =============================================================================
// Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> ColorGradientModule::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    // Mode selection
    {
        ModuleParamDesc p;
        p.id = "mode";
        p.displayName = "Color Mode";
        p.tooltip = "Solid color or gradient";
        p.type = ParamType::Enum;
        p.defaultValue = static_cast<int>(GradientMode::Solid);
        p.enumOptions = {"Solid", "Linear Gradient", "Radial Gradient", "Outline"};
        p.order = 0;
        params.push_back(p);
    }

    // Solid color (visible when mode == Solid)
    {
        ModuleParamDesc p;
        p.id = "solidColor";
        p.displayName = "Color";
        p.tooltip = "Solid fill color (for Solid/Outline mode)";
        p.type = ParamType::Color;
        p.order = 1;
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
        p.order = 2;
        params.push_back(p);
    }

    // Preset selection
    {
        ModuleParamDesc p;
        p.id = "preset";
        p.displayName = "Preset";
        p.tooltip = "Load a gradient preset";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = presetNames();
        p.order = 3;
        params.push_back(p);
    }
    
    // Edit Gradient button (triggers dialog)
    // Note: displayName ending with "..." signals to UI to show as button
    {
        ModuleParamDesc p;
        p.id = "editGradient";
        p.displayName = "Edit Gradient...";
        p.tooltip = "Open gradient editor dialog";
        p.type = ParamType::String;
        p.order = 4;
        params.push_back(p);
    }
    
    // Outline width (visible when mode == Outline)
    {
        ModuleParamDesc p;
        p.id = "outlineWidth";
        p.displayName = "Outline Width";
        p.tooltip = "Width of the outline stroke";
        p.type = ParamType::Float;
        p.minValue = 0.01f;
        p.maxValue = 0.2f;
        p.defaultValue = 0.05f;
        p.order = 5;
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
    return false;
}

bool ColorGradientModule::setParam(const std::string& id, const ParamValue& value)
{
    if (id == "mode")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            setMode(static_cast<GradientMode>(*v));
            return true;
        }
    }
    if (id == "solidColor")
    {
        if (value.index() == 7)
        {
            m_solidColor = std::get<7>(value);
            return true;
        }
    }
    if (id == "angle")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            setAngle(*v);
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
            m_outlineWidth = std::clamp(*v, 0.01f, 0.2f);
            return true;
        }
    }
    if (id == "editGradient")
    {
        // Button was clicked - this is handled by the UI
        // The UI should open a GradientEditorDialog
        return true;
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
    m_outlineWidth = std::clamp(width, 0.01f, 0.2f);
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
}

void ColorGradientModule::clearStops()
{
    m_stops.clear();
    m_midpoints.clear();
    ensureMinimumStops();
}

// =============================================================================
// Midpoints Management
// =============================================================================

void ColorGradientModule::setMidpoint(size_t index, float position)
{
    if (index < m_midpoints.size())
    {
        m_midpoints[index].position = std::clamp(position, 0.0f, 1.0f);
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
        p.midpoints = {{0.5f}, {0.5f}};
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
        p.midpoints = {{0.5f}, {0.5f}};
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
        p.midpoints = {{0.5f}};
        m_builtinPresets["Neon"] = p;
    }
    
    // Rainbow
    {
        GradientPreset p;
        p.name = "Rainbow";
        p.mode = GradientMode::Linear;
        p.angle = 0.0f;  // Horizontal
        p.stops = {
            {0.0f,  {1.0f, 0.0f, 0.0f, 1.0f}},  // Red
            {0.17f, {1.0f, 0.5f, 0.0f, 1.0f}},  // Orange
            {0.33f, {1.0f, 1.0f, 0.0f, 1.0f}},  // Yellow
            {0.5f,  {0.0f, 1.0f, 0.0f, 1.0f}},  // Green
            {0.67f, {0.0f, 1.0f, 1.0f, 1.0f}},  // Cyan
            {0.83f, {0.0f, 0.0f, 1.0f, 1.0f}},  // Blue
            {1.0f,  {0.5f, 0.0f, 1.0f, 1.0f}}   // Violet
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
    std::vector<std::string> names;
    
    // Add builtin presets first
    for (const auto& pair : m_builtinPresets)
    {
        names.push_back(pair.first);
    }
    
    // Then user presets
    for (const auto& pair : m_userPresets)
    {
        names.push_back(pair.first);
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
    
    m_currentPreset.clear();
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
    
    // Apply non-linear interpolation based on midpoint
    // mid < 0.5: shift towards start color
    // mid > 0.5: shift towards end color
    // mid == 0.5: linear (t unchanged)
    
    if (std::abs(mid - 0.5f) < 0.001f)
    {
        return t;
    }
    
    // Use power curve for smooth adjustment
    if (mid < 0.5f)
    {
        // Shift towards start
        float power = 0.5f / mid;
        return std::pow(t, power);
    }
    else
    {
        // Shift towards end
        float power = (1.0f - mid) / 0.5f;
        return 1.0f - std::pow(1.0f - t, power);
    }
}

} // namespace lumi::modules
