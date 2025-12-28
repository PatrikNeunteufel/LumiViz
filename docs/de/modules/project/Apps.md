# Apps.cmake — Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-17  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung  
> **Zielgruppe:** Build-System-Entwickler  
> **Modul:** [cmake/project/Apps.cmake](../../../../cmake/project/Apps.cmake)  
> **Modul-Version:** 1.0.0  
> **Basiert auf:** ModuleDoc v0.5  
> **Sprache:** Deutsch  
> **English:** [Apps.md](../../../en/modules/project/Apps.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Konzept](#3-konzept)
4. [Pipeline-Ablauf](#4-pipeline-ablauf)
5. [Generierte Targets](#5-generierte-targets)
6. [Filter-Mechanismen](#6-filter-mechanismen)
7. [Verwendungsbeispiele](#7-verwendungsbeispiele)
8. [Fehlerbehandlung](#8-fehlerbehandlung)
9. [Best Practices](#9-best-practices)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

Das `Apps`-Modul ist der **Orchestrator für die App-Container-Pipeline**. Es iteriert über alle `apps[]` Einträge in der Solution.json und koordiniert die Erstellung aller zugehörigen Targets.

### Zweck

- Iteration über `apps[]` Array in Solution.json
- Anwendung von Skip- und Platform-Filtern
- Koordination der Target-Erstellung (Core, Runner, Tests)
- Debug-Output für Pipeline-Verfolgung

### Generierte Targets pro App

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `{AppName}.Core` | STATIC Library | Business-Logik, testbar |
| `{AppName}` | Executable | Entry-Point (main()) |
| `{AppName}.UnitTests` | Test Executable | Unit Tests (optional) |
| `{AppName}.IntegrationTests` | Test Executable | Integration Tests (optional) |

---

## 2. Abhängigkeiten

| Abhängigkeit | Typ | Beschreibung |
|--------------|-----|--------------|
| CMake 3.19+ | System | JSON-Funktionen |
| `Errors.cmake` | Modul | Fehlerbehandlung |
| `Debug.cmake` | Modul | Debug-Ausgabe |
| `Json.cmake` | Modul | JSON-Parsing |
| `Context.cmake` | Modul | Context-Object-Pattern |
| `Solution.cmake` | Modul | Solution-Properties |

### Auto-Loaded

| Modul | Beschreibung |
|-------|--------------|
| `AppCollect.cmake` | JSON → Context Transformation |
| `AppCreate.cmake` | Target-Erstellung |

---

## 3. Konzept

### 3.1 Pipeline-Architektur

```
Solution.json
     │
     ▼
┌─────────────────────────────────────────────────────────┐
│                    Apps.cmake                            │
│  ┌─────────────────────────────────────────────────┐    │
│  │              Für jede App:                       │    │
│  │                                                  │    │
│  │  1. JSON extrahieren                            │    │
│  │  2. AppCollect → Context                        │    │
│  │  3. Skip-Filter prüfen                          │    │
│  │  4. BUILD_ONLY-Filter prüfen                    │    │
│  │  5. Platform-Filter prüfen                      │    │
│  │  6. Duplikat-Check                              │    │
│  │  7. AppCreate aufrufen:                         │    │
│  │     a. _create_app_core()                       │    │
│  │     b. _create_app_runner()                     │    │
│  │     c. _create_app_tests() (wenn BUILD_TESTS)   │    │
│  └─────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────┘
```

### 3.2 Separation of Concerns

| Modul | Verantwortung |
|-------|---------------|
| `Apps.cmake` | Orchestrierung, Filter, Iteration |
| `AppCollect.cmake` | JSON-Parsing, Context-Befüllung |
| `AppCreate.cmake` | CMake-Target-Erstellung |

---

## 4. Pipeline-Ablauf

### 4.1 Schritt-für-Schritt

```
1. Solution.json laden (GLOBAL PROPERTY)
2. Prüfen ob 'apps' Array existiert
3. Für jeden Eintrag im Array:
   │
   ├─ JSON extrahieren
   ├─ Name validieren (E401 wenn fehlt)
   ├─ Context erstellen & befüllen
   │
   ├─ Skip-Check (skip=true → überspringen)
   ├─ BUILD_ONLY-Check (nicht in Liste → überspringen)
   ├─ Platform-Check (nicht unterstützt → überspringen)
   │
   ├─ Duplikat-Check (E102 wenn Target existiert)
   │
   └─ Targets erstellen:
      ├─ _create_app_core()    → {AppName}.Core
      ├─ _create_app_runner()  → {AppName}
      └─ _create_app_tests()   → {AppName}.*Tests (wenn BUILD_TESTS)
```

### 4.2 Early-Exit-Bedingungen

| Bedingung | Aktion |
|-----------|--------|
| Kein `apps` Array | Return (kein Fehler) |
| Leeres `apps` Array | Return (kein Fehler) |
| `skip=true` | Continue zur nächsten App |
| Nicht in `BUILD_ONLY` | Continue zur nächsten App |
| Platform nicht unterstützt | Continue zur nächsten App |

---

## 5. Generierte Targets

### 5.1 Target-Namenskonvention

```
AudioPlayer.Core            ← STATIC Library
AudioPlayer                 ← Executable
AudioPlayer.UnitTests       ← Test (wenn tests.unit definiert)
AudioPlayer.IntegrationTests ← Test (wenn tests.integration definiert)
```

### 5.2 Target-Abhängigkeiten

```
AudioPlayer.Core
     │
     ├──────────────────┐
     ▼                  ▼
AudioPlayer      AudioPlayer.UnitTests
                        │
                        ▼
              AudioPlayer.IntegrationTests
```

---

## 6. Filter-Mechanismen

### 6.1 Skip-Flag

```json
{
    "apps": [
        {
            "name": "DisabledApp",
            "skip": true
        }
    ]
}
```

### 6.2 BUILD_ONLY

```bash
cmake -B build -DBUILD_ONLY="AudioPlayer;VideoPlayer"
```

Das Modul prüft sowohl `{AppName}` als auch `{AppName}.Core` in der BUILD_ONLY-Liste.

### 6.3 Platform-Filter

```json
{
    "apps": [
        {
            "name": "WindowsOnlyApp",
            "platforms": ["windows"]
        }
    ]
}
```

**Unterstützte Plattformen:**
- `windows` → WIN32
- `linux` → CMAKE_SYSTEM_NAME = "Linux"
- `macos` → APPLE
- `unix` → UNIX

---

## 7. Verwendungsbeispiele

### 7.1 CMakeLists.txt Integration

```cmake
# Nach Libraries und Executables
include(cmake/project/Apps.cmake)
```

### 7.2 Solution.json mit Apps

```json
{
    "solution": {
        "name": "MyProject",
        "version": "1.0.0"
    },
    
    "externals": {
        "bass": { "path": "externals/bass" },
        "doctest": { "path": "externals/doctest" }
    },
    
    "apps": [
        {
            "name": "AudioPlayer",
            "core": {
                "externals": ["bass"]
            },
            "runner": {
                "type": "CONSOLE"
            },
            "tests": {
                "framework": "doctest",
                "unit": {
                    "labels": ["unit", "audio"]
                }
            }
        }
    ]
}
```

### 7.3 Build-Kommandos

```bash
# Alle Apps bauen
cmake -B build
cmake --build build

# Nur bestimmte App
cmake -B build -DBUILD_ONLY="AudioPlayer"

# Mit Tests
cmake -B build -DBUILD_TESTS=ON
ctest --test-dir build -L AudioPlayer
```

---

## 8. Fehlerbehandlung

### 8.1 Ausgelöste Fehler

| Code | Bedingung | Meldung |
|------|-----------|---------|
| `E401` | `name` fehlt | `App #N has no 'name' field` |
| `E102` | Target existiert | `Target '{AppName}.Core' already exists` |
| `E102` | Target existiert | `Target '{AppName}' already exists` |

### 8.2 Fehler aus Sub-Modulen

Zusätzliche Fehler können von `AppCollect` und `AppCreate` ausgelöst werden (E402-E407, W401-W403).

---

## 9. Best Practices

### 9.1 Do's

| Empfehlung | Begründung |
|------------|------------|
| Apps nach Libraries/Executables laden | Abhängigkeiten müssen existieren |
| Eindeutige App-Namen verwenden | Vermeidet E102 |
| Platform-Filter für OS-spezifische Apps | Sauberer Cross-Platform-Build |

### 9.2 Don'ts

| Vermeiden | Grund |
|-----------|-------|
| App-Name = Library-Name | Target-Kollision |
| Apps.cmake mehrfach includen | `include_guard(GLOBAL)` verhindert das |
| Tests ohne BUILD_TESTS erwarten | Tests werden nur bei Flag erstellt |

---

## 10. Siehe auch

- [AppCollect.cmake](AppCollect.md) — JSON-Parsing
- [AppCreate.cmake](AppCreate.md) — Target-Erstellung
- [Executables.cmake](Executables.md) — Ähnliches Pattern für Executables
- [AppContainer_Concept.md](../../projects/buildsystem/concepts/AppContainer_Concept.md) — Architektur-Konzept
- [ErrorCodes.md](../../references/ErrorCodes.md) — Fehlercode-Referenz

---

## 11. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-17** | **Initial: Phase 8 App-Container Orchestrator, Skip/Platform/BUILD_ONLY Filter, Target-Erstellung via AppCreate** |
