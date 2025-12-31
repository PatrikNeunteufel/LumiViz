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
#include "UI/managers/DockManager.hpp"
#include "UI/managers/MenuManager.hpp"
#include "UI/managers/MenuInit.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/EventBus.hpp"
#include "services/MenuRegistry.hpp"
#include "services/events/UIEvents.hpp"

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
    
    // Create ServiceContainer
    m_pServices = std::make_unique<ServiceContainer>();
    
    // Register EventBus service
    m_pServices->registerSingleton<IEventBus, EventBus>();
    
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

    m_pDockManager = std::make_unique<DockManager>(*m_pServices, this);

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

    // -------------------------------------------------------------------------
    // Subscribe to Events
    // -------------------------------------------------------------------------
    
    setupEventHandlers();

    BasicLogger::logDebug("  UI setup complete");
}

void MainWindow::setupMenuBar()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Menu Bar Setup with Self-Registration
    // -------------------------------------------------------------------------
    // Das Menü wird jetzt über das MenuRegistry-System aufgebaut.
    // Die Basis-Container sind in MenuAutoReg.cpp definiert.
    // Feature-spezifische Items werden dezentral in den jeweiligen
    // Komponenten-Dateien registriert.
    //
    // MenuManager baut die QMenuBar aus dem Registry auf und
    // integriert DockManager für Panels/Perspectives.
    //
    // WICHTIG: initMenuRegistrations() muss VOR buildMenuBar() aufgerufen werden,
    // um sicherzustellen, dass die statischen Registrierungen vom Linker
    // nicht entfernt wurden (Dead Code Elimination).

    // Ensure menu registrations are linked (prevents linker optimization)
    initMenuRegistrations();

    // Create MenuManager
    m_pMenuManager = std::make_unique<MenuManager>(*m_pServices, this);
    
    // Build menu bar from registry
    m_pMenuManager->buildMenuBar(menuBar());
    
    // Integrate DockManager for Panels and Perspectives
    if (m_pDockManager)
    {
        // Get the Panels submenu and populate with dock widgets
        QMenu* pPanelsMenu = m_pMenuManager->menu("menu.view.panels");
        if (pPanelsMenu)
        {
            m_pDockManager->populatePanelsMenu(pPanelsMenu);
        }
        
        // Get the Perspectives submenu and populate
        QMenu* pPerspectivesMenu = m_pMenuManager->menu("menu.view.perspectives");
        if (pPerspectivesMenu)
        {
            m_pDockManager->populatePerspectivesMenu(pPerspectivesMenu);
        }
    }
    
    BasicLogger::logDebug("  Menu bar created via MenuManager");
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

void MainWindow::setupEventHandlers()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Event-Driven Menu Actions
    // -------------------------------------------------------------------------
    // Menu items trigger Events via EventBus. MainWindow subscribes to these
    // events and executes the actual actions. This decouples menu definitions
    // from the implementation.

    auto* pEventBus = m_pServices->tryResolve<IEventBus>();
    if (pEventBus == nullptr)
    {
        BasicLogger::logWarning("EventBus not available - menu events won't work");
        return;
    }

    // Create Visualizer event
    pEventBus->subscribe<CreateVisualizerEvent>([this](const CreateVisualizerEvent& e) {
        QString title = e.title.empty() 
            ? tr("Visualizer %1").arg(m_pDockManager->dockWidgetCount() + 1)
            : QString::fromStdString(e.title);
        m_pDockManager->createVisualizer(title, DockPosition::Center);
    });

    // Reset Layout event
    pEventBus->subscribe<ResetLayoutEvent>([this](const ResetLayoutEvent& /*e*/) {
        m_pDockManager->resetLayout();
    });

    // Frame Mode Changed event
    pEventBus->subscribe<FrameModeChangedEvent>([this](const FrameModeChangedEvent& e) {
        BasicLogger::logInfo("Frame mode changed to: " + std::to_string(e.mode));
        emit frameModeChangeRequested(e.mode);
    });

    // Open Dialog event
    pEventBus->subscribe<OpenDialogEvent>([this](const OpenDialogEvent& e) {
        if (e.dialogId == "about")
        {
            // TODO: Open About dialog via DialogManager
            BasicLogger::logDebug("About dialog requested");
        }
    });

    BasicLogger::logDebug("  Event handlers registered");
}
