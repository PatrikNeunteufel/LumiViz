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
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"
#include "audio/IAudioEngine.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QSettings>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

#include <BasicLogger.h>

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

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

void SettingsPanel::setupConnections()
{
    connect(m_pAudioDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPanel::onAudioDeviceChanged);
    connect(m_pFrameModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPanel::onFrameModeChanged);
    connect(m_pTargetFpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsPanel::onTargetFpsChanged);
    connect(m_pVSyncCheckBox, &QCheckBox::toggled,
            this, &SettingsPanel::onVSyncChanged);
    connect(m_pResetImportDirButton, &QPushButton::clicked,
            this, &SettingsPanel::onResetImportBrowserDir);
    connect(m_pAvsRenderScaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [](int value) {
                QSettings settings;
                settings.setValue(QStringLiteral("import/avsRenderScaleDivisor"),
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
