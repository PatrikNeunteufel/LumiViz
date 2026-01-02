/**
 ****************************************************************************************
 * @file   WaveformVisualizer.hpp
 * @brief  Audio waveform visualizer with multiple display styles
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 2.0.0
 *
 * @details
 * Displays audio waveform data as an oscillating line, bars, or mirrored display.
 * Uses modular architecture with AudioSourceModule and WaveformModule.
 ****************************************************************************************
 */

#pragma once

#include "VisualizerBase.hpp"
#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/source/AudioSourceModule.hpp"
#include "visualizers/modules/WaveformModule.hpp"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <vector>
#include <chrono>

/**
 * @class WaveformVisualizer
 * @brief Audio waveform display with multiple styles
 *
 * Visualizes audio waveform data in real-time with customizable appearance.
 * 
 * @par Module Architecture
 * - AudioSourceModule: Audio input and preprocessing
 * - WaveformModule: Display style and appearance (includes ColorGradientModule)
 */
class WaveformVisualizer : public VisualizerBase
{
public:
    WaveformVisualizer();
    ~WaveformVisualizer() override;

    // =========================================================================
    // IVisualizer Parameter Interface
    // =========================================================================

    [[nodiscard]] bool hasParameterSupport() const override { return true; }
    [[nodiscard]] std::vector<lumi::modules::ModuleParamDesc> paramDescs() const override;
    [[nodiscard]] bool getParam(const std::string& id, lumi::modules::ParamValue& out) const override;
    bool setParam(const std::string& id, const lumi::modules::ParamValue& value) override;

    // =========================================================================
    // IVisualizer Audio Interface
    // =========================================================================

    [[nodiscard]] bool usesAudio() const override { return true; }
    void updateWaveform(const float* waveform, int count) override;
    void updateSpectrum(const float* spectrum, int count) override;

    // =========================================================================
    // Module Access
    // =========================================================================

    /**
     * @brief Get access to the audio source module
     * @return Pointer to the audio source module (never null)
     */
    [[nodiscard]] lumi::modules::AudioSourceModule* audioSource() { return &m_audioSource; }

    /**
     * @brief Get access to the waveform display module
     * @return Pointer to the waveform module (never null)
     */
    [[nodiscard]] lumi::modules::WaveformModule* waveform() { return &m_waveform; }

    /**
     * @brief Reset all parameters to hardcoded defaults
     */
    void resetToDefaults() override;

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

    bool createShaders();

    // =========================================================================
    // Modules
    // =========================================================================

    lumi::modules::AudioSourceModule m_audioSource;
    lumi::modules::WaveformModule m_waveform;

    // =========================================================================
    // OpenGL Resources
    // =========================================================================

    std::unique_ptr<QOpenGLShaderProgram> m_shader;
    std::unique_ptr<QOpenGLShaderProgram> m_glowShader;
    std::unique_ptr<QOpenGLBuffer> m_vertexBuffer;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;

    // Uniform locations
    int m_uniformAspect = -1;
    int m_uniformAmplitude = -1;
    int m_uniformColorMode = -1;
    int m_uniformColor0 = -1;
    int m_uniformColor1 = -1;
    int m_uniformTime = -1;

    // =========================================================================
    // Waveform Display Buffer
    // =========================================================================

    std::vector<float> m_smoothedWaveform;
    std::vector<float> m_displayWaveform;

    // =========================================================================
    // Background
    // =========================================================================

    float m_bgColorR = 0.02f;
    float m_bgColorG = 0.02f;
    float m_bgColorB = 0.05f;

    // =========================================================================
    // Animation
    // =========================================================================

    float m_totalTime = 0.0f;
    std::chrono::steady_clock::time_point m_startTime;
};
