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
    // Initialize gradient with "Ocean" preset
    m_colorGradient.setMode(GradientMode::Linear);
    m_colorGradient.loadPreset("Ocean");

    resizeBands(m_bandCount);
}

// =============================================================================
// Configuration
// =============================================================================

void EqualizerModule::resetToDefaults()
{
    m_bandCount = 64;
    m_gain = 1.0f;
    m_barGapPx = 2.0f;
    m_orientation = BarOrientation::BottomUp;

    m_gradientDomain = GradientDomain::Position;

    m_spawnerConfig = PeakSpawnerConfig{};
    m_particleConfig = PeakParticleConfig{};
    m_peakColorConfig = PeakColorConfig{};
    m_drawPeaksBehindBars = false;

    // Load "Ocean" gradient preset as default
    m_colorGradient.loadPreset("Ocean");

    resizeBands(m_bandCount);
}

// =============================================================================
// Processing
// =============================================================================

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
    m_bandColors.resize(count, Color4f{1, 1, 1, 1});
    m_spawners.resize(count);
    m_lastSpawnTime.resize(count, 0.0f);

    for (auto& s : m_spawners)
        s.reset();
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
