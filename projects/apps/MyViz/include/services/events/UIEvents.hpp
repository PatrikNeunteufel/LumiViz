/**
 ****************************************************************************************
 * @file   UIEvents.hpp
 * @brief  UI-related events for the EventBus
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.1.0 - Added Visualizer Config Events
 ****************************************************************************************
 */

#pragma once

#include "Event.hpp"
#include <string>

class QMutex;  // render mutex in VisualizerChangedEvent (pointer only)

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
 * @brief Request to import an AVS preset into the active Multi Effect visualizer
 *        (Import Roadmap 5.7 — handled by MainWindow: file dialog + loadAvsFile).
 */
struct ImportAvsPresetEvent : public Event
{
    EVENT_TYPE_NAME("ImportAvsPresetEvent")
};

/**
 * @brief Request to load a host effect-chain preset (.lvfx) into the active
 *        Multi Effect visualizer.
 */
struct LoadEffectChainEvent : public Event
{
    EVENT_TYPE_NAME("LoadEffectChainEvent")
};

/**
 * @brief Request to save the active Multi Effect visualizer's chain (.lvfx).
 */
struct SaveEffectChainEvent : public Event
{
    EVENT_TYPE_NAME("SaveEffectChainEvent")
};

/**
 * @brief Emitted after the Multi Effect host's chain was replaced (import/load),
 *        so the chain editor panel can rebuild its tree.
 */
struct EffectChainChangedEvent : public Event
{
    EVENT_TYPE_NAME("EffectChainChangedEvent")
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
 *
 * Since the render-thread decoupling the event also carries the render mutex
 * of the publishing VisualizerWidget: any parameter/gradient/tap access on
 * visualizerPtr must hold it (the render thread renders concurrently).
 */
struct VisualizerChangedEvent : public Event
{
    EVENT_TYPE_NAME("VisualizerChangedEvent")

    std::string visualizerId;
    std::string visualizerName;
    void* visualizerPtr;    ///< Pointer to IVisualizer (cast to avoid include)
    QMutex* renderMutex;    ///< Guards UI access against the render thread

    VisualizerChangedEvent(std::string id, std::string name,
                           void* vizPtr = nullptr, QMutex* mutex = nullptr)
        : visualizerId(std::move(id))
        , visualizerName(std::move(name))
        , visualizerPtr(vizPtr)
        , renderMutex(mutex)
    {}
};

// =============================================================================
// Layout Events
// =============================================================================
// Note: The legacy "Visualizer Configuration Events" (ColorScheme/Smoothing/
// PeakHold/Shape) were removed in Phase 4 step 0 — they had no publisher;
// visualizer configuration goes through IVisualizer::setParam().

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

// =============================================================================
// Fullscreen Events
// =============================================================================

/**
 * @brief Request to toggle fullscreen mode
 *
 * sourceVisualizer (a VisualizerWidget*, cast to avoid the include) names the
 * widget that requested the toggle (double-click/Esc) — fullscreen then shows
 * THAT visualizer. nullptr (menu/F11) falls back to the primary one.
 */
struct ToggleFullscreenEvent : public Event
{
    EVENT_TYPE_NAME("ToggleFullscreenEvent")

    void* sourceVisualizer = nullptr;

    ToggleFullscreenEvent() = default;
    explicit ToggleFullscreenEvent(void* source) : sourceVisualizer(source) {}
};

/**
 * @brief Request to exit fullscreen mode
 */
struct ExitFullscreenEvent : public Event
{
    EVENT_TYPE_NAME("ExitFullscreenEvent")
};
