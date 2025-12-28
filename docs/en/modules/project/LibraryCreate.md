# LibraryCreate.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [LibraryCreate.md](../../en/modules/project/LibraryCreate.md)  
> **Module:** [`cmake/project/LibraryCreate.cmake`](../../../../cmake/project/LibraryCreate.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Library-Typen](#4-library-typen)
5. [Verarbeitung](#5-verarbeitung)
6. [Errorbehandlung](#6-fehlerbehandlung)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

Das `LibraryCreate.cmake` Modul **erstellt CMake-Library-Targets** aus einem vorbereiteten Context. Es unterstützt STATIC, SHARED und INTERFACE Libraries.

### Kernidee

Analog zu ExecutableCreate — der Context enthält alle Daten, Create erstellt das Target.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| Target-Erstellung | add_library() mit korrektem Typ |
| Source-Sammlung | GLOB oder Source.cmake |
| Public Headers | Include-Directories setzen |
| Dependencies | Interne Libraries, Externals |

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| Context.cmake | 0.5.0 | `ctx_get` |
| Errors.cmake | 0.5.0 | `cmake_fatal`, `cmake_warn` |
| Debug.cmake | 0.5.0 | `dbg` |
| OutputDirs.cmake | 0.5.0 | `setup_output_dirs` |
| Warnings.cmake | 0.5.0 | `apply_warnings` |
| CompilerOptions.cmake | 0.5.0 | `apply_compiler_options` |
| Orchestrator.cmake | 0.7.0 | `apply_external_to_target` |

---

## 3. API-Reference

### 3.1 _create_library_target()

Erstellt ein CMake-Library-Target aus dem Context.

```cmake
_create_library_target(<CTX>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `CTX` | String | ✓ | Context-Prefix (z.B. `LIB_0`) |

**Erwartete Context-Keys:**

| Key | Usage |
|-----|------------|
| NAME | Target-Name |
| PATH | Source-Verzeichnis |
| TYPE | STATIC, SHARED, INTERFACE |
| VERSION | Target-Version |
| PUBLIC_HEADERS | Öffentliche Include-Verzeichnis |
| DEPENDENCIES | Interne Libraries |
| EXTERNALS | Externe Dependencies |
| EXTERNAL_OPTIONS | Per-External Optionen (JSON) |

---

## 4. Library-Typen

### 4.1 STATIC Library

```cmake
add_library(${name} STATIC ${sources})
```

Standard für die meisten Libraries.

### 4.2 SHARED Library

```cmake
add_library(${name} SHARED ${sources})
```

Erzeugt .dll/.so/.dylib.

### 4.3 INTERFACE Library

```cmake
add_library(${name} INTERFACE)
target_include_directories(${name} INTERFACE ${public_headers})
```

Header-Only Library — keine Sources, nur Include-Directories.

---

## 5. Verarbeitung

### 5.1 Ablauf

```
_create_library_target(CTX)
    │
    ├── 1. Context-Daten lesen
    │
    ├── 2. TYPE == INTERFACE?
    │   ├── Yes → add_library(INTERFACE)
    │   │       └── Nur Public Headers setzen
    │   │
    │   └── No → Weiter mit STATIC/SHARED
    │
    ├── 3. Source-Verzeichnis validieren
    │   └── E001 wenn nicht existiert
    │
    ├── 4. Target erstellen (STATIC/SHARED)
    │
    ├── 5. Sources sammeln (GLOB)
    │
    ├── 6. Public Headers setzen
    │   └── PUBLIC Include-Directory
    │
    ├── 7. Interne Dependencies linken
    │
    ├── 8. Externals anwenden (NEU: mit Optionen)
    │   └── apply_external_to_target(name, ext, options)
    │
    ├── 9. Standard-Module anwenden
    │   ├── apply_warnings()
    │   ├── apply_compiler_options()
    │   └── setup_output_dirs()
    │
    └── 10. Version als Property setzen
```

### 5.2 Public Headers

```cmake
target_include_directories(${name}
    PUBLIC "${CMAKE_SOURCE_DIR}/${public_headers}"
    PRIVATE "${src_dir}"
)
```

- **PUBLIC:** Für Consumer der Library sichtbar
- **PRIVATE:** Nur intern verwendet

---

## 6. Errorbehandlung

### 6.1 Fatal Errors

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | Source-Pfad existiert nicht | Pfad prüfen/anlegen |
| E101 | Interne Dependency existiert nicht | Library-Reihenfolge prüfen |

### 6.2 Warnings

| Code | Bedingung | Empfehlung |
|------|-----------|------------|
| W101 | Keine Sources gefunden | Source-Dateien hinzufügen |

---

## 7. See Also

- [Libraries.cmake](Libraries.md) — Ruft _create_library_target auf
- [LibraryCollect.cmake](LibraryCollect.md) — Befüllt den Context
- [ExecutableCreate.cmake](ExecutableCreate.md) — Analoges Modul für Executables

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.6.0** | **2025-12-20** | **EXTERNAL_OPTIONS Support: apply_external_to_target() statt direktem Linking** |
| 0.5.1 | 2025-12-17 | collect_sources() Integration, SourceCollect.cmake Dependency |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0 |
| 0.1.0 | 2025-12-07 | Initial (Clean Start): STATIC/SHARED/INTERFACE Support |
