# Fetched/Handler.cmake — Git External Pipeline

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [Handler_cmake.md](../../en/modules/externals/Handler_cmake.md)  
> **Modul:** [cmake/externals/Fetched/Handler.cmake](../../../cmake/externals/Fetched/Handler.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API-Referenz](#3-api-referenz)
4. [Processing Pipeline](#4-processing-pipeline)
5. [Hook-Integration](#5-hook-integration)
6. [Verwendungsbeispiele](#6-verwendungsbeispiele)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

`Fetched/Handler.cmake` orchestriert die komplette Verarbeitung von Git-basierten Externals.

### Pipeline-Übersicht

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

## 2. Abhängigkeiten

### Benötigte Module

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Fehlerbehandlung |
| `Debug.cmake` | Debug-Ausgaben |
| `Json.cmake` | JSON-Parsing |

### Auto-geladene Module

| Modul | Zweck |
|-------|-------|
| `Core/Fetch.cmake` | FetchContent Wrapper |
| `Hooks/HookLoader.cmake` | Hook-System |
| `Registry/Targets.cmake` | Target-Registry |

---

## 3. API-Referenz

### 3.1 _handle_fetched_external()

```cmake
_handle_fetched_external(EXT_NAME EXT_JSON)
```

| Parameter | Typ | Beschreibung |
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

| Step | Aktion | Beschreibung |
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

## 6. Verwendungsbeispiele

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

## 7. Fehlerbehandlung

| Code | Fehler | Beschreibung |
|------|--------|--------------|
| E201 | Keine Targets | External ohne Targets, PostFetch Hook fehlt |
| E202 | Fetch fehlgeschlagen | FetchContent konnte nicht laden |

---

## 8. Siehe auch

- [Fetch_cmake.md](Fetch_cmake.md) — FetchContent Wrapper
- [HookLoader_cmake.md](HookLoader_cmake.md) — Hook-System
- [Targets_cmake.md](Targets_cmake.md) — Target-Registry
- [Orchestrator_cmake.md](Orchestrator_cmake.md) — Type Dispatcher

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
