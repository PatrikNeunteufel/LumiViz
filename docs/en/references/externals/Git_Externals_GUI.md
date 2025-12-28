# Git Externals: GUI & Graphics — Reference

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Git_Externals_GUI.md](../../en/references/externals/Git_Externals_GUI.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Window & Input](#2-window--input)
3. [Immediate Mode GUI](#3-immediate-mode-gui)
4. [Game Development](#4-game-development)
5. [Vulkan / Low-Level Graphics](#5-vulkan--low-level-graphics)
6. [Schnellreferenz](#6-schnellreferenz)
7. [See Also](#7-siehe-auch)
8. [Changelog](#8-changelog)

---

## 1. Overview

This document describes Bibliotheken für grafische Benutzeroberflächen, Fenster-Management und Rendering.

### Kategorien

| Kategorie | Bibliotheken |
|-----------|--------------|
| Window & Input | glfw, SDL2, SDL3 |
| Immediate Mode GUI | imgui, imgui_docking |
| Game Development | raylib, sokol |
| Vulkan | vulkan-headers, volk, VMA |

---

## 2. Window & Input

### 2.1 glfw

> **Zweck:** Cross-platform Window/Input-Handling für OpenGL, Vulkan und OpenGL ES.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/glfw/glfw.git` |
| **Aktueller Tag** | `3.4` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch erforderlich |
| **Target** | `glfw` |

```json
"glfw": {
    "git": "https://github.com/glfw/glfw.git",
    "tag": "3.4"
}
```

**PreFetch Hook (glfw.cmake):**
```cmake
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include <GLFW/glfw3.h>

glfwInit();
GLFWwindow* window = glfwCreateWindow(800, 600, "Hello", NULL, NULL);
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
}
glfwTerminate();
```

---

### 2.2 SDL2

> **Zweck:** Cross-platform Multimedia (Video, Audio, Input, Threading).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/libsdl-org/SDL.git` |
| **Aktueller Tag** | `release-2.30.0` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |

```json
"sdl2": {
    "git": "https://github.com/libsdl-org/SDL.git",
    "tag": "release-2.30.0"
}
```

**PreFetch Hook (sdl2.cmake):**
```cmake
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
```

---

### 2.3 SDL3

> **Zweck:** Nächste Generation von SDL mit verbesserter API.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/libsdl-org/SDL.git` |
| **Aktueller Tag** | `release-3.2.0` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |

```json
"sdl3": {
    "git": "https://github.com/libsdl-org/SDL.git",
    "tag": "release-3.2.0"
}
```

---

## 3. Immediate Mode GUI

### 3.1 imgui

> **Zweck:** Bloat-free Immediate Mode GUI für Tools und Debug-UIs.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/ocornut/imgui.git` |
| **Aktueller Tag** | `v1.91.6` (stable), `v1.91.6-docking` (docking) |
| **CMake Support** | ❌ |
| **Hook** | PostFetch erforderlich 🔧 |
| **Dependencies** | Backend (GLFW, SDL, etc.) + Renderer (OpenGL, Vulkan, etc.) |

```json
"imgui": {
    "git": "https://github.com/ocornut/imgui.git",
    "tag": "v1.91.6",
    "cmakeSupport": false
}
```

**Docking-Variante (gleicher Hook):**
```json
"imgui_docking": {
    "git": "https://github.com/ocornut/imgui.git",
    "tag": "v1.91.6-docking",
    "cmakeSupport": false,
    "hook": "imgui"
}
```

**PostFetch Hook:** Siehe `cmake/externals/Hooks/PostFetch/imgui.cmake`

Der Hook erstellt ein `imgui` Target mit:
- Core ImGui files
- OpenGL3 Backend
- GLFW Backend
- Win32 Backend (auf Windows)

**Automatisches Linking:**
- `glad` (wenn vorhanden) + `IMGUI_IMPL_OPENGL_LOADER_GLAD`
- `glfw` (wenn vorhanden)

**Important - Reihenfolge in Solution.json:**
```json
"externals": {
    "glad": { ... },   // 1. zuerst
    "glfw": { ... },   // 2. dann
    "imgui": { ... }   // 3. zuletzt
}
```

**Usage:**
```cpp
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

ImGui::CreateContext();
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init("#version 330");

// In render loop:
ImGui_ImplOpenGL3_NewFrame();
ImGui_ImplGlfw_NewFrame();
ImGui::NewFrame();

ImGui::Begin("Hello");
ImGui::Text("Hello, ImGui!");
ImGui::End();

ImGui::Render();
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
```

---

## 4. Game Development

### 4.1 raylib

> **Zweck:** Einfache Bibliothek für Spiele-Entwicklung und Rapid Prototyping.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/raysan5/raylib.git` |
| **Aktueller Tag** | `5.0` |
| **CMake Support** | ✅ |
| **Hook** | PreFetch empfohlen |

```json
"raylib": {
    "git": "https://github.com/raysan5/raylib.git",
    "tag": "5.0"
}
```

**PreFetch Hook (raylib.cmake):**
```cmake
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
```

**Usage:**
```cpp
#include "raylib.h"

InitWindow(800, 450, "raylib example");
while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello, raylib!", 10, 10, 20, DARKGRAY);
    EndDrawing();
}
CloseWindow();
```

---

### 4.2 sokol

> **Zweck:** Minimale Cross-platform Libraries (App, GFX, Audio, etc.).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/floooh/sokol.git` |
| **Aktueller Tag** | – (branch: master) |
| **CMake Support** | ❌ (Header-only) |
| **Hook** | PostFetch erforderlich 🔧 |

```json
"sokol": {
    "git": "https://github.com/floooh/sokol.git",
    "branch": "master",
    "cmakeSupport": false
}
```

**PostFetch Hook (sokol.cmake):**
```cmake
add_library(${HOOK_EXTERNAL_NAME} INTERFACE)
target_include_directories(${HOOK_EXTERNAL_NAME} INTERFACE "${HOOK_SOURCE_DIR}")
_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
```

---

## 5. Vulkan / Low-Level Graphics

### 5.1 Vulkan-Headers

> **Zweck:** Offizielle Vulkan API Headers.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/KhronosGroup/Vulkan-Headers.git` |
| **Aktueller Tag** | `v1.3.280` |
| **CMake Support** | ✅ |
| **Hook** | – |

```json
"vulkan-headers": {
    "git": "https://github.com/KhronosGroup/Vulkan-Headers.git",
    "tag": "v1.3.280"
}
```

---

### 5.2 volk (Vulkan Loader)

> **Zweck:** Meta-loader für Vulkan (lädt Funktionspointer dynamisch).

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/zeux/volk.git` |
| **Aktueller Tag** | `1.3.280` |
| **CMake Support** | ✅ |
| **Hook** | – |

```json
"volk": {
    "git": "https://github.com/zeux/volk.git",
    "tag": "1.3.280"
}
```

---

### 5.3 VulkanMemoryAllocator

> **Zweck:** Einfache Vulkan Memory-Allokation.

| Aspekt | Wert |
|--------|------|
| **Repository** | `https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git` |
| **Aktueller Tag** | `v3.0.1` |
| **CMake Support** | ✅ |
| **Hook** | – |

```json
"vma": {
    "git": "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git",
    "tag": "v3.0.1"
}
```

---

## 6. Schnellreferenz

| Bibliothek | Tag | CMake | Hook | Hauptverwendung |
|------------|-----|-------|------|-----------------|
| glfw | 3.4 | ✅ | PreFetch | Window/Input |
| sdl2 | release-2.30.0 | ✅ | PreFetch | Multimedia |
| sdl3 | release-3.2.0 | ✅ | PreFetch | Multimedia (next-gen) |
| imgui | v1.91.6 | ❌ | PostFetch 🔧 | Immediate Mode GUI |
| raylib | 5.0 | ✅ | PreFetch | Simple game dev |
| sokol | master | ❌ | PostFetch 🔧 | Minimal cross-platform |
| vulkan-headers | v1.3.280 | ✅ | – | Vulkan API |
| volk | 1.3.280 | ✅ | – | Vulkan loader |
| vma | v3.0.1 | ✅ | – | Vulkan memory |

---

## 7. See Also

- [Git_Externals_Reference.md](Git_Externals.md) — Hauptübersicht
- [Git_Externals_Core.md](Git_Externals_Core.md) — Logging & Testing
- [Git_Externals_Media.md](Git_Externals_Media.md) — Audio & Math
- [Externals.md](../Externals.md) — Local Externals (glad)

---

## 8. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Initial: Ausgelagert aus Git_Externals_Reference** |
