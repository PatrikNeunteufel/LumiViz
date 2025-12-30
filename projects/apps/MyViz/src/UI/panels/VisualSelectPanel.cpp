/**
 ****************************************************************************************
 * @file   VisualSelectPanel.cpp
 * @brief  VisualSelectPanel implementation (Stub)
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/panels/VisualSelectPanel.hpp"
#include "services/PanelRegistry.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QGroupBox>
#include <QSplitter>

// =============================================================================
// Construction
// =============================================================================

VisualSelectPanel::VisualSelectPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("visual_select"), tr("Visualizers"), parent)
{
    setupUI();
    setupConnections();
    populateVisualizers();
}

// =============================================================================
// IPanel Implementation
// =============================================================================

int VisualSelectPanel::preferredArea() const
{
    return Qt::LeftDockWidgetArea;
}

// =============================================================================
// Lifecycle
// =============================================================================

void VisualSelectPanel::onActivate()
{
    // Refresh visualizer list from registry
    populateVisualizers();
}

// =============================================================================
// Slots
// =============================================================================

void VisualSelectPanel::onSelectionChanged()
{
    auto* item = m_pVisualizerList->currentItem();
    if (item == nullptr)
    {
        return;
    }

    QString visualizerId = item->data(Qt::UserRole).toString();
    
    // Update preview and description
    // TODO: Get from VisualizerRegistry
    m_pDescriptionLabel->setText(item->toolTip());

    Q_EMIT visualizerSelected(visualizerId);
}

// =============================================================================
// Private Methods
// =============================================================================

void VisualSelectPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // Splitter for list and details
    auto* splitter = new QSplitter(Qt::Vertical, this);

    // Visualizer list
    auto* listGroup = new QGroupBox(tr("Available Visualizers"), this);
    auto* listLayout = new QVBoxLayout(listGroup);
    listLayout->setContentsMargins(4, 4, 4, 4);

    m_pVisualizerList = new QListWidget(listGroup);
    m_pVisualizerList->setIconSize(QSize(48, 48));
    m_pVisualizerList->setSpacing(2);
    listLayout->addWidget(m_pVisualizerList);

    splitter->addWidget(listGroup);

    // Details section
    auto* detailsGroup = new QGroupBox(tr("Details"), this);
    auto* detailsLayout = new QVBoxLayout(detailsGroup);
    detailsLayout->setContentsMargins(4, 4, 4, 4);

    // Preview
    m_pPreviewLabel = new QLabel(detailsGroup);
    m_pPreviewLabel->setFixedHeight(80);
    m_pPreviewLabel->setAlignment(Qt::AlignCenter);
    m_pPreviewLabel->setStyleSheet("background-color: #1a1a2e; border-radius: 4px;");
    m_pPreviewLabel->setText(tr("Preview"));
    detailsLayout->addWidget(m_pPreviewLabel);

    // Description
    m_pDescriptionLabel = new QLabel(detailsGroup);
    m_pDescriptionLabel->setWordWrap(true);
    m_pDescriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_pDescriptionLabel->setText(tr("Select a visualizer to see details."));
    detailsLayout->addWidget(m_pDescriptionLabel);

    // Settings stack (per-visualizer settings)
    m_pSettingsStack = new QStackedWidget(detailsGroup);
    m_pSettingsStack->addWidget(new QLabel(tr("No settings available"), m_pSettingsStack));
    detailsLayout->addWidget(m_pSettingsStack);

    detailsLayout->addStretch();

    splitter->addWidget(detailsGroup);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);
}

void VisualSelectPanel::setupConnections()
{
    connect(m_pVisualizerList, &QListWidget::currentItemChanged,
            this, &VisualSelectPanel::onSelectionChanged);
    connect(m_pVisualizerList, &QListWidget::itemDoubleClicked,
            this, &VisualSelectPanel::onSelectionChanged);
}

void VisualSelectPanel::populateVisualizers()
{
    m_pVisualizerList->clear();

    // TODO: Get from VisualizerRegistry
    // For now, add hardcoded entries

    auto addVisualizer = [this](const QString& id, const QString& name, 
                                 const QString& description) {
        auto* item = new QListWidgetItem(name, m_pVisualizerList);
        item->setData(Qt::UserRole, id);
        item->setToolTip(description);
        // TODO: Set icon from visualizer
    };

    addVisualizer("spectrum", tr("Spectrum Analyzer"),
                  tr("Classic frequency spectrum visualization with bars or lines."));
    
    addVisualizer("waveform", tr("Waveform"),
                  tr("Real-time audio waveform display showing amplitude over time."));
    
    addVisualizer("spectrogram", tr("Spectrogram"),
                  tr("Scrolling frequency-time display with color-coded intensity."));
    
    addVisualizer("circular", tr("Circular Spectrum"),
                  tr("Radial spectrum visualization with customizable geometry."));
    
    addVisualizer("particles", tr("Particle System"),
                  tr("Audio-reactive particle effects driven by frequency bands."));

    // Select first item
    if (m_pVisualizerList->count() > 0)
    {
        m_pVisualizerList->setCurrentRow(0);
    }
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================

REGISTER_PANEL("visual_select", "Visualizers", true, VisualSelectPanel)
