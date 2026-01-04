/**
 ****************************************************************************************
 * @file   SuperscopeVisualizer.hpp
 * @brief  Programmable point/line visualizer inspired by Winamp AVS Superscope
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * SuperscopeVisualizer renders programmable shapes using mathematical expressions.
 * Features:
 * - Multiple builtin presets (Circle, Spiral, Lissajous, etc.)
 * - Dots, Lines, or Thick Lines rendering
 * - Color gradients and per-point coloring
 * - Glow effects
 * - Beat-reactive animation
 ****************************************************************************************
 */

#pragma once

#include "VisualizerBase.hpp"
#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/source/AudioSourceModule.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <vector>
#include <deque>

// Forward declarations
class QOpenGLContext;

/**
 * @class SuperscopeVisualizer
 * @brief Programmable point/line visualizer with expression support
 *
 * Features:
 * - 14 builtin presets (Horizontal Scope, Circle, Spiral, etc.)
 * - Dots, Lines, Thick Lines render modes
 * - Color gradient support
 * - Glow effect with customizable intensity
 * - Beat-reactive animation
 * - Aspect ratio correction
 */
class SuperscopeVisualizer : public VisualizerBase
{
public:
    SuperscopeVisualizer();
    ~SuperscopeVisualizer() override;

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
    [[nodiscard]] lumi::modules::SuperscopeModule* superscope() { return &m_superscope; }

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

    void renderPoints(const std::vector<lumi::modules::SuperscopePoint>& points);
    void renderLines(const std::vector<lumi::modules::SuperscopePoint>& points);
    void renderLinesAsTriangleStrip(const std::vector<lumi::modules::SuperscopePoint>& points, float lineWidth);
    void renderThickLines(const std::vector<lumi::modules::SuperscopePoint>& points);

    void uploadVertexData(const std::vector<float>& vertices);

    // =========================================================================
    // Modules
    // =========================================================================

    lumi::modules::AudioSourceModule m_audioSource;
    lumi::modules::SuperscopeModule m_superscope;

    // =========================================================================
    // OpenGL Resources
    // =========================================================================

    std::unique_ptr<QOpenGLShaderProgram> m_pointShader;
    std::unique_ptr<QOpenGLShaderProgram> m_lineShader;
    std::unique_ptr<QOpenGLBuffer> m_vertexBuffer;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;

    // Point shader uniforms
    int m_pointUniPointSize = -1;
    int m_pointUniGlowEnabled = -1;
    int m_pointUniGlowIntensity = -1;

    // Line shader uniforms
    int m_lineUniAlpha = -1;

    // =========================================================================
    // Audio Data Buffers
    // =========================================================================

    std::vector<float> m_waveformLeft;
    std::vector<float> m_waveformRight;
    std::vector<float> m_spectrumLeft;
    std::vector<float> m_spectrumRight;

    // =========================================================================
    // Beat Detection State
    // =========================================================================

    float m_beatEnergy = 0.0f;
    float m_beatThreshold = 0.0f;
    bool m_isBeat = false;

    // =========================================================================
    // Hold/Fade Frame History
    // =========================================================================

    struct HeldFrame
    {
        std::vector<lumi::modules::SuperscopePoint> points;
        float age = 0.0f;
        float alpha = 1.0f;
    };

    std::deque<HeldFrame> m_heldFrames;

    void updateHeldFrames(float deltaTime);
    void renderHeldFrames();

    // =========================================================================
    // State
    // =========================================================================

    float m_totalTime = 0.0f;

    // =========================================================================
    // OpenGL Context Tracking
    // =========================================================================

    QOpenGLContext* m_lastContext = nullptr;
};
