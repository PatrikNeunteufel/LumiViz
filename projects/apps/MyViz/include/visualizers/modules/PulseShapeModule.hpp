/**
 ****************************************************************************************
 * @file   PulseShapeModule.hpp
 * @brief  Pulse shape module for audio-reactive visualizers
 *
 * Provides various pulse shapes and their rendering parameters.
 * Supports shapes like Circle, Ring, Flash, N-gon, Star, Wave.
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "ColorSchemeModule.hpp"

#include <string>
#include <vector>
#include <functional>

namespace lumi::modules {

// =============================================================================
// Pulse Shape Types
// =============================================================================

/**
 * @brief Available pulse shape types
 */
enum class PulseShape
{
    Circle,      ///< Filled circle that scales with audio
    Ring,        ///< Expanding ring on beat/audio
    Flash,       ///< Fullscreen color flash
    Ngon,        ///< N-sided polygon (triangle, square, hexagon, etc.)
    Star,        ///< N-pointed star shape
    Wave,        ///< Concentric wave rings
    RadialBars,  ///< Bars arranged radially
    Blob,        ///< Organic blob shape with audio-reactive deformation
    Tunnel,      ///< Tunnel/zoom effect
    Grid         ///< Pulsing grid pattern
};

/**
 * @brief Pulse trigger mode
 */
enum class PulseTrigger
{
    Continuous, ///< Always pulsing (amplitude-based)
    OnBeat,     ///< Trigger on beat detection
    OnThreshold ///< Trigger when audio exceeds threshold
};

/**
 * @brief Pulse decay mode
 */
enum class PulseDecay
{
    Linear,     ///< Linear decay
    Exponential,///< Exponential decay (smoother)
    Hold,       ///< Hold then drop
    Bounce      ///< Bounce effect
};

// =============================================================================
// Pulse State
// =============================================================================

/**
 * @brief Current state of a pulse animation
 */
struct PulseState
{
    float intensity = 0.0f;     ///< Current intensity [0..1]
    float phase = 0.0f;         ///< Animation phase [0..1]
    float size = 0.0f;          ///< Current size [0..1]
    float rotation = 0.0f;      ///< Current rotation (radians)
    bool active = false;        ///< Is pulse currently active
    float timeSinceTrigger = 0.0f; ///< Time since last trigger
};

// =============================================================================
// Vertex for Shape Rendering
// =============================================================================

/**
 * @brief Vertex data for shape rendering
 */
struct ShapeVertex
{
    float x = 0.0f;
    float y = 0.0f;
    float u = 0.0f;  ///< Texture coordinate / position in shape [0..1]
    float v = 0.0f;  ///< Radial position [0..1]
};

// =============================================================================
// PulseShapeModule
// =============================================================================

/**
 * @class PulseShapeModule
 * @brief Module for pulse shape generation and animation
 *
 * Generates vertex data for various pulse shapes and handles
 * animation parameters like decay, easing, and audio reactivity.
 *
 * @par Example Usage
 * @code
 * PulseShapeModule pulse;
 * pulse.setShape(PulseShape::Ring);
 * pulse.setDecay(PulseDecay::Exponential);
 * pulse.setDecayTime(0.5f);
 *
 * // In update loop:
 * pulse.trigger(beatIntensity);
 * pulse.update(deltaTime);
 *
 * // Get current state for rendering:
 * PulseState state = pulse.state();
 * auto vertices = pulse.generateVertices();
 * @endcode
 */
class PulseShapeModule
{
public:
    // =========================================================================
    // Construction
    // =========================================================================

    PulseShapeModule();
    ~PulseShapeModule() = default;

    // =========================================================================
    // Module Interface
    // =========================================================================

    [[nodiscard]] static const char* moduleName() { return "PulseShape"; }
    [[nodiscard]] static const char* moduleDescription()
    {
        return "Pulse shape generation and animation";
    }

    /// @brief Reset to default state
    void reset();

    // =========================================================================
    // Shape Configuration
    // =========================================================================

    /**
     * @brief Set pulse shape type
     * @param shape The shape to render
     */
    void setShape(PulseShape shape) { m_shape = shape; }

    /**
     * @brief Get current shape
     */
    [[nodiscard]] PulseShape shape() const { return m_shape; }

    /**
     * @brief Set number of sides (for Ngon, Star, RadialBars)
     * @param sides Number of sides (3-64)
     */
    void setSides(int sides) { m_sides = std::clamp(sides, 3, 64); }

    /**
     * @brief Get number of sides
     */
    [[nodiscard]] int sides() const { return m_sides; }

    /**
     * @brief Set inner radius ratio (for Ring, Tunnel)
     * @param ratio Inner radius as fraction of outer [0..1]
     */
    void setInnerRadiusRatio(float ratio)
    {
        m_innerRadiusRatio = std::clamp(ratio, 0.0f, 0.99f);
    }

    /**
     * @brief Get inner radius ratio
     */
    [[nodiscard]] float innerRadiusRatio() const { return m_innerRadiusRatio; }

    // =========================================================================
    // Size & Position
    // =========================================================================

    /**
     * @brief Set base size (radius as fraction of viewport)
     * @param size Base size [0..1]
     */
    void setBaseSize(float size) { m_baseSize = std::clamp(size, 0.0f, 2.0f); }

    /**
     * @brief Get base size
     */
    [[nodiscard]] float baseSize() const { return m_baseSize; }

    /**
     * @brief Set size range for audio modulation
     * @param min Minimum size multiplier
     * @param max Maximum size multiplier
     */
    void setSizeRange(float min, float max)
    {
        m_sizeMin = min;
        m_sizeMax = max;
    }

    /**
     * @brief Set center position
     * @param x X position [-1..1]
     * @param y Y position [-1..1]
     */
    void setCenter(float x, float y)
    {
        m_centerX = x;
        m_centerY = y;
    }

    /**
     * @brief Get center X
     */
    [[nodiscard]] float centerX() const { return m_centerX; }

    /**
     * @brief Get center Y
     */
    [[nodiscard]] float centerY() const { return m_centerY; }

    // =========================================================================
    // Animation
    // =========================================================================

    /**
     * @brief Set trigger mode
     * @param trigger How the pulse is triggered
     */
    void setTrigger(PulseTrigger trigger) { m_trigger = trigger; }

    /**
     * @brief Get trigger mode
     */
    [[nodiscard]] PulseTrigger triggerMode() const { return m_trigger; }

    /**
     * @brief Set decay mode
     * @param decay How the pulse fades
     */
    void setDecay(PulseDecay decay) { m_decay = decay; }

    /**
     * @brief Get decay mode
     */
    [[nodiscard]] PulseDecay decay() const { return m_decay; }

    /**
     * @brief Set decay time
     * @param seconds Time for pulse to fully decay
     */
    void setDecayTime(float seconds) { m_decayTime = std::max(0.01f, seconds); }

    /**
     * @brief Get decay time
     */
    [[nodiscard]] float decayTime() const { return m_decayTime; }

    /**
     * @brief Set attack time (for Continuous mode)
     * @param seconds Time to reach full intensity
     */
    void setAttackTime(float seconds) { m_attackTime = std::max(0.0f, seconds); }

    /**
     * @brief Get attack time
     */
    [[nodiscard]] float attackTime() const { return m_attackTime; }

    // =========================================================================
    // Rotation
    // =========================================================================

    /**
     * @brief Set rotation speed
     * @param degreesPerSecond Rotation speed
     */
    void setRotationSpeed(float degreesPerSecond)
    {
        m_rotationSpeed = degreesPerSecond;
    }

    /**
     * @brief Get rotation speed
     */
    [[nodiscard]] float rotationSpeed() const { return m_rotationSpeed; }

    /**
     * @brief Set base rotation
     * @param degrees Initial rotation
     */
    void setBaseRotation(float degrees) { m_baseRotation = degrees; }

    /**
     * @brief Get base rotation
     */
    [[nodiscard]] float baseRotation() const { return m_baseRotation; }

    /**
     * @brief Enable audio-reactive rotation
     * @param enabled True to modulate rotation with audio
     * @param scale Degrees to add per unit amplitude
     */
    void setAudioRotation(bool enabled, float scale = 45.0f)
    {
        m_audioRotationEnabled = enabled;
        m_audioRotationScale = scale;
    }

    // =========================================================================
    // Threshold
    // =========================================================================

    /**
     * @brief Set trigger threshold (for OnThreshold mode)
     * @param threshold Audio level to trigger [0..1]
     */
    void setThreshold(float threshold)
    {
        m_threshold = std::clamp(threshold, 0.0f, 1.0f);
    }

    /**
     * @brief Get trigger threshold
     */
    [[nodiscard]] float threshold() const { return m_threshold; }

    // =========================================================================
    // Wave-specific parameters
    // =========================================================================

    /**
     * @brief Set number of concentric waves
     * @param count Number of waves (1-10)
     */
    void setWaveCount(int count) { m_waveCount = std::clamp(count, 1, 10); }

    /**
     * @brief Get wave count
     */
    [[nodiscard]] int waveCount() const { return m_waveCount; }

    /**
     * @brief Set wave speed (expansion rate)
     * @param speed Units per second
     */
    void setWaveSpeed(float speed) { m_waveSpeed = speed; }

    /**
     * @brief Get wave speed
     */
    [[nodiscard]] float waveSpeed() const { return m_waveSpeed; }

    // =========================================================================
    // Update & Trigger
    // =========================================================================

    /**
     * @brief Update animation state
     * @param deltaTime Time since last update in seconds
     * @param audioLevel Current audio level [0..1]
     */
    void update(float deltaTime, float audioLevel = 0.0f);

    /**
     * @brief Trigger pulse (for OnBeat/OnThreshold modes)
     * @param intensity Trigger intensity [0..1]
     */
    void trigger(float intensity = 1.0f);

    /**
     * @brief Get current pulse state
     * @return Current animation state
     */
    [[nodiscard]] const PulseState& state() const { return m_state; }

    /**
     * @brief Get effective size (base * audio modulation)
     * @return Size for rendering
     */
    [[nodiscard]] float effectiveSize() const;

    /**
     * @brief Get effective rotation (base + speed + audio)
     * @return Rotation in radians
     */
    [[nodiscard]] float effectiveRotation() const;

    // =========================================================================
    // Vertex Generation
    // =========================================================================

    /**
     * @brief Generate vertices for current shape
     * @param segments Number of segments for curved shapes
     * @return Vector of vertices
     */
    [[nodiscard]] std::vector<ShapeVertex> generateVertices(int segments = 64) const;

    /**
     * @brief Generate vertices for circle shape
     */
    [[nodiscard]] std::vector<ShapeVertex> generateCircle(int segments) const;

    /**
     * @brief Generate vertices for ring shape
     */
    [[nodiscard]] std::vector<ShapeVertex> generateRing(int segments) const;

    /**
     * @brief Generate vertices for N-gon shape
     */
    [[nodiscard]] std::vector<ShapeVertex> generateNgon() const;

    /**
     * @brief Generate vertices for star shape
     */
    [[nodiscard]] std::vector<ShapeVertex> generateStar() const;

    // =========================================================================
    // Utility
    // =========================================================================

    /**
     * @brief Get shape name as string
     * @param shape Shape type
     * @return Human-readable name
     */
    static const char* shapeName(PulseShape shape);

    /**
     * @brief Get all available shape names
     */
    static std::vector<const char*> availableShapes();

private:
    /**
     * @brief Calculate decay multiplier
     */
    [[nodiscard]] float calculateDecay(float t) const;

    // Configuration
    PulseShape m_shape = PulseShape::Circle;
    PulseTrigger m_trigger = PulseTrigger::Continuous;
    PulseDecay m_decay = PulseDecay::Exponential;

    // Shape parameters
    int m_sides = 6;
    float m_innerRadiusRatio = 0.7f;

    // Size
    float m_baseSize = 0.5f;
    float m_sizeMin = 0.8f;
    float m_sizeMax = 1.2f;

    // Position
    float m_centerX = 0.0f;
    float m_centerY = 0.0f;

    // Animation timing
    float m_decayTime = 0.5f;
    float m_attackTime = 0.05f;
    float m_threshold = 0.5f;

    // Rotation
    float m_rotationSpeed = 0.0f;
    float m_baseRotation = 0.0f;
    bool m_audioRotationEnabled = false;
    float m_audioRotationScale = 45.0f;

    // Wave parameters
    int m_waveCount = 3;
    float m_waveSpeed = 1.0f;

    // Runtime state
    PulseState m_state;
    float m_currentAudioLevel = 0.0f;
    float m_accumulatedRotation = 0.0f;
};

} // namespace lumi::modules
