/**
 ****************************************************************************************
 * @file   MenuManager.hpp
 * @brief  Manager for building menus from MenuRegistry
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: MenuManager
 *
 * Der MenuManager:
 *   - Baut QMenuBar aus MenuRegistry auf
 *   - Unterstützt dynamische Updates
 *   - Verknüpft mit PanelManager für View-Menü
 *   - Verknüpft mit DialogManager für Dialoge
 *
 * ### Automatische Menü-Struktur
 *
 * ```
 * MenuBar
 * ├── File
 * │   └── Exit
 * ├── View
 * │   └── Panels (auto-generiert aus PanelRegistry)
 * │       ├── ☑ Player
 * │       ├── ☑ Playlist
 * │       └── ...
 * └── Help
 *     └── About... (F1)
 * ```
 ****************************************************************************************
 */

#pragma once

#include <QObject>
#include <QString>
#include <QHash>

// Forward declarations
class ServiceContainer;
class PanelManager;
class QMenuBar;
class QMenu;
class QAction;

/**
 * @class MenuManager
 * @brief Builds and manages application menus
 */
class MenuManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct MenuManager
     * @param services ServiceContainer for dependency injection
     * @param parent Parent QObject
     */
    explicit MenuManager(ServiceContainer& services, QObject* parent = nullptr);

    ~MenuManager() override;

    // =========================================================================
    // Initialization
    // =========================================================================

    /**
     * @brief Build menu bar from MenuRegistry
     * @param menuBar Target QMenuBar
     */
    void buildMenuBar(QMenuBar* menuBar);

    /**
     * @brief Set PanelManager for View menu integration
     * @param panelManager Panel manager instance
     */
    void setPanelManager(PanelManager* panelManager);

    // =========================================================================
    // Menu Access
    // =========================================================================

    /**
     * @brief Get menu by ID
     * @param menuId Menu ID (e.g., "menu.file")
     * @return QMenu or nullptr
     */
    [[nodiscard]] QMenu* menu(const QString& menuId) const;

    /**
     * @brief Get action by ID
     * @param actionId Action ID (e.g., "menu.file.exit")
     * @return QAction or nullptr
     */
    [[nodiscard]] QAction* action(const QString& actionId) const;

    // =========================================================================
    // Dynamic Updates
    // =========================================================================

    /**
     * @brief Rebuild menus (e.g., after registry change)
     */
    void rebuild();

    /**
     * @brief Update checkable states (e.g., panel visibility)
     */
    void updateCheckStates();

Q_SIGNALS:
    /**
     * @brief Emitted when a menu action is triggered
     */
    void actionTriggered(const QString& actionId);

private Q_SLOTS:
    void onActionTriggered();
    void onPanelVisibilityChanged(const QString& panelId, bool visible);

private:
    void buildMenuRecursive(QMenu* parentMenu, const QString& parentId);
    void buildPanelsMenu(QMenu* panelsMenu);
    QMenu* createMenu(const QString& menuId, const QString& title, QWidget* parent);
    QAction* createAction(const QString& actionId, QWidget* parent);

    ServiceContainer& m_services;
    PanelManager* m_panelManager = nullptr;
    QMenuBar* m_menuBar = nullptr;

    // Menu ID → QMenu
    QHash<QString, QMenu*> m_menus;

    // Action ID → QAction
    QHash<QString, QAction*> m_actions;

    // Panel ID → QAction (for View/Panels menu)
    QHash<QString, QAction*> m_panelActions;
};
