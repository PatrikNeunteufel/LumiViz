# VisualizerWidget — OpenGL Visualisierungs-Widget

> **Version:** 1.0.0  
> **Datum:** 2025-12-28  
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
5. [OpenGL-Integration](#5-opengl-integration)
6. [Rendering-Flow](#6-rendering-flow)
7. [Thread-Sicherheit](#7-thread-sicherheit)
8. [Erweiterung](#8-erweiterung)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

### 1.1 Zweck

VisualizerWidget ist ein QOpenGLWidget, das hardware-beschleunigte Audio-Visualisierungen rendert. Es bildet das Herzstück der visuellen Darstellung in MyViz.

### 1.2 Verantwortlichkeiten

- OpenGL-Context-Management
- VSync-Synchronisation über SwapBuffers
- Rendering von Audio-Visualisierungen
- Viewport und Projection-Handling

### 1.3 Nicht-Verantwortlichkeiten

- Audio-Datenverarbeitung (→ AudioEngine)
- FFT-Berechnung (→ AudioEngine)
- Window-Management (→ MainWindow)
- Event-Loop (→ Application)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6::Widgets | Extern | QWidget-Basisklasse |
| Qt6::OpenGL | Extern | QOpenGLFunctions |
| Qt6::OpenGLWidgets | Extern | QOpenGLWidget |
| BasicLogger | Intern | Logging |

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
format.setSwapInterval(1);                 // VSync ON
format.setSamples(4);                      // 4x MSAA
setFormat(format);
```

| Einstellung | Wert | Bedeutung |
|-------------|------|-----------|
| Version | 3.3 | Mindestversion für Core Profile |
| Profile | Core | Kein deprecated OpenGL |
| SwapInterval | 1 | VSync aktiviert |
| Samples | 4 | Anti-Aliasing |
| DepthBuffer | 24 bit | Für 3D-Rendering |

---

## 5. OpenGL-Integration

### 5.1 Context-Lifecycle

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
    [swapBuffers]  ◄── VSync wartet hier
         │
         ▼
    Nächster Frame
```

### 5.2 initializeGL()

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

### 5.3 paintGL()

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

## 6. Rendering-Flow

### 6.1 Aufruf-Kette

```
Application::run()
      │
      ├── processEvents()
      │
      ├── MainWindow::requestRender()
      │         │
      │         └── VisualizerWidget::update()
      │                    │
      │                    ▼
      │              [Paint Event]
      │                    │
      │                    ▼
      │              paintGL()
      │                    │
      │                    ▼
      │              swapBuffers() ◄── VSync
      │
      └── Frame-Timing
```

### 6.2 update() vs repaint()

| Methode | Verhalten | Verwendung |
|---------|-----------|------------|
| `update()` | Async, coalesced | ✅ Empfohlen |
| `repaint()` | Sync, sofort | ❌ Vermeiden |

---

## 7. Thread-Sicherheit

**Nicht thread-safe.**

Alle OpenGL-Aufrufe und Widget-Methoden müssen vom Main-Thread erfolgen. Der OpenGL-Context ist an den erstellenden Thread gebunden.

---

## 8. Erweiterung

### 8.1 Audio-Daten hinzufügen

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

### 8.2 Shader hinzufügen

```cpp
// In initializeGL():
m_pShaderProgram = std::make_unique<QOpenGLShaderProgram>();
m_pShaderProgram->addShaderFromSourceCode(
    QOpenGLShader::Vertex, vertexShaderSource);
m_pShaderProgram->addShaderFromSourceCode(
    QOpenGLShader::Fragment, fragmentShaderSource);
m_pShaderProgram->link();
```

### 8.3 VAO/VBO für Geometrie

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

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-28** | **Initial: QOpenGLWidget, VSync, Rainbow-Demo** |
