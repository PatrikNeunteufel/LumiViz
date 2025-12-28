# App Template

> **Version:** 0.2.1  
> **Datum:** 2025-12-18  
> **Typ:** Vorlage  
> **Status:** Aktiv  
> **English:** [README.md](README.md)

---

CMake Architecture V2 - App-Container Vorlage

## Übersicht

Diese Vorlage stellt die Standardstruktur für App-Container im CMake Architecture V2 Build-System bereit. Sie trennt den Einstiegspunkt (`main/`) von der Anwendungslogik (`include/`, `src/`) für maximale Testbarkeit.

## Verzeichnisstruktur

```
App/
├── README.md                      # Englische Version
├── README_de.md                   # Diese Datei (Deutsch)
├── include/                       # Öffentliche Header
│   ├── Source.cmake
│   └── Application.hpp
├── src/                           # Implementierung
│   ├── Source.cmake
│   └── Application.cpp
├── main/                          # Einstiegspunkt (nicht testbar)
│   ├── Source.cmake
│   └── main.cpp
├── pch/                           # Vorkompilierter Header
│   └── pch.h
└── tests/                         # Tests
    ├── unit/
    │   └── {TestName}/            # Jeder Test im benannten Unterordner
    │       ├── Source.cmake
    │       ├── test_main.cpp
    │       └── *.cpp
    ├── integration/
    │   └── {TestName}/
    │       └── ...
    ├── performance/
    │   └── {TestName}/
    │       └── ...
    ├── smoke/
    │   └── {TestName}/
    │       └── ...
    └── system/
        └── {TestName}/
            └── ...
```

## Architektur

| Verzeichnis | Verantwortung | Testbar |
|-------------|---------------|---------|
| `include/` + `src/` | Gesamte Logik, UI, Services | ✅ Ja |
| `main/` | Nur Einstiegspunkt | ❌ Nein |
| `pch/` | Vorkompilierter Header | — |
| `tests/` | Test-Code | — |

## Verwendung

### 1. Vorlage kopieren

Kopiere dieses Verzeichnis in dein Projekt:

```bash
cp -r projects/templates/App projects/apps/DeinAppName
```

### 2. Solution.json konfigurieren

#### Minimale Konfiguration

```json
"apps": [
    {
        "name": "DeinAppName",
        "displayName": "Deine Anwendung",
        "version": "0.1.0",
        
        "core": {
            "dependencies": [],
            "externals": []
        },

        "runner": {
            "type": "GUI",
            "externals": []
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
                }
            ]
        }
    }
]
```

#### Vollständige Konfiguration (Mehrere Tests)

```json
"apps": [
    {
        "name": "DeinAppName",
        "displayName": "Deine Anwendung",
        "version": "0.1.0",
        "description": "Anwendungsbeschreibung",
        
        "core": {
            "dependencies": ["EineBibliothek"],
            "externals": ["bass", "qt6"]
        },

        "runner": {
            "type": "GUI",
            "externals": ["glad", "glfw"]
        },

        "pch": {
            "enabled": true
        },

        "tests": {
            "skip": false,
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
                    "path": "tests/unit/utils",
                    "timeout": 30,
                    "labels": ["unit", "utils", "fast"]
                },
                {
                    "name": "IntegrationTests",
                    "type": "integration",
                    "path": "tests/integration/IntegrationTests",
                    "timeout": 120,
                    "labels": ["integration", "slow"],
                    "externals": ["bass"]
                },
                {
                    "name": "Benchmarks",
                    "type": "performance",
                    "path": "tests/performance/Benchmarks",
                    "timeout": 300,
                    "labels": ["performance", "nightly"],
                    "skip": true
                }
            ]
        },

        "platforms": ["windows", "linux", "macos"]
    }
]
```

### 3. Test-Verzeichnisse erstellen

Für jedes Test-Target das passende Verzeichnis erstellen:

```bash
# Für "name": "UnitTests", "path": "tests/unit/UnitTests"
mkdir -p tests/unit/UnitTests
```

Jedes Test-Verzeichnis benötigt:
- `Source.cmake` — Dateiliste
- `test_main.cpp` — Framework-Einstiegspunkt
- `*.cpp` — Deine Test-Dateien

### 4. Application-Klasse anpassen

Bearbeite `src/Application.cpp`:
- Initialisiere deine Services in `init()`
- Implementiere deine Hauptschleife in `run()`
- Räume Ressourcen in `shutdown()` auf

## Test-Konfiguration

### Test-Target Schema

```json
{
    "name": "TestName",           // Pflicht: Target-Name
    "type": "unit",               // Pflicht: Test-Typ
    "skip": false,                // Optional: Test überspringen
    "path": "tests/unit/TestName", // Optional: Pfad (Standard: tests/{type}/{name})
    "framework": "doctest",       // Optional: Überschreibt globales Framework
    "timeout": 30,                // Optional: Timeout in Sekunden
    "labels": ["unit", "fast"],   // Optional: CTest Labels
    "externals": ["bass"],        // Optional: Zusätzliche Externals
    "parallel": true              // Optional: Parallele Ausführung erlaubt
}
```

### Bekannte Test-Typen

| Typ | Standard Timeout | Standard Parallel | Verwendung |
|-----|------------------|-------------------|------------|
| `unit` | 30s | ✅ true | Isolierte Funktions-/Klassen-Tests |
| `integration` | 120s | ✅ true | Komponenten-Interaktionstests |
| `performance` | 300s | ❌ false | Benchmarks, Zeitmessungen |
| `system` | 180s | ❌ false | End-to-End Tests |
| `smoke` | 10s | ✅ true | Schnelle "Startet es?" Checks |
| `fuzz` | 60s | ❌ false | Zufällige/ungültige Eingaben testen |
| `security` | 120s | ❌ false | Sicherheitslücken-Tests |
| `ui` | 180s | ❌ false | Benutzeroberflächen-Tests |
| `api` | 60s | ✅ true | API-Endpunkt-Tests |

Unbekannte Typen verwenden: 60s Timeout, parallel=true, label=[typ]

### Framework überschreiben

```json
"tests": {
    "framework": "doctest",           // Globaler Standard
    "targets": [
        {
            "name": "UnitTests",
            "type": "unit"
            // Verwendet "doctest" vom globalen Standard
        },
        {
            "name": "FuzzTests",
            "type": "fuzz",
            "framework": "googletest"  // Überschreibt für diesen Test
        }
    ]
}
```

Unterstützte Frameworks: `doctest`, `googletest`, `catch2`

### Parallele Ausführung

Tests mit `parallel: false` oder serielle Typen laufen mit CTests `RUN_SERIAL` Property.

⚠️ **Warnung W402:** Wenn du explizit `parallel: true` für Typen setzt, die standardmäßig seriell sind (performance, system, fuzz, security, ui), erscheint eine Warnung. Dies ist erlaubt, kann aber ungenaue Ergebnisse liefern.

### Skip-Feature

Tests können temporär deaktiviert werden ohne sie aus der Konfiguration zu entfernen.

#### Globales Skip (alle Tests)

```json
"tests": {
    "skip": true,           // Alle Tests dieser App überspringen
    "framework": "doctest",
    "targets": [
        { "name": "UnitTests", "type": "unit" },
        { "name": "IntegrationTests", "type": "integration" }
    ]
}
```

#### Per-Target Skip

```json
"tests": {
    "framework": "doctest",
    "targets": [
        {
            "name": "UnitTests",
            "type": "unit",
            "skip": false    // Wird gebaut
        },
        {
            "name": "SlowTests",
            "type": "integration",
            "skip": true     // Temporär deaktiviert
        }
    ]
}
```

#### Skip-Logik

| Global `tests.skip` | Target `skip` | Ergebnis |
|---------------------|---------------|----------|
| `true` | egal | ⏭️ Übersprungen |
| `false`/fehlt | `true` | ⏭️ Übersprungen |
| `false`/fehlt | `false`/fehlt | ✅ Wird gebaut |

**Hinweis:** Globales Skip hat Vorrang vor individuellen Target-Skip-Einstellungen.

## Generierte Targets

| Solution.json `name` | CMake Target |
|---------------------|--------------|
| `UnitTests` | `DeinAppName.UnitTests` |
| `Core_UnitTests` | `DeinAppName.Core_UnitTests` |
| `IntegrationTests` | `DeinAppName.IntegrationTests` |

## test_main.cpp

Jedes Test-Verzeichnis muss eine `test_main.cpp` enthalten:

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
```

**Wichtig:** Dieses Define darf nur **einmal** pro Test-Executable erscheinen. Füge es nicht in deine Test-Dateien ein!

## PCH (Vorkompilierter Header)

### PCH Geltungsbereich

- **Core (`src/`)**: Verwendet PCH — füge `#include "pch.h"` als erste Zeile ein
- **Runner (`main/`)**: Verwendet PCH NICHT
- **Tests**: Verwenden PCH NICHT

### PCH aktivieren/deaktivieren

```json
"pch": {
    "enabled": true,    // oder false zum Deaktivieren
    "header": "pch.h"   // optional, Standard ist "pch.h"
}
```

## Build Defines

| Define | Bedingung |
|--------|-----------|
| `APP_GUI` | `runner.type = "GUI"` (nur Windows) |

## runner.type

| Typ | Windows | Linux/macOS |
|-----|---------|-------------|
| `GUI` | `WinMain` (kein Konsolenfenster) | `main` |
| `CONSOLE` | `main` (mit Konsole) | `main` |

## Migration von alter Struktur

Bei Upgrade von der alten `tests.unit`/`tests.integration` Struktur:

### Vorher (Alt)
```json
"tests": {
    "framework": "doctest",
    "unit": { "timeout": 30 },
    "integration": { "timeout": 120 }
}
```

```
tests/unit/Application_Tests.cpp
tests/integration/Application_Integration_Tests.cpp
```

### Nachher (Neu)
```json
"tests": {
    "framework": "doctest",
    "targets": [
        { "name": "UnitTests", "type": "unit", "path": "tests/unit/UnitTests" },
        { "name": "IntegrationTests", "type": "integration", "path": "tests/integration/IntegrationTests" }
    ]
}
```

```
tests/unit/UnitTests/Application_Tests.cpp
tests/integration/IntegrationTests/Application_Integration_Tests.cpp
```

## Siehe auch

- [App Tests Targets Konzept](../../docs/de/concepts/App_Tests_Targets_Concept.md)
- [Solution Schema](../../docs/de/references/Solution_Schema.md)
- [App Creation Guide](../../docs/de/userguides/App_Creation_Guide.md)

---

## Änderungsprotokoll

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.2.1** | **2025-12-18** | **Skip-Feature Dokumentation hinzugefügt (global tests.skip + per-target skip)** |
| 0.2.0 | 2025-12-18 | Neue tests.targets[] Struktur, mehrere Tests pro Typ, flexible Test-Typen |
| 0.1.3 | 2025-12-18 | test_main.cpp für doctest hinzugefügt |
| 0.1.2 | 2025-12-18 | PCH include in main.cpp hinzugefügt |
| 0.1.1 | 2025-12-18 | Integration/Performance Templates hinzugefügt |
| 0.1.0 | 2025-12-17 | Initiale Vorlage |
