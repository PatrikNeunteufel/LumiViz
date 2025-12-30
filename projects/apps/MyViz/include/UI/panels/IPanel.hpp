/**
 ****************************************************************************************
 * @file   IPanel.hpp
 * @brief  Interface for all dock panels
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Panel Interface
 *
 * Alle Panels implementieren dieses Interface für konsistentes Verhalten:
 *   - Eindeutige ID für Persistenz
 *   - Titel für DockWidget
 *   - Lifecycle-Methoden (activate, deactivate)
 *   - State-Persistenz (saveState, restoreState)
 ****************************************************************************************
 */

#pragma once

#include <QString>

/**
 * @class IPanel
 * @brief Interface for dock panels
 *
 * Panels are dockable widgets that can be shown/hidden and rearranged.
 * Each panel has a unique ID and can persist its state.
 */
class IPanel
{
public:
    virtual ~IPanel() = default;

    // =========================================================================
    // Identification
    // =========================================================================

    /**
     * @brief Get panel unique identifier
     * @return Panel ID (e.g., "player", "playlist", "config")
     */
    [[nodiscard]] virtual QString panelId() const = 0;

    /**
     * @brief Get panel display title
     * @return Title shown in dock widget header
     */
    [[nodiscard]] virtual QString panelTitle() const = 0;

    // =========================================================================
    // Lifecycle
    // =========================================================================

    /**
     * @brief Called when panel becomes visible/active
     *
     * Use for starting timers, subscribing to events, etc.
     */
    virtual void onActivate() {}

    /**
     * @brief Called when panel becomes hidden/inactive
     *
     * Use for stopping timers, unsubscribing from events, etc.
     */
    virtual void onDeactivate() {}

    // =========================================================================
    // State Persistence
    // =========================================================================

    /**
     * @brief Save panel-specific state
     *
     * Called when application closes or layout is saved.
     * Override to save custom settings (e.g., selected items, scroll position).
     */
    virtual void saveState() {}

    /**
     * @brief Restore panel-specific state
     *
     * Called when application starts or layout is restored.
     * Override to restore custom settings.
     */
    virtual void restoreState() {}

    // =========================================================================
    // Optional Features
    // =========================================================================

    /**
     * @brief Check if panel can be closed
     * @return true if panel can be closed (default: true)
     *
     * Override to prevent closing (e.g., unsaved changes).
     */
    [[nodiscard]] virtual bool canClose() const { return true; }

    /**
     * @brief Get preferred initial dock area
     * @return Qt::DockWidgetArea flags
     *
     * Override to suggest initial position.
     */
    [[nodiscard]] virtual int preferredArea() const { return 0; }
};
