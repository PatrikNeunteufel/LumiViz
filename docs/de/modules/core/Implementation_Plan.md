# Implementation Plan — CMake Architecture

> **Version:** 1.0.0  
> **Datum:** 2025-12-17  
> **Typ:** Concept  
> **Status:** Phase 1-7 abgeschlossen, Phase 8 in Arbeit  
> **Basiert auf:** master_concept v0.5, Solution_Schema v0.1, ErrorCodes v0.1  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [implementation_plan.md](../../en/projects/buildsystem/concepts/Implementation_Plan.md)

Dieser Plan beschreibt die schrittweise Umsetzung des CMake Build-Systems.

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Phase 1: Foundation](#2-phase-1-foundation)
3. [Phase 2: Solution & Validation](#3-phase-2-solution--validation)
4. [Phase 3: Executable Pipeline](#4-phase-3-executable-pipeline)
5. [Phase 4: Library Pipeline](#5-phase-4-library-pipeline)
6. [Phase 5: Lokale Externals](#6-phase-5-lokale-externals)
7. [Phase 6: Fetched Externals + Hooks](#7-phase-6-fetched-externals--hooks)
8. [Phase 7: Test-Pipeline](#8-phase-7-test-pipeline)
9. [Phase 8: AppContainer (geplant)](#9-phase-8-appcontainer-geplant)
10. [Phase 9: System-Externals (geplant)](#10-phase-9-system-externals-geplant)
11. [Checkliste](#11-checkliste)
12. [Siehe auch](#12-siehe-auch)
13. [Changelog](#13-changelog)

---

## 1. Übersicht

### Phasen-Status

| Phase | Name | Status |
|-------|------|--------|
| 1 | Foundation | ✅ Abgeschlossen |
| 2 | Solution & Validation | ✅ Abgeschlossen |
| 3 | Executable Pipeline | ✅ Abgeschlossen |
| 4 | Library Pipeline | ✅ Abgeschlossen |
| 5 | Lokale Externals | ✅ Abgeschlossen |
| 6 | Fetched Externals + Hooks | ✅ Abgeschlossen |
| 7 | Test-Pipeline | ✅ Abgeschlossen |
| 8 | AppContainer | 🔄 In Arbeit |
| 9 | System-Externals | 🔄 Geplant |

### Phasen-Flow

```
Phase 1: Foundation (Core-Module)
    ↓
Phase 2: Solution & Validation
    ↓
Phase 3: Executable Pipeline
    ↓
Phase 4: Library Pipeline
    ↓
Phase 5: Lokale Externals
    ↓
Phase 6: Fetched Externals + Hooks (.externals/ Caching)
    ↓
Phase 7: Test-Pipeline (doctest, googletest, catch2)
    ↓
Phase 8: AppContainer (geplant)
    ↓
Phase 9: System-Externals (geplant)
```

---

## 2. Phase 1: Foundation

**Status:** ✅ Abgeschlossen

**Ziel:** Grundlegende Infrastruktur für alle weiteren Module.

### Module

| Modul | Beschreibung |
|-------|--------------|
| `Errors.cmake` | `cmake_fatal()`, `cmake_warn()`, `cmake_assert()` |
| `Debug.cmake` | Debug-System mit Leveln |
| `Context.cmake` | Context-Objekt-Pattern |
| `Json.cmake` | JSON-Hilfsfunktionen |
| `Validation.cmake` | Schema-Validierung |
| `SourceCollect.cmake` | Source-Datei-Management |
| `OutputDirs.cmake` | Zielverzeichnisse |
| `Warnings.cmake` | Warning-Level |
| `CompilerOptions.cmake` | Compiler-Konfiguration |

### Erfolgskriterium

```bash
cmake -B build -DRUN_BUILD_SYSTEM_TESTS=ON
# Phase 1 Tests bestehen
```

---

## 3. Phase 2: Solution & Validation

**Status:** ✅ Abgeschlossen

**Ziel:** Solution.json einlesen und validieren.

### Module

| Modul | Beschreibung |
|-------|--------------|
| `Solution.cmake` | JSON laden, GLOBAL Properties setzen |

### Funktionen

```cmake
# Solution.json laden
_load_solution_json()

# Pflichtfelder validieren
validate_solution_structure()

# Settings mit Defaults mergen
_apply_settings_defaults()
```

### Erfolgskriterium

```cmake
# Nach include(Solution.cmake):
# - SOLUTION_JSON ist gesetzt
# - SOLUTION_NAME ist verfügbar
# - SOLUTION_VERSION ist verfügbar
# - SOLUTION_SETTINGS_JSON ist verfügbar
```

---

## 4. Phase 3: Executable Pipeline

**Status:** ✅ Abgeschlossen

**Ziel:** Executables aus Solution.json erstellen.

### Module

| Modul | Beschreibung |
|-------|--------------|
| `Executables.cmake` | Hauptschleife über Executables |
| `ExecutableCollect.cmake` | JSON → Context |
| `ExecutableCreate.cmake` | Target erstellen |

### Pipeline

```
1. JSON parsen
2. Context erstellen (ctx_create)
3. Felder extrahieren (ctx_set)
4. BUILD_ONLY prüfen
5. skip prüfen
6. Source.cmake laden oder GLOB
7. add_executable()
8. PCH konfigurieren
9. CompilerOptions anwenden
10. Warnings setzen
11. OutputDirs setzen
```

### Erfolgskriterium

```bash
cmake -B build
cmake --build build
./build/bin/MinimalConsole  # "Hello World"
```

---

## 5. Phase 4: Library Pipeline

**Status:** ✅ Abgeschlossen

**Ziel:** Libraries aus Solution.json erstellen.

### Module

| Modul | Beschreibung |
|-------|--------------|
| `Libraries.cmake` | Hauptschleife |
| `LibraryCollect.cmake` | JSON → Context |
| `LibraryCreate.cmake` | Target erstellen |
| `Dependencies.cmake` | Interne Abhängigkeiten |

### Library-Typen

| Typ | CMake |
|-----|-------|
| STATIC | `add_library(X STATIC)` |
| SHARED | `add_library(X SHARED)` |
| INTERFACE | `add_library(X INTERFACE)` |

### Erfolgskriterium

```cmake
# Library wird erstellt
# Executable kann gegen Library linken
target_link_libraries(MyApp PRIVATE CoreLib)
```

---

## 6. Phase 5: Lokale Externals

**Status:** ✅ Abgeschlossen

**Ziel:** Lokale Externals einbinden.

### Module

| Modul | Beschreibung |
|-------|--------------|
| `Orchestrator.cmake` | Dispatch nach External-Typ |
| `Local/Attach.cmake` | Include.cmake aufrufen |

### Ablauf

```
1. External-Typ erkennen (path → local)
2. Include.cmake Pfad bestimmen
3. Variablen setzen (EXECUTABLE_NAME, EXTERNAL_ELEMENT_OPTIONS)
4. Include.cmake laden
5. Warnings prüfen (W103, W104)
```

### Erfolgskriterium

```cmake
# BASS lädt korrekt
# target_link_libraries funktioniert
# DLLs werden kopiert
```

---

## 7. Phase 6: Fetched Externals + Hooks

**Status:** ✅ Abgeschlossen

**Ziel:** Git-Externals fetchen mit .externals/ Caching, Hook-System.

### Module

| Modul | Beschreibung |
|-------|--------------|
| `Core/Fetch.cmake` | FetchContent-Wrapper mit Caching |
| `Core/Hash.cmake` | Config-Hashing |
| `Core/Policies.cmake` | Update-Strategie |
| `Hooks/HookLoader.cmake` | Hook-System |
| `Registry/Targets.cmake` | Target-Registry |

### .externals/ Caching (Fetch v0.2.0)

```
project_root/
├── .externals/                 ← Gefetchte Externals (gitignored)
│   ├── glfw/
│   ├── imgui/
│   └── .lockfile.json
├── externals/                  ← Lokale Externals (committed)
│   └── bass/
└── build/                      ← Alle Presets teilen .externals/
```

**Vorteile:**
- Download nur einmal (nicht pro Preset)
- Offline-Modus möglich (`EXTERNALS_OFFLINE=ON`)
- Force re-fetch (`EXTERNALS_FORCE_FETCH=ON`)

### Hook-Ablauf

```
1. Convention-Pfad prüfen
2. Expliziten Pfad prüfen (E216 wenn fehlt)
3. PreFetch Hook laden (wenn vorhanden)
4. FetchContent_Declare/MakeAvailable
5. PostFetch Hook laden (wenn vorhanden)
6. Target in Registry eintragen
```

### Neue Error/Warning Codes

| Code | Typ | Beschreibung |
|------|-----|--------------|
| E218 | Error | External nicht gecached und Offline-Modus aktiv |
| W302 | Warning | Version-Mismatch aber Offline-Modus - nutze Cache |

### Erfolgskriterium

```cmake
# spdlog wird gefetcht nach .externals/spdlog/
# ImGui PostFetch Hook erstellt Target
# E216 bei fehlendem expliziten Hook
# Preset-Wechsel nutzt Cache
```

---

## 8. Phase 7: Test-Pipeline

**Status:** ✅ Abgeschlossen

**Ziel:** Test-Targets mit Framework-Integration und CTest.

### Module

| Modul | Beschreibung |
|-------|--------------|
| `Tests.cmake` | Hauptschleife über tests Array |
| `TestCollect.cmake` | JSON → Context |
| `TestCreate.cmake` | Test-Target erstellen |
| `TestFrameworks.cmake` | Framework-spezifische Konfiguration |

### Solution.json Schema

```json
{
    "tests": [
        {
            "name": "CoreLib_Tests",
            "type": "unit",
            "framework": "doctest",
            "dependencies": ["CoreLib"],
            "externals": ["doctest"],
            "labels": ["unit", "fast"],
            "timeout": 30
        }
    ]
}
```

### Unterstützte Frameworks

| Framework | Beschreibung | Empfohlen für |
|-----------|--------------|---------------|
| `doctest` | Schnell, Header-only | Unit Tests, CI |
| `googletest` | Feature-reich, Mocking | Große Projekte |
| `catch2` | BDD-Style, Sections | Lesbare Tests |

### Test-Typen

| Typ | Beschreibung | Typische Labels |
|-----|--------------|-----------------|
| `unit` | Einzelne Funktionen/Klassen | `fast`, `isolated` |
| `integration` | Komponenten-Zusammenspiel | `slow`, `database` |
| `system` | Gesamtsystem | `e2e`, `slow` |
| `performance` | Benchmarks | `benchmark`, `slow` |
| `smoke` | Schnelle Basis-Tests | `fast`, `critical` |

### CTest-Integration

```bash
# Alle Tests
ctest --test-dir build

# Nur Unit Tests
ctest -L unit

# Parallel
ctest -j8

# Verbose bei Fehlern
ctest --output-on-failure
```

### Erfolgskriterium

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build
# Alle Tests bestehen
```

---

## 9. Phase 8: AppContainer (in Arbeit)

**Status:** 🔄 In Arbeit

**Ziel:** Testbare App-Architektur mit Core/Runner-Trennung.

→ **Detail-Konzept:** [AppContainer_Concept.md](AppContainer_Concept.md)

### Module

| Modul | Status | Beschreibung |
|-------|--------|--------------|
| `Apps.cmake` | ✅ Implementiert | Pipeline-Orchestrator, Filter (skip, platform, BUILD_ONLY) |
| `AppCollect.cmake` | ✅ Implementiert | JSON → Context Transformation |
| `AppCreate.cmake` | ✅ Implementiert | Target-Erstellung (Core, Runner, Tests) |
| `Phase8.cmake` | ✅ Implementiert | Build-System-Test |

### Generierte Targets

Pro App werden folgende Targets erstellt:

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `{AppName}.Core` | STATIC Library | Business-Logik (testbar) |
| `{AppName}` | Executable | Entry Point (main()) |
| `{AppName}.UnitTests` | Executable | Unit Tests (optional) |
| `{AppName}.IntegrationTests` | Executable | Integration Tests (optional) |

### Verzeichnisstruktur

```
projects/apps/{AppName}/
├── include/    → PUBLIC Headers (Core)
├── src/        → Implementation (Core)
├── main/       → Entry Point (Runner)
├── pch/        → Precompiled Headers (optional)
└── tests/
    ├── unit/        → Unit Tests
    └── integration/ → Integration Tests
```

### Error-Code-Bereich

| Bereich | Codes | Kategorie |
|---------|-------|-----------|
| E4xx | E401-E407 | AppContainer-Fehler |
| W4xx | W401-W403 | AppContainer-Warnungen |

### Offene Punkte

- [ ] SourceCollect.cmake Integration (aktuell GLOB)
- [ ] UserGuide für App-Container
- [ ] Englische Dokumentation

---

## 10. Phase 9: System-Externals (geplant)

**Status:** 🔄 Geplant

**Ziel:** System-Bibliotheken via `find_package()` integrieren.

→ **Detail-Konzept:** [System_Externals_Concept.md](System_Externals_Concept.md)

### Kernpunkte

- Qt, Boost, OpenCV über `find_package()` einbinden
- Neue `find_package` Feld in Solution.json externals
- Hybrid-Externals (System-fallback auf fetched)

### Error-Code-Bereich

| Bereich | Kategorie |
|---------|-----------|
| E5xx | System-External-spezifische Fehler |

---

## 11. Checkliste

### Phase 1 ✅

- ✅ Errors.cmake
- ✅ Debug.cmake
- ✅ Context.cmake
- ✅ Json.cmake
- ✅ Validation.cmake
- ✅ SourceCollect.cmake
- ✅ OutputDirs.cmake
- ✅ Warnings.cmake
- ✅ CompilerOptions.cmake
- ✅ Phase 1 Tests bestehen

### Phase 2 ✅

- ✅ Solution.cmake
- ✅ SOLUTION_* Properties gesetzt
- ✅ Schema-Validierung funktioniert

### Phase 3 ✅

- ✅ Executables.cmake
- ✅ ExecutableCollect.cmake
- ✅ ExecutableCreate.cmake
- ✅ MinimalConsole baut und läuft

### Phase 4 ✅

- ✅ Libraries.cmake
- ✅ LibraryCollect.cmake
- ✅ LibraryCreate.cmake
- ✅ Dependencies.cmake
- ✅ Executable linkt gegen Library

### Phase 5 ✅

- ✅ Orchestrator.cmake
- ✅ Local/Attach.cmake
- ✅ BASS lädt korrekt
- ✅ W103/W104 Prüfung

### Phase 6 ✅

- ✅ Core/Fetch.cmake (v0.2.0 mit .externals/)
- ✅ Hooks/HookLoader.cmake
- ✅ Registry/Targets.cmake
- ✅ Git-External wird gefetcht
- ✅ Hook-System funktioniert
- ✅ E216 bei fehlendem Hook
- ✅ Offline-Modus (E218, W302)

### Phase 7 ✅

- ✅ Tests.cmake
- ✅ TestCollect.cmake
- ✅ TestCreate.cmake
- ✅ TestFrameworks.cmake
- ✅ CTest-Integration
- ✅ doctest Support
- ✅ googletest Support
- ✅ catch2 Support
- ✅ Labels und Timeout

### Phase 8 🔄

- ✅ Apps.cmake
- ✅ AppCollect.cmake
- ✅ AppCreate.cmake
- ✅ Phase8.cmake (Build-System-Test)
- ✅ DemoPlayer Test-App
- ✅ Core/Runner/Tests Trennung
- ✅ Platform-Filter
- ✅ Unit Tests mit doctest
- [ ] SourceCollect.cmake Integration
- [ ] UserGuide für App-Container

### Phase 9 (geplant)

- [ ] System/FindExternal.cmake
- [ ] Hybrid-External-Support
- [ ] Qt/Boost/OpenCV Tests

---

## 12. Siehe auch

- [master_concept.md](master_concept.md) — Architektur
- [guidelines.md](../standards/guidelines.md) — Konventionen
- [AppContainer_Concept.md](AppContainer_Concept.md) — Phase 8 Detail
- [System_Externals_Concept.md](System_Externals_Concept.md) — Phase 9 Detail
- [Solution_Schema](../../../references/Solution_Schema.md) — JSON-Schema
- [ErrorCodes](../../../references/ErrorCodes.md) — Fehlercodes

---

## 13. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.1** | **2025-12-17** | **Phase 8 in Arbeit: Apps.cmake, AppCollect.cmake, AppCreate.cmake implementiert, DemoPlayer Test-App, Core/Runner/Tests Trennung** |
| 0.5.0 | 2025-12-14 | Phase 1-7 abgeschlossen, Fetch v0.2 Details integriert, Test-Pipeline Details integriert, Phase 8/9 als geplant referenziert, Blueprint v0.5.0 Format |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): Phasen aus v1.5 übernommen |
