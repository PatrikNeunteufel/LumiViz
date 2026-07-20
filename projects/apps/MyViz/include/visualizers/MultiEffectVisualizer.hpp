/**
 ****************************************************************************************
 * @file   MultiEffectVisualizer.hpp
 * @brief  Multi-effect chain host — renders an AVS-style effect chain
 *         (Import Roadmap 5.1 skeleton + 5.2 blend engine/nesting)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.2.0
 *
 * @details
 * Hosts a lumi::multieffect::ChainNode tree and renders it after the AVS model
 * (analysis doc §5.1): a persistent ping-pong FBO pair as working surface —
 * render effects draw in place on the current buffer, transform effects read
 * current → write partner → swap. The frame result is blitted to the window's
 * framebuffer at the end.
 *
 * 5.2: non-root lists own a persistent buffer pair (AVS thisfb) — the parent
 * image is blended in (in-blend), the children render on the list surface, the
 * result is blended back (out-blend). Blend batch 1 per decision E3; the rest
 * falls back to Replace (compile warning). OnBeat activation and the EEL list
 * slot pair (init/frame → enabled/clear/beat/alphain/alphaout) run per list;
 * all script hosts share one preset-local ScriptContext (decision §10.3).
 *
 * Threading contract (Visualizer_Architecture §12): all GL objects live on the
 * render thread; GUI-side chain edits must hold the widget's renderMutex() and
 * run compileChain() afterwards (decision E4/E5).
 ****************************************************************************************
 */

#pragma once

#include "visualizers/VisualizerBase.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/modules/processing/BeatEstimator.hpp"
#include "visualizers/modules/processing/BeatModule.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"
#include "visualizers/modules/scripting/ScriptGridModule.hpp"
#include "visualizers/modules/scripting/ScriptLutModule.hpp"
#include "visualizers/render/OffscreenBufferPool.hpp"
#include "visualizers/render/ScopeRenderer.hpp"
#include "scripting/ScriptContext.hpp"
#include "scripting/ScriptSlotHost.hpp"

#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <unordered_map>
#include <vector>

/**
 * @class MultiEffectVisualizer
 * @brief IVisualizer hosting an editable multi-effect chain (AVS import target)
 */
class MultiEffectVisualizer : public VisualizerBase
{
public:
    MultiEffectVisualizer();
    ~MultiEffectVisualizer() override = default;

    // =========================================================================
    // Chain access — GUI writes only under the widget's renderMutex() (§12)
    // =========================================================================

    /** Replace the whole chain (runs the compile pass; returns its result). */
    lumi::multieffect::CompileResult setChain(lumi::multieffect::ChainNode root);

    /** Mutable chain root — caller must hold renderMutex() and re-compile. */
    [[nodiscard]] lumi::multieffect::ChainNode& chain() { return m_root; }
    [[nodiscard]] const lumi::multieffect::ChainNode& chain() const { return m_root; }

    /** Re-run the chain compile pass after in-place edits (decision E4). */
    lumi::multieffect::CompileResult recompileChain();

    /**
     * @brief Parse an .avs file, translate it into the chain, and install it.
     * @param path Absolute path to a Nullsoft AVS preset.
     * @param outReport Optional: receives parser + translation warnings.
     * @return true if the file parsed as an AVS preset (unsupported effects are
     *         conserved as passthrough, not a failure).
     *
     * GUI-thread call — the caller must hold the widget's renderMutex() (the
     * render thread walks the chain). The full File-menu/editor hookup is 5.7.
     */
    bool loadAvsFile(const QString& path, QStringList* outReport = nullptr);

    /** Save the current chain as a host preset (.lvfx JSON). */
    bool saveChainFile(const QString& path) const;

    /** Load a host preset (.lvfx). GUI-thread call — hold renderMutex() (5.7 UI). */
    bool loadChainFile(const QString& path, QStringList* outReport = nullptr);

protected:
    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    // =========================================================================
    // Surfaces
    // =========================================================================

    /** One ping-pong FBO pair (root working surface or a list's thisfb). */
    struct SurfacePair
    {
        std::unique_ptr<QOpenGLFramebufferObject> fbo[2];
        int currentIndex = 0;

        [[nodiscard]] QOpenGLFramebufferObject* current() const
        {
            return fbo[currentIndex].get();
        }
        [[nodiscard]] QOpenGLFramebufferObject* partner() const
        {
            return fbo[1 - currentIndex].get();
        }
        [[nodiscard]] bool ready() const { return fbo[0] && fbo[1]; }
        void swap() { currentIndex = 1 - currentIndex; }
        void destroy()
        {
            fbo[0].reset();
            fbo[1].reset();
            currentIndex = 0;
        }
    };

    /** Per-leaf render-thread state (beat counters, scripts), keyed by nodeId. */
    struct LeafRuntime
    {
        int beatCounter = 0;     ///< OnBeat Clear: beats since last clear
        int beatFramesLeft = 0;  ///< Colorfade: frames the beat faders stay on
        bool mirrorH = true;     ///< Mirror onbeat-random: active horizontal axis
        bool mirrorV = false;    ///< Mirror onbeat-random: active vertical axis

        // Color Modifier: scripted 256-entry LUT + its GL upload
        std::unique_ptr<lumi::modules::ScriptLutModule> lut;
        unsigned int lutTexture = 0;  ///< GL 256x1 RGB texture (deleted in onCleanup)
        std::string lutCompiled;      ///< source snapshot the module was built from

        // Movement / Dynamic Movement: scripted displacement grid
        std::unique_ptr<lumi::modules::ScriptGridModule> grid;
        std::string gridCompiled;

        // SuperScope: scripted point generator (drawn via the shared renderer)
        std::unique_ptr<lumi::modules::SuperscopeModule> scope;
        std::string scopeCompiled;

        float rotoAngle = 0.0f;  ///< Roto Blitter: accumulated rotation (rad)

        // Custom BPM
        std::int64_t customLastMs = 0;  ///< arbitrary-mode last emit time
        int customSkipCount = 0;        ///< skip-mode beat counter
    };

    /** Render-thread state of one list node, keyed by ChainNode::nodeId. */
    struct ListRuntime
    {
        SurfacePair surface;       ///< persistent thisfb (trails inside the list)
        int beatFramesLeft = 0;    ///< OnBeat activation window
        bool needsClear = true;    ///< first use → clear
        bool seenThisFrame = false;
        // EEL list slots (compiled lazily on the render thread).
        // NB: member cannot be named "slots" — Qt reserves that keyword.
        std::unique_ptr<lumi::scripting::ScriptSlotHost> slotHost;
        std::string compiledInit;   ///< sources the current compile is based on
        std::string compiledFrame;
    };

    bool ensureSurfacePair(SurfacePair& pair, int width, int height,
                           bool* outResized = nullptr);
    void destroySurfaces();
    bool ensurePipelines();

    // Active render target = top of the surface stack (root at the bottom).
    [[nodiscard]] SurfacePair& active() { return *m_surfaceStack.back(); }
    void bindActive();

    // =========================================================================
    // Chain walk (render thread)
    // =========================================================================

    void renderNode(const lumi::multieffect::ChainNode& node);
    void renderList(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::ListParams& params);
    void runClear(const lumi::multieffect::ClearParams& params);
    void runFadeout(const lumi::multieffect::FadeoutParams& params);
    void runInvert();
    void runBrightness(const lumi::multieffect::BrightnessParams& params);
    void runFastBrightness(const lumi::multieffect::FastBrightnessParams& params);
    void runBlur(const lumi::multieffect::BlurParams& params);
    void runMirror(const lumi::multieffect::ChainNode& node,
                   const lumi::multieffect::MirrorParams& params);
    void runOnBeatClear(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::OnBeatClearParams& params);
    void runColorfade(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::ColorfadeParams& params);
    void runColorModifier(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::ColorModifierParams& params);
    void runMovement(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::MovementParams& params);
    void runDynamicMovement(const lumi::multieffect::ChainNode& node,
                            const lumi::multieffect::DynamicMovementParams& params);
    /** Shared grid-warp: run the module, build the mesh, sample current→partner. */
    void applyGridWarp(LeafRuntime& rt, int xres, int yres, bool wrap);
    void runBlitterFeedback(const lumi::multieffect::BlitterFeedbackParams& params);
    void runRotoBlitter(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::RotoBlitterParams& params);
    /** Shared roto/zoom feedback pass: sample current transformed, blend, swap. */
    void feedbackPass(float zoom, float angleRad, bool blend);
    void runBufferSave(const lumi::multieffect::BufferSaveParams& params);
    void runCustomBpm(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::CustomBpmParams& params);
    void runSuperScope(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::SuperScopeParams& params);
    void runDebugBars(const lumi::multieffect::DebugBarsParams& params);

    [[nodiscard]] uint32_t nextRandom();  ///< host LCG (Mirror onbeat-random)

    /** Full-screen transform pass on the active pair: sample current, write partner, swap. */
    void transformPass(QOpenGLShaderProgram& shader);

    /**
     * Blend src over dst.current() into dst.partner(), then swap dst.
     * Unimplemented modes fall back to Replace (decision E3); Ignore is a no-op.
     */
    void blendPass(SurfacePair& dst, unsigned int srcTexture,
                   lumi::multieffect::BlendMode mode, int adjustAlpha);

    lumi::multieffect::ChainNode m_root;

    // GL state (render thread only)
    SurfacePair m_rootSurface;
    std::vector<SurfacePair*> m_surfaceStack;
    std::unordered_map<uint64_t, ListRuntime> m_listRuntimes;
    int m_surfaceWidth = 0;
    int m_surfaceHeight = 0;
    bool m_firstFrame = true;
    bool m_pendingRuntimeReset = false;  ///< a new chain was installed (loadAvsFile)

    /** Free all per-node GL runtimes (render thread, context current). */
    void resetRuntimes();

    std::unique_ptr<QOpenGLShaderProgram> m_fadeShader;
    std::unique_ptr<QOpenGLShaderProgram> m_invertShader;
    std::unique_ptr<QOpenGLShaderProgram> m_barsShader;
    std::unique_ptr<QOpenGLShaderProgram> m_blendShader;
    std::unique_ptr<QOpenGLShaderProgram> m_brightShader;
    std::unique_ptr<QOpenGLShaderProgram> m_blurShader;
    std::unique_ptr<QOpenGLShaderProgram> m_mirrorShader;
    std::unique_ptr<QOpenGLShaderProgram> m_colorfadeShader;
    std::unique_ptr<QOpenGLShaderProgram> m_lutShader;
    std::unique_ptr<QOpenGLShaderProgram> m_warpShader;
    std::unique_ptr<QOpenGLShaderProgram> m_feedbackShader;
    std::unique_ptr<QOpenGLVertexArrayObject> m_quadVao;
    std::unique_ptr<QOpenGLBuffer> m_quadVbo;
    std::unique_ptr<QOpenGLVertexArrayObject> m_warpVao;
    std::unique_ptr<QOpenGLBuffer> m_warpVbo;
    std::vector<float> m_warpVertices;  ///< reused CPU scratch for the warp mesh

    std::unordered_map<uint64_t, LeafRuntime> m_leafRuntimes;
    lumi::render::OffscreenBufferPool m_bufferPool;  ///< 8 global buffers (Buffer Save)
    lumi::render::ScopeRenderer m_scopeRenderer;     ///< shared scope draw (E6)
    uint32_t m_rng = 0x9E3779B9u;  ///< host-local LCG state

    // Chain-scoped beat (design doc block 4; mutable within the frame)
    lumi::modules::BeatModule m_beat;
    lumi::modules::BeatEstimator m_beatEstimator{0};
    bool m_frameBeat = false;  ///< beat flag effects/list scripts may mutate

    // Preset-local shared script state (decision §10.3)
    std::shared_ptr<lumi::scripting::ScriptContext> m_scriptContext;

    // Frame-scoped inputs
    float m_time = 0.0f;       ///< seconds since init (DebugBars orbit)
    float m_deltaTime = 0.0f;  ///< seconds since last frame (script modules)
    float m_audioLevel = 0.0f; ///< smoothed waveform RMS 0..1
};
