/**
 ****************************************************************************************
 * @file   PulsingVisualizer.hpp
 * @brief  Audio-reactive pulsing visualizer with modular architecture
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 3.0.0
 ****************************************************************************************
 */

#pragma once

#include "VisualizerBase.hpp"
#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/source/AudioSourceModule.hpp"
#include "visualizers/modules/ColorGradientModule.hpp"
#include "visualizers/modules/PulseShapeModule.hpp"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <array>
#include <vector>
#include <chrono>

// Forward declarations
class QOpenGLContext;

/**
 * @class PulsingVisualizer
 * @brief Audio-reactive pulsing effect visualizer
 */
class PulsingVisualizer : public VisualizerBase
{
public:
    PulsingVisualizer();
    ~PulsingVisualizer() override;

    // =========================================================================
    // IVisualizer Parameter Interface
    // =========================================================================

    [[nodiscard]] bool hasParameterSupport() const override { return true; }
    [[nodiscard]] std::vector<lumi::modules::ModuleParamDesc> paramDescs() const override;
    [[nodiscard]] bool getParam(const std::string& id, lumi::modules::ParamValue& out) const override;
    bool setParam(const std::string& id, const lumi::modules::ParamValue& value) override;

    // =========================================================================
    // Legacy API - Shape Configuration
    // =========================================================================

    void setShape(lumi::modules::PulseShape shape);
    [[nodiscard]] lumi::modules::PulseShape shape() const;

    void setSides(int sides);
    [[nodiscard]] int sides() const;

    // =========================================================================
    // Legacy API - Color/Gradient Configuration
    // =========================================================================

    void loadGradientPreset(const std::string& name);
    
    /**
     * @brief Get access to the color gradient module for editing
     * @return Pointer to the gradient module (never null)
     */
    [[nodiscard]] lumi::modules::ColorGradientModule* colorGradient() { return &m_colorGradient; }
    
    /**
     * @brief Get access to the audio source module
     * @return Pointer to the audio source module (never null)
     */
    [[nodiscard]] lumi::modules::AudioSourceModule* audioSource() { return &m_audioSource; }
    
    /**
     * @brief Reset all parameters to hardcoded defaults
     */
    void resetToDefaults() override;

    void setRotationSpeed(float degreesPerSecond);
    [[nodiscard]] float rotationSpeed() const;

    void setBeatReverseRotation(bool enabled);
    [[nodiscard]] bool beatReverseRotation() const;

    void setBeatBrightnessEnabled(bool enabled);
    [[nodiscard]] bool beatBrightnessEnabled() const;

    // =========================================================================
    // Legacy API - Audio Configuration
    // =========================================================================

    void setBeatSensitivity(float sensitivity);
    [[nodiscard]] float beatSensitivity() const;

    void setSmoothingTime(float milliseconds);
    [[nodiscard]] float smoothingTime() const;

    // =========================================================================
    // Legacy API - Background Configuration
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
    // Private Methods
    // =========================================================================

    float detectBeat(float bassLevel);
    bool createShaders();
    void updateVertexBuffer();
    void renderPulse(float audioLevel, float beatIntensity);
    void rebuildShape();

    // =========================================================================
    // Modules
    // =========================================================================

    lumi::modules::AudioSourceModule m_audioSource;
    lumi::modules::ColorGradientModule m_colorGradient;
    lumi::modules::PulseShapeModule m_pulseShape;

    // =========================================================================
    // OpenGL Resources
    // =========================================================================

    std::unique_ptr<QOpenGLShaderProgram> m_shader;
    std::unique_ptr<QOpenGLBuffer> m_vertexBuffer;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
    int m_vertexCount = 0;
    bool m_needsRebuild = true;

    // Uniform locations
    int m_uniformAspect = -1;
    int m_uniformSize = -1;
    int m_uniformRotation = -1;
    int m_uniformInnerRadius = -1;
    int m_uniformOutlineWidth = -1;
    // For multi-stop gradients we pass texture or colors array
    int m_uniformColorMode = -1;
    int m_uniformGradientAngle = -1;
    int m_uniformColor0 = -1;
    int m_uniformColor1 = -1;
    int m_uniformColor2 = -1;
    int m_uniformColor3 = -1;
    int m_uniformColor4 = -1;
    int m_uniformColor5 = -1;
    int m_uniformColor6 = -1;
    int m_uniformColor7 = -1;
    int m_uniformStopPos = -1;    // vec4 with stop positions 0-3
    int m_uniformStopPos2 = -1;   // vec4 with stop positions 4-7
    int m_uniformMidpoints = -1;  // vec4 with midpoints 0-3
    int m_uniformMidpoints2 = -1; // vec4 with midpoints 4-6
    int m_uniformStopCount = -1;

    // =========================================================================
    // Shape Parameters
    // =========================================================================

    float m_innerRadius = 0.5f;
    float m_minSize = 0.3f;
    float m_maxSize = 0.9f;
    float m_rotationSpeed = 0.0f;
    float m_currentRotation = 0.0f;
    bool m_beatReverseRotation = false;  ///< Reverse rotation on beat
    float m_rotationDirection = 1.0f;    ///< Current direction (+1 or -1)

    // =========================================================================
    // Audio State
    // =========================================================================

    float m_beatIntensity = 0.0f;
    float m_lastBassLevel = 0.0f;
    float m_beatThreshold = 0.4f;
    float m_beatSensitivity = 1.0f;
    bool m_beatBrightnessEnabled = true;
    int m_lowBand = 0;
    int m_highBand = 8;

    // =========================================================================
    // Background
    // =========================================================================

    bool m_backgroundSolid = true;
    float m_backgroundFade = 0.1f;
    float m_bgColorR = 0.02f;
    float m_bgColorG = 0.02f;
    float m_bgColorB = 0.05f;

    // =========================================================================
    // Timing
    // =========================================================================

    std::chrono::steady_clock::time_point m_startTime;
    float m_totalTime = 0.0f;
    
    // =========================================================================
    // OpenGL Context Tracking
    // =========================================================================
    
    QOpenGLContext* m_lastContext = nullptr;  ///< Track context for resource validity
};
