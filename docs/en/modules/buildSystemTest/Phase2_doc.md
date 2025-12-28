# phase2.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Path:** `cmake/buildSystemTest/phase2.cmake`  
> **Status:** Stable  
> **Language:** English  

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Tests](#2-tests)
3. [Successs-Flag](#3-erfolgs-flag)
4. [See Also](#4-siehe-auch)
5. [Changelog](#5-changelog)

---

## 1. Overview

**Phase 2** testet die **Solution.json Verarbeitung** — das Laden und Validieren der Projektkonfiguration.

| Aspekt | Description |
|--------|--------------|
| **Zweck** | Solution.json Validation |
| **Debug-ID** | `PHASE2_TEST` |
| **Dependencies** | Phase 1, Solution.cmake |

---

## 2. Tests

### 2.1 Solution Properties

| Property | Required |
|----------|---------|
| `SOLUTION_NAME` | ✅ |
| `SOLUTION_VERSION` | ✅ |
| `SOLUTION_SCHEMA_VERSION` | ✅ |
| `SOLUTION_DESCRIPTION` | ❌ |
| `SOLUTION_AUTHORS` | ❌ |

### 2.2 Settings Properties

| Property | Required | Default |
|----------|---------|---------|
| `SOLUTION_CXX_STANDARD` | ✅ | 17 |
| `SOLUTION_C_STANDARD` | ❌ | 11 |
| `SOLUTION_DEFAULT_LIBRARY_TYPE` | ✅ | STATIC |
| `SOLUTION_DEFAULT_EXECUTABLE_TYPE` | ✅ | CONSOLE |
| `SOLUTION_SOURCE_MODE` | ✅ | auto |

### 2.3 CMake Cache Variables

| Variable | Prüft |
|----------|-------|
| `CMAKE_CXX_STANDARD` | Gesetzt aus Solution |
| `CMAKE_C_STANDARD` | Gesetzt aus Solution |
| `CMAKE_CXX_STANDARD_REQUIRED` | TRUE |
| `CMAKE_CXX_EXTENSIONS` | OFF |

### 2.4 Externals Policy

| Property | Required |
|----------|---------|
| `SOLUTION_EXTERNALS_CACHE_ROOT` | ✅ |
| `SOLUTION_EXTERNALS_SOURCE_ROOT` | ❌ |
| `SOLUTION_EXTERNALS_UPDATE_POLICY` | ✅ |

### 2.5 Externals JSON

Prüft, dass `SOLUTION_EXTERNALS_JSON` vorhanden und nicht leer ist.

### 2.6 project() Kompatibilität

| Prüfung | Erwartet |
|---------|----------|
| `PROJECT_NAME` | = `SOLUTION_NAME` |
| `PROJECT_VERSION` | = `SOLUTION_VERSION` |

---

## 3. Successs-Flag

```cmake
set(PHASE2_TEST_PASSED TRUE CACHE BOOL "Phase 2 Test passed" FORCE)
```

---

## 4. See Also

- [Solution.md](../modules/project/Solution.md)
- [Solution_Schema.md](../reference/Solution_Schema.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Blueprint v0.5.0 Format** |
| 0.1.0 | 2025-12-05 | Initial |
