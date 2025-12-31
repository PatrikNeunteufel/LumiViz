/**
 ****************************************************************************************
 * @file   VisualSelectPanel.cpp
 * @brief  VisualSelectPanel implementation with VisualizerRegistry integration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 ****************************************************************************************
 */

#include "UI/panels/VisualSelectPanel.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/VisualizerRegistry.hpp"
#include "services/events/UIEvents.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QSplitter>

#include <BasicLogger.h>

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
        m_selectedVisualizerId.clear();
        m_pDescriptionLabel->setText(tr("Select a visualizer to see details."));
        m_pCategoryLabel->clear();
        m_pApplyButton->setEnabled(false);
        return;
    }

    m_selectedVisualizerId = item->data(Qt::UserRole).toString();
    
    // Get details from registry
    auto& registry = VisualizerRegistry::instance();
    const auto* desc = registry.descriptor(m_selectedVisualizerId.toStdString());
    
    if (desc != nullptr)
    {
        m_pDescriptionLabel->setText(QString::fromStdString(desc->description));
        m_pCategoryLabel->setText(tr("Category: %1").arg(QString::fromStdString(desc->category)));
    }
    else
    {
        m_pDescriptionLabel->setText(item->toolTip());
        m_pCategoryLabel->clear();
    }
    
    m_pApplyButton->setEnabled(true);
    
    Q_EMIT visualizerSelected(m_selectedVisualizerId);
}

void VisualSelectPanel::onApplyClicked()
{
    applySelectedVisualizer();
}

void VisualSelectPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item != nullptr)
    {
        m_selectedVisualizerId = item->data(Qt::UserRole).toString();
        applySelectedVisualizer();
    }
}

void VisualSelectPanel::applySelectedVisualizer()
{
    if (m_selectedVisualizerId.isEmpty())
    {
        return;
    }
    
    // Publish event to change visualizer
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus != nullptr)
    {
        eventBus->publish(ChangeVisualizerEvent{m_selectedVisualizerId.toStdString()});
        BasicLogger::logInfo("VisualSelectPanel: Applied visualizer '" + 
                             m_selectedVisualizerId.toStdString() + "'");
    }
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
    m_pVisualizerList->setIconSize(QSize(32, 32));
    m_pVisualizerList->setSpacing(2);
    m_pVisualizerList->setAlternatingRowColors(true);
    listLayout->addWidget(m_pVisualizerList);

    splitter->addWidget(listGroup);

    // Details section
    auto* detailsGroup = new QGroupBox(tr("Details"), this);
    auto* detailsLayout = new QVBoxLayout(detailsGroup);
    detailsLayout->setContentsMargins(4, 4, 4, 4);

    // Preview placeholder
    m_pPreviewLabel = new QLabel(detailsGroup);
    m_pPreviewLabel->setFixedHeight(60);
    m_pPreviewLabel->setAlignment(Qt::AlignCenter);
    m_pPreviewLabel->setStyleSheet("background-color: #1a1a2e; border-radius: 4px; color: #666;");
    m_pPreviewLabel->setText(tr("Preview"));
    detailsLayout->addWidget(m_pPreviewLabel);

    // Category
    m_pCategoryLabel = new QLabel(detailsGroup);
    m_pCategoryLabel->setStyleSheet("color: gray; font-size: 10px;");
    detailsLayout->addWidget(m_pCategoryLabel);

    // Description
    m_pDescriptionLabel = new QLabel(detailsGroup);
    m_pDescriptionLabel->setWordWrap(true);
    m_pDescriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_pDescriptionLabel->setText(tr("Select a visualizer to see details."));
    m_pDescriptionLabel->setMinimumHeight(40);
    detailsLayout->addWidget(m_pDescriptionLabel);

    // Settings stack (per-visualizer settings - future)
    m_pSettingsStack = new QStackedWidget(detailsGroup);
    m_pSettingsStack->addWidget(new QLabel(tr("No settings available"), m_pSettingsStack));
    m_pSettingsStack->setVisible(false);  // Hide for now
    detailsLayout->addWidget(m_pSettingsStack);

    detailsLayout->addStretch();

    // Apply button
    m_pApplyButton = new QPushButton(tr("Apply"), detailsGroup);
    m_pApplyButton->setEnabled(false);
    m_pApplyButton->setToolTip(tr("Apply selected visualizer to active window"));
    detailsLayout->addWidget(m_pApplyButton);

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
            this, &VisualSelectPanel::onItemDoubleClicked);
    connect(m_pApplyButton, &QPushButton::clicked,
            this, &VisualSelectPanel::onApplyClicked);
}

void VisualSelectPanel::populateVisualizers()
{
    m_pVisualizerList->clear();
    
    // Get visualizers from registry
    auto& registry = VisualizerRegistry::instance();
    auto descriptors = registry.descriptors();
    
    QString currentCategory;
    
    for (const auto& desc : descriptors)
    {
        QString id = QString::fromStdString(desc.id);
        QString name = QString::fromStdString(desc.name);
        QString description = QString::fromStdString(desc.description);
        QString category = QString::fromStdString(desc.category);
        
        // Add category separator if changed
        if (!category.isEmpty() && category != currentCategory)
        {
            currentCategory = category;
            auto* separator = new QListWidgetItem(category, m_pVisualizerList);
            separator->setFlags(Qt::NoItemFlags);
            separator->setBackground(QColor(40, 40, 50));
            separator->setForeground(QColor(150, 150, 150));
            QFont font = separator->font();
            font.setBold(true);
            font.setPointSize(font.pointSize() - 1);
            separator->setFont(font);
        }
        
        auto* item = new QListWidgetItem(name, m_pVisualizerList);
        item->setData(Qt::UserRole, id);
        item->setToolTip(description);
        
        // Add audio indicator if uses audio
        if (desc.usesAudio)
        {
            item->setText(name + " 🎵");
        }
    }
    
    // If no visualizers registered, show placeholder
    if (descriptors.empty())
    {
        auto* placeholder = new QListWidgetItem(tr("No visualizers available"), m_pVisualizerList);
        placeholder->setFlags(Qt::NoItemFlags);
        placeholder->setForeground(QColor(128, 128, 128));
    }
    else
    {
        // Select first selectable item
        for (int i = 0; i < m_pVisualizerList->count(); ++i)
        {
            auto* item = m_pVisualizerList->item(i);
            if (item->flags() & Qt::ItemIsSelectable)
            {
                m_pVisualizerList->setCurrentRow(i);
                break;
            }
        }
    }
    
    BasicLogger::logDebug("VisualSelectPanel: Populated " + 
                          std::to_string(descriptors.size()) + " visualizers");
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================
// NOTE: Registration is now handled centrally in PanelAutoReg.cpp
// to avoid linker issues with static libraries (dead code elimination).
// The REGISTER_PANEL macro is no longer used here.
