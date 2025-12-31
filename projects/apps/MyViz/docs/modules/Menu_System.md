# Menu System

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Status:** Aktuell

---

## Übersicht

Das Menü-System verwendet Self-Registration für dezentrale Menü-Definition.

```
┌──────────────────────────────────────────────────────────────────────────┐
│                           Menu System                                     │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│   ┌─────────────────────┐         ┌─────────────────────┐                │
│   │    MenuRegistry     │◄────────│   MenuAutoReg.cpp   │                │
│   │   (Menü-Struktur)   │         │   (Registrierungen) │                │
│   └─────────┬───────────┘         └─────────────────────┘                │
│             │                                                             │
│             ▼                                                             │
│   ┌─────────────────────┐         ┌─────────────────────┐                │
│   │    MenuManager      │────────►│     QMenuBar        │                │
│   │   (Menü aufbauen)   │         │   (Qt Widget)       │                │
│   └─────────────────────┘         └─────────────────────┘                │
│                                                                           │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## Komponenten

### MenuRegistry

Speichert die Menü-Struktur (Containers, Groups, Items).

```cpp
class MenuRegistry
{
public:
    static MenuRegistry& instance();
    
    // Container (Menü/Submenü)
    void registerContainer(const MenuContainerDesc& desc, bool overwrite = false);
    
    // Group (logische Gruppierung mit Separator)
    void registerGroup(const MenuGroupDesc& desc, bool overwrite = false);
    
    // Item (Aktion)
    void registerItem(const MenuItemDesc& desc, bool overwrite = false);
    
    // Abfrage
    const MenuContainerDesc* container(const std::string& id) const;
    const MenuGroupDesc* group(const std::string& id) const;
    const MenuItemDesc* item(const std::string& id) const;
    
    std::vector<MenuNode> children(const std::string& parentId) const;
};
```

### MenuManager

Baut die Qt-Menüstruktur auf Basis der Registry.

```cpp
class MenuManager : public QObject
{
public:
    MenuManager(ServiceContainer& services, QObject* parent = nullptr);
    
    void buildMenuBar(QMenuBar* menuBar);
    QMenu* menu(const QString& id) const;
    QAction* action(const QString& id) const;
};
```

---

## Descriptor-Strukturen

### MenuNodeDesc (Basis)

```cpp
struct MenuNodeDesc
{
    std::string id;        // Eindeutige ID (z.B. "menu.file.exit")
    std::string parentId;  // Parent ID (z.B. "menu.file")
    int order;             // Sortierung (100, 200, ...)
};
```

### MenuContainerDesc

```cpp
struct MenuContainerDesc
{
    MenuNodeDesc node;
    std::string title;       // Anzeigename
    bool isMenuBar = false;  // True für Top-Level
};
```

### MenuGroupDesc

```cpp
struct MenuGroupDesc
{
    MenuNodeDesc node;
    // Gruppen haben keinen Titel - nur Separator vor/nach
};
```

### MenuItemDesc

```cpp
struct MenuItemDesc
{
    MenuNodeDesc node;
    std::string title;
    MenuActionFn action;                    // [](ServiceContainer&) { ... }
    std::optional<MenuCheckedFn> isChecked; // Für Checkable Items
    std::optional<MenuEnabledFn> isEnabled; // Für Disabled Items
    std::optional<std::string> shortcut;    // "Ctrl+N", "F1", etc.
};
```

---

## Menü-Hierarchie

```
menu                              (Root - implizit)
├── menu.file                     (Container: "File")
│   ├── menu.file.exit            (Item: "Exit")
│
├── menu.view                     (Container: "View")
│   ├── menu.view.panels          (Container: "Panels")
│   │   └── [von DockManager]
│   ├── menu.view.perspectives    (Container: "Perspectives")
│   │   └── [von DockManager]
│   ├── menu.view.newvisualizer   (Item: "New Visualizer")
│   ├── menu.view.group.layout    (Group - Separator)
│   ├── menu.view.resetlayout     (Item: "Reset Layout")
│   └── menu.view.savedefault     (Item: "Save Layout as Default")
│
├── menu.settings                 (Container: "Settings")
│   ├── menu.settings.vsync       (Item: Checkable "VSync")
│   ├── menu.settings.framemode   (Container: "Frame Mode")
│   │   ├── unlimited             (Item: Exclusive "Unlimited")
│   │   ├── fps60                 (Item: Exclusive "60 FPS")
│   │   └── fps30                 (Item: Exclusive "30 FPS")
│
└── menu.help                     (Container: "Help")
    └── menu.help.about           (Item: "About")
```

---

## MenuAutoReg.cpp

### Struktur

```cpp
#include "services/MenuRegistry.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

void initMenuDefaults(MenuRegistry& registry)
{
    // =========================================================================
    // FILE MENU
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.file", "", 100}, "File", false});
    
    registry.registerItem(
        MenuItemDesc{
            {"menu.file.exit", "menu.file", 900},
            "Exit",
            [](ServiceContainer&) { qApp->quit(); },
            {}, {}, "Alt+F4"
        });
    
    // =========================================================================
    // VIEW MENU
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.view", "", 200}, "View", false});
    
    // Panels submenu - populated by DockManager
    registry.registerContainer(
        MenuContainerDesc{{"menu.view.panels", "menu.view", 100}, "Panels", false});
    
    // New Visualizer
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.newvisualizer", "menu.view", 50},
            "New Visualizer",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>()) {
                    eventBus->publish(CreateVisualizerEvent{});
                }
            },
            {}, {}, "Ctrl+N"
        });
    
    // Reset Layout
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.resetlayout", "menu.view", 900},
            "Reset Layout",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>()) {
                    eventBus->publish(ResetLayoutEvent{});
                }
            }
        });
    
    // =========================================================================
    // SETTINGS MENU
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.settings", "", 300}, "Settings", false});
    
    // VSync (Checkable)
    registry.registerItem(
        MenuItemDesc{
            {"menu.settings.vsync", "menu.settings", 100},
            "VSync",
            [](ServiceContainer& svc) {
                // Toggle VSync
            },
            [](ServiceContainer&) -> bool { return g_vsyncEnabled; },  // isChecked
            {}, {}
        });
    
    // =========================================================================
    // HELP MENU
    // =========================================================================
    
    registry.registerContainer(
        MenuContainerDesc{{"menu.help", "", 900}, "Help", false});
    
    registry.registerItem(
        MenuItemDesc{
            {"menu.help.about", "menu.help", 900},
            "About",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>()) {
                    eventBus->publish(OpenDialogEvent{"about"});
                }
            },
            {}, {}, "F1"
        });
}
```

---

## Exclusive Menu Items (QActionGroup)

### Registration

```cpp
// Frame Mode Submenu
registry.registerContainer(
    MenuContainerDesc{{"menu.settings.framemode", "menu.settings", 200}, "Frame Mode", false});

// Exclusive Group
registry.registerGroup(
    MenuGroupDesc{{"menu.settings.framemode.group", "menu.settings.framemode", 100}});

// Items mit actionGroup
registry.registerItem(
    MenuItemDesc{
        {"menu.settings.framemode.unlimited", "menu.settings.framemode", 100},
        "Unlimited",
        [](ServiceContainer& svc) {
            if (auto* eventBus = svc.tryResolve<IEventBus>()) {
                eventBus->publish(FrameModeChangedEvent{FrameMode::Unlimited});
            }
        },
        [](ServiceContainer&) { return g_frameMode == FrameMode::Unlimited; },  // isChecked
        {},
        {},
        "menu.settings.framemode.group"  // actionGroup
    });
```

### MenuManager Handling

```cpp
void MenuManager::buildMenuRecursive(QMenu* parentMenu, const QString& parentId)
{
    // ...
    
    // Für Items mit actionGroup
    if (desc->actionGroup.has_value())
    {
        QString groupId = QString::fromStdString(*desc->actionGroup);
        
        // ActionGroup erstellen oder finden
        if (!m_actionGroups.contains(groupId))
        {
            auto* group = new QActionGroup(this);
            group->setExclusive(true);
            m_actionGroups.insert(groupId, group);
        }
        
        action->setCheckable(true);
        m_actionGroups[groupId]->addAction(action);
    }
}
```

---

## Panels-Menü

Das Panels-Menü wird von `DockManager::populatePanelsMenu()` befüllt:

```cpp
void DockManager::populatePanelsMenu(QMenu* pMenu)
{
    for (auto* pDock : m_impl->pAdsDockManager->dockWidgetsMap())
    {
        if (pDock != nullptr)
        {
            pMenu->addAction(pDock->toggleViewAction());
        }
    }
}
```

Die `toggleViewAction()` von Qt-ADS:
- Ist automatisch checkable
- Synchronisiert mit Panel-Sichtbarkeit
- Aktualisiert Häkchen automatisch

---

## Neues Menü-Item hinzufügen

### 1. Einfaches Item

```cpp
// In MenuAutoReg.cpp
registry.registerItem(
    MenuItemDesc{
        {"menu.file.open", "menu.file", 100},
        "Open...",
        [](ServiceContainer& svc) {
            // Handle open
        },
        {}, {}, "Ctrl+O"
    });
```

### 2. Checkable Item

```cpp
registry.registerItem(
    MenuItemDesc{
        {"menu.settings.fullscreen", "menu.settings", 500},
        "Fullscreen",
        [](ServiceContainer& svc) {
            // Toggle fullscreen
            g_fullscreen = !g_fullscreen;
        },
        [](ServiceContainer&) { return g_fullscreen; },  // isChecked
        {}
    });
```

### 3. Neues Submenü

```cpp
// Container
registry.registerContainer(
    MenuContainerDesc{{"menu.file.recent", "menu.file", 200}, "Recent Files", false});

// Items im Submenü
registry.registerItem(
    MenuItemDesc{
        {"menu.file.recent.file1", "menu.file.recent", 100},
        "file1.mp3",
        [](ServiceContainer& svc) { /* open file1 */ }
    });
```

---

## Best Practices

### 1. ID-Konvention

```
menu.<top-level>.<item>
menu.<top-level>.<submenu>.<item>
```

### 2. Order-Konvention

- 100er-Schritte für Erweiterbarkeit
- Exit immer 900
- About immer 900

### 3. Events statt direkter Aktionen

```cpp
// GUT: Event publizieren
[](ServiceContainer& svc) {
    if (auto* eventBus = svc.tryResolve<IEventBus>()) {
        eventBus->publish(MyEvent{});
    }
}

// SCHLECHT: Direkte Manipulation
[](ServiceContainer& svc) {
    svc.resolve<MyService>().doSomething();  // Tight coupling!
}
```

---

## Siehe auch

- [Registry Architecture](../architecture/Registry_Architecture.md) - Registry Grundlagen
- [Registry LazyInit](../architecture/Registry_LazyInit.md) - Lazy-Init Pattern
- [Event Architecture](../architecture/Event_Architecture.md) - Event-Handling
