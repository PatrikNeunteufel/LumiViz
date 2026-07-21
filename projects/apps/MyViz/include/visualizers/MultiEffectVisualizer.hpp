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

    /** One Starfield particle (normalized coords, z in (0,1]). */
    struct Star
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 1.0f;
        float speed = 0.5f;
    };

    /** One Dot-Fountain particle (polar around the fountain axis). */
    struct FountainP
    {
        float a = 0.0f;   ///< azimuth angle
        float r = 0.0f;   ///< radius from the axis
        float h = 0.0f;   ///< height
        float vh = 0.0f;  ///< vertical velocity
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

        // Color Map: 256x1 gradient LUT texture built from the stops (r_colormap)
        unsigned int cmTexture = 0;   ///< GL 256x1 RGB texture (deleted in onCleanup)
        std::string cmSnapshot;       ///< stop snapshot the texture was built from

        // Picture / Texer: decoded image texture (r_picture / Texer / Texer II)
        unsigned int picTexture = 0;  ///< GL RGBA texture (deleted in onCleanup)
        int picW = 0;
        int picH = 0;

        // Texer II / Triangle: EEL point-loop scripts
        std::unique_ptr<lumi::scripting::ScriptSlotHost> texerHost;
        std::string texerCompiled;
        std::unique_ptr<lumi::scripting::ScriptSlotHost> triHost;
        std::string triCompiled;

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

        // Mosaic: on-beat quality ease-back (r_mosaic thisQuality/nF)
        float mosaicQuality = 0.0f;  ///< current interpolated block count (0 = init)
        int mosaicFramesLeft = 0;    ///< frames left easing back to `quality`

        // SuperScope: gradient preset the module currently holds (reload on change)
        std::string scopeGradientLoaded;

        // Interferences: accumulating rotation + on-beat morph state (r_interf)
        bool interfSeeded = false;
        float interfRotation = 0.0f;  ///< persistent rotation accumulator (0..255 units)
        float interfStatus = 0.0f;    ///< beat-morph phase (0..pi)

        // Water: previous frame buffer (r_water lastframe); render-thread owned
        std::unique_ptr<QOpenGLFramebufferObject> waterLast;
        int waterW = 0;
        int waterH = 0;

        // Bump: EEL light-position script + eased depth (r_bump)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> bumpHost;
        std::string bumpCompiled;
        float bumpX = 0.5f;      ///< light position (0..1)
        float bumpY = 0.5f;
        float bumpDepth = 0.0f;  ///< eased strength (0 = uninitialised)
        int bumpFramesLeft = 0;  ///< frames left easing depth back

        // Dynamic Shift: EEL global-offset script (r_shift)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> shiftHost;
        std::string shiftCompiled;

        // Dynamic Distance Modifier: EEL radial-distance script (r_ddm)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> ddmHost;
        std::string ddmCompiled;

        // Jheriko: Global — EEL that sets preset-global reg/gmegabuf
        std::unique_ptr<lumi::scripting::ScriptSlotHost> jherikoHost;
        std::string jherikoCompiled;
        bool jherikoInited = false;

        // Interleave: eased stripe spacing (r_interleave cur_x/cur_y)
        bool interSeeded = false;
        float interCurX = 1.0f;
        float interCurY = 1.0f;

        // Scope-render effects (r_simple/oscstar/oscring/rotstar)
        int scopeColorPos = 0;      ///< colour-table cycle position
        float scopeRot = 0.0f;      ///< accumulating rotation (oscstar/oscring/rotstar)
        float bsRv[2] = {3.14159f, 0.0f};  ///< Bass Spin per-channel angle
        float bsV[2] = {0.0f, 0.0f};       ///< Bass Spin per-channel angular velocity
        float bsLastA[2] = {0.0f, 0.0f};   ///< Bass Spin per-channel bass-energy history

        // Moving Particle: spring-particle state (r_parts)
        bool mpSeeded = false;
        float mpCx = 0.0f, mpCy = 0.0f;   ///< spring target (randomized on beat)
        float mpVx = 0.0f, mpVy = 0.0f;   ///< velocity
        float mpPx = 0.0f, mpPy = 0.0f;   ///< position (-1..1)
        float mpSize = 8.0f;              ///< eased radius

        // Water Bump: RGBA16F height ping-pong (.r current, .g previous height)
        std::unique_ptr<QOpenGLFramebufferObject> wbHeight[2];
        int wbCur = 0;   ///< index of the current height buffer
        int wbW = 0;
        int wbH = 0;

        // Starfield: CPU star particles + warp-speed ease (r_stars)
        std::vector<Star> stars;
        float starSpeed = 0.0f;    ///< eased current warp speed (0 = uninitialised)
        int starBeatFrames = 0;    ///< frames left easing back to warpSpeed

        int timescopeX = 0;        ///< Timescope: current spectrogram column

        // Dot Grid / Plane / Fountain (r_dotgrid/dotpln/dotfnt)
        float dotColorPos = 0.0f;  ///< Dot Grid: colour-table cycle position
        float dotOffX = 0.0f;      ///< Dot Grid: scroll offset
        float dotOffY = 0.0f;
        float dotRot = 0.0f;       ///< Dot Plane/Fountain: accumulating rotation
        std::vector<FountainP> fountain;  ///< Dot Fountain particles

        int apeChanMode = -1;      ///< Channel Shift on-beat held permutation

        // Fractal 2D (Batch H): EEL view-driver + gradient palette LUT
        std::unique_ptr<lumi::scripting::ScriptSlotHost> fracHost;
        std::string fracCompiled;      ///< source snapshot the host was built from
        bool fracInited = false;       ///< init slot has run once
        unsigned int fracLut = 0;      ///< GL 256x1 RGB palette (deleted in onCleanup)
        std::string fracLutSnapshot;   ///< gradient preset the LUT was baked from
        float fracColorPhase = 0.0f;   ///< accumulated palette drift (colorCycle)
        float fracTime = 0.0f;         ///< accumulated animation phase (Domain Warp / Zoomer log-zoom)

        // Strange Attractor / Flame (Batch H): persistent orbit + view rotation
        double saX = 0.1, saY = 0.0, saZ = 0.0;  ///< current orbit position
        float saRot = 0.0f;            ///< accumulated view rotation
        bool saSeeded = false;

        // Reaction-Diffusion (Batch H): RGBA16F ping-pong (A in .r, B in .g)
        std::unique_ptr<QOpenGLFramebufferObject> rdBuf[2];
        int rdCur = 0;
        int rdW = 0;
        int rdH = 0;
        bool rdSeeded = false;

        // Video Delay: per-node frame ring buffer (r_videodelay)
        std::vector<std::unique_ptr<QOpenGLFramebufferObject>> delayRing;
        int delayHead = 0;
        int delayFilled = 0;
        int delayW = 0;
        int delayH = 0;
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
    void runDynamicShift(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::DynamicShiftParams& params);
    void runDynamicDistanceModifier(
        const lumi::multieffect::ChainNode& node,
        const lumi::multieffect::DynamicDistanceModifierParams& params);
    void runMovingParticle(const lumi::multieffect::ChainNode& node,
                           const lumi::multieffect::MovingParticleParams& params);
    void runColorMap(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::ColorMapParams& params);
    void runBufferBlend(const lumi::multieffect::BufferBlendParams& params);
    void runJherikoGlobal(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::JherikoGlobalParams& params);
    void runColorClip(const lumi::multieffect::ColorClipParams& params);
    void runUniqueTone(const lumi::multieffect::UniqueToneParams& params);
    void runInterleave(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::InterleaveParams& params);
    void runConvolution(const lumi::multieffect::ConvolutionParams& params);
    void runNormalise();
    void runMultiFilter(const lumi::multieffect::MultiFilterParams& params);
    void runAddBorders(const lumi::multieffect::AddBordersParams& params);
    void runSimpleScope(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::SimpleScopeParams& params);
    void runBassSpin(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::BassSpinParams& params);
    void runOscStar(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::OscStarParams& params);
    void runOscRing(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::OscRingParams& params);
    void runRotatingStars(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::RotatingStarsParams& params);
    void runPicture(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::PictureParams& params);
    void runPictureII(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::PictureIIParams& params);
    void runTexer(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::TexerParams& params);
    void runTexerII(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::TexerIIParams& params);
    void runTriangle(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::TriangleParams& params);
    /// Decode the base64 image into rt.picTexture once; true when ready.
    bool ensureEmbeddedTexture(LeafRuntime& rt, const std::string& imageData);
    /// Draw an embedded (base64) image over the frame, blended.
    void drawEmbeddedImage(LeafRuntime& rt, const std::string& imageData, int blend,
                           bool keepAspect);
    /// Draw a scope point-list (additive) via the shared ScopeRenderer.
    void drawScopeShape(const std::vector<lumi::modules::SuperscopePoint>& pts, bool dots);
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
    void runSetRenderMode(const lumi::multieffect::SetRenderModeParams& params);
    void runSuperScope(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::SuperScopeParams& params);
    void runDebugBars(const lumi::multieffect::DebugBarsParams& params);
    void runMosaic(const lumi::multieffect::ChainNode& node,
                   const lumi::multieffect::MosaicParams& params);
    void runGrain(const lumi::multieffect::GrainParams& params);
    void runScatter(const lumi::multieffect::ScatterParams& params);
    void runInterferences(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::InterferencesParams& params);
    void runWater(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::WaterParams& params);
    void runBump(const lumi::multieffect::ChainNode& node,
                 const lumi::multieffect::BumpParams& params);
    void runWaterBump(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::WaterBumpParams& params);
    void runStarfield(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::StarfieldParams& params);
    void runTimescope(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::TimescopeParams& params);
    void runDotGrid(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::DotGridParams& params);
    void runDotPlane(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::DotPlaneParams& params);
    void runDotFountain(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::DotFountainParams& params);
    void runChannelShift(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::ChannelShiftParams& params);
    void runColorReduction(const lumi::multieffect::ColorReductionParams& params);
    void runMultiplier(const lumi::multieffect::MultiplierParams& params);
    void runVideoDelay(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::VideoDelayParams& params);
    void runMultiDelay(const lumi::multieffect::MultiDelayParams& params);
    void runFractal2D(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::Fractal2DParams& params);
    void runDomainWarp(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::DomainWarpParams& params);
    void runFractal3D(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::Fractal3DParams& params);
    void runLyapunov(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::LyapunovParams& params);
    void runKleinian(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::KleinianParams& params);
    void runFractalZoomer(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::FractalZoomerParams& params);
    void runStrangeAttractor(const lumi::multieffect::ChainNode& node,
                             const lumi::multieffect::StrangeAttractorParams& params);
    void runFlame(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::FlameParams& params);
    void runReactionDiffusion(const lumi::multieffect::ChainNode& node,
                              const lumi::multieffect::ReactionDiffusionParams& params);
    /// Bake a gradient preset into a node's 256x1 palette LUT (Batch H shared).
    void ensureFractalLut(LeafRuntime& rt, const std::string& preset);

    /// Rebuild m_visdata (AVS layout) from this frame's waveform + spectrum.
    void buildVisData();
    /// Feed the shared audio contract (visdata + gettime + bass/mid/treb/vol/beat/
    /// time) into a script engine — every scripted module gets the same set (E1).
    void feedAudio(lumi::scripting::LuaScriptEngine& engine);

    [[nodiscard]] uint32_t nextRandom();  ///< host LCG (Mirror onbeat-random)

    /** Full-screen transform pass on the active pair: sample current, write partner, swap. */
    void transformPass(QOpenGLShaderProgram& shader);

    /**
     * Blend src over dst.current() into dst.partner(), then swap dst.
     * Ignore is a no-op. For BlendMode::Buffer, `bufferTexture` supplies the
     * global-buffer depth source (`bufferInvert` flips it); a missing buffer
     * (bufferTexture == 0) leaves dst untouched, matching AVS' `if (!buf) break`.
     */
    void blendPass(SurfacePair& dst, unsigned int srcTexture,
                   lumi::multieffect::BlendMode mode, int adjustAlpha,
                   unsigned int bufferTexture = 0, bool bufferInvert = false);

    /** Texture of pool slot `n` at the surface size, or 0 if nothing is saved. */
    unsigned int poolTexture(int n);

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
    std::unique_ptr<QOpenGLShaderProgram> m_mosaicShader;
    std::unique_ptr<QOpenGLShaderProgram> m_grainShader;
    std::unique_ptr<QOpenGLShaderProgram> m_scatterShader;
    std::unique_ptr<QOpenGLShaderProgram> m_interfShader;
    std::unique_ptr<QOpenGLShaderProgram> m_waterShader;
    std::unique_ptr<QOpenGLShaderProgram> m_bumpShader;
    std::unique_ptr<QOpenGLShaderProgram> m_shiftShader;  ///< Dynamic Shift (r_shift)
    std::unique_ptr<QOpenGLShaderProgram> m_ddmShader;    ///< Dynamic Distance Modifier (r_ddm)
    std::unique_ptr<QOpenGLShaderProgram> m_colorMapShader;  ///< Color Map APE
    std::unique_ptr<QOpenGLShaderProgram> m_bufferBlendShader;  ///< Buffer blend APE
    std::unique_ptr<QOpenGLShaderProgram> m_pictureShader;      ///< Picture (r_picture)
    std::unique_ptr<QOpenGLShaderProgram> m_spriteShader;       ///< Texer / Texer II sprites
    std::unique_ptr<QOpenGLShaderProgram> m_colorClipShader;    ///< Color Clip (r_contrast)
    std::unique_ptr<QOpenGLShaderProgram> m_uniqueToneShader;   ///< Unique Tone (r_onetone)
    std::unique_ptr<QOpenGLShaderProgram> m_interleaveShader;   ///< Interleave (r_interleave)
    std::unique_ptr<QOpenGLShaderProgram> m_convolutionShader;  ///< Convolution APE
    std::unique_ptr<QOpenGLShaderProgram> m_normaliseShader;    ///< Normalise APE
    std::unique_ptr<QOpenGLShaderProgram> m_multiFilterShader;  ///< MultiFilter APE
    std::unique_ptr<QOpenGLShaderProgram> m_addBordersShader;   ///< Add Borders APE
    std::unique_ptr<QOpenGLFramebufferObject> m_reduceFbo;      ///< Normalise min/max readback (32x32)
    std::unique_ptr<QOpenGLShaderProgram> m_wbPropShader;  ///< water-bump wave propagation
    std::unique_ptr<QOpenGLShaderProgram> m_wbDispShader;  ///< water-bump refraction
    std::unique_ptr<QOpenGLShaderProgram> m_timescopeShader;
    std::unique_ptr<QOpenGLShaderProgram> m_apeShader;  ///< Channel Shift / Color Reduction / Multiplier
    std::unique_ptr<QOpenGLShaderProgram> m_fractal2DShader;  ///< Fractal 2D escape-time (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_domainWarpShader;  ///< Domain-warp fBm (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_fractal3DShader;   ///< Fractal 3D raymarch (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_lyapunovShader;    ///< Lyapunov (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_kleinianShader;    ///< Kleinian tiling (Batch H)
    std::unique_ptr<QOpenGLShaderProgram> m_rdShader;          ///< Reaction-Diffusion sim/show (Batch H)
    unsigned int m_specTex = 0;  ///< 1D spectrum upload for Timescope (deleted in onCleanup)

    // Multi Delay: 6 host-shared frame ring buffers (r_multidelay), cleared with
    // the runtimes. Input nodes fill them; output nodes read the delayed frame.
    std::vector<std::unique_ptr<QOpenGLFramebufferObject>> m_mdRing[6];
    int m_mdHead[6] = {0, 0, 0, 0, 0, 0};
    int m_mdW = 0;
    int m_mdH = 0;
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

    // Live render mode set by a Set Render Mode node for the following render
    // effects (AVS semantics). Reset at frame start; `set` means "override".
    struct RenderMode
    {
        bool set = false;    ///< a Set Render Mode node applied this frame
        int lineWidth = 1;   ///< line width for following scopes (px)
        int lineBlend = 1;   ///< 0 replace, 1 additive, 2 50/50
        int alpha = 128;     ///< Adjustable-blend alpha 0..255
    };
    RenderMode m_renderMode;

    // AVS-layout visualisation data (spectrum L/R + waveform L/R, 576 each),
    // rebuilt once per frame and fed to every scripted engine (getspec/getosc).
    std::array<unsigned char, 576 * 4> m_visdata{};

    // Preset-local shared script state (decision §10.3)
    std::shared_ptr<lumi::scripting::ScriptContext> m_scriptContext;

    // Frame-scoped inputs
    float m_time = 0.0f;       ///< seconds since init (DebugBars orbit)
    float m_deltaTime = 0.0f;  ///< seconds since last frame (script modules)
    float m_audioLevel = 0.0f; ///< smoothed waveform RMS 0..1
};
