/**
 ****************************************************************************************
 * @file   PanelManager.hpp
 * @brief  Manager for panel instantiation and lifecycle
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: PanelManager
 *
 * Der PanelManager:
 *   - Instantiiert alle registrierten Panels aus PanelRegistry
 *   - Erstellt DockWidgets für Qt-ADS
 *   - Verwaltet Panel-Sichtbarkeit
 *   - Persistiert Panel-Layout
 *
 * ### Verwendung
 *
 * ```cpp
 * // In Application::init()
 * m_panelManager = std::make_unique<PanelManager>(m_services, dockManager);
 * m_panelManager->createAllPanels();
 *
 * // Panel togglen
 * m_panelManager->togglePanel("playlist");
 *
 * // Panel-Sichtbarkeit prüfen
 * bool visible = m_panelManager->isPanelVisible("player");
 * ```
 ****************************************************************************************
 */

#pragma once

#include <QObject>
#include <QString>
#include <QHash>
#include <memory>

// Forward declarations
class ServiceContainer;
class QWidget;
class PanelBase;

namespace ads {
class CDockManager;
class CDockWidget;
}

/**
 * @class PanelManager
 * @brief Manages panel instantiation and lifecycle
 */
class PanelManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct PanelManager
     * @param services ServiceContainer for dependency injection
     * @param dockManager Qt-ADS DockManager
     * @param parent Parent QObject
     */
    explicit PanelManager(ServiceContainer& services,
                          ads::CDockManager* dockManager,
                          QObject* parent = nullptr);

    ~PanelManager() override;

    // =========================================================================
    // Panel Creation
    // =========================================================================

    /**
     * @brief Create all registered panels
     *
     * Iterates through PanelRegistry and creates DockWidgets for each.
     */
    void createAllPanels();

    /**
     * @brief Create a specific panel
     * @param panelId Panel ID from registry
     * @return Created DockWidget or nullptr if failed
     */
    ads::CDockWidget* createPanel(const QString& panelId);

    // =========================================================================
    // Panel Access
    // =========================================================================

    /**
     * @brief Get panel widget by ID
     * @param panelId Panel ID
     * @return Panel widget or nullptr
     */
    [[nodiscard]] PanelBase* panel(const QString& panelId) const;

    /**
     * @brief Get dock widget by panel ID
     * @param panelId Panel ID
     * @return DockWidget or nullptr
     */
    [[nodiscard]] ads::CDockWidget* dockWidget(const QString& panelId) const;

    /**
     * @brief Get all panel IDs
     * @return List of created panel IDs
     */
    [[nodiscard]] QStringList panelIds() const;

    // =========================================================================
    // Panel Visibility
    // =========================================================================

    /**
     * @brief Check if panel is visible
     * @param panelId Panel ID
     */
    [[nodiscard]] bool isPanelVisible(const QString& panelId) const;

    /**
     * @brief Show a panel
     * @param panelId Panel ID
     */
    void showPanel(const QString& panelId);

    /**
     * @brief Hide a panel
     * @param panelId Panel ID
     */
    void hidePanel(const QString& panelId);

    /**
     * @brief Toggle panel visibility
     * @param panelId Panel ID
     */
    void togglePanel(const QString& panelId);

    // =========================================================================
    // State Persistence
    // =========================================================================

    /**
     * @brief Save all panel states
     */
    void saveState();

    /**
     * @brief Restore all panel states
     */
    void restoreState();

Q_SIGNALS:
    /**
     * @brief Emitted when a panel is created
     */
    void panelCreated(const QString& panelId);

    /**
     * @brief Emitted when panel visibility changes
     */
    void panelVisibilityChanged(const QString& panelId, bool visible);

private Q_SLOTS:
    void onDockWidgetVisibilityChanged(bool visible);

private:
    ads::CDockWidget* createDockWidget(const QString& panelId,
                                        QWidget* content,
                                        const QString& title);

    ServiceContainer& m_services;
    ads::CDockManager* m_dockManager = nullptr;

    // Panel ID → Panel widget
    QHash<QString, PanelBase*> m_panels;

    // Panel ID → Dock widget
    QHash<QString, ads::CDockWidget*> m_dockWidgets;
};
