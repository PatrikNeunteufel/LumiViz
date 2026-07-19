/**
 ****************************************************************************************
 * @file   PanelBase.hpp
 * @brief  Base class for all dock panels
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Panel-Basisklasse
 *
 * PanelBase bietet:
 *   - IPanel-Implementierung
 *   - ServiceContainer-Zugriff
 *   - EventBus-Convenience-Methoden
 *   - Automatische State-Persistenz
 *
 * ### Verwendung
 *
 * ```cpp
 * class PlayerPanel : public PanelBase {
 *     Q_OBJECT
 * public:
 *     PlayerPanel(ServiceContainer& services, QWidget* parent = nullptr)
 *         : PanelBase(services, "player", tr("Player"), parent)
 *     {
 *         setupUI();
 *     }
 *
 * protected:
 *     void onActivate() override {
 *         // Start audio monitoring
 *     }
 * };
 *
 * // Self-registration:
 * REGISTER_PANEL("player", "Player", true, PlayerPanel)
 * ```
 ****************************************************************************************
 */

#pragma once

#include "IPanel.hpp"
#include "services/IEventBus.hpp"

#include <QWidget>
#include <QString>

#include <vector>

// Forward declarations
class ServiceContainer;
class IEventBus;

/**
 * @class PanelBase
 * @brief Abstract base class for dock panels
 *
 * Implements IPanel and provides common functionality for all panels.
 */
class PanelBase : public QWidget, public IPanel
{
    Q_OBJECT

public:
    /**
     * @brief Construct a panel
     * @param services ServiceContainer reference
     * @param id Unique panel identifier
     * @param title Display title
     * @param parent Parent widget
     */
    explicit PanelBase(ServiceContainer& services,
                       const QString& id,
                       const QString& title,
                       QWidget* parent = nullptr);

    ~PanelBase() override = default;

    // =========================================================================
    // IPanel Implementation
    // =========================================================================

    [[nodiscard]] QString panelId() const override { return m_panelId; }
    [[nodiscard]] QString panelTitle() const override { return m_title; }

    void saveState() override;
    void restoreState() override;

    // =========================================================================
    // Visibility Tracking
    // =========================================================================

    /**
     * @brief Check if panel is currently active
     * @return true if panel is visible and active
     */
    [[nodiscard]] bool isActive() const { return m_isActive; }

public Q_SLOTS:
    /**
     * @brief Set panel active state
     * @param active true to activate, false to deactivate
     *
     * Called by DockManager when visibility changes.
     */
    void setActive(bool active);

Q_SIGNALS:
    /**
     * @brief Emitted when panel is activated
     */
    void activated();

    /**
     * @brief Emitted when panel is deactivated
     */
    void deactivated();

    /**
     * @brief Emitted when panel requests to be closed
     */
    void closeRequested();

protected:
    /**
     * @brief Access the service container
     */
    [[nodiscard]] ServiceContainer& services() { return m_services; }
    [[nodiscard]] const ServiceContainer& services() const { return m_services; }

    /**
     * @brief Access the event bus (convenience)
     * @return Pointer to IEventBus or nullptr if not registered
     */
    [[nodiscard]] IEventBus* eventBus() const;

    /**
     * @brief Get settings key prefix for this panel
     * @return Key prefix (e.g., "panels/player/")
     */
    [[nodiscard]] QString settingsPrefix() const;

    /**
     * @brief Show event - triggers activation
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief Hide event - triggers deactivation
     */
    void hideEvent(QHideEvent* event) override;

    /**
     * @brief RAII storage for EventBus subscriptions of this panel
     *
     * Push handles from IEventBus::subscribeScoped() here. They unsubscribe
     * automatically when cleared (onDeactivate) AND on panel destruction —
     * a panel can therefore never leave a dangling handler on the bus, even
     * if it is destroyed without a preceding hide event.
     */
    std::vector<IEventBus::SubscriberHandle> m_eventSubscriptions;

private:
    ServiceContainer& m_services;
    QString m_panelId;
    QString m_title;
    bool m_isActive = false;
};
