/**
 ****************************************************************************************
 * @file   MenuAutoReg.cpp
 * @brief  Application-specific menu registrations for MyViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.2.0
 *
 * @details
 * Diese Datei definiert die **MyViz-spezifische Menüstruktur**.
 * Sie wird automatisch von MenuRegistry::instance() beim ersten Zugriff aufgerufen.
 *
 * ## Menü-Struktur
 *
 * ```
 * File (100)
 * ├── Open Audio... (100)    [Ctrl+O]
 * ├── ─────────────── (800)  [Separator]
 * └── Exit (900)             [Alt+F4]
 *
 * View (200)
 * ├── New Visualizer (100)   [Ctrl+N]
 * ├── ─────────────── (50)   [Separator]
 * ├── Panels (100)           [→ DockManager]
 * ├── Perspectives (200)     [→ DockManager]
 * ├── ─────────────── (800)  [Separator]
 * └── Reset Layout (900)
 *
 * Settings (300)
 * └── Frame Mode (100)       [Exclusive Container]
 *     ├── Limited (100)      [●] Default
 *     ├── Unlimited (200)    [○]
 *     └── VSync (300)        [○]
 *
 * Help (900)
 * └── About MyViz... (100)   [F1]
 * ```
 *
 * ## Trennung Framework vs. App
 *
 * - **MenuRegistry.hpp/cpp** = Wiederverwendbares Framework
 * - **MenuAutoReg.cpp** = MyViz-spezifische Menüs (diese Datei)
 *
 * ## Linker-Garantie
 *
 * MenuRegistry.cpp deklariert `extern void initMenuDefaults(MenuRegistry&)`.
 * Diese Referenz erzwingt, dass der Linker MenuAutoReg.cpp einbindet,
 * auch bei statischen Libraries.
 ****************************************************************************************
 */

#include "services/MenuRegistry.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QApplication>
#include <BasicLogger.h>

// =============================================================================
// initMenuDefaults - Called by MenuRegistry::instance() on first access
// =============================================================================

/**
 * @brief Register all default menus for MyViz
 * 
 * @param registry Reference to the MenuRegistry singleton
 * 
 * This function is called automatically by MenuRegistry::instance() on first access.
 * It registers all standard menus (File, View, Settings, Help) and their items.
 */
void initMenuDefaults(MenuRegistry& registry)
{
    // =========================================================================
    // FILE MENU (100)
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.file", "toplevel", 100}, "File", false},
        false);
    
    // Group for file operations (Open, etc.)
    registry.registerGroup(
        MenuGroupDesc{{"menu.file.group.open", "menu.file", 100}},
        false);
    
    // Open Audio... (Ctrl+O)
    registry.registerItem(
        MenuItemDesc{
            {"menu.file.open", "menu.file", 100},
            "Open Audio...",
            [](ServiceContainer& /*svc*/) {
                BasicLogger::logDebug("Open Audio clicked");
            },
            {},  // isChecked
            {},  // isEnabled
            "Ctrl+O"
        },
        false);
    
    // Group for exit (separator before exit)
    registry.registerGroup(
        MenuGroupDesc{{"menu.file.group.exit", "menu.file", 800}},
        false);
    
    // Exit action
    registry.registerItem(
        MenuItemDesc{
            {"menu.file.exit", "menu.file", 900},
            "Exit",
            [](ServiceContainer& /*svc*/) { QApplication::quit(); },
            {},  // isChecked
            {},  // isEnabled
            "Alt+F4"
        },
        false);
    
    // =========================================================================
    // VIEW MENU (200)
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.view", "toplevel", 200}, "View", false},
        false);
    
    // Group for visualizer actions (New Visualizer, etc.)
    registry.registerGroup(
        MenuGroupDesc{{"menu.view.group.visualizers", "menu.view", 50}},
        false);
    
    // New Visualizer (Ctrl+N)
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.newvisualizer", "menu.view", 100},
            "New Visualizer",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                {
                    eventBus->publish(CreateVisualizerEvent{});
                }
                BasicLogger::logDebug("New Visualizer clicked");
            },
            {},  // isChecked
            {},  // isEnabled
            "Ctrl+N"
        },
        false);
    
    // Panels submenu - auto-populated by MenuManager from PanelRegistry
    registry.registerContainer(
        MenuContainerDesc{{"menu.view.panels", "menu.view", 100}, "Panels", false},
        false);
    
    // Perspectives submenu - items added by DockManager
    registry.registerContainer(
        MenuContainerDesc{{"menu.view.perspectives", "menu.view", 200}, "Perspectives", false},
        false);
    
    // Group for layout actions (Reset Layout, etc.)
    registry.registerGroup(
        MenuGroupDesc{{"menu.view.group.layout", "menu.view", 800}},
        false);
    
    // Reset Layout
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.resetlayout", "menu.view", 900},
            "Reset Layout",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                {
                    eventBus->publish(ResetLayoutEvent{});
                }
                BasicLogger::logDebug("Reset Layout clicked");
            },
            {},  // isChecked
            {},  // isEnabled
            {}   // shortcut
        },
        false);
    
    // Save Layout as Default
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.savedefault", "menu.view", 910},
            "Save Layout as Default",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                {
                    eventBus->publish(SaveDefaultLayoutEvent{});
                }
                BasicLogger::logDebug("Save Layout as Default clicked");
            },
            {},  // isChecked
            {},  // isEnabled
            {}   // shortcut
        },
        false);
    
    // =========================================================================
    // SETTINGS MENU (300)
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.settings", "toplevel", 300}, "Settings", false},
        false);
    
    // Frame Mode submenu - EXCLUSIVE: only one can be checked at a time
    registry.registerContainer(
        MenuContainerDesc{{"menu.settings.framemode", "menu.settings", 100}, "Frame Mode", true},
        false);
    
    // Frame Mode: Limited (60 FPS)
    registry.registerItem(
        MenuItemDesc{
            {"menu.settings.framemode.limited", "menu.settings.framemode", 100},
            "Limited (60 FPS)",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                {
                    eventBus->publish(FrameModeChangedEvent{0});
                }
                BasicLogger::logInfo("Frame mode changed to: Limited");
            },
            [](ServiceContainer& /*svc*/) { return true; },  // isChecked (default)
            {},  // isEnabled
            {}   // shortcut
        },
        false);
    
    // Frame Mode: Unlimited
    registry.registerItem(
        MenuItemDesc{
            {"menu.settings.framemode.unlimited", "menu.settings.framemode", 200},
            "Unlimited",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                {
                    eventBus->publish(FrameModeChangedEvent{1});
                }
                BasicLogger::logInfo("Frame mode changed to: Unlimited");
            },
            [](ServiceContainer& /*svc*/) { return false; },  // isChecked
            {},  // isEnabled
            {}   // shortcut
        },
        false);
    
    // Frame Mode: VSync
    registry.registerItem(
        MenuItemDesc{
            {"menu.settings.framemode.vsync", "menu.settings.framemode", 300},
            "VSync",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                {
                    eventBus->publish(FrameModeChangedEvent{2});
                }
                BasicLogger::logInfo("Frame mode changed to: VSync");
            },
            [](ServiceContainer& /*svc*/) { return false; },  // isChecked
            {},  // isEnabled
            {}   // shortcut
        },
        false);
    
    // =========================================================================
    // HELP MENU (900)
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.help", "toplevel", 900}, "Help", false},
        false);
    
    // About MyViz... (F1)
    registry.registerItem(
        MenuItemDesc{
            {"menu.help.about", "menu.help", 100},
            "About MyViz...",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                {
                    eventBus->publish(OpenDialogEvent{"about"});
                }
                BasicLogger::logDebug("About clicked");
            },
            {},  // isChecked
            {},  // isEnabled
            "F1"
        },
        false);
}
