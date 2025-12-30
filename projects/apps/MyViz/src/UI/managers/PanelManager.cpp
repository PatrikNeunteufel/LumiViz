/**
 ****************************************************************************************
 * @file   PanelManager.cpp
 * @brief  PanelManager implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/managers/PanelManager.hpp"
#include "UI/panels/PanelBase.hpp"
#include "services/PanelRegistry.hpp"
#include "services/ServiceContainer.hpp"

#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>

#include <QSettings>

// =============================================================================
// Construction
// =============================================================================

PanelManager::PanelManager(ServiceContainer& services,
                           ads::CDockManager* dockManager,
                           QObject* parent)
    : QObject(parent)
    , m_services(services)
    , m_dockManager(dockManager)
{
}

PanelManager::~PanelManager()
{
    // DockWidgets are owned by DockManager, panels by DockWidgets
    // Just clear our references
    m_panels.clear();
    m_dockWidgets.clear();
}

// =============================================================================
// Panel Creation
// =============================================================================

void PanelManager::createAllPanels()
{
    auto& registry = PanelRegistry::instance();
    auto descriptors = registry.descriptors();

    for (const auto& desc : descriptors)
    {
        QString panelId = QString::fromStdString(desc.id);
        
        auto* dockWidget = createPanel(panelId);
        if (dockWidget != nullptr)
        {
            // Set initial visibility based on descriptor
            if (!desc.defaultVisible)
            {
                dockWidget->closeDockWidget();
            }
        }
    }
}

ads::CDockWidget* PanelManager::createPanel(const QString& panelId)
{
    // Check if already created
    if (m_dockWidgets.contains(panelId))
    {
        return m_dockWidgets.value(panelId);
    }

    auto& registry = PanelRegistry::instance();
    std::string idStd = panelId.toStdString();

    // Check if registered
    if (!registry.has(idStd))
    {
        return nullptr;
    }

    // Get descriptor for metadata
    const auto* desc = registry.descriptor(idStd);
    if (desc == nullptr)
    {
        return nullptr;
    }

    // Create panel widget
    auto panelWidget = registry.create(idStd, m_services);
    if (!panelWidget)
    {
        return nullptr;
    }

    // Cast to PanelBase
    auto* panelBase = qobject_cast<PanelBase*>(panelWidget.get());
    if (panelBase == nullptr)
    {
        return nullptr;
    }

    // Create dock widget
    QString title = QString::fromStdString(desc->title);
    auto* dockWidget = createDockWidget(panelId, panelWidget.release(), title);

    if (dockWidget != nullptr)
    {
        m_panels.insert(panelId, panelBase);
        m_dockWidgets.insert(panelId, dockWidget);

        // Determine dock area from preferredArea()
        ads::DockWidgetArea area = ads::CenterDockWidgetArea;
        int preferred = panelBase->preferredArea();
        
        if (preferred & Qt::LeftDockWidgetArea)
        {
            area = ads::LeftDockWidgetArea;
        }
        else if (preferred & Qt::RightDockWidgetArea)
        {
            area = ads::RightDockWidgetArea;
        }
        else if (preferred & Qt::TopDockWidgetArea)
        {
            area = ads::TopDockWidgetArea;
        }
        else if (preferred & Qt::BottomDockWidgetArea)
        {
            area = ads::BottomDockWidgetArea;
        }

        // Add to dock manager
        m_dockManager->addDockWidget(area, dockWidget);

        Q_EMIT panelCreated(panelId);
    }

    return dockWidget;
}

// =============================================================================
// Panel Access
// =============================================================================

PanelBase* PanelManager::panel(const QString& panelId) const
{
    return m_panels.value(panelId, nullptr);
}

ads::CDockWidget* PanelManager::dockWidget(const QString& panelId) const
{
    return m_dockWidgets.value(panelId, nullptr);
}

QStringList PanelManager::panelIds() const
{
    return m_panels.keys();
}

// =============================================================================
// Panel Visibility
// =============================================================================

bool PanelManager::isPanelVisible(const QString& panelId) const
{
    auto* dock = dockWidget(panelId);
    if (dock == nullptr)
    {
        return false;
    }
    return !dock->isClosed();
}

void PanelManager::showPanel(const QString& panelId)
{
    auto* dock = dockWidget(panelId);
    if (dock != nullptr)
    {
        dock->toggleView(true);
    }
}

void PanelManager::hidePanel(const QString& panelId)
{
    auto* dock = dockWidget(panelId);
    if (dock != nullptr)
    {
        dock->closeDockWidget();
    }
}

void PanelManager::togglePanel(const QString& panelId)
{
    auto* dock = dockWidget(panelId);
    if (dock != nullptr)
    {
        dock->toggleView();
    }
}

// =============================================================================
// State Persistence
// =============================================================================

void PanelManager::saveState()
{
    // Save individual panel states
    for (auto it = m_panels.begin(); it != m_panels.end(); ++it)
    {
        if (it.value() != nullptr)
        {
            it.value()->saveState();
        }
    }
}

void PanelManager::restoreState()
{
    // Restore individual panel states
    for (auto it = m_panels.begin(); it != m_panels.end(); ++it)
    {
        if (it.value() != nullptr)
        {
            it.value()->restoreState();
        }
    }
}

// =============================================================================
// Private Slots
// =============================================================================

void PanelManager::onDockWidgetVisibilityChanged(bool visible)
{
    auto* dock = qobject_cast<ads::CDockWidget*>(sender());
    if (dock == nullptr)
    {
        return;
    }

    // Find panel ID
    QString panelId = m_dockWidgets.key(dock);
    if (!panelId.isEmpty())
    {
        Q_EMIT panelVisibilityChanged(panelId, visible);
    }
}

// =============================================================================
// Private Methods
// =============================================================================

ads::CDockWidget* PanelManager::createDockWidget(const QString& panelId,
                                                  QWidget* content,
                                                  const QString& title)
{
    // Use Qt-ADS 4.4.x Factory API
    auto* dockWidget = m_dockManager->createDockWidget(title);
    dockWidget->setObjectName(panelId);
    dockWidget->setWidget(content);

    // Configure dock widget features
    dockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, true);
    dockWidget->setFeature(ads::CDockWidget::DockWidgetMovable, true);
    dockWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, true);

    // Connect visibility signal
    connect(dockWidget, &ads::CDockWidget::viewToggled,
            this, &PanelManager::onDockWidgetVisibilityChanged);

    return dockWidget;
}
