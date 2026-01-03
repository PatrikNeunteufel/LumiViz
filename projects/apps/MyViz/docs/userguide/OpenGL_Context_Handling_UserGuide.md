# OpenGL Context Handling — Benutzerhandbuch

> **Version:** 1.0.0  
> **Datum:** 2026-01-03  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [OpenGL_Context_Handling_UserGuide.md](../../en/guides/OpenGL_Context_Handling_UserGuide.md)

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Wie funktioniert das Context-Tracking?](#4-wie-funktioniert-das-context-tracking)
5. [Wie passe ich den Header an?](#5-wie-passe-ich-den-header-an)
6. [Wie implementiere ich onInitialize?](#6-wie-implementiere-ich-oninitialize)
7. [Wie implementiere ich onRender?](#7-wie-implementiere-ich-onrender)
8. [Wie sichere ich Hilfsfunktionen ab?](#8-wie-sichere-ich-hilfsfunktionen-ab)
9. [Stolpersteine und Lösungen](#9-stolpersteine-und-lösungen)
10. [Troubleshooting](#10-troubleshooting)
11. [Siehe auch](#11-siehe-auch)
12. [Changelog](#changelog)

---

## 1. Überblick

Dieses Handbuch beschreibt das **Context-Tracking Pattern** für OpenGL-basierte Visualizer in Qt-ADS (Advanced Docking System) Umgebungen.

### Features

- Crash-Vermeidung beim Abdocken von Panels
- Automatische Re-Initialisierung bei Kontext-Wechsel
- Graceful Degradation bei fehlendem Kontext
- Vollständige Code-Templates zum Kopieren

### Problemstellung

Bei Verwendung von Qt-ADS mit QOpenGLWidget-Visualizern tritt ein kritischer Crash auf, wenn Dock-Widgets abgedockt werden. Die Ursache: OpenGL-Ressourcen sind an einen spezifischen Kontext gebunden und werden beim Erstellen eines Floating-Widgets ungültig.

---

## 2. Voraussetzungen

### Systemanforderungen

- [ ] Qt 6.5+ mit OpenGL-Modulen
- [ ] Qt-ADS (Advanced Docking System)
- [ ] OpenGL 3.3+ Core Profile
- [ ] Compiler: MSVC 2022 / GCC 11+ / Clang 14+

### Architektur-Voraussetzungen

- [ ] Visualizer erbt von `VisualizerBase`
- [ ] Verwendung von `QOpenGLWidget` als Basis
- [ ] Implementierung der virtuellen Methoden `onInitialize()`, `onRender()`, `onResize()`, `onCleanup()`

---

## 3. Schnellstart

Füge diese drei Änderungen zu jedem neuen Visualizer hinzu:

**1. Header — Context-Tracking Member:**

```cpp
// Forward Declaration
class QOpenGLContext;

class YourVisualizer : public VisualizerBase
{
private:
    QOpenGLContext* m_lastContext = nullptr;
};
```

**2. onInitialize() — Kontext speichern:**

```cpp
void YourVisualizer::onInitialize()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) return;
    
    // ... Shader, VAO, VBO erstellen ...
    
    m_lastContext = ctx;  // Am Ende speichern!
}
```

**3. onRender() — Kontext prüfen:**

```cpp
void YourVisualizer::onRender(float deltaTime)
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) return;
    
    if (m_lastContext != ctx)
    {
        m_shader.reset();
        m_vao.reset();
        onInitialize();
        m_lastContext = ctx;
    }
    
    // ... Rendering ...
}
```

---

## 4. Wie funktioniert das Context-Tracking?

### 4.1 Das Problem

OpenGL-Ressourcen sind **kontext-spezifisch**:

```
┌─────────────────────────────────────────────────────────────┐
│ Ursprüngliches MainWindow                                   │
│ ┌─────────────────────────────────────────────────────────┐ │
│ │ OpenGL Context A                                        │ │
│ │ ├── Shader Program (ID: 1)         ← gültig             │ │
│ │ ├── VAO (ID: 1)                    ← gültig             │ │
│ │ └── Uniform Locations              ← gültig             │ │
│ └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘

        │ Panel abdocken (Undocking)
        ▼

┌─────────────────────────────────────────────────────────────┐
│ Neues Floating Window                                       │
│ ┌─────────────────────────────────────────────────────────┐ │
│ │ OpenGL Context B (NEU!)                                 │ │
│ │ ├── Shader Program (ID: 1)         ← UNGÜLTIG!          │ │
│ │ ├── VAO (ID: 1)                    ← UNGÜLTIG!          │ │
│ │ └── Uniform Locations (-1)         ← CRASH!             │ │
│ └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 Die Lösung

Das Context-Tracking Pattern erkennt Kontext-Wechsel und re-initialisiert automatisch:

```
┌────────────────────────────────────────────────────────────┐
│ onRender(deltaTime)                                        │
├────────────────────────────────────────────────────────────┤
│ 1. ctx = currentContext()                                  │
│ 2. if (!ctx) return;                     ← Null-Check      │
│ 3. if (m_lastContext != ctx)             ← Wechsel?        │
│    ├── Alte Ressourcen freigeben                           │
│    ├── onInitialize() aufrufen                             │
│    └── m_lastContext = ctx                                 │
│ 4. Normales Rendering...                                   │
└────────────────────────────────────────────────────────────┘
```

---

## 5. Wie passe ich den Header an?

### 5.1 Forward Declaration hinzufügen

Füge am Anfang des Headers nach den Includes hinzu:

```cpp
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>

// Forward Declaration für Context-Tracking (PFLICHT!)
class QOpenGLContext;
```

### 5.2 Member-Variable hinzufügen

Füge im `private`-Bereich der Klasse hinzu:

```cpp
private:
    // OpenGL Resources
    std::unique_ptr<QOpenGLShaderProgram> m_shader;
    std::unique_ptr<QOpenGLBuffer> m_vertexBuffer;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
    int m_vertexCount = 0;
    
    // Uniform Locations
    int m_uniformAspect = -1;
    int m_uniformTime = -1;
    // ... weitere Uniforms ...
    
    // =========================================================================
    // OpenGL Context Tracking (PFLICHT für Qt-ADS Kompatibilität!)
    // =========================================================================
    
    QOpenGLContext* m_lastContext = nullptr;
```

---

## 6. Wie implementiere ich onInitialize?

### 6.1 Vollständiges Template

```cpp
void YourVisualizer::onInitialize()
{
    BasicLogger::logInfo("YourVisualizer::onInitialize() - START");
    
    // =========================================================================
    // KRITISCH: Kontext-Check VOR functions()-Aufruf
    // =========================================================================
    
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        BasicLogger::logError("No OpenGL context (ctx is null)!");
        return;
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (!gl)
    {
        BasicLogger::logError("No OpenGL functions!");
        return;
    }
    
    // =========================================================================
    // Shader erstellen
    // =========================================================================
    
    m_shader = std::make_unique<QOpenGLShaderProgram>();
    
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER))
    {
        BasicLogger::logError("Vertex shader failed: " + m_shader->log().toStdString());
        return;
    }
    
    if (!m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAGMENT_SHADER))
    {
        BasicLogger::logError("Fragment shader failed: " + m_shader->log().toStdString());
        return;
    }
    
    if (!m_shader->link())
    {
        BasicLogger::logError("Shader link failed: " + m_shader->log().toStdString());
        return;
    }
    
    // =========================================================================
    // Uniform Locations abrufen
    // =========================================================================
    
    m_uniformAspect = m_shader->uniformLocation("uAspect");
    m_uniformTime = m_shader->uniformLocation("uTime");
    // ... weitere Uniforms ...
    
    // =========================================================================
    // VAO und VBO erstellen
    // =========================================================================
    
    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    if (!m_vao->create())
    {
        BasicLogger::logError("VAO creation failed");
        return;
    }
    
    m_vertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    if (!m_vertexBuffer->create())
    {
        BasicLogger::logError("VBO creation failed");
        return;
    }
    
    // Vertex-Daten hochladen...
    
    // =========================================================================
    // KRITISCH: Kontext speichern für Change-Detection
    // =========================================================================
    
    m_lastContext = ctx;
    
    BasicLogger::logInfo("YourVisualizer::onInitialize() - SUCCESS");
}
```

### 6.2 Wichtige Punkte

| Schritt | Warum? |
|---------|--------|
| `currentContext()` zuerst | Kann `nullptr` sein während Undocking |
| `ctx->functions()` prüfen | Sicherstellen dass GL-Aufrufe möglich |
| `m_lastContext = ctx` am Ende | Für spätere Change-Detection |

---

## 7. Wie implementiere ich onRender?

### 7.1 Vollständiges Template

```cpp
void YourVisualizer::onRender(float deltaTime)
{
    // =========================================================================
    // KRITISCH: Kontext-Check VOR functions()-Aufruf
    // =========================================================================
    
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;  // Kein Kontext → kein Rendering
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (!gl)
    {
        return;
    }
    
    // =========================================================================
    // KRITISCH: Context-Change-Detection
    // =========================================================================
    
    if (m_lastContext != ctx)
    {
        BasicLogger::logInfo("OpenGL context changed, reinitializing...");
        
        // Alte Ressourcen freigeben (im neuen Kontext ungültig)
        m_shader.reset();
        m_vertexBuffer.reset();
        m_vao.reset();
        m_vertexCount = 0;
        
        // Im neuen Kontext neu initialisieren
        onInitialize();
        
        // Neuen Kontext speichern
        m_lastContext = ctx;
        
        // Prüfen ob Re-Initialisierung erfolgreich war
        if (!m_shader || !m_vao)
        {
            BasicLogger::logWarning("Reinitialization failed");
            return;
        }
    }
    
    // =========================================================================
    // Ressourcen-Check vor Verwendung
    // =========================================================================
    
    if (!m_shader || !m_vao || m_vertexCount == 0)
    {
        return;
    }
    
    // =========================================================================
    // Normales Rendering
    // =========================================================================
    
    gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_shader->bind();
    m_vao->bind();
    
    // Uniforms setzen
    gl->glUniform1f(m_uniformAspect, aspectRatio());
    // ... weitere Uniforms ...
    
    // Zeichnen
    gl->glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    
    m_vao->release();
    m_shader->release();
}
```

### 7.2 Reihenfolge der Checks

```
1. Context null?           → return (während Undocking)
2. Functions null?         → return (sollte nicht passieren)
3. Context gewechselt?     → Re-Initialisierung
4. Ressourcen vorhanden?   → return wenn nicht
5. Rendering              → normal fortfahren
```

---

## 8. Wie sichere ich Hilfsfunktionen ab?

### 8.1 onResize absichern

```cpp
void YourVisualizer::onResize(const QSize& size)
{
    // KRITISCH: Kontext-Check auch hier!
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (gl)
    {
        gl->glViewport(0, 0, size.width(), size.height());
    }
}
```

### 8.2 Andere OpenGL-Hilfsfunktionen

Alle Funktionen die `gl->` aufrufen müssen abgesichert werden:

```cpp
void YourVisualizer::uploadGradientUniforms()
{
    // KRITISCH: Kontext-Check
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        return;
    }
    
    QOpenGLFunctions* gl = ctx->functions();
    if (!gl)
    {
        return;
    }
    
    // Jetzt sicher OpenGL-Calls machen
    gl->glUniform4f(m_uniformColor, r, g, b, a);
}
```

### 8.3 Funktionen mit Viewport-Zugriff

Bei Funktionen die den Viewport lesen, verwende einen Fallback:

```cpp
void YourVisualizer::buildVertices()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    float pixelHeight = 0.002f;  // Default-Fallback
    
    if (ctx)
    {
        QOpenGLFunctions* gl = ctx->functions();
        if (gl)
        {
            GLint viewport[4];
            gl->glGetIntegerv(GL_VIEWPORT, viewport);
            pixelHeight = viewport[3] > 0 
                ? 2.0f / static_cast<float>(viewport[3]) 
                : 0.002f;
        }
    }
    
    // Verwende pixelHeight...
}
```

---

## 9. Stolpersteine und Lösungen

### 9.1 Vergessene Forward Declaration

**Problem:** Compiler-Fehler `unknown type name 'QOpenGLContext'`

**Ursache:** Forward Declaration fehlt im Header.

**Lösung:** Füge hinzu:

```cpp
class QOpenGLContext;
```

### 9.2 Kontext nicht gespeichert

**Problem:** Endlosschleife bei Re-Initialisierung.

**Ursache:** `m_lastContext = ctx;` fehlt in `onInitialize()`.

**Lösung:** Stelle sicher dass am Ende von `onInitialize()` steht:

```cpp
m_lastContext = ctx;
```

### 9.3 Ressourcen nicht zurückgesetzt

**Problem:** Crash trotz Context-Tracking.

**Ursache:** Alte Ressourcen nicht freigegeben vor Re-Init.

**Lösung:** Vor `onInitialize()` aufrufen:

```cpp
m_shader.reset();
m_vertexBuffer.reset();
m_vao.reset();
m_vertexCount = 0;
```

### 9.4 Hilfsfunktion nicht abgesichert

**Problem:** Crash in einer Hilfsfunktion, nicht in `onRender()`.

**Ursache:** Hilfsfunktion macht OpenGL-Calls ohne Kontext-Check.

**Lösung:** Jeden OpenGL-Aufruf absichern, auch in Hilfsfunktionen.

---

## 10. Troubleshooting

### Checkliste bei Crashes

- [ ] Forward Declaration `class QOpenGLContext;` im Header?
- [ ] Member `QOpenGLContext* m_lastContext = nullptr;` vorhanden?
- [ ] `currentContext()` Check am Anfang von `onInitialize()`?
- [ ] `m_lastContext = ctx;` am Ende von `onInitialize()`?
- [ ] Context-Change-Detection in `onRender()`?
- [ ] Ressourcen-Reset vor Re-Initialisierung?
- [ ] Alle Hilfsfunktionen mit OpenGL-Calls abgesichert?

### Häufige Crash-Adressen

| Adresse | Bedeutung | Lösung |
|---------|-----------|--------|
| `0x00000027` | Null-Pointer + kleiner Offset | `currentContext()` Check fehlt |
| `0x00000028` | Null-Pointer + kleiner Offset | `ctx->functions()` Check fehlt |
| `0xFFFFFFFF...` | -1 (ungültige Uniform-Location) | Context-Tracking fehlt |
| Adresse in Qt6OpenGL | Ungültige Ressource | Re-Initialisierung fehlt |

### Debug-Logging aktivieren

Füge Logging hinzu um den Kontext-Wechsel zu beobachten:

```cpp
if (m_lastContext != ctx)
{
    BasicLogger::logInfo("Context changed: " + 
        std::to_string(reinterpret_cast<uintptr_t>(m_lastContext)) + " -> " +
        std::to_string(reinterpret_cast<uintptr_t>(ctx)));
}
```

### Test-Szenario

1. App starten
2. Visualizer aktivieren
3. Player-Panel abdocken (volle Breite)
4. Config-Panel abdocken (volle Höhe)
5. Floating-Widget wieder andocken
6. Wiederholen mit verschiedenen Visualizern

---

## 11. Siehe auch

- [VisualizerBase.hpp](../../include/visualizers/VisualizerBase.hpp) — Basis-Klasse für Visualizer
- [PulsingVisualizer.cpp](../../src/visualizers/PulsingVisualizer.cpp) — Referenz-Implementierung
- [WaveformVisualizer.cpp](../../src/visualizers/WaveformVisualizer.cpp) — Referenz-Implementierung
- [Qt-ADS Documentation](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System) — Docking-System Dokumentation

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-03** | **Initial: Vollständige Dokumentation des Context-Tracking Patterns** |
