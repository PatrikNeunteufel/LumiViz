# VisualizerWidget — OpenGL Visualisierungs-Widget

> **Version:** 2.0.0  
> **Datum:** 2025-12-29  
> **Typ:** CppModuleDoc  
> **Status:** In Entwicklung  
> **Modul:** MyViz::UI::VisualizerWidget  
> **Dateien:** VisualizerWidget.hpp, VisualizerWidget.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6::Widgets, Qt6::OpenGL, Qt6::OpenGLWidgets  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Qt6-Konzepte](#4-qt6-konzepte)
5. [VSync-Steuerung](#5-vsync-steuerung)
6. [OpenGL-Integration](#6-opengl-integration)
7. [Rendering-Flow](#7-rendering-flow)
8. [Thread-Sicherheit](#8-thread-sicherheit)
9. [Erweiterung](#9-erweiterung)
10. [Changelog](#10-changelog)

---

## 1. Übersicht

### 1.1 Zweck

VisualizerWidget ist ein QOpenGLWidget, das hardware-beschleunigte Audio-Visualisierungen rendert. Es bildet das Herzstück der visuellen Darstellung in MyViz.

### 1.2 Verantwortlichkeiten

- OpenGL-Context-Management
- Runtime VSync-Steuerung (plattformübergreifend)
- Rendering von Audio-Visualisierungen
- Viewport und Projection-Handling

### 1.3 Nicht-Verantwortlichkeiten

- Audio-Datenverarbeitung (→ AudioEngine)
- FFT-Berechnung (→ AudioEngine)
- Window-Management (→ MainWindow)
- Event-Loop / Frame-Timing (→ Application)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6::Widgets | Extern | QWidget-Basisklasse |
| Qt6::OpenGL | Extern | QOpenGLFunctions |
| Qt6::OpenGLWidgets | Extern | QOpenGLWidget |
| Qt6::Gui | Extern | QGuiApplication, QPlatformNativeInterface |
| BasicLogger | Intern | Logging |
| libEGL (Linux) | System | EGL VSync auf Wayland |
| libdl (Linux) | System | Dynamisches Laden von libEGL |

---

## 3. API

### 3.1 Konstruktor / Destruktor

```cpp
explicit VisualizerWidget(QWidget* parent = nullptr);
~VisualizerWidget() override;
```

### 3.2 Öffentliche Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `setClearColor(r, g, b, a)` | `float` × 4 | `void` | Setzt Hintergrundfarbe |
| `setVSync(enabled)` | `bool` | `void` | Aktiviert/deaktiviert VSync zur Laufzeit |

### 3.3 Geschützte Virtuelle Methoden (Override)

| Methode | Beschreibung |
|---------|--------------|
| `initializeGL()` | Einmalig bei Context-Erstellung |
| `resizeGL(w, h)` | Bei Größenänderung |
| `paintGL()` | Jeden Frame (Rendering) |

---

## 4. Qt6-Konzepte

### 4.1 Doppelte Vererbung

```cpp
class VisualizerWidget : public QOpenGLWidget, protected QOpenGLFunctions
```

| Basisklasse | Zweck |
|-------------|-------|
| **QOpenGLWidget** | Widget mit OpenGL-Context |
| **QOpenGLFunctions** | Plattformübergreifende GL-Funktionen |

### 4.2 QSurfaceFormat

Im Konstruktor wird das OpenGL-Format konfiguriert:

```cpp
QSurfaceFormat format;
format.setVersion(3, 3);                   // OpenGL 3.3
format.setProfile(QSurfaceFormat::CoreProfile);
format.setSwapInterval(0);                 // VSync initial OFF
format.setSamples(4);                      // 4x MSAA
setFormat(format);
```

| Einstellung | Wert | Bedeutung |
|-------------|------|-----------|
| Version | 3.3 | Mindestversion für Core Profile |
| Profile | Core | Kein deprecated OpenGL |
| SwapInterval | 0 | VSync initial aus (Runtime-Steuerung) |
| Samples | 4 | Anti-Aliasing |
| DepthBuffer | 24 bit | Für 3D-Rendering |

> **Hinweis:** VSync wird zur Laufzeit über `setVSync()` gesteuert, nicht über QSurfaceFormat.

---

## 5. VSync-Steuerung

### 5.1 Übersicht

Ab Version 2.0.0 unterstützt VisualizerWidget **Runtime VSync-Steuerung** über plattformspezifische APIs. Dies ermöglicht das Umschalten zwischen Frame-Modi ohne Widget-Neustart.

### 5.2 Plattform-APIs

| Platform | Display Server | API | Funktion |
|----------|----------------|-----|----------|
| **Windows** | - | WGL | `wglSwapIntervalEXT(interval)` |
| **Linux** | Wayland | EGL | `eglSwapInterval(display, interval)` |
| **Linux** | X11 + EGL | EGL | `eglSwapInterval(display, interval)` |
| **Linux** | X11 + Mesa | GLX | `glXSwapIntervalMESA(interval)` |
| **Linux** | X11 + NVIDIA | GLX | `glXSwapIntervalEXT(display, drawable, interval)` |
| **macOS** | - | CGL | `CGLSetParameter(ctx, kCGLCPSwapInterval, &val)` |

### 5.3 Linux-Fallback-Strategie

```
┌─────────────────────────────────────────┐
│ 1. EGL (Wayland + moderne X11)          │
│    nativeResourceForIntegration()       │
│    dlopen("libEGL.so.1")                │
└────────────────┬────────────────────────┘
                 │ Fallback
                 ▼
┌─────────────────────────────────────────┐
│ 2. Mesa GLX (X11, einfache API)         │
│    glXSwapIntervalMESA(interval)        │
└────────────────┬────────────────────────┘
                 │ Fallback
                 ▼
┌─────────────────────────────────────────┐
│ 3. GLX EXT (X11, NVIDIA)                │
│    glXSwapIntervalEXT(dpy, drw, int)    │
└─────────────────────────────────────────┘
```

### 5.4 Verwendung

```cpp
// VSync aktivieren (für VSync FrameMode)
visualizer->setVSync(true);

// VSync deaktivieren (für Limited/Unlimited FrameMode)
visualizer->setVSync(false);
```

### 5.5 Log-Ausgaben

```
[INFO] VSync ENABLED (wglSwapIntervalEXT)           // Windows
[INFO] VSync ENABLED (eglSwapInterval - wayland)    // Linux Wayland
[INFO] VSync ENABLED (eglSwapInterval - xcb)        // Linux X11 + EGL
[INFO] VSync ENABLED (glXSwapIntervalMESA)          // Linux X11 + Mesa
[INFO] VSync ENABLED (glXSwapIntervalEXT)           // Linux X11 + NVIDIA
[INFO] VSync ENABLED (CGLSetParameter)              // macOS
```

---

## 6. OpenGL-Integration

### 6.1 Context-Lifecycle

```
Widget erstellt
      │
      ▼
show() aufgerufen
      │
      ▼
┌─────────────────┐
│ initializeGL()  │ ◄── OpenGL-Ressourcen erstellen
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  resizeGL(w,h)  │ ◄── Initiale Größe
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   paintGL()     │ ◄── Rendering-Loop
└────────┬────────┘
         │
    [swapBuffers]  ◄── VSync wartet hier (wenn aktiviert)
         │
         ▼
    Nächster Frame
```

### 6.2 initializeGL()

Wird **einmal** aufgerufen wenn der OpenGL-Context bereit ist:

```cpp
void VisualizerWidget::initializeGL()
{
    // WICHTIG: Muss als erstes aufgerufen werden!
    initializeOpenGLFunctions();
    
    // OpenGL-State konfigurieren
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    
    // Shader laden, VAOs/VBOs erstellen...
}
```

### 6.3 paintGL()

Wird **jeden Frame** aufgerufen:

```cpp
void VisualizerWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Shader binden
    // Uniforms setzen
    // Geometrie zeichnen
    
    // swapBuffers() wird automatisch von Qt aufgerufen!
}
```

---

## 7. Rendering-Flow

### 7.1 Timer-basiertes Rendering (ab v2.0.0)

```
Application::run()
      │
      ├── QTimer (PreciseTimer)
      │         │
      │         └── timeout Signal
      │                  │
      │                  ▼
      │         MainWindow::requestRender()
      │                  │
      │                  └── DockManager::requestRenderAll()
      │                           │
      │                           └── VisualizerWidget::update()
      │                                    │
      │                                    ▼
      │                              [Paint Event]
      │                                    │
      │                                    ▼
      │                              paintGL()
      │                                    │
      │                                    ▼
      │                              swapBuffers()
      │                                    │
      │                              [VSync wenn aktiv]
      │
      └── exec() Event-Loop
```

### 7.2 Frame-Modi und Timer-Intervalle

| FrameMode | VSync | Timer Intervall | Erwartete FPS |
|-----------|-------|-----------------|---------------|
| **Limited** | OFF | 16ms | ~60 |
| **Unlimited** | OFF | 0ms | 500-1000+ |
| **VSync** | ON | 1ms | Monitor Hz |

### 7.3 update() vs repaint()

| Methode | Verhalten | Verwendung |
|---------|-----------|------------|
| `update()` | Async, coalesced | ✅ Empfohlen |
| `repaint()` | Sync, sofort | ❌ Vermeiden |

---

## 8. Thread-Sicherheit

**Nicht thread-safe.**

Alle OpenGL-Aufrufe und Widget-Methoden müssen vom Main-Thread erfolgen. Der OpenGL-Context ist an den erstellenden Thread gebunden.

---

## 9. Erweiterung

### 9.1 Audio-Daten hinzufügen

```cpp
// In VisualizerWidget.hpp:
void setAudioData(const float* spectrum, int size);

// In VisualizerWidget.cpp:
void VisualizerWidget::setAudioData(const float* spectrum, int size)
{
    // Daten kopieren für nächsten Frame
    m_spectrum.assign(spectrum, spectrum + size);
    update();  // Repaint anfordern
}
```

### 9.2 Shader hinzufügen

```cpp
// In initializeGL():
m_pShaderProgram = std::make_unique<QOpenGLShaderProgram>();
m_pShaderProgram->addShaderFromSourceCode(
    QOpenGLShader::Vertex, vertexShaderSource);
m_pShaderProgram->addShaderFromSourceCode(
    QOpenGLShader::Fragment, fragmentShaderSource);
m_pShaderProgram->link();
```

### 9.3 VAO/VBO für Geometrie

```cpp
// In initializeGL():
glGenVertexArrays(1, &m_vao);
glGenBuffers(1, &m_vbo);

glBindVertexArray(m_vao);
glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

// Vertex-Attribute definieren...
glBindVertexArray(0);
```

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.0.0** | **2025-12-29** | **Runtime VSync-Steuerung (Windows/Linux/macOS), Wayland-Support, EGL-Integration** |
| 1.0.0 | 2025-12-28 | Initial: QOpenGLWidget, VSync, Rainbow-Demo |
