/**
 ****************************************************************************************
 * @file   OscilloscopeVisualizer.hpp
 * @brief  Classic oscilloscope-style audio visualizer with trigger and grid
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "VisualizerBase.hpp"
#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/source/AudioSourceModule.hpp"
#include "visualizers/modules/OscilloscopeModule.hpp"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <vector>
#include <deque>

// Forward declarations
class QOpenGLContext;

/**
 * @brief Phosphor persistence frame for fade effect
 */
struct PhosphorFrame
{
    std::vector<float> samples;
    float age = 0.0f;
    float alpha = 1.0f;
};

/**
 * @class OscilloscopeVisualizer
 * @brief Classic oscilloscope-style audio visualizer
 *
 * Features:
 * - Trigger synchronization (rising/falling edge)
 * - Timebase control (time/division)
 * - Dual channel support (Left/Right)
 * - Grid with divisions (10x8)
 * - Phosphor persistence effect
 * - Classic green/yellow/cyan color schemes
 */
class OscilloscopeVisualizer : public VisualizerBase
{
public:
    OscilloscopeVisualizer();
    ~OscilloscopeVisualizer() override;

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

    void updateWaveform(const float* waveform, int count) override;
    void updateSpectrum(const float* spectrum, int count) override;

    // =========================================================================
    // Module Access
    // =========================================================================

    [[nodiscard]] lumi::modules::AudioSourceModule* audioSource() { return &m_audioSource; }
    [[nodiscard]] lumi::modules::OscilloscopeModule* oscilloscope() { return &m_oscilloscope; }

    void resetToDefaults() override;

protected:
    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    // =========================================================================
    // Private Methods
    // =========================================================================

    bool createShaders();

    void splitStereoData(const std::vector<float>& interleaved,
                         std::vector<float>& left,
                         std::vector<float>& right);

    void resampleWaveform(const std::vector<float>& source,
                          std::vector<float>& target,
                          int targetSize,
                          float gain);

    void buildLineVertices(const std::vector<float>& samples,
                           float yOffset,
                           float yScale,
                           float lineWidth,
                           std::vector<float>& vertices);

    void renderGrid();
    void renderChannel(int channelIndex,
                       const std::vector<float>& samples,
                       const lumi::modules::ChannelConfigBase& config);
    void renderTriggerLevel();

    void uploadGradientUniforms(int channelIndex);

    void updatePhosphorFrames(float deltaTime);

    // =========================================================================
    // Modules
    // =========================================================================

    lumi::modules::AudioSourceModule m_audioSource;
    lumi::modules::OscilloscopeModule m_oscilloscope;

    // =========================================================================
    // OpenGL Resources
    // =========================================================================

    std::unique_ptr<QOpenGLShaderProgram> m_lineShader;
    std::unique_ptr<QOpenGLShaderProgram> m_gridShader;
    std::unique_ptr<QOpenGLBuffer> m_vertexBuffer;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;

    // Line shader uniforms (8-stop gradient)
    int m_lineUniColor[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    int m_lineUniStopPos = -1;
    int m_lineUniStopPos2 = -1;
    int m_lineUniStopCount = -1;
    int m_lineUniGradientMode = -1;
    int m_lineUniGradientAngle = -1;
    int m_lineUniAlpha = -1;

    // Grid shader uniforms
    int m_gridUniColor = -1;

    // =========================================================================
    // Waveform Data Buffers
    // =========================================================================

    std::vector<float> m_rawWaveformLeft;
    std::vector<float> m_rawWaveformRight;
    
    // Processed channel data (6 channels: CH1-CH4, M1-M2)
    std::array<std::vector<float>, lumi::modules::OscilloscopeModule::TOTAL_CHANNELS> m_processedChannels;
    std::array<std::vector<float>, lumi::modules::OscilloscopeModule::TOTAL_CHANNELS> m_displayChannels;

    // =========================================================================
    // Phosphor Persistence Buffers (per channel)
    // =========================================================================

    std::array<std::deque<PhosphorFrame>, lumi::modules::OscilloscopeModule::TOTAL_CHANNELS> m_phosphorBuffers;

    // =========================================================================
    // Trigger State
    // =========================================================================

    int m_lastTriggerPoint = 0;
    bool m_triggered = false;
    float m_holdoffTimer = 0.0f;

    // =========================================================================
    // State
    // =========================================================================

    float m_totalTime = 0.0f;

    // =========================================================================
    // OpenGL Context Tracking
    // =========================================================================

    QOpenGLContext* m_lastContext = nullptr;  ///< Track context for resource validity
};
