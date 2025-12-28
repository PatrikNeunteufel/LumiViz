# phase5.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Path:** `cmake/buildSystemTest/phase5.cmake`  
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

**Phase 5** testet die **Local Externals Pipeline** — das Registrieren und Anwenden lokaler externer Bibliotheken.

| Aspekt | Description |
|--------|--------------|
| **Zweck** | Local Externals Validation |
| **Debug-ID** | `PHASE5_TEST` |
| **Dependencies** | Phase 1-4, Externals.cmake |

---

## 2. Tests

### 2.1 SOLUTION_EXTERNALS_JSON

Prüft, dass die Externals-JSON-Property existiert:

```cmake
get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)
```

### 2.2 External Definition (bass)

Prüft, dass `bass` im Externals-Block definiert ist:

```cmake
_json_has_key("${_externals_json}" "bass" _has_bass)
```

### 2.3 External Registration

Prüft, dass `bass` registriert wurde:

| Property | Description |
|----------|--------------|
| `EXTERNAL_bass_REGISTERED` | `TRUE` |
| `EXTERNAL_bass_PATH` | Pfad zum External |
| `EXTERNAL_bass_INCLUDE` | Include-Verzeichnis |

### 2.4 External in Target

Prüft, dass `MinimalConsole` die External-Library gelinkt hat:

```cmake
get_target_property(_link_libs MinimalConsole LINK_LIBRARIES)
string(FIND "${_link_libs}" "bass" _bass_pos)
```

### 2.5 External Options

Prüft `external_options` Verarbeitung (z.B. `BASS_FLAC`):

```cmake
_json_get_object_or_empty("${_exe_json}" "external_options" _ext_opts)
_json_has_key("${_ext_opts}" "bass" _has_bass_opts)
```

---

## 3. Successs-Flag

```cmake
set(PHASE5_TEST_PASSED TRUE CACHE BOOL "Phase 5 Test passed" FORCE)
```

---

## 4. See Also

- [Externals.md](../modules/project/Externals.md)
- [Orchestrator.md](../modules/externals/Orchestrator.md)
- [Attach.md](../modules/externals/Local/Attach.md)
- [Local_Externals.md](../reference/Local_Externals.md)

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Blueprint v0.5.0 Format** |
| 0.1.0 | 2025-12-08 | Initial |
