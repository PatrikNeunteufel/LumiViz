# hooks/HookLoader.cmake — Pre/PostFetch Hook System

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [HookLoader_cmake.md](../../en/modules/externals/HookLoader_cmake.md)  
> **Modul:** [cmake/externals/hooks/HookLoader.cmake](../../../cmake/externals/hooks/HookLoader.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [Convention Paths](#3-convention-paths)
4. [API-Referenz](#4-api-referenz)
5. [Hook-Variablen](#5-hook-variablen)
6. [Hook-Typen](#6-hook-typen)
7. [Hook Reuse](#7-hook-reuse)
8. [Verwendungsbeispiele](#8-verwendungsbeispiele)
9. [Fehlerbehandlung](#9-fehlerbehandlung)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

`hooks/HookLoader.cmake` implementiert das Hook-System für Fetched Externals.

### Kernfunktionen

- **PreFetch** — CMake-Optionen setzen bevor External konfiguriert wird
- **PostFetch** — Targets erstellen für Externals ohne CMakeLists.txt
- **Hook Reuse** — Gleiche Hooks für Varianten (z.B. imgui/imgui_docking)

### Convention over Configuration

```
cmake/externals/hooks/
├── prefetch/
│   ├── glfw.cmake
│   └── spdlog.cmake
└── postfetch/
    └── imgui.cmake
```

---

## 2. Abhängigkeiten

| Modul | Zweck |
|-------|-------|
| `Errors.cmake` | Fehlerbehandlung |
| `Debug.cmake` | Debug-Ausgaben |
| `Json.cmake` | JSON-Parsing |
| `Core/Fetch.cmake` | `_get_external_source_dir()` |

---

## 3. Convention Paths

| Typ | Pfad |
|-----|------|
| PreFetch | `cmake/externals/hooks/prefetch/{name}.cmake` |
| PostFetch | `cmake/externals/hooks/postfetch/{name}.cmake` |

---

## 4. API-Referenz

### 4.1 _load_prefetch_hook()

```cmake
_load_prefetch_hook(EXT_NAME EXT_JSON)
```

Lädt einen PreFetch Hook (vor FetchContent_MakeAvailable).

---

### 4.2 _load_postfetch_hook()

```cmake
_load_postfetch_hook(EXT_NAME EXT_JSON)
```

Lädt einen PostFetch Hook (nach FetchContent_MakeAvailable).

---

### 4.3 _get_hook_name()

```cmake
_get_hook_name(EXT_NAME EXT_JSON OUT_HOOK_NAME)
```

Bestimmt welcher Hook verwendet werden soll (eigener Name oder `hook` Override).

---

## 5. Hook-Variablen

### In allen Hooks verfügbar

| Variable | Beschreibung |
|----------|--------------|
| `HOOK_EXTERNAL_NAME` | Name des Externals (für Target-Namen verwenden!) |
| `HOOK_EXTERNAL_JSON` | JSON-Definition des Externals |

### Nur in PostFetch Hooks

| Variable | Beschreibung |
|----------|--------------|
| `HOOK_SOURCE_DIR` | Pfad zum Source-Verzeichnis |

---

## 6. Hook-Typen

### PreFetch Hooks

Laufen **vor** `FetchContent_MakeAvailable()`.

```cmake
# cmake/externals/hooks/prefetch/glfw.cmake
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
```

### PostFetch Hooks

Laufen **nach** `FetchContent_MakeAvailable()`.

```cmake
# cmake/externals/hooks/postfetch/imgui.cmake
add_library(${HOOK_EXTERNAL_NAME} STATIC
    ${HOOK_SOURCE_DIR}/imgui.cpp
    ...
)
_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
```

---

## 7. Hook Reuse

```json
{
    "imgui": {
        "git": "https://github.com/ocornut/imgui.git",
        "tag": "v1.91.6",
        "cmakeSupport": false
    },
    "imgui_docking": {
        "git": "https://github.com/ocornut/imgui.git",
        "branch": "docking",
        "hook": "imgui",
        "cmakeSupport": false
    }
}
```

→ Beide verwenden `imgui.cmake` Hooks, erstellen aber unterschiedliche Targets.

---

## 8. Verwendungsbeispiele

### Minimal PreFetch

```cmake
# cmake/externals/hooks/prefetch/catch2.cmake
set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
```

### PostFetch mit Target

```cmake
# cmake/externals/hooks/postfetch/stb.cmake
add_library(${HOOK_EXTERNAL_NAME} INTERFACE)
target_include_directories(${HOOK_EXTERNAL_NAME} INTERFACE ${HOOK_SOURCE_DIR})
_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
```

---

## 9. Fehlerbehandlung

| Code | Fehler | Beschreibung |
|------|--------|--------------|
| E216 | Hook nicht gefunden | Explizit angegebener Hook existiert nicht |

---

## 10. Siehe auch

- [Handler_cmake.md](Handler_cmake.md) — Hook-Aufrufer
- [Targets_cmake.md](Targets_cmake.md) — Target-Registrierung
- [glfw_PreFetch.md](glfw_PreFetch.md) — GLFW PreFetch Hook
- [imgui_PostFetch.md](imgui_PostFetch.md) — ImGui PostFetch Hook

---

## 11. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
