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
#include "visualizers/MilkdropVisualizer.hpp"
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
#include "services/VideoFrameCache.hpp"

#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <limits>
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
     * Letzter GL-Kompilierfehler eines Shadertoy-Nodes ("" = fehlerfrei).
     * Der Panel-Editor zeigt ihn beim (Neu-)Aufbau an — dank `#line 1` im
     * Wrapper tragen die Treiber-Logs die ZEILEN DES NUTZER-CODES (Strang S).
     */
    [[nodiscard]] std::string shadertoyError(uint64_t nodeId) const
    {
        const auto it = m_leafRuntimes.find(nodeId);
        return it != m_leafRuntimes.end() ? it->second.stError : std::string();
    }

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

    /**
     * @brief Parse a .milk preset and install it as a single-Milkdrop-node
     *        chain (N2 routing — Entscheid E1/E2). GUI thread, under
     *        renderMutex(); no GL (the render thread applies revisions lazily).
     * @return false only when the file does not parse as a MilkDrop preset.
     */
    bool loadMilkFile(const QString& path, QStringList* outReport = nullptr);
    /// S63: .milk→.milk-Wechsel ohne Runtime-Reset — bestehende Milkdrop-Wurzel
    /// bekommt nur neue Params (Revision++), die Feedback-Historie bleibt
    /// erhalten (Original-Semantik: ein Preset erbt das Bild des Vorgaengers)
    bool replaceMilkdropPresetInPlace(lumi::milkdrop::PresetState& state,
                                      const QString& presetDir,
                                      std::map<std::string, std::string> embeddedImages);

    /**
     * @brief Load a translated milkdrop .lvfx sister document (M6.1 format)
     *        and install it as a single-Milkdrop-node chain. GUI thread,
     *        under renderMutex().
     */
    bool loadMilkDocument(const QString& path, QStringList* outReport = nullptr);

    /** Save the current chain as a host preset (.lvfx JSON). */
    bool saveChainFile(const QString& path) const;

    /** Load a host preset (.lvfx). GUI-thread call — hold renderMutex() (5.7 UI). */
    bool loadChainFile(const QString& path, QStringList* outReport = nullptr);

    /**
     * @brief Divisor des Render-Scale-Knotens, den loadAvsFile automatisch als
     *        erstes Kind einfuegt (Entscheid S47, Variante 2): Der Wert kommt
     *        aus der App-Einstellung und wirkt NUR im Moment des Imports —
     *        danach ist der Knoten im Preset die einzige Wahrheit. 1 = neutral.
     */
    void setImportRenderScaleDivisor(int divisor)
    {
        m_importRenderScaleDivisor = divisor < 1 ? 1 : (divisor > 8 ? 8 : divisor);
    }

    /**
     * @brief App-Default fuer den Puffer-Wechsel beim .milk→.milk-Tausch
     *        (S66, Settings-Panel): greift, wenn der Milkdrop-Node auf
     *        PufferWechsel::AppEinstellung steht. Wie beim Render-Scale vor
     *        jedem loadMilk*-Aufruf unter renderMutex() setzen.
     * @param modus  Behalten/Loeschen/Fading (AppEinstellung ist hier ungueltig
     *               und faellt auf Behalten zurueck)
     * @param fading Erbe-Anteil 0..1 fuer den Modus Fading
     */
    void setMilkdropPufferWechselDefault(lumi::multieffect::PufferWechsel modus,
                                         double fading, double ausblendSek = 2.0)
    {
        using lumi::multieffect::PufferWechsel;
        m_milkPufferWechselDefault =
            modus == PufferWechsel::AppEinstellung ? PufferWechsel::Behalten : modus;
        m_milkPufferFadingDefault =
            fading < 0.0 ? 0.0 : (fading > 1.0 ? 1.0 : fading);
        m_milkPufferAusblendSekDefault =
            ausblendSek < 0.1 ? 0.1 : (ausblendSek > 60.0 ? 60.0 : ausblendSek);
    }

    /**
     * @brief Sicht-Blende (S67, Settings-Panel `milkdrop/sichtBlende`): an ⇒
     *        Milkdrop-Nodes blenden nach frischer Rausch-Saat (Kaltstart/
     *        Resize/Loeschen-Wipe) ~0,5 s von Schwarz ein (rein kosmetisch).
     *        Wie die anderen Milkdrop-Defaults unter renderMutex() setzen.
     */
    void setMilkdropSichtBlende(bool an) { m_milkSichtBlende = an; }

    /**
     * @brief Erzwingt deterministisch alle N Frames einen Beat statt des
     *        Detektors (0 = Detektor) — Gegenstueck zu AvsRef --beat-period
     *        fuer frame-exakte Diffs von History-Presets (S46-Merkregel:
     *        Beat-Divergenz laesst sie nach ~100 Frames auseinanderlaufen).
     *        Frame-Zaehlung startet je geladenem Preset bei 0 (Beat auf 0, N, …).
     */
    void setBeatPeriodOverride(int frames)
    {
        m_beatPeriodOverride = frames > 0 ? frames : 0;
        m_beatPeriodFrame = 0;
    }

    /**
     * @brief Test-Hook (GL-Gates, S48): Kopie der aktuellen Root-Surface als
     *        Bild — nur mit current GL-Context aufrufen (nach render());
     *        leer, solange noch kein Frame gerendert wurde. Liest die interne
     *        Surface VOR dem Present (unabhaengig vom Fenster-Framebuffer).
     */
    [[nodiscard]] QImage debugGrabRootSurface() const;

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

    /** One FyrewurX spark (NDC position/velocity, remaining life). */
    struct FwSpark
    {
        float x = 0.0f, y = 0.0f;
        float vx = 0.0f, vy = 0.0f;
        float life = 0.0f;     ///< seconds remaining
        float lifeMax = 1.0f;  ///< spawn lifetime (for the fade)
        float r = 1.0f, g = 1.0f, b = 1.0f;
    };

    /** One Starfield particle (normalized coords, z in (0,1]). */
    /// Ein Stern wie r_stars.cpp: X/Y sind GANZZAHLIGE Pixel-Abstaende zur
    /// Bildmitte, Z die Tiefe, Speed der Einzeltempo-Faktor.
    struct Star
    {
        int x = 0;
        int y = 0;
        float z = 1.0f;
        float speed = 0.5f;
    };

    /** One Dot-Fountain particle (polar around the fountain axis). */
    /// Ein Punkt der Dot-Fountain-Hoehenwand (`FountainPoint`, r_dotfnt.cpp:46).
    /// Das Feld ist ein 30x256-GITTER, kein Partikelschwarm: 30 Speichen mal
    /// 256 Alterungsstufen. Je Frame rutscht jede Stufe eine weiter nach hinten
    /// und bekommt dabei ihre Physik; Stufe 0 wird aus dem Spektrum neu gesetzt.
    struct FountainP
    {
        float r = 0.0f;   ///< Radius von der Achse
        float dr = 0.0f;  ///< Radius-Zuwachs je Frame
        float h = 0.0f;   ///< Hoehe
        float dh = 0.0f;  ///< Hoehen-Zuwachs je Frame
        float ax = 0.0f;  ///< sin(Speichenwinkel) — einmal beim Erzeugen
        float ay = 0.0f;  ///< cos(Speichenwinkel)
        uint32_t c = 0;   ///< Farbe aus der 64er-Tabelle
    };

    /** Per-leaf render-thread state (beat counters, scripts), keyed by nodeId. */
    struct LeafRuntime
    {
        int beatCounter = 0;     ///< OnBeat Clear: beats since last clear
        int beatFramesLeft = 0;  ///< Colorfade: frames the beat faders stay on
        /// Colorfade: der laufende Fader-Zustand (`faderpos` in r_colorfade).
        /// Er wandert je Frame um EINEN Schritt auf sein Ziel zu, wenn
        /// `slowFade` gesetzt ist — deshalb muss er ueber Frames leben.
        /// `fadeSeeded` traegt zusaetzlich, ob er schon einmal aus den
        /// Preset-Werten gesetzt wurde (Startwert, s. `interfRotationSeed`).
        int fadePos[3] = {0, 0, 0};
        bool fadeSeeded = false;
        int mirrorRBeat = 0;     ///< Mirror onbeat-random: current direction bits
        float mirrorF[4] = {0.0f, 0.0f, 0.0f, 0.0f};  ///< smooth factors per direction
        int mirrorFrames = 0;    ///< Mirror: frame counter for the `slower` ramp

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
        std::string picSnapshot;      ///< Bilddaten hinter der Textur — Wechsel = neu aufbauen

        // Text (r_text): word-cycler state + rendered glyph texture
        int textCurWord = 0;
        int textNf = 0;                ///< frames since last word switch
        int textOddEven = 0;           ///< insertBlank alternator
        int textNb = 0;                ///< onbeat: frames the beat word stays
        float textRandX = 0.0f;        ///< randomPos offset (fraction of width)
        float textRandY = 0.0f;
        std::string textSnapshot;      ///< draw-state snapshot behind textTexture
        unsigned int textTexture = 0;  ///< GL RGBA glyph layer (deleted in onCleanup)

        // AVI (r_avi): VfW stream handles (opaque void* — vfw.h stays in the .cpp)
        void* aviFile = nullptr;       ///< PAVIFILE
        void* aviStream = nullptr;     ///< PAVISTREAM
        void* aviGetFrame = nullptr;   ///< PGETFRAME
        int aviLength = 0;
        int aviFrameIndex = 0;
        int aviPersistLeft = 0;        ///< beat persist window countdown
        std::int64_t aviLastMs = 0;    ///< last frame advance (speed throttle)
        bool aviTried = false;         ///< open attempted once (no retry spam)
        std::string aviPath;           ///< geoeffneter Pfad — Wechsel = neu oeffnen
        int aviWarnedBpp = 0;          ///< bit depth already reported as unsupported
        unsigned int aviTexture = 0;   ///< GL RGBA frame texture (deleted in onCleanup)
        /// Qt-Multimedia-Fallback (S59): Frame-Cache fuer Codecs, die das
        /// 64-Bit-VfW nicht dekodiert (Indeo & Co.)
        std::shared_ptr<lumi::services::VideoFrameCache::Clip> aviClip;

        // Texer II / Triangle: EEL point-loop scripts
        std::unique_ptr<lumi::scripting::ScriptSlotHost> texerHost;
        std::string texerCompiled;
        std::unique_ptr<lumi::scripting::ScriptSlotHost> triHost;
        std::string triCompiled;

        /// Parameter-Skript eines Knotens (Strang D): init/frame/beat rechnen
        /// die Regler je Frame aus. Nur angelegt, wenn wirklich Code da ist.
        std::unique_ptr<lumi::scripting::ScriptSlotHost> paramHost;
        std::string paramCompiled;
        /**
         * Fortgeschriebener Stand der Skript-Groessen (Entscheid Patrik S54).
         *
         * Ohne ihn konnte der Init-Slot nichts bewirken: die Vorbelegung aus
         * den Params lief VOR jedem Frame und ueberschrieb, was Init einmalig
         * gesetzt hatte. Jetzt belegt nur der erste Frame aus den Params vor —
         * danach traegt diese Tabelle den Wert weiter, und die Frames schreiben
         * ihn fort.
         *
         * `paramSeen` haelt den zuletzt gesehenen PARAM-Wert: dreht der Benutzer
         * am Regler, gewinnt der Regler und die Fortschreibung beginnt neu.
         * Sonst waere jeder Regler tot, sobald ein Skript im Knoten steht.
         */
        std::unordered_map<std::string, double> paramState;
        std::unordered_map<std::string, double> paramSeen;
        /// Letzte Fehlermeldung des Parameter-Skripts ("" = fehlerfrei).
        /// Zugleich die Sperre gegen eine Meldung je Frame (S54).
        std::string paramError;

        /**
         * Bloom: die vom Parameter-Skript gerechneten Werte, damit sie den
         * **Present-Pfad** erreichen.
         *
         * `post` ist per Vorgabe an, und dann kehrt `runBloom` sofort zurueck —
         * der Glow entsteht erst beim Present. Bis S54 lief das Skript in
         * genau diesem Normalfall nie, seine drei Felder standen wirkungslos
         * im Panel (gefunden von den Feld-Sonden). Jetzt rechnet `runBloom`
         * das Skript IMMER und legt das Ergebnis hier ab.
         */
        float bloomIntensity = 1.0f;
        float bloomThreshold = 0.0f;
        int bloomRadius = 8;
        float bloomVigStrength = 0.3f;

        // Movement / Dynamic Movement: scripted displacement grid
        std::unique_ptr<lumi::modules::ScriptGridModule> grid;
        std::string gridCompiled;
        int gridFieldW = 0;  ///< surface size the static field was computed for
        int gridFieldH = 0;
        /// Movement sourcemapped runtime bits (r_trans member state; bit1
        /// toggles bit0 on every beat); -1 = seed from params on first frame
        int moveSourceMapped = -1;
        int moveSourceMappedSeen = -1;  ///< zuletzt uebernommener Preset-Wert
        /// Movement (r_trans): per-Pixel-Tabelle als R32I-Textur — wie im
        /// Original nur bei Groessen-/Skriptwechsel neu gebaut (teuer!)
        unsigned int moveTabTex = 0;  ///< GL R32I w*h (deleted in onCleanup)
        /// Grain (r_grain): depthBuffer als RG8-Textur; die Zufallszuege des
        /// Originals laufen ueber den geteilten Preset-Strom (S49)
        unsigned int grainTex = 0;
        int grainW = 0;
        int grainH = 0;
        bool grainSeeded = false;
        int moveTabW = 0;
        int moveTabH = 0;
        std::string moveTabKey;  ///< Skript + Flags, hinter denen die Tabelle steht

        // SuperScope: scripted point generator (drawn via the shared renderer)
        std::unique_ptr<lumi::modules::SuperscopeModule> scope;
        std::string scopeCompiled;

        // Roto Blitter (r_rotblit, S48): Richtungs- und Zoom-Ease-Zustand —
        // die Rotation selbst akkumuliert uebers FEEDBACK, nicht hier.
        bool rotoSeeded = false;
        float rotoRev = 1.0f;     ///< Zielrichtung (+1/-1; Beat toggelt)
        float rotoRevPos = 1.0f;  ///< geeaster Richtungsfaktor
        int rotoFpos = 31;        ///< scale_fpos (Zoom-Ease)
        // Blitter Feedback (r_blit, S48): fpos-Ease (+-3/Frame Richtung scale)
        bool bfSeeded = false;
        int bfFpos = 30;

        // Custom BPM
        std::int64_t customLastMs = 0;  ///< arbitrary-mode last emit time
        int customSkipCount = 0;        ///< skip-mode beat counter
        int customBeatCount = 0;        ///< alle Beats seit Preset-Start (skipfirst)

        // Mosaic: on-beat quality ease-back (r_mosaic thisQuality/nF)
        float mosaicQuality = 0.0f;  ///< current interpolated block count (0 = init)
        int mosaicFramesLeft = 0;    ///< frames left easing back to `quality`

        // Buffer Save: alternate-direction toggle (r_stack dir_ch)
        bool bufDirCh = false;

        // FyrewurX: live sparks (spawned per beat, gravity-integrated)
        std::vector<FwSpark> fwSparks;

        // SuperScope: gradient preset the module currently holds (reload on change)
        std::string scopeGradientLoaded;

        // Interferences: accumulating rotation + on-beat morph state (r_interf)
        bool interfSeeded = false;
        /// Laufender Drehwinkel in 1/255 Umdrehungen. GANZZAHLIG wie in der
        /// Referenz (`rotation` ist dort ein int, r_interf.cpp:384) — eine
        /// float-Summe laeuft anders auf.
        int interfRotation = 0;
        float interfStatus = 0.0f;    ///< beat-morph phase (0..pi)
        /// Der Preset-Wert, mit dem der Zaehler oben gesetzt wurde. `rotation`
        /// ist ein STARTWERT: nach dem Seeden laeuft `interfRotation`
        /// selbstaendig weiter, und ein Panel-Edit ruft nur `recompileChain()`
        /// — ohne diesen Vergleich kam ein neuer Wert nie an (Strang F, S57).
        int interfRotationSeed = -1;

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
        /// Bass Spin: last_a ist im Original EIN Member ueber beide Kanaele
        /// (r_bspin.cpp — der linke Kanal beeinflusst das a des rechten; S48)
        int bsLastA = 0;
        int bsLx[2][2] = {{0, 0}, {0, 0}};  ///< letzte Spitze [spoke][kanal] (px)
        int bsLy[2][2] = {{0, 0}, {0, 0}};

        // Moving Particle: spring-particle state (r_parts)
        bool mpSeeded = false;
        float mpCx = 0.0f, mpCy = 0.0f;   ///< spring target (randomized on beat)
        float mpVx = 0.0f, mpVy = 0.0f;   ///< velocity
        float mpPx = 0.0f, mpPy = 0.0f;   ///< position (-1..1)
        float mpSize = 8.0f;              ///< eased radius

        // Bloom (Lights-Etappe 1): kleine Glow-RTs — [0] Downsample/Gauss-Ziel,
        // [1] Gauss-Zwischenpuffer; Groesse = Surface / 2^downsample
        std::unique_ptr<QOpenGLFramebufferObject> bloomRt[2];
        int bloomW = 0;
        int bloomH = 0;

        // 3D Camera (Lights-Etappe 1): EEL-Slots init/frame/beat — duerfen
        // die Kamera-Parameter ueberschreiben (dynamische Modulparameter)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> cam3dHost;
        std::string cam3dCompiled;

        // SuperScope 3D (Lights-Etappe 1): EEL-Quartett (x/y/z/size/rgb)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> scope3dHost;
        std::string scope3dCompiled;

        // Terrain 3D (Lights-Etappe 2): Hoehen-Simulation + Mesh-Grid
        std::unique_ptr<lumi::scripting::ScriptSlotHost> terrainHost;
        std::string terrainCompiled;
        std::vector<float> terrainBase;  ///< h0 (prozedural, fester Seed)
        std::vector<float> terrainH;     ///< aktuelle Hoehen
        std::vector<float> terrainV;     ///< Feder-Geschwindigkeiten
        int terrainRes = 0;              ///< Aufloesung der Puffer (Reset bei Wechsel)
        std::unique_ptr<QOpenGLVertexArrayObject> terrainVao;
        std::unique_ptr<QOpenGLBuffer> terrainVbo;  ///< pos.xyz (je Frame)
        std::unique_ptr<QOpenGLBuffer> terrainIbo;  ///< Quad-Dreiecke (je res)
        int terrainIndexCount = 0;

        // Glow Orbs (Lights-Etappe 2): EEL-Quartett (Point je Orb)
        std::unique_ptr<lumi::scripting::ScriptSlotHost> orbsHost;
        std::string orbsCompiled;

        // Water Bump: RGBA16F height ping-pong (.r current, .g previous height)
        std::unique_ptr<QOpenGLFramebufferObject> wbHeight[2];
        int wbCur = 0;   ///< index of the current height buffer
        int wbW = 0;
        int wbH = 0;

        // Starfield: CPU star particles + warp-speed ease (r_stars)
        std::vector<Star> stars;
        float starSpeed = 0.0f;    ///< CurrentSpeed (r_stars); 0 = uninitialisiert
        int starBeatFrames = 0;    ///< nc — Frames bis WarpSpeed wieder gilt
        float starIncBeat = 0.0f;  ///< incBeat — Schrittweite der Rueckkehr
        int starW = 0;             ///< Groesse, fuer die die Sterne gesetzt wurden
        int starH = 0;

        int timescopeX = 0;        ///< Timescope: current spectrogram column

        // Dot Grid / Plane / Fountain (r_dotgrid/dotpln/dotfnt)
        float dotColorPos = 0.0f;  ///< Dot Grid: colour-table cycle position
        float dotOffX = 0.0f;      ///< Dot Grid: scroll offset
        float dotOffY = 0.0f;
        float dotRot = 0.0f;       ///< Dot Fountain: accumulating rotation
        /// Preset-Startwinkel, mit dem `dotRot` geseedet wurde (Bauart
        /// `interfRotationSeed`: STARTWERT, danach laeuft der Winkel selbst).
        float dotRotSeed = std::numeric_limits<float>::quiet_NaN();
        std::vector<FountainP> fountain;  ///< Dot Fountain particles

        // Dot Plane (r_dotpln, S48-Neuschrieb): scrollendes 64x64-Grid mit
        // Physik; die Injektionszeile traegt Hoehe UND Farbe aus dem Spektrum
        // (color_tab[Byte>>2]) — die Farbe wandert mit der Zeile mit.
        std::vector<float> dpHeights;    ///< atable (64*64)
        std::vector<float> dpVel;        ///< vtable
        std::vector<uint32_t> dpColors;  ///< ctable (FB-Ints)
        float dpR = 0.0f;                ///< akkumulierte Rotation (Grad)
        /// Preset-Startwinkel, mit dem `dpR` geseedet wurde (s. `dotRotSeed`).
        float dpRSeed = std::numeric_limits<float>::quiet_NaN();
        bool dpSeeded = false;

        int apeChanMode = -1;      ///< Channel Shift on-beat held permutation
        int apeChanSeed = -1;      ///< Preset-Wert, aus dem apeChanMode geseedet wurde

        // Fractal 2D (Batch H): EEL view-driver + gradient palette LUT
        std::unique_ptr<lumi::scripting::ScriptSlotHost> fracHost;
        std::string fracCompiled;      ///< source snapshot the host was built from
        bool fracInited = false;       ///< init slot has run once
        unsigned int fracLut = 0;      ///< GL 256x1 RGB palette (deleted in onCleanup)
        std::string fracLutSnapshot;   ///< gradient preset the LUT was baked from
        float fracColorPhase = 0.0f;   ///< accumulated palette drift (colorCycle)
        float fracTime = 0.0f;         ///< accumulated animation phase (Domain Warp / Zoomer log-zoom)
        float fracRot = 0.0f;          ///< Fractal Zoomer: akkumulierte Rotation (S48: eigenes Feld, war rotoAngle)

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

        // Milkdrop-Meganode (N1, Entscheid E1): der komplette MilkDrop-Kern als
        // Engine je Node; GL-Freigabe via cleanup() in resetRuntimes
        std::unique_ptr<MilkdropVisualizer> milk;
        uint64_t milkRevision = 0;  ///< zuletzt uebernommene Params-Revision
        /// Puffer-Wechsel (S66): zuletzt gesehener wechselZaehler — bei neuem
        /// Stand meldet der Host wechselErbe beim Kern an
        uint64_t milkWechselZaehler = 0;

        // Shadertoy-Node (Strang S, S65): pro Node kompiliertes Wrapper-Programm.
        // Programm stirbt mit m_leafRuntimes.clear() (Context ist dort current).
        std::unique_ptr<QOpenGLShaderProgram> stProgram;
        std::string stCompiled;  ///< Code-Snapshot hinter den Programmen (alle Pässe)
        std::string stError;     ///< letzter Kompilierfehler ("" = ok; Panel fragt ab)
        int stFrame = 0;         ///< iFrame seit Kompilierung (deterministisch)
        // Multipass (S4): Buffer A..D als RGBA32F-Ping-Pong in Chain-Auflösung.
        // Nach jedem Buffer-Render wird cur GESWAPT — dadurch liest ein
        // späterer Pass automatisch das frische Bild, ein früherer/selbst-
        // referenzierender das Vorframe (Original-Semantik).
        std::unique_ptr<QOpenGLShaderProgram> stBufProgram[4];
        std::unique_ptr<QOpenGLFramebufferObject> stBufFbo[4][2];
        int stBufCur[4] = {0, 0, 0, 0};
        int stBufW = 0;
        int stBufH = 0;
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

    /**
     * Render-thread state of one host group (HG1), keyed by nodeId: an own
     * persistent surface (the group's feedback image — never cleared per
     * frame), an OWN buffer pool (Buffer-Save slots do not leak between
     * groups) and an OWN ScriptContext (reg/q/gmegabuf group-local).
     */
    struct GroupRuntime
    {
        SurfacePair surface;
        bool needsClear = true;
        bool seenThisFrame = false;
        std::unique_ptr<lumi::render::OffscreenBufferPool> pool;
        std::shared_ptr<lumi::scripting::ScriptContext> context;
        /// HG2: Blend-Gewicht 0..1 — folgt `enabled` ueber crossfadeSeconds
        /// (0 startend = frisch aktivierte Gruppen blenden EIN); eine
        /// deaktivierte Gruppe rendert weiter, bis das Gewicht 0 erreicht.
        double blendWeight = 0.0;
        /// HG3: Laufzeit seit (Re-)Aktivierung der Gruppe — progress-Quelle
        /// fuer Milkdrop-Nodes in der Gruppe (Playlist liefert spaeter die
        /// echte Slot-Dauer); Reset beim Frischstart nach vollem Ausblenden.
        double activeSeconds = 0.0;
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
    /// Host-Gruppe (HG1): Kinder rendern auf den persistenten Gruppen-Buffer
    /// (Feedback, kein per-Frame-Clear) mit gruppen-eigenem Buffer-Pool +
    /// ScriptContext; blendOut mischt das Gruppen-Bild auf den Parent.
    void renderHostGroup(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::HostGroupParams& params);
    void runClear(const lumi::multieffect::ChainNode& node,
                 const lumi::multieffect::ClearParams& params);
    /// Milkdrop-Meganode (N1): rendert die feste MilkDrop-Pipeline in den
    /// aktiven Chain-Buffer (Composite-Ziel = beim Kern-Frame-Start gebundenes FBO)
    void runMilkdropNode(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::MilkdropNodeParams& params);
    /// Host-Audio (Kanal-Kopien) interleaved an den Milkdrop-Kern durchreichen
    void feedMilkAudio(MilkdropVisualizer& milk);
    void runFadeout(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::FadeoutParams& params);
    void runInvert();
    void runBrightness(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::BrightnessParams& params);
    void runFastBrightness(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::FastBrightnessParams& params);
    void runBlur(const lumi::multieffect::ChainNode& node,
                const lumi::multieffect::BlurParams& params);
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
    void runBufferBlend(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::BufferBlendParams& params);
    void runJherikoGlobal(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::JherikoGlobalParams& params);
    void runColorClip(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::ColorClipParams& params);
    void runUniqueTone(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::UniqueToneParams& params);
    void runInterleave(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::InterleaveParams& params);
    void runConvolution(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::ConvolutionParams& params);
    void runNormalise();
    void runMultiFilter(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::MultiFilterParams& params);
    void runAddBorders(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::AddBordersParams& params);
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
    void runText(const lumi::multieffect::ChainNode& node,
                 const lumi::multieffect::TextParams& params);
    void runAvi(const lumi::multieffect::ChainNode& node,
                const lumi::multieffect::AviParams& params);
    /// Release the VfW handles of an AVI node (safe on empty runtimes).
    static void closeAviRuntime(LeafRuntime& rt);
    /// GL blend state for an AVS BLEND_LINE mode 0..9 (S9; 8 falls back to add).
    static void applyLineBlend(int mode, int adjustAlpha);
    /// Restore GL_FUNC_ADD + disable blending after a line-blend draw.
    static void resetLineBlend();
    void runPictureII(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::PictureIIParams& params);
    void runTexer(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::TexerParams& params);
    void runTexerII(const lumi::multieffect::ChainNode& node,
                    const lumi::multieffect::TexerIIParams& params);
    void runTriangle(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::TriangleParams& params);
    /// Decode the base64 image into rt.picTexture once; true when ready.
    bool ensureEmbeddedTexture(LeafRuntime& rt, const std::string& imageData,
                               bool fallbackDot = false);
    /// Draw an embedded (base64) image over the frame, blended.
    void drawEmbeddedImage(LeafRuntime& rt, const std::string& imageData, int blend,
                           bool keepAspect);
    /// Draw a scope point-list via the shared ScopeRenderer. Honors an active
    /// Set Render Mode (S3/S9: BLEND_LINE + line width); additive otherwise.
    void drawScopeShape(const std::vector<lumi::modules::SuperscopePoint>& pts, bool dots);
    /// Dot-batch variant (Dot Grid/Plane/Fountain). blend: 0 replace,
    /// 1 additive, 2 50/50, 3 BLEND_LINE (folgt SRM — Referenz-Default).
    void drawDots(const std::vector<lumi::modules::SuperscopePoint>& pts, float dotSize,
                  int blend = 3);
    void runDynamicMovement(const lumi::multieffect::ChainNode& node,
                            const lumi::multieffect::DynamicMovementParams& params);
    /** Shared grid-warp: run the module, build the mesh, sample current→partner. */
    /// Options for applyGridWarp beyond the mandatory grid geometry.
    struct GridWarpOptions
    {
        bool wrap = false;
        bool blend = false;        ///< alpha-mix moved pixel onto the original
        bool nomove = false;       ///< no displacement, alpha-blend source only
        bool staticField = false;  ///< evaluate the field only on compile/resize
        bool subpixel = true;      ///< bilinear (on) vs nearest (off) sampling
        unsigned int srcTexture = 0;  ///< warp source; 0 = current frame
    };
    void applyGridWarp(LeafRuntime& rt, int xres, int yres,
                       const GridWarpOptions& opt);
    /**
     * Bit-treuer Warp-Pass fuer Dynamic Movement (r_dmove): die Fixpunkt-Tabelle
     * des Gitters geht als Integer-Textur an den Shader, der die separable
     * Ganzzahl-Interpolation des Originals geschlossen nachrechnet. Liefert
     * false, wenn das Original an dieser Geometrie abbricht (Band der Breite 0).
     */
    bool applyGridWarpFx(LeafRuntime& rt, int xres, int yres,
                         const GridWarpOptions& opt);
    void applyGridScatter(LeafRuntime& rt, int xres, int yres,
                          const GridWarpOptions& opt);
    /**
     * Bit-treuer Movement-Pass (r_trans): das Punkt-Skript laeuft je PIXEL in
     * eine Tabelle (nur bei Groessen-/Skriptwechsel), der Shader dekodiert sie
     * samt 5-Bit-Subpixel. false = kein lebender Punkt-Slot (Gitter-Fallback).
     */
    bool applyMovementTable(LeafRuntime& rt,
                            const lumi::multieffect::MovementParams& params);
    void runBlitterFeedback(const lumi::multieffect::ChainNode& node,
                            const lumi::multieffect::BlitterFeedbackParams& params);
    void runRotoBlitter(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::RotoBlitterParams& params);
    /**
     * Gemeinsamer Feedback-Pass (r_blit/r_rotblit, S48): affines Sampling in
     * Pixeln (src = map*dest + off), wrap = Kachel-Modulo wie r_rotblit,
     * blend = BLEND_AVG mit dem Original, subpixel = BLEND4-Bilinear.
     */
    void feedbackPass(const QMatrix2x2& map, const QVector2D& off, bool wrap,
                      bool blend, bool subpixel);
    void runBufferSave(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::BufferSaveParams& params);
    void runFyrewurX(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::FyrewurXParams& params);
    void runMetaballs3D(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::Metaballs3DParams& params);
    void runTentacles3D(const lumi::multieffect::ChainNode& node,
                        const lumi::multieffect::Tentacles3DParams& params);
    void runCustomBpm(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::CustomBpmParams& params);
    void runSetRenderMode(const lumi::multieffect::SetRenderModeParams& params);
    void runSuperScope(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::SuperScopeParams& params);
    void runDebugBars(const lumi::multieffect::DebugBarsParams& params);
    void runMosaic(const lumi::multieffect::ChainNode& node,
                   const lumi::multieffect::MosaicParams& params);
    void runGrain(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::GrainParams& params);
    void runScatter(const lumi::multieffect::ScatterParams& params);
    void runInterferences(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::InterferencesParams& params);
    void runWater(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::WaterParams& params);
    void runBump(const lumi::multieffect::ChainNode& node,
                 const lumi::multieffect::BumpParams& params);
    void runWaterBump(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::WaterBumpParams& params);
    /// Bloom (Lights-Etappe 1), In-Chain-Modus (post=false): Glow erzeugen
    /// und additiv auf die Surface compositen; post=true rendert als No-op
    /// (der Glow entsteht dann erst beim Present — kein Feedback).
    void runBloom(const lumi::multieffect::ChainNode& node,
                  const lumi::multieffect::BloomParams& params);
    /// Bloom-Glow erzeugen: Downsample (+Threshold) → separierbarer
    /// 25-Tap-Gauss in rt.bloomRt; liefert die Glow-Textur (0 bei Fehler).
    /// `threshold`/`radius` kommen getrennt herein, weil sie im Knoten aus der
    /// Strang-D-Frame-Kopie stammen koennen und nicht aus `params` (S54).
    unsigned int ensureBloomGlow(LeafRuntime& rt,
                                 const lumi::multieffect::BloomParams& params,
                                 unsigned int srcTexture, float threshold,
                                 int radius);
    /// 3D Camera (Lights-Etappe 1): setzt m_camera3d fuer folgende 3D-Module
    /// (Frame-Zustand wie Set Render Mode); EEL-Slots als dynamische Params.
    void runCamera3D(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::Camera3DParams& params);
    /// SuperScope 3D (Lights-Etappe 1): EEL-Punktschleife in Weltkoordinaten,
    /// Projektion via m_camera3d, additive Soft-Sprites oder Linien.
    void runSuperScope3D(const lumi::multieffect::ChainNode& node,
                         const lumi::multieffect::SuperScope3DParams& params);
    /// Terrain 3D (Lights-Etappe 2): Heightfield-Sim (Ringe/Relax/Skript-
    /// megabuf) + dunkles Mesh (Depth) + additive Gitterpunkt-Sprites.
    void runTerrain3D(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::Terrain3DParams& params);
    /// Glow Orbs (Lights-Etappe 2): Ellipsoide (Verlauf+flash, Depth) + Halos.
    void runGlowOrbs(const lumi::multieffect::ChainNode& node,
                     const lumi::multieffect::GlowOrbsParams& params);
    /// View/Proj aus m_camera3d (Aspekt = interne Surface, near 0.05).
    void computeCamera3D(QMatrix4x4& view, QMatrix4x4& proj) const;
    /// Gemeinsames Depth-RT (Etappe-2-Entscheid 1): Depth-Textur an das
    /// AKTUELL gebundene Draw-FBO haengen, einmal je Frame loeschen,
    /// Depth-Test an. end3DDepth() loest das Attachment wieder.
    void begin3DDepth();
    void end3DDepth();
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
    void runColorReduction(const lumi::multieffect::ChainNode& node,
                          const lumi::multieffect::ColorReductionParams& params);
    void runMultiplier(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::MultiplierParams& params);
    void runVideoDelay(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::VideoDelayParams& params);
    void runMultiDelay(const lumi::multieffect::ChainNode& node,
                       const lumi::multieffect::MultiDelayParams& params);
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
    /// Shadertoy-Node (Strang S): ein Fragment-Pass in Chain-Auflösung.
    void runShadertoy(const lumi::multieffect::ChainNode& node,
                      const lumi::multieffect::ShadertoyParams& params);
    /// 512×2-Audio-Textur des Shadertoy-Vertrags hochladen (Zeile 0 = FFT,
    /// Zeile 1 = Waveform, 0..1) — je Aufruf frisch (1 KiB, unkritisch).
    void updateShadertoyAudioTexture();
    /// Bake a gradient preset into a node's 256x1 palette LUT (Batch H shared).
    void ensureFractalLut(LeafRuntime& rt, const std::string& preset);

    /// Rebuild m_visdata (AVS layout) from this frame's waveform + spectrum.
    void buildVisData();
    /// Rohe visdata-Bytes (AVS-Vertrag, S48): getSpectrum()/getWaveform()
    /// sind NORMALISIERT (App-Skala) — AVS-treue Effekte lesen stattdessen
    /// diese Bloecke (Spektrum linear /16, Waveform-Bytes ^128-Konvention);
    /// dieselbe Quelle wie getspec/getosc der Skripte (eine Wahrheit).
    [[nodiscard]] const unsigned char* visSpectrum(int channel) const
    {
        return m_visdata.data() + (channel & 1) * 576;
    }
    [[nodiscard]] const unsigned char* visWaveform(int channel) const
    {
        return m_visdata.data() + (2 + (channel & 1)) * 576;
    }
    /// Normalisierte Welle/Spektrum nach dem AVS-Kanalfeld: 0 = links,
    /// 1 = rechts, alles andere = Mitte. `getWaveform()`/`getSpectrum()`
    /// mischen bei Stereo-Material selbst zur Mitte (`mixToMono`), deshalb ist
    /// der Mittenfall genau der bisherige Aufruf ohne Kanal — ein Knoten mit
    /// der ueblichen Vorgabe `channel = 2` zeichnet unveraendert weiter.
    /// EINE Stelle, weil Osc Star und Osc Ring dieselbe Wahl treffen (S57: dort
    /// wurde der Kanal gar nicht gelesen, beide Sonden waren stumm).
    [[nodiscard]] std::vector<float> waveOfChannel(int channel) const
    {
        return (channel == 0 || channel == 1) ? getWaveformChannel(channel)
                                             : getWaveform();
    }
    [[nodiscard]] std::vector<float> specOfChannel(int channel) const
    {
        return (channel == 0 || channel == 1) ? getSpectrumChannel(channel)
                                             : getSpectrum();
    }
    /// Feed the shared audio contract (visdata + gettime + bass/mid/treb/vol/beat/
    /// time) into a script engine — every scripted module gets the same set (E1).
    void feedAudio(lumi::scripting::LuaScriptEngine& engine);

    /// Ein Reglerwert, den das Parameter-Skript lesen und schreiben darf.
    struct ParamVar
    {
        const char* name;  ///< EEL-Bezeichner (klein geschrieben)
        double* value;     ///< rein VOR dem Lauf, raus DANACH
    };

    /**
     * Parameter-Skript eines Knotens fahren (Strang D, Knoten_Parameter_Konzept §6).
     *
     * Sind alle drei Quellen leer, passiert NICHTS — kein Host, kein Transpiler,
     * kein Lua-Aufruf. Erst ein nicht-leeres Feld kostet Rechenzeit (opt-in).
     * Die aktuellen Reglerwerte gehen als Startwerte hinein, damit ein Skript,
     * das nur eine Variable setzt, alle anderen unangetastet laesst.
     */
    void runParamScript(LeafRuntime& rt, const char* prefix, const std::string& init,
                        const std::string& frame, const std::string& beat,
                        const std::vector<ParamVar>& vars);

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
    /// Scratch ping-pong for blended Buffer-Save writes into pool FBOs
    SurfacePair m_bufferScratch;
    std::vector<SurfacePair*> m_surfaceStack;
    std::unordered_map<uint64_t, ListRuntime> m_listRuntimes;
    std::unordered_map<uint64_t, GroupRuntime> m_groupRuntimes;  ///< HG1
    /// HG2: laufende Gewichtssumme des normalisierten Gruppen-Mixes — startet
    /// je Frame beim Hintergrund-Anteil max(0, 1 - Summe blendender Gruppen)
    double m_blendRunningSum = 1.0;
    /// HG3: Laufzeit der AKTIVEN Gruppe waehrend des Kinder-Walks (Scope wie
    /// activePool/activeContext); -1 = ausserhalb jeder Host-Gruppe
    double m_groupActiveSeconds = -1.0;
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
    /// Feld-Shader des Metaballs-3D-Nachbaus (S52)
    std::unique_ptr<QOpenGLShaderProgram> m_metaballShader;
    std::unique_ptr<QOpenGLShaderProgram> m_colorfadeShader;
    std::unique_ptr<QOpenGLShaderProgram> m_lutShader;
    std::unique_ptr<QOpenGLShaderProgram> m_warpShader;
    std::unique_ptr<QOpenGLShaderProgram> m_warpFxShader;  ///< r_dmove-Fixpunkt
    std::unique_ptr<QOpenGLShaderProgram> m_moveTabShader;  ///< r_trans-Tabelle
    std::unique_ptr<QOpenGLShaderProgram> m_moveRemapShader;
    std::unique_ptr<QOpenGLShaderProgram> m_textShader;
    std::unique_ptr<QOpenGLShaderProgram> m_feedbackShader;
    std::unique_ptr<QOpenGLShaderProgram> m_mosaicShader;
    std::unique_ptr<QOpenGLShaderProgram> m_grainShader;
    std::unique_ptr<QOpenGLShaderProgram> m_scatterShader;
    std::unique_ptr<QOpenGLShaderProgram> m_interfShader;
    std::unique_ptr<QOpenGLShaderProgram> m_waterShader;
    std::unique_ptr<QOpenGLShaderProgram> m_bumpShader;
    std::unique_ptr<QOpenGLShaderProgram> m_presentShader;  ///< Quad-Present (Render Scale, S47)
    std::unique_ptr<QOpenGLShaderProgram> m_bloomDownShader;   ///< Bloom: Downsample + Threshold
    std::unique_ptr<QOpenGLShaderProgram> m_bloomGaussShader;  ///< Bloom: separierbarer 25-Tap-Gauss
    std::unique_ptr<QOpenGLShaderProgram> m_bloomCompShader;   ///< Bloom: additives Composite + Vignette
    std::unique_ptr<QOpenGLShaderProgram> m_sprite3dShader;    ///< SuperScope 3D: Soft-Sprites
    std::unique_ptr<QOpenGLShaderProgram> m_terrain3dShader;   ///< Terrain 3D: opakes Mesh + Fog
    std::unique_ptr<QOpenGLShaderProgram> m_orb3dShader;       ///< Glow Orbs: Ellipsoid + Verlauf
    std::unique_ptr<QOpenGLShaderProgram> m_flatShader;        ///< Flat-Color-Fill (my_triangle, S48)
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
    /// Convolution-Pass-2-Ziel (in-place-Kante, S59) — liest Pass 1 + Eingabe
    std::unique_ptr<QOpenGLFramebufferObject> m_convScratch;
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
    /// Shadertoy-Audio (Strang S): geteilte 512×2-R8-Textur (Zeile 0 = FFT,
    /// Zeile 1 = Waveform); je runShadertoy frisch hochgeladen, Freigabe onCleanup
    unsigned int m_stAudioTex = 0;

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
    /// r_dmove-Gittertabelle als RGBA32I-Textur (x16, y16, a16, 0) + CPU-Scratch
    unsigned int m_warpTabTex = 0;
    std::vector<int> m_warpTab;
    // SuperScope 3D: dynamisches Sprite-Mesh (6 Vertices je Punkt:
    // center.xy + corner.xy + half.xy + rgb; je Frame neu hochgeladen)
    std::unique_ptr<QOpenGLVertexArrayObject> m_sprite3dVao;
    std::unique_ptr<QOpenGLBuffer> m_sprite3dVbo;
    std::vector<float> m_sprite3dVertices;  ///< CPU-Scratch (wie m_warpVertices)
    // Glow Orbs: geteilte Einheitskugel (16x12, Position = Normale)
    std::unique_ptr<QOpenGLVertexArrayObject> m_orbVao;
    std::unique_ptr<QOpenGLBuffer> m_orbVbo;
    std::unique_ptr<QOpenGLBuffer> m_orbIbo;
    int m_orbIndexCount = 0;
    // Flat-Fill-Dreiecke (my_triangle-Ersatz: Bass Spin Modus 1, S48):
    // dynamisches pos.xy-Mesh, Uniform-Farbe, Replace ohne Blend
    std::unique_ptr<QOpenGLVertexArrayObject> m_triVao;
    std::unique_ptr<QOpenGLBuffer> m_triVbo;
    /// GL_TRIANGLES aus NDC-Paaren (x,y je Vertex) in Uniform-Farbe zeichnen
    void drawFlatTriangles(const std::vector<float>& xyNdc, const QVector3D& color);
    // Gemeinsames Depth-RT der opaken 3D-Module (Etappe-2-Entscheid 1):
    // eigene Textur statt FBO-Attachment-Paar — ueberlebt das Farb-Ping-Pong.
    unsigned int m_depth3dTex = 0;  ///< GL_DEPTH_COMPONENT24 (onCleanup)
    int m_depth3dW = 0;
    int m_depth3dH = 0;
    bool m_depth3dCleared = false;  ///< je Frame einmal loeschen (onRender-Reset)

    std::unordered_map<uint64_t, LeafRuntime> m_leafRuntimes;
    std::vector<float> m_milkWaveScratch;  ///< interleavte Audio-Kopien (Meganode)
    std::vector<float> m_milkSpecScratch;
    lumi::render::OffscreenBufferPool m_bufferPool;  ///< 8 global buffers (Buffer Save)
    /// HG1: Buffer-Save-Slots des AKTIVEN Scopes — Root zeigt auf m_bufferPool,
    /// innerhalb einer Host-Gruppe auf deren GroupRuntime::pool (kein Leak
    /// zwischen Gruppen). renderHostGroup schaltet um (Stack-Disziplin).
    lumi::render::OffscreenBufferPool* m_activePool = nullptr;
    [[nodiscard]] lumi::render::OffscreenBufferPool& activePool()
    {
        return m_activePool != nullptr ? *m_activePool : m_bufferPool;
    }
    lumi::render::ScopeRenderer m_scopeRenderer;     ///< shared scope draw (E6)
    uint32_t m_rng = 0x9E3779B9u;  ///< host-local LCG state

    // Chain-scoped beat (design doc block 4; mutable within the frame)
    lumi::modules::BeatModule m_beat;
    lumi::modules::BeatEstimator m_beatEstimator{0};
    bool m_frameBeat = false;  ///< beat flag effects/list scripts may mutate
    int m_beatPeriodOverride = 0;  ///< >0: Beat alle N Frames (AvsRef --beat-period)
    int m_beatPeriodFrame = 0;     ///< Frame-Zaehler des Overrides (Reset beim Laden)
    int m_importRenderScaleDivisor = 1;  ///< Auto-Render-Scale beim AVS-Import (S47)
    /// App-Default Puffer-Wechsel (S66; Node-Einstellung AppEinstellung
    /// delegiert hierher — Behalten = Original-Semantik)
    lumi::multieffect::PufferWechsel m_milkPufferWechselDefault =
        lumi::multieffect::PufferWechsel::Behalten;
    double m_milkPufferFadingDefault = 0.5;  ///< Erbe-Anteil des App-Defaults
    double m_milkPufferAusblendSekDefault = 2.0;  ///< Ausblend-Dauer (s)
    bool m_milkSichtBlende = false;  ///< Sicht-Blende nach Saat (S67, QSettings)

    // Live render mode set by a Set Render Mode node for the following render
    // effects (AVS semantics). Reset at frame start; `set` means "override".
    struct RenderMode
    {
        bool set = false;    ///< a Set Render Mode node applied this frame
        int lineWidth = 1;   ///< line width for following scopes (px)
        /// AVS BLEND_LINE 0..9 (r_defs.h:267-283, S9). g_line_blend_mode
        /// starts as REPLACE (0), is reset per frame and saved/reset/restored
        /// around every list (r_list.cpp:693-694/744, S3).
        int lineBlend = 0;
        int alpha = 128;     ///< Adjustable-blend alpha 0..255
    };
    RenderMode m_renderMode;

    // 3D-Kamera-Zustand, gesetzt von einem camera3d-Knoten fuer die folgenden
    // 3D-Module (Lights-Etappe 1). Reset je Frame auf die Fallback-Kamera:
    // Position 0/0/+1/tan(fov/2), Blick auf den Ursprung (three.js-Konvention
    // wie Lights: x+ rechts, y+ oben, z+ zum Betrachter) — x/y in [-1,1] bei
    // z=0 fuellen das Bild (vertikal). Fog aus (start >= end).
    struct Camera3D
    {
        QVector3D pos{0.0f, 0.0f, 3.7320508f};
        QVector3D target{0.0f, 0.0f, 0.0f};
        float fovDeg = 30.0f;
        float rollDeg = 0.0f;
        float fogStart = 0.0f;   ///< Fog aktiv nur wenn fogEnd > fogStart
        float fogEnd = 0.0f;
        QVector3D fogColor{0.0f, 0.0f, 0.0f};
    };
    Camera3D m_camera3d;

    // AVS-layout visualisation data (spectrum L/R + waveform L/R, 576 each),
    // rebuilt once per frame and fed to every scripted engine (getspec/getosc).
    std::array<unsigned char, 576 * 4> m_visdata{};

    // Preset-local shared script state (decision §10.3)
    std::shared_ptr<lumi::scripting::ScriptContext> m_scriptContext;
    /// HG1: Kontext des AKTIVEN Scopes — Skript-Hosts werden gegen diesen
    /// erzeugt (reg/q/gmegabuf bleiben gruppen-lokal). Root = m_scriptContext.
    std::shared_ptr<lumi::scripting::ScriptContext> m_activeContext;
    [[nodiscard]] const std::shared_ptr<lumi::scripting::ScriptContext>& activeContext()
    {
        return m_activeContext ? m_activeContext : m_scriptContext;
    }

    // Frame-scoped inputs
    float m_time = 0.0f;       ///< seconds since init (DebugBars orbit)
    /// Uhrstand VOR dem Inkrement dieses Frames — das ist die Uhr, die
    /// EEL-`gettime()` sieht: AVS misst "Sekunden seit Start", der erste
    /// Frame liest 0. Mit `m_time` (schon inkrementiert) las er 1/60 — ein
    /// Preset mit 1s-FPS-Zaehlfenster (el-vis_hypno07) feuerte sein Fenster
    /// damit einen Frame zu frueh mit Zaehlerstand 0 und mass fps=0 (S59).
    /// DOUBLE, eigener Akkumulator: die float-Summe von m_time driftet nach
    /// zwei Dutzend Frames um eine Millisekunde — genau die Koernung, in der
    /// gettime() tickt.
    double m_scriptClock = 0.0;
    double m_scriptClockAccum = 0.0;  ///< Summe aller dt (double)
    float m_deltaTime = 0.0f;  ///< seconds since last frame (script modules)
    float m_audioLevel = 0.0f; ///< smoothed waveform RMS 0..1
};
