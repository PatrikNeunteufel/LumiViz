/**
 ****************************************************************************************
 * @file   WaveformModule.cpp
 * @brief  Waveform display module implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/WaveformModule.hpp"

#include <algorithm>

namespace lumi::modules {

// =============================================================================
// Construction
// =============================================================================

WaveformModule::WaveformModule()
{
    // Set default gradient (cyan to magenta)
    m_colorGradient.clearStops();
    m_colorGradient.addStop(0.0f, {0.0f, 1.0f, 1.0f, 1.0f});   // Cyan
    m_colorGradient.addStop(1.0f, {1.0f, 0.0f, 1.0f, 1.0f});   // Magenta
    m_colorGradient.setMode(GradientMode::Linear);
}

// =============================================================================
// Module Interface
// =============================================================================

void WaveformModule::reset()
{
    m_style = WaveformStyle::Line;
    m_orientation = WaveformOrientation::Horizontal;
    m_lineWidth = 2.0f;
    m_amplitude = 0.8f;
    m_sampleCount = 256;
    m_smoothing = 0.3f;
    m_glowEnabled = true;
    m_glowIntensity = 0.5f;
    m_mirrorGap = 0.05f;

    // Reset gradient to default
    m_colorGradient.reset();
    m_colorGradient.clearStops();
    m_colorGradient.addStop(0.0f, {0.0f, 1.0f, 1.0f, 1.0f});
    m_colorGradient.addStop(1.0f, {1.0f, 0.0f, 1.0f, 1.0f});
    m_colorGradient.setMode(GradientMode::Linear);
}

// =============================================================================
// Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> WaveformModule::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    // -------------------------------------------------------------------------
    // Style Parameters
    // -------------------------------------------------------------------------

    {
        ModuleParamDesc p;
        p.id = "style";
        p.displayName = "Style";
        p.tooltip = "Waveform display style";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Line", "Bars", "Mirror", "Filled", "Dots"};
        p.order = 0;
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "lineWidth";
        p.displayName = "Line Width";
        p.tooltip = "Width of the waveform line in pixels";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 10.0f;
        p.defaultValue = 2.0f;
        p.order = 1;
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "amplitude";
        p.displayName = "Amplitude";
        p.tooltip = "Vertical scale of the waveform";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 2.0f;
        p.defaultValue = 0.8f;
        p.order = 2;
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "sampleCount";
        p.displayName = "Sample Count";
        p.tooltip = "Number of samples to display";
        p.type = ParamType::Int;
        p.minValue = 64.0f;
        p.maxValue = 1024.0f;
        p.defaultValue = 256;
        p.order = 3;
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "smoothing";
        p.displayName = "Smoothing";
        p.tooltip = "Temporal smoothing factor";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 0.95f;
        p.defaultValue = 0.3f;
        p.order = 4;
        params.push_back(p);
    }

    // -------------------------------------------------------------------------
    // Mirror Mode Parameters (conditional)
    // -------------------------------------------------------------------------

    {
        ModuleParamDesc p;
        p.id = "mirrorGap";
        p.displayName = "Mirror Gap";
        p.tooltip = "Gap between mirrored halves";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 0.5f;
        p.defaultValue = 0.05f;
        p.order = 5;
        p.dependsOn = "style";
        p.dependsValues = {2};  // Mirror = 2
        params.push_back(p);
    }

    // -------------------------------------------------------------------------
    // Glow Effect
    // -------------------------------------------------------------------------

    {
        ModuleParamDesc p;
        p.id = "glowEnabled";
        p.displayName = "Glow Enabled";
        p.tooltip = "Add glow effect to waveform";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.subGroup = "Glow";
        p.order = 10;
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "glowIntensity";
        p.displayName = "Glow Intensity";
        p.tooltip = "Intensity of the glow effect";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.defaultValue = 0.5f;
        p.subGroup = "Glow";
        p.order = 11;
        p.dependsOn = "glowEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // -------------------------------------------------------------------------
    // Color SubGroup (from ColorGradientModule)
    // -------------------------------------------------------------------------

    for (const auto& p : m_colorGradient.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "color." + p.id;
        prefixed.subGroup = "Color";
        prefixed.order = 20 + p.order;

        // Prefix dependsOn reference
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "color." + prefixed.dependsOn;
        }

        params.push_back(prefixed);
    }

    return params;
}

bool WaveformModule::getParam(const std::string& id, ParamValue& out) const
{
    // Color module parameters
    if (id.rfind("color.", 0) == 0)
    {
        return m_colorGradient.getParam(id.substr(6), out);
    }

    // Waveform-specific parameters
    if (id == "style")
    {
        out = static_cast<int>(m_style);
        return true;
    }
    if (id == "lineWidth")
    {
        out = m_lineWidth;
        return true;
    }
    if (id == "amplitude")
    {
        out = m_amplitude;
        return true;
    }
    if (id == "sampleCount")
    {
        out = m_sampleCount;
        return true;
    }
    if (id == "smoothing")
    {
        out = m_smoothing;
        return true;
    }
    if (id == "mirrorGap")
    {
        out = m_mirrorGap;
        return true;
    }
    if (id == "glowEnabled")
    {
        out = m_glowEnabled;
        return true;
    }
    if (id == "glowIntensity")
    {
        out = m_glowIntensity;
        return true;
    }

    return false;
}

bool WaveformModule::setParam(const std::string& id, const ParamValue& value)
{
    // Color module parameters
    if (id.rfind("color.", 0) == 0)
    {
        return m_colorGradient.setParam(id.substr(6), value);
    }

    // Waveform-specific parameters
    if (id == "style")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_style = static_cast<WaveformStyle>(std::clamp(*v, 0, 4));
            return true;
        }
    }
    if (id == "lineWidth")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_lineWidth = std::clamp(*v, 1.0f, 10.0f);
            return true;
        }
    }
    if (id == "amplitude")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_amplitude = std::clamp(*v, 0.1f, 2.0f);
            return true;
        }
    }
    if (id == "sampleCount")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_sampleCount = std::clamp(*v, 64, 1024);
            return true;
        }
    }
    if (id == "smoothing")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_smoothing = std::clamp(*v, 0.0f, 0.95f);
            return true;
        }
    }
    if (id == "mirrorGap")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_mirrorGap = std::clamp(*v, 0.0f, 0.5f);
            return true;
        }
    }
    if (id == "glowEnabled")
    {
        if (auto* v = std::get_if<bool>(&value))
        {
            m_glowEnabled = *v;
            return true;
        }
    }
    if (id == "glowIntensity")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_glowIntensity = std::clamp(*v, 0.0f, 1.0f);
            return true;
        }
    }

    return false;
}

// =============================================================================
// Utility
// =============================================================================

const char* WaveformModule::styleName(WaveformStyle style)
{
    switch (style)
    {
        case WaveformStyle::Line:   return "Line";
        case WaveformStyle::Bars:   return "Bars";
        case WaveformStyle::Mirror: return "Mirror";
        case WaveformStyle::Filled: return "Filled";
        case WaveformStyle::Dots:   return "Dots";
        default:                    return "Unknown";
    }
}

std::vector<const char*> WaveformModule::availableStyles()
{
    return {"Line", "Bars", "Mirror", "Filled", "Dots"};
}

} // namespace lumi::modules
