# MainWindow — Hauptfenster mit Qt-ADS Docking

> **Version:** 2.1.0  
> **Datum:** 2025-12-29  
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
6. [Frame-Mode-Integration](#6-frame-mode-integration)
7. [Changelog](#7-changelog)

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
- Frame-Mode-Steuerung via Menü

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

| Slot | Parameter | Beschreibung |
|------|-----------|--------------|
| `updateFpsDisplay(fps)` | `double` | FPS in Status-Bar anzeigen |
| `onNewVisualizer()` | - | Neuen Visualizer erstellen |
| `setVSyncOnAllVisualizers(enabled)` | `bool` | VSync auf allen Visualizern setzen |

### 3.4 Signals

| Signal | Parameter | Beschreibung |
|--------|-----------|--------------|
| `frameModeChangeRequested(mode)` | `int` | Emittiert bei Frame-Mode-Wechsel im Menü |

**Mode-Werte:**

| Wert | FrameMode |
|------|-----------|
| 0 | Limited (60 FPS) |
| 1 | Unlimited |
| 2 | VSync |

---

## 4. Architektur

### 4.1 Komponenten-Hierarchie

```
MainWindow (QMainWindow)
├── QMenuBar
│   ├── File Menu
│   ├── View Menu (DockManager)
│   ├── Settings Menu
│   │   └── Frame Mode ► (connected to signal)
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
      └── QTimer::timeout
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
    ├── ◉ Limited (60 FPS)   → emit frameModeChangeRequested(0)
    ├── ○ Unlimited          → emit frameModeChangeRequested(1)
    └── ○ VSync              → emit frameModeChangeRequested(2)
```

---

## 6. Frame-Mode-Integration

### 6.1 Signal-Slot-Verbindung

Die Frame-Mode-Steuerung verbindet MainWindow mit Application:

```cpp
// In Application::run():
QObject::connect(m_pMainWindow.get(), &MainWindow::frameModeChangeRequested,
                 [this](int mode) {
    switch (mode)
    {
        case 0: 
            m_impl->frameMode = FrameMode::Limited;
            m_impl->pMainWindow->setVSyncOnAllVisualizers(false);
            break;
        case 1: 
            m_impl->frameMode = FrameMode::Unlimited;
            m_impl->pMainWindow->setVSyncOnAllVisualizers(false);
            break;
        case 2: 
            m_impl->frameMode = FrameMode::VSync;
            m_impl->pMainWindow->setVSyncOnAllVisualizers(true);
            break;
    }
    m_impl->updateTimerInterval();
});
```

### 6.2 VSync-Steuerung

Bei Mode-Wechsel wird VSync auf allen Visualizern entsprechend gesetzt:

| FrameMode | VSync | Timer Intervall |
|-----------|-------|-----------------|
| Limited | OFF | 16ms |
| Unlimited | OFF | 0ms |
| VSync | ON | 1ms |

### 6.3 Flow-Diagramm

```
┌──────────────────────────────────────────────────────────────────┐
│ User wählt "VSync" im Menü                                       │
└────────────────────────┬─────────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────────┐
│ MainWindow::emit frameModeChangeRequested(2)                     │
└────────────────────────┬─────────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────────┐
│ Application Lambda Handler                                        │
│   ├── frameMode = VSync                                          │
│   ├── setVSyncOnAllVisualizers(true)                             │
│   └── updateTimerInterval() → 1ms                                │
└────────────────────────┬─────────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────────┐
│ Für jeden VisualizerWidget:                                      │
│   └── setVSync(true) → wglSwapIntervalEXT(1) / eglSwapInterval() │
└──────────────────────────────────────────────────────────────────┘
```

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.1.0** | **2025-12-29** | **frameModeChangeRequested Signal, setVSyncOnAllVisualizers Slot, Frame-Mode-Anbindung** |
| 2.0.0 | 2025-12-28 | Qt-ADS Integration, Multi-Visualizer, Menüs, Status-Bar |
| 1.1.0 | 2025-12-28 | VisualizerWidget als Central Widget |
| 1.0.0 | 2025-12-28 | Initial: Leeres Fenster |
