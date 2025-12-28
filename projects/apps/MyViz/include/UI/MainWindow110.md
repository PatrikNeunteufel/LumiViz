# MainWindow — Hauptfenster der Anwendung

> **Version:** 1.1.0  
> **Datum:** 2025-12-28  
> **Typ:** CppModuleDoc  
> **Status:** In Entwicklung  
> **Modul:** MyViz::UI::MainWindow  
> **Dateien:** MainWindow.hpp, MainWindow.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6::Widgets, VisualizerWidget  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Architektur](#4-architektur)
5. [Verwendung](#5-verwendung)
6. [Thread-Sicherheit](#6-thread-sicherheit)
7. [Qt6-Konzepte](#7-qt6-konzepte)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

### 1.1 Zweck

MainWindow ist das Hauptfenster der MyViz-Anwendung. Es erbt von `QMainWindow` und hostet das `VisualizerWidget` als Central Widget für OpenGL-Rendering.

### 1.2 Verantwortlichkeiten

- Fenster-Konfiguration (Titel, Größe, Minimum)
- Hosting des VisualizerWidget als Central Widget
- Weiterleitung von Render-Anfragen
- (Zukünftig: Menü, Toolbar, StatusBar, Docking)

### 1.3 Nicht-Verantwortlichkeiten

- Anwendungs-Lifecycle (→ Application)
- Event-Loop (→ Application)
- Audio-Verarbeitung (→ AudioEngine)
- OpenGL-Rendering (→ VisualizerWidget)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6::Widgets | Extern | QMainWindow Basisklasse |
| VisualizerWidget | Intern | OpenGL Central Widget |
| BasicLogger | Intern | Logging |

---

## 3. API

### 3.1 Konstruktor / Destruktor

```cpp
explicit MainWindow(QWidget* parent = nullptr);
~MainWindow() override;
```

### 3.2 Öffentliche Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `visualizer()` | — | `VisualizerWidget*` | Gibt das Visualizer-Widget zurück |
| `requestRender()` | — | `void` | Fordert Neuzeichnung an |

### 3.3 Geerbte Methoden (wichtigste)

| Methode | Quelle | Beschreibung |
|---------|--------|--------------|
| `show()` | QWidget | Zeigt das Fenster |
| `close()` | QWidget | Schließt das Fenster |
| `isVisible()` | QWidget | Prüft Sichtbarkeit |
| `resize(w, h)` | QWidget | Ändert Größe |

---

## 4. Architektur

### 4.1 Widget-Hierarchie

```
MainWindow (QMainWindow)
    │
    └── VisualizerWidget (QOpenGLWidget) ◄── Central Widget
            │
            ├── initializeGL()
            ├── resizeGL()
            └── paintGL()
```

### 4.2 Fenster-Layout

```
+------------------------------------------+
|  MyViz - Audio Visualizer        [_][□][X]|
+------------------------------------------+
|              (Menu Bar - TODO)            |
+------------------------------------------+
|                                          |
|         VisualizerWidget                 |
|         (OpenGL Rendering)               |
|                                          |
|    ┌────────────────────────────────┐    |
|    │                                │    |
|    │   Audio Visualization          │    |
|    │   - Spectrum Bars              │    |
|    │   - Waveform                   │    |
|    │   - 3D Effects                 │    |
|    │                                │    |
|    └────────────────────────────────┘    |
|                                          |
+------------------------------------------+
|              (Status Bar - TODO)          |
+------------------------------------------+
```

### 4.3 Render-Flow

```
Application::run()
      │
      └── requestRender()
               │
               ▼
      MainWindow::requestRender()
               │
               ▼
      VisualizerWidget::update()
               │
               ▼
         [Paint Event]
               │
               ▼
         paintGL()
               │
               ▼
         swapBuffers() (VSync)
```

---

## 5. Verwendung

### 5.1 Mit Application-Klasse (Standard)

```cpp
// In Application::init()
m_pMainWindow = std::make_unique<MainWindow>();
m_pMainWindow->show();

// In Application::run() - jeden Frame
m_pMainWindow->requestRender();
```

### 5.2 Zugriff auf Visualizer

```cpp
// Audio-Daten an Visualizer übergeben (zukünftig)
m_pMainWindow->visualizer()->setAudioData(spectrum, size);

// Hintergrundfarbe ändern
m_pMainWindow->visualizer()->setClearColor(0.1f, 0.1f, 0.2f);
```

### 5.3 Standalone (ohne Application)

```cpp
#include "UI/MainWindow.hpp"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

---

## 6. Thread-Sicherheit

**Nicht thread-safe.**

Alle Qt-Widget-Operationen müssen vom Main-Thread (GUI-Thread) erfolgen.

---

## 7. Qt6-Konzepte

### 7.1 Q_OBJECT Macro

Erforderlich für Signals/Slots und Meta-Object System.

### 7.2 Parent-Child Ownership

```cpp
// VisualizerWidget wird von MainWindow geowned
m_pVisualizer = new VisualizerWidget(this);  // 'this' = parent
setCentralWidget(m_pVisualizer);

// Kein delete nötig! Qt löscht automatisch.
```

### 7.3 update() vs repaint()

| Methode | Verhalten | Empfehlung |
|---------|-----------|------------|
| `update()` | Asynchron, coalesced | ✅ Verwenden |
| `repaint()` | Synchron, blockiert | ❌ Vermeiden |

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2025-12-28** | **VisualizerWidget als Central Widget, requestRender()** |
| 1.0.0 | 2025-12-28 | Initial: Leeres Fenster |
