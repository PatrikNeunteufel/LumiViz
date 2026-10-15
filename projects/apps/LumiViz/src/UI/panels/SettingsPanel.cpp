/**
 ****************************************************************************************
 * @file   SettingsPanel.cpp
 * @brief  SettingsPanel implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/panels/SettingsPanel.hpp"
#include "UI/dialogs/DatensicherungDialoge.hpp"
#include "UI/managers/ShortcutManager.hpp"
#include "scripting/ScriptFormatter.hpp"  // FormatOptions-Defaults (Editor-Tab, S69)
#include "core/GpuInfo.hpp"
#include "core/GpuPreference.hpp"
#include "core/Mitgeliefert.hpp"
#include "core/Suchordner.hpp"
#include "loop/Sample.hpp"  // „Chosen Sample Files": kopieren oder am Ort lassen (S115)
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/LiveVideoFeed.hpp"  // Kamera-Tab: Freigabe + Testaufnahmen-Ordner (S70)
#include "services/ShortcutRegistry.hpp"
#include "services/events/UIEvents.hpp"
#include "audio/IAudioEngine.hpp"
#include "audio/SongtextQuelle.hpp"  // Benutzerordner der Songtexte (S86)
#include "audio/Wiederaufnahme.hpp"  // Resume playback (S88)
#include "UI/dialogs/LyricsAlternativesDialog.hpp"  // Sperrliste der Songtexte (S87)
#include "visualizers/MultiEffectVisualizer.hpp"    // Schalter der Karaoke-Ebene (S88)
#include "composer/Sitzung.hpp"                     // Composer-Naht N9
#include "sichttest/Anbindung.hpp"                  // der Haken zum Sichttest nur mit Steuerung

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QCoreApplication>
#include <QKeySequenceEdit>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QProcess>
#include <QSettings>
#include <QSpacerItem>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QSignalBlocker>
#include <QTimer>
#include <QToolTip>
#include <QUrl>
#include <QCamera>                 // Kamera-Tab (S70)
#include <QCameraDevice>
#include <QDateTime>
#include <QFile>
#include <QListWidget>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QMediaRecorder>

#include <algorithm>
#include <optional>

#include <BasicLogger.h>

// =============================================================================
// GPU-Praeferenz-Helfer
// =============================================================================

namespace
{

/// Exe-Pfad, wie ihn die Windows-Grafikeinstellung als Wertnamen erwartet.
std::wstring nativeExePath()
{
    return QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
        .toStdWString();
}

/// GL_RENDERER eines frischen Kontexts — dieselbe Karte, auf der auch die
/// Visualizer-Kontexte landen. Einmalige Abfrage beim Panel-Aufbau.
QString activeOpenGlRenderer()
{
    QOffscreenSurface surface;
    surface.create();
    QOpenGLContext context;
    if (!context.create() || !context.makeCurrent(&surface))
    {
        return QStringLiteral("unknown");
    }
    const GLubyte* renderer = context.functions()->glGetString(GL_RENDERER);
    const QString name =
        (renderer != nullptr)
            ? QString::fromLatin1(reinterpret_cast<const char*>(renderer))
            : QStringLiteral("unknown");
    context.doneCurrent();
    return name;
}

} // anonymous namespace

// =============================================================================
// Construction
// =============================================================================

SettingsPanel::SettingsPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("settings"), tr("Settings"), parent)
{
    setupUI();
    setupConnections();
    populateAudioDevices();
}

// =============================================================================
// IPanel Implementation
// =============================================================================

int SettingsPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

// =============================================================================
// Lifecycle
// =============================================================================

void SettingsPanel::onActivate()
{
    subscribeToEvents();
}

void SettingsPanel::onDeactivate()
{
    unsubscribeFromEvents();
}

// =============================================================================
// Event Subscription
// =============================================================================

void SettingsPanel::subscribeToEvents()
{
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        return;
    }
    
    // Listen for frame mode changes from elsewhere
    m_eventSubscriptions.push_back(eventBus->subscribeScoped<FrameModeChangedEvent>(
        [this](const FrameModeChangedEvent& e) {
            if (!m_isUpdating)
            {
                m_isUpdating = true;
                m_pFrameModeCombo->setCurrentIndex(e.mode);
                m_isUpdating = false;
            }
        }));
}

void SettingsPanel::unsubscribeFromEvents()
{
    // RAII handles unsubscribe on destruction; clearing releases them now.
    m_eventSubscriptions.clear();
}

// =============================================================================
// Slots
// =============================================================================

void SettingsPanel::onAudioDeviceChanged(int index)
{
    if (m_isUpdating || index < 0)
    {
        return;
    }
    
    int deviceId = m_pAudioDeviceCombo->itemData(index).toInt();
    
    auto* audioEngine = services().tryResolve<IAudioEngine>();
    if (audioEngine != nullptr)
    {
        audioEngine->setDevice(deviceId);
        BasicLogger::logInfo("SettingsPanel: Audio device changed to: " + 
                             std::to_string(deviceId));
    }
}

void SettingsPanel::onFrameModeChanged(int index)
{
    if (m_isUpdating || index < 0)
    {
        return;
    }
    
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus != nullptr)
    {
        m_isUpdating = true;
        eventBus->publish(FrameModeChangedEvent{index});
        m_isUpdating = false;
        
        BasicLogger::logInfo("SettingsPanel: Frame mode changed to: " + 
                             std::to_string(index));
    }
    
    // Enable/disable target FPS based on mode
    bool isLimited = (index == 0);  // Limited mode
    m_pTargetFpsSpinBox->setEnabled(isLimited);
    
    // Sync VSync checkbox
    m_pVSyncCheckBox->blockSignals(true);
    m_pVSyncCheckBox->setChecked(index == 2);  // VSync mode
    m_pVSyncCheckBox->blockSignals(false);
}

void SettingsPanel::onGpuPreferenceChanged(int index)
{
    if (m_isUpdating || index < 0)
    {
        return;
    }

    const auto mode = static_cast<GpuPreference::Mode>(
        m_pGpuPreferenceCombo->itemData(index).toInt());

    // Unveraendert (z. B. programmatisches Zuruecksetzen) — kein Neustart.
    const auto stored = GpuPreference::readForExecutable(nativeExePath());
    if (stored.value_or(GpuPreference::Mode::Automatic) == mode)
    {
        return;
    }

    if (!GpuPreference::writeForExecutable(nativeExePath(), mode))
    {
        QMessageBox::warning(
            this, tr("Graphics Card"),
            tr("Could not store the GPU preference in the Windows registry."));
        m_isUpdating = true;
        const int oldIdx = m_pGpuPreferenceCombo->findData(static_cast<int>(
            stored.value_or(GpuPreference::Mode::Automatic)));
        m_pGpuPreferenceCombo->setCurrentIndex(oldIdx >= 0 ? oldIdx : 0);
        m_isUpdating = false;
        return;
    }

    BasicLogger::logInfo(std::string("SettingsPanel: GPU preference -> ") +
                         GpuPreference::modeToString(mode) +
                         ", restarting now");

    // SOFORT-Neustart (Entscheid S61): neue Instanz starten, diese beenden.
    // Verzoegert ueber den Event-Loop, damit der Combo-Signal-Pfad sauber
    // zurueckkehrt, bevor Fenster und Render-Threads abgebaut werden.
    QTimer::singleShot(0, qApp, []() {
        const QString exe = QCoreApplication::applicationFilePath();
        QStringList args = QCoreApplication::arguments();
        if (!args.isEmpty())
        {
            args.removeFirst();  // argv[0] ist die Exe selbst
        }
        if (!QProcess::startDetached(exe, args, QDir::currentPath()))
        {
            BasicLogger::logError("SettingsPanel: restart failed to launch (" +
                                  exe.toStdString() + ")");
            return;  // lieber weiterlaufen als kommentarlos enden
        }
        QCoreApplication::quit();
    });
}

void SettingsPanel::onTargetFpsChanged(int value)
{
    if (m_isUpdating)
    {
        return;
    }
    
    // TODO: Publish target FPS change event
    BasicLogger::logDebug("SettingsPanel: Target FPS changed to: " + 
                          std::to_string(value));
}

void SettingsPanel::onVSyncChanged(bool checked)
{
    if (m_isUpdating)
    {
        return;
    }
    
    // VSync checkbox changes frame mode
    if (checked)
    {
        m_pFrameModeCombo->setCurrentIndex(2);  // VSync mode
    }
    else if (m_pFrameModeCombo->currentIndex() == 2)
    {
        m_pFrameModeCombo->setCurrentIndex(0);  // Back to Limited
    }
    
    BasicLogger::logDebug("SettingsPanel: VSync " + 
                          std::string(checked ? "enabled" : "disabled"));
}

void SettingsPanel::onResetImportBrowserDir()
{
    // The Import Browser owns its setting — it clears the stored path and
    // navigates home (permanent subscription, works while hidden).
    if (auto* eventBus = services().tryResolve<IEventBus>())
    {
        eventBus->publish(ResetImportBrowserDirEvent{});
        BasicLogger::logInfo("SettingsPanel: Import Browser start folder reset requested");
    }
}

void SettingsPanel::fuelleSuchordner(int waehle)
{
    if (m_pSearchDirList == nullptr) return;
    const QStringList ordner = lumi::core::suchordner::liste();
    {
        const QSignalBlocker ruhe(m_pSearchDirList);
        m_pSearchDirList->clear();
        for (const QString& einer : ordner)
        {
            auto* eintrag = new QListWidgetItem(QDir::toNativeSeparators(einer), m_pSearchDirList);
            // ein Ordner, den es gerade nicht gibt (ein Laufwerk fehlt), bleibt stehen — und sagt es
            if (!QFileInfo(einer).isDir())
            {
                eintrag->setText(tr("%1  (not found)").arg(QDir::toNativeSeparators(einer)));
                eintrag->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
            }
            eintrag->setData(Qt::UserRole, einer);
        }
        if (waehle >= 0 && waehle < m_pSearchDirList->count()) m_pSearchDirList->setCurrentRow(waehle);
    }
    const int zeile = m_pSearchDirList->currentRow();
    m_pSearchDirRemoveButton->setEnabled(zeile >= 0);
    m_pSearchDirUpButton->setEnabled(zeile > 0);
    m_pSearchDirDownButton->setEnabled(zeile >= 0 && zeile < m_pSearchDirList->count() - 1);
}

void SettingsPanel::onAddSearchDir()
{
    QStringList ordner = lumi::core::suchordner::liste();
    const QString start = ordner.isEmpty() ? QDir::homePath() : ordner.last();
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Add search folder"), start);
    if (dir.isEmpty()) return;  // abgebrochen — die Liste bleibt
    ordner << dir;
    lumi::core::suchordner::setzeListe(ordner);
    const QStringList neu = lumi::core::suchordner::liste();
    fuelleSuchordner(static_cast<int>(neu.indexOf(QDir::cleanPath(dir), 0, Qt::CaseInsensitive)));
    BasicLogger::logInfo("SettingsPanel: search folder added (" + dir.toStdString() + "), " +
                         std::to_string(neu.size()) + " in the list");
}

void SettingsPanel::onRemoveSearchDir()
{
    const int zeile = m_pSearchDirList->currentRow();
    QStringList ordner = lumi::core::suchordner::liste();
    if (zeile < 0 || zeile >= ordner.size()) return;
    const QString weg = ordner.takeAt(zeile);
    lumi::core::suchordner::setzeListe(ordner);
    fuelleSuchordner(std::min(zeile, static_cast<int>(ordner.size()) - 1));
    BasicLogger::logInfo("SettingsPanel: search folder removed (" + weg.toStdString() + ")");
}

void SettingsPanel::onMoveSearchDir(int richtung)
{
    const int zeile = m_pSearchDirList->currentRow();
    QStringList ordner = lumi::core::suchordner::liste();
    const int ziel = zeile + richtung;
    if (zeile < 0 || zeile >= ordner.size() || ziel < 0 || ziel >= ordner.size()) return;
    ordner.swapItemsAt(zeile, ziel);
    lumi::core::suchordner::setzeListe(ordner);
    fuelleSuchordner(ziel);
}

void SettingsPanel::onOpenAppDataDir()
{
    const QString path =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty())
    {
        QMessageBox::warning(this, tr("User Data"),
                             tr("Qt reports no writable data location."));
        return;
    }
    // Anlegen, bevor geoeffnet wird: beim ersten Start existiert der Ordner
    // noch nicht, und der Dateimanager oeffnet dann stillschweigend nichts.
    QDir().mkpath(path);
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
    {
        QMessageBox::warning(this, tr("User Data"),
                             tr("Could not open the folder:\n%1").arg(path));
        return;
    }
    BasicLogger::logInfo("SettingsPanel: user data folder opened (" +
                         path.toStdString() + ")");
}

// =============================================================================
// UI Setup
// =============================================================================

void SettingsPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->addTab(createAudioTab(), tr("Audio"));
    m_pTabWidget->addTab(createPerformanceTab(), tr("Performance"));
    m_pTabWidget->addTab(createPanelsTab(), tr("Panels"));
    m_pTabWidget->addTab(createEditorTab(), tr("Editor"));
    m_pTabWidget->addTab(createKameraTab(), tr("Camera"));
    m_pTabWidget->addTab(createHotkeyTab(), tr("Hotkeys"));

    mainLayout->addWidget(m_pTabWidget);
}

QWidget* SettingsPanel::createAudioTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QFormLayout(widget);
    layout->setSpacing(8);

    // Audio Device
    m_pAudioDeviceCombo = new QComboBox(widget);
    m_pAudioDeviceCombo->setToolTip(tr("Select audio output device"));
    layout->addRow(tr("Device:"), m_pAudioDeviceCombo);

    // Buffer Size
    m_pBufferSizeSpinBox = new QSpinBox(widget);
    m_pBufferSizeSpinBox->setRange(256, 8192);
    m_pBufferSizeSpinBox->setSingleStep(256);
    m_pBufferSizeSpinBox->setValue(1024);
    m_pBufferSizeSpinBox->setSuffix(tr(" samples"));
    m_pBufferSizeSpinBox->setToolTip(tr("Lower = less latency, higher = more stable"));
    layout->addRow(tr("Buffer Size:"), m_pBufferSizeSpinBox);

    // Sample Rate
    m_pSampleRateSpinBox = new QSpinBox(widget);
    m_pSampleRateSpinBox->setRange(22050, 192000);
    m_pSampleRateSpinBox->setSingleStep(1000);
    m_pSampleRateSpinBox->setValue(44100);
    m_pSampleRateSpinBox->setSuffix(tr(" Hz"));
    m_pSampleRateSpinBox->setToolTip(tr("Audio sample rate"));
    layout->addRow(tr("Sample Rate:"), m_pSampleRateSpinBox);

    // S88 (Wunsch Patrik): „Resume playback" — derselbe Schalter steht im
    // Player-Panel; beide lesen und schreiben `player/resumePlayback`.
    {
        auto* resume = new QCheckBox(tr("Enable"), widget);
        resume->setObjectName(QStringLiteral("resumePlayback"));
        resume->setChecked(lumi::audio::wiederaufnahmeAn());
        resume->setToolTip(
            tr("If LumiViz is closed while a track is playing, the next start continues "
               "that track at the same position. Closed while paused or stopped: it stays "
               "silent."));
        connect(resume, &QCheckBox::toggled, this,
                [](bool an) { lumi::audio::setWiederaufnahme(an); });
        // der Schalter im Player-Panel kann den Wert geaendert haben
        auto* uhr = new QTimer(resume);
        uhr->setInterval(1000);
        connect(uhr, &QTimer::timeout, resume, [resume]() {
            const bool an = lumi::audio::wiederaufnahmeAn();
            if (resume->isChecked() != an)
            {
                const QSignalBlocker ruhe(resume);
                resume->setChecked(an);
            }
        });
        uhr->start();
        layout->addRow(tr("Resume playback:"), resume);
    }

    // Songtexte (S86, Entscheide Patrik 3a/4a): Online-Suche als Opt-in und
    // der Speicherort eines Funds. Beide wirken sofort (QSettings), gelesen
    // werden sie beim naechsten Titelwechsel (audio/SongtextOnline).
    {
        auto* online = new QCheckBox(tr("Enable"), widget);
        online->setObjectName(QStringLiteral("lyricsOnlineSearch"));
        online->setChecked(
            QSettings().value(QStringLiteral("lyrics/onlineSearch"), false).toBool());
        online->setToolTip(
            tr("When no lyrics are found locally, ask lrclib.net. This sends artist, title, "
               "album and duration of the playing track to that service. Off: nothing "
               "leaves this computer."));
        connect(online, &QCheckBox::toggled, this, [](bool an) {
            QSettings().setValue(QStringLiteral("lyrics/onlineSearch"), an);
        });
        layout->addRow(tr("Search lyrics online:"), online);

        auto* ablage = new QComboBox(widget);
        ablage->setObjectName(QStringLiteral("lyricsSaveLocation"));
        ablage->addItems({tr("User folder"), tr("Next to the song")});
        ablage->setCurrentIndex(
            QSettings().value(QStringLiteral("lyrics/saveLocation"), 0).toInt() == 1 ? 1 : 0);
        ablage->setToolTip(
            tr("Where lyrics fetched online are saved. User folder: %1. Next to the song: "
               "same name as the song with .lrc or .txt — if that folder is not writable, "
               "the user folder is used.")
                .arg(QDir::toNativeSeparators(lumi::audio::songtextOrdner())));
        connect(ablage, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [](int index) {
                    QSettings().setValue(QStringLiteral("lyrics/saveLocation"), index);
                });
        layout->addRow(tr("Save fetched lyrics:"), ablage);

        // S87: Eintraege, die mit „Wrong lyrics" verworfen wurden — einsehen
        // und wieder freigeben
        auto* sperren = new QPushButton(tr("Show…"), widget);
        sperren->setObjectName(QStringLiteral("lyricsBlockedShow"));
        sperren->setToolTip(
            tr("Lyrics entries you marked as wrong. They are not fetched again; unblock an "
               "entry here to allow it again."));
        connect(sperren, &QPushButton::clicked, this,
                [this]() { zeigeLyricsSperrliste(this); });
        layout->addRow(tr("Blocked lyrics:"), sperren);

        // S88: Titel, zu denen der Dienst nichts hatte, werden eine Weile nicht
        // wieder gefragt — hier laesst sich das zuruecksetzen
        auto* vergessen = new QPushButton(tr("Ask again"), widget);
        vergessen->setObjectName(QStringLiteral("lyricsNotFoundForget"));
        vergessen->setToolTip(
            tr("Tracks without lyrics online are remembered and not asked again for %1 days "
               "(instrumental: %2 days). This forgets them, so the next playback asks again.")
                .arg(lumi::audio::kSongtextMerkerTageNichtGefunden)
                .arg(lumi::audio::kSongtextMerkerTageInstrumental));
        connect(vergessen, &QPushButton::clicked, this, [vergessen]() {
            const int n = lumi::audio::vergissSongtextFehlanzeigen();
            QToolTip::showText(vergessen->mapToGlobal(QPoint(0, vergessen->height())),
                               tr("%1 remembered track(s) forgotten.").arg(n), vergessen);
        });
        layout->addRow(tr("Lyrics not found:"), vergessen);

        /* Karaoke-Ebene: Schalter, Mischart und Staerke stehen seit S89 im Karaoke-Panel. */
    }

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

QWidget* SettingsPanel::createPerformanceTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QFormLayout(widget);
    layout->setSpacing(8);

    // Frame Mode
    m_pFrameModeCombo = new QComboBox(widget);
    m_pFrameModeCombo->addItems({
        tr("Limited (60 FPS)"),
        tr("Unlimited"),
        tr("VSync")
    });
    m_pFrameModeCombo->setToolTip(tr("Frame rate limiting mode"));
    layout->addRow(tr("Frame Mode:"), m_pFrameModeCombo);

    // Target FPS
    m_pTargetFpsSpinBox = new QSpinBox(widget);
    m_pTargetFpsSpinBox->setRange(15, 240);
    m_pTargetFpsSpinBox->setValue(60);
    m_pTargetFpsSpinBox->setSuffix(tr(" FPS"));
    m_pTargetFpsSpinBox->setToolTip(tr("Target frame rate (Limited mode only)"));
    layout->addRow(tr("Target FPS:"), m_pTargetFpsSpinBox);

    // VSync
    m_pVSyncCheckBox = new QCheckBox(tr("Enable"), widget);
    m_pVSyncCheckBox->setToolTip(tr("Synchronize with monitor refresh rate"));
    layout->addRow(tr("VSync:"), m_pVSyncCheckBox);

    // GPU-Praeferenz: der Windows-Eintrag pro Anwendung ist die einzige
    // Steuerung (core/GpuPreference) und greift erst beim Prozessstart —
    // deshalb loest eine Aenderung SOFORT den Neustart aus (Entscheid S61).
    // Die Combo traegt den Registry-Zahlenwert als itemData, damit die
    // Reihenfolge der Eintraege frei bleibt.
    m_pGpuPreferenceCombo = new QComboBox(widget);
    m_pGpuPreferenceCombo->addItem(
        tr("Automatic (Windows decides)"),
        static_cast<int>(GpuPreference::Mode::Automatic));
    m_pGpuPreferenceCombo->addItem(
        tr("High Performance"),
        static_cast<int>(GpuPreference::Mode::HighPerformance));
    m_pGpuPreferenceCombo->addItem(
        tr("Power Saving"),
        static_cast<int>(GpuPreference::Mode::PowerSaving));
    m_pGpuPreferenceCombo->setToolTip(
        tr("Which graphics card renders LumiViz. Stored per application in "
           "Windows; changing it RESTARTS LumiViz immediately."));
    {
        const auto stored = GpuPreference::readForExecutable(nativeExePath());
        const int mode = static_cast<int>(
            stored.value_or(GpuPreference::Mode::Automatic));
        const int idx = m_pGpuPreferenceCombo->findData(mode);
        m_pGpuPreferenceCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    layout->addRow(tr("Graphics Card:"), m_pGpuPreferenceCombo);

    // Tatsaechlich genutzte GPU (GL_RENDERER) — die Wahrheit nach dem Start,
    // nicht die Einstellung. Tooltip listet alle erkannten Karten.
    m_pActiveGpuLabel = new QLabel(activeOpenGlRenderer(), widget);
    m_pActiveGpuLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    {
        QStringList lines;
        for (const auto& gpu : GpuInfo::enumerate())
        {
            lines << QStringLiteral("%1 — %2, %3 MB VRAM")
                         .arg(QString::fromStdString(gpu.name),
                              QString::fromStdString(gpu.typeString()))
                         .arg(gpu.vramMB());
        }
        m_pActiveGpuLabel->setToolTip(
            tr("Detected graphics cards:\n") + lines.join(QLatin1Char('\n')));
    }
    layout->addRow(tr("Active GPU:"), m_pActiveGpuLabel);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

QWidget* SettingsPanel::createPanelsTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QFormLayout(widget);
    layout->setSpacing(8);

    // Effect Browser (bis S82 „Import Browser"): forget the persisted start
    // folder (back to the exe dir, where the bundled presets/ folder lives — S73)
    m_pResetImportDirButton = new QPushButton(tr("Reset Start Folder"), widget);
    m_pResetImportDirButton->setToolTip(
        tr("Forget the Effect Browser's saved folder and start in the "
           "application folder again — that is where the bundled presets/ "
           "folder lives"));
    layout->addRow(tr("Effect Browser:"), m_pResetImportDirButton);

    // AVS-Import: Divisor des automatisch eingefuegten Render-Scale-Knotens.
    // Wirkt NUR im Moment des Imports — danach ist der Knoten im Preset die
    // einzige Wahrheit (SSOT der Kette, Entscheid S47).
    m_pAvsRenderScaleSpinBox = new QSpinBox(widget);
    m_pAvsRenderScaleSpinBox->setRange(1, 8);
    m_pAvsRenderScaleSpinBox->setPrefix(tr("window / "));
    m_pAvsRenderScaleSpinBox->setToolTip(
        tr("AVS imports get a Render Scale node with this divisor (1 = "
           "neutral). Classic Winamp presets use fixed pixel sizes — 2 or 4 "
           "restores the original fullscreen look."));
    {
        QSettings settings;
        m_pAvsRenderScaleSpinBox->setValue(
            settings.value(QStringLiteral("import/avsRenderScaleDivisor"), 1).toInt());
    }
    layout->addRow(tr("AVS Import Render Scale:"), m_pAvsRenderScaleSpinBox);

    // Movement-Tabelle (S94): Vorgabe der App ist der Bau ueber mehrere Bilder
    // verteilt (kein Stillstand); „in einem Stueck" ist das Verhalten des
    // Originals und das der Vergleichslaeufe. Wirkt sofort auf jede Tabelle,
    // die ab jetzt entsteht.
    {
        auto* tabelle = new QComboBox(widget);
        tabelle->addItem(tr("Spread over frames (no freeze)"));
        tabelle->addItem(tr("At once (like original AVS)"));
        tabelle->setCurrentIndex(
            QSettings().value(QLatin1String(MultiEffectVisualizer::kMovementEinstellung), false).toBool()
                ? 1
                : 0);
        tabelle->setToolTip(
            tr("The Movement effect evaluates its script once per pixel and stores the result "
               "in a table — about half a second per Movement in Full HD.\n\n"
               "Spread over frames: the table is built a few milliseconds per frame; until it "
               "is ready the effect uses a coarse 96 x 72 grid, so the first second or two "
               "look slightly different. Scripts that read reg, megabuf, rand or audio see "
               "these values over several frames instead of one instant.\n\n"
               "At once: the picture stands still while the table is built, exactly like "
               "original AVS. Choose this to compare a preset with the original."));
        connect(tabelle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [](int index) {
            QSettings().setValue(QLatin1String(MultiEffectVisualizer::kMovementEinstellung),
                                 index == 1);
            MultiEffectVisualizer::setMovementScheibeMs(
                index == 1 ? 0.0 : MultiEffectVisualizer::kMovementScheibeAppMs);
        });
        layout->addRow(tr("Movement Table:"), tabelle);
    }

    // Composer-Naht N9 (docs/ui/Composer_Konzept.md §12.2): was Open
    // Composition tut und was am Ende einer Komposition geschieht (S95).
    // Beides liest die Sitzung des Composers, wenn es so weit ist.
    {
        using lumi::composer::AmEnde;
        using lumi::composer::Sitzung;
        auto* oeffnen = new QComboBox(widget);
        oeffnen->setObjectName(QStringLiteral("composerOpenBehaviour"));
        oeffnen->addItem(tr("Starts playing"));
        oeffnen->addItem(tr("Holds at the start"));
        oeffnen->setCurrentIndex(Sitzung::oeffnenHaelt() ? 1 : 0);
        oeffnen->setToolTip(
            tr("What File → Open Composition... does after loading.\n\n"
               "Starts playing: the Composer is switched on and the composition runs at once.\n\n"
               "Holds at the start: the Composer is switched on and waits at position zero — "
               "press Play in the Composer panel."));
        connect(oeffnen, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [](int index) { Sitzung::setOeffnenHaelt(index == 1); });
        layout->addRow(tr("Open Composition:"), oeffnen);

        auto* ende = new QComboBox(widget);
        ende->setObjectName(QStringLiteral("composerAtEnd"));
        ende->addItem(tr("Hold at the end"));
        ende->addItem(tr("Return to the start"));
        ende->addItem(tr("Switch the Composer off"));
        ende->setCurrentIndex(static_cast<int>(Sitzung::amEnde()));
        ende->setToolTip(
            tr("What happens when a composition reaches its end and Loop is off.\n\n"
               "Hold at the end: the picture stays black, the karaoke layer keeps running, "
               "the Composer stays on. Play starts again from the beginning.\n\n"
               "Return to the start: jumps to position zero and waits there, like Stop.\n\n"
               "Switch the Composer off: the effect chain delivers the picture again; the "
               "composition stays loaded in the Composer panel."));
        connect(ende, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [](int index) { Sitzung::setAmEnde(static_cast<AmEnde>(index)); });
        layout->addRow(tr("End of a Composition:"), ende);

        // Wechsel zwischen Player und Composer (Wunsch Patrik S98)
        auto* playerWeiter = new QCheckBox(tr("Player resumes when the Composer hands the audio back"), widget);
        playerWeiter->setObjectName(QStringLiteral("composerPlayerResumes"));
        playerWeiter->setChecked(Sitzung::playerSpieltWeiter());
        playerWeiter->setToolTip(
            tr("A composition with an audio channel pauses the player while it plays.\n\n"
               "On: if the player was playing when the Composer took over, it continues by "
               "itself when the Composer is switched off (or its last audio channel is disabled).\n\n"
               "Off: the player stays paused — press Play to continue."));
        connect(playerWeiter, &QCheckBox::toggled, this,
                [](bool an) { Sitzung::setPlayerSpieltWeiter(an); });
        layout->addRow(tr("Composer off:"), playerWeiter);

        auto* nachEinWeiter = new QCheckBox(tr("Composition continues when the Composer is switched on again"), widget);
        nachEinWeiter->setObjectName(QStringLiteral("composerResumesWhenSwitchedOn"));
        nachEinWeiter->setChecked(Sitzung::laeuftNachEinWeiter());
        nachEinWeiter->setToolTip(
            tr("On: if the composition was playing when the Composer was switched off, it "
               "continues by itself when the Composer is switched on again.\n\n"
               "Off: Composer on waits at the remembered position — press Play to continue.\n\n"
               "A newly loaded composition follows \"Open Composition\" above."));
        connect(nachEinWeiter, &QCheckBox::toggled, this,
                [](bool an) { Sitzung::setLaeuftNachEinWeiter(an); });
        layout->addRow(tr("Composer on:"), nachEinWeiter);

        // Laenge eines hineingezogenen Bild-Clips (Wunsch Patrik S98)
        auto* clipLaenge = new QSpinBox(widget);
        clipLaenge->setObjectName(QStringLiteral("composerNewClipSeconds"));
        clipLaenge->setRange(1, 600);
        clipLaenge->setSuffix(tr(" s"));
        clipLaenge->setValue(static_cast<int>(Sitzung::neuerClipMs() / 1000));
        clipLaenge->setToolTip(
            tr("How long a preset becomes when it is dragged into a composition.\n\n"
               "In a gap it still ends at the next clip. Songs keep their own length."));
        connect(clipLaenge, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [](int sekunden) { Sitzung::setNeuerClipSekunden(sekunden); });
        layout->addRow(tr("New clip length:"), clipLaenge);

        // Mitlaufen der Zeitachse mit der Abspielmarke (S102, Entscheid Patrik ZO6)
        auto* mitlaufen = new QComboBox(widget);
        mitlaufen->setObjectName(QStringLiteral("composerScrollWithPlayheadMode"));
        mitlaufen->addItem(tr("Smooth"));
        mitlaufen->addItem(tr("By page"));
        mitlaufen->setCurrentIndex(Sitzung::mitlaufenSeitenweise() ? 1 : 0);
        mitlaufen->setToolTip(
            tr("How the Composer timeline follows the playhead while a composition plays and the "
               "time is zoomed in (switch \"Scroll with playhead\" in the Composer panel).\n\n"
               "Smooth: the playhead stays in the middle and the clips move under it.\n\n"
               "By page: the view stands still until the playhead leaves it, then turns one page."));
        connect(mitlaufen, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [](int index) { Sitzung::setMitlaufenSeitenweise(index == 1); });
        layout->addRow(tr("Scroll with playhead:"), mitlaufen);

        // Sichttest-Steuerung (S105, Entscheid Patrik L1: a mit Einstellung, Vorgabe aus).
        // Nur, wenn die Steuerung neben der Exe liegt: im Release-Paket fehlt der
        // Tester, der Haken haette dort keine Wirkung (Entscheid Patrik S105).
        if (lumi::sichttest::vorhanden())
        {
            auto* sichttestSpielt = new QCheckBox(tr("A loaded template starts playing"), widget);
            sichttestSpielt->setObjectName(QStringLiteral("composerSichttestPlaysAfterLoad"));
            sichttestSpielt->setChecked(Sitzung::sichttestSpieltNachLaden());
            sichttestSpielt->setToolTip(
                tr("Only for LumiViz started with --testing, when the tester Sichttest loads a "
                   "composition for a test section.\n\n"
                   "Off: the composition is loaded and waits — Composer off, position zero, nothing "
                   "plays. Every test section starts from the same state.\n\n"
                   "On: after loading, the Composer is switched on and the composition plays at once."));
            connect(sichttestSpielt, &QCheckBox::toggled, this,
                    [](bool an) { Sitzung::setSichttestSpieltNachLaden(an); });
            layout->addRow(tr("Sichttest:"), sichttestSpielt);
        }
    }

    // MilkDrop-Puffer-Wechsel (S66): App-Default fuer Milkdrop-Nodes, deren
    // eigener Schalter auf "App-Einstellung" steht. Behalten = Original-
    // Semantik (jedes Preset erbt das Bild des Vorgaengers).
    m_pMilkPufferWechselCombo = new QComboBox(widget);
    m_pMilkPufferWechselCombo->addItem(tr("Keep image (original)"),
                                       QStringLiteral("behalten"));
    m_pMilkPufferWechselCombo->addItem(tr("Clear (fresh start)"),
                                       QStringLiteral("loeschen"));
    m_pMilkPufferWechselCombo->addItem(tr("Fade (one-time mix, see %)"),
                                       QStringLiteral("fading"));
    m_pMilkPufferWechselCombo->addItem(tr("Fade out (over time, see s)"),
                                       QStringLiteral("ausblenden"));
    m_pMilkPufferWechselCombo->setToolTip(
        tr("What happens to the inherited feedback image when a MilkDrop "
           "preset is replaced by another one. Keep = original MilkDrop "
           "behavior (presets inherit the predecessor's image). Clear = "
           "fresh deterministic start like after app launch. Fade = mix of "
           "inherited image and fresh seed. Nodes can override this per "
           "node in the Effect Chain editor."));
    {
        QSettings settings;
        const QString stored =
            settings.value(QStringLiteral("milkdrop/pufferWechsel"),
                           QStringLiteral("behalten"))
                .toString();
        const int idx = m_pMilkPufferWechselCombo->findData(stored);
        m_pMilkPufferWechselCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    layout->addRow(tr("MilkDrop Preset Switch:"), m_pMilkPufferWechselCombo);

    m_pMilkPufferFadingSpinBox = new QSpinBox(widget);
    m_pMilkPufferFadingSpinBox->setRange(0, 100);
    m_pMilkPufferFadingSpinBox->setSuffix(tr(" %"));
    m_pMilkPufferFadingSpinBox->setToolTip(
        tr("Fade mode only: how much of the inherited image survives "
           "(0 % = like Clear, 100 % = like Keep)."));
    {
        QSettings settings;
        m_pMilkPufferFadingSpinBox->setValue(
            settings.value(QStringLiteral("milkdrop/pufferFadingProzent"), 50)
                .toInt());
    }
    m_pMilkPufferFadingSpinBox->setEnabled(
        m_pMilkPufferWechselCombo->currentData().toString() ==
        QLatin1String("fading"));
    layout->addRow(tr("MilkDrop Fade Amount:"), m_pMilkPufferFadingSpinBox);

    m_pMilkPufferAusblendSpinBox = new QDoubleSpinBox(widget);
    m_pMilkPufferAusblendSpinBox->setRange(0.1, 60.0);
    m_pMilkPufferAusblendSpinBox->setSingleStep(0.5);
    m_pMilkPufferAusblendSpinBox->setSuffix(tr(" s"));
    m_pMilkPufferAusblendSpinBox->setToolTip(
        tr("Fade-out mode only: how long until the inherited image has "
           "completely died away (fresh drawing of the new preset stays)."));
    {
        QSettings settings;
        m_pMilkPufferAusblendSpinBox->setValue(
            settings.value(QStringLiteral("milkdrop/pufferAusblendSek"), 2.0)
                .toDouble());
    }
    m_pMilkPufferAusblendSpinBox->setEnabled(
        m_pMilkPufferWechselCombo->currentData().toString() ==
        QLatin1String("ausblenden"));
    layout->addRow(tr("MilkDrop Fade-out Time:"), m_pMilkPufferAusblendSpinBox);

    // Sicht-Blende (S67): versteckt die Rausch-Saat (Kaltstart/Clear) hinter
    // einer kurzen Schwarz-Einblendung — rein kosmetisch, Dynamik unberuehrt.
    m_pMilkSichtBlendeCheckBox = new QCheckBox(tr("Enable"), widget);
    m_pMilkSichtBlendeCheckBox->setToolTip(
        tr("Briefly fades in from black (~0.5 s) whenever a MilkDrop buffer "
           "starts from the fresh noise seed (app start, resize, Clear "
           "switch) — hides the visible noise. Purely cosmetic: the preset "
           "dynamics and the seed energy are unaffected. Takes effect the "
           "next time a preset or chain is loaded."));
    {
        QSettings settings;
        m_pMilkSichtBlendeCheckBox->setChecked(
            settings.value(QStringLiteral("milkdrop/sichtBlende"), true).toBool());
    }
    layout->addRow(tr("MilkDrop Start Fade-in:"), m_pMilkSichtBlendeCheckBox);

    // Bilder-Suchordner (Vorgabe S50, umgesetzt S53): AVS legt seine Bilder im
    // AVS-Wurzelverzeichnis ab, die Presets aber in Unterordnern. Der Import
    // sucht bereits drei Ebenen aufwaerts — dieser Ordner ist die letzte
    // Zuflucht, wenn die Bilder ganz woanders liegen.
    // Seit S116 (Wunsch Patrik) eine Liste, und sie gilt auch fuer MilkDrop:
    // Texturen und Sprites, die beim Preset nicht liegen (core/Suchordner.hpp).
    {
        auto* feld = new QWidget(widget);
        auto* spalte = new QVBoxLayout(feld);
        spalte->setContentsMargins(0, 0, 0, 0);
        m_pSearchDirList = new QListWidget(feld);
        m_pSearchDirList->setObjectName(QStringLiteral("settingsSearchDirs"));
        m_pSearchDirList->setMaximumHeight(m_pSearchDirList->fontMetrics().lineSpacing() * 5 + 8);
        m_pSearchDirList->setToolTip(
            tr("Folders searched for the images of AVS presets (Texer, Picture, AVI) and for the textures "
               "and sprites of MilkDrop presets, when they are not found next to the preset or in the "
               "folders above it. Searched from top to bottom; in each folder also in \"textures\" and "
               "\"sprites\" for MilkDrop. Takes effect the next time a preset is loaded. The first folder "
               "is also where the image chooser of the Effect Chain editor starts."));
        spalte->addWidget(m_pSearchDirList);
        auto* reihe = new QHBoxLayout;
        m_pSearchDirAddButton = new QPushButton(tr("Add..."), feld);
        m_pSearchDirAddButton->setObjectName(QStringLiteral("settingsSearchDirAdd"));
        m_pSearchDirRemoveButton = new QPushButton(tr("Remove"), feld);
        m_pSearchDirRemoveButton->setObjectName(QStringLiteral("settingsSearchDirRemove"));
        m_pSearchDirUpButton = new QPushButton(QStringLiteral("▲"), feld);
        m_pSearchDirUpButton->setObjectName(QStringLiteral("settingsSearchDirUp"));
        m_pSearchDirUpButton->setToolTip(tr("Search this folder earlier"));
        m_pSearchDirUpButton->setFixedWidth(30);
        m_pSearchDirDownButton = new QPushButton(QStringLiteral("▼"), feld);
        m_pSearchDirDownButton->setObjectName(QStringLiteral("settingsSearchDirDown"));
        m_pSearchDirDownButton->setToolTip(tr("Search this folder later"));
        m_pSearchDirDownButton->setFixedWidth(30);
        reihe->addWidget(m_pSearchDirAddButton);
        reihe->addWidget(m_pSearchDirRemoveButton);
        reihe->addWidget(m_pSearchDirUpButton);
        reihe->addWidget(m_pSearchDirDownButton);
        reihe->addStretch(1);
        spalte->addLayout(reihe);
        fuelleSuchordner();
        layout->addRow(tr("Image and Texture Search Folders:"), feld);
    }

    // Benutzerdaten-Ordner im Dateimanager oeffnen. Dort landen die eigenen
    // Knoten-Voreinstellungen (`nodepresets/<typkey>/`, S53) — ohne diesen Knopf
    // muesste man den Pfad raten.
    m_pOpenAppDataButton = new QPushButton(tr("Open Folder"), widget);
    m_pOpenAppDataButton->setToolTip(
        tr("Open the folder holding your own data (node presets, …) in the file "
           "manager. It is created if it does not exist yet."));
    // Export und Import der eigenen Daten (Datenablage_Konzept.md §4a, S114):
    // die drei Knoepfe stehen in einer Zeile.
    m_pExportDataButton = new QPushButton(tr("Export..."), widget);
    m_pExportDataButton->setObjectName(QStringLiteral("userDataExport"));
    m_pExportDataButton->setToolTip(
        tr("Write your own presets, templates, trails and fades — all or some — into one file "
           "(.lvdata) to keep or to carry to another computer."));
    m_pImportDataButton = new QPushButton(tr("Import..."), widget);
    m_pImportDataButton->setObjectName(QStringLiteral("userDataImport"));
    m_pImportDataButton->setToolTip(
        tr("Read a .lvdata file. You choose what to take and what happens when a name already exists."));
    auto* userDataRow = new QWidget(widget);
    auto* userDataLayout = new QHBoxLayout(userDataRow);
    userDataLayout->setContentsMargins(0, 0, 0, 0);
    userDataLayout->addWidget(m_pOpenAppDataButton);
    userDataLayout->addWidget(m_pExportDataButton);
    userDataLayout->addWidget(m_pImportDataButton);
    userDataLayout->addStretch(1);
    layout->addRow(tr("User Data:"), userDataRow);

    // Loop, Stufe 11a (Entscheid S2 a, Patrik S115): was mit einer Tondatei
    // geschieht, die im Instrument-Creator ueber „File..." gewaehlt wird. Der
    // Dialog dort merkt die Wahl mit „Don't ask again" — hier laesst sie sich
    // aendern und zuruecknehmen.
    {
        auto* wahl = new QComboBox(widget);
        wahl->setObjectName(QStringLiteral("sampleCopyChoice"));
        wahl->addItem(tr("Ask each time"), static_cast<int>(lumi::loop::SampleKopie::Fragen));
        wahl->addItem(tr("Copy to My samples"), static_cast<int>(lumi::loop::SampleKopie::Kopieren));
        wahl->addItem(tr("Use it where it is"), static_cast<int>(lumi::loop::SampleKopie::AmOrt));
        wahl->setCurrentIndex(wahl->findData(static_cast<int>(lumi::loop::sampleKopieWahl())));
        wahl->setToolTip(
            tr("What happens to a sound file chosen with File... in the Instrument Creator. A copy "
               "in My samples goes along with an export and stays when the original moves; without "
               "a copy the instrument names the file where it lies."));
        connect(wahl, QOverload<int>::of(&QComboBox::activated), widget, [wahl](int index) {
            lumi::loop::setzeSampleKopieWahl(static_cast<lumi::loop::SampleKopie>(wahl->itemData(index).toInt()));
        });
        layout->addRow(tr("Chosen Sample Files:"), wahl);
    }

    // Mitgeliefertes aus dem Projektordner statt aus der Kopie neben der Exe
    // (Datenablage_Konzept.md, D9 a mit Schalter; Patrik S114). Die Zeile gibt
    // es nur im Entwicklungsbaum: neben einer weitergegebenen Exe liegt kein
    // Projekt, beide Reihenfolgen laesen dasselbe.
    if (lumi::core::mitgeliefert::projektVorhanden())
    {
        m_pBundledFromProjectCheckBox = new QCheckBox(tr("Read from the project folder first"), widget);
        m_pBundledFromProjectCheckBox->setObjectName(QStringLiteral("bundledFromProjectFirst"));
        m_pBundledFromProjectCheckBox->setToolTip(
            tr("Off (default): built-in presets, templates, instruments and icons are read from the "
               "copy next to the executable — what a shipped build reads; a file changed under "
               "asset/ needs a build first. On: they are read straight from the project folder. "
               "Takes effect after a restart."));
        m_pBundledFromProjectCheckBox->setChecked(lumi::core::mitgeliefert::projektZuerst());
        layout->addRow(tr("Bundled Data:"), m_pBundledFromProjectCheckBox);
    }

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

QWidget* SettingsPanel::createHotkeyTab()
{
    auto* widget = new QWidget(this);
    auto* outer = new QVBoxLayout(widget);
    outer->setSpacing(8);

    auto* manager = services().tryResolve<ShortcutManager>();
    if (manager == nullptr)
    {
        outer->addWidget(new QLabel(tr("Hotkey layer not available."), widget));
        return widget;
    }

    m_pHotkeyStatusLabel = new QLabel(widget);
    m_pHotkeyStatusLabel->setWordWrap(true);

    // Je Kategorie eine Gruppe — die Reihenfolge folgt der Registry, damit
    // Doku und UI dieselbe Ordnung zeigen (docs/ui/Hotkey_Konzept.md §4).
    auto* grid = new QGridLayout();
    grid->setColumnStretch(0, 1);
    int row = 0;
    auto lastCategory = std::optional<lumi::services::ShortcutCategory>{};

    for (const lumi::services::ShortcutAction& action : lumi::services::shortcutActions())
    {
        if (!lastCategory.has_value() || *lastCategory != action.category)
        {
            lastCategory = action.category;
            const QString title = QString::fromUtf8(
                lumi::services::shortcutCategoryLabel(action.category).data(),
                static_cast<int>(
                    lumi::services::shortcutCategoryLabel(action.category).size()));
            auto* header = new QLabel(QStringLiteral("— %1 —").arg(title), widget);
            grid->addWidget(header, row++, 0, 1, 3);
        }

        const QString id = QString::fromUtf8(action.id.data(),
                                             static_cast<int>(action.id.size()));
        QString label = QString::fromUtf8(action.label.data(),
                                          static_cast<int>(action.label.size()));
        if (!action.wired) label += tr("  (reserved, no function yet)");
        grid->addWidget(new QLabel(label, widget), row, 0);

        auto* edit = new QKeySequenceEdit(manager->sequenceFor(id), widget);
        edit->setMaximumSequenceLength(1);
        grid->addWidget(edit, row, 1);

        auto* reset = new QPushButton(tr("Default"), widget);
        reset->setToolTip(tr("Back to %1")
                              .arg(ShortcutManager::defaultSequenceFor(id).toString(
                                  QKeySequence::NativeText)));
        grid->addWidget(reset, row, 2);

        // Uebernehmen erst, wenn die Aufnahme fertig ist — sonst wuerde jede
        // Zwischenstufe geprueft und abgelehnt.
        connect(edit, &QKeySequenceEdit::editingFinished, this,
                [this, manager, id, edit]() {
                    const QString error = manager->setSequence(id, edit->keySequence());
                    if (error.isEmpty())
                    {
                        m_pHotkeyStatusLabel->setText(tr("Saved."));
                        return;
                    }
                    // Abgelehnt: die alte Belegung zurueckschreiben, damit das
                    // Feld nie etwas zeigt, was nicht gilt.
                    edit->setKeySequence(manager->sequenceFor(id));
                    m_pHotkeyStatusLabel->setText(tr("Not assigned: %1").arg(error));
                });
        connect(reset, &QPushButton::clicked, this, [manager, id, edit]() {
            manager->resetToDefault(id);
            edit->setKeySequence(manager->sequenceFor(id));
        });
        ++row;
    }

    outer->addLayout(grid);

    auto* resetAll = new QPushButton(tr("All to defaults"), widget);
    connect(resetAll, &QPushButton::clicked, this, [this, manager, widget]() {
        manager->resetAllToDefaults();
        // Alle Aufnahmefelder nachziehen (Reihenfolge = Registry-Reihenfolge).
        const auto edits = widget->findChildren<QKeySequenceEdit*>();
        int i = 0;
        for (const lumi::services::ShortcutAction& a : lumi::services::shortcutActions())
        {
            if (i >= edits.size()) break;
            edits.at(i++)->setKeySequence(manager->sequenceFor(
                QString::fromUtf8(a.id.data(), static_cast<int>(a.id.size()))));
        }
        m_pHotkeyStatusLabel->setText(tr("All hotkeys reset to defaults."));
    });
    outer->addWidget(resetAll);
    outer->addWidget(m_pHotkeyStatusLabel);
    outer->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

QWidget* SettingsPanel::createEditorTab()
{
    // Beautify-Format der Skript-Editoren (Offene_Punkte §7, S69): gemeinsamer
    // Block fuer ALLE Skript-Typen (AVS-/Milkdrop-EEL, LumiViz, GLSL/HLSL).
    // SSOT der Defaults ist FormatOptions — gelesen wird der Block von
    // EelScriptEditing::formatOptionsFromSettings() beim Beautify-Klick.
    auto* widget = new QWidget();
    auto* layout = new QFormLayout(widget);

    const lumi::scripting::FormatOptions def;

    m_pEditorIndentSpinBox = new QSpinBox(widget);
    m_pEditorIndentSpinBox->setRange(0, 16);
    m_pEditorIndentSpinBox->setSuffix(tr(" spaces"));
    m_pEditorIndentSpinBox->setToolTip(
        tr("Beautify: spaces per indent level (EEL paren depth, GLSL/HLSL "
           "brace depth)."));
    m_pEditorOpSpacesCheckBox = new QCheckBox(tr("Enable"), widget);
    m_pEditorOpSpacesCheckBox->setToolTip(
        tr("Beautify (EEL only): single spaces around binary operators "
           "(x = 1 + 2). Off = compact (x=1+2). GLSL/HLSL lines keep their "
           "own spacing either way."));
    m_pEditorMaxBlankSpinBox = new QSpinBox(widget);
    m_pEditorMaxBlankSpinBox->setRange(0, 9);
    m_pEditorMaxBlankSpinBox->setToolTip(
        tr("Beautify: consecutive blank lines are clamped to this many."));
    {
        QSettings settings;
        m_pEditorIndentSpinBox->setValue(
            settings.value(QStringLiteral("editor/indentWidth"), def.indentWidth)
                .toInt());
        m_pEditorOpSpacesCheckBox->setChecked(
            settings
                .value(QStringLiteral("editor/spaceAroundOperators"),
                       def.spaceAroundOperators)
                .toBool());
        m_pEditorMaxBlankSpinBox->setValue(
            settings.value(QStringLiteral("editor/maxBlankLines"), def.maxBlankLines)
                .toInt());
    }
    layout->addRow(tr("Beautify Indent Width:"), m_pEditorIndentSpinBox);
    layout->addRow(tr("Spaces around Operators:"), m_pEditorOpSpacesCheckBox);
    layout->addRow(tr("Max. Blank Lines:"), m_pEditorMaxBlankSpinBox);

    auto* hint = new QLabel(
        tr("Applies to the Beautify button of the large script editors in the "
           "Effect Chain panel. Beautify only rewrites whitespace — tokens "
           "and comments survive unchanged."),
        widget);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color:#888"));
    layout->addRow(hint);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

QWidget* SettingsPanel::createKameraTab()
{
    // Kamera-Testaufnahmen (S70, Idee Patrik): kurze Clips benutzerlokal
    // aufnehmen. Dreifacher Nutzen (Offene_Punkte §7): der Windows-
    // Berechtigungsdialog kommt beim bewussten Klick · die Aufnahme laeuft als
    // Datei ueber den VideoFrameCache im Frame-Schritt (deterministisch,
    // sondentauglich) · der videoSource-Knoten bekommt mit "Testaufnahme"
    // eine dritte Betriebsart samt Fallback ohne Geraet.
    auto* widget = new QWidget();
    auto* layout = new QFormLayout(widget);

    m_pKamGeraetCombo = new QComboBox(widget);
    m_pKamGeraetCombo->setToolTip(
        tr("Camera device for the camera clip. Filling this list does not "
           "open a camera — that only happens when you click Record."));
    aktualisiereKamGeraete();
    layout->addRow(tr("Camera:"), m_pKamGeraetCombo);

    auto* suchenBtn = new QPushButton(tr("Rescan Devices"), widget);
    connect(suchenBtn, &QPushButton::clicked, this,
            [this]() { aktualisiereKamGeraete(); });
    layout->addRow(QString(), suchenBtn);

    m_pKamDauerSpin = new QSpinBox(widget);
    m_pKamDauerSpin->setRange(2, 30);
    m_pKamDauerSpin->setSuffix(tr(" s"));
    m_pKamDauerSpin->setValue(
        QSettings().value(QStringLiteral("kamera/aufnahmeDauer"), 5).toInt());
    m_pKamDauerSpin->setToolTip(tr("Length of the camera clip in seconds."));
    layout->addRow(tr("Duration:"), m_pKamDauerSpin);

    m_pKamAufnahmeButton = new QPushButton(tr("● Record Camera Clip"), widget);
    m_pKamAufnahmeButton->setToolTip(
        tr("Starts the camera and records a clip into the per-user folder "
           "(Windows may ask for camera permission). The click also grants "
           "camera access to the Video Source nodes for this app session."));
    connect(m_pKamAufnahmeButton, &QPushButton::clicked, this,
            [this]() { starteTestaufnahme(); });
    layout->addRow(m_pKamAufnahmeButton);

    m_pKamStatusLabel = new QLabel(widget);
    m_pKamStatusLabel->setWordWrap(true);
    layout->addRow(m_pKamStatusLabel);

    m_pKamListe = new QListWidget(widget);
    m_pKamListe->setFixedHeight(120);
    m_pKamListe->setToolTip(
        tr("Existing camera clips — selectable in the Video Source node "
           "under Source 'Camera Clip'."));
    aktualisiereTestaufnahmenListe();
    layout->addRow(tr("Camera clips:"), m_pKamListe);

    auto* loeschenBtn = new QPushButton(tr("Delete Selected Clip"), widget);
    connect(loeschenBtn, &QPushButton::clicked, this, [this]() {
        auto* item = m_pKamListe != nullptr ? m_pKamListe->currentItem() : nullptr;
        if (item == nullptr) return;
        const QString datei =
            lumi::services::LiveVideoFeed::testaufnahmenOrdner() +
            QStringLiteral("/") + item->text();
        if (QMessageBox::question(
                this, tr("Delete Camera Clip"),
                tr("Really delete '%1'?").arg(item->text())) !=
            QMessageBox::Yes)
            return;
        QFile::remove(datei);
        aktualisiereTestaufnahmenListe();
    });
    auto* ordnerBtn = new QPushButton(tr("Open Folder"), widget);
    ordnerBtn->setToolTip(tr("Opens the per-user camera clips folder "
                             "in the file manager."));
    connect(ordnerBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(
            lumi::services::LiveVideoFeed::testaufnahmenOrdner()));
    });
    auto* knopfZeile = new QWidget(widget);
    auto* knopfLayout = new QHBoxLayout(knopfZeile);
    knopfLayout->setContentsMargins(0, 0, 0, 0);
    knopfLayout->addWidget(loeschenBtn);
    knopfLayout->addWidget(ordnerBtn);
    layout->addRow(knopfZeile);

    auto* hint = new QLabel(
        tr("Camera clips are stored per user (not in the project) and serve "
           "the Video Source node as a deterministic camera substitute — also "
           "as a fallback when no camera is connected."),
        widget);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color:#888"));
    layout->addRow(hint);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

void SettingsPanel::aktualisiereKamGeraete()
{
    if (m_pKamGeraetCombo == nullptr) return;
    const QString vorher = m_pKamGeraetCombo->currentData().toString();
    m_pKamGeraetCombo->clear();
    m_pKamGeraetCombo->addItem(tr("— none —"), QString());
    const auto geraete = QMediaDevices::videoInputs();
    for (const QCameraDevice& d : geraete)
    {
        m_pKamGeraetCombo->addItem(d.description(), QString::fromUtf8(d.id()));
        if (QString::fromUtf8(d.id()) == vorher)
            m_pKamGeraetCombo->setCurrentIndex(m_pKamGeraetCombo->count() - 1);
    }
}

void SettingsPanel::aktualisiereTestaufnahmenListe()
{
    if (m_pKamListe == nullptr) return;
    m_pKamListe->clear();
    const QDir ordner(lumi::services::LiveVideoFeed::testaufnahmenOrdner());
    const auto eintraege = ordner.entryList(QDir::Files, QDir::Name);
    for (const QString& name : eintraege) m_pKamListe->addItem(name);
}

void SettingsPanel::starteTestaufnahme()
{
    if (m_pKamRecorder != nullptr) return;  // eine Aufnahme laeuft schon
    const QString id = m_pKamGeraetCombo->currentData().toString();
    if (id.isEmpty())
    {
        m_pKamStatusLabel->setText(tr("⚠ Please select a camera first."));
        return;
    }
    QCameraDevice geraet;
    const auto geraete = QMediaDevices::videoInputs();
    for (const QCameraDevice& d : geraete)
    {
        if (QString::fromUtf8(d.id()) == id)
        {
            geraet = d;
            break;
        }
    }
    if (geraet.isNull())
    {
        m_pKamStatusLabel->setText(tr("⚠ Camera no longer available — "
                                      "rescan devices."));
        return;
    }

    // DIE ausdrueckliche Nutzeraktion (Vertrag Offene_Punkte §7): ab jetzt
    // duerfen auch die videoSource-Knoten die Kamera dieses App-Laufs starten.
    auto& feeds = lumi::services::LiveVideoFeed::instance();
    feeds.erlaubeKamera();
    // Diese Aufnahme ist eine EIGENE Qt-Multimedia-Pipeline neben dem Dienst
    // — sie muss sich melden, sonst greift der Notausgang in
    // Application::shutdown nach einer Testaufnahme nicht (Befund S71).
    feeds.merkeFremdenFeed();

    const int sek = m_pKamDauerSpin->value();
    QSettings().setValue(QStringLiteral("kamera/aufnahmeDauer"), sek);
    // Dateiname englisch seit S84 (Entscheid 2a); vorhandene `testaufnahme_*`
    // bleiben gueltig — der Knoten merkt sich den Dateinamen, und der Ordner
    // (`testaufnahmen`) bleibt derselbe.
    const QString datei =
        lumi::services::LiveVideoFeed::testaufnahmenOrdner() +
        QStringLiteral("/camera_clip_") +
        QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")) +
        QStringLiteral(".mp4");

    m_pKamera = new QCamera(geraet, this);
    m_pKamSession = new QMediaCaptureSession(this);
    m_pKamRecorder = new QMediaRecorder(this);
    m_pKamSession->setCamera(m_pKamera);
    m_pKamSession->setRecorder(m_pKamRecorder);
    m_pKamRecorder->setOutputLocation(QUrl::fromLocalFile(datei));
    QMediaFormat format(QMediaFormat::MPEG4);
    format.setVideoCodec(QMediaFormat::VideoCodec::H264);
    m_pKamRecorder->setMediaFormat(format);
    connect(m_pKamRecorder, &QMediaRecorder::errorOccurred, this,
            [this](QMediaRecorder::Error, const QString& text) {
                m_pKamStatusLabel->setText(
                    tr("⚠ Recording failed: %1").arg(text));
                beendeTestaufnahme();
            });
    connect(m_pKamRecorder, &QMediaRecorder::recorderStateChanged, this,
            [this](QMediaRecorder::RecorderState zustand) {
                if (zustand == QMediaRecorder::StoppedState)
                    beendeTestaufnahme();
            });
    m_pKamera->start();
    m_pKamRecorder->record();
    m_pKamStatusLabel->setText(tr("● Recording (%1 s)…").arg(sek));
    m_pKamAufnahmeButton->setEnabled(false);
    QTimer::singleShot(sek * 1000, this, [this]() {
        if (m_pKamRecorder != nullptr) m_pKamRecorder->stop();
    });
}

void SettingsPanel::beendeTestaufnahme(bool synchron)
{
    if (m_pKamRecorder == nullptr) return;  // schon abgeraeumt (Fehler-Pfad)
    m_pKamRecorder->disconnect(this);  // gegen Reentry aus recorderStateChanged
    if (m_pKamRecorder->recorderState() != QMediaRecorder::StoppedState)
        m_pKamRecorder->stop();
    if (m_pKamera != nullptr) m_pKamera->stop();
    // Die Session zuerst von ihren Teilnehmern loesen, damit die Zerstoerung
    // in keiner Reihenfolge auf ein halb abgebautes Gespann trifft.
    if (m_pKamSession != nullptr)
    {
        m_pKamSession->setRecorder(nullptr);
        m_pKamSession->setCamera(nullptr);
    }
    if (synchron)
    {
        // Herunterfahren: keine Event-Loop mehr fuer deleteLater (s. Header)
        delete m_pKamRecorder;
        delete m_pKamSession;
        delete m_pKamera;
    }
    else
    {
        m_pKamRecorder->deleteLater();
        if (m_pKamSession != nullptr) m_pKamSession->deleteLater();
        if (m_pKamera != nullptr) m_pKamera->deleteLater();
    }
    m_pKamRecorder = nullptr;
    m_pKamSession = nullptr;
    m_pKamera = nullptr;
    if (synchron) return;  // beim Beenden keine UI mehr anfassen
    m_pKamAufnahmeButton->setEnabled(true);
    if (!m_pKamStatusLabel->text().startsWith(QStringLiteral("⚠")))
        m_pKamStatusLabel->setText(tr("✓ Camera clip saved."));
    aktualisiereTestaufnahmenListe();
}

void SettingsPanel::setupConnections()
{
    // Eine laufende Testaufnahme wird VOR dem Fenster-/GL-Abbau geordnet
    // beendet: sonst sterben Kamera/Session/Recorder als QObject-Kinder
    // mitten im ~MainWindow — die Konstellation, die den Teardown vergiftet
    // (Befund S71). Synchron, weil es hier keine Event-Loop mehr gibt.
    if (auto* app = QCoreApplication::instance())
    {
        connect(app, &QCoreApplication::aboutToQuit, this,
                [this] { beendeTestaufnahme(true); });
    }
    connect(m_pAudioDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPanel::onAudioDeviceChanged);
    connect(m_pFrameModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPanel::onFrameModeChanged);
    connect(m_pGpuPreferenceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPanel::onGpuPreferenceChanged);
    connect(m_pTargetFpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsPanel::onTargetFpsChanged);
    connect(m_pVSyncCheckBox, &QCheckBox::toggled,
            this, &SettingsPanel::onVSyncChanged);
    connect(m_pResetImportDirButton, &QPushButton::clicked,
            this, &SettingsPanel::onResetImportBrowserDir);
    connect(m_pOpenAppDataButton, &QPushButton::clicked,
            this, &SettingsPanel::onOpenAppDataDir);
    connect(m_pSearchDirAddButton, &QPushButton::clicked, this, &SettingsPanel::onAddSearchDir);
    connect(m_pSearchDirRemoveButton, &QPushButton::clicked, this, &SettingsPanel::onRemoveSearchDir);
    connect(m_pSearchDirUpButton, &QPushButton::clicked, this, [this] { onMoveSearchDir(-1); });
    connect(m_pSearchDirDownButton, &QPushButton::clicked, this, [this] { onMoveSearchDir(1); });
    // eine andere Zeile gewaehlt: nur die Knoepfe nachziehen (die Liste nicht neu fuellen —
    // sie meldet gerade selbst)
    connect(m_pSearchDirList, &QListWidget::currentRowChanged, this, [this](int zeile) {
        m_pSearchDirRemoveButton->setEnabled(zeile >= 0);
        m_pSearchDirUpButton->setEnabled(zeile > 0);
        m_pSearchDirDownButton->setEnabled(zeile >= 0 && zeile < m_pSearchDirList->count() - 1);
    });
    connect(m_pExportDataButton, &QPushButton::clicked, this, [this] { lumi::ui::zeigeDatenExport(this); });
    connect(m_pImportDataButton, &QPushButton::clicked, this,
            [this] { lumi::ui::zeigeDatenImport(this, services().tryResolve<IEventBus>()); });
    if (m_pBundledFromProjectCheckBox != nullptr)
        connect(m_pBundledFromProjectCheckBox, &QCheckBox::toggled, this,
                [](bool checked) { lumi::core::mitgeliefert::setzeProjektZuerst(checked); });
    connect(m_pAvsRenderScaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [](int value) {
                QSettings settings;
                settings.setValue(QStringLiteral("import/avsRenderScaleDivisor"),
                                  value);
            });
    connect(m_pMilkPufferWechselCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                if (index < 0) return;
                const QString key =
                    m_pMilkPufferWechselCombo->itemData(index).toString();
                QSettings settings;
                settings.setValue(QStringLiteral("milkdrop/pufferWechsel"), key);
                m_pMilkPufferFadingSpinBox->setEnabled(
                    key == QLatin1String("fading"));
                m_pMilkPufferAusblendSpinBox->setEnabled(
                    key == QLatin1String("ausblenden"));
            });
    connect(m_pMilkPufferFadingSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged), this, [](int value) {
                QSettings settings;
                settings.setValue(
                    QStringLiteral("milkdrop/pufferFadingProzent"), value);
            });
    connect(m_pMilkPufferAusblendSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [](double value) {
                QSettings settings;
                settings.setValue(QStringLiteral("milkdrop/pufferAusblendSek"),
                                  value);
            });

    connect(m_pMilkSichtBlendeCheckBox, &QCheckBox::toggled, this,
            [](bool checked) {
                QSettings settings;
                settings.setValue(QStringLiteral("milkdrop/sichtBlende"), checked);
            });

    // Editor-Tab (Beautify-Format, S69) — Werte greifen beim naechsten
    // Beautify-Klick (formatOptionsFromSettings liest je Aufruf frisch).
    connect(m_pEditorIndentSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [](int value) {
                QSettings settings;
                settings.setValue(QStringLiteral("editor/indentWidth"), value);
            });
    connect(m_pEditorOpSpacesCheckBox, &QCheckBox::toggled, this,
            [](bool checked) {
                QSettings settings;
                settings.setValue(QStringLiteral("editor/spaceAroundOperators"),
                                  checked);
            });
    connect(m_pEditorMaxBlankSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [](int value) {
                QSettings settings;
                settings.setValue(QStringLiteral("editor/maxBlankLines"), value);
            });
}

void SettingsPanel::populateAudioDevices()
{
    m_pAudioDeviceCombo->clear();
    m_pAudioDeviceCombo->addItem(tr("Default Device"), -1);
    
    // Get devices from audio engine
    auto* audioEngine = services().tryResolve<IAudioEngine>();
    if (audioEngine != nullptr)
    {
        auto devices = audioEngine->getDevices();
        for (const auto& device : devices)
        {
            m_pAudioDeviceCombo->addItem(device.name, device.id);
        }
    }
    
    BasicLogger::logDebug("SettingsPanel: Populated " + 
                          std::to_string(m_pAudioDeviceCombo->count()) + " audio devices");
}
