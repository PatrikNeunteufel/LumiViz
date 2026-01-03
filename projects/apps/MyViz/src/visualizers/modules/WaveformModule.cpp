/**
 ****************************************************************************************
 * @file   WaveformModule.cpp
 * @brief  Advanced waveform display module implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 3.0.0
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
    // Initialize channel configs with defaults
    // Mono: center
    m_channelConfigs[CHANNEL_MONO].lineOffset = 0.0f;
    m_channelConfigs[CHANNEL_MONO].amplitude = 0.8f;
    m_channelConfigs[CHANNEL_MONO].lineWidth = 2.0f;
    m_channelConfigs[CHANNEL_MONO].fillEnabled = false;
    m_channelConfigs[CHANNEL_MONO].fillOpacity = 0.3f;
    m_channelConfigs[CHANNEL_MONO].fillBrightness = -0.3f;
    m_channelConfigs[CHANNEL_MONO].visible = true;

    // Left: upper (for Stereo/Both modes)
    m_channelConfigs[CHANNEL_LEFT].lineOffset = 0.5f;
    m_channelConfigs[CHANNEL_LEFT].amplitude = 0.4f;
    m_channelConfigs[CHANNEL_LEFT].lineWidth = 2.0f;
    m_channelConfigs[CHANNEL_LEFT].fillEnabled = false;
    m_channelConfigs[CHANNEL_LEFT].fillOpacity = 0.3f;
    m_channelConfigs[CHANNEL_LEFT].fillBrightness = -0.3f;
    m_channelConfigs[CHANNEL_LEFT].visible = true;

    // Right: lower (for Stereo/Both modes)
    m_channelConfigs[CHANNEL_RIGHT].lineOffset = -0.5f;
    m_channelConfigs[CHANNEL_RIGHT].amplitude = 0.4f;
    m_channelConfigs[CHANNEL_RIGHT].lineWidth = 2.0f;
    m_channelConfigs[CHANNEL_RIGHT].fillEnabled = false;
    m_channelConfigs[CHANNEL_RIGHT].fillOpacity = 0.3f;
    m_channelConfigs[CHANNEL_RIGHT].fillBrightness = -0.3f;
    m_channelConfigs[CHANNEL_RIGHT].visible = true;

    // Initialize per-channel gradients with different default colors
    // Mono: Cyan
    m_colorGradients[CHANNEL_MONO].clearStops();
    m_colorGradients[CHANNEL_MONO].addStop(0.0f, {0.0f, 1.0f, 1.0f, 1.0f});  // Cyan
    m_colorGradients[CHANNEL_MONO].addStop(1.0f, {0.0f, 1.0f, 1.0f, 1.0f});  // Cyan
    m_colorGradients[CHANNEL_MONO].setMode(GradientMode::Solid);

    // Left: Green
    m_colorGradients[CHANNEL_LEFT].clearStops();
    m_colorGradients[CHANNEL_LEFT].addStop(0.0f, {0.2f, 1.0f, 0.4f, 1.0f});  // Green
    m_colorGradients[CHANNEL_LEFT].addStop(1.0f, {0.2f, 1.0f, 0.4f, 1.0f});  // Green
    m_colorGradients[CHANNEL_LEFT].setMode(GradientMode::Solid);

    // Right: Orange
    m_colorGradients[CHANNEL_RIGHT].clearStops();
    m_colorGradients[CHANNEL_RIGHT].addStop(0.0f, {1.0f, 0.5f, 0.2f, 1.0f});  // Orange
    m_colorGradients[CHANNEL_RIGHT].addStop(1.0f, {1.0f, 0.5f, 0.2f, 1.0f});  // Orange
    m_colorGradients[CHANNEL_RIGHT].setMode(GradientMode::Solid);
}

// =============================================================================
// Module Interface
// =============================================================================

void WaveformModule::reset()
{
    m_channelMode = WaveformChannelMode::Mono;

    // Reset channel configs - Mono
    m_channelConfigs[CHANNEL_MONO].lineOffset = 0.0f;
    m_channelConfigs[CHANNEL_MONO].amplitude = 0.8f;
    m_channelConfigs[CHANNEL_MONO].lineWidth = 2.0f;
    m_channelConfigs[CHANNEL_MONO].fillEnabled = false;
    m_channelConfigs[CHANNEL_MONO].fillOpacity = 0.3f;
    m_channelConfigs[CHANNEL_MONO].fillBrightness = -0.3f;
    m_channelConfigs[CHANNEL_MONO].visible = true;

    // Left
    m_channelConfigs[CHANNEL_LEFT].lineOffset = 0.5f;
    m_channelConfigs[CHANNEL_LEFT].amplitude = 0.4f;
    m_channelConfigs[CHANNEL_LEFT].lineWidth = 2.0f;
    m_channelConfigs[CHANNEL_LEFT].fillEnabled = false;
    m_channelConfigs[CHANNEL_LEFT].fillOpacity = 0.3f;
    m_channelConfigs[CHANNEL_LEFT].fillBrightness = -0.3f;
    m_channelConfigs[CHANNEL_LEFT].visible = true;

    // Right
    m_channelConfigs[CHANNEL_RIGHT].lineOffset = -0.5f;
    m_channelConfigs[CHANNEL_RIGHT].amplitude = 0.4f;
    m_channelConfigs[CHANNEL_RIGHT].lineWidth = 2.0f;
    m_channelConfigs[CHANNEL_RIGHT].fillEnabled = false;
    m_channelConfigs[CHANNEL_RIGHT].fillOpacity = 0.3f;
    m_channelConfigs[CHANNEL_RIGHT].fillBrightness = -0.3f;
    m_channelConfigs[CHANNEL_RIGHT].visible = true;

    // Global settings
    m_lineStyle = WaveformLineStyle::Line;
    m_dashLength = 10.0f;
    m_dashGap = 5.0f;
    m_sampleCount = 512;
    m_smoothing = 0.3f;
    m_displayWidth = 1.0f;

    // Effects
    m_mirrorEnabled = false;
    m_holdEnabled = false;
    m_fadeTime = 1.0f;
    m_maxHoldFrames = 60;

    // Reset per-channel gradients
    // Mono: Cyan
    m_colorGradients[CHANNEL_MONO].reset();
    m_colorGradients[CHANNEL_MONO].clearStops();
    m_colorGradients[CHANNEL_MONO].addStop(0.0f, {0.0f, 1.0f, 1.0f, 1.0f});
    m_colorGradients[CHANNEL_MONO].addStop(1.0f, {0.0f, 1.0f, 1.0f, 1.0f});
    m_colorGradients[CHANNEL_MONO].setMode(GradientMode::Solid);

    // Left: Green
    m_colorGradients[CHANNEL_LEFT].reset();
    m_colorGradients[CHANNEL_LEFT].clearStops();
    m_colorGradients[CHANNEL_LEFT].addStop(0.0f, {0.2f, 1.0f, 0.4f, 1.0f});
    m_colorGradients[CHANNEL_LEFT].addStop(1.0f, {0.2f, 1.0f, 0.4f, 1.0f});
    m_colorGradients[CHANNEL_LEFT].setMode(GradientMode::Solid);

    // Right: Orange
    m_colorGradients[CHANNEL_RIGHT].reset();
    m_colorGradients[CHANNEL_RIGHT].clearStops();
    m_colorGradients[CHANNEL_RIGHT].addStop(0.0f, {1.0f, 0.5f, 0.2f, 1.0f});
    m_colorGradients[CHANNEL_RIGHT].addStop(1.0f, {1.0f, 0.5f, 0.2f, 1.0f});
    m_colorGradients[CHANNEL_RIGHT].setMode(GradientMode::Solid);
}

// =============================================================================
// Channel Settings
// =============================================================================

void WaveformModule::setChannelMode(WaveformChannelMode mode)
{
    m_channelMode = mode;
}

WaveformChannelConfig& WaveformModule::channelConfig(int channel)
{
    return m_channelConfigs[std::clamp(channel, 0, 2)];
}

const WaveformChannelConfig& WaveformModule::channelConfig(int channel) const
{
    return m_channelConfigs[std::clamp(channel, 0, 2)];
}

ColorGradientModule& WaveformModule::colorGradient(int channel)
{
    return m_colorGradients[std::clamp(channel, 0, 2)];
}

const ColorGradientModule& WaveformModule::colorGradient(int channel) const
{
    return m_colorGradients[std::clamp(channel, 0, 2)];
}

// =============================================================================
// Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> WaveformModule::paramDescs() const
{
    std::vector<ModuleParamDesc> params;
    int order = 0;

    // =========================================================================
    // Channel Mode (first, determines which params are visible)
    // =========================================================================

    {
        ModuleParamDesc p;
        p.id = "channelMode";
        p.displayName = "Channel Mode";
        p.tooltip = "Which audio channels to display";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Mono", "Stereo", "Both"};
        p.subGroup = "Channel";
        p.order = order++;
        params.push_back(p);
    }

    // =========================================================================
    // Layout SubGroup - Per Channel
    // =========================================================================

    // Mono settings (visible in Mono and Both modes)
    {
        ModuleParamDesc p;
        p.id = "monoOffset";
        p.displayName = "Mono Offset";
        p.tooltip = "Vertical offset for mono channel";
        p.type = ParamType::Float;
        p.minValue = -1.0f;
        p.maxValue = 1.0f;
        p.defaultValue = 0.0f;
        p.subGroup = "Layout";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {0, 2};  // Mono, Both
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "monoAmplitude";
        p.displayName = "Mono Amplitude";
        p.tooltip = "Amplitude for mono channel";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 2.0f;
        p.defaultValue = 0.8f;
        p.subGroup = "Layout";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {0, 2};  // Mono, Both
        params.push_back(p);
    }

    // Left channel settings (visible in Stereo and Both modes)
    {
        ModuleParamDesc p;
        p.id = "leftOffset";
        p.displayName = "Left Offset";
        p.tooltip = "Vertical offset for left channel";
        p.type = ParamType::Float;
        p.minValue = -1.0f;
        p.maxValue = 1.0f;
        p.defaultValue = 0.5f;
        p.subGroup = "Layout";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {1, 2};  // Stereo, Both
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "leftAmplitude";
        p.displayName = "Left Amplitude";
        p.tooltip = "Amplitude for left channel";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 2.0f;
        p.defaultValue = 0.4f;
        p.subGroup = "Layout";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {1, 2};  // Stereo, Both
        params.push_back(p);
    }

    // Right channel settings (visible in Stereo and Both modes)
    {
        ModuleParamDesc p;
        p.id = "rightOffset";
        p.displayName = "Right Offset";
        p.tooltip = "Vertical offset for right channel";
        p.type = ParamType::Float;
        p.minValue = -1.0f;
        p.maxValue = 1.0f;
        p.defaultValue = -0.5f;
        p.subGroup = "Layout";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {1, 2};  // Stereo, Both
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "rightAmplitude";
        p.displayName = "Right Amplitude";
        p.tooltip = "Amplitude for right channel";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 2.0f;
        p.defaultValue = 0.4f;
        p.subGroup = "Layout";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {1, 2};  // Stereo, Both
        params.push_back(p);
    }

    // Global layout settings
    {
        ModuleParamDesc p;
        p.id = "displayWidth";
        p.displayName = "Display Width";
        p.tooltip = "Width as fraction of viewport";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 1.0f;
        p.defaultValue = 1.0f;
        p.subGroup = "Layout";
        p.order = order++;
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "sampleCount";
        p.displayName = "Sample Count";
        p.tooltip = "Number of samples to display";
        p.type = ParamType::Int;
        p.minValue = 64.0f;
        p.maxValue = 2048.0f;
        p.defaultValue = 512;
        p.step = 1.0f;
        p.widget = ParamWidget::Spinbox;  // Force spinbox
        p.subGroup = "Layout";
        p.order = order++;
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
        p.subGroup = "Layout";
        p.order = order++;
        params.push_back(p);
    }

    // =========================================================================
    // Line Style SubGroup
    // =========================================================================

    {
        ModuleParamDesc p;
        p.id = "lineStyle";
        p.displayName = "Style";
        p.tooltip = "Line drawing style";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Line", "Dots", "Dashed"};
        p.subGroup = "Line";
        p.order = order++;
        params.push_back(p);
    }

    // Per-channel line width
    {
        ModuleParamDesc p;
        p.id = "monoLineWidth";
        p.displayName = "Mono Line Width";
        p.tooltip = "Line width for mono channel";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 10.0f;
        p.defaultValue = 2.0f;
        p.subGroup = "Line";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {0, 2};  // Mono, Both
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "leftLineWidth";
        p.displayName = "Left Line Width";
        p.tooltip = "Line width for left channel";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 10.0f;
        p.defaultValue = 2.0f;
        p.subGroup = "Line";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {1, 2};  // Stereo, Both
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "rightLineWidth";
        p.displayName = "Right Line Width";
        p.tooltip = "Line width for right channel";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 10.0f;
        p.defaultValue = 2.0f;
        p.subGroup = "Line";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {1, 2};  // Stereo, Both
        params.push_back(p);
    }

    // Dash settings (global, conditional on style)
    {
        ModuleParamDesc p;
        p.id = "dashLength";
        p.displayName = "Dash Length";
        p.tooltip = "Length of dashes in pixels";
        p.type = ParamType::Float;
        p.minValue = 2.0f;
        p.maxValue = 50.0f;
        p.defaultValue = 10.0f;
        p.subGroup = "Line";
        p.order = order++;
        p.dependsOn = "lineStyle";
        p.dependsValues = {2};  // Dashed
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "dashGap";
        p.displayName = "Dash Gap";
        p.tooltip = "Gap between dashes in pixels";
        p.type = ParamType::Float;
        p.minValue = 1.0f;
        p.maxValue = 50.0f;
        p.defaultValue = 5.0f;
        p.subGroup = "Line";
        p.order = order++;
        p.dependsOn = "lineStyle";
        p.dependsValues = {2};  // Dashed
        params.push_back(p);
    }

    // =========================================================================
    // Fill SubGroup - Per Channel (Mono, Left, Right order)
    // =========================================================================

    // --- MONO FILL ---
    {
        ModuleParamDesc p;
        p.id = "monoFillEnabled";
        p.displayName = "Mono Fill";
        p.tooltip = "Fill mono channel to zero line";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {0, 2};  // Mono, Both
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "monoFillOpacity";
        p.displayName = "Mono Fill Opacity";
        p.tooltip = "Fill opacity for mono channel";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.defaultValue = 0.3f;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "monoFillEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "monoFillBrightness";
        p.displayName = "Mono Fill Brightness";
        p.tooltip = "Brightness relative to line";
        p.type = ParamType::Float;
        p.minValue = -1.0f;
        p.maxValue = 1.0f;
        p.defaultValue = -0.3f;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "monoFillEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // --- LEFT FILL ---
    {
        ModuleParamDesc p;
        p.id = "leftFillEnabled";
        p.displayName = "Left Fill";
        p.tooltip = "Fill left channel to zero line";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {1, 2};  // Stereo, Both
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "leftFillOpacity";
        p.displayName = "Left Fill Opacity";
        p.tooltip = "Fill opacity for left channel";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.defaultValue = 0.3f;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "leftFillEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "leftFillBrightness";
        p.displayName = "Left Fill Brightness";
        p.tooltip = "Brightness relative to line";
        p.type = ParamType::Float;
        p.minValue = -1.0f;
        p.maxValue = 1.0f;
        p.defaultValue = -0.3f;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "leftFillEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // --- RIGHT FILL ---
    {
        ModuleParamDesc p;
        p.id = "rightFillEnabled";
        p.displayName = "Right Fill";
        p.tooltip = "Fill right channel to zero line";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "channelMode";
        p.dependsValues = {1, 2};  // Stereo, Both
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "rightFillOpacity";
        p.displayName = "Right Fill Opacity";
        p.tooltip = "Fill opacity for right channel";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.defaultValue = 0.3f;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "rightFillEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "rightFillBrightness";
        p.displayName = "Right Fill Brightness";
        p.tooltip = "Brightness relative to line";
        p.type = ParamType::Float;
        p.minValue = -1.0f;
        p.maxValue = 1.0f;
        p.defaultValue = -0.3f;
        p.subGroup = "Fill";
        p.order = order++;
        p.dependsOn = "rightFillEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // =========================================================================
    // Effects SubGroup (Global)
    // =========================================================================

    {
        ModuleParamDesc p;
        p.id = "mirrorEnabled";
        p.displayName = "Mirror";
        p.tooltip = "Mirror waveform at zero line";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.subGroup = "Effects";
        p.order = order++;
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "holdEnabled";
        p.displayName = "Hold/Fade";
        p.tooltip = "Enable persistence effect (trails)";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.subGroup = "Effects";
        p.order = order++;
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "fadeTime";
        p.displayName = "Fade Time";
        p.tooltip = "Time for trails to fade out (seconds)";
        p.type = ParamType::Float;
        p.minValue = 0.1f;
        p.maxValue = 5.0f;
        p.defaultValue = 1.0f;
        p.subGroup = "Effects";
        p.order = order++;
        p.dependsOn = "holdEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    {
        ModuleParamDesc p;
        p.id = "maxHoldFrames";
        p.displayName = "Max Frames";
        p.tooltip = "Maximum number of held frames";
        p.type = ParamType::Int;
        p.minValue = 1.0f;
        p.maxValue = 120.0f;
        p.defaultValue = 60;
        p.step = 1.0f;
        p.widget = ParamWidget::Spinbox;  // Force spinbox
        p.subGroup = "Effects";
        p.order = order++;
        p.dependsOn = "holdEnabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // =========================================================================
    // Color SubGroups - Per Channel (only show when channel is active)
    // =========================================================================

    // Mono color (visible in Mono and Both modes)
    for (const auto& p : m_colorGradients[CHANNEL_MONO].paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "monoColor." + p.id;
        prefixed.displayName = p.displayName;  // No prefix - subGroup name has it
        prefixed.subGroup = "Line Color Mono";
        prefixed.order = 200 + p.order;
        
        // First-level params (no dependsOn) depend on channelMode
        // Nested params (with dependsOn) keep their dependency but prefixed
        if (p.dependsOn.empty())
        {
            prefixed.dependsOn = "channelMode";
            prefixed.dependsValues = {0, 2};  // Mono, Both
        }
        else
        {
            prefixed.dependsOn = "monoColor." + p.dependsOn;
            prefixed.dependsValues = p.dependsValues;
        }

        params.push_back(prefixed);
    }

    // Left color (visible in Stereo and Both modes)
    for (const auto& p : m_colorGradients[CHANNEL_LEFT].paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "leftColor." + p.id;
        prefixed.displayName = p.displayName;  // No prefix
        prefixed.subGroup = "Line Color Left";
        prefixed.order = 300 + p.order;
        
        if (p.dependsOn.empty())
        {
            prefixed.dependsOn = "channelMode";
            prefixed.dependsValues = {1, 2};  // Stereo, Both
        }
        else
        {
            prefixed.dependsOn = "leftColor." + p.dependsOn;
            prefixed.dependsValues = p.dependsValues;
        }

        params.push_back(prefixed);
    }

    // Right color (visible in Stereo and Both modes)
    for (const auto& p : m_colorGradients[CHANNEL_RIGHT].paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "rightColor." + p.id;
        prefixed.displayName = p.displayName;  // No prefix
        prefixed.subGroup = "Line Color Right";
        prefixed.order = 400 + p.order;
        
        if (p.dependsOn.empty())
        {
            prefixed.dependsOn = "channelMode";
            prefixed.dependsValues = {1, 2};  // Stereo, Both
        }
        else
        {
            prefixed.dependsOn = "rightColor." + p.dependsOn;
            prefixed.dependsValues = p.dependsValues;
        }

        params.push_back(prefixed);
    }

    return params;
}

bool WaveformModule::getParam(const std::string& id, ParamValue& out) const
{
    // Per-channel color gradient parameters
    if (id.rfind("monoColor.", 0) == 0)
    {
        return m_colorGradients[CHANNEL_MONO].getParam(id.substr(10), out);
    }
    if (id.rfind("leftColor.", 0) == 0)
    {
        return m_colorGradients[CHANNEL_LEFT].getParam(id.substr(10), out);
    }
    if (id.rfind("rightColor.", 0) == 0)
    {
        return m_colorGradients[CHANNEL_RIGHT].getParam(id.substr(11), out);
    }
    
    // Legacy: color.* maps to mono gradient for compatibility
    if (id.rfind("color.", 0) == 0)
    {
        return m_colorGradients[CHANNEL_MONO].getParam(id.substr(6), out);
    }

    // Channel mode
    if (id == "channelMode")
    {
        out = static_cast<int>(m_channelMode);
        return true;
    }

    // Mono channel
    if (id == "monoOffset")
    {
        out = m_channelConfigs[CHANNEL_MONO].lineOffset;
        return true;
    }
    if (id == "monoAmplitude")
    {
        out = m_channelConfigs[CHANNEL_MONO].amplitude;
        return true;
    }
    if (id == "monoLineWidth")
    {
        out = m_channelConfigs[CHANNEL_MONO].lineWidth;
        return true;
    }
    if (id == "monoFillEnabled")
    {
        out = m_channelConfigs[CHANNEL_MONO].fillEnabled;
        return true;
    }
    if (id == "monoFillOpacity")
    {
        out = m_channelConfigs[CHANNEL_MONO].fillOpacity;
        return true;
    }
    if (id == "monoFillBrightness")
    {
        out = m_channelConfigs[CHANNEL_MONO].fillBrightness;
        return true;
    }

    // Left channel
    if (id == "leftOffset")
    {
        out = m_channelConfigs[CHANNEL_LEFT].lineOffset;
        return true;
    }
    if (id == "leftAmplitude")
    {
        out = m_channelConfigs[CHANNEL_LEFT].amplitude;
        return true;
    }
    if (id == "leftLineWidth")
    {
        out = m_channelConfigs[CHANNEL_LEFT].lineWidth;
        return true;
    }
    if (id == "leftFillEnabled")
    {
        out = m_channelConfigs[CHANNEL_LEFT].fillEnabled;
        return true;
    }
    if (id == "leftFillOpacity")
    {
        out = m_channelConfigs[CHANNEL_LEFT].fillOpacity;
        return true;
    }
    if (id == "leftFillBrightness")
    {
        out = m_channelConfigs[CHANNEL_LEFT].fillBrightness;
        return true;
    }

    // Right channel
    if (id == "rightOffset")
    {
        out = m_channelConfigs[CHANNEL_RIGHT].lineOffset;
        return true;
    }
    if (id == "rightAmplitude")
    {
        out = m_channelConfigs[CHANNEL_RIGHT].amplitude;
        return true;
    }
    if (id == "rightLineWidth")
    {
        out = m_channelConfigs[CHANNEL_RIGHT].lineWidth;
        return true;
    }
    if (id == "rightFillEnabled")
    {
        out = m_channelConfigs[CHANNEL_RIGHT].fillEnabled;
        return true;
    }
    if (id == "rightFillOpacity")
    {
        out = m_channelConfigs[CHANNEL_RIGHT].fillOpacity;
        return true;
    }
    if (id == "rightFillBrightness")
    {
        out = m_channelConfigs[CHANNEL_RIGHT].fillBrightness;
        return true;
    }

    // Global layout
    if (id == "displayWidth")
    {
        out = m_displayWidth;
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

    // Line style
    if (id == "lineStyle")
    {
        out = static_cast<int>(m_lineStyle);
        return true;
    }
    if (id == "dashLength")
    {
        out = m_dashLength;
        return true;
    }
    if (id == "dashGap")
    {
        out = m_dashGap;
        return true;
    }

    // Fill brightness (global)
    if (id == "fillBrightness")
    {
        out = m_channelConfigs[CHANNEL_MONO].fillBrightness;
        return true;
    }

    // Effects
    if (id == "mirrorEnabled")
    {
        out = m_mirrorEnabled;
        return true;
    }
    if (id == "holdEnabled")
    {
        out = m_holdEnabled;
        return true;
    }
    if (id == "fadeTime")
    {
        out = m_fadeTime;
        return true;
    }
    if (id == "maxHoldFrames")
    {
        out = m_maxHoldFrames;
        return true;
    }

    return false;
}

bool WaveformModule::setParam(const std::string& id, const ParamValue& value)
{
    // Per-channel color gradient parameters
    if (id.rfind("monoColor.", 0) == 0)
    {
        return m_colorGradients[CHANNEL_MONO].setParam(id.substr(10), value);
    }
    if (id.rfind("leftColor.", 0) == 0)
    {
        return m_colorGradients[CHANNEL_LEFT].setParam(id.substr(10), value);
    }
    if (id.rfind("rightColor.", 0) == 0)
    {
        return m_colorGradients[CHANNEL_RIGHT].setParam(id.substr(11), value);
    }
    
    // Legacy: color.* maps to mono gradient for compatibility
    if (id.rfind("color.", 0) == 0)
    {
        return m_colorGradients[CHANNEL_MONO].setParam(id.substr(6), value);
    }

    // Channel mode
    if (id == "channelMode")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_channelMode = static_cast<WaveformChannelMode>(std::clamp(*v, 0, 2));
            return true;
        }
    }

    // Mono channel
    if (id == "monoOffset")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_MONO].lineOffset = std::clamp(*v, -1.0f, 1.0f);
            return true;
        }
    }
    if (id == "monoAmplitude")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_MONO].amplitude = std::clamp(*v, 0.1f, 2.0f);
            return true;
        }
    }
    if (id == "monoLineWidth")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_MONO].lineWidth = std::clamp(*v, 1.0f, 10.0f);
            return true;
        }
    }
    if (id == "monoFillEnabled")
    {
        if (auto* v = std::get_if<bool>(&value))
        {
            m_channelConfigs[CHANNEL_MONO].fillEnabled = *v;
            return true;
        }
    }
    if (id == "monoFillOpacity")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_MONO].fillOpacity = std::clamp(*v, 0.0f, 1.0f);
            return true;
        }
    }
    if (id == "monoFillBrightness")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_MONO].fillBrightness = std::clamp(*v, -1.0f, 1.0f);
            return true;
        }
    }

    // Left channel
    if (id == "leftOffset")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_LEFT].lineOffset = std::clamp(*v, -1.0f, 1.0f);
            return true;
        }
    }
    if (id == "leftAmplitude")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_LEFT].amplitude = std::clamp(*v, 0.1f, 2.0f);
            return true;
        }
    }
    if (id == "leftLineWidth")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_LEFT].lineWidth = std::clamp(*v, 1.0f, 10.0f);
            return true;
        }
    }
    if (id == "leftFillEnabled")
    {
        if (auto* v = std::get_if<bool>(&value))
        {
            m_channelConfigs[CHANNEL_LEFT].fillEnabled = *v;
            return true;
        }
    }
    if (id == "leftFillOpacity")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_LEFT].fillOpacity = std::clamp(*v, 0.0f, 1.0f);
            return true;
        }
    }
    if (id == "leftFillBrightness")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_LEFT].fillBrightness = std::clamp(*v, -1.0f, 1.0f);
            return true;
        }
    }

    // Right channel
    if (id == "rightOffset")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_RIGHT].lineOffset = std::clamp(*v, -1.0f, 1.0f);
            return true;
        }
    }
    if (id == "rightAmplitude")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_RIGHT].amplitude = std::clamp(*v, 0.1f, 2.0f);
            return true;
        }
    }
    if (id == "rightLineWidth")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_RIGHT].lineWidth = std::clamp(*v, 1.0f, 10.0f);
            return true;
        }
    }
    if (id == "rightFillEnabled")
    {
        if (auto* v = std::get_if<bool>(&value))
        {
            m_channelConfigs[CHANNEL_RIGHT].fillEnabled = *v;
            return true;
        }
    }
    if (id == "rightFillOpacity")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_RIGHT].fillOpacity = std::clamp(*v, 0.0f, 1.0f);
            return true;
        }
    }
    if (id == "rightFillBrightness")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_channelConfigs[CHANNEL_RIGHT].fillBrightness = std::clamp(*v, -1.0f, 1.0f);
            return true;
        }
    }

    // Global layout
    if (id == "displayWidth")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_displayWidth = std::clamp(*v, 0.1f, 1.0f);
            return true;
        }
    }
    if (id == "sampleCount")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_sampleCount = std::clamp(*v, 64, 2048);
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

    // Line style
    if (id == "lineStyle")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_lineStyle = static_cast<WaveformLineStyle>(std::clamp(*v, 0, 2));
            return true;
        }
    }
    if (id == "dashLength")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_dashLength = std::clamp(*v, 2.0f, 50.0f);
            return true;
        }
    }
    if (id == "dashGap")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_dashGap = std::clamp(*v, 1.0f, 50.0f);
            return true;
        }
    }

    // Fill brightness (apply to all channels)
    if (id == "fillBrightness")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            float brightness = std::clamp(*v, -1.0f, 1.0f);
            m_channelConfigs[CHANNEL_MONO].fillBrightness = brightness;
            m_channelConfigs[CHANNEL_LEFT].fillBrightness = brightness;
            m_channelConfigs[CHANNEL_RIGHT].fillBrightness = brightness;
            return true;
        }
    }

    // Effects
    if (id == "mirrorEnabled")
    {
        if (auto* v = std::get_if<bool>(&value))
        {
            m_mirrorEnabled = *v;
            return true;
        }
    }
    if (id == "holdEnabled")
    {
        if (auto* v = std::get_if<bool>(&value))
        {
            m_holdEnabled = *v;
            return true;
        }
    }
    if (id == "fadeTime")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_fadeTime = std::clamp(*v, 0.1f, 5.0f);
            return true;
        }
    }
    if (id == "maxHoldFrames")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_maxHoldFrames = std::clamp(*v, 1, 120);
            return true;
        }
    }

    return false;
}

// =============================================================================
// Utility
// =============================================================================

const char* WaveformModule::lineStyleName(WaveformLineStyle style)
{
    switch (style)
    {
        case WaveformLineStyle::Line:   return "Line";
        case WaveformLineStyle::Dots:   return "Dots";
        case WaveformLineStyle::Dashed: return "Dashed";
        default:                        return "Unknown";
    }
}

const char* WaveformModule::channelModeName(WaveformChannelMode mode)
{
    switch (mode)
    {
        case WaveformChannelMode::Mono:   return "Mono";
        case WaveformChannelMode::Stereo: return "Stereo";
        case WaveformChannelMode::Both:   return "Both";
        default:                          return "Unknown";
    }
}

} // namespace lumi::modules
