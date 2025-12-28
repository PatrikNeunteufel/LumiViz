# App-Container — Architecture-Concept

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Concept  
> **Status:** Implementiert  
> **Phase:** 8 (abgeschlossen)  
> **Target Audience:** Build System Developers, Architekten  
> **Language:** English  
> **German:** [AppContainer_Concept.md](../../en/projects/buildsystem/concepts/AppContainer_Concept.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Verzeichnisstruktur](#2-verzeichnisstruktur)
3. [Solution.json Integration](#3-solutionjson-integration)
4. [Tests-Configuration](#4-tests-konfiguration)
5. [CMake-Module](#5-cmake-module)
6. [Error Codes](#6-error-codes)
7. [Build-Ausgabe Struktur](#7-build-ausgabe-struktur)
8. [Example: Vollständiger App-Container](#8-beispiel-vollständiger-app-container)
9. [Migration: Executable → App-Container](#9-migration-executable--app-container)
10. [See Also](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Overview

### 1.1 Motivation

Das aktuelle Build-System unterstützt Executables als monolithische Einheiten. Das führt zu Problemen bei der Testbarkeit:

| Problem | Description |
|---------|--------------|
| **Nicht testbar** | Business-Logik ist mit `main()` vermischt |
| **Code-Duplizierung** | Um Module zu testen, müssen Sources manuell kopiert werden |
| **Zirkuläre Dependencies** | Test-Targets können nicht einfach gegen Executable-Code linken |

### 1.2 Lösung: App-Container

Ein **App-Container** trennt die Anwendungslogik von der Einstiegspunkt-Logik:

```
┌─────────────────────────────────────────────────────────────────┐
│                      App-Container                              │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │              AppName.Core (STATIC Library)                │  │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐                    │  │
│  │  │ Module A│  │ Module B│  │ Module C│   ...              │  │
│  │  └─────────┘  └─────────┘  └─────────┘                    │  │
│  └───────────────────────────────────────────────────────────┘  │
│                           │                                     │
│            ┌──────────────┼──────────────┐                      │
│            ▼              ▼              ▼                      │
│     ┌───────────┐  ┌────────────┐  ┌────────────┐               │
│     │   main/   │  │ UnitTests  │  │ Int.Tests  │               │
│     │ (Runner)  │  │            │  │            │               │
│     └───────────┘  └────────────┘  └────────────┘               │
│            │                                                    │
│            ▼                                                    │
│     ┌───────────┐                                               │
│     │ AppName   │                                               │
│     │(Executable)│                                              │
│     └───────────┘                                               │
└─────────────────────────────────────────────────────────────────┘
```

### 1.3 Kompatibilität

| Modus | JSON-Array | Description | Testbarkeit |
|-------|------------|--------------|-------------|
| **Executable** | `executables[]` | Monolithisches Executable | Eingeschränkt |
| **App-Container** | `apps[]` | Getrennte Core-Library + Runner | Volle Testbarkeit |

**Beide Modi verwenden dieselben Default-Pfade:**
- Executables: `projects/exec/{name}/`
- App-Container: `projects/apps/{name}/`

**Bestehende Projekte funktionieren weiterhin** — die `executables[]` Sektion bleibt vollständig unterstützt.

### 1.4 Designprinzipien

| Prinzip | Umsetzung |
|---------|-----------|
| **Single Source of Truth** | Alles in `Solution.json`, keine separate `app.json` |
| **Convention over Configuration** | Feste Verzeichnisstruktur, minimale Configuration |
| **Zentrale Externals** | Externals nur in `externals{}` Block, nie in Apps |
| **Konsistenz** | Gleiche Patterns wie Legacy (Source.cmake, PCH, etc.) |
| **Flexible Tests** | Beliebig viele Tests pro App via `tests.targets[]` |

---

## 2. Verzeichnisstruktur

### 2.1 App-Container Layout

```
projects/apps/{AppName}/
├── include/                    # PUBLIC Headers → {AppName}.Core
│   ├── Source.cmake
│   └── *.hpp
├── src/                        # Implementation → {AppName}.Core
│   ├── Source.cmake
│   └── *.cpp
├── main/                       # Entry Point → {AppName}
│   ├── Source.cmake
│   └── main.cpp
├── pch/                        # Precompiled Header (optional)
│   └── pch.h
└── tests/
    ├── unit/
    │   └── {TestName}/         # → {AppName}.{TestName}
    │       ├── Source.cmake
    │       ├── test_main.cpp
    │       └── test_*.cpp
    ├── integration/
    │   └── {TestName}/
    │       └── ...
    └── performance/
        └── {TestName}/
            └── ...
```

### 2.2 Symmetrie include/ ↔ src/

```
include/                    src/
├── Audio/                  ├── Audio/
│   ├── Player.hpp          │   ├── Player.cpp
│   └── Decoder.hpp         │   └── Decoder.cpp
├── UI/                     ├── UI/
│   └── Window.hpp          │   └── Window.cpp
└── Core/                   └── Core/
    └── Application.hpp         └── Application.cpp
```

### 2.3 Gesamtstruktur im Projekt

```
project_root/
├── Solution.json
├── projects/
│   ├── apps/                    # App-Container
│   │   ├── MyVisualizer/
│   │   └── DemoPlayer/
│   ├── exec/                    # Legacy Executables
│   │   └── MinimalConsole/
│   ├── libs/                    # Libraries
│   │   └── CoreLib/
│   └── tests/                   # Standalone Tests
│       └── CoreLib_UnitTests/
└── externals/
    └── bass/
```

---

## 3. Solution.json Integration

### 3.1 Vollständiges Example

```json
{
    "solution": {
        "name": "MySolution",
        "version": "1.0.0"
    },
    
    "externals": {
        "bass": { "path": "externals/bass" },
        "glad": { "path": "externals/glad" },
        "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" },
        "doctest": { "path": "externals/doctest" }
    },
    
    "apps": [
        {
            "name": "MyVisualizer",
            "displayName": "My Visualizer Application",
            "version": "1.0.0",
            
            "core": {
                "dependencies": ["CoreLib"],
                "externals": ["bass"]
            },
            
            "runner": {
                "type": "GUI",
                "externals": ["glad", "glfw"]
            },
            
            "pch": {
                "enabled": true
            },
            
            "tests": {
                "framework": "doctest",
                "targets": [
                    {
                        "name": "UnitTests",
                        "type": "unit",
                        "path": "tests/unit/UnitTests"
                    },
                    {
                        "name": "IntegrationTests",
                        "type": "integration",
                        "path": "tests/integration/IntegrationTests",
                        "externals": ["bass"]
                    }
                ]
            },
            
            "platforms": ["windows", "linux", "macos"]
        }
    ]
}
```

### 3.2 App-Definition Schema

| Feld | Typ | Required | Default | Description |
|------|-----|---------|---------|--------------|
| `name` | string | ✅ | — | Eindeutiger App-Name |
| `displayName` | string | — | `name` | Anzeigename |
| `version` | string | — | Solution-Version | App-Version |
| `description` | string | — | `""` | Description |
| `skip` | bool | — | `false` | Gesamte App überspringen |
| `path` | string | — | `projects/apps/{name}` | Basis-Pfad |
| `core` | object | — | `{}` | Core-Library Configuration |
| `runner` | object | — | `{}` | Runner-Executable Configuration |
| `pch` | object | — | `{}` | PCH Configuration |
| `tests` | object | — | `{}` | Test-Configuration |
| `platforms` | array | — | alle | Zielplattformen |

### 3.3 Core-Definition Schema

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `dependencies` | array | `[]` | Interne Library-Dependencies |
| `externals` | array | `[]` | Externe Dependencies |

### 3.4 Runner-Definition Schema

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `type` | string | `"CONSOLE"` | CONSOLE oder GUI |
| `externals` | array | `[]` | Zusätzliche Externals (GUI, etc.) |

### 3.5 PCH-Definition Schema

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `enabled` | bool | `false` | PCH aktivieren |
| `header` | string | `"pch.h"` | Header-Dateiname in `pch/` |

---

## 4. Tests-Configuration

### 4.1 Flexible Test-Targets

Die Tests werden über ein `targets[]` Array konfiguriert, das beliebig viele Tests mit individueller Configuration erlaubt.

→ **Detail-Concept:** [App_Tests_Targets_Concept.md](App_Tests_Targets_Concept.md)

### 4.2 Tests-Definition Schema

| Feld | Typ | Default | Description |
|------|-----|---------|--------------|
| `skip` | bool | `false` | **Alle Tests dieser App überspringen** |
| `framework` | string | — | Default-Framework für alle Tests |
| `targets` | array | `[]` | Test-Target Definitionen |

### 4.3 Test-Target Schema

| Feld | Typ | Required | Default | Description |
|------|-----|---------|---------|--------------|
| `name` | string | ✅ | — | Target-Name (→ `{AppName}.{name}`) |
| `type` | string | ✅ | — | Test-Typ (unit, integration, etc.) |
| `skip` | bool | — | `false` | Diesen Test überspringen |
| `path` | string | — | `tests/{type}/{name}` | Pfad relativ zum App-Verzeichnis |
| `framework` | string | — | `tests.framework` | Framework Override |
| `timeout` | number | — | Typ-abhängig | CTest Timeout (Sekunden) |
| `labels` | array | — | Typ-abhängig | CTest Labels |
| `externals` | array | — | `[]` | Zusätzliche Externals |
| `parallel` | bool | — | Typ-abhängig | Parallele Ausführung |

### 4.4 Bekannte Test-Typen

| Typ | Default Timeout | Default Parallel | Description |
|-----|-----------------|------------------|--------------|
| `unit` | 30s | true | Isolierte Funktions-/Klassen-Tests |
| `integration` | 120s | true | Komponenten-Interaktionen |
| `performance` | 300s | false | Benchmarks, Zeitmessungen |
| `system` | 180s | false | End-to-End Tests |
| `smoke` | 10s | true | Schnelle Basis-Checks |
| `fuzz` | 60s | false | Zufällige/ungültige Eingaben |
| `security` | 120s | false | Sicherheitstests |
| `ui` | 180s | false | Benutzeroberflächen-Tests |
| `api` | 60s | true | API-Endpunkt-Tests |

### 4.5 Skip-Feature

Das Skip-Feature ermöglicht temporäres Deaktivieren von Tests:

```json
"tests": {
    "skip": true,           // Global: Alle Tests überspringen
    "framework": "doctest",
    "targets": [
        {
            "name": "UnitTests",
            "type": "unit",
            "skip": false    // Ignoriert wenn global skip=true
        },
        {
            "name": "SlowTests",
            "type": "performance",
            "skip": true     // Individuell übersprungen
        }
    ]
}
```

**Skip-Logik:**

| Global `tests.skip` | Target `skip` | Ergebnis |
|---------------------|---------------|----------|
| `true` | egal | ⏭️ Übersprungen |
| `false`/fehlt | `true` | ⏭️ Übersprungen |
| `false`/fehlt | `false`/fehlt | ✅ Wird gebaut |

### 4.6 Example: Mehrere Tests

```json
"tests": {
    "framework": "doctest",
    "targets": [
        {
            "name": "Core_UnitTests",
            "type": "unit",
            "path": "tests/unit/core",
            "timeout": 30,
            "labels": ["unit", "core", "fast"]
        },
        {
            "name": "Utils_UnitTests",
            "type": "unit",
            "path": "tests/unit/utils"
        },
        {
            "name": "Audio_IntegrationTests",
            "type": "integration",
            "externals": ["bass"],
            "timeout": 120
        },
        {
            "name": "Benchmarks",
            "type": "performance",
            "skip": true,
            "labels": ["performance", "nightly"]
        }
    ]
}
```

---

## 5. CMake-Module

### 5.1 Modul-Overview

| Modul | Version | Description |
|-------|---------|--------------|
| `Apps.cmake` | v0.5.x | Pipeline-Orchestrator, Filter |
| `AppCollect.cmake` | v0.6.1 | JSON → Context, Tests-Parsing |
| `AppCreate.cmake` | v0.6.1 | Core, Runner, Tests erstellen |
| `Json.cmake` | v0.6.0 | JSON-Helper (zentral) |

### 5.2 Pipeline

```
1. Apps.cmake: apps Array iterieren
2. AppCollect: JSON parsen, Context erstellen
   - App-Metadaten
   - Core-Configuration
   - Runner-Configuration
   - Tests mit targets[]
3. Filter prüfen:
   - skip
   - platforms
   - BUILD_ONLY
4. AppCreate:
   a. Core Library erstellen (STATIC)
   b. PCH konfigurieren (wenn enabled)
   c. Runner Executable erstellen
   d. Tests erstellen (für jedes Target in tests.targets[])
      - Global skip prüfen
      - Per-Target skip prüfen
      - Type-Defaults anwenden
      - Framework auflösen
      - CTest registrieren
```

### 5.3 Generierte Targets

Für eine App `MyVisualizer` mit 3 Test-Targets:

| Target | Typ | Description |
|--------|-----|--------------|
| `MyVisualizer.Core` | STATIC Library | Business-Logik |
| `MyVisualizer` | Executable | Entry Point + Core |
| `MyVisualizer.UnitTests` | Executable | Unit Tests |
| `MyVisualizer.IntegrationTests` | Executable | Integration Tests |
| `MyVisualizer.Benchmarks` | Executable | Performance Tests |

---

## 6. Error Codes

### 6.1 App-Container Errors (E4xx)

| Code | Description |
|------|--------------|
| E401 | App definition: 'name' is required |
| E402 | App path does not exist |
| E403 | App has no src/ directory |
| E404 | App has no source files in src/ |
| E405 | App dependency not found |
| E406 | App has no main/ directory |
| E407 | App has no source files in main/ |

### 6.2 App-Tests Errors

| Code | Description |
|------|--------------|
| E301 | No framework specified for test |
| E302 | Unknown test framework |
| E303 | Test target: 'name' missing |
| E304 | Test target: 'type' missing |
| E305 | Test path does not exist |
| E306 | No source files in test path |

### 6.3 App-Container Warnings (W4xx)

| Code | Description |
|------|--------------|
| W401 | App has no include/ directory |
| W402 | PCH enabled but header not found |
| W402 | Test with serial type has parallel=true |
| W403 | Tests directory exists but no sources found |

---

## 7. Build-Ausgabe Struktur

```
out/build/{preset}/
├── apps/
│   └── MyVisualizer/
│       ├── bin/
│       │   └── Debug/
│       │       └── MyVisualizer.exe
│       ├── lib/
│       │   └── Debug/
│       │       └── MyVisualizer.Core.lib
│       └── tests/
│           └── Debug/
│               ├── MyVisualizer.UnitTests.exe
│               ├── MyVisualizer.IntegrationTests.exe
│               └── MyVisualizer.Benchmarks.exe
├── exec/
│   └── MinimalConsole/
└── lib/
    └── CoreLib/
```

---

## 8. Example: Vollständiger App-Container

### 8.1 Solution.json

```json
{
    "apps": [
        {
            "name": "AudioPlayer",
            "displayName": "Audio Player",
            "version": "2.0.0",
            
            "core": {
                "dependencies": ["BasicLogger"],
                "externals": ["bass", "spdlog"]
            },
            
            "runner": {
                "type": "GUI",
                "externals": ["imgui", "glad", "glfw"]
            },
            
            "pch": {
                "enabled": true
            },
            
            "tests": {
                "framework": "doctest",
                "targets": [
                    {
                        "name": "Core_UnitTests",
                        "type": "unit",
                        "path": "tests/unit/core",
                        "labels": ["unit", "audio", "fast"]
                    },
                    {
                        "name": "Playback_IntegrationTests",
                        "type": "integration",
                        "path": "tests/integration/playback",
                        "externals": ["bass"],
                        "labels": ["integration", "audio"]
                    },
                    {
                        "name": "Benchmarks",
                        "type": "performance",
                        "path": "tests/performance/benchmarks",
                        "skip": true
                    }
                ]
            }
        }
    ]
}
```

### 8.2 CMake-Ausgabe

```
[Apps] --- Processing: AudioPlayer ---
[Apps]   Path: projects/apps/AudioPlayer
[Apps]   Core: AudioPlayer.Core
[Apps]     - Dependencies: BasicLogger
[Apps]     - Externals: bass, spdlog
[Apps]   Runner: AudioPlayer (GUI)
[Apps]     - Externals: imgui, glad, glfw
[Apps]   PCH: Enabled (pch/pch.h)
[Apps]   Tests:
[Apps]     - Created: AudioPlayer.Core_UnitTests (unit, doctest)
[Apps]     - Created: AudioPlayer.Playback_IntegrationTests (integration, doctest)
[Apps]     - SKIP: AudioPlayer.Benchmarks (skip=true)
```

---

## 9. Migration: Executable → App-Container

### 9.1 Vorher (Executable)

```json
"executables": [
    {
        "name": "MyApp",
        "type": "CONSOLE",
        "path": "projects/exec/MyApp/src",
        "externals": ["bass"]
    }
]
```

```
projects/exec/MyApp/
└── src/
    ├── main.cpp
    ├── Application.cpp
    └── Application.hpp
```

### 9.2 Nachher (App-Container)

```json
"apps": [
    {
        "name": "MyApp",
        "core": {
            "externals": ["bass"]
        },
        "runner": {
            "type": "CONSOLE"
        },
        "tests": {
            "framework": "doctest",
            "targets": [
                { "name": "UnitTests", "type": "unit" }
            ]
        }
    }
]
```

```
projects/apps/MyApp/
├── include/
│   └── Application.hpp
├── src/
│   └── Application.cpp
├── main/
│   └── main.cpp
└── tests/
    └── unit/
        └── UnitTests/
            ├── Source.cmake
            ├── test_main.cpp
            └── test_Application.cpp
```

### 9.3 Migrations-Schritte

1. **Verzeichnis erstellen:** `projects/apps/{name}/`
2. **Sources aufteilen:**
   - Headers → `include/`
   - Implementation → `src/`
   - main.cpp → `main/`
3. **Solution.json anpassen:**
   - Von `executables[]` nach `apps[]` verschieben
   - Externals in `core` oder `runner` aufteilen
4. **Tests hinzufügen:**
   - `tests/unit/{TestName}/` erstellen
   - `test_main.cpp` mit Framework-Main
   - Test-Dateien erstellen

---

## 10. See Also

- [App_Tests_Targets_Concept.md](App_Tests_Targets_Concept.md) — Flexible Test-Configuration
- [Solution_Schema.md](../../../references/Solution_Schema.md) — JSON-Schema Reference
- [ErrorCodes.md](../../../references/ErrorCodes.md) — Errorcodes
- [App Template](../../../../projects/templates/App/README.md) — Template für neue Apps

---

## 11. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.6.0** | **2025-12-18** | **Phase 8 abgeschlossen: Neue tests.targets[] Struktur (ersetzt tests.unit/integration), Skip-Feature (global + per-target), Type-based Defaults, Framework Override, W402 parallel Warning, Status auf "Implementiert"** |
| 0.5.0 | 2025-12-14 | Initial: Concept für Core/Runner-Trennung |
