# Registries — Self-Registration Pattern

> **Version:** 1.1.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::Services::Registries  
> **Dateien:** PanelRegistry.hpp, DialogRegistry.hpp, MenuRegistry.hpp, VisualizerRegistry.hpp, WidgetRegistry.hpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** C++17 STL, ServiceContainer  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [Registry-Typen](#3-registry-typen)
4. [Self-Registration Makros](#4-self-registration-makros)
5. [Verwendung](#5-verwendung)
6. [Best Practices](#6-best-practices)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Die **Registries** implementieren das **Self-Registration Pattern** für automatische Komponenten-Registrierung beim Programmstart. Anstatt alle Panels, Dialoge, Visualizer etc. zentral aufzulisten, registriert sich jede Komponente selbst.

### 1.2 Vorteile

| Aspekt | Zentrale Registrierung | Self-Registration |
|--------|------------------------|-------------------|
| **Wartung** | Eine große Liste | Dezentral in jeder Datei |
| **Erweiterung** | Zentralen Code ändern | Nur neue Datei hinzufügen |
| **Kopplung** | Alle Includes nötig | Keine Abhängigkeiten |
| **Build-Abhängigkeit** | Zentraldatei muss neu bauen | Nur geänderte Datei |

### 1.3 Wie es funktioniert

```cpp
// Am Ende von SpectrumPanel.cpp:
REGISTER_PANEL("spectrum", "Spectrum Analyzer", true, SpectrumPanel)
```

Das Makro erzeugt eine **statische Variable**, deren Konstruktor beim Programmstart ausgeführt wird:

```
┌──────────────────────────────────────────────────────────────────────────┐
│                          Programmstart                                    │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  1. Static Initialization Phase                                           │
│     ┌───────────────────┐                                                │
│     │ SpectrumPanel.cpp │───► PanelRegistry::registerPanel("spectrum")  │
│     └───────────────────┘                                                │
│     ┌───────────────────┐                                                │
│     │ WaveformPanel.cpp │───► PanelRegistry::registerPanel("waveform")  │
│     └───────────────────┘                                                │
│     ┌───────────────────┐                                                │
│     │  AboutDialog.cpp  │───► DialogRegistry::registerDialog("about")   │
│     └───────────────────┘                                                │
│                                                                           │
│  2. main() wird aufgerufen                                                │
│     ┌───────────────────┐                                                │
│     │ Application::init │───► Panels aus Registry erstellen             │
│     └───────────────────┘                                                │
│                                                                           │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Architektur

### 2.1 Gemeinsame Struktur

Alle Registries folgen dem gleichen Pattern:

```
┌────────────────────────────────────────────────────────────────┐
│                    XxxRegistry (Singleton)                      │
├────────────────────────────────────────────────────────────────┤
│ + static instance() : XxxRegistry&                             │
│ + registerXxx(descriptor, factory)                             │
│ + has(id) : bool                                               │
│ + create(id, services) : unique_ptr<Xxx>                       │
│ + descriptors() : vector<XxxDescriptor>                        │
├────────────────────────────────────────────────────────────────┤
│ - m_descriptors : map<string, XxxDescriptor>                   │
│ - m_factories   : map<string, Factory>                         │
│ - m_mutex       : mutex                                        │
└────────────────────────────────────────────────────────────────┘
```

### 2.2 Descriptor-Struktur

Jede Registry hat einen Descriptor mit Metadaten:

```cpp
struct PanelDescriptor {
    std::string id;              // Unique ID
    std::string title;           // Display name
    int order = 0;               // Sort order
    bool defaultVisible = false; // Initial visibility
    std::string menuPath;        // Menu location
};

struct DialogDescriptor {
    std::string id;
    std::string title;
    int width = 400;
    int height = 300;
};

struct VisualizerDescriptor {
    std::string id;
    std::string name;
    std::string description;
    std::string category;
    std::string thumbnailPath;
};
```

### 2.3 Factory-Pattern

```cpp
// Factory-Typ: Erstellt Instanz mit ServiceContainer
using Factory = std::function<std::unique_ptr<QWidget>(ServiceContainer&)>;

// Registrierung mit Factory
registry.registerPanel(
    PanelDescriptor{"spectrum", "Spectrum", 0, true},
    [](ServiceContainer& svc) {
        return std::make_unique<SpectrumPanel>(svc);
    }
);

// Später: Instanz erstellen
auto panel = registry.create("spectrum", services);
```

---

## 3. Registry-Typen

### 3.1 PanelRegistry

Für Dock-Panels (Qt-ADS CDockWidget).

```cpp
// Registrierung
REGISTER_PANEL("playlist", "Playlist", true, PlaylistPanel)
REGISTER_PANEL_ORDERED("config", "Settings", 100, false, ConfigPanel)
REGISTER_PANEL_MENU("debug", "Debug", "View/Debug", 900, false, DebugPanel)

// Verwendung im PanelManager
for (const auto& desc : PanelRegistry::instance().descriptors()) {
    auto panel = PanelRegistry::instance().create(desc.id, m_services);
    createDockWidget(desc.id, desc.title, panel.release());
}
```

### 3.2 DialogRegistry

Für modale/nichtmodale Dialoge.

```cpp
// Registrierung
REGISTER_DIALOG("about", "About MyViz", AboutDialog)
REGISTER_DIALOG_SIZE("preferences", "Preferences", 600, 400, PreferencesDialog)

// Verwendung
void MainWindow::showAbout() {
    auto dialog = DialogRegistry::instance().create("about", m_services);
    dialog->exec();
}
```

### 3.3 MenuRegistry

Für Menü-Einträge und Actions. **Hinweis:** Verwendet seit v2.1.0 direkte Registrierung statt Makros, um Linker-Probleme bei statischen Libraries zu vermeiden.

```cpp
// Direkte Registrierung (empfohlen für statische Libraries)
void initMenuItemsAutoReg()
{
    auto& registry = MenuRegistry::instance();
    
    registry.registerItem(
        MenuItemDesc{
            {"menu.file.open", "menu.file", 100},
            "Open Audio...",
            [](ServiceContainer& svc) { /* ... */ },
            {},      // isChecked
            {},      // isEnabled
            "Ctrl+O" // shortcut
        },
        false);
}

// Container mit exclusive Flag (QActionGroup)
registry.registerContainer(
    MenuContainerDesc{
        {"menu.settings.framemode", "menu.settings", 100},
        "Frame Mode",
        true  // exclusive = Radio-Button-Stil
    },
    false);
```

**Siehe auch:** [MenuRegistry.md](MenuRegistry.md) und [MenuManager.md](../UI/managers/MenuManager.md) für Details.

### 3.4 VisualizerRegistry

Für Visualizer-Effekte.

```cpp
// Registrierung
REGISTER_VISUALIZER("pulsing", "Pulsing Circles", 
                    "Audio-reactive circles", PulsingVisualizer)
REGISTER_VISUALIZER_CAT("spectrum", "Spectrum Bars", 
                        "FFT visualization", "Spectrum", SpectrumVisualizer)

// Verwendung
void VisualizerWidget::setVisualizer(const QString& id) {
    m_visualizer = VisualizerRegistry::instance().create(id.toStdString(), m_services);
}
```

### 3.5 WidgetRegistry

Für allgemeine Widgets.

```cpp
// Registrierung
REGISTER_WIDGET("gpu-selector", "GPU Selector", GpuSelectorWidget)

// Verwendung
auto widget = WidgetRegistry::instance().create("gpu-selector", m_services);
```

---

## 4. Self-Registration Makros

### 4.1 Panel-Makros

```cpp
// Basis-Makro
REGISTER_PANEL(ID, TITLE, DEFAULT_VISIBLE, TYPE)

// Mit Sort-Order
REGISTER_PANEL_ORDERED(ID, TITLE, ORDER, DEFAULT_VISIBLE, TYPE)

// Mit Menü-Pfad
REGISTER_PANEL_MENU(ID, TITLE, MENU_PATH, ORDER, DEFAULT_VISIBLE, TYPE)
```

### 4.2 Dialog-Makros

```cpp
// Basis-Makro
REGISTER_DIALOG(ID, TITLE, TYPE)

// Mit Größe
REGISTER_DIALOG_SIZE(ID, TITLE, WIDTH, HEIGHT, TYPE)
```

### 4.3 Visualizer-Makros

```cpp
// Basis-Makro
REGISTER_VISUALIZER(ID, NAME, DESCRIPTION, TYPE)

// Mit Kategorie
REGISTER_VISUALIZER_CAT(ID, NAME, DESCRIPTION, CATEGORY, TYPE)

// Mit Thumbnail
REGISTER_VISUALIZER_FULL(ID, NAME, DESCRIPTION, CATEGORY, THUMBNAIL, TYPE)
```

### 4.4 Makro-Implementierung

```cpp
#define REGISTER_PANEL(ID_STR, TITLE_STR, DEFAULT_VIS, TYPE)              \
    namespace {                                                            \
        struct TYPE##__AutoPanelReg {                                     \
            TYPE##__AutoPanelReg() {                                      \
                PanelRegistry::instance().registerPanel(                   \
                    PanelDescriptor{ID_STR, TITLE_STR, 0, DEFAULT_VIS},   \
                    [](ServiceContainer& svc) -> std::unique_ptr<QWidget> \
                    { return std::make_unique<TYPE>(svc); },              \
                    false                                                  \
                );                                                         \
            }                                                              \
        } TYPE##__autoPanelRegInstance;                                   \
    }
```

---

## 5. Verwendung

### 5.1 Panel implementieren

```cpp
// SpectrumPanel.hpp
class SpectrumPanel : public PanelBase
{
    Q_OBJECT
public:
    explicit SpectrumPanel(ServiceContainer& services, QWidget* parent = nullptr);
    // ...
};

// SpectrumPanel.cpp
#include "SpectrumPanel.hpp"
#include "services/PanelRegistry.hpp"

SpectrumPanel::SpectrumPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, parent)
{
    // ...
}

// Am Ende der Datei:
REGISTER_PANEL("spectrum", "Spectrum Analyzer", true, SpectrumPanel)
```

### 5.2 Visualizer implementieren

```cpp
// PulsingVisualizer.hpp
class PulsingVisualizer : public VisualizerBase
{
public:
    explicit PulsingVisualizer(ServiceContainer& services);
    void render(float deltaTime) override;
    // ...
};

// PulsingVisualizer.cpp
#include "PulsingVisualizer.hpp"
#include "services/VisualizerRegistry.hpp"

// Am Ende der Datei:
REGISTER_VISUALIZER("pulsing", "Pulsing Circles", 
                    "Audio-reactive pulsing circles", 
                    PulsingVisualizer)
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

### 6.1 Eindeutige IDs verwenden

```cpp
// ❌ Schlecht: Generische IDs
REGISTER_PANEL("panel1", ...)
REGISTER_PANEL("main", ...)

// ✅ Gut: Beschreibende, eindeutige IDs
REGISTER_PANEL("spectrum-analyzer", ...)
REGISTER_PANEL("playlist-manager", ...)
```

### 6.2 Registrierung am Datei-Ende

```cpp
// ❌ Schlecht: Am Anfang (vor Klassen-Definition)
REGISTER_PANEL(...)  // TYPE ist noch nicht definiert!

class MyPanel { ... };

// ✅ Gut: Am Ende der .cpp Datei
class MyPanel { ... };

MyPanel::MyPanel() { ... }

// HIER: Nach allen Implementierungen
REGISTER_PANEL("mypanel", "My Panel", false, MyPanel)
```

### 6.3 Abhängigkeiten über ServiceContainer

```cpp
// ❌ Schlecht: Globale Abhängigkeiten
[](ServiceContainer&) {
    return std::make_unique<MyPanel>(g_audioEngine);  // Global!
}

// ✅ Gut: Aus ServiceContainer
[](ServiceContainer& svc) {
    return std::make_unique<MyPanel>(
        svc.resolve<IAudioEngine>(),
        svc.resolve<IEventBus>()
    );
}
```

### 6.4 Order-Werte strukturieren

```cpp
// Empfohlene Order-Bereiche:
// 0-99:     Core Panels (Player, Playlist)
// 100-199:  Visualizer Panels
// 200-299:  Analysis Panels
// 300-399:  Configuration
// 900-999:  Debug/Development

REGISTER_PANEL_ORDERED("player", "Player", 10, true, PlayerPanel)
REGISTER_PANEL_ORDERED("playlist", "Playlist", 20, true, PlaylistPanel)
REGISTER_PANEL_ORDERED("spectrum", "Spectrum", 100, true, SpectrumPanel)
REGISTER_PANEL_ORDERED("config", "Settings", 300, false, ConfigPanel)
REGISTER_PANEL_ORDERED("debug", "Debug Log", 900, false, DebugPanel)
```

### 6.5 Linker-Problem bei statischen Libraries

⚠️ **Wichtig:** Bei statischen Libraries (.lib/.a) können Makro-basierte Registrierungen vom Linker entfernt werden ("dead code elimination").

```cpp
// ❌ Problem: Statische Makros werden vom Linker entfernt
// In SpectrumPanel.cpp:
REGISTER_PANEL("spectrum", ...)  // Wird möglicherweise entfernt!

// ✅ Lösung: Explizite Init-Funktion mit direkter Registrierung
// In PanelAutoReg.cpp:
void initPanelRegistrations()
{
    PanelRegistry::instance().registerPanel(...);  // Direkt
}

// In main.cpp oder Application.cpp:
initPanelRegistrations();  // Aufruf erzwingt Linkage
```

**Siehe auch:** MenuRegistry verwendet dieses Pattern seit v2.1.0.

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2025-12-31** | **MenuRegistry: Direkte Registrierung statt Makros (Linker-Fix), +exclusive Container** |
| 1.0.0 | 2025-12-31 | Initial: Panel, Dialog, Menu, Visualizer, Widget Registries |
