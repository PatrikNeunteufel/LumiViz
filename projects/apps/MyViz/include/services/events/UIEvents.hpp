/**
 ****************************************************************************************
 * @file   UIEvents.hpp
 * @brief  UI-related events for the EventBus
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "Event.hpp"
#include <string>

// =============================================================================
// Panel Events
// =============================================================================

/**
 * @brief Emitted when a panel is opened
 */
struct PanelOpenedEvent : public Event
{
    EVENT_TYPE_NAME("PanelOpenedEvent")

    std::string panelId;

    explicit PanelOpenedEvent(std::string id)
        : panelId(std::move(id))
    {}
};

/**
 * @brief Emitted when a panel is closed
 */
struct PanelClosedEvent : public Event
{
    EVENT_TYPE_NAME("PanelClosedEvent")

    std::string panelId;

    explicit PanelClosedEvent(std::string id)
        : panelId(std::move(id))
    {}
};

/**
 * @brief Request to toggle a panel's visibility
 */
struct TogglePanelEvent : public Event
{
    EVENT_TYPE_NAME("TogglePanelEvent")

    std::string panelId;

    explicit TogglePanelEvent(std::string id)
        : panelId(std::move(id))
    {}
};

// =============================================================================
// Dialog Events
// =============================================================================

/**
 * @brief Request to open a dialog
 */
struct OpenDialogEvent : public Event
{
    EVENT_TYPE_NAME("OpenDialogEvent")

    std::string dialogId;

    explicit OpenDialogEvent(std::string id)
        : dialogId(std::move(id))
    {}
};

/**
 * @brief Emitted when a dialog is closed
 */
struct DialogClosedEvent : public Event
{
    EVENT_TYPE_NAME("DialogClosedEvent")

    std::string dialogId;
    int result;  // QDialog::DialogCode

    DialogClosedEvent(std::string id, int res)
        : dialogId(std::move(id))
        , result(res)
    {}
};

// =============================================================================
// Menu Events
// =============================================================================

/**
 * @brief Emitted when a menu item is triggered
 */
struct MenuItemTriggeredEvent : public Event
{
    EVENT_TYPE_NAME("MenuItemTriggeredEvent")

    std::string menuId;

    explicit MenuItemTriggeredEvent(std::string id)
        : menuId(std::move(id))
    {}
};

// =============================================================================
// Application Events
// =============================================================================

/**
 * @brief Application is about to quit
 */
struct ApplicationQuitEvent : public Event
{
    EVENT_TYPE_NAME("ApplicationQuitEvent")
};

/**
 * @brief Request to quit the application
 */
struct RequestQuitEvent : public Event
{
    EVENT_TYPE_NAME("RequestQuitEvent")
};

/**
 * @brief Theme has changed
 */
struct ThemeChangedEvent : public Event
{
    EVENT_TYPE_NAME("ThemeChangedEvent")

    std::string themeName;

    explicit ThemeChangedEvent(std::string name)
        : themeName(std::move(name))
    {}
};

// =============================================================================
// Frame Events
// =============================================================================

/**
 * @brief Frame mode has changed
 */
struct FrameModeChangedEvent : public Event
{
    EVENT_TYPE_NAME("FrameModeChangedEvent")

    int mode;  // 0=Limited, 1=Unlimited, 2=VSync

    explicit FrameModeChangedEvent(int m)
        : mode(m)
    {}
};

/**
 * @brief FPS update notification
 */
struct FpsUpdateEvent : public Event
{
    EVENT_TYPE_NAME("FpsUpdateEvent")

    double fps;
    uint64_t frameCount;

    FpsUpdateEvent(double f, uint64_t count)
        : fps(f)
        , frameCount(count)
    {}
};

// =============================================================================
// Visualizer Events
// =============================================================================

/**
 * @brief Request to create a new visualizer
 */
struct CreateVisualizerEvent : public Event
{
    EVENT_TYPE_NAME("CreateVisualizerEvent")
    
    std::string title;  // Optional title, empty = auto-generate
    
    CreateVisualizerEvent() = default;
    explicit CreateVisualizerEvent(std::string t)
        : title(std::move(t))
    {}
};

/**
 * @brief Request to change the active visualizer
 */
struct ChangeVisualizerEvent : public Event
{
    EVENT_TYPE_NAME("ChangeVisualizerEvent")

    std::string visualizerId;

    explicit ChangeVisualizerEvent(std::string id)
        : visualizerId(std::move(id))
    {}
};

/**
 * @brief Emitted when the visualizer has changed
 */
struct VisualizerChangedEvent : public Event
{
    EVENT_TYPE_NAME("VisualizerChangedEvent")

    std::string visualizerId;
    std::string visualizerName;

    VisualizerChangedEvent(std::string id, std::string name)
        : visualizerId(std::move(id))
        , visualizerName(std::move(name))
    {}
};

// =============================================================================
// Layout Events
// =============================================================================

/**
 * @brief Request to reset the window layout to default
 */
struct ResetLayoutEvent : public Event
{
    EVENT_TYPE_NAME("ResetLayoutEvent")
};

/**
 * @brief Request to save current layout as default
 */
struct SaveDefaultLayoutEvent : public Event
{
    EVENT_TYPE_NAME("SaveDefaultLayoutEvent")
};
