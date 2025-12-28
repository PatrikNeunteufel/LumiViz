# phase4.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Path:** `cmake/buildSystemTest/phase4.cmake`  
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

**Phase 4** testet die **Library Pipeline** — das Erstellen von Library-Targets.

| Aspekt | Description |
|--------|--------------|
| **Zweck** | Library Pipeline Validation |
| **Debug-ID** | `PHASE4_TEST` |
| **Dependencies** | Phase 1-2, Libraries.cmake |

---

## 2. Tests

### 2.1 Libraries Array

Prüft, dass `libraries` in Solution.json existiert:

```cmake
_json_has_key("${_solution_json}" "libraries" _has_libraries)
_json_array_length("${_solution_json}" "libraries" _lib_count)
```

### 2.2 BasicLogger Target

| Prüfung | Erwartet |
|---------|----------|
| Target existiert | `TARGET BasicLogger` |
| Target-Typ | `INTERFACE_LIBRARY` |

### 2.3 Include Directories

```cmake
get_target_property(_includes BasicLogger INTERFACE_INCLUDE_DIRECTORIES)
```

Prüft, dass Include-Verzeichnisse gesetzt sind.

### 2.4 Executable-Linking

Prüft, dass `MinimalConsole` gegen `BasicLogger` linkt:

```cmake
get_target_property(_deps MinimalConsole LINK_LIBRARIES)
if("BasicLogger" IN_LIST _deps)
    # OK
endif()
```

---

## 3. Successs-Flag

```cmake
set(PHASE4_TEST_PASSED TRUE CACHE BOOL "Phase 4 Test passed" FORCE)
```

---

## 4. See Also

- [Libraries.md](../modules/project/Libraries.md)
- [LibraryCollect.md](../modules/project/LibraryCollect.md)
- [LibraryCreate.md](../modules/project/LibraryCreate.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Blueprint v0.5.0 Format** |
| 0.1.0 | 2025-12-07 | Initial |
