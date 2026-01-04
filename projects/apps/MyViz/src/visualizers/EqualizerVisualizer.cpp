/**
 ****************************************************************************************
 * @file   EqualizerVisualizer.cpp
 * @brief  Implementation of spectrum analyzer visualizer
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/EqualizerVisualizer.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QMutexLocker>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>

using namespace lumi::modules;

namespace
{

// =============================================================================
// Shader Sources
// =============================================================================

// Shader für Bars - verwendet Gradient
const char* BAR_VERTEX_SHADER = R"(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in float aGradientT;  // Gradient parameter [0,1]

out float vGradientT;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    vGradientT = aGradientT;
}
)";

const char* BAR_FRAGMENT_SHADER = R"(
#version 330 core

in float vGradientT;
out vec4 FragColor;

// Up to 8 color stops
uniform vec4 uColor0;
uniform vec4 uColor1;
uniform vec4 uColor2;
uniform vec4 uColor3;
uniform vec4 uColor4;
uniform vec4 uColor5;
uniform vec4 uColor6;
uniform vec4 uColor7;
uniform vec4 uStopPos;      // Positions of stops 0-3
uniform vec4 uStopPos2;     // Positions of stops 4-7
uniform int uStopCount;     // Number of active stops (2-8)
uniform int uGradientMode;  // 0=Solid, 1=Linear, 2=Radial
uniform float uGradientAngle;
uniform float uAlpha;

vec4 getColor(int idx)
{
    if (idx == 0) return uColor0;
    if (idx == 1) return uColor1;
    if (idx == 2) return uColor2;
    if (idx == 3) return uColor3;
    if (idx == 4) return uColor4;
    if (idx == 5) return uColor5;
    if (idx == 6) return uColor6;
    return uColor7;
}

float getStopPos(int idx)
{
    if (idx < 4) {
        if (idx == 0) return uStopPos.x;
        if (idx == 1) return uStopPos.y;
        if (idx == 2) return uStopPos.z;
        return uStopPos.w;
    } else {
        if (idx == 4) return uStopPos2.x;
        if (idx == 5) return uStopPos2.y;
        if (idx == 6) return uStopPos2.z;
        return uStopPos2.w;
    }
}

vec4 sampleGradient(float t)
{
    t = clamp(t, 0.0, 1.0);
    
    if (uStopCount <= 1 || t <= getStopPos(0)) return uColor0;
    if (t >= getStopPos(uStopCount - 1)) return getColor(uStopCount - 1);
    
    for (int i = 0; i < uStopCount - 1; i++) {
        float pos0 = getStopPos(i);
        float pos1 = getStopPos(i + 1);
        
        if (t >= pos0 && t < pos1) {
            float localT = (t - pos0) / max(pos1 - pos0, 0.001);
            return mix(getColor(i), getColor(i + 1), localT);
        }
    }
    
    return uColor0;
}

void main()
{
    vec4 color;
    
    if (uGradientMode == 0)
    {
        // Solid color
        color = uColor0;
    }
    else
    {
        // Linear or Radial - use gradient parameter
        color = sampleGradient(vGradientT);
    }
    
    FragColor = color;
    FragColor.a *= uAlpha;
}
)";

// Shader für Peaks/Particles - verwendet direkte Vertex-Farben
const char* PEAK_VERTEX_SHADER = R"(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;

out vec4 vColor;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)";

const char* PEAK_FRAGMENT_SHADER = R"(
#version 330 core

in vec4 vColor;
out vec4 FragColor;

uniform float uAlpha;

void main()
{
    FragColor = vColor;
    FragColor.a *= uAlpha;
}
)";

} // anonymous namespace

// =============================================================================
// Constructor / Destructor
// =============================================================================

EqualizerVisualizer::EqualizerVisualizer()
    : VisualizerBase(QStringLiteral("equalizer"),
                     QObject::tr("Equalizer"),
                     QObject::tr("Spectrum analyzer with bars and peak markers"))
{
    m_spectrumData.resize(2048, 0.0f);
    
    // Sync AudioSourceModule bands with Equalizer bands
    m_audioSource.setBands(m_equalizer.bandCount());
}

EqualizerVisualizer::~EqualizerVisualizer()
{
    // Cleanup handled in onCleanup()
}

// =============================================================================
// IVisualizer Parameter Interface
// =============================================================================

std::vector<ModuleParamDesc> EqualizerVisualizer::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    // =========================================================================
    // 1. Audio Source Parameters
    // =========================================================================
    int audioOrder = 0;
    for (const auto& p : m_audioSource.paramDescs())
    {
        // Skip "bands" - we use eq.bands instead and sync automatically
        if (p.id == "bands")
            continue;
            
        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "1. Audio";
        prefixed.order = audioOrder++;  // Sequential order
        
        // Keep subGroup for UI organization
        // prefixed.subGroup is already set from AudioSourceModule
        
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "audio." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }

    // =========================================================================
    // 2. Equalizer Display Parameters
    // =========================================================================
    
    // Bands
    {
        ModuleParamDesc p;
        p.id = "eq.bands";
        p.displayName = "Bands";
        p.group = "2. Equalizer";
        p.type = ParamType::Int;
        p.defaultValue = 64;
        p.minValue = 8;
        p.maxValue = 256;
        p.step = 1;
        p.tooltip = "Number of frequency bands";
        p.order = 0;
        params.push_back(p);
    }
    
    // Bar Gap
    {
        ModuleParamDesc p;
        p.id = "eq.barGap";
        p.displayName = "Bar Gap";
        p.group = "2. Equalizer";
        p.type = ParamType::Float;
        p.defaultValue = 2.0f;
        p.minValue = 0.0f;
        p.maxValue = 20.0f;
        p.unit = "px";
        p.tooltip = "Gap between bars";
        p.order = 1;
        params.push_back(p);
    }
    
    // Orientation
    {
        ModuleParamDesc p;
        p.id = "eq.orientation";
        p.displayName = "Orientation";
        p.group = "2. Equalizer";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Bottom Up", "Top Down"};
        p.tooltip = "Bar growth direction";
        p.order = 2;
        params.push_back(p);
    }

    // =========================================================================
    // 3. Color Parameters
    // =========================================================================
    
    // Gradient Domain (Position, Amplitude, Time, Beat)
    {
        ModuleParamDesc p;
        p.id = "color.domain";
        p.displayName = "Color Mapping";
        p.group = "3. Color";
        p.type = ParamType::Enum;
        p.defaultValue = 0;  // Position
        p.enumOptions = {"Position", "Amplitude", "Time", "Beat"};
        p.tooltip = "What drives the gradient color";
        p.order = 0;
        params.push_back(p);
    }
    
    // Color Gradient Parameters (from ColorGradientModule)
    for (const auto& p : m_equalizer.colorGradient().paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "color." + p.id;
        prefixed.group = "3. Color";
        prefixed.order = 10 + p.order;  // After domain
        
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "color." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }

    // =========================================================================
    // 4. Peak Hold / Spawner Parameters
    // =========================================================================
    
    // Enable Peaks
    {
        ModuleParamDesc p;
        p.id = "peak.enabled";
        p.displayName = "Enable Peaks";
        p.group = "4. Peak Hold";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.tooltip = "Show peak hold markers";
        p.order = 0;
        params.push_back(p);
    }
    
    // Hold Delay
    {
        ModuleParamDesc p;
        p.id = "peak.holdDelay";
        p.displayName = "Hold Delay";
        p.group = "4. Peak Hold";
        p.type = ParamType::Float;
        p.defaultValue = 120.0f;
        p.minValue = 0.0f;
        p.maxValue = 2000.0f;
        p.unit = "ms";
        p.tooltip = "Time to hold at peak before moving";
        p.order = 1;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Gravity (-15 to +15: negative=float up, 0=stay, positive=fall down)
    {
        ModuleParamDesc p;
        p.id = "peak.gravity";
        p.displayName = "Gravity";
        p.group = "4. Peak Hold";
        p.type = ParamType::Float;
        p.defaultValue = 9.81f;
        p.minValue = -15.0f;
        p.maxValue = 15.0f;
        p.tooltip = "Fall acceleration (negative=float up, 0=stay, positive=fall)";
        p.order = 2;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Air Resistance / Falloff
    {
        ModuleParamDesc p;
        p.id = "peak.falloff";
        p.displayName = "Air Resistance";
        p.group = "4. Peak Hold";
        p.type = ParamType::Float;
        p.defaultValue = 0.5f;
        p.minValue = 0.0f;
        p.maxValue = 20.0f;
        p.tooltip = "Damping / air resistance";
        p.order = 3;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Bounce Elasticity
    {
        ModuleParamDesc p;
        p.id = "peak.bounce";
        p.displayName = "Bounce";
        p.group = "4. Peak Hold";
        p.type = ParamType::Float;
        p.defaultValue = 0.25f;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.tooltip = "Elasticity when hitting bar top or boundaries";
        p.order = 4;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Respawn on Leave
    {
        ModuleParamDesc p;
        p.id = "peak.respawnOnLeave";
        p.displayName = "Respawn On Leave";
        p.group = "4. Peak Hold";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Respawn peak when it leaves visible range";
        p.order = 5;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Draw peaks behind bars
    {
        ModuleParamDesc p;
        p.id = "peak.behind";
        p.displayName = "Peaks Behind Bars";
        p.group = "4. Peak Hold";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Draw peak markers behind bars";
        p.order = 6;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // =========================================================================
    // 5. Peak Thickness Parameters
    // =========================================================================
    
    // Thickness Mode
    {
        ModuleParamDesc p;
        p.id = "thickness.mode";
        p.displayName = "Thickness Mode";
        p.group = "5. Peak Thickness";
        p.type = ParamType::Enum;
        p.defaultValue = 0;  // Off
        p.enumOptions = {"Fixed", "Direct (thicker at high)", "Inverse (thicker at low)"};
        p.tooltip = "How peak thickness changes with amplitude";
        p.order = 0;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Base Thickness
    {
        ModuleParamDesc p;
        p.id = "thickness.base";
        p.displayName = "Base Thickness";
        p.group = "5. Peak Thickness";
        p.type = ParamType::Float;
        p.defaultValue = 2.0f;
        p.minValue = 1.0f;
        p.maxValue = 20.0f;
        p.unit = "px";
        p.tooltip = "Base thickness in pixels";
        p.order = 1;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Scale Thickness (only when mode != Off)
    {
        ModuleParamDesc p;
        p.id = "thickness.scale";
        p.displayName = "Thickness Scale";
        p.group = "5. Peak Thickness";
        p.type = ParamType::Float;
        p.defaultValue = 4.0f;
        p.minValue = 0.0f;
        p.maxValue = 20.0f;
        p.unit = "px";
        p.tooltip = "Additional thickness based on amplitude";
        p.order = 2;
        p.dependsOn = "thickness.mode";
        p.dependsValues = {1, 2};  // Direct, Inverse
        params.push_back(p);
    }

    // =========================================================================
    // 6. Spring Physics Parameters
    // =========================================================================
    
    // Enable Spring
    {
        ModuleParamDesc p;
        p.id = "spring.enabled";
        p.displayName = "Enable Spring";
        p.group = "6. Spring Physics";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Use spring physics instead of gravity";
        p.order = 0;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Spring Constant
    {
        ModuleParamDesc p;
        p.id = "spring.k";
        p.displayName = "Spring Constant";
        p.group = "6. Spring Physics";
        p.type = ParamType::Float;
        p.defaultValue = 40.0f;
        p.minValue = 1.0f;
        p.maxValue = 200.0f;
        p.tooltip = "Spring stiffness (higher = faster oscillation)";
        p.order = 1;
        p.dependsOn = "spring.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Spring Damping
    {
        ModuleParamDesc p;
        p.id = "spring.damping";
        p.displayName = "Spring Damping";
        p.group = "6. Spring Physics";
        p.type = ParamType::Float;
        p.defaultValue = 10.0f;
        p.minValue = 0.0f;
        p.maxValue = 50.0f;
        p.tooltip = "Damping (higher = less oscillation)";
        p.order = 2;
        p.dependsOn = "spring.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Use Delay in Spring Mode
    {
        ModuleParamDesc p;
        p.id = "spring.useDelay";
        p.displayName = "Use Hold Delay";
        p.group = "6. Spring Physics";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.tooltip = "Apply hold delay before spring starts";
        p.order = 3;
        p.dependsOn = "spring.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // =========================================================================
    // 7. Particle System Parameters
    // =========================================================================
    
    // Spawn Particles
    {
        ModuleParamDesc p;
        p.id = "particle.spawn";
        p.displayName = "Spawn Particles";
        p.group = "7. Particles";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Spawn particles on peak hits";
        p.order = 0;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Min Delta to Spawn
    {
        ModuleParamDesc p;
        p.id = "particle.minDelta";
        p.displayName = "Min Rise";
        p.group = "7. Particles";
        p.type = ParamType::Float;
        p.defaultValue = 0.0f;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.tooltip = "Minimum amplitude rise to spawn a particle";
        p.order = 1;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Min Interval
    {
        ModuleParamDesc p;
        p.id = "particle.minInterval";
        p.displayName = "Min Interval";
        p.group = "7. Particles";
        p.type = ParamType::Float;
        p.defaultValue = 0.0f;
        p.minValue = 0.0f;
        p.maxValue = 1000.0f;
        p.unit = "ms";
        p.tooltip = "Minimum time between particle spawns";
        p.order = 2;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Max Particles Per Band
    {
        ModuleParamDesc p;
        p.id = "particle.maxPerBand";
        p.displayName = "Max Per Band";
        p.group = "7. Particles";
        p.type = ParamType::Int;
        p.defaultValue = 8;
        p.minValue = 1;
        p.maxValue = 32;
        p.tooltip = "Maximum particles per frequency band";
        p.order = 3;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Freeze Particle Color
    {
        ModuleParamDesc p;
        p.id = "particle.freezeColor";
        p.displayName = "Freeze Color";
        p.group = "7. Particles";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Keep color from spawn moment";
        p.order = 4;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Bind Particle Color to Spawner
    {
        ModuleParamDesc p;
        p.id = "particle.bindToSpawner";
        p.displayName = "Bind to Spawner";
        p.group = "7. Particles";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Particle follows spawner color live";
        p.order = 5;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // =========================================================================
    // 8. Peak Color Parameters
    // =========================================================================
    
    // Auto Color (use gradient)
    {
        ModuleParamDesc p;
        p.id = "peakColor.auto";
        p.displayName = "Auto Color";
        p.group = "8. Peak Color";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.tooltip = "Use gradient for peak colors";
        p.order = 0;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }
    
    // Fixed Peak Color
    {
        ModuleParamDesc p;
        p.id = "peakColor.fixed";
        p.displayName = "Fixed Color";
        p.group = "8. Peak Color";
        p.type = ParamType::Color;
        p.tooltip = "Fixed color when auto is off";
        p.order = 1;
        p.dependsOn = "peakColor.auto";
        p.dependsValues = {false};
        params.push_back(p);
    }
    
    // Freeze Spawner Color
    {
        ModuleParamDesc p;
        p.id = "peakColor.freeze";
        p.displayName = "Freeze Spawner Color";
        p.group = "8. Peak Color";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Don't update spawner color live";
        p.order = 2;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    return params;
}

bool EqualizerVisualizer::getParam(const std::string& id, ParamValue& out) const
{
    // Audio parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }
    
    // Color parameters
    if (id == "color.domain")
    {
        out = static_cast<int>(m_equalizer.gradientDomain());
        return true;
    }
    if (id.rfind("color.", 0) == 0)
    {
        return m_equalizer.colorGradient().getParam(id.substr(6), out);
    }
    
    // Equalizer parameters
    if (id == "eq.bands") { out = m_equalizer.bandCount(); return true; }
    if (id == "eq.barGap") { out = m_equalizer.barGapPx(); return true; }
    if (id == "eq.orientation") { out = static_cast<int>(m_equalizer.orientation()); return true; }
    
    // Peak parameters
    const auto& spawnerCfg = m_equalizer.spawnerConfig();
    if (id == "peak.enabled") { out = spawnerCfg.enabled; return true; }
    if (id == "peak.holdDelay") { out = spawnerCfg.delayMs; return true; }
    if (id == "peak.gravity") { out = spawnerCfg.gravity; return true; }
    if (id == "peak.falloff") { out = spawnerCfg.falloffPerSec; return true; }
    if (id == "peak.bounce") { out = spawnerCfg.bounceElasticity; return true; }
    if (id == "peak.respawnOnLeave") { out = spawnerCfg.respawnOnLeave; return true; }
    if (id == "peak.behind") { out = m_equalizer.drawPeaksBehindBars(); return true; }
    
    // Thickness parameters
    const auto& thickCfg = spawnerCfg.thickness;
    if (id == "thickness.mode") { out = static_cast<int>(thickCfg.mode); return true; }
    if (id == "thickness.base") { out = thickCfg.basePx; return true; }
    if (id == "thickness.scale") { out = thickCfg.scalePx; return true; }
    
    // Spring parameters
    const auto& springCfg = spawnerCfg.spring;
    if (id == "spring.enabled") { out = springCfg.enabled; return true; }
    if (id == "spring.k") { out = springCfg.k; return true; }
    if (id == "spring.damping") { out = springCfg.damping; return true; }
    if (id == "spring.useDelay") { out = spawnerCfg.useDelay; return true; }
    
    // Particle parameters
    const auto& particleCfg = m_equalizer.particleConfig();
    if (id == "particle.spawn") { out = particleCfg.spawnEachPeak; return true; }
    if (id == "particle.minDelta") { out = particleCfg.minDelta; return true; }
    if (id == "particle.minInterval") { out = particleCfg.minIntervalMs; return true; }
    if (id == "particle.maxPerBand") { out = particleCfg.maxPerBand; return true; }
    if (id == "particle.freezeColor") { out = particleCfg.colorFreezeParticles; return true; }
    if (id == "particle.bindToSpawner") { out = particleCfg.colorBoundToSpawner; return true; }
    
    // Peak color parameters
    const auto& peakColorCfg = m_equalizer.peakColorConfig();
    if (id == "peakColor.auto") { out = peakColorCfg.autoColor; return true; }
    if (id == "peakColor.fixed") { out.emplace<7>(peakColorCfg.fixedColor); return true; }
    if (id == "peakColor.freeze") { out = peakColorCfg.freezeSpawner; return true; }
    
    return false;
}

bool EqualizerVisualizer::setParam(const std::string& id, const ParamValue& value)
{
    // Audio parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }
    
    // Color parameters
    if (id == "color.domain")
    {
        if (auto* v = std::get_if<int>(&value))
        {
            m_equalizer.setGradientDomain(static_cast<GradientDomain>(*v));
            return true;
        }
        return false;
    }
    if (id.rfind("color.", 0) == 0)
    {
        return m_equalizer.colorGradient().setParam(id.substr(6), value);
    }
    
    // Helper lambdas
    auto getInt = [&]() -> int {
        if (auto* v = std::get_if<int>(&value)) return *v;
        if (auto* v = std::get_if<float>(&value)) return static_cast<int>(*v);
        return 0;
    };
    auto getFloat = [&]() -> float {
        if (auto* v = std::get_if<float>(&value)) return *v;
        if (auto* v = std::get_if<int>(&value)) return static_cast<float>(*v);
        return 0.0f;
    };
    auto getBool = [&]() -> bool {
        if (auto* v = std::get_if<bool>(&value)) return *v;
        if (auto* v = std::get_if<int>(&value)) return *v != 0;
        return false;
    };
    auto getColor = [&]() -> Color4f {
        // Color4f is at index 7 in ParamValue variant
        if (value.index() == 7) return std::get<7>(value);
        return Color4f{1, 1, 1, 1};
    };
    
    // Equalizer parameters
    if (id == "eq.bands") 
    { 
        int bands = getInt();
        m_equalizer.setBandCount(bands); 
        m_audioSource.setBands(bands);  // Keep AudioSourceModule in sync
        return true; 
    }
    if (id == "eq.barGap") { m_equalizer.setBarGapPx(getFloat()); return true; }
    if (id == "eq.orientation") { m_equalizer.setOrientation(static_cast<BarOrientation>(getInt())); return true; }
    
    // Peak parameters
    auto& spawnerCfg = m_equalizer.spawnerConfig();
    if (id == "peak.enabled") { spawnerCfg.enabled = getBool(); return true; }
    if (id == "peak.holdDelay") { spawnerCfg.delayMs = getFloat(); return true; }
    if (id == "peak.gravity") { spawnerCfg.gravity = getFloat(); return true; }
    if (id == "peak.falloff") { spawnerCfg.falloffPerSec = getFloat(); return true; }
    if (id == "peak.bounce") { spawnerCfg.bounceElasticity = getFloat(); return true; }
    if (id == "peak.respawnOnLeave") { spawnerCfg.respawnOnLeave = getBool(); return true; }
    if (id == "peak.behind") { m_equalizer.setDrawPeaksBehindBars(getBool()); return true; }
    
    // Thickness parameters
    auto& thickCfg = spawnerCfg.thickness;
    if (id == "thickness.mode") { thickCfg.mode = static_cast<ThicknessMode>(getInt()); return true; }
    if (id == "thickness.base") { thickCfg.basePx = getFloat(); return true; }
    if (id == "thickness.scale") { thickCfg.scalePx = getFloat(); return true; }
    
    // Spring parameters
    auto& springCfg = spawnerCfg.spring;
    if (id == "spring.enabled") { springCfg.enabled = getBool(); return true; }
    if (id == "spring.k") { springCfg.k = getFloat(); return true; }
    if (id == "spring.damping") { springCfg.damping = getFloat(); return true; }
    if (id == "spring.useDelay") { spawnerCfg.useDelay = getBool(); return true; }
    
    // Particle parameters
    auto& particleCfg = m_equalizer.particleConfig();
    if (id == "particle.spawn") { particleCfg.spawnEachPeak = getBool(); return true; }
    if (id == "particle.minDelta") { particleCfg.minDelta = getFloat(); return true; }
    if (id == "particle.minInterval") { particleCfg.minIntervalMs = getFloat(); return true; }
    if (id == "particle.maxPerBand") { particleCfg.maxPerBand = getInt(); return true; }
    if (id == "particle.freezeColor") { particleCfg.colorFreezeParticles = getBool(); return true; }
    if (id == "particle.bindToSpawner") { particleCfg.colorBoundToSpawner = getBool(); return true; }
    
    // Peak color parameters
    auto& peakColorCfg = m_equalizer.peakColorConfig();
    if (id == "peakColor.auto") { peakColorCfg.autoColor = getBool(); return true; }
    if (id == "peakColor.fixed") { peakColorCfg.fixedColor = getColor(); return true; }
    if (id == "peakColor.freeze") { peakColorCfg.freezeSpawner = getBool(); return true; }
    
    return false;
}

void EqualizerVisualizer::resetToDefaults()
{
    m_audioSource.resetToDefaults();
    m_equalizer.resetToDefaults();
    
    // Sync AudioSourceModule bands with Equalizer bands
    m_audioSource.setBands(m_equalizer.bandCount());
}

// =============================================================================
// IVisualizer Audio Interface
// =============================================================================

void EqualizerVisualizer::updateSpectrum(const float* spectrum, int count)
{
    if (!spectrum || count <= 0)
        return;

    QMutexLocker lock(&m_spectrumMutex);
    
    if (static_cast<size_t>(count) > m_spectrumData.size())
        m_spectrumData.resize(count);
    
    std::copy(spectrum, spectrum + count, m_spectrumData.begin());
    m_spectrumCount = count;
    m_hasNewSpectrum = true;
}

// =============================================================================
// VisualizerBase Implementation
// =============================================================================

void EqualizerVisualizer::onInitialize()
{
    BasicLogger::logInfo("EqualizerVisualizer: Initializing...");
    
    auto* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        BasicLogger::logWarning("EqualizerVisualizer: No OpenGL context");
        return;
    }

    m_lastContext = ctx;
    auto* f = ctx->functions();

    // Create shaders
    if (!createShaders())
    {
        BasicLogger::logWarning("EqualizerVisualizer: Failed to create shaders");
        return;
    }

    // =========================================================================
    // Bar VAO/VBO (gradient-based: pos.xy + gradientT)
    // =========================================================================
    m_barVao = std::make_unique<QOpenGLVertexArrayObject>();
    if (!m_barVao->create())
    {
        BasicLogger::logWarning("EqualizerVisualizer: Failed to create bar VAO");
        return;
    }

    m_barVertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    if (!m_barVertexBuffer->create())
    {
        BasicLogger::logWarning("EqualizerVisualizer: Failed to create bar VBO");
        return;
    }
    m_barVertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    m_barVao->bind();
    m_barVertexBuffer->bind();
    
    // Pre-allocate: Max 256 bands * 6 verts * 3 floats (pos.xy + gradientT)
    m_barVertexBuffer->allocate(256 * 6 * 3 * sizeof(float));

    // Position (vec2) + GradientT (float) = 3 floats per vertex
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 
                             reinterpret_cast<void*>(2 * sizeof(float)));

    m_barVertexBuffer->release();
    m_barVao->release();

    // =========================================================================
    // Peak VAO/VBO (direct color: pos.xy + color.rgba)
    // =========================================================================
    m_peakVao = std::make_unique<QOpenGLVertexArrayObject>();
    if (!m_peakVao->create())
    {
        BasicLogger::logWarning("EqualizerVisualizer: Failed to create peak VAO");
        return;
    }

    m_peakVertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    if (!m_peakVertexBuffer->create())
    {
        BasicLogger::logWarning("EqualizerVisualizer: Failed to create peak VBO");
        return;
    }
    m_peakVertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    m_peakVao->bind();
    m_peakVertexBuffer->bind();
    
    // Pre-allocate: Max 256 bands * 6 verts * 6 floats (pos.xy + color.rgba)
    // Plus particles: 2048 * 6 verts * 6 floats
    m_peakVertexBuffer->allocate((256 + 2048) * 6 * 6 * sizeof(float));

    // Position (vec2) + Color (vec4) = 6 floats per vertex
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 
                             reinterpret_cast<void*>(2 * sizeof(float)));

    m_peakVertexBuffer->release();
    m_peakVao->release();

    // Enable blending
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    BasicLogger::logInfo("EqualizerVisualizer: Initialized successfully");
}

void EqualizerVisualizer::onRender(float deltaTime)
{
    auto* ctx = QOpenGLContext::currentContext();
    if (!ctx || !m_barShader || !m_barVao || !m_peakShader || !m_peakVao)
        return;

    if (ctx != m_lastContext)
    {
        m_lastContext = ctx;
    }

    auto* f = ctx->functions();
    m_totalTime += deltaTime;

    // Get spectrum data
    std::vector<float> spectrum;
    int spectrumCount = 0;
    {
        QMutexLocker lock(&m_spectrumMutex);
        spectrum = m_spectrumData;
        spectrumCount = m_spectrumCount;
        m_hasNewSpectrum = false;
    }

    // Process through AudioSourceModule (applies gain, mapping, smoothing, normalization)
    if (spectrumCount > 0)
    {
        // Sync AudioSourceModule bands with Equalizer bands
        if (m_audioSource.bandCount() != m_equalizer.bandCount())
        {
            m_audioSource.setBands(m_equalizer.bandCount());
        }
        
        // Process raw spectrum through AudioSourceModule
        m_audioSource.update(spectrum.data(), spectrumCount, deltaTime);
        
        // Pass processed data to Equalizer (skips internal mapping/smoothing/normalization)
        m_equalizer.updateFromProcessed(m_audioSource.spectrum(), 
                                         m_audioSource.bandCount(), 
                                         deltaTime);
    }

    // Clear background
    f->glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);

    // Render based on z-order setting
    if (m_equalizer.drawPeaksBehindBars())
    {
        renderPeaks();
        renderParticles();
        renderBars();
    }
    else
    {
        renderBars();
        renderPeaks();
        renderParticles();
    }
}

void EqualizerVisualizer::onResize(const QSize& size)
{
    auto* ctx = QOpenGLContext::currentContext();
    if (ctx)
    {
        ctx->functions()->glViewport(0, 0, size.width(), size.height());
    }
}

void EqualizerVisualizer::onCleanup()
{
    BasicLogger::logInfo("EqualizerVisualizer: Cleaning up...");
    m_barShader.reset();
    m_barVertexBuffer.reset();
    m_barVao.reset();
    m_peakShader.reset();
    m_peakVertexBuffer.reset();
    m_peakVao.reset();
    m_lastContext = nullptr;
}

// =============================================================================
// Private Methods
// =============================================================================

bool EqualizerVisualizer::createShaders()
{
    // =========================================================================
    // Bar Shader (gradient-based)
    // =========================================================================
    m_barShader = std::make_unique<QOpenGLShaderProgram>();
    
    if (!m_barShader->addShaderFromSourceCode(QOpenGLShader::Vertex, BAR_VERTEX_SHADER))
    {
        BasicLogger::logWarning("EqualizerVisualizer: Bar vertex shader failed: " +
                                m_barShader->log().toStdString());
        return false;
    }
    if (!m_barShader->addShaderFromSourceCode(QOpenGLShader::Fragment, BAR_FRAGMENT_SHADER))
    {
        BasicLogger::logWarning("EqualizerVisualizer: Bar fragment shader failed: " +
                                m_barShader->log().toStdString());
        return false;
    }
    if (!m_barShader->link())
    {
        BasicLogger::logWarning("EqualizerVisualizer: Bar shader linking failed: " +
                                m_barShader->log().toStdString());
        return false;
    }
    
    // Get bar shader uniform locations
    for (int i = 0; i < 8; ++i)
    {
        m_uniColor[i] = m_barShader->uniformLocation(QString("uColor%1").arg(i));
    }
    m_uniStopPos = m_barShader->uniformLocation("uStopPos");
    m_uniStopPos2 = m_barShader->uniformLocation("uStopPos2");
    m_uniStopCount = m_barShader->uniformLocation("uStopCount");
    m_uniGradientMode = m_barShader->uniformLocation("uGradientMode");
    m_uniGradientAngle = m_barShader->uniformLocation("uGradientAngle");
    m_uniAlpha = m_barShader->uniformLocation("uAlpha");

    // =========================================================================
    // Peak Shader (direct color per vertex)
    // =========================================================================
    m_peakShader = std::make_unique<QOpenGLShaderProgram>();
    
    if (!m_peakShader->addShaderFromSourceCode(QOpenGLShader::Vertex, PEAK_VERTEX_SHADER))
    {
        BasicLogger::logWarning("EqualizerVisualizer: Peak vertex shader failed: " +
                                m_peakShader->log().toStdString());
        return false;
    }
    if (!m_peakShader->addShaderFromSourceCode(QOpenGLShader::Fragment, PEAK_FRAGMENT_SHADER))
    {
        BasicLogger::logWarning("EqualizerVisualizer: Peak fragment shader failed: " +
                                m_peakShader->log().toStdString());
        return false;
    }
    if (!m_peakShader->link())
    {
        BasicLogger::logWarning("EqualizerVisualizer: Peak shader linking failed: " +
                                m_peakShader->log().toStdString());
        return false;
    }
    
    m_peakUniAlpha = m_peakShader->uniformLocation("uAlpha");

    return true;
}

void EqualizerVisualizer::uploadGradientUniforms()
{
    const auto& grad = m_equalizer.colorGradient();
    
    // Upload color stops
    auto stops = grad.stops();
    int stopCount = std::min(static_cast<int>(stops.size()), 8);
    
    for (int i = 0; i < 8; ++i)
    {
        if (i < stopCount)
        {
            const auto& c = stops[i].color;
            m_barShader->setUniformValue(m_uniColor[i], c[0], c[1], c[2], c[3]);
        }
        else
        {
            m_barShader->setUniformValue(m_uniColor[i], 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
    
    // Upload stop positions
    float pos[8] = {0, 0.14f, 0.28f, 0.42f, 0.57f, 0.71f, 0.85f, 1.0f};
    for (int i = 0; i < stopCount && i < 8; ++i)
    {
        pos[i] = stops[i].position;
    }
    m_barShader->setUniformValue(m_uniStopPos, pos[0], pos[1], pos[2], pos[3]);
    m_barShader->setUniformValue(m_uniStopPos2, pos[4], pos[5], pos[6], pos[7]);
    
    // Upload other uniforms
    m_barShader->setUniformValue(m_uniStopCount, stopCount);
    m_barShader->setUniformValue(m_uniGradientMode, static_cast<int>(grad.mode()));
    m_barShader->setUniformValue(m_uniGradientAngle, grad.angle());
    m_barShader->setUniformValue(m_uniAlpha, 1.0f);
}

void EqualizerVisualizer::renderBars()
{
    auto* ctx = QOpenGLContext::currentContext();
    if (!ctx || !m_barShader || !m_barVao)
        return;

    auto* f = ctx->functions();

    const auto& bands = m_equalizer.bands();
    int bandCount = m_equalizer.bandCount();

    if (bandCount <= 0 || bands.empty())
        return;

    float w = static_cast<float>(width());
    float gap = m_equalizer.barGapPx();
    
    // Calculate bar width in pixels, then convert to normalized coords
    float totalGaps = static_cast<float>(bandCount - 1) * gap;
    float barWidthPx = (w - totalGaps) / static_cast<float>(bandCount);
    barWidthPx = std::max(1.0f, barWidthPx);
    
    // Normalized bar width and gap
    float barWidthNorm = (barWidthPx / w) * 2.0f;
    float gapNorm = (gap / w) * 2.0f;

    // Build vertex data: 6 vertices per bar (2 triangles)
    // Each vertex: pos.x, pos.y, gradientT
    std::vector<float> vertices;
    vertices.reserve(bandCount * 6 * 3);

    bool topDown = (m_equalizer.orientation() == BarOrientation::TopDown);

    for (int i = 0; i < bandCount; ++i)
    {
        float amplitude = bands[i];
        
        // Gradient parameter based on domain
        float gradT;
        GradientDomain domain = m_equalizer.gradientDomain();
        if (domain == GradientDomain::Amplitude)
        {
            gradT = amplitude;
        }
        else
        {
            // Position-based (default)
            gradT = static_cast<float>(i) / static_cast<float>(bandCount - 1);
        }

        // Normalized X position (-1 to 1)
        float x0 = -1.0f + static_cast<float>(i) * (barWidthNorm + gapNorm);
        float x1 = x0 + barWidthNorm;
        
        // Normalized Y position (-1 to 1)
        float y0, y1;
        if (topDown)
        {
            y0 = 1.0f;
            y1 = 1.0f - amplitude * 2.0f;
        }
        else
        {
            y0 = -1.0f;
            y1 = -1.0f + amplitude * 2.0f;
        }

        // Triangle 1
        vertices.push_back(x0); vertices.push_back(y0); vertices.push_back(gradT);
        vertices.push_back(x1); vertices.push_back(y0); vertices.push_back(gradT);
        vertices.push_back(x0); vertices.push_back(y1); vertices.push_back(gradT);

        // Triangle 2
        vertices.push_back(x1); vertices.push_back(y0); vertices.push_back(gradT);
        vertices.push_back(x1); vertices.push_back(y1); vertices.push_back(gradT);
        vertices.push_back(x0); vertices.push_back(y1); vertices.push_back(gradT);
    }

    if (vertices.empty())
        return;

    // Upload and draw
    m_barVao->bind();
    m_barVertexBuffer->bind();
    m_barVertexBuffer->allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    m_barShader->bind();
    uploadGradientUniforms();

    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size() / 3));

    m_barShader->release();
    m_barVertexBuffer->release();
    m_barVao->release();
}

void EqualizerVisualizer::renderPeaks()
{
    if (!m_equalizer.spawnerConfig().enabled)
        return;

    auto* ctx = QOpenGLContext::currentContext();
    if (!ctx || !m_peakShader || !m_peakVao)
        return;

    auto* f = ctx->functions();

    const auto& spawners = m_equalizer.spawners();
    int bandCount = m_equalizer.bandCount();

    if (bandCount <= 0 || spawners.empty())
        return;

    float w = static_cast<float>(width());
    float h = static_cast<float>(height());
    float gap = m_equalizer.barGapPx();
    
    float totalGaps = static_cast<float>(bandCount - 1) * gap;
    float barWidthPx = (w - totalGaps) / static_cast<float>(bandCount);
    barWidthPx = std::max(1.0f, barWidthPx);
    
    float barWidthNorm = (barWidthPx / w) * 2.0f;
    float gapNorm = (gap / w) * 2.0f;

    bool topDown = (m_equalizer.orientation() == BarOrientation::TopDown);

    // Build vertex data for peak markers
    // Each vertex: pos.x, pos.y, color.r, color.g, color.b, color.a (6 floats)
    std::vector<float> vertices;
    vertices.reserve(bandCount * 6 * 6);

    for (int i = 0; i < bandCount; ++i)
    {
        const auto& s = spawners[i];
        
        // Thickness in pixels, then normalized
        float thicknessPx = m_equalizer.calcPeakThickness(s.position) * 
                           m_equalizer.spawnerConfig().thickness.maxPx;
        thicknessPx = std::max(2.0f, thicknessPx);
        float thicknessNorm = (thicknessPx / h) * 2.0f;
        
        float x0 = -1.0f + static_cast<float>(i) * (barWidthNorm + gapNorm);
        float x1 = x0 + barWidthNorm;
        
        // Peak Y position
        float peakY;
        if (topDown)
        {
            peakY = 1.0f - s.position * 2.0f;
        }
        else
        {
            peakY = -1.0f + s.position * 2.0f;
        }

        float y0 = peakY - thicknessNorm * 0.5f;
        float y1 = peakY + thicknessNorm * 0.5f;

        // Use the spawner's stored color (set when touching bar)
        const auto& c = s.color;

        // Triangle 1
        vertices.push_back(x0); vertices.push_back(y0); 
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
        
        vertices.push_back(x1); vertices.push_back(y0);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
        
        vertices.push_back(x0); vertices.push_back(y1);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);

        // Triangle 2
        vertices.push_back(x1); vertices.push_back(y0);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
        
        vertices.push_back(x1); vertices.push_back(y1);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
        
        vertices.push_back(x0); vertices.push_back(y1);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
    }

    if (vertices.empty())
        return;

    m_peakVao->bind();
    m_peakVertexBuffer->bind();
    m_peakVertexBuffer->allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    m_peakShader->bind();
    m_peakShader->setUniformValue(m_peakUniAlpha, 1.0f);

    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size() / 6));

    m_peakShader->release();
    m_peakVertexBuffer->release();
    m_peakVao->release();
}

void EqualizerVisualizer::renderParticles()
{
    const auto& particles = m_equalizer.particles();
    if (particles.empty())
        return;

    auto* ctx = QOpenGLContext::currentContext();
    if (!ctx || !m_peakShader || !m_peakVao)
        return;

    auto* f = ctx->functions();

    int bandCount = m_equalizer.bandCount();
    float w = static_cast<float>(width());
    float h = static_cast<float>(height());
    float gap = m_equalizer.barGapPx();
    
    float totalGaps = static_cast<float>(bandCount - 1) * gap;
    float barWidthPx = (w - totalGaps) / static_cast<float>(bandCount);
    barWidthPx = std::max(1.0f, barWidthPx);
    
    float barWidthNorm = (barWidthPx / w) * 2.0f;
    float gapNorm = (gap / w) * 2.0f;

    bool topDown = (m_equalizer.orientation() == BarOrientation::TopDown);

    // Each vertex: pos.x, pos.y, color.r, color.g, color.b, color.a (6 floats)
    std::vector<float> vertices;
    vertices.reserve(particles.size() * 6 * 6);

    for (const auto& p : particles)
    {
        if (!p.alive || p.bandIndex < 0 || p.bandIndex >= bandCount)
            continue;

        float thicknessPx = m_equalizer.calcPeakThickness(p.position) * 
                           m_equalizer.spawnerConfig().thickness.maxPx * 0.5f;
        thicknessPx = std::max(1.0f, thicknessPx);
        float thicknessNorm = (thicknessPx / h) * 2.0f;
        
        float x0 = -1.0f + static_cast<float>(p.bandIndex) * (barWidthNorm + gapNorm);
        float x1 = x0 + barWidthNorm;
        
        float particleY;
        if (topDown)
        {
            particleY = 1.0f - p.position * 2.0f;
        }
        else
        {
            particleY = -1.0f + p.position * 2.0f;
        }

        float y0 = particleY - thicknessNorm * 0.5f;
        float y1 = particleY + thicknessNorm * 0.5f;

        // Use the particle's stored color
        const auto& c = p.color;

        // Triangle 1
        vertices.push_back(x0); vertices.push_back(y0);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
        
        vertices.push_back(x1); vertices.push_back(y0);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
        
        vertices.push_back(x0); vertices.push_back(y1);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);

        // Triangle 2
        vertices.push_back(x1); vertices.push_back(y0);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
        
        vertices.push_back(x1); vertices.push_back(y1);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
        
        vertices.push_back(x0); vertices.push_back(y1);
        vertices.push_back(c[0]); vertices.push_back(c[1]); vertices.push_back(c[2]); vertices.push_back(c[3]);
    }

    if (vertices.empty())
        return;

    m_peakVao->bind();
    m_peakVertexBuffer->bind();
    m_peakVertexBuffer->allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    m_peakShader->bind();
    m_peakShader->setUniformValue(m_peakUniAlpha, 1.0f);

    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size() / 6));

    m_peakShader->release();
    m_peakVertexBuffer->release();
    m_peakVao->release();
}
