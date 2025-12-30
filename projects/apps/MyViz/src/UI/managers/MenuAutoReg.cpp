/**
 ****************************************************************************************
 * @file   MenuAutoReg.cpp
 * @brief  Automatic menu registrations for standard menus
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * Diese Datei registriert die Standard-Menüstruktur beim Programmstart.
 * Die Self-Registration-Makros werden beim statischen Init ausgeführt.
 *
 * ## Menü-Struktur
 *
 * ```
 * File (100)
 * ├── Exit (900)
 * View (200)
 * └── Panels (100) ← Auto-generiert aus PanelRegistry
 * Help (900)
 * └── About... (100) F1
 * ```
 ****************************************************************************************
 */

#include "services/MenuRegistry.hpp"
#include "services/DialogRegistry.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QApplication>

// =============================================================================
// FILE MENU
// =============================================================================

REGISTER_MENU_CONTAINER("menu.file", "File", "toplevel", 100)

REGISTER_MENU_ITEM_SHORTCUT("menu.file.exit", "Exit", "menu.file", 900, "Alt+F4",
    [](ServiceContainer& /*svc*/) {
        QApplication::quit();
    })

// =============================================================================
// VIEW MENU
// =============================================================================

REGISTER_MENU_CONTAINER("menu.view", "View", "toplevel", 200)

// Panels submenu - will be auto-populated by MenuManager
REGISTER_MENU_CONTAINER("menu.view.panels", "Panels", "menu.view", 100)

// =============================================================================
// HELP MENU
// =============================================================================

REGISTER_MENU_CONTAINER("menu.help", "Help", "toplevel", 900)

REGISTER_MENU_ITEM_SHORTCUT("menu.help.about", "About...", "menu.help", 100, "F1",
    [](ServiceContainer& svc) {
        // Trigger via EventBus
        if (auto* eventBus = svc.tryResolve<IEventBus>())
        {
            eventBus->publish(OpenDialogEvent{"about"});
        }
    })
