# PostFetch/imgui.cmake — ImGui PostFetch Hook

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** ModuleDoc  
> **Status:** Aktiv  
> **Basiert auf:** ModuleDoc v0.5, Doc v0.5  
> **Zielgruppe:** Build-System-Entwickler  
> **Sprache:** Deutsch  
> **English:** [imgui_PostFetch.md](../../en/modules/externals/imgui_PostFetch.md)  
> **Hook:** [cmake/externals/hooks/postfetch/imgui.cmake](../../../cmake/externals/hooks/postfetch/imgui.cmake)  
> **Modul-Version:** 1.0.0

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Warum ein PostFetch Hook?](#2-warum-ein-postfetch-hook)
3. [Erstellte Targets](#3-erstellte-targets)
4. [Backends](#4-backends)
5. [Hook Reuse](#5-hook-reuse)
6. [Solution.json Konfiguration](#6-solutionjson-konfiguration)
7. [Abhängigkeiten](#7-abhängigkeiten)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Der `imgui.cmake` PostFetch Hook erstellt CMake-Targets für Dear ImGui, da ImGui selbst kein CMakeLists.txt enthält.

---

## 2. Warum ein PostFetch Hook?

ImGui ist ein **header-centric** Projekt ohne CMake-Support. Der PostFetch Hook:

1. Sammelt die nötigen Source-Dateien
2. Erstellt eine STATIC Library
3. Konfiguriert Include-Directories
4. Linkt Backend-Abhängigkeiten
5. Registriert das Target

---

## 3. Erstellte Targets

| Target | Typ | Beschreibung |
|--------|-----|--------------|
| `${HOOK_EXTERNAL_NAME}` | STATIC | Kombinierte ImGui Library mit Backends |

Der Target-Name entspricht dem External-Namen:
- `"imgui"` → Target `imgui`
- `"imgui_docking"` → Target `imgui_docking`

---

## 4. Backends

| Backend | Bedingung |
|---------|-----------|
| OpenGL3 | Automatisch wenn vorhanden |
| GLFW | Automatisch wenn vorhanden |
| Win32 | Nur auf Windows |

---

## 5. Hook Reuse

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

**Wichtig:** Der Hook verwendet `${HOOK_EXTERNAL_NAME}` für Target-Namen.

---

## 6. Solution.json Konfiguration

### Standard ImGui

```json
{
    "imgui": {
        "git": "https://github.com/ocornut/imgui.git",
        "tag": "v1.91.6",
        "cmakeSupport": false
    }
}
```

### In Executable verwenden

```json
{
    "executables": [{
        "name": "MyGuiApp",
        "externals": ["glad", "glfw", "imgui"]
    }]
}
```

**Reihenfolge wichtig:** glad und glfw vor imgui.

---

## 7. Abhängigkeiten

| External | Verwendung |
|----------|------------|
| `glad` | OpenGL Loader (PUBLIC gelinkt) |
| `glfw` | Window/Input Backend (PUBLIC gelinkt) |

---

## 8. Siehe auch

- [HookLoader_cmake.md](HookLoader_cmake.md) — Hook-System
- [Targets_cmake.md](Targets_cmake.md) — Target-Registrierung
- [glfw_PreFetch.md](glfw_PreFetch.md) — GLFW PreFetch Hook

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Dokumentation auf Blueprint v0.5.0 migriert** |
