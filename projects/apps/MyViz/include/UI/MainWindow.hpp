/**
 ****************************************************************************************
 * @file   MainWindow.hpp
 * @brief  Main Application Window - Qt6 Tutorial
 *         Contains VisualizerWidget as central widget
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This file is part of the Qt6 Tutorial series for MyViz.
 * It demonstrates:
 *   - QMainWindow inheritance
 *   - Embedding QOpenGLWidget as central widget
 *   - Q_OBJECT macro usage
 *
 * @see MainWindow.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Qt Includes
// =============================================================================

#include <QMainWindow>

// =============================================================================
// Forward Declarations
// =============================================================================
// Forward declarations avoid including headers in .hpp files.
// This reduces compilation time and circular dependencies.

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class VisualizerWidget;  // Our OpenGL visualization widget

/**
 * @class MainWindow
 * @brief Main application window for MyViz.
 *
 * MainWindow is the primary window of the application. It contains the
 * VisualizerWidget (QOpenGLWidget) as its central widget.
 *
 * ## Qt6 Tutorial: Window Structure
 *
 * ```
 * +------------------------------------------+
 * |              Menu Bar (TODO)             |
 * +------------------------------------------+
 * |                                          |
 * |        VisualizerWidget (OpenGL)         |
 * |                                          |
 * |       ┌────────────────────────┐         |
 * |       │  Audio Visualization   │         |
 * |       │  (Spectrum, Waveform)  │         |
 * |       └────────────────────────┘         |
 * |                                          |
 * +------------------------------------------+
 * |              Status Bar (TODO)           |
 * +------------------------------------------+
 * ```
 *
 * The VisualizerWidget handles all OpenGL rendering with VSync support.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Constructs the MainWindow.
     *
     * @param parent Optional parent widget (nullptr for top-level window).
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MainWindow() override;

    // Non-copyable
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

    // =========================================================================
    // Public Interface
    // =========================================================================

    /**
     * @brief Gets the visualizer widget.
     *
     * Use this to:
     *   - Trigger repaints (update())
     *   - Pass audio data for visualization
     *   - Configure rendering options
     *
     * @return Pointer to the VisualizerWidget (never null after construction)
     */
    [[nodiscard]] VisualizerWidget* visualizer() const noexcept;

    /**
     * @brief Requests a repaint of the visualizer.
     *
     * Call this from the main loop to trigger rendering.
     * With VSync enabled, this will synchronize with the monitor refresh.
     *
     * ## Qt6 Tutorial: update() vs repaint()
     *
     * - update()  : Schedules a paint event (non-blocking, coalesced)
     * - repaint() : Immediate repaint (blocking, not recommended)
     *
     * Always prefer update() for smooth rendering.
     */
    void requestRender();

private:
    // =========================================================================
    // Private Methods
    // =========================================================================

    /**
     * @brief Sets up the user interface.
     *
     * Creates and configures:
     *   - VisualizerWidget as central widget
     *   - Window title and size
     *   - (Future: menu bar, status bar)
     */
    void setupUi();

    // =========================================================================
    // Private Members
    // =========================================================================

    VisualizerWidget* m_pVisualizer{nullptr};  // Owned by Qt parent-child system
};
