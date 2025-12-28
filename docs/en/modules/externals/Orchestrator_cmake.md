# Orchestrator.cmake — External Type Dispatcher

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Orchestrator_cmake.md](../../en/modules/externals/Orchestrator_cmake.md)  
> **Module:** [cmake/externals/Orchestrator.cmake](../../../cmake/externals/Orchestrator.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Type Detection](#4-type-detection)
5. [Processing Flow](#5-processing-flow)
6. [Usagesbeispiele](#6-verwendungsbeispiele)
7. [Errorbehandlung](#7-fehlerbehandlung)
8. [Debug-Ausgaben](#8-debug-ausgaben)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

`Orchestrator.cmake` ist der zentrale Entry-Point für die Verarbeitung von Externals. Das Modul erkennt den Typ eines Externals anhand der JSON-Felder und dispatcht zur entsprechenden Handler-Funktion.

### Kernfunktionen

- **Type Detection** — Erkennt Local vs. Fetched anhand JSON-Feldern
- **Dispatch** — Delegiert an `_attach_local_external()` oder `_handle_fetched_external()`
- **Options Extraction** — Extrahiert target-spezifische External-Optionen
- **Apply** — Wendet Externals auf CMake-Targets an

### Architecture-Position

```
Externals.cmake (project)
         │
         ▼
┌─────────────────────┐
│  Orchestrator.cmake │  ← Entry-Point
└──────────┬──────────┘
           │
     ┌─────┴─────┐
     ▼           ▼
┌─────────┐ ┌──────────┐
│ Local/  │ │ Fetched/ │
│ Attach  │ │ Handler  │
└─────────┘ └──────────┘
```

---

## 2. Dependencies

### Benötigte Module

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Errorbehandlung (`cmake_fatal`, `cmake_warn`) |
| `Debug.cmake` | Debug-Ausgaben (`dbg`) |
| `Json.cmake` | JSON-Parsing (`_json_get_string`, `_json_has_key`) |
| `Validation.cmake` | Source-Validierung (`validate_external_source`) |

### Auto-geladene Module

| Modul | Zweck |
|-------|-------|
| `Local/Attach.cmake` | Handler für lokale Externals |
| `Fetched/Handler.cmake` | Handler für Git-basierte Externals |
| `System/Handler.cmake` | Handler für System Externals (find_package) |

---

## 3. API-Reference

### 3.1 _orchestrate_external()

Hauptfunktion zur Verarbeitung eines Externals.

```cmake
_orchestrate_external(EXT_NAME EXT_JSON)
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals (z.B. "bass", "spdlog") |
| `EXT_JSON` | JSON | JSON-Definition aus Solution.json |

**Verhalten:**
1. Validiert Source-Feld (genau eines von `path` oder `git`)
2. Erkennt Typ anhand vorhandenem Feld
3. Dispatcht zu entsprechendem Handler

---

### 3.2 _get_external_options_for_target()

Extrahiert target-spezifische Optionen für ein External.

```cmake
_get_external_options_for_target(TARGET_NAME EXT_NAME TARGET_JSON OUT_VAR)
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `TARGET_NAME` | String | Name des Targets (für Debug) |
| `EXT_NAME` | String | Name des Externals |
| `TARGET_JSON` | JSON | JSON des Targets (Executable/Library) |
| `OUT_VAR` | Variable | Ausgabe: Options-JSON oder `{}` |

---

### 3.3 apply_external_to_target()

Wendet ein External auf ein CMake-Target an.

```cmake
apply_external_to_target(TARGET_NAME EXT_NAME EXT_OPTIONS)
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `TARGET_NAME` | String | CMake-Target, das das External nutzt |
| `EXT_NAME` | String | Name des Externals |
| `EXT_OPTIONS` | JSON | Options-JSON für dieses External |

**Verhalten:**
1. Prüft ob External in `SKIPPED_EXTERNALS` → **E013 FATAL**
2. Holt External-Definition aus `SOLUTION_EXTERNALS_JSON`
3. Erkennt Typ (system/git/path)
4. Wendet typ-spezifisch an

**Für lokale Externals — setzt Variablen:**

| Variable | Description |
|----------|--------------|
| `EXTERNAL_NAME` | Name des Externals |
| `EXTERNAL_ROOT` | Root-Pfad des Externals |
| `EXTERNAL_OPTIONS` | JSON-Options-String |
| `EXECUTABLE_NAME` | Target-Name (Legacy) |

**Für Fetched Externals:**

Linkt das registrierte Target via `_link_external_to_target()`.

---

## 4. Type Detection

### Erkennungslogik (Priorität)

| JSON-Feld | Erkannter Typ | Handler |
|-----------|---------------|---------|
| `"system": true` | System | `_handle_system_external()` |
| `"git"` | Fetched | `_handle_fetched_external()` |
| `"path"` | Local | `_attach_local_external()` |
| Keines | Error E012 | — |

> **Priorität:** system → git → path

### Example-Definitionen

**System:**
```json
{
    "qt6": {
        "system": true,
        "package": "Qt6",
        "components": ["Core", "Widgets"]
    }
}
```

**Lokal:**
```json
{
    "bass": {
        "path": "externals/bass",
        "version": "2.4.17"
    }
}
```

**Fetched:**
```json
{
    "spdlog": {
        "git": "https://github.com/gabime/spdlog.git",
        "tag": "v1.12.0"
    }
}
```

---

## 5. Processing Flow

### Orchestration Flow

```
┌─────────────────────────────────────────────────────────┐
│              _orchestrate_external()                     │
├─────────────────────────────────────────────────────────┤
│  1. validate_external_source()                          │
│     └─ E012 wenn kein path/git                          │
│                                                         │
│  2. _json_has_key("path") → _is_local                   │
│     _json_has_key("git")  → _is_fetched                 │
│                                                         │
│  3. Dispatch:                                           │
│     ├─ _is_local  → _attach_local_external()            │
│     └─ _is_fetched → _handle_fetched_external()         │
└─────────────────────────────────────────────────────────┘
```

---

## 6. Usagesbeispiele

### Basis-Usage (automatisch via ExecutableCreate)

```cmake
# In ExecutableCreate.cmake - automatisch aufgerufen
foreach(_ext IN LISTS _externals)
    _get_external_options_for_target("${_name}" "${_ext}" "${_exe_json}" _ext_opts)
    apply_external_to_target("${_name}" "${_ext}" "${_ext_opts}")
endforeach()
```

### Custom Include Path (optional)

```json
{
    "bass": {
        "path": "externals/bass",
        "include": "cmake/custom/bass_special.cmake"
    }
}
```

> **Note:** Das `include` Feld ist optional. Default: `cmake/externals/Includes/{name}/Include.cmake`

---

## 7. Errorbehandlung

### Error-Codes

| Code | Error | Description |
|------|--------|--------------|
| E010 | External nicht definiert | External nicht in `externals` Block |
| E012 | Kein Source-Feld | Weder `path` noch `git` noch `system` vorhanden |
| E013 | Geskipptes External verwendet | Target referenziert External mit `skip: true` |
| E213 | Include.cmake fehlt | Lokales External ohne Include.cmake |

### Skip-Validierung

`apply_external_to_target()` prüft vor der Anwendung, ob das External geskippt ist:

```cmake
get_property(_skipped_list GLOBAL PROPERTY SKIPPED_EXTERNALS)
if("${EXT_NAME}" IN_LIST _skipped_list)
    cmake_fatal("E013" "External '${EXT_NAME}' is skipped but used by target '${TARGET_NAME}'...")
endif()
```

### Warningen

| Code | Warning | Description |
|------|---------|--------------|
| W101 | External nicht ready | Fetched External noch nicht verfügbar |

---

## 8. Debug-Ausgaben

### Debug-ID: `EXTERNALS`

| Level | Ausgabe |
|-------|---------|
| `DBG_RARE` | Type: LOCAL / FETCHED |
| `DBG_RARE` | Applying {ext} to {target} |
| `DBG_ULTRA_RARE` | Options für External |

### Aktivierung

```cmake
cmake -DDEBUG_CATEGORIES="EXTERNALS" ..
```

---

## 9. See Also

- [Attach_cmake.md](Attach_cmake.md) — Lokale Externals
- [Handler_cmake.md](Handler_cmake.md) — Git Externals
- [Targets_cmake.md](Targets_cmake.md) — Target Registry
- [Externals_cmake.md](../project/Externals_cmake.md) — Entry-Point (Project)

---

## 10. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.7.0** | **2025-12-20** | **Skip-Validierung: E013 bei Usage geskippter Externals in apply_external_to_target()** |
| 0.6.0 | 2025-12-18 | Phase 9: System Externals Support, `_handle_system_external()` |
| 0.5.0 | 2025-12-15 | Dokumentation auf Blueprint v0.5.0 migriert |
