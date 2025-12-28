/**
 ****************************************************************************************
 * @file   MainWindow.cpp
 * @brief  Main Application Window Implementation - Qt6 Tutorial
 *         Demonstrates basic QMainWindow setup without menu
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

 // =============================================================================
 // Includes
 // =============================================================================

 // Precompiled Header (must be first if PCH is enabled)
#include "pch.h"

// Corresponding header (always include your own header first after PCH)
#include "UI/MainWindow.hpp"

// Qt includes (already in PCH, but shown for tutorial clarity)
#include <QWidget>

// =============================================================================
// Construction / Destruction
// =============================================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)  // Initialize base class with parent
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Constructor Initialization
    // -------------------------------------------------------------------------
    // In Qt, the constructor typically:
    // 1. Initializes the base class with the parent
    // 2. Calls a setup method for UI configuration
    //
    // We separate UI setup from construction because:
    // - It keeps the constructor clean
    // - It allows for potential re-initialization
    // - It makes the code easier to read and maintain

    setupUi();
}

MainWindow::~MainWindow()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Destructor
    // -------------------------------------------------------------------------
    // Qt's parent-child system handles memory management:
    // - All child widgets are automatically deleted when parent is deleted
    // - We don't need to manually delete widgets created with 'this' as parent
    //
    // The destructor is defaulted here (= default would also work).
    // We leave it empty to show where cleanup code would go if needed.

    // Note: If we had non-Qt resources (file handles, custom allocations),
    // we would clean them up here.
}

// =============================================================================
// Private Methods
// =============================================================================

void MainWindow::setupUi()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Window Configuration
    // -------------------------------------------------------------------------

    // --- Window Title ---
    // setWindowTitle() sets the text shown in the title bar.
    // This is the first thing users see, so make it descriptive.
    setWindowTitle(QStringLiteral("MyViz - Audio Visualizer"));

    // --- Window Size ---
    // resize() sets the initial size of the window in pixels.
    // Users can still resize the window unless we set fixed size.
    //
    // Common resolutions:
    //   - 1280x720  (HD)
    //   - 1920x1080 (Full HD)
    //   - 800x600   (Small window)
    resize(1280, 720);

    // --- Minimum Size (Optional) ---
    // setMinimumSize() prevents the window from being resized too small.
    // This ensures UI elements remain visible and usable.
    setMinimumSize(800, 600);

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Central Widget
    // -------------------------------------------------------------------------
    // QMainWindow REQUIRES a central widget. The central widget is the main
    // content area of the window (see diagram in MainWindow.hpp).
    //
    // Here we create an empty QWidget as placeholder. Later, this will be
    // replaced with our visualization widget.
    //
    // IMPORTANT: The 'new' without 'delete' is intentional!
    // Qt's parent-child system takes ownership. When MainWindow is deleted,
    // it automatically deletes all its children, including this widget.

    auto* pCentralWidget = new QWidget(this);  // 'this' = MainWindow is parent
    setCentralWidget(pCentralWidget);

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: No Menu Bar
    // -------------------------------------------------------------------------
    // By default, QMainWindow has no menu bar until you call menuBar().
    // We explicitly set it to nullptr to ensure no menu bar appears.
    // This is for clarity - in a real app, you'd likely have a menu.
    //
    // To add a menu later:
    //   QMenuBar* pMenuBar = menuBar();  // Creates if doesn't exist
    //   QMenu* pFileMenu = pMenuBar->addMenu("&File");
    //   pFileMenu->addAction("&Exit", this, &QWidget::close);

    // Note: setMenuBar(nullptr) is optional here since we're not creating one.
    // Shown for tutorial purposes to make the intention explicit.

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Background Color (Optional)
    // -------------------------------------------------------------------------
    // We can set a background color using stylesheets.
    // This makes it clear the window is working, even with no content.
    //
    // Qt Stylesheets use CSS-like syntax:
    //   background-color: #1a1a2e;  <- Dark blue-gray (good for visualizers)

    pCentralWidget->setStyleSheet(
        QStringLiteral("background-color: #1a1a2e;"));
}