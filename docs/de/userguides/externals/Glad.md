# GLAD — UserGuide

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [glad.md](../../../en/userguides/externals/Glad.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [GLAD generieren](#2-glad-generieren)
3. [Solution.json Konfiguration](#3-solutionjson-konfiguration)
4. [C++ Verwendung](#4-c-verwendung)
5. [Mit GLFW kombinieren](#5-mit-glfw-kombinieren)
6. [Mit ImGui kombinieren](#6-mit-imgui-kombinieren)
7. [Troubleshooting](#7-troubleshooting)
8. [Weiterführende Informationen](#8-weiterführende-informationen)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

**GLAD** ist ein OpenGL Loading Library Generator. OpenGL-Funktionen sind zur Laufzeit nicht direkt verfügbar — sie müssen dynamisch geladen werden. GLAD generiert den Code dafür.

| Aspekt | Wert |
|--------|------|
| **Typ** | Local External (Generiert) |
| **Pfad** | `externals/glad` |
| **Lizenz** | MIT / Public Domain |
| **Generator** | [glad.dav1d.de](https://glad.dav1d.de/) |

### Warum GLAD?

| Vorteil | Beschreibung |
|---------|--------------|
| 🎯 **Konfigurierbar** | Nur benötigte OpenGL-Version |
| 🔧 **Generiert** | Kein Build-System nötig |
| 📦 **Standalone** | Keine Abhängigkeiten |
| ✅ **Modern** | Unterstützt Core Profile |

---

## 2. GLAD generieren

### 2.1 Web-Konfigurator

1. Öffne **https://glad.dav1d.de/**

2. Wähle Einstellungen:

| Einstellung | Empfehlung | Beschreibung |
|-------------|------------|--------------|
| **Language** | C/C++ | Standard für CMake |
| **Specification** | OpenGL | Desktop-OpenGL |
| **API gl** | 3.3 oder höher | Minimum für Core Profile |
| **Profile** | Core | Ohne deprecated Functions |
| **Generate a loader** | ✅ | Loader-Code generieren |

3. Klicke **GENERATE**

4. Lade ZIP herunter

5. Entpacke nach `externals/glad/`

### 2.2 Standard-Konfigurationen

| Konfiguration | API | Profile | Anwendungsfall |
|---------------|-----|---------|----------------|
| **Standard** | gl 3.3 | Core | Desktop-Apps, ImGui |
| **Modern** | gl 4.5 | Core | Compute Shaders, DSA |
| **Compat** | gl 3.3 | Compatibility | Legacy-Code |
| **Mobile** | gles2 3.0 | — | Android, iOS, WebGL |
| **Vulkan** | vulkan 1.3 | — | Vulkan statt OpenGL |

### 2.3 Optionale Extensions

Im Konfigurator können Extensions aktiviert werden:

| Extension | Beschreibung |
|-----------|--------------|
| `GL_ARB_debug_output` | Debug-Callbacks |
| `GL_ARB_direct_state_access` | DSA Functions (4.5) |
| `GL_ARB_texture_filter_anisotropic` | Anisotrope Filterung |
| `GL_ARB_compute_shader` | Compute Shaders |

### 2.4 Verzeichnisstruktur

Nach dem Entpacken:

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

---

## 3. Solution.json Konfiguration

### 3.1 Minimal

```json
{
    "externals": {
        "glad": {
            "path": "externals/glad"
        }
    },
    "executables": [
        {
            "name": "OpenGLApp",
            "type": "GUI",
            "externals": ["glad"]
        }
    ]
}
```

### 3.2 Mit GLFW

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

### 3.3 Vollständige GUI-Anwendung

```json
{
    "externals": {
        "glad": { "path": "externals/glad" },
        "glfw": { "git": "https://github.com/glfw/glfw.git", "tag": "3.4" },
        "imgui": { "git": "https://github.com/ocornut/imgui.git", "tag": "v1.91.6", "cmakeSupport": false }
    },
    "executables": [
        {
            "name": "GuiApp",
            "type": "GUI",
            "externals": ["glad", "glfw", "imgui"]
        }
    ]
}
```

---

## 4. C++ Verwendung

### 4.1 Grundlagen (nur GLAD)

```cpp
#include <glad/glad.h>
#include <iostream>

// Hinweis: Ohne GLFW funktioniert GLAD nicht alleine!
// Ein OpenGL-Kontext muss existieren, bevor gladLoadGL() aufgerufen wird.
```

### 4.2 Minimales Beispiel mit GLFW

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // 1. GLFW initialisieren
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // 2. OpenGL Version setzen
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // 3. Fenster erstellen
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // 4. Kontext aktivieren
    glfwMakeContextCurrent(window);
    
    // 5. GLAD laden - MUSS NACH glfwMakeContextCurrent!
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // 6. OpenGL Info ausgeben
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // 7. Viewport setzen
    glViewport(0, 0, 800, 600);
    
    // 8. Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Clear
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Swap & Poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 9. Cleanup
    glfwTerminate();
    return 0;
}
```

### 4.3 Dreiecks rendern

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Triangle", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    // Shader kompilieren
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Vertex Data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}
```

---

## 5. Mit GLFW kombinieren

### 5.1 Reihenfolge

```
1. glfwInit()
2. glfwCreateWindow()
3. glfwMakeContextCurrent()    ← OpenGL-Kontext erstellt
4. gladLoadGLLoader()          ← GLAD lädt OpenGL-Funktionen
5. OpenGL-Code verwenden
```

> **WICHTIG:** `gladLoadGLLoader()` MUSS nach `glfwMakeContextCurrent()` aufgerufen werden!

### 5.2 Callback für Resize

```cpp
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // ... window erstellen ...
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    // Callback registrieren
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // ...
}
```

Siehe [glfw.md](glfw.md) für mehr Details zu GLFW.

---

## 6. Mit ImGui kombinieren

### 6.1 Reihenfolge

```
1. glfwInit()
2. glfwCreateWindow()
3. glfwMakeContextCurrent()
4. gladLoadGLLoader()          ← GLAD zuerst
5. ImGui::CreateContext()      ← ImGui danach
6. ImGui_ImplGlfw_InitForOpenGL()
7. ImGui_ImplOpenGL3_Init()
```

### 6.2 Beispiel

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui App", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // VSync
    
    // GLAD laden
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // ImGui Frame starten
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // ImGui Widgets
        ImGui::Begin("Hello");
        ImGui::Text("Hello, World!");
        if (ImGui::Button("Click me")) {
            // ...
        }
        ImGui::End();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
```

Siehe [imgui.md](imgui.md) für mehr Details zu ImGui.

---

## 7. Troubleshooting

### 7.1 "undefined reference to gl..."

**Problem:** OpenGL-Funktionen nicht gefunden

**Ursachen & Lösungen:**

1. **GLAD nicht geladen:**
   ```cpp
   // FALSCH
   glClear(GL_COLOR_BUFFER_BIT);  // Crash!
   
   // RICHTIG
   gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
   glClear(GL_COLOR_BUFFER_BIT);  // OK
   ```

2. **Kein OpenGL-Kontext:**
   ```cpp
   // FALSCH
   gladLoadGLLoader(...);  // Kein Kontext!
   
   // RICHTIG
   glfwMakeContextCurrent(window);
   gladLoadGLLoader(...);
   ```

### 7.2 "glad.h not found"

**Problem:** Header nicht gefunden

**Lösung:** Verzeichnisstruktur prüfen:
```
externals/glad/include/glad/glad.h  ← Muss existieren
```

### 7.3 GLAD gibt false zurück

**Problem:** `gladLoadGLLoader()` gibt 0 zurück

**Lösungen:**
1. OpenGL-Kontext existiert nicht
2. Treiber unterstützt angefragte Version nicht
3. Bei macOS: Forward Compat aktivieren:
   ```cpp
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   ```

### 7.4 Funktionen fehlen

**Problem:** Bestimmte OpenGL-Funktionen nicht verfügbar

**Lösung:** GLAD mit höherer Version neu generieren oder Extensions aktivieren.

---

## 8. Weiterführende Informationen

### Offizielle Ressourcen

| Ressource | Link |
|-----------|------|
| **GLAD Generator** | [glad.dav1d.de](https://glad.dav1d.de/) |
| **GitHub** | [github.com/Dav1dde/glad](https://github.com/Dav1dde/glad) |
| **OpenGL Registry** | [khronos.org/registry/OpenGL](https://www.khronos.org/registry/OpenGL/) |
| **LearnOpenGL** | [learnopengl.com](https://learnopengl.com/) |

### Siehe auch

- [Externals.md](../Externals.md) — Externals Kombinationen
- [Local_Externals_GUI.md](../../references/externals/Local_Externals_GUI.md) — Referenz
- [glfw.md](glfw.md) — Window & Input
- [imgui.md](imgui.md) — Immediate Mode GUI

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Initial: Detaillierter UserGuide für GLAD** |
