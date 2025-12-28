# SourceCollect.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [SourceCollect.md](../../en/modules/core/SourceCollect.md)  
> **Module:** [`cmake/core/SourceCollect.cmake`](../../../../cmake/core/SourceCollect.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [Concept](#3-konzept)
4. [API-Reference](#4-api-referenz)
5. [Source.cmake Format](#5-sourcecmake-format)
6. [Usagesbeispiele](#6-verwendungsbeispiele)
7. [Errorbehandlung](#7-fehlerbehandlung)
8. [Best Practices](#8-best-practices)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

Das `SourceCollect.cmake` Modul verwaltet **Source-Dateien** für Executables, Libraries und Tests. Es unterstützt explizite Deklaration via Source.cmake sowie GLOB als Fallback.

### Kernidee

Explizite Source-Kontrolle als Standard, mit flexiblen Modi für verschiedene Anwendungsfälle.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| Sammlung | Sources, Headers, Templates, Module |
| Modi | explicit, glob, auto |
| Kategorien | Compilable, Headers, Extras, C++20 Modules |
| Include-Pfade | Automatisch + aus Source.cmake |

### Usage durch

- ExecutableCreate.cmake
- LibraryCreate.cmake
- TestCreate.cmake

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| Errors.cmake | 0.1.2 | `cmake_fatal`, `cmake_warn` |
| Debug.cmake | 0.1.1 | `dbg` |

---

## 3. Concept

### 3.1 Source-Modi

| Modus | Description | Empfohlen |
|-------|--------------|-----------|
| `explicit` | Source.cmake erforderlich (Default) | ✅ Yes |
| `glob` | Automatisches Sammeln via Wildcard | ⚠️ Nur für Prototypen |
| `auto` | Source.cmake wenn vorhanden, sonst GLOB | ⚪ Übergang |

> Bei `auto`: Source.cmake wird verwendet wenn vorhanden **und nicht-leer**. Bei leerer Source.cmake → Fallback auf GLOB mit W111 + W110.

### 3.2 Datei-Kategorien

| Kategorie | Variablen-Suffix | Extensions | Description |
|-----------|------------------|------------|--------------|
| Sources | `_SOURCES` | .cpp, .cxx, .cc, .c | Kompilierbare Dateien |
| Headers | `_HEADERS` | .h, .hpp, .hxx, .hh | Deklarationen |
| Templates | `_TEMPLATES` | .tpp, .txx, .ipp | Template-Implementationen |
| Inlines | `_INLINES` | .inl | Inline-Implementationen |
| Impl | `_IMPL` | .impl | PIMPL-Details |
| Modules | `_MODULES` | .ixx, .cppm, .mpp | C++20 Module (experimentell) |

### 3.3 Verarbeitungsfluss

```
collect_sources()
    │
    ├── Mode = explicit?
    │   └── _collect_sources_from_cmake()
    │       └── include(Source.cmake)
    │
    ├── Mode = glob?
    │   └── _collect_sources_glob()
    │       └── file(GLOB_RECURSE ...)
    │
    └── Mode = auto?
        ├── Source.cmake vorhanden? → _collect_sources_from_cmake()
        └── Nicht vorhanden? → _collect_sources_glob()
```

---

## 4. API-Reference

### 4.1 collect_sources()

Haupteinstiegspunkt für Source-Sammlung.

```cmake
collect_sources(<TARGET_NAME> <SOURCE_DIR> 
                <OUT_SOURCES> <OUT_HEADERS> <OUT_EXTRAS> <OUT_MODULES> <OUT_INCLUDES>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `TARGET_NAME` | String | ✓ | Name des Targets |
| `SOURCE_DIR` | Path | ✓ | Source-Verzeichnis |
| `OUT_SOURCES` | Variable | ✓ | Output: Compilable files |
| `OUT_HEADERS` | Variable | ✓ | Output: Header files |
| `OUT_EXTRAS` | Variable | ✓ | Output: Templates + Inlines + Impl |
| `OUT_MODULES` | Variable | ✓ | Output: C++20 Module Units |
| `OUT_INCLUDES` | Variable | ✓ | Output: Include-Pfade |

**Example:**

```cmake
collect_sources(MyApp "${_source_dir}" 
    _sources _headers _extras _modules _includes)
```

---

### 4.2 _collect_sources_from_cmake()

Lädt Source.cmake und sammelt alle definierten Dateien.

```cmake
_collect_sources_from_cmake(<TARGET_NAME> <SOURCE_DIR>
                            <OUT_SOURCES> <OUT_HEADERS> <OUT_EXTRAS> <OUT_MODULES> <OUT_INCLUDES>)
```

**Error:**

| Code | Bedingung |
|------|-----------|
| E104 | Source.cmake nicht gefunden |

**Warningen:**

| Code | Bedingung |
|------|-----------|
| W101 | Keine compilable Files definiert |
| W109 | C++20 Module verwendet |

---

### 4.3 _collect_sources_glob()

GLOB-basierte Sammlung (Fallback).

```cmake
_collect_sources_glob(<SOURCE_DIR>
                      <OUT_SOURCES> <OUT_HEADERS> <OUT_EXTRAS> <OUT_MODULES>)
```

**Warningen:**

| Code | Bedingung |
|------|-----------|
| W101 | Source.cmake defines no files | explicit-Modus |
| W110 | GLOB fallback active | Immer bei GLOB |
| W111 | Source.cmake exists but is empty | auto-Modus mit Pfad |

---

### 4.4 _apply_sources_to_target()

Wendet gesammelte Dateien auf ein Target an.

```cmake
_apply_sources_to_target(<TARGET_NAME>
                         <SOURCES> <HEADERS> <EXTRAS> <MODULES> <INCLUDES> <SOURCE_DIR>)
```

**Automatisch hinzugefügte Include-Pfade:**

1. `SOURCE_DIR` — immer
2. `SOURCE_DIR/pch/` — wenn vorhanden

---

### 4.5 collect_files()

Helper für kontrollierte Wildcards in Source.cmake.

```cmake
collect_files(<OUT_VAR>
    DIRECTORY <path>
    EXTENSIONS <ext1> [ext2...]
    [EXCLUDE <pattern1> [pattern2...]]
)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `OUT_VAR` | Variable | ✓ | Output für Datei-Liste |
| `DIRECTORY` | Path | ✓ | Zu durchsuchendes Verzeichnis |
| `EXTENSIONS` | List | ✓ | Extensions ohne Punkt |
| `EXCLUDE` | List | — | Regex-Pattern zum Ausschließen |

**Error:**

| Code | Bedingung |
|------|-----------|
| E001 | DIRECTORY fehlt |
| E001 | EXTENSIONS fehlt |

**Example:**

```cmake
collect_files(_generated
    DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/generated"
    EXTENSIONS cpp c
    EXCLUDE "*_test.cpp" "test_*.cpp"
)
list(APPEND ${TARGET}_SOURCES ${_generated})
```

---

### 4.6 _get_source_mode()

Ermittelt den Source-Modus aus Solution-Settings.

```cmake
_get_source_mode(<OUT_VAR>)
```

**Rückgabe:** "explicit" (Default), "glob", oder "auto"

**Warningen:**

| Code | Bedingung |
|------|-----------|
| W101 | Ungültiger Modus |

---

## 5. Source.cmake Format

### 5.1 Minimale Source.cmake

```cmake
set(${TARGET}_SOURCES
    main.cpp
)
```

### 5.2 Vollständige Source.cmake

```cmake
# Compilable sources
set(${TARGET}_SOURCES
    main.cpp
    app/Application.cpp
    app/Window.cpp
    utils/FileIO.cpp
)

# Header files
set(${TARGET}_HEADERS
    app/Application.h
    app/Window.h
    utils/FileIO.h
)

# Template implementations
set(${TARGET}_TEMPLATES
    utils/Container.tpp
)

# Inline implementations
set(${TARGET}_INLINES
    utils/FastMath.inl
)

# PIMPL details
set(${TARGET}_IMPL
    app/ApplicationImpl.impl
)

# C++20 modules (experimental)
set(${TARGET}_MODULES
    math.ixx
)

# Additional include paths
set(${TARGET}_INCLUDES
    include
    ../shared/include
)
```

### 5.3 Mit collect_files() für generierte Dateien

```cmake
set(${TARGET}_SOURCES
    main.cpp
    core/Engine.cpp
)

# Generated files
collect_files(_generated
    DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/generated"
    EXTENSIONS cpp
    EXCLUDE "*_test.cpp"
)
list(APPEND ${TARGET}_SOURCES ${_generated})
```

---

## 6. Usagesbeispiele

### 6.1 In ExecutableCreate.cmake

```cmake
function(_create_executable_target CTX)
    ctx_get(${CTX} NAME _name)
    ctx_get(${CTX} PATH _path)
    
    set(_source_dir "${CMAKE_SOURCE_DIR}/${_path}")
    
    # Sources sammeln
    collect_sources(${_name} "${_source_dir}"
        _sources _headers _extras _modules _includes)
    
    # Target erstellen
    add_executable(${_name} ${_sources})
    
    # Sources anwenden
    _apply_sources_to_target(${_name}
        "${_sources}" "${_headers}" "${_extras}" "${_modules}" "${_includes}"
        "${_source_dir}")
endfunction()
```

---

## 7. Errorbehandlung

### 7.1 Fatal Errors

| Code | Description | Lösung |
|------|--------------|--------|
| E104 | Source.cmake nicht gefunden | Source.cmake erstellen oder Modus auf `auto` ändern |
| E001 | Requiredfeld fehlt (collect_files) | DIRECTORY und EXTENSIONS angeben |

### 7.2 Warnings

| Code | Description | Empfehlung |
|------|--------------|------------|
| W101 | Keine Sources definiert | Source.cmake prüfen |
| W109 | C++20 Module verwendet | Experimentell — nur mit CMake 3.28+ |
| W110 | GLOB Fallback aktiv | Auf explizite Source.cmake migrieren |

---

## 8. Best Practices

### 8.1 Modus "explicit" bevorzugen

```cmake
# ✅ Gut - explizite Kontrolle
# In Solution.json:
"settings": {
    "source": {
        "mode": "explicit"
    }
}
```

### 8.2 Source.cmake für jedes Target

```
projects/exec/MyApp/
├── src/
│   ├── Source.cmake    # ← Required bei mode=explicit
│   ├── main.cpp
│   └── ...
```

### 8.3 collect_files() nur für generierte Dateien

```cmake
# ✅ Gut - nur für generierte Dateien
collect_files(_gen DIRECTORY "${_dir}/generated" ...)

# ❌ Schlecht - für normale Sources
collect_files(_all DIRECTORY "${_dir}" ...)  # Besser: Explizit listen!
```

---

## 9. See Also

- [ExecutableCreate.cmake](../project/ExecutableCreate.md) — Verwendet SourceCollect
- [LibraryCreate.cmake](../project/LibraryCreate.md) — Verwendet SourceCollect
- [Solution.cmake](../project/Solution.md) — Setzt SOLUTION_SOURCE_MODE
- [ErrorCodes.md](../../../reference/ErrorCodes.md) — E104, W101, W109, W110

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.1** | **2025-12-17** | **collect_sources() Integration, SourceCollect.cmake Dependency** |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Table of Contents mit Ankern, Kapitel-Nummerierung |
| 0.1.1 | 2025-12-05 | English translation (Language Standards v0.1.1) |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): collect_sources, _collect_sources_from_cmake, _collect_sources_glob, _apply_sources_to_target, collect_files, _get_source_mode |
