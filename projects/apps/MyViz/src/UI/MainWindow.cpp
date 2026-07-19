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
#include "UI/managers/DialogManager.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/EventBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/CommandBus.hpp"
#include "services/MenuRegistry.hpp"
#include "services/events/UIEvents.hpp"

// Audio Services
#include "audio/IAudioEngine.hpp"
#include "audio/BassEngine.hpp"
#include "audio/IAudioPlayer.hpp"
#include "audio/AudioPlayer.hpp"
#include "audio/IPlaylist.hpp"
#include "audio/Playlist.hpp"

// Qt
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QStatusBar>
#include <QLabel>
#include <QKeySequence>
#include <QKeyEvent>
#include <QTimer>

// Qt-ADS (needed for fullscreen dock state save/restore)
#include <DockManager.h>
#include <DockWidget.h>

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

    // Register CommandBus (undo/redo) — publishes history events on the EventBus
    m_pServices->registerSingleton<ICommandBus>([](ServiceContainer& c) {
        return std::make_unique<CommandBus>(c.tryResolve<IEventBus>());
    });

    // Setup Audio Services
    setupAudioServices();
    
    setupUi();
    
    // -------------------------------------------------------------------------
    // Audio Update Timer
    // -------------------------------------------------------------------------
    // AudioPlayer::update() must be called regularly to:
    // - Detect track end
    // - Publish position events for UI updates
    //
    // 30 Hz (33ms) is sufficient for smooth progress bar updates
    
    m_pAudioUpdateTimer = new QTimer(this);
    m_pAudioUpdateTimer->setInterval(33);  // ~30 Hz
    connect(m_pAudioUpdateTimer, &QTimer::timeout, this, &MainWindow::onAudioUpdate);
    m_pAudioUpdateTimer->start();
    
    BasicLogger::logDebug("Audio update timer started (30 Hz)");
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

    // DialogManager unsubscribes from the EventBus in its destructor —
    // it MUST die before m_pServices.reset() below (member order alone
    // would destroy it AFTER the explicit reset → access violation).
    m_pDialogManager.reset();

    // -------------------------------------------------------------------------
    // Cleanup Services
    // -------------------------------------------------------------------------
    // Audio services are managed by ServiceContainer and will be cleaned up
    // when m_pServices is destroyed (automatic via unique_ptr destructor)

    BasicLogger::logDebug("MainWindow destructor - cleaning up ServiceContainer");
    m_pServices.reset();
    
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
    // Create Dialog Manager
    // -------------------------------------------------------------------------
    // Opens dialogs from the DialogRegistry on OpenDialogEvent (e.g. "about").

    m_pDialogManager = std::make_unique<DialogManager>(*m_pServices, this);
    m_pDialogManager->subscribeToEvents();

    BasicLogger::logDebug("  DialogManager created");

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
    // Das Menü wird über das MenuRegistry-System aufgebaut.
    // Die Default-Menüs werden automatisch beim ersten Zugriff auf
    // MenuRegistry::instance() registriert (lazy-init in initDefaults()).
    //
    // MenuManager baut die QMenuBar aus dem Registry auf und
    // integriert DockManager für Panels/Perspectives.

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
    // -------------------------------------------------------------------------
    // Create Initial Visualizer BEFORE Layout Restore
    // -------------------------------------------------------------------------
    // Qt-ADS restoreState() can only position widgets that already exist.
    // So we must create the visualizer first, then restore the layout.
    // The layout will then move the visualizer to its saved position.
    
    auto* pVisualizer = m_pDockManager->createVisualizer(
        QString(), DockPosition::Center);  // Title is now dynamic

    if (pVisualizer != nullptr)
    {
        // Set default visualizer to Pulsing
        pVisualizer->setVisualizer(QStringLiteral("pulsing"));
        BasicLogger::logDebug("  Default visualizer created with Pulsing effect");
    }
    
    // -------------------------------------------------------------------------
    // Restore Layout AFTER all widgets are created
    // -------------------------------------------------------------------------
    // Now all widgets exist (Panels from DockManager constructor, Visualizer above).
    // Qt-ADS can restore their positions from the saved state.
    
    m_pDockManager->restoreLayout();

    // Save this as the default perspective (only if no perspectives exist)
    if (m_pDockManager->perspectiveNames().isEmpty())
    {
        m_pDockManager->savePerspective("Default");
    }
}

void MainWindow::setupEventHandlers()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Event-Driven Menu Actions
    // -------------------------------------------------------------------------
    // Menu items trigger Events via EventBus. MainWindow subscribes to these
    // events and executes the actual actions. This decouples menu definitions
    // from the implementation.
    //
    // NOTE: Most dock-related events (CreateVisualizer, ResetLayout, ChangeVisualizer,
    // TogglePanel) are now handled by DockManager. MainWindow only handles
    // events that require MainWindow-specific actions.

    auto* pEventBus = m_pServices->tryResolve<IEventBus>();
    if (pEventBus == nullptr)
    {
        BasicLogger::logWarning("EventBus not available - menu events won't work");
        return;
    }

    // Frame Mode Changed event - MainWindow-specific (emits signal)
    pEventBus->subscribe<FrameModeChangedEvent>([this](const FrameModeChangedEvent& e) {
        BasicLogger::logInfo("Frame mode changed to: " + std::to_string(e.mode));
        emit frameModeChangeRequested(e.mode);
    });

    // OpenDialogEvent is handled by the DialogManager (created in setupUI).

    // Fullscreen Toggle event
    pEventBus->subscribe<ToggleFullscreenEvent>([this](const ToggleFullscreenEvent&) {
        toggleFullscreen();
    });

    // Fullscreen Exit event (Esc key)
    pEventBus->subscribe<ExitFullscreenEvent>([this](const ExitFullscreenEvent&) {
        if (isFullScreen())
        {
            showNormal();
            BasicLogger::logDebug("Exited fullscreen mode");
        }
    });

    BasicLogger::logDebug("  Event handlers registered");
}

void MainWindow::onAudioUpdate()
{
    // -------------------------------------------------------------------------
    // Audio Update Callback
    // -------------------------------------------------------------------------
    // Called ~30 times per second to update audio playback state.
    // This triggers position events for UI updates (progress bar, time display).
    
    static int updateCount = 0;
    updateCount++;
    
    auto* pPlayer = m_pServices->tryResolve<IAudioPlayer>();
    if (pPlayer != nullptr)
    {
        pPlayer->update();
        
        // =====================================================================
        // Audio Data → Visualizer
        // =====================================================================
        // Get audio stream and FFT data, then forward to visualizers
        
        auto* pEngine = m_pServices->tryResolve<IAudioEngine>();
        if (pEngine != nullptr)
        {
            // Cast to AudioPlayer to get stream handle
            auto* audioPlayer = dynamic_cast<AudioPlayer*>(pPlayer);
            if (audioPlayer != nullptr)
            {
                AudioStreamHandle stream = audioPlayer->currentStream();
                
                if (stream != 0)  // Valid stream
                {
                    // Get FFT spectrum data (512 bins is good for visualization)
                    constexpr int FFT_SIZE = 1024;
                    std::vector<float> spectrum(FFT_SIZE);
                    
                    // Get waveform data for oscilloscope-style visualizers
                    constexpr int WAVEFORM_SIZE = 1024;
                    std::vector<float> waveform(WAVEFORM_SIZE);
                    
                    bool hasSpectrum = pEngine->getFFTData(stream, spectrum.data(), FFT_SIZE);
                    bool hasWaveform = pEngine->getWaveformData(stream, waveform.data(), WAVEFORM_SIZE);
                    
                    if (hasSpectrum || hasWaveform)
                    {
                        // Debug: Log first few updates
                        if (updateCount <= 5 || updateCount % 150 == 0)
                        {
                            float sum = 0.0f;
                            for (int i = 0; i < 32; ++i) sum += spectrum[i];
                            BasicLogger::logDebug("Audio FFT data: sum(0-31)=" + 
                                                  std::to_string(sum) +
                                                  ", visualizers=" + 
                                                  std::to_string(visualizers().size()));
                        }
                        
                        // Forward to all visualizers
                        for (auto* pViz : visualizers())
                        {
                            if (pViz != nullptr)
                            {
                                // Send first half of FFT (useful frequency bins)
                                if (hasSpectrum)
                                {
                                    pViz->updateSpectrum(spectrum.data(), FFT_SIZE / 2);
                                }
                                
                                // Send waveform data for oscilloscope-style visualizers
                                if (hasWaveform)
                                {
                                    pViz->updateWaveform(waveform.data(), WAVEFORM_SIZE);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::setupAudioServices()
{
    // -------------------------------------------------------------------------
    // Audio Services Initialization
    // -------------------------------------------------------------------------
    // IMPORTANT: ServiceContainer uses std::mutex (not recursive_mutex).
    // Calling resolve() inside a factory causes deadlock because the mutex
    // is already held by the outer tryResolve() call.
    //
    // Solution: Register services without nested resolve() calls, then
    // manually wire them together after all are created.
    
    // 1. Register BassEngine (no dependencies)
    m_pServices->registerSingleton<IAudioEngine>([](ServiceContainer& /*c*/) {
        auto engine = std::make_unique<BassEngine>();
        if (!engine->initialize())
        {
            BasicLogger::logError("Failed to initialize BassEngine!");
            return std::unique_ptr<IAudioEngine>(nullptr);
        }
        BasicLogger::logDebug("  BassEngine initialized");
        return std::unique_ptr<IAudioEngine>(engine.release());
    });
    
    // 2. Get EventBus reference (already registered, no factory call)
    auto& eventBus = m_pServices->resolve<IEventBus>();
    
    // 3. Force BassEngine initialization now
    auto* pEngine = m_pServices->tryResolve<IAudioEngine>();
    if (pEngine == nullptr)
    {
        BasicLogger::logError("Failed to create BassEngine!");
        return;
    }
    
    // 4. Register Playlist with captured EventBus reference
    m_pServices->registerSingleton<IPlaylist>([&eventBus](ServiceContainer& /*c*/) {
        auto playlist = std::make_unique<Playlist>(eventBus);
        BasicLogger::logDebug("  Playlist created");
        return std::unique_ptr<IPlaylist>(playlist.release());
    });
    
    // 5. Force Playlist initialization
    auto* pPlaylist = m_pServices->tryResolve<IPlaylist>();
    
    // 6. Register AudioPlayer with captured references (no resolve() in factory!)
    m_pServices->registerSingleton<IAudioPlayer>(
        [pEngine, &eventBus, pPlaylist](ServiceContainer& /*c*/) {
            auto player = std::make_unique<AudioPlayer>(*pEngine, eventBus);
            
            if (pPlaylist != nullptr)
            {
                player->setPlaylist(pPlaylist);
                BasicLogger::logDebug("  AudioPlayer connected to Playlist");
            }
            
            BasicLogger::logDebug("  AudioPlayer created");
            return std::unique_ptr<IAudioPlayer>(player.release());
        });
    
    // 7. Force AudioPlayer initialization
    auto* pPlayer = m_pServices->tryResolve<IAudioPlayer>();
    
    if (pEngine != nullptr && pPlaylist != nullptr && pPlayer != nullptr)
    {
        BasicLogger::logInfo("Audio services initialized successfully");
    }
    else
    {
        BasicLogger::logError("Failed to initialize some audio services!");
    }
}

// =============================================================================
// Fullscreen
// =============================================================================

void MainWindow::toggleFullscreen()
{
    if (m_isFullscreen)
    {
        exitFullscreen();
    }
    else
    {
        enterFullscreen();
    }
}

void MainWindow::enterFullscreen()
{
    if (m_isFullscreen)
    {
        return;
    }
    
    // Get the primary visualizer
    VisualizerWidget* visualizer = primaryVisualizer();
    if (visualizer == nullptr)
    {
        BasicLogger::logWarning("No visualizer available for fullscreen");
        return;
    }
    
    m_pFullscreenVisualizer = visualizer;
    m_isFullscreen = true;
    
    // Store current window state
    m_wasMaximized = isMaximized();
    m_normalGeometry = geometry();
    
    // Hide all UI elements except the visualizer
    menuBar()->hide();
    statusBar()->hide();
    
    // Hide all dock widgets except the one containing our visualizer
    if (m_pDockManager)
    {
        auto* adsMgr = m_pDockManager->adsDockManager();
        if (adsMgr)
        {
            // Just hide the dock widgets - don't close them or save state
            m_hiddenDocksForFullscreen.clear();
            
            for (auto* dockWidget : adsMgr->dockWidgetsMap())
            {
                if (dockWidget && dockWidget->widget() != visualizer)
                {
                    if (!dockWidget->isClosed())
                    {
                        m_hiddenDocksForFullscreen.push_back(dockWidget);
                        dockWidget->hide();
                    }
                }
            }
        }
    }
    
    // Make the main window fullscreen
    showFullScreen();
    
    // Ensure visualizer is visible and focused
    visualizer->show();
    visualizer->setFocus();
    visualizer->raise();
    
    BasicLogger::logInfo("Entered fullscreen mode");
}

void MainWindow::exitFullscreen()
{
    if (!m_isFullscreen)
    {
        return;
    }
    
    m_isFullscreen = false;
    
    // Restore window state
    if (m_wasMaximized)
    {
        showMaximized();
    }
    else
    {
        showNormal();
        setGeometry(m_normalGeometry);
    }
    
    // Show UI elements
    menuBar()->show();
    statusBar()->show();
    
    // Show the dock widgets that were hidden for fullscreen
    for (auto* dockWidget : m_hiddenDocksForFullscreen)
    {
        if (dockWidget)
        {
            dockWidget->show();
        }
    }
    m_hiddenDocksForFullscreen.clear();
    
    m_pFullscreenVisualizer = nullptr;
    
    BasicLogger::logInfo("Exited fullscreen mode");
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // Esc exits fullscreen
    if (event->key() == Qt::Key_Escape && m_isFullscreen)
    {
        exitFullscreen();
        event->accept();
        return;
    }
    
    // Pass to parent for normal handling
    QMainWindow::keyPressEvent(event);
}
