# Libraries.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** In Entwicklung (Pre-Release)  
> **Basiert auf:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Libraries.md](../../en/modules/project/Libraries.md)  
> **Modul:** [`cmake/project/Libraries.cmake`](../../../../cmake/project/Libraries.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Verarbeitung](#3-verarbeitung)
4. [Filter-Mechanismen](#4-filter-mechanismen)
5. [Fehlerbehandlung](#5-fehlerbehandlung)
6. [Debug-Ausgaben](#6-debug-ausgaben)
7. [Siehe auch](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Das `Libraries.cmake` Modul ist der **Orchestrator der Library-Pipeline**. Es iteriert über alle Libraries in Solution.json und delegiert an LibraryCollect und LibraryCreate.

### Kernidee

Analog zu Executables.cmake — ein Modul koordiniert den gesamten Library-Prozess.

### Verantwortlichkeiten

| Bereich | Beschreibung |
|---------|--------------|
| Iteration | Über libraries-Array |
| Filter | skip, BUILD_ONLY, platform |
| Delegation | An Collect und Create Module |
| Duplikat-Check | Target existiert bereits? |

### Lädt automatisch

- LibraryCollect.cmake
- LibraryCreate.cmake

---

## 2. Abhängigkeiten

| Modul | Version | Verwendung |
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

**Hinweis:** Libraries verwendet `platform` (Singular), Executables `platforms` (Plural/Array).

---

## 5. Fehlerbehandlung

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

## 7. Siehe auch

- [LibraryCollect.cmake](LibraryCollect.md) — JSON zu Context
- [LibraryCreate.cmake](LibraryCreate.md) — Context zu Target
- [Executables.cmake](Executables.md) — Analoges Modul für Executables
- [ErrorCodes.md](../../../reference/ErrorCodes.md) — E001, E102

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Inhaltsverzeichnis mit Ankern, Kapitel-Nummerierung** |
| 0.1.0 | 2025-12-07 | Initial (Clean Start): Pipeline-Orchestration |
