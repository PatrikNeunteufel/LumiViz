/**
 ****************************************************************************************
 * @file   DialogManager.hpp
 * @brief  Manager for showing dialogs from DialogRegistry
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: DialogManager
 *
 * Der DialogManager:
 *   - Zeigt Dialoge aus DialogRegistry an
 *   - Unterstützt modale und nicht-modale Dialoge
 *   - Verwaltet Dialog-Lebenszyklus
 *   - Reagiert auf OpenDialogEvent
 *
 * ### Verwendung
 *
 * ```cpp
 * // Direkt
 * dialogManager.show("about");
 *
 * // Via EventBus
 * eventBus.publish(OpenDialogEvent{"settings"});
 * ```
 ****************************************************************************************
 */

#pragma once

#include <QObject>
#include <QString>
#include <QHash>
#include <QPointer>
#include <memory>

// Forward declarations
class ServiceContainer;
class QWidget;
class QDialog;
class IEventBus;

/**
 * @class DialogManager
 * @brief Manages dialog creation and display
 */
class DialogManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct DialogManager
     * @param services ServiceContainer for dependency injection
     * @param parent Parent widget for dialogs
     */
    explicit DialogManager(ServiceContainer& services, QWidget* parent = nullptr);

    ~DialogManager() override;

    // =========================================================================
    // Dialog Display
    // =========================================================================

    /**
     * @brief Show a dialog by ID
     * @param dialogId Dialog ID from registry
     * @return Dialog result for modal dialogs, 0 for modeless
     */
    int show(const QString& dialogId);

    /**
     * @brief Show a dialog and return immediately (modeless)
     * @param dialogId Dialog ID from registry
     * @return Created dialog or nullptr if failed
     */
    QDialog* showModeless(const QString& dialogId);

    /**
     * @brief Check if a modeless dialog is open
     * @param dialogId Dialog ID
     */
    [[nodiscard]] bool isOpen(const QString& dialogId) const;

    /**
     * @brief Close a modeless dialog
     * @param dialogId Dialog ID
     */
    void close(const QString& dialogId);

    /**
     * @brief Close all open dialogs
     */
    void closeAll();

    // =========================================================================
    // EventBus Integration
    // =========================================================================

    /**
     * @brief Subscribe to OpenDialogEvent
     *
     * Call this after EventBus is available.
     */
    void subscribeToEvents();

Q_SIGNALS:
    /**
     * @brief Emitted when a dialog is opened
     */
    void dialogOpened(const QString& dialogId);

    /**
     * @brief Emitted when a dialog is closed
     */
    void dialogClosed(const QString& dialogId, int result);

private Q_SLOTS:
    void onDialogFinished(int result);

private:
    ServiceContainer& m_services;
    QWidget* m_parentWidget = nullptr;

    // Dialog ID → Open modeless dialog
    QHash<QString, QPointer<QDialog>> m_openDialogs;

    // EventBus subscription ID
    uint64_t m_eventSubscriptionId = 0;
};
