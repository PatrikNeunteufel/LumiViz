# Guidelines — CMake Build-System

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Standard  
> **Status:** Stable  
> **Target Audience:** Build System Developers  
> **Scope:** Alle CMake-Module in `cmake/`  
> **Durchsetzung:** Code Review, Build-System-Tests  
> **Language:** English  
> **German:** [guidelines.md](../../en/projects/buildsystem/standards/Guidelines.md)

Dieses Dokument enthält Coding-Konventionen, Stil-Entscheidungen und Best Practices für die Implementation des CMake Build-Systems.

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Datei-Struktur](#2-datei-struktur)
3. [Namenskonventionen](#3-namenskonventionen)
4. [Cache-Variablen](#4-cache-variablen)
5. [Error-Handling](#5-error-handling)
6. [Pipelines](#6-pipelines)
7. [Local Externals](#7-local-externals)
8. [Hook-System](#8-hook-system)
9. [Source.cmake](#9-sourcecmake)
10. [Debug-System](#10-debug-system)
11. [Code-Qualität](#11-code-qualität)
12. [Compiler-Overrides](#12-compiler-overrides)
13. [Checkliste](#13-checkliste)
14. [See Also](#14-siehe-auch)
15. [Changelog](#15-changelog)

---

## 1. Overview

### Ziele

- Einheitlicher Code-Stil für alle CMake-Module
- Konsistente Namensgebung
- Klare Errorbehandlung
- Testbare Module

### Scope

| Verzeichnis | Betroffen |
|-------------|-----------|
| `cmake/core/` | ✅ |
| `cmake/project/` | ✅ |
| `cmake/externals/` | ✅ |
| `cmake/buildSystemTest/` | ✅ |
| `externals/*/Include.cmake` | ✅ |

---

## 2. Datei-Struktur

### 2.1 Standard-Header

**Regel:** Jede `.cmake`-Datei beginnt mit dem Standard-Header.

**Verbindlichkeit:** MUSS

```cmake
# ==============================================================================
# cmake/pfad/Dateiname.cmake — Kurze Description
# ==============================================================================
#
# Version:      X.Y.Z
# Date:        YYYY-MM-DD
# Status:       [In Development | Stable | Deprecated]
# Dependencies: [Liste oder "Keine"]
#
# ==============================================================================

include_guard(GLOBAL)
```

### 2.2 Include Guard

**Regel:** Alle Module verwenden `include_guard(GLOBAL)`.

**Verbindlichkeit:** MUSS

**Begründung:** `GLOBAL` statt `DIRECTORY`, weil Module aus verschiedenen Verzeichnissen geladen werden können.

---

## 3. Namenskonventionen

### 3.1 Functions

| Typ | Prefix | Examples | Verbindlichkeit |
|-----|--------|-----------|-----------------|
| Öffentliche API | `cmake_` | `cmake_fatal()`, `cmake_warn()` | MUSS |
| Context-API | `ctx_` | `ctx_create()`, `ctx_set()`, `ctx_get()` | MUSS |
| JSON-Helper | `_json_` | `_json_get_string()`, `_json_has_key()` | MUSS |
| Source-Helper | `_collect_` | `_collect_sources_from_cmake()` | SOLL |
| Interne/Private | `_` | `_helper()`, `_process_item()` | MUSS |
| Validierung | `validate_` | `validate_external_source()` | SOLL |
| Hook-Loader | `load_` | `load_prefetch_hook()` | SOLL |

### 3.2 Variablen

| Typ | Konvention | Examples | Verbindlichkeit |
|-----|------------|-----------|-----------------|
| Lokale Variable | `_snake_case` | `_name`, `_ext_list` | MUSS |
| Modul-Konstante | `_UPPER_SNAKE` | `_DEFAULT_VALUE` | SOLL |
| Output-Parameters | `OUT_VAR` | `function(foo OUT_VAR)` | MUSS |
| Parsed Arguments | `ARG_*` | `ARG_TYPE`, `ARG_FLAG` | MUSS |
| Globale Properties | `UPPER_SNAKE_CASE` | `SOLUTION_JSON` | MUSS |
| Context-Keys | `UPPER_SNAKE_CASE` | `ctx_set(EXE NAME)` | MUSS |
| Cache-Variablen | `UPPER_SNAKE_CASE` | `BUILD_TESTS` | MUSS |

### 3.3 Errorcodes

| Bereich | Prefix | Description |
|---------|--------|--------------|
| JSON/Parsing | `E0xx` | E001, E002, E010, E012 |
| Target-Erstellung | `E1xx` | E101, E102, E103, E104 |
| Externals | `E2xx` | E201, E213-E218 |
| Tests | `E3xx` | E301, E302 |
| AppContainer | `E4xx` | E401-E407 |
| System-Externals | `E5xx` | E501-E505 |
| Deprecation | `W0xx` | W001, W002 |
| Configuration | `W1xx` | W101-W110 |
| Tools/Setup | `W2xx` | W201 |
| External-Caching | `W3xx` | W301, W302 |
| AppContainer | `W4xx` | W401-W403 |
| System-Externals | `W5xx` | W501, W502 |

---

## 4. Cache-Variablen

### 4.1 Build Control

| Variable | Typ | Default | Description |
|----------|-----|---------|--------------|
| `BUILD_TESTS` | BOOL | ON | Projekt-Tests aktivieren |
| `BUILD_ONLY` | STRING | "" | Nur bestimmte Targets |
| `RUN_BUILD_SYSTEM_TESTS` | BOOL | OFF | CMake-Modul-Tests |

### 4.2 Externals

| Variable | Typ | Default | Description |
|----------|-----|---------|--------------|
| `EXTERNALS_OFFLINE` | BOOL | OFF | Nur Cache, kein Netzwerk |
| `EXTERNALS_FORCE_FETCH` | BOOL | OFF | Cache ignorieren |

### 4.3 Code-Qualität

| Variable | Typ | Default | Description |
|----------|-----|---------|--------------|
| `ENABLE_CLANG_TIDY` | BOOL | OFF | Clang-Tidy aktivieren |
| `CLANG_TIDY_STRICT` | BOOL | OFF | Warnings als Errors |
| `ENABLE_CLANG_FORMAT_CHECK` | BOOL | OFF | Format-Checks |

### 4.4 Compiler-Optionen

| Variable | Typ | Default | Description |
|----------|-----|---------|--------------|
| `ENABLE_STRICT_CONFORMANCE` | BOOL | ON | MSVC strict mode |
| `NO_EXCEPTIONS` | BOOL | OFF | Exceptions deaktivieren |
| `NO_RTTI` | BOOL | OFF | RTTI deaktivieren |

---

## 5. Error-Handling

### 5.1 Functions

**Regel:** Einheitliches Error-Handling über `Errors.cmake`.

**Verbindlichkeit:** MUSS

```cmake
# Fataler Error - bricht Build ab
cmake_fatal("E001" "Description mit ${variable}")

# Warning - Build läuft weiter
cmake_warn("W001" "Description mit ${variable}")

# Assertion - für interne Prüfungen
cmake_assert(DEFINED _variable "Variable muss definiert sein")

# Feld-Validierung
cmake_require_field(CTX "name" "Executable")
```

### 5.2 Error-Format

**Regel:** Errormeldungen folgen diesem Format.

**Verbindlichkeit:** MUSS

```
[E101] Dependency 'CoreLib' für 'MyApp' existiert nicht
 ^      ^                                ^
 |      |                                |
 Code   Description                     Context
```

---

## 6. Pipelines

### 6.1 Executable-Pipeline

```
1. ExecutableCollect    → JSON → Context
2. Validation           → Requiredfelder prüfen
3. ExecutableCreate     → add_executable()
4. SourceCollect        → Sources sammeln
5. Dependencies         → Interne Libs linken
6. Externals            → Externe Libs linken
7. CompilerOptions      → Flags setzen
8. Warnings             → Warning-Level
9. OutputDirs           → Zielverzeichnisse
```

### 6.2 Library-Pipeline

Analog zu Executables, zusätzlich:
- PUBLIC/PRIVATE Headers
- STATIC/SHARED/INTERFACE Typen

### 6.3 Test-Pipeline

```
1. TestCollect          → JSON → Context
2. Validation           → Requiredfelder prüfen
3. TestCreate           → add_executable()
4. Framework-Setup      → doctest/googletest/catch2
5. CTest-Registration   → add_test()
6. Labels/Timeout       → Properties setzen
```

### 6.4 App-Container-Pipeline (Phase 8)

```
1. AppCollect           → JSON → Context
2. Validation           → Requiredfelder prüfen
3. AppCreate:
   a. Core Library      → add_library(STATIC)
   b. Runner Executable → add_executable()
   c. App Tests         → Test-Targets
```

---

## 7. Local Externals

### 7.1 Include.cmake MUSS

**Verbindlichkeit:** MUSS

```cmake
# ✅ Libraries linken
target_link_libraries(${EXECUTABLE_NAME} PRIVATE bass)

# ✅ Include-Verzeichnisse
target_include_directories(${EXECUTABLE_NAME} PRIVATE ...)

# ✅ Compile-Definitions
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE ...)

# ✅ DLLs kopieren (Windows)
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD ...)
```

### 7.2 Include.cmake DARF NICHT

**Verbindlichkeit:** DARF NICHT

```cmake
# ❌ KEINE Executables erstellen
add_executable(bass_example ...)

# ❌ KEINE Example-Subdirectories
add_subdirectory(examples)
add_subdirectory(tests)

# ❌ KEINE globalen Cache-Variablen
set(GLOBAL_VAR "value" CACHE INTERNAL "")
```

**Begründung:** IDE Clutter durch unerwünschte Targets.

---

## 8. Hook-System

### 8.1 Wann Hooks verwenden?

| Situation | Hook nötig? |
|-----------|-------------|
| Standard CMake-Projekt | ❌ No |
| CMake-Variablen VOR Fetch | ✅ PreFetch |
| Kein CMakeLists.txt | ✅ PostFetch |
| Patches nötig | ✅ PostFetch |
| Lokales External | ❌ Include.cmake |

### 8.2 Hook-Struktur

**Regel:** Hooks verwenden `include_guard(GLOBAL)`.

**Verbindlichkeit:** MUSS

```cmake
# cmake/externals/Hooks/PostFetch/imgui.cmake

include_guard(GLOBAL)  # Important für shared Hooks!

message(STATUS "[ImGui PostFetch] Creating target...")

FetchContent_GetProperties(imgui)
if(imgui_POPULATED)
    if(NOT TARGET imgui)
        add_library(imgui STATIC ...)
    endif()
endif()
```

---

## 9. Source.cmake

### 9.1 Explizite Listen bevorzugen

**Verbindlichkeit:** SOLL

```cmake
# ✅ Empfohlen: Explizite Auflistung
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/app.cpp"
)
```

### 9.2 GLOB nur für generierte Dateien

**Verbindlichkeit:** KANN

```cmake
# ✅ OK: GLOB mit Excludes für generierte Dateien
collect_files(_generated
    DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/generated"
    EXTENSIONS cpp
    EXCLUDE "*_test.cpp"
)
```

### 9.3 Hierarchische Struktur

**Verbindlichkeit:** SOLL

```cmake
# Hauptverzeichnis
list(APPEND ${TARGET_NAME}_SOURCES ${_local_sources})

# Unterverzeichnisse einbinden
include("${CMAKE_CURRENT_LIST_DIR}/core/Source.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/ui/Source.cmake")
```

---

## 10. Debug-System

### 10.1 Usage

**Verbindlichkeit:** SOLL

```cmake
# Am Anfang des Moduls
set(_SHOW_DEBUG OFF)
dbg_init(ID MY_DBG LEVEL ${DBG_SHOW_MUCH} SWITCH ${_SHOW_DEBUG} TAG "MODUL")

# Im Code
dbg(${DBG_OFTEN} "Phase Start" ID MY_DBG)
dbg(${DBG_COMMON} "Processing ${_name}" ID MY_DBG)
dbg(${DBG_RARE} "Details: ${_val}" ID MY_DBG)

# Am Ende
enddbgblock(ID MY_DBG)
```

### 10.2 Debug-Level

| Level | Verwenden für |
|-------|---------------|
| `DBG_OFTEN` | Phasen-Start/Ende |
| `DBG_COMMON` | Features, Dateien |
| `DBG_NORMAL` | Zwischenschritte |
| `DBG_RARE` | Details, Pfade |
| `DBG_ULTRA_RARE` | Loop-Iterationen |

---

## 11. Code-Qualität

### 11.1 Clang-Tidy & Clang-Format

| Was | Wo |
|-----|----|
| `.clang-format` | Root-Verzeichnis |
| `.clang-tidy` | Root-Verzeichnis |
| Enable/Disable | CMake Cache-Variable |

### 11.2 Integration

```cmake
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        set_target_properties(${TARGET} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--config-file=..."
        )
    endif()
endif()
```

---

## 12. Compiler-Overrides

**Regel:** Per-Target Overrides für spezielle Situationen.

**Verbindlichkeit:** KANN

```cmake
# Alter Code: Strict mode überspringen
apply_compiler_options(OldModule SKIP_STRICT_CONFORMANCE)

# Win32 API: min/max Makros erlauben
apply_compiler_options(Win32Target SKIP_NOMINMAX)

# Externe Lib: Exceptions erzwingen
apply_compiler_options(ExternalLib FORCE_EXCEPTIONS)

# Tests: Clang-Tidy überspringen
apply_compiler_options(TestTarget SKIP_CLANG_TIDY)
```

---

## 13. Checkliste

Vor Code Review prüfen:

**Datei-Struktur:**
- [ ] Standard-Header vorhanden
- [ ] `include_guard(GLOBAL)` vorhanden
- [ ] Dependencies dokumentiert

**Namenskonventionen:**
- [ ] Funktions-Prefixe korrekt (`cmake_`, `ctx_`, `_json_`, `_`)
- [ ] Variablen-Konventionen eingehalten
- [ ] Errorcodes im richtigen Bereich

**Error-Handling:**
- [ ] `cmake_fatal()` für fatale Error
- [ ] `cmake_warn()` für Warningen
- [ ] Errorformat eingehalten

**Best Practices:**
- [ ] Keine globalen Variablen
- [ ] Context-Pattern verwendet
- [ ] Debug-Ausgaben mit `dbg()`

---

## 14. See Also

- [master_concept.md](../concepts/master_concept.md) — Architecture
- [implementation_plan.md](../concepts/implementation_plan.md) — Phasen-Plan
- [CMake_Standard.md](../../../standards/CMake_Standard.md) — Allgemeiner CMake-Stil
- [ErrorCodes](../../../references/ErrorCodes.md) — Errorcodes-Reference

---

## 15. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 (Standard), Errorcodes erweitert (E3xx-E5xx, W3xx-W5xx), Externals-Caching Variablen, App-Container-Pipeline, Checkliste hinzugefügt** |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): Inhalte aus v1.7 übernommen |
