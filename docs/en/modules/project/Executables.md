# Executables.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Executables.md](../../en/modules/project/Executables.md)  
> **Module:** [`cmake/project/Executables.cmake`](../../../../cmake/project/Executables.cmake)  
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

Das `Executables.cmake` Modul ist der **Orchestrator der Executable-Pipeline**. Es iteriert über alle Executables in Solution.json und delegiert an ExecutableCollect und ExecutableCreate.

### Kernidee

Ein Modul koordiniert den gesamten Prozess — vom JSON-Parsing bis zur Target-Erstellung.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| Iteration | Über executables-Array |
| Filter | skip, BUILD_ONLY, platforms |
| Delegation | An Collect und Create Module |
| Duplikat-Check | Target existiert bereits? |

### Lädt automatisch

- ExecutableCollect.cmake
- ExecutableCreate.cmake

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
Executables.cmake
    │
    ├── 1. Load Sub-Modules
    │   ├── ExecutableCollect.cmake
    │   └── ExecutableCreate.cmake
    │
    ├── 2. Get executables array from SOLUTION_JSON
    │   └── Return early if empty
    │
    └── 3. For each executable:
        │
        ├── 3.1 Extract JSON
        │   └── E001 if no name
        │
        ├── 3.2 Create Context (EXE_0, EXE_1, ...)
        │
        ├── 3.3 _collect_executable() → Fills Context
        │
        ├── 3.4 Check Filters:
        │   ├── skip=true → Continue
        │   ├── BUILD_ONLY → Not in list → Continue
        │   └── platforms → Not matching → Continue
        │
        ├── 3.5 Duplicate Check
        │   └── E102 if target exists
        │
        └── 3.6 _create_executable_target() → CMake Target
```

### 3.2 Context-Benennung

Jedes Executable bekommt einen nummerierten Context:

| Index | Context-Prefix |
|-------|----------------|
| 0 | `EXE_0` |
| 1 | `EXE_1` |
| n | `EXE_n` |

---

## 4. Filter-Mechanismen

### 4.1 skip-Flag

In Solution.json:
```json
{
    "name": "OldTool",
    "skip": true
}
```

Executable wird komplett übersprungen.

### 4.2 BUILD_ONLY

CMake-Aufruf:
```bash
cmake -B build -DBUILD_ONLY="MyApp;OtherApp"
```

Nur die angegebenen Executables werden gebaut. Alle anderen werden übersprungen.

### 4.3 platforms

In Solution.json:
```json
{
    "name": "WinTool",
    "platforms": ["windows"]
}
```

Unterstützte Werte:

| Wert | Bedingung |
|------|-----------|
| `windows` | WIN32 |
| `linux` | CMAKE_SYSTEM_NAME == "Linux" |
| `macos` | APPLE |
| `unix` | UNIX |

Mehrere Plattformen möglich:
```json
"platforms": ["windows", "linux"]
```

---

## 5. Errorbehandlung

### 5.1 Fatal Errors

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | Executable hat kein `name`-Feld | Name hinzufügen |
| E102 | Target existiert bereits | Namen eindeutig machen |

---

## 6. Debug-Ausgaben

### 6.1 Standard-Output

```
-- [Executables] === Executable Pipeline Start ===
-- [Executables] Processing 3 executable(s)...
-- [Executables] --- Processing: MyApp ---
-- [Executables]   Created: MyApp
-- [Executables] --- Processing: OldTool ---
-- [Executables]   SKIP: OldTool (skip=true in Solution.json)
-- [Executables] --- Processing: WinOnly ---
-- [Executables]   SKIP: WinOnly (platform not supported: windows)
-- [Executables] 
-- [Executables] === Executable Pipeline Complete ===
-- -------------------------------------------
```

---

## 7. See Also

- [ExecutableCollect.cmake](ExecutableCollect.md) — JSON zu Context
- [ExecutableCreate.cmake](ExecutableCreate.md) — Context zu Target
- [Solution.cmake](Solution.md) — Liefert SOLUTION_JSON
- [Context.cmake](../core/Context.md) — Context-System
- [ErrorCodes.md](../../../reference/ErrorCodes.md) — E001, E102

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Table of Contents mit Ankern, Kapitel-Nummerierung** |
| 0.1.0 | 2025-12-05 | Initial (Clean Start): Pipeline-Orchestration, Filter (skip, BUILD_ONLY, platforms) |
