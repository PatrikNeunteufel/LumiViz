/**
 ****************************************************************************************
 * @file   MainWindow.cpp
 * @brief  Main Application Window Implementation - Qt6 Tutorial
 *         Uses Qt-ADS for dockable visualizer panels
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
#include "UI/DockManager.hpp"
#include "UI/widgets/VisualizerWidget.hpp"

// Qt
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QStatusBar>
#include <QLabel>
#include <QKeySequence>

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
    // Qt6 Tutorial: Destructor with Qt-ADS
    // -------------------------------------------------------------------------
    // CRITICAL: DockManager must be destroyed BEFORE QMainWindow base destructor!
    //
    // Qt-ADS internally uses QPointer to track floating containers.
    // If we don't explicitly destroy it first, Qt's parent-child cleanup
    // can delete widgets in wrong order → dangling pointers → crash.
    //
    // Solution: Reset unique_ptr explicitly here, before base destructor runs.

    BasicLogger::logDebug("MainWindow destructor - cleaning up DockManager");
    
    if (m_pDockManager)
    {
        // Close all dock widgets first to avoid dangling references
        m_pDockManager->closeAll();
        m_pDockManager.reset();
    }
    
    BasicLogger::logDebug("MainWindow destructor complete");
}

// =============================================================================
// Dock Manager Access
// =============================================================================

DockManager* MainWindow::dockManager() const noexcept
{
    return m_pDockManager.get();
}

// =============================================================================
// Visualizer Access
// =============================================================================

std::vector<VisualizerWidget*> MainWindow::visualizers() const
{
    if (m_pDockManager)
    {
        return m_pDockManager->visualizers();
    }
    return {};
}

VisualizerWidget* MainWindow::primaryVisualizer() const
{
    auto vizList = visualizers();
    return vizList.empty() ? nullptr : vizList.front();
}

// =============================================================================
// Render Control
// =============================================================================

void MainWindow::requestRender()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Rendering All Visualizers
    // -------------------------------------------------------------------------
    // With multiple visualizers, we need to update all of them.
    // Each VisualizerWidget has its own OpenGL context.
    //
    // DockManager::requestRenderAll() calls update() on each visualizer.

    if (m_pDockManager)
    {
        m_pDockManager->requestRenderAll();
    }
}

// =============================================================================
// Public Slots
// =============================================================================

void MainWindow::updateFpsDisplay(double fps)
{
    if (m_pFpsLabel != nullptr)
    {
        m_pFpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
    }
}

void MainWindow::onNewVisualizer()
{
    if (m_pDockManager)
    {
        // Generate unique name
        static int counter = 1;
        QString title = QString("Visualizer %1").arg(counter++);
        
        // Create in center (creates tab)
        m_pDockManager->createVisualizer(title, DockPosition::Center);
        
        BasicLogger::logInfo("Created new visualizer: " + title.toStdString());
    }
}

void MainWindow::setVSyncOnAllVisualizers(bool enabled)
{
    auto vizList = visualizers();
    for (auto* pViz : vizList)
    {
        if (pViz != nullptr)
        {
            pViz->setVSync(enabled);
        }
    }
    BasicLogger::logDebug("VSync " + std::string(enabled ? "enabled" : "disabled") + 
                          " on " + std::to_string(vizList.size()) + " visualizer(s)");
}

// =============================================================================
// Private Slots
// =============================================================================

void MainWindow::onVisualizerCreated(VisualizerWidget* pVisualizer)
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Visualizer Created Callback
    // -------------------------------------------------------------------------
    // This slot is called whenever a new visualizer is created.
    // Use it to:
    //   - Connect signals/slots
    //   - Set up audio data routing
    //   - Configure visualizer settings

    if (pVisualizer != nullptr)
    {
        BasicLogger::logDebug("New visualizer connected");
        
        // Future: Connect audio data
        // connect(m_pAudioEngine, &AudioEngine::fftDataReady,
        //         pVisualizer, &VisualizerWidget::setAudioData);
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
    // Create Dock Manager
    // -------------------------------------------------------------------------
    // DockManager creates the Qt-ADS CDockManager internally.
    // It takes over the central area of MainWindow.

    m_pDockManager = std::make_unique<DockManager>(this);

    // Connect signals
    connect(m_pDockManager.get(), &DockManager::visualizerCreated,
            this, &MainWindow::onVisualizerCreated);

    BasicLogger::logDebug("  DockManager created");

    // -------------------------------------------------------------------------
    // Setup UI Components
    // -------------------------------------------------------------------------

    setupMenuBar();
    setupStatusBar();
    setupDefaultLayout();

    BasicLogger::logDebug("  UI setup complete");
}

void MainWindow::setupMenuBar()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Menu Bar Setup
    // -------------------------------------------------------------------------
    // QMainWindow::menuBar() creates a menu bar if none exists.
    // Menus are created via addMenu().
    //
    // Qt 6.4+ requires: addAction(text, shortcut, object, slot)
    // NOT the old: addAction(text, object, slot, shortcut)

    QMenuBar* pMenuBar = menuBar();

    // -------------------------------------------------------------------------
    // File Menu
    // -------------------------------------------------------------------------

    QMenu* pFileMenu = pMenuBar->addMenu(tr("&File"));

    auto* pOpenAction = pFileMenu->addAction(tr("&Open Audio..."));
    pOpenAction->setShortcut(QKeySequence::Open);
    connect(pOpenAction, &QAction::triggered, this, []() {
        // TODO: Open file dialog for audio file
        BasicLogger::logDebug("Open Audio clicked");
    });

    pFileMenu->addSeparator();

    auto* pExitAction = pFileMenu->addAction(tr("E&xit"));
    pExitAction->setShortcut(QKeySequence::Quit);
    connect(pExitAction, &QAction::triggered, this, &QWidget::close);

    // -------------------------------------------------------------------------
    // View Menu (from DockManager)
    // -------------------------------------------------------------------------

    QMenu* pViewMenu = m_pDockManager->createViewMenu(this);
    pMenuBar->addMenu(pViewMenu);

    // Add "New Visualizer" action to View menu
    auto* pNewVizAction = new QAction(tr("&New Visualizer"), this);
    pNewVizAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(pNewVizAction, &QAction::triggered, this, &MainWindow::onNewVisualizer);
    pViewMenu->insertAction(pViewMenu->actions().isEmpty() ? nullptr : pViewMenu->actions().first(), 
                            pNewVizAction);
    if (!pViewMenu->actions().isEmpty() && pViewMenu->actions().size() > 1)
    {
        pViewMenu->insertSeparator(pViewMenu->actions().at(1));
    }

    // -------------------------------------------------------------------------
    // Settings Menu
    // -------------------------------------------------------------------------

    QMenu* pSettingsMenu = pMenuBar->addMenu(tr("&Settings"));

    // Frame Mode submenu
    QMenu* pFrameModeMenu = pSettingsMenu->addMenu(tr("&Frame Mode"));
    
    auto* pLimitedAction = pFrameModeMenu->addAction(tr("&Limited (60 FPS)"));
    pLimitedAction->setCheckable(true);
    pLimitedAction->setChecked(true);  // Default
    
    auto* pUnlimitedAction = pFrameModeMenu->addAction(tr("&Unlimited"));
    pUnlimitedAction->setCheckable(true);
    
    auto* pVSyncAction = pFrameModeMenu->addAction(tr("&VSync"));
    pVSyncAction->setCheckable(true);

    // Make them exclusive
    auto* pFrameModeGroup = new QActionGroup(this);
    pFrameModeGroup->addAction(pLimitedAction);
    pFrameModeGroup->addAction(pUnlimitedAction);
    pFrameModeGroup->addAction(pVSyncAction);
    
    // Connect actions to signal
    connect(pLimitedAction, &QAction::triggered, this, [this]() {
        BasicLogger::logInfo("Frame mode changed to: Limited");
        emit frameModeChangeRequested(0);  // 0 = Limited
    });
    
    connect(pUnlimitedAction, &QAction::triggered, this, [this]() {
        BasicLogger::logInfo("Frame mode changed to: Unlimited");
        emit frameModeChangeRequested(1);  // 1 = Unlimited
    });
    
    connect(pVSyncAction, &QAction::triggered, this, [this]() {
        BasicLogger::logInfo("Frame mode changed to: VSync");
        emit frameModeChangeRequested(2);  // 2 = VSync
    });

    // -------------------------------------------------------------------------
    // Help Menu
    // -------------------------------------------------------------------------

    QMenu* pHelpMenu = pMenuBar->addMenu(tr("&Help"));

    pHelpMenu->addAction(tr("&About MyViz"), this, []() {
        BasicLogger::logDebug("About clicked");
    });

    BasicLogger::logDebug("  Menu bar created");
}

void MainWindow::setupStatusBar()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Status Bar Setup
    // -------------------------------------------------------------------------
    // QMainWindow::statusBar() creates a status bar if none exists.
    // Use addPermanentWidget() for persistent displays (like FPS).

    QStatusBar* pStatusBar = statusBar();

    // FPS display (permanent, right-aligned)
    m_pFpsLabel = new QLabel("FPS: --");
    m_pFpsLabel->setMinimumWidth(80);
    pStatusBar->addPermanentWidget(m_pFpsLabel);

    // Initial message
    pStatusBar->showMessage(tr("Ready"));

    BasicLogger::logDebug("  Status bar created");
}

void MainWindow::setupDefaultLayout()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Default Docking Layout
    // -------------------------------------------------------------------------
    // Create the initial visualizer panels.
    // Users can rearrange these via drag-and-drop.
    //
    // Default layout:
    // +----------------------------------+
    // | Spectrum (Center)                |
    // +----------------------------------+
    //
    // Single visualizer for now. User can add more via menu.

    auto* pSpectrum = m_pDockManager->createVisualizer(
        tr("Spectrum Analyzer"), DockPosition::Center);

    if (pSpectrum != nullptr)
    {
        BasicLogger::logDebug("  Default visualizer created");
    }

    // Save this as the default perspective
    m_pDockManager->savePerspective("Default");
}
