/**
 ****************************************************************************************
 * @file   MainWindow.cpp
 * @brief  Main Application Window Implementation - Qt6 Tutorial
 *         Contains VisualizerWidget as central widget
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// =============================================================================
// Includes
// =============================================================================

#include "pch.h"
#include "UI/MainWindow.hpp"
#include "UI/widget/VisualizerWidget.hpp"

// BasicLogger
#include <BasicLogger.h>

// =============================================================================
// Construction / Destruction
// =============================================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    BasicLogger::logDebug("MainWindow constructor");
    setupUi();
}

MainWindow::~MainWindow()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Destructor
    // -------------------------------------------------------------------------
    // Qt's parent-child system handles memory management.
    // VisualizerWidget is owned by MainWindow and will be deleted automatically.
    //
    // We don't need to delete m_pVisualizer - Qt does it for us!

    BasicLogger::logDebug("MainWindow destructor");
}

// =============================================================================
// Public Interface
// =============================================================================

VisualizerWidget* MainWindow::visualizer() const noexcept
{
    return m_pVisualizer;
}

void MainWindow::requestRender()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Triggering Repaints
    // -------------------------------------------------------------------------
    // update() schedules a paint event for the widget.
    //
    // Key points:
    //   - Non-blocking: Returns immediately
    //   - Coalesced: Multiple update() calls become one paint event
    //   - Efficient: Only repaints when event loop processes it
    //
    // For VSync rendering:
    // When the event loop processes the paint event, paintGL() is called,
    // and then swapBuffers() waits for the vertical blank.

    if (m_pVisualizer != nullptr)
    {
        m_pVisualizer->update();
    }
}

// =============================================================================
// Private Methods
// =============================================================================

void MainWindow::setupUi()
{
    BasicLogger::logDebug("MainWindow::setupUi()");

    // -------------------------------------------------------------------------
    // Window Configuration
    // -------------------------------------------------------------------------

    setWindowTitle(QStringLiteral("MyViz - Audio Visualizer"));
    resize(1280, 720);
    setMinimumSize(800, 600);

    BasicLogger::logDebug("  Window size: 1280x720 (min: 800x600)");

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: VisualizerWidget as Central Widget
    // -------------------------------------------------------------------------
    // Instead of an empty QWidget, we now use our OpenGL widget.
    //
    // The VisualizerWidget:
    //   - Provides hardware-accelerated rendering
    //   - Handles VSync automatically (if enabled in QSurfaceFormat)
    //   - Can receive audio data for visualization (TODO)
    //
    // 'this' makes MainWindow the parent, so:
    //   - Qt manages memory (auto-delete on MainWindow destruction)
    //   - VisualizerWidget inherits some properties
    //   - Proper widget hierarchy for event propagation

    m_pVisualizer = new VisualizerWidget(this);
    setCentralWidget(m_pVisualizer);

    BasicLogger::logDebug("  VisualizerWidget created and set as central widget");

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Focus Policy
    // -------------------------------------------------------------------------
    // Set focus policy so the visualizer can receive keyboard events.
    // StrongFocus: Can receive focus via Tab and mouse click.

    m_pVisualizer->setFocusPolicy(Qt::StrongFocus);

    // -------------------------------------------------------------------------
    // Future: Menu Bar
    // -------------------------------------------------------------------------
    // QMenuBar* pMenuBar = menuBar();
    // QMenu* pFileMenu = pMenuBar->addMenu("&File");
    // pFileMenu->addAction("&Open...", this, &MainWindow::onOpenFile);
    // pFileMenu->addSeparator();
    // pFileMenu->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);
    //
    // QMenu* pViewMenu = pMenuBar->addMenu("&View");
    // pViewMenu->addAction("&Fullscreen", this, &MainWindow::toggleFullscreen, Qt::Key_F11);

    // -------------------------------------------------------------------------
    // Future: Status Bar
    // -------------------------------------------------------------------------
    // statusBar()->showMessage("Ready");
    // // Later: Show FPS, audio info, etc.

    BasicLogger::logDebug("  UI setup complete");
}
