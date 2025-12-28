# Libraries.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Libraries.md](../../en/modules/project/Libraries.md)  
> **Module:** [`cmake/project/Libraries.cmake`](../../../../cmake/project/Libraries.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [Verarbeitung](#3-verarbeitung)
4. [Filter-Mechanismen](#4-filter-mechanismen)
5. [Errorbehandlung](#5-fehlerbehandlung)
6. [Debug-Ausgaben](#6-debug-ausgaben)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

Das `Libraries.cmake` Modul ist der **Orchestrator der Library-Pipeline**. Es iteriert über alle Libraries in Solution.json und delegiert an LibraryCollect und LibraryCreate.

### Kernidee

Analog zu Executables.cmake — ein Modul koordiniert den gesamten Library-Prozess.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| Iteration | Über libraries-Array |
| Filter | skip, BUILD_ONLY, platform |
| Delegation | An Collect und Create Module |
| Duplikat-Check | Target existiert bereits? |

### Lädt automatisch

- LibraryCollect.cmake
- LibraryCreate.cmake

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| Errors.cmake | 0.5.0 | `cmake_fatal` |
| Debug.cmake | 0.5.0 | `dbg`, `dbg_init`, `dbgspace`, `enddbgblock` |
| Json.cmake | 0.5.0 | `_json_has_key`, `_json_array_length`, `_json_array_get`, `_json_get_string` |
| Context.cmake | 0.5.0 | `ctx_create`, `ctx_get` |
| Solution.cmake | 0.5.0 | `SOLUTION_JSON` Property |

---

## 3. Verarbeitung

### 3.1 Pipeline-Ablauf

```
Libraries.cmake
    │
    ├── 1. Load Sub-Modules
    │   ├── LibraryCollect.cmake
    │   └── LibraryCreate.cmake
    │
    ├── 2. Get libraries array from SOLUTION_JSON
    │   └── Return early if empty
    │
    └── 3. For each library:
        │
        ├── 3.1 Extract JSON
        │   └── E001 if no name
        │
        ├── 3.2 Create Context (LIB_0, LIB_1, ...)
        │
        ├── 3.3 _collect_library() → Fills Context
        │
        ├── 3.4 Check Filters:
        │   ├── skip=true → Continue
        │   ├── BUILD_ONLY → Not in list → Continue
        │   └── platform → Not matching → Continue
        │
        ├── 3.5 Duplicate Check
        │   └── E102 if target exists
        │
        └── 3.6 _create_library_target() → CMake Target
```

### 3.2 Context-Benennung

| Index | Context-Prefix |
|-------|----------------|
| 0 | `LIB_0` |
| 1 | `LIB_1` |
| n | `LIB_n` |

---

## 4. Filter-Mechanismen

### 4.1 skip-Flag

```json
{
    "name": "LegacyLib",
    "skip": true
}
```

### 4.2 BUILD_ONLY

```bash
cmake -B build -DBUILD_ONLY="CoreLib;UtilLib"
```

### 4.3 platform

```json
{
    "name": "WinHelper",
    "platform": "windows"
}
```

**Note:** Libraries verwendet `platform` (Singular), Executables `platforms` (Plural/Array).

---

## 5. Errorbehandlung

### 5.1 Fatal Errors

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | Library hat kein `name`-Feld | Name hinzufügen |
| E102 | Target existiert bereits | Namen eindeutig machen |

---

## 6. Debug-Ausgaben

```
-- [Libraries] === Library Pipeline Start ===
-- [Libraries] Processing 2 library(ies)...
-- [Libraries] --- Processing: CoreLib ---
-- [Libraries]   Created: CoreLib
-- [Libraries] --- Processing: UtilLib ---
-- [Libraries]   Created: UtilLib
-- [Libraries] 
-- [Libraries] === Library Pipeline Complete ===
-- -------------------------------------------
```

---

## 7. See Also

- [LibraryCollect.cmake](LibraryCollect.md) — JSON zu Context
- [LibraryCreate.cmake](LibraryCreate.md) — Context zu Target
- [Executables.cmake](Executables.md) — Analoges Modul für Executables
- [ErrorCodes.md](../../../reference/ErrorCodes.md) — E001, E102

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Table of Contents mit Ankern, Kapitel-Nummerierung** |
| 0.1.0 | 2025-12-07 | Initial (Clean Start): Pipeline-Orchestration |
