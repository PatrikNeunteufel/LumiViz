# Fetched/Handler.cmake — Git External Pipeline

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers  
> **Language:** English  
> **German:** [Handler_cmake.md](../../en/modules/externals/Handler_cmake.md)  
> **Module:** [cmake/externals/Fetched/Handler.cmake](../../../cmake/externals/Fetched/Handler.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API-Reference](#3-api-referenz)
4. [Processing Pipeline](#4-processing-pipeline)
5. [Hook-Integration](#5-hook-integration)
6. [Usagesbeispiele](#6-verwendungsbeispiele)
7. [Errorbehandlung](#7-fehlerbehandlung)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

`Fetched/Handler.cmake` orchestriert die komplette Verarbeitung von Git-basierten Externals.

### Pipeline-Overview

```
┌─────────────────────────────────────────────────────────────┐
│                  _handle_fetched_external()                  │
├─────────────────────────────────────────────────────────────┤
│  1. Declare    → _fetch_git_external()                      │
│  2. PreFetch   → _load_prefetch_hook()                      │
│  3. Available  → _make_external_available()                 │
│  4. PostFetch  → _load_postfetch_hook()                     │
│  5. Register   → _auto_register_external_targets()          │
│  6. Validate   → _validate_external_targets()               │
│  7. Ready      → EXTERNAL_${NAME}_READY = TRUE              │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Dependencies

### Benötigte Module

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Errorbehandlung |
| `Debug.cmake` | Debug-Ausgaben |
| `Json.cmake` | JSON-Parsing |

### Auto-geladene Module

| Modul | Zweck |
|-------|-------|
| `Core/Fetch.cmake` | FetchContent Wrapper |
| `Hooks/HookLoader.cmake` | Hook-System |
| `Registry/Targets.cmake` | Target-Registry |

---

## 3. API-Reference

### 3.1 _handle_fetched_external()

```cmake
_handle_fetched_external(EXT_NAME EXT_JSON)
```

| Parameters | Typ | Description |
|-----------|-----|--------------|
| `EXT_NAME` | String | Name des Externals |
| `EXT_JSON` | JSON | JSON-Definition |

**Setzt Property:** `EXTERNAL_${NAME}_READY = TRUE`

---

### 3.2 _is_external_ready()

```cmake
_is_external_ready(EXT_NAME OUT_VAR)
```

Prüft ob ein External vollständig verarbeitet wurde.

---

## 4. Processing Pipeline

### Detaillierter Flow

| Step | Aktion | Description |
|------|--------|--------------|
| 1 | Declare | `_fetch_git_external()` — Prüft Cache |
| 2 | PreFetch | `_load_prefetch_hook()` — CMake Optionen setzen |
| 3 | Available | `_make_external_available()` — Download |
| 4 | PostFetch | `_load_postfetch_hook()` — Targets erstellen |
| 5 | Register | `_auto_register_external_targets()` — Targets registrieren |
| 6 | Validate | `_validate_external_targets()` — Prüfen |
| 7 | Ready | `EXTERNAL_${NAME}_READY = TRUE` |

---

## 5. Hook-Integration

### Wann werden Hooks benötigt?

| Szenario | PreFetch | PostFetch |
|----------|----------|-----------|
| CMake-Support, Standard-Config | ✗ | ✗ |
| CMake-Support, Tests deaktivieren | ✓ | ✗ |
| Kein CMake-Support (z.B. ImGui) | ✗ | ✓ |
| Spezielle Target-Namen | ✓ | ✗ |

---

## 6. Usagesbeispiele

### Standard CMake External

```json
{
    "spdlog": {
        "git": "https://github.com/gabime/spdlog.git",
        "tag": "v1.12.0"
    }
}
```

### External ohne CMake-Support

```json
{
    "imgui": {
        "git": "https://github.com/ocornut/imgui.git",
        "tag": "v1.89.9",
        "cmakeSupport": false
    }
}
```

---

## 7. Errorbehandlung

| Code | Error | Description |
|------|--------|--------------|
| E201 | Keine Targets | External ohne Targets, PostFetch Hook fehlt |
| E202 | Fetch fehlgeschlagen | FetchContent konnte nicht laden |

---

## 8. See Also

- [Fetch_cmake.md](Fetch_cmake.md) — FetchContent Wrapper
- [HookLoader_cmake.md](HookLoader_cmake.md) — Hook-System
- [Targets_cmake.md](Targets_cmake.md) — Target-Registry
- [Orchestrator_cmake.md](Orchestrator_cmake.md) — Type Dispatcher

---

## 9. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
