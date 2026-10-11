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
#include "AppInfo.hpp"  // Name/Version aus Solution.json (S73)
#include "UI/managers/DockManager.hpp"
#include "UI/managers/MenuManager.hpp"
#include "UI/managers/ScreenshotManager.hpp"
#include "UI/managers/ShortcutManager.hpp"
#include "UI/managers/DialogManager.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "visualizers/milkdrop/MilkdropSerializer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"  // Karaoke-Ablage schreiben (S89)
#include "UI/panels/KaraokePanel.hpp"                   // setzeKaraokeAn (KE4)
#include "UI/PresetSpeichern.hpp"                       // Save gegen Save As (S89)
#include "UI/PresetListen.hpp"                          // Favoriten / zuletzt (S89)
#include "UI/StartDatei.hpp"                            // `LumiViz.exe <datei>` (S101)
#include "UI/StartPreset.hpp"                           // laufendes Preset ueber den Neustart (S93)
#include "composer/AppAnbindung.hpp"                    // Composer-Naht N8
#include "loop/Offen.hpp"                               // Loop: Rueckfrage vor dem Beenden
#include "loop/Spieler.hpp"                             // Loop: Ton fuer die Visuals
#include "composer/SichttestAktionen.hpp"               // Composer-Naht N21
#include "sichttest/Anbindung.hpp"                      // Composer-Naht N20
#include "sichttest/AppAktionen.hpp"                    // Composer-Naht N20
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/EventBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/CommandBus.hpp"
#include "services/LiveVideoFeed.hpp"  // Kamera-Stopp im closeEvent (S70)
#include "services/MenuRegistry.hpp"
#include "services/events/UIEvents.hpp"

// Audio Services
#include "audio/AudioEvents.hpp"  // TrackChangedEvent -> Host Switcher (S82)
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
#include <QElapsedTimer>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "audio/SongtextOnline.hpp"  // Lyrics online (S86)
#include "audio/Wiederaufnahme.hpp"  // Resume playback (S88)
#include "audio/SongtextQuelle.hpp"  // Lyrics (S86)
#include <QFileDialog>
#include <QMessageBox>
#include <QMutexLocker>

// Qt-ADS (needed for fullscreen dock state save/restore)
#include <DockManager.h>
#include <DockWidget.h>

// BasicLogger
#include <BasicLogger.h>

namespace
{

/// Puffer-Wechsel-Default (S66) aus der App-Einstellung an den Host geben —
/// unter renderMutex() VOR jedem loadMilk*-Aufruf (Muster: Render-Scale S47).
/// Nodes mit eigenem Schalter ("Behalten"/"Löschen"/"Fading") ignorieren ihn.
void applyMilkdropPufferWechselDefault(MultiEffectVisualizer& host)
{
    using lumi::multieffect::PufferWechsel;
    QSettings settings;
    const QString key =
        settings.value(QStringLiteral("milkdrop/pufferWechsel"),
                       QStringLiteral("behalten"))
            .toString();
    const PufferWechsel modus = key == QLatin1String("loeschen")
                                    ? PufferWechsel::Loeschen
                                : key == QLatin1String("fading")
                                    ? PufferWechsel::Fading
                                : key == QLatin1String("ausblenden")
                                    ? PufferWechsel::Ausblenden
                                    : PufferWechsel::Behalten;
    const double fading =
        settings.value(QStringLiteral("milkdrop/pufferFadingProzent"), 50)
            .toInt() /
        100.0;
    const double ausblendSek =
        settings.value(QStringLiteral("milkdrop/pufferAusblendSek"), 2.0)
            .toDouble();
    host.setMilkdropPufferWechselDefault(modus, fading, ausblendSek);
    // Sicht-Blende (S67): ~0,5 s Schwarz-Einblendung ueber der Rausch-Saat
    // (Kaltstart/Loeschen-Wipe) — Default AN, abschaltbar im Settings-Panel
    host.setMilkdropSichtBlende(
        settings.value(QStringLiteral("milkdrop/sichtBlende"), true).toBool());
}

} // anonymous namespace

// =============================================================================
// Construction / Destruction
// =============================================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    BasicLogger::logDebug("MainWindow constructor");

    // Movement-Tabelle in Scheiben (S94): in der App entsteht die Tabelle des
    // Knotens Movement ueber mehrere Bilder verteilt statt in einem Stueck
    // (rund 0,5 s Stillstand je Knoten in Full HD). Standalones und Tests
    // lassen die Vorgabe „sofort" stehen. Einstellungen → Performance →
    // Movement Table schaltet auf „in einem Stueck" wie das AVS-Original;
    // LUMIVIZ_MOVEMENT_SCHEIBE_MS stellt die Scheibe fuer eine Diagnose um
    // (0 = sofort) und geht der Einstellung vor.
    {
        bool gesetzt = false;
        const double wunsch =
            qEnvironmentVariable("LUMIVIZ_MOVEMENT_SCHEIBE_MS").toDouble(&gesetzt);
        const bool amStueck =
            QSettings().value(QLatin1String(MultiEffectVisualizer::kMovementEinstellung), false).toBool();
        MultiEffectVisualizer::setMovementScheibeMs(
            gesetzt ? wunsch : (amStueck ? 0.0 : MultiEffectVisualizer::kMovementScheibeAppMs));
    }

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

    // Hotkey-Schicht (docs/ui/Hotkey_Konzept.md): Taste -> Aktion -> Ereignis.
    // MUSS vor setupUi() stehen: der DockManager baut in seinem Konstruktor
    // bereits die Panels, und das Settings-Panel loest den Manager beim Aufbau
    // seines Hotkey-Tabs auf. Stand die Registrierung spaeter (setupMenuBar),
    // fand das Panel nichts und zeigte "Hotkey layer not available".
    //
    // Der Container besitzt ihn, damit alle dieselbe Instanz sehen (eine
    // Wahrheit fuer die aktive Belegung).
    m_pServices->registerSingleton<ShortcutManager>(
        [](ServiceContainer& c) { return std::make_unique<ShortcutManager>(c); });
    m_pServices->resolve<ShortcutManager>();  // jetzt erzeugen: Filter haengen

    // Screenshot-Ablage: ein Ordner je Programmlauf. Die Startzeit wird HIER
    // genommen und nicht erst beim ersten Bild — sonst hiesse der Ordner nach
    // dem Zeitpunkt des ersten Tastendrucks (docs/ui/Screenshot_Ablage.md).
    // Ohne Qt-Elternteil: der unique_ptr besitzt ihn allein (sonst loeschte Qt
    // ihn ein zweites Mal).
    m_pScreenshotManager =
        std::make_unique<ScreenshotManager>(QDateTime::currentDateTime());

    setupUi();

    // Erst jetzt gibt es Visualizer-Widgets, an die sich der Manager haengen kann.
    m_pScreenshotManager->attach(primaryVisualizer());

    // -------------------------------------------------------------------------
    // Session Playlist
    // -------------------------------------------------------------------------
    // Restore AFTER setupUi(): the panels must exist so the Loaded event
    // reaches the PlaylistPanel. Save on aboutToQuit (same pattern as the
    // dock layout persistence in DockManager).

    restoreSessionPlaylist();

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        // S88 „Resume playback": den Stand des Players festhalten, solange er
        // noch spielt — VOR allem, was beim Beenden die Wiedergabe anhaelt
        if (auto* pPlayer = m_pServices->tryResolve<IAudioPlayer>())
            lumi::audio::speichereWiederaufnahme(lumi::audio::standZumBeenden(*pPlayer));
        saveSessionPlaylist();
    });

    // S93: das zuletzt aktive Preset wieder anwerfen — verzoegert ueber die
    // Ereignisschleife, damit der Visualizer steht, wenn das Lade-Ereignis
    // kommt. Bis S93 tat das der Effect Browser aus seinem Konstruktor, mit
    // dem, was ER sich gemerkt hatte.
    //
    // S101 (`LumiViz.exe <datei>`, Entscheid Patrik X1): eine Datei als
    // Argument geht vor. Ein Preset laeuft dann statt des zuletzt aktiven;
    // Karaoke-Preset und Komposition kommen weiter unten dazu.
    const QString startDatei =
        lumi::ui::startdatei::ausArgumenten(QCoreApplication::arguments());
    const lumi::ui::startdatei::Art startArt = lumi::ui::startdatei::art(startDatei);
    if (!startDatei.isEmpty())
        BasicLogger::logInfo("Start: file given as argument: " + startDatei.toStdString());
    QTimer::singleShot(0, this, [this, startDatei, startArt]() {
        auto* bus = m_pServices->tryResolve<IEventBus>();
        if (bus == nullptr) return;
        if (startArt == lumi::ui::startdatei::Art::Preset)
        {
            (void)lumi::ui::startdatei::oeffne(*bus, startDatei, /*beimStart=*/true);
            return;
        }
        const QString pfad = lumi::ui::startpreset::waehle(lumi::ui::startpreset::mitgeliefert());
        if (pfad.isEmpty()) return;
        BasicLogger::logInfo("Start: restoring last preset: " + pfad.toStdString());
        lumi::ui::startpreset::lade(*bus, pfad);
    });

    // Composer-Naht N4 (docs/ui/Composer_Konzept.md §12.2): war beim Beenden
    // eine Komposition geladen, kommt sie wieder ins Panel — mit „Resume after
    // restart" auch ihr Stand. Nach dem Preset der Kette: sie legt sich darueber.
    QTimer::singleShot(0, this, [this]() {
        if (VisualizerWidget* widget = primaryVisualizer())
            (void)lumi::composer::stelleKompositionWiederHer(*widget);
    });

    // S101: die Datei als Argument, soweit sie kein Preset der Kette ist —
    // nach dem Preset der Kette und dem Wiederherstellen der Komposition.
    QTimer::singleShot(0, this, [this, startDatei, startArt]() {
        auto* bus = m_pServices->tryResolve<IEventBus>();
        if (bus == nullptr) return;
        if (startArt == lumi::ui::startdatei::Art::Karaoke)
            (void)lumi::ui::startdatei::oeffne(*bus, startDatei, /*beimStart=*/true);
        // Composer-Naht N8 (docs/ui/Composer_Konzept.md §12.2; S101): eine
        // .lvcomp als Argument — derselbe Weg wie File → Open Composition
        if (startArt == lumi::ui::startdatei::Art::Komposition)
            bus->publish(lumi::composer::OpenCompositionEvent{startDatei.toStdString()});
    });

    // S88 „Resume playback" (Wunsch Patrik): wurde beim letzten Mal waehrend
    // des Abspielens beendet, dort weiterspielen — erst wenn die
    // Ereignisschleife laeuft (Panels aktiv, Abonnenten des Titelwechsels da).
    QTimer::singleShot(0, this, [this]() {
        if (!lumi::audio::wiederaufnahmeAn()) return;
        auto* pPlayer = m_pServices->tryResolve<IAudioPlayer>();
        if (pPlayer == nullptr) return;
        const auto stand = lumi::audio::ladeWiederaufnahme();
        if (lumi::audio::nimmWiederAuf(*pPlayer, stand))
            BasicLogger::logInfo("Resume playback: " + stand.datei.toStdString() + " at " +
                                 std::to_string(stand.positionMs / 1000) + " s");
    });

    // Composer-Naht N20 (docs/ui/Composer_Konzept.md §12.2): nur mit
    // `--testing` die Sichttest-Steuerung laden und mit dem Tester verbinden —
    // NACH allem oben, damit keine Aktion vom wiederhergestellten Startzustand
    // ueberdeckt wird (Entscheid Patrik W1: der Start selbst bleibt, wie er ist).
    if (lumi::sichttest::verlangt(QCoreApplication::arguments()))
        QTimer::singleShot(0, this, [this]() {
            QString grund;
            m_pSichttest =
                lumi::sichttest::Anbindung::lade(QCoreApplication::applicationVersion(), &grund);
            if (m_pSichttest == nullptr)
            {
                BasicLogger::logWarning("Sichttest: --testing given, but no control: " +
                                        grund.toStdString());
                return;
            }
            // Composer-Naht N21: die Aktionen, die Composer-Code rufen
            if (VisualizerWidget* widget = primaryVisualizer())
            {
                lumi::composer::meldeSichttestAktionen(*m_pSichttest, *widget);
                // Composer-Naht N20: Panels, Karaoke-Ebene, Playlist und Player
                lumi::sichttest::meldeAppAktionen(*m_pSichttest, *widget);
            }
            BasicLogger::logInfo("Sichttest: --testing, " +
                                 std::to_string(m_pSichttest->aktionen().size()) +
                                 " actions registered, project root " +
                                 QDir::toNativeSeparators(m_pSichttest->root()).toStdString());
            (void)m_pSichttest->verbinde();
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

    setWindowTitle(tr("%1 - Audio Visualizer").arg(lumi::appName()));
    resize(1280, 720);
    setMinimumSize(800, 600);

    BasicLogger::logDebug("  Window size: 1280x720 (min: 800x600)");

    // -------------------------------------------------------------------------
    // Create Dock Manager
    // -------------------------------------------------------------------------
    // DockManager creates the Qt-ADS CDockManager internally.
    // It takes over the central area of MainWindow.

    m_pDockManager = std::make_unique<DockManager>(*m_pServices, this);
    // S89: der Wechsel des Visualizers verwirft die Effektkette — vorher die
    // Rueckfrage zu ungespeicherten Aenderungen. Derselbe Visualizer noch
    // einmal ist kein Wechsel (und kein Verlust).
    m_pDockManager->setVisualizerWechselWaechter([this](const QString& vizId) {
        const VisualizerWidget* widget = primaryVisualizer();
        if (widget != nullptr && widget->currentVisualizerId() == vizId) return true;
        return darfKetteErsetzen(tr("Change Visualizer"));
    });

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

    // Rueckmeldung zum Screenshot: im Vollbild sieht man die Zeile nicht, aber
    // dort ist ohnehin die Ablage der Beleg (fehler.log + Bild).
    if (m_pScreenshotManager != nullptr)
    {
        connect(m_pScreenshotManager.get(), &ScreenshotManager::shotWritten, this,
                [this](const QString& file) {
                    statusBar()->showMessage(tr("Screenshot: %1").arg(file), 6000);
                });
        connect(m_pScreenshotManager.get(), &ScreenshotManager::shotFailed, this,
                [this](const QString& reason) {
                    statusBar()->showMessage(tr("Screenshot failed: %1").arg(reason),
                                             8000);
                });
    }

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
        // Zuletzt gewaehlten Visualizer wiederherstellen (S73). Vorher stand
        // hier fest "multieffect" — wer mit Superscope oder Pulsing arbeitete,
        // landete nach jedem Start wieder im Multieffekt-Host. Der Multieffekt
        // bleibt die Vorgabe fuer den allerersten Start (Ziel des AVS-Imports).
        QSettings settings;
        const QString wanted =
            settings
                .value(QStringLiteral("ui/lastVisualizer"),
                       QStringLiteral("multieffect"))
                .toString();
        pVisualizer->setVisualizer(wanted);

        // Jede weitere Wahl mitschreiben. Bewusst NUR fuer den ersten
        // Visualizer: weitere Fenster sind Beiwerk, sonst gewaenne das zuletzt
        // umgeschaltete Nebenfenster die Vorgabe des naechsten Starts.
        connect(pVisualizer, &VisualizerWidget::visualizerChanged, this,
                [](const QString& id) {
                    if (id.isEmpty()) return;
                    QSettings s;
                    s.setValue(QStringLiteral("ui/lastVisualizer"), id);
                });

        BasicLogger::logDebug("  Default visualizer created with: " +
                              wanted.toStdString());
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

    // Screenshot des Visuals (Hotkey Druck)
    pEventBus->subscribe<ScreenshotRequestEvent>(
        [this](const ScreenshotRequestEvent&) {
            if (m_pScreenshotManager == nullptr) return;
            // Im Vollbild ist ein anderes Widget im Bild als im Fenster.
            m_pScreenshotManager->attach(m_isFullscreen ? m_pFullscreenVisualizer
                                                        : primaryVisualizer());
            m_pScreenshotManager->requestShot();
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
            // ungespeichert? (S89) — ausser die Frage ist schon gestellt (S91)
            if (!ev.confirmed && !darfKetteErsetzen(tr("Import AVS Preset"))) return;
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
                // Entscheid S47: Import-Render-Scale aus der App-Einstellung
                // (Settings-Panel, Panels-Tab) — wirkt nur beim Import.
                host->setImportRenderScaleDivisor(
                    QSettings()
                        .value(QStringLiteral("import/avsRenderScaleDivisor"), 1)
                        .toInt());
                ok = host->loadAvsFile(path, &report);
                if (ok) host->merkeKettenStand();  // frisch geladen = ungeaendert (S89)
            }
            if (!ok)
            {
                reportProblem(tr("Import AVS Preset"),
                              tr("Not a valid AVS preset:\n%1").arg(path));
                pEventBus->publish(AvsImportResultEvent{path.toStdString(), false, 0});
                return;
            }
            // Ab hier haengt das Bild an diesem Preset — der Screenshot soll es
            // beim Namen nennen und seinen Pfad daneben legen.
            if (m_pScreenshotManager != nullptr)
            {
                setzeLaufendesPreset(QFileInfo(path).absoluteFilePath());
            }
            pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
            // NUR Probleme (Parser, Passthrough, fehlende Dateien) kommen hier
            // an — die planmaessigen Hinweise wie die `_p`-Umbenennungen stehen
            // im "Import Notes"-Knoten der Kette (Entscheid Patrik S51). Vorher
            // liefen beide durch diesen Dialog, und ein Dutzend Umbenennungen
            // machte die echten Probleme unsichtbar.
            reportProblem(tr("Import AVS Preset"), report);
            pEventBus->publish(AvsImportResultEvent{
                path.toStdString(), true, static_cast<int>(report.size())});
        });

    pEventBus->subscribe<ImportMilkPresetEvent>(
        [this, findMultiEffect, pEventBus](const ImportMilkPresetEvent& ev) {
            // ungespeichert? (S89) — ausser die Frage ist schon gestellt (S91)
            if (!ev.confirmed && !darfKetteErsetzen(tr("Import MilkDrop Preset"))) return;
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
                // Puffer-Wechsel-Default (S66) aus der App-Einstellung — greift
                // fuer Nodes auf "App-Einstellung" (Muster: Render-Scale S47).
                applyMilkdropPufferWechselDefault(*host);
                ok = host->loadMilkFile(path, &report);
                if (ok) host->merkeKettenStand();  // frisch geladen = ungeaendert (S89)
            }
            if (!ok)
            {
                reportProblem(tr("Import MilkDrop Preset"),
                              tr("Not a valid MilkDrop preset:\n%1").arg(path));
                return;
            }
            if (m_pScreenshotManager != nullptr)
            {
                setzeLaufendesPreset(QFileInfo(path).absoluteFilePath());
            }
            pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
            // "ℹ"-Zeilen sind reine Bestaetigungen — nur echte Warnungen melden
            QStringList warnings;
            for (const QString& line : report)
            {
                if (!line.startsWith(QStringLiteral("ℹ"))) warnings.append(line);
            }
            reportProblem(tr("Import MilkDrop Preset"), warnings);
        });

    // Songwechsel -> Renderer (S82, Host Switcher: Song-Slot, „bei Songwechsel
    // weiter"). Bis hierher erreichte TrackChangedEvent nur Player- und
    // Playlist-Panel. Der Zaehler ist prozessweit und atomar — kein Lock.
    // S86 (Entscheide 3a/4a): Online-Suche nach dem Songtext — Opt-in in den
    // Einstellungen, erst wenn keine lokale Quelle etwas liefert; der Fund wird
    // gespeichert und gilt nur, wenn derselbe Titel noch laeuft.
    // S88 (KE1): Karaoke-Ebene — Songtext ueber jedem Preset; Schalter,
    // Mischart und Staerke stehen seit S89 im Karaoke-Panel (die Mischart in
    // den Knoten der Ebene, s. Application: uebernehmeAlteKaraokeMischung)
    MultiEffectVisualizer::setKaraokeEbene(
        QSettings().value(QStringLiteral("karaoke/enabled"), false).toBool());
    auto* songtextOnline = new lumi::audio::SongtextOnline(this);
    auto laufenderSong = std::make_shared<QString>();
    connect(songtextOnline, &lumi::audio::SongtextOnline::gefunden, this,
            [laufenderSong](const QString& songPfad,
                            const lumi::multieffect::lyrics::Songtext& text, const QString&) {
                if (songPfad == *laufenderSong) MultiEffectVisualizer::setSongtext(text);
            });
    // S87: Stand des Abrufs fuer die Lyrics-Tabelle im Editor („…downloading…")
    connect(songtextOnline, &lumi::audio::SongtextOnline::zustand, this,
            [laufenderSong](const QString& songPfad, lumi::audio::SongtextAbruf stand) {
                if (songPfad == *laufenderSong) MultiEffectVisualizer::setSongtextAbruf(stand);
            });
    pEventBus->subscribe<TrackChangedEvent>(
        [songtextOnline, laufenderSong](const TrackChangedEvent& e) {
            // S86: die Tags des Titels gehen mit — der Text-Knoten setzt sie in
            // Vorlagen ein ({title}, {artist} …)
            lumi::multieffect::texttags::SongTags tags;
            tags.title = e.track.title.toStdString();
            tags.artist = e.track.artist.toStdString();
            tags.album = e.track.album.toStdString();
            tags.year = e.track.year.toStdString();
            tags.genre = e.track.genre.toStdString();
            tags.track = e.track.trackNumber.toStdString();
            tags.file = QFileInfo(e.track.filePath).completeBaseName().toStdString();
            MultiEffectVisualizer::notifySongChanged(tags);
            // S86: Songtext suchen (lokale Quellen in fester Reihenfolge) und
            // die Abspielposition auf Anfang — der Lyrics-Knoten laeuft an ihr
            const auto lokal = lumi::audio::findeSongtext(e.track);
            MultiEffectVisualizer::setSongtext(lokal);
            // S88: der Versatz, der nur fuer diesen Titel gilt (offsets.json)
            MultiEffectVisualizer::setSongtextTitelVersatz(
                lumi::audio::songtextTitelVersatzMs(e.track));
            MultiEffectVisualizer::notifyPlayPosition(0);
            *laufenderSong = e.track.filePath;
            lumi::audio::setzeLaufendenTitel(e.track);  // S87: fuer „Alternatives…"
            // neuer Titel: der Stand des vorigen Abrufs gilt nicht mehr
            MultiEffectVisualizer::setSongtextAbruf(lumi::audio::SongtextAbruf::Ruhe);
            // nichts Lokales: online fragen — die Suche prueft die Einstellung
            // selbst und bricht eine noch laufende Anfrage ab
            if (lokal.leer())
                songtextOnline->suche(e.track);
            else
                songtextOnline->suche(TrackInfo{});  // nur abbrechen
        });
    pEventBus->subscribe<PlaybackPositionEvent>([](const PlaybackPositionEvent& e) {
        MultiEffectVisualizer::notifyPlayPosition(e.positionMs);
    });
    pEventBus->subscribe<PlaybackStateEvent>([](const PlaybackStateEvent& e) {
        MultiEffectVisualizer::notifyPlayState(e.state == PlaybackState::Playing);
    });
    // S91: das Tempo geht in die Hochrechnung der Position zwischen zwei Meldungen
    pEventBus->subscribe<PlaybackTempoEvent>([](const PlaybackTempoEvent& e) {
        MultiEffectVisualizer::notifyPlayTempo(static_cast<double>(e.tempo));
    });

    pEventBus->subscribe<NewEffectChainEvent>(
        [this, findMultiEffect, pEventBus](const NewEffectChainEvent&) {
            // Leere Kette als Ausgangspunkt fuer ein neues Preset. Wie beim
            // Import wird der Host bei Bedarf aktiviert — sonst muesste man
            // erst den Visualizer wechseln, um „neu" anwaehlen zu koennen.
            auto [widget, host] = findMultiEffect();
            if (host == nullptr && widget != nullptr)
            {
                widget->setVisualizer(QStringLiteral("multieffect"));
                host = dynamic_cast<MultiEffectVisualizer*>(widget->visualizer());
            }
            if (host == nullptr)
            {
                QMessageBox::information(this, tr("New Effect Chain"),
                                         tr("No visualizer available."));
                return;
            }
            // Eine Kette kann viel Arbeit sein — nicht kommentarlos wegwerfen.
            // Seit S89 fragt das die gemeinsame Rueckfrage, und nur noch, wenn
            // es wirklich ungespeicherte Aenderungen gibt.
            if (!darfKetteErsetzen(tr("New Effect Chain"))) return;

            lumi::multieffect::ChainNode root;
            root.params = lumi::multieffect::ListParams{};
            root.displayName = "Effect List";
            {
                QMutexLocker lock(&widget->renderMutex());
                (void)host->setChain(std::move(root));
            }
            if (m_pScreenshotManager != nullptr)
            {
                setzeLaufendesPreset(QString());
            }
            pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
        });

    // Karaoke an/aus aus dem Menue oder per Tastenkuerzel (KE4): derselbe Weg
    // wie der Haken im Karaoke-Panel; der Menue-Haken zieht bei jedem Wechsel
    // nach, egal wer geschaltet hat.
    pEventBus->subscribe<KaraokeToggleEvent>([pEventBus](const KaraokeToggleEvent&) {
        KaraokePanel::setzeKaraokeAn(pEventBus, !MultiEffectVisualizer::karaokeEbene());
    });
    pEventBus->subscribe<KaraokeEnabledChangedEvent>(
        [this](const KaraokeEnabledChangedEvent& e) {
            if (auto* aktion = findChild<QAction*>(QStringLiteral("menu.view.karaoke")))
                aktion->setChecked(e.enabled);
        });

    // Karaoke-Preset (.lvkar) aus dem Effect Browser (S89): in die Karaoke-
    // Ebene laden, nicht in die Kette. Der Stand wird zur laufenden Ebene
    // (Ablage), das Karaoke-Panel liest ihn neu.
    // Derselbe Weg wie im Karaoke-Panel: erst die Rueckfrage zu ungespeicherten
    // Aenderungen der Ebene (Entscheid 5a), dann laden.
    pEventBus->subscribe<LoadKaraokePresetEvent>(
        [this, findMultiEffect, pEventBus](const LoadKaraokePresetEvent& ev) {
            const QString path = QString::fromStdString(ev.path);
            auto [widget, host] = findMultiEffect();
            if (host == nullptr)
            {
                QMessageBox::information(
                    this, tr("Load Karaoke Preset"),
                    tr("The karaoke layer works with the \"Multi Effect\" visualizer."));
                return;
            }
            const KaraokePanel::Ziel ziel{host, &widget->renderMutex(), pEventBus, this};
            if (!KaraokePanel::darfKaraokeErsetzen(ziel, tr("Load Karaoke Preset"))) return;
            QStringList report;
            // S92 (1a): der Doppelklick schaltet die Ebene auch ein
            if (!KaraokePanel::ladeUndZeigeKaraokeDatei(ziel, path, &report))
                reportProblem(tr("Load Karaoke Preset"),
                              tr("Could not load:\n%1\n%2").arg(path, report.join(QLatin1Char('\n'))));
        });

    pEventBus->subscribe<LoadEffectChainEvent>(
        [this, findMultiEffect, pEventBus](const LoadEffectChainEvent& ev) {
            // ungespeichert? (S89) — Composer-Naht N24: ausser die Frage ist schon gestellt
            if (!ev.confirmed && !darfKetteErsetzen(tr("Load Effect Chain"))) return;
            QString path = QString::fromStdString(ev.path);
            if (path.isEmpty())
            {
                // S91: „Load…" im Ketten-Panel bietet jedes Preset an — vorher nur
                // .lvfx, und eine ueber „All Files" gewaehlte .avs/.milk wurde
                // als Kette gelesen und scheiterte
                path = QFileDialog::getOpenFileName(
                    this, ev.anyPreset ? tr("Load Preset") : tr("Load Effect Chain"), QString(),
                    ev.anyPreset
                        ? tr("Presets (*.lvfx *.lvfx2 *.avs *.milk);;LumiViz Effect Chain (*.lvfx "
                             "*.lvfx2);;AVS Presets (*.avs);;MilkDrop Presets (*.milk);;All Files (*)")
                        : tr("LumiViz Effect Chain (*.lvfx *.lvfx2);;All Files (*)"));
            }
            if (path.isEmpty()) return;
            // eine .avs / .milk gehoert dem Import — auch wenn sie ueber „All
            // Files" kam. Die Rueckfrage oben gilt, der Import fragt nicht noch einmal.
            switch (lumi::ui::presetDateiArt(path))
            {
                case lumi::ui::PresetDateiArt::Avs:
                    pEventBus->publish(ImportAvsPresetEvent{path.toStdString(), true});
                    return;
                case lumi::ui::PresetDateiArt::Milk:
                    pEventBus->publish(ImportMilkPresetEvent{path.toStdString(), true});
                    return;
                case lumi::ui::PresetDateiArt::Kette:
                    break;
            }

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
                    // Puffer-Wechsel-Default (S66) — wie beim .milk-Import
                    applyMilkdropPufferWechselDefault(*host);
                    ok = host->loadMilkDocument(path, &report);
                    if (ok) host->merkeKettenStand();  // frisch geladen = ungeaendert (S89)
                }
                if (!ok)
                {
                    reportProblem(tr("Load Effect Chain"),
                                  tr("Could not load:\n%1").arg(path));
                    return;
                }
                if (m_pScreenshotManager != nullptr)
                {
                    setzeLaufendesPreset(
                        QFileInfo(path).absoluteFilePath());
                }
                pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
                QStringList warnings;
                for (const QString& line : report)
                {
                    if (!line.startsWith(QStringLiteral("ℹ"))) warnings.append(line);
                }
                reportProblem(tr("Load Milkdrop Preset"), warnings);
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
                // Milkdrop-Defaults auch hier — die Kette kann Milkdrop-Nodes
                // enthalten (Sicht-Blende S67 braucht den Stand VOR dem ersten
                // Frame der frischen Runtime)
                applyMilkdropPufferWechselDefault(*host);
                ok = host->loadChainFile(path, &report);
                if (ok) host->merkeKettenStand();  // frisch geladen = ungeaendert (S89)
            }
            if (!ok)
            {
                reportProblem(tr("Load Effect Chain"),
                              tr("Could not load:\n%1").arg(path));
                return;
            }
            if (m_pScreenshotManager != nullptr)
            {
                setzeLaufendesPreset(QFileInfo(path).absoluteFilePath());
            }
            reportProblem(tr("Load Effect Chain"), report);
            pEventBus->publish(EffectChainChangedEvent{});  // refresh the editor
        });

    // Save / Save As (S89, Entscheid Patrik 3a) — Regel: UI/PresetSpeichern.hpp
    pEventBus->subscribe<SaveEffectChainEvent>([this](const SaveEffectChainEvent& ev) {
        // Composer-Naht N11 (docs/ui/Composer_Konzept.md §12.2): haengt Effect
        // Chain an einem Clip der Komposition, speichert der Composer dessen
        // Kette — immer unter neuem Namen. Nur hier, beim Speichern von Hand:
        // die Rueckfrage zur Kette der App (darfKetteErsetzen) bleibt bei ihr.
        if (VisualizerWidget* widget = primaryVisualizer();
            widget != nullptr && lumi::composer::clipKetteGebunden(*widget))
        {
            (void)lumi::composer::speichereClipKette(*widget, this);
            return;
        }
        (void)speichereKette(ev.saveAs);
    });

    // Composer-Naht N8 (docs/ui/Composer_Konzept.md §12.2): Menue File →
    // Open / Close Composition; alles Weitere in composer/AppAnbindung
    pEventBus->subscribe<lumi::composer::OpenCompositionEvent>(
        [this](const lumi::composer::OpenCompositionEvent& ev) {
            if (VisualizerWidget* widget = primaryVisualizer())
                (void)lumi::composer::oeffneKomposition(*widget, this,
                                                        QString::fromStdString(ev.path));
        });
    pEventBus->subscribe<lumi::composer::CloseCompositionEvent>(
        [this](const lumi::composer::CloseCompositionEvent&) {
            if (VisualizerWidget* widget = primaryVisualizer())
                lumi::composer::schliesseKomposition(*widget);
        });

    BasicLogger::logDebug("  Event handlers registered");
}

// =============================================================================
// Speichern und ungespeicherte Aenderungen (S89)
// =============================================================================

QString MainWindow::laufendesPreset() const
{
    return m_pScreenshotManager != nullptr ? m_pScreenshotManager->currentPreset() : QString();
}

void MainWindow::setzeLaufendesPreset(const QString& absolutePath)
{
    if (m_pScreenshotManager != nullptr) m_pScreenshotManager->setCurrentPreset(absolutePath);
    lumi::ui::PresetListen::setzeLaufendesKettenPreset(absolutePath);
    // S93: fuer den naechsten Start festhalten — HIER, weil jeder Ladeweg
    // hier durchkommt (Browser, Preset-Zeile, Load, Save As, Import). Leer
    // (neue Kette) laesst das zuletzt benannte Preset stehen.
    lumi::ui::startpreset::sichere(absolutePath);
    // S89: geladen, importiert oder gespeichert — an die Spitze von „zuletzt",
    // und die Preset-Zeile des Effect-Chain-Panels zieht nach
    if (!absolutePath.isEmpty())
        lumi::ui::PresetListen::merkeBenutzt(lumi::ui::PresetArt::Kette, absolutePath);
    if (auto* bus = m_pServices->tryResolve<IEventBus>())
    {
        bus->publish(CurrentPresetChangedEvent{absolutePath.toStdString()});
        bus->publish(PresetListsChangedEvent{});
    }
}

bool MainWindow::speichereKette(bool saveAs)
{
    // N2: Milkdrop lebt als Chain-Node — Speichern laeuft IMMER ueber den
    // Chain-Serializer (Milkdrop-Nodes betten ihr Preset ein). Alte
    // milkdrop-Schwester-Dokumente bleiben LADbar.
    VisualizerWidget* widget = primaryVisualizer();
    auto* host = widget != nullptr ? dynamic_cast<MultiEffectVisualizer*>(widget->visualizer())
                                   : nullptr;
    if (host == nullptr)
    {
        QMessageBox::information(this, tr("Save Effect Chain"),
                                 tr("Select the \"Multi Effect\" visualizer first."));
        return false;
    }

    // HG1 (Entwurf §6): Ketten mit mindestens einer Host-Gruppe speichern als
    // .lvfx2 — flache Ketten bleiben .lvfx.
    bool hasGroup = false;
    {
        QMutexLocker lock(&widget->renderMutex());
        hasGroup = lumi::multieffect::chainHasHostGroup(host->chain());
    }
    const QString laufend = laufendesPreset();

    QString path;
    if (!saveAs && lumi::ui::stillSpeicherbar(laufend, hasGroup, lumi::ui::mitgelieferteOrdner()))
    {
        path = laufend;  // „Save": die eigene Datei, ohne Dialog
    }
    else
    {
        // Namensvorschlag (Wunsch S65): das zuletzt geladene/importierte Preset
        // — ein importiertes AVS/MilkDrop speichert so als gleichnamige Kette.
        QString suggestion;
        if (!laufend.isEmpty())
        {
            const QFileInfo last(laufend);
            suggestion = last.absolutePath() + QLatin1Char('/') + last.completeBaseName() +
                         lumi::ui::kettenEndung(hasGroup);
        }
        path = QFileDialog::getSaveFileName(this, tr("Save Effect Chain"), suggestion,
                                            tr("LumiViz Effect Chain (*.lvfx *.lvfx2)"));
        if (path.isEmpty()) return false;
        if (path.endsWith(".lvfx2", Qt::CaseInsensitive)) path.chop(6);
        else if (path.endsWith(".lvfx", Qt::CaseInsensitive)) path.chop(5);
        path += lumi::ui::kettenEndung(hasGroup);
    }

    bool ok = false;
    {
        QMutexLocker lock(&widget->renderMutex());
        ok = host->saveChainFile(path);
        if (ok) host->merkeKettenStand();  // ab jetzt gilt die Kette als gespeichert
    }
    if (!ok)
    {
        QMessageBox::warning(this, tr("Save Effect Chain"), tr("Could not save:\n%1").arg(path));
        return false;
    }
    // die gespeicherte Datei ist jetzt das laufende Preset (bis S88 blieb es
    // die zuletzt GELADENE — der naechste Dialog schlug den alten Namen vor)
    setzeLaufendesPreset(QFileInfo(path).absoluteFilePath());
    return true;
}

bool MainWindow::darfKetteErsetzen(const QString& title)
{
    VisualizerWidget* widget = primaryVisualizer();
    auto* host = widget != nullptr ? dynamic_cast<MultiEffectVisualizer*>(widget->visualizer())
                                   : nullptr;
    if (host == nullptr) return true;  // keine Kette, nichts zu verlieren
    bool geaendert = false;
    {
        QMutexLocker lock(&widget->renderMutex());
        geaendert = host->ketteGeaendert();
    }
    if (!geaendert) return true;

    // Vollbild: keine Dialoge (S52) — aber ungespeichert ein Preset weiter ist
    // hoechstwahrscheinlich ein Unfall: Vollbild verlassen, dann fragen.
    if (m_isFullscreen) exitFullscreen();

    const QString laufend = laufendesPreset();
    const QString name =
        laufend.isEmpty() ? tr("The new effect chain") : QFileInfo(laufend).fileName();
    QMessageBox frage(this);
    frage.setIcon(QMessageBox::Question);
    frage.setWindowTitle(title);
    frage.setText(tr("%1 has unsaved changes.").arg(name));
    frage.setInformativeText(tr("Save them before continuing?"));
    frage.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    frage.setDefaultButton(QMessageBox::Save);
    switch (frage.exec())
    {
        case QMessageBox::Save: return speichereKette(false);  // abgebrochen = zurueck
        case QMessageBox::Discard: return true;
        default: return false;
    }
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
                // Composer-Naht N13 (docs/ui/Composer_Konzept.md §12.2): fuehrt
                // der Composer Ton, kommen Spektrum und Wellenform von seinem
                // Mixer — die Summe der Songs mit ihren Pegeln, dasselbe, was
                // zu hoeren ist. Sonst der Stream des Players wie immer.
                if (const AudioStreamHandle mixer = lumi::composer::tonMixer();
                    mixer != INVALID_STREAM)
                {
                    stream = mixer;
                }
                // Loop (docs/audio/Loop_Konzept.md, L4): spielt das Loop-Panel
                // allein, kommen Spektrum und Wellenform von seinem Mixer. Der
                // Composer steht darueber — fuehrt er, spielt der Loop nicht.
                else if (const AudioStreamHandle loopMixer = lumi::loop::tonMixer();
                         loopMixer != INVALID_STREAM)
                {
                    stream = loopMixer;
                }

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

                    // Composer-Naht N13, waechst mit N17 (S101): der Mute fuer
                    // Durchsagen sitzt an der Lautstaerke, und die aendert das
                    // Spektrum nicht. Traegt das Mute-Profil „Visuals follow",
                    // gehen Spektrum und Wellenform hier mit dem Ton zurueck —
                    // sonst ist der Faktor 1 und nichts wird angefasst.
                    const float muteSicht = lumi::composer::muteFaktorFuerVisuals();
                    if (muteSicht < 1.0f)
                    {
                        for (float& wert : spectrum) wert *= muteSicht;
                        for (float& wert : waveform) wert *= muteSicht;
                    }

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
                                // Composer-Naht N13 / N17: wie oben
                                if (muteSicht < 1.0f)
                                    for (float& wert : specI) wert *= muteSicht;
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
    // Diagnose (S84, Befund „Freeze beim Vollbild-Wechsel"): Dauer je Schritt.
    QElapsedTimer uhr;
    uhr.start();
    std::string schritte;
    const auto mark = [&](const char* name) {
        schritte += std::string(" ") + name + "=" + std::to_string(uhr.restart()) + "ms";
    };
    visualizer->hide();
    mark("hide");
    dock->takeWidget();
    mark("takeWidget");
    visualizer->setParent(nullptr);
    mark("setParent");
    visualizer->showFullScreen();
    mark("showFullScreen");

    // Keyboard focus must land in the embedded GL window, otherwise Esc
    // never reaches it (the reparented native child starts unfocused)
    visualizer->activateWindow();
    visualizer->setFocus();
    visualizer->activateGLWindow();
    mark("focus");

    BasicLogger::logInfo("Entered fullscreen mode (top-level visualizer);" + schritte);
}

void MainWindow::reportProblem(const QString& title, const QString& problem)
{
    reportProblem(title, QStringList{problem});
}

void MainWindow::reportProblem(const QString& title, const QStringList& problems)
{
    if (problems.isEmpty()) return;

    // Vollbild: KEIN Dialog. Er landete hinter dem randlosen Fenster, blockierte
    // die Vorfuehrung und war beim Durchblaettern nach jedem zweiten Preset im
    // Weg (Vorgabe Patrik, Session 52). Stattdessen Protokoll + Bild.
    if (m_isFullscreen && m_pScreenshotManager != nullptr)
    {
        m_pScreenshotManager->attach(m_pFullscreenVisualizer);
        m_pScreenshotManager->reportProblem(title, problems);
        return;
    }

    QMessageBox::warning(this, title,
                         problems.size() == 1
                             ? problems.front()
                             : tr("%1 problem(s):\n\n%2")
                                   .arg(problems.size())
                                   .arg(problems.mid(0, 20).join(QLatin1Char('\n'))));
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
    // Diagnose (S84, Befund „Freeze beim Vollbild-Wechsel"): Dauer je Schritt.
    QElapsedTimer uhr;
    uhr.start();
    std::string schritte;
    const auto mark = [&](const char* name) {
        schritte += std::string(" ") + name + "=" + std::to_string(uhr.restart()) + "ms";
    };
    if (m_pFullscreenVisualizer != nullptr && m_pFullscreenDock != nullptr)
    {
        m_pFullscreenVisualizer->hide();
        mark("hide");
        m_pFullscreenVisualizer->setWindowState(Qt::WindowNoState);
        mark("windowState");
        m_pFullscreenDock->setWidget(m_pFullscreenVisualizer);
        mark("setWidget");
        m_pFullscreenVisualizer->show();
        mark("show");
        m_pFullscreenDock->raise();
        mark("raise");

        // Native handle resync: after reparenting FROM top-level, Qt leaves
        // the facade's native window at a stale absolute position (measured
        // -7,-30 — the "doubled bar" strip). Recreating the native handles
        // rebuilds the parent chain; the render thread rides the same
        // surface-destroy/expose cycle as when undocking.
        QPointer<VisualizerWidget> viz = m_pFullscreenVisualizer;
        QTimer::singleShot(0, this, [viz]() {
            if (viz != nullptr)
            {
                QElapsedTimer t;
                t.start();
                viz->recreateNativeWindow();
                BasicLogger::logInfo("Fullscreen exit: recreateNativeWindow=" +
                                     std::to_string(t.elapsed()) + "ms");
            }
        });
    }

    m_pFullscreenVisualizer = nullptr;
    m_pFullscreenDock = nullptr;

    BasicLogger::logInfo("Exited fullscreen mode;" + schritte);
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
    // S89: ungespeicherte Aenderungen an der Kette? Cancel haelt das Fenster offen.
    if (!darfKetteErsetzen(tr("Quit LumiViz")))
    {
        event->ignore();
        return;
    }
    // Composer-Naht N10 (docs/ui/Composer_Konzept.md §12.2): dieselbe Frage
    // fuer eine bearbeitete Komposition
    if (VisualizerWidget* widget = primaryVisualizer();
        widget != nullptr &&
        !lumi::composer::darfKompositionErsetzen(*widget, this, tr("Quit LumiViz")))
    {
        event->ignore();
        return;
    }
    // und fuer einen bearbeiteten Loop im Loop-Panel (docs/audio/Loop_Konzept.md §4c)
    if (!lumi::loop::darfBeenden(tr("Quit LumiViz")))
    {
        event->ignore();
        return;
    }
    BasicLogger::logInfo("MainWindow closed - quitting application");
    // Kamera-Feeds werden hier bewusst NICHT gestoppt (S70, dritter Anlauf):
    // eine STERBENDE MF-/D3D-Pipeline waehrend des GL-Teardowns verklemmt
    // den GPU-Kanal (Haupt-Thread hing in nvoglv64). Reihenfolge jetzt:
    // Riegel bei aboutToQuit (keine Neustarts mehr), GL/Fenster abbauen
    // waehrend die Kamera normal weiterlaeuft, DANACH der Kamera-Stopp in
    // Application::shutdown (LiveVideoFeed::herunterfahren).
    QMainWindow::closeEvent(event);
    QCoreApplication::quit();
}

// =============================================================================
// Eine Datei von einem weiteren Start (S101, core/EinzelInstanz)
// =============================================================================

void MainWindow::oeffneUebergebeneDatei(const QString& datei)
{
    // nach vorn — auch aus dem minimierten Zustand
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    show();
    raise();
    activateWindow();

    if (datei.isEmpty()) return;
    BasicLogger::logInfo("File handed over by another start: " + datei.toStdString());
    auto* bus = m_pServices->tryResolve<IEventBus>();
    if (bus == nullptr) return;
    // Composer-Naht N8 (docs/ui/Composer_Konzept.md §12.2; S101): eine .lvcomp —
    // derselbe Weg wie File → Open Composition, mit der Rueckfrage bei Ungespeichertem
    if (lumi::ui::startdatei::art(datei) == lumi::ui::startdatei::Art::Komposition)
    {
        bus->publish(lumi::composer::OpenCompositionEvent{datei.toStdString()});
        return;
    }
    (void)lumi::ui::startdatei::oeffne(*bus, datei, /*beimStart=*/false);
}
