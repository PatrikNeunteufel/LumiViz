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
#include "UI/managers/ShortcutManager.hpp"
#include "core/GpuInfo.hpp"
#include "core/GpuPreference.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/ShortcutRegistry.hpp"
#include "services/events/UIEvents.hpp"
#include "audio/IAudioEngine.hpp"

#include <QVBoxLayout>
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
#include <QTimer>
#include <QUrl>

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

void SettingsPanel::updateImageSearchDirButton()
{
    if (m_pImageSearchDirButton == nullptr) return;
    QSettings settings;
    const QString dir =
        settings.value(QStringLiteral("import/imageSearchDir")).toString();
    m_pImageSearchDirButton->setText(dir.isEmpty() ? tr("(none) — choose…") : dir);
}

void SettingsPanel::onChooseImageSearchDir()
{
    QSettings settings;
    const QString current =
        settings.value(QStringLiteral("import/imageSearchDir")).toString();
    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Image search folder"), current.isEmpty() ? QDir::homePath() : current);
    if (dir.isEmpty()) return;  // abgebrochen — bestehende Wahl bleibt
    settings.setValue(QStringLiteral("import/imageSearchDir"), dir);
    updateImageSearchDirButton();
    BasicLogger::logInfo("SettingsPanel: image search folder set (" +
                         dir.toStdString() + ")");
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
        tr("Which graphics card renders MyViz. Stored per application in "
           "Windows; changing it RESTARTS MyViz immediately."));
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

    // Import Browser: forget the persisted start folder (back to home)
    m_pResetImportDirButton = new QPushButton(tr("Reset Start Folder"), widget);
    m_pResetImportDirButton->setToolTip(
        tr("Forget the Import Browser's saved folder and start at the home "
           "directory again"));
    layout->addRow(tr("Import Browser:"), m_pResetImportDirButton);

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

    // Bilder-Suchordner (Vorgabe S50, umgesetzt S53): AVS legt seine Bilder im
    // AVS-Wurzelverzeichnis ab, die Presets aber in Unterordnern. Der Import
    // sucht bereits drei Ebenen aufwaerts — dieser Ordner ist die letzte
    // Zuflucht, wenn die Bilder ganz woanders liegen.
    m_pImageSearchDirButton = new QPushButton(widget);
    m_pImageSearchDirButton->setToolTip(
        tr("Extra folder searched for Picture/Texer images when the import finds "
           "none next to the preset. Also the starting folder of the image "
           "chooser in the Effect Chain editor."));
    updateImageSearchDirButton();
    layout->addRow(tr("Image Search Folder:"), m_pImageSearchDirButton);

    // Benutzerdaten-Ordner im Dateimanager oeffnen. Dort landen die eigenen
    // Knoten-Voreinstellungen (`nodepresets/<typkey>/`, S53) — ohne diesen Knopf
    // muesste man den Pfad raten.
    m_pOpenAppDataButton = new QPushButton(tr("Open Folder"), widget);
    m_pOpenAppDataButton->setToolTip(
        tr("Open the folder holding your own data (node presets, …) in the file "
           "manager. It is created if it does not exist yet."));
    layout->addRow(tr("User Data:"), m_pOpenAppDataButton);

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

void SettingsPanel::setupConnections()
{
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
    connect(m_pImageSearchDirButton, &QPushButton::clicked,
            this, &SettingsPanel::onChooseImageSearchDir);
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
