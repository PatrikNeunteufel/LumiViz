/**
 ****************************************************************************************
 * @file   DockManager.cpp
 * @brief  Qt-ADS Docking Manager Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// =============================================================================
// Includes
// =============================================================================

#include "pch.h"
#include "UI/managers/DockManager.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "services/ServiceContainer.hpp"

// Qt-ADS
#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>
#include <DockAreaTitleBar.h>

// Qt
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QSettings>

// BasicLogger
#include <BasicLogger.h>

// =============================================================================
// Private Implementation
// =============================================================================

struct DockManager::Impl
{
    ServiceContainer* pServices{nullptr};
    QMainWindow* pMainWindow{nullptr};
    ads::CDockManager* pAdsDockManager{nullptr};
    
    // Track all visualizers for batch operations
    std::vector<VisualizerWidget*> visualizers;
    
    // Counter for unique dock widget names
    int visualizerCounter{0};
    
    // Default layout state (for reset)
    QByteArray defaultState;
};

// =============================================================================
// Helper Functions
// =============================================================================

namespace
{

/**
 * @brief Converts DockPosition enum to Qt-ADS DockWidgetArea.
 */
ads::DockWidgetArea positionToDockArea(DockPosition pos)
{
    switch (pos)
    {
        case DockPosition::Left:    return ads::LeftDockWidgetArea;
        case DockPosition::Right:   return ads::RightDockWidgetArea;
        case DockPosition::Top:     return ads::TopDockWidgetArea;
        case DockPosition::Bottom:  return ads::BottomDockWidgetArea;
        case DockPosition::Center:  return ads::CenterDockWidgetArea;
        case DockPosition::Floating:return ads::NoDockWidgetArea;  // Special case
        default:                    return ads::CenterDockWidgetArea;
    }
}

} // anonymous namespace

// =============================================================================
// Construction / Destruction
// =============================================================================

DockManager::DockManager(ServiceContainer& services, QMainWindow* pMainWindow)
    : QObject(pMainWindow)
    , m_impl(std::make_unique<Impl>())
{
    BasicLogger::logDebug("DockManager constructor");
    
    m_impl->pServices = &services;
    m_impl->pMainWindow = pMainWindow;
    
    // -------------------------------------------------------------------------
    // Qt-ADS Configuration
    // -------------------------------------------------------------------------
    // Configure Qt-ADS features before creating the DockManager
    
    ads::CDockManager::setConfigFlags(
        ads::CDockManager::DefaultOpaqueConfig |
        ads::CDockManager::DockAreaHasTabsMenuButton |
        ads::CDockManager::DockAreaHasUndockButton |
        ads::CDockManager::DockAreaCloseButtonClosesTab |
        ads::CDockManager::TabCloseButtonIsToolButton |
        ads::CDockManager::AllTabsHaveCloseButton |
        ads::CDockManager::OpaqueSplitterResize |
        ads::CDockManager::FocusHighlighting
    );
    
    // Auto-hide feature (sidebar)
    ads::CDockManager::setAutoHideConfigFlags(
        ads::CDockManager::DefaultAutoHideConfig
    );
    
    // -------------------------------------------------------------------------
    // Create Qt-ADS DockManager
    // -------------------------------------------------------------------------
    
    m_impl->pAdsDockManager = new ads::CDockManager(pMainWindow);
    
    // Connect signals
    connect(m_impl->pAdsDockManager, &ads::CDockManager::dockWidgetRemoved,
            this, [this](ads::CDockWidget* pDock) {
                emit dockWidgetClosed(pDock->objectName());
            });
    
    // Note: stateChanged signal doesn't exist in Qt-ADS
    // Use perspectiveListChanged or dockAreasAdded/Removed instead
    connect(m_impl->pAdsDockManager, &ads::CDockManager::perspectiveListChanged,
            this, &DockManager::layoutChanged);
    
    BasicLogger::logDebug("  Qt-ADS DockManager created");
}

DockManager::~DockManager()
{
    BasicLogger::logDebug("DockManager destructor");
    
    // Clear tracking lists first
    m_impl->visualizers.clear();
    
    // Qt-ADS DockManager is owned by MainWindow (parent-child)
    // But we should disconnect signals to avoid callbacks during destruction
    if (m_impl->pAdsDockManager != nullptr)
    {
        m_impl->pAdsDockManager->disconnect(this);
    }
}

// =============================================================================
// Visualizer Creation
// =============================================================================

VisualizerWidget* DockManager::createVisualizer(
    const QString& title,
    DockPosition position)
{
    BasicLogger::logDebug("Creating visualizer: " + title.toStdString());
    
    // Create the OpenGL widget with ServiceContainer
    auto* pVisualizer = new VisualizerWidget(*m_impl->pServices);
    
    // Create dock widget wrapper (Qt-ADS 4.4.x Factory API)
    auto* pDock = m_impl->pAdsDockManager->createDockWidget(title);
    pDock->setWidget(pVisualizer);
    pDock->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetFloatable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetMovable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetClosable, true);
    
    // Track the visualizer
    m_impl->visualizers.push_back(pVisualizer);
    m_impl->visualizerCounter++;
    
    // Handle close - remove from tracking
    connect(pDock, &ads::CDockWidget::closed, this, [this, pVisualizer]() {
        auto& v = m_impl->visualizers;
        v.erase(std::remove(v.begin(), v.end(), pVisualizer), v.end());
    });
    
    // Add to dock manager
    if (position == DockPosition::Floating)
    {
        m_impl->pAdsDockManager->addDockWidgetFloating(pDock);
    }
    else
    {
        ads::DockWidgetArea area = positionToDockArea(position);
        m_impl->pAdsDockManager->addDockWidget(area, pDock);
    }
    
    // Save default state after first widget
    if (m_impl->visualizerCounter == 1)
    {
        m_impl->defaultState = m_impl->pAdsDockManager->saveState();
    }
    
    emit visualizerCreated(pVisualizer);
    
    BasicLogger::logDebug("  Visualizer created at position: " + 
                          std::to_string(static_cast<int>(position)));
    
    return pVisualizer;
}

VisualizerWidget* DockManager::createVisualizerRelativeTo(
    const QString& title,
    DockPosition position,
    ads::CDockWidget* pReference)
{
    BasicLogger::logDebug("Creating visualizer relative: " + title.toStdString());
    
    // Create the OpenGL widget with ServiceContainer
    auto* pVisualizer = new VisualizerWidget(*m_impl->pServices);
    
    // Create dock widget wrapper (Qt-ADS 4.4.x Factory API)
    auto* pDock = m_impl->pAdsDockManager->createDockWidget(title);
    pDock->setWidget(pVisualizer);
    pDock->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetFloatable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetMovable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetClosable, true);
    
    // Track the visualizer
    m_impl->visualizers.push_back(pVisualizer);
    m_impl->visualizerCounter++;
    
    // Handle close
    connect(pDock, &ads::CDockWidget::closed, this, [this, pVisualizer]() {
        auto& v = m_impl->visualizers;
        v.erase(std::remove(v.begin(), v.end(), pVisualizer), v.end());
    });
    
    // Get reference area
    ads::CDockAreaWidget* pArea = pReference ? pReference->dockAreaWidget() : nullptr;
    ads::DockWidgetArea area = positionToDockArea(position);
    
    if (pArea != nullptr && position != DockPosition::Floating)
    {
        // Add relative to reference
        m_impl->pAdsDockManager->addDockWidget(area, pDock, pArea);
    }
    else if (position == DockPosition::Floating)
    {
        m_impl->pAdsDockManager->addDockWidgetFloating(pDock);
    }
    else
    {
        // Fallback: Add to main area
        m_impl->pAdsDockManager->addDockWidget(area, pDock);
    }
    
    emit visualizerCreated(pVisualizer);
    
    return pVisualizer;
}

// =============================================================================
// Generic Dock Widget Creation
// =============================================================================

ads::CDockWidget* DockManager::createDockWidget(
    const QString& title,
    QWidget* pContent,
    DockPosition position)
{
    BasicLogger::logDebug("Creating dock widget: " + title.toStdString());
    
    // Use Qt-ADS 4.4.x Factory API
    auto* pDock = m_impl->pAdsDockManager->createDockWidget(title);
    pDock->setWidget(pContent);
    pDock->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetFloatable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetMovable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetClosable, true);
    
    if (position == DockPosition::Floating)
    {
        m_impl->pAdsDockManager->addDockWidgetFloating(pDock);
    }
    else
    {
        ads::DockWidgetArea area = positionToDockArea(position);
        m_impl->pAdsDockManager->addDockWidget(area, pDock);
    }
    
    return pDock;
}

// =============================================================================
// Dock Widget Access
// =============================================================================

std::vector<VisualizerWidget*> DockManager::visualizers() const
{
    return m_impl->visualizers;
}

ads::CDockWidget* DockManager::dockWidget(const QString& title) const
{
    return m_impl->pAdsDockManager->findDockWidget(title);
}

int DockManager::dockWidgetCount() const
{
    return m_impl->pAdsDockManager->dockWidgetsMap().count();
}

// =============================================================================
// Layout Management
// =============================================================================

QByteArray DockManager::saveState() const
{
    return m_impl->pAdsDockManager->saveState();
}

bool DockManager::restoreState(const QByteArray& state)
{
    if (state.isEmpty())
    {
        return false;
    }
    return m_impl->pAdsDockManager->restoreState(state);
}

void DockManager::savePerspective(const QString& name)
{
    m_impl->pAdsDockManager->addPerspective(name);
    BasicLogger::logDebug("Saved perspective: " + name.toStdString());
}

void DockManager::loadPerspective(const QString& name)
{
    m_impl->pAdsDockManager->openPerspective(name);
    BasicLogger::logDebug("Loaded perspective: " + name.toStdString());
}

QStringList DockManager::perspectiveNames() const
{
    return m_impl->pAdsDockManager->perspectiveNames();
}

// =============================================================================
// Menu Integration
// =============================================================================

QMenu* DockManager::createViewMenu(QWidget* pParent)
{
    auto* pMenu = new QMenu(tr("&View"), pParent);
    
    // Add dock widget toggle actions
    auto* pDockWidgetsMenu = pMenu->addMenu(tr("&Panels"));
    
    for (auto* pDock : m_impl->pAdsDockManager->dockWidgetsMap())
    {
        pDockWidgetsMenu->addAction(pDock->toggleViewAction());
    }
    
    pMenu->addSeparator();
    
    // Perspectives submenu
    auto* pPerspectivesMenu = pMenu->addMenu(tr("Perspecti&ves"));
    
    auto* pSaveAction = pPerspectivesMenu->addAction(tr("&Save Current..."));
    connect(pSaveAction, &QAction::triggered, this, [this]() {
        // TODO: Show dialog for perspective name
        savePerspective("Custom");
    });
    
    pPerspectivesMenu->addSeparator();
    
    for (const QString& name : perspectiveNames())
    {
        auto* pAction = pPerspectivesMenu->addAction(name);
        connect(pAction, &QAction::triggered, this, [this, name]() {
            loadPerspective(name);
        });
    }
    
    pMenu->addSeparator();
    
    // Reset layout
    auto* pResetAction = pMenu->addAction(tr("&Reset Layout"));
    connect(pResetAction, &QAction::triggered, this, &DockManager::resetLayout);
    
    return pMenu;
}

void DockManager::populatePanelsMenu(QMenu* pMenu)
{
    if (pMenu == nullptr || m_impl->pAdsDockManager == nullptr)
    {
        return;
    }
    
    // Add toggle actions for each dock widget
    for (auto* pDock : m_impl->pAdsDockManager->dockWidgetsMap())
    {
        if (pDock != nullptr)
        {
            pMenu->addAction(pDock->toggleViewAction());
        }
    }
}

void DockManager::populatePerspectivesMenu(QMenu* pMenu)
{
    if (pMenu == nullptr)
    {
        return;
    }
    
    // Save Current action
    auto* pSaveAction = pMenu->addAction(tr("&Save Current..."));
    connect(pSaveAction, &QAction::triggered, this, [this]() {
        // TODO: Show dialog for perspective name input
        savePerspective("Custom");
    });
    
    // Separator before saved perspectives
    if (!perspectiveNames().isEmpty())
    {
        pMenu->addSeparator();
        
        // Add action for each saved perspective
        for (const QString& name : perspectiveNames())
        {
            auto* pAction = pMenu->addAction(name);
            connect(pAction, &QAction::triggered, this, [this, name]() {
                loadPerspective(name);
            });
        }
    }
}

// =============================================================================
// Qt-ADS Access
// =============================================================================

ads::CDockManager* DockManager::adsDockManager() const noexcept
{
    return m_impl->pAdsDockManager;
}

// =============================================================================
// Slots
// =============================================================================

void DockManager::requestRenderAll()
{
    for (auto* pVisualizer : m_impl->visualizers)
    {
        if (pVisualizer != nullptr)
        {
            pVisualizer->update();
        }
    }
}

void DockManager::resetLayout()
{
    BasicLogger::logDebug("Resetting layout to default");
    
    if (!m_impl->defaultState.isEmpty())
    {
        m_impl->pAdsDockManager->restoreState(m_impl->defaultState);
    }
}

void DockManager::closeAll()
{
    BasicLogger::logDebug("Closing all dock widgets");
    
    // Close all dock widgets
    for (auto* pDock : m_impl->pAdsDockManager->dockWidgetsMap())
    {
        pDock->closeDockWidget();
    }
    
    m_impl->visualizers.clear();
}
