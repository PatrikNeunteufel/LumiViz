# App Tests Targets Concept

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Status:** Implementiert  
> **Author:** CMake Architecture Team

---

## 1. Overview

This concept describes die flexible Test-Configuration für App-Container mittels `tests.targets[]` Array. Es ermöglicht beliebig viele Tests pro App mit individueller Configuration.

### 1.1 Ziele

- Beliebig viele Tests pro App
- Beliebige Test-Typen (nicht auf unit/integration/performance beschränkt)
- Individuelles Framework pro Test
- Sinnvolle Defaults für bekannte Typen
- Klare Ordnerstruktur-Konventionen
- **Flexibles Überspringen von Tests (global und per-target)**

### 1.2 Nicht-Ziele

- Abwärtskompatibilität zur alten `tests.unit`/`tests.integration`/`tests.performance` Struktur
- Automatische Migration bestehender Projekte

---

## 2. JSON-Schema

### 2.1 Vollständige Struktur

```json
{
    "apps": [
        {
            "name": "MyApp",
            "tests": {
                "skip": false,
                "framework": "doctest",
                "targets": [
                    {
                        "name": "Core_UnitTests",
                        "type": "unit",
                        "skip": false,
                        "path": "tests/unit/core",
                        "framework": "doctest",
                        "timeout": 30,
                        "labels": ["unit", "core", "fast"],
                        "externals": [],
                        "parallel": true
                    }
                ]
            }
        }
    ]
}
```

### 2.2 Feld-Definitionen

#### tests (Object)

| Feld | Typ | Required | Default | Description |
|------|-----|---------|---------|--------------|
| `skip` | bool | No | `false` | **Alle Tests dieser App überspringen** |
| `framework` | string | No | — | Globales Default-Framework für alle Tests |
| `targets` | array | Yes | `[]` | Liste der Test-Targets |

#### tests.targets[] (Object)

| Feld | Typ | Required | Default | Description |
|------|-----|---------|---------|--------------|
| `name` | string | Yes | — | Target-Name (wird zu `{AppName}.{name}`) |
| `type` | string | Yes | — | Test-Typ (unit, integration, etc.) |
| `skip` | bool | No | `false` | **Diesen Test überspringen** |
| `path` | string | No | `tests/{type}/{name}` | Pfad relativ zum App-Verzeichnis |
| `framework` | string | No | `tests.framework` | Test-Framework (doctest, googletest, catch2) |
| `timeout` | number | No | Typ-abhängig | CTest Timeout in Sekunden |
| `labels` | array | No | Typ-abhängig | CTest Labels |
| `externals` | array | No | `[]` | Zusätzliche Externals für diesen Test |
| `parallel` | bool | No | Typ-abhängig | Parallele Ausführung erlaubt |

---

## 2.5 Skip-Feature

Das Skip-Feature ermöglicht das temporäre Deaktivieren von Tests ohne sie aus der Configuration zu entfernen.

### 2.5.1 Globales Skip

Überspringt **alle** Tests einer App:

```json
"tests": {
    "skip": true,
    "framework": "doctest",
    "targets": [
        { "name": "UnitTests", "type": "unit" },
        { "name": "IntegrationTests", "type": "integration" }
    ]
}
```

**CMake-Ausgabe:**
```
[Apps]   SKIP: All tests for MyApp (tests.skip=true)
```

### 2.5.2 Per-Target Skip

Überspringt einzelne Tests:

```json
"tests": {
    "framework": "doctest",
    "targets": [
        { 
            "name": "UnitTests", 
            "type": "unit",
            "skip": false
        },
        { 
            "name": "IntegrationTests", 
            "type": "integration",
            "skip": true
        },
        { 
            "name": "PerformanceTests", 
            "type": "performance",
            "skip": true
        }
    ]
}
```

**CMake-Ausgabe:**
```
[Apps]   Created: MyApp.UnitTests (unit, doctest)
[Apps]   SKIP: MyApp.IntegrationTests (skip=true)
[Apps]   SKIP: MyApp.PerformanceTests (skip=true)
```

### 2.5.3 Skip-Logik

| Global `tests.skip` | Target `skip` | Ergebnis |
|---------------------|---------------|----------|
| `true` | egal | ⏭️ Übersprungen |
| `false` / fehlt | `true` | ⏭️ Übersprungen |
| `false` / fehlt | `false` / fehlt | ✅ Wird gebaut |

**Important:** Globales Skip hat Vorrang — wenn `tests.skip: true`, werden alle Tests übersprungen, unabhängig von individuellen `skip`-Einstellungen.

### 2.5.4 Anwendungsfälle

| Szenario | Configuration |
|----------|---------------|
| CI schneller machen | `"tests": { "skip": true }` für Apps mit langsamen Tests |
| Broken Test temporär deaktivieren | `"skip": true` für einzelnen Test |
| Performance-Tests nur nachts | `"skip": true`, Nightly-Pipeline setzt auf `false` |
| Plattform-spezifische Tests | Mit `platforms: []` kombinieren |

### 2.5.5 Konsistenz mit anderen Targets

Das Skip-Feature verhält sich konsistent mit anderen Target-Typen:

| Target-Typ | Skip-Property | Verhalten |
|------------|---------------|-----------|
| `executables[]` | `skip: true` | Target wird nicht erstellt |
| `libraries[]` | `skip: true` | Target wird nicht erstellt |
| `tests[]` (standalone) | `skip: true` | Test wird nicht erstellt |
| `apps[].tests.skip` | `skip: true` | Alle App-Tests werden übersprungen |
| `apps[].tests.targets[].skip` | `skip: true` | Einzelner App-Test wird übersprungen |

---

## 3. Bekannte Test-Typen

Diese Typen haben vordefinierte Defaults. Unbekannte Typen sind erlaubt und erhalten generische Defaults.

### 3.1 Typ-Definitionen

| Typ | Description | Default Timeout | Default Labels | Parallel |
|-----|--------------|-----------------|----------------|----------|
| `unit` | Isolierte Funktions-/Klassen-Tests | 30s | `["unit", "fast"]` | true |
| `integration` | Komponenten-Interaktionen | 120s | `["integration"]` | true |
| `performance` | Zeitmessung, Benchmarks | 300s | `["performance", "benchmark"]` | false |
| `system` | End-to-End durch gesamtes System | 180s | `["system", "e2e", "slow"]` | false |
| `smoke` | Schneller Basis-Check | 10s | `["smoke", "critical", "fast"]` | true |
| `fuzz` | Zufällige/ungültige Eingaben | 60s | `["fuzz", "security"]` | false |
| `security` | Sicherheitstests | 120s | `["security"]` | false |
| `ui` | Benutzeroberflächen-Tests | 180s | `["ui", "slow"]` | false |
| `api` | API-Endpunkt-Tests | 60s | `["api", "integration"]` | true |
| *(unbekannt)* | Benutzerdefinierter Typ | 60s | `["{type}"]` | true |

### 3.2 Usageszweck

#### unit
- **Wann:** Einzelne Functions, Klassen, Module isoliert testen
- **Eigenschaften:** Schnell, keine externen Dependencies, bei jedem Build
- **Example:** `Application::init()` gibt `true` zurück

#### integration
- **Wann:** Zusammenspiel mehrerer Komponenten testen
- **Eigenschaften:** Darf externe Ressourcen nutzen (Dateien, Datenbank)
- **Example:** Application initialisiert und kommuniziert mit Logger

#### performance
- **Wann:** Laufzeit, Speicher, Durchsatz messen
- **Eigenschaften:** Mehrfache Durchläufe, statistische Auswertung, Baselines
- **Example:** Init-Zeit unter 100ms

#### system
- **Wann:** Gesamtes System End-to-End testen
- **Eigenschaften:** Vollständiger Workflow, alle Komponenten zusammen
- **Example:** Benutzer startet App, lädt Datei, exportiert Ergebnis

#### smoke
- **Wann:** Schneller Basis-Check nach Build
- **Eigenschaften:** "Startet es überhaupt?", kritische Pfade
- **Example:** Application kann erstellt werden ohne Exception

#### fuzz
- **Wann:** Robustheit gegen ungültige Eingaben testen
- **Eigenschaften:** Zufällige Daten, Grenzen, Sonderzeichen
- **Example:** API mit zufälligen Strings aufrufen

#### security
- **Wann:** Sicherheitslücken aufdecken
- **Eigenschaften:** Injection, Buffer Overflow, Authentication
- **Example:** SQL-Injection in Eingabefeldern

#### ui
- **Wann:** Benutzeroberfläche testen
- **Eigenschaften:** GUI-Interaktionen, Events, Rendering
- **Example:** Button-Klick löst erwartete Aktion aus

#### api
- **Wann:** API-Endpunkte testen
- **Eigenschaften:** Request/Response, Statuscode, Payload
- **Example:** GET /status gibt 200 OK zurück

---

## 4. Ordnerstruktur

### 4.1 Konvention

```
{AppPath}/
└── tests/
    └── {type}/
        └── {name}/           ← Oder kurz: {name} wenn eindeutig
            ├── Source.cmake
            ├── test_main.cpp
            └── test_*.cpp
```

### 4.2 Path-Auflösung

| Configuration | Resultierender Pfad |
|---------------|---------------------|
| `path` nicht angegeben | `tests/{type}/{name}` |
| `path: "tests/unit/core"` | `tests/unit/core` (explizit) |
| `path: "tests/mytest"` | `tests/mytest` (flach) |

### 4.3 Example-Struktur

```
projects/apps/MyVisualizer/
├── include/
├── src/
├── main/
├── pch/
└── tests/
    ├── unit/
    │   ├── core/                    ← Core_UnitTests
    │   │   ├── Source.cmake
    │   │   ├── test_main.cpp
    │   │   └── test_Application.cpp
    │   └── utils/                   ← Utils_UnitTests
    │       ├── Source.cmake
    │       ├── test_main.cpp
    │       └── test_StringUtils.cpp
    ├── integration/
    │   └── api/                     ← API_IntegrationTests
    │       ├── Source.cmake
    │       ├── test_main.cpp
    │       └── test_ApiEndpoints.cpp
    ├── performance/
    │   └── benchmarks/              ← Benchmarks
    │       ├── Source.cmake
    │       ├── test_main.cpp
    │       └── bench_Algorithms.cpp
    └── smoke/
        └── startup/                 ← Startup_SmokeTests
            ├── Source.cmake
            ├── test_main.cpp
            └── test_Startup.cpp
```

---

## 5. Framework-Auflösung

### 5.1 Priorität

```
1. Test-spezifisch:  targets[].framework
         ↓ (falls leer)
2. Global:           tests.framework
         ↓ (falls leer)
3. FEHLER:           E301 - No framework specified
```

### 5.2 Unterstützte Frameworks

| Framework | Target-Name | Description |
|-----------|-------------|--------------|
| `doctest` | doctest | Header-only, schnell, C++11 |
| `googletest` | gmock_main | Google Test + Mock, feature-reich |
| `catch2` | Catch2WithMain | BDD-Style, moderne Syntax |

### 5.3 Example

```json
"tests": {
    "framework": "doctest",
    "targets": [
        {
            "name": "Core_UnitTests",
            "type": "unit"
            // → Verwendet "doctest" (global)
        },
        {
            "name": "API_IntegrationTests",
            "type": "integration",
            "framework": "googletest"
            // → Verwendet "googletest" (override)
        }
    ]
}
```

---

## 6. Generierte Targets

### 6.1 Naming-Konvention

```
{AppName}.{TestName}
```

| App | Test-Name | CMake Target |
|-----|-----------|--------------|
| MyVisualizer | Core_UnitTests | `MyVisualizer.Core_UnitTests` |
| MyVisualizer | API_IntegrationTests | `MyVisualizer.API_IntegrationTests` |
| DemoPlayer | Smoke | `DemoPlayer.Smoke` |

### 6.2 CTest-Registrierung

Jeder Test wird automatisch bei CTest registriert mit:
- Name: `{AppName}.{TestName}`
- Timeout: Aus Configuration oder Typ-Default
- Labels: App-Name + Typ + konfigurierte Labels
- WORKING_DIRECTORY: `${CMAKE_BINARY_DIR}`

---

## 7. Vollständiges Example

### 7.1 Solution.json

```json
{
    "apps": [
        {
            "name": "MyVisualizer",
            "displayName": "My Visualizer",
            "version": "0.1.0",
            
            "core": {
                "dependencies": [],
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
                "skip": false,
                "framework": "doctest",
                "targets": [
                    {
                        "name": "Core_UnitTests",
                        "type": "unit",
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
                        "name": "Audio_IntegrationTests",
                        "type": "integration",
                        "timeout": 120,
                        "labels": ["integration", "audio"],
                        "externals": ["bass"]
                    },
                    {
                        "name": "Benchmarks",
                        "type": "performance",
                        "path": "tests/performance/benchmarks",
                        "timeout": 300,
                        "labels": ["performance", "nightly"],
                        "skip": true
                    },
                    {
                        "name": "Startup_Smoke",
                        "type": "smoke",
                        "path": "tests/smoke/startup",
                        "timeout": 10,
                        "labels": ["smoke", "critical"]
                    },
                    {
                        "name": "InputFuzz",
                        "type": "fuzz",
                        "framework": "googletest",
                        "timeout": 180,
                        "labels": ["fuzz", "security", "nightly"],
                        "skip": true
                    }
                ]
            }
        }
    ]
}
```

### 7.2 Resultierende Targets

| CMake Target | Framework | Timeout | Status |
|--------------|-----------|---------|--------|
| `MyVisualizer.Core_UnitTests` | doctest | 30s | ✅ Gebaut |
| `MyVisualizer.Utils_UnitTests` | doctest | 30s | ✅ Gebaut |
| `MyVisualizer.Audio_IntegrationTests` | doctest | 120s | ✅ Gebaut |
| `MyVisualizer.Benchmarks` | doctest | 300s | ⏭️ Übersprungen |
| `MyVisualizer.Startup_Smoke` | doctest | 10s | ✅ Gebaut |
| `MyVisualizer.InputFuzz` | googletest | 180s | ⏭️ Übersprungen |

---

## 8. Migration von alter Struktur

### 8.1 Alte Struktur (nicht mehr unterstützt)

```json
"tests": {
    "framework": "doctest",
    "unit": {
        "timeout": 30,
        "labels": ["unit", "fast"]
    },
    "integration": {
        "timeout": 120
    },
    "performance": {
        "timeout": 300
    }
}
```

### 8.2 Neue Struktur

```json
"tests": {
    "framework": "doctest",
    "targets": [
        {
            "name": "UnitTests",
            "type": "unit",
            "path": "tests/unit",
            "timeout": 30,
            "labels": ["unit", "fast"]
        },
        {
            "name": "IntegrationTests",
            "type": "integration",
            "path": "tests/integration",
            "timeout": 120
        },
        {
            "name": "PerformanceTests",
            "type": "performance",
            "path": "tests/performance",
            "timeout": 300
        }
    ]
}
```

### 8.3 Ordner-Anpassung

```bash
# Vorher:
tests/unit/Application_Tests.cpp

# Nachher (Unterordner mit Test-Name):
tests/unit/UnitTests/Application_Tests.cpp

# Oder mit explizitem path: "tests/unit" bleibt wie es ist
```

---

## 9. Errorbehandlung

| Code | Bedingung | Meldung |
|------|-----------|---------|
| E301 | Kein Framework | No framework specified for test '{name}'. Set tests.framework or targets[].framework |
| E302 | Unbekanntes Framework | Unknown framework '{fw}' for test '{name}'. Valid: doctest, googletest, catch2 |
| E303 | Fehlender Name | Test target [{idx}] missing required 'name' field |
| E304 | Fehlender Type | Test target '{name}' missing required 'type' field |
| E305 | Pfad existiert nicht | Test '{name}' path does not exist: {path} |
| E306 | Keine Sources | Test '{name}': No source files found in {path} |
| W401 | Leeres targets Array | App '{name}': tests.targets is empty |
| W402 | Paralleler Serial-Typ | Test '{name}' (type '{type}') has parallel=true. This type typically runs serial for accurate results. |

### 9.1 Parallel-Warning W402

Bestimmte Test-Typen sollten nicht parallel ausgeführt werden, da dies die Ergebnisse verfälschen kann:

| Typ | Grund |
|-----|-------|
| `performance` | Zeitmessungen werden durch konkurrierende Tests ungenau |
| `system` | End-to-End Tests können sich gegenseitig beeinflussen |
| `fuzz` | Ressourcenintensiv, kann System destabilisieren |
| `security` | Isolation wichtig für valide Ergebnisse |
| `ui` | GUI-Tests können sich gegenseitig blockieren |

Wenn `parallel: true` für diese Typen explizit gesetzt wird, erscheint Warning W402.
Das System blockiert nicht, da der Benutzer möglicherweise weiß, was er tut (z.B. dedizierter Test-Server).

---

## 10. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.1** | **2025-12-18** | **Neu: Skip-Feature (global tests.skip + per-target skip), Status auf "Implementiert" geändert** |
| 0.1.0 | 2025-12-18 | Initial: Concept für flexible Test-Targets |
