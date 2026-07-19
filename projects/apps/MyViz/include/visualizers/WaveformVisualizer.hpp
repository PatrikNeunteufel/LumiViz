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
#include "visualizers/modules/processing/SmoothingModule.hpp"
#include "visualizers/modules/postfx/PostFxModule.hpp"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <vector>

// Forward declarations
class QOpenGLContext;

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

    void updateWaveform(const float* waveform, int count) override;
    void updateSpectrum(const float* spectrum, int count) override;

    // =========================================================================
    // Module Access
    // =========================================================================

    [[nodiscard]] lumi::modules::AudioSourceModule* audioSource() { return &m_audioSource; }
    [[nodiscard]] lumi::modules::WaveformModule* waveform() { return &m_waveform; }

    /// @brief Audio-source handle (Phase 4 — generic module-preset access)
    [[nodiscard]] lumi::modules::AudioSourceModule* audioSourceModule() override { return &m_audioSource; }

    /// @brief Tap points (Phase 4 Schritt 6) — stage outputs for the group preview
    [[nodiscard]] std::vector<TapPoint> tapPoints() override
    {
        using lumi::modules::PipelineStage;
        return {
            {"tap.audio", "Analyse (Baender)", PipelineStage::AudioSource,
             [this]() {
                 const float* data = m_audioSource.spectrum();
                 const int count = m_audioSource.bandCount();
                 return (data != nullptr && count > 0)
                     ? std::vector<float>(data, data + count)
                     : std::vector<float>{};
             },
             TapDisplay::Bars},
            {"tap.map", "Mapping (Samples Mono)", PipelineStage::Mapping,
             [this]() { return m_displayMono; }, TapDisplay::Curve},
        };
    }

    /// @brief Gradient handles (Phase 4) — makes Left/Right editable too
    [[nodiscard]] std::vector<GradientHandle> gradients() override
    {
        using WM = lumi::modules::WaveformModule;
        return {
            {"mono", "Mono", "color.mono.", &m_waveform.colorGradient(WM::CHANNEL_MONO)},
            {"left", "Left", "color.left.", &m_waveform.colorGradient(WM::CHANNEL_LEFT)},
            {"right", "Right", "color.right.", &m_waveform.colorGradient(WM::CHANNEL_RIGHT)},
        };
    }

    void resetToDefaults() override;

protected:
    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    bool createShaders();

    /// @brief Mirror the stage-1 smoothing config into the display smoothers (E3)
    void syncDisplaySmoothing();
    
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

    // Display smoothing (E3 — config mirrors audio.smooth.*, per-index state)
    lumi::modules::SmoothingModule m_displaySmoothMono;
    lumi::modules::SmoothingModule m_displaySmoothLeft;
    lumi::modules::SmoothingModule m_displaySmoothRight;

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
    std::vector<float> m_displayLeft;
    std::vector<float> m_displayRight;
    std::vector<float> m_displayMono;

    // =========================================================================
    // Hold/Fade Trails (per channel — shared PostFx mechanics, 5.6)
    // =========================================================================

    lumi::modules::HoldFadeEffect m_heldFramesMono;
    lumi::modules::HoldFadeEffect m_heldFramesLeft;
    lumi::modules::HoldFadeEffect m_heldFramesRight;

    // =========================================================================
    // State
    // =========================================================================

    float m_totalTime = 0.0f;
    
    // =========================================================================
    // OpenGL Context Tracking
    // =========================================================================
    
    QOpenGLContext* m_lastContext = nullptr;  ///< Track context for resource validity
};
