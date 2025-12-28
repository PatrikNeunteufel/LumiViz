# Externals — UserGuide

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Externals.md](../../en/userguides/Externals.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Externe Bibliotheken einbinden](#2-externe-bibliotheken-einbinden)
3. [Kombinationen](#3-kombinationen)
4. [Reihenfolge und Dependencies](#4-reihenfolge-und-abhängigkeiten)
5. [Typische Projektsetups](#5-typische-projektsetups)
6. [Troubleshooting](#6-troubleshooting)
7. [Detail-Guides](#7-detail-guides)
8. [See Also](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Overview

Dieser Guide erklärt, wie externe Bibliotheken im CMake Architecture Build-System kombiniert werden. Für detaillierte Informationen zu einzelnen Externals siehe die [Detail-Guides](#7-detail-guides).

### External-Typen

| Typ | Quelle | Examples |
|-----|--------|-----------|
| **Local** | `path` | doctest, lua54, glad, BASS |
| **Git** | `git` | glfw, imgui, googletest, Qt6 |

### Grundprinzip

```json
{
    "externals": {
        "name": { "path": "..." }
    },
    "executables": [
        {
            "name": "MyApp",
            "externals": ["name"]
        }
    ]
}
```

---

## 2. Externe Bibliotheken einbinden

### 2.1 Local External

```json
{
    "externals": {
        "doctest": {
            "path": "externals/doctest"
        }
    }
}
```

### 2.2 Git External

```json
{
    "externals": {
        "glfw": {
            "git": "https://github.com/glfw/glfw.git",
            "tag": "3.4"
        }
    }
}
```

### 2.3 Git External ohne CMake-Support

```json
{
    "externals": {
        "imgui": {
            "git": "https://github.com/ocornut/imgui.git",
            "tag": "v1.91.6",
            "cmakeSupport": false
        }
    }
}
```

### 2.4 Mit Options

```json
{
    "executables": [
        {
            "name": "AudioPlayer",
            "externals": ["bass"],
            "external_options": {
                "bass": {
                    "BASS_FLAC": true,
                    "BASS_FX": true
                }
            }
        }
    ]
}
```

---

## 3. Kombinationen

### 3.1 OpenGL GUI-Anwendung

Die klassische Kombination für OpenGL-basierte GUI-Anwendungen:

| External | Typ | Funktion |
|----------|-----|----------|
| **glad** | Local | OpenGL Function Loader |
| **glfw** | Git | Window & Input |
| **imgui** | Git | Immediate Mode GUI |

```json
{
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" },
        "imgui": { "git": "https://github.com/ocornut/imgui.git", "tag": "v1.91.6", "cmakeSupport": false }
    },
    "executables": [
        {
            "name": "MyGuiApp",
            "type": "GUI",
            "externals": ["glad", "glfw", "imgui"]
        }
    ]
}
```

### 3.2 Scripted Application

Lua-Scripting mit komfortablen C++ Bindings:

| External | Typ | Funktion |
|----------|-----|----------|
| **lua54** | Local | Lua 5.4 Core Engine |
| **sol2** | Git | Modern C++ Bindings |

```json
{
    "externals": {
        "lua54": { "path": "externals/lua54" },
        "sol2": { "git": "https://github.com/ThePhD/sol2.git", "tag": "v3.3.1" }
    },
    "executables": [
        {
            "name": "ScriptedApp",
            "externals": ["lua54", "sol2"]
        }
    ]
}
```

### 3.3 Audio Player

Professionelle Audio-Wiedergabe mit Plugins:

| External | Typ | Funktion |
|----------|-----|----------|
| **BASS** | Local | Audio Core + Plugins |

```json
{
    "externals": {
        "bass": { "path": "externals/bass" }
    },
    "executables": [
        {
            "name": "AudioPlayer",
            "externals": ["bass"],
            "external_options": {
                "bass": {
                    "BASS_FLAC": true,
                    "BASS_OPUS": true,
                    "BASS_FX": true
                }
            }
        }
    ]
}
```

### 3.4 Unit Testing

| Kombination | Anwendungsfall |
|-------------|----------------|
| **doctest** (Local) | Schnelle Tests, Header-Only |
| **googletest** (Git) | Mocking mit GMock |
| **catch2** (Git) | BDD-Style Testing |

```json
{
    "externals": {
        "doctest": { "path": "externals/doctest" }
    },
    "tests": [
        {
            "name": "UnitTests",
            "framework": "doctest",
            "externals": ["doctest"]
        }
    ]
}
```

### 3.5 Qt6 Desktop Application

Professionelle Desktop-Anwendung mit Qt6:

| External | Typ | Funktion |
|----------|-----|----------|
| **Qt6** | System | GUI Framework |

```json
{
    "externals": {
        "qt6": {
            "system": true,
            "components": ["Widgets", "Core", "Gui"]
        }
    },
    "executables": [
        {
            "name": "QtApp",
            "type": "GUI",
            "externals": ["qt6"]
        }
    ]
}
```

### 3.6 Game mit Scripting und Audio

Komplexe Kombination für Spiele:

```json
{
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" },
        "imgui": { "git": "https://github.com/ocornut/imgui.git", "tag": "v1.91.6", "cmakeSupport": false },
        "lua54": { "path": "externals/lua54" },
        "sol2": { "git": "https://github.com/ThePhD/sol2.git", "tag": "v3.3.1" },
        "bass": { "path": "externals/bass" }
    },
    "executables": [
        {
            "name": "MyGame",
            "type": "GUI",
            "externals": ["glad", "glfw", "imgui", "lua54", "sol2", "bass"],
            "external_options": {
                "bass": { "BASS_FLAC": true }
            }
        }
    ]
}
```

---

## 4. Reihenfolge und Dependencies

### 4.1 Kritische Reihenfolgen

Einige Externals haben **Dependencies**, die eine bestimmte Reihenfolge erfordern:

| Kombination | Reihenfolge | Grund |
|-------------|-------------|-------|
| OpenGL + ImGui | glad → glfw → imgui | imgui benötigt OpenGL-Kontext |
| Lua + sol2 | lua54 → sol2 | sol2 wraps lua54 |

### 4.2 glad → glfw → imgui

```
1. glad      - Lädt OpenGL-Functions
2. glfw      - Erstellt Window mit OpenGL-Kontext
3. imgui     - Verwendet OpenGL für Rendering
```

**Im Code:**

```cpp
// 1. GLFW initialisieren
glfwInit();
GLFWwindow* window = glfwCreateWindow(800, 600, "App", nullptr, nullptr);
glfwMakeContextCurrent(window);

// 2. GLAD laden (NACH MakeContextCurrent!)
gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

// 3. ImGui initialisieren (NACH GLAD!)
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init("#version 330");
```

### 4.3 lua54 → sol2

```
1. lua54    - Lua Core Engine
2. sol2     - C++ Wrapper um lua54
```

**Im Code:**

```cpp
// sol2 inkludiert lua automatisch
#include <sol/sol.hpp>

sol::state lua;  // Verwendet lua54 intern
lua.open_libraries(sol::lib::base);
```

---

## 5. Typische Projektsetups

### 5.1 Minimales GUI-Projekt

```json
{
    "solution": {
        "name": "MinimalGui",
        "version": "1.0.0"
    },
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" }
    },
    "executables": [
        {
            "name": "MinimalGui",
            "type": "GUI",
            "externals": ["glad", "glfw"]
        }
    ]
}
```

### 5.2 Projekt mit Tests

```json
{
    "solution": {
        "name": "TestedProject",
        "version": "1.0.0"
    },
    "externals": {
        "doctest": { "path": "externals/doctest" }
    },
    "libraries": [
        {
            "name": "CoreLib",
            "type": "static"
        }
    ],
    "executables": [
        {
            "name": "MainApp",
            "libraries": ["CoreLib"]
        }
    ],
    "tests": [
        {
            "name": "CoreTests",
            "framework": "doctest",
            "externals": ["doctest"],
            "libraries": ["CoreLib"]
        }
    ]
}
```

### 5.3 Multi-Executable Projekt

```json
{
    "solution": {
        "name": "MultiApp",
        "version": "1.0.0"
    },
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" },
        "imgui": { "git": "https://github.com/ocornut/imgui.git", "tag": "v1.91.6", "cmakeSupport": false },
        "doctest": { "path": "externals/doctest" }
    },
    "executables": [
        {
            "name": "Editor",
            "type": "GUI",
            "externals": ["glad", "glfw", "imgui"]
        },
        {
            "name": "CLI",
            "type": "console"
        }
    ],
    "tests": [
        {
            "name": "AllTests",
            "framework": "doctest",
            "externals": ["doctest"]
        }
    ]
}
```

---

## 6. Troubleshooting

### 6.1 "undefined reference to gl..."

**Problem:** OpenGL-Functions nicht gefunden

**Lösung:** glad vor glfw in externals-Liste, und `gladLoadGLLoader()` nach `glfwMakeContextCurrent()` aufrufen.

### 6.2 "imgui.h not found"

**Problem:** ImGui-Header nicht gefunden

**Lösung:** `"cmakeSupport": false` für imgui setzen, PostFetch-Hook prüfen.

### 6.3 "lua.h not found" mit sol2

**Problem:** sol2 findet Lua nicht

**Lösung:** lua54 VOR sol2 in der externals-Liste.

### 6.4 BASS-Plugin nicht geladen

**Problem:** FLAC/Opus-Dateien werden nicht abgespielt

**Lösung:** Entsprechende Option in `external_options` aktivieren:

```json
"external_options": {
    "bass": { "BASS_FLAC": true }
}
```

### 6.5 Qt6 nicht gefunden

**Problem:** CMake findet Qt6 nicht

**Lösung:** `CMAKE_PREFIX_PATH` auf Qt6-Installation setzen:

```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 ..
```

---

## 7. Detail-Guides

Für detaillierte Informationen zu einzelnen Externals:

### Local Externals

| External | Guide | Description |
|----------|-------|--------------|
| doctest | [externals/doctest.md](externals/doctest.md) | Header-Only Testing |
| lua54 | [externals/lua54.md](externals/lua54.md) | Lua 5.4 Scripting |
| glad | [externals/glad.md](externals/glad.md) | OpenGL Loader |
| BASS | [externals/bass.md](externals/bass.md) | Audio mit Plugins |

### Git Externals

| External | Guide | Description |
|----------|-------|--------------|
| glfw | [externals/glfw.md](externals/glfw.md) | Window & Input |
| imgui | [externals/imgui.md](externals/imgui.md) | Immediate Mode GUI |
| googletest | [externals/googletest.md](externals/googletest.md) | Testing + GMock |
| catch2 | [externals/catch2.md](externals/catch2.md) | BDD-Style Testing |
| Qt6 | [externals/qt6.md](externals/qt6.md) | Desktop GUI Framework |

---

## 8. See Also

- [Externals.md](../references/Externals.md) — Reference aller Externals
- [Local_Externals.md](../references/externals/Local_Externals.md) — Local Externals Reference
- [Git_Externals.md](../references/externals/Git_Externals.md) — Git Externals Reference
- [Solution_Schema.md](../references/Solution_Schema.md) — JSON-Schema

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Fokus auf Kombinationen; Detail-Guides ausgelagert** |
| 0.5.0 | 2025-12-14 | Initial |
