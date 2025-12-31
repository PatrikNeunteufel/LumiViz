# Registry Lazy-Init Pattern

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Status:** Aktuell

---

## Übersicht

Das Lazy-Init Pattern löst das Problem der "Dead Code Elimination" bei statischen Libraries.

```
┌──────────────────────────────────────────────────────────────────────────┐
│                         Lazy-Init Pattern                                 │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│   ┌─────────────────────┐         ┌─────────────────────┐                │
│   │   XxxRegistry.cpp   │         │   XxxAutoReg.cpp    │                │
│   │   (Framework)       │         │   (App-spezifisch)  │                │
│   ├─────────────────────┤         ├─────────────────────┤                │
│   │ extern initXxxDefaults()─────►│ void initXxxDefaults() │             │
│   │                     │         │ {                   │                │
│   │ instance() {        │         │   registry.register();│              │
│   │   if (!initialized) │         │   registry.register();│              │
│   │     initXxxDefaults();│        │ }                   │                │
│   │ }                   │         │                     │                │
│   └─────────────────────┘         └─────────────────────┘                │
│                                                                           │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## Das Problem

### Dead Code Elimination

Bei statischen Libraries entfernt der Linker unreferenzierte Translation Units:

```cpp
// In MyPanel.cpp - WIRD VOM LINKER ENTFERNT!
static bool registered = []() {
    PanelRegistry::instance().registerPanel(...);
    return true;
}();
```

Der Linker sieht:
- `registered` wird nirgends verwendet
- → Translation Unit wird entfernt
- → Panel wird nie registriert

---

## Die Lösung

### Erzwungene Linkage durch extern

```cpp
// XxxRegistry.cpp (Framework):
extern void initXxxDefaults(XxxRegistry& registry);  // ← Deklaration

XxxRegistry& XxxRegistry::instance()
{
    static XxxRegistry registry;
    static bool initialized = false;
    
    if (!initialized)
    {
        initialized = true;
        initXxxDefaults(registry);  // ← Aufruf erzwingt Linkage
    }
    
    return registry;
}

// XxxAutoReg.cpp (App-spezifisch):
void initXxxDefaults(XxxRegistry& registry)  // ← Definition MUSS existieren
{
    registry.registerXxx(...);
    registry.registerXxx(...);
}
```

### Warum funktioniert das?

1. `extern` Deklaration ohne Definition = Linker-Fehler
2. Linker MUSS `XxxAutoReg.cpp` einbinden
3. → Registrierungen werden garantiert ausgeführt

---

## Alle 5 Registries

| Registry | AutoReg-Datei | Pfad |
|----------|---------------|------|
| MenuRegistry | MenuAutoReg.cpp | `src/UI/managers/` |
| VisualizerRegistry | VisualizerAutoReg.cpp | `src/visualizers/` |
| PanelRegistry | PanelAutoReg.cpp | `src/UI/panels/` |
| DialogRegistry | DialogAutoReg.cpp | `src/UI/dialogs/` |
| WidgetRegistry | WidgetAutoReg.cpp | `src/UI/widgets/` |

---

## Implementierung: MenuRegistry

### MenuRegistry.cpp

```cpp
#include "services/MenuRegistry.hpp"

extern void initMenuDefaults(MenuRegistry& registry);

MenuRegistry& MenuRegistry::instance()
{
    static MenuRegistry registry;
    static bool initialized = false;
    
    if (!initialized)
    {
        initialized = true;
        initMenuDefaults(registry);
    }
    
    return registry;
}
```

### MenuAutoReg.cpp

```cpp
#include "services/MenuRegistry.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

void initMenuDefaults(MenuRegistry& registry)
{
    // File Menu
    registry.registerContainer(
        MenuContainerDesc{{"menu.file", "", 100}, "File", false});
    
    registry.registerItem(
        MenuItemDesc{
            {"menu.file.exit", "menu.file", 900},
            "Exit",
            [](ServiceContainer&) { qApp->quit(); },
            {}, {}, "Alt+F4"
        });
    
    // View Menu
    registry.registerContainer(
        MenuContainerDesc{{"menu.view", "", 200}, "View", false});
    
    // ... weitere Menü-Items
}
```

---

## Implementierung: PanelRegistry

### PanelRegistry.cpp

```cpp
extern void initPanelDefaults(PanelRegistry& registry);

PanelRegistry& PanelRegistry::instance()
{
    static PanelRegistry registry;
    static bool initialized = false;
    
    if (!initialized)
    {
        initialized = true;
        initPanelDefaults(registry);
    }
    
    return registry;
}
```

### PanelAutoReg.cpp

```cpp
#include "services/PanelRegistry.hpp"
#include "UI/panels/PlayerPanel.hpp"
#include "UI/panels/PlaylistPanel.hpp"
#include "UI/panels/ConfigPanel.hpp"
#include "UI/panels/VisualSelectPanel.hpp"

void initPanelDefaults(PanelRegistry& registry)
{
    // Player Panel
    registry.registerPanel(
        PanelDescriptor{
            "player", "Player", 100, true, "View/Panels"
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<PlayerPanel>(svc);
        });
    
    // Playlist Panel
    registry.registerPanel(
        PanelDescriptor{
            "playlist", "Playlist", 200, true, "View/Panels"
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<PlaylistPanel>(svc);
        });
    
    // Config Panel (Settings)
    registry.registerPanel(
        PanelDescriptor{
            "config", "Settings", 300, false, "View/Panels"  // defaultVisible=false
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<ConfigPanel>(svc);
        });
    
    // Visual Select Panel
    registry.registerPanel(
        PanelDescriptor{
            "visual_select", "Visualizers", 400, true, "View/Panels"
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<VisualSelectPanel>(svc);
        });
}
```

---

## Implementierung: WidgetRegistry

### Besonderheit: allowMultiple

```cpp
struct WidgetDescriptor
{
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    int order = 0;
    bool allowMultiple = false;  // ← NEU: Erlaubt mehrere Instanzen
};
```

### WidgetAutoReg.cpp

```cpp
#include "services/WidgetRegistry.hpp"
#include "UI/widgets/VisualizerWidget.hpp"

void initWidgetDefaults(WidgetRegistry& registry)
{
    // VisualizerWidget - allowMultiple = false
    registry.registerWidget(
        WidgetDescriptor{
            "visualizer",
            "Visualizer",
            "Visualizers",
            "OpenGL visualization widget",
            100,
            false  // allowMultiple = false
        },
        [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QWidget> {
            return std::make_unique<VisualizerWidget>(svc, parent);
        });
}
```

### Verwendung von allowMultiple

```cpp
// In DockManager::subscribeToEvents()
eventBus->subscribe<CreateVisualizerEvent>(
    [this](const CreateVisualizerEvent& e) {
        const auto* desc = WidgetRegistry::instance().descriptor("visualizer");
        
        if (desc && !desc->allowMultiple && !m_impl->visualizers.empty()) {
            // Existiert bereits - nicht erstellen
            return;
        }
        
        createVisualizer(...);
    });
```

---

## Registrierte Komponenten

### Panels

| ID | Titel | Order | Default Visible |
|----|-------|-------|-----------------|
| player | Player | 100 | ✅ |
| playlist | Playlist | 200 | ✅ |
| config | Settings | 300 | ❌ |
| visual_select | Visualizers | 400 | ✅ |

### Dialoge

| ID | Titel | Order | Modal | Shortcut |
|----|-------|-------|-------|----------|
| about | About MyViz | 900 | ✅ | F1 |

### Widgets

| ID | Name | allowMultiple |
|----|------|---------------|
| visualizer | Visualizer | ❌ |

### Visualizers

| ID | Name | Kategorie |
|----|------|-----------|
| pulsing | Pulsing | Effects |

---

## Neue Komponente hinzufügen

### 1. Panel hinzufügen

```cpp
// In PanelAutoReg.cpp
registry.registerPanel(
    PanelDescriptor{
        "mypanel",      // id
        "My Panel",     // title
        500,            // order
        true,           // defaultVisible
        "View/Panels"   // menuPath
    },
    [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
        return std::make_unique<MyPanel>(svc);
    });
```

### 2. Dialog hinzufügen

```cpp
// In DialogAutoReg.cpp
registry.registerDialog(
    DialogDescriptor{
        "mydialog",     // id
        "My Dialog",    // title
        100,            // order
        true,           // modal
        "Help",         // menuPath
        "F2"            // shortcut
    },
    [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QDialog> {
        return std::make_unique<MyDialog>(svc, parent);
    });
```

### 3. Visualizer hinzufügen

```cpp
// In VisualizerAutoReg.cpp
registry.registerVisualizer(
    VisualizerDescriptor{
        "myvis",        // id
        "My Visualizer",// name
        "Effects",      // category
        "Cool effect",  // description
        200             // order
    },
    []() -> std::unique_ptr<IVisualizer> {
        return std::make_unique<MyVisualizer>();
    });
```

---

## Source.cmake Anpassung

Jede AutoReg-Datei muss in der entsprechenden `Source.cmake` eingetragen sein:

```cmake
# src/UI/panels/Source.cmake
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/PanelAutoReg.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PlayerPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PlaylistPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ConfigPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/VisualSelectPanel.cpp"
)
```

---

## Vorteile des Patterns

| Aspekt | Vorteil |
|--------|---------|
| **Linkage** | Garantiert durch extern-Referenz |
| **Debugging** | Klarer Call-Stack |
| **Trennung** | Framework vs. App-Code |
| **Erweiterbar** | Neue Komponenten ohne Framework-Änderung |
| **Testbar** | AutoReg kann gemockt werden |

---

## Siehe auch

- [Registry Architecture](Registry_Architecture.md) - Registry-Grundlagen
- [Panel System](../modules/Panel_System.md) - Panel-Details
- [Menu System](../modules/Menu_System.md) - Menü-Details
