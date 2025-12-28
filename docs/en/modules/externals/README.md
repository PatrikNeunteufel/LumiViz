# Externals — External-Management-System

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Language:** English  
> **German:** [README.md](../../../en/modules/externals/README.md)

---

## Quick-Start

**Externe Libraries einbinden?**
1. [Orchestrator_cmake.md](Orchestrator_cmake.md) — Haupt-Entry verstehen
2. [../../userguides/Externals.md](../../userguides/Externals.md) — Praktische Anleitung
3. [../../userguides/Adding_Externals.md](../../userguides/Adding_Externals.md) — Neue External hinzufügen

**Git-basierte Library fetchen?**
1. [core/Fetch_cmake.md](core/Fetch_cmake.md) — FetchContent-Wrapper
2. [hooks/HookLoader_cmake.md](hooks/HookLoader_cmake.md) — Pre/Post-Fetch Hooks
3. [fetched/Handler_cmake.md](fetched/Handler_cmake.md) — Post-Fetch Verarbeitung

**Lokale Library einbinden?**
1. [locals/Attach_cmake.md](locals/Attach_cmake.md) — Local-Attach-System
2. [includes/](includes/README.md) — Include-Definitionen pro Library

**Hook für Library schreiben?**
1. [hooks/HookLoader_cmake.md](hooks/HookLoader_cmake.md) — Hook-System verstehen
2. [hooks/prefetch/Glfw.md](hooks/prefetch/Glfw.md) — Example: Pre-Fetch Hook
3. [hooks/postfetch/Imgui.md](hooks/postfetch/Imgui.md) — Example: Post-Fetch Hook

**Target-Registry verstehen?**
1. [registry/Targets_cmake.md](registry/Targets_cmake.md) — Zentrale Target-Verwaltung

---

## Overview

Das External-Management-System handhabt alle externen Dependencies: Git-basierte (fetched) und lokale (local) Libraries. Es nutzt ein zentralisiertes Registry-System und Hook-Mechanismen.

### Architecture

```
Orchestrator.cmake           ← Haupt-Entry, koordiniert alles
    │
    ├── registry/Targets.cmake   ← Zentrale Target-Registry
    │
    ├── core/Fetch.cmake         ← Git FetchContent
    │       │
    │       └── hooks/           ← Pre/Post-Fetch Hooks
    │           ├── HookLoader.cmake
    │           ├── prefetch/Glfw.cmake
    │           └── postfetch/Imgui.cmake
    │
    ├── fetched/Handler.cmake    ← Verarbeitet gefetchte Libs
    │
    ├── locals/Attach.cmake      ← Bindet lokale Libs ein
    │
    └── includes/                ← Library-spezifische Includes
        ├── bass/Bass_Include.cmake
        ├── doctest/Doctest_Include.cmake
        ├── glad/Glad_Include.cmake
        └── lua54/Lua54_Include.cmake
```

---

## Dateien

| Datei | Description |
|-------|--------------|
| [Orchestrator_cmake.md](Orchestrator_cmake.md) | Haupt-Orchestrierung — koordiniert alle External-Operationen |

---

## Unterordner

| Ordner | Dateien | Description |
|--------|---------|--------------|
| [core/](core/README.md) | 1 | Kern-Functions |
| [fetched/](fetched/README.md) | 1 | Handler für gefetchte Libraries |
| [hooks/](hooks/README.md) | 3 | Pre/Post-Fetch Hook-System |
| [includes/](includes/README.md) | 4 | Library-spezifische Includes |
| [locals/](locals/README.md) | 1 | Lokale Library-Integration |
| [registry/](registry/README.md) | 1 | Target-Registry |

---

## Unterordner-Dateien (Direktzugriff)

### core/ — Kern-Functions

| Datei | Description |
|-------|--------------|
| [core/Fetch_cmake.md](core/Fetch_cmake.md) | Git FetchContent Wrapper mit Hook-Integration |

### fetched/ — Post-Fetch Handler

| Datei | Description |
|-------|--------------|
| [fetched/Handler_cmake.md](fetched/Handler_cmake.md) | Verarbeitet Libraries nach Git-Fetch |

### locals/ — Lokale Libraries

| Datei | Description |
|-------|--------------|
| [locals/Attach_cmake.md](locals/Attach_cmake.md) | Bindet vorinstallierte Libraries an Targets |

### registry/ — Target-Registry

| Datei | Description |
|-------|--------------|
| [registry/Targets_cmake.md](registry/Targets_cmake.md) | Zentrale Verwaltung aller External-Targets |

### hooks/ — Hook-System

| Datei | Description |
|-------|--------------|
| [hooks/HookLoader_cmake.md](hooks/HookLoader_cmake.md) | Hook-Loader und Discovery |
| [hooks/prefetch/Glfw.md](hooks/prefetch/Glfw.md) | GLFW Pre-Fetch Hook |
| [hooks/postfetch/Imgui.md](hooks/postfetch/Imgui.md) | ImGui Post-Fetch Hook |

### includes/ — Library-Includes

| Datei | Description |
|-------|--------------|
| [includes/bass/Bass_Include.md](includes/bass/Bass_Include.md) | BASS Audio Library + Plugins |
| [includes/doctest/Doctest_Include.md](includes/doctest/Doctest_Include.md) | doctest Testing Framework |
| [includes/glad/Glad_Include.md](includes/glad/Glad_Include.md) | GLAD OpenGL Loader |
| [includes/lua54/Lua54_Include.md](includes/lua54/Lua54_Include.md) | Lua 5.4 Scripting Engine |

---

## External-Typen

| Typ | Quelle | Examples | Reference |
|-----|--------|-----------|----------|
| **Git (fetched)** | Git Repository | GLFW, ImGui, doctest | [Git_Externals.md](../../references/externals/Git_Externals.md) |
| **Local** | System/SDK | BASS, Lua, Qt6 | [Local_Externals.md](../../references/externals/Local_Externals.md) |

---

## See Also

- [../README.md](../README.md) — Modul-Overview
- [../../references/Externals.md](../../references/Externals.md) — Externals-Reference
- [../../references/externals/](../../references/externals/README.md) — External-Definitionen
- [../../userguides/Externals.md](../../userguides/Externals.md) — Externals User Guide
