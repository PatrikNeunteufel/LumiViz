/**
 ****************************************************************************************
 * @file   VisualizerRenderThread.hpp
 * @brief  Dedicated OpenGL render thread + GL window for visualizer rendering
 *
 * @author Patrik Neunteufel
 * @date   July 2026
 * @version 1.0.0 - Render-Thread-Entkopplung (Render_Thread_Entwurf.md)
 *
 * @details
 * ## Architecture (see docs/visuals/Render_Thread_Entwurf.md)
 *
 * ```
 * VisualizerWidget (facade, GUI thread)
 *   ├── VisualizerGLWindow : QWindow      (events arrive on GUI thread)
 *   ├── QOpenGLContext                    (owned, lives on the render thread)
 *   └── VisualizerRenderThread : QThread
 *         loop: wait exposed → apply commands (visualizer swap, pacing,
 *         resize) → take audio snapshot → render (under render mutex)
 *         → swapBuffers (outside the mutex) → pacing sleep
 * ```
 *
 * ## Threading contract
 *
 * - All public methods are called from the GUI thread.
 * - The visualizer's GL lifecycle (initialize/render/resize/cleanup) runs
 *   ONLY on the render thread.
 * - UI access to the visualizer (setParam/getParam/gradients/taps) must hold
 *   the render mutex passed to the constructor; the thread holds it during
 *   render — never during swapBuffers (VSync wait does not block the UI).
 * - The QOpenGLContext must be created on the GUI thread and moved to this
 *   thread BEFORE start(); at the end of run() it is moved back so the
 *   facade may destroy it.
 * - BasicLogger is NOT thread-safe: the render thread never logs directly;
 *   diagnostics leave the thread via queued signals.
 ****************************************************************************************
 */

#pragma once

#include <QWindow>
#include <QThread>
#include <QAtomicInt>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QSize>

#include <memory>
#include <vector>

class QOpenGLContext;
class IVisualizer;
class VisualizerRenderThread;

/**
 * @brief Frame pacing of the render thread (mirrors Application FrameMode)
 */
enum class RenderPacing
{
    Limited,    ///< software sleep to target FPS (VSync off)
    Unlimited,  ///< free-running (VSync off)
    VSync       ///< swapBuffers blocks on display refresh (render thread only)
};

/**
 * @class VisualizerGLWindow
 * @brief Bare OpenGL QWindow embedded via createWindowContainer.
 *
 * Forwards expose/resize/surface events (GUI thread) to the render thread
 * and re-publishes double-clicks as a signal for the facade.
 */
class VisualizerGLWindow : public QWindow
{
    Q_OBJECT

public:
    VisualizerGLWindow();

    /// Attach/detach the render thread (nullptr = events are ignored)
    void attachThread(VisualizerRenderThread* thread) { m_thread = thread; }

Q_SIGNALS:
    /// Left double-click inside the GL area (facade toggles fullscreen)
    void doubleClicked();

    /// Escape pressed inside the GL area (facade exits fullscreen)
    void escapePressed();

protected:
    void exposeEvent(QExposeEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    VisualizerRenderThread* m_thread = nullptr;
};

/**
 * @class VisualizerRenderThread
 * @brief Render loop: waits for an exposed surface, applies queued state
 *        changes, takes the latest audio snapshot, renders and swaps.
 */
class VisualizerRenderThread : public QThread
{
    Q_OBJECT

public:
    /**
     * @param window      GL window (surface); lives on the GUI thread
     * @param context     GL context; must be moved to this thread before start()
     * @param renderMutex render mutex shared with UI accessors (facade-owned)
     */
    VisualizerRenderThread(VisualizerGLWindow& window,
                           QOpenGLContext& context,
                           QMutex& renderMutex,
                           QObject* parent = nullptr);
    ~VisualizerRenderThread() override;

    // =========================================================================
    // Commands (GUI thread)
    // =========================================================================

    /**
     * @brief Write the newest audio snapshot (main-thread audio tick)
     *
     * The thread applies the snapshot at the start of the next frame via
     * updateSpectrum/updateWaveform — visualizers are unchanged.
     */
    void updateAudio(const float* spectrum, int spectrumCount,
                     const float* waveform, int waveformCount);

    /// Hand over per-channel (stereo) audio (interleaved) to the render thread.
    void updateAudioStereo(const float* specInterleaved, int binsPerCh,
                           const float* waveInterleaved, int frames, int channels);

    /**
     * @brief Swap the rendered visualizer.
     *
     * @param next   visualizer to adopt (owned by the facade; GL-initialized
     *               on the render thread before its first frame); may be null
     * @param retire previous visualizer; GL-cleaned and DELETED on the
     *               render thread
     */
    void setVisualizer(IVisualizer* next, std::unique_ptr<IVisualizer> retire);

    /**
     * @brief Change frame pacing (swap interval is applied on the thread)
     */
    void setPacing(RenderPacing pacing, int targetFps);

    // =========================================================================
    // Surface synchronization (GUI thread, driven by VisualizerGLWindow)
    // =========================================================================

    void onExposeChanged(bool exposed, const QSize& size);
    void onResize(const QSize& size);

    /**
     * @brief SurfaceAboutToBeDestroyed: blocks until the thread released the
     *        surface via doneCurrent() (undock/reparent — Entwurf §4).
     */
    void releaseSurfaceBlocking();

    /**
     * @brief Stop the loop, GL-clean the current visualizer and join.
     *
     * The current (adopted) visualizer is cleaned up but NOT deleted — it is
     * owned by the facade. Pending retirees are deleted.
     */
    void stopAndWait();

    /**
     * @brief Naechstes fertig gerendertes Bild aufnehmen (Screenshot).
     *
     * Die Aufnahme MUSS auf dem Render-Thread passieren: `glReadPixels` braucht
     * den aktuellen Kontext, den nur dieser Thread haelt. Der Aufruf setzt
     * lediglich eine Marke; das Bild kommt als `frameCaptured` zurueck (queued
     * in den GUI-Thread). Mehrfaches Anfordern vor dem naechsten Frame ergibt
     * EIN Bild — der Anwender drueckt schneller als 60 Hz nicht.
     */
    void requestCapture();

Q_SIGNALS:
    /// FPS measured on the render thread (~1 s interval, queued to GUI)
    void fpsMeasured(double fps);

    /// Aufgenommenes Bild, bereits aufrecht und ohne Alpha (siehe run()).
    void frameCaptured(const QImage& image);

protected:
    void run() override;

private:
    void applySwapInterval(int interval);

    VisualizerGLWindow& m_window;
    QOpenGLContext& m_context;
    QMutex& m_renderMutex;

    // Loop state (m_mutex)
    QMutex m_mutex;
    QWaitCondition m_cond;                 ///< wakes the render loop
    QWaitCondition m_surfaceReleasedCond;  ///< signals: surface released
    bool m_exit = false;
    bool m_exposed = false;
    bool m_releaseSurface = false;
    bool m_surfaceReleased = false;
    bool m_hasResize = false;
    QSize m_size;

    // Visualizer swap command (m_mutex)
    bool m_hasAdopt = false;
    IVisualizer* m_pendingAdopt = nullptr;
    std::unique_ptr<IVisualizer> m_pendingRetire;

    // Pacing command (m_mutex)
    RenderPacing m_pacing = RenderPacing::Limited;
    int m_targetFps = 60;
    bool m_pacingDirty = true;

    // Audio snapshot (m_audioMutex) — writer: main thread, reader: render
    QMutex m_audioMutex;
    std::vector<float> m_spectrum;
    std::vector<float> m_waveform;
    bool m_audioDirty = false;
    // Per-channel (stereo) interleaved audio + dims (getspec/getosc channels).
    std::vector<float> m_specI, m_waveI;
    int m_stereoBins = 0, m_stereoFrames = 0, m_stereoChannels = 1;
    bool m_stereoDirty = false;

    /// Aufnahme-Anforderung (GUI-Thread setzt, Render-Thread loescht)
    QAtomicInt m_captureRequested{0};

    // Render-thread-only state
    IVisualizer* m_current = nullptr;  ///< non-owning (facade owns)
};
