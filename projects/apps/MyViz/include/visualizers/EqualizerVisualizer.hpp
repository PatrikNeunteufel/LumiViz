/**
 ****************************************************************************************
 * @file   EqualizerVisualizer.hpp
 * @brief  Spectrum analyzer visualizer with bars, peak markers and particles
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * Classic equalizer/spectrum display with:
 * - Vertical bars mapped to frequency bands
 * - Configurable frequency scales (Linear/Log/Mel)
 * - Peak hold markers with physics
 * - Optional particle effects
 * - 8-stop gradient color mapping (like WaveformVisualizer)
 ****************************************************************************************
 */

#pragma once

#include "VisualizerBase.hpp"
#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/source/AudioSourceModule.hpp"
#include "visualizers/modules/EqualizerModule.hpp"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <vector>

// Forward declarations
class QOpenGLContext;

/**
 * @class EqualizerVisualizer
 * @brief Spectrum analyzer with bars and peak markers
 */
class EqualizerVisualizer : public VisualizerBase
{
public:
    EqualizerVisualizer();
    ~EqualizerVisualizer() override;

    // =========================================================================
    // IVisualizer Parameter Interface
    // =========================================================================

    [[nodiscard]] bool hasParameterSupport() const override { return true; }
    [[nodiscard]] std::vector<lumi::modules::ModuleParamDesc> paramDescs() const override;
    [[nodiscard]] bool getParam(const std::string& id, lumi::modules::ParamValue& out) const override;
    bool setParam(const std::string& id, const lumi::modules::ParamValue& value) override;
    void resetToDefaults() override;

    // =========================================================================
    // IVisualizer Audio Interface
    // =========================================================================

    void updateSpectrum(const float* spectrum, int count) override;

    // =========================================================================
    // Module Access
    // =========================================================================

    [[nodiscard]] lumi::modules::AudioSourceModule* audioSource() { return &m_audioSource; }

    /// @brief Audio-source handle (Phase 4 — generic module-preset access)
    [[nodiscard]] lumi::modules::AudioSourceModule* audioSourceModule() override { return &m_audioSource; }

    [[nodiscard]] lumi::modules::EqualizerModule& equalizerModule() { return m_equalizer; }
    [[nodiscard]] const lumi::modules::EqualizerModule& equalizerModule() const { return m_equalizer; }

    /// @brief Gradient handles (Phase 4 — generic editor/preview access)
    [[nodiscard]] std::vector<GradientHandle> gradients() override
    {
        return {{"main", "Color", "color.", &m_equalizer.colorGradient()}};
    }

    /// @brief Tap points (Phase 4 pilot) — stage outputs for the group preview
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
             }},
            {"tap.map", "Mapping (Bandwerte)", PipelineStage::Mapping,
             [this]() { return m_equalizer.bands(); }},
        };
    }

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
    void uploadGradientUniforms();
    void renderBars();
    void renderPeaks();
    void renderParticles();

    // =========================================================================
    // Modules
    // =========================================================================

    lumi::modules::AudioSourceModule m_audioSource;
    lumi::modules::EqualizerModule m_equalizer;

    // =========================================================================
    // OpenGL Resources
    // =========================================================================

    // Bar shader (gradient-based)
    std::unique_ptr<QOpenGLShaderProgram> m_barShader;
    std::unique_ptr<QOpenGLBuffer> m_barVertexBuffer;
    std::unique_ptr<QOpenGLVertexArrayObject> m_barVao;
    
    // Peak shader (direct color per vertex)
    std::unique_ptr<QOpenGLShaderProgram> m_peakShader;
    std::unique_ptr<QOpenGLBuffer> m_peakVertexBuffer;
    std::unique_ptr<QOpenGLVertexArrayObject> m_peakVao;
    
    // Bar shader uniforms (8-stop gradient)
    int m_uniColor[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    int m_uniStopPos = -1;
    int m_uniStopPos2 = -1;
    int m_uniStopCount = -1;
    int m_uniGradientMode = -1;
    int m_uniGradientAngle = -1;
    int m_uniAlpha = -1;
    
    // Peak shader uniforms
    int m_peakUniAlpha = -1;

    // =========================================================================
    // Audio State
    // =========================================================================

    std::vector<float> m_spectrumData;
    int m_spectrumCount = 0;
    bool m_hasNewSpectrum = false;
    mutable QMutex m_spectrumMutex;

    // =========================================================================
    // Rendering State
    // =========================================================================

    float m_totalTime = 0.0f;
    
    // =========================================================================
    // OpenGL Context Tracking
    // =========================================================================
    
    QOpenGLContext* m_lastContext = nullptr;
};
