# Solution.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5, Solution_Schema v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Solution.md](../../en/modules/project/Solution.md)  
> **Module:** [`cmake/project/Solution.cmake`](../../../../cmake/project/Solution.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [Gesetzte Properties](#3-gesetzte-properties)
4. [Verarbeitung](#4-verarbeitung)
5. [Debug-Ausgaben](#5-debug-ausgaben)
6. [Errorbehandlung](#6-fehlerbehandlung)
7. [Usagesbeispiele](#7-verwendungsbeispiele)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

Das `Solution.cmake` Modul ist der **zentrale Configurationslader** des Build-Systems. Es liest die `Solution.json` und setzt globale Properties für alle nachfolgenden Module.

### Kernidee

Eine JSON-Datei definiert die gesamte Projekt-Configuration — Solution.cmake macht diese für CMake verfügbar.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| JSON-Laden | Liest und validiert Solution.json |
| Metadata | Name, Version, Description, Autoren |
| Standards | C/C++ Standard, Extensions |
| Defaults | Library/Executable Typen |
| Externals | Cache-Pfade, Update-Policy |

### Usage durch

- Alle nachfolgenden Module (Executables, Libraries, Externals, Tests)

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| Errors.cmake | 0.5.0 | `cmake_fatal`, `cmake_warn` |
| Debug.cmake | 0.5.0 | `dbg`, `dbg_init`, `enddbgblock` |
| Json.cmake | 0.5.0 | Alle `_json_*` Functions |
| Validation.cmake | 0.5.0 | Schema-Validierung (indirekt) |

**Ladereihenfolge:** Solution.cmake muss nach den Core-Modulen geladen werden.

---

## 3. Gesetzte Properties

### 3.1 Globale Properties

| Property | Typ | Description |
|----------|-----|--------------|
| `SOLUTION_JSON` | String | Kompletter JSON-String |
| `SOLUTION_NAME` | String | Projekt-Name |
| `SOLUTION_VERSION` | String | Projekt-Version (z.B. "1.0.0") |
| `SOLUTION_DESCRIPTION` | String | Projekt-Description |
| `SOLUTION_AUTHORS` | List | Autoren (Semikolon-getrennt) |
| `SOLUTION_SCHEMA_VERSION` | String | Schema-Version (z.B. "0.1") |
| `SOLUTION_SETTINGS_JSON` | String | settings-Block als JSON |
| `SOLUTION_EXTERNALS_JSON` | String | externals-Block als JSON |
| `SOLUTION_LIBRARIES_JSON` | String | libraries-Array als JSON |
| `SOLUTION_EXECUTABLES_JSON` | String | executables-Array als JSON |
| `SOLUTION_CXX_STANDARD` | Int | C++ Standard (z.B. 20) |
| `SOLUTION_C_STANDARD` | Int | C Standard (z.B. 17) |
| `SOLUTION_DEFAULT_LIBRARY_TYPE` | String | Default: "STATIC" |
| `SOLUTION_DEFAULT_EXECUTABLE_TYPE` | String | Default: "CONSOLE" |
| `SOLUTION_SOURCE_MODE` | String | "explicit", "glob", oder "auto" |
| `SOLUTION_EXTERNALS_CACHE_ROOT` | Path | Default: "externals/_cache" |
| `SOLUTION_EXTERNALS_SOURCE_ROOT` | Path | Default: "externals/_src" |
| `SOLUTION_EXTERNALS_UPDATE_POLICY` | String | Default: "checkout" |
| `SOLUTION_EXTERNALS_POLICY_JSON` | String | externalsPolicy-Block als JSON |

### 3.2 CMake Cache-Variablen

| Variable | Wert | Description |
|----------|------|--------------|
| `CMAKE_CXX_STANDARD` | Aus settings.standards | C++ Standard |
| `CMAKE_CXX_STANDARD_REQUIRED` | ON/OFF | Standard erforderlich |
| `CMAKE_CXX_EXTENSIONS` | ON/OFF | Compiler-Extensions |
| `CMAKE_C_STANDARD` | Aus settings.standards | C Standard |
| `CMAKE_C_STANDARD_REQUIRED` | ON/OFF | Standard erforderlich |
| `CMAKE_C_EXTENSIONS` | ON/OFF | Compiler-Extensions |

---

## 4. Verarbeitung

### 4.1 Ablauf

```
Solution.json
    │
    ├── 1. Datei lesen
    │   └── E002 wenn nicht gefunden
    │
    ├── 2. Schema-Version prüfen
    │   ├── E002 wenn fehlt
    │   └── W001 wenn < 0.1
    │
    ├── 3. solution-Block extrahieren
    │   ├── E001 wenn fehlt
    │   ├── name (Required) → SOLUTION_NAME
    │   ├── version → SOLUTION_VERSION
    │   ├── description → SOLUTION_DESCRIPTION
    │   └── authors[] → SOLUTION_AUTHORS
    │
    ├── 4. settings-Block extrahieren
    │   ├── standards → CMAKE_*_STANDARD
    │   ├── defaults → SOLUTION_DEFAULT_*
    │   └── sources.mode → SOLUTION_SOURCE_MODE
    │
    ├── 5. externalsPolicy extrahieren
    │   ├── cacheRoot → SOLUTION_EXTERNALS_CACHE_ROOT
    │   ├── sourceRoot → SOLUTION_EXTERNALS_SOURCE_ROOT
    │   └── updatePolicy → SOLUTION_EXTERNALS_UPDATE_POLICY
    │
    └── 6. Arrays extrahieren
        ├── externals → SOLUTION_EXTERNALS_JSON
        ├── libraries → SOLUTION_LIBRARIES_JSON
        └── executables → SOLUTION_EXECUTABLES_JSON
```

### 4.2 Version-Parsing

Die Version kann als String oder Objekt angegeben werden:

```json
// String-Format
"version": "1.2.3"

// Objekt-Format
"version": {
    "major": 1,
    "minor": 2,
    "patch": 3
}
```

---

## 5. Debug-Ausgaben

### 5.1 Standard-Output (SHOW_LEVEL = 2)

```
-- [Solution] Solution.json loaded
-- [Solution] MySolution v1.0.0
-- [Solution] C++ Standard: 20
-- [Solution] Source mode: explicit
-- [Solution] Externals defined: 8
-- [Solution] Executables: 3, Libraries: 1, Tests: 2
-- -------------------------------------------
```

### 5.2 Verbose-Output (SHOW_LEVEL = 5)

```bash
cmake -B build -DDEBUG_DEFAULT_LEVEL=5
```

```
-- [Solution] Solution.json loaded
-- [Solution] Schema version: 0.1
-- [Solution] MySolution v1.0.0
-- [Solution] Description: Multi-Project Solution
-- [Solution] Authors: Author Name
-- [Solution] C++ Standard: 20
-- [Solution] C Standard: 17
-- [Solution] Default library type: STATIC
-- [Solution] Default executable type: CONSOLE
-- [Solution] Source mode: explicit
-- [Solution] Externals cache: externals/_cache
-- [Solution] Externals source: externals/_src
-- [Solution] Update policy: checkout
-- [Solution] Externals defined: 8
-- [Solution] Executables: 3, Libraries: 1, Tests: 2
-- -------------------------------------------
```

---

## 6. Errorbehandlung

### 6.1 Fatal Errors

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | `solution`-Block fehlt | Block zur JSON hinzufügen |
| E001 | `solution.name` fehlt | Name-Feld hinzufügen |
| E002 | Solution.json nicht gefunden | Datei im Projekt-Root erstellen |
| E002 | `schemaVersion` fehlt | Feld hinzufügen |

### 6.2 Warnings

| Code | Bedingung | Empfehlung |
|------|-----------|------------|
| W001 | schemaVersion < 0.1 | Schema auf 0.1 aktualisieren |

---

## 7. Usagesbeispiele

### 7.1 Minimale Solution.json

```json
{
    "schemaVersion": "0.1",
    "solution": {
        "name": "MyProject"
    }
}
```

### 7.2 Vollständige Solution.json

```json
{
    "schemaVersion": "0.1",
    "solution": {
        "name": "MyProject",
        "version": "1.0.0",
        "description": "My awesome project",
        "authors": ["Developer Name"]
    },
    "settings": {
        "standards": {
            "cxx_standard": "20",
            "cxx_standard_required": true,
            "cxx_extensions": false,
            "c_standard": "17"
        },
        "defaults": {
            "library_type": "STATIC",
            "executable_type": "CONSOLE"
        },
        "sources": {
            "mode": "explicit"
        }
    },
    "externalsPolicy": {
        "cacheRoot": "externals/_cache",
        "sourceRoot": "externals/_src",
        "updatePolicy": "checkout"
    },
    "externals": { },
    "libraries": [ ],
    "executables": [ ],
    "tests": [ ]
}
```

### 7.3 Properties abfragen

```cmake
# In anderen Modulen
get_property(_name GLOBAL PROPERTY SOLUTION_NAME)
get_property(_version GLOBAL PROPERTY SOLUTION_VERSION)
get_property(_cxx GLOBAL PROPERTY SOLUTION_CXX_STANDARD)

message(STATUS "Building ${_name} v${_version} with C++${_cxx}")
```

---

## 8. See Also

- [Solution_Schema.md](../../../reference/Solution_Schema.md) — JSON-Schema Reference
- [Executables.cmake](Executables.md) — Verwendet SOLUTION_EXECUTABLES_JSON
- [Libraries.cmake](Libraries.md) — Verwendet SOLUTION_LIBRARIES_JSON
- [Externals.cmake](Externals.md) — Verwendet SOLUTION_EXTERNALS_JSON
- [ErrorCodes.md](../../../reference/ErrorCodes.md) — E001, E002, W001

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Table of Contents mit Ankern, Kapitel-Nummerierung** |
| 0.1.1 | 2025-12-07 | SOLUTION_LIBRARIES_JSON, SOLUTION_EXECUTABLES_JSON hinzugefügt |
| 0.1.0 | 2025-12-03 | Initial (Clean Start): JSON-Parsing, Global Properties, Debug-Output |
