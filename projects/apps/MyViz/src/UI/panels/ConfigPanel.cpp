/**
 ****************************************************************************************
 * @file   ConfigPanel.cpp
 * @brief  ConfigPanel implementation - Visualizer configuration only
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.1.0
 ****************************************************************************************
 */

#include "UI/panels/ConfigPanel.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>

#include <BasicLogger.h>

// =============================================================================
// Construction
// =============================================================================

ConfigPanel::ConfigPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("config"), tr("Visualizer Config"), parent)
{
    setupUI();
    setupConnections();
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
}

void ConfigPanel::onDeactivate()
{
    unsubscribeFromEvents();
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
    
    // TODO: Subscribe to active visualizer changed events
    // TODO: Subscribe to visualizer config changed events
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

// =============================================================================
// Slots
// =============================================================================

void ConfigPanel::onSmoothingChanged(int value)
{
    if (m_isUpdating)
    {
        return;
    }
    
    m_pSmoothingLabel->setText(QString("%1%").arg(value));
    
    // TODO: Publish smoothing change event for active visualizer
    float smoothing = static_cast<float>(value) / 100.0f;
    Q_UNUSED(smoothing);
    
    BasicLogger::logDebug("ConfigPanel: Smoothing changed to: " + 
                          std::to_string(value) + "%");
}

void ConfigPanel::onPeakHoldChanged(bool checked)
{
    if (m_isUpdating)
    {
        return;
    }
    
    // TODO: Publish peak hold change event for active visualizer
    BasicLogger::logDebug("ConfigPanel: Peak hold " + 
                          std::string(checked ? "enabled" : "disabled"));
}

void ConfigPanel::onColorSchemeChanged(int index)
{
    if (m_isUpdating || index < 0)
    {
        return;
    }
    
    QString scheme = m_pColorSchemeCombo->currentText();
    
    // TODO: Publish color scheme change event for active visualizer
    BasicLogger::logDebug("ConfigPanel: Color scheme changed to: " + 
                          scheme.toStdString());
}

// =============================================================================
// UI Setup
// =============================================================================

void ConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    // --- Smoothing Section ---
    auto* smoothingGroup = new QGroupBox(tr("Smoothing"), this);
    auto* smoothingLayout = new QHBoxLayout(smoothingGroup);
    
    m_pSmoothingSlider = new QSlider(Qt::Horizontal, smoothingGroup);
    m_pSmoothingSlider->setRange(0, 100);
    m_pSmoothingSlider->setValue(50);
    m_pSmoothingSlider->setToolTip(tr("Smoothing factor for visualization data"));
    smoothingLayout->addWidget(m_pSmoothingSlider);
    
    m_pSmoothingLabel = new QLabel(QStringLiteral("50%"), smoothingGroup);
    m_pSmoothingLabel->setMinimumWidth(40);
    m_pSmoothingLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    smoothingLayout->addWidget(m_pSmoothingLabel);
    
    mainLayout->addWidget(smoothingGroup);

    // --- Display Options Section ---
    auto* displayGroup = new QGroupBox(tr("Display Options"), this);
    auto* displayLayout = new QVBoxLayout(displayGroup);
    
    m_pPeakHoldCheckBox = new QCheckBox(tr("Show Peak Hold"), displayGroup);
    m_pPeakHoldCheckBox->setChecked(true);
    m_pPeakHoldCheckBox->setToolTip(tr("Show peak indicators on spectrum visualizers"));
    displayLayout->addWidget(m_pPeakHoldCheckBox);
    
    mainLayout->addWidget(displayGroup);

    // --- Color Scheme Section ---
    auto* colorGroup = new QGroupBox(tr("Color Scheme"), this);
    auto* colorLayout = new QVBoxLayout(colorGroup);
    
    m_pColorSchemeCombo = new QComboBox(colorGroup);
    m_pColorSchemeCombo->addItems({
        tr("Classic"),
        tr("Fire"),
        tr("Ocean"),
        tr("Neon"),
        tr("Monochrome"),
        tr("Rainbow")
    });
    m_pColorSchemeCombo->setToolTip(tr("Color scheme for the active visualizer"));
    colorLayout->addWidget(m_pColorSchemeCombo);
    
    mainLayout->addWidget(colorGroup);

    // Spacer
    mainLayout->addStretch();
    
    // Info label
    auto* infoLabel = new QLabel(tr("Settings apply to the active visualizer"), this);
    infoLabel->setStyleSheet("color: gray; font-size: 10px;");
    infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);
}

void ConfigPanel::setupConnections()
{
    connect(m_pSmoothingSlider, &QSlider::valueChanged,
            this, &ConfigPanel::onSmoothingChanged);
    connect(m_pPeakHoldCheckBox, &QCheckBox::toggled,
            this, &ConfigPanel::onPeakHoldChanged);
    connect(m_pColorSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConfigPanel::onColorSchemeChanged);
}
