# Orchestrator.cmake — External Type Dispatcher

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Orchestrator_cmake.md](../../en/modules/externals/Orchestrator_cmake.md)  
> **Modul:** [cmake/externals/Orchestrator.cmake](../../../cmake/externals/Orchestrator.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Type Detection](#4-type-detection)
5. [Processing Flow](#5-processing-flow)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Debug-Ausgaben](#8-debug-ausgaben)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

`Orchestrator.cmake` ist der zentrale Entry-Point für die Verarbeitung von Externals. Das Modul erkennt den Typ eines Externals anhand der JSON-Felder und dispatcht zur entsprechenden Handler-Funktion.

### Kernfunktionen

- **Type Detection** — Erkennt Local vs. Fetched anhand JSON-Feldern
- **Dispatch** — Delegiert an `_attach_local_external()` oder `_handle_fetched_external()`
- **Options Extraction** — Extrahiert target-spezifische External-Optionen
- **Apply** — Wendet Externals auf CMake-Targets an

### Architektur-Position

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

## 2. Abhängigkeiten

### Benötigte Module

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Fehlerbehandlung (`cmake_fatal`, `cmake_warn`) |
| `Debug.cmake` | Debug-Ausgaben (`dbg`) |
| `Json.cmake` | JSON-Parsing (`_json_get_string`, `_json_has_key`) |
| `Validation.cmake` | Source-Validierung (`validate_external_source`) |

### Auto-geladene Module

| Modul | Zweck |
|-------|-------|
| `Local/Attach.cmake` | Handler für lokale Externals |
| `Fetched/Handler.cmake` | Handler für Git-basierte Externals |

---

## 3. API-Referenz

### 3.1 _orchestrate_external()

Hauptfunktion zur Verarbeitung eines Externals.

```cmake
_orchestrate_external(EXT_NAME EXT_JSON)
```

| Parameter | Typ | Beschreibung |
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

| Parameter | Typ | Beschreibung |
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

| Parameter | Typ | Beschreibung |
|-----------|-----|--------------|
| `TARGET_NAME` | String | CMake-Target, das das External nutzt |
| `EXT_NAME` | String | Name des Externals |
| `EXT_OPTIONS` | JSON | Options-JSON für dieses External |

**Für lokale Externals — setzt Variablen:**

| Variable | Beschreibung |
|----------|--------------|
| `EXTERNAL_NAME` | Name des Externals |
| `EXTERNAL_ROOT` | Root-Pfad des Externals |
| `EXTERNAL_OPTIONS` | JSON-Options-String |
| `EXECUTABLE_NAME` | Target-Name (Legacy) |

**Für Fetched Externals:**

Linkt das registrierte Target via `_link_external_to_target()`.

---

## 4. Type Detection

### Erkennungslogik

| JSON-Feld | Erkannter Typ | Handler |
|-----------|---------------|---------|
| `"path"` | Local | `_attach_local_external()` |
| `"git"` | Fetched | `_handle_fetched_external()` |
| Keines | Fehler E012 | — |
| Beide | Fehler (Validation) | — |

### Beispiel-Definitionen

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

## 6. Verwendungsbeispiele

### Basis-Verwendung (automatisch via ExecutableCreate)

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

> **Hinweis:** Das `include` Feld ist optional. Default: `cmake/externals/Includes/{name}/Include.cmake`

---

## 7. Fehlerbehandlung

### Fehler-Codes

| Code | Fehler | Beschreibung |
|------|--------|--------------|
| E010 | External nicht definiert | External nicht in `externals` Block |
| E012 | Kein Source-Feld | Weder `path` noch `git` vorhanden |
| E213 | Include.cmake fehlt | Lokales External ohne Include.cmake |

### Warnungen

| Code | Warnung | Beschreibung |
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

## 9. Siehe auch

- [Attach_cmake.md](Attach_cmake.md) — Lokale Externals
- [Handler_cmake.md](Handler_cmake.md) — Git Externals
- [Targets_cmake.md](Targets_cmake.md) — Target Registry
- [Externals_cmake.md](../project/Externals_cmake.md) — Entry-Point (Project)

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
