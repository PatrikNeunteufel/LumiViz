# CMakeLists.txt — Reference

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [CMakeLists.md](../../en/reference/CMakeLists.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Struktur](#2-struktur)
3. [Phasen](#3-phasen)
4. [Optionen](#4-optionen)
5. [Modul-Lade-Reihenfolge](#5-modul-lade-reihenfolge)
6. [Konsolen-Ausgaben](#6-konsolen-ausgaben)
7. [Build-System-Tests](#7-build-system-tests)
8. [Verzeichnisstruktur](#8-verzeichnisstruktur)
9. [Best Practices](#9-best-practices)
10. [Troubleshooting](#10-fehlerbehebung)
11. [See Also](#11-siehe-auch)
12. [Changelog](#12-changelog)

---

## 1. Overview

Die `CMakeLists.txt` ist die **Top-Level-Configuration** des modularen CMake Architecture Build-Systems. Sie orchestriert das Laden aller Module und Pipelines.

### Verantwortlichkeiten

| Aufgabe | Description |
|---------|--------------|
| **CMake-Version** | Minimum-Version definieren (3.19+) |
| **Optionen** | Cache-Variablen für Build-Configuration |
| **Core-Module** | In korrekter Reihenfolge laden |
| **Solution.json** | Projekt-Configuration verarbeiten |
| **project()** | CMake-Projekt definieren |
| **Pipelines** | Libraries → Externals → Executables → Tests |
| **Tests** | Optional: Build-System-Tests ausführen |

### Minimale CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.19)

# === Phase 1: Core-Module ===
include(cmake/core/Errors.cmake)
include(cmake/core/Debug.cmake)
include(cmake/core/Json.cmake)
include(cmake/core/Validation.cmake)
include(cmake/core/Context.cmake)
include(cmake/core/SourceCollect.cmake)
include(cmake/core/OutputDirs.cmake)
include(cmake/core/Warnings.cmake)
include(cmake/core/CompilerOptions.cmake)

# === Phase 2: Solution ===
include(cmake/project/Solution.cmake)

# === CMake Project ===
project(${_SOLUTION_NAME} VERSION ${_SOLUTION_VERSION} LANGUAGES CXX)

# === Phase 4: Libraries ===
include(cmake/project/Libraries.cmake)

# === Phase 5: Externals ===
include(cmake/project/Externals.cmake)

# === Phase 3: Executables ===
include(cmake/project/Executables.cmake)

# === Phase 6-7: Tests ===
include(cmake/project/Tests.cmake)
```

---

## 2. Struktur

### 2.1 Abschnitte

| Abschnitt | Description |
|-----------|--------------|
| **Options** | Cache-Variablen für Build-Configuration |
| **Phase 1** | Core-Module laden (9 Module) |
| **Info** | Debug-Ausgaben zur Configuration |
| **Phase 2** | Solution.cmake laden |
| **Project** | CMake `project()` Aufruf |
| **Phase 4** | Libraries (MUSS vor Executables!) |
| **Phase 5** | Externals (Local + Git) |
| **Phase 3** | Executables |
| **Phase 6-7** | Tests |
| **Tests** | Build-System-Tests (optional) |
| **Cleanup** | Temporäre Variablen aufräumen |

### 2.2 Phasen-Nummern

> **Note:** Die Phasen-Nummern entsprechen der **Entwicklungsreihenfolge**, nicht der Ausführungsreihenfolge. Phase 4 (Libraries) muss vor Phase 3 (Executables) ausgeführt werden!

---

## 3. Phasen

### Overview

| Phase | Name | Module | Status |
|-------|------|--------|--------|
| **1** | Core | Errors, Debug, Json, Validation, Context, SourceCollect, OutputDirs, Warnings, CompilerOptions | ✅ |
| **2** | Solution | Solution.cmake | ✅ |
| **3** | Executables | Executables, ExecutableCollect, ExecutableCreate | ✅ |
| **4** | Libraries | Libraries, LibraryCollect, LibraryCreate | ✅ |
| **5** | Externals | Externals, Orchestrator, Fetch, Attach, Handler, HookLoader, Targets | ✅ |
| **6** | Tests | Tests, TestCollect, TestCreate | ✅ |
| **7** | Fetch | (in Phase 5 integriert) | ✅ |
| **8** | AppContainer | (geplant) | ⏳ |
| **9** | System Externals | (geplant) | ⏳ |

### Ausführungsreihenfolge

```
1. Core-Module      (Phase 1)
2. Solution         (Phase 2)
3. project()        
4. Libraries        (Phase 4)  ← VOR Executables!
5. Externals        (Phase 5)
6. Executables      (Phase 3)
7. Tests            (Phase 6-7)
```

---

## 4. Optionen

### 4.1 Cache-Variablen

| Option | Typ | Default | Description |
|--------|-----|---------|--------------|
| `RUN_BUILD_SYSTEM_TESTS` | BOOL | `ON` | Build-System-Tests ausführen |
| `TEST_PHASE` | STRING | `""` | Spezifische Phase(n) testen |
| `DEBUG_MESSAGES` | BOOL | `ON` | Debug-Ausgaben aktivieren |
| `DEBUG_DEFAULT_LEVEL` | STRING | `2` | Debug-Verbosity (1-5) |
| `BUILD_ONLY` | STRING | `""` | Nur bestimmte Targets bauen |

### 4.2 Usage

```bash
# Standard-Build
cmake -B build

# Ohne Build-System-Tests
cmake -B build -DRUN_BUILD_SYSTEM_TESTS=OFF

# Nur Phase 4 testen
cmake -B build -DTEST_PHASE=4

# Verbose Debug (Level 5)
cmake -B build -DDEBUG_DEFAULT_LEVEL=5

# Nur bestimmtes Target
cmake -B build -DBUILD_ONLY="MyApp"

# Qt6 Pfad setzen
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.6.0/gcc_64
```

### 4.3 Debug-Level

| Level | Description |
|-------|--------------|
| 1 | Nur Error |
| 2 | Error + Warningen (Default) |
| 3 | + Info |
| 4 | + Details |
| 5 | + Trace (sehr verbose) |

---

## 5. Modul-Lade-Reihenfolge

### 5.1 Phase 1: Core-Module

Die Reihenfolge ist **kritisch** — spätere Module hängen von früheren ab:

```cmake
# 1. Errorbehandlung (keine Dependencies)
include(cmake/core/Errors.cmake)

# 2. Debug-Ausgaben (nutzt Errors)
include(cmake/core/Debug.cmake)

# 3. JSON-Parsing (nutzt Errors, Debug)
include(cmake/core/Json.cmake)

# 4. Validierung (nutzt Errors, Debug, Json)
include(cmake/core/Validation.cmake)

# 5. Context-System (nutzt alle vorherigen)
include(cmake/core/Context.cmake)

# 6. Source-Sammlung (nutzt Context)
include(cmake/core/SourceCollect.cmake)

# 7. Output-Verzeichnisse (nutzt Context)
include(cmake/core/OutputDirs.cmake)

# 8. Compiler-Warningen (nutzt Context)
include(cmake/core/Warnings.cmake)

# 9. Compiler-Optionen (nutzt Warnings)
include(cmake/core/CompilerOptions.cmake)
```

### 5.2 Phase 2: Solution

```cmake
include(cmake/project/Solution.cmake)
```

Lädt und validiert `Solution.json`, setzt `_SOLUTION_NAME`, `_SOLUTION_VERSION`.

### 5.3 project() Aufruf

```cmake
project(${_SOLUTION_NAME} VERSION ${_SOLUTION_VERSION} LANGUAGES CXX)
```

### 5.4 Phase 4: Libraries

```cmake
include(cmake/project/Libraries.cmake)
```

**MUSS vor Executables geladen werden!** Executables können gegen Libraries linken.

### 5.5 Phase 5: Externals

```cmake
include(cmake/project/Externals.cmake)
```

Lädt den Externals-Orchestrator für Local und Git Externals.

### 5.6 Phase 3: Executables

```cmake
include(cmake/project/Executables.cmake)
```

### 5.7 Phase 6-7: Tests

```cmake
include(cmake/project/Tests.cmake)
```

---

## 6. Konsolen-Ausgaben

### 6.1 Regel

> **Alle Konsolen-Ausgaben erfolgen über das Debug-System (`dbg()`).**  
> Keine direkten `message()`-Aufrufe außer für Error via `cmake_fatal()`.

### 6.2 Debug-IDs

| ID | Modul | Description |
|----|-------|--------------|
| `CMAKE_MAIN` | CMakeLists.txt | Hauptkonfiguration |
| `BUILD_TEST` | Tests | Build-System-Tests |
| `SOLUTION` | Solution.cmake | Solution-Verarbeitung |
| `LIBRARIES` | Libraries.cmake | Library-Pipeline |
| `EXECUTABLES` | Executables.cmake | Executable-Pipeline |
| `EXTERNALS` | Externals.cmake | Externals-Pipeline |
| `TESTS` | Tests.cmake | Test-Pipeline |

### 6.3 Example-Ausgabe

```
-- [CMake] === CMake Architecture ===
-- [CMake] CMake Version: 3.28.0
-- [CMake] Generator: Ninja
-- [CMake] Build Type: Release
-- -------------------------------------------
-- [Solution] Solution.json loaded
-- [Solution] MyProject v1.0.0
-- [Solution] Externals: 4, Libraries: 2, Executables: 3, Tests: 1
-- -------------------------------------------
-- [Libraries] === Library Pipeline Start ===
-- [Libraries] Processing 2 library(ies)...
-- [Libraries]   Created: CoreLib (STATIC)
-- [Libraries]   Created: Utils (STATIC)
-- [Libraries] === Library Pipeline Complete ===
-- -------------------------------------------
-- [Externals] === Externals Pipeline Start ===
-- [Externals] Processing 4 external(s)...
-- [Externals]   Local: doctest, glad
-- [Externals]   Git: glfw (3.4), imgui (v1.91.6)
-- [Externals] === Externals Pipeline Complete ===
-- -------------------------------------------
-- [Executables] === Executable Pipeline Start ===
-- [Executables] Processing 3 executable(s)...
-- [Executables]   Created: MainApp (GUI)
-- [Executables]   Created: CLI (console)
-- [Executables]   Created: Editor (GUI)
-- [Executables] === Executable Pipeline Complete ===
-- -------------------------------------------
-- [Tests] === Test Pipeline Start ===
-- [Tests] Processing 1 test(s)...
-- [Tests]   Created: UnitTests (doctest)
-- [Tests] === Test Pipeline Complete ===
-- -------------------------------------------
-- [CMake] === Configuration Complete ===
```

---

## 7. Build-System-Tests

### 7.1 Phasen-Tests

| Phase | Datei | Testet |
|-------|-------|--------|
| 1 | `phase1.cmake` | Core-Module (Context, JSON, Debug, Errors) |
| 2 | `phase2.cmake` | Solution.cmake (Properties, Settings) |
| 3 | `phase3.cmake` | Executable-Pipeline |
| 4 | `phase4.cmake` | Library-Pipeline |
| 5 | `phase5.cmake` | Externals-Pipeline |
| 6 | `phase6.cmake` | Test-Pipeline |

### 7.2 Test-Flags

Nach erfolgreichem Test wird ein Cache-Flag gesetzt:

```cmake
PHASE1_TEST_PASSED = TRUE
PHASE2_TEST_PASSED = TRUE
PHASE3_TEST_PASSED = TRUE
PHASE4_TEST_PASSED = TRUE
PHASE5_TEST_PASSED = TRUE
PHASE6_TEST_PASSED = TRUE
```

### 7.3 Deaktivieren

```bash
cmake -B build -DRUN_BUILD_SYSTEM_TESTS=OFF
```

### 7.4 Einzelne Phase testen

```bash
cmake -B build -DTEST_PHASE=4
```

---

## 8. Verzeichnisstruktur

```
project/
├── CMakeLists.txt              ← Diese Datei
├── Solution.json               ← Projekt-Configuration
├── cmake/
│   ├── core/                   ← Phase 1: Core-Module
│   │   ├── Errors.cmake
│   │   ├── Debug.cmake
│   │   ├── Json.cmake
│   │   ├── Validation.cmake
│   │   ├── Context.cmake
│   │   ├── SourceCollect.cmake
│   │   ├── OutputDirs.cmake
│   │   ├── Warnings.cmake
│   │   └── CompilerOptions.cmake
│   ├── project/                ← Phase 2-7: Projekt-Module
│   │   ├── Solution.cmake
│   │   ├── Libraries.cmake
│   │   ├── LibraryCollect.cmake
│   │   ├── LibraryCreate.cmake
│   │   ├── Executables.cmake
│   │   ├── ExecutableCollect.cmake
│   │   ├── ExecutableCreate.cmake
│   │   ├── Externals.cmake
│   │   ├── Tests.cmake
│   │   ├── TestCollect.cmake
│   │   └── TestCreate.cmake
│   ├── externals/              ← Externals-System
│   │   ├── Orchestrator.cmake
│   │   ├── core/
│   │   │   └── Fetch.cmake
│   │   ├── Local/
│   │   │   └── Attach.cmake
│   │   ├── Fetched/
│   │   │   └── Handler.cmake
│   │   ├── Hook/
│   │   │   ├── HookLoader.cmake
│   │   │   ├── prefetch/
│   │   │   └── postfetch/
│   │   ├── Registry/
│   │   │   └── Targets.cmake
│   │   └── includes/           ← Include.cmake pro External
│   │       ├── doctest/
│   │       ├── lua54/
│   │       ├── glad/
│   │       └── bass/
│   └── buildSystemTest/        ← Build-System-Tests
│       ├── phase1.cmake
│       ├── phase2.cmake
│       ├── phase3.cmake
│       ├── phase4.cmake
│       ├── phase5.cmake
│       └── phase6.cmake
├── externals/                  ← Local Externals
│   ├── doctest/
│   ├── lua54/
│   ├── glad/
│   └── bass/
└── projects/
    ├── libs/                   ← Libraries
    ├── exec/                   ← Executables
    └── tests/                  ← Tests
```

---

## 9. Best Practices

### 9.1 Keine direkten message()-Aufrufe

```cmake
# ❌ Falsch
message(STATUS "Loading configuration...")

# ✅ Richtig
dbg(${DBG_COMMON} "Loading configuration..." ID CMAKE_MAIN)
```

### 9.2 Error über Errors-Modul

```cmake
# ❌ Falsch
message(FATAL_ERROR "Something went wrong")

# ✅ Richtig
cmake_fatal("E001" "Something went wrong")
```

### 9.3 Libraries vor Executables

```cmake
# ✅ Korrekte Reihenfolge
include(cmake/project/Libraries.cmake)    # ZUERST
include(cmake/project/Externals.cmake)    # DANN
include(cmake/project/Executables.cmake)  # ZULETZT

# ❌ Falsche Reihenfolge - Executables können nicht linken!
include(cmake/project/Executables.cmake)
include(cmake/project/Libraries.cmake)
```

### 9.4 Variablen aufräumen

```cmake
# Am Ende der Datei
unset(_sol_name)
unset(_sol_version)
unset(_temp_var)
```

### 9.5 Guard für mehrfaches Include

```cmake
# In jedem Modul
if(DEFINED _MODULE_NAME_INCLUDED)
    return()
endif()
set(_MODULE_NAME_INCLUDED TRUE)
```

---

## 10. Troubleshooting

### 10.1 Module nicht gefunden

```
CMake Error at CMakeLists.txt:XX (include):
  include could not find requested file: cmake/core/Errors.cmake
```

**Lösung:** Sicherstellen, dass `cmake/core/` Verzeichnis existiert und alle Module vorhanden sind.

### 10.2 Solution.json fehlt

```
[E002] Solution.json not found: /path/to/project/Solution.json
```

**Lösung:** `Solution.json` im Projekt-Root erstellen.

### 10.3 Library nicht gefunden beim Linken

```
[E101] Dependency 'CoreLib' for executable 'MyApp' does not exist
```

**Lösung:** Sicherstellen, dass `Libraries.cmake` **vor** `Executables.cmake` geladen wird.

### 10.4 External nicht gefunden

```
[E201] External 'glfw' not defined in Solution.json
```

**Lösung:** External in `Solution.json` unter `externals` definieren.

### 10.5 Keine Debug-Ausgaben

**Lösung:** Debug-Level erhöhen:
```bash
cmake -B build -DDEBUG_DEFAULT_LEVEL=5
```

### 10.6 Qt6 nicht gefunden

```
Could not find a package configuration file provided by "Qt6"
```

**Lösung:** `CMAKE_PREFIX_PATH` setzen:
```bash
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.6.0/gcc_64
```

---

## 11. See Also

### Core-Module (Phase 1)

- [Errors.md](modules/core/Errors.md)
- [Debug.md](modules/core/Debug.md)
- [Json.md](modules/core/Json.md)
- [Validation.md](modules/core/Validation.md)
- [Context.md](modules/core/Context.md)
- [SourceCollect.md](modules/core/SourceCollect.md)
- [OutputDirs.md](modules/core/OutputDirs.md)
- [Warnings.md](modules/core/Warnings.md)
- [CompilerOptions.md](modules/core/CompilerOptions.md)

### Project-Module

- [Solution.md](modules/project/Solution.md)
- [Libraries.md](modules/project/Libraries.md)
- [Executables.md](modules/project/Executables.md)
- [Externals.md](modules/project/Externals.md)
- [Tests.md](modules/project/Tests.md)

### Referenceen

- [Solution_Schema.md](Solution_Schema.md) — JSON-Schema
- [Externals.md](Externals.md) — External Libraries
- [CMakePresets.md](CMakePresets.md) — Preset-Configuration

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Blueprint v0.5.0 Format; alle 9 Phasen dokumentiert; Externals-Pipeline hinzugefügt; Tests-Pipeline hinzugefügt** |
| 0.1.1 | 2025-12-07 | Libraries.cmake hinzugefügt, SourceCollect.cmake in Core |
| 0.1.0 | 2025-12-05 | Initial: Clean Start mit Debug-System |
