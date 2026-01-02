# MyViz Dokumentationsübersicht

> **Version:** 1.2.0  
> **Datum:** 2026-01-02  
> **Status:** Aktuell

---

## Zielgruppe

| Dokument | Zielgruppe |
|----------|------------|
| OVERVIEW.md | Alle Entwickler |
| architecture/*.md | Senior Entwickler, Architekten |
| modules/*.md | Feature-Entwickler |
| integration/*.md | Neue Teammitglieder |

---

## Dokumentationsstruktur

```
docs/
├── OVERVIEW.md                          # Diese Datei
├── README_MODULES.md                    # Modul-System Kurzreferenz
│
├── architecture/                        # Architektur-Entscheidungen
│   ├── Registry_Architecture.md         # Registry Pattern + Self-Registration
│   ├── Event_Architecture.md            # EventBus + Dezentrale Events
│   ├── Layout_Persistence.md            # Qt-ADS Layout-Speicherung
│   └── Registry_LazyInit.md             # Lazy-Init Pattern für Registries
│
├── modules/                             # Modul-Dokumentation
│   ├── Audio_System.md                  # BASS Audio Engine
│   ├── Menu_System.md                   # MenuRegistry + MenuManager
│   ├── Panel_System.md                  # PanelRegistry + DockManager
│   ├── IModule.md                       # NEU: Basis-Interface
│   ├── SmoothingModule.md               # NEU: Smoothing-Algorithmen
│   ├── AudioSourceModule.md             # NEU: FFT-Verarbeitung
│   ├── ColorGradientModule.md           # NEU: Gradient-System
│   └── Preset_System.md                 # NEU: File-basierte Presets
│
└── integration/                         # Integration Guides
    └── Application_Integration.md       # Wie alles zusammenspielt
```

---

## Modul-Dokumentation (inline)

Diese Dokumentation befindet sich direkt bei den Header-Dateien:

| Modul | Pfad | Beschreibung |
|-------|------|--------------|
| Application | `include/Application.md` | App-Lifecycle |
| ServiceContainer | `include/services/ServiceContainer.md` | Dependency Injection |
| EventBus | `include/services/EventBus.md` | Publish/Subscribe |
| Registries | `include/services/Registries.md` | Alle 5 Registries |
| MainWindow | `include/UI/MainWindow.md` | Hauptfenster |
| DockManager | `include/UI/managers/DockManager.md` | Qt-ADS Integration |
| MenuManager | `include/UI/managers/MenuManager.md` | Menü-Aufbau |
| PanelBase | `include/UI/panels/PanelBase.md` | Panel-Basisklasse |
| ConfigPanel | `include/UI/panels/ConfigPanel.md` | Visualizer-Konfiguration |
| WidgetBase | `include/UI/widgets/WidgetBase.md` | Widget-Basisklasse |
| VisualizerWidget | `include/UI/widgets/VisualizerWidget.md` | OpenGL Canvas |
| Visualizers | `include/visualizers/Visualizers.md` | Visualizer-Effekte |
| GpuInfo | `include/core/GpuInfo.md` | GPU-Enumeration |
| GpuSelector | `include/core/GpuSelector.md` | GPU-Auswahl |
| Audio System | `include/audio/README.md` | BASS Integration |

---

## Architektur-Übersicht

```
┌─────────────────────────────────────────────────────────────────────────┐
│                              Application                                 │
│                         (Lifecycle Manager)                              │
└─────────────────────────────────┬───────────────────────────────────────┘
                                  │
┌─────────────────────────────────┴───────────────────────────────────────┐
│                           ServiceContainer                               │
│                        (Dependency Injection)                            │
├─────────────────────────────────────────────────────────────────────────┤
│  IEventBus (EventBus)      │  IAudioEngine (BassEngine)                 │
│  IAudioPlayer (AudioPlayer) │  IAudioAnalyzer (AudioAnalyzer)           │
└─────────────────────────────────┬───────────────────────────────────────┘
                                  │
┌─────────────────────────────────┴───────────────────────────────────────┐
│                              Registries                                  │
│                          (Self-Registration)                             │
├─────────────────────────────────────────────────────────────────────────┤
│  MenuRegistry     │  PanelRegistry    │  DialogRegistry                 │
│  WidgetRegistry   │  VisualizerRegistry                                 │
└─────────────────────────────────┬───────────────────────────────────────┘
                                  │
┌─────────────────────────────────┴───────────────────────────────────────┐
│                              MainWindow                                  │
│                           (Qt QMainWindow)                               │
├─────────────────────────────────────────────────────────────────────────┤
│  MenuManager          │  DockManager           │  StatusBar             │
│  (Menü-Aufbau)        │  (Qt-ADS Panels)       │  (FPS-Anzeige)         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Event-Flow

```
┌────────────────────┐     ┌────────────────────┐     ┌────────────────────┐
│   Menu Action      │────►│     EventBus       │────►│    Handler         │
│   (User klickt)    │     │   (Pub/Sub)        │     │ (DockManager etc.) │
└────────────────────┘     └────────────────────┘     └────────────────────┘

Beispiel: User klickt "View → New Visualizer"
  1. MenuManager löst CreateVisualizerEvent aus
  2. EventBus verteilt Event an alle Subscriber
  3. DockManager empfängt Event und erstellt Visualizer
```

---

## Wichtige Patterns

### 1. Lazy-Init Singleton

Alle Registries verwenden Lazy-Init mit erzwungener Linkage:

```cpp
// In XxxRegistry.cpp
extern void initXxxDefaults(XxxRegistry& registry);

XxxRegistry& XxxRegistry::instance()
{
    static XxxRegistry registry;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initXxxDefaults(registry);  // ← Erzwingt Linkage
    }
    return registry;
}
```

### 2. Self-Registration

App-spezifische Registrierungen in separaten `*AutoReg.cpp` Dateien:

| Registry | AutoReg-Datei | Pfad |
|----------|---------------|------|
| MenuRegistry | MenuAutoReg.cpp | src/UI/managers/ |
| PanelRegistry | PanelAutoReg.cpp | src/UI/panels/ |
| DialogRegistry | DialogAutoReg.cpp | src/UI/dialogs/ |
| WidgetRegistry | WidgetAutoReg.cpp | src/UI/widgets/ |
| VisualizerRegistry | VisualizerAutoReg.cpp | src/visualizers/ |

### 3. Dezentrale Event-Handler

DockManager handhabt alle Dock-Events - MainWindow muss nicht geändert werden:

```
EventBus
├─► DockManager: CreateVisualizer, ResetLayout, ChangeVisualizer, TogglePanel
└─► MainWindow: FrameMode (emit signal), OpenDialog
```

---

## Schnellstart

### Neues Panel hinzufügen

1. Panel-Klasse von `PanelBase` ableiten
2. In `PanelAutoReg.cpp` registrieren
3. **Fertig** - Menü und DockManager werden automatisch aktualisiert

### Neuen Visualizer hinzufügen

1. Von `IVisualizer` ableiten
2. In `VisualizerAutoReg.cpp` registrieren
3. **Fertig** - Erscheint automatisch in VisualSelectPanel

### Neues Menü-Item hinzufügen

1. In `MenuAutoReg.cpp` registrieren
2. Event in `UIEvents.hpp` definieren (falls nötig)
3. Handler in passendem Manager implementieren

---

## Siehe auch

- [Registry Architecture](architecture/Registry_Architecture.md)
- [Event Architecture](architecture/Event_Architecture.md)
- [Layout Persistence](architecture/Layout_Persistence.md)
- [Application Integration](integration/Application_Integration.md)
