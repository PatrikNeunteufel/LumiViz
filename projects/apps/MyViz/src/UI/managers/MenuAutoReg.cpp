/**
 ****************************************************************************************
 * @file   MenuAutoReg.cpp
 * @brief  Automatic menu registrations for standard menu structure
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.1.0
 *
 * @details
 * Diese Datei registriert die **Basis-Menüstruktur** beim Programmstart.
 *
 * ## Menü-Struktur (Basis-Container)
 *
 * ```
 * File (100)
 * ├── [Open Audio...] ← registriert in MenuItemsAutoReg.cpp
 * ├── ─────────────── (separator via group)
 * └── Exit (900)
 *
 * View (200)
 * ├── [New Visualizer] ← registriert in MenuItemsAutoReg.cpp
 * ├── ─────────────────
 * ├── Panels (100) ← Auto-populated by MenuManager from PanelRegistry
 * ├── Perspectives (200) ← Items via DockManager
 * └── [Reset Layout] ← registriert in MenuItemsAutoReg.cpp
 *
 * Settings (300)
 * └── Frame Mode (100) ← Items registriert in MenuItemsAutoReg.cpp
 *
 * Help (900)
 * └── [About MyViz...] ← registriert in MenuItemsAutoReg.cpp
 * ```
 *
 * ## WICHTIG: Manuelle Registrierung
 *
 * Um Linker-Optimierungsprobleme zu vermeiden, werden die Container
 * direkt in initMenuAutoReg() registriert (nicht über statische Makros).
 ****************************************************************************************
 */

#include "services/MenuRegistry.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>

// =============================================================================
// Manual Registration Function
// =============================================================================

/**
 * @brief Registers all base menu containers
 * 
 * MUST be called before MenuManager::buildMenuBar()!
 * This performs the actual registration at runtime, avoiding linker issues.
 */
void initMenuAutoReg()
{
    auto& registry = MenuRegistry::instance();
    
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
    
    // =========================================================================
    // HELP MENU (900)
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.help", "toplevel", 900}, "Help", false},
        false);
}
