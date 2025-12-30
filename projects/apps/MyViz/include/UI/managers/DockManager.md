# DockManager — Qt-ADS Docking Integration

> **Version:** 1.0.0  
> **Datum:** 2025-12-28  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::UI::DockManager  
> **Dateien:** DockManager.hpp, DockManager.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt-ADS, Qt6::Widgets  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Layout-Features](#5-layout-features)
6. [Konfiguration](#6-konfiguration)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

### 1.1 Zweck

DockManager kapselt Qt-ADS (Advanced Docking System) und ermöglicht eine flexible, konfigurierbare UI mit:
- Mehrere Visualizer-Panels
- Tabs bei Überlagerung
- Drag & Drop Anordnung
- Floating Windows
- Layout speichern/laden

### 1.2 Qt-ADS Features

```
┌─────────────────────────────────────────────────────────────────┐
│ MainWindow                                                      │
├─────────────────────────────────────────────────────────────────┤
│ ┌───────────┬───────────┬───────────┐                           │
│ │ Spectrum  │ Waveform  │ 3D        │ ◄── Tabs                  │
│ ├───────────┴───────────┴───────────┤                           │
│ │                                   │ ◄── Active Panel          │
│ │    [VisualizerWidget]             │                           │
│ │                                   │                           │
│ └───────────────────────────────────┘                           │
│                                                                 │
│ Features:                                                       │
│   • Drag Tab → Split (horizontal/vertikal)                      │
│   • Drag Tab → Floating Window                                  │
│   • Double-Click Tab → Maximize                                 │
│   • Auto-Hide → Sidebar                                         │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt-ADS | Extern | Advanced Docking System |
| Qt6::Widgets | Extern | QMainWindow, QMenu |
| VisualizerWidget | Intern | OpenGL Visualizer |
| BasicLogger | Intern | Logging |

---

## 3. API

### 3.1 DockPosition Enum

```cpp
enum class DockPosition
{
    Center,     // Tabs (wenn belegt)
    Left,
    Right,
    Top,
    Bottom,
    Floating    // Schwebendes Fenster
};
```

### 3.2 Konstruktor

```cpp
explicit DockManager(QMainWindow* pMainWindow);
```

### 3.3 Visualizer-Erstellung

| Methode | Beschreibung |
|---------|--------------|
| `createVisualizer(title, pos)` | Neuer Visualizer an Position |
| `createVisualizerRelativeTo(title, pos, ref)` | Relativ zu anderem Widget |

### 3.4 Layout-Management

| Methode | Beschreibung |
|---------|--------------|
| `saveState()` | Speichert Layout als QByteArray |
| `restoreState(state)` | Stellt Layout wieder her |
| `savePerspective(name)` | Speichert benanntes Layout |
| `loadPerspective(name)` | Lädt benanntes Layout |

### 3.5 Render-Kontrolle

| Methode | Beschreibung |
|---------|--------------|
| `requestRenderAll()` | Update auf allen Visualizern |
| `visualizers()` | Liste aller Visualizer |

---

## 4. Verwendung

### 4.1 Grundlegende Einrichtung

```cpp
// In MainWindow:
m_pDockManager = std::make_unique<DockManager>(this);

// Visualizer erstellen
auto* viz1 = m_pDockManager->createVisualizer("Spectrum", DockPosition::Center);
auto* viz2 = m_pDockManager->createVisualizer("Waveform", DockPosition::Right);
```

### 4.2 Layout speichern/laden

```cpp
// Speichern in QSettings
QSettings settings;
settings.setValue("layout", m_pDockManager->saveState());

// Laden
QByteArray state = settings.value("layout").toByteArray();
m_pDockManager->restoreState(state);
```

### 4.3 Menü-Integration

```cpp
QMenu* viewMenu = m_pDockManager->createViewMenu(this);
menuBar()->addMenu(viewMenu);
// Enthält: Panel-Toggles, Perspectives, Reset Layout
```

---

## 5. Layout-Features

### 5.1 Docking-Zonen

```
┌─────────────────────────────────────────┐
│                  TOP                    │
├─────────┬─────────────────────┬─────────┤
│         │                     │         │
│  LEFT   │      CENTER         │  RIGHT  │
│         │                     │         │
├─────────┴─────────────────────┴─────────┤
│                 BOTTOM                  │
└─────────────────────────────────────────┘
```

### 5.2 Tab-Verhalten

Wenn ein Widget in eine bereits belegte Zone gedockt wird, entstehen automatisch Tabs:

```
┌───────────┬───────────┬───────────┐
│ Spectrum  │ Waveform  │ Effects   │ ◄── Klickbar
├───────────┴───────────┴───────────┤
│                                   │
│    [Aktives Widget]               │
│                                   │
└───────────────────────────────────┘
```

### 5.3 Splitting

Drag auf Rand einer Zone → Split:

```
┌─────────────────┬─────────────────┐
│                 │                 │
│   Spectrum      │    Waveform     │
│                 │                 │
└─────────────────┴─────────────────┘
```

### 5.4 Auto-Hide

Widgets können in eine Sidebar minimiert werden:

```
┌─[S]──────────────────────────────────────┐
│ │                                        │
│ │         Main Content                   │
│ │                                        │
└─┴────────────────────────────────────────┘
  ▲
  Hover → Slide-Out Panel
```

---

## 6. Konfiguration

### 6.1 Qt-ADS Config Flags

Im Konstruktor werden folgende Optionen aktiviert:

```cpp
ads::CDockManager::setConfigFlags(
    DefaultOpaqueConfig |
    DockAreaHasTabsMenuButton |
    DockAreaHasUndockButton |
    DockAreaCloseButtonClosesTab |
    TabCloseButtonIsToolButton |
    AllTabsHaveCloseButton |
    OpaqueSplitterResize |
    FocusHighlighting
);
```

### 6.2 Auto-Hide Config

```cpp
ads::CDockManager::setAutoHideConfigFlags(
    DefaultAutoHideConfig |
    AutoHideFeatureEnabled |
    DockAreaHasAutoHideButton
);
```

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-28** | **Initial: Qt-ADS Integration, Multi-Visualizer, Perspectives** |
