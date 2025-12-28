# glad/Include.cmake — GLAD OpenGL Loader Integration

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** ModuleDoc  
> **Status:** Aktiv  
> **Based on:** ModuleDoc v0.5, Doc v0.5  
> **Target Audience:** Build System Developers, C++ Developers  
> **Language:** English  
> **German:** [glad_Include.md](../../en/modules/externals/includes/glad_Include.md)  
> **Module:** [cmake/externals/includes/glad/Include.cmake](../../../../cmake/externals/includes/glad/Include.cmake)  
> **Module Version:** 1.0.0

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [GLAD Konfigurator](#2-glad-konfigurator)
3. [Verfügbare Variablen](#3-verfügbare-variablen)
4. [Erstellte Targets](#4-erstellte-targets)
5. [Verzeichnisstruktur](#5-verzeichnisstruktur)
6. [Include.cmake Implementation](#6-includecmake-implementierung)
7. [Erweiterte Configurationen](#7-erweiterte-konfigurationen)
8. [Usagesbeispiele](#8-verwendungsbeispiele)
9. [Integration mit anderen Externals](#9-integration-mit-anderen-externals)
10. [Errorbehandlung](#10-fehlerbehandlung)
11. [See Also](#11-siehe-auch)
12. [Changelog](#12-changelog)

---

## 1. Overview

Die `glad/Include.cmake` integriert den **GLAD OpenGL Loader** als lokales External in das Build-System.

### Was ist GLAD?

GLAD ist ein **OpenGL Loading Library Generator**. OpenGL-Functions sind nicht direkt verfügbar — sie müssen zur Laufzeit geladen werden. GLAD generiert den Code dafür.

### Kernfunktionen

| Funktion | Description |
|----------|--------------|
| OpenGL Loading | Lädt OpenGL-Funktionspointeru zur Laufzeit |
| Multi-API | OpenGL, OpenGL ES, Vulkan, EGL, GLX, WGL |
| Versionsspezifisch | Nur benötigte Functions für gewählte Version |
| Erweiterungen | Optionale OpenGL-Extensions ladbar |

### Target-Erstellung

Erstellt ein `STATIC` Target `glad` das die generierte `glad.c` kompiliert.

---

## 2. GLAD Konfigurator

GLAD muss über den Web-Konfigurator generiert werden.

### 2.1 Standard-Configuration (OpenGL 3.3 Core)

**Schritt-für-Schritt Anleitung:**

1. Öffne **https://glad.dav1d.de/**

2. Wähle folgende Einstellungen:

| Einstellung | Wert | Erklärung |
|-------------|------|-----------|
| **Language** | C/C++ | Generiert C-kompatiblen Code |
| **Specification** | OpenGL | Standard-Desktop-OpenGL |
| **API gl** | Version 3.3 | Minimum für moderne Features |
| **Profile** | Core | Ohne deprecated Functions |
| **Generate a loader** | ✅ (aktiviert) | Lädt Functions automatisch |

3. Klicke **GENERATE**

4. Lade das **ZIP-Archiv** herunter

5. Entpacke nach `externals/glad/`

### 2.2 Konfigurator-Optionen im Detail

#### Language

| Option | Usage |
|--------|------------|
| **C/C++** | Standard für CMake-Projekte |
| D | D-Sprache |
| Nim | Nim-Sprache |
| Pascal | Pascal/Delphi |

#### Specification

| Option | Description |
|--------|--------------|
| **OpenGL** | Desktop-OpenGL (Windows, Linux, macOS) |
| OpenGL ES | Mobile/Embedded (Android, iOS, WebGL) |
| Vulkan | Moderne Low-Level-API |
| EGL | Display/Context Management |
| GLX | X11 OpenGL Extension |
| WGL | Windows OpenGL Extension |

#### API gl Version

| Version | Features | Empfehlung |
|---------|----------|------------|
| 2.1 | Fixed-Function + Shaders | Legacy-Support |
| **3.3** | Core Profile, VAOs, UBOs | **Empfohlen** |
| 4.0 | Tessellation | Fortgeschritten |
| 4.3 | Compute Shaders | Fortgeschritten |
| 4.5 | DSA, Clip Control | Modern |
| 4.6 | SPIR-V, Anisotropic | Aktuellste |

#### Profile

| Option | Description |
|--------|--------------|
| **Core** | Nur moderne API, keine deprecated Functions |
| Compatibility | Alte + neue API (größer, langsamer) |

### 2.3 Optionale Extensions

Im Konfigurator können **Extensions** aktiviert werden:

| Extension | Description | Anwendungsfall |
|-----------|--------------|----------------|
| `GL_ARB_debug_output` | Debug-Callbacks | Debugging |
| `GL_ARB_texture_filter_anisotropic` | Anisotrope Filterung | Bessere Texturen |
| `GL_ARB_clip_control` | Depth-Range Control | Reverse-Z |
| `GL_ARB_direct_state_access` | DSA Functions | Weniger Binds |
| `GL_ARB_compute_shader` | Compute Shaders | GPGPU |

**Example mit Extensions:**

1. Im Konfigurator unter "Extensions" die gewünschten auswählen
2. Generieren und herunterladen
3. Im Code prüfen:

```cpp
if (GLAD_GL_ARB_debug_output) {
    glDebugMessageCallback(debugCallback, nullptr);
}
```

---

## 3. Verfügbare Variablen

Diese Variablen werden vom Orchestrator bereitgestellt:

| Variable | Description |
|----------|--------------|
| `EXTERNAL_NAME` | `"glad"` |
| `EXTERNAL_PATH` | Absoluter Pfad zu `externals/glad` |
| `EXTERNAL_JSON` | JSON-Element aus Solution.json |
| `EXTERNAL_OPTIONS` | Target-spezifische Options (JSON) |

---

## 4. Erstellte Targets

| Target | Typ | Description |
|--------|-----|--------------|
| `glad` | STATIC | Kompilierte GLAD-Library |

### Automatische Dependencies

```cmake
find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL)
```

---

## 5. Verzeichnisstruktur

Nach dem Entpacken des ZIP-Archivs:

```
externals/glad/
├── include/
│   ├── glad/
│   │   └── glad.h          ← OpenGL-Funktionsdeklarationen
│   └── KHR/
│       └── khrplatform.h   ← Plattform-Typen
└── src/
    └── glad.c              ← Loader-Implementation
```

> **Note:** Die Include.cmake liegt im Convention-Pfad `cmake/externals/includes/glad/Include.cmake`, nicht im External-Ordner selbst.

---

## 6. Include.cmake Implementation

### 6.1 Standard-Implementation

```cmake
# cmake/externals/includes/glad/Include.cmake
# GLAD OpenGL Loader
# Version: 0.5.0

# Source-Dateien
set(_glad_sources
    "${EXTERNAL_PATH}/src/glad.c"
)

# Static Library erstellen
add_library(glad STATIC ${_glad_sources})

# Include-Verzeichnisse
target_include_directories(glad PUBLIC
    "${EXTERNAL_PATH}/include"
)

# OpenGL linken
find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL)

# Warningen für externen Code unterdrücken
if(MSVC)
    target_compile_options(glad PRIVATE /W0)
else()
    target_compile_options(glad PRIVATE -w)
endif()

# Target registrieren
_register_external_target("glad" "glad" PRIMARY)
```

### 6.2 Mit OpenGL ES Support

Wenn GLAD für OpenGL ES generiert wurde:

```cmake
# Für OpenGL ES (z.B. Android, Raspberry Pi)
if(OPENGL_ES)
    find_package(OpenGL REQUIRED COMPONENTS EGL GLES2)
    target_link_libraries(glad PUBLIC OpenGL::EGL OpenGL::GLES2)
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(glad PUBLIC OpenGL::GL)
endif()
```

---

## 7. Erweiterte Configurationen

### 7.1 OpenGL 4.5 mit Extensions

**Konfigurator-Einstellungen:**
- API gl: **Version 4.5**
- Profile: **Core**
- Extensions: `GL_ARB_debug_output`, `GL_ARB_direct_state_access`

**Angepasste Include.cmake (optional):**

```cmake
# cmake/externals/includes/glad/Include.cmake
# GLAD OpenGL 4.5 mit Extensions

add_library(glad STATIC "${EXTERNAL_PATH}/src/glad.c")

target_include_directories(glad PUBLIC
    "${EXTERNAL_PATH}/include"
)

find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL)

# Compile-Definition für OpenGL-Version
target_compile_definitions(glad PUBLIC
    GLAD_GL_VERSION_4_5=1
)

if(MSVC)
    target_compile_options(glad PRIVATE /W0)
else()
    target_compile_options(glad PRIVATE -w)
endif()

_register_external_target("glad" "glad" PRIMARY)
```

### 7.2 OpenGL ES 3.0 (Mobile)

**Konfigurator-Einstellungen:**
- Specification: **OpenGL ES**
- API gles2: **Version 3.0**

**Angepasste Include.cmake:**

```cmake
# cmake/externals/includes/glad/Include.cmake
# GLAD OpenGL ES 3.0

add_library(glad STATIC "${EXTERNAL_PATH}/src/glad.c")

target_include_directories(glad PUBLIC
    "${EXTERNAL_PATH}/include"
)

# OpenGL ES Libraries
if(ANDROID)
    target_link_libraries(glad PUBLIC EGL GLESv3)
elseif(EMSCRIPTEN)
    # WebGL - keine explizite Linkung nötig
else()
    find_package(OpenGL REQUIRED COMPONENTS EGL GLES3)
    target_link_libraries(glad PUBLIC OpenGL::EGL OpenGL::GLES3)
endif()

target_compile_options(glad PRIVATE -w)

_register_external_target("glad" "glad" PRIMARY)
```

### 7.3 Vulkan Loader

**Konfigurator-Einstellungen:**
- Specification: **Vulkan**
- API vulkan: **Version 1.3**

**Angepasste Include.cmake:**

```cmake
# cmake/externals/includes/glad/Include.cmake
# GLAD Vulkan Loader

add_library(glad STATIC "${EXTERNAL_PATH}/src/glad_vulkan.c")

target_include_directories(glad PUBLIC
    "${EXTERNAL_PATH}/include"
)

find_package(Vulkan REQUIRED)
target_link_libraries(glad PUBLIC Vulkan::Vulkan)

_register_external_target("glad" "glad" PRIMARY)
```

---

## 8. Usagesbeispiele

### 8.1 Basis-Usage mit GLFW

**Solution.json:**

```json
{
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": {
            "git": "https://github.com/glfw/glfw.git",
            "tag": "3.4"
        }
    },
    "executables": [
        {
            "name": "OpenGLApp",
            "type": "GUI",
            "externals": ["glad", "glfw"]
        }
    ]
}
```

**C++:**

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // GLFW initialisieren
    if (!glfwInit()) {
        std::cerr << "GLFW init failed" << std::endl;
        return -1;
    }

    // OpenGL 3.3 Core anfordern
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Fenster erstellen
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // GLAD laden - MUSS nach glfwMakeContextCurrent!
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed" << std::endl;
        return -1;
    }

    // OpenGL-Version ausgeben
    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // Render-Loop
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

### 8.2 Mit Debug-Callback (GL_ARB_debug_output)

```cpp
#include <glad/glad.h>

void GLAPIENTRY debugCallback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    std::cerr << "[OpenGL] " << message << std::endl;
}

int main() {
    // ... GLFW/GLAD init ...

    // Debug-Callback aktivieren (wenn Extension verfügbar)
    if (GLAD_GL_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debugCallback, nullptr);
        std::cout << "Debug output enabled" << std::endl;
    }

    // ... Rest der Anwendung ...
}
```

### 8.3 Extension-Prüfung

```cpp
#include <glad/glad.h>
#include <iostream>

void checkExtensions() {
    std::cout << "Extension Support:" << std::endl;
    
    std::cout << "  GL_ARB_debug_output: " 
              << (GLAD_GL_ARB_debug_output ? "yes" : "no") << std::endl;
    
    std::cout << "  GL_ARB_direct_state_access: " 
              << (GLAD_GL_ARB_direct_state_access ? "yes" : "no") << std::endl;
    
    std::cout << "  GL_ARB_texture_filter_anisotropic: " 
              << (GLAD_GL_ARB_texture_filter_anisotropic ? "yes" : "no") << std::endl;
}
```

---

## 9. Integration mit anderen Externals

### 9.1 Reihenfolge in Solution.json

GLAD muss **vor** Externals definiert werden, die es benötigen:

```json
"externals": {
    "glad": { "path": "externals/glad" },     // 1. Zuerst
    "glfw": { ... },                           // 2. Dann GLFW
    "imgui": { ..., "cmakeSupport": false }    // 3. Zuletzt ImGui
}
```

### 9.2 ImGui-Integration

Der ImGui PostFetch-Hook erkennt GLAD automatisch:

```cmake
# In cmake/externals/hooks/postfetch/imgui.cmake
if(TARGET glad)
    target_link_libraries(imgui PUBLIC glad)
    target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
    message(STATUS "[imgui] Linked: glad")
endif()
```

### 9.3 Typische GUI-Kombination

```json
{
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": {
            "git": "https://github.com/glfw/glfw.git",
            "tag": "3.4"
        },
        "imgui": {
            "git": "https://github.com/ocornut/imgui.git",
            "tag": "v1.91.6",
            "cmakeSupport": false
        }
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

---

## 10. Errorbehandlung

### 10.1 CMake-Error

| Code | Description | Lösung |
|------|--------------|--------|
| E213 | Include.cmake nicht gefunden | Pfad prüfen: `cmake/externals/includes/glad/` |
| E214 | External-Pfad existiert nicht | `externals/glad/` erstellen und GLAD entpacken |
| — | OpenGL nicht gefunden | OpenGL-Entwicklungspakete installieren |

### 10.2 Laufzeit-Error

**"gladLoadGLLoader failed":**

```cpp
// Häufigster Error: Kein aktiver OpenGL-Context
glfwMakeContextCurrent(window);  // MUSS vor gladLoadGLLoader!

if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    // Error
}
```

**"Undefined reference to glXXX":**

- GLAD-Version zu niedrig für verwendete Funktion
- Neu generieren mit höherer OpenGL-Version

**"Extension not available":**

```cpp
// Immer prüfen vor Usage!
if (GLAD_GL_ARB_debug_output) {
    // Extension verfügbar
} else {
    // Fallback oder Feature deaktivieren
}
```

### 10.3 Linux: OpenGL-Pakete

```bash
# Ubuntu/Debian
sudo apt install libgl1-mesa-dev

# Fedora
sudo dnf install mesa-libGL-devel

# Arch
sudo pacman -S mesa
```

---

## 11. See Also

- [Externals_Reference.md](../../../reference/Externals_Reference.md) — Alle Externals
- [Externals_UserGuide.md](../../../guides/Externals_UserGuide.md) — GUI-App Anleitung
- [imgui_PostFetch.md](../hooks/postfetch/imgui_PostFetch.md) — ImGui-Integration
- [glfw_PreFetch.md](../hooks/prefetch/glfw_PreFetch.md) — GLFW-Configuration
- [GLAD Web Generator](https://glad.dav1d.de/) — Offizieller Konfigurator
- [GLAD GitHub](https://github.com/Dav1dde/glad) — Source Code

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.0** | **2025-12-15** | **Neues Dokument: Blueprint v0.5.0, Konfigurator-Anleitung, Extensions, OpenGL ES/Vulkan Varianten** |
