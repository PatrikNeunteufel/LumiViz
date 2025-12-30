/**
 ****************************************************************************************
 * @file   WidgetBase.hpp
 * @brief  Base class for registerable widgets
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Widget-Basisklasse
 *
 * WidgetBase bietet:
 *   - IWidget-Implementierung
 *   - ServiceContainer-Zugriff
 *   - EventBus-Convenience-Methoden
 *
 * ### Verwendung
 *
 * ```cpp
 * class VolumeWidget : public WidgetBase {
 *     Q_OBJECT
 * public:
 *     VolumeWidget(ServiceContainer& services, QWidget* parent = nullptr)
 *         : WidgetBase(services, "volume", tr("Volume"), parent)
 *     {
 *         setupUI();
 *     }
 * };
 *
 * // Self-registration:
 * REGISTER_WIDGET("volume", "Volume Control", VolumeWidget)
 * ```
 ****************************************************************************************
 */

#pragma once

#include "IWidget.hpp"

#include <QWidget>
#include <QString>

// Forward declarations
class ServiceContainer;
class IEventBus;

/**
 * @class WidgetBase
 * @brief Abstract base class for registerable widgets
 *
 * Implements IWidget and provides common functionality.
 */
class WidgetBase : public QWidget, public IWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct a widget
     * @param services ServiceContainer reference
     * @param id Unique widget identifier
     * @param name Display name
     * @param parent Parent widget
     */
    explicit WidgetBase(ServiceContainer& services,
                        const QString& id,
                        const QString& name,
                        QWidget* parent = nullptr);

    ~WidgetBase() override = default;

    // =========================================================================
    // IWidget Implementation
    // =========================================================================

    [[nodiscard]] QString widgetId() const override { return m_widgetId; }
    [[nodiscard]] QString widgetName() const override { return m_name; }

    // =========================================================================
    // Update Control
    // =========================================================================

    /**
     * @brief Check if updates are running
     */
    [[nodiscard]] bool isUpdating() const { return m_isUpdating; }

public Q_SLOTS:
    /**
     * @brief Start widget updates
     */
    void start();

    /**
     * @brief Stop widget updates
     */
    void stop();

Q_SIGNALS:
    /**
     * @brief Emitted when updates start
     */
    void updatesStarted();

    /**
     * @brief Emitted when updates stop
     */
    void updatesStopped();

protected:
    /**
     * @brief Access the service container
     */
    [[nodiscard]] ServiceContainer& services() { return m_services; }
    [[nodiscard]] const ServiceContainer& services() const { return m_services; }

    /**
     * @brief Access the event bus (convenience)
     */
    [[nodiscard]] IEventBus* eventBus() const;

    /**
     * @brief Show event - auto-starts updates
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief Hide event - auto-stops updates
     */
    void hideEvent(QHideEvent* event) override;

private:
    ServiceContainer& m_services;
    QString m_widgetId;
    QString m_name;
    bool m_isUpdating = false;
};
