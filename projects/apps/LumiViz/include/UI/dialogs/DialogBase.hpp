/**
 ****************************************************************************************
 * @file   DialogBase.hpp
 * @brief  Base class for all dialogs
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Dialog-Basisklasse
 *
 * DialogBase bietet:
 *   - Zugriff auf ServiceContainer
 *   - Konsistente Dialog-Einstellungen
 *   - Event-Bus-Integration
 *
 * ### Verwendung
 *
 * ```cpp
 * class AboutDialog : public DialogBase {
 *     Q_OBJECT
 * public:
 *     AboutDialog(ServiceContainer& services, QWidget* parent = nullptr)
 *         : DialogBase(services, "About LumiViz", parent)
 *     {
 *         setupUI();
 *     }
 * };
 * ```
 ****************************************************************************************
 */

#pragma once

#include <QDialog>
#include <QString>

// Forward declarations
class ServiceContainer;
class IEventBus;

/**
 * @class DialogBase
 * @brief Abstract base class for all dialogs
 *
 * Provides common functionality:
 *   - ServiceContainer access
 *   - EventBus access
 *   - Consistent sizing and positioning
 */
class DialogBase : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construct a dialog
     * @param services ServiceContainer reference
     * @param title Dialog window title
     * @param parent Parent widget
     */
    explicit DialogBase(ServiceContainer& services,
                        const QString& title,
                        QWidget* parent = nullptr);

    ~DialogBase() override = default;

    /**
     * @brief Get dialog unique identifier
     *
     * Override in derived classes if needed.
     * Default: class name
     */
    [[nodiscard]] virtual QString dialogId() const;

protected:
    /**
     * @brief Access the service container
     * @return Reference to ServiceContainer
     */
    [[nodiscard]] ServiceContainer& services() { return m_services; }
    [[nodiscard]] const ServiceContainer& services() const { return m_services; }

    /**
     * @brief Access the event bus (convenience)
     * @return Pointer to IEventBus or nullptr if not registered
     */
    [[nodiscard]] IEventBus* eventBus() const;

    /**
     * @brief Center dialog on parent or screen
     */
    void centerOnParent();

    /**
     * @brief Set minimum size relative to parent
     * @param widthRatio Width ratio (0.0 - 1.0)
     * @param heightRatio Height ratio (0.0 - 1.0)
     */
    void setRelativeMinimumSize(double widthRatio, double heightRatio);

    /**
     * @brief Show event handler - centers dialog
     */
    void showEvent(QShowEvent* event) override;

private:
    ServiceContainer& m_services;
};
