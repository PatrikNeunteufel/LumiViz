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
 * through the warp mesh).
 * M5: blur pyramid (3 levels x H+V passes over 6 downsampled textures, ref
 * BlurPasses) + shader stage B — Md1Default/Md1Plus classified shaders render
 * exactly via baked constants (decay/echo/gamma/filters + linear blur mixes);
 * Custom shaders keep the live MD1 fallback (import report says so).
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

#include <QImage>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QString>
#include <QStringList>

#include <array>
#include <map>
#include <set>
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

    /**
     * @brief Load a translated preset from a .lvfx sister document (M6; no GL,
     *        call under renderMutex()) — MilkdropSerializer format
     */
    bool loadPresetDocument(const QString& path, QStringList* report = nullptr);

    /// @brief Save the current preset as a .lvfx sister document (M6)
    [[nodiscard]] bool savePresetDocument(const QString& path) const;

    /**
     * @brief Adopt an already-translated preset (N1 Meganode: the chain node's
     *        params are the SSOT). No GL — scripts compile and shaders
     *        transpile here; GL programs rebuild lazily on the next frame.
     * @param presetDir Texture search base (C2 lookup paths)
     */
    void applyPresetState(lumi::milkdrop::PresetState state, const QString& presetDir,
                          QStringList* report = nullptr);

    /**
     * @brief Eingebettete Bilder aus dem .lvfx (S43): Key = Textur-Basisname
     *        bzw. Sprite-imageName, Value = Base64 der Original-Dateibytes.
     *        VOR applyPresetState setzen — Loader nutzt sie als Fallback,
     *        wenn die Datei in den Asset-Ordnern nicht gefunden wird.
     */
    void setEmbeddedImages(std::map<std::string, std::string> images)
    {
        m_embeddedImages = std::move(images);
    }

    /// HG3: progress-Quelle vom Host (Host-Gruppen-Laufzeit; die Playlist
    /// liefert spaeter die echte Slot-Dauer). < 0 = eigener 60-s-Zyklus.
    void setProgressOverride(double p) { m_progressOverride = p; }

    /**
     * @brief Puffer-Wechsel (S66): Erbe-Anteil des Feedback-Bildes fuer den
     *        NAECHSTEN Frame anmelden (kein GL — der Render-Thread wendet ihn
     *        vor dem Frame an). keep 1 = Behalten (No-op), 0 = wie Kaltstart
     *        (frische Rausch-Saat), dazwischen = Mix Erbe/Saat.
     */
    void requestFeedbackErbe(double keep);

    /**
     * @brief Puffer-Wechsel "Ausblenden" (S66): das geerbte Bild stirbt ueber
     *        `sekunden` weg — das Echo wird je Frame zusaetzlich gedaempft
     *        (nach dem Warp, VOR den frischen Zeichnungen des neuen Presets).
     *        Nach Ablauf ist nur noch der Eigenanteil des Presets im Puffer.
     */
    void requestFeedbackAusblenden(double sekunden);

    /**
     * @brief Sicht-Blende (S67): an ⇒ nach jeder frischen Rausch-Saat
     *        (Kaltstart/Resize-Saat, Loeschen-/Fading-Wipe) blendet das
     *        Composite ~0,5 s von Schwarz ein — rein kosmetisch am Ziel,
     *        die Puffer-Dynamik (Verstaerker-Zuendung) bleibt unberuehrt.
     *        Default AUS (Pruefstaende/Triage unveraendert); die App schaltet
     *        sie per QSettings `milkdrop/sichtBlende` zu.
     */
    void setSichtBlende(bool an);

    /// @brief Currently loaded preset state (for tests/panel)
    [[nodiscard]] const lumi::milkdrop::PresetState& presetState() const { return m_state; }

    /// @brief Test/Debug (C1): assembled GLSL fragment for a preset shader text
    [[nodiscard]] static std::string debugAssembleFragment(const std::string& hlslText,
                                                           bool isWarp);
    /// @brief Test/Debug (C1): the custom passes' vertex shader source
    [[nodiscard]] static const char* debugCustomVertexShader();

    /// @brief Transpiled GLSL of the LOADED preset ("" = MD1 path) — regression
    ///        gate for prepareCustomShaders (Session-41 Befund) + standalone
    [[nodiscard]] const std::string& warpCustomSource() const { return m_warpCustomSrc; }
    [[nodiscard]] const std::string& compCustomSource() const { return m_compCustomSrc; }
    /// @brief First GL compile/link error of the custom programs ("" = none)
    [[nodiscard]] const std::string& customGlError() const { return m_customGlError; }

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
        std::array<double, 3> blurMin{0.0, 0.0, 0.0};   ///< blurN_min (per-frame vars)
        std::array<double, 3> blurMax{1.0, 1.0, 1.0};
        double blurEdgeDarken = 0.25;
        std::array<double, 32> qVals{};                 ///< q1-q32 (custom shaders, C1)
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

    /// One preset sprite at runtime (MilkDrop2077 [SPRITEn]): PRIVATE script
    /// context like the original texmgr slots (no reg/q sharing) + life state
    struct SpriteRuntime
    {
        lumi::milkdrop::SpriteState def;
        std::shared_ptr<lumi::scripting::ScriptContext> context;
        std::unique_ptr<lumi::scripting::ScriptSlotHost> script;
        bool varsSeeded = false;  ///< Startwerte (Sprite*-Keys) einmal gesetzt
        bool dead = false;        ///< done!=0 → beendet (bis zum Preset-Reload)
    };

    // --- script plumbing ---------------------------------------------------------------
    /// Adopt a translated state: scripts + report + runtime reset (shared tail
    /// of loadMilkFile / loadPresetDocument)
    void applyState(lumi::milkdrop::PresetState state, QStringList* report);
    void rebuildScripts(QStringList* report);
    void runPerFrameInit();
    void pushFrameInputs();                 ///< preset values + audio/time into the engine
    void pullFrameOutputs(FrameVars& fv);   ///< read i/o vars back after per_frame
    void updateAudio(float deltaTime);
    void pushCommonInputs(lumi::scripting::LuaScriptEngine& engine);  ///< audio/time set

    // --- render passes (render thread, context current) ---------------------------------
    bool ensureGlResources();
    void releaseGlResources();
    /// @brief Blur levels the baked composite actually samples (0 = skip passes)
    [[nodiscard]] int activeBlurLevels() const;
    bool ensureBlurTargets(int sourceW, int sourceH);
    void releaseBlurTargets();
    /// Kaltstart-Saat (S63): beide Feedback-Texturen mit deterministischem
    /// Vollbereichs-Rauschen fuellen — Verstaerker-Presets ohne eigene
    /// Energiequelle brauchen ererbten Pufferinhalt (Original: undef. VRAM)
    void seedFeedbackNoise(int w, int h);
    /// Kaltstart-Basis (S66): das deterministische Saat-Rauschen als RGBA8-
    /// Puffer (bzw. Nullen unter LUMIVIZ_MILKDROP_NOSEED) — gemeinsame Quelle
    /// von seedFeedbackNoise und applyFeedbackErbe
    [[nodiscard]] static std::vector<unsigned char> kaltstartBasis(int w, int h);
    /// Puffer-Wechsel (S66): beide Feedback-Texturen = Erbe*keep + Saat*(1-keep)
    /// (keep<=0: reine Saat ohne Readback). Render-Thread, Kontext current.
    void applyFeedbackErbe(int w, int h, double keep);
    /// @brief H+V blur chain over the previous frame (call BEFORE beginFrame)
    void runBlurPasses(const FrameVars& fv);
    void computeWarpMesh(const FrameVars& fv);
    void drawWarpPass(const FrameVars& fv);
    void drawMotionVectors(const FrameVars& fv);
    void drawCustomShapes();
    void drawCustomWaves();
    void drawBasicWave(const FrameVars& fv);
    void drawBorders(const FrameVars& fv);
    void drawDarkenCenter();
    void compositeToScreen(const FrameVars& fv);
    /// Calibration grid overlay on the SCREEN (after composite — never enters
    /// the feedback loop); toggled via the `render.debugGrid` parameter
    void drawDebugGrid();
    void drawColorQuads(const float* vertexData, int vertexCount, unsigned int glMode);

    /// bilinear lookup in the warp-mesh UVs: where did (fx,fy) come from?
    [[nodiscard]] bool reversePropagate(double fx, double fy, double& outX,
                                        double& outY) const;

    // --- Stufe C1: transpiled custom shaders (GLSL) ---------------------------------------
    /// Transpile Custom warp/comp HLSL at load time (fills m_warp/compCustomSrc)
    void prepareCustomShaders(QStringList* report);
    bool ensureCustomPrograms();      ///< (re)build GL programs when sources changed
    void releaseCustomGl();
    bool ensureNoiseTextures();
    /// C2: resolve + load the preset's custom textures (GUI thread, QImage)
    void loadCustomTextures(const std::vector<std::string>& samplerNames,
                            QStringList* report);
    void ensureCustomTextureUploads();  ///< render thread: QImage -> GL textures
    /// Bind every sampler the program actually uses + feed the per-frame uniforms
    void feedCustomUniforms(QOpenGLShaderProgram& program, const FrameVars& fv,
                            unsigned int mainTexture);

    // --- Preset-Sprites (MilkDrop2077; Port von DrawUserSprites) --------------------------
    void rebuildSprites(QStringList* report);   ///< applyState-Schwanz (GUI, kein GL)
    void loadSpriteImages(QStringList* report); ///< GUI thread: QImage + Colorkey→Alpha
    void ensureSpriteUploads();                 ///< render thread (rev-gekoppelt)
    /// Nach dem Composite ueber das Bild zeichnen; burn!=0 zusaetzlich in den
    /// Feedback-Buffer (naechster Frame warpt es), done!=0 beendet das Sprite
    void drawUserSprites();

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
    // Loudness-Diagnose (S43): min/max seit der letzten Trace-Zeile
    // (erste Zeile nach ~1 s, danach ~alle 5 s)
    int m_traceFrames = 0;
    bool m_traceFirst = true;
    double m_traceBassMin = 1.0;
    double m_traceBassMax = 1.0;
    std::array<float, kWaveBuffer> m_waveL{};     ///< smoothed waveform (basic wave, spec §0)
    std::array<float, kWaveBuffer> m_waveR{};
    std::array<float, kWaveBuffer> m_waveRawL{};  ///< unsmoothed (custom waves filter selbst)
    std::array<float, kWaveBuffer> m_waveRawR{};
    std::vector<float> m_spectrumL;               ///< frame copies (custom spectrum waves)
    std::vector<float> m_spectrumR;

    // --- GL ------------------------------------------------------------------------------
    QOpenGLContext* m_lastContext = nullptr;
    /// Composite-Ziel: das beim onRender-EINTRITT gebundene Draw-FBO (Standalone:
    /// Qt-Default-FBO; Meganode: aktiver Chain-Buffer des MultiEffect-Hosts)
    unsigned int m_targetFbo = 0;
    lumi::render::FeedbackBuffer m_feedback;
    /// Puffer-Wechsel (S66): angemeldeter Erbe-Anteil fuer den naechsten Frame
    /// (-1 = nichts pending; GUI-Thread schreibt unter renderMutex, Render-
    /// Thread konsumiert vor dem Frame)
    double m_pendingFeedbackErbe = -1.0;
    /// Lade-Diagnose (S66): >0 = noch so viele Frames Audio-Futter tracen
    int m_traceLoadFrames = 0;
    /// Zeit-Ausblendung (S66): Restzeit/Dauer in Sekunden (<=0 = inaktiv).
    /// Waehrend der Restzeit daempft ein Dim-Pass das Echo nach dem Warp so,
    /// dass das Erbe nach der Dauer unter die 8-bit-Sichtbarkeit faellt.
    double m_erbeAusblendRest = 0.0;
    double m_erbeAusblendDauer = 0.0;
    /// Sicht-Blende (S67): Composite blendet nach frischer Saat von Schwarz
    /// ein (kosmetisch, s. setSichtBlende). Rest in Sekunden, <=0 = inaktiv.
    static constexpr double kSichtBlendeSek = 0.5;
    bool m_sichtBlendeAn = false;
    double m_sichtBlendeRest = 0.0;
    lumi::render::ScopeRenderer m_scope;
    std::unique_ptr<QOpenGLShaderProgram> m_warpProgram;    ///< pos+uv, prev texture × decay
    std::unique_ptr<QOpenGLShaderProgram> m_textureProgram; ///< pos+uv quad × uniform color
    std::unique_ptr<QOpenGLShaderProgram> m_colorProgram;   ///< pos+rgba (borders/filters)
    std::unique_ptr<QOpenGLShaderProgram> m_shapeProgram;   ///< pos+uv+rgba (textured shapes)
    std::unique_ptr<QOpenGLShaderProgram> m_blurHProgram;   ///< long horizontal blur pass
    std::unique_ptr<QOpenGLShaderProgram> m_blurVProgram;   ///< short vertical + edge darken
    std::unique_ptr<QOpenGLShaderProgram> m_blurLayerProgram; ///< composite blur term (scale+bias)
    std::array<unsigned int, 6> m_blurTex{};                ///< 6 chain textures (raw GL ids)
    std::array<unsigned int, 6> m_blurFbo{};
    std::array<std::array<int, 2>, 6> m_blurSizes{};
    int m_blurSrcW = 0;                                     ///< chain layout source size
    int m_blurSrcH = 0;

    // --- Stufe C1: custom shader runtime --------------------------------------------------
    std::string m_warpCustomSrc;    ///< assembled GLSL fragment source ("" = MD1 path)
    std::string m_compCustomSrc;
    int m_customRev = 0;            ///< bumped on load; render thread rebuilds lazily
    int m_customBuiltRev = -1;
    std::unique_ptr<QOpenGLShaderProgram> m_warpCustomProgram;
    std::unique_ptr<QOpenGLShaderProgram> m_compCustomProgram;
    std::string m_customGlError;    ///< first GL compile/link error (panel/debug)
    std::array<unsigned int, 4> m_noiseTex{};   ///< lq, lq_lite, mq, hq (AddNoiseTex-Port)
    std::array<unsigned int, 2> m_noiseVolTex{};///< noisevol lq, hq (AddNoiseVol-Port, C3)
    unsigned int m_placeholderTex = 0;          ///< 1x1 grey (fehlende Texturen)
    std::array<unsigned int, 4> m_samplerObj{}; ///< wrapLin, clampLin, wrapPoint, clampPoint
    /// Startwert des Engine-PRNG — auch der Kaltstart-Reset bei
    /// Puffer-Wechsel "Loeschen" (S66) springt hierauf zurueck
    static constexpr unsigned int kRandSeedInit = 0x9e3779b9u;
    unsigned int m_randSeed = kRandSeedInit;    ///< rand_frame/rand_preset PRNG
    std::array<float, 4> m_hueRandStart{};      ///< fShader-Wash Phasen (m_fRandStart-Port)
    double m_progressOverride = -1.0;           ///< HG3: progress vom Host (<0 = 60-s-Zyklus)

    // --- Preset-Sprites (MilkDrop2077) -----------------------------------------------------
    std::vector<SpriteRuntime> m_spriteRt;
    std::map<std::string, QImage> m_spriteImages;       ///< imageName -> RGBA (Colorkey angewandt)
    std::map<std::string, unsigned int> m_spriteTexIds; ///< Uploads (render thread)
    int m_spriteUploadRev = -1;                         ///< Upload-Stand (= m_customRev)
    std::unique_ptr<QOpenGLShaderProgram> m_spriteProgram;  ///< tex × Farbe, uUseTexAlpha

    // --- Stufe C2: custom textures (asset-Pack / neben dem Preset) -------------------------
    QString m_presetDir;                                    ///< Suchbasis fuer Texturen
    std::map<std::string, std::string> m_embeddedImages;    ///< .lvfx-Einbettung (S43)
    /**
     * @brief Zufallsparameter einer Rotationsmatrix (`rot_s/d/f/vf/uf` 1..4)
     *
     * Je Preset einmal gewuerfelt (`state.cpp:RandomizePresetVars`): Basiswinkel,
     * Drehgeschwindigkeit je Achse und eine Verschiebung. Die vier `rot_rand*`
     * stehen NICHT hier — die wuerfeln jeden Frame neu.
     */
    struct RotParams
    {
        float base[3]{};   ///< Startwinkel x/y/z
        float speed[3]{};  ///< Winkel je Sekunde x/y/z
        float xlate[3]{};  ///< Verschiebung x/y/z
    };
    std::array<RotParams, 20> m_rotParams{};

    /// Vom Preset deklarierte Sampler-Uniforms (auch die ohne gefundene Datei) —
    /// nur diese werden gebunden, damit der Platzhalter keine fremde Uniform trifft
    std::set<std::string> m_customSamplerNames;
    std::map<std::string, QImage> m_customImages;           ///< sampler-Uniform-Name -> Bild
    std::map<std::string, std::array<int, 2>> m_texSizes;   ///< Basisname -> (w,h)
    std::map<std::string, unsigned int> m_customTexIds;     ///< Uploads (render thread)
    int m_texUploadRev = -1;                                ///< Upload-Stand (= m_customRev)
    std::unique_ptr<QOpenGLVertexArrayObject> m_shapeVao;
    std::unique_ptr<QOpenGLBuffer> m_shapeVbo;
    std::unique_ptr<QOpenGLVertexArrayObject> m_meshVao;
    std::unique_ptr<QOpenGLBuffer> m_meshVbo;
    std::unique_ptr<QOpenGLVertexArrayObject> m_quadVao;
    std::unique_ptr<QOpenGLBuffer> m_quadVbo;
    std::vector<float> m_meshData;          ///< triangulated pos.xy + uv per vertex
    std::vector<double> m_vertexUv;         ///< (gx+1)*(gy+1)*2 warp UVs (math space)

    // --- Diagnose-Trace (Session 41): nur Zustandswechsel loggen, kein Frame-Spam ---------
    struct TraceState
    {
        int rev = -2;                              ///< -2 = noch nie geloggt
        bool warpSrc = false, compSrc = false;     ///< Custom-GLSL vorhanden?
        bool warpProg = false, compProg = false;   ///< GL-Programme gebaut?
        bool glError = false;                      ///< m_customGlError gesetzt?
        bool glResFail = false;                    ///< ensureGlResources schlug fehl
        bool operator==(const TraceState&) const = default;
    };
    TraceState m_traceState;         ///< zuletzt geloggter Render-Zustand
    int m_traceWarpDrawRev = -1;     ///< Custom-Branch in drawWarpPass gemeldet (je Rev)
    int m_traceCompDrawRev = -1;     ///< Custom-Branch in compositeToScreen gemeldet
    bool m_traceGlInfoLogged = false;

    // --- parameters ----------------------------------------------------------------------
    int m_meshX = kDefaultMeshX;
    int m_meshY = kDefaultMeshY;
    bool m_debugGrid = false;   ///< calibration grid overlay (Config panel toggle)

    // aspect factors of the internal surface (plugin.cpp:2035)
    double m_aspectX = 1.0, m_aspectY = 1.0;
};
