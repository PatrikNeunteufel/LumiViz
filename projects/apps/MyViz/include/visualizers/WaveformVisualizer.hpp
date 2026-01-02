/**
 ****************************************************************************************
 * @file   WaveformVisualizer.hpp
 * @brief  Advanced audio waveform visualizer with 8-stop gradient and per-channel settings
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 4.0.0
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
#include <deque>

/**
 * @brief Held frame for fade effect
 */
struct HeldWaveformFrame
{
    std::vector<float> samples;
    float age = 0.0f;
    float alpha = 1.0f;
};

/**
 * @class WaveformVisualizer
 * @brief Advanced audio waveform with per-channel settings and 8-stop gradient
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

    [[nodiscard]] lumi::modules::AudioSourceModule* audioSource() { return &m_audioSource; }
    [[nodiscard]] lumi::modules::WaveformModule* waveform() { return &m_waveform; }

    void resetToDefaults() override;

protected:
    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    bool createShaders();
    
    void splitStereoData(const std::vector<float>& interleaved,
                         std::vector<float>& left,
                         std::vector<float>& right);
    
    void resampleWaveform(const std::vector<float>& source,
                          std::vector<float>& target,
                          std::vector<float>& smoothed,
                          int targetSize,
                          float smoothing,
                          float gain);
    
    void buildThickLineVertices(const std::vector<float>& samples,
                                float offset,
                                float amplitude,
                                float lineWidth,
                                std::vector<float>& vertices);
    
    void buildFillVertices(const std::vector<float>& samples,
                           float offset,
                           float amplitude,
                           std::vector<float>& vertices);
    
    void updateHeldFrames(float deltaTime);
    
    /// @brief Render a single channel
    /// @param channelIndex 0=Mono, 1=Left, 2=Right
    void renderChannel(int channelIndex,
                       const std::vector<float>& samples,
                       const lumi::modules::WaveformChannelConfig& config,
                       float alpha);
    
    /// @brief Upload gradient uniforms to shader for specific channel
    void uploadGradientUniforms(QOpenGLShaderProgram* shader, int channelIndex, bool isLine);

    // =========================================================================
    // Modules
    // =========================================================================

    lumi::modules::AudioSourceModule m_audioSource;
    lumi::modules::WaveformModule m_waveform;

    // =========================================================================
    // OpenGL Resources
    // =========================================================================

    std::unique_ptr<QOpenGLShaderProgram> m_lineShader;
    std::unique_ptr<QOpenGLShaderProgram> m_fillShader;
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
    
    // Fill shader uniforms (8-stop gradient)
    int m_fillUniColor[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    int m_fillUniStopPos = -1;
    int m_fillUniStopPos2 = -1;
    int m_fillUniStopCount = -1;
    int m_fillUniGradientMode = -1;
    int m_fillUniGradientAngle = -1;
    int m_fillUniAlpha = -1;
    int m_fillUniBrightness = -1;

    // =========================================================================
    // Waveform Data Buffers
    // =========================================================================

    std::vector<float> m_rawWaveformLeft;
    std::vector<float> m_rawWaveformRight;
    std::vector<float> m_smoothedLeft;
    std::vector<float> m_smoothedRight;
    std::vector<float> m_smoothedMono;
    std::vector<float> m_displayLeft;
    std::vector<float> m_displayRight;
    std::vector<float> m_displayMono;

    // =========================================================================
    // Hold/Fade Buffers (per channel)
    // =========================================================================

    std::deque<HeldWaveformFrame> m_heldFramesMono;
    std::deque<HeldWaveformFrame> m_heldFramesLeft;
    std::deque<HeldWaveformFrame> m_heldFramesRight;

    // =========================================================================
    // State
    // =========================================================================

    float m_totalTime = 0.0f;
};
