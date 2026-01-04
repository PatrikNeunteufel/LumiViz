# SuperscopeVisualizer — Konzept

> **Version:** 0.1.0  
> **Datum:** 2026-01-04  
> **Typ:** Concept  
> **Status:** Entwurf  
> **Autor:** Claude  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Referenz: AVS Superscope](#2-referenz-avs-superscope)
3. [Architektur](#3-architektur)
4. [SuperscopeModule](#4-superscopemodule)
5. [Expression-System](#5-expression-system)
6. [Rendering](#6-rendering)
7. [Presets](#7-presets)
8. [Implementierungsplan](#8-implementierungsplan)
9. [Offene Fragen](#9-offene-fragen)

---

## 1. Übersicht

### 1.1 Zweck

SuperscopeVisualizer ist eine **programmierbare Punkt-/Linien-Visualisierung** inspiriert von Winamp AVS Superscope. Benutzer können mathematische Ausdrücke eingeben, um komplexe audio-reaktive Formen zu erzeugen.

### 1.2 Kernfeatures

| Feature | Beschreibung |
|---------|--------------|
| **Expression-Code** | Mathematische Formeln für x, y, Farbe |
| **4-Phasen-System** | Init, Beat, Frame, Point |
| **Audio-Reaktion** | Oszilloskop- und Spektrum-Daten |
| **Render-Modi** | Punkte, Linien, Dots mit Glow |
| **Farbverläufe** | Pro-Punkt-Farbe oder Gradient |
| **Builtin-Presets** | Klassische AVS-Effekte |

### 1.3 Zielgruppe

- **Endbenutzer**: Presets auswählen und Parameter anpassen
- **Fortgeschrittene**: Eigene Expressions schreiben
- **Entwickler**: System erweitern

---

## 2. Referenz: AVS Superscope

### 2.1 Original-Konzept

AVS Superscope (Winamp) verwendet eine Expression-Sprache mit vier Code-Bereichen:

```
┌─────────────────────────────────────────────────────────┐
│ Init (einmal beim Start)                                │
│   n=200; t=0; pi=acos(-1);                              │
├─────────────────────────────────────────────────────────┤
│ Beat (bei jedem Beat)                                   │
│   t=t+rand(100)/50;                                     │
├─────────────────────────────────────────────────────────┤
│ Frame (einmal pro Frame)                                │
│   t=t+0.01;                                             │
├─────────────────────────────────────────────────────────┤
│ Point (n-mal pro Frame)                                 │
│   x=sin(i*pi*2); y=cos(i*pi*2)+v*0.5;                   │
│   red=i; green=1-i; blue=v;                             │
└─────────────────────────────────────────────────────────┘
```

### 2.2 Variablen

| Variable | Typ | Bereich | Beschreibung |
|----------|-----|---------|--------------|
| `n` | Input | 1-10000 | Anzahl Punkte (in Init setzen) |
| `i` | Input | 0.0-1.0 | Index des aktuellen Punkts (0=erster, 1=letzter) |
| `v` | Input | -1.0-1.0 | Audiowert (Waveform/Spectrum) an Position i |
| `b` | Input | 0/1 | Beat-Indikator (1 während Beat) |
| `w` | Input | Pixel | Fensterbreite |
| `h` | Input | Pixel | Fensterhöhe |
| `x` | Output | -1.0-1.0 | X-Koordinate des Punkts |
| `y` | Output | -1.0-1.0 | Y-Koordinate des Punkts |
| `red` | Output | 0.0-1.0 | Rot-Komponente |
| `green` | Output | 0.0-1.0 | Grün-Komponente |
| `blue` | Output | 0.0-1.0 | Blau-Komponente |
| `skip` | Output | 0/1 | Punkt überspringen (Linie unterbrechen) |
| `linesize` | Output | 1.0-... | Linienbreite (optional) |

### 2.3 Funktionen

| Funktion | Beschreibung |
|----------|--------------|
| `sin(x)`, `cos(x)`, `tan(x)` | Trigonometrie |
| `asin(x)`, `acos(x)`, `atan(x)`, `atan2(y,x)` | Inverse Trigonometrie |
| `sqrt(x)`, `pow(x,y)`, `exp(x)`, `log(x)`, `log10(x)` | Potenz/Wurzel |
| `abs(x)`, `sign(x)`, `floor(x)`, `ceil(x)` | Rundung |
| `min(a,b)`, `max(a,b)`, `clamp(x,lo,hi)` | Bereichs-Funktionen |
| `rand(n)` | Zufallszahl 0 bis n-1 |
| `if(cond,a,b)` | Bedingte Auswertung |
| `equal(a,b)`, `above(a,b)`, `below(a,b)` | Vergleiche (0 oder 1) |
| `getosc(pos,size,chan)` | Waveform-Wert bei Position |
| `getspec(pos,size,chan)` | Spektrum-Wert bei Position |

---

## 3. Architektur

### 3.1 Komponenten-Übersicht

```
┌─────────────────────────────────────────────────────────────┐
│                    SuperscopeVisualizer                     │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐  │
│  │ AudioSourceModule│  │ SuperscopeModule │  │ ColorGradient│  │
│  │                 │  │                 │  │   Module    │  │
│  │ - FFT           │  │ - Expressions   │  │             │  │
│  │ - Waveform      │  │ - Variables     │  │ - Presets   │  │
│  │ - Smoothing     │  │ - Render Mode   │  │ - Stops     │  │
│  └─────────────────┘  └─────────────────┘  └─────────────┘  │
│                              │                              │
│                    ┌─────────▼─────────┐                    │
│                    │ ExpressionEngine  │                    │
│                    │                   │                    │
│                    │ - Lua Interpreter │                    │
│                    │ - Variable Binding│                    │
│                    │ - Function Library│                    │
│                    └───────────────────┘                    │
├─────────────────────────────────────────────────────────────┤
│                      OpenGL Renderer                        │
│  - GL_POINTS, GL_LINE_STRIP                                 │
│  - Point Sprites mit Glow                                   │
│  - Additive Blending                                        │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 Datenfluss

```
Audio Input
    │
    ▼
┌──────────────┐
│AudioSource   │──► Waveform[576] + Spectrum[576]
└──────────────┘
    │
    ▼
┌──────────────┐
│Expression    │──► Init (einmal)
│Engine        │──► Beat (bei Beat)
│              │──► Frame (pro Frame)
│              │──► Point (n × pro Frame)
└──────────────┘
    │
    ▼
┌──────────────┐
│Point Buffer  │──► [(x, y, r, g, b, skip), ...]
└──────────────┘
    │
    ▼
┌──────────────┐
│OpenGL Render │──► Bildschirm
└──────────────┘
```

### 3.3 Klassendiagramm

```cpp
class SuperscopeVisualizer : public IVisualizer
{
    AudioSourceModule m_audioSource;
    SuperscopeModule m_superscope;
    ColorGradientModule m_colorGradient;
    
    std::vector<PointData> m_points;
    GLuint m_vao, m_vbo;
};

class SuperscopeModule : public IModule
{
    ExpressionEngine m_engine;
    
    std::string m_initCode;
    std::string m_beatCode;
    std::string m_frameCode;
    std::string m_pointCode;
    
    int m_pointCount;
    RenderMode m_renderMode;
    AudioSource m_audioSource;
    
    std::unordered_map<std::string, double> m_variables;
};

class ExpressionEngine
{
    lua_State* m_lua;
    
    void compile(const std::string& code);
    void execute();
    void setVariable(const std::string& name, double value);
    double getVariable(const std::string& name);
};
```

---

## 4. SuperscopeModule

### 4.1 Parameter

| ID | Typ | Bereich | Default | Beschreibung |
|----|-----|---------|---------|--------------|
| `preset` | Enum | [Custom], ... | Spiral | Builtin-Preset |
| `pointCount` | Int | 8-4096 | 256 | Anzahl Punkte (Standard-n) |
| `renderMode` | Enum | Dots/Lines/Thick | Lines | Render-Modus |
| `audioSource` | Enum | Waveform/Spectrum | Waveform | Audio-Datenquelle |
| `audioChannel` | Enum | L/R/Mono/Mid/Side | Mono | Audio-Kanal |
| `blendMode` | Enum | Replace/Add/Alpha | Add | Blending-Modus |
| `lineWidth` | Float | 1.0-10.0 | 2.0 | Linienbreite |
| `dotSize` | Float | 1.0-20.0 | 4.0 | Punktgröße |
| `glowEnabled` | Bool | - | true | Glow-Effekt |
| `glowIntensity` | Float | 0.0-2.0 | 0.5 | Glow-Stärke |
| `initCode` | String | - | "" | Init-Expression |
| `beatCode` | String | - | "" | Beat-Expression |
| `frameCode` | String | - | "" | Frame-Expression |
| `pointCode` | String | - | "" | Point-Expression |

### 4.2 Render-Modi

| Modus | Beschreibung | OpenGL |
|-------|--------------|--------|
| **Dots** | Einzelne Punkte | GL_POINTS mit Point Sprites |
| **Lines** | Verbundene Linie | GL_LINE_STRIP |
| **Thick** | Dicke Linien mit Glow | Triangle-Strip oder Geometry Shader |

### 4.3 Audio-Zugriff

```cpp
// Im Point-Code verfügbar
v = getAudioValue(i, audioSource, audioChannel);

// Zusätzliche Funktionen
getosc(pos, size, chan)  // pos: 0-1, size: Samples zu mitteln, chan: 0=L, 1=R, 2=Mono
getspec(pos, size, chan) // pos: 0-1 (0=Bass, 1=Treble)
```

---

## 5. Expression-System

### 5.1 Implementierungs-Optionen

| Option | Vorteile | Nachteile |
|--------|----------|-----------|
| **Lua** | Mächtig, gut dokumentiert, bereits integriert | Syntax unterscheidet sich von AVS |
| **Custom Parser** | AVS-kompatibel | Aufwändig zu implementieren |
| **ExprTk** | Schnell, math-fokussiert | Neue Dependency |
| **TinyExpr** | Minimal, einfach | Begrenzte Features |

**Empfehlung:** Lua mit AVS-kompatibler Wrapper-Schicht.

### 5.2 Lua-Integration

```lua
-- Beispiel: Spiral mit Audio
-- Init:
n = 200
t = 0
pi = math.pi

-- Frame:
t = t + 0.02

-- Point:
r = 0.3 + i * 0.5 + v * 0.2
x = math.sin(i * pi * 6 + t) * r
y = math.cos(i * pi * 6 + t) * r
red = i
green = 1 - i
blue = 0.5 + v * 0.5
```

### 5.3 AVS-Kompatibilitäts-Layer

Um AVS-Syntax zu unterstützen, wird ein Präprozessor verwendet:

```cpp
std::string convertAvsToLua(const std::string& avsCode)
{
    // Ersetzungen:
    // "=" → " = " (falls nicht bereits)
    // ";" → "\n"
    // "sin(" → "math.sin("
    // "if(cond,a,b)" → "(cond ~= 0) and a or b"
    // etc.
}
```

### 5.4 Builtin-Funktionen

```lua
-- Audio-Zugriff
function getosc(pos, size, chan)
    -- Zugriff auf Waveform-Buffer
end

function getspec(pos, size, chan)
    -- Zugriff auf Spektrum-Buffer
end

-- AVS-Kompatibilität
function equal(a, b) return (a == b) and 1 or 0 end
function above(a, b) return (a > b) and 1 or 0 end
function below(a, b) return (a < b) and 1 or 0 end
function band(a, b) return (a ~= 0 and b ~= 0) and 1 or 0 end
function bor(a, b) return (a ~= 0 or b ~= 0) and 1 or 0 end
function bnot(a) return (a == 0) and 1 or 0 end
function if_(cond, a, b) return (cond ~= 0) and a or b end
function rand(n) return math.random(0, n - 1) end
function sqr(x) return x * x end
function sigmoid(x, c) return 1 / (1 + math.exp(-x * c)) end
```

---

## 6. Rendering

### 6.1 Point-Buffer

```cpp
struct PointData
{
    float x, y;       // Position (-1 bis 1)
    float r, g, b, a; // Farbe (0 bis 1)
    float skip;       // 1.0 = überspringen (Linie unterbrechen)
};
```

### 6.2 Render-Pipeline

```cpp
void SuperscopeVisualizer::render(float dt)
{
    // 1. Expression-Execution
    executeInit();  // Nur einmal
    if (isBeat()) executeBeat();
    executeFrame();
    
    // 2. Point-Berechnung
    m_points.clear();
    for (int p = 0; p < m_superscope.pointCount(); ++p)
    {
        float i = static_cast<float>(p) / (m_superscope.pointCount() - 1);
        float v = getAudioValue(i);
        
        m_engine.setVariable("i", i);
        m_engine.setVariable("v", v);
        executePoint();
        
        PointData pt;
        pt.x = m_engine.getVariable("x");
        pt.y = m_engine.getVariable("y");
        pt.r = m_engine.getVariable("red");
        pt.g = m_engine.getVariable("green");
        pt.b = m_engine.getVariable("blue");
        pt.skip = m_engine.getVariable("skip");
        m_points.push_back(pt);
    }
    
    // 3. OpenGL-Rendering
    renderPoints();
}
```

### 6.3 Shader

```glsl
// Vertex Shader
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aSkip;

out vec4 vColor;
out float vSkip;

uniform float uPointSize;
uniform float uAspect;

void main()
{
    gl_Position = vec4(aPos.x / uAspect, aPos.y, 0.0, 1.0);
    gl_PointSize = uPointSize;
    vColor = aColor;
    vSkip = aSkip;
}

// Fragment Shader (für Dots mit Glow)
#version 330 core
in vec4 vColor;
out vec4 FragColor;

uniform bool uGlowEnabled;
uniform float uGlowIntensity;

void main()
{
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);
    
    if (dist > 1.0) discard;
    
    float alpha = 1.0;
    if (uGlowEnabled)
    {
        alpha = exp(-dist * dist * 2.0) * uGlowIntensity;
    }
    
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
```

---

## 7. Presets

### 7.1 Builtin-Presets

| Name | Beschreibung | Code-Skizze |
|------|--------------|-------------|
| **Horizontal Scope** | Klassische Wellenform | `x=i*2-1; y=v;` |
| **Circle** | Audio-reaktiver Kreis | `x=sin(i*pi2)*r; y=cos(i*pi2)*r;` |
| **Spiral** | Rotierende Spirale | `x=sin(i*pi6+t)*r; y=cos(i*pi6+t)*r;` |
| **Starfield** | Sterne mit Bewegung | Polarkoordinaten mit Geschwindigkeit |
| **Lissajous** | Lissajous-Figuren | `x=sin(a*i+t); y=cos(b*i);` |
| **Flower** | Blumen-Formen | Rose-Kurve: `r=cos(k*theta)` |
| **Heart** | Herz-Form | Parametrische Herzgleichung |
| **DNA** | Doppelhelix | Zwei verschobene Sinuswellen |
| **Spectrum Bars** | Vertikale Spektrum-Balken | `x=i*2-1; y=getspec(i)*2;` |
| **Circular Spectrum** | Radiales Spektrum | Spektrum auf Kreis gemappt |

### 7.2 Preset-Format

```json
{
    "header": {
        "name": "Spiral",
        "visualizerId": "superscope",
        "description": "Rotating spiral with audio reaction",
        "version": 1
    },
    "parameters": {
        "scope.pointCount": 256,
        "scope.renderMode": 1,
        "scope.audioSource": 0,
        "scope.lineWidth": 2.0,
        "scope.initCode": "n=256; t=0; pi=acos(-1);",
        "scope.beatCode": "",
        "scope.frameCode": "t=t+0.02;",
        "scope.pointCode": "r=0.3+i*0.5+v*0.2; x=sin(i*pi*6+t)*r; y=cos(i*pi*6+t)*r; red=i; green=1-i; blue=0.5;"
    }
}
```

### 7.3 Klassische AVS-Presets

Einige klassische AVS-Superscope-Codes zur Inspiration:

```
// El-Vis Style - Fractal
Init: n=500; t=0;
Frame: t=t+0.01;
Point: x=sin(i*3.14159*4+t)*cos(i*3.14159*2); y=cos(i*3.14159*4+t)*sin(i*3.14159*6)+v*0.3;

// UnConeD Style - 3D Rotation
Init: n=200; pi=acos(-1); t=0;
Frame: t=t+0.03;
Point: px=sin(i*pi*2); py=cos(i*pi*2); pz=sin(i*pi*4+t)*0.3+v*0.2;
       rx=px*cos(t)-pz*sin(t); rz=px*sin(t)+pz*cos(t);
       x=rx/(1-rz*0.5); y=py/(1-rz*0.5);
```

---

## 8. Implementierungsplan

### Phase 1: Grundstruktur (Tag 1-2)

- [ ] SuperscopeModule mit Basis-Parametern
- [ ] SuperscopeVisualizer mit einfachem Rendering
- [ ] Hardcodierte Beispiel-Funktion (kein Expression-Parser)

### Phase 2: Expression-Engine (Tag 3-5)

- [ ] Lua-Integration für Expressions
- [ ] Variable-Binding (i, v, x, y, red, green, blue)
- [ ] Builtin-Funktionen (sin, cos, getosc, etc.)
- [ ] AVS-zu-Lua-Konverter (optional)

### Phase 3: Rendering-Modi (Tag 6-7)

- [ ] GL_LINE_STRIP für Lines
- [ ] GL_POINTS mit Point Sprites für Dots
- [ ] Glow-Effekt
- [ ] Skip-Funktionalität (Linie unterbrechen)

### Phase 4: UI & Presets (Tag 8-10)

- [ ] ConfigPanel-Integration
- [ ] Code-Editor für Expressions
- [ ] Builtin-Presets
- [ ] Preset-Speichern/Laden

### Phase 5: Polish (Tag 11+)

- [ ] Weitere Builtin-Presets
- [ ] Performance-Optimierung
- [ ] Dokumentation

---

## 9. Offene Fragen

### 9.1 Expression-Syntax

**Frage:** AVS-kompatible Syntax oder Lua-native?

**Option A:** AVS-Syntax mit Konverter
- Pro: Kompatibel mit existierenden AVS-Presets
- Con: Zusätzliche Komplexität

**Option B:** Lua-Native
- Pro: Einfacher zu implementieren, mächtiger
- Con: Bestehende AVS-Codes müssen manuell konvertiert werden

**Empfehlung:** Lua-Native mit optionalem AVS-Import.

### 9.2 Code-Editor UI

**Frage:** Wie soll der Code-Editor im ConfigPanel aussehen?

**Optionen:**
1. **Inline TextEdit** - Einfache mehrzeilige Textfelder
2. **Popup-Dialog** - Größerer Editor mit Syntax-Highlighting
3. **Tab-basiert** - Tabs für Init/Beat/Frame/Point

**Empfehlung:** Popup-Dialog mit Tabs für die vier Code-Bereiche.

### 9.3 Performance

**Frage:** Wie viele Points sind praktikabel?

- Lua-Ausführung: ~100k Calls/Frame bei 60 FPS möglich
- OpenGL-Rendering: Millionen von Punkten kein Problem
- **Empfehlung:** Default 256, Max 4096, Warnung ab 1024

### 9.4 ColorGradient-Integration

**Frage:** Wie interagiert ColorGradientModule mit per-Point-Farben?

**Optionen:**
1. **Entweder/Oder** - ColorMode: Gradient oder Expression
2. **Kombination** - Expression überschreibt nur wenn gesetzt
3. **Gradient als Fallback** - Expression kann auf Gradient zugreifen

**Empfehlung:** ColorMode-Enum: Solid, Gradient, Expression

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.0** | **2026-01-04** | **Initial: Konzept-Entwurf** |
