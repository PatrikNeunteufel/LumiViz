/**
 ****************************************************************************************
 * @file   EqualizerModule.hpp
 * @brief  Modular equalizer/spectrum analyzer with peak hold and particles
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * Equalizer visualization module with:
 * - Color gradient mapping by position or amplitude
 * - Peak-hold spawners with physics (hold/gravity/spring)
 * - Optional peak particles with spawn/fade effects
 * - Configurable bar rendering (orientation, gap, z-order)
 *
 * Band mapping/smoothing/dB-normalization is done by AudioSourceModule —
 * this module consumes pre-processed bands via updateFromProcessed().
 ****************************************************************************************
 */

#pragma once

#include "IModule.hpp"  // For Color4f
#include "ColorGradientModule.hpp"  // For GradientDomain, gradient sampling

#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

namespace lumi::modules {

// =============================================================================
// Enums (using existing where possible)
// =============================================================================

/**
 * @brief Bar orientation
 */
enum class BarOrientation
{
    BottomUp = 0,   ///< Bars grow upward
    TopDown         ///< Bars grow downward
};

/**
 * @brief Peak thickness mode
 */
enum class ThicknessMode
{
    Off = 0,      ///< Fixed thickness
    Direct,       ///< Thicker at higher amplitude
    Inverse       ///< Thicker at lower amplitude
};

// =============================================================================
// Structures
// =============================================================================

/**
 * @brief State for a single peak spawner
 */
struct PeakSpawner
{
    float position = 0.0f;       ///< Current position [0..1]
    float velocity = 0.0f;       ///< Current velocity
    float holdTimer = 0.0f;      ///< Time remaining in hold phase (ms)
    float peakValue = 0.0f;      ///< Last peak value
    Color4f color{1, 1, 1, 1};   ///< Current spawner color
    
    void reset()
    {
        position = 0.0f;
        velocity = 0.0f;
        holdTimer = 0.0f;
        peakValue = 0.0f;
    }
};

/**
 * @brief State for a single peak particle
 */
struct PeakParticle
{
    float position = 0.0f;       ///< Current position [0..1]
    float velocity = 0.0f;       ///< Current velocity
    float holdTimer = 0.0f;      ///< Time remaining in hold phase (ms)
    float spawnPosition = 0.0f;  ///< Position where spawned
    float lifetime = 0.0f;       ///< Time since spawn (ms)
    Color4f color{1, 1, 1, 1};   ///< Particle color
    int bandIndex = 0;           ///< Which band this particle belongs to
    bool alive = true;           ///< Is this particle still active?
};

/**
 * @brief Thickness configuration
 */
struct ThicknessConfig
{
    ThicknessMode mode = ThicknessMode::Off;
    float basePx = 2.0f;
    float scalePx = 4.0f;
    float minPx = 1.0f;
    float maxPx = 12.0f;
};

/**
 * @brief Spring physics configuration
 */
struct SpringConfig
{
    bool enabled = false;
    float k = 40.0f;           ///< Spring constant
    float damping = 10.0f;     ///< Damping factor
};

/**
 * @brief Peak spawner configuration
 */
struct PeakSpawnerConfig
{
    bool enabled = true;
    float delayMs = 120.0f;           ///< Hold delay before falling
    float falloffPerSec = 0.5f;       ///< Linear damping (air resistance)
    float gravity = 9.81f;            ///< Acceleration (Earth gravity)
    bool respawnOnLeave = false;      ///< Respawn when leaving [0..1]
    float bounceElasticity = 0.25f;   ///< Bounce factor at boundaries
    bool useDelay = true;             ///< Apply delay in spring mode
    SpringConfig spring;              ///< Spring physics
    ThicknessConfig thickness;        ///< Thickness calculation
};

/**
 * @brief Peak particle configuration
 */
struct PeakParticleConfig
{
    bool spawnEachPeak = false;       ///< Spawn particles on peaks
    float minDelta = 0.0f;            ///< Minimum amplitude rise to spawn
    float minIntervalMs = 0.0f;       ///< Minimum time between spawns
    int maxPerBand = 8;               ///< Maximum particles per band
    bool colorFreezeParticles = false; ///< Keep spawn color forever
    bool colorBoundToSpawner = false;  ///< Follow spawner color live
};

/**
 * @brief Color configuration for peaks
 */
struct PeakColorConfig
{
    bool autoColor = true;            ///< Use gradient for color
    Color4f fixedColor{1, 1, 1, 1};   ///< Fixed color when auto=false
    bool freezeSpawner = false;       ///< Don't update spawner color live
};

// =============================================================================
// EqualizerModule
// =============================================================================

/**
 * @brief Equalizer visualization module
 *
 * Helper module (not derived from IModule) — parameters are declared and
 * routed by EqualizerVisualizer via the configuration accessors below.
 */
class EqualizerModule
{
public:
    static constexpr int MAX_BANDS = 256;
    static constexpr int MAX_PARTICLES_TOTAL = 2048;

    EqualizerModule();
    ~EqualizerModule() = default;

    /// @brief Reset all configuration to defaults
    void resetToDefaults();

    // =========================================================================
    // Processing
    // =========================================================================

    /**
     * @brief Update with already processed spectrum data (from AudioSourceModule)
     * @param processedBands Pre-processed band values [0..1]
     * @param count Number of bands
     * @param deltaTime Time since last frame (seconds)
     * @note Skips internal mapping/smoothing/normalization - just updates colors, spawners, particles
     */
    void updateFromProcessed(const float* processedBands, int count, float deltaTime);

    /**
     * @brief Get processed band values
     * @return Band amplitudes [0..1]
     */
    [[nodiscard]] const std::vector<float>& bands() const { return m_bands; }

    /**
     * @brief Get band colors
     * @return Colors for each band
     */
    [[nodiscard]] const std::vector<Color4f>& bandColors() const { return m_bandColors; }

    /**
     * @brief Get peak spawners
     * @return Spawner states for each band
     */
    [[nodiscard]] const std::vector<PeakSpawner>& spawners() const { return m_spawners; }

    /**
     * @brief Get active particles
     * @return Active particle list
     */
    [[nodiscard]] const std::vector<PeakParticle>& particles() const { return m_particles; }

    // =========================================================================
    // Configuration Accessors
    // =========================================================================

    // --- Bands ---
    void setBandCount(int count);
    [[nodiscard]] int bandCount() const { return m_bandCount; }

    /// @brief Display scaling of bar heights (render.heightScale, E1)
    void setHeightScale(float scale) { m_heightScale = std::max(0.0f, scale); }
    [[nodiscard]] float heightScale() const { return m_heightScale; }

    void setBarGapPx(float gap) { m_barGapPx = std::max(0.0f, gap); }
    [[nodiscard]] float barGapPx() const { return m_barGapPx; }

    void setOrientation(BarOrientation o) { m_orientation = o; }
    [[nodiscard]] BarOrientation orientation() const { return m_orientation; }

    // --- Gradient ---
    void setGradientDomain(GradientDomain domain) { m_gradientDomain = domain; }
    [[nodiscard]] GradientDomain gradientDomain() const { return m_gradientDomain; }

    [[nodiscard]] ColorGradientModule& colorGradient() { return m_colorGradient; }
    [[nodiscard]] const ColorGradientModule& colorGradient() const { return m_colorGradient; }

    // --- Peak Spawner ---
    [[nodiscard]] PeakSpawnerConfig& spawnerConfig() { return m_spawnerConfig; }
    [[nodiscard]] const PeakSpawnerConfig& spawnerConfig() const { return m_spawnerConfig; }

    // --- Peak Particles ---
    [[nodiscard]] PeakParticleConfig& particleConfig() { return m_particleConfig; }
    [[nodiscard]] const PeakParticleConfig& particleConfig() const { return m_particleConfig; }

    // --- Peak Colors ---
    [[nodiscard]] PeakColorConfig& peakColorConfig() { return m_peakColorConfig; }
    [[nodiscard]] const PeakColorConfig& peakColorConfig() const { return m_peakColorConfig; }

    // --- Z-Order ---
    void setDrawPeaksBehindBars(bool behind) { m_drawPeaksBehindBars = behind; }
    [[nodiscard]] bool drawPeaksBehindBars() const { return m_drawPeaksBehindBars; }

    // =========================================================================
    // Utility
    // =========================================================================

    /**
     * @brief Calculate peak marker thickness (normalized 0-1)
     * @param amplitude Current amplitude [0..1]
     * @return Thickness normalized [0..1]
     */
    [[nodiscard]] float calcPeakThickness(float amplitude) const;

private:
    // =========================================================================
    // Private Methods
    // =========================================================================

    void resizeBands(int count);
    void updateColors();
    void updateSpawners(float deltaTime);
    void updateParticles(float deltaTime);
    void spawnParticle(int bandIndex, float position, const Color4f& color);
    void removeDeadParticles();

    // =========================================================================
    // Configuration
    // =========================================================================

    int m_bandCount = 64;
    float m_heightScale = 1.0f;
    float m_barGapPx = 2.0f;
    BarOrientation m_orientation = BarOrientation::BottomUp;

    // Gradient
    GradientDomain m_gradientDomain = GradientDomain::Position;
    ColorGradientModule m_colorGradient;

    // Peak config
    PeakSpawnerConfig m_spawnerConfig;
    PeakParticleConfig m_particleConfig;
    PeakColorConfig m_peakColorConfig;
    bool m_drawPeaksBehindBars = false;

    // =========================================================================
    // State
    // =========================================================================

    std::vector<float> m_bands;              ///< Processed band values [0..1]
    std::vector<Color4f> m_bandColors;       ///< Colors per band
    std::vector<PeakSpawner> m_spawners;     ///< Peak spawners per band
    std::vector<PeakParticle> m_particles;   ///< Active particles
    std::vector<float> m_lastSpawnTime;      ///< Time since last spawn per band
};

// =============================================================================
// Inline Implementations
// =============================================================================

inline float EqualizerModule::calcPeakThickness(float amplitude) const
{
    const auto& cfg = m_spawnerConfig.thickness;
    float thick = cfg.basePx;
    
    switch (cfg.mode)
    {
        case ThicknessMode::Off:
            break;
        case ThicknessMode::Direct:
            thick += amplitude * cfg.scalePx;
            break;
        case ThicknessMode::Inverse:
            thick += (1.0f - amplitude) * cfg.scalePx;
            break;
    }
    
    // Return normalized (0-1 range based on max)
    return std::clamp(thick, cfg.minPx, cfg.maxPx) / cfg.maxPx;
}

} // namespace lumi::modules
