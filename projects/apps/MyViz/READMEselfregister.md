# Menu Self-Registration Refactoring

> **Datum:** 2025-12-31  
> **Version:** 2.0.0  

---

## Zusammenfassung

Das Menü-System wurde von **hardcoded** auf **Self-Registration** umgestellt.

### Vorher (Hardcoded)

```
MainWindow::setupMenus()
├── File (hardcoded)
│   ├── Open Audio... (hardcoded)
│   └── Exit (hardcoded)
├── View ← DockManager::createViewMenu()
│   ├── New Visualizer (hardcoded)
│   ├── Panels (von Qt-ADS)
│   ├── Perspectives (hardcoded)
│   └── Reset Layout (hardcoded)
├── Settings (hardcoded)
│   └── Frame Mode (hardcoded)
└── Help (hardcoded)
    └── About MyViz (hardcoded)
```

### Nachher (Self-Registration)

```
MenuAutoReg.cpp (Container-Struktur)
├── menu.file
├── menu.view
│   ├── menu.view.panels
│   └── menu.view.perspectives
├── menu.settings
│   └── menu.settings.framemode
└── menu.help

MenuItemsAutoReg.cpp (Items - später dezentralisieren)
├── menu.file.open → "Open Audio..."
├── menu.view.newvisualizer → "New Visualizer"
├── menu.view.resetlayout → "Reset Layout"
├── menu.settings.framemode.limited → "Limited (60 FPS)"
├── menu.settings.framemode.unlimited → "Unlimited"
├── menu.settings.framemode.vsync → "VSync"
└── menu.help.about → "About MyViz..."

MainWindow
└── setupMenuBar() → MenuManager::buildMenuBar()
└── setupEventHandlers() → EventBus Subscriptions
```

---

## Geänderte Dateien

| Datei | Änderung |
|-------|----------|
| **MenuAutoReg.cpp** | Erweitert um Settings-Container, Groups |
| **MenuItemsAutoReg.cpp** | **NEU** - Alle Menu-Items |
| **MenuManager.cpp** | Unverändert (nutzt Registry) |
| **DockManager.hpp** | +populatePanelsMenu(), +populatePerspectivesMenu() |
| **DockManager.cpp** | +Implementierung der neuen Methoden |
| **MainWindow.hpp** | +MenuManager Member, +setupEventHandlers() |
| **MainWindow.cpp** | setupMenuBar() → MenuManager, +setupEventHandlers() |
| **UIEvents.hpp** | +CreateVisualizerEvent, +ResetLayoutEvent |
| **managers/Source.cmake** | +MenuItemsAutoReg.cpp |

---

## Architektur

### Datenfluss

```
┌─────────────────────────────────────────────────────────────────────────┐
│                      Static Initialization                               │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  MenuAutoReg.cpp                    MenuItemsAutoReg.cpp                │
│  ┌─────────────────┐                ┌─────────────────────┐             │
│  │ REGISTER_MENU_  │                │ REGISTER_MENU_ITEM  │             │
│  │ CONTAINER(...)  │                │ (...callback...)    │             │
│  └────────┬────────┘                └──────────┬──────────┘             │
│           │                                    │                         │
│           └──────────────┬─────────────────────┘                        │
│                          ▼                                               │
│                 ┌─────────────────┐                                      │
│                 │  MenuRegistry   │                                      │
│                 │  (Singleton)    │                                      │
│                 └────────┬────────┘                                      │
└──────────────────────────┼──────────────────────────────────────────────┘
                           │
┌──────────────────────────┼──────────────────────────────────────────────┐
│                     Runtime                                              │
├──────────────────────────┼──────────────────────────────────────────────┤
│                          ▼                                               │
│                 ┌─────────────────┐                                      │
│                 │  MenuManager    │                                      │
│                 │ buildMenuBar()  │                                      │
│                 └────────┬────────┘                                      │
│                          │                                               │
│           ┌──────────────┼──────────────┐                               │
│           ▼              ▼              ▼                                │
│     ┌──────────┐  ┌──────────┐  ┌──────────────┐                        │
│     │ QMenuBar │  │ QMenu    │  │ QAction      │                        │
│     │          │  │ (File,   │  │ (Exit,       │                        │
│     │          │  │  View,..)│  │  Open, ...)  │                        │
│     └──────────┘  └──────────┘  └──────┬───────┘                        │
│                                        │                                 │
│                                        │ triggered()                     │
│                                        ▼                                 │
│                               ┌─────────────────┐                        │
│                               │ MenuRegistry::  │                        │
│                               │ item->onClick() │                        │
│                               └────────┬────────┘                        │
│                                        │                                 │
│                                        │ EventBus::publish()             │
│                                        ▼                                 │
│                               ┌─────────────────┐                        │
│                               │    EventBus     │                        │
│                               └────────┬────────┘                        │
│                                        │                                 │
│                                        │ notify subscribers              │
│                                        ▼                                 │
│                               ┌─────────────────┐                        │
│                               │   MainWindow    │                        │
│                               │ (Event Handler) │                        │
│                               └─────────────────┘                        │
└─────────────────────────────────────────────────────────────────────────┘
```

### Event-Typen für Menü-Aktionen

| Event | Beschreibung |
|-------|--------------|
| `CreateVisualizerEvent` | Neuen Visualizer erstellen |
| `ResetLayoutEvent` | Layout zurücksetzen |
| `FrameModeChangedEvent` | Frame-Modus ändern (0/1/2) |
| `OpenDialogEvent` | Dialog öffnen (z.B. "about") |

---

## Zukünftige Dezentralisierung

Die Items in `MenuItemsAutoReg.cpp` sollten später in die jeweiligen Komponenten verschoben werden:

```cpp
// Beispiel: AboutDialog.cpp
REGISTER_DIALOG("about", "About MyViz", AboutDialog)
REGISTER_MENU_ITEM("menu.help.about", "About MyViz...", "menu.help", 100,
    [](ServiceContainer& svc) {
        svc.resolve<DialogManager>().open("about");
    })

// Beispiel: AudioPlayer.cpp
REGISTER_MENU_ITEM_SHORTCUT("menu.file.open", "Open Audio...", 
    "menu.file", 100, "Ctrl+O",
    [](ServiceContainer& svc) {
        svc.resolve<AudioPlayer>().openFileDialog();
    })
```

---

## TODO

1. ~~**QActionGroup für Frame Mode** - Die drei Frame-Mode-Items sollten exklusiv sein~~ ✅ Implementiert via `exclusive` Flag
2. **Dezentralisierung** - Items in Komponenten-Dateien verschieben
3. **DialogManager Integration** - About-Dialog implementieren
4. **Settings Persistenz** - Frame-Mode-Auswahl speichern

---

## Neue Features

### Exklusive Container (QActionGroup)

Container können als `exclusive` markiert werden. Alle Items darin verhalten sich dann wie Radio-Buttons (nur eines kann aktiv sein):

```cpp
// In MenuAutoReg.cpp:
REGISTER_MENU_CONTAINER_EXCLUSIVE("menu.settings.framemode", "Frame Mode", 
                                   "menu.settings", 100)

// Items darin sind automatisch exklusiv:
REGISTER_MENU_ITEM_CHECKED("menu.settings.framemode.limited", "Limited (60 FPS)", ...)
REGISTER_MENU_ITEM_CHECKED("menu.settings.framemode.unlimited", "Unlimited", ...)
REGISTER_MENU_ITEM_CHECKED("menu.settings.framemode.vsync", "VSync", ...)
```
