# ColorGradientModule — Gradient Color System

> **Version:** 1.0.0  
> **Datum:** 2026-01-02  
> **Typ:** CppModuleDoc  
> **Status:** Stabil  
> **Modul:** lumi::modules::ColorGradientModule  
> **Dateien:** ColorGradientModule.hpp, ColorGradientModule.cpp  
> **Namespace:** lumi::modules  
> **Abhängigkeiten:** IModule, glm  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Interna](#5-interna)
6. [Thread-Sicherheit](#6-thread-sicherheit)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

### 1.1 Zweck

ColorGradientModule verwaltet **Farbverläufe** für Visualizer-Shapes. Es unterstützt Solid, Linear und Radial Gradients mit bis zu 8 Color Stops.

### 1.2 Verantwortlichkeiten

- Builtin Gradient-Presets (Fire, Ocean, Neon, etc.)
- User-Presets (.grad Dateien)
- Color Stop Management mit Midpoints
- OpenGL Uniform-Daten generieren

### 1.3 Nicht-Verantwortlichkeiten

- Kein Rendering (→ Visualizer)
- Keine Shader-Kompilierung

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| IModule | Intern | Basis-Interface |
| glm | Extern | vec4 für Farben |
| nlohmann/json | Extern | Preset-Dateien |
| std::filesystem | Standard | Datei-Operationen |

---

## 3. API

### 3.1 Konstruktion

```cpp
ColorGradientModule();
~ColorGradientModule() = default;
```

### 3.2 GradientMode Enum

| Wert | Name | Beschreibung |
|------|------|--------------|
| 0 | Solid | Einfarbig |
| 1 | LinearGradient | Linearer Verlauf |
| 2 | RadialGradient | Radialer Verlauf |

### 3.3 Builtin Presets

| Name | Beschreibung |
|------|--------------|
| Fire | Rot → Orange → Gelb |
| Ocean | Dunkelblau → Cyan → Weiß |
| Neon | Cyan → Magenta → Gelb |
| Forest | Dunkelgrün → Hellgrün |
| Sunset | Violett → Orange → Gelb |
| Rainbow | Vollständiges Spektrum |
| Monochrome | Schwarz → Weiß |

### 3.4 Parameter

| ID | Typ | Bereich | Default | Beschreibung |
|----|-----|---------|---------|--------------|
| `preset` | Enum | [Custom], Builtins, User... | Fire | Gradient-Preset |
| `mode` | Enum | Solid, Linear, Radial | Linear | Gradient-Modus |
| `solidColor` | Color4f | RGBA | Weiß | Farbe bei Solid |
| `angle` | Float | 0-360° | 0 | Winkel bei Linear |
| `outlineWidth` | Float | 1-15 | 2.0 | Outline-Breite |

### 3.5 Öffentliche Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `sample(t)` | `float` | `Color4f` | Farbe bei Position 0-1 |
| `addStop(pos, color)` | `float`, `Color4f` | `void` | Color Stop hinzufügen |
| `removeStop(index)` | `int` | `void` | Stop entfernen |
| `clearStops()` | — | `void` | Alle Stops entfernen |
| `stopCount()` | — | `int` | Anzahl Stops |
| `getStopPosition(i)` | `int` | `float` | Position von Stop i |
| `getStopColor(i)` | `int` | `Color4f` | Farbe von Stop i |
| `setStopPosition(i, pos)` | `int`, `float` | `void` | Position ändern |
| `setStopColor(i, color)` | `int`, `Color4f` | `void` | Farbe ändern |
| `setMidpoint(i, pos)` | `int`, `float` | `void` | Midpoint setzen |
| `getMidpoint(i)` | `int` | `float` | Midpoint abfragen |
| `loadPreset(name)` | `string` | `void` | Preset laden |
| `savePreset(name)` | `string` | `void` | User-Preset speichern |
| `deletePreset(name)` | `string` | `bool` | User-Preset löschen |
| `reset()` | — | `void` | Auf Fire zurücksetzen |

### 3.6 OpenGL Integration

| Methode | Rückgabe | Beschreibung |
|---------|----------|--------------|
| `getUniformColors()` | `std::array<glm::vec4, 8>` | Farben für Shader |
| `getUniformPositions()` | `std::array<float, 8>` | Positionen für Shader |
| `getUniformMidpoints()` | `std::array<float, 7>` | Midpoints für Shader |

### 3.7 Statische Methoden

| Methode | Parameter | Beschreibung |
|---------|-----------|--------------|
| `setUserPresetsDirectory(path)` | `string` | Preset-Verzeichnis setzen |

---

## 4. Verwendung

### 4.1 Einfaches Beispiel

```cpp
ColorGradientModule gradient;
gradient.loadPreset("Ocean");

// Farbe bei 50% Position
Color4f color = gradient.sample(0.5f);
```

### 4.2 Custom Gradient erstellen

```cpp
gradient.clearStops();
gradient.addStop(0.0f, {1.0f, 0.0f, 0.0f, 1.0f});  // Rot
gradient.addStop(0.5f, {1.0f, 1.0f, 0.0f, 1.0f});  // Gelb
gradient.addStop(1.0f, {0.0f, 1.0f, 0.0f, 1.0f});  // Grün

// Midpoint für ersten Übergang verschieben
gradient.setMidpoint(0, 0.3f);  // Rot dominiert länger
```

### 4.3 Shader-Uniforms

```cpp
// Im Visualizer::render()
auto colors = m_gradient.getUniformColors();
auto positions = m_gradient.getUniformPositions();

glUniform4fv(m_uniformColors, 8, glm::value_ptr(colors[0]));
glUniform1fv(m_uniformPositions, 8, positions.data());
glUniform1i(m_uniformStopCount, m_gradient.stopCount());
```

### 4.4 User-Presets

```cpp
// Directory setzen (einmalig)
ColorGradientModule::setUserPresetsDirectory("/path/to/presets");

// Preset speichern
gradient.savePreset("MySunset");

// Preset laden
gradient.loadPreset("MySunset");

// User-Preset löschen
gradient.deletePreset("MySunset");
```

---

## 5. Interna

### 5.1 Color Interpolation

```cpp
Color4f sample(float t) const
{
    // Finde umgebende Stops
    int i = findStopBefore(t);
    
    // Lokale Position zwischen Stop i und i+1
    float localT = (t - m_stops[i].position) / 
                   (m_stops[i+1].position - m_stops[i].position);
    
    // Midpoint-Anpassung
    localT = remapWithMidpoint(localT, m_midpoints[i]);
    
    // Linear interpolieren
    return lerp(m_stops[i].color, m_stops[i+1].color, localT);
}
```

### 5.2 Midpoint-Remapping

```cpp
float remapWithMidpoint(float t, float midpoint)
{
    // Midpoint 0.5 = keine Änderung
    // Midpoint 0.3 = Erste Farbe dominiert länger
    // Midpoint 0.7 = Zweite Farbe dominiert länger
    
    if (t < midpoint)
    {
        return 0.5f * t / midpoint;
    }
    else
    {
        return 0.5f + 0.5f * (t - midpoint) / (1.0f - midpoint);
    }
}
```

### 5.3 File-Format (.grad)

```json
{
  "name": "CustomGradient",
  "mode": 1,
  "angle": 90.0,
  "outlineWidth": 2.0,
  "stops": [
    {"pos": 0.0, "r": 1.0, "g": 0.0, "b": 0.0, "a": 1.0},
    {"pos": 0.5, "r": 1.0, "g": 1.0, "b": 0.0, "a": 1.0},
    {"pos": 1.0, "r": 0.0, "g": 1.0, "b": 0.0, "a": 1.0}
  ],
  "midpoints": [
    {"index": 0, "position": 0.3},
    {"index": 1, "position": 0.5}
  ]
}
```

---

## 6. Thread-Sicherheit

**Nicht thread-safe.** Zugriff vom Render-Thread.

---

## 7. Fehlerbehandlung

- `addStop()` mit pos außerhalb 0-1: Clamp auf gültigen Bereich
- Max 8 Stops: Weitere werden ignoriert
- `removeStop()` bei letzten 2 Stops: Keine Aktion (Minimum 2 Stops)
- `sample()` mit t außerhalb 0-1: Clamp
- `loadPreset()` mit ungültigem Namen: Log-Warnung, keine Änderung
- `deletePreset()` bei Builtin: Gibt `false` zurück
- Keine Exceptions

---

## 8. Siehe auch

- [IModule.md](IModule.md) — Basis-Interface
- [Preset_System.md](Preset_System.md) — File-Format Details
- [PulsingVisualizer](../../include/visualizers/Visualizers.md) — Verwendet ColorGradientModule

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: Gradient System, Midpoints, File-Presets, outlineWidth** |
