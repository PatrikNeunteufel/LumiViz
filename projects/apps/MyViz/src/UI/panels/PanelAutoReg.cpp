/**
 ****************************************************************************************
 * @file   PanelAutoReg.cpp
 * @brief  Application-specific panel registrations for MyViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * Diese Datei definiert die **MyViz-spezifischen Panels**.
 * Sie wird automatisch von PanelRegistry::instance() beim ersten Zugriff aufgerufen.
 *
 * ## Panel-Liste
 *
 * | ID | Titel | Default Visible | Beschreibung |
 * |----|-------|-----------------|--------------|
 * | player | Player | true | Audio Player Controls |
 * | playlist | Playlist | true | Playlist Management |
 * | config | Settings | false | Visualizer Configuration |
 * | visual_select | Visualizers | true | Visualizer Selection |
 *
 * ## Trennung Framework vs. App
 *
 * - **PanelRegistry.hpp/cpp** = Wiederverwendbares Framework
 * - **PanelAutoReg.cpp** = MyViz-spezifische Panels (diese Datei)
 *
 * ## Linker-Garantie
 *
 * PanelRegistry.cpp deklariert `extern void initPanelDefaults(PanelRegistry&)`.
 * Diese Referenz erzwingt, dass der Linker PanelAutoReg.cpp einbindet,
 * auch bei statischen Libraries.
 *
 * ## Neue Panels hinzufügen
 *
 * 1. Panel-Klasse erstellen (von PanelBase ableiten)
 * 2. Header hier inkludieren
 * 3. Registrierung in initPanelDefaults() hinzufügen
 ****************************************************************************************
 */

#include "services/PanelRegistry.hpp"

// Panel Headers
#include "UI/panels/PlayerPanel.hpp"
#include "UI/panels/PlaylistPanel.hpp"
#include "UI/panels/ConfigPanel.hpp"
#include "UI/panels/VisualSelectPanel.hpp"

// =============================================================================
// initPanelDefaults - Called by PanelRegistry::instance() on first access
// =============================================================================

/**
 * @brief Register all default panels for MyViz
 * 
 * @param registry Reference to the PanelRegistry singleton
 * 
 * This function is called automatically by PanelRegistry::instance() on first access.
 * It registers all available panels.
 */
void initPanelDefaults(PanelRegistry& registry)
{
    // =========================================================================
    // PLAYER PANEL
    // =========================================================================
    // Audio playback controls (Play/Pause/Stop, Volume, Seek)
    
    registry.registerPanel(
        PanelDescriptor{
            "player",           // id
            "Player",           // title
            100,                // order
            true,               // defaultVisible
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<PlayerPanel>(svc);
        },
        false);
    
    // =========================================================================
    // PLAYLIST PANEL
    // =========================================================================
    // Playlist management (Add, Remove, Reorder tracks)
    
    registry.registerPanel(
        PanelDescriptor{
            "playlist",         // id
            "Playlist",         // title
            200,                // order
            true,               // defaultVisible
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<PlaylistPanel>(svc);
        },
        false);
    
    // =========================================================================
    // CONFIG PANEL (Settings)
    // =========================================================================
    // Visualizer configuration and settings
    
    registry.registerPanel(
        PanelDescriptor{
            "config",           // id
            "Settings",         // title
            300,                // order
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<ConfigPanel>(svc);
        },
        false);
    
    // =========================================================================
    // VISUAL SELECT PANEL (Visualizers)
    // =========================================================================
    // Visualizer selection panel
    
    registry.registerPanel(
        PanelDescriptor{
            "visual_select",    // id
            "Visualizers",      // title
            400,                // order
            true,               // defaultVisible
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<VisualSelectPanel>(svc);
        },
        false);
}
