/**
 ****************************************************************************************
 * @file   MenuItemsAutoReg.cpp
 * @brief  Feature-specific menu item registrations
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.1.0
 *
 * @details
 * Diese Datei enthält die konkreten Menu-Item-Registrierungen.
 * 
 * ## Zukünftige Dezentralisierung
 *
 * Diese Items sollten später in die jeweiligen Komponenten verschoben werden:
 *
 * | Item                | Ziel-Datei                    |
 * |---------------------|-------------------------------|
 * | Open Audio...       | AudioPlayer.cpp               |
 * | New Visualizer      | VisualizerWidget.cpp          |
 * | Reset Layout        | DockManager.cpp               |
 * | Frame Mode Items    | Application.cpp / Settings    |
 * | About MyViz         | AboutDialog.cpp               |
 *
 * ## WICHTIG: Manuelle Registrierung
 *
 * Um Linker-Optimierungsprobleme zu vermeiden, werden die Items
 * direkt in initMenuItemsAutoReg() registriert (nicht über statische Makros).
 ****************************************************************************************
 */

#include "services/MenuRegistry.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <BasicLogger.h>

// =============================================================================
// Manual Registration Function
// =============================================================================

/**
 * @brief Registers all menu items
 * 
 * MUST be called before MenuManager::buildMenuBar()!
 * This performs the actual registration at runtime, avoiding linker issues.
 */
void initMenuItemsAutoReg()
{
    auto& registry = MenuRegistry::instance();
    
    // =========================================================================
    // FILE MENU ITEMS
    // =========================================================================
    
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
    
    // =========================================================================
    // VIEW MENU ITEMS
    // =========================================================================
    
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
    
    // =========================================================================
    // SETTINGS MENU ITEMS - Frame Mode
    // =========================================================================
    
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
    // HELP MENU ITEMS
    // =========================================================================
    
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
