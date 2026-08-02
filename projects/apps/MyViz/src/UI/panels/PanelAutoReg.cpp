/**
 ****************************************************************************************
 * @file   PanelAutoReg.cpp
 * @brief  Application-specific panel registrations for MyViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.1.0
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
 * | config | Visualizer Config | false | Active Visualizer Configuration |
 * | settings | Settings | false | Audio & Performance Settings |
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
#include "UI/panels/ImportBrowserPanel.hpp"
#include "UI/panels/ShadertoyBrowserPanel.hpp"
#include "UI/panels/ConfigPanel.hpp"
#include "UI/panels/MultiEffectPanel.hpp"
#include "UI/panels/SettingsPanel.hpp"
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
    // IMPORT BROWSER PANEL (AVS/MilkDrop preset folder browser)
    // =========================================================================
    // Browse a folder of .avs/.milk presets; double-click imports into the host.

    registry.registerPanel(
        PanelDescriptor{
            "import_browser",   // id
            "Import Browser",   // title
            250,                // order
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<ImportBrowserPanel>(svc);
        },
        false);

    // =========================================================================
    // SHADERTOY BROWSER PANEL (Strang S3)
    // =========================================================================
    // Shadertoy-API-Suche mit Vorschaubildern; Doppelklick laedt den Shader
    // als Ein-Node-Chain (AppData/shadertoy, LoadEffectChainEvent).

    registry.registerPanel(
        PanelDescriptor{
            "shadertoy_browser",  // id
            "Shadertoy Browser",  // title
            260,                  // order
            false,                // defaultVisible (hidden by default)
            "View/Panels"         // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<ShadertoyBrowserPanel>(svc);
        },
        false);

    // =========================================================================
    // CONFIG PANEL (Visualizer Config)
    // =========================================================================
    // Configuration for the currently active visualizer

    registry.registerPanel(
        PanelDescriptor{
            "config",           // id
            "Visualizer Config", // title
            300,                // order
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<ConfigPanel>(svc);
        },
        false);
    
    // =========================================================================
    // MULTI EFFECT CHAIN PANEL (Import Roadmap 5.7b)
    // =========================================================================
    // Tree editor for the Multi Effect host's effect chain

    registry.registerPanel(
        PanelDescriptor{
            "multieffect_chain",      // id
            "Effect Chain",           // title
            320,                      // order
            false,                    // defaultVisible (hidden by default)
            "View/Panels"             // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<MultiEffectPanel>(svc);
        },
        false);

    // =========================================================================
    // SETTINGS PANEL
    // =========================================================================
    // Global application settings (Audio, Performance)
    
    registry.registerPanel(
        PanelDescriptor{
            "settings",         // id
            "Settings",         // title
            350,                // order
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<SettingsPanel>(svc);
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
