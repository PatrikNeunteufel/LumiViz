/**
 ****************************************************************************************
 * @file   VisualizerWidget.hpp
 * @brief  Facade widget for visualizer rendering on a dedicated render thread
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 3.0.0 - Render thread decoupling (Render_Thread_Entwurf.md)
 *
 * @details
 * ## Version 3.0.0 Changes
 *
 * VisualizerWidget is no longer a QOpenGLWidget: GL rendering moved off the
 * main thread. The widget is now a thin FACADE around
 *
 * ```
 * WidgetBase<QWidget>
 *       │
 *       └── VisualizerWidget (facade — public API unchanged)
 *             ├── createWindowContainer(…)
 *             ├── VisualizerGLWindow : QWindow   (embedded native window)
 *             ├── QOpenGLContext                 (owned, on the render thread)
 *             └── VisualizerRenderThread         (render loop + pacing + FPS)
 * ```
 *
 * - The visualizer's GL lifecycle runs ONLY on the render thread; undocking
 *   no longer recreates the context (the context is ours) — the visualizers'
 *   context-tracking pattern remains as a safety net.
 * - UI access to the active visualizer (setParam/getParam/gradients/
 *   tapPoints/audioSourceModule/presets) MUST hold renderMutex();
 *   VisualizerChangedEvent carries the mutex to subscribers.
 * - Audio data is handed over via snapshot buffer (updateSpectrum/
 *   updateWaveform write; the thread applies at frame start).
 * - Frame pacing (Limited/Unlimited/VSync) and the FPS measurement live on
 *   the render thread; FPS arrives via the fpsMeasured() signal.
 *
 * @see VisualizerRenderThread.hpp and docs/visuals/Render_Thread_Entwurf.md
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include "WidgetBase.hpp"
#include "UI/widgets/VisualizerRenderThread.hpp"  // RenderPacing, window, thread

#include <QString>
#include <QMutex>
#include <memory>

// Forward declarations
class IVisualizer;
class QOpenGLContext;

/**
 * @class VisualizerWidget
 * @brief Facade widget hosting a visualizer rendered on a dedicated thread.
 *
 * ## Visualizer Management
 *
 * ```cpp
 * // Set visualizer by ID (from VisualizerRegistry)
 * widget->setVisualizer("spectrum");
 *
 * // Get current visualizer ID
 * QString current = widget->visualizerId();
 *
 * // Connect to change signal
 * connect(widget, &VisualizerWidget::visualizerChanged,
 *         this, &MyClass::onVisualizerChanged);
 * ```
 *
 * ## Thread safety
 *
 * Any call into visualizer() beyond immutable identity data must hold
 * renderMutex() — see class details above.
 */
class VisualizerWidget : public StandardWidgetBase
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Constructs the VisualizerWidget.
     * @param services ServiceContainer for dependency injection
     * @param parent Parent widget (typically a dock content area)
     */
    explicit VisualizerWidget(ServiceContainer& services, QWidget* parent = nullptr);

    /**
     * @brief Destructor - stops the render thread and cleans up.
     */
    ~VisualizerWidget() override;

    // Non-copyable
    VisualizerWidget(const VisualizerWidget&) = delete;
    VisualizerWidget& operator=(const VisualizerWidget&) = delete;
    VisualizerWidget(VisualizerWidget&&) = delete;
    VisualizerWidget& operator=(VisualizerWidget&&) = delete;

    // =========================================================================
    // Visualizer Management
    // =========================================================================

    /**
     * @brief Set active visualizer by ID
     * @param id Visualizer ID from VisualizerRegistry (e.g., "pulsing", "spectrum")
     * @return true if visualizer was loaded successfully
     *
     * The visualizer is created from VisualizerRegistry on the GUI thread;
     * its GL initialization happens on the render thread before the first
     * frame. If the ID is not found, the current visualizer remains active.
     */
    bool setVisualizer(const QString& id);

    /**
     * @brief Get current visualizer ID
     * @return Active visualizer ID or empty string if none
     */
    [[nodiscard]] QString currentVisualizerId() const { return m_currentVisualizerId; }

    /**
     * @brief Get current visualizer name
     * @return Active visualizer display name or empty string
     */
    [[nodiscard]] QString currentVisualizerName() const;

    /**
     * @brief Check if a visualizer is active
     */
    [[nodiscard]] bool hasVisualizer() const { return m_visualizer != nullptr; }

    /**
     * @brief Get the active visualizer
     * @return Pointer to active IVisualizer, or nullptr if none
     *
     * @warning Parameter/gradient/tap access on the returned pointer must
     *          hold renderMutex() (the render thread renders concurrently).
     */
    [[nodiscard]] IVisualizer* visualizer() const { return m_visualizer.get(); }

    /**
     * @brief Render mutex guarding UI access against the render thread
     */
    [[nodiscard]] QMutex& renderMutex() { return m_renderMutex; }

    // =========================================================================
    // Frame Pacing
    // =========================================================================

    /**
     * @brief Set frame pacing of the render thread
     * @param pacing Limited (software target FPS) / Unlimited / VSync
     * @param targetFps Target FPS for Limited mode
     */
    void setFrameMode(RenderPacing pacing, int targetFps);

    /**
     * @brief Give keyboard focus to the embedded GL window (fullscreen: Esc)
     */
    void activateGLWindow();

    /**
     * @brief Drop and recreate the native window handles.
     *
     * After reparenting FROM top-level (fullscreen exit) Qt leaves the
     * facade's native window at a stale absolute position (visible as a
     * displaced strip). Recreating the handles rebuilds the native parent
     * chain; for the render thread this is the same surface-destroy/expose
     * cycle as undocking.
     */
    void recreateNativeWindow();

    // =========================================================================
    // Audio Data (snapshot hand-over to the render thread)
    // =========================================================================

    /**
     * @brief Update visualizer with spectrum data
     * @param spectrum Frequency spectrum (0.0 - 1.0)
     * @param count Number of spectrum bands
     */
    void updateSpectrum(const float* spectrum, int count);

    /**
     * @brief Update visualizer with waveform data
     * @param waveform Waveform samples (-1.0 to 1.0)
     * @param count Number of samples
     */
    void updateWaveform(const float* waveform, int count);

    /// Feed per-channel (stereo) audio (interleaved) through to the render thread.
    void updateAudioStereo(const float* specInterleaved, int binsPerCh,
                           const float* waveInterleaved, int frames, int channels);

Q_SIGNALS:
    /**
     * @brief Emitted when the active visualizer changes
     * @param id New visualizer ID
     */
    void visualizerChanged(const QString& id);

    /**
     * @brief Emitted when visualizer loading fails
     * @param id Attempted visualizer ID
     * @param error Error message
     */
    void visualizerError(const QString& id, const QString& error);

    /**
     * @brief FPS measured on the render thread (~1 s interval)
     */
    void fpsMeasured(double fps);

protected:
    // =========================================================================
    // WidgetBase Overrides
    // =========================================================================

    /**
     * @brief Called when updates start (widget shown)
     *
     * The render thread pauses/resumes on expose events by itself — nothing
     * to do here.
     */
    void onStartUpdates() override;

    /**
     * @brief Called when updates stop (widget hidden)
     */
    void onStopUpdates() override;

    /**
     * @brief Esc fallback when focus sits on the container instead of the
     *        embedded GL window (fullscreen exit)
     */
    void keyPressEvent(QKeyEvent* event) override;

private:
    // =========================================================================
    // Private Methods
    // =========================================================================

    /**
     * @brief Load default visualizer (pulsing)
     */
    void loadDefaultVisualizer();

    // =========================================================================
    // Private Members
    // =========================================================================

    // Render infrastructure (Entwurf §1)
    VisualizerGLWindow* m_glWindow = nullptr;  ///< owned by the container widget
    std::unique_ptr<QOpenGLContext> m_context;
    std::unique_ptr<VisualizerRenderThread> m_thread;
    QMutex m_renderMutex;

    // Active visualizer (owned here; GL lifecycle on the render thread)
    std::unique_ptr<IVisualizer> m_visualizer;
    QString m_currentVisualizerId;
};
