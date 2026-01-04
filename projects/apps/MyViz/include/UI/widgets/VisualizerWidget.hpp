/**
 ****************************************************************************************
 * @file   VisualizerWidget.hpp
 * @brief  OpenGL Visualization Widget - Qt6 Tutorial
 *         Hardware-accelerated rendering with VSync support
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.2.0 - Added Config Event Support
 *
 * @details
 * ## Version 2.2.0 Changes
 *
 * - Added subscribeToConfigEvents() for receiving ConfigPanel events
 * - Color scheme, smoothing, peak hold can now be changed at runtime
 *
 * ## Version 2.1.0 Changes
 *
 * - Now inherits from WidgetBase<QOpenGLWidget> for consistency
 * - ServiceContainer and EventBus access via base class
 * - Auto start/stop updates on show/hide
 *
 * ## Visualizer Architecture
 *
 * ```
 * WidgetBase<QOpenGLWidget>
 *       │
 *       └── VisualizerWidget
 *             │
 *             ├── OpenGL Context Management
 *             ├── VSync Control
 *             └── Active Visualizer (IVisualizer*)
 *                   │
 *                   ├── initialize()
 *                   ├── render(deltaTime)
 *                   ├── resize(size)
 *                   └── cleanup()
 * ```
 *
 * @see VisualizerWidget.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "WidgetBase.hpp"

#include <QString>
#include <QElapsedTimer>
#include <memory>
#include <vector>

// Forward declarations
class IVisualizer;

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
class QMouseEvent;
QT_END_NAMESPACE

/**
 * @brief Type alias for OpenGL-based widget base
 */
using OpenGLWidgetBase = WidgetBase<QOpenGLWidget>;

/**
 * @class VisualizerWidget
 * @brief OpenGL widget for audio visualization rendering.
 *
 * VisualizerWidget provides a hardware-accelerated canvas for rendering
 * audio visualizations. It manages an active IVisualizer instance and
 * delegates rendering to it.
 *
 * ## Inheritance
 *
 * - WidgetBase<QOpenGLWidget>: ServiceContainer, EventBus, auto start/stop
 * - QOpenGLFunctions: OpenGL function pointers
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
 */
class VisualizerWidget : public OpenGLWidgetBase, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Constructs the VisualizerWidget.
     * @param services ServiceContainer for dependency injection
     * @param parent Parent widget (typically MainWindow's central widget)
     */
    explicit VisualizerWidget(ServiceContainer& services, QWidget* parent = nullptr);

    /**
     * @brief Destructor - cleans up active visualizer.
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
     * The visualizer is loaded from VisualizerRegistry and initialized.
     * If the ID is not found, the current visualizer remains active.
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
     */
    [[nodiscard]] IVisualizer* visualizer() const { return m_visualizer.get(); }

    // =========================================================================
    // Public Interface
    // =========================================================================

    /**
     * @brief Sets the clear color (background).
     * @param r Red component (0.0 - 1.0)
     * @param g Green component (0.0 - 1.0)
     * @param b Blue component (0.0 - 1.0)
     * @param a Alpha component (0.0 - 1.0), default 1.0
     */
    void setClearColor(float r, float g, float b, float a = 1.0f);

    /**
     * @brief Enables or disables VSync at runtime.
     * @param enabled true = VSync ON, false = VSync OFF
     */
    void setVSync(bool enabled);

    // =========================================================================
    // Audio Data (pass-through to visualizer)
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

protected:
    // =========================================================================
    // WidgetBase Overrides
    // =========================================================================

    /**
     * @brief Called when updates start (widget shown)
     */
    void onStartUpdates() override;

    /**
     * @brief Called when updates stop (widget hidden)
     */
    void onStopUpdates() override;

    // =========================================================================
    // QOpenGLWidget Virtual Methods
    // =========================================================================

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    /**
     * @brief Handle double-click to toggle fullscreen
     */
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    // =========================================================================
    // Private Methods
    // =========================================================================

    /**
     * @brief Load default visualizer (pulsing)
     */
    void loadDefaultVisualizer();

    /**
     * @brief Cleanup current visualizer
     */
    void cleanupVisualizer();

    /**
     * @brief Subscribe to config events from ConfigPanel
     */
    void subscribeToConfigEvents();

    /**
     * @brief Unsubscribe from config events
     */
    void unsubscribeFromConfigEvents();

    // =========================================================================
    // Private Members
    // =========================================================================

    // Active visualizer
    std::unique_ptr<IVisualizer> m_visualizer;
    QString m_currentVisualizerId;

    // Clear color (fallback when no visualizer)
    float m_clearR{0.1f};
    float m_clearG{0.1f};
    float m_clearB{0.15f};
    float m_clearA{1.0f};

    // Timing
    QElapsedTimer m_frameTimer;
    float m_lastFrameTime{0.0f};

    // Frame counter (for debugging)
    uint64_t m_frameCount{0};

    // OpenGL initialized flag
    bool m_glInitialized{false};

    // Config event subscription IDs
    std::vector<int> m_configSubscriptionIds;
};
