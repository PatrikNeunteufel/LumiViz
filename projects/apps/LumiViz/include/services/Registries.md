# Registries — Self-Registration Pattern

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Services::Registries  
> **Dateien:** PanelRegistry.hpp/cpp, DialogRegistry.hpp/cpp, MenuRegistry.hpp/cpp, VisualizerRegistry.hpp/cpp, WidgetRegistry.hpp/cpp + *AutoReg.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** C++17 STL, ServiceContainer  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Lazy-Init Pattern](#2-lazy-init-pattern)
3. [Registry-Typen](#3-registry-typen)
4. [AutoReg-Dateien](#4-autoreg-dateien)
5. [Verwendung](#5-verwendung)
6. [Best Practices](#6-best-practices)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Die **Registries** implementieren das **Self-Registration Pattern** mit **Lazy-Init** für automatische Komponenten-Registrierung. Anstatt alle Panels, Dialoge, Visualizer etc. zentral aufzulisten, werden sie in separaten `*AutoReg.cpp` Dateien registriert.

### 1.2 Die 5 Registries

| Registry | Zweck | AutoReg-Datei |
|----------|-------|---------------|
| **MenuRegistry** | Menü-Struktur | `src/UI/managers/MenuAutoReg.cpp` |
| **PanelRegistry** | Dock-Panels | `src/UI/panels/PanelAutoReg.cpp` |
| **DialogRegistry** | Modale Dialoge | `src/UI/dialogs/DialogAutoReg.cpp` |
| **WidgetRegistry** | Widgets | `src/UI/widgets/WidgetAutoReg.cpp` |
| **VisualizerRegistry** | Visualizer-Effekte | `src/visualizers/VisualizerAutoReg.cpp` |

### 1.3 Vorteile

| Aspekt | Zentrale Registrierung | Self-Registration |
|--------|------------------------|-------------------|
| **Wartung** | Eine große Liste | Dezentral in AutoReg |
| **Erweiterung** | Zentralen Code ändern | Nur AutoReg erweitern |
| **Kopplung** | Alle Includes nötig | Framework/App getrennt |
| **Linkage** | Probleme bei statischen Libs | ✅ Garantiert durch extern |

---

## 2. Lazy-Init Pattern

### 2.1 Das Problem

Bei statischen Libraries entfernt der Linker unreferenzierte Translation Units ("dead code elimination"). Statische `REGISTER_*` Makros werden als "unused" betrachtet und entfernt.

### 2.2 Die Lösung: Erzwungene Linkage

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
void initXxxDefaults(XxxRegistry& registry)  // ← Definition MUSS existieren
{
    registry.registerXxx(...);
    registry.registerXxx(...);
}
```

### 2.3 Warum funktioniert das?

1. `extern` Deklaration ohne Definition = Linker-Fehler
2. Linker MUSS `XxxAutoReg.cpp` einbinden
3. → Registrierungen werden garantiert ausgeführt

---

## 3. Registry-Typen

### 3.1 PanelRegistry

```cpp
struct PanelDescriptor {
    std::string id;              // Unique ID (= objectName für Layout)
    std::string title;           // Display name
    int order = 0;               // Sort order (100er Schritte)
    bool defaultVisible = true;  // Initial visibility
    std::string menuPath;        // Menu location
};

// Factory
using PanelFactory = std::function<std::unique_ptr<QWidget>(ServiceContainer&)>;
```

**Registrierte Panels:**

| ID | Titel | Order | Default Visible |
|----|-------|-------|-----------------|
| player | Player | 100 | ✅ |
| playlist | Playlist | 200 | ✅ |
| config | Settings | 300 | ❌ |
| visual_select | Visualizers | 400 | ✅ |

### 3.2 DialogRegistry

```cpp
struct DialogDescriptor {
    std::string id;
    std::string title;
    int order = 0;
    bool modal = true;
    std::string menuPath;
    std::string shortcut;
};

// Factory
using DialogFactory = std::function<std::unique_ptr<QDialog>(ServiceContainer&, QWidget*)>;
```

**Registrierte Dialoge:**

| ID | Titel | Modal | Shortcut |
|----|-------|-------|----------|
| about | About LumiViz | ✅ | F1 |

### 3.3 WidgetRegistry

```cpp
struct WidgetDescriptor {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    int order = 0;
    bool allowMultiple = false;  // ← NEU: Erlaubt mehrere Instanzen
};
```

**Registrierte Widgets:**

| ID | Name | allowMultiple |
|----|------|---------------|
| visualizer | Visualizer | ❌ |

### 3.4 VisualizerRegistry

```cpp
struct VisualizerDescriptor {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    int order = 0;
};

// Factory
using VisualizerFactory = std::function<std::unique_ptr<IVisualizer>()>;
```

**Registrierte Visualizer:**

| ID | Name | Kategorie |
|----|------|-----------|
| pulsing | Pulsing | Effects |

### 3.5 MenuRegistry

Siehe [MenuRegistry.md](MenuRegistry.md) und [MenuManager.md](../UI/managers/MenuManager.md) für Details.

---

## 4. AutoReg-Dateien

### 4.1 Struktur

Jede Registry hat eine zugehörige AutoReg-Datei:

| Registry | AutoReg-Datei | Pfad |
|----------|---------------|------|
| MenuRegistry | MenuAutoReg.cpp | `src/UI/managers/` |
| PanelRegistry | PanelAutoReg.cpp | `src/UI/panels/` |
| DialogRegistry | DialogAutoReg.cpp | `src/UI/dialogs/` |
| WidgetRegistry | WidgetAutoReg.cpp | `src/UI/widgets/` |
| VisualizerRegistry | VisualizerAutoReg.cpp | `src/visualizers/` |

### 4.2 Beispiel: PanelAutoReg.cpp

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
            "player",           // id (= objectName)
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
        PanelDescriptor{"playlist", "Playlist", 200, true, "View/Panels"},
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<PlaylistPanel>(svc);
        });
    
    // Config Panel (Settings) - hidden by default
    registry.registerPanel(
        PanelDescriptor{"config", "Settings", 300, false, "View/Panels"},
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<ConfigPanel>(svc);
        });
    
    // Visual Select Panel
    registry.registerPanel(
        PanelDescriptor{"visual_select", "Visualizers", 400, true, "View/Panels"},
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<VisualSelectPanel>(svc);
        });
}
```

### 4.3 Beispiel: WidgetAutoReg.cpp

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
            false  // allowMultiple
        },
        [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QWidget> {
            return std::make_unique<VisualizerWidget>(svc, parent);
        });
}
```

---

## 5. Verwendung

### 5.1 Neues Panel hinzufügen

```cpp
// 1. Panel-Klasse erstellen
class MyPanel : public PanelBase { ... };

// 2. In PanelAutoReg.cpp registrieren
void initPanelDefaults(PanelRegistry& registry)
{
    // ... bestehende Panels ...
    
    registry.registerPanel(
        PanelDescriptor{"mypanel", "My Panel", 500, true, "View/Panels"},
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<MyPanel>(svc);
        });
}

// 3. FERTIG - Erscheint automatisch im Menü und DockManager
```

### 5.2 Neuen Visualizer hinzufügen

```cpp
// 1. Visualizer-Klasse erstellen
class MyVisualizer : public IVisualizer { ... };

// 2. In VisualizerAutoReg.cpp registrieren
void initVisualizerDefaults(VisualizerRegistry& registry)
{
    registry.registerVisualizer(
        VisualizerDescriptor{"myvis", "My Visualizer", "Effects", "Cool effect", 200},
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<MyVisualizer>();
        });
}

// 3. FERTIG - Erscheint automatisch in VisualSelectPanel
```

### 5.3 Alle registrierten Komponenten auflisten

```cpp
void Application::listComponents()
{
    qDebug() << "=== Registered Panels ===";
    for (const auto& desc : PanelRegistry::instance().descriptors()) {
        qDebug() << "  -" << desc.id.c_str() << ":" << desc.title.c_str();
    }
    
    qDebug() << "=== Registered Visualizers ===";
    for (const auto& desc : VisualizerRegistry::instance().descriptors()) {
        qDebug() << "  -" << desc.id.c_str() << ":" << desc.name.c_str();
    }
}
```

---

## 6. Best Practices

### 6.1 IDs

- **Lowercase, keine Leerzeichen:** `player`, `visual_select`
- **Eindeutig** innerhalb der Registry
- **Stabil** - ändern bricht Layout-Persistence!

### 6.2 Order-Werte

100er-Schritte für Erweiterbarkeit:

```cpp
// 0-99:     Reserved
// 100-199:  Core Panels (Player, Playlist)
// 200-299:  Content Panels
// 300-399:  Configuration
// 400-499:  Selection
// 900-999:  Debug/Development
```

### 6.3 Factories

- Lambda mit `ServiceContainer&` Parameter
- `std::make_unique` für Ownership
- Keine globalen Abhängigkeiten!

```cpp
// ✅ Gut: Aus ServiceContainer
[](ServiceContainer& svc) {
    return std::make_unique<MyPanel>(
        svc.resolve<IAudioEngine>(),
        svc.resolve<IEventBus>()
    );
}

// ❌ Schlecht: Globale Abhängigkeiten
[](ServiceContainer&) {
    return std::make_unique<MyPanel>(g_audioEngine);  // Global!
}
```

### 6.4 Source.cmake

AutoReg-Dateien müssen in Source.cmake eingetragen sein:

```cmake
# src/UI/panels/Source.cmake
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/PanelAutoReg.cpp"  # ← Wichtig!
    "${CMAKE_CURRENT_LIST_DIR}/PlayerPanel.cpp"
    # ...
)
```

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.0.0** | **2025-12-31** | **Lazy-Init Pattern für alle Registries, +allowMultiple in WidgetRegistry, alte Makros entfernt** |
| 1.1.0 | 2025-12-31 | MenuRegistry: Direkte Registrierung statt Makros |
| 1.0.0 | 2025-12-31 | Initial: Panel, Dialog, Menu, Visualizer, Widget Registries |
