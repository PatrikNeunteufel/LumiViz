/**
 ****************************************************************************************
 * @file   PanelAutoReg.cpp
 * @brief  Application-specific panel registrations for LumiViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.1.0
 *
 * @details
 * Diese Datei definiert die **LumiViz-spezifischen Panels**.
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
 * - **PanelAutoReg.cpp** = LumiViz-spezifische Panels (diese Datei)
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
#include "UI/panels/AudioBrowserPanel.hpp"
#include "UI/panels/TagViewPanel.hpp"
#include "UI/panels/LyricsSyncPanel.hpp"
#include "UI/panels/KaraokePanel.hpp"
#include "UI/panels/ImportBrowserPanel.hpp"
#include "UI/panels/ShadertoyBrowserPanel.hpp"
#include "UI/panels/ConfigPanel.hpp"
#include "UI/panels/MultiEffectPanel.hpp"
#include "UI/panels/SettingsPanel.hpp"
#include "UI/panels/VisualSelectPanel.hpp"
#include "composer/ComposerPanel.hpp"  // Composer-Naht N8
#include "loop/LoopPanel.hpp"

// =============================================================================
// initPanelDefaults - Called by PanelRegistry::instance() on first access
// =============================================================================

/**
 * @brief Register all default panels for LumiViz
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
    // EFFECT BROWSER PANEL (.avs/.milk/.lvfx preset folder browser)
    // =========================================================================
    // Browse a folder of presets; double-click loads into the host. Titel seit
    // S82 „Effect Browser" (laedt auch eigene .lvfx) — die id bleibt
    // "import_browser": an ihr haengen Dock-Layout und gespeicherter Ordner.

    registry.registerPanel(
        PanelDescriptor{
            "import_browser",   // id (bewusst unveraendert, s. o.)
            "Effect Browser",   // title
            250,                // order
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<ImportBrowserPanel>(svc);
        },
        false);

    // =========================================================================
    // AUDIO BROWSER PANEL (S84) — Ordner mit Musik, Gegenstueck zum Effect
    // Browser: Doppelklick spielt, Ziehen in Playlist/Player/Visualisierung.
    // =========================================================================

    registry.registerPanel(
        PanelDescriptor{
            "audio_browser",    // id
            "Audio Browser",    // title
            240,                // order (vor dem Effect Browser)
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<AudioBrowserPanel>(svc);
        },
        false);

    // =========================================================================
    // TAG VIEW PANEL (S86) — Tags und Cover des laufenden Titels, nur Anzeige
    // =========================================================================

    registry.registerPanel(
        PanelDescriptor{
            "tag_view",         // id
            "Tag View",         // title
            245,                // order (nach dem Audio Browser)
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<TagViewPanel>(svc);
        },
        false);

    // =========================================================================
    // LYRICS SYNC PANEL (S87) — Songtext nachsynchronisieren (Zeiten je Zeile)
    // =========================================================================

    registry.registerPanel(
        PanelDescriptor{
            "lyrics_sync",      // id
            "Lyrics Sync",      // title
            246,                // order (nach Tag View)
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<LyricsSyncPanel>(svc);
        },
        false);

    // =========================================================================
    // KARAOKE PANEL (S89, KE2) — Songtext ueber jedem Preset: Schalter,
    // Karaoke-Presets, Aussehen der Zeile
    // =========================================================================

    registry.registerPanel(
        PanelDescriptor{
            "karaoke",          // id
            "Karaoke",          // title
            247,                // order (nach Lyrics Sync)
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<KaraokePanel>(svc);
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
    // COMPOSER PANEL — Composer-Naht N8 (docs/ui/Composer_Konzept.md §12.2)
    // =========================================================================
    // Visuals ueber der Zeit: Transport, Zeitachse, Kanaele und Clips

    registry.registerPanel(
        PanelDescriptor{
            "composer",         // id
            "Composer",         // title
            330,                // order (nach Effect Chain)
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<lumi::composer::ComposerPanel>(svc);
        },
        false);

    // =========================================================================
    // LOOP PANEL (docs/audio/Loop_Konzept.md) — ein Loop aus einer `.lvloop`:
    // laden und abspielen; das Raster folgt
    // =========================================================================

    registry.registerPanel(
        PanelDescriptor{
            "loop",             // id
            "Loop",             // title
            335,                // order (nach dem Composer)
            false,              // defaultVisible (hidden by default)
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<lumi::loop::LoopPanel>(svc);
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
