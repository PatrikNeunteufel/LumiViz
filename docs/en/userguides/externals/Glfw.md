# GLFW — UserGuide

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Guide  
> **Status:** Stable  
> **Target Audience:** Alle Entwickler  
> **Language:** English  
> **German:** [glfw.md](../../../en/userguides/externals/Glfw.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Solution.json Configuration](#2-solutionjson-konfiguration)
3. [C++ Usage](#3-c-verwendung)
4. [Input Handling](#4-input-handling)
5. [Fortgeschrittene Techniken](#5-fortgeschrittene-techniken)
6. [Mit GLAD und ImGui](#6-mit-glad-und-imgui)
7. [Troubleshooting](#7-troubleshooting)
8. [Weiterführende Informationen](#8-weiterführende-informationen)
9. [Changelog](#9-changelog)

---

## 1. Overview

**GLFW** ist eine plattformübergreifende Library für Window-Erstellung, OpenGL-Kontexte und Input-Handling.

| Aspekt | Wert |
|--------|------|
| **Typ** | Git External |
| **Repository** | https://github.com/glfw/glfw |
| **Empfohlener Tag** | 3.4 |
| **Lizenz** | zlib/libpng |
| **Website** | [glfw.org](https://www.glfw.org/) |

### Warum GLFW?

| Vorteil | Description |
|---------|--------------|
| 🪟 **Cross-Platform** | Windows, Linux, macOS |
| 🎮 **Input** | Keyboard, Mouse, Gamepad |
| 🖥️ **Multi-Monitor** | Fullscreen & Windowed |
| ⚡ **Lightweight** | Minimaler Overhead |

---

## 2. Solution.json Configuration

### 2.1 Minimal

```json
{
    "externals": {
        "glfw": {
            "git": "https://github.com/glfw/glfw.git",
            "tag": "3.4"
        }
    },
    "executables": [
        {
            "name": "WindowApp",
            "type": "GUI",
            "externals": ["glfw"]
        }
    ]
}
```

### 2.2 Mit GLAD

```json
{
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" }
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

### 2.3 PreFetch Hook

```cmake
# cmake/externals/hooks/prefetch/glfw.cmake
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
```

---

## 3. C++ Usage

### 3.1 Fenster erstellen

```cpp
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // GLFW initialisieren
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Fenster erstellen
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // OpenGL-Kontext aktivieren
    glfwMakeContextCurrent(window);
    
    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Events verarbeiten
        glfwPollEvents();
        
        // Buffer tauschen
        glfwSwapBuffers(window);
    }
    
    // Aufräumen
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
```

### 3.2 OpenGL-Version setzen

```cpp
glfwInit();

// OpenGL 3.3 Core Profile
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// Für macOS erforderlich
#ifdef __APPLE__
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL 3.3", nullptr, nullptr);
```

### 3.3 Window Hints

| Hint | Description | Default |
|------|--------------|---------|
| `GLFW_RESIZABLE` | Größe änderbar | `GLFW_TRUE` |
| `GLFW_VISIBLE` | Sichtbar | `GLFW_TRUE` |
| `GLFW_DECORATED` | Titelleiste | `GLFW_TRUE` |
| `GLFW_FOCUSED` | Fokussiert | `GLFW_TRUE` |
| `GLFW_MAXIMIZED` | Maximiert | `GLFW_FALSE` |
| `GLFW_FLOATING` | Immer oben | `GLFW_FALSE` |
| `GLFW_TRANSPARENT_FRAMEBUFFER` | Transparent | `GLFW_FALSE` |
| `GLFW_SAMPLES` | MSAA Samples | 0 |

```cpp
glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
glfwWindowHint(GLFW_SAMPLES, 4);  // 4x MSAA
glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
```

### 3.4 Fullscreen

```cpp
// Primärer Monitor
GLFWmonitor* monitor = glfwGetPrimaryMonitor();
const GLFWvidmode* mode = glfwGetVideoMode(monitor);

// Fullscreen-Fenster
GLFWwindow* window = glfwCreateWindow(
    mode->width, mode->height, 
    "Fullscreen", 
    monitor,        // Monitor für Fullscreen
    nullptr
);

// Borderless Fullscreen (Windowed Fullscreen)
glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
GLFWwindow* window = glfwCreateWindow(
    mode->width, mode->height,
    "Borderless",
    nullptr,        // Kein Monitor = Windowed
    nullptr
);
glfwSetWindowPos(window, 0, 0);
```

### 3.5 VSync

```cpp
glfwMakeContextCurrent(window);

// VSync aktivieren (1 = ON, 0 = OFF)
glfwSwapInterval(1);
```

---

## 4. Input Handling

### 4.1 Keyboard

```cpp
// Polling
while (!glfwWindowShouldClose(window)) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        // W gedrückt
    }
    
    glfwPollEvents();
}

// Callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        // Toggle Fullscreen
    }
    
    // Modifier prüfen
    if (mods & GLFW_MOD_CONTROL) {
        if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            // Ctrl+S
        }
    }
}

glfwSetKeyCallback(window, key_callback);
```

### 4.2 Mouse

```cpp
// Position
double xpos, ypos;
glfwGetCursorPos(window, &xpos, &ypos);

// Buttons
if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    // Linke Maustaste
}

// Callbacks
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    // Maus bewegt
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // Rechtsklick
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Scroll: yoffset > 0 = nach oben
}

glfwSetCursorPosCallback(window, cursor_position_callback);
glfwSetMouseButtonCallback(window, mouse_button_callback);
glfwSetScrollCallback(window, scroll_callback);
```

### 4.3 Cursor-Modi

```cpp
// Normal
glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

// Versteckt
glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

// Gefangen (für FPS-Spiele)
glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

// Raw Mouse Input (wenn unterstützt)
if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}
```

### 4.4 Gamepad

```cpp
// Gamepad verbunden?
if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
    // Name
    const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
    
    // Als Gamepad?
    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
            // Buttons
            if (state.buttons[GLFW_GAMEPAD_BUTTON_A]) {
                // A gedrückt
            }
            
            // Analog Sticks (-1.0 bis 1.0)
            float leftX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            float leftY = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
            
            // Trigger (0.0 bis 1.0)
            float leftTrigger = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
        }
    }
}
```

---

## 5. Fortgeschrittene Techniken

### 5.1 Callbacks

```cpp
// Fenster-Größe geändert
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Fenster geschlossen
void window_close_callback(GLFWwindow* window) {
    // Speichern?
}

// Error Callback
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Registrieren
glfwSetErrorCallback(error_callback);
glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
glfwSetWindowCloseCallback(window, window_close_callback);
```

### 5.2 User Pointer

```cpp
struct AppState {
    bool running = true;
    float cameraX = 0;
    float cameraY = 0;
};

AppState state;
glfwSetWindowUserPointer(window, &state);

// In Callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    AppState* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    
    if (key == GLFW_KEY_LEFT) {
        state->cameraX -= 1.0f;
    }
}
```

### 5.3 Multi-Monitor

```cpp
// Alle Monitore
int count;
GLFWmonitor** monitors = glfwGetMonitors(&count);

for (int i = 0; i < count; i++) {
    const char* name = glfwGetMonitorName(monitors[i]);
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
    
    std::cout << "Monitor " << i << ": " << name 
              << " (" << mode->width << "x" << mode->height << ")" << std::endl;
}

// Fenster auf zweitem Monitor
GLFWwindow* window = glfwCreateWindow(800, 600, "Second Monitor", monitors[1], nullptr);
```

### 5.4 Clipboard

```cpp
// Text in Clipboard
glfwSetClipboardString(window, "Hello, Clipboard!");

// Text aus Clipboard
const char* text = glfwGetClipboardString(window);
if (text) {
    std::cout << "Clipboard: " << text << std::endl;
}
```

### 5.5 Time

```cpp
double lastTime = glfwGetTime();

while (!glfwWindowShouldClose(window)) {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // FPS berechnen
    static double fpsTime = 0;
    static int frames = 0;
    frames++;
    fpsTime += deltaTime;
    if (fpsTime >= 1.0) {
        std::cout << "FPS: " << frames << std::endl;
        frames = 0;
        fpsTime = 0;
    }
    
    // Bewegung mit deltaTime
    float speed = 5.0f;
    position += velocity * speed * (float)deltaTime;
    
    glfwPollEvents();
    glfwSwapBuffers(window);
}
```

---

## 6. Mit GLAD und ImGui

### 6.1 Vollständiges Example

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

int main() {
    // GLFW Setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "App", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    // GLAD Setup
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("Demo");
        ImGui::Text("Hello!");
        ImGui::End();
        
        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
```

Siehe [glad.md](glad.md) und [imgui.md](imgui.md) für mehr Details.

---

## 7. Troubleshooting

### 7.1 "Failed to create window"

**Problem:** `glfwCreateWindow` gibt nullptr zurück

**Lösungen:**
1. `glfwInit()` aufrufen
2. OpenGL-Version prüfen (nicht zu hoch für Treiber)
3. `glfwSetErrorCallback` für Details

### 7.2 Keine OpenGL-Functions

**Problem:** `glClear` etc. crashen

**Lösung:** GLAD nach `glfwMakeContextCurrent` laden:
```cpp
glfwMakeContextCurrent(window);
gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);  // Hier!
```

### 7.3 macOS schwarzes Fenster

**Problem:** Fenster bleibt schwarz

**Lösung:** Forward Compat aktivieren:
```cpp
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
```

### 7.4 Input funktioniert nicht

**Problem:** Keyboard/Mouse reagiert nicht

**Lösung:** `glfwPollEvents()` aufrufen!

---

## 8. Weiterführende Informationen

### Offizielle Ressourcen

| Ressource | Link |
|-----------|------|
| **Website** | [glfw.org](https://www.glfw.org/) |
| **GitHub** | [github.com/glfw/glfw](https://github.com/glfw/glfw) |
| **Dokumentation** | [glfw.org/documentation](https://www.glfw.org/documentation.html) |
| **Input Guide** | [glfw.org/docs/latest/input_guide.html](https://www.glfw.org/docs/latest/input_guide.html) |

### See Also

- [Externals.md](../Externals.md) — Externals Kombinationen
- [Git_Externals_GUI.md](../../references/externals/Git_Externals_GUI.md) — Reference
- [glad.md](glad.md) — OpenGL Loader
- [imgui.md](imgui.md) — GUI Framework

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Initial: Detaillierter UserGuide für GLFW** |
