# MainWindow — Hauptfenster mit Qt-ADS Docking

> **Version:** 2.0.0  
> **Datum:** 2025-12-28  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::UI::MainWindow  
> **Dateien:** MainWindow.hpp, MainWindow.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6::Widgets, DockManager, VisualizerWidget  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Architektur](#4-architektur)
5. [Menüstruktur](#5-menüstruktur)
6. [Changelog](#6-changelog)

---

## 1. Übersicht

### 1.1 Zweck

MainWindow ist das Hauptfenster der MyViz-Anwendung. Es verwendet Qt-ADS (Advanced Docking System) für eine flexible Multi-Visualizer-Oberfläche.

### 1.2 Features

- Dockbare Visualizer-Panels
- Tabs bei Überlagerung
- Menüleiste mit View-Menü
- Status-Bar mit FPS-Anzeige
- Layout speichern/laden (Perspectives)

### 1.3 UI-Layout

```
┌─────────────────────────────────────────────────────────────────┐
│ File   View   Settings   Help                                   │
├─────────────────────────────────────────────────────────────────┤
│ ┌───────────────────────────────────────────────────────────┐   │
│ │ Spectrum Analyzer │ Waveform │ 3D Effects │              │   │
│ ├───────────────────────────────────────────────────────────┤   │
│ │                                                           │   │
│ │              [Active VisualizerWidget]                    │   │
│ │                                                           │   │
│ │                                                           │   │
│ └───────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│ Ready                                              FPS: 60.0    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6::Widgets | Extern | QMainWindow, QMenu, QStatusBar |
| DockManager | Intern | Qt-ADS Wrapper |
| VisualizerWidget | Intern | OpenGL Rendering |
| BasicLogger | Intern | Logging |

---

## 3. API

### 3.1 Konstruktor / Destruktor

```cpp
explicit MainWindow(QWidget* parent = nullptr);
~MainWindow() override;
```

### 3.2 Öffentliche Methoden

| Methode | Rückgabe | Beschreibung |
|---------|----------|--------------|
| `dockManager()` | `DockManager*` | Zugriff auf Docking-System |
| `visualizers()` | `vector<VisualizerWidget*>` | Alle Visualizer |
| `primaryVisualizer()` | `VisualizerWidget*` | Erster Visualizer |
| `requestRender()` | `void` | Update alle Visualizer |

### 3.3 Public Slots

| Slot | Beschreibung |
|------|--------------|
| `updateFpsDisplay(fps)` | FPS in Status-Bar anzeigen |
| `onNewVisualizer()` | Neuen Visualizer erstellen |

---

## 4. Architektur

### 4.1 Komponenten-Hierarchie

```
MainWindow (QMainWindow)
├── QMenuBar
│   ├── File Menu
│   ├── View Menu (DockManager)
│   ├── Settings Menu
│   └── Help Menu
├── DockManager (unique_ptr)
│   └── ads::CDockManager
│       ├── CDockWidget "Spectrum"
│       │   └── VisualizerWidget #1
│       ├── CDockWidget "Waveform"
│       │   └── VisualizerWidget #2
│       └── ...
└── QStatusBar
    └── QLabel (FPS)
```

### 4.2 Render-Flow

```
Application::run()
      │
      └── MainWindow::requestRender()
               │
               └── DockManager::requestRenderAll()
                        │
                        ├── VisualizerWidget #1 → update()
                        ├── VisualizerWidget #2 → update()
                        └── ...
```

---

## 5. Menüstruktur

### 5.1 File

```
File
├── Open Audio...     (Ctrl+O)
├── ─────────────────
└── Exit              (Alt+F4)
```

### 5.2 View

```
View
├── New Visualizer    (Ctrl+N)
├── ─────────────────
├── Panels ►
│   ├── ☑ Spectrum Analyzer
│   ├── ☑ Waveform
│   └── ...
├── Perspectives ►
│   ├── Save Current...
│   ├── ─────────────────
│   └── Default
├── ─────────────────
└── Reset Layout
```

### 5.3 Settings

```
Settings
└── Frame Mode ►
    ├── ◉ Limited (60 FPS)
    ├── ○ Unlimited
    └── ○ VSync
```

---

## 6. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.0.0** | **2025-12-28** | **Qt-ADS Integration, Multi-Visualizer, Menüs, Status-Bar** |
| 1.1.0 | 2025-12-28 | VisualizerWidget als Central Widget |
| 1.0.0 | 2025-12-28 | Initial: Leeres Fenster |
