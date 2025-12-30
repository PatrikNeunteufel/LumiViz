/**
 ****************************************************************************************
 * @file   ConfigPanel.cpp
 * @brief  ConfigPanel implementation (Stub)
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/panels/ConfigPanel.hpp"
#include "services/PanelRegistry.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>

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

void ConfigPanel::saveState()
{
    PanelBase::saveState();
    // TODO: Save settings
}

void ConfigPanel::restoreState()
{
    PanelBase::restoreState();
    // TODO: Restore settings
}

// =============================================================================
// Slots
// =============================================================================

void ConfigPanel::onAudioDeviceChanged(int index)
{
    Q_UNUSED(index)
    // TODO: Change audio device via service
}

void ConfigPanel::onFrameModeChanged(int index)
{
    Q_UNUSED(index)
    // TODO: Change frame mode via EventBus
}

void ConfigPanel::onSmoothingChanged(int value)
{
    Q_UNUSED(value)
    // TODO: Update smoothing via EventBus
}

// =============================================================================
// Private Methods
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

    // Audio Device
    m_pAudioDeviceCombo = new QComboBox(widget);
    layout->addRow(tr("Device:"), m_pAudioDeviceCombo);

    // Buffer Size
    m_pBufferSizeSpinBox = new QSpinBox(widget);
    m_pBufferSizeSpinBox->setRange(256, 8192);
    m_pBufferSizeSpinBox->setSingleStep(256);
    m_pBufferSizeSpinBox->setValue(1024);
    m_pBufferSizeSpinBox->setSuffix(tr(" samples"));
    layout->addRow(tr("Buffer Size:"), m_pBufferSizeSpinBox);

    // Sample Rate
    m_pSampleRateSpinBox = new QSpinBox(widget);
    m_pSampleRateSpinBox->setRange(22050, 192000);
    m_pSampleRateSpinBox->setSingleStep(1000);
    m_pSampleRateSpinBox->setValue(44100);
    m_pSampleRateSpinBox->setSuffix(tr(" Hz"));
    layout->addRow(tr("Sample Rate:"), m_pSampleRateSpinBox);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

QWidget* ConfigPanel::createVisualsTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QFormLayout(widget);

    // Smoothing
    auto* smoothingWidget = new QWidget(widget);
    auto* smoothingLayout = new QHBoxLayout(smoothingWidget);
    smoothingLayout->setContentsMargins(0, 0, 0, 0);

    m_pSmoothingSlider = new QSlider(Qt::Horizontal, smoothingWidget);
    m_pSmoothingSlider->setRange(0, 100);
    m_pSmoothingSlider->setValue(50);
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
    layout->addRow(tr("Color Scheme:"), m_pColorSchemeCombo);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    return widget;
}

QWidget* ConfigPanel::createPerformanceTab()
{
    auto* widget = new QWidget(this);
    auto* layout = new QFormLayout(widget);

    // Frame Mode
    m_pFrameModeCombo = new QComboBox(widget);
    m_pFrameModeCombo->addItems({
        tr("Limited (60 FPS)"),
        tr("Unlimited"),
        tr("VSync")
    });
    layout->addRow(tr("Frame Mode:"), m_pFrameModeCombo);

    // Target FPS
    m_pTargetFpsSpinBox = new QSpinBox(widget);
    m_pTargetFpsSpinBox->setRange(15, 240);
    m_pTargetFpsSpinBox->setValue(60);
    m_pTargetFpsSpinBox->setSuffix(tr(" FPS"));
    layout->addRow(tr("Target FPS:"), m_pTargetFpsSpinBox);

    // VSync
    m_pVSyncCheckBox = new QCheckBox(tr("Enable"), widget);
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
    connect(m_pSmoothingSlider, &QSlider::valueChanged,
            this, &ConfigPanel::onSmoothingChanged);
}

void ConfigPanel::populateAudioDevices()
{
    m_pAudioDeviceCombo->clear();
    m_pAudioDeviceCombo->addItem(tr("Default Device"));
    // TODO: Enumerate actual audio devices
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================

REGISTER_PANEL("config", "Settings", false, ConfigPanel)
