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
#include "UI/managers/PanelManager.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/WidgetRegistry.hpp"
#include "services/events/UIEvents.hpp"

// Qt-ADS
#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>
#include <DockAreaTitleBar.h>
#include <FloatingDockContainer.h>

// Qt
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QCoreApplication>
#include <QTimer>

// STL
#include <algorithm>
#include <unordered_map>

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
    PanelManager* pPanelManager{nullptr};  // Manages all panels
    
    // Track all visualizers for batch operations
    std::vector<VisualizerWidget*> visualizers;
    
    // Map visualizers to their dock widgets (for re-docking after fullscreen)
    std::unordered_map<VisualizerWidget*, ads::CDockWidget*> visualizerToDock;
    
    // Counter for unique dock widget names
    int visualizerCounter{0};
    
    // Default layout state (for reset)
    QByteArray defaultState;

    // Event subscription IDs
    std::vector<int> subscriptionIds;

    // Debounce flag for the native-handle resync after dock layout changes
    bool nativeResyncPending{false};
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
                // Remove visualizer from tracking list if applicable
                QWidget* widget = pDock->widget();
                auto* visualizer = qobject_cast<VisualizerWidget*>(widget);
                if (visualizer != nullptr)
                {
                    auto& v = m_impl->visualizers;
                    v.erase(std::remove(v.begin(), v.end(), visualizer), v.end());
                    BasicLogger::logDebug("Visualizer removed from tracking");
                }
                emit dockWidgetClosed(pDock->objectName());
            });
    
    // Note: stateChanged signal doesn't exist in Qt-ADS
    // Use perspectiveListChanged or dockAreasAdded/Removed instead
    connect(m_impl->pAdsDockManager, &ads::CDockManager::perspectiveListChanged,
            this, &DockManager::layoutChanged);

    // Native handle resync on float/redock (Session 42): reparenting docks
    // leaves embedded native GL windows at stale absolute positions — the
    // Session-31 "doubled bar" ghost, previously only fixed for the
    // fullscreen-exit path. Undocking fires floatingWidgetCreated; redocking
    // destroys the floating container; a drop into a new area fires
    // dockAreaCreated. All three schedule one debounced resync.
    connect(m_impl->pAdsDockManager, &ads::CDockManager::floatingWidgetCreated,
            this, [this](ads::CFloatingDockContainer* pFloating) {
                scheduleNativeResync();
                connect(pFloating, &QObject::destroyed,
                        this, [this]() { scheduleNativeResync(); });
            });
    connect(m_impl->pAdsDockManager, &ads::CDockManager::dockAreaCreated,
            this, [this](ads::CDockAreaWidget*) { scheduleNativeResync(); });
    
    BasicLogger::logDebug("  Qt-ADS DockManager created");
    
    // -------------------------------------------------------------------------
    // Create PanelManager and load all registered panels
    // -------------------------------------------------------------------------
    
    m_impl->pPanelManager = new PanelManager(services, m_impl->pAdsDockManager, this);
    m_impl->pPanelManager->createAllPanels();
    
    BasicLogger::logDebug("  PanelManager created, panels loaded");
    
    // NOTE: Layout restore is NOT done here!
    // MainWindow must call restoreLayout() AFTER creating all widgets (including Visualizer).
    // Otherwise Qt-ADS cannot restore positions for widgets that don't exist yet.
    
    // -------------------------------------------------------------------------
    // Subscribe to Events (dezentral - keine MainWindow Änderungen nötig)
    // -------------------------------------------------------------------------
    
    subscribeToEvents();
    
    // -------------------------------------------------------------------------
    // Connect to aboutToQuit to save layout before destruction
    // -------------------------------------------------------------------------
    // IMPORTANT: We save layout here instead of in destructor because
    // Qt destroys child objects (including ads::CDockManager) before our
    // destructor is called, making it impossible to save state there.
    
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        BasicLogger::logDebug("aboutToQuit - saving layout + panel states");
        // Panel states (e.g. Import Browser folder, playlist) — the panels are
        // still alive here; nothing else calls PanelManager::saveState().
        if (m_impl->pPanelManager != nullptr)
        {
            m_impl->pPanelManager->saveState();
        }
        saveLayoutToSettings();
    });
}

DockManager::~DockManager()
{
    BasicLogger::logDebug("DockManager destructor");
    
    // NOTE: Layout is saved in response to QCoreApplication::aboutToQuit signal,
    // NOT in destructor, because Qt-ADS DockManager might already be destroyed.
    
    // Unsubscribe from events
    unsubscribeFromEvents();
    
    // Clear tracking lists first
    m_impl->visualizers.clear();
    
    // PanelManager is owned by us (parent-child), will be deleted automatically
    
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
    
    // Generate dynamic title
    int vizNumber = m_impl->visualizerCounter + 1;
    QString dynamicTitle = (vizNumber == 1) 
        ? QStringLiteral("Visualizer") 
        : QStringLiteral("Visualizer %1").arg(vizNumber);
    
    // Create dock widget wrapper (Qt-ADS 4.4.x Factory API)
    auto* pDock = m_impl->pAdsDockManager->createDockWidget(dynamicTitle);
    
    // IMPORTANT: Set a fixed objectName for layout persistence!
    // Qt-ADS uses objectName to identify widgets when restoring state.
    // The title can change (e.g., "Visualizer" → "Visualizer: Pulsing"),
    // but the objectName must stay the same.
    QString objectName = (vizNumber == 1)
        ? QStringLiteral("visualizer")
        : QStringLiteral("visualizer_%1").arg(vizNumber);
    pDock->setObjectName(objectName);
    
    pDock->setWidget(pVisualizer);
    pDock->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetFloatable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetMovable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetClosable, true);
    
    // Track the visualizer
    m_impl->visualizers.push_back(pVisualizer);
    m_impl->visualizerToDock[pVisualizer] = pDock;
    m_impl->visualizerCounter++;
    
    // Update dock title when visualizer changes
    connect(pVisualizer, &VisualizerWidget::visualizerChanged, this, 
        [pDock, vizNumber](const QString& vizId) {
            Q_UNUSED(vizId);
            // Get the visualizer widget to retrieve name
            auto* viz = qobject_cast<VisualizerWidget*>(pDock->widget());
            if (viz)
            {
                QString vizName = viz->currentVisualizerName();
                QString newTitle;
                if (vizName.isEmpty())
                {
                    newTitle = (vizNumber == 1) 
                        ? QStringLiteral("Visualizer") 
                        : QStringLiteral("Visualizer %1").arg(vizNumber);
                }
                else
                {
                    newTitle = (vizNumber == 1)
                        ? QStringLiteral("Visualizer: %1").arg(vizName)
                        : QStringLiteral("Visualizer %1: %2").arg(vizNumber).arg(vizName);
                }
                pDock->setWindowTitle(newTitle);
            }
        });
    
    // Handle close - remove from tracking
    connect(pDock, &ads::CDockWidget::closed, this, [this, pVisualizer]() {
        auto& v = m_impl->visualizers;
        v.erase(std::remove(v.begin(), v.end(), pVisualizer), v.end());
        m_impl->visualizerToDock.erase(pVisualizer);
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
    
    // Generate title
    int vizNumber = m_impl->visualizerCounter + 1;
    QString dynamicTitle = title.isEmpty() 
        ? ((vizNumber == 1) 
            ? QStringLiteral("Visualizer") 
            : QStringLiteral("Visualizer %1").arg(vizNumber))
        : title;
    
    // Create dock widget wrapper (Qt-ADS 4.4.x Factory API)
    auto* pDock = m_impl->pAdsDockManager->createDockWidget(dynamicTitle);
    
    // IMPORTANT: Set a fixed objectName for layout persistence!
    QString objectName = (vizNumber == 1)
        ? QStringLiteral("visualizer")
        : QStringLiteral("visualizer_%1").arg(vizNumber);
    pDock->setObjectName(objectName);
    
    pDock->setWidget(pVisualizer);
    pDock->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetFloatable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetMovable, true);
    pDock->setFeature(ads::CDockWidget::DockWidgetClosable, true);
    
    // Track the visualizer
    m_impl->visualizers.push_back(pVisualizer);
    m_impl->visualizerToDock[pVisualizer] = pDock;
    m_impl->visualizerCounter++;
    
    // Handle close
    connect(pDock, &ads::CDockWidget::closed, this, [this, pVisualizer]() {
        auto& v = m_impl->visualizers;
        v.erase(std::remove(v.begin(), v.end(), pVisualizer), v.end());
        m_impl->visualizerToDock.erase(pVisualizer);
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

bool DockManager::restoreLayout()
{
    BasicLogger::logDebug("DockManager::restoreLayout() called");
    
    if (restoreLayoutFromSettings())
    {
        BasicLogger::logInfo("Layout restored from settings");
        return true;
    }
    else
    {
        // No saved layout - apply default visibility and save as default
        if (m_impl->pPanelManager != nullptr)
        {
            m_impl->pPanelManager->applyDefaultVisibility();
        }
        m_impl->defaultState = m_impl->pAdsDockManager->saveState();
        BasicLogger::logInfo("No saved layout - applied defaults");
        return false;
    }
}

// =============================================================================
// Event Subscription (Private)
// =============================================================================

void DockManager::subscribeToEvents()
{
    auto* eventBus = m_impl->pServices->tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        BasicLogger::logWarning("DockManager: EventBus not available");
        return;
    }
    
    // Create Visualizer event
    int id1 = eventBus->subscribe<CreateVisualizerEvent>(
        [this](const CreateVisualizerEvent& e) {
            // Check if multiple instances are allowed via WidgetRegistry
            const auto* desc = WidgetRegistry::instance().descriptor("visualizer");
            if (desc != nullptr && !desc->allowMultiple)
            {
                // Check if visualizer already exists
                if (!m_impl->visualizers.empty())
                {
                    BasicLogger::logInfo("Multiple visualizers not allowed - focusing existing");
                    // Focus existing visualizer instead of creating new one
                    if (m_impl->visualizers[0] != nullptr)
                    {
                        // Find the dock widget containing this visualizer and raise it
                        for (auto* dock : m_impl->pAdsDockManager->dockWidgetsMap())
                        {
                            if (dock->widget() == m_impl->visualizers[0])
                            {
                                dock->raise();
                                break;
                            }
                        }
                    }
                    return;
                }
            }
            
            QString title = e.title.empty() 
                ? QString()  // DockManager will generate dynamic title
                : QString::fromStdString(e.title);
            createVisualizer(title, DockPosition::Center);
        });
    m_impl->subscriptionIds.push_back(id1);
    
    // Reset Layout event
    int id2 = eventBus->subscribe<ResetLayoutEvent>(
        [this](const ResetLayoutEvent& /*e*/) {
            resetLayout();
        });
    m_impl->subscriptionIds.push_back(id2);
    
    // Change Visualizer event - apply to first visualizer
    int id3 = eventBus->subscribe<ChangeVisualizerEvent>(
        [this](const ChangeVisualizerEvent& e) {
            if (m_impl->visualizers.empty())
            {
                BasicLogger::logWarning("No visualizer to change");
                return;
            }
            
            // Apply to first visualizer (TODO: apply to focused one)
            QString vizId = QString::fromStdString(e.visualizerId);
            m_impl->visualizers[0]->setVisualizer(vizId);
            BasicLogger::logInfo("Changed visualizer to: " + e.visualizerId);
        });
    m_impl->subscriptionIds.push_back(id3);
    
    // Toggle Panel event
    int id4 = eventBus->subscribe<TogglePanelEvent>(
        [this](const TogglePanelEvent& e) {
            if (m_impl->pPanelManager != nullptr)
            {
                QString panelId = QString::fromStdString(e.panelId);
                m_impl->pPanelManager->togglePanel(panelId);
            }
        });
    m_impl->subscriptionIds.push_back(id4);
    
    // Save Default Layout event
    int id5 = eventBus->subscribe<SaveDefaultLayoutEvent>(
        [this](const SaveDefaultLayoutEvent& /*e*/) {
            saveDefaultLayout();
        });
    m_impl->subscriptionIds.push_back(id5);
    
    BasicLogger::logDebug("  DockManager subscribed to events");
}

void DockManager::unsubscribeFromEvents()
{
    auto* eventBus = m_impl->pServices->tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        return;
    }
    
    for (int id : m_impl->subscriptionIds)
    {
        eventBus->unsubscribe(id);
    }
    m_impl->subscriptionIds.clear();
}

// =============================================================================
// Layout Persistence (Private)
// =============================================================================

namespace
{
    constexpr const char* SETTINGS_GROUP = "DockManager";
    constexpr const char* SETTINGS_STATE = "State";
    constexpr const char* SETTINGS_PERSPECTIVES = "Perspectives";
    constexpr const char* SETTINGS_VERSION = "Version";
    constexpr int LAYOUT_VERSION = 4;  // Increment when dock widget structure changes
}

bool DockManager::restoreLayoutFromSettings()
{
    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);
    
    // Check version - ignore incompatible old layouts
    int savedVersion = settings.value(SETTINGS_VERSION, 0).toInt();
    if (savedVersion != LAYOUT_VERSION)
    {
        BasicLogger::logInfo("DockManager: Ignoring old layout (version " + 
                             std::to_string(savedVersion) + " != " + 
                             std::to_string(LAYOUT_VERSION) + ")");
        
        // Clear old settings
        settings.remove("");  // Remove all keys in current group
        settings.endGroup();
        
        return false;
    }
    
    QByteArray state = settings.value(SETTINGS_STATE).toByteArray();
    
    if (state.isEmpty())
    {
        settings.endGroup();
        return false;
    }
    
    // Restore dock state
    bool success = m_impl->pAdsDockManager->restoreState(state);
    
    settings.endGroup();
    
    // Also save as default for reset
    if (success)
    {
        m_impl->defaultState = state;
        BasicLogger::logInfo("DockManager: Layout restored successfully");
    }
    else
    {
        BasicLogger::logWarning("DockManager: Failed to restore layout");
    }
    
    return success;
}

void DockManager::saveLayoutToSettings()
{
    if (m_impl->pAdsDockManager == nullptr)
    {
        return;
    }
    
    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);
    
    // Save version for compatibility check
    settings.setValue(SETTINGS_VERSION, LAYOUT_VERSION);
    
    // Save current dock state
    QByteArray state = m_impl->pAdsDockManager->saveState();
    settings.setValue(SETTINGS_STATE, state);
    
    // Save perspective names
    QStringList perspNames = m_impl->pAdsDockManager->perspectiveNames();
    settings.setValue(SETTINGS_PERSPECTIVES, perspNames);
    
    settings.endGroup();
    settings.sync();
    
    BasicLogger::logDebug("DockManager: Layout saved to settings (version " + 
                          std::to_string(LAYOUT_VERSION) + ")");
}

void DockManager::saveDefaultLayout()
{
    m_impl->defaultState = m_impl->pAdsDockManager->saveState();
    BasicLogger::logDebug("DockManager: Default layout captured");
}

void DockManager::scheduleNativeResync()
{
    if (m_impl->nativeResyncPending)
    {
        return;
    }
    m_impl->nativeResyncPending = true;

    QTimer::singleShot(0, this, [this]() {
        m_impl->nativeResyncPending = false;

        // Skip during startup layout construction — nothing is on screen yet
        if (m_impl->pMainWindow == nullptr || !m_impl->pMainWindow->isVisible())
        {
            return;
        }

        for (auto* pVisualizer : m_impl->visualizers)
        {
            // Fullscreen visualizers are top-level and get their resync in
            // MainWindow::exitFullscreen(); hidden ones (closed dock or
            // inactive tab) must not be forced visible by the show() step
            if (pVisualizer == nullptr || pVisualizer->isWindow() ||
                !pVisualizer->isVisible())
            {
                continue;
            }
            pVisualizer->recreateNativeWindow();
        }

        BasicLogger::logDebug(
            "DockManager: native window resync after dock layout change");
    });
}

void DockManager::redockVisualizer(VisualizerWidget* pVisualizer)
{
    if (pVisualizer == nullptr)
    {
        return;
    }
    
    // Find the dock widget for this visualizer
    auto it = m_impl->visualizerToDock.find(pVisualizer);
    if (it == m_impl->visualizerToDock.end())
    {
        BasicLogger::logWarning("DockManager: Cannot redock - visualizer not found in map");
        return;
    }
    
    ads::CDockWidget* pDock = it->second;
    if (pDock == nullptr)
    {
        BasicLogger::logWarning("DockManager: Cannot redock - dock widget is null");
        return;
    }
    
    // Reparent the visualizer back to the dock widget
    pVisualizer->setParent(pDock);
    pDock->setWidget(pVisualizer);
    pVisualizer->show();
    
    // Make sure the dock widget is visible
    pDock->toggleView(true);
    
    BasicLogger::logDebug("DockManager: Visualizer re-docked successfully");
}
