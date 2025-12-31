/**
 ****************************************************************************************
 * @file   DialogAutoReg.cpp
 * @brief  Application-specific dialog registrations for MyViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * Diese Datei definiert die **MyViz-spezifischen Dialoge**.
 * Sie wird automatisch von DialogRegistry::instance() beim ersten Zugriff aufgerufen.
 *
 * ## Dialog-Liste
 *
 * | ID | Titel | Menü-Pfad | Shortcut |
 * |----|-------|-----------|----------|
 * | about | About MyViz | Help | F1 |
 *
 * ## Trennung Framework vs. App
 *
 * - **DialogRegistry.hpp/cpp** = Wiederverwendbares Framework
 * - **DialogAutoReg.cpp** = MyViz-spezifische Dialoge (diese Datei)
 *
 * ## Linker-Garantie
 *
 * DialogRegistry.cpp deklariert `extern void initDialogDefaults(DialogRegistry&)`.
 * Diese Referenz erzwingt, dass der Linker DialogAutoReg.cpp einbindet,
 * auch bei statischen Libraries.
 *
 * ## Neue Dialoge hinzufügen
 *
 * 1. Dialog-Klasse erstellen (von QDialog ableiten)
 * 2. Header hier inkludieren
 * 3. Registrierung in initDialogDefaults() hinzufügen
 ****************************************************************************************
 */

#include "services/DialogRegistry.hpp"

// Dialog Headers
#include "UI/dialogs/AboutDialog.hpp"

// =============================================================================
// initDialogDefaults - Called by DialogRegistry::instance() on first access
// =============================================================================

/**
 * @brief Register all default dialogs for MyViz
 * 
 * @param registry Reference to the DialogRegistry singleton
 * 
 * This function is called automatically by DialogRegistry::instance() on first access.
 * It registers all available dialogs.
 */
void initDialogDefaults(DialogRegistry& registry)
{
    // =========================================================================
    // ABOUT DIALOG
    // =========================================================================
    // Application information dialog (Help → About)
    
    registry.registerDialog(
        DialogDescriptor{
            "about",            // id
            "About MyViz",      // title
            900,                // order (last in Help menu)
            true,               // modal
            "Help",             // menuPath
            "F1"                // shortcut
        },
        [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QDialog> {
            return std::make_unique<AboutDialog>(svc, parent);
        },
        false);
    
    // =========================================================================
    // SETTINGS DIALOG (TODO)
    // =========================================================================
    // Application settings dialog
    // 
    // registry.registerDialog(
    //     DialogDescriptor{
    //         "settings", "Settings", 500, true, "File", "Ctrl+,"
    //     },
    //     [](ServiceContainer& svc, QWidget* parent) {
    //         return std::make_unique<SettingsDialog>(svc, parent);
    //     },
    //     false);
}
