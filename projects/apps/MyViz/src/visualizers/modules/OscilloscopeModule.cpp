/**
 ****************************************************************************************
 * @file   OscilloscopeModule.cpp
 * @brief  Professional oscilloscope with 4 signal + 2 math channels
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 2.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/OscilloscopeModule.hpp"

#include <BasicLogger.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

namespace lumi::modules {

// =============================================================================
// Construction
// =============================================================================

OscilloscopeModule::OscilloscopeModule()
{
    reset();
}

void OscilloscopeModule::reset()
{
    m_timePerDiv = 10.0f;
    m_sampleCount = 512;
    m_triggerEnabled = true;
    m_triggerLevel = 0.0f;
    m_triggerTolerance = 0.1f;
    m_triggerPosition = 0.5f;
    m_triggerEdge = TriggerEdge::Rising;
    m_triggerMode = TriggerMode::Auto;
    m_triggerIndicatorStyle = TriggerIndicatorStyle::Crosshair;
    m_triggerChannel = CH1;
    m_triggerHoldoff = 0.0f;
    m_triggerFadeTime = 2.0f;

    // Signal channels
    for (int i = 0; i < SIGNAL_CHANNELS; ++i)
    {
        m_signalChannels[i] = SignalChannelConfig{};
        m_signalChannels[i].visible = (i == 0);
        m_acCouplingStates[i].reset();
    }
    m_signalChannels[0].source = SignalSource::Left;
    m_signalChannels[1].source = SignalSource::Right;
    m_signalChannels[2].source = SignalSource::Mid;
    m_signalChannels[3].source = SignalSource::Side;

    // Math channels
    for (int i = 0; i < MATH_CHANNELS; ++i)
    {
        m_mathChannels[i] = MathChannelConfig{};
        m_mathChannels[i].visible = false;
    }
    m_mathChannels[0].operation = MathOperation::Add;
    m_mathChannels[1].operation = MathOperation::Subtract;

    // Grid & Display
    m_gridStyle = GridStyle::Lines;
    m_gridColor = {0.3f, 0.3f, 0.3f, 1.0f};
    m_gridBrightness = 1.0f;
    m_gridLineWidth = 1.0f;
    m_gridDotSize = 2.0f;
    m_gridCrossSize = 5.0f;
    m_bgColorR = 0.02f;
    m_bgColorG = 0.05f;
    m_bgColorB = 0.02f;
    m_interpolation = true;

    // Colors: CH1=Yellow, CH2=Cyan, CH3=Magenta, CH4=Green, M1=Orange, M2=White
    Color4f colors[TOTAL_CHANNELS] = {
        {1.0f, 1.0f, 0.2f, 1.0f}, {0.2f, 1.0f, 1.0f, 1.0f},
        {1.0f, 0.2f, 1.0f, 1.0f}, {0.2f, 1.0f, 0.2f, 1.0f},
        {1.0f, 0.6f, 0.2f, 1.0f}, {0.9f, 0.9f, 0.9f, 1.0f}
    };
    for (int i = 0; i < TOTAL_CHANNELS; ++i)
    {
        m_colorGradients[i].setMode(GradientMode::Solid);
        m_colorGradients[i].setSolidColor(colors[i]);
    }
}

// =============================================================================
// Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> OscilloscopeModule::paramDescs() const
{
    std::vector<ModuleParamDesc> params;
    int order = 0;

    // Timebase
    {
        ModuleParamDesc p; p.id = "timePerDiv"; p.displayName = "Time/Div";
        p.tooltip = "Milliseconds per division"; p.type = ParamType::Float;
        p.minValue = 0.1f; p.maxValue = 100.0f; p.defaultValue = 10.0f;
        p.subGroup = "Timebase"; p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "sampleCount"; p.displayName = "Samples";
        p.tooltip = "Display resolution (number of points drawn). Higher = smoother line, Lower = faster rendering";
        p.type = ParamType::Int; p.minValue = 32; p.maxValue = 8192;
        p.defaultValue = 512; p.subGroup = "Timebase"; p.order = order++;
        params.push_back(p);
    }

    // Trigger
    {
        ModuleParamDesc p; p.id = "triggerEnabled"; p.displayName = "Trigger";
        p.type = ParamType::Bool; p.defaultValue = true;
        p.subGroup = "Trigger"; p.order = order++;
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "triggerLevel"; p.displayName = "Level";
        p.type = ParamType::Float; p.minValue = -1.0f; p.maxValue = 1.0f;
        p.defaultValue = 0.0f; p.subGroup = "Trigger"; p.order = order++;
        p.dependsOn = "triggerEnabled"; p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "triggerTolerance"; p.displayName = "Tolerance";
        p.tooltip = "Trigger tolerance in divisions (0 = exact, higher = more lenient)";
        p.type = ParamType::Float; p.minValue = 0.0f; p.maxValue = 2.0f;
        p.defaultValue = 0.1f; p.subGroup = "Trigger"; p.order = order++;
        p.dependsOn = "triggerEnabled"; p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "triggerPosition"; p.displayName = "Position";
        p.tooltip = "Horizontal trigger position (0=left, 1=right)";
        p.type = ParamType::Float; p.minValue = 0.0f; p.maxValue = 1.0f;
        p.defaultValue = 0.5f; p.subGroup = "Trigger"; p.order = order++;
        p.dependsOn = "triggerEnabled"; p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "triggerEdge"; p.displayName = "Edge";
        p.type = ParamType::Enum; p.defaultValue = 0;
        p.enumOptions = {"Rising", "Falling", "Both"};
        p.subGroup = "Trigger"; p.order = order++;
        p.dependsOn = "triggerEnabled"; p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "triggerMode"; p.displayName = "Mode";
        p.type = ParamType::Enum; p.defaultValue = 0;
        p.enumOptions = {"Auto", "Normal", "Single"};
        p.subGroup = "Trigger"; p.order = order++;
        p.dependsOn = "triggerEnabled"; p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "triggerIndicator"; p.displayName = "Indicator";
        p.tooltip = "Trigger indicator display style";
        p.type = ParamType::Enum; p.defaultValue = 1;
        p.enumOptions = {"Arrows", "Crosshair"};
        p.subGroup = "Trigger"; p.order = order++;
        p.dependsOn = "triggerEnabled"; p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "triggerChannel"; p.displayName = "Source";
        p.type = ParamType::Enum; p.defaultValue = 0;
        p.enumOptions = {"CH1", "CH2", "CH3", "CH4", "M1", "M2"};
        p.subGroup = "Trigger"; p.order = order++;
        p.dependsOn = "triggerEnabled"; p.dependsValues = {true};
        params.push_back(p);
    }
    {
        ModuleParamDesc p; p.id = "triggerFadeTime"; p.displayName = "Fade Time";
        p.tooltip = "Fade duration for Normal/Single modes (seconds)";
        p.type = ParamType::Float; p.minValue = 0.1f; p.maxValue = 10.0f;
        p.defaultValue = 2.0f; p.subGroup = "Trigger"; p.order = order++;
        p.dependsOn = "triggerMode"; p.dependsValues = {1, 2}; // Normal, Single
        params.push_back(p);
    }

    // Signal Channels CH1-CH4
    const char* chNames[SIGNAL_CHANNELS] = {"CH1", "CH2", "CH3", "CH4"};
    for (int ch = 0; ch < SIGNAL_CHANNELS; ++ch)
    {
        std::string pfx = "ch" + std::to_string(ch + 1) + ".";
        std::string sg = chNames[ch];
        int bo = 100 + ch * 20;

        { ModuleParamDesc p; p.id = pfx + "visible"; p.displayName = sg + " Enabled";
          p.type = ParamType::Bool; p.defaultValue = (ch == 0);
          p.subGroup = sg; p.order = bo++; params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "source"; p.displayName = "Source";
          p.type = ParamType::Enum; p.defaultValue = ch;
          p.enumOptions = {"Left", "Right", "Mono", "Mid", "Side"};
          p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "mode"; p.displayName = "Mode";
          p.type = ParamType::Enum; p.defaultValue = 0;
          p.enumOptions = {"Waveform", "Envelope"};
          p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "coupling"; p.displayName = "Coupling";
          p.type = ParamType::Enum; p.defaultValue = 0;
          p.enumOptions = {"DC", "AC"};
          p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "voltsPerDiv"; p.displayName = "Scale";
          p.type = ParamType::Float; p.minValue = 0.01f; p.maxValue = 2.0f;
          p.defaultValue = 0.5f; p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "offset"; p.displayName = "Offset";
          p.type = ParamType::Float; p.minValue = -4.0f; p.maxValue = 4.0f;
          p.defaultValue = 0.0f; p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "lineWidth"; p.displayName = "Line Width";
          p.type = ParamType::Float; p.minValue = 1.0f; p.maxValue = 5.0f;
          p.defaultValue = 2.0f; p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }
    }

    // Math Channels M1-M2
    const char* mNames[MATH_CHANNELS] = {"M1", "M2"};
    for (int m = 0; m < MATH_CHANNELS; ++m)
    {
        std::string pfx = "m" + std::to_string(m + 1) + ".";
        std::string sg = mNames[m];
        int bo = 200 + m * 20;

        { ModuleParamDesc p; p.id = pfx + "visible"; p.displayName = sg + " Enabled";
          p.type = ParamType::Bool; p.defaultValue = false;
          p.subGroup = sg; p.order = bo++; params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "operation"; p.displayName = "Operation";
          p.type = ParamType::Enum; p.defaultValue = m;
          p.enumOptions = {"A + B", "A - B", "A × B", "|A|", "Rectify", "-A", "|A - B|"};
          p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "sourceA"; p.displayName = "Source A";
          p.type = ParamType::Enum; p.defaultValue = 0;
          p.enumOptions = {"CH1", "CH2", "CH3", "CH4"};
          p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "sourceB"; p.displayName = "Source B";
          p.type = ParamType::Enum; p.defaultValue = 1;
          p.enumOptions = {"CH1", "CH2", "CH3", "CH4"};
          p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "voltsPerDiv"; p.displayName = "Scale";
          p.type = ParamType::Float; p.minValue = 0.01f; p.maxValue = 2.0f;
          p.defaultValue = 0.5f; p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "offset"; p.displayName = "Offset";
          p.type = ParamType::Float; p.minValue = -4.0f; p.maxValue = 4.0f;
          p.defaultValue = 0.0f; p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }

        { ModuleParamDesc p; p.id = pfx + "lineWidth"; p.displayName = "Line Width";
          p.type = ParamType::Float; p.minValue = 1.0f; p.maxValue = 5.0f;
          p.defaultValue = 2.0f; p.subGroup = sg; p.order = bo++;
          p.dependsOn = pfx + "visible"; p.dependsValues = {true};
          params.push_back(p); }
    }

    // Grid
    { ModuleParamDesc p; p.id = "gridStyle"; p.displayName = "Grid Style";
      p.type = ParamType::Enum; p.defaultValue = 1;
      p.enumOptions = {"None", "Lines", "Dots", "Cross"};
      p.subGroup = "Grid"; p.order = order++; params.push_back(p); }

    { ModuleParamDesc p; p.id = "gridBrightness"; p.displayName = "Brightness";
      p.type = ParamType::Float; p.minValue = 0.0f; p.maxValue = 2.0f;
      p.defaultValue = 1.0f; p.subGroup = "Grid"; p.order = order++;
      params.push_back(p); }

    { ModuleParamDesc p; p.id = "gridLineWidth"; p.displayName = "Line Width";
      p.tooltip = "Grid line width in pixels";
      p.type = ParamType::Float; p.minValue = 0.5f; p.maxValue = 3.0f;
      p.defaultValue = 1.0f; p.subGroup = "Grid"; p.order = order++;
      p.dependsOn = "gridStyle"; p.dependsValues = {1}; // Lines
      params.push_back(p); }

    { ModuleParamDesc p; p.id = "gridDotSize"; p.displayName = "Dot Size";
      p.tooltip = "Dot size in pixels";
      p.type = ParamType::Float; p.minValue = 1.0f; p.maxValue = 5.0f;
      p.defaultValue = 2.0f; p.subGroup = "Grid"; p.order = order++;
      p.dependsOn = "gridStyle"; p.dependsValues = {2}; // Dots
      params.push_back(p); }

    { ModuleParamDesc p; p.id = "gridCrossSize"; p.displayName = "Cross Size";
      p.tooltip = "Cross marker size in pixels";
      p.type = ParamType::Float; p.minValue = 2.0f; p.maxValue = 10.0f;
      p.defaultValue = 5.0f; p.subGroup = "Grid"; p.order = order++;
      p.dependsOn = "gridStyle"; p.dependsValues = {3}; // Cross
      params.push_back(p); }

    // Display
    { ModuleParamDesc p; p.id = "interpolation"; p.displayName = "Interpolation";
      p.type = ParamType::Bool; p.defaultValue = true;
      p.subGroup = "Display"; p.order = order++; params.push_back(p); }

    // Color SubGroups per channel
    const char* colorGroups[TOTAL_CHANNELS] = {
        "Line Color CH1", "Line Color CH2", "Line Color CH3", "Line Color CH4",
        "Line Color M1", "Line Color M2"
    };
    const char* visParams[TOTAL_CHANNELS] = {
        "ch1.visible", "ch2.visible", "ch3.visible", "ch4.visible",
        "m1.visible", "m2.visible"
    };
    const char* colorPfx[TOTAL_CHANNELS] = {
        "ch1Color.", "ch2Color.", "ch3Color.", "ch4Color.", "m1Color.", "m2Color."
    };

    for (int c = 0; c < TOTAL_CHANNELS; ++c)
    {
        for (const auto& p : m_colorGradients[c].paramDescs())
        {
            ModuleParamDesc pf = p;
            pf.id = std::string(colorPfx[c]) + p.id;
            pf.subGroup = colorGroups[c];
            pf.order = 500 + c * 20 + p.order;
            if (p.dependsOn.empty())
            {
                pf.dependsOn = visParams[c];
                pf.dependsValues = {true};
            }
            else
            {
                pf.dependsOn = std::string(colorPfx[c]) + p.dependsOn;
            }
            params.push_back(pf);
        }
    }

    return params;
}

bool OscilloscopeModule::getParam(const std::string& id, ParamValue& out) const
{
    // Color gradients
    const char* colorPfx[TOTAL_CHANNELS] = {
        "ch1Color.", "ch2Color.", "ch3Color.", "ch4Color.", "m1Color.", "m2Color."
    };
    for (int c = 0; c < TOTAL_CHANNELS; ++c)
    {
        size_t len = std::strlen(colorPfx[c]);
        if (id.rfind(colorPfx[c], 0) == 0)
            return m_colorGradients[c].getParam(id.substr(len), out);
    }

    // Timebase & Trigger
    if (id == "timePerDiv") { out = m_timePerDiv; return true; }
    if (id == "sampleCount") { out = m_sampleCount; return true; }
    if (id == "triggerEnabled") { out = m_triggerEnabled; return true; }
    if (id == "triggerLevel") { out = m_triggerLevel; return true; }
    if (id == "triggerTolerance") { out = m_triggerTolerance; return true; }
    if (id == "triggerPosition") { out = m_triggerPosition; return true; }
    if (id == "triggerEdge") { out = static_cast<int>(m_triggerEdge); return true; }
    if (id == "triggerMode") { out = static_cast<int>(m_triggerMode); return true; }
    if (id == "triggerIndicator") { out = static_cast<int>(m_triggerIndicatorStyle); return true; }
    if (id == "triggerChannel") { out = m_triggerChannel; return true; }
    if (id == "triggerFadeTime") { out = m_triggerFadeTime; return true; }

    // Signal channels
    for (int ch = 0; ch < SIGNAL_CHANNELS; ++ch)
    {
        std::string pfx = "ch" + std::to_string(ch + 1) + ".";
        const auto& cfg = m_signalChannels[ch];
        if (id == pfx + "visible") { out = cfg.visible; return true; }
        if (id == pfx + "source") { out = static_cast<int>(cfg.source); return true; }
        if (id == pfx + "mode") { out = static_cast<int>(cfg.mode); return true; }
        if (id == pfx + "coupling") { out = static_cast<int>(cfg.coupling); return true; }
        if (id == pfx + "voltsPerDiv") { out = cfg.voltsPerDiv; return true; }
        if (id == pfx + "offset") { out = cfg.offset; return true; }
        if (id == pfx + "lineWidth") { out = cfg.lineWidth; return true; }
    }

    // Math channels
    for (int m = 0; m < MATH_CHANNELS; ++m)
    {
        std::string pfx = "m" + std::to_string(m + 1) + ".";
        const auto& cfg = m_mathChannels[m];
        if (id == pfx + "visible") { out = cfg.visible; return true; }
        if (id == pfx + "operation") { out = static_cast<int>(cfg.operation); return true; }
        if (id == pfx + "sourceA") { out = cfg.sourceA; return true; }
        if (id == pfx + "sourceB") { out = cfg.sourceB; return true; }
        if (id == pfx + "voltsPerDiv") { out = cfg.voltsPerDiv; return true; }
        if (id == pfx + "offset") { out = cfg.offset; return true; }
        if (id == pfx + "lineWidth") { out = cfg.lineWidth; return true; }
    }

    // Grid & Display
    if (id == "gridStyle") { out = static_cast<int>(m_gridStyle); return true; }
    if (id == "gridBrightness") { out = m_gridBrightness; return true; }
    if (id == "gridLineWidth") { out = m_gridLineWidth; return true; }
    if (id == "gridDotSize") { out = m_gridDotSize; return true; }
    if (id == "gridCrossSize") { out = m_gridCrossSize; return true; }
    if (id == "interpolation") { out = m_interpolation; return true; }

    return false;
}

bool OscilloscopeModule::setParam(const std::string& id, const ParamValue& value)
{
    // Color gradients
    const char* colorPfx[TOTAL_CHANNELS] = {
        "ch1Color.", "ch2Color.", "ch3Color.", "ch4Color.", "m1Color.", "m2Color."
    };
    for (int c = 0; c < TOTAL_CHANNELS; ++c)
    {
        size_t len = std::strlen(colorPfx[c]);
        if (id.rfind(colorPfx[c], 0) == 0)
            return m_colorGradients[c].setParam(id.substr(len), value);
    }

    // Timebase
    if (id == "timePerDiv") { 
        if (auto* v = std::get_if<float>(&value)) { setTimePerDiv(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setTimePerDiv(static_cast<float>(*v)); return true; }
    }
    if (id == "sampleCount") { 
        if (auto* v = std::get_if<int>(&value)) { setSampleCount(*v); return true; }
        if (auto* v = std::get_if<float>(&value)) { setSampleCount(static_cast<int>(*v)); return true; }
    }

    // Trigger
    if (id == "triggerEnabled") { if (auto* v = std::get_if<bool>(&value)) { m_triggerEnabled = *v; return true; } }
    if (id == "triggerLevel") { 
        if (auto* v = std::get_if<float>(&value)) { setTriggerLevel(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setTriggerLevel(static_cast<float>(*v)); return true; }
    }
    if (id == "triggerTolerance") { 
        if (auto* v = std::get_if<float>(&value)) { setTriggerTolerance(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setTriggerTolerance(static_cast<float>(*v)); return true; }
    }
    if (id == "triggerPosition") { 
        if (auto* v = std::get_if<float>(&value)) { setTriggerPosition(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setTriggerPosition(static_cast<float>(*v)); return true; }
    }
    if (id == "triggerEdge") { 
        if (auto* v = std::get_if<int>(&value)) { m_triggerEdge = static_cast<TriggerEdge>(*v); return true; }
        if (auto* v = std::get_if<float>(&value)) { m_triggerEdge = static_cast<TriggerEdge>(static_cast<int>(*v)); return true; }
    }
    if (id == "triggerMode") { 
        if (auto* v = std::get_if<int>(&value)) { m_triggerMode = static_cast<TriggerMode>(*v); return true; }
        if (auto* v = std::get_if<float>(&value)) { m_triggerMode = static_cast<TriggerMode>(static_cast<int>(*v)); return true; }
    }
    if (id == "triggerIndicator") { 
        if (auto* v = std::get_if<int>(&value)) { m_triggerIndicatorStyle = static_cast<TriggerIndicatorStyle>(*v); return true; }
        if (auto* v = std::get_if<float>(&value)) { m_triggerIndicatorStyle = static_cast<TriggerIndicatorStyle>(static_cast<int>(*v)); return true; }
    }
    if (id == "triggerChannel") { 
        if (auto* v = std::get_if<int>(&value)) { setTriggerChannel(*v); return true; }
        if (auto* v = std::get_if<float>(&value)) { setTriggerChannel(static_cast<int>(*v)); return true; }
    }
    if (id == "triggerFadeTime") { 
        if (auto* v = std::get_if<float>(&value)) { setTriggerFadeTime(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setTriggerFadeTime(static_cast<float>(*v)); return true; }
    }

    // Signal channels
    for (int ch = 0; ch < SIGNAL_CHANNELS; ++ch)
    {
        std::string pfx = "ch" + std::to_string(ch + 1) + ".";
        auto& cfg = m_signalChannels[ch];
        if (id == pfx + "visible") { if (auto* v = std::get_if<bool>(&value)) { cfg.visible = *v; return true; } }
        if (id == pfx + "source") { 
            if (auto* v = std::get_if<int>(&value)) { cfg.source = static_cast<SignalSource>(*v); return true; }
            if (auto* v = std::get_if<float>(&value)) { cfg.source = static_cast<SignalSource>(static_cast<int>(*v)); return true; }
        }
        if (id == pfx + "mode") { 
            if (auto* v = std::get_if<int>(&value)) { cfg.mode = static_cast<SignalMode>(*v); return true; }
            if (auto* v = std::get_if<float>(&value)) { cfg.mode = static_cast<SignalMode>(static_cast<int>(*v)); return true; }
        }
        if (id == pfx + "coupling") { 
            if (auto* v = std::get_if<int>(&value)) { cfg.coupling = static_cast<CouplingMode>(*v); m_acCouplingStates[ch].reset(); return true; }
            if (auto* v = std::get_if<float>(&value)) { cfg.coupling = static_cast<CouplingMode>(static_cast<int>(*v)); m_acCouplingStates[ch].reset(); return true; }
        }
        if (id == pfx + "voltsPerDiv") { 
            if (auto* v = std::get_if<float>(&value)) { cfg.voltsPerDiv = std::clamp(*v, 0.01f, 2.0f); return true; }
            if (auto* v = std::get_if<int>(&value)) { cfg.voltsPerDiv = std::clamp(static_cast<float>(*v), 0.01f, 2.0f); return true; }
        }
        if (id == pfx + "offset") { 
            if (auto* v = std::get_if<float>(&value)) { cfg.offset = std::clamp(*v, -4.0f, 4.0f); return true; }
            if (auto* v = std::get_if<int>(&value)) { cfg.offset = std::clamp(static_cast<float>(*v), -4.0f, 4.0f); return true; }
        }
        if (id == pfx + "lineWidth") { 
            if (auto* v = std::get_if<float>(&value)) { cfg.lineWidth = std::clamp(*v, 1.0f, 5.0f); return true; }
            if (auto* v = std::get_if<int>(&value)) { cfg.lineWidth = std::clamp(static_cast<float>(*v), 1.0f, 5.0f); return true; }
        }
    }

    // Math channels
    for (int m = 0; m < MATH_CHANNELS; ++m)
    {
        std::string pfx = "m" + std::to_string(m + 1) + ".";
        auto& cfg = m_mathChannels[m];
        if (id == pfx + "visible") { if (auto* v = std::get_if<bool>(&value)) { cfg.visible = *v; return true; } }
        if (id == pfx + "operation") { 
            if (auto* v = std::get_if<int>(&value)) { cfg.operation = static_cast<MathOperation>(*v); return true; }
            if (auto* v = std::get_if<float>(&value)) { cfg.operation = static_cast<MathOperation>(static_cast<int>(*v)); return true; }
        }
        if (id == pfx + "sourceA") { 
            if (auto* v = std::get_if<int>(&value)) { cfg.sourceA = std::clamp(*v, 0, SIGNAL_CHANNELS - 1); return true; }
            if (auto* v = std::get_if<float>(&value)) { cfg.sourceA = std::clamp(static_cast<int>(*v), 0, SIGNAL_CHANNELS - 1); return true; }
        }
        if (id == pfx + "sourceB") { 
            if (auto* v = std::get_if<int>(&value)) { cfg.sourceB = std::clamp(*v, 0, SIGNAL_CHANNELS - 1); return true; }
            if (auto* v = std::get_if<float>(&value)) { cfg.sourceB = std::clamp(static_cast<int>(*v), 0, SIGNAL_CHANNELS - 1); return true; }
        }
        if (id == pfx + "voltsPerDiv") { 
            if (auto* v = std::get_if<float>(&value)) { cfg.voltsPerDiv = std::clamp(*v, 0.01f, 2.0f); return true; }
            if (auto* v = std::get_if<int>(&value)) { cfg.voltsPerDiv = std::clamp(static_cast<float>(*v), 0.01f, 2.0f); return true; }
        }
        if (id == pfx + "offset") { 
            if (auto* v = std::get_if<float>(&value)) { cfg.offset = std::clamp(*v, -4.0f, 4.0f); return true; }
            if (auto* v = std::get_if<int>(&value)) { cfg.offset = std::clamp(static_cast<float>(*v), -4.0f, 4.0f); return true; }
        }
        if (id == pfx + "lineWidth") { 
            if (auto* v = std::get_if<float>(&value)) { cfg.lineWidth = std::clamp(*v, 1.0f, 5.0f); return true; }
            if (auto* v = std::get_if<int>(&value)) { cfg.lineWidth = std::clamp(static_cast<float>(*v), 1.0f, 5.0f); return true; }
        }
    }

    // Grid & Display
    if (id == "gridStyle") { 
        if (auto* v = std::get_if<int>(&value)) { m_gridStyle = static_cast<GridStyle>(*v); return true; }
        if (auto* v = std::get_if<float>(&value)) { m_gridStyle = static_cast<GridStyle>(static_cast<int>(*v)); return true; }
    }
    if (id == "gridBrightness") { 
        if (auto* v = std::get_if<float>(&value)) { setGridBrightness(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setGridBrightness(static_cast<float>(*v)); return true; }
    }
    if (id == "gridLineWidth") { 
        if (auto* v = std::get_if<float>(&value)) { setGridLineWidth(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setGridLineWidth(static_cast<float>(*v)); return true; }
    }
    if (id == "gridDotSize") { 
        if (auto* v = std::get_if<float>(&value)) { setGridDotSize(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setGridDotSize(static_cast<float>(*v)); return true; }
    }
    if (id == "gridCrossSize") { 
        if (auto* v = std::get_if<float>(&value)) { setGridCrossSize(*v); return true; }
        if (auto* v = std::get_if<int>(&value)) { setGridCrossSize(static_cast<float>(*v)); return true; }
    }
    if (id == "interpolation") { if (auto* v = std::get_if<bool>(&value)) { m_interpolation = *v; return true; } }

    return false;
}

// =============================================================================
// Settings
// =============================================================================

void OscilloscopeModule::setTimePerDiv(float msPerDiv) { m_timePerDiv = std::clamp(msPerDiv, 0.1f, 100.0f); }
void OscilloscopeModule::setSampleCount(int count) { m_sampleCount = std::clamp(count, 32, 8192); }
void OscilloscopeModule::setTriggerLevel(float level) { m_triggerLevel = std::clamp(level, -1.0f, 1.0f); }
void OscilloscopeModule::setTriggerPosition(float pos) { m_triggerPosition = std::clamp(pos, 0.0f, 1.0f); }
void OscilloscopeModule::setTriggerChannel(int ch) { m_triggerChannel = std::clamp(ch, 0, TOTAL_CHANNELS - 1); }
void OscilloscopeModule::setTriggerHoldoff(float ms) { m_triggerHoldoff = std::max(0.0f, ms); }
void OscilloscopeModule::setTriggerFadeTime(float sec) { m_triggerFadeTime = std::clamp(sec, 0.1f, 10.0f); }
void OscilloscopeModule::setGridColor(float r, float g, float b, float a) { m_gridColor = {r, g, b, a}; }
void OscilloscopeModule::setGridBrightness(float b) { m_gridBrightness = std::clamp(b, 0.0f, 2.0f); }
void OscilloscopeModule::setBackgroundColor(float r, float g, float b) { m_bgColorR = r; m_bgColorG = g; m_bgColorB = b; }

// =============================================================================
// Channel Access
// =============================================================================

SignalChannelConfig& OscilloscopeModule::signalChannel(int i) { return m_signalChannels[std::clamp(i, 0, SIGNAL_CHANNELS - 1)]; }
const SignalChannelConfig& OscilloscopeModule::signalChannel(int i) const { return m_signalChannels[std::clamp(i, 0, SIGNAL_CHANNELS - 1)]; }
MathChannelConfig& OscilloscopeModule::mathChannel(int i) { return m_mathChannels[std::clamp(i, 0, MATH_CHANNELS - 1)]; }
const MathChannelConfig& OscilloscopeModule::mathChannel(int i) const { return m_mathChannels[std::clamp(i, 0, MATH_CHANNELS - 1)]; }

const ChannelConfigBase& OscilloscopeModule::channelBase(int i) const
{
    if (i < SIGNAL_CHANNELS) return m_signalChannels[i];
    return m_mathChannels[i - SIGNAL_CHANNELS];
}

ColorGradientModule& OscilloscopeModule::colorGradient(int ch) { return m_colorGradients[std::clamp(ch, 0, TOTAL_CHANNELS - 1)]; }
const ColorGradientModule& OscilloscopeModule::colorGradient(int ch) const { return m_colorGradients[std::clamp(ch, 0, TOTAL_CHANNELS - 1)]; }
ACCouplingState& OscilloscopeModule::acCouplingState(int ch) { return m_acCouplingStates[std::clamp(ch, 0, SIGNAL_CHANNELS - 1)]; }

// =============================================================================
// Signal Processing
// =============================================================================

void OscilloscopeModule::processSignals(const float* left, const float* right, int count,
                                         std::array<std::vector<float>, TOTAL_CHANNELS>& output)
{
    for (int c = 0; c < TOTAL_CHANNELS; ++c) output[c].resize(count);

    // Signal channels
    for (int ch = 0; ch < SIGNAL_CHANNELS; ++ch)
    {
        const auto& cfg = m_signalChannels[ch];
        for (int i = 0; i < count; ++i)
        {
            float l = left[i], r = right[i];
            switch (cfg.source)
            {
                case SignalSource::Left: output[ch][i] = l; break;
                case SignalSource::Right: output[ch][i] = r; break;
                case SignalSource::Mono:
                case SignalSource::Mid: output[ch][i] = (l + r) * 0.5f; break;
                case SignalSource::Side: output[ch][i] = (l - r) * 0.5f; break;
            }
        }
        if (cfg.coupling == CouplingMode::AC)
            applyACCoupling(output[ch].data(), count, m_acCouplingStates[ch]);
        if (cfg.mode == SignalMode::Envelope)
        {
            std::vector<float> tmp = output[ch];
            computeEnvelope(tmp.data(), count, output[ch].data(), cfg.envelopeAttack * 44.1f, cfg.envelopeRelease * 44.1f);
        }
    }

    // Math channels
    for (int m = 0; m < MATH_CHANNELS; ++m)
    {
        const auto& cfg = m_mathChannels[m];
        int outIdx = SIGNAL_CHANNELS + m;
        const float* srcA = output[cfg.sourceA].data();
        const float* srcB = output[cfg.sourceB].data();
        for (int i = 0; i < count; ++i)
        {
            float a = srcA[i], b = srcB[i];
            switch (cfg.operation)
            {
                case MathOperation::Add: output[outIdx][i] = a + b; break;
                case MathOperation::Subtract: output[outIdx][i] = a - b; break;
                case MathOperation::Multiply: output[outIdx][i] = a * b; break;
                case MathOperation::Absolute: output[outIdx][i] = std::fabs(a); break;
                case MathOperation::Rectify: output[outIdx][i] = std::max(0.0f, a); break;
                case MathOperation::Invert: output[outIdx][i] = -a; break;
                case MathOperation::Difference: output[outIdx][i] = std::fabs(a - b); break;
            }
        }
    }
}

void OscilloscopeModule::applyACCoupling(float* samples, int count, ACCouplingState& state)
{
    if (count <= 0) return;
    
    // Calculate DC offset as running average of the entire buffer
    // This gives a stable reference for AC coupling
    float dcOffset = 0.0f;
    for (int i = 0; i < count; ++i)
    {
        dcOffset += samples[i];
    }
    dcOffset /= static_cast<float>(count);
    
    // Smooth the DC offset using EMA to avoid sudden jumps
    state.prevOutput = state.alpha * state.prevOutput + (1.0f - state.alpha) * dcOffset;
    
    // Subtract the smoothed DC offset from all samples
    for (int i = 0; i < count; ++i)
    {
        samples[i] -= state.prevOutput;
    }
}

void OscilloscopeModule::computeEnvelope(const float* samples, int count, float* output, float attack, float release)
{
    float env = 0.0f;
    float attCoef = 1.0f - std::exp(-1.0f / std::max(1.0f, attack));
    float relCoef = 1.0f - std::exp(-1.0f / std::max(1.0f, release));
    for (int i = 0; i < count; ++i)
    {
        float abs = std::fabs(samples[i]);
        env += (abs > env ? attCoef : relCoef) * (abs - env);
        output[i] = env;
    }
}

// =============================================================================
// Trigger Detection
// =============================================================================

int OscilloscopeModule::findTriggerPoint(const float* samples, int count) const
{
    if (!m_triggerEnabled || count < 2)
    {
        return 0;
    }

    // Tolerance as absolute value (1 div = 0.25 in normalized -1 to 1 range)
    constexpr float DIV_SIZE = 2.0f / static_cast<float>(DIVISIONS_Y);  // 0.25
    const float tolerance = m_triggerTolerance * DIV_SIZE;
    
    // The trigger level with tolerance zone
    const float levelMin = m_triggerLevel - tolerance;
    const float levelMax = m_triggerLevel + tolerance;

    // Track min/max for debugging
    float minSample = samples[0];
    float maxSample = samples[0];

    // Search through samples for a crossing
    for (int i = 1; i < count; ++i)
    {
        float prev = samples[i - 1];
        float curr = samples[i];
        
        minSample = std::min(minSample, curr);
        maxSample = std::max(maxSample, curr);
        
        bool triggered = false;
        
        // Simple crossing detection:
        // Rising = signal goes from below level to at/above level
        // Falling = signal goes from above level to at/below level
        
        switch (m_triggerEdge)
        {
            case TriggerEdge::Rising:
                // prev must be below trigger zone, curr must be in or above zone
                triggered = (prev < levelMin) && (curr >= levelMin);
                break;
                
            case TriggerEdge::Falling:
                // prev must be above trigger zone, curr must be in or below zone
                triggered = (prev > levelMax) && (curr <= levelMax);
                break;
                
            case TriggerEdge::Both:
                triggered = ((prev < levelMin) && (curr >= levelMin)) ||
                            ((prev > levelMax) && (curr <= levelMax));
                break;
        }
        
        if (triggered)
        {
            // Log trigger found (rate-limited)
            static int triggerLogCounter = 0;
            if (++triggerLogCounter % 60 == 0)
            {
                std::ostringstream oss;
                oss << "TRIGGER FOUND: idx=" << i << " prev=" << prev << " curr=" << curr
                    << " level=" << m_triggerLevel << " tol=" << tolerance;
                BasicLogger::logDebug(oss.str());
            }
            return i;
        }
    }
    
    // No trigger found - log sample range for debugging
    static int noTrigLogCounter = 0;
    if (++noTrigLogCounter % 120 == 0)
    {
        std::ostringstream oss;
        oss << "NO TRIGGER: level=" << m_triggerLevel 
            << " levelMin=" << levelMin << " levelMax=" << levelMax
            << " samples=[" << minSample << "," << maxSample << "]"
            << " edge=" << triggerEdgeName(m_triggerEdge);
        BasicLogger::logDebug(oss.str());
    }
    
    if (m_triggerMode == TriggerMode::Auto)
    {
        return 0;  // Auto mode: show from start
    }
    
    return -1;  // Signal: no trigger
}

// =============================================================================
// Utility
// =============================================================================

const char* OscilloscopeModule::signalSourceName(SignalSource s)
{
    switch (s) { case SignalSource::Left: return "Left"; case SignalSource::Right: return "Right";
                 case SignalSource::Mono: return "Mono"; case SignalSource::Mid: return "Mid";
                 case SignalSource::Side: return "Side"; }
    return "?";
}
const char* OscilloscopeModule::signalModeName(SignalMode m) { return m == SignalMode::Waveform ? "Waveform" : "Envelope"; }
const char* OscilloscopeModule::mathOperationName(MathOperation o)
{
    switch (o) { case MathOperation::Add: return "A+B"; case MathOperation::Subtract: return "A-B";
                 case MathOperation::Multiply: return "A×B"; case MathOperation::Absolute: return "|A|";
                 case MathOperation::Rectify: return "Rect"; case MathOperation::Invert: return "-A";
                 case MathOperation::Difference: return "|A-B|"; }
    return "?";
}
const char* OscilloscopeModule::triggerEdgeName(TriggerEdge e) { switch (e) { case TriggerEdge::Rising: return "Rising"; case TriggerEdge::Falling: return "Falling"; case TriggerEdge::Both: return "Both"; } return "?"; }
const char* OscilloscopeModule::triggerModeName(TriggerMode m) { switch (m) { case TriggerMode::Auto: return "Auto"; case TriggerMode::Normal: return "Normal"; case TriggerMode::Single: return "Single"; } return "?"; }
const char* OscilloscopeModule::couplingModeName(CouplingMode c) { return c == CouplingMode::DC ? "DC" : "AC"; }
const char* OscilloscopeModule::gridStyleName(GridStyle s) { switch (s) { case GridStyle::None: return "None"; case GridStyle::Lines: return "Lines"; case GridStyle::Dots: return "Dots"; case GridStyle::Cross: return "Cross"; } return "?"; }

} // namespace lumi::modules
