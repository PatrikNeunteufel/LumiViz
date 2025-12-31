/**
 ****************************************************************************************
 * @file   ConfigPanel.cpp
 * @brief  ConfigPanel implementation with service integration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 ****************************************************************************************
 */

#include "UI/panels/ConfigPanel.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"
#include "audio/IAudioEngine.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>

#include <BasicLogger.h>

// =============================================================================
// Construction
// =============================================================================

ConfigPanel::ConfigPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("config"), tr("Settings"), parent)
{
    setupUI();
    setupConnections();
    populateAudioDevices();
}

// =============================================================================
// IPanel Implementation
// =============================================================================

int ConfigPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

// =============================================================================
// Lifecycle
// =============================================================================

void ConfigPanel::onActivate()
{
    subscribeToEvents();
    syncWithCurrentSettings();
}

void ConfigPanel::onDeactivate()
{
    unsubscribeFromEvents();
}

void ConfigPanel::saveState()
{
    PanelBase::saveState();
    // Settings are applied immediately, no need to save here
}

void ConfigPanel::restoreState()
{
    PanelBase::restoreState();
    syncWithCurrentSettings();
}

// =============================================================================
// Event Subscription
// =============================================================================

void ConfigPanel::subscribeToEvents()
{
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        return;
    }
    
    // Listen for frame mode changes from elsewhere
    int id1 = eventBus->subscribe<FrameModeChangedEvent>(
        [this](const FrameModeChangedEvent& e) {
            if (!m_isUpdating)
            {
                m_isUpdating = true;
                m_pFrameModeCombo->setCurrentIndex(e.mode);
                m_isUpdating = false;
            }
        });
    m_subscriptionIds.push_back(id1);
}

void ConfigPanel::unsubscribeFromEvents()
{
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        return;
    }
    
    for (int id : m_subscriptionIds)
    {
        eventBus->unsubscribe(id);
    }
    m_subscriptionIds.clear();
}

void ConfigPanel::syncWithCurrentSettings()
{
    // Sync with audio engine
    auto* audioEngine = services().tryResolve<IAudioEngine>();
    if (audioEngine != nullptr)
    {
        // Update device combo
        // Audio engine doesn't expose current device yet
    }
    
    // TODO: Sync with config/settings service
}

// =============================================================================
// Slots
// =============================================================================

void ConfigPanel::onAudioDeviceChanged(int index)
{
    if (m_isUpdating || index < 0)
    {
        return;
    }
    
    QString deviceName = m_pAudioDeviceCombo->itemData(index).toString();
    
    auto* audioEngine = services().tryResolve<IAudioEngine>();
    if (audioEngine != nullptr)
    {
        // TODO: audioEngine->setOutputDevice(deviceName);
        BasicLogger::logInfo("ConfigPanel: Audio device changed to: " + 
                             deviceName.toStdString());
    }
}

void ConfigPanel::onFrameModeChanged(int index)
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
        
        BasicLogger::logInfo("ConfigPanel: Frame mode changed to: " + 
                             std::to_string(index));
    }
    
    // Enable/disable target FPS based on mode
    bool isLimited = (index == 0);  // Limited mode
    m_pTargetFpsSpinBox->setEnabled(isLimited);
}

void ConfigPanel::onTargetFpsChanged(int value)
{
    if (m_isUpdating)
    {
        return;
    }
    
    // TODO: Publish target FPS change event
    BasicLogger::logDebug("ConfigPanel: Target FPS changed to: " + 
                          std::to_string(value));
}

void ConfigPanel::onVSyncChanged(bool checked)
{
    if (m_isUpdating)
    {
        return;
    }
    
    // VSync checkbox changes frame mode to VSync
    if (checked)
    {
        m_pFrameModeCombo->setCurrentIndex(2);  // VSync mode
    }
    
    BasicLogger::logDebug("ConfigPanel: VSync " + 
                          std::string(checked ? "enabled" : "disabled"));
}

void ConfigPanel::onSmoothingChanged(int value)
{
    if (m_isUpdating)
    {
        return;
    }
    
    // TODO: Publish smoothing change event for visualizers
    float smoothing = static_cast<float>(value) / 100.0f;
    Q_UNUSED(smoothing);
    
    BasicLogger::logDebug("ConfigPanel: Smoothing changed to: " + 
                          std::to_string(value) + "%");
}

// =============================================================================
// UI Setup
// =============================================================================

void ConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->addTab(createAudioTab(), tr("Audio"));
    m_pTabWidget->addTab(createVisualsTab(), tr("Visuals"));
    m_pTabWidget->addTab(createPerformanceTab(), tr("Performance"));

    mainLayout->addWidget(m_pTabWidget);
}

QWidget* ConfigPanel::createAudioTab()
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

QWidget* ConfigPanel::createVisualsTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QFormLayout(widget);
    layout->setSpacing(8);

    // Smoothing
    auto* smoothingWidget = new QWidget(widget);
    auto* smoothingLayout = new QHBoxLayout(smoothingWidget);
    smoothingLayout->setContentsMargins(0, 0, 0, 0);

    m_pSmoothingSlider = new QSlider(Qt::Horizontal, smoothingWidget);
    m_pSmoothingSlider->setRange(0, 100);
    m_pSmoothingSlider->setValue(50);
    m_pSmoothingSlider->setToolTip(tr("Smoothing factor for visualization"));
    smoothingLayout->addWidget(m_pSmoothingSlider);

    auto* smoothingLabel = new QLabel(QStringLiteral("50%"), smoothingWidget);
    smoothingLabel->setMinimumWidth(40);
    smoothingLayout->addWidget(smoothingLabel);

    connect(m_pSmoothingSlider, &QSlider::valueChanged, 
            [smoothingLabel](int value) {
                smoothingLabel->setText(QString("%1%").arg(value));
            });

    layout->addRow(tr("Smoothing:"), smoothingWidget);

    // Peak Hold
    m_pPeakHoldCheckBox = new QCheckBox(tr("Enable"), widget);
    m_pPeakHoldCheckBox->setChecked(true);
    m_pPeakHoldCheckBox->setToolTip(tr("Show peak indicators on spectrum"));
    layout->addRow(tr("Peak Hold:"), m_pPeakHoldCheckBox);

    // Color Scheme
    m_pColorSchemeCombo = new QComboBox(widget);
    m_pColorSchemeCombo->addItems({
        tr("Classic"),
        tr("Fire"),
        tr("Ocean"),
        tr("Neon"),
        tr("Monochrome")
    });
    m_pColorSchemeCombo->setToolTip(tr("Color scheme for visualizers"));
    layout->addRow(tr("Color Scheme:"), m_pColorSchemeCombo);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

QWidget* ConfigPanel::createPerformanceTab()
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

void ConfigPanel::setupConnections()
{
    connect(m_pAudioDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConfigPanel::onAudioDeviceChanged);
    connect(m_pFrameModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConfigPanel::onFrameModeChanged);
    connect(m_pTargetFpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ConfigPanel::onTargetFpsChanged);
    connect(m_pVSyncCheckBox, &QCheckBox::toggled,
            this, &ConfigPanel::onVSyncChanged);
    connect(m_pSmoothingSlider, &QSlider::valueChanged,
            this, &ConfigPanel::onSmoothingChanged);
}

void ConfigPanel::populateAudioDevices()
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
    
    BasicLogger::logDebug("ConfigPanel: Populated " + 
                          std::to_string(m_pAudioDeviceCombo->count()) + " audio devices");
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================
// NOTE: Registration is now handled centrally in PanelAutoReg.cpp
// to avoid linker issues with static libraries (dead code elimination).
// The REGISTER_PANEL macro is no longer used here.
