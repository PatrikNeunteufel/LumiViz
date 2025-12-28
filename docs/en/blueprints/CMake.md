# CMake — Standard für CMake-Scripts

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Blueprint v0.5  
> **Target Audience:** Build System Developers  
> **Scope:** Alle .cmake Dateien  
> **Language:** English  
> **German:** [CMake.md](../../en/blueprints/CMake.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Modul-Kategorien](#2-modul-kategorien)
3. [Datei-Header](#3-datei-header)
4. [Namenskonventionen](#4-namenskonventionen)
5. [Funktions-Dokumentation](#5-funktions-dokumentation)
6. [Code-Struktur](#6-code-struktur)
7. [Error-Handling](#7-error-handling)
8. [Versionierung](#8-versionierung)
9. [Best Practices](#9-best-practices)
10. [Review Checklist](#10-review-checkliste)
11. [See Also](#11-siehe-auch)
12. [Changelog](#12-changelog)

---

## 1. Overview

This blueprint defines die **verbindliche Struktur** für alle CMake-Module (.cmake Dateien) im CMake Architecture Projekt.

### Ziele

- Einheitliche Modul-Struktur
- Konsistente Dokumentation im Code
- Klare Namenskonventionen
- Standardisiertes Error-Handling

### Abgrenzung

| Dokument | Fokus |
|----------|-------|
| **CMake Blueprint** | Struktur von .cmake Dateien |
| **CMake Standard** | Allgemeine CMake-Konventionen |
| **ModuleDoc Blueprint** | Struktur der Modul-Dokumentation |

---

## 2. Modul-Kategorien

### 2.1 Kategorien und Pfade

| Kategorie | Pfad | Description |
|-----------|------|--------------|
| **Core** | `cmake/core/` | Grundbausteine (Errors, Debug, Context, ...) |
| **Project** | `cmake/project/` | Pipelines (Executables, Libraries, Tests) |
| **Externals** | `cmake/externals/` | External-Handling (Orchestrator, Fetch, ...) |
| **BuildSystemTest** | `cmake/buildSystemTest/` | Interne Build-System-Tests |

### 2.2 Modul-Typen

| Typ | Description | Examples |
|-----|--------------|-----------|
| **Utility** | Wiederverwendbare Hilfsfunktionen | Errors.cmake, Json.cmake |
| **Pipeline** | Verarbeitet Solution.json | Executables.cmake |
| **Orchestrator** | Koordiniert andere Module | Orchestrator.cmake |
| **Config** | Konfiguriert Targets | Warnings.cmake |
| **Hook** | Anpassung für Externals | PreFetch/*.cmake |
| **Include** | Setup für lokale Externals | externals/*/Include.cmake |

---

## 3. Datei-Header

### 3.1 Required-Header

Jede .cmake Datei beginnt mit diesem Header:

```cmake
# cmake/[pfad]/[ModulName].cmake
# ==============================
# [Kurzbeschreibung des Moduls]
#
# Version: X.Y.Z
# Date:    YYYY-MM-DD
# Status:  [Development | Stable | Deprecated]
# Author:  CMake Architecture Team
#
# Dependencies:
#   - [Abhängigkeit 1]
#   - [Abhängigkeit 2]
#   - None (base module)
#
# Provides:
#   - function_1()
#   - function_2()
#   - MACRO_NAME
#
# Used by:
#   - [Modul das dieses verwendet]
#   - [Weiteres Modul]
```

### 3.2 Header-Felder

| Feld | Required | Description |
|------|---------|--------------|
| Pfad-Kommentar | ✓ | Vollständiger Pfad zum Modul |
| Kurzbeschreibung | ✓ | Eine Zeile, was das Modul macht |
| Version | ✓ | SemVer Format |
| Date | ✓ | ISO 8601 |
| Status | ✓ | Development / Stable / Deprecated |
| Author | ✓ | Team oder Person |
| Dependencies | ✓ | Liste oder "None" |
| Provides | ✓ | Öffentliche API |
| Used by | Optional | Wo wird dieses Modul verwendet? |

### 3.3 Include Guard

Nach dem Header immer:

```cmake
include_guard(GLOBAL)
```

---

## 4. Namenskonventionen

### 4.1 Dateinamen

| Kategorie | Muster | Example |
|-----------|--------|----------|
| Core | `[Name].cmake` | `Context.cmake` |
| Project | `[Name].cmake` | `Executables.cmake` |
| Pipeline | `[Type][Action].cmake` | `ExecutableCollect.cmake` |
| Hook | `[external].cmake` | `imgui.cmake` |
| Include | `Include.cmake` | `externals/bass/Include.cmake` |

### 4.2 Funktionsnamen

| Typ | Muster | Example |
|-----|--------|----------|
| Public | `modul_action()` | `ctx_create()`, `cmake_fatal()` |
| Internal | `_modul_action()` | `_json_parse_object()` |
| Pipeline | `pipeline_action()` | `executables_process()` |

### 4.3 Variablen

| Scope | Muster | Example |
|-------|--------|----------|
| Local | `_snake_case` | `_result`, `_temp_list` |
| Cache | `UPPER_SNAKE_CASE` | `DEBUG_CONTEXT` |
| Global Property | `${PREFIX}_${KEY}` | `EXE_MyApp_NAME` |
| Output | `OUT_VAR` (Parameters) | `ctx_get(... OUT_VAR)` |

---

## 5. Funktions-Dokumentation

### 5.1 Dokumentationsblock-Format

Jede öffentliche Funktion erhält einen Dokumentationsblock:

```cmake
# ============================================================================
# function_name - Kurzbeschreibung
# ============================================================================
#[[
    function_name(PARAM1 PARAM2 [OPTIONAL])
    
    Ausführliche Description, was die Funktion macht.
    
    Parameterss:
        PARAM1   - Mandatory: Description
        PARAM2   - Mandatory: Description
        OPTIONAL - Optional: Description (Default: wert)
    
    Returns:
        Was die Funktion zurückgibt (Variable, Property, etc.)
    
    Example:
        function_name("wert1" "wert2")
        function_name("wert1" "wert2" "optional")
    
    Errors:
        E0xxx - Wenn Bedingung nicht erfüllt
    
    Since: v0.1.0
]]
function(function_name PARAM1 PARAM2)
    # Implementation
endfunction()
```

### 5.2 Trennlinien

Zwischen Functions immer eine visuelle Trennung:

```cmake
# ============================================================================
# function_one - Description
# ============================================================================
function(function_one)
endfunction()

# ============================================================================
# function_two - Description
# ============================================================================
function(function_two)
endfunction()
```

---

## 6. Code-Struktur

### 6.1 Modul-Aufbau

```cmake
# 1. Header (Required)
# cmake/core/Example.cmake
# ========================
# ...

# 2. Include Guard (Required)
include_guard(GLOBAL)

# 3. Cache-Variablen (Optional)
set(EXAMPLE_DEBUG OFF CACHE BOOL "Enable debug output")

# 4. Dependencies (Optional)
include(cmake/core/Errors.cmake)

# 5. Private Helpers (Optional)
function(_example_helper)
endfunction()

# 6. Public API (Required)
function(example_main)
endfunction()
```

### 6.2 Einrückung

- **4 Spaces**, keine Tabs
- Verschachtelte Blöcke einrücken

```cmake
function(example)
    if(CONDITION)
        foreach(item IN LISTS items)
            message(STATUS "${item}")
        endforeach()
    endif()
endfunction()
```

---

## 7. Error-Handling

### 7.1 Error-Functions verwenden

Immer das Errors-Modul verwenden:

```cmake
include(cmake/core/Errors.cmake)

# Fataler Error (bricht ab)
cmake_fatal(E0501 "Executable '${name}' not found")

# Warning (läuft weiter)
cmake_warn(W0201 "Deprecated option '${opt}' used")
```

### 7.2 Error-Code-Kategorien

| Präfix | Kategorie |
|--------|-----------|
| E01xx | Core |
| E02xx | Validation |
| E03xx | Output |
| E04xx | Warnings |
| E05xx | Executable |
| E06xx | Library |
| E07xx | Test |
| E20xx | External |

---

## 8. Versionierung

### 8.1 Semantic Versioning

Module verwenden SemVer:

| Teil | Bedeutung | Wann erhöhen? |
|------|-----------|---------------|
| MAJOR | Breaking Changes | API-Änderung, Parameters-Änderung |
| MINOR | Neue Features | Neue Funktion, neuer Parameters |
| PATCH | Bugfixes | Errorkorrektur, Optimierung |

### 8.2 Version im Header aktuell halten

Bei jeder Änderung:
1. Version im Header erhöhen
2. Date aktualisieren
3. Dokumentation anpassen (falls nötig)

---

## 9. Best Practices

### 9.1 Do's ✓

| Regel | Example |
|-------|----------|
| GLOBAL PROPERTY für Context | `set_property(GLOBAL PROPERTY ...)` |
| Errorbehandlung mit Errors.cmake | `cmake_fatal(E0501 "...")` |
| Dokumentation für jede public Funktion | Siehe Abschnitt 5 |
| Früh validieren | Required-Parameters am Anfang prüfen |
| Aussagekräftige Variablennamen | `_executable_name` statt `_n` |

### 9.2 Don'ts ✗

| Vermeiden | Grund | Stattdessen |
|-----------|-------|-------------|
| `PARENT_SCOPE` für Context | Funktioniert nicht bei >2 Ebenen | GLOBAL PROPERTY |
| Hardcodierte Pfade | Nicht portabel | `CMAKE_CURRENT_*` |
| `message(FATAL_ERROR)` direkt | Inkonsistente Meldungen | `cmake_fatal()` |
| Globale Variablen | Seiteneffekte | Context-Pattern |
| Undokumentierte Functions | Unwartbar | Dokumentationsblock |

---

## 10. Review Checklist

Vor Merge eines CMake-Moduls prüfen:

**Header:**
- [ ] Required-Header vollständig
- [ ] Version aktuell
- [ ] Dependencies korrekt

**Code:**
- [ ] `include_guard(GLOBAL)` vorhanden
- [ ] Namenskonventionen eingehalten
- [ ] Alle public Functions dokumentiert
- [ ] Error-Handling via Errors.cmake

**Qualität:**
- [ ] Keine hardcodierten Pfade
- [ ] Keine PARENT_SCOPE für Context-Daten
- [ ] Tests vorhanden (falls möglich)

---

## 11. See Also

- [Cpp.md](Cpp.md) — C++/C Dateistruktur
- [ModuleDoc.md](ModuleDoc.md) — Dokumentation für Module
- [CMake.md](../standards/CMake_Standard.md) — CMake Coding-Standard
- [ErrorCodes.md](../references/ErrorCodes.md) — Alle Errorcodes

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-13** | **Neu: Table of Contents mit Ankern, Error-Handling-Kategorien, Best Practices erweitert** |
| 0.1.0 | 2025-12-03 | Initial (als CMake_Blueprint) |
