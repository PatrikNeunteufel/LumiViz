/**
 ****************************************************************************************
 * @file   PulsingVisualizer.hpp
 * @brief  Audio-reactive pulsing visualizer with modular architecture
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 3.0.0
 *
 * @details
 * ## Audio-Reactive Pulsing Visualizer
 *
 * Fully audio-reactive visualizer featuring:
 *   - Multiple pulse shapes (Circle, Ring, Flash, N-gon, Star, Wave)
 *   - Various color schemes (Fire, Ocean, Neon, Rainbow, etc.)
 *   - Beat detection and amplitude tracking
 *   - Configurable decay modes and animation
 *   - OpenGL rendering with shaders
 *
 * ### Module Architecture (v3.0)
 *
 * ```
 * PulsingVisualizer
 * ├── AudioSourceModule     ← FFT processing + embedded smoothing
 * │   └── SmoothingModule   ← EMA/SMA/WMA/DEMA
 * ├── ColorSchemeModule     ← Color gradients and schemes
 * └── PulseShapeModule      ← Shape generation and animation
 * ```
 *
 * ### Parameter Hierarchy
 *
 * ```
 * audio.*                   ← AudioSourceModule parameters
 *   audio.scale             ← Frequency scale (Linear/Log/Mel)
 *   audio.bands             ← Number of bands
 *   audio.gain              ← Input gain
 *   audio.smooth.*          ← Embedded SmoothingModule
 *     audio.smooth.algorithm
 *     audio.smooth.timeMs
 * shape.*                   ← PulseShapeModule parameters
 * color.*                   ← ColorSchemeModule parameters
 * anim.*                    ← Animation parameters
 * bg.*                      ← Background parameters
 * ```
 ****************************************************************************************
 */

#pragma once

#include "VisualizerBase.hpp"
#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/source/AudioSourceModule.hpp"
#include "visualizers/modules/ColorSchemeModule.hpp"
#include "visualizers/modules/PulseShapeModule.hpp"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <chrono>

/**
 * @class PulsingVisualizer
 * @brief Audio-reactive pulsing effect visualizer with modular architecture
 *
 * Renders various pulsing shapes that react to audio input.
 * Uses modular architecture with AudioSourceModule, ColorSchemeModule,
 * and PulseShapeModule for flexible configuration.
 *
 * @par Example Usage
 * @code
 * PulsingVisualizer viz;
 *
 * // Configure via parameter system
 * viz.setParam("audio.smooth.timeMs", 50.0f);
 * viz.setParam("audio.gain", 1.5f);
 * viz.setParam("shape.type", 0);  // Circle
 * viz.setParam("color.scheme", 1); // Neon
 *
 * // Or use legacy setters
 * viz.setShape(lumi::modules::PulseShape::Ring);
 * viz.setColorScheme(lumi::modules::ColorSchemeType::Fire);
 * @endcode
 */
class PulsingVisualizer : public VisualizerBase
{
public:
    PulsingVisualizer();
    ~PulsingVisualizer() override;

    // =========================================================================
    // IVisualizer Parameter Interface (overrides)
    // =========================================================================

    /**
     * @brief Check if visualizer supports parameter introspection
     */
    [[nodiscard]] bool hasParameterSupport() const override { return true; }

    /**
     * @brief Get all parameter descriptors for ConfigPanel
     * @return Vector of parameter descriptors from all modules
     */
    [[nodiscard]] std::vector<lumi::modules::ModuleParamDesc> paramDescs() const override;

    /**
     * @brief Get parameter value by hierarchical path
     * @param id Parameter path (e.g., "audio.smooth.timeMs")
     * @param out Output value
     * @return true if parameter found
     */
    [[nodiscard]] bool getParam(const std::string& id, 
                                lumi::modules::ParamValue& out) const override;

    /**
     * @brief Set parameter value by hierarchical path
     * @param id Parameter path (e.g., "audio.smooth.timeMs")
     * @param value New value
     * @return true if parameter set successfully
     */
    bool setParam(const std::string& id, 
                  const lumi::modules::ParamValue& value) override;

    // =========================================================================
    // Module Access
    // =========================================================================

    /**
     * @brief Access the audio source module
     */
    [[nodiscard]] lumi::modules::AudioSourceModule& audioSource() { return m_audioSource; }
    [[nodiscard]] const lumi::modules::AudioSourceModule& audioSource() const { return m_audioSource; }

    /**
     * @brief Access the color scheme module
     */
    [[nodiscard]] lumi::modules::ColorSchemeModule& colorSchemeModule() { return m_colorScheme; }
    [[nodiscard]] const lumi::modules::ColorSchemeModule& colorSchemeModule() const { return m_colorScheme; }

    /**
     * @brief Access the pulse shape module
     */
    [[nodiscard]] lumi::modules::PulseShapeModule& pulseShapeModule() { return m_pulseShape; }
    [[nodiscard]] const lumi::modules::PulseShapeModule& pulseShapeModule() const { return m_pulseShape; }

    // =========================================================================
    // Shape Configuration (Legacy API)
    // =========================================================================

    void setShape(lumi::modules::PulseShape shape);
    [[nodiscard]] lumi::modules::PulseShape shape() const;

    void setSides(int sides);
    [[nodiscard]] int sides() const;

    // =========================================================================
    // Color Configuration (Legacy API)
    // =========================================================================

    void setColorScheme(lumi::modules::ColorSchemeType scheme);
    [[nodiscard]] lumi::modules::ColorSchemeType colorScheme() const;

    void setColorAnimationSpeed(float cyclesPerSecond);
    [[nodiscard]] float colorAnimationSpeed() const;

    void setBeatBrightnessEnabled(bool enabled);
    [[nodiscard]] bool beatBrightnessEnabled() const;

    // =========================================================================
    // Animation Configuration (Legacy API)
    // =========================================================================

    void setTrigger(lumi::modules::PulseTrigger trigger);
    [[nodiscard]] lumi::modules::PulseTrigger trigger() const;

    void setDecay(lumi::modules::PulseDecay decay);
    [[nodiscard]] lumi::modules::PulseDecay decay() const;

    void setDecayTime(float seconds);
    [[nodiscard]] float decayTime() const;

    // =========================================================================
    // Size Configuration (Legacy API)
    // =========================================================================

    void setBaseSize(float size);
    [[nodiscard]] float baseSize() const;

    void setSizeRange(float min, float max);

    // =========================================================================
    // Rotation Configuration (Legacy API)
    // =========================================================================

    void setRotationSpeed(float degreesPerSecond);
    [[nodiscard]] float rotationSpeed() const;

    void setAudioRotationEnabled(bool enabled);
    [[nodiscard]] bool audioRotationEnabled() const;

    // =========================================================================
    // Audio Configuration (Legacy API - now delegates to AudioSourceModule)
    // =========================================================================

    void setAudioRange(int lowBand, int highBand);

    void setBeatSensitivity(float sensitivity);
    [[nodiscard]] float beatSensitivity() const;

    void setSmoothingTime(float milliseconds);
    [[nodiscard]] float smoothingTime() const;

    // =========================================================================
    // Background Configuration (Legacy API)
    // =========================================================================

    void setBackgroundSolid(bool solid);
    [[nodiscard]] bool backgroundSolid() const;

    void setBackgroundColor(float r, float g, float b);

protected:
    // =========================================================================
    // VisualizerBase Implementation
    // =========================================================================

    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    // =========================================================================
    // Audio Processing
    // =========================================================================

    /**
     * @brief Detect beat from bass level
     * @param bassLevel Current bass level
     * @return Beat intensity (0-1)
     */
    float detectBeat(float bassLevel);

    // =========================================================================
    // OpenGL Resources
    // =========================================================================

    bool createShaders();
    void updateVertexBuffer();
    void renderPulse(float audioLevel, float beatIntensity);
    void generateCircleVertices(int segments);

    // =========================================================================
    // Modules (NEW in v3.0)
    // =========================================================================

    lumi::modules::AudioSourceModule m_audioSource;  ///< FFT processing + smoothing
    lumi::modules::ColorSchemeModule m_colorScheme;  ///< Color gradients
    lumi::modules::PulseShapeModule m_pulseShape;    ///< Shape generation

    // =========================================================================
    // OpenGL Resources
    // =========================================================================

    std::unique_ptr<QOpenGLShaderProgram> m_shader;
    std::unique_ptr<QOpenGLBuffer> m_vertexBuffer;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
    int m_vertexCount = 0;

    // Uniform locations
    int m_uniformColor = -1;
    int m_uniformIntensity = -1;
    int m_uniformAspect = -1;

    // =========================================================================
    // Audio State (Beat Detection)
    // =========================================================================

    float m_beatIntensity = 0.0f;
    float m_lastBassLevel = 0.0f;
    float m_beatThreshold = 0.0f;
    float m_beatSensitivity = 1.0f;

    // Audio range (legacy)
    int m_lowBand = 0;
    int m_highBand = 8;

    // =========================================================================
    // Background
    // =========================================================================

    bool m_backgroundSolid = true;
    float m_backgroundFade = 0.1f;
    float m_bgColorR = 0.05f;
    float m_bgColorG = 0.05f;
    float m_bgColorB = 0.1f;

    // =========================================================================
    // Timing
    // =========================================================================

    std::chrono::steady_clock::time_point m_startTime;
    float m_totalTime = 0.0f;
};
