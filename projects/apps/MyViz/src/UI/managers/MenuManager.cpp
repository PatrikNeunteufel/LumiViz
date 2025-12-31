/**
 ****************************************************************************************
 * @file   MenuManager.cpp
 * @brief  MenuManager implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/managers/MenuManager.hpp"
#include "UI/managers/PanelManager.hpp"
#include "services/MenuRegistry.hpp"
#include "services/PanelRegistry.hpp"
#include "services/ServiceContainer.hpp"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QKeySequence>

// =============================================================================
// Construction
// =============================================================================

MenuManager::MenuManager(ServiceContainer& services, QObject* parent)
    : QObject(parent)
    , m_services(services)
{
}

MenuManager::~MenuManager()
{
    // Actions and menus are owned by QMenuBar
    m_menus.clear();
    m_actions.clear();
    m_panelActions.clear();
}

// =============================================================================
// Initialization
// =============================================================================

void MenuManager::buildMenuBar(QMenuBar* menuBar)
{
    if (menuBar == nullptr)
    {
        return;
    }

    m_menuBar = menuBar;
    m_menus.clear();
    m_actions.clear();

    auto& registry = MenuRegistry::instance();

    // Get top-level containers (children of "toplevel")
    auto topLevel = registry.childrenOf(MenuRegistry::rootId());

    for (const auto& node : topLevel)
    {
        if (node.type == MenuNodeType::Container)
        {
            const auto* desc = registry.container(node.id);
            if (desc != nullptr)
            {
                QString menuId = QString::fromStdString(node.id);
                QString title = QString::fromStdString(desc->title);

                QMenu* menu = createMenu(menuId, title, menuBar);
                menuBar->addMenu(menu);

                // Build children recursively
                buildMenuRecursive(menu, menuId);
            }
        }
    }
}

void MenuManager::setPanelManager(PanelManager* panelManager)
{
    // Disconnect old
    if (m_panelManager != nullptr)
    {
        disconnect(m_panelManager, &PanelManager::panelVisibilityChanged,
                   this, &MenuManager::onPanelVisibilityChanged);
    }

    m_panelManager = panelManager;

    // Connect new
    if (m_panelManager != nullptr)
    {
        connect(m_panelManager, &PanelManager::panelVisibilityChanged,
                this, &MenuManager::onPanelVisibilityChanged);
    }
}

// =============================================================================
// Menu Access
// =============================================================================

QMenu* MenuManager::menu(const QString& menuId) const
{
    return m_menus.value(menuId, nullptr);
}

QAction* MenuManager::action(const QString& actionId) const
{
    return m_actions.value(actionId, nullptr);
}

// =============================================================================
// Dynamic Updates
// =============================================================================

void MenuManager::rebuild()
{
    if (m_menuBar != nullptr)
    {
        m_menuBar->clear();
        buildMenuBar(m_menuBar);
    }
}

void MenuManager::updateCheckStates()
{
    if (m_panelManager == nullptr)
    {
        return;
    }

    // Update panel action check states
    for (auto it = m_panelActions.begin(); it != m_panelActions.end(); ++it)
    {
        QString panelId = it.key();
        QAction* action = it.value();

        if (action != nullptr)
        {
            action->setChecked(m_panelManager->isPanelVisible(panelId));
        }
    }

    // Update other checkable items from MenuRegistry
    auto& registry = MenuRegistry::instance();
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it)
    {
        QString actionId = it.key();
        QAction* action = it.value();

        const auto* itemDesc = registry.item(actionId.toStdString());
        if (itemDesc != nullptr && itemDesc->isChecked)
        {
            action->setChecked(itemDesc->isChecked(m_services));
        }
    }
}

// =============================================================================
// Private Slots
// =============================================================================

void MenuManager::onActionTriggered()
{
    auto* action = qobject_cast<QAction*>(sender());
    if (action == nullptr)
    {
        return;
    }

    QString actionId = action->objectName();

    // Check if it's a panel toggle action
    QString panelId = m_panelActions.key(action);
    if (!panelId.isEmpty() && m_panelManager != nullptr)
    {
        m_panelManager->togglePanel(panelId);
        return;
    }

    // Otherwise, execute callback from registry
    auto& registry = MenuRegistry::instance();
    const auto* itemDesc = registry.item(actionId.toStdString());
    if (itemDesc != nullptr && itemDesc->onClick)
    {
        itemDesc->onClick(m_services);
    }

    Q_EMIT actionTriggered(actionId);
}

void MenuManager::onPanelVisibilityChanged(const QString& panelId, bool visible)
{
    auto* action = m_panelActions.value(panelId);
    if (action != nullptr)
    {
        action->setChecked(visible);
    }
}

// =============================================================================
// Private Methods
// =============================================================================

void MenuManager::buildMenuRecursive(QMenu* parentMenu, const QString& parentId)
{
    auto& registry = MenuRegistry::instance();
    auto children = registry.childrenOf(parentId.toStdString());

    int lastGroupOrder = -1;
    
    // Check if parent container is exclusive
    const auto* parentDesc = registry.container(parentId.toStdString());
    QActionGroup* pActionGroup = nullptr;
    if (parentDesc != nullptr && parentDesc->exclusive)
    {
        pActionGroup = new QActionGroup(parentMenu);
        pActionGroup->setExclusive(true);
    }

    for (const auto& node : children)
    {
        QString nodeId = QString::fromStdString(node.id);

        switch (node.type)
        {
        case MenuNodeType::Container:
        {
            const auto* desc = registry.container(node.id);
            if (desc != nullptr)
            {
                QString title = QString::fromStdString(desc->title);

                // Create submenu and recurse
                // Note: "Panels" and "Perspectives" submenus are populated 
                // externally by DockManager::populatePanelsMenu() and 
                // populatePerspectivesMenu() for proper toggle action support.
                QMenu* subMenu = createMenu(nodeId, title, parentMenu);
                parentMenu->addMenu(subMenu);
                buildMenuRecursive(subMenu, nodeId);
            }
            break;
        }

        case MenuNodeType::Group:
        {
            // Add separator if not first item
            if (lastGroupOrder >= 0 && lastGroupOrder != node.order)
            {
                parentMenu->addSeparator();
            }
            lastGroupOrder = node.order;
            break;
        }

        case MenuNodeType::Item:
        {
            QAction* action = createAction(nodeId, parentMenu);
            if (action != nullptr)
            {
                // Add to action group if parent is exclusive
                if (pActionGroup != nullptr)
                {
                    pActionGroup->addAction(action);
                }
                parentMenu->addAction(action);
            }
            break;
        }
        }
    }
}

void MenuManager::buildPanelsMenu(QMenu* panelsMenu)
{
    auto& panelRegistry = PanelRegistry::instance();
    auto descriptors = panelRegistry.descriptors();

    for (const auto& desc : descriptors)
    {
        QString panelId = QString::fromStdString(desc.id);
        QString title = QString::fromStdString(desc.title);

        auto* action = new QAction(title, panelsMenu);
        action->setObjectName("panel." + panelId);
        action->setCheckable(true);

        // Set initial check state
        if (m_panelManager != nullptr)
        {
            action->setChecked(m_panelManager->isPanelVisible(panelId));
        }
        else
        {
            action->setChecked(desc.defaultVisible);
        }

        connect(action, &QAction::triggered, this, &MenuManager::onActionTriggered);

        panelsMenu->addAction(action);
        m_panelActions.insert(panelId, action);
    }
}

QMenu* MenuManager::createMenu(const QString& menuId, const QString& title, QWidget* parent)
{
    auto* menu = new QMenu(title, parent);
    menu->setObjectName(menuId);
    m_menus.insert(menuId, menu);
    return menu;
}

QAction* MenuManager::createAction(const QString& actionId, QWidget* parent)
{
    auto& registry = MenuRegistry::instance();
    const auto* desc = registry.item(actionId.toStdString());

    if (desc == nullptr)
    {
        return nullptr;
    }

    QString title = QString::fromStdString(desc->title);
    auto* action = new QAction(title, parent);
    action->setObjectName(actionId);

    // Shortcut
    if (!desc->shortcut.empty())
    {
        action->setShortcut(QKeySequence(QString::fromStdString(desc->shortcut)));
    }

    // Checkable
    if (desc->isChecked)
    {
        action->setCheckable(true);
        action->setChecked(desc->isChecked(m_services));
    }

    // Enabled state
    if (desc->isEnabled)
    {
        action->setEnabled(desc->isEnabled(m_services));
    }

    connect(action, &QAction::triggered, this, &MenuManager::onActionTriggered);

    m_actions.insert(actionId, action);
    return action;
}
