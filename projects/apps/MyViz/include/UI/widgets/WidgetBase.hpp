/**
 ****************************************************************************************
 * @file   WidgetBase.hpp
 * @brief  Template base class for registerable widgets
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * ## Qt6 Tutorial: Widget-Basisklasse (Template)
 *
 * WidgetBase ist jetzt ein Template, das verschiedene Qt-Widget-Basisklassen
 * unterstützt. Dies ermöglicht einheitlichen ServiceContainer/EventBus-Zugriff
 * für alle Widget-Typen.
 *
 * ### Unterstützte Basisklassen
 *
 * | Basisklasse       | Include                    | Verwendung                    |
 * |-------------------|----------------------------|-------------------------------|
 * | QWidget           | `<QWidget>`                | Standard-Widgets              |
 * | QOpenGLWidget     | `<QOpenGLWidget>`          | OpenGL-Rendering              |
 * | QFrame            | `<QFrame>`                 | Widgets mit Rahmen            |
 * | QAbstractScrollArea | `<QAbstractScrollArea>`  | Scrollbare Bereiche           |
 * | QGraphicsView     | `<QGraphicsView>`          | 2D-Szenen                     |
 * | QQuickWidget      | `<QQuickWidget>`           | QML-Integration               |
 *
 * ### Verwendung
 *
 * ```cpp
 * // Standard-Widget
 * class VolumeWidget : public WidgetBase<QWidget> {
 *     Q_OBJECT
 * public:
 *     VolumeWidget(ServiceContainer& services, QWidget* parent = nullptr)
 *         : WidgetBase(services, "volume", tr("Volume"), parent)
 *     {}
 * };
 *
 * // OpenGL-Widget
 * class VisualizerWidget : public WidgetBase<QOpenGLWidget>,
 *                          protected QOpenGLFunctions {
 *     Q_OBJECT
 * public:
 *     VisualizerWidget(ServiceContainer& services, QWidget* parent = nullptr)
 *         : WidgetBase(services, "visualizer", tr("Visualizer"), parent)
 *     {}
 * };
 * ```
 *
 * ### Type Aliases
 *
 * ```cpp
 * using StandardWidgetBase = WidgetBase<QWidget>;
 * using OpenGLWidgetBase = WidgetBase<QOpenGLWidget>;
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
 * @brief Template base class for registerable widgets
 *
 * @tparam BaseWidget Qt widget base class (QWidget, QOpenGLWidget, etc.)
 *
 * Implements IWidget and provides:
 *   - ServiceContainer access
 *   - EventBus convenience methods
 *   - Auto start/stop on show/hide
 */
template<typename BaseWidget = QWidget>
class WidgetBase : public BaseWidget, public IWidget
{
    // Note: Q_OBJECT cannot be used in templates directly.
    // Derived classes must add Q_OBJECT themselves.

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

    /**
     * @brief Start widget updates
     */
    void startUpdates() override;

    /**
     * @brief Stop widget updates
     */
    void stopUpdates() override;

protected:
    /**
     * @brief Access the service container
     */
    [[nodiscard]] ServiceContainer& services() { return m_services; }
    [[nodiscard]] const ServiceContainer& services() const { return m_services; }

    /**
     * @brief Access the event bus (convenience)
     * @return EventBus pointer or nullptr if not registered
     */
    [[nodiscard]] IEventBus* eventBus();

    /**
     * @brief Called when updates start
     *
     * Override to start timers, animations, etc.
     */
    virtual void onStartUpdates() {}

    /**
     * @brief Called when updates stop
     *
     * Override to stop timers, cleanup, etc.
     */
    virtual void onStopUpdates() {}

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

// =============================================================================
// Type Aliases for Common Base Classes
// =============================================================================

/**
 * @brief Standard widget base (QWidget)
 */
using StandardWidgetBase = WidgetBase<QWidget>;

// Note: OpenGLWidgetBase requires #include <QOpenGLWidget> before use
// using OpenGLWidgetBase = WidgetBase<QOpenGLWidget>;

// =============================================================================
// Template Implementation
// =============================================================================

#include "WidgetBase.tpp"
