# ExecutableCollect.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** In Development (Pre-Release)  
> **Based on:** ModuleDoc v0.5, master_concept v0.5, guidelines v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [ExecutableCollect.md](../../en/modules/project/ExecutableCollect.md)  
> **Module:** [`cmake/project/ExecutableCollect.cmake`](../../../../cmake/project/ExecutableCollect.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Context-Keys](#4-context-keys)
5. [JSON-Mapping](#5-json-mapping)
6. [Errorbehandlung](#6-fehlerbehandlung)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

Das `ExecutableCollect.cmake` Modul **sammelt Daten** aus dem JSON einer Executable und speichert sie in einem Context. Es ist für das Parsing und die Normalisierung der Executable-Definitionen zuständig.

### Kernidee

Trennung von Concerns: Collect sammelt und normalisiert Daten, Create verwendet sie.

### Verantwortlichkeiten

| Bereich | Description |
|---------|--------------|
| JSON-Parsing | Alle Felder extrahieren |
| Normalisierung | Defaults setzen, Typen vereinheitlichen |
| Context-Befüllung | Alle Werte als Context-Keys speichern |

---

## 2. Dependencies

| Modul | Version | Usage |
|-------|---------|------------|
| Json.cmake | 0.5.0 | Alle `_json_*` Functions |
| Context.cmake | 0.5.0 | `ctx_set` |
| Debug.cmake | 0.5.0 | `dbg` |

---

## 3. API-Reference

### 3.1 _collect_executable()

Sammelt alle Felder einer Executable aus JSON in einen Context.

```cmake
_collect_executable(<EXE_JSON> <CTX>)
```

**Parameters:**

| Parameters | Typ | Required | Description |
|-----------|-----|---------|--------------|
| `EXE_JSON` | String | ✓ | JSON-String der Executable |
| `CTX` | String | ✓ | Context-Prefix (z.B. `EXE_0`) |

**Rückgabe:** Keine (befüllt Context)

**Example:**

```cmake
_json_array_get("${_solution_json}" "executables" 0 _exe_json)
ctx_create(EXE_0)
_collect_executable("${_exe_json}" EXE_0)

# Danach verfügbar:
ctx_get(EXE_0 NAME _name)
ctx_get(EXE_0 PATH _path)
```

---

## 4. Context-Keys

### 4.1 Requiredfelder

| Key | Typ | Description |
|-----|-----|--------------|
| `NAME` | String | Target-Name (Required in JSON) |

### 4.2 Optionale Felder

| Key | Typ | Default | Description |
|-----|-----|---------|--------------|
| `DISPLAY_NAME` | String | NAME | Anzeigename |
| `DESCRIPTION` | String | "" | Description |
| `VERSION` | String | Solution-Version | Version |
| `PATH` | Path | `projects/exec/{name}/src` | Source-Pfad |
| `TYPE` | Enum | Aus settings | CONSOLE, GUI, CLI, HEADLESS, WORKER |
| `SKIP` | Bool | FALSE | Überspringen |
| `PCH_ENABLED` | Bool | FALSE | PCH aktiviert |
| `PCH_HEADER` | String | `pch.h` | PCH Header-Datei |
| `PCH_PATH` | String | "" | Custom PCH-Pfad (relativ zu projects/) |
| `DEPENDENCIES` | List | "" | Interne Dependencies |
| `EXTERNALS` | List | "" | Externe Dependencies |
| `EXTERNAL_OPTIONS` | JSON | "{}" | Externe-spezifische Optionen |
| `PLATFORMS` | List | "" (= alle) | Unterstützte Plattformen |
| `DEFINES` | List | "" | Präprozessor-Definitionen |
| `COMPILE_OPTIONS` | List | "" | Zusätzliche Compiler-Optionen |
| `LINK_OPTIONS` | List | "" | Zusätzliche Linker-Optionen |

**Note:** PCH wird implizit aktiviert wenn `pch.header` oder `pch.path` angegeben ist und `pch.enabled` nicht explizit `false` ist.

---

## 5. JSON-Mapping

### 5.1 Minimale JSON

```json
{
    "name": "MyApp"
}
```

Ergebnis:
- NAME = "MyApp"
- PATH = "projects/exec/MyApp/src" (Convention)
- TYPE = SOLUTION_DEFAULT_EXECUTABLE_TYPE
- VERSION = SOLUTION_VERSION

### 5.2 Vollständige JSON

```json
{
    "name": "MyApp",
    "displayName": "My Application",
    "description": "A sample application",
    "version": "1.0.0",
    "path": "src/apps/myapp",
    "type": "GUI",
    "skip": false,
    "pch": {
        "enabled": true,
        "header": "stdafx.h"
    },
    "dependencies": ["CoreLib", "UtilLib"],
    "externals": ["imgui", "glfw"],
    "external_options": {
        "imgui": { "backend": "glfw" }
    },
    "platforms": ["windows", "linux"],
    "defines": ["MY_APP_VERSION=1"],
    "compile_options": ["-ffast-math"],
    "link_options": ["-static"]
}
```

### 5.3 Type-Konvertierung

Der `type`-Wert wird automatisch in Großbuchstaben konvertiert:

| JSON | Context |
|------|---------|
| "console" | "CONSOLE" |
| "Gui" | "GUI" |
| "CLI" | "CLI" |

---

## 6. Errorbehandlung

### 6.1 Fatal Errors

| Code | Bedingung | Lösung |
|------|-----------|--------|
| E001 | `name`-Feld fehlt | Name hinzufügen |

### 6.2 Implizite Defaults

Fehlende optionale Felder führen nicht zu Errorn, sondern werden mit sinnvollen Defaults befüllt.

---

## 7. See Also

- [Executables.cmake](Executables.md) — Ruft _collect_executable auf
- [ExecutableCreate.cmake](ExecutableCreate.md) — Verwendet den befüllten Context
- [Context.cmake](../core/Context.md) — Context-System
- [Json.cmake](../core/Json.md) — JSON-Parsing

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.1** | **2025-12-18** | **PCH_PATH hinzugefügt, implizite PCH-Aktivierung dokumentiert** |
| 0.5.0 | 2025-12-15 | Migration auf Blueprint v0.5.0: Neuer Header mit Zielgruppe/Sprache/English-Link/Modul-Link, nummeriertes Table of Contents mit Ankern, Kapitel-Nummerierung |
| 0.1.0 | 2025-12-05 | Initial (Clean Start): JSON zu Context Mapping |
