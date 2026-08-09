# Application — Zentrale Anwendungsklasse

> **Version:** 2.0.0  
> **Datum:** 2025-12-29  
> **Typ:** CppModuleDoc  
> **Status:** In Entwicklung  
> **Modul:** LumiViz::Application  
> **Dateien:** Application.hpp, Application.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6::Widgets, BasicLogger, QTimer  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [FrameMode-Konzept](#5-framemode-konzept)
6. [Timer-basiertes Rendering](#6-timer-basiertes-rendering)
7. [Frame-Mode-Integration](#7-frame-mode-integration)
8. [Thread-Sicherheit](#8-thread-sicherheit)
9. [Fehlerbehandlung](#9-fehlerbehandlung)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Die Application-Klasse ist der zentrale Einstiegspunkt für die LumiViz-Anwendung. Sie kapselt `QApplication` und bietet einen konfigurierbaren Event-Loop mit verschiedenen Frame-Timing-Modi.

### 1.2 Verantwortlichkeiten

- Lifecycle-Management (init/run/shutdown) — die vollständige Abbau-Kette
  inkl. aller `aboutToQuit`-Handler und der verbindlichen Reihenfolge-Regeln
  steht in
  [`docs/core-services/Bootstrap_Integration.md`](../docs/core-services/Bootstrap_Integration.md)
  §5 (Kette) und §6 (Lebenszyklus-Vertrag für Threads/Fremd-Pipelines)
- Qt-Event-Loop Kontrolle via `exec()`
- Timer-basiertes Frame-Timing mit drei Modi (Limited, Unlimited, VSync)
- FPS-Messung in Echtzeit
- VSync-Steuerung über VisualizerWidgets
- Logging über BasicLogger

### 1.3 Nicht-Verantwortlichkeiten

- UI-Aufbau (→ MainWindow)
- Audio-Verarbeitung (→ AudioEngine)
- Visualisierung (→ VisualizerWidget)
- OpenGL-Rendering (→ VisualizerWidget)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6::Widgets | Extern | QApplication, Event-System |
| Qt6::Core | Extern | QTimer |
| BasicLogger | Intern | Logging in Datei |
| MainWindow | Intern | Hauptfenster |
| GpuSelector | Intern | GPU-Auswahl |

---

## 3. API

### 3.1 Enum FrameMode

```cpp
enum class FrameMode
{
    Limited,    // Software-Begrenzung auf Target-FPS
    Unlimited,  // Keine Begrenzung (Maximum FPS)
    VSync       // Hardware-synchronisiert via SwapBuffers
};
```

### 3.2 Konstruktion

```cpp
Application();
~Application();
```

### 3.3 Lifecycle-Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `init(argc, argv)` | `int`, `char**` | `bool` | Initialisiert Qt und UI |
| `run()` | — | `int` | Startet Event-Loop mit QTimer |
| `shutdown()` | — | `void` | Räumt auf |
| `requestQuit()` | — | `void` | Beendet Loop |

### 3.4 Frame-Mode-Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `setFrameMode(mode)` | `FrameMode` | `void` | Setzt Timing-Modus und aktualisiert Timer |
| `frameMode()` | — | `FrameMode` | Aktueller Modus |
| `setTargetFps(fps)` | `int` | `void` | Ziel-FPS für Limited (1-1000) |
| `targetFps()` | — | `int` | Aktuelles Ziel |

### 3.5 Statistik-Methoden

| Methode | Rückgabe | Beschreibung |
|---------|----------|--------------|
| `currentFps()` | `double` | Gemessene FPS (jede Sekunde aktualisiert) |
| `frameCount()` | `uint64_t` | Frames seit `run()` |

### 3.6 Info-Methoden

| Methode | Rückgabe | Beschreibung |
|---------|----------|--------------|
| `name()` | `const string&` | Anwendungsname ("LumiViz") |
| `version()` | `const string&` | Version ("0.1.0") |
| `isInitialized()` | `bool` | Zustand |
| `isRunning()` | `bool` | Zustand |

---

## 4. Verwendung

### 4.1 Einfaches Beispiel

```cpp
#include "Application.hpp"

int main(int argc, char* argv[])
{
    Application app;
    
    if (!app.init(argc, argv))
    {
        return 1;
    }
    
    return app.run();
}
```

### 4.2 Mit Frame-Mode-Konfiguration

```cpp
Application app;
app.init(argc, argv);

// Frame-Mode kann auch über das Menü geändert werden!
// Settings → Frame Mode → Limited / Unlimited / VSync

return app.run();
```

### 4.3 FPS-Abfrage zur Laufzeit

```cpp
// Im MainWindow oder anderen Komponenten:
void updateTitleBar()
{
    int fps = static_cast<int>(app.currentFps());
    setWindowTitle(QString("LumiViz - %1 FPS").arg(fps));
}
```

---

## 5. FrameMode-Konzept

### 5.1 Vergleich

| Modus | Timer | VSync | CPU | FPS |
|-------|-------|-------|-----|-----|
| **Limited** | 16ms | OFF | ~5% | 60 (targetFps) |
| **Unlimited** | 0ms | OFF | 100% | 500-1000+ |
| **VSync** | 1ms | ON | ~5% | Monitor-Hz |

### 5.2 Timing-Diagramm

```
Limited (60 FPS, Timer = 16ms):
├─Timer─┤─Timer─┤─Timer─┤─Timer─┤
  16ms    16ms    16ms    16ms
    │       │       │       │
    └─paint─┴─paint─┴─paint─┘

Unlimited (Timer = 0ms):
├┤├┤├┤├┤├┤├┤├┤├┤├┤├┤├┤├┤├┤├┤
 paint paint paint ...  (so schnell wie möglich)

VSync (Timer = 1ms, SwapInterval = 1):
├─Timer─┤─Timer─┤─Timer─┤
  1ms     1ms     1ms
    │       │       │
    └─paint─┴─paint─┘
         │       │
    [GPU Wait] [GPU Wait]  ◄── VSync synchronisiert
```

### 5.3 Wann welchen Modus?

- **Entwicklung:** `Unlimited` um Performance-Probleme zu finden
- **Produktion:** `VSync` für beste User-Experience (kein Tearing)
- **Laptop/Akku:** `Limited` mit 30-60 FPS

---

## 6. Timer-basiertes Rendering

### 6.1 Warum QTimer statt processEvents-Loop?

Ab Version 2.0.0 verwendet Application einen `QTimer` anstelle einer manuellen `processEvents()`-Schleife.

**Vorher (Problem: 32 FPS statt 60):**
```cpp
while (m_running)
{
    processEvents();           // ~16ms (DWM/VSync)
    mainWindow->requestRender();
    sleep_for(frameDuration);  // +16ms
}
// Total: ~32ms = 31 FPS!
```

**Nachher (korrekt: 60 FPS):**
```cpp
QTimer frameTimer;
frameTimer.setTimerType(Qt::PreciseTimer);
connect(&frameTimer, &QTimer::timeout, [this]() {
    mainWindow->requestRender();
});
frameTimer.start(16);  // 16ms für 60 FPS
app.exec();
```

### 6.2 Timer-Intervalle

| FrameMode | Timer Intervall | Begründung |
|-----------|-----------------|------------|
| **Limited** | `1000 / targetFps` ms | Exakte FPS-Kontrolle |
| **Unlimited** | 0ms | So schnell wie Event-Loop erlaubt |
| **VSync** | 1ms | GPU-Timing dominiert via SwapBuffers |

### 6.3 Impl-Member

```cpp
struct Application::Impl
{
    QTimer* pFrameTimer{nullptr};  // Owned by QApplication
    
    void updateTimerInterval()
    {
        switch (frameMode)
        {
            case FrameMode::Limited:
                pFrameTimer->setInterval(1000 / targetFps);
                break;
            case FrameMode::Unlimited:
                pFrameTimer->setInterval(0);
                break;
            case FrameMode::VSync:
                pFrameTimer->setInterval(1);
                break;
        }
    }
};
```

---

## 7. Frame-Mode-Integration

### 7.1 Signal-Slot-Verbindung

Application verbindet sich mit MainWindow's `frameModeChangeRequested` Signal:

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

### 7.2 VSync-Steuerung

Bei VSync-Modus wird echter Hardware-VSync auf allen VisualizerWidgets aktiviert:

```
User: Settings → Frame Mode → VSync
           │
           ▼
MainWindow::emit frameModeChangeRequested(2)
           │
           ▼
Application Handler:
  ├── frameMode = VSync
  ├── setVSyncOnAllVisualizers(true)
  │        │
  │        └── VisualizerWidget::setVSync(true)
  │                  │
  │                  └── wglSwapIntervalEXT(1)  [Windows]
  │                  └── eglSwapInterval(...)   [Linux]
  │                  └── CGLSetParameter(...)   [macOS]
  │
  └── updateTimerInterval() → 1ms
```

---

## 8. Thread-Sicherheit

**Nicht thread-safe.**

Alle Methoden müssen vom Main-Thread aufgerufen werden. Die Application-Klasse ist für Single-Threaded-Nutzung ausgelegt.

Ausnahme: `requestQuit()` kann theoretisch von anderen Threads aufgerufen werden (setzt nur einen `bool`), aber es wird empfohlen, Qt-Signals zu verwenden.

---

## 9. Fehlerbehandlung

- `init()` gibt `false` zurück bei Fehlern
- `run()` gibt Exit-Code zurück (0 = Erfolg)
- Fehler werden über BasicLogger protokolliert
- Keine Exceptions

### Log-Beispiel

```
[2025-12-29 14:30:00] [INFO] Application::init() starting...
[2025-12-29 14:30:00] [DEBUG] Creating QApplication...
[2025-12-29 14:30:00] [DEBUG] Creating MainWindow...
[2025-12-29 14:30:00] [INFO] LumiViz v0.1.0 initialized
[2025-12-29 14:30:00] [INFO] Application::run() - Entering event loop
[2025-12-29 14:30:00] [INFO]   FrameMode: Limited
[2025-12-29 14:30:00] [DEBUG] Timer interval set to 16ms (Limited)
[2025-12-29 14:30:05] [DEBUG] FPS: 60 | Mode: Limited | Frame: 300
[2025-12-29 14:30:10] [INFO] Frame mode changed to: VSync
[2025-12-29 14:30:10] [DEBUG] Timer interval set to 1ms (VSync)
[2025-12-29 14:30:10] [INFO] VSync ENABLED (wglSwapIntervalEXT)
```

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.0.0** | **2025-12-29** | **QTimer-basiertes Rendering, Frame-Mode via Menü, VSync-Steuerung, exec() statt processEvents-Loop** |
| 1.0.0 | 2025-12-28 | Initial: FrameMode enum, FPS-Messung, Custom Event Loop |
