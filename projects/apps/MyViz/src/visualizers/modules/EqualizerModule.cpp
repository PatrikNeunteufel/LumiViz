/**
 ****************************************************************************************
 * @file   EqualizerModule.cpp
 * @brief  Implementation of the modular equalizer/spectrum analyzer
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/EqualizerModule.hpp"

#include <cmath>
#include <algorithm>

namespace lumi::modules {

// =============================================================================
// Constructor
// =============================================================================

EqualizerModule::EqualizerModule()
{
    // Initialize gradient with default colors
    m_colorGradient.setMode(GradientMode::Linear);
    m_colorGradient.clearStops();
    m_colorGradient.addStop(0.0f, {0.0f, 0.8f, 1.0f, 1.0f});  // Cyan
    m_colorGradient.addStop(1.0f, {1.0f, 0.2f, 0.2f, 1.0f});  // Red

    resizeBands(m_bandCount);
}

// =============================================================================
// IModule Interface
// =============================================================================

std::vector<ModuleParamDesc> EqualizerModule::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    // --- General ---
    {
        ModuleParamDesc p;
        p.id = "bands";
        p.displayName = "Bands";
        p.group = "General";
        p.type = ParamType::Int;
        p.defaultValue = 64;
        p.minValue = 8;
        p.maxValue = 256;
        p.tooltip = "Number of visual bands";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "gain";
        p.displayName = "Gain";
        p.group = "General";
        p.type = ParamType::Float;
        p.defaultValue = 1.0f;
        p.minValue = 0.0f;
        p.maxValue = 4.0f;
        p.tooltip = "Amplitude multiplier";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "bar_gap_px";
        p.displayName = "Bar Gap";
        p.group = "General";
        p.type = ParamType::Float;
        p.defaultValue = 2.0f;
        p.minValue = 0.0f;
        p.maxValue = 20.0f;
        p.unit = "px";
        p.tooltip = "Pixel gap between bars";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "orientation";
        p.displayName = "Orientation";
        p.group = "General";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Bottom Up", "Top Down"};
        p.tooltip = "Bar growth direction";
        params.push_back(p);
    }

    // --- Audio ---
    {
        ModuleParamDesc p;
        p.id = "audio.scale";
        p.displayName = "Frequency Scale";
        p.group = "Audio";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Linear", "Log", "Mel"};
        p.tooltip = "Frequency to band mapping";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "audio.emaAlpha";
        p.displayName = "Smoothing";
        p.group = "Audio";
        p.type = ParamType::Float;
        p.defaultValue = 0.0f;
        p.minValue = 0.0f;
        p.maxValue = 0.99f;
        p.tooltip = "EMA smoothing factor (0=off, higher=smoother)";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "audio.floorDb";
        p.displayName = "Floor dB";
        p.group = "Audio";
        p.type = ParamType::Float;
        p.defaultValue = -60.0f;
        p.minValue = -120.0f;
        p.maxValue = 0.0f;
        p.unit = "dB";
        p.tooltip = "Level mapped to 0.0";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "audio.ceilDb";
        p.displayName = "Ceiling dB";
        p.group = "Audio";
        p.type = ParamType::Float;
        p.defaultValue = 0.0f;
        p.minValue = -60.0f;
        p.maxValue = 20.0f;
        p.unit = "dB";
        p.tooltip = "Level mapped to 1.0";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "audio.clamp01";
        p.displayName = "Clamp 0-1";
        p.group = "Audio";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.tooltip = "Clamp output to [0,1] range";
        params.push_back(p);
    }

    // --- Gradient ---
    {
        ModuleParamDesc p;
        p.id = "grad.domain";
        p.displayName = "Color Domain";
        p.group = "Gradient";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"By Position", "By Amplitude", "By Time", "By Beat"};
        p.tooltip = "Source for gradient sampling";
        params.push_back(p);
    }

    // --- Peak Spawner ---
    {
        ModuleParamDesc p;
        p.id = "peak.enabled";
        p.displayName = "Enable Peaks";
        p.group = "Peak";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.tooltip = "Show peak markers";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.delay_ms";
        p.displayName = "Hold Delay";
        p.group = "Peak";
        p.type = ParamType::Float;
        p.defaultValue = 120.0f;
        p.minValue = 0.0f;
        p.maxValue = 2000.0f;
        p.unit = "ms";
        p.tooltip = "Time to hold at peak before falling";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.falloff_per_sec";
        p.displayName = "Falloff";
        p.group = "Peak";
        p.type = ParamType::Float;
        p.defaultValue = 3.0f;
        p.minValue = 0.0f;
        p.maxValue = 20.0f;
        p.tooltip = "Linear damping (air resistance)";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.gravity";
        p.displayName = "Gravity";
        p.group = "Peak";
        p.type = ParamType::Float;
        p.defaultValue = 5.0f;
        p.minValue = -20.0f;
        p.maxValue = 20.0f;
        p.tooltip = "Acceleration (sign depends on orientation)";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.respawn_on_leave";
        p.displayName = "Respawn on Leave";
        p.group = "Peak";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Respawn at band height when leaving [0,1]";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.bounce_elasticity";
        p.displayName = "Bounce";
        p.group = "Peak";
        p.type = ParamType::Float;
        p.defaultValue = 0.25f;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.tooltip = "Elasticity when hitting boundaries";
        params.push_back(p);
    }

    // --- Peak Spring ---
    {
        ModuleParamDesc p;
        p.id = "peak.spring.enabled";
        p.displayName = "Spring Mode";
        p.group = "Peak Spring";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Use spring physics instead of hold-then-fall";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.spring.k";
        p.displayName = "Spring K";
        p.group = "Peak Spring";
        p.type = ParamType::Float;
        p.defaultValue = 40.0f;
        p.minValue = 1.0f;
        p.maxValue = 200.0f;
        p.tooltip = "Spring constant (stiffness)";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.spring.damping";
        p.displayName = "Spring Damping";
        p.group = "Peak Spring";
        p.type = ParamType::Float;
        p.defaultValue = 10.0f;
        p.minValue = 0.0f;
        p.maxValue = 50.0f;
        p.tooltip = "Spring damping (reduces oscillation)";
        params.push_back(p);
    }

    // --- Peak Thickness ---
    {
        ModuleParamDesc p;
        p.id = "peak.thick.mode";
        p.displayName = "Thickness Mode";
        p.group = "Peak Thickness";
        p.type = ParamType::Enum;
        p.defaultValue = 0;
        p.enumOptions = {"Off", "Direct", "Inverse"};
        p.tooltip = "How amplitude affects thickness";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.thick.base_px";
        p.displayName = "Base Thickness";
        p.group = "Peak Thickness";
        p.type = ParamType::Float;
        p.defaultValue = 2.0f;
        p.minValue = 0.5f;
        p.maxValue = 20.0f;
        p.unit = "px";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.thick.scale_px";
        p.displayName = "Scale";
        p.group = "Peak Thickness";
        p.type = ParamType::Float;
        p.defaultValue = 4.0f;
        p.minValue = 0.0f;
        p.maxValue = 20.0f;
        p.unit = "px";
        params.push_back(p);
    }

    // --- Peak Color ---
    {
        ModuleParamDesc p;
        p.id = "peak.color_auto";
        p.displayName = "Auto Color";
        p.group = "Peak Color";
        p.type = ParamType::Bool;
        p.defaultValue = true;
        p.tooltip = "Use gradient for peak color";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.color_freeze_spawner";
        p.displayName = "Freeze Spawner Color";
        p.group = "Peak Color";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Don't update spawner color live";
        params.push_back(p);
    }

    // --- Particles ---
    {
        ModuleParamDesc p;
        p.id = "peak.spawn.each_peak";
        p.displayName = "Spawn Particles";
        p.group = "Particles";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Spawn particles at peaks";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.spawn.min_delta";
        p.displayName = "Min Delta";
        p.group = "Particles";
        p.type = ParamType::Float;
        p.defaultValue = 0.0f;
        p.minValue = 0.0f;
        p.maxValue = 1.0f;
        p.tooltip = "Minimum amplitude rise to spawn";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.spawn.min_interval_ms";
        p.displayName = "Min Interval";
        p.group = "Particles";
        p.type = ParamType::Float;
        p.defaultValue = 60.0f;
        p.minValue = 0.0f;
        p.maxValue = 1000.0f;
        p.unit = "ms";
        p.tooltip = "Minimum time between spawns per band";
        params.push_back(p);
    }
    {
        ModuleParamDesc p;
        p.id = "peak.max_particles_per_band";
        p.displayName = "Max Per Band";
        p.group = "Particles";
        p.type = ParamType::Int;
        p.defaultValue = 8;
        p.minValue = 1;
        p.maxValue = 32;
        p.tooltip = "Maximum particles per band";
        params.push_back(p);
    }

    // --- Z-Order ---
    {
        ModuleParamDesc p;
        p.id = "peak.draw_behind_bars";
        p.displayName = "Draw Behind Bars";
        p.group = "Rendering";
        p.type = ParamType::Bool;
        p.defaultValue = false;
        p.tooltip = "Draw peaks behind bars";
        params.push_back(p);
    }

    return params;
}

bool EqualizerModule::getParam(const std::string& id, ParamValue& out) const
{
    // General
    if (id == "bands") { out = m_bandCount; return true; }
    if (id == "gain") { out = m_gain; return true; }
    if (id == "bar_gap_px") { out = m_barGapPx; return true; }
    if (id == "orientation") { out = static_cast<int>(m_orientation); return true; }

    // Audio
    if (id == "audio.scale") { out = static_cast<int>(m_frequencyScale); return true; }
    if (id == "audio.emaAlpha") { out = m_emaAlpha; return true; }
    if (id == "audio.floorDb") { out = m_floorDb; return true; }
    if (id == "audio.ceilDb") { out = m_ceilDb; return true; }
    if (id == "audio.clamp01") { out = m_clamp01; return true; }

    // Gradient
    if (id == "grad.domain") { out = static_cast<int>(m_gradientDomain); return true; }

    // Peak Spawner
    if (id == "peak.enabled") { out = m_spawnerConfig.enabled; return true; }
    if (id == "peak.delay_ms") { out = m_spawnerConfig.delayMs; return true; }
    if (id == "peak.falloff_per_sec") { out = m_spawnerConfig.falloffPerSec; return true; }
    if (id == "peak.gravity") { out = m_spawnerConfig.gravity; return true; }
    if (id == "peak.respawn_on_leave") { out = m_spawnerConfig.respawnOnLeave; return true; }
    if (id == "peak.bounce_elasticity") { out = m_spawnerConfig.bounceElasticity; return true; }

    // Peak Spring
    if (id == "peak.spring.enabled") { out = m_spawnerConfig.spring.enabled; return true; }
    if (id == "peak.spring.k") { out = m_spawnerConfig.spring.k; return true; }
    if (id == "peak.spring.damping") { out = m_spawnerConfig.spring.damping; return true; }

    // Peak Thickness
    if (id == "peak.thick.mode") { out = static_cast<int>(m_spawnerConfig.thickness.mode); return true; }
    if (id == "peak.thick.base_px") { out = m_spawnerConfig.thickness.basePx; return true; }
    if (id == "peak.thick.scale_px") { out = m_spawnerConfig.thickness.scalePx; return true; }

    // Peak Color
    if (id == "peak.color_auto") { out = m_peakColorConfig.autoColor; return true; }
    if (id == "peak.color_freeze_spawner") { out = m_peakColorConfig.freezeSpawner; return true; }

    // Particles
    if (id == "peak.spawn.each_peak") { out = m_particleConfig.spawnEachPeak; return true; }
    if (id == "peak.spawn.min_delta") { out = m_particleConfig.minDelta; return true; }
    if (id == "peak.spawn.min_interval_ms") { out = m_particleConfig.minIntervalMs; return true; }
    if (id == "peak.max_particles_per_band") { out = m_particleConfig.maxPerBand; return true; }

    // Z-Order
    if (id == "peak.draw_behind_bars") { out = m_drawPeaksBehindBars; return true; }

    return false;
}

bool EqualizerModule::setParam(const std::string& id, const ParamValue& value)
{
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

    // General
    if (id == "bands") { setBandCount(getInt()); return true; }
    if (id == "gain") { setGain(getFloat()); return true; }
    if (id == "bar_gap_px") { setBarGapPx(getFloat()); return true; }
    if (id == "orientation") { setOrientation(static_cast<BarOrientation>(getInt())); return true; }

    // Audio
    if (id == "audio.scale") { setFrequencyScale(static_cast<FrequencyScale>(getInt())); return true; }
    if (id == "audio.emaAlpha") { setEmaAlpha(getFloat()); return true; }
    if (id == "audio.floorDb") { setFloorDb(getFloat()); return true; }
    if (id == "audio.ceilDb") { setCeilDb(getFloat()); return true; }
    if (id == "audio.clamp01") { setClamp01(getBool()); return true; }

    // Gradient
    if (id == "grad.domain") { setGradientDomain(static_cast<GradientDomain>(getInt())); return true; }

    // Peak Spawner
    if (id == "peak.enabled") { m_spawnerConfig.enabled = getBool(); return true; }
    if (id == "peak.delay_ms") { m_spawnerConfig.delayMs = getFloat(); return true; }
    if (id == "peak.falloff_per_sec") { m_spawnerConfig.falloffPerSec = getFloat(); return true; }
    if (id == "peak.gravity") { m_spawnerConfig.gravity = getFloat(); return true; }
    if (id == "peak.respawn_on_leave") { m_spawnerConfig.respawnOnLeave = getBool(); return true; }
    if (id == "peak.bounce_elasticity") { m_spawnerConfig.bounceElasticity = getFloat(); return true; }

    // Peak Spring
    if (id == "peak.spring.enabled") { m_spawnerConfig.spring.enabled = getBool(); return true; }
    if (id == "peak.spring.k") { m_spawnerConfig.spring.k = getFloat(); return true; }
    if (id == "peak.spring.damping") { m_spawnerConfig.spring.damping = getFloat(); return true; }

    // Peak Thickness
    if (id == "peak.thick.mode") { m_spawnerConfig.thickness.mode = static_cast<ThicknessMode>(getInt()); return true; }
    if (id == "peak.thick.base_px") { m_spawnerConfig.thickness.basePx = getFloat(); return true; }
    if (id == "peak.thick.scale_px") { m_spawnerConfig.thickness.scalePx = getFloat(); return true; }

    // Peak Color
    if (id == "peak.color_auto") { m_peakColorConfig.autoColor = getBool(); return true; }
    if (id == "peak.color_freeze_spawner") { m_peakColorConfig.freezeSpawner = getBool(); return true; }

    // Particles
    if (id == "peak.spawn.each_peak") { m_particleConfig.spawnEachPeak = getBool(); return true; }
    if (id == "peak.spawn.min_delta") { m_particleConfig.minDelta = getFloat(); return true; }
    if (id == "peak.spawn.min_interval_ms") { m_particleConfig.minIntervalMs = getFloat(); return true; }
    if (id == "peak.max_particles_per_band") { m_particleConfig.maxPerBand = getInt(); return true; }

    // Z-Order
    if (id == "peak.draw_behind_bars") { m_drawPeaksBehindBars = getBool(); return true; }

    return false;
}

void EqualizerModule::resetToDefaults()
{
    m_bandCount = 64;
    m_gain = 1.0f;
    m_barGapPx = 2.0f;
    m_orientation = BarOrientation::BottomUp;

    m_frequencyScale = FrequencyScale::Linear;
    m_emaAlpha = 0.0f;
    m_floorDb = -60.0f;
    m_ceilDb = 0.0f;
    m_clamp01 = true;

    m_gradientDomain = GradientDomain::Position;

    m_spawnerConfig = PeakSpawnerConfig{};
    m_particleConfig = PeakParticleConfig{};
    m_peakColorConfig = PeakColorConfig{};
    m_drawPeaksBehindBars = false;

    m_colorGradient.clearStops();
    m_colorGradient.addStop(0.0f, {0.0f, 0.8f, 1.0f, 1.0f});
    m_colorGradient.addStop(1.0f, {1.0f, 0.2f, 0.2f, 1.0f});

    resizeBands(m_bandCount);
}

// =============================================================================
// Processing
// =============================================================================

void EqualizerModule::processSpectrum(const float* spectrum, int count, float deltaTime)
{
    if (!spectrum || count <= 0)
        return;

    // 1) Map FFT to bands
    mapSpectrum(spectrum, count);

    // 2) Apply EMA smoothing
    applyEMA(deltaTime);

    // 3) Convert to dB and normalize
    normalizeDb();

    // 4) Update colors
    updateColors();

    // 5) Update peak spawners
    updateSpawners(deltaTime);

    // 6) Update particles
    updateParticles(deltaTime);
}

void EqualizerModule::updateFromProcessed(const float* processedBands, int count, float deltaTime)
{
    if (!processedBands || count <= 0)
        return;

    // Resize if needed
    if (count != m_bandCount)
    {
        setBandCount(count);
    }

    // Copy pre-processed bands directly (already mapped, smoothed, normalized by AudioSourceModule)
    for (int i = 0; i < m_bandCount && i < count; ++i)
    {
        m_bands[i] = std::clamp(processedBands[i], 0.0f, 1.0f);
    }

    // Update colors based on current bands
    updateColors();

    // Update peak spawners
    updateSpawners(deltaTime);

    // Update particles
    updateParticles(deltaTime);
}

void EqualizerModule::setBandCount(int count)
{
    count = std::clamp(count, 1, MAX_BANDS);
    if (count != m_bandCount)
    {
        m_bandCount = count;
        resizeBands(count);
    }
}

// =============================================================================
// Private Methods
// =============================================================================

void EqualizerModule::resizeBands(int count)
{
    m_bands.resize(count, 0.0f);
    m_rawBands.resize(count, 0.0f);
    m_bandColors.resize(count, Color4f{1, 1, 1, 1});
    m_spawners.resize(count);
    m_lastSpawnTime.resize(count, 0.0f);

    for (auto& s : m_spawners)
        s.reset();

    m_primed = false;
}

void EqualizerModule::mapSpectrum(const float* spectrum, int count)
{
    // Map FFT bins to visual bands based on frequency scale
    for (int i = 0; i < m_bandCount; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(m_bandCount - 1);
        float binF = mapFrequencyToBin(t, count);
        
        int bin0 = static_cast<int>(binF);
        int bin1 = std::min(bin0 + 1, count - 1);
        float frac = binF - static_cast<float>(bin0);
        
        bin0 = std::clamp(bin0, 0, count - 1);
        
        // Linear interpolation between bins
        float value = spectrum[bin0] * (1.0f - frac) + spectrum[bin1] * frac;
        m_rawBands[i] = value * m_gain;
    }
}

float EqualizerModule::mapFrequencyToBin(float normalizedPos, int fftSize) const
{
    // Maps normalized position [0,1] to FFT bin index
    // Using different frequency scales
    
    constexpr float MIN_FREQ = 20.0f;
    constexpr float MAX_FREQ = 20000.0f;
    constexpr float SAMPLE_RATE = 44100.0f;
    
    float freq;
    
    switch (m_frequencyScale)
    {
        case FrequencyScale::Linear:
            freq = MIN_FREQ + normalizedPos * (MAX_FREQ - MIN_FREQ);
            break;
            
        case FrequencyScale::Log:
            freq = MIN_FREQ * std::pow(MAX_FREQ / MIN_FREQ, normalizedPos);
            break;
            
        case FrequencyScale::Mel:
        {
            // Mel scale: m = 2595 * log10(1 + f/700)
            float melMin = 2595.0f * std::log10(1.0f + MIN_FREQ / 700.0f);
            float melMax = 2595.0f * std::log10(1.0f + MAX_FREQ / 700.0f);
            float mel = melMin + normalizedPos * (melMax - melMin);
            freq = 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
            break;
        }
    }
    
    // Convert frequency to FFT bin
    float bin = freq * static_cast<float>(fftSize) / SAMPLE_RATE;
    return std::clamp(bin, 0.0f, static_cast<float>(fftSize - 1));
}

void EqualizerModule::applyEMA(float deltaTime)
{
    (void)deltaTime;  // Could be used for time-based smoothing
    
    if (m_emaAlpha <= 0.0f)
    {
        // No smoothing
        m_bands = m_rawBands;
        return;
    }
    
    if (!m_primed)
    {
        // Prime with first frame
        m_bands = m_rawBands;
        m_primed = true;
        return;
    }
    
    // EMA: output = alpha * prev + (1-alpha) * current
    for (int i = 0; i < m_bandCount; ++i)
    {
        m_bands[i] = m_emaAlpha * m_bands[i] + (1.0f - m_emaAlpha) * m_rawBands[i];
    }
}

void EqualizerModule::normalizeDb()
{
    float range = m_ceilDb - m_floorDb;
    if (range <= 0.0f) range = 1.0f;
    
    for (int i = 0; i < m_bandCount; ++i)
    {
        float value = m_bands[i];
        
        // Convert to dB
        float db = (value > 1e-10f) ? 20.0f * std::log10(value) : -120.0f;
        
        // Normalize to [0,1] based on floor/ceil
        float normalized = (db - m_floorDb) / range;
        
        if (m_clamp01)
        {
            normalized = std::clamp(normalized, 0.0f, 1.0f);
        }
        
        m_bands[i] = normalized;
    }
}

void EqualizerModule::updateColors()
{
    for (int i = 0; i < m_bandCount; ++i)
    {
        float t;
        switch (m_gradientDomain)
        {
            case GradientDomain::Position:
                t = static_cast<float>(i) / static_cast<float>(m_bandCount - 1);
                break;
            case GradientDomain::Amplitude:
                t = m_bands[i];
                break;
            case GradientDomain::Time:
            case GradientDomain::Beat:
            default:
                t = static_cast<float>(i) / static_cast<float>(m_bandCount - 1);
                break;
        }
        
        m_bandColors[i] = m_colorGradient.sample(t);
    }
}

void EqualizerModule::updateSpawners(float deltaTime)
{
    if (!m_spawnerConfig.enabled)
        return;
    
    float dtMs = deltaTime * 1000.0f;
    float gravity = m_spawnerConfig.gravity;
    
    // Flip gravity for TopDown orientation
    if (m_orientation == BarOrientation::TopDown)
        gravity = -gravity;
    
    for (int i = 0; i < m_bandCount; ++i)
    {
        auto& s = m_spawners[i];
        float bandValue = m_bands[i];
        
        // Check if peak is touching or below bar (contact)
        bool isTouchingBar = (s.position <= bandValue);
        
        // Check for new peak - spawner touches bar from below
        if (bandValue > s.peakValue)
        {
            // New peak - snap to position
            s.position = bandValue;
            s.velocity = 0.0f;
            s.holdTimer = m_spawnerConfig.delayMs;
            s.peakValue = bandValue;
            
            // Spawn particle if enabled
            if (m_particleConfig.spawnEachPeak)
            {
                float delta = bandValue - s.peakValue;
                if (delta >= m_particleConfig.minDelta &&
                    m_lastSpawnTime[i] >= m_particleConfig.minIntervalMs)
                {
                    spawnParticle(i, s.position, s.color);
                    m_lastSpawnTime[i] = 0.0f;
                }
            }
        }
        
        // UPDATE COLOR ON CONTACT (when peak touches bar)
        // This ensures color updates when gradient changes AND peak is on bar
        if (isTouchingBar && m_peakColorConfig.autoColor && !m_peakColorConfig.freezeSpawner)
        {
            s.color = m_bandColors[i];
        }
        else if (!m_peakColorConfig.autoColor)
        {
            s.color = m_peakColorConfig.fixedColor;
        }
        
        // Update spawn timer
        m_lastSpawnTime[i] += dtMs;
        
        // Physics update
        if (s.holdTimer > 0.0f)
        {
            s.holdTimer -= dtMs;
        }
        else
        {
            if (m_spawnerConfig.spring.enabled)
            {
                // Spring physics toward band value
                // Gravity: positive = fall down, negative = float up
                float springForce = m_spawnerConfig.spring.k * (bandValue - s.position);
                float dampingForce = -m_spawnerConfig.spring.damping * s.velocity;
                float accel = springForce + dampingForce - gravity;
                
                s.velocity += accel * deltaTime;
                s.position += s.velocity * deltaTime;
            }
            else
            {
                // Classic hold-then-fall
                // Gravity: positive = fall down, negative = float up
                s.velocity -= gravity * deltaTime;
                s.velocity -= s.velocity * m_spawnerConfig.falloffPerSec * deltaTime;
                s.position += s.velocity * deltaTime;
            }
            
            // Boundary handling
            if (s.position < 0.0f || s.position > 1.0f)
            {
                if (m_spawnerConfig.respawnOnLeave)
                {
                    s.position = bandValue;
                    s.velocity = 0.0f;
                    s.holdTimer = m_spawnerConfig.delayMs;
                }
                else
                {
                    // Bounce
                    s.position = std::clamp(s.position, 0.0f, 1.0f);
                    s.velocity *= -m_spawnerConfig.bounceElasticity;
                }
            }
        }
        
        // Track peak decay
        s.peakValue = std::max(s.peakValue - 0.01f, bandValue);
    }
}

void EqualizerModule::updateParticles(float deltaTime)
{
    float dtMs = deltaTime * 1000.0f;
    float gravity = m_spawnerConfig.gravity;
    
    if (m_orientation == BarOrientation::TopDown)
        gravity = -gravity;
    
    for (auto& p : m_particles)
    {
        if (!p.alive)
            continue;
        
        p.lifetime += dtMs;
        
        // Update color if bound to spawner
        if (m_particleConfig.colorBoundToSpawner && !m_particleConfig.colorFreezeParticles)
        {
            if (p.bandIndex >= 0 && p.bandIndex < static_cast<int>(m_spawners.size()))
            {
                p.color = m_spawners[p.bandIndex].color;
            }
        }
        
        // Physics
        if (p.holdTimer > 0.0f)
        {
            p.holdTimer -= dtMs;
        }
        else
        {
            // Gravity: positive = fall down, negative = float up
            p.velocity -= gravity * deltaTime;
            p.velocity -= p.velocity * m_spawnerConfig.falloffPerSec * deltaTime;
            p.position += p.velocity * deltaTime;
        }
        
        // Check bounds
        if (p.position < 0.0f || p.position > 1.0f)
        {
            if (m_spawnerConfig.respawnOnLeave)
            {
                // Respawn at band height
                if (p.bandIndex >= 0 && p.bandIndex < m_bandCount)
                {
                    p.position = m_bands[p.bandIndex];
                    p.velocity = 0.0f;
                    p.holdTimer = m_spawnerConfig.delayMs;
                }
            }
            else
            {
                p.alive = false;
            }
        }
    }
    
    removeDeadParticles();
}

void EqualizerModule::spawnParticle(int bandIndex, float position, const Color4f& color)
{
    // Count existing particles for this band
    int bandCount = 0;
    for (const auto& p : m_particles)
    {
        if (p.alive && p.bandIndex == bandIndex)
            ++bandCount;
    }
    
    // Remove oldest if at limit
    if (bandCount >= m_particleConfig.maxPerBand)
    {
        for (auto& p : m_particles)
        {
            if (p.alive && p.bandIndex == bandIndex)
            {
                p.alive = false;
                break;
            }
        }
    }
    
    // Check total particle limit
    if (static_cast<int>(m_particles.size()) >= MAX_PARTICLES_TOTAL)
    {
        removeDeadParticles();
        if (static_cast<int>(m_particles.size()) >= MAX_PARTICLES_TOTAL)
            return;
    }
    
    // Create new particle
    PeakParticle p;
    p.position = position;
    p.velocity = 0.0f;
    p.holdTimer = m_spawnerConfig.delayMs;
    p.spawnPosition = position;
    p.lifetime = 0.0f;
    p.color = color;
    p.bandIndex = bandIndex;
    p.alive = true;
    
    m_particles.push_back(p);
}

void EqualizerModule::removeDeadParticles()
{
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const PeakParticle& p) { return !p.alive; }),
        m_particles.end());
}

} // namespace lumi::modules
