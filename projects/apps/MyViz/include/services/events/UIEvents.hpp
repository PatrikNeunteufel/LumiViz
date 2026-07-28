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
 *        (Import Roadmap 5.7 — handled by MainWindow: loadAvsFile).
 *
 * `path` empty  → MainWindow opens a file dialog (menu entry).
 * `path` set    → import that file directly (Import Browser panel double-click).
 * The handler auto-activates the Multi Effect host if it is not the active
 * visualizer.
 */
struct ImportAvsPresetEvent : public Event
{
    EVENT_TYPE_NAME("ImportAvsPresetEvent")

    std::string path;  ///< empty = ask via file dialog

    ImportAvsPresetEvent() = default;
    explicit ImportAvsPresetEvent(std::string p)
        : path(std::move(p))
    {}
};

/**
 * @brief Request to import a MilkDrop preset (.milk). Reserved for Import
 *        Roadmap 6 — currently answered with a "not yet supported" notice.
 */
struct ImportMilkPresetEvent : public Event
{
    EVENT_TYPE_NAME("ImportMilkPresetEvent")

    std::string path;  ///< empty = ask via file dialog

    ImportMilkPresetEvent() = default;
    explicit ImportMilkPresetEvent(std::string p)
        : path(std::move(p))
    {}
};

/**
 * @brief Ein Preset weiter / zurueck (Hotkey `preset.next` / `preset.previous`).
 *
 * Absichtlich ein EREIGNIS und keine Panel-Methode: die Quelle, aus der das
 * naechste Preset kommt, wechselt ueber die Ausbaustufen (aktives Verzeichnis des
 * Import-Browsers -> Visual-Playlist -> Composer-Spur), die Aktion und ihre Taste
 * bleiben dieselben (`docs/ui/Hotkey_Konzept.md` §1/§3). Ein neuer Empfaenger
 * ersetzt spaeter den alten, ohne dass eine Belegung angefasst wird.
 *
 * `delta` ist die Schrittweite mit Vorzeichen (+1 = weiter, -1 = zurueck).
 */
struct PresetStepEvent : public Event
{
    EVENT_TYPE_NAME("PresetStepEvent")

    int delta = 1;

    PresetStepEvent() = default;
    explicit PresetStepEvent(int d)
        : delta(d)
    {}
};

/**
 * @brief Screenshot des Visuals anfordern (Hotkey `view.screenshot`).
 *
 * Traegt bewusst nichts: WO abgelegt wird und WIE die Datei heisst, weiss der
 * `ScreenshotManager`. Der Filter kennt nur die Absicht.
 */
struct ScreenshotRequestEvent : public Event
{
    EVENT_TYPE_NAME("ScreenshotRequestEvent")
};

/**
 * @brief Emitted after an AVS import attempt so a browsing panel can show a
 *        non-modal status. noteCount is the number of import report lines
 *        (passthrough/parser notes); 0 on a clean import.
 */
struct AvsImportResultEvent : public Event
{
    EVENT_TYPE_NAME("AvsImportResultEvent")

    std::string path;
    bool ok = false;
    int noteCount = 0;

    AvsImportResultEvent(std::string p, bool success, int notes)
        : path(std::move(p))
        , ok(success)
        , noteCount(notes)
    {}
};

/**
 * @brief Request to start a FRESH, empty effect chain (File → New Effect Chain).
 *
 * Setzt den Multi-Effect-Host auf eine leere Wurzel-Liste zurueck — der
 * Ausgangspunkt fuer ein neues Preset, ohne den Umweg ueber „alles von Hand
 * loeschen". Aktiviert den Host, falls ein anderer Visualizer laeuft.
 */
struct NewEffectChainEvent : public Event
{
    EVENT_TYPE_NAME("NewEffectChainEvent")
};

/**
 * @brief Request to load a host effect-chain preset (.lvfx) into the active
 *        Multi Effect visualizer. `path` empty = ask via file dialog;
 *        `path` set = load that file directly (Import Browser double-click).
 */
struct LoadEffectChainEvent : public Event
{
    EVENT_TYPE_NAME("LoadEffectChainEvent")

    std::string path;  ///< empty = ask via file dialog

    LoadEffectChainEvent() = default;
    explicit LoadEffectChainEvent(std::string p)
        : path(std::move(p))
    {}
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

/**
 * @brief Reset the Import Browser's remembered start folder (Settings panel
 *        action) — the panel forgets the stored path and returns to home
 */
struct ResetImportBrowserDirEvent : public Event
{
    EVENT_TYPE_NAME("ResetImportBrowserDirEvent")
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
