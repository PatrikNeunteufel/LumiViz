/**
 ****************************************************************************************
 * @file   MenuAutoReg.cpp
 * @brief  Application-specific menu registrations for LumiViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.2.0
 *
 * @details
 * Diese Datei definiert die **LumiViz-spezifische Menüstruktur**.
 * Sie wird automatisch von MenuRegistry::instance() beim ersten Zugriff aufgerufen.
 *
 * ## Menü-Struktur
 *
 * ```
 * File (100)
 * ├── Open Audio... (100)    [Ctrl+O]
 * ├── … Effect Chain, Composition …
 * ├── Publish a file... (510)
 * ├── Import package... (520)
 * ├── ─────────────── (800)  [Separator]
 * └── Exit (900)             [Alt+F4]
 *
 * Edit (150)
 * ├── Undo (100)             [Ctrl+Z]
 * └── Redo (200)             [Ctrl+Y]
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
 * └── About LumiViz... (100)   [F1]
 * ```
 *
 * ## Trennung Framework vs. App
 *
 * - **MenuRegistry.hpp/cpp** = Wiederverwendbares Framework
 * - **MenuAutoReg.cpp** = LumiViz-spezifische Menüs (diese Datei)
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
#include "services/ICommandBus.hpp"
#include "services/events/UIEvents.hpp"
#include "composer/AppAnbindung.hpp"  // Composer-Naht N8
#include "UI/dialogs/PaketDialoge.hpp"  // Publish (S125)

#include <QApplication>
#include <QMainWindow>
#include <QSettings>
#include <BasicLogger.h>

namespace
{
/// Das Hauptfenster als Eltern der Dialoge aus dem Menue.
QWidget* hauptfenster()
{
    for (QWidget* w : QApplication::topLevelWidgets())
        if (qobject_cast<QMainWindow*>(w) != nullptr) return w;
    return QApplication::activeWindow();
}
}  // namespace

// =============================================================================
// initMenuDefaults - Called by MenuRegistry::instance() on first access
// =============================================================================

/**
 * @brief Register all default menus for LumiViz
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
    
    // Group for AVS/effect-chain import & host presets (Import Roadmap 5.7)
    registry.registerGroup(
        MenuGroupDesc{{"menu.file.group.import", "menu.file", 300}},
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.file.importavs", "menu.file", 300},
            "Import AVS Preset...",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                    eventBus->publish(ImportAvsPresetEvent{});
            },
            {}, {}, "Ctrl+I"
        },
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.file.newchain", "menu.file", 305},
            "New Effect Chain",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                    eventBus->publish(NewEffectChainEvent{});
            },
            {}, {}, "Ctrl+Shift+N"
        },
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.file.loadchain", "menu.file", 310},
            "Load Effect Chain...",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                    eventBus->publish(LoadEffectChainEvent{});
            },
            {}, {}, ""
        },
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.file.savechain", "menu.file", 320},
            "Save Effect Chain",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                    eventBus->publish(SaveEffectChainEvent{false});
            },
            {}, {}, "Ctrl+S"
        },
        false);

    // S89 (Entscheid Patrik 3a): Save ueberschreibt still nur eine eigene
    // .lvfx/.lvfx2, Save As fragt immer (Regel: UI/PresetSpeichern.hpp)
    registry.registerItem(
        MenuItemDesc{
            {"menu.file.savechainas", "menu.file", 325},
            "Save Effect Chain As...",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                    eventBus->publish(SaveEffectChainEvent{true});
            },
            {}, {}, "Ctrl+Shift+S"
        },
        false);

    // Composer-Naht N8 (docs/ui/Composer_Konzept.md §12.2): eine Komposition
    // ein- und ausschalten, solange es kein Panel gibt (C2a)
    registry.registerGroup(
        MenuGroupDesc{{"menu.file.group.composer", "menu.file", 400}},
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.file.opencomposition", "menu.file", 410},
            "Open Composition...",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                    eventBus->publish(lumi::composer::OpenCompositionEvent{});
            },
            {}, {}, ""
        },
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.file.closecomposition", "menu.file", 420},
            "Close Composition",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                    eventBus->publish(lumi::composer::CloseCompositionEvent{});
            },
            {}, {}, ""
        },
        false);

    // Publish (docs/core-services/Publish_Konzept.md, Entscheid Patrik P2 a, S125):
    // eine Datei mit allem, was sie nennt, als Paket — und ein Paket importieren
    registry.registerGroup(
        MenuGroupDesc{{"menu.file.group.publish", "menu.file", 500}},
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.file.publishfile", "menu.file", 510},
            "Publish a file...",
            [](ServiceContainer& /*svc*/) { lumi::ui::zeigePublishDatei(hauptfenster()); },
            {}, {}, ""
        },
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.file.importpackage", "menu.file", 520},
            "Import package...",
            [](ServiceContainer& svc) {
                lumi::ui::zeigeImportPaket(hauptfenster(), QString(), svc.tryResolve<IEventBus>());
            },
            {}, {}, ""
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
            [](ServiceContainer& /*svc*/) {
                // Das Fenster SCHLIESSEN statt die App zu beenden (S89): nur so
                // laeuft die Rueckfrage zu ungespeicherten Aenderungen
                // (MainWindow::closeEvent); das Fenster beendet danach selbst.
                for (QWidget* w : QApplication::topLevelWidgets())
                    if (qobject_cast<QMainWindow*>(w) != nullptr)
                    {
                        w->close();
                        return;
                    }
                QApplication::quit();
            },
            {},  // isChecked
            {},  // isEnabled
            "Alt+F4"
        },
        false);
    
    // =========================================================================
    // EDIT MENU (150) — Undo/Redo via CommandBus (Phase 4)
    // =========================================================================

    registry.registerContainer(
        MenuContainerDesc{{"menu.edit", "toplevel", 150}, "Edit", false},
        false);

    registry.registerGroup(
        MenuGroupDesc{{"menu.edit.group.history", "menu.edit", 100}},
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.edit.undo", "menu.edit", 100},
            "Undo",
            [](ServiceContainer& svc) {
                if (auto* commandBus = svc.tryResolve<ICommandBus>())
                {
                    commandBus->undo();
                }
            },
            {},  // isChecked
            {},  // isEnabled (no-op undo on empty history is harmless)
            "Ctrl+Z"
        },
        false);

    registry.registerItem(
        MenuItemDesc{
            {"menu.edit.redo", "menu.edit", 200},
            "Redo",
            [](ServiceContainer& svc) {
                if (auto* commandBus = svc.tryResolve<ICommandBus>())
                {
                    commandBus->redo();
                }
            },
            {},  // isChecked
            {},  // isEnabled
            "Ctrl+Y"
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
    
    // Fullscreen — die Taste (Vorgabe F11, umbelegbar) gehoert der
    // Tastenkuerzel-Tabelle `view.fullscreen`; das Menue zeigt sie nur (§9.5)
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.fullscreen", "menu.view", 150},
            "Fullscreen",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                {
                    eventBus->publish(ToggleFullscreenEvent{});
                }
                BasicLogger::logDebug("Fullscreen toggled");
            },
            {},  // isChecked
            {},  // isEnabled
            {},  // shortcut
            "view.fullscreen"
        },
        false);
    
    // Karaoke an/aus (S89, KE4): Songtext ueber jedem Preset. Die Taste steht
    // in der Tastenkuerzel-Tabelle (`view.karaoke`, umbelegbar), nicht hier —
    // sonst loeste ein Druck zweimal aus; das Menue zeigt sie nur (§9.5, S129).
    // Der Haken kommt aus der gemerkten Einstellung: der Renderer wird erst
    // nach dem Menue geschaltet.
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.karaoke", "menu.view", 160},
            "Karaoke",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>())
                    eventBus->publish(KaraokeToggleEvent{});
            },
            [](ServiceContainer& /*svc*/) {
                return QSettings().value(QStringLiteral("karaoke/enabled"), false).toBool();
            },
            {},  // isEnabled
            {},  // shortcut
            "view.karaoke"
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
    
    // About LumiViz... (F1)
    registry.registerItem(
        MenuItemDesc{
            {"menu.help.about", "menu.help", 100},
            "About LumiViz...",
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
