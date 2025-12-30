# Application — Zentrale Anwendungsklasse

> **Version:** 1.0.0  
> **Datum:** 2025-12-28  
> **Typ:** CppModuleDoc  
> **Status:** In Entwicklung  
> **Modul:** MyViz::Application  
> **Dateien:** Application.hpp, Application.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6::Widgets, BasicLogger  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [FrameMode-Konzept](#5-framemode-konzept)
6. [Thread-Sicherheit](#6-thread-sicherheit)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Die Application-Klasse ist der zentrale Einstiegspunkt für die MyViz-Anwendung. Sie kapselt `QApplication` und bietet einen konfigurierbaren Event-Loop mit verschiedenen Frame-Timing-Modi.

### 1.2 Verantwortlichkeiten

- Lifecycle-Management (init/run/shutdown)
- Qt-Event-Loop Kontrolle
- Frame-Timing mit drei Modi (Limited, Unlimited, VSync)
- FPS-Messung in Echtzeit
- Logging über BasicLogger

### 1.3 Nicht-Verantwortlichkeiten

- UI-Aufbau (→ MainWindow)
- Audio-Verarbeitung (→ AudioEngine)
- Visualisierung (→ VisualizerWidget)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6::Widgets | Extern | QApplication, Event-System |
| BasicLogger | Intern | Logging in Datei |
| MainWindow | Intern | Hauptfenster |

---

## 3. API

### 3.1 Enum FrameMode

```cpp
enum class FrameMode
{
    Limited,    // Software-Begrenzung auf Target-FPS
    Unlimited,  // Keine Begrenzung (100% CPU)
    VSync       // GPU-synchronisiert
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
| `run()` | — | `int` | Startet Event-Loop |
| `shutdown()` | — | `void` | Räumt auf |
| `requestQuit()` | — | `void` | Beendet Loop am Frame-Ende |

### 3.4 Frame-Mode-Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `setFrameMode(mode)` | `FrameMode` | `void` | Setzt Timing-Modus |
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
| `name()` | `const string&` | Anwendungsname ("MyViz") |
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

// Für Entwicklung: Unlimited (zeigt maximale Performance)
app.setFrameMode(FrameMode::Unlimited);

// Für Produktion: VSync (energieeffizient, kein Tearing)
// app.setFrameMode(FrameMode::VSync);

// Für Batterie: Limited mit niedriger FPS
// app.setFrameMode(FrameMode::Limited);
// app.setTargetFps(30);

return app.run();
```

### 4.3 FPS-Abfrage zur Laufzeit

```cpp
// Im MainWindow oder anderen Komponenten:
void updateTitleBar()
{
    int fps = static_cast<int>(app.currentFps());
    setWindowTitle(QString("MyViz - %1 FPS").arg(fps));
}
```

---

## 5. FrameMode-Konzept

### 5.1 Vergleich

| Modus | CPU | FPS | Verwendung |
|-------|-----|-----|------------|
| **Limited** | ~5% | Konstant (targetFps) | Batterie, konstantes Timing |
| **Unlimited** | 100% | Maximum | Benchmarking, Tests |
| **VSync** | ~5% | Monitor-Refresh | Produktion, kein Tearing |

### 5.2 Timing-Diagramm

```
Limited (60 FPS):
├─Update─┤───Sleep───├─Update─┤───Sleep───├─
   2ms       14ms        2ms       14ms

Unlimited:
├─Update─├─Update─├─Update─├─Update─├─Update─├─
   2ms      2ms      2ms      2ms      2ms

VSync (60Hz Monitor):
├─Update─┤──GPU Wait──├─Update─┤──GPU Wait──├─
   2ms        14ms         2ms       14ms
```

### 5.3 Wann welchen Modus?

- **Entwicklung:** `Unlimited` um Performance-Probleme zu finden
- **Produktion:** `VSync` für beste User-Experience
- **Laptop/Akku:** `Limited` mit 30-60 FPS

---

## 6. Thread-Sicherheit

**Nicht thread-safe.**

Alle Methoden müssen vom Main-Thread aufgerufen werden. Die Application-Klasse ist für Single-Threaded-Nutzung ausgelegt.

Ausnahme: `requestQuit()` kann theoretisch von anderen Threads aufgerufen werden (setzt nur einen `bool`), aber es wird empfohlen, Qt-Signals zu verwenden.

---

## 7. Fehlerbehandlung

- `init()` gibt `false` zurück bei Fehlern
- `run()` gibt Exit-Code zurück (0 = Erfolg)
- Fehler werden über BasicLogger protokolliert
- Keine Exceptions

### Log-Beispiel

```
[2025-12-28 14:30:00] [INFO] Application::init() starting...
[2025-12-28 14:30:00] [DEBUG] Creating QApplication...
[2025-12-28 14:30:00] [DEBUG] Creating MainWindow...
[2025-12-28 14:30:00] [INFO] MyViz v0.1.0 initialized
[2025-12-28 14:30:00] [INFO] Application::run() - Entering event loop
[2025-12-28 14:30:00] [INFO]   FrameMode: VSync
[2025-12-28 14:30:05] [DEBUG] FPS: 60 | Mode: VSync | Frame: 300
```

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-28** | **Initial: FrameMode enum, FPS-Messung, Custom Event Loop** |
