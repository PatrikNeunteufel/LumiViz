# App Creation Guide

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Entwurf  
> **Target Audience:** Entwickler  
> **Language:** English

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Prerequisites](#2-voraussetzungen)
3. [Neue App erstellen](#3-neue-app-erstellen)
4. [Solution.json konfigurieren](#4-solutionjson-konfigurieren)
5. [Application-Klasse anpassen](#5-application-klasse-anpassen)
6. [Source.cmake pflegen](#6-sourcecmake-pflegen)
7. [Precompiled Header](#7-precompiled-header)
8. [GUI-Anwendung mit Qt](#8-gui-anwendung-mit-qt)
9. [Console-Anwendung](#9-console-anwendung)
10. [Tests schreiben](#10-tests-schreiben)
11. [Best Practices](#11-best-practices)
12. [Troubleshooting](#12-troubleshooting)
13. [See Also](#13-siehe-auch)
14. [Changelog](#14-changelog)

---

## 1. Overview

This guide describes, wie eine neue Anwendung mit dem App-Container-Concept des CMake Architecture Build-Systems erstellt wird.

### App-Container-Architecture

```
MeineApp/
├── include/        # Header (.hpp)
├── src/            # Implementation (.cpp)
├── main/           # Entry Point (main.cpp)
├── pch/            # Precompiled Header
└── tests/unit/     # Unit Tests
```

| Ordner | Verantwortung | Externals | Testbar |
|--------|---------------|-----------|---------|
| `include/` + `src/` | Gesamte Logik, UI, Services | Qt, BASS, etc. | ✅ Yes |
| `main/` | Nur Entry Point | — | ❌ No |
| `pch/` | Precompiled Header | — | — |
| `tests/unit/` | Unit Tests | doctest | — |

### Warum diese Trennung?

Die Trennung ermöglicht vollständige Testbarkeit der Anwendungslogik ohne die Anwendung starten zu müssen. Der main/-Ordner enthält nur den plattformspezifischen Entry Point.

---

## 2. Prerequisites

- CMake Architecture Build-System eingerichtet
- Solution.json vorhanden
- Benötigte Externals im `externals` Block definiert

---

## 3. Neue App erstellen

### 3.1 Template kopieren

```bash
cp -r projects/templates/App projects/apps/MeineApp
```

### 3.2 Verzeichnisstruktur

Nach dem Kopieren:

```
projects/apps/MeineApp/
├── README.md
├── include/
│   ├── Source.cmake
│   └── Application.hpp
├── src/
│   ├── Source.cmake
│   └── Application.cpp
├── main/
│   ├── Source.cmake
│   └── main.cpp              # Nicht ändern!
├── pch/
│   └── pch.h
└── tests/
    └── unit/
        └── Application_Tests.cpp
```

> **Important:** Die `main.cpp` sollte nicht geändert werden. Sie ist generisch und funktioniert für alle App-Typen.

---

## 4. Solution.json konfigurieren

### 4.1 App-Eintrag hinzufügen

```json
"apps": [
    {
        "name": "MeineApp",
        "displayName": "Meine Anwendung",
        "version": "0.1.0",
        "description": "Description der Anwendung",
        
        "core": {
            "dependencies": [],
            "externals": ["qt6", "bass"]
        },
        
        "runner": {
            "type": "GUI",
            "externals": []
        },
        
        "tests": {
            "framework": "doctest",
            "unit": {
                "timeout": 30,
                "labels": ["unit", "app", "fast"]
            }
        },
        
        "platforms": ["windows", "linux", "macos"]
    }
]
```

### 4.2 Felder erklärt

| Feld | Description |
|------|--------------|
| `name` | Technischer Name (Target-Name) |
| `displayName` | Anzeigename |
| `version` | Semantische Version |
| `core.dependencies` | Interne Libraries |
| `core.externals` | Externe Libraries (Qt, BASS, etc.) |
| `runner.type` | `GUI` oder `CONSOLE` |
| `runner.externals` | Externals nur für runner (selten benötigt) |
| `tests.framework` | Test-Framework (doctest, googletest, catch2) |

### 4.3 runner.type

| Typ | Windows | Linux/macOS | Usage |
|-----|---------|-------------|------------|
| `GUI` | `WinMain` | `main` | Qt, OpenGL, Fenster-Apps |
| `CONSOLE` | `main` | `main` | CLI-Tools, Server |

Bei `GUI` unter Windows wird kein Console-Fenster geöffnet.

---

## 5. Application-Klasse anpassen

### 5.1 Header erweitern

`include/Application.hpp`:

```cpp
#pragma once

#include <memory>
#include <string>

// Forward Declarations
class QApplication;
class MainWindow;

class Application {
public:
    Application();
    ~Application();
    
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    [[nodiscard]] bool init(int argc, char* argv[]);
    [[nodiscard]] int run();
    void shutdown();
    
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::string& version() const noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    bool m_initialized{false};
    bool m_running{false};
};
```

---

## 6. Source.cmake pflegen

Jeder Ordner mit Quellcode enthält eine `Source.cmake`. Bei neuen Dateien muss diese aktualisiert werden:

### 6.1 Neue Header hinzufügen

`include/Source.cmake`:

```cmake
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/Application.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/MainWindow.hpp"      # NEU
    "${CMAKE_CURRENT_LIST_DIR}/AudioEngine.hpp"     # NEU
)
```

### 6.2 Neue Sources hinzufügen

`src/Source.cmake`:

```cmake
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/Application.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MainWindow.cpp"      # NEU
    "${CMAKE_CURRENT_LIST_DIR}/AudioEngine.cpp"     # NEU
)
```

### 6.3 Unterordner einbinden

Bei Unterordnern am Ende der `Source.cmake`:

```cmake
# Subfolder rekursiv einbinden
include("${CMAKE_CURRENT_LIST_DIR}/audio/Source.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/ui/Source.cmake")
```

---

## 7. Precompiled Header

### 7.1 Zweck

`pch/pch.h` enthält häufig verwendete, stabile Includes um Build-Zeiten zu reduzieren.

### 7.2 PCH in Solution.json aktivieren

PCH muss explizit aktiviert werden:

```json
"apps": [{
    "name": "MeineApp",
    "pch": {
        "enabled": true
    }
}]
```

**Implizite Aktivierung:** PCH wird auch aktiviert wenn nur `header` oder `path` angegeben ist (und `enabled` nicht `false`):

```json
// Implizit aktiviert durch header
"pch": { "header": "stdafx.h" }

// Implizit aktiviert durch path
"pch": { "path": "common/pch" }
```

### 7.3 Suchpfad-Priorität

Wenn kein custom `path` angegeben ist, sucht das Build-System in dieser Reihenfolge:

| Priorität | Pfad | Example |
|-----------|------|----------|
| 1 | `{app}/pch/{header}` | `projects/apps/MeineApp/pch/pch.h` |
| 2 | `{app}/src/{header}` | `projects/apps/MeineApp/src/pch.h` |
| 3 | `{app}/{header}` | `projects/apps/MeineApp/pch.h` |

Mit custom `path` (relativ zu `projects/`):

```json
"pch": { "path": "common/pch", "header": "shared.h" }
// → projects/common/pch/shared.h
```

### 7.4 Inhalt anpassen

```cpp
#pragma once

// Standard Library
#include <memory>
#include <string>
#include <vector>

// Qt (bei GUI-Apps)
#include <QApplication>
#include <QMainWindow>
#include <QString>

// Audio (bei Audio-Apps)
#include <bass.h>
```

### 7.5 Note zur Extension

Die `.h` Extension ist Standard für PCH, auch bei C++. Einige Compiler und Tools verarbeiten `.hpp` nicht korrekt als PCH.

---

## 8. GUI-Anwendung mit Qt

### 8.1 Application.cpp für Qt

```cpp
#include "Application.hpp"
#include "MainWindow.hpp"

#include <QApplication>

struct Application::Impl {
    std::string name{"MeineApp"};
    std::string version{"0.1.0"};
    
    std::unique_ptr<QApplication> qtApp;
    std::unique_ptr<MainWindow> mainWindow;
};

bool Application::init(int argc, char* argv[]) {
    if (m_initialized) return false;
    
    m_impl->qtApp = std::make_unique<QApplication>(argc, argv);
    m_impl->mainWindow = std::make_unique<MainWindow>();
    m_impl->mainWindow->show();
    
    m_initialized = true;
    return true;
}

int Application::run() {
    if (!m_initialized) return 1;
    m_running = true;
    return m_impl->qtApp->exec();
}

void Application::shutdown() {
    m_impl->mainWindow.reset();
    m_impl->qtApp.reset();
    m_initialized = false;
}
```

### 8.2 MainWindow mit Docking

Siehe [App Template Reference](../references/App_Template_Reference.md) für Details zu QDockWidget.

---

## 9. Console-Anwendung

### 9.1 Application.cpp für Console

```cpp
#include "Application.hpp"
#include <iostream>

struct Application::Impl {
    std::string name{"MeinTool"};
    std::string version{"0.1.0"};
    bool shouldRun{true};
};

bool Application::init(int argc, char* argv[]) {
    // Configuration laden, Services starten
    m_initialized = true;
    return true;
}

int Application::run() {
    m_running = true;
    while (m_impl->shouldRun) {
        // Hauptlogik
    }
    return 0;
}

void Application::shutdown() {
    m_impl->shouldRun = false;
    m_initialized = false;
}
```

---

## 10. Tests schreiben

### 10.1 Test-Datei

Tests liegen direkt in `tests/unit/`:

```cpp
// tests/unit/Application_Tests.cpp

#include <doctest.h>
#include "Application.hpp"

TEST_SUITE("Application") {
    
    TEST_CASE("Init Success") {
        Application app;
        char arg0[] = "TestApp";
        char* argv[] = {arg0, nullptr};
        
        CHECK(app.init(1, argv));
        CHECK(app.isInitialized());
    }
}
```

### 10.2 Qt-Widgets testen

Qt-Widgets können ohne `QApplication::exec()` getestet werden:

```cpp
#include <doctest.h>
#include <QApplication>
#include "MainWindow.hpp"

TEST_CASE("MainWindow Creation") {
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);
    
    MainWindow window;
    CHECK(window.menuBar() != nullptr);
}
```

---

## 11. Best Practices

### 11.1 Trennung von Concerns

- `include/` + `src/` enthalten alle UI-Klassen, Services und Logik
- `main/` enthält nur die generische main.cpp
- Keine Geschäftslogik in main.cpp

### 11.2 Source.cmake aktuell halten

Bei jeder neuen Datei die entsprechende `Source.cmake` aktualisieren.

### 11.3 PCH sinnvoll nutzen

Nur stabile, häufig verwendete Header in `pch.h`:
- ✅ Standard Library
- ✅ Qt Core Headers
- ❌ Eigene Header (ändern sich oft)

---

## 12. Troubleshooting

### Problem: Console-Fenster bei GUI-App (Windows)

**Lösung:** `runner.type` auf `"GUI"` setzen.

### Problem: Neue Dateien werden nicht kompiliert

**Lösung:** `Source.cmake` im entsprechenden Ordner aktualisieren.

### Problem: PCH wird nicht verwendet

**Lösung:** Prüfen ob `pch/pch.h` existiert und in CMake konfiguriert ist.

---

## 13. See Also

- [App Template Reference](../references/App_Template_Reference.md)
- [Solution Schema](../references/Solution_Schema.md)
- [AppContainer Concept](../concepts/AppContainer_Concept.md)

---

## 14. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.1** | **2025-12-18** | **PCH-Aktivierung via Solution.json dokumentiert, Suchpfad-Priorität, implizite Aktivierung** |
| 0.1.0 | 2025-12-17 | Initial mit korrekter Struktur (include/, src/, main/, pch/) |
