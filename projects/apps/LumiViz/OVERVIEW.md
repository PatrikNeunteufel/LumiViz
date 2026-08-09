# LumiViz Dokumentationsübersicht

> **Datum:** 2025-12-31  
> **Version:** 1.0.0  

---

## Dokumentationsstatus

### ✅ Vorhandene Dokumentation

| Modul | Datei | Status |
|-------|-------|--------|
| **Application** | `include/Application.md` | ✅ Aktuell |
| **ServiceContainer** | `include/services/ServiceContainer.md` | ✅ Aktuell |
| **EventBus** | `include/services/EventBus.md` | ✅ **Neu** |
| **Registries** | `include/services/Registries.md` | ✅ **Neu** |
| **MainWindow** | `include/UI/MainWindow.md` | ✅ Aktuell |
| **DockManager** | `include/UI/managers/DockManager.md` | ✅ Aktuell |
| **PanelBase** | `include/UI/panels/PanelBase.md` | ✅ **Neu** |
| **VisualizerWidget** | `include/UI/widgets/VisualizerWidget.md` | ✅ Aktuell |
| **WidgetBase** | `include/UI/widgets/WidgetBase.md` | ✅ **Neu** |
| **Visualizers** | `include/visualizers/README.md` | ✅ **Neu** |
| **GpuInfo** | `include/core/GpuInfo.md` | ✅ Aktuell |
| **GpuSelector** | `include/core/GpuSelector.md` | ✅ **Neu** |
| **Audio System** | `include/audio/README.md` | ✅ Aktuell |

### 📁 Dokumentationsstruktur

```
LumiViz/
├── README.md                          # Projekt-Übersicht (EN)
├── README_de.md                       # Projekt-Übersicht (DE)
├── READMEaudio.md                     # Audio-System Übersicht
├── Application_Integration.md         # Integration Guide
├── Tutorial_Registry_Architecture.md  # Registry Tutorial
│
├── include/
│   ├── Application.md                 # Application Klasse
│   │
│   ├── services/
│   │   ├── ServiceContainer.md        # DI Container
│   │   ├── EventBus.md                # Pub/Sub System ✨
│   │   └── Registries.md              # Self-Registration ✨
│   │
│   ├── UI/
│   │   ├── MainWindow.md              # Hauptfenster
│   │   ├── managers/
│   │   │   └── DockManager.md         # Qt-ADS Integration
│   │   ├── panels/
│   │   │   └── PanelBase.md           # Panel-Architektur ✨
│   │   └── widgets/
│   │       ├── VisualizerWidget.md    # OpenGL Widget
│   │       └── WidgetBase.md          # Widget-Template ✨
│   │
│   ├── core/
│   │   ├── GpuInfo.md                 # GPU Enumeration
│   │   └── GpuSelector.md             # GPU Selection ✨
│   │
│   ├── visualizers/
│   │   └── README.md                  # Visualizer-System ✨
│   │
│   └── audio/
│       └── README.md                  # Audio Services
```

---

## Neue Dokumentation (2025-12-31)

### 1. EventBus.md
- Publish/Subscribe Pattern
- Event-Definitionen
- Thread-sichere Queue
- Prioritäten

### 2. Registries.md
- Self-Registration Pattern
- PanelRegistry, DialogRegistry, MenuRegistry
- VisualizerRegistry, WidgetRegistry
- Makros (REGISTER_PANEL, etc.)

### 3. PanelBase.md
- Panel-Lifecycle
- Activation/Deactivation
- EventBus Integration
- Qt-ADS Docking

### 4. WidgetBase.md
- Template-Architektur
- Unterstützte Basisklassen
- Auto Start/Stop
- Q_OBJECT Einschränkung

### 5. Visualizers README.md
- VisualizerBase API
- Audio-Daten (Spectrum/Waveform)
- OpenGL-Integration
- Self-Registration

### 6. GpuSelector.md
- GPU-Präferenzen
- Hybrid Graphics
- Export-Flags
- Mismatch-Erkennung

---

## Dokumentationsformat

Alle Modul-Dokumentationen folgen diesem Format:

```markdown
# ModulName — Kurzbeschreibung

> **Version:** x.x.x  
> **Datum:** YYYY-MM-DD  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Pfad::Modul  
> **Dateien:** Datei.hpp, Datei.cpp  
> **Abhängigkeiten:** ...  

---

## Inhaltsverzeichnis
1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
...
n. [Changelog](#n-changelog)
```
