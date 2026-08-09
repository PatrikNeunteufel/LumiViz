# Visualizers — OpenGL Audio Visualization

> **Version:** 1.0.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Visualizers  
> **Dateien:** IVisualizer.hpp, VisualizerBase.hpp, VisualizerBase.cpp, *Visualizer.hpp/cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6, OpenGL, GLAD, EventBus  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [VisualizerBase API](#3-visualizerbase-api)
4. [Visualizer erstellen](#4-visualizer-erstellen)
5. [Audio-Daten](#5-audio-daten)
6. [OpenGL-Tipps](#6-opengl-tipps)
7. [Existierende Visualizer](#7-existierende-visualizer)
8. [Best Practices](#8-best-practices)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

### 1.1 Zweck

**Visualizers** sind OpenGL-basierte Audio-Visualisierungen, die in Echtzeit auf Musik reagieren. Sie empfangen FFT-Spektrum und Waveform-Daten über den EventBus und rendern Animationen.

### 1.2 Features

| Feature | Beschreibung |
|---------|-------------|
| **Self-Registration** | Automatische Registrierung beim Start |
| **Hot-Swap** | Visualizer zur Laufzeit wechseln |
| **Thread-Safe Audio** | Audio-Daten sicher vom Analyzer-Thread |
| **Resize-Handling** | Automatische Viewport-Anpassung |
| **Lifecycle** | Initialize → Render → Cleanup |

### 1.3 Datenfluss

```
┌─────────────┐     ┌──────────────┐     ┌─────────────────┐
│ AudioPlayer │────►│AudioAnalyzer │────►│   EventBus      │
│             │     │ (FFT/Beat)   │     │                 │
└─────────────┘     └──────────────┘     └────────┬────────┘
                                                  │
                                                  │ AudioDataEvent
                                                  ▼
                    ┌──────────────────────────────────────────┐
                    │            VisualizerWidget              │
                    │  (QOpenGLWidget)                         │
                    ├──────────────────────────────────────────┤
                    │                                          │
                    │  ┌────────────────────────────────────┐  │
                    │  │         IVisualizer                │  │
                    │  │                                    │  │
                    │  │  updateSpectrum(spectrum, count)   │  │
                    │  │  render(deltaTime)                 │  │
                    │  │                                    │  │
                    │  └────────────────────────────────────┘  │
                    │                                          │
                    └──────────────────────────────────────────┘
```

---

## 2. Architektur

### 2.1 Klassendiagramm

```
                    ┌─────────────────────┐
                    │    IVisualizer      │  (Interface)
                    ├─────────────────────┤
                    │ + visualizerId()    │
                    │ + visualizerName()  │
                    │ + initialize()      │
                    │ + render()          │
                    │ + resize()          │
                    │ + cleanup()         │
                    │ + updateSpectrum()  │
                    │ + updateWaveform()  │
                    └──────────┬──────────┘
                               │
                               ▼
         ┌─────────────────────────────────────────┐
         │            VisualizerBase               │
         ├─────────────────────────────────────────┤
         │ # m_id, m_name, m_description           │
         │ # m_initialized : bool                  │
         │ # m_viewportSize : QSize                │
         │ # m_spectrum, m_waveform : vector<f>    │
         ├─────────────────────────────────────────┤
         │ # onInitialize() [pure virtual]         │
         │ # onRender(deltaTime) [pure virtual]    │
         │ # onResize(size) [pure virtual]         │
         │ # onCleanup() [pure virtual]            │
         │ # getSpectrum() : vector<float>         │
         │ # getWaveform() : vector<float>         │
         │ # hasNewAudioData() : bool              │
         └──────────┬──────────────────────────────┘
                    │
    ┌───────────────┼───────────────┬───────────────┐
    ▼               ▼               ▼               ▼
┌─────────┐   ┌─────────┐   ┌─────────────┐   ┌──────────┐
│ Pulsing │   │ Spectrum│   │  Waveform   │   │ Particle │
│Visualizer│  │ Bars    │   │ Visualizer  │   │Visualizer│
└─────────┘   └─────────┘   └─────────────┘   └──────────┘
```

### 2.2 Integration mit VisualizerWidget

```cpp
// VisualizerWidget besitzt IVisualizer
class VisualizerWidget : public QOpenGLWidget {
    std::unique_ptr<IVisualizer> m_visualizer;
    
    void paintGL() override {
        if (m_visualizer) {
            m_visualizer->render(m_deltaTime);
        }
    }
    
    void setVisualizer(const QString& id) {
        // Cleanup old
        if (m_visualizer) m_visualizer->cleanup();
        
        // Create new from registry
        m_visualizer = VisualizerRegistry::instance()
            .create(id.toStdString(), m_services);
        
        // Initialize in GL context
        makeCurrent();
        m_visualizer->initialize();
        m_visualizer->resize(size());
        doneCurrent();
    }
};
```

---

## 3. VisualizerBase API

### 3.1 Identifikation

```cpp
[[nodiscard]] QString visualizerId() const;          // z.B. "pulsing"
[[nodiscard]] QString visualizerName() const;        // z.B. "Pulsing Circles"
[[nodiscard]] QString visualizerDescription() const; // z.B. "Audio-reactive..."
```

### 3.2 Lifecycle (final, nicht überschreiben)

```cpp
void initialize();            // Ruft onInitialize()
void render(float deltaTime); // Ruft onRender()
void resize(const QSize&);    // Ruft onResize()
void cleanup();               // Ruft onCleanup()

[[nodiscard]] bool isInitialized() const;
```

### 3.3 Override Points (müssen implementiert werden)

```cpp
virtual void onInitialize() = 0;           // Setup shaders, buffers
virtual void onRender(float deltaTime) = 0; // Draw frame
virtual void onResize(const QSize& size) = 0; // Update projection
virtual void onCleanup() = 0;              // Free resources
```

### 3.4 Audio-Daten (thread-safe)

```cpp
// Aufgerufen von VisualizerWidget (aus EventBus Callback)
void updateSpectrum(const float* spectrum, int count);
void updateWaveform(const float* waveform, int count);

// In onRender() verwenden:
std::vector<float> getSpectrum() const;  // Thread-safe Kopie
std::vector<float> getWaveform() const;  // Thread-safe Kopie
bool hasNewAudioData();                   // Wurde aktualisiert?
```

### 3.5 Viewport-Helpers

```cpp
[[nodiscard]] QSize viewportSize() const;
[[nodiscard]] int width() const;
[[nodiscard]] int height() const;
[[nodiscard]] float aspectRatio() const;
```

---

## 4. Visualizer erstellen

### 4.1 Header-Datei

```cpp
// PulsingVisualizer.hpp
#pragma once

#include "VisualizerBase.hpp"

class PulsingVisualizer : public VisualizerBase
{
public:
    explicit PulsingVisualizer(ServiceContainer& services);
    ~PulsingVisualizer() override = default;

protected:
    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    // OpenGL resources
    unsigned int m_shaderProgram = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    
    // Animation state
    float m_time = 0.0f;
    float m_beatPulse = 0.0f;
};
```

### 4.2 Implementation

```cpp
// PulsingVisualizer.cpp
#include "PulsingVisualizer.hpp"
#include "services/VisualizerRegistry.hpp"

#include <glad/glad.h>
#include <cmath>

// Vertex Shader
static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment Shader
static const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform float uTime;
uniform float uBass;
uniform vec2 uResolution;

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(uv, center);
    
    // Pulsing ring
    float ring = smoothstep(0.3 + uBass * 0.2, 0.31 + uBass * 0.2, dist) *
                 smoothstep(0.35 + uBass * 0.2, 0.34 + uBass * 0.2, dist);
    
    // Color cycling
    vec3 color = vec3(
        0.5 + 0.5 * sin(uTime),
        0.5 + 0.5 * sin(uTime + 2.094),
        0.5 + 0.5 * sin(uTime + 4.188)
    );
    
    FragColor = vec4(color * ring, 1.0);
}
)";

PulsingVisualizer::PulsingVisualizer(ServiceContainer& services)
    : VisualizerBase("pulsing", 
                     QObject::tr("Pulsing Circles"),
                     QObject::tr("Audio-reactive pulsing circles"))
{
    Q_UNUSED(services);
}

void PulsingVisualizer::onInitialize()
{
    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Fullscreen quad
    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void PulsingVisualizer::onRender(float deltaTime)
{
    m_time += deltaTime;
    
    // Get audio data
    auto spectrum = getSpectrum();
    float bass = 0.0f;
    if (spectrum.size() > 10) {
        // Average of first 10 bins (bass frequencies)
        for (int i = 0; i < 10; i++) {
            bass += spectrum[i];
        }
        bass /= 10.0f;
    }
    
    // Smooth beat pulse
    m_beatPulse = m_beatPulse * 0.9f + bass * 0.1f;
    
    // Render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(m_shaderProgram);
    
    glUniform1f(glGetUniformLocation(m_shaderProgram, "uTime"), m_time);
    glUniform1f(glGetUniformLocation(m_shaderProgram, "uBass"), m_beatPulse);
    glUniform2f(glGetUniformLocation(m_shaderProgram, "uResolution"), 
                static_cast<float>(width()), 
                static_cast<float>(height()));
    
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PulsingVisualizer::onResize(const QSize& size)
{
    glViewport(0, 0, size.width(), size.height());
}

void PulsingVisualizer::onCleanup()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteProgram(m_shaderProgram);
}

// Self-Registration am Ende:
REGISTER_VISUALIZER("pulsing", "Pulsing Circles", 
                    "Audio-reactive pulsing circles", 
                    PulsingVisualizer)
```

---

## 5. Audio-Daten

### 5.1 Spectrum-Daten

```cpp
// FFT-Spectrum: 512-2048 Frequency Bins
// Index 0 = niedrigste Frequenz (~20 Hz)
// Index N = höchste Frequenz (~20 kHz)
// Werte: 0.0 - 1.0 (normalisiert)

auto spectrum = getSpectrum();

// Bass (20-250 Hz) ≈ erste 10-50 Bins
float bass = 0.0f;
for (int i = 0; i < 20; i++) {
    bass += spectrum[i];
}
bass /= 20.0f;

// Mitten (250-4000 Hz)
float mids = 0.0f;
for (int i = 20; i < 200; i++) {
    mids += spectrum[i];
}
mids /= 180.0f;

// Höhen (4000-20000 Hz)
float highs = 0.0f;
for (int i = 200; i < spectrum.size(); i++) {
    highs += spectrum[i];
}
highs /= (spectrum.size() - 200);
```

### 5.2 Waveform-Daten

```cpp
// Raw Audio Samples: -1.0 bis 1.0
// Typisch 512-2048 Samples

auto waveform = getWaveform();

// Für Oszilloskop-artige Darstellung:
for (size_t i = 0; i < waveform.size(); i++) {
    float x = static_cast<float>(i) / waveform.size() * 2.0f - 1.0f;
    float y = waveform[i];
    // Draw point at (x, y)
}
```

### 5.3 Smoothing

```cpp
class MyVisualizer : public VisualizerBase {
    std::vector<float> m_smoothedSpectrum;
    float m_smoothFactor = 0.8f;  // 0.0 = instant, 1.0 = frozen
    
    void onRender(float deltaTime) override {
        auto spectrum = getSpectrum();
        
        // Resize if needed
        if (m_smoothedSpectrum.size() != spectrum.size()) {
            m_smoothedSpectrum = spectrum;
        }
        
        // Apply smoothing
        for (size_t i = 0; i < spectrum.size(); i++) {
            m_smoothedSpectrum[i] = m_smoothedSpectrum[i] * m_smoothFactor 
                                  + spectrum[i] * (1.0f - m_smoothFactor);
        }
        
        // Use m_smoothedSpectrum for rendering
    }
};
```

---

## 6. OpenGL-Tipps

### 6.1 Shader-Fehler prüfen

```cpp
void checkShaderError(unsigned int shader) {
    int success;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, log);
        qWarning() << "Shader compile error:" << log;
    }
}
```

### 6.2 Resource-Cleanup

```cpp
void PulsingVisualizer::onCleanup()
{
    // WICHTIG: Alle OpenGL-Ressourcen freigeben!
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
    
    m_vao = m_vbo = m_shaderProgram = 0;
}
```

### 6.3 Framebuffer für Effekte

```cpp
// Für Post-Processing, Blur, etc.
unsigned int m_fbo, m_fboTexture;

void onInitialize() {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    
    glGenTextures(1, &m_fboTexture);
    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D, m_fboTexture, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
```

---

## 7. Existierende Visualizer

### 7.1 PulsingVisualizer

- **ID:** `pulsing`
- **Effekt:** Pulsierende Kreise die auf Bass reagieren
- **Shader:** Fragment-Shader mit SDF-Ringen

### 7.2 (Geplant) SpectrumBars

- **ID:** `spectrum-bars`
- **Effekt:** Klassische Spektrum-Balken
- **Features:** Spiegelung, Glow, 3D-Perspektive

### 7.3 (Geplant) WaveformScope

- **ID:** `waveform`
- **Effekt:** Oszilloskop-artige Wellenform
- **Features:** Stereo, Lissajous, Trigger

---

## 8. Best Practices

### 8.1 Performance

```cpp
// ❌ Schlecht: Shader in jedem Frame kompilieren
void onRender() {
    auto shader = compileShader(src);  // LANGSAM!
}

// ✅ Gut: Einmal in onInitialize
void onInitialize() {
    m_shader = compileShader(src);
}
```

### 8.2 Audio-Daten Caching

```cpp
// ❌ Schlecht: Mehrfach kopieren
void onRender() {
    auto s1 = getSpectrum();  // Kopie + Lock
    auto s2 = getSpectrum();  // Nochmal!
}

// ✅ Gut: Einmal kopieren
void onRender() {
    auto spectrum = getSpectrum();  // Eine Kopie
    float bass = computeBass(spectrum);
    float mids = computeMids(spectrum);
}
```

### 8.3 Cleanup nicht vergessen

```cpp
// ❌ Schlecht: Memory Leak
~MyVisualizer() {
    // onCleanup() nicht aufgerufen!
}

// ✅ Gut: cleanup() wird von VisualizerWidget aufgerufen
// VisualizerWidget::setVisualizer() ruft cleanup() auf altem Visualizer
```

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-31** | **Initial: VisualizerBase, PulsingVisualizer** |
