# Registry Architecture

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Status:** Aktuell

---

## Übersicht

Das Registry-Pattern ermöglicht dezentrale Registrierung von Komponenten ohne Änderung am Framework-Code.

```
┌──────────────────────────────────────────────────────────────────────────┐
│                           Registry Pattern                                │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│   ┌─────────────────────┐         ┌─────────────────────┐                │
│   │    XxxRegistry      │◄────────│   XxxAutoReg.cpp    │                │
│   │   (Framework)       │         │   (App-spezifisch)  │                │
│   ├─────────────────────┤         ├─────────────────────┤                │
│   │ • Singleton         │         │ • initXxxDefaults() │                │
│   │ • Lazy-Init         │         │ • Alle Registrier.  │                │
│   │ • Type-Erasure      │         │ • Pro App anpassbar │                │
│   └─────────────────────┘         └─────────────────────┘                │
│                                                                           │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## Die 5 Registries

| Registry | Zweck | Komponenten |
|----------|-------|-------------|
| **MenuRegistry** | Menü-Struktur | Container, Groups, Items |
| **PanelRegistry** | Dock-Panels | PlayerPanel, PlaylistPanel, ... |
| **DialogRegistry** | Modale Dialoge | AboutDialog, ... |
| **WidgetRegistry** | Widgets | VisualizerWidget, ... |
| **VisualizerRegistry** | Visualizer-Effekte | PulsingVisualizer, ... |

---

## Lazy-Init Pattern

### Problem: Dead Code Elimination

Bei statischen Libraries entfernt der Linker unreferenzierte Translation Units.
Statische `REGISTER_*` Makros werden als "unused" betrachtet und entfernt.

### Lösung: Erzwungene Linkage

```cpp
// In XxxRegistry.cpp (Framework):
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

// In XxxAutoReg.cpp (App-spezifisch):
void initXxxDefaults(XxxRegistry& registry)  // ← Definition
{
    registry.registerXxx(...);
    registry.registerXxx(...);
}
```

### Vorteile

1. **Garantierte Linkage** - Linker kann AutoReg.cpp nicht entfernen
2. **Keine Makro-Magie** - Explizite Funktionsaufrufe
3. **Framework/App Trennung** - Registry wiederverwendbar
4. **Einfaches Debugging** - Klarer Call-Stack

---

## Registry-Struktur

### Gemeinsame Basis

Alle Registries folgen dem gleichen Pattern:

```cpp
class XxxRegistry
{
public:
    // Singleton
    static XxxRegistry& instance();
    
    // Registrierung
    void registerXxx(const XxxDescriptor& desc, Factory factory, bool overwrite = false);
    
    // Abfrage
    bool has(const std::string& id) const;
    const XxxDescriptor* descriptor(const std::string& id) const;
    std::vector<XxxDescriptor> descriptors() const;
    
    // Erstellung
    std::unique_ptr<XxxType> create(const std::string& id, ServiceContainer& svc);

private:
    XxxRegistry() = default;
    std::unordered_map<std::string, Entry> m_entries;
};
```

### Descriptor-Strukturen

```cpp
// MenuRegistry
struct MenuItemDesc {
    MenuNodeDesc node;      // id, parentId, order
    std::string title;
    MenuActionFn action;
    std::optional<MenuCheckedFn> isChecked;
    std::optional<MenuEnabledFn> isEnabled;
    std::optional<std::string> shortcut;
};

// PanelRegistry
struct PanelDescriptor {
    std::string id;
    std::string title;
    int order;
    bool defaultVisible;
    std::string menuPath;
};

// DialogRegistry
struct DialogDescriptor {
    std::string id;
    std::string title;
    int order;
    bool modal;
    std::string menuPath;
    std::string shortcut;
};

// WidgetRegistry
struct WidgetDescriptor {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    int order;
    bool allowMultiple;  // Erlaubt mehrere Instanzen
};

// VisualizerRegistry
struct VisualizerDescriptor {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    int order;
};
```

---

## AutoReg-Dateien

### Struktur

Jede Registry hat eine zugehörige AutoReg-Datei:

| Registry | AutoReg-Datei | Pfad |
|----------|---------------|------|
| MenuRegistry | MenuAutoReg.cpp | `src/UI/managers/` |
| PanelRegistry | PanelAutoReg.cpp | `src/UI/panels/` |
| DialogRegistry | DialogAutoReg.cpp | `src/UI/dialogs/` |
| WidgetRegistry | WidgetAutoReg.cpp | `src/UI/widgets/` |
| VisualizerRegistry | VisualizerAutoReg.cpp | `src/visualizers/` |

### Beispiel: PanelAutoReg.cpp

```cpp
#include "services/PanelRegistry.hpp"
#include "UI/panels/PlayerPanel.hpp"
#include "UI/panels/PlaylistPanel.hpp"
// ...

void initPanelDefaults(PanelRegistry& registry)
{
    // Player Panel
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
        });
    
    // Playlist Panel
    registry.registerPanel(
        PanelDescriptor{
            "playlist",
            "Playlist",
            200,
            true,
            "View/Panels"
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<PlaylistPanel>(svc);
        });
    
    // ... weitere Panels
}
```

---

## Neue Komponente hinzufügen

### 1. Neues Panel

```cpp
// 1. Panel-Klasse erstellen (von PanelBase ableiten)
class MyPanel : public PanelBase { ... };

// 2. In PanelAutoReg.cpp registrieren
registry.registerPanel(
    PanelDescriptor{"mypanel", "My Panel", 500, true, "View/Panels"},
    [](ServiceContainer& svc) { return std::make_unique<MyPanel>(svc); });

// 3. FERTIG - Erscheint automatisch im Menü und DockManager
```

### 2. Neuer Visualizer

```cpp
// 1. Visualizer-Klasse erstellen (von IVisualizer ableiten)
class MyVisualizer : public IVisualizer { ... };

// 2. In VisualizerAutoReg.cpp registrieren
registry.registerVisualizer(
    VisualizerDescriptor{"myvis", "My Visualizer", "Effects", "Cool effect", 500},
    []() { return std::make_unique<MyVisualizer>(); });

// 3. FERTIG - Erscheint automatisch in VisualSelectPanel
```

### 3. Neuer Dialog

```cpp
// 1. Dialog-Klasse erstellen (von QDialog ableiten)
class MyDialog : public QDialog { ... };

// 2. In DialogAutoReg.cpp registrieren
registry.registerDialog(
    DialogDescriptor{"mydialog", "My Dialog", 100, true, "Help", "F2"},
    [](ServiceContainer& svc, QWidget* parent) {
        return std::make_unique<MyDialog>(svc, parent);
    });

// 3. FERTIG - Erscheint automatisch im Menü
```

---

## Best Practices

### 1. IDs

- Lowercase, keine Leerzeichen: `player`, `visual_select`
- Eindeutig innerhalb der Registry
- Stabil (ändern bricht Layout-Persistence!)

### 2. Order

- 100er-Schritte für Erweiterbarkeit
- Logische Gruppierung (Player vor Playlist)

### 3. Factories

- Lambda mit ServiceContainer Parameter
- `std::make_unique` für Ownership

### 4. Trennung Framework/App

- Registry-Klasse: Wiederverwendbar
- AutoReg-Datei: App-spezifisch

---

## Siehe auch

- [Registry LazyInit](Registry_LazyInit.md) - Details zum Lazy-Init Pattern
- [Event Architecture](Event_Architecture.md) - Wie Events funktionieren
- [Panel System](../modules/Panel_System.md) - Panel-Details
