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
#include "visualizers/milkdrop/MilkdropSerializer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
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
#include <QCloseEvent>
#include <QTimer>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMutexLocker>

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
    // Session Playlist
    // -------------------------------------------------------------------------
    // Restore AFTER setupUi(): the panels must exist so the Loaded event
    // reaches the PlaylistPanel. Save on aboutToQuit (same pattern as the
    // dock layout persistence in DockManager).

    restoreSessionPlaylist();

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        saveSessionPlaylist();
    });

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
// Public Slots
// =============================================================================
// Note: The former requestRender()/frame timer path is gone — every
// VisualizerWidget renders on its own thread (Render_Thread_Entwurf.md).

void MainWindow::updateFpsDisplay(double fps)
{
    if (m_pFpsLabel != nullptr)
    {
        m_pFpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
    }

    // Throttled FPS log (~5 s) — successor of the old frame-timer log
    static int callCount = 0;
    if ((callCount++ % 5) == 0)
    {
        BasicLogger::logDebug("FPS (render thread): " +
                              std::to_string(static_cast<int>(fps)));
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

void MainWindow::setFrameModeOnAllVisualizers(int mode, int targetFps)
{
    // Menu index → render pacing (0 = Limited, 1 = Unlimited, 2 = VSync)
    RenderPacing pacing = RenderPacing::Limited;
    switch (mode)
    {
        case 1: pacing = RenderPacing::Unlimited; break;
        case 2: pacing = RenderPacing::VSync; break;
        default: break;
    }

    auto vizList = visualizers();
    for (auto* pViz : vizList)
    {
        if (pViz != nullptr)
        {
            pViz->setFrameMode(pacing, targetFps);
        }
    }
    BasicLogger::logDebug("Frame mode " + std::to_string(mode) + " (target " +
                          std::to_string(targetFps) + " fps) on " +
                          std::to_string(vizList.size()) + " visualizer(s)");
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

        // Status bar shows the FPS of the PRIMARY visualizer (the first one);
        // the value arrives queued from its render thread.
        if (visualizers().size() == 1 || primaryVisualizer() == pVisualizer)
        {
            connect(pVisualizer, &VisualizerWidget::fpsMeasured,
                    this, &MainWindow::updateFpsDisplay);
        }
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
        // Set default visualizer to the Multi Effect host (AVS import target)
        pVisualizer->setVisualizer(QStringLiteral("multieffect"));
        BasicLogger::logDebug("  Default visualizer created with Multi Effect host");
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

    // Fullscreen Toggle event — carries the requesting visualizer (or null)
    pEventBus->subscribe<ToggleFullscreenEvent>([this](const ToggleFullscreenEvent& e) {
        if (m_isFullscreen)
        {
            exitFullscreen();
        }
        else
        {
            enterFullscreen(static_cast<VisualizerWidget*>(e.sourceVisualizer));
        }
    });

    // Fullscreen Exit event (Esc key)
    pEventBus->subscribe<ExitFullscreenEvent>([this](const ExitFullscreenEvent&) {
        if (isFullScreen())
        {
            showNormal();
            BasicLogger::logDebug("Exited fullscreen mode");
        }
    });

    // -------------------------------------------------------------------------
    // AVS import + host effect-chain presets (Import Roadmap 5.7)
    // -------------------------------------------------------------------------
    // The active visualizer must be the Multi Effect host; chain mutation runs
    // under the widget's renderMutex() (the render thread walks the chain).
    auto findMultiEffect =
        [this]() -> std::pair<VisualizerWidget*, MultiEffectVisualizer*> {
        VisualizerWidget* widget = primaryVisualizer();
        if (widget == nullptr) return {nullptr, nullptr};
        return {widget, dynamic_cast<MultiEffectVisualizer*>(widget->visualizer())};
    };
    auto requireMultiEffect =
        [this, findMultiEffect](const char* title)
        -> std::pair<VisualizerWidget*, MultiEffectVisualizer*> {
        auto pair = findMultiEffect();
        if (pair.second == nullptr)
        {
            QMessageBox::information(this, title,
                                     tr("Select the \"Multi Effect\" visualizer first."));
        }
        return pair;
    };

    pEventBus->subscribe<ImportAvsPresetEvent>(
        [this, findMultiEffect, pEventBus](const ImportAvsPresetEvent& ev) {
            // Auto-activate the Multi Effect host if it is not already active
            // (lets the Import Browser panel import without a manual switch).
            auto [widget, host] = findMultiEffect();
            if (host == nullptr && widget != nullptr)
            {
                widget->setVisualizer(QStringLiteral("multieffect"));
                host = dynamic_cast<MultiEffectVisualizer*>(widget->visualizer());
            }
            if (host == nullptr)
            {
                QMessageBox::information(
                    this, tr("Import AVS Preset"),
                    tr("No visualizer available to import into."));
                return;
            }

            QString path = QString::fromStdString(ev.path);
            if (path.isEmpty())
            {
                path = QFileDialog::getOpenFileName(
                    this, tr("Import AVS Preset"), QString(),
                    tr("AVS Presets (*.avs);;All Files (*)"));
            }
            if (path.isEmpty()) return;

            QStringList report;
            bool ok = false;
            {
                QMutexLocker lock(&widget->renderMutex());
                ok = host->loadAvsFile(path, &report);
            }
            if (!ok)
            {
                QMessageBox::warning(this, tr("Import AVS Preset"),
                                     tr("Not a valid AVS preset:\n%1").arg(path));
                pEventBus->publish(AvsImportResultEvent{path.toStdString(), false, 0});
                return;
            }
            pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
            // Notes are import problems (passthrough/parser) — surface them; a
            // clean import stays silent (dialog only on problems).
            if (!report.isEmpty())
            {
                QMessageBox::information(
                    this, tr("Import AVS Preset"),
                    tr("Imported with %1 note(s):\n\n%2")
                        .arg(report.size())
                        .arg(report.mid(0, 20).join("\n")));
            }
            pEventBus->publish(AvsImportResultEvent{
                path.toStdString(), true, static_cast<int>(report.size())});
        });

    pEventBus->subscribe<ImportMilkPresetEvent>(
        [this, findMultiEffect, pEventBus](const ImportMilkPresetEvent& ev) {
            // N2 (Entscheide E1/E2): .milk landet als Milkdrop-Chain-Node im
            // MultiEffect-Host — der Standalone-Milkdrop ist Geschichte.
            auto [widget, host] = findMultiEffect();
            if (host == nullptr && widget != nullptr)
            {
                widget->setVisualizer(QStringLiteral("multieffect"));
                host = dynamic_cast<MultiEffectVisualizer*>(widget->visualizer());
            }
            if (host == nullptr)
            {
                QMessageBox::information(
                    this, tr("Import MilkDrop Preset"),
                    tr("No visualizer available to import into."));
                return;
            }

            QString path = QString::fromStdString(ev.path);
            if (path.isEmpty())
            {
                path = QFileDialog::getOpenFileName(
                    this, tr("Import MilkDrop Preset"), QString(),
                    tr("MilkDrop Presets (*.milk);;All Files (*)"));
            }
            if (path.isEmpty()) return;

            QStringList report;
            bool ok = false;
            {
                QMutexLocker lock(&widget->renderMutex());
                ok = host->loadMilkFile(path, &report);
            }
            if (!ok)
            {
                QMessageBox::warning(this, tr("Import MilkDrop Preset"),
                                     tr("Not a valid MilkDrop preset:\n%1").arg(path));
                return;
            }
            pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
            // "ℹ"-Zeilen sind reine Bestaetigungen — Dialog nur bei echten Warnungen
            bool hasWarnings = false;
            for (const QString& line : report)
            {
                if (!line.startsWith(QStringLiteral("ℹ"))) hasWarnings = true;
            }
            if (hasWarnings)
            {
                QMessageBox::information(
                    this, tr("Import MilkDrop Preset"),
                    tr("Imported with %1 note(s):\n\n%2")
                        .arg(report.size())
                        .arg(report.mid(0, 20).join("\n")));
            }
        });

    pEventBus->subscribe<LoadEffectChainEvent>(
        [this, findMultiEffect, pEventBus](const LoadEffectChainEvent& ev) {
            QString path = QString::fromStdString(ev.path);
            if (path.isEmpty())
            {
                path = QFileDialog::getOpenFileName(
                    this, tr("Load Effect Chain"), QString(),
                    tr("LumiViz Effect Chain (*.lvfx);;All Files (*)"));
            }
            if (path.isEmpty()) return;

            // .lvfx dispatch (M6/N2): a milkdrop sister document becomes a
            // Milkdrop chain node in the MultiEffect host (Entscheid E1/E2).
            if (lumi::milkdrop::isMilkdropFile(path))
            {
                auto [widget, host] = findMultiEffect();
                if (host == nullptr && widget != nullptr)
                {
                    widget->setVisualizer(QStringLiteral("multieffect"));
                    host = dynamic_cast<MultiEffectVisualizer*>(widget->visualizer());
                }
                if (host == nullptr)
                {
                    QMessageBox::information(this, tr("Load Effect Chain"),
                                             tr("No visualizer available to load into."));
                    return;
                }
                QStringList report;
                bool ok = false;
                {
                    QMutexLocker lock(&widget->renderMutex());
                    ok = host->loadMilkDocument(path, &report);
                }
                if (!ok)
                {
                    QMessageBox::warning(this, tr("Load Effect Chain"),
                                         tr("Could not load:\n%1").arg(path));
                    return;
                }
                pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
                bool hasWarnings = false;
                for (const QString& line : report)
                {
                    if (!line.startsWith(QStringLiteral("ℹ"))) hasWarnings = true;
                }
                if (hasWarnings)
                {
                    QMessageBox::information(
                        this, tr("Load Milkdrop Preset"),
                        tr("Loaded with %1 note(s):\n\n%2")
                            .arg(report.size())
                            .arg(report.mid(0, 20).join("\n")));
                }
                return;
            }

            // Auto-activate the Multi Effect host (Import Browser loads .lvfx too).
            auto [widget, host] = findMultiEffect();
            if (host == nullptr && widget != nullptr)
            {
                widget->setVisualizer(QStringLiteral("multieffect"));
                host = dynamic_cast<MultiEffectVisualizer*>(widget->visualizer());
            }
            if (host == nullptr)
            {
                QMessageBox::information(this, tr("Load Effect Chain"),
                                         tr("No visualizer available to load into."));
                return;
            }

            QStringList report;
            bool ok = false;
            {
                QMutexLocker lock(&widget->renderMutex());
                ok = host->loadChainFile(path, &report);
            }
            if (!ok)
            {
                QMessageBox::warning(this, tr("Load Effect Chain"),
                                     tr("Could not load:\n%1").arg(path));
                return;
            }
            pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
        });

    pEventBus->subscribe<SaveEffectChainEvent>(
        [this, requireMultiEffect](const SaveEffectChainEvent&) {
            // N2: Milkdrop lebt als Chain-Node — Speichern laeuft IMMER ueber
            // den Chain-Serializer (Milkdrop-Nodes betten ihr Preset ein).
            // Alte milkdrop-Schwester-Dokumente bleiben LADbar (Dispatch oben).
            auto [widget, host] = requireMultiEffect("Save Effect Chain");
            if (host == nullptr) return;
            QString path = QFileDialog::getSaveFileName(
                this, tr("Save Effect Chain"), QString(),
                tr("LumiViz Effect Chain (*.lvfx)"));
            if (path.isEmpty()) return;
            if (!path.endsWith(".lvfx", Qt::CaseInsensitive)) path += ".lvfx";

            bool ok = false;
            {
                QMutexLocker lock(&widget->renderMutex());
                ok = host->saveChainFile(path);
            }
            if (!ok)
            {
                QMessageBox::warning(this, tr("Save Effect Chain"),
                                     tr("Could not save:\n%1").arg(path));
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

                        // Per-channel (stereo) audio for getspec/getosc L/R
                        // (additive; the mono forward above stays as the fallback).
                        const int channels = pEngine->getStreamChannels(stream);
                        if (channels >= 2 && hasWaveform)
                        {
                            std::vector<float> specI(
                                static_cast<size_t>(FFT_SIZE) * channels);
                            const bool hasStereoFFT = pEngine->getFFTDataStereo(
                                stream, specI.data(), FFT_SIZE);
                            if (hasStereoFFT)
                            {
                                for (auto* pViz : visualizers())
                                {
                                    if (pViz != nullptr)
                                    {
                                        pViz->updateAudioStereo(
                                            specI.data(), FFT_SIZE / 2, waveform.data(),
                                            WAVEFORM_SIZE / channels, channels);
                                    }
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
// Session Playlist Persistence
// =============================================================================

namespace
{
    /**
     * @brief Directory + file path of the auto-saved session playlist.
     */
    QString sessionPlaylistDir()
    {
        return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    QString sessionPlaylistPath()
    {
        return sessionPlaylistDir() + QStringLiteral("/session.m3u8");
    }

    constexpr const char* SESSION_INDEX_KEY = "playlist/sessionIndex";
}

void MainWindow::saveSessionPlaylist()
{
    auto* pPlaylist = m_pServices->tryResolve<IPlaylist>();
    if (pPlaylist == nullptr)
    {
        return;
    }

    const QString filePath = sessionPlaylistPath();
    QSettings settings;

    if (pPlaylist->isEmpty())
    {
        // No playlist loaded: remove session data so the next start
        // does not restore a stale list
        QFile::remove(filePath);
        settings.remove(QLatin1String(SESSION_INDEX_KEY));
        BasicLogger::logDebug("Session playlist: empty - nothing to save");
        return;
    }

    QDir().mkpath(sessionPlaylistDir());

    if (pPlaylist->save(filePath))
    {
        settings.setValue(QLatin1String(SESSION_INDEX_KEY), pPlaylist->currentIndex());
        BasicLogger::logInfo("Session playlist saved: " + filePath.toStdString() +
                             " (" + std::to_string(pPlaylist->count()) + " tracks)");
    }
    else
    {
        BasicLogger::logWarning("Failed to save session playlist: " +
                                filePath.toStdString());
    }
}

void MainWindow::restoreSessionPlaylist()
{
    auto* pPlaylist = m_pServices->tryResolve<IPlaylist>();
    if (pPlaylist == nullptr)
    {
        return;
    }

    const QString filePath = sessionPlaylistPath();
    if (!QFile::exists(filePath))
    {
        BasicLogger::logDebug("Session playlist: no session file - starting empty");
        return;
    }

    if (!pPlaylist->load(filePath))
    {
        BasicLogger::logWarning("Failed to load session playlist: " +
                                filePath.toStdString());
        return;
    }

    // Restore current track selection (does NOT start playback)
    const QSettings settings;
    const int index = settings.value(QLatin1String(SESSION_INDEX_KEY), -1).toInt();
    if (index >= 0 && index < pPlaylist->count())
    {
        pPlaylist->setCurrentIndex(index);
    }

    BasicLogger::logInfo("Session playlist restored: " +
                         std::to_string(pPlaylist->count()) + " tracks" +
                         (index >= 0 ? ", current index " + std::to_string(index) : ""));
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

void MainWindow::enterFullscreen(VisualizerWidget* requested)
{
    if (m_isFullscreen)
    {
        return;
    }

    // The requesting visualizer (double-click/Esc source) wins; menu/F11
    // fall back to the primary one
    VisualizerWidget* visualizer = (requested != nullptr) ? requested
                                                          : primaryVisualizer();
    if (visualizer == nullptr)
    {
        BasicLogger::logWarning("No visualizer available for fullscreen");
        return;
    }

    // Find the ADS dock hosting the visualizer (to re-embed on exit)
    ads::CDockWidget* dock = nullptr;
    for (QWidget* p = visualizer->parentWidget(); p != nullptr; p = p->parentWidget())
    {
        dock = qobject_cast<ads::CDockWidget*>(p);
        if (dock != nullptr)
        {
            break;
        }
    }
    if (dock == nullptr)
    {
        BasicLogger::logWarning("Fullscreen: no hosting dock found");
        return;
    }

    m_pFullscreenVisualizer = visualizer;
    m_pFullscreenDock = dock;
    m_isFullscreen = true;

    // TRUE fullscreen: take the widget OUT of the dock and show it as a
    // borderless top-level window — no tab bar, no dock title, no side tabs.
    // The main window stays untouched behind it. The GL surface is recreated
    // by the reparenting; the render thread handles that (Entwurf §4).
    // Hide BEFORE reparenting: no transient top-level flash / stale regions.
    visualizer->hide();
    dock->takeWidget();
    visualizer->setParent(nullptr);
    visualizer->showFullScreen();

    // Keyboard focus must land in the embedded GL window, otherwise Esc
    // never reaches it (the reparented native child starts unfocused)
    visualizer->activateWindow();
    visualizer->setFocus();
    visualizer->activateGLWindow();

    BasicLogger::logInfo("Entered fullscreen mode (top-level visualizer)");
}

void MainWindow::exitFullscreen()
{
    if (!m_isFullscreen)
    {
        return;
    }

    m_isFullscreen = false;

    // Re-embed the visualizer into its dock. Order matters: hide FIRST and
    // drop the fullscreen state invisibly — showNormal() on the still
    // top-level widget briefly showed a plain window and left stale
    // native-window regions behind.
    if (m_pFullscreenVisualizer != nullptr && m_pFullscreenDock != nullptr)
    {
        m_pFullscreenVisualizer->hide();
        m_pFullscreenVisualizer->setWindowState(Qt::WindowNoState);
        m_pFullscreenDock->setWidget(m_pFullscreenVisualizer);
        m_pFullscreenVisualizer->show();
        m_pFullscreenDock->raise();

        // Native handle resync: after reparenting FROM top-level, Qt leaves
        // the facade's native window at a stale absolute position (measured
        // -7,-30 — the "doubled bar" strip). Recreating the native handles
        // rebuilds the parent chain; the render thread rides the same
        // surface-destroy/expose cycle as when undocking.
        QPointer<VisualizerWidget> viz = m_pFullscreenVisualizer;
        QTimer::singleShot(0, this, [viz]() {
            if (viz != nullptr)
            {
                viz->recreateNativeWindow();
            }
        });
    }

    m_pFullscreenVisualizer = nullptr;
    m_pFullscreenDock = nullptr;

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

void MainWindow::closeEvent(QCloseEvent* event)
{
    BasicLogger::logInfo("MainWindow closed - quitting application");
    QMainWindow::closeEvent(event);
    QCoreApplication::quit();
}
