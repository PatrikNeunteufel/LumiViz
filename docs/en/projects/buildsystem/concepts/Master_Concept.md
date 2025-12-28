# Master Concept — CMake Architecture

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Concept  
> **Status:** Stable (Phase 1-9 abgeschlossen)  
> **Target Audience:** Build System Developers, Architekten  
> **Language:** English  
> **German:** [master_concept.md](../../en/projects/buildsystem/concepts/Master_Concept.md)

Dieses Dokument dient als **zentrale Reference** für das CMake Build-System. Es definiert die Struktur, Prinzipien und das technische Fundament.

---

## Table of Contents

1. [Vision](#1-vision)
2. [Projektstruktur](#2-projektstruktur)
3. [Solution.json Schema](#3-solutionjson-schema)
4. [External-Typen](#4-external-typen)
5. [Hook-System](#5-hook-system)
6. [External-Caching (.externals/)](#6-external-caching-externals)
7. [Context-Objekt Pattern](#7-context-objekt-pattern)
8. [Source-Management](#8-source-management)
9. [Error-Handling](#9-error-handling)
10. [Pipelines](#10-pipelines)
11. [Test-Pipeline](#11-test-pipeline)
12. [App-Container](#12-app-container)
13. [Cache-Variablen](#13-cache-variablen)
14. [Build-System-Tests](#14-build-system-tests)
15. [Dokumentation](#15-dokumentation)
16. [Geplante Erweiterungen](#16-geplante-erweiterungen)
17. [See Also](#17-siehe-auch)
18. [Changelog](#18-changelog)

---

## 1. Vision

Ein modulares, stabiles und plattformübergreifendes CMake-System, das große Multi-Executable-Projekte verwalten kann, JSON-gesteuert ist und minimalen globalen Zustand besitzt.

### Kernprinzipien

- **Deklarativ über JSON** — Imperativ nur wo nötig
- **Kein globaler State** — Alles über Context-Objekte
- **Testbar auf jeder Ebene** — Unit, Integration, End-to-End
- **Fail-fast** — Klare Errormeldungen
- **Single Source of Truth** — Für External-Versionen
- **Convention over Configuration** — Für optionale Features

---

## 2. Projektstruktur

Die CMake-Infrastruktur ist in klar getrennte Verantwortungsbereiche aufgeteilt.

```
CMakeLists.txt                         # Top-Level: Module laden
Solution.json                          # JSON-Definition aller Targets
CMakePresets.json                      # Build-Presets

cmake/
  core/                                # Grundbausteine
    Errors.cmake                       # Error-Handling
    Debug.cmake                        # Debug-System
    Context.cmake                      # Context-Objekt-Pattern
    Json.cmake                         # JSON-Helferfunktionen
    Validation.cmake                   # Schema-Validierung
    SourceCollect.cmake                # Source-Datei-Management
    OutputDirs.cmake                   # Zielordner
    Warnings.cmake                     # Warnlevel
    CompilerOptions.cmake              # Compiler-Optionen

  externals/                           # External-System
    Orchestrator.cmake                 # High-Level Workflow
    Core/                              # Für fetched Externals
      Fetch.cmake                      # FetchContent mit .externals/ Caching
      Hash.cmake
      Policies.cmake
    Hooks/                             # Hook-System
      HookLoader.cmake
      PreFetch/
      PostFetch/
    Local/                             # Für lokale Externals
      Attach.cmake
    Registry/                          # Target-Verwaltung
      Targets.cmake
      Linking.cmake

  project/                             # Pipelines
    Solution.cmake                     # Solution.json laden
    Executables.cmake                  # Executable-Pipeline
    ExecutableCollect.cmake
    ExecutableCreate.cmake
    Libraries.cmake                    # Library-Pipeline
    LibraryCollect.cmake
    LibraryCreate.cmake
    Tests.cmake                        # Test-Pipeline (standalone)
    TestCollect.cmake
    TestCreate.cmake
    TestFrameworks.cmake               # Framework-spezifische Configuration
    Apps.cmake                         # App-Container Pipeline
    AppCollect.cmake
    AppCreate.cmake
    Dependencies.cmake                 # Interne Dependencies

.externals/                            # Gefetchte Externals (gitignored)
  glfw/
  imgui/
  .lockfile.json

externals/                             # Lokale Externals im Repo
  bass/
    Include.cmake
  lua/
    Include.cmake
  doctest/
    Include.cmake

projects/
  apps/                                # App-Container (Phase 8)
    MyVisualizer/
    DemoPlayer/
  exec/                                # Legacy Executables
    MinimalConsole/
  libs/                                # Libraries
    CoreLib/
  tests/                               # Standalone Tests
    CoreLib_UnitTests/
```

---

## 3. Solution.json Schema

Die Solution.json ist das Herzstück der deklarativen Configuration.

**Aktuelle Schema-Version:** `0.1`

### Root-Level Struktur

```json
{
    "schemaVersion": "0.1",
    "solution": { },
    "settings": { },
    "externalsPolicy": { },
    "externals": { },
    "libraries": [ ],
    "executables": [ ],
    "tests": [ ],
    "apps": [ ]
}
```

| Block | Required | Description |
|-------|---------|--------------|
| `schemaVersion` | ✅ | Version des JSON-Schemas |
| `solution` | ✅ | Metadaten (Name, Version, Autoren) |
| `settings` | ❌ | Globale Build-Einstellungen |
| `externalsPolicy` | ❌ | Cache-Verzeichnis, Update-Strategie |
| `externals` | ❌ | Zentrale External-Definitionen |
| `libraries` | ❌ | Interne Libraries |
| `executables` | ❌ | Ausführbare Programme (monolithisch) |
| `tests` | ❌ | Standalone Test-Targets |
| `apps` | ❌ | **App-Container (Core/Runner/Tests)** |

### Zentraler Externals-Block

**Alle External-Definitionen werden zentral definiert.** Executables/Apps referenzieren nur über Namen.

| Problem (dezentral) | Lösung (zentral) |
|---------------------|------------------|
| Version-Drift | Eine Version für alle |
| Update-Aufwand | Eine Stelle ändern |
| Inkonsistente Flags | Einheitliche Configuration |

---

## 4. External-Typen

Der Typ wird automatisch über das vorhandene Feld erkannt:

| Erkennungsfeld | Typ | Status |
|----------------|-----|--------|
| `system` | **system** | ✅ Implementiert |
| `path` | **local** | ✅ Implementiert |
| `git` | **fetched** | ✅ Implementiert |
| `vcpkg` | **vcpkg** | ⬜ Geplant |
| `conan` | **conan** | ⬜ Geplant |

**Validierung:** Genau eines dieser Felder muss vorhanden sein (Error E012).

### Lokale Externals

Liegen bereits im Repository.

```json
"externals": {
    "bass": { "path": "externals/bass" },
    "lua": { "path": "externals/lua" }
}
```

**Convention:** Jedes lokale External muss eine `Include.cmake` haben.

### Fetched Externals

Werden aus Git geklont in `.externals/`.

```json
"externals": {
    "spdlog": {
        "git": "https://github.com/gabime/spdlog.git",
        "tag": "v1.12.0"
    }
}
```

| Feld | Required | Description |
|------|---------|--------------|
| `git` | ✅ | Repository URL |
| `tag` | ❌* | Git-Tag |
| `branch` | ❌* | Git-Branch |
| `commit` | ❌* | Commit-Hash |
| `hooks` | ❌ | Pre/PostFetch Hooks |
| `shallow` | ❌ | Shallow Clone (Default: true) |

*Genau eines von `tag`, `branch`, `commit` erforderlich (Error E215).

### System Externals

Bereits auf dem System installierte Bibliotheken via `find_package()`.

```json
"externals": {
    "qt6": {
        "system": true,
        "package": "Qt6",
        "version": ">=6.5.0",
        "components": ["Core", "Widgets", "Gui"],
        "hints": ["${QT_ROOT}"],
        "backup": "E:/Backup/Qt/6.7.0"
    }
}
```

| Feld | Required | Default | Description |
|------|---------|---------|--------------|
| `system` | ✅ | — | Muss `true` sein |
| `package` | ✅ | — | Name für `find_package()` |
| `version` | ❌ | — | Version-Constraint |
| `components` | ❌ | `[]` | Package-Komponenten |
| `hints` | ❌ | `[]` | Zusätzliche Suchpfade |
| `backup` | ❌ | — | Notfall-Pfad (W501) |
| `required` | ❌ | `true` | Error wenn nicht gefunden |

**Pfad-Auflösung (Stufen):**
1. Environment-Variablen ($QT_ROOT, $BOOST_ROOT)
2. CMAKE_PREFIX_PATH
3. hints[] aus Solution.json
4. Standard-Pfade (C:/Qt/, /opt/Qt/, etc.)
5. backup Pfad (mit W501 Warning)
6. Error E501/E503

→ Detail: [System_Externals_Concept.md](System_Externals_Concept.md)

---

## 5. Hook-System

Für Externals die spezielle Behandlung benötigen.

### Convention over Configuration

| Situation | Verhalten |
|-----------|-----------|
| Keine Hooks angegeben, kein Convention-Pfad | Kein Hook |
| Keine Hooks angegeben, Convention-Pfad existiert | Auto-Load |
| Hooks explizit angegeben, Datei existiert | Laden |
| Hooks explizit angegeben, Datei fehlt | **Error E216** |

**Convention-Pfade:**
- PreFetch: `cmake/externals/Hooks/PreFetch/${name}.cmake`
- PostFetch: `cmake/externals/Hooks/PostFetch/${name}.cmake`

---

## 6. External-Caching (.externals/)

**Implementiert in Phase 6 (Fetch.cmake v0.2.0)**

### Verzeichnisstruktur

```
project_root/
├── .externals/                 ← Gefetchte Externals (gitignored, versteckt)
│   ├── glfw/
│   ├── imgui/
│   └── .lockfile.json          ← Versions-Tracking
│
├── externals/                  ← Lokale Externals (im Repository)
│   ├── bass/
│   └── lua/
│
└── build/                      ← Build-Verzeichnisse (alle teilen .externals/)
    ├── msvc-debug/
    ├── msvc-release/
    └── clang-release/
```

### Klare Trennung

| Verzeichnis | Inhalt | Git-Status | Quelle |
|-------------|--------|------------|--------|
| `externals/` | Lokale Externals | ✅ Committed | `path` in Solution.json |
| `.externals/` | Gefetchte Externals | ❌ Gitignored | `git` in Solution.json |

### CMake-Optionen

| Option | Default | Description |
|--------|---------|--------------|
| `EXTERNALS_OFFLINE` | OFF | Kein Netzwerk, nur Cache |
| `EXTERNALS_FORCE_FETCH` | ON | Cache ignorieren, neu laden |

### Vorteile

| Aspekt | Vorher (v0.1.0) | Nachher (v0.2.0) |
|--------|-----------------|------------------|
| **Download** | Bei jedem Configure | Nur einmal |
| **Preset-Wechsel** | Neuer Download | Sofort (Cache) |
| **Offline** | Error | Funktioniert |
| **Speicher** | N × Kopien | 1 × Kopie |

---

## 7. Context-Objekt Pattern

Statt globaler Variablen nutzt jedes Target einen eigenen Namensraum.

```cmake
ctx_create(EXE_MyApp)
ctx_set(EXE_MyApp NAME "MyApp")
ctx_set(EXE_MyApp PATH "src/app")
ctx_get(EXE_MyApp NAME _name)
```

**Vorteile:**
- Isolierte Namensräume
- Parallele Verarbeitung möglich
- Kein State-Leaking

---

## 8. Source-Management

Drei Modi für Source-Dateien:

| Mode | Description |
|------|--------------|
| `explicit` | Source.cmake erforderlich (Default) |
| `glob` | Automatisches Sammeln |
| `auto` | Source.cmake wenn vorhanden, sonst GLOB |

**Empfehlung:** `explicit` für maximale Kontrolle.

---

## 9. Error-Handling

Einheitliches System über `Errors.cmake`:

```cmake
cmake_fatal("E001" "Description")   # Bricht ab
cmake_warn("W001" "Description")    # Läuft weiter
cmake_assert(CONDITION "Message")    # Interne Prüfung
```

### Errorcode-Bereiche

| Bereich | Kategorie |
|---------|-----------|
| E0xx | JSON/Parsing |
| E1xx | Target-Erstellung |
| E2xx | Externals |
| E3xx | Tests |
| E4xx | App-Container |
| E5xx | System-Externals (Phase 9) |
| W0xx | Deprecation |
| W1xx | Configuration |
| W2xx | Tools/Setup |
| W3xx | External-Caching |
| W4xx | App-Container Warningen |

---

## 10. Pipelines

### Executable-Pipeline

1. **Collect:** JSON → Context
2. **Validate:** Requiredfelder, Dependencies
3. **Create:** CMake-Target erstellen
4. **Configure:** PCH, Sources, Externals, Options

### Library-Pipeline

Analog zu Executables, zusätzlich:
- PUBLIC/PRIVATE Headers
- STATIC/SHARED/INTERFACE Typen

### App-Pipeline

Erweiterte Pipeline für testbare Apps:
1. **Collect:** JSON → Context (inkl. tests.targets[])
2. **Filter:** skip, platforms, BUILD_ONLY
3. **Create Core:** STATIC Library
4. **Create Runner:** Executable (linkt gegen Core)
5. **Create Tests:** Für jedes Target in tests.targets[]

---

## 11. Test-Pipeline

**Implementiert in Phase 7**

### Abgrenzung

| Test-Art | Zweck | Ort | Aktivierung |
|----------|-------|-----|-------------|
| **Build-System-Tests** | CMake-Module testen | `cmake/buildSystemTest/` | `RUN_BUILD_SYSTEM_TESTS=ON` |
| **Standalone Tests** | Library-Code testen | `projects/tests/` | `BUILD_TESTS=ON` |
| **App-Tests** | App-Code testen | `projects/apps/{App}/tests/` | `BUILD_TESTS=ON` |

### Test-Definition in Solution.json

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
            "timeout": 30,
            "skip": false
        }
    ]
}
```

### Unterstützte Frameworks

| Framework | Empfohlen für |
|-----------|---------------|
| `doctest` | Schnelle Unit Tests, CI |
| `googletest` | Mocking, parametrisierte Tests |
| `catch2` | BDD-Style, integrierte Benchmarks |

### Test-Typen

| Typ | Description |
|-----|--------------|
| `unit` | Einzelne Functions/Klassen |
| `integration` | Komponenten-Zusammenspiel |
| `system` | Gesamtsystem |
| `performance` | Benchmarks |
| `smoke` | Schnelle Basis-Tests |

---

## 12. App-Container

**Implementiert in Phase 8**

App-Container ermöglichen testbare Anwendungsarchitektur durch Trennung von Business-Logik und Entry-Point.

→ **Detail-Concept:** [AppContainer_Concept.md](AppContainer_Concept.md)  
→ **Test-Concept:** [App_Tests_Targets_Concept.md](App_Tests_Targets_Concept.md)

### Architecture

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
└─────────────────────────────────────────────────────────────────┘
```

### Verzeichnisstruktur

```
projects/apps/{AppName}/
├── include/    → PUBLIC Headers (Core)
├── src/        → Implementation (Core)
├── main/       → Entry Point (Runner)
├── pch/        → Precompiled Headers (optional)
└── tests/
    └── {type}/
        └── {TestName}/
            ├── Source.cmake
            ├── test_main.cpp
            └── test_*.cpp
```

### App-Definition in Solution.json

```json
{
    "apps": [
        {
            "name": "MyVisualizer",
            "displayName": "My Visualizer",
            "version": "1.0.0",
            "skip": false,
            
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
                "skip": false,
                "framework": "doctest",
                "targets": [
                    {
                        "name": "UnitTests",
                        "type": "unit",
                        "skip": false
                    }
                ]
            }
        }
    ]
}
```

### Generierte Targets

| Target | Typ | Description |
|--------|-----|--------------|
| `{AppName}.Core` | STATIC Library | Business-Logik |
| `{AppName}` | Executable | Entry Point |
| `{AppName}.{TestName}` | Executable | Tests |

### Flexible Test-Configuration

- Beliebig viele Tests pro App via `tests.targets[]`
- Beliebige Test-Typen (nicht nur unit/integration/performance)
- Type-based Defaults für Timeout, Parallel, Labels
- Framework Override pro Test
- Skip-Feature (global + per-target)

---

## 13. Cache-Variablen

| Variable | Default | Description |
|----------|---------|--------------|
| `BUILD_TESTS` | ON | Projekt-Tests aktivieren |
| `BUILD_ONLY` | "" | Nur bestimmte Targets |
| `RUN_BUILD_SYSTEM_TESTS` | OFF | CMake-Modul-Tests |
| `ENABLE_CLANG_TIDY` | OFF | Code-Qualitätschecks |
| `ENABLE_STRICT_CONFORMANCE` | ON | MSVC strict mode |
| `NO_EXCEPTIONS` | OFF | Exceptions deaktivieren |
| `NO_RTTI` | OFF | RTTI deaktivieren |
| `EXTERNALS_OFFLINE` | OFF | Offline-Modus |
| `EXTERNALS_FORCE_FETCH` | OFF | Force re-fetch |

---

## 14. Build-System-Tests

| Test | Description |
|------|--------------|
| `test_context.cmake` | Context-API |
| `test_json_helpers.cmake` | JSON-Parsing |
| `test_errors.cmake` | Error-Handling |
| `test_validation.cmake` | Schema-Validierung |
| `test_simple_executable.cmake` | Minimales Projekt |
| `test_externals_pipeline.cmake` | Fetch + Registry |
| `test_local_externals.cmake` | Lokale Externals |
| `test_hooks.cmake` | Hook-System |

---

## 15. Dokumentation

| Dokument | Description |
|----------|--------------|
| **Solution_Schema** | JSON-Schema-Dokumentation (inkl. apps[], system externals) |
| **ErrorCodes** | Alle Errorcodes (E0xx-E5xx, W0xx-W5xx) |
| **guidelines** | Coding-Konventionen |
| **implementation_plan** | Phasen-basierter Plan |
| **AppContainer_Concept** | App-Container Architecture |
| **App_Tests_Targets_Concept** | Flexible App-Tests |
| **System_Externals_Concept** | System Externals (find_package) |

---

## 16. Geplante Erweiterungen

### Post-Release

- vcpkg/Conan Integration
- Parallelisierung des External-Fetchings
- IDE-Integration (VSCode, CLion)
- Export-Mechanismus für Libraries
- Lockfile-System für reproduzierbare Builds

---

## 17. See Also

- [implementation_plan.md](implementation_plan.md) — Phasen-basierter Plan
- [guidelines.md](../standards/guidelines.md) — Konventionen
- [AppContainer_Concept.md](AppContainer_Concept.md) — App-Container Detail
- [App_Tests_Targets_Concept.md](App_Tests_Targets_Concept.md) — Flexible App-Tests
- [System_Externals_Concept.md](System_Externals_Concept.md) — Phase 9 Detail
- [Future_Enhancements.md](Future_Enhancements.md) — Post-Release Features

---

## 18. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.7.0** | **2025-12-19** | **Phase 9 abgeschlossen: System Externals (§4), system Typ vor path/git, Geplante Erweiterungen aktualisiert (§16)** |
| 0.6.0 | 2025-12-18 | Phase 8 abgeschlossen: App-Container Section (§12) hinzugefügt, apps[] in Schema (§3), App-Pipeline in Pipelines (§10), Errorcode-Bereiche erweitert (E4xx, W4xx) |
| 0.5.0 | 2025-12-14 | Phase 1-7 abgeschlossen, Fetch v0.2 (.externals/ Caching) integriert, Test-Pipeline integriert, Blueprint v0.5.0 Format, Referenceen auf Phase 8/9 |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): Struktur aus v1.7 übernommen |
