/**
 ****************************************************************************************
 * @file   MilkdropVisualizer.hpp
 * @brief  MilkDrop preset host — MD1 render core (Import-Phase Roadmap 6, M3)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Own visualizer next to MultiEffect (fixed frame pipeline, no effect chain —
 * MilkDrop_Import_Konzept.md §2.1). M3: double buffer, warp mesh with per_pixel
 * equations (per VERTEX), decay, basic waveform (modes 0-7), borders, darken
 * center, MD1 composite (video echo + gamma + brighten/darken/solarize/invert).
 * M4: custom waves + shapes (init/per_frame/per_point with t1-t8 snapshots,
 * up to 16 each — MD3 superset) and motion vectors (reverse propagation
 * through the warp mesh). Blur/shader patterns follow in M5.
 *
 * Y convention (§2.1): ONE internal math space, no per-draw flips — the single
 * vertical flip happens at the composite (presentation) pass.
 *
 * Scripts: EEL (Dialect::Milkdrop) via ScriptSlotHost — Init = per_frame_init,
 * Frame = per_frame, Point = per_pixel. q1-q64 snapshots per the M2 contract
 * (§2.4); `monitor` persists across frames. Audio inputs come from MilkLoudness
 * (bass/mid/treb relative to the long-term average + *_att).
 *
 * Threading: loadMilkFile() is called from the GUI thread under the widget's
 * renderMutex() (MainWindow import path, like MultiEffect::loadAvsFile); it
 * touches no GL. All GL objects live in the render thread (context-change
 * detection like PulsingVisualizer).
 ****************************************************************************************
 */

#pragma once

#include "visualizers/VisualizerBase.hpp"

#include "scripting/ScriptContext.hpp"
#include "scripting/ScriptSlotHost.hpp"
#include "visualizers/milkdrop/MilkdropPresetState.hpp"
#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/processing/MilkLoudness.hpp"
#include "visualizers/render/FeedbackBuffer.hpp"
#include "visualizers/render/ScopeRenderer.hpp"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QString>
#include <QStringList>

#include <array>
#include <memory>
#include <vector>

class QOpenGLContext;
class QOpenGLFunctions;

/**
 * @class MilkdropVisualizer
 * @brief MD1 MilkDrop pipeline: warp feedback + waveform + composite
 */
class MilkdropVisualizer : public VisualizerBase
{
public:
    static constexpr int kDefaultMeshX = 32;  ///< original default grid
    static constexpr int kDefaultMeshY = 24;
    static constexpr int kMaxMeshX = 96;      ///< cap, consistent with the DM cap (§6.1)
    static constexpr int kMaxMeshY = 72;
    static constexpr int kWaveSamples = 480;  ///< NUM_WAVEFORM_SAMPLES (defines.h:188)
    static constexpr int kWaveBuffer = 576;   ///< sample buffer incl. peek-ahead

    MilkdropVisualizer();
    ~MilkdropVisualizer() override = default;

    /**
     * @brief Load and translate a .milk preset (no GL; call under renderMutex())
     * @param report Optional import notes (parser warnings, transpile errors)
     * @return false when the file is not a MilkDrop preset at all
     */
    bool loadMilkFile(const QString& path, QStringList* report = nullptr);

    /// @brief Currently loaded preset state (for tests/panel)
    [[nodiscard]] const lumi::milkdrop::PresetState& presetState() const { return m_state; }

    // =========================================================================
    // Parameters (generic ConfigPanel + VisualizerPresetManager support)
    // =========================================================================

    [[nodiscard]] bool hasParameterSupport() const override { return true; }
    [[nodiscard]] std::vector<lumi::modules::ModuleParamDesc> paramDescs() const override;
    [[nodiscard]] bool getParam(const std::string& id,
                                lumi::modules::ParamValue& out) const override;
    bool setParam(const std::string& id, const lumi::modules::ParamValue& value) override;
    void resetToDefaults() override;

protected:
    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    // --- per-frame variable block (post-per_frame values driving the render) ---------
    struct FrameVars
    {
        double zoom = 1.0, zoomExp = 1.0, rot = 0.0, warp = 1.0;
        double cx = 0.5, cy = 0.5, dx = 0.0, dy = 0.0, sx = 1.0, sy = 1.0;
        double decay = 0.98, gamma = 2.0;
        double echoZoom = 2.0, echoAlpha = 0.0;
        int echoOrient = 0;
        double waveA = 0.8, waveR = 1.0, waveG = 1.0, waveB = 1.0;
        double waveX = 0.5, waveY = 0.5, waveMystery = 0.0;
        int waveMode = 0;
        bool waveDots = false, waveThick = false, waveAdditive = false, waveBrighten = true;
        bool darkenCenter = false, wrap = true;
        bool invert = false, brighten = false, darken = false, solarize = false;
        double obSize = 0.01, obR = 0, obG = 0, obB = 0, obA = 0;
        double ibSize = 0.01, ibR = 0.25, ibG = 0.25, ibB = 0.25, ibA = 0;
        double mvX = 12.0, mvY = 9.0, mvDX = 0.0, mvDY = 0.0, mvL = 0.9;
        double mvR = 1.0, mvG = 1.0, mvB = 1.0, mvA = 1.0;
    };

    /// One custom wave/shape at runtime: definition + its own script host
    /// (shared preset context) + the frozen post-init t1-t8 values
    struct WaveRuntime
    {
        lumi::milkdrop::WaveState def;
        std::unique_ptr<lumi::scripting::ScriptSlotHost> script;
        std::array<double, 8> tInit{};
    };
    struct ShapeRuntime
    {
        lumi::milkdrop::ShapeState def;
        std::unique_ptr<lumi::scripting::ScriptSlotHost> script;
        std::array<double, 8> tInit{};
    };

    // --- script plumbing ---------------------------------------------------------------
    void rebuildScripts(QStringList* report);
    void runPerFrameInit();
    void pushFrameInputs();                 ///< preset values + audio/time into the engine
    void pullFrameOutputs(FrameVars& fv);   ///< read i/o vars back after per_frame
    void updateAudio(float deltaTime);
    void pushCommonInputs(lumi::scripting::LuaScriptEngine& engine);  ///< audio/time set

    // --- render passes (render thread, context current) ---------------------------------
    bool ensureGlResources();
    void releaseGlResources();
    void computeWarpMesh(const FrameVars& fv);
    void drawWarpPass(const FrameVars& fv);
    void drawMotionVectors(const FrameVars& fv);
    void drawCustomShapes();
    void drawCustomWaves();
    void drawBasicWave(const FrameVars& fv);
    void drawBorders(const FrameVars& fv);
    void drawDarkenCenter();
    void compositeToScreen(const FrameVars& fv);
    void drawColorQuads(const float* vertexData, int vertexCount, unsigned int glMode);

    /// bilinear lookup in the warp-mesh UVs: where did (fx,fy) come from?
    [[nodiscard]] bool reversePropagate(double fx, double fy, double& outX,
                                        double& outY) const;

    // --- preset -------------------------------------------------------------------------
    lumi::milkdrop::PresetState m_state;
    std::shared_ptr<lumi::scripting::ScriptContext> m_context;
    std::unique_ptr<lumi::scripting::ScriptSlotHost> m_script;
    std::vector<WaveRuntime> m_waveRt;      ///< enabled custom waves (M4)
    std::vector<ShapeRuntime> m_shapeRt;    ///< enabled custom shapes (M4)
    bool m_initRan = false;
    double m_monitor = 0.0;                 ///< persists across frames (original)

    // --- audio / time ---------------------------------------------------------------------
    lumi::modules::MilkLoudness m_loudness;
    double m_time = 0.0;
    double m_fps = 60.0;
    long m_frame = 0;
    std::array<float, kWaveBuffer> m_waveL{};     ///< smoothed waveform (basic wave, spec §0)
    std::array<float, kWaveBuffer> m_waveR{};
    std::array<float, kWaveBuffer> m_waveRawL{};  ///< unsmoothed (custom waves filter selbst)
    std::array<float, kWaveBuffer> m_waveRawR{};
    std::vector<float> m_spectrumL;               ///< frame copies (custom spectrum waves)
    std::vector<float> m_spectrumR;

    // --- GL ------------------------------------------------------------------------------
    QOpenGLContext* m_lastContext = nullptr;
    lumi::render::FeedbackBuffer m_feedback;
    lumi::render::ScopeRenderer m_scope;
    std::unique_ptr<QOpenGLShaderProgram> m_warpProgram;    ///< pos+uv, prev texture × decay
    std::unique_ptr<QOpenGLShaderProgram> m_textureProgram; ///< pos+uv quad × uniform color
    std::unique_ptr<QOpenGLShaderProgram> m_colorProgram;   ///< pos+rgba (borders/filters)
    std::unique_ptr<QOpenGLShaderProgram> m_shapeProgram;   ///< pos+uv+rgba (textured shapes)
    std::unique_ptr<QOpenGLVertexArrayObject> m_shapeVao;
    std::unique_ptr<QOpenGLBuffer> m_shapeVbo;
    std::unique_ptr<QOpenGLVertexArrayObject> m_meshVao;
    std::unique_ptr<QOpenGLBuffer> m_meshVbo;
    std::unique_ptr<QOpenGLVertexArrayObject> m_quadVao;
    std::unique_ptr<QOpenGLBuffer> m_quadVbo;
    std::vector<float> m_meshData;          ///< triangulated pos.xy + uv per vertex
    std::vector<double> m_vertexUv;         ///< (gx+1)*(gy+1)*2 warp UVs (math space)

    // --- parameters ----------------------------------------------------------------------
    int m_meshX = kDefaultMeshX;
    int m_meshY = kDefaultMeshY;

    // aspect factors of the internal surface (plugin.cpp:2035)
    double m_aspectX = 1.0, m_aspectY = 1.0;
};
