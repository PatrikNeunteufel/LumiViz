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
#include "visualizers/VisualizerPresetManager.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QMutexLocker>

#include <BasicLogger.h>

#include <cmath>
#include <algorithm>
#include <map>
#include <string>

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

// =============================================================================
// Legacy-Key-Migration (Phase 4 Schritt 5.1)
// =============================================================================

/**
 * @brief Alias map old→new schema (Parameter_Key_Migration.md §6/§7.1)
 *
 * Applies to presets with formatVersion < 2. Unchanged keys are registered
 * as identity entries so the map doubles as the loader's key whitelist (§9).
 */
void registerLegacyKeyAliases()
{
    std::map<std::string, std::string> aliases;

    // Stage 1: audio.* unchanged (identity) — WITHOUT audio.bands (E2, below)
    for (const char* key : {"preset", "scale", "floorDb", "ceilDb", "clamp01", "gain",
                            "smooth.preset", "smooth.algorithm", "smooth.timeMs",
                            "smooth.windowSize", "smooth.primeFirstFrame"})
    {
        aliases.emplace(std::string("audio.") + key, std::string("audio.") + key);
    }

    // E2: ONE key map.bands replaces the eq.bands ↔ audio.bands pair. Both old
    // keys translate to it; on conflicting values eq.bands wins (§7.1) because
    // preset.parameters iterates in key order — "audio.bands" < "eq.bands",
    // so eq.bands is applied last.
    aliases.emplace("eq.bands", "map.bands");
    aliases.emplace("audio.bands", "map.bands");
    aliases.emplace("eq.orientation", "map.orientation");
    aliases.emplace("eq.barGap", "render.barGap");
    // render.heightScale is NEW (E1) — no legacy key, nothing to register

    // Stage 3: color.* → color.main.* (domain + the 8 gradient sub-keys)
    aliases.emplace("color.domain", "color.main.domain");
    for (const char* key : {"mode", "solidColor", "angle", "preset", "editGradient",
                            "outlineWidth", "gradientPresetName", "gradientData"})
    {
        aliases.emplace(std::string("color.") + key, std::string("color.main.") + key);
    }

    // Stage 5: peak.* / particle.* unchanged (identity)
    for (const char* key : {"peak.enabled", "peak.holdDelay", "peak.gravity",
                            "peak.falloff", "peak.bounce", "peak.respawnOnLeave",
                            "peak.behind", "particle.spawn", "particle.minDelta",
                            "particle.minInterval", "particle.maxPerBand",
                            "particle.freezeColor", "particle.bindToSpawner"})
    {
        aliases.emplace(key, key);
    }

    // Stage 5: thickness/spring/peakColor pulled under peak.*
    aliases.emplace("thickness.mode", "peak.thickness.mode");
    aliases.emplace("thickness.base", "peak.thickness.base");
    aliases.emplace("thickness.scale", "peak.thickness.scale");
    aliases.emplace("spring.enabled", "peak.spring.enabled");
    aliases.emplace("spring.k", "peak.spring.k");
    aliases.emplace("spring.damping", "peak.spring.damping");
    aliases.emplace("spring.useDelay", "peak.spring.useDelay");
    aliases.emplace("peakColor.auto", "peak.color.auto");
    aliases.emplace("peakColor.fixed", "peak.color.fixed");
    aliases.emplace("peakColor.freeze", "peak.color.freeze");

    lumi::VisualizerPresetManager::registerKeyAliases(QStringLiteral("equalizer"),
                                                      std::move(aliases));
}

} // anonymous namespace

// =============================================================================
// Constructor / Destructor
// =============================================================================

EqualizerVisualizer::EqualizerVisualizer()
    : VisualizerBase(QStringLiteral("equalizer"),
                     QObject::tr("Equalizer"),
                     QObject::tr("Spectrum analyzer with bars and peak markers"))
{
    // Idempotent on every construction — a magic static would not survive
    // clearKeyAliases() (tests) since the registration would never re-fire
    registerLegacyKeyAliases();

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
    // Stage 1: Audio Source
    // =========================================================================
    for (const auto& p : m_audioSource.paramDescs())
    {
        // Skip "bands" - replaced by map.bands (E2), which drives both modules
        if (p.id == "bands")
            continue;

        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "Audio";
        prefixed.stage = PipelineStage::AudioSource;
        // Keep original order from AudioSourceModule for consistent UI layout

        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "audio." + prefixed.dependsOn;
        }

        params.push_back(prefixed);
    }

    // =========================================================================
    // Stage 2: Mapping
    // =========================================================================

    // Bands (replaces the old eq.bands ↔ audio.bands pair, E2)
    {
        ModuleParamDesc p;
        p.id = "map.bands";
        p.displayName = "Bands";
        p.group = "Mapping";
        p.stage = PipelineStage::Mapping;
        p.type = ParamType::Int;
        p.defaultValue = 64;
        p.minValue = 8;
        p.maxValue = 256;
        p.step = 1;
        p.tooltip = "Number of frequency bands";
        p.order = 0;
        params.push_back(p);
    }

    // Orientation
    {
        ModuleParamDesc p;
        p.id = "map.orientation";
        p.displayName = "Orientation";
        p.group = "Mapping";
        p.stage = PipelineStage::Mapping;
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Bottom Up", "Top Down"};
        p.tooltip = "Bar growth direction";
        p.order = 1;
        params.push_back(p);
    }

    // =========================================================================
    // Stage 3: Color
    // =========================================================================

    // Gradient Domain (Position, Amplitude, Time, Beat)
    {
        ModuleParamDesc p;
        p.id = "color.main.domain";
        p.displayName = "Color Mapping";
        p.group = "Color";
        p.stage = PipelineStage::Color;
        p.type = ParamType::Enum;
        p.defaultValue = 0;  // Position
        p.enumOptions = {"Position", "Amplitude", "Time", "Beat"};
        p.tooltip = "What drives the gradient color";
        p.order = 0;
        params.push_back(p);
    }

    // Color Gradient Parameters (from ColorGradientModule), handle "main"
    for (const auto& p : m_equalizer.colorGradient().paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "color.main." + p.id;
        prefixed.group = "Color";
        prefixed.stage = PipelineStage::Color;
        prefixed.order = 10 + p.order;  // After domain

        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "color.main." + prefixed.dependsOn;
        }

        params.push_back(prefixed);
    }

    // =========================================================================
    // Stage 4: Render
    // =========================================================================

    // Height Scale (NEW, E1 — display scaling of bar heights, no legacy key)
    {
        ModuleParamDesc p;
        p.id = "render.heightScale";
        p.displayName = "Height Scale";
        p.group = "Render";
        p.stage = PipelineStage::Render;
        p.type = ParamType::Float;
        p.defaultValue = 1.0f;
        p.minValue = 0.0f;
        p.maxValue = 4.0f;
        p.step = 0.05f;
        p.tooltip = "Display scaling of bar heights";
        p.order = 0;
        params.push_back(p);
    }

    // Bar Gap
    {
        ModuleParamDesc p;
        p.id = "render.barGap";
        p.displayName = "Bar Gap";
        p.group = "Render";
        p.stage = PipelineStage::Render;
        p.type = ParamType::Float;
        p.defaultValue = 2.0f;
        p.minValue = 0.0f;
        p.maxValue = 20.0f;
        p.unit = "px";
        p.tooltip = "Gap between bars";
        p.order = 1;
        params.push_back(p);
    }

    // =========================================================================
    // Stage 5: Peak / Particles (sub-groups: Peak Hold, Thickness, Spring
    // Physics, Particles, Peak Color — ordered via order offsets 0/10/20/30/40)
    // =========================================================================

    // --- Peak Hold (spawner) ---

    // Enable Peaks
    {
        ModuleParamDesc p;
        p.id = "peak.enabled";
        p.displayName = "Enable Peaks";
        p.group = "Peaks";
        p.subGroup = "Peak Hold";
        p.stage = PipelineStage::PeakParticle;
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
        p.group = "Peaks";
        p.subGroup = "Peak Hold";
        p.stage = PipelineStage::PeakParticle;
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
        p.group = "Peaks";
        p.subGroup = "Peak Hold";
        p.stage = PipelineStage::PeakParticle;
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
        p.group = "Peaks";
        p.subGroup = "Peak Hold";
        p.stage = PipelineStage::PeakParticle;
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
        p.group = "Peaks";
        p.subGroup = "Peak Hold";
        p.stage = PipelineStage::PeakParticle;
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
        p.group = "Peaks";
        p.subGroup = "Peak Hold";
        p.stage = PipelineStage::PeakParticle;
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
        p.group = "Peaks";
        p.subGroup = "Peak Hold";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Draw peak markers behind bars";
        p.order = 6;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // --- Thickness (peak markers) ---

    // Thickness Mode
    {
        ModuleParamDesc p;
        p.id = "peak.thickness.mode";
        p.displayName = "Thickness Mode";
        p.group = "Peaks";
        p.subGroup = "Thickness";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Enum;
        p.defaultValue = 0;  // Fixed
        p.enumOptions = {"Fixed", "Direct (thicker at high)", "Inverse (thicker at low)"};
        p.tooltip = "How peak thickness changes with amplitude";
        p.order = 10;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Base Thickness
    {
        ModuleParamDesc p;
        p.id = "peak.thickness.base";
        p.displayName = "Base Thickness";
        p.group = "Peaks";
        p.subGroup = "Thickness";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Float;
        p.defaultValue = 2.0f;
        p.minValue = 1.0f;
        p.maxValue = 20.0f;
        p.unit = "px";
        p.tooltip = "Base thickness in pixels";
        p.order = 11;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Scale Thickness (only when mode != Off)
    {
        ModuleParamDesc p;
        p.id = "peak.thickness.scale";
        p.displayName = "Thickness Scale";
        p.group = "Peaks";
        p.subGroup = "Thickness";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Float;
        p.defaultValue = 4.0f;
        p.minValue = 0.0f;
        p.maxValue = 20.0f;
        p.unit = "px";
        p.tooltip = "Additional thickness based on amplitude";
        p.order = 12;
        p.dependsOn = "peak.thickness.mode";
        p.dependsValues = {1, 2};  // Direct, Inverse
        params.push_back(p);
    }

    // --- Spring Physics ---

    // Enable Spring
    {
        ModuleParamDesc p;
        p.id = "peak.spring.enabled";
        p.displayName = "Enable Spring";
        p.group = "Peaks";
        p.subGroup = "Spring Physics";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Use spring physics instead of gravity";
        p.order = 20;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Spring Constant
    {
        ModuleParamDesc p;
        p.id = "peak.spring.k";
        p.displayName = "Spring Constant";
        p.group = "Peaks";
        p.subGroup = "Spring Physics";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Float;
        p.defaultValue = 40.0f;
        p.minValue = 1.0f;
        p.maxValue = 200.0f;
        p.tooltip = "Spring stiffness (higher = faster oscillation)";
        p.order = 21;
        p.dependsOn = "peak.spring.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Spring Damping
    {
        ModuleParamDesc p;
        p.id = "peak.spring.damping";
        p.displayName = "Spring Damping";
        p.group = "Peaks";
        p.subGroup = "Spring Physics";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Float;
        p.defaultValue = 10.0f;
        p.minValue = 0.0f;
        p.maxValue = 50.0f;
        p.tooltip = "Damping (higher = less oscillation)";
        p.order = 22;
        p.dependsOn = "peak.spring.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Use Delay in Spring Mode
    {
        ModuleParamDesc p;
        p.id = "peak.spring.useDelay";
        p.displayName = "Use Hold Delay";
        p.group = "Peaks";
        p.subGroup = "Spring Physics";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.tooltip = "Apply hold delay before spring starts";
        p.order = 23;
        p.dependsOn = "peak.spring.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // --- Particles ---

    // Spawn Particles
    {
        ModuleParamDesc p;
        p.id = "particle.spawn";
        p.displayName = "Spawn Particles";
        p.group = "Peaks";
        p.subGroup = "Particles";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Spawn particles on peak hits";
        p.order = 30;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Min Delta to Spawn
    {
        ModuleParamDesc p;
        p.id = "particle.minDelta";
        p.displayName = "Min Rise";
        p.group = "Peaks";
        p.subGroup = "Particles";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Float;
        p.defaultValue = 0.0f;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.tooltip = "Minimum amplitude rise to spawn a particle";
        p.order = 31;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Min Interval
    {
        ModuleParamDesc p;
        p.id = "particle.minInterval";
        p.displayName = "Min Interval";
        p.group = "Peaks";
        p.subGroup = "Particles";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Float;
        p.defaultValue = 0.0f;
        p.minValue = 0.0f;
        p.maxValue = 1000.0f;
        p.unit = "ms";
        p.tooltip = "Minimum time between particle spawns";
        p.order = 32;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Max Particles Per Band
    {
        ModuleParamDesc p;
        p.id = "particle.maxPerBand";
        p.displayName = "Max Per Band";
        p.group = "Peaks";
        p.subGroup = "Particles";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Int;
        p.defaultValue = 8;
        p.minValue = 1;
        p.maxValue = 32;
        p.tooltip = "Maximum particles per frequency band";
        p.order = 33;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Freeze Particle Color
    {
        ModuleParamDesc p;
        p.id = "particle.freezeColor";
        p.displayName = "Freeze Color";
        p.group = "Peaks";
        p.subGroup = "Particles";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Keep color from spawn moment";
        p.order = 34;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Bind Particle Color to Spawner
    {
        ModuleParamDesc p;
        p.id = "particle.bindToSpawner";
        p.displayName = "Bind to Spawner";
        p.group = "Peaks";
        p.subGroup = "Particles";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Particle follows spawner color live";
        p.order = 35;
        p.dependsOn = "particle.spawn";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // --- Peak Color ---

    // Auto Color (use gradient)
    {
        ModuleParamDesc p;
        p.id = "peak.color.auto";
        p.displayName = "Auto Color";
        p.group = "Peaks";
        p.subGroup = "Peak Color";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.tooltip = "Use gradient for peak colors";
        p.order = 40;
        p.dependsOn = "peak.enabled";
        p.dependsValues = {true};
        params.push_back(p);
    }

    // Fixed Peak Color
    {
        ModuleParamDesc p;
        p.id = "peak.color.fixed";
        p.displayName = "Fixed Color";
        p.group = "Peaks";
        p.subGroup = "Peak Color";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Color;
        p.tooltip = "Fixed color when auto is off";
        p.order = 41;
        p.dependsOn = "peak.color.auto";
        p.dependsValues = {false};
        params.push_back(p);
    }

    // Freeze Spawner Color
    {
        ModuleParamDesc p;
        p.id = "peak.color.freeze";
        p.displayName = "Freeze Spawner Color";
        p.group = "Peaks";
        p.subGroup = "Peak Color";
        p.stage = PipelineStage::PeakParticle;
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Don't update spawner color live";
        p.order = 42;
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
    
    // Color parameters (handle "main")
    if (id == "color.main.domain")
    {
        out = static_cast<int>(m_equalizer.gradientDomain());
        return true;
    }
    if (id.rfind("color.main.", 0) == 0)
    {
        return m_equalizer.colorGradient().getParam(id.substr(11), out);
    }

    // Mapping parameters
    if (id == "map.bands") { out = m_equalizer.bandCount(); return true; }
    if (id == "map.orientation") { out = static_cast<int>(m_equalizer.orientation()); return true; }

    // Render parameters
    if (id == "render.heightScale") { out = m_equalizer.heightScale(); return true; }
    if (id == "render.barGap") { out = m_equalizer.barGapPx(); return true; }

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
    if (id == "peak.thickness.mode") { out = static_cast<int>(thickCfg.mode); return true; }
    if (id == "peak.thickness.base") { out = thickCfg.basePx; return true; }
    if (id == "peak.thickness.scale") { out = thickCfg.scalePx; return true; }

    // Spring parameters
    const auto& springCfg = spawnerCfg.spring;
    if (id == "peak.spring.enabled") { out = springCfg.enabled; return true; }
    if (id == "peak.spring.k") { out = springCfg.k; return true; }
    if (id == "peak.spring.damping") { out = springCfg.damping; return true; }
    if (id == "peak.spring.useDelay") { out = spawnerCfg.useDelay; return true; }
    
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
    if (id == "peak.color.auto") { out = peakColorCfg.autoColor; return true; }
    if (id == "peak.color.fixed") { out = makeColorValue(peakColorCfg.fixedColor); return true; }
    if (id == "peak.color.freeze") { out = peakColorCfg.freezeSpawner; return true; }
    
    return false;
}

bool EqualizerVisualizer::setParam(const std::string& id, const ParamValue& value)
{
    // Audio parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }
    
    // Color parameters (handle "main")
    if (id == "color.main.domain")
    {
        // float-für-int-Vertrag: JSON-geladene Presets liefern Zahlen als float
        if (auto* v = std::get_if<int>(&value))
        {
            m_equalizer.setGradientDomain(static_cast<GradientDomain>(*v));
            return true;
        }
        if (auto* v = std::get_if<float>(&value))
        {
            m_equalizer.setGradientDomain(static_cast<GradientDomain>(static_cast<int>(*v)));
            return true;
        }
        return false;
    }
    if (id.rfind("color.main.", 0) == 0)
    {
        return m_equalizer.colorGradient().setParam(id.substr(11), value);
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
        if (holdsColor(value)) return lumi::modules::getColor(value);
        return Color4f{1, 1, 1, 1};
    };
    
    // Mapping parameters — map.bands drives BOTH modules (buffer-resize coupling, §7.2)
    if (id == "map.bands")
    {
        int bands = getInt();
        m_equalizer.setBandCount(bands);
        m_audioSource.setBands(bands);
        return true;
    }
    if (id == "map.orientation") { m_equalizer.setOrientation(static_cast<BarOrientation>(getInt())); return true; }

    // Render parameters
    if (id == "render.heightScale") { m_equalizer.setHeightScale(getFloat()); return true; }
    if (id == "render.barGap") { m_equalizer.setBarGapPx(getFloat()); return true; }
    
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
    if (id == "peak.thickness.mode") { thickCfg.mode = static_cast<ThicknessMode>(getInt()); return true; }
    if (id == "peak.thickness.base") { thickCfg.basePx = getFloat(); return true; }
    if (id == "peak.thickness.scale") { thickCfg.scalePx = getFloat(); return true; }

    // Spring parameters
    auto& springCfg = spawnerCfg.spring;
    if (id == "peak.spring.enabled") { springCfg.enabled = getBool(); return true; }
    if (id == "peak.spring.k") { springCfg.k = getFloat(); return true; }
    if (id == "peak.spring.damping") { springCfg.damping = getFloat(); return true; }
    if (id == "peak.spring.useDelay") { spawnerCfg.useDelay = getBool(); return true; }
    
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
    if (id == "peak.color.auto") { peakColorCfg.autoColor = getBool(); return true; }
    if (id == "peak.color.fixed") { peakColorCfg.fixedColor = getColor(); return true; }
    if (id == "peak.color.freeze") { peakColorCfg.freezeSpawner = getBool(); return true; }
    
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
