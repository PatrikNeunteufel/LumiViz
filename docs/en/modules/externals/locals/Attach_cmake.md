# Local/Attach.cmake — Lokale External Handler

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Attach_cmake.md](../../en/modules/externals/Attach_cmake.md)  
> **Module:** [cmake/externals/Local/Attach.cmake](../../../cmake/externals/Local/Attach.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Include.cmake Konvention](#4-includecmake-konvention)
5. [Usagesbeispiele](#5-verwendungsbeispiele)
6. [Errorbehandlung](#6-fehlerbehandlung)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

`Local/Attach.cmake` verarbeitet lokale Externals — Libraries, die manuell im Projekt abgelegt werden (z.B. proprietäre Libraries wie BASS).

### Kernfunktionen

- **Path-Validierung** — Prüft ob External-Verzeichnis existiert
- **Include.cmake Lookup** — Convention oder Custom Path
- **Best-Practice-Checks** — Warningen für problematische Patterns
- **Registrierung** — Speichert Pfade für spätere Usage

---

## 2. Dependencies

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Errorbehandlung (`cmake_fatal`, `cmake_warn`) |
| `Debug.cmake` | Debug-Ausgaben (`dbg`) |
| `Json.cmake` | JSON-Parsing |
| `Validation.cmake` | Source-Validierung |

---

## 3. API-Reference

### 3.1 _attach_local_external()

```cmake
_attach_local_external(EXT_NAME EXT_JSON)
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals |
| `EXT_JSON` | JSON | JSON-Definition mit `path` Feld |

**Setzt Properties:**

| Property | Description |
|----------|--------------|
| `EXTERNAL_${NAME}_PATH` | Absoluter Pfad zum External |
| `EXTERNAL_${NAME}_INCLUDE` | Pfad zur Include.cmake |
| `EXTERNAL_${NAME}_REGISTERED` | TRUE wenn registriert |

---

### 3.2 is_external_registered()

```cmake
is_external_registered(EXT_NAME OUT_VAR)
```

Prüft ob ein External registriert ist.

---

## 4. Include.cmake Konvention

### Convention Path (Default)

```
cmake/externals/includes/{name}/Include.cmake
```

**Example:** External `bass` → `cmake/externals/includes/bass/Include.cmake`

### Custom Include Path (optional)

```json
{
    "bass": {
        "path": "externals/bass",
        "include": "cmake/custom/bass_special.cmake"
    }
}
```

> **Note:** Das `include` Feld ist optional. Ohne Angabe wird der Convention Path verwendet.

---

## 5. Usagesbeispiele

### Minimal (Convention)

```json
{
    "bass": {
        "path": "externals/bass"
    }
}
```

→ Sucht automatisch: `cmake/externals/includes/bass/Include.cmake`

### Mit Version

```json
{
    "lua54": {
        "path": "externals/lua54",
        "version": "5.4.6"
    }
}
```

→ Sucht: `cmake/externals/includes/lua54/Include.cmake`

---

## 6. Errorbehandlung

| Code | Error | Description |
|------|--------|--------------|
| E001 | Path leer | `path` Feld ist leer |
| E213 | Include.cmake fehlt | Include.cmake nicht gefunden |
| E214 | Pfad existiert nicht | External-Verzeichnis nicht vorhanden |

### Warningen

| Code | Warning | Description |
|------|---------|--------------|
| W103 | add_executable | External erstellt Executable |
| W104 | add_subdirectory | Examples/Tests werden hinzugefügt |

---

## 7. See Also

- [Orchestrator_cmake.md](Orchestrator_cmake.md) — Type Dispatcher
- [Externals_Reference.md](../../reference/Externals_Reference.md) — Verzeichnisstrukturen

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
