/**
 ****************************************************************************************
 * @file   IWidget.hpp
 * @brief  Interface for registerable widgets
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Widget Interface
 *
 * Alle über WidgetRegistry registrierbaren Widgets implementieren dieses Interface.
 * Im Gegensatz zu Panels sind Widgets:
 *   - Nicht dockbar
 *   - Einbettbar in andere Widgets/Layouts
 *   - Oft kleiner und spezialisierter
 ****************************************************************************************
 */

#pragma once

#include <QString>

/**
 * @class IWidget
 * @brief Interface for registerable widgets
 *
 * Widgets are embeddable components that can be used anywhere.
 */
class IWidget
{
public:
    virtual ~IWidget() = default;

    // =========================================================================
    // Identification
    // =========================================================================

    /**
     * @brief Get widget unique identifier
     * @return Widget ID (e.g., "volume", "spectrum_mini")
     */
    [[nodiscard]] virtual QString widgetId() const = 0;

    /**
     * @brief Get widget display name
     * @return Name shown in widget selectors
     */
    [[nodiscard]] virtual QString widgetName() const = 0;

    // =========================================================================
    // Lifecycle
    // =========================================================================

    /**
     * @brief Called when widget should start updating
     *
     * Use for starting timers, subscribing to events.
     */
    virtual void startUpdates() {}

    /**
     * @brief Called when widget should stop updating
     *
     * Use for stopping timers, unsubscribing from events.
     */
    virtual void stopUpdates() {}

    // =========================================================================
    // State
    // =========================================================================

    /**
     * @brief Save widget state
     * @return State data as QByteArray
     */
    virtual QByteArray saveState() const { return {}; }

    /**
     * @brief Restore widget state
     * @param state Previously saved state
     */
    virtual void restoreState(const QByteArray& state) { Q_UNUSED(state) }
};
