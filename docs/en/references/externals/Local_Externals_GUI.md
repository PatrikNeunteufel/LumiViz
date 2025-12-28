# Local Externals — GUI

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Reference  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [Local_Externals_GUI.md](../../en/references/externals/Local_Externals_GUI.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [GLAD OpenGL Loader](#2-glad-opengl-loader)
3. [See Also](#3-siehe-auch)
4. [Changelog](#4-changelog)

---

## 1. Overview

This document describes lokale GUI/Graphics-Externals für das CMake Architecture Build-System.

| Library | Description | Lizenz |
|---------|--------------|--------|
| **glad** | OpenGL Function Loader (generiert) | MIT / Public Domain |

---

## 2. GLAD OpenGL Loader

### Kurzinfo

| Aspekt | Wert |
|--------|------|
| **Typ** | 🔧 Generiert |
| **Pfad** | `externals/glad` |
| **Include.cmake** | `cmake/externals/includes/glad/Include.cmake` |
| **Plattformen** | Alle |
| **Generator** | [glad.dav1d.de](https://glad.dav1d.de/) |

### Was ist GLAD?

GLAD ist ein **OpenGL Loading Library Generator**. OpenGL-Functions sind nicht direkt verfügbar — sie müssen zur Laufzeit geladen werden. GLAD generiert den Code dafür.

### GLAD generieren

1. Öffne **https://glad.dav1d.de/**

2. Wähle Einstellungen:

| Einstellung | Empfehlung | Description |
|-------------|------------|--------------|
| **Language** | C/C++ | Standard für CMake |
| **Specification** | OpenGL | Desktop-OpenGL |
| **API gl** | 3.3+ | Minimum für Core Profile |
| **Profile** | Core | Ohne deprecated Functions |

3. Klicke **GENERATE**

4. Lade ZIP herunter

5. Entpacke nach `externals/glad/`

### Solution.json

```json
{
    "externals": {
        "glad": {
            "path": "externals/glad"
        }
    }
}
```

### Verzeichnisstruktur

```
externals/glad/
├── include/
│   ├── glad/
│   │   └── glad.h
│   └── KHR/
│       └── khrplatform.h
└── src/
    └── glad.c
```

### Alternative Configurationen

| Configuration | Specification | API | Anwendungsfall |
|---------------|---------------|-----|----------------|
| **Standard** | OpenGL | gl 3.3 Core | Desktop-Apps, ImGui |
| **Modern** | OpenGL | gl 4.5 Core | Fortgeschrittene Features |
| **Mobile** | OpenGL ES | gles2 3.0 | Android, iOS, WebGL |
| **Vulkan** | Vulkan | vulkan 1.3 | Low-Level API |

### Optionale Extensions

Im Konfigurator können Extensions aktiviert werden:

| Extension | Description |
|-----------|--------------|
| `GL_ARB_debug_output` | Debug-Callbacks |
| `GL_ARB_direct_state_access` | DSA Functions |
| `GL_ARB_texture_filter_anisotropic` | Bessere Texturen |

### Usagesbeispiel

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {
    // GLFW initialisieren
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    // GLAD laden - MUSS nach glfwMakeContextCurrent!
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }
    
    // OpenGL verwenden
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
```

### Typische Kombination (OpenGL + ImGui)

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

> **Reihenfolge wichtig:** glad → glfw → imgui (imgui linkt automatisch gegen glad und glfw)

### Vergleich mit Git-Alternativen

| Feature | glad (Local) | glfw (Git) | imgui (Git) | SDL2 (Git) |
|---------|--------------|------------|-------------|------------|
| **Typ** | OpenGL Loader | Window/Input | GUI | Multimedia |
| **Generiert** | ✅ | ❌ | ❌ | ❌ |
| **CMake-Support** | — | ✅ | ❌ | ✅ |
| **Hook** | — | PreFetch | PostFetch 🔧 | PreFetch |

### Detail-Dokumentation

→ [glad_Include.md](../../modules/externals/includes/glad/Glad_Include.md)

---

## 3. See Also

- [Externals.md](../Externals.md) — Hauptübersicht aller Externals
- [Local_Externals.md](Local_Externals.md) — Local Externals Overview
- [Git_Externals_GUI.md](Git_Externals_GUI.md) — Git GUI-Externals (glfw, imgui, SDL2)
- [Externals_UserGuide.md](../../userguides/Externals.md) — GUI-App Anleitung

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Eigene Kategorie GUI (parallel zu Git)** |
