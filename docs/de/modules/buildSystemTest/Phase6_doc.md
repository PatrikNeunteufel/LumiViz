# phase6.cmake — Modul-Dokumentation

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Pfad:** `cmake/buildSystemTest/phase6.cmake`  
> **Status:** Stabil  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Tests](#2-tests)
3. [Erfolgs-Flag](#3-erfolgs-flag)
4. [Siehe auch](#4-siehe-auch)
5. [Changelog](#5-changelog)

---

## 1. Übersicht

**Phase 6** testet die **Git Externals Pipeline** — FetchContent-Integration, Hooks und Target Registry.

| Aspekt | Beschreibung |
|--------|--------------|
| **Zweck** | Git Externals & Hooks Validation |
| **Debug-ID** | `PHASE6_TEST` |
| **Abhängigkeiten** | Phase 1-5, Fetch.cmake, HookLoader.cmake |

---

## 2. Tests

### 2.1 Git External mit CMake-Support (GLFW)

| Prüfung | Beschreibung |
|---------|--------------|
| `TARGET glfw` | Target existiert |
| `EXTERNAL_TARGET_glfw` | In Registry eingetragen |

### 2.2 Git External ohne CMake-Support (ImGui)

Prüft, dass PostFetch-Hook das Target erstellt:

```cmake
if(TARGET imgui OR TARGET imgui_docking)
    # PostFetch Hook hat funktioniert
endif()
```

### 2.3 Hook Reuse

Prüft, dass `imgui_docking` den `imgui`-Hook wiederverwendet:

| Property | Beschreibung |
|----------|--------------|
| `EXTERNAL_HOOK_imgui` | Hook-Pfad |
| `EXTERNAL_HOOK_imgui_docking` | Sollte gleich sein |

Beide Targets sollten existieren und unterschiedlich sein.

### 2.4 Externals JSON Parsing

Prüft JSON-Felder:

| Feld | External | Beschreibung |
|------|----------|--------------|
| `git` | glfw | Git-URL |
| `cmakeSupport` | imgui | false |
| `hook` | imgui_docking | "imgui" |

### 2.5 Fetched External Linking

Prüft `imGuiApp` Links:

```cmake
get_target_property(_libs imGuiApp LINK_LIBRARIES)
# Sollte glfw und imgui enthalten
```

---

## 3. Erfolgs-Flag

```cmake
set(PHASE6_TEST_PASSED TRUE CACHE BOOL "Phase 6 Test passed" FORCE)
```

---

## 4. Siehe auch

- [Fetch.md](../modules/externals/core/Fetch.md)
- [HookLoader.md](../modules/externals/Hook/HookLoader.md)
- [Handler.md](../modules/externals/Fetched/Handler.md)
- [Targets.md](../modules/externals/Registry/Targets.md)
- [Git_Externals.md](../reference/Git_Externals.md)

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Blueprint v0.5.0 Format** |
| 0.1.0 | 2025-12-12 | Initial |
