# LumiPulse — Visual System Architecture

> **Version:** 1.2.0  
> **Datum:** 2025-12-31  
> **Typ:** Concept / Architecture  
> **Status:** Draft  
> **Zielgruppe:** Entwickler, Architekten  
> **Bezug:** LumiPulse_Modules.md, LumiPulse_NodeSystem.md  
> **Sprache:** Deutsch

---

## Inhaltsverzeichnis

1. [Einleitung](#1-einleitung)
2. [Architektur-Übersicht](#2-architektur-übersicht)
   - 2.1 [Schichten-Modell](#21-schichten-modell)
   - 2.2 [Datenfluss](#22-datenfluss)
   - 2.3 [Ordnerstruktur](#23-ordnerstruktur)
   - 2.4 [Namespace-Struktur](#24-namespace-struktur)
3. [Modul-System](#3-modul-system)
4. [Modul-Katalog](#4-modul-katalog)
5. [BasisVisuals (Phase 1)](#5-basisvisuals-phase-1)
6. [Node-Graph-System (Phase N)](#6-node-graph-system-phase-n)
   - 6.1 [Konzept](#61-konzept)
   - 6.2 [Typen-System](#62-typen-system)
     - 6.2.1-4 Port-Semantik, DataFlow, Numerisch, Script/Text
     - 6.2.5 [Shader-Typen](#625-datentypen---shader-)
     - 6.2.6 Typ-Kompatibilität
     - 6.2.7 Port-Darstellung
   - 6.3 [INode Interface](#63-inode-interface)
   - 6.4 [Node-Kategorien](#64-node-kategorien)
   - 6.5 [NodeGraph](#65-nodegraph)
7. [Parameter-System](#7-parameter-system)
8. [Parameter-zu-Input-Konvertierung](#8-parameter-zu-input-konvertierung)
9. [Native Value Nodes](#9-native-value-nodes)
   - 9.1 [Übersicht](#91-übersicht)
   - 9.2 [Vollständige Node-Referenz](#92-vollständige-native-value-node-referenz)
   - 9.3 [Beispiel: Audio-reaktiver Parameter](#93-beispiel-audio-reaktiver-parameter)
10. [Text/Script Nodes](#10-textscript-nodes)
    - 10.1 [Konzept](#101-konzept)
    - 10.2 [Script-Kontexte (Hooks)](#102-script-kontexte-hooks)
    - 10.3 [SuperscopeNode mit Scripts](#103-superscopenode-mit-scripts)
    - 10.4 [Vollständige Text/Script Node-Referenz](#104-vollständige-textscript-node-referenz)
    - 10.5 [Shader Nodes](#105-shader-nodes)
    - 10.6 [Shader-Pipeline Beispiel](#106-shader-pipeline-beispiel)
11. [Event-Hooks & Scripting](#11-event-hooks--scripting)
    - 11.1 [System-Variablen](#111-system-variablen)
    - 11.2 [Lua API](#112-lua-api)
    - 11.3 [Beispiel-Scripts](#113-beispiel-scripts)
12. [ConfigPanel Integration](#12-configpanel-integration)
13. [Preset-System](#13-preset-system)
14. [Serialisierung](#14-serialisierung)
15. [Implementierungs-Roadmap](#15-implementierungs-roadmap)
16. [Design-Entscheidungen](#16-design-entscheidungen)
17. [Offene Punkte](#17-offene-punkte)
18. [Siehe auch](#18-siehe-auch)
19. [Changelog](#19-changelog)

---

## 1. Einleitung

### 1.1 Zweck

Dieses Dokument definiert die vollständige Architektur des LumiPulse Visual Systems. Es beschreibt:

- **Module** als wiederverwendbare Algorithmus-Bausteine
- **BasisVisuals** als monolithische Visualizer (Phase 1)
- **Node-Graph** als flexibles Kompositionssystem (Phase N)
- **Parameter-System** mit dynamischer Konfiguration
- **Scripting-Integration** für fortgeschrittene Anpassungen

### 1.2 Phasen-Modell

| Phase | Name | Beschreibung | Zeitrahmen |
|-------|------|--------------|------------|
| **1** | BasisVisuals | Monolithische Visuals mit eingebetteten Modulen | Aktuell |
| **2** | ConfigPanel | UI für Modul-Parameter mit Code-Folding | Nächster Schritt |
| **3** | Preset-System | Speichern/Laden von Visual-Konfigurationen | Nach Phase 2 |
| **4** | Node-Graph UI | Visueller Node-Editor | Mittelfristig |
| **5** | Parameter-Links | Parameter↔Input Konvertierung | Mit Phase 4 |
| **6** | Scripting | Lua/Expression für dynamische Parameter | Langfristig |

### 1.3 Kernprinzipien

1. **Wiederverwendbarkeit**: Module sind in mehreren Visuals/Nodes nutzbar
2. **Komposition**: Komplexe Effekte durch Kombination einfacher Bausteine
3. **Konfigurierbarkeit**: Alle Parameter über einheitliches System
4. **Erweiterbarkeit**: Neue Module/Nodes ohne Kernänderungen
5. **Serialisierbarkeit**: Vollständige Persistenz als JSON

---

## 2. Architektur-Übersicht

### 2.1 Schichten-Modell

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              PRESENTATION                                   │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │ ConfigPanel │  │ NodeEditor  │  │PresetBrowser│  │VisualWidget │         │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘         │
└─────────┼────────────────┼────────────────┼────────────────┼────────────────┘
          │                │                │                │
┌─────────┼────────────────┼────────────────┼────────────────┼────────────────┐
│         │           APPLICATION           │                │                │
│         ▼                ▼                ▼                ▼                │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                        VisualizerManager                             │   │
│  │  • Active Visual Management                                          │   │
│  │  • Render Loop Coordination                                          │   │
│  │  • Event Distribution                                                │   │
│  └───────────────────────────────┬──────────────────────────────────────┘   │
│                                  │                                          │
│  ┌───────────────────────────────┼──────────────────────────────────────┐   │
│  │                        NodeGraph (Phase N)                           │   │
│  │  • Node Instances                                                    │   │
│  │  • Connections                                                       │   │
│  │  • Execution Order                                                   │   │
│  └───────────────────────────────┬──────────────────────────────────────┘   │
└──────────────────────────────────┼──────────────────────────────────────────┘
                                   │
┌──────────────────────────────────┼─────────────────────────────────────────┐
│                                  │         DOMAIN                          │
│         ┌────────────────────────┼────────────────────────┐                │
│         │                        ▼                        │                │
│         │  ┌─────────────────────────────────────────┐    │                │
│         │  │           BasisVisual / Node            │    │                │
│         │  │  ┌─────────┐ ┌─────────┐ ┌─────────┐    │    │                │
│         │  │  │ Module  │ │ Module  │ │ Module  │    │    │                │
│         │  │  │    A    │ │    B    │ │    C    │    │    │                │
│         │  │  └────┬────┘ └────┬────┘ └────┬────┘    │    │                │
│         │  │       └───────────┼───────────┘         │    │                │
│         │  │                   ▼                     │    │                │
│         │  │            [Render Output]              │    │                │
│         │  └─────────────────────────────────────────┘    │                │
│         │                                                 │                │
│         │  ┌──────────────────────────────────────────┐   │                │
│         │  │              Module Library              │   │                │
│         │  │  AudioSource, Smoothing, Gradient,       │   │                │
│         │  │  Bars, Line, Peak, Blur, Bloom, ...      │   │                │
│         │  └──────────────────────────────────────────┘   │                │
│         └─────────────────────────────────────────────────┘                │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Datenfluss

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           AUDIO → VISUAL PIPELINE                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐               │
│  │  BASS    │    │  Audio   │    │ Smoothing│    │  Visual  │               │
│  │  Engine  │───►│  Source  │───►│  Module  │───►│  Module  │───► Display   │
│  │          │    │  Module  │    │          │    │          │               │
│  └──────────┘    └──────────┘    └──────────┘    └──────────┘               │
│       │              │               │               │                      │
│       │              ▼               ▼               ▼                      │
│       │         ┌────────┐     ┌────────┐     ┌────────┐                    │
│       │         │PCM/FFT │     │Smoothed│     │Rendered│                    │
│       │         │ Data   │     │ Bands  │     │ Frame  │                    │
│       │         └────────┘     └────────┘     └────────┘                    │
│       │                                                                     │
│       │  Beat Detection Path:                                               │
│       │  ┌──────────┐    ┌──────────┐                                       │
│       └─►│  Beat    │───►│  Event   │───► perBeat Hooks                     │
│          │ Detector │    │   Bus    │                                       │
│          └──────────┘    └──────────┘                                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 2.3 Ordnerstruktur

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          PROJECT STRUCTURE                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  LumiViz/                                                                     │
│  ├── include/                                                               │
│  │   └── visualizers/                                                       │
│  │       ├── IVisual.hpp                 ← Visual Interface                 │
│  │       ├── VisualRegistry.hpp          ← Visual Factory/Registry          │
│  │       ├── VisualContext.hpp           ← Render Context                   │
│  │       │                                                                  │
│  │       ├── modules/                    ← MODULE DEFINITIONS               │
│  │       │   ├── IModule.hpp             ← Module Interface                 │
│  │       │   ├── ModuleParamDesc.hpp     ← Parameter Descriptor             │
│  │       │   │                                                              │
│  │       │   ├── source/                 ← Source Modules                   │
│  │       │   │   ├── AudioSourceModule.hpp                                  │
│  │       │   │   ├── WaveformSourceModule.hpp                               │
│  │       │   │   └── BeatDetectModule.hpp                                   │
│  │       │   │                                                              │
│  │       │   ├── processing/             ← Processing Modules               │
│  │       │   │   ├── SmoothingModule.hpp                                    │
│  │       │   │   ├── NormalizeModule.hpp                                    │
│  │       │   │   └── ClampModule.hpp                                        │
│  │       │   │                                                              │
│  │       │   ├── render/                 ← Render Modules                   │
│  │       │   │   ├── BarsModule.hpp                                         │
│  │       │   │   ├── LineModule.hpp                                         │
│  │       │   │   ├── SuperscopeModule.hpp                                   │
│  │       │   │   ├── ShapeModule.hpp                                        │
│  │       │   │   ├── RingModule.hpp                                         │
│  │       │   │   ├── DotPlaneModule.hpp                                     │
│  │       │   │   └── Spectrum3DModule.hpp                                   │
│  │       │   │                                                              │
│  │       │   ├── effect/                 ← Effect Modules                   │
│  │       │   │   ├── BlurModule.hpp                                         │
│  │       │   │   ├── BloomModule.hpp                                        │
│  │       │   │   ├── MovementModule.hpp                                     │
│  │       │   │   └── ColorFadeModule.hpp                                    │
│  │       │   │                                                              │
│  │       │   ├── color/                  ← Color Modules                    │
│  │       │   │   ├── GradientModule.hpp                                     │
│  │       │   │   └── HSVShiftModule.hpp                                     │
│  │       │   │                                                              │
│  │       │   └── peak/                   ← Peak Modules                     │
│  │       │       ├── PeakSpawnerModule.hpp                                  │
│  │       │       └── PeakParticleModule.hpp                                 │
│  │       │                                                                  │
│  │       ├── nodes/                      ← NODE DEFINITIONS                 │
│  │       │   ├── INode.hpp               ← Node Interface                   │
│  │       │   ├── NodeGraph.hpp           ← Graph Container                  │
│  │       │   ├── NodeRegistry.hpp        ← Node Factory                     │
│  │       │   ├── PortTypes.hpp           ← Port Type Definitions            │
│  │       │   │                                                              │
│  │       │   ├── source/                 ← Source Nodes                     │
│  │       │   │   ├── AudioSourceNode.hpp                                    │
│  │       │   │   ├── WaveformSourceNode.hpp                                 │
│  │       │   │   └── BeatDetectNode.hpp                                     │
│  │       │   │                                                              │
│  │       │   ├── render/                 ← Render Nodes                     │
│  │       │   │   ├── BarsNode.hpp                                           │
│  │       │   │   ├── LineNode.hpp                                           │
│  │       │   │   ├── SuperscopeNode.hpp                                     │
│  │       │   │   └── ... (weitere)                                          │
│  │       │   │                                                              │
│  │       │   ├── value/                  ← Native Value Nodes               │
│  │       │   │   ├── FloatNode.hpp                                          │
│  │       │   │   ├── IntNode.hpp                                            │
│  │       │   │   ├── BoolNode.hpp                                           │
│  │       │   │   ├── StringNode.hpp                                         │
│  │       │   │   ├── Vec2Node.hpp                                           │
│  │       │   │   ├── Vec3Node.hpp                                           │
│  │       │   │   ├── Vec4Node.hpp                                           │
│  │       │   │   ├── ColorNode.hpp                                          │
│  │       │   │   └── TimeNode.hpp                                           │
│  │       │   │                                                              │
│  │       │   ├── math/                   ← Math Nodes                       │
│  │       │   │   ├── MathNode.hpp        ← +,-,*,/,pow,sin,cos...           │
│  │       │   │   ├── TrigNode.hpp        ← sin,cos,tan,atan2...             │
│  │       │   │   ├── VectorMathNode.hpp  ← dot,cross,normalize...           │
│  │       │   │   ├── ClampNode.hpp                                          │
│  │       │   │   ├── LerpNode.hpp                                           │
│  │       │   │   └── MapRangeNode.hpp                                       │
│  │       │   │                                                              │
│  │       │   ├── script/                 ← Script/Text Nodes                │
│  │       │   │   ├── ExpressionNode.hpp  ← Einfache Expressions             │
│  │       │   │   ├── LuaNode.hpp         ← Vollständiges Lua                │
│  │       │   │   ├── PerFrameScriptNode.hpp                                 │
│  │       │   │   ├── PerPointScriptNode.hpp                                 │
│  │       │   │   └── PerBeatScriptNode.hpp                                  │
│  │       │   │                                                              │
│  │       │   ├── shader/                 ← Shader Nodes                     │
│  │       │   │   ├── ShaderTypes.hpp     ← ShaderSource, ShaderProgram      │
│  │       │   │   ├── VertexShaderNode.hpp                                   │
│  │       │   │   ├── FragmentShaderNode.hpp                                 │
│  │       │   │   ├── GeometryShaderNode.hpp                                 │
│  │       │   │   ├── ComputeShaderNode.hpp                                  │
│  │       │   │   ├── ShaderProgramNode.hpp                                  │
│  │       │   │   ├── ShaderRenderNode.hpp                                   │
│  │       │   │   ├── GlslIncludeNode.hpp                                    │
│  │       │   │   └── presets/            ← Shader Preset Nodes              │
│  │       │   │       ├── GlowShaderNode.hpp                                 │
│  │       │   │       ├── BlurShaderNode.hpp                                 │
│  │       │   │       ├── ChromaticShaderNode.hpp                            │
│  │       │   │       ├── DistortShaderNode.hpp                              │
│  │       │   │       ├── KaleidoscopeShaderNode.hpp                         │
│  │       │   │       ├── FeedbackShaderNode.hpp                             │
│  │       │   │       └── ColorGradeShaderNode.hpp                           │
│  │       │   │                                                              │
│  │       │   ├── audio/                  ← Audio Analysis Nodes             │
│  │       │   │   ├── AudioLevelNode.hpp  ← bass,mid,treb,left,right         │
│  │       │   │   ├── BandSplitNode.hpp   ← Split Spectrum in Bands          │
│  │       │   │   └── FFTNode.hpp                                            │
│  │       │   │                                                              │
│  │       │   ├── composite/              ← Composite Nodes                  │
│  │       │   │   ├── BlendNode.hpp                                          │
│  │       │   │   ├── BufferNode.hpp                                         │
│  │       │   │   └── LayerNode.hpp                                          │
│  │       │   │                                                              │
│  │       │   └── utility/                ← Utility Nodes                    │
│  │       │       ├── CommentNode.hpp                                        │
│  │       │       ├── RerouteNode.hpp                                        │
│  │       │       └── GroupNode.hpp                                          │
│  │       │                                                                  │
│  │       └── basics/                     ← BASIS VISUALS (Phase 1)          │
│  │           ├── EqualizerVisual.hpp                                        │
│  │           ├── WaveformVisual.hpp                                         │
│  │           ├── SpectrumVisual.hpp                                         │
│  │           ├── SuperscopeVisual.hpp                                       │
│  │           ├── RingVisual.hpp                                             │
│  │           └── DotPlaneVisual.hpp                                         │
│  │                                                                          │
│  └── src/                                                                   │
│      └── visualizers/                                                       │
│          ├── VisualRegistry.cpp                                             │
│          ├── modules/                    ← Module Implementations           │
│          │   ├── source/                                                    │
│          │   ├── processing/                                                │
│          │   ├── render/                                                    │
│          │   ├── effect/                                                    │
│          │   ├── color/                                                     │
│          │   └── peak/                                                      │
│          ├── nodes/                      ← Node Implementations             │
│          │   ├── NodeGraph.cpp                                              │
│          │   ├── NodeRegistry.cpp                                           │
│          │   ├── source/                                                    │
│          │   ├── render/                                                    │
│          │   ├── value/                                                     │
│          │   ├── math/                                                      │
│          │   ├── script/                                                    │
│          │   ├── shader/                 ← Shader Implementations           │
│          │   │   └── presets/                                               │
│          │   ├── audio/                                                     │
│          │   ├── composite/                                                 │
│          │   └── utility/                                                   │
│          └── basics/                     ← BasisVisual Implementations      │
│              ├── EqualizerVisual.cpp                                        │
│              ├── WaveformVisual.cpp                                         │
│              └── ...                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 2.4 Namespace-Struktur

```cpp
namespace lumi {
    namespace modules {
        // IModule, SmoothingModule, GradientModule, etc.
    }
    
    namespace nodes {
        // INode, NodeGraph, FloatNode, MathNode, etc.
    }
    
    namespace visuals {
        // IVisual, EqualizerVisual, WaveformVisual, etc.
    }
}
```

---

## 3. Modul-System

### 3.1 IModule Interface

```cpp
// include/modules/IModule.hpp
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <array>
#include <functional>
#include <memory>

namespace lumi::modules {

// ═══════════════════════════════════════════════════════════════════════════
// Parameter Value Types
// ═══════════════════════════════════════════════════════════════════════════

using Color4f = std::array<float, 4>;  // RGBA normalized
using Vec2f = std::array<float, 2>;
using Vec3f = std::array<float, 3>;
using Vec4f = std::array<float, 4>;

using ParamValue = std::variant<
    bool,
    int,
    float,
    std::string,
    Vec2f,
    Vec3f,
    Vec4f,
    Color4f
>;

// ═══════════════════════════════════════════════════════════════════════════
// Parameter Descriptor
// ═══════════════════════════════════════════════════════════════════════════

enum class ParamType {
    Bool,
    Int,
    Float,
    String,
    Enum,
    Vec2,
    Vec3,
    Vec4,
    Color
};

enum class ParamWidget {
    Default,        // Auto-select based on type
    Slider,         // Float/Int with range
    Spinbox,        // Numeric with arrows
    Checkbox,       // Bool toggle
    Dropdown,       // Enum selection
    ColorPicker,    // Color4f
    TextInput,      // String
    TextArea,       // Multi-line string (for scripts)
    Knob,           // Rotary control
    Toggle,         // On/Off switch
    ButtonGroup     // Enum as buttons
};

struct ModuleParamDesc {
    // Identification
    std::string id;              // Unique within module (e.g., "emaAlpha")
    std::string displayName;     // UI label (e.g., "EMA Alpha")
    std::string group;           // Collapsible group (e.g., "Smoothing")
    std::string subGroup;        // Nested group (e.g., "Advanced")
    std::string tooltip;         // Help text
    
    // Type & Value
    ParamType type = ParamType::Float;
    ParamValue defaultValue;
    
    // Constraints
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float step = 0.01f;
    std::vector<std::string> enumOptions;  // For Enum type
    
    // UI Hints
    ParamWidget widget = ParamWidget::Default;
    int order = 0;               // Sort order within group
    bool advanced = false;       // Hide in "Advanced" section
    bool canBeInput = true;      // Can be converted to node input
    
    // Dependencies (conditional visibility)
    std::string dependsOn;       // Other param ID
    ParamValue dependsValue;     // Required value for visibility
    
    // Units & Formatting
    std::string unit;            // e.g., "ms", "Hz", "%", "px"
    std::string format;          // printf format (e.g., "%.2f")
};

// ═══════════════════════════════════════════════════════════════════════════
// Module Interface
// ═══════════════════════════════════════════════════════════════════════════

class IModule {
public:
    virtual ~IModule() = default;
    
    // ─────────────────────────────────────────────────────────────────────
    // Identification
    // ─────────────────────────────────────────────────────────────────────
    
    /// Unique module type ID (e.g., "smoothing", "gradient")
    [[nodiscard]] virtual const char* moduleId() const = 0;
    
    /// Human-readable name (e.g., "Smoothing", "Color Gradient")
    [[nodiscard]] virtual const char* displayName() const = 0;
    
    /// Module category (e.g., "Processing", "Render", "Effect")
    [[nodiscard]] virtual const char* category() const = 0;
    
    /// Description for tooltips
    [[nodiscard]] virtual const char* description() const = 0;
    
    // ─────────────────────────────────────────────────────────────────────
    // Parameter Introspection
    // ─────────────────────────────────────────────────────────────────────
    
    /// Get all parameter descriptors
    [[nodiscard]] virtual std::vector<ModuleParamDesc> paramDescs() const = 0;
    
    /// Get parameter value by ID
    [[nodiscard]] virtual bool getParam(const std::string& id, 
                                        ParamValue& out) const = 0;
    
    /// Set parameter value by ID
    virtual bool setParam(const std::string& id, 
                         const ParamValue& value) = 0;
    
    /// Reset all parameters to defaults
    virtual void resetToDefaults() = 0;
    
    // ─────────────────────────────────────────────────────────────────────
    // Lifecycle
    // ─────────────────────────────────────────────────────────────────────
    
    /// Called once after construction
    virtual void initialize() {}
    
    /// Called when parent visual becomes active
    virtual void activate() {}
    
    /// Called when parent visual becomes inactive
    virtual void deactivate() {}
    
    /// Called each frame before render
    virtual void update(float deltaTime) { (void)deltaTime; }
};

// ═══════════════════════════════════════════════════════════════════════════
// Module with Processing
// ═══════════════════════════════════════════════════════════════════════════

template<typename TInput, typename TOutput>
class IProcessingModule : public IModule {
public:
    /// Process input data and produce output
    virtual TOutput process(const TInput& input) = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// Module with Rendering
// ═══════════════════════════════════════════════════════════════════════════

struct RenderContext {
    float deltaTime = 0.0f;
    int frameNumber = 0;
    int viewportWidth = 0;
    int viewportHeight = 0;
    float aspectRatio = 1.0f;
    bool beatThisFrame = false;
    float beatIntensity = 0.0f;
};

class IRenderModule : public IModule {
public:
    /// Render to current OpenGL context
    virtual void render(const RenderContext& ctx) = 0;
};

} // namespace lumi::modules
```

### 3.2 Modul-Hierarchie

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           MODULE HIERARCHY                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  IModule (Base)                                                             │
│  ├── moduleId(), displayName(), category(), description()                   │
│  ├── paramDescs(), getParam(), setParam(), resetToDefaults()                │
│  └── initialize(), activate(), deactivate(), update()                       │
│      │                                                                      │
│      ├── IProcessingModule<TIn, TOut>                                       │
│      │   └── process(TInput) → TOutput                                      │
│      │       │                                                              │
│      │       ├── SmoothingModule : IProcessingModule<float[], float[]>      │
│      │       ├── NormalizeModule : IProcessingModule<float[], float[]>      │
│      │       └── FFTModule : IProcessingModule<PCMData, SpectrumData>       │
│      │                                                                      │
│      ├── IRenderModule                                                      │
│      │   └── render(RenderContext)                                          │
│      │       │                                                              │
│      │       ├── BarsModule                                                 │
│      │       ├── LineModule                                                 │
│      │       ├── SuperscopeModule                                           │
│      │       ├── ShapeModule                                                │
│      │       └── TextModule                                                 │
│      │                                                                      │
│      ├── IEffectModule : IRenderModule                                      │
│      │   └── render() operates on existing framebuffer                      │
│      │       │                                                              │
│      │       ├── BlurModule                                                 │
│      │       ├── BloomModule                                                │
│      │       ├── ColorFadeModule                                            │
│      │       └── MovementModule                                             │
│      │                                                                      │
│      └── ISourceModule                                                      │
│          └── getData() → SourceData                                         │
│              │                                                              │
│              ├── AudioSourceModule                                          │
│              ├── BeatDetectModule                                           │
│              └── MicInputModule                                             │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 3.3 Modul-Komposition

Module können andere Module als Member enthalten:

```cpp
class AudioSourceModule : public ISourceModule {
private:
    SmoothingModule m_Smoothing;  // Embedded module
    
public:
    std::vector<ModuleParamDesc> paramDescs() const override {
        auto params = getOwnParams();
        
        // Aggregate smoothing params with prefix
        for (const auto& p : m_Smoothing.paramDescs()) {
            auto prefixed = p;
            prefixed.id = "smooth." + p.id;
            prefixed.group = "Smoothing";
            params.push_back(prefixed);
        }
        
        return params;
    }
};
```

---

## 4. Modul-Katalog

### 4.1 Source Module

#### 4.1.1 AudioSourceModule

**Verantwortung:** FFT-Daten abrufen, in Bänder mappen, normalisieren.

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `scale` | Enum | Linear/Log/Mel | Log | Frequenz→Band Mapping |
| `bands` | Int | 8-512 | 64 | Anzahl Output-Bänder |
| `floorDb` | Float | -120..0 | -60 | Untere dB-Grenze (→ 0.0) |
| `ceilDb` | Float | -60..20 | 0 | Obere dB-Grenze (→ 1.0) |
| `clamp01` | Bool | — | true | Auf [0,1] beschränken |
| `requestedFft` | Int | 256-8192 | 2048 | FFT-Größe |
| `smooth.*` | — | — | — | Eingebettetes SmoothingModule |

```
Input:  Raw FFT bins from BASS Engine
Output: Normalized band values [0..1]

┌─────────────────────────────────────────────────────────┐
│                    AudioSourceModule                    │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  FFT Bins (2048)                                        │
│  ████████████████████████████████████████████████████   │
│                         │                               │
│                         ▼                               │
│  ┌─────────────────────────────────────────────────┐    │
│  │           Frequency Mapping (Log/Mel)           │    │
│  └─────────────────────────────────────────────────┘    │
│                         │                               │
│                         ▼                               │
│  ┌─────────────────────────────────────────────────┐    │
│  │              dB Normalization                   │    │
│  │         (floorDb → 0.0, ceilDb → 1.0)           │    │
│  └─────────────────────────────────────────────────┘    │
│                         │                               │
│                         ▼                               │
│  ┌─────────────────────────────────────────────────┐    │
│  │             SmoothingModule                     │    │
│  │            (SMA/EMA/WMA/DEMA)                   │    │
│  └─────────────────────────────────────────────────┘    │
│                         │                               │
│                         ▼                               │
│  Normalized Bands (64)                                  │
│  ▃▅▇█▇▅▃▂▃▅▆▇█▇▅▄▃▃▄▅▆▇▇▆▅▄▃▂▂▃▄▅▆▆▅▄▃▂▁▁▂▃▃▃▂▂▁▁▁▁▁    │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

#### 4.1.2 BeatDetectModule

**Verantwortung:** Beat-Erkennung aus Audio-Energie.

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `sensitivity` | Float | 0.1-3.0 | 1.0 | Beat-Schwelle |
| `minIntervalMs` | Float | 50-500 | 120 | Min. Zeit zwischen Beats |
| `freqLow` | Float | 20-200 | 60 | Untere Frequenz (Hz) |
| `freqHigh` | Float | 100-500 | 250 | Obere Frequenz (Hz) |
| `historyMs` | Float | 200-2000 | 1000 | Energie-Historie |
| `algorithm` | Enum | Energy/Onset/Flux | Energy | Erkennungs-Algorithmus |

```
Output Events:
  • beatDetected (bool) - Beat diesen Frame?
  • beatIntensity (float) - Stärke 0..1
  • bpm (float) - Geschätzte BPM
```

#### 4.1.3 WaveformSourceModule

**Verantwortung:** Raw PCM-Daten für Waveform-Darstellung.

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `samples` | Int | 128-4096 | 512 | Anzahl Samples |
| `channel` | Enum | Left/Right/Mono/Stereo | Stereo | Kanal-Auswahl |

---

### 4.2 Processing Module

#### 4.2.1 SmoothingModule

**Verantwortung:** Zeitliche Glättung von Werten.

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `algorithm` | Enum | None/SMA/EMA/WMA/DEMA | EMA | Algorithmus |
| `timeMs` | Float | 0-500 | 50 | Glättungszeit (ms) |
| `preset` | Enum | Instant/Reactive/Balanced/Smooth/Sluggish | Balanced | Vordefinierte Presets |
| `primeFirstFrame` | Bool | — | true | Erstes Frame primen |

**Algorithmen im Detail:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SMOOTHING ALGORITHMS                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  NONE (Pass-through)                                                        │
│  ───────────────────                                                        │
│  out[n] = in[n]                                                             │
│  • Kein Lag, kein Smoothing                                                 │
│  • Für Beat-reaktive Effekte                                                │
│                                                                             │
│  SMA (Simple Moving Average)                                                │
│  ──────────────────────────                                                 │
│  out[n] = (in[n] + in[n-1] + ... + in[n-N+1]) / N                           │
│  • Gleichmäßige Gewichtung                                                  │
│  • Größerer Lag, sehr glatt                                                 │
│  • Window-Size aus timeMs berechnet                                         │
│                                                                             │
│  EMA (Exponential Moving Average)                                           │
│  ─────────────────────────────────                                          │
│  out[n] = α * in[n] + (1-α) * out[n-1]                                      │
│  α = 1 - e^(-deltaTime / τ)    wobei τ = timeMs / 1000                      │
│  • Exponentiell abnehmende Gewichtung                                       │
│  • Weniger Lag als SMA                                                      │
│  • Frame-Rate-unabhängig                                                    │
│                                                                             │
│  WMA (Weighted Moving Average)                                              │
│  ─────────────────────────────                                              │
│  out[n] = Σ(w[i] * in[n-i]) / Σw[i]    wobei w[i] = N - i                   │
│  • Neuere Werte stärker gewichtet                                           │
│  • Kompromiss zwischen SMA und EMA                                          │
│                                                                             │
│  DEMA (Double Exponential Moving Average)                                   │
│  ────────────────────────────────────────                                   │
│  EMA1 = EMA(in)                                                             │
│  EMA2 = EMA(EMA1)                                                           │
│  out = 2 * EMA1 - EMA2                                                      │
│  • Reduzierter Lag bei gleicher Glättung                                    │
│  • Kann überschwingen                                                       │
│                                                                             │
│  Visuelle Charakteristik:                                                   │
│  ───────────────────────                                                    │
│                                                                             │
│  Input:    ╱╲    ╱╲╱╲      ╱╲                                               │
│           ╱  ╲  ╱    ╲    ╱  ╲                                              │
│                                                                             │
│  None:     ╱╲    ╱╲╱╲      ╱╲     (identisch)                               │
│           ╱  ╲  ╱    ╲    ╱  ╲                                              │
│                                                                             │
│  EMA:     ╱╲    ╱─╲       ╱╲     (abgerundet)                               │
│          ╱  ╲  ╱   ╲     ╱  ╲                                               │
│                                                                             │
│  SMA:    ╱──╲  ╱───╲     ╱──╲    (verzögert + glatt)                        │
│         ╱    ╲╱     ╲   ╱    ╲                                              │
│                                                                             │
│  DEMA:    ╱╲   ╱╲╱╲      ╱╲       (weniger Lag)                             │
│          ╱  ╲ ╱    ╲    ╱  ╲                                                │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 4.2.2 NormalizeModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `mode` | Enum | Peak/RMS/Auto | Auto | Normalisierungsmodus |
| `targetLevel` | Float | 0.1-1.0 | 0.8 | Zielpegel |
| `attackMs` | Float | 1-100 | 10 | Anstiegszeit |
| `releaseMs` | Float | 50-2000 | 500 | Abfallzeit |

---

### 4.3 Render Module - Bars

#### 4.3.1 BarsModule

**Verantwortung:** Balken-Geometrie berechnen und rendern.

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `orientation` | Enum | BottomUp/TopDown/LeftRight/RightLeft | BottomUp | Balkenrichtung |
| `gapPx` | Float | 0-20 | 2 | Abstand zwischen Balken (px) |
| `gapRatio` | Float | 0-0.9 | 0 | Abstand relativ zu Breite |
| `roundedCorners` | Bool | — | false | Abgerundete Ecken |
| `cornerRadius` | Float | 0-20 | 4 | Eckenradius (px) |
| `minHeight` | Float | 0-0.1 | 0.01 | Minimale Balkenhöhe |

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         BARS ORIENTATIONS                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  BottomUp              TopDown               LeftRight          RightLeft   │
│  ─────────             ─────────             ─────────          ─────────   │
│                        ██ ██ █               │                          │   │
│  █                     ██ ██ ██              │█████              █████│     │
│  ██                    ██ ██ ██              │████                ████│     │
│  ██ █                  ██ ██ ██              │███                  ███│     │
│  ██ ██                 ██ ██ ██              │██████            ██████│     │
│  ██ ██ █               ██ ██ ██ █            │███                  ███│     │
│  ██ ██ ██              ██ ██ ██ ██           │                          │   │
│  ─────────             ─────────             ─────────          ─────────   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 4.3.2 Spectrum3DModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `perspective` | Float | 0-1 | 0.3 | 3D-Perspektivenstärke |
| `depth` | Float | 0-1 | 0.5 | Z-Tiefe |
| `rotation` | Float | -45..45 | 15 | Y-Rotation (Grad) |
| `spacing` | Float | 0-1 | 0.1 | Z-Abstand zwischen Reihen |
| `historyRows` | Int | 1-20 | 8 | Anzahl History-Reihen |

---

### 4.4 Render Module - Scope

#### 4.4.1 LineModule

**Verantwortung:** Waveform als Linie oder Kreis rendern.

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| **Allgemein** |
| `shape` | Enum | Line/Circle | Line | Darstellungsform |
| `channelMode` | Enum | Mono/Left/Right/Stereo/Both | Stereo | Kanal-Modus |
| `samples` | Int | 64-2048 | 512 | Anzahl Linienpunkte |
| **Linie** |
| `lineWidth` | Float | 0.5-10 | 2.0 | Linienbreite (px) |
| `useGradient` | Bool | — | false | Gradient statt Farbe |
| `fillEnabled` | Bool | — | false | Fläche füllen |
| `fillAlpha` | Float | 0-1 | 0.3 | Füll-Transparenz |
| **Kreis** |
| `circleDiameter` | Float | 0.1-2.0 | 0.6 | Durchmesser (% von min(w,h)) |
| `circleOffsetX` | Float | -1..1 | 0 | X-Verschiebung |
| `circleOffsetY` | Float | -1..1 | 0 | Y-Verschiebung |
| **Kanäle (je left/right/mono)** |
| `*.lineColor` | Color | — | — | Linienfarbe |
| `*.fillColor` | Color | — | — | Füllfarbe |
| `*.offsetY` | Float | -1..1 | ±0.25 | Y-Offset (Stereo) |

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           LINE SHAPES                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Line (Stereo)                         Circle (Mono)                        │
│  ─────────────                         ─────────────                        │
│                                                                             │
│  Left Channel:                             ╱╲                               │
│  ╱╲    ╱╲╱╲      ╱╲                      ╱    ╲                             │
│ ╱  ╲  ╱    ╲    ╱  ╲                   ╱   ●   ╲                            │
│╱────╲╱──────╲──╱────╲────             │    │    │                           │
│                                       │    │    │                           │
│  Right Channel:                        ╲   │   ╱                            │
│────╱╲────╱──╲──────╱╲────               ╲  │  ╱                             │
│   ╱  ╲  ╱    ╲    ╱  ╲                    ╲│╱                               │
│  ╱    ╲╱      ╲  ╱    ╲                                                     │
│                                        (Radius = Amplitude)                 │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 4.4.2 SuperscopeModule

**Verantwortung:** Punkt-basiertes Rendering mit Custom-Gleichungen (später Lua).

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `points` | Int | 32-4096 | 512 | Anzahl Punkte |
| `drawMode` | Enum | Dots/Lines/Thick | Lines | Zeichenmodus |
| `pointSize` | Float | 1-20 | 3 | Punktgröße (px) |
| `lineWidth` | Float | 0.5-10 | 2 | Linienbreite (px) |
| `colorMode` | Enum | Fixed/Gradient/Script | Fixed | Farbmodus |
| `fixedColor` | Color | — | Cyan | Feste Farbe |
| **Scripts (Phase 6)** |
| `initScript` | Text | — | — | onInit Code |
| `perPointScript` | Text | — | — | perPoint Code |
| `perFrameScript` | Text | — | — | perFrame Code |

**Systemvariablen für Scripts:**

| Variable | Typ | Beschreibung |
|----------|-----|--------------|
| `n` | Int | Punkt-Index (0..points-1) |
| `i` | Float | Normalisierter Index (0..1) |
| `v` | Float | Audio-Wert an diesem Punkt |
| `x` | Float | X-Position (Output: -1..1) |
| `y` | Float | Y-Position (Output: -1..1) |
| `r` | Float | Rot (0..1) |
| `g` | Float | Grün (0..1) |
| `b` | Float | Blau (0..1) |
| `a` | Float | Alpha (0..1) |
| `t` | Float | Zeit (Sekunden) |
| `beat` | Float | Beat-Intensität (0..1) |
| `bass` | Float | Bass-Level (0..1) |
| `mid` | Float | Mid-Level (0..1) |
| `treb` | Float | Treble-Level (0..1) |

---

### 4.5 Render Module - Shape

#### 4.5.1 ShapeModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `sides` | Int | 3-64 | 6 | Anzahl Seiten (3=Dreieck, 4=Quadrat, etc.) |
| `radius` | Float | 0.01-1 | 0.3 | Radius (% vom Viewport) |
| `rotation` | Float | 0-360 | 0 | Rotation (Grad) |
| `rotationSpeed` | Float | -360..360 | 0 | Auto-Rotation (°/s) |
| `centerX` | Float | -1..1 | 0 | X-Position |
| `centerY` | Float | -1..1 | 0 | Y-Position |
| `lineWidth` | Float | 0-10 | 2 | Randbreite (0 = nur Fill) |
| `lineColor` | Color | — | Weiß | Randfarbe |
| `fillEnabled` | Bool | — | true | Füllung aktiviert |
| `fillColor` | Color | — | Transparent | Füllfarbe |
| `audioReactive` | Bool | — | true | Radius reagiert auf Audio |
| `audioScale` | Float | 0-2 | 0.5 | Audio-Reaktivitätsstärke |

#### 4.5.2 RingModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `innerRadius` | Float | 0-0.9 | 0.2 | Innerer Radius |
| `outerRadius` | Float | 0.1-1 | 0.4 | Äußerer Radius |
| `segments` | Int | 16-256 | 64 | Segmente im Ring |
| `segmentGap` | Float | 0-0.5 | 0 | Lücke zwischen Segmenten |
| `startAngle` | Float | 0-360 | 0 | Start-Winkel |
| `endAngle` | Float | 0-360 | 360 | End-Winkel |
| `audioMode` | Enum | None/RadiusIn/RadiusOut/Both | RadiusOut | Audio-Reaktion |

---

### 4.6 Effect Module

#### 4.6.1 BlurModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `algorithm` | Enum | Box/Gaussian/Kawase | Gaussian | Blur-Algorithmus |
| `radius` | Float | 0-50 | 5 | Blur-Radius (px) |
| `passes` | Int | 1-10 | 2 | Anzahl Durchläufe |
| `direction` | Enum | Both/Horizontal/Vertical | Both | Blur-Richtung |

#### 4.6.2 BloomModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `threshold` | Float | 0-1 | 0.8 | Helligkeits-Schwelle |
| `intensity` | Float | 0-5 | 1.5 | Bloom-Stärke |
| `radius` | Float | 1-50 | 10 | Blur-Radius |
| `tintColor` | Color | — | Weiß | Bloom-Färbung |
| `blendMode` | Enum | Add/Screen | Add | Überblendungsmodus |

#### 4.6.3 MovementModule (Dynamic)

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `effect` | Enum | None/ZoomIn/ZoomOut/Rotate/Warp | None | Bewegungseffekt |
| `speed` | Float | 0-2 | 0.02 | Effektgeschwindigkeit |
| `blend` | Float | 0-1 | 0.9 | Feedback-Stärke |
| `wrapMode` | Enum | Clamp/Wrap/Mirror | Clamp | Rand-Verhalten |
| **Script (Phase 6)** |
| `perPixelScript` | Text | — | — | Custom Displacement |

---

### 4.7 Color Module

#### 4.7.1 GradientModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `preset` | Enum | Fire/Ocean/Neon/Rainbow/Custom | Fire | Gradient-Preset |
| `domain` | Enum | ByPosition/ByAmplitude | ByPosition | Mapping-Modus |
| `colorLow` | Color | — | Cyan | Farbe für Wert 0 |
| `colorHigh` | Color | — | Rot | Farbe für Wert 1 |
| `biasMid` | Float | 0-1 | 0.5 | Mittelpunkt-Verschiebung |
| `customStops` | Bool | — | false | Benutzerdefinierte Stops |
| `stopCount` | Int | 2-16 | 2 | Anzahl Stops |
| `stop[n].pos` | Float | 0-1 | — | Position im Gradient |
| `stop[n].color` | Color | — | — | Farbe am Stop |

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         GRADIENT PRESETS                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Fire:     ██████████████████████████████████████████████████████████████   │
│            (Schwarz → Rot → Orange → Gelb → Weiß)                           │
│                                                                             │
│  Ocean:    ██████████████████████████████████████████████████████████████   │
│            (Dunkelblau → Cyan → Türkis → Weiß)                              │
│                                                                             │
│  Neon:     ██████████████████████████████████████████████████████████████   │
│            (Pink → Lila → Blau → Cyan → Grün)                               │
│                                                                             │
│  Rainbow:  ██████████████████████████████████████████████████████████████   │
│            (Rot → Orange → Gelb → Grün → Cyan → Blau → Lila)                │
│                                                                             │
│  Classic:  ██████████████████████████████████████████████████████████████   │
│            (Cyan → Rot) - Default                                           │
│                                                                             │
│  Mono:     ██████████████████████████████████████████████████████████████   │
│            (Schwarz → Weiß)                                                 │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

### 4.8 Peak Module

#### 4.8.1 PeakSpawnerModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| **Allgemein** |
| `enabled` | Bool | — | true | Peak-System aktiv |
| `drawBehind` | Bool | — | false | Hinter Balken zeichnen |
| **Physik** |
| `physicsMode` | Enum | Classic/Spring | Classic | Physik-Modus |
| `delayMs` | Float | 0-1000 | 120 | Hold-Zeit (Classic) |
| `gravity` | Float | 0-50 | 5 | Fallbeschleunigung |
| `falloff` | Float | 0-20 | 3 | Luftwiderstand |
| `springK` | Float | 1-200 | 40 | Federhärte (Spring) |
| `springDamping` | Float | 0-50 | 10 | Dämpfung (Spring) |
| `bounceElasticity` | Float | 0-1 | 0.25 | Prall-Elastizität |
| `respawnOnLeave` | Bool | — | false | Respawn außerhalb [0,1] |
| **Farbe** |
| `colorMode` | Enum | Auto/Fixed | Auto | Farbmodus |
| `fixedColor` | Color | — | Weiß | Feste Farbe |
| `freezeOnSpawn` | Bool | — | false | Farbe einfrieren |
| **Dicke** |
| `thicknessMode` | Enum | Fixed/Direct/Inverse | Fixed | Dicken-Modus |
| `basePx` | Float | 1-20 | 2 | Basis-Dicke |
| `scalePx` | Float | 0-20 | 3 | Zusätzliche Dicke |
| `minPx` | Float | 0.5-10 | 1 | Minimale Dicke |
| `maxPx` | Float | 2-50 | 8 | Maximale Dicke |

#### 4.8.2 PeakParticleModule

| Parameter | Typ | Bereich | Default | Beschreibung |
|-----------|-----|---------|---------|--------------|
| `enabled` | Bool | — | false | Partikel aktiviert |
| `spawnMinDelta` | Float | 0-1 | 0.05 | Min. Amplitude für Spawn |
| `spawnMinInterval` | Float | 0-1000 | 80 | Min. Zeit zwischen Spawns (ms) |
| `maxPerBand` | Int | 1-50 | 6 | Max. Partikel pro Band |
| `lifetime` | Float | 0.1-10 | 2.0 | Lebensdauer (Sekunden) |
| `fadeMode` | Enum | None/Alpha/Size/Both | Alpha | Fade-Modus |
| `colorFollow` | Bool | — | false | Farbe = Spawner-Farbe |
| `colorFreeze` | Bool | — | false | Spawn-Farbe behalten |

---

## 5. BasisVisuals (Phase 1)

### 5.1 Konzept

BasisVisuals sind **monolithische Visualizer-Klassen**, die Module als Member enthalten. Sie implementieren eine feste Pipeline und exponieren alle Modul-Parameter aggregiert.

```cpp
// include/visuals/IVisual.hpp
#pragma once

#include "modules/IModule.hpp"
#include <vector>
#include <memory>
#include <string>

namespace lumi::visuals {

struct VisualContext {
    float deltaTime = 0.0f;
    int frameNumber = 0;
    int viewportWidth = 0;
    int viewportHeight = 0;
    bool beatThisFrame = false;
    float beatIntensity = 0.0f;
    const float* bandData = nullptr;
    int bandCount = 0;
    const float* waveformLeft = nullptr;
    const float* waveformRight = nullptr;
    int waveformSamples = 0;
};

class IVisual {
public:
    virtual ~IVisual() = default;
    
    // ─────────────────────────────────────────────────────────────────────
    // Identification
    // ─────────────────────────────────────────────────────────────────────
    
    [[nodiscard]] virtual const char* visualId() const = 0;
    [[nodiscard]] virtual const char* displayName() const = 0;
    [[nodiscard]] virtual const char* category() const = 0;
    [[nodiscard]] virtual const char* description() const = 0;
    [[nodiscard]] virtual const char* iconPath() const { return nullptr; }
    
    // ─────────────────────────────────────────────────────────────────────
    // Module Access
    // ─────────────────────────────────────────────────────────────────────
    
    /// Get list of embedded modules (for ConfigPanel)
    [[nodiscard]] virtual std::vector<modules::IModule*> getModules() = 0;
    
    /// Get module by ID
    [[nodiscard]] virtual modules::IModule* getModule(const std::string& id) = 0;
    
    // ─────────────────────────────────────────────────────────────────────
    // Aggregated Parameters
    // ─────────────────────────────────────────────────────────────────────
    
    /// Get all parameters from all modules (prefixed)
    [[nodiscard]] virtual std::vector<modules::ModuleParamDesc> allParams() const = 0;
    
    /// Get parameter value (full path: "module.param")
    [[nodiscard]] virtual bool getParam(const std::string& path, 
                                        modules::ParamValue& out) const = 0;
    
    /// Set parameter value (full path: "module.param")
    virtual bool setParam(const std::string& path, 
                         const modules::ParamValue& value) = 0;
    
    /// Reset all modules to defaults
    virtual void resetAllToDefaults() = 0;
    
    // ─────────────────────────────────────────────────────────────────────
    // Lifecycle
    // ─────────────────────────────────────────────────────────────────────
    
    virtual void initialize() = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void update(const VisualContext& ctx) = 0;
    virtual void render(const VisualContext& ctx) = 0;
};

} // namespace lumi::visuals
```

### 5.2 EqualizerVisual

```cpp
// include/visuals/EqualizerVisual.hpp
#pragma once

#include "IVisual.hpp"
#include "modules/AudioSourceModule.hpp"
#include "modules/GradientModule.hpp"
#include "modules/BarsModule.hpp"
#include "modules/PeakSpawnerModule.hpp"
#include "modules/PeakParticleModule.hpp"

namespace lumi::visuals {

class EqualizerVisual : public IVisual {
public:
    // ═══════════════════════════════════════════════════════════════════
    // Static Metadata
    // ═══════════════════════════════════════════════════════════════════
    
    static const char* staticId() { return "equalizer"; }
    static const char* staticName() { return "Equalizer"; }
    static const char* staticCategory() { return "Bars"; }
    static const char* staticDescription() { 
        return "Frequency spectrum as animated bars with peak hold"; 
    }
    
    // ═══════════════════════════════════════════════════════════════════
    // IVisual Implementation
    // ═══════════════════════════════════════════════════════════════════
    
    const char* visualId() const override { return staticId(); }
    const char* displayName() const override { return staticName(); }
    const char* category() const override { return staticCategory(); }
    const char* description() const override { return staticDescription(); }
    
    std::vector<modules::IModule*> getModules() override {
        return {
            &m_AudioSource,
            &m_Gradient,
            &m_Bars,
            &m_PeakSpawner,
            &m_PeakParticle
        };
    }
    
    modules::IModule* getModule(const std::string& id) override {
        if (id == "audio") return &m_AudioSource;
        if (id == "gradient") return &m_Gradient;
        if (id == "bars") return &m_Bars;
        if (id == "peak") return &m_PeakSpawner;
        if (id == "particle") return &m_PeakParticle;
        return nullptr;
    }
    
    std::vector<modules::ModuleParamDesc> allParams() const override;
    bool getParam(const std::string& path, modules::ParamValue& out) const override;
    bool setParam(const std::string& path, const modules::ParamValue& value) override;
    void resetAllToDefaults() override;
    
    void initialize() override;
    void activate() override;
    void deactivate() override;
    void update(const VisualContext& ctx) override;
    void render(const VisualContext& ctx) override;
    
private:
    // ═══════════════════════════════════════════════════════════════════
    // Modules (Pipeline Order)
    // ═══════════════════════════════════════════════════════════════════
    
    modules::AudioSourceModule m_AudioSource;   // 1. FFT → Bands
    modules::GradientModule m_Gradient;         // 2. Color Mapping
    modules::BarsModule m_Bars;                 // 3. Bar Geometry
    modules::PeakSpawnerModule m_PeakSpawner;   // 4. Peak Markers
    modules::PeakParticleModule m_PeakParticle; // 5. Peak Particles
    
    // ═══════════════════════════════════════════════════════════════════
    // Local State
    // ═══════════════════════════════════════════════════════════════════
    
    std::vector<float> m_BandValues;
    std::vector<modules::Color4f> m_BandColors;
};

// Auto-Registration
REGISTER_VISUAL(EqualizerVisual)

} // namespace lumi::visuals
```

**Pipeline-Diagramm:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        EQUALIZER VISUAL PIPELINE                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                         AudioSourceModule                             │  │
│  │  ┌─────────────┐   ┌──────────────┐   ┌──────────────────────────┐    │  │
│  │  │ FFT Input   │──►│ Band Mapping │──►│ SmoothingModule          │    │  │
│  │  │ (2048 bins) │   │ (Log/Mel)    │   │ (EMA, 50ms)              │    │  │
│  │  └─────────────┘   └──────────────┘   └────────────┬─────────────┘    │  │
│  └────────────────────────────────────────────────────┼──────────────────┘  │
│                                                       │                     │
│                                                       ▼                     │
│                                               [Normalized Bands]            │
│                                               float[64] (0..1)              │
│                                                       │                     │
│                      ┌────────────────────────────────┼──────────────┐      │
│                      │                                │              │      │
│                      ▼                                ▼              ▼      │
│  ┌───────────────────────────┐   ┌─────────────────────────────────────┐    │
│  │     GradientModule        │   │            BarsModule               │    │
│  │  ┌─────────────────────┐  │   │  ┌───────────────────────────────┐  │    │
│  │  │ Domain: ByPosition  │  │   │  │ Orientation: BottomUp         │  │    │
│  │  │ Preset: Fire        │  │   │  │ Gap: 2px                      │  │    │
│  │  └──────────┬──────────┘  │   │  │ MinHeight: 0.01               │  │    │
│  │             │             │   │  └───────────────┬───────────────┘  │    │
│  └─────────────┼─────────────┘   └──────────────────┼──────────────────┘    │
│                │                                    │                       │
│                ▼                                    ▼                       │
│         [Band Colors]                        [Bar Geometry]                 │
│         Color4f[64]                          Rect[64]                       │
│                │                                    │                       │
│                └────────────────┬───────────────────┘                       │
│                                 ▼                                           │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                         RENDER BARS                                  │   │
│  │  ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██ ██      │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                 │                                           │
│                                 ▼                                           │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      PeakSpawnerModule                              │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │ Physics: Classic, Delay: 120ms, Gravity: 5, Falloff: 3      │    │    │
│  │  │ Color: Auto (from Gradient), Thickness: Fixed 2px           │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  │                                                                     │    │
│  │  ── ── ── ── ── ── ── ── ── ── ── ── ── ── ── ── ── ── ── ──        │    │
│  │  (Peak markers above bars)                                          │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                 │                                           │
│                                 ▼                                           │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                      PeakParticleModule                              │   │
│  │  ┌──────────────────────────────────────────────────────────────┐    │   │
│  │  │ Enabled: false (optional trail particles)                    │    │   │
│  │  └──────────────────────────────────────────────────────────────┘    │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.3 Weitere BasisVisuals

#### 5.3.1 WaveformVisual

| Module | Reihenfolge | Beschreibung |
|--------|-------------|--------------|
| `WaveformSourceModule` | 1 | PCM-Daten |
| `SmoothingModule` | 2 | Optional |
| `GradientModule` | 3 | Farbgebung |
| `LineModule` | 4 | Linien-Rendering |

#### 5.3.2 SpectrumVisual

| Module | Reihenfolge | Beschreibung |
|--------|-------------|--------------|
| `AudioSourceModule` | 1 | FFT → Bands |
| `GradientModule` | 2 | Farbgebung |
| `Spectrum3DModule` | 3 | 3D-Darstellung |
| `PeakSpawnerModule` | 4 | Peak-Marker |

#### 5.3.3 SuperscopeVisual

| Module | Reihenfolge | Beschreibung |
|--------|-------------|--------------|
| `WaveformSourceModule` | 1 | PCM-Daten |
| `SuperscopeModule` | 2 | Point-based Rendering |

#### 5.3.4 RingVisual

| Module | Reihenfolge | Beschreibung |
|--------|-------------|--------------|
| `AudioSourceModule` | 1 | FFT → Bands |
| `GradientModule` | 2 | Farbgebung |
| `RingModule` | 3 | Kreis-Segmente |

#### 5.3.5 DotPlaneVisual

| Module | Reihenfolge | Beschreibung |
|--------|-------------|--------------|
| `AudioSourceModule` | 1 | FFT → Bands |
| `DotPlaneModule` | 2 | 2D-Punktfeld |

---

## 6. Node-Graph-System (Phase N)

### 6.1 Konzept

In Phase N werden BasisVisuals in **Node-Graphen** umgewandelt. Jedes Modul wird zu einem **Node**, Verbindungen sind explizit.

### 6.2 Typen-System

#### 6.2.1 Port-Semantik (Form im Node-Editor)

| Symbol | Semantik | Beschreibung | Verwendung |
|--------|----------|--------------|------------|
| **◯** | DataFlow | Kontinuierlicher Datenstrom | PCM, Spectrum, Image, Geometry |
| **◻** | Control | Berechnetes Signal | Float, Int, Vector (von anderen Nodes) |
| **△** | UIParam | Interaktiver Parameter | Slider, Knob, Toggle (direkter User-Input) |

#### 6.2.2 Datentypen - DataFlow (◯)

| Typ | Symbol | Farbe | Beschreibung | C++ Typ |
|-----|--------|-------|--------------|---------|
| `PCM` | 🔵 | Blau | Audio-Samples (Zeit-Domain) | `PcmData` |
| `Spectrum` | 🟣 | Lila | FFT-Bins oder gemappte Bänder | `SpectrumData` |
| `Image` | 🔴 | Rot | 2D-Texture/Framebuffer | `ImageData` |
| `Geometry` | 🟠 | Orange | Vertex/Mesh-Daten | `GeometryData` |
| `Event` | 🟡 | Gelb | Trigger/Impulse (einmalig) | `EventData` |
| `String` | ⚪ | Weiß | Text-Daten | `std::string` |
| `Meta` | ⚫ | Schwarz | Strukturierte Kontext-Daten | `MetaData` |

#### 6.2.3 Datentypen - Numerisch (◻/△)

| Typ | Symbol | Farbe | Beschreibung | C++ Typ |
|-----|--------|-------|--------------|---------|
| `Bool` | ⬜ | Grau | Wahrheitswert | `bool` |
| `Int` | 🟫 | Braun | Ganzzahl (32-bit signed) | `int32_t` |
| `UInt` | 🟦 | Hellblau | Ganzzahl (32-bit unsigned) | `uint32_t` |
| `Float` | 🟩 | Grün | Fließkomma (32-bit) | `float` |
| `Vec2` | 🟪 | Pink | 2D-Vektor | `std::array<float,2>` |
| `Vec3` | 🟪 | Pink | 3D-Vektor | `std::array<float,3>` |
| `Vec4` | 🟪 | Pink | 4D-Vektor | `std::array<float,4>` |
| `Color` | 🟪 | Pink | RGBA-Farbe (normalisiert) | `std::array<float,4>` |

#### 6.2.4 Datentypen - Script/Text (◻)

| Typ | Symbol | Farbe | Beschreibung | C++ Typ |
|-----|--------|-------|--------------|---------|
| `Script` | 📜 | Gold | Lua-Code-Block | `std::string` |
| `Expression` | 📐 | Cyan | Mathematischer Ausdruck | `std::string` |

#### 6.2.5 Datentypen - Shader (◻)

| Typ | Symbol | Farbe | Beschreibung | C++ Typ |
|-----|--------|-------|--------------|---------|
| `VertexShader` | 🔷 | Dunkelblau | GLSL Vertex Shader Code | `ShaderSource` |
| `FragmentShader` | 🔶 | Dunkelorange | GLSL Fragment/Pixel Shader Code | `ShaderSource` |
| `GeometryShader` | 🔻 | Dunkelrot | GLSL Geometry Shader Code | `ShaderSource` |
| `ComputeShader` | ⬛ | Schwarz | GLSL Compute Shader Code | `ShaderSource` |
| `ShaderProgram` | 💠 | Türkis | Kompiliertes Shader-Programm | `ShaderProgram*` |

**Shader-Typ Struktur:**

```cpp
// include/visualizers/nodes/shader/ShaderTypes.hpp
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace lumi::nodes {

// ═══════════════════════════════════════════════════════════════════════════
// Shader Source
// ═══════════════════════════════════════════════════════════════════════════

enum class ShaderStage {
    Vertex,
    Fragment,
    Geometry,
    Compute
};

struct ShaderSource {
    ShaderStage stage;
    std::string code;
    std::string name;  // Für Debug/Error-Meldungen
    
    // Automatisch extrahierte Uniforms
    struct UniformInfo {
        std::string name;
        std::string type;  // "float", "vec2", "sampler2D", etc.
        bool isArray = false;
        int arraySize = 0;
    };
    std::vector<UniformInfo> uniforms;
    
    // Automatisch extrahierte Inputs/Outputs
    struct AttributeInfo {
        int location;
        std::string name;
        std::string type;
    };
    std::vector<AttributeInfo> inputs;
    std::vector<AttributeInfo> outputs;
};

// ═══════════════════════════════════════════════════════════════════════════
// Compiled Shader Program
// ═══════════════════════════════════════════════════════════════════════════

struct ShaderProgram {
    uint32_t glProgramId = 0;
    bool isCompiled = false;
    std::string compileError;
    
    // Uniform Locations (cached)
    std::unordered_map<std::string, int> uniformLocations;
    
    // Für Hot-Reload
    ShaderSource vertexSource;
    ShaderSource fragmentSource;
    ShaderSource geometrySource;  // Optional
    
    bool hasGeometryShader() const { return !geometrySource.code.empty(); }
};

} // namespace lumi::nodes
```

#### 6.2.6 Typ-Kompatibilität

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        TYPE COMPATIBILITY MATRIX                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Automatische Konvertierung (implizit):                                     │
│  ══════════════════════════════════════                                     │
│                                                                             │
│  Int ──────► Float        (verlustfrei)                                     │
│  Bool ─────► Int          (0 oder 1)                                        │
│  Bool ─────► Float        (0.0 oder 1.0)                                    │
│  Float ────► Vec2/3/4     (alle Komponenten gleich)                         │
│  Vec3 ─────► Color        (RGB, Alpha = 1.0)                                │
│  Color ────► Vec4         (direkt)                                          │
│                                                                             │
│  Explizite Konvertierung (via Converter-Node):                              │
│  ═════════════════════════════════════════════                              │
│                                                                             │
│  Float ────► Int          (truncate)                                        │
│  String ───► Float/Int    (parse)                                           │
│  Spectrum ─► Float        (average/peak)                                    │
│  PCM ──────► Float        (level)                                           │
│                                                                             │
│  Keine Konvertierung möglich:                                               │
│  ════════════════════════════                                               │
│                                                                             │
│  Image ────✖ Spectrum                                                      │
│  Geometry ─✖ PCM                                                           │
│  Event ────✖ Float (verwende EventToFloat-Node)                            │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 6.2.7 Port-Darstellung im Editor

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          PORT VISUAL REPRESENTATION                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  DataFlow Ports (◯):                                                        │
│  ═══════════════════                                                        │
│                                                                             │
│    🔵◯─ PCM        Ausgefüllter Kreis + Farbe                               │
│    🟣◯─ Spectrum                                                            │
│    🔴◯─ Image                                                               │
│                                                                             │
│  Control Ports (◻):                                                         │
│  ══════════════════                                                         │
│                                                                             │
│    🟩◻─ Float      Ausgefülltes Quadrat + Farbe                             │
│    🟫◻─ Int                                                                 │
│    🟪◻─ Vec3                                                                │
│                                                                             │
│  UIParam Ports (△):                                                         │
│  ══════════════════                                                         │
│                                                                             │
│    🟩△─ Float      Ausgefülltes Dreieck + Farbe                             │
│                    (hat zusätzlich inline Slider/Input)                     │
│                                                                             │
│  Verbundene vs. Unverbundene Ports:                                         │
│  ══════════════════════════════════                                         │
│                                                                             │
│    ●─ verbunden (ausgefüllt)                                                │
│    ○─ unverbunden (nur Rand)                                                │
│                                                                             │
│  Beispiel-Node:                                                             │
│  ┌─────────────────────────────────┐                                        │
│  │         MathNode                │                                        │
│  │                                 │                                        │
│  │  🟩●─ a        op: [+ ▼]       │                                         │
│  │  🟩○─ b (5.0)         🟩─● out │                                          │
│  │                                 │                                        │
│  └─────────────────────────────────┘                                        │
│       ↑                       ↑                                             │
│    verbunden              verbunden                                         │
│       ↑                                                                     │
│  unverbunden mit Default-Wert                                               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    BASISVISUAL → NODE-GRAPH TRANSFORMATION                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  PHASE 1 (BasisVisual)              PHASE N (Node-Graph)                    │
│  ═════════════════════              ═════════════════════                   │
│                                                                             │
│  class EqualizerVisual {            ┌─────────────────────────────────────┐ │
│    AudioSourceModule m_Audio;       │         NodeGraph "Equalizer"       │ │
│    GradientModule m_Gradient;       │                                     │ │
│    BarsModule m_Bars;        ──►    │  [AudioSrc]──►[Gradient]──►[Bars]   │ │
│    PeakModule m_Peak;               │       │            │          │     │ │
│  };                                 │       └────────────┴──►[Peak]──┘    │ │
│                                     │                                     │ │
│  // Interne Verdrahtung             │  // Explizite Connections           │ │
│  // nicht sichtbar                  │  // editierbar im NodeEditor        │ │
│                                     └─────────────────────────────────────┘ │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 6.2 INode Interface

```cpp
// include/nodes/INode.hpp
#pragma once

#include "modules/IModule.hpp"
#include <vector>
#include <memory>

namespace lumi::nodes {

// ═══════════════════════════════════════════════════════════════════════════
// Port Types
// ═══════════════════════════════════════════════════════════════════════════

enum class PortType {
    // Data Flow
    Float,
    FloatArray,
    Int,
    Bool,
    String,
    Color,
    Spectrum,      // float[] (FFT bands)
    Waveform,      // float[] (PCM samples)
    Image,         // Framebuffer texture
    Geometry,      // Vertex data
    
    // Special
    Any,           // Type-agnostic (for pass-through)
    Trigger        // Event/pulse
};

enum class PortDirection {
    Input,
    Output
};

struct PortDesc {
    std::string id;
    std::string displayName;
    PortType type;
    PortDirection direction;
    bool optional = false;
    modules::ParamValue defaultValue;  // For optional inputs
};

// ═══════════════════════════════════════════════════════════════════════════
// Node Interface
// ═══════════════════════════════════════════════════════════════════════════

class INode {
public:
    virtual ~INode() = default;
    
    // ─────────────────────────────────────────────────────────────────────
    // Identification
    // ─────────────────────────────────────────────────────────────────────
    
    [[nodiscard]] virtual const char* nodeTypeId() const = 0;
    [[nodiscard]] virtual const char* displayName() const = 0;
    [[nodiscard]] virtual const char* category() const = 0;
    [[nodiscard]] virtual const char* description() const = 0;
    
    /// Unique instance ID (set by NodeGraph)
    std::string instanceId;
    
    // ─────────────────────────────────────────────────────────────────────
    // Ports
    // ─────────────────────────────────────────────────────────────────────
    
    [[nodiscard]] virtual std::vector<PortDesc> ports() const = 0;
    
    /// Get input value (from connection or default)
    [[nodiscard]] virtual bool getInput(const std::string& portId, 
                                        modules::ParamValue& out) const = 0;
    
    /// Set output value (called during process)
    virtual void setOutput(const std::string& portId, 
                          const modules::ParamValue& value) = 0;
    
    // ─────────────────────────────────────────────────────────────────────
    // Parameters (convertible to inputs)
    // ─────────────────────────────────────────────────────────────────────
    
    [[nodiscard]] virtual std::vector<modules::ModuleParamDesc> params() const = 0;
    [[nodiscard]] virtual bool getParam(const std::string& id, 
                                        modules::ParamValue& out) const = 0;
    virtual bool setParam(const std::string& id, 
                         const modules::ParamValue& value) = 0;
    
    /// Check if parameter is exposed as input
    [[nodiscard]] virtual bool isParamExposed(const std::string& paramId) const = 0;
    
    /// Expose parameter as input port
    virtual void exposeParam(const std::string& paramId, bool expose) = 0;
    
    // ─────────────────────────────────────────────────────────────────────
    // Processing
    // ─────────────────────────────────────────────────────────────────────
    
    virtual void initialize() = 0;
    virtual void process(float deltaTime) = 0;
    
    // Optional: rendering (for visual nodes)
    virtual void render() {}
    virtual bool hasRenderOutput() const { return false; }
};

} // namespace lumi::nodes
```

### 6.3 Node-Kategorien

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           NODE CATEGORIES                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  📥 SOURCE                      📊 PROCESSING                              │
│  ────────                       ──────────────                              │
│  • AudioSourceNode              • SmoothingNode                             │
│  • BeatDetectNode               • NormalizeNode                             │
│  • WaveformSourceNode           • ClampNode                                 │
│  • MicInputNode                 • MathNode (+, -, *, /, pow, sin, cos)      │
│  • FileSourceNode               • MixNode                                   │
│                                 • SplitNode                                 │
│                                 • MergeNode                                 │
│                                                                             │
│  🎨 RENDER                      ✨ EFFECT                                  │
│  ────────                       ────────                                    │
│  • BarsNode                     • BlurNode                                  │
│  • LineNode                     • BloomNode                                 │
│  • SuperscopeNode               • ColorFadeNode                             │
│  • ShapeNode                    • MovementNode                              │
│  • RingNode                     • MirrorNode                                │
│  • TextNode                     • KaleidoscopeNode                          │
│  • PictureNode                  • InvertNode                                │
│  • DotPlaneNode                 • HSVShiftNode                              │
│                                                                             │
│  🎨 COLOR                       📦 COMPOSITE                               │
│  ─────────                      ─────────────                               │
│  • GradientNode                 • BlendNode                                 │
│  • ColorNode                    • BufferSaveNode                            │
│  • HSVNode                      • BufferLoadNode                            │
│  • RGBNode                      • MaskNode                                  │
│                                 • LayerNode                                 │
│                                                                             │
│  📐 VALUE (Native)              📝 SCRIPT                                  │
│  ────────────────               ──────────                                  │
│  • FloatNode                    • LuaNode                                   │
│  • IntNode                      • ExpressionNode                            │
│  • BoolNode                     • PerFrameScriptNode                        │
│  • StringNode                   • PerPointScriptNode                        │
│  • Vec2Node                     • PerBeatScriptNode                         │
│  • Vec3Node                                                                 │
│  • ColorNode                                                                │
│                                                                             │
│  🔧 UTILITY                     📤 OUTPUT                                  │
│  ─────────────                  ──────────                                  │
│  • CommentNode                  • DisplayNode                               │
│  • RerouteNode                  • RecordNode                                │
│  • GroupNode                    • StreamNode                                │
│  • PresetNode                                                               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 6.4 NodeGraph

```cpp
// include/nodes/NodeGraph.hpp
#pragma once

#include "INode.hpp"
#include <unordered_map>
#include <vector>

namespace lumi::nodes {

struct Connection {
    std::string fromNodeId;
    std::string fromPortId;
    std::string toNodeId;
    std::string toPortId;
};

class NodeGraph {
public:
    // ─────────────────────────────────────────────────────────────────────
    // Node Management
    // ─────────────────────────────────────────────────────────────────────
    
    /// Add node to graph, returns assigned instance ID
    std::string addNode(std::unique_ptr<INode> node);
    
    /// Remove node by instance ID
    bool removeNode(const std::string& instanceId);
    
    /// Get node by instance ID
    INode* getNode(const std::string& instanceId);
    const INode* getNode(const std::string& instanceId) const;
    
    /// Get all nodes
    std::vector<INode*> getAllNodes();
    
    // ─────────────────────────────────────────────────────────────────────
    // Connection Management
    // ─────────────────────────────────────────────────────────────────────
    
    /// Create connection between ports
    bool connect(const std::string& fromNodeId, const std::string& fromPortId,
                 const std::string& toNodeId, const std::string& toPortId);
    
    /// Remove connection
    bool disconnect(const std::string& toNodeId, const std::string& toPortId);
    
    /// Get all connections
    const std::vector<Connection>& getConnections() const;
    
    /// Check if connection is valid (type compatibility)
    bool canConnect(const std::string& fromNodeId, const std::string& fromPortId,
                    const std::string& toNodeId, const std::string& toPortId) const;
    
    // ─────────────────────────────────────────────────────────────────────
    // Execution
    // ─────────────────────────────────────────────────────────────────────
    
    /// Process all nodes in topological order
    void process(float deltaTime);
    
    /// Render all visual nodes
    void render();
    
    /// Recompute execution order (call after topology change)
    void updateExecutionOrder();
    
private:
    std::unordered_map<std::string, std::unique_ptr<INode>> m_Nodes;
    std::vector<Connection> m_Connections;
    std::vector<std::string> m_ExecutionOrder;  // Topologically sorted
    bool m_OrderDirty = true;
    int m_NextNodeId = 1;
    
    void topologicalSort();
    void propagateValues();
};

} // namespace lumi::nodes
```

---

## 7. Parameter-System

### 7.1 Parameter-Pfade

Parameter werden über hierarchische Pfade adressiert:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          PARAMETER PATH STRUCTURE                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  BasisVisual Parameter Path:                                                │
│  ───────────────────────────                                                │
│                                                                             │
│    "audio.scale"           → AudioSourceModule.scale                        │
│    "audio.smooth.timeMs"   → AudioSourceModule.SmoothingModule.timeMs       │
│    "gradient.preset"       → GradientModule.preset                          │
│    "bars.gapPx"            → BarsModule.gapPx                               │
│    "peak.delayMs"          → PeakSpawnerModule.delayMs                      │
│                                                                             │
│  Node Parameter Path:                                                       │
│  ────────────────────                                                       │
│                                                                             │
│    "audio_1.scale"         → Node instance "audio_1", param "scale"         │
│    "gradient_1.preset"     → Node instance "gradient_1", param "preset"     │
│                                                                             │
│  Syntax:                                                                    │
│  ───────                                                                    │
│                                                                             │
│    BasisVisual:  <module>.<param>                                           │
│    Nested:       <module>.<submodule>.<param>                               │
│    Node:         <instanceId>.<param>                                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 7.2 Parameter-Gruppen (für UI)

```cpp
// Parameter groups for ConfigPanel code-folding
struct ParamGroup {
    std::string id;              // "audio", "gradient", etc.
    std::string displayName;     // "Audio Source", "Color Gradient"
    std::string icon;            // Optional icon path
    int order;                   // Sort order
    bool collapsedByDefault;     // Initial fold state
    std::vector<std::string> paramIds;  // Parameters in this group
    std::vector<ParamGroup> subGroups;  // Nested groups
};
```

**Beispiel für EqualizerVisual:**

```
▼ Audio Source                    [order: 0]
  │  Scale: [Log ▼]
  │  Bands: [64]
  │  Floor dB: [-60]
  │  Ceil dB: [0]
  │
  └─▶ Smoothing                   [order: 0, nested]
       │  Algorithm: [EMA ▼]
       │  Time (ms): [50]
       └  Preset: [Balanced ▼]

▶ Color Gradient                  [order: 1, collapsed]

▶ Bars                            [order: 2, collapsed]

▼ Peak Hold                       [order: 3]
  │  ☑ Enabled
  │  Physics: [Classic ▼]
  │  Delay (ms): [120]
  │  Gravity: [5.0]
  │
  ├─▶ Appearance                  [order: 0, nested]
  │
  └─▶ Particles                   [order: 1, nested]
```

---

## 8. Parameter-zu-Input-Konvertierung

### 8.1 Konzept

Jeder Node-Parameter kann zu einem **Input-Port** konvertiert werden. Dies ermöglicht:

- **Dynamische Steuerung** durch andere Nodes
- **Animation** über Zeit-basierte Nodes
- **Audio-Reaktivität** durch Audio-Analyse-Nodes
- **Synchronisation** zwischen Nodes (shared settings)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    PARAMETER → INPUT CONVERSION                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  BEFORE (Parameter)                 AFTER (Input Port)                      │
│  ══════════════════                 ═══════════════════                     │
│                                                                             │
│  ┌─────────────────────┐            ┌─────────────────────┐                 │
│  │     BarsNode        │            │     BarsNode        │                 │
│  │                     │            │                     │                 │
│  │  gapPx: [====○===]  │            │  ●──gapPx           │ ◄── Input Port  │
│  │         ↑           │     ──►    │                     │                 │
│  │    Parameter UI     │            │  (UI hidden when    │                 │
│  │                     │            │   port connected)   │                 │
│  └─────────────────────┘            └─────────────────────┘                 │
│                                              ▲                              │
│                                              │                              │
│                                     ┌────────┴────────┐                     │
│                                     │                 │                     │
│                            ┌────────────────┐  ┌────────────────┐           │
│                            │  FloatNode     │  │ MathNode       │           │
│                            │  value: 5.0    │  │ bass * 10      │           │
│                            │  ○──out        │  │ ○──out         │           │
│                            └────────────────┘  └────────────────┘           │
│                                                                             │
│                            Statische Konfiguration  Audio-reaktiv           │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 8.2 UI-Darstellung (wie ComfyUI/Blender)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    PARAMETER EXPOSURE UI                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Parameter als Wert (Default):                                              │
│  ──────────────────────────────                                             │
│                                                                             │
│  ┌─ BarsNode ───────────────────────────────────────────────────────────┐   │
│  │                                                                      │   │
│  │    Gap (px):   [====○======]  5.0     [⚙]  ◄── Click to expose      │   │
│  │                                                                      │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│                                                                             │
│  Parameter als Input (exponiert):                                           │
│  ─────────────────────────────────                                          │
│                                                                             │
│  ┌─ BarsNode ───────────────────────────────────────────────────────────┐   │
│  │                                                                      │   │
│  │  ●─ Gap (px)   [not connected: 5.0]   [⚙]  ◄── Click to unexpose    │   │
│  │  │                                                                   │   │
│  │  │  (Parameter UI shows default when unconnected)                    │   │
│  │                                                                      │   │
│  └──┼───────────────────────────────────────────────────────────────────┘   │
│     │                                                                       │
│     │  Connection                                                           │
│     │                                                                       │
│  ┌──┴─ FloatNode ───────────────────────────────────────────────────────┐   │
│  │                                                                      │   │
│  │    Value:   [====○======]  5.0                         ─○ Out        │   │
│  │                                                                      │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│                                                                             │
│  Parameter mit Verbindung (überschrieben):                                  │
│  ──────────────────────────────────────────                                 │
│                                                                             │
│  ┌─ BarsNode ───────────────────────────────────────────────────────────┐   │
│  │                                                                      │   │
│  │  ●─ Gap (px)   ← FloatNode.out                        [⚙] [✖]      │   │
│  │  │             (Parameter UI hidden, value from connection)          │   │
│  │                                                                      │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  [✖] = Disconnect button                                                    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 8.3 Implementation

```cpp
class INode {
public:
    // ...
    
    // ─────────────────────────────────────────────────────────────────────
    // Parameter Exposure
    // ─────────────────────────────────────────────────────────────────────
    
    /// Get dynamic port list (base ports + exposed params)
    std::vector<PortDesc> allPorts() const {
        auto ports = basePorts();
        
        for (const auto& param : params()) {
            if (isParamExposed(param.id)) {
                ports.push_back(PortDesc{
                    .id = "param_" + param.id,
                    .displayName = param.displayName,
                    .type = paramTypeToPortType(param.type),
                    .direction = PortDirection::Input,
                    .optional = true,
                    .defaultValue = param.defaultValue
                });
            }
        }
        
        return ports;
    }
    
    /// Get effective parameter value (from connection or local)
    template<typename T>
    T getEffectiveParam(const std::string& paramId) const {
        if (isParamExposed(paramId)) {
            modules::ParamValue connectedValue;
            if (getInput("param_" + paramId, connectedValue)) {
                return std::get<T>(connectedValue);
            }
        }
        
        modules::ParamValue localValue;
        getParam(paramId, localValue);
        return std::get<T>(localValue);
    }
    
private:
    std::set<std::string> m_ExposedParams;
};
```

---

## 9. Native Value Nodes

### 9.1 Übersicht

Native Value Nodes sind einfache Nodes, die konstante oder berechnete Werte ausgeben:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          NATIVE VALUE NODES                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐                 │
│  │   FloatNode    │  │    IntNode     │  │   BoolNode     │                 │
│  │                │  │                │  │                │                 │
│  │  value: 1.5    │  │  value: 42     │  │  value: ☑     │                 │
│  │         ─○ out │  │         ─○ out │  │         ─○ out │                 │
│  └────────────────┘  └────────────────┘  └────────────────┘                 │
│                                                                             │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐                 │
│  │  StringNode    │  │   Vec2Node     │  │   Vec3Node     │                 │
│  │                │  │                │  │                │                 │
│  │  value: "Hi"   │  │  x: 0.5        │  │  x: 1.0        │                 │
│  │         ─○ out │  │  y: 0.3        │  │  y: 0.5        │                 │
│  └────────────────┘  │         ─○ out │  │  z: 0.0        │                 │
│                      └────────────────┘  │         ─○ out │                 │
│                                          └────────────────┘                 │
│                                                                             │
│  ┌────────────────┐  ┌────────────────────────────────────┐                 │
│  │   ColorNode    │  │           MathNode                 │                 │
│  │                │  │                                    │                 │
│  │  r: 1.0        │  │  ●─ a          operation: [+ ▼]    │                 │
│  │  g: 0.5        │  │  ●─ b                      ─○ out  │                 │
│  │  b: 0.0        │  │                                    │                 │
│  │  a: 1.0        │  │  Operations: +, -, *, /, pow,      │                 │
│  │         ─○ out │  │              sin, cos, tan, abs,   │                 │
│  └────────────────┘  │              min, max, clamp,      │                 │
│                      │              lerp, smoothstep      │                 │
│                      └────────────────────────────────────┘                 │
│                                                                             │
│  ┌────────────────────────────────────────────────────────────────────┐     │
│  │                         TimeNode                                   │     │
│  │                                                                    │     │
│  │  ─○ time      (seconds since start)                                │     │
│  │  ─○ delta     (seconds since last frame)                           │     │
│  │  ─○ frame     (frame number)                                       │     │
│  │  ─○ beat      (1.0 on beat, decays)                                │     │
│  │  ─○ bpm       (estimated BPM)                                      │     │
│  └────────────────────────────────────────────────────────────────────┘     │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                        AudioLevelNode                               │    │
│  │                                                                     │    │
│  │  ─○ bass      (low frequency level 0..1)                            │    │
│  │  ─○ mid       (mid frequency level 0..1)                            │    │
│  │  ─○ treb      (high frequency level 0..1)                           │    │
│  │  ─○ left      (left channel level 0..1)                             │    │
│  │  ─○ right     (right channel level 0..1)                            │    │
│  │  ─○ mono      (mono level 0..1)                                     │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 9.2 Vollständige Native Value Node Referenz

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    COMPLETE NATIVE VALUE NODE REFERENCE                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🟩 FLOAT NODE                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  FloatNode               Category: Value/Numeric                      │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          🟩─◻ out     (Float)                │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  value        Float     -∞..+∞        Default: 0.0                    │  │
│  │  min          Float     Slider-Min    Default: 0.0                    │  │
│  │  max          Float     Slider-Max    Default: 1.0                    │  │
│  │  step         Float     Inkrement     Default: 0.01                   │  │
│  │                                                                       │  │
│  │  UI-WIDGET                                                            │  │
│  │  ─────────                                                            │  │
│  │  [═══════○═══════]  0.5                                               │  │
│  │   ↑              ↑   ↑                                                │  │
│  │  min           max  value input                                       │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🟫 INT NODE                                                                │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  IntNode                 Category: Value/Numeric                      │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          🟫─◻ out     (Int)                  │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  value        Int       -2³¹..2³¹-1   Default: 0                      │  │
│  │  min          Int       Slider-Min    Default: 0                      │  │
│  │  max          Int       Slider-Max    Default: 100                    │  │
│  │  step         Int       Inkrement     Default: 1                      │  │
│  │                                                                       │  │
│  │  UI-WIDGET                                                            │  │
│  │  ─────────                                                            │  │
│  │  [═══════○═══════]  42     [▲]                                        │  │
│  │                            [▼]                                        │  │
│  │                         Spinbox-Arrows                                │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  ⬜ BOOL NODE                                                                │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  BoolNode                Category: Value/Logic                        │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          ⬜─◻ out     (Bool)                  │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  value        Bool      true/false    Default: false                  │  │
│  │                                                                       │  │
│  │  UI-WIDGET                                                            │  │
│  │  ─────────                                                            │  │
│  │  ☑ Enabled    oder    [ON|off]  (Toggle-Switch)                      │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  ⚪ STRING NODE                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  StringNode              Category: Value/Text                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          ⚪─◻ out     (String)               │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  value        String    Beliebiger Text                               │  │
│  │  multiline    Bool      Mehrzeilige Eingabe    Default: false         │  │
│  │                                                                       │  │
│  │  UI-WIDGET                                                            │  │
│  │  ─────────                                                            │  │
│  │  Single-line: [Hello World_____________]                              │  │
│  │  Multi-line:  ┌────────────────────────┐                              │  │
│  │               │ Line 1                 │                              │  │
│  │               │ Line 2                 │                              │  │
│  │               └────────────────────────┘                              │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🟪 VEC2 NODE                                                               │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  Vec2Node                Category: Value/Vector                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ x      (Float, optional)   🟪─◻ vec     (Vec2)                   │  │
│  │  🟩◻─ y      (Float, optional)   🟩─◻ x       (Float)                  │  │
│  │                                   🟩─◻ y       (Float)                │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  x            Float     -∞..+∞        Default: 0.0                    │  │
│  │  y            Float     -∞..+∞        Default: 0.0                    │  │
│  │                                                                       │  │
│  │  UI-WIDGET                                                            │  │
│  │  ─────────                                                            │  │
│  │  X: [═══○═══]  0.5    Y: [═══○═══]  0.3                               │  │
│  │                                                                       │  │
│  │  ODER als 2D-Picker:                                                  │  │
│  │  ┌─────────────┐                                                      │  │
│  │  │      ●      │  ← Drag-Punkt                                        │  │
│  │  │             │                                                      │  │
│  │  └─────────────┘                                                      │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🟪 VEC3 NODE                                                               │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  Vec3Node                Category: Value/Vector                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ x      (Float, optional)   🟪─◻ vec     (Vec3)                  │  │
│  │  🟩◻─ y      (Float, optional)   🟩─◻ x       (Float)                 │  │
│  │  🟩◻─ z      (Float, optional)   🟩─◻ y       (Float)                 │  │
│  │                                   🟩─◻ z       (Float)                │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  x            Float     -∞..+∞        Default: 0.0                    │  │
│  │  y            Float     -∞..+∞        Default: 0.0                    │  │
│  │  z            Float     -∞..+∞        Default: 0.0                    │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🟪 VEC4 NODE                                                               │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  Vec4Node                Category: Value/Vector                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ x      (Float, optional)   🟪─◻ vec     (Vec4)                  │  │
│  │  🟩◻─ y      (Float, optional)   🟩─◻ x       (Float)                 │  │
│  │  🟩◻─ z      (Float, optional)   🟩─◻ y       (Float)                 │  │
│  │  🟩◻─ w      (Float, optional)   🟩─◻ z       (Float)                 │  │
│  │                                   🟩─◻ w       (Float)                │  │
│  │                                                                      │  │
│  │  PARAMETERS                                                          │  │
│  │  ──────────                                                          │  │
│  │  x            Float     -∞..+∞        Default: 0.0                   │  │
│  │  y            Float     -∞..+∞        Default: 0.0                   │  │
│  │  z            Float     -∞..+∞        Default: 0.0                   │  │
│  │  w            Float     -∞..+∞        Default: 0.0                   │  │
│  │                                                                      │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                            │
│  ══════════════════════════════════════════════════════════════════════════│
│  🟪 COLOR NODE                                                             │
│  ══════════════════════════════════════════════════════════════════════════│
│                                                                            │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │  ColorNode               Category: Value/Color                       │  │
│  ├──────────────────────────────────────────────────────────────────────┤  │
│  │                                                                      │  │
│  │  INPUTS                           OUTPUTS                            │  │
│  │  ────────                         ────────                           │  │
│  │  🟩◻─ r      (Float, optional)   🟪─◻ color   (Color)                 │  │
│  │  🟩◻─ g      (Float, optional)   🟩─◻ r       (Float)                 │  │
│  │  🟩◻─ b      (Float, optional)   🟩─◻ g       (Float)                 │  │
│  │  🟩◻─ a      (Float, optional)   🟩─◻ b       (Float)                 │  │
│  │                                   🟩─◻ a       (Float)               │  │
│  │                                                                      │  │
│  │  PARAMETERS                                                          │  │
│  │  ──────────                                                          │  │
│  │  r            Float     0..1          Default: 1.0                   │  │
│  │  g            Float     0..1          Default: 1.0                   │  │
│  │  b            Float     0..1          Default: 1.0                   │  │
│  │  a            Float     0..1          Default: 1.0                   │  │
│  │  hex          String    "#RRGGBBAA"   (Alternativ-Input)             │  │
│  │                                                                      │  │
│  │  UI-WIDGET                                                           │  │
│  │  ─────────                                                           │  │
│  │  [████] ← Color Swatch (Click öffnet Picker)                         │  │
│  │  ┌─────────────────────────┐                                         │  │
│  │  │    Color Wheel / HSV    │                                         │  │
│  │  │    Alpha Slider         │                                         │  │
│  │  │    Hex: #FF8000FF       │                                         │  │
│  │  └─────────────────────────┘                                         │  │
│  │                                                                      │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                            │
│  ═══════════════════════════════════════════════════════════════════════════│
│  ⏱️ TIME NODE                                                               │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  TimeNode                Category: Value/System                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine - System-Node)            🟩─◻ time    (Float) Sekunden       │  │
│  │                                   🟩─◻ delta   (Float) Delta-Time     │  │
│  │                                   🟫─◻ frame   (Int)   Frame-Nummer   │  │
│  │                                   🟩─◻ fps     (Float) Framerate      │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  (keine - Read-only Outputs)                                          │  │
│  │                                                                       │  │
│  │  INFO-DISPLAY                                                         │  │
│  │  ────────────                                                         │  │
│  │  Time:  00:05:23.456                                                  │  │
│  │  Frame: 19234                                                         │  │
│  │  FPS:   60.0                                                          │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🎵 AUDIO LEVEL NODE                                                        │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  AudioLevelNode          Category: Value/Audio                        │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine - System-Node)            🟩─◻ bass    (Float) 20-250 Hz      │  │
│  │                                   🟩─◻ mid     (Float) 250-4000 Hz    │  │
│  │                                   🟩─◻ treb    (Float) 4000-20000 Hz  │  │
│  │                                   🟩─◻ left    (Float) L-Channel      │  │
│  │                                   🟩─◻ right   (Float) R-Channel      │  │
│  │                                   🟩─◻ mono    (Float) (L+R)/2        │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  smoothing    Float     0..1     Glättung        Default: 0.1         │  │
│  │  normalize    Bool               Auto-Normalize  Default: false       │  │
│  │                                                                       │  │
│  │  LIVE-DISPLAY                                                         │  │
│  │  ────────────                                                         │  │
│  │  Bass: [████████░░]  0.82                                             │  │
│  │  Mid:  [████░░░░░░]  0.45                                             │  │
│  │  Treb: [██░░░░░░░░]  0.23                                             │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🥁 BEAT NODE                                                               │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  BeatNode                Category: Value/Audio                        │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine - System-Node)            🟩─◻ beat      (Float) 0..1 decay   │  │
│  │                                   🟫─◻ beatCount (Int)   Total beats  │  │
│  │                                   🟩─◻ bpm       (Float) Estimated    │  │
│  │                                   🟩─◻ phase     (Float) 0..1 cycle   │  │
│  │                                   🟡─◯ trigger   (Event) Beat-Pulse   │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  sensitivity  Float     0.1-3.0  Beat-Schwelle   Default: 1.0         │  │
│  │  decayRate    Float     0.1-20   Decay-Speed     Default: 5.0         │  │
│  │  minBpm       Float     60-200   BPM-Untergrenze Default: 80          │  │
│  │  maxBpm       Float     80-300   BPM-Obergrenze  Default: 180         │  │
│  │                                                                       │  │
│  │  LIVE-DISPLAY                                                         │  │
│  │  ────────────                                                         │  │
│  │  ●───────────────── (beat indicator, pulses on beat)                  │  │
│  │  BPM: 128    Beats: 523                                               │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📐 MATH NODE                                                               │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  MathNode                Category: Math/Basic                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ a      (Float)             🟩─◻ out     (Float)                  │  │
│  │  🟩◻─ b      (Float, optional)                                        │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  operation    Enum      Siehe unten    Default: Add                   │  │
│  │  defaultB     Float     Fallback wenn b nicht verbunden               │  │
│  │                                                                       │  │
│  │  OPERATIONEN (Einzel-Input: nur a)                                    │  │
│  │  ─────────────────────────────────                                    │  │
│  │  Negate      -a                                                       │  │
│  │  Abs         |a|                                                      │  │
│  │  Sign        sign(a) → -1, 0, 1                                       │  │
│  │  Floor       floor(a)                                                 │  │
│  │  Ceil        ceil(a)                                                  │  │
│  │  Round       round(a)                                                 │  │
│  │  Fract       a - floor(a)                                             │  │
│  │  Sqrt        √a                                                       │  │
│  │  Exp         eᵃ                                                       │  │
│  │  Log         ln(a)                                                    │  │
│  │  Log10       log₁₀(a)                                                 │  │
│  │  Sin         sin(a)                                                   │  │
│  │  Cos         cos(a)                                                   │  │
│  │  Tan         tan(a)                                                   │  │
│  │  Asin        arcsin(a)                                                │  │
│  │  Acos        arccos(a)                                                │  │
│  │  Atan        arctan(a)                                                │  │
│  │                                                                       │  │
│  │  OPERATIONEN (Zwei Inputs: a, b)                                      │  │
│  │  ───────────────────────────────                                      │  │
│  │  Add         a + b                                                    │  │
│  │  Subtract    a - b                                                    │  │
│  │  Multiply    a * b                                                    │  │
│  │  Divide      a / b (b≠0, sonst 0)                                     │  │
│  │  Modulo      a % b                                                    │  │
│  │  Power       aᵇ                                                       │  │
│  │  Min         min(a, b)                                                │  │
│  │  Max         max(a, b)                                                │  │
│  │  Atan2       atan2(a, b)                                              │  │
│  │  Step        b < a ? 0 : 1                                            │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📊 CLAMP NODE                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ClampNode               Category: Math/Range                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ value  (Float)             🟩─◻ out     (Float)                 │  │
│  │  🟩◻─ min    (Float, optional)                                        │  │
│  │  🟩◻─ max    (Float, optional)                                        │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  min          Float              Default: 0.0                         │  │
│  │  max          Float              Default: 1.0                         │  │
│  │                                                                       │  │
│  │  FORMEL                                                               │  │
│  │  ──────                                                               │  │
│  │  out = max(min, min(max, value))                                      │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📈 LERP NODE                                                               │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  LerpNode                Category: Math/Interpolation                 │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ a      (Float)             🟩─◻ out     (Float)                 │  │
│  │  🟩◻─ b      (Float)                                                  │  │
│  │  🟩◻─ t      (Float)  0..1 mix                                        │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  a            Float              Default: 0.0                         │  │
│  │  b            Float              Default: 1.0                         │  │
│  │  t            Float              Default: 0.5                         │  │
│  │  clampT       Bool               Clamp t to 0..1   Default: true      │  │
│  │                                                                       │  │
│  │  FORMEL                                                               │  │
│  │  ──────                                                               │  │
│  │  out = a + (b - a) * t                                                │  │
│  │  out = a * (1 - t) + b * t                                            │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🗺️ MAP RANGE NODE                                                         │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  MapRangeNode            Category: Math/Range                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ value  (Float)             🟩─◻ out     (Float)                 │  │
│  │  🟩◻─ inMin  (Float, optional)                                        │  │
│  │  🟩◻─ inMax  (Float, optional)                                        │  │
│  │  🟩◻─ outMin (Float, optional)                                        │  │
│  │  🟩◻─ outMax (Float, optional)                                        │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  inMin        Float              Default: 0.0                         │  │
│  │  inMax        Float              Default: 1.0                         │  │
│  │  outMin       Float              Default: 0.0                         │  │
│  │  outMax       Float              Default: 1.0                         │  │
│  │  clamp        Bool               Clamp output      Default: false     │  │
│  │                                                                       │  │
│  │  FORMEL                                                               │  │
│  │  ──────                                                               │  │
│  │  t = (value - inMin) / (inMax - inMin)                                │  │
│  │  out = outMin + t * (outMax - outMin)                                 │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│  〰️ SMOOTHSTEP NODE                                                         │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  SmoothstepNode          Category: Math/Interpolation                 │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ x      (Float)             🟩─◻ out     (Float)                │  │
│  │  🟩◻─ edge0  (Float, optional)                                       │  │
│  │  🟩◻─ edge1  (Float, optional)                                       │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  edge0        Float              Default: 0.0                        │  │
│  │  edge1        Float              Default: 1.0                        │  │
│  │                                                                       │  │
│  │  FORMEL                                                               │  │
│  │  ──────                                                               │  │
│  │  t = clamp((x - edge0) / (edge1 - edge0), 0, 1)                      │  │
│  │  out = t * t * (3 - 2 * t)   // Hermite interpolation                │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🎲 RANDOM NODE                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  RandomNode              Category: Value/Random                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟡◯─ reseed (Event, optional)   🟩─◻ out     (Float)                │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  min          Float              Default: 0.0                        │  │
│  │  max          Float              Default: 1.0                        │  │
│  │  seed         Int                Default: 0 (random)                 │  │
│  │  mode         Enum               PerFrame/OnTrigger/Once             │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🌊 NOISE NODE                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  NoiseNode               Category: Value/Random                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ x      (Float)             🟩─◻ out     (Float) -1..1          │  │
│  │  🟩◻─ y      (Float, optional)                                       │  │
│  │  🟩◻─ z      (Float, optional)                                       │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  type         Enum      Perlin/Simplex/Value/Worley                  │  │
│  │  octaves      Int       1-8         Default: 4                       │  │
│  │  frequency    Float     0.01-100    Default: 1.0                     │  │
│  │  persistence  Float     0-1         Default: 0.5                     │  │
│  │  seed         Int                   Default: 0                       │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🔀 COMPARE NODE                                                            │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  CompareNode             Category: Math/Logic                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ a      (Float)             ⬜─◻ out     (Bool)                 │  │
│  │  🟩◻─ b      (Float)                                                 │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  operation    Enum      ==, !=, <, <=, >, >=                         │  │
│  │  epsilon      Float     Toleranz für ==     Default: 0.0001          │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🔲 SELECT NODE (IF/ELSE)                                                   │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  SelectNode              Category: Math/Logic                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  ⬜◻─ condition (Bool)           🟩─◻ out     (Float)                │  │
│  │  🟩◻─ ifTrue    (Float)                                              │  │
│  │  🟩◻─ ifFalse   (Float)                                              │  │
│  │                                                                       │  │
│  │  FORMEL                                                               │  │
│  │  ──────                                                               │  │
│  │  out = condition ? ifTrue : ifFalse                                  │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 9.3 Beispiel: Audio-reaktiver Parameter

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    AUDIO-REACTIVE BLUR RADIUS                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Goal: Blur-Radius pulsiert mit Bass                                        │
│                                                                              │
│  ┌──────────────────┐                                                       │
│  │  AudioLevelNode  │                                                       │
│  │                  │                                                       │
│  │           ─○ bass├───┐                                                   │
│  └──────────────────┘   │                                                   │
│                         │                                                   │
│                         ▼                                                   │
│  ┌──────────────────────────────────┐                                       │
│  │           MathNode               │                                       │
│  │                                  │                                       │
│  │  ●─ a (bass)    op: [* ▼]       │                                       │
│  │  ●─ b ──────────────────────┐   │                                       │
│  │                      ─○ out─┼───┼──┐                                    │
│  └──────────────────────────────┘   │  │                                    │
│                         ▲           │  │                                    │
│  ┌──────────────────┐   │           │  │                                    │
│  │    FloatNode     │   │           │  │                                    │
│  │                  │   │           │  │                                    │
│  │  value: 20.0  ─○─┼───┘           │  │     (bass * 20 = 0..20)           │
│  └──────────────────┘               │  │                                    │
│                                     │  │                                    │
│  ┌──────────────────────────────────┘  │                                    │
│  │                                     │                                    │
│  │  ┌──────────────────────────────────┴──┐                                │
│  │  │           MathNode                  │                                │
│  │  │                                     │                                │
│  └──┼●─ a (bass*20)    op: [+ ▼]         │                                │
│     │●─ b ──────────────────────┐        │                                │
│     │                    ─○ out─┼────────┼──┐                              │
│     └──────────────────────────────┘     │  │                              │
│                         ▲                │  │                              │
│  ┌──────────────────┐   │                │  │                              │
│  │    FloatNode     │   │                │  │                              │
│  │                  │   │                │  │                              │
│  │  value: 5.0   ─○─┼───┘                │  │    (bass*20 + 5 = 5..25)    │
│  └──────────────────┘                    │  │                              │
│                                          │  │                              │
│                                          │  │                              │
│  ┌───────────────────────────────────────┘  │                              │
│  │                                          │                              │
│  │  ┌───────────────────────────────────────┴──┐                           │
│  │  │              BlurNode                    │                           │
│  │  │                                          │                           │
│  └──┼●─ radius (exposed)                       │                           │
│     │●─ image                         ─○ out   │                           │
│     │                                          │                           │
│     └──────────────────────────────────────────┘                           │
│                                                                              │
│  Result: Blur radius = 5 + (bass * 20) = 5..25 px                           │
│          (minimum 5px, maximum 25px when bass is at 1.0)                    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 10. Text/Script Nodes

### 10.1 Konzept

Text-Nodes enthalten **Code-Strings** für dynamische Konfiguration. Sie werden in verschiedenen Kontexten ausgeführt:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           TEXT/SCRIPT NODES                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌────────────────────────────────────────────────────────────────────┐    │
│  │                       ExpressionNode                                │    │
│  │                                                                     │    │
│  │  ●─ a                                                              │    │
│  │  ●─ b              expression: [sin(a * 3.14) * b        ]        │    │
│  │  ●─ c                                                    ─○ out   │    │
│  │  ●─ d                                                              │    │
│  │                                                                     │    │
│  │  • Einfache mathematische Ausdrücke                                │    │
│  │  • Variablen: a, b, c, d (inputs), t (time), beat, bass, mid, treb │    │
│  │  • Funktionen: sin, cos, tan, abs, min, max, clamp, lerp, pow     │    │
│  └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│  ┌────────────────────────────────────────────────────────────────────┐    │
│  │                         LuaNode                                     │    │
│  │                                                                     │    │
│  │  ●─ in1                                                            │    │
│  │  ●─ in2                                                   ─○ out1 │    │
│  │  ●─ in3                                                   ─○ out2 │    │
│  │                                                                     │    │
│  │  ┌──────────────────────────────────────────────────────────────┐  │    │
│  │  │  -- Lua Script                                               │  │    │
│  │  │  local phase = t * 2 * math.pi                               │  │    │
│  │  │  out1 = math.sin(phase) * in1                                │  │    │
│  │  │  out2 = beat > 0.5 and in2 or in3                            │  │    │
│  │  └──────────────────────────────────────────────────────────────┘  │    │
│  │                                                                     │    │
│  │  • Vollständiges Lua 5.4                                           │    │
│  │  • Sandboxed (kein I/O, limitierte Ausführungszeit)                │    │
│  │  • Zugriff auf alle System-Variablen                               │    │
│  └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10.2 Script-Kontexte (Hooks)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SCRIPT CONTEXTS / HOOKS                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Hook           │ Aufruf-Zeitpunkt           │ Verwendung                   │
│  ═══════════════╪════════════════════════════╪══════════════════════════════│
│  onInit         │ Visual wird aktiviert      │ Initialisierung, Reset       │
│  ───────────────┼────────────────────────────┼──────────────────────────────│
│  perFrame       │ Einmal pro Frame           │ Globale Updates, Animation   │
│  ───────────────┼────────────────────────────┼──────────────────────────────│
│  perBeat        │ Bei erkanntem Beat         │ Beat-synchrone Effekte       │
│  ───────────────┼────────────────────────────┼──────────────────────────────│
│  perPoint       │ Für jeden Punkt (Scope)    │ Punkt-Position, Farbe        │
│  ───────────────┼────────────────────────────┼──────────────────────────────│
│  perPixel       │ Für jeden Pixel (Shader)   │ Displacement, Farb-Transform │
│  ───────────────┼────────────────────────────┼──────────────────────────────│
│  perVertex      │ Für jeden Vertex (3D)      │ Vertex-Transformation        │
│  ───────────────┼────────────────────────────┼──────────────────────────────│
│  perBand        │ Für jedes Frequenzband     │ Band-spezifische Effekte     │
│  ───────────────┼────────────────────────────┼──────────────────────────────│
│  onParamChange  │ Parameter wurde geändert   │ Validierung, Abhängigkeiten  │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10.3 SuperscopeNode mit Scripts

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SUPERSCOPE WITH SCRIPTS                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌────────────────────────────────────────────────────────────────────┐    │
│  │                      SuperscopeNode                                 │    │
│  │                                                                     │    │
│  │  Points:    [512]                                                  │    │
│  │  DrawMode:  [Lines ▼]                                              │    │
│  │  LineWidth: [2.0]                                                  │    │
│  │                                                                     │    │
│  │  ┌─ onInit ─────────────────────────────────────────────────────┐  │    │
│  │  │  -- Called once when visual becomes active                   │  │    │
│  │  │  rot = 0                                                     │  │    │
│  │  │  scale = 1                                                   │  │    │
│  │  └──────────────────────────────────────────────────────────────┘  │    │
│  │                                                                     │    │
│  │  ┌─ perFrame ───────────────────────────────────────────────────┐  │    │
│  │  │  -- Called once per frame                                    │  │    │
│  │  │  rot = rot + dt * 0.5                                        │  │    │
│  │  │  scale = 0.5 + bass * 0.5                                    │  │    │
│  │  └──────────────────────────────────────────────────────────────┘  │    │
│  │                                                                     │    │
│  │  ┌─ perPoint ───────────────────────────────────────────────────┐  │    │
│  │  │  -- Called for each point (i = 0..1, v = audio value)        │  │    │
│  │  │  local angle = i * 2 * math.pi + rot                         │  │    │
│  │  │  local radius = 0.3 + v * 0.4                                │  │    │
│  │  │  x = math.cos(angle) * radius * scale                        │  │    │
│  │  │  y = math.sin(angle) * radius * scale                        │  │    │
│  │  │  r = 0.2 + v * 0.8                                           │  │    │
│  │  │  g = 1.0 - i                                                 │  │    │
│  │  │  b = i                                                       │  │    │
│  │  │  a = 1                                                       │  │    │
│  │  └──────────────────────────────────────────────────────────────┘  │    │
│  │                                                                     │    │
│  └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│  Ergebnis: Rotierender Kreis, dessen Radius mit Audio pulsiert             │
│            Farbe variiert entlang des Kreises (Rot→Blau)                    │
│            Helligkeit reagiert auf Audio-Amplitude                          │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10.4 Vollständige Text/Script Node-Referenz

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    COMPLETE TEXT/SCRIPT NODE REFERENCE                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📐 EXPRESSION NODE                                                         │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ExpressionNode         Category: Script/Math                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ a      (Float, optional)   🟩─◻ out    (Float)                 │  │
│  │  🟩◻─ b      (Float, optional)                                        │  │
│  │  🟩◻─ c      (Float, optional)                                        │  │
│  │  🟩◻─ d      (Float, optional)                                        │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  expression   String    "sin(a * 3.14) * b"                          │  │
│  │                                                                       │  │
│  │  AVAILABLE VARIABLES                                                  │  │
│  │  ───────────────────                                                  │  │
│  │  a, b, c, d    Input-Werte (oder 0 wenn nicht verbunden)             │  │
│  │  t             Zeit (Sekunden)                                        │  │
│  │  dt            Delta-Time                                             │  │
│  │  beat          Beat-Intensität (0..1)                                 │  │
│  │  bass          Bass-Level (0..1)                                      │  │
│  │  mid           Mitten-Level (0..1)                                    │  │
│  │  treb          Höhen-Level (0..1)                                     │  │
│  │  pi            3.14159...                                             │  │
│  │  e             2.71828...                                             │  │
│  │                                                                       │  │
│  │  AVAILABLE FUNCTIONS                                                  │  │
│  │  ───────────────────                                                  │  │
│  │  sin, cos, tan, asin, acos, atan, atan2                              │  │
│  │  abs, floor, ceil, round, sign, fract                                │  │
│  │  min, max, clamp, lerp, smoothstep                                   │  │
│  │  pow, sqrt, exp, log, log10                                          │  │
│  │  mod (%), step, pulse                                                 │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📜 LUA NODE                                                                │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  LuaNode                 Category: Script/Advanced                    │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ in1    (Float, optional)   🟩─◻ out1   (Float)                 │  │
│  │  🟩◻─ in2    (Float, optional)   🟩─◻ out2   (Float)                 │  │
│  │  🟩◻─ in3    (Float, optional)   🟩─◻ out3   (Float)                 │  │
│  │  🟩◻─ in4    (Float, optional)   🟩─◻ out4   (Float)                 │  │
│  │  🟪◻─ color  (Color, optional)   🟪─◻ colorOut (Color)               │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  script       TextArea   Lua-Code (multi-line)                       │  │
│  │  autoInputs   Bool       Automatisch Inputs aus Code erkennen        │  │
│  │  autoOutputs  Bool       Automatisch Outputs aus Code erkennen       │  │
│  │                                                                       │  │
│  │  EXECUTION                                                            │  │
│  │  ─────────                                                            │  │
│  │  • Sandboxed (kein os, io, debug)                                    │  │
│  │  • Timeout: 10ms pro Aufruf                                          │  │
│  │  • Zugriff auf alle System-Variablen                                 │  │
│  │  • State bleibt zwischen Frames erhalten                             │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🎬 PER-FRAME SCRIPT NODE                                                   │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  PerFrameScriptNode      Category: Script/Timing                      │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          🟩─◻ value1  (Float)               │  │
│  │                                   🟩─◻ value2  (Float)               │  │
│  │                                   🟩─◻ value3  (Float)               │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  initScript     TextArea   Einmalig bei Aktivierung                  │  │
│  │  frameScript    TextArea   Jeden Frame ausgeführt                    │  │
│  │                                                                       │  │
│  │  TYPISCHER USE-CASE                                                   │  │
│  │  ──────────────────                                                   │  │
│  │  • Globale Animationsvariablen                                       │  │
│  │  • Akkumulatoren (z.B. Rotationswinkel)                              │  │
│  │  • Beat-reaktive Pulsierungen                                        │  │
│  │                                                                       │  │
│  │  BEISPIEL                                                             │  │
│  │  ────────                                                             │  │
│  │  -- initScript:                                                       │  │
│  │  rot = 0                                                              │  │
│  │  pulse = 0                                                            │  │
│  │                                                                       │  │
│  │  -- frameScript:                                                      │  │
│  │  rot = rot + dt * 0.5                                                 │  │
│  │  pulse = lerp(pulse, beat, dt * 10)                                   │  │
│  │  value1 = rot                                                         │  │
│  │  value2 = pulse                                                       │  │
│  │  value3 = math.sin(t * 2)                                             │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🎯 PER-POINT SCRIPT NODE                                                   │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  PerPointScriptNode      Category: Script/Render                      │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟣◯─ spectrum (Spectrum)        🟠─◯ geometry (Geometry)            │  │
│  │  🟩◻─ param1  (Float, optional)                                      │  │
│  │  🟩◻─ param2  (Float, optional)                                      │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  points       Int        Anzahl Punkte (32-4096)                     │  │
│  │  initScript   TextArea   Einmalig bei Aktivierung                    │  │
│  │  frameScript  TextArea   Einmal pro Frame (vor perPoint)             │  │
│  │  pointScript  TextArea   Für jeden Punkt ausgeführt                  │  │
│  │                                                                       │  │
│  │  POINT-KONTEXT VARIABLEN                                              │  │
│  │  ────────────────────────                                             │  │
│  │  n      int     Punkt-Index (0..points-1)                            │  │
│  │  i      float   Normalisierter Index (0..1)                          │  │
│  │  v      float   Audio-Wert an diesem Punkt                           │  │
│  │                                                                       │  │
│  │  OUTPUT VARIABLEN (zu setzen in pointScript)                         │  │
│  │  ───────────────────────────────────────────                         │  │
│  │  x, y, z    Position (-1..1)                                         │  │
│  │  r, g, b, a Farbe (0..1)                                             │  │
│  │  skip       bool (true = Punkt überspringen)                         │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🥁 PER-BEAT SCRIPT NODE                                                    │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  PerBeatScriptNode       Category: Script/Audio                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟡◯─ beatIn  (Event, optional)  🟡─◯ trigger  (Event)               │  │
│  │                                   🟩─◻ value    (Float)               │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  initScript   TextArea   Einmalig bei Aktivierung                    │  │
│  │  beatScript   TextArea   Bei jedem erkannten Beat                    │  │
│  │  frameScript  TextArea   Jeden Frame (für Decay etc.)                │  │
│  │                                                                       │  │
│  │  BEAT-KONTEXT VARIABLEN                                               │  │
│  │  ───────────────────────                                              │  │
│  │  beatCount    int     Anzahl Beats seit Start                        │  │
│  │  beatIntensity float  Stärke des aktuellen Beats (0..1)              │  │
│  │  bpm          float   Geschätzte BPM                                 │  │
│  │  timeSinceBeat float  Sekunden seit letztem Beat                     │  │
│  │                                                                       │  │
│  │  TYPISCHER USE-CASE                                                   │  │
│  │  ──────────────────                                                   │  │
│  │  • Farb-Zyklen bei Beat                                              │  │
│  │  • Einmalige Effekt-Trigger                                          │  │
│  │  • Zähler für Patterns                                               │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🖼️ PER-PIXEL SCRIPT NODE (SHADER-LIKE)                                    │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  PerPixelScriptNode      Category: Script/Effect                      │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🔴◯─ image   (Image)            🔴─◯ image   (Image)                │  │
│  │  🟩◻─ param1  (Float, optional)                                      │  │
│  │  🟩◻─ param2  (Float, optional)                                      │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  pixelScript  TextArea   Für jeden Pixel (GPU-accelerated via GLSL)  │  │
│  │                                                                       │  │
│  │  PIXEL-KONTEXT VARIABLEN                                              │  │
│  │  ────────────────────────                                             │  │
│  │  u, v       float   UV-Koordinaten (0..1)                            │  │
│  │  px, py     int     Pixel-Koordinaten                                │  │
│  │  inR/G/B/A  float   Eingangs-Pixel-Farbe                             │  │
│  │                                                                       │  │
│  │  OUTPUT VARIABLEN                                                     │  │
│  │  ────────────────                                                     │  │
│  │  outR/G/B/A float   Ausgangs-Pixel-Farbe                             │  │
│  │  -- ODER --                                                           │  │
│  │  du, dv     float   Displacement für Warp-Effekte                    │  │
│  │                                                                       │  │
│  │  HINWEIS                                                              │  │
│  │  ───────                                                              │  │
│  │  Wird intern zu GLSL-Shader kompiliert für GPU-Performance           │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📊 PER-BAND SCRIPT NODE                                                    │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  PerBandScriptNode       Category: Script/Audio                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟣◯─ spectrum (Spectrum)        🟣─◯ spectrum (Spectrum, modified)  │  │
│  │  🟩◻─ param1  (Float, optional)  🟠─◯ geometry (Geometry, optional)  │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  bandScript   TextArea   Für jedes Band ausgeführt                   │  │
│  │  outputMode   Enum       ModifySpectrum / GenerateGeometry / Both    │  │
│  │                                                                       │  │
│  │  BAND-KONTEXT VARIABLEN                                               │  │
│  │  ───────────────────────                                              │  │
│  │  n          int     Band-Index (0..bands-1)                          │  │
│  │  i          float   Normalisierter Index (0..1)                      │  │
│  │  v          float   Aktueller Band-Wert (0..1)                       │  │
│  │  freq       float   Ungefähre Frequenz dieses Bands (Hz)             │  │
│  │  bands      int     Gesamtanzahl Bänder                              │  │
│  │                                                                       │  │
│  │  OUTPUT VARIABLEN                                                     │  │
│  │  ────────────────                                                     │  │
│  │  v          float   Modifizierter Band-Wert (wenn ModifySpectrum)    │  │
│  │  x, y       float   Position (wenn GenerateGeometry)                 │  │
│  │  r, g, b, a float   Farbe (wenn GenerateGeometry)                    │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📝 STRING NODE                                                             │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  StringNode              Category: Value/Text                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          ⚪─◻ out     (String)               │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  value        String    Der auszugebende Text                        │  │
│  │  multiline    Bool      Mehrzeilige Eingabe erlauben                 │  │
│  │                                                                       │  │
│  │  USE-CASES                                                            │  │
│  │  ─────────                                                            │  │
│  │  • Script-Code für LuaNode                                           │  │
│  │  • Text für TextOverlayNode                                          │  │
│  │  • Dateinamen, Pfade                                                 │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🔗 STRING CONCAT NODE                                                      │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  StringConcatNode        Category: Value/Text                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  ⚪◻─ a       (String)           ⚪─◻ out     (String)               │  │
│  │  ⚪◻─ b       (String)                                                │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  separator    String    Trennzeichen (default: "")                   │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🔢 STRING FORMAT NODE                                                      │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  StringFormatNode        Category: Value/Text                         │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🟩◻─ value   (Float)            ⚪─◻ out     (String)               │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  format       String    Printf-Format (z.B. "%.2f", "%d")            │  │
│  │  prefix       String    Präfix (z.B. "BPM: ")                        │  │
│  │  suffix       String    Suffix (z.B. " Hz")                          │  │
│  │                                                                       │  │
│  │  USE-CASES                                                            │  │
│  │  ─────────                                                            │  │
│  │  • BPM-Anzeige: "BPM: 120"                                           │  │
│  │  • FPS-Counter: "60 FPS"                                             │  │
│  │  • Debug-Output                                                       │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10.5 Shader Nodes

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SHADER NODE REFERENCE                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Shader Nodes ermöglichen die direkte Programmierung von GPU-Shadern.       │
│  Sie verwenden GLSL (OpenGL Shading Language) und werden zur Laufzeit       │
│  kompiliert. Fehler werden im Node-Editor angezeigt.                        │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🔷 VERTEX SHADER NODE                                                      │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  VertexShaderNode        Category: Shader/Stage                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          🔷─◻ shader   (VertexShader)       │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  code         TextArea   GLSL Vertex Shader Code                     │  │
│  │  name         String     Name für Debug-Meldungen                    │  │
│  │                                                                       │  │
│  │  DEFAULT CODE                                                         │  │
│  │  ────────────                                                         │  │
│  │  #version 330 core                                                    │  │
│  │  layout(location = 0) in vec3 aPos;                                  │  │
│  │  layout(location = 1) in vec2 aTexCoord;                             │  │
│  │                                                                       │  │
│  │  out vec2 vTexCoord;                                                 │  │
│  │                                                                       │  │
│  │  uniform mat4 uMVP;                                                  │  │
│  │  uniform float uTime;                                                │  │
│  │                                                                       │  │
│  │  void main() {                                                       │  │
│  │      vTexCoord = aTexCoord;                                          │  │
│  │      gl_Position = uMVP * vec4(aPos, 1.0);                          │  │
│  │  }                                                                   │  │
│  │                                                                       │  │
│  │  AUTO-UNIFORMS (automatisch gebunden)                                │  │
│  │  ────────────────────────────────────                                │  │
│  │  uTime       float     Zeit (Sekunden)                               │  │
│  │  uDelta      float     Delta-Time                                    │  │
│  │  uResolution vec2      Viewport-Größe                                │  │
│  │  uMVP        mat4      Model-View-Projection Matrix                  │  │
│  │  uBass       float     Bass-Level                                    │  │
│  │  uMid        float     Mid-Level                                     │  │
│  │  uTreb       float     Treble-Level                                  │  │
│  │  uBeat       float     Beat-Intensität                               │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🔶 FRAGMENT SHADER NODE                                                    │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  FragmentShaderNode      Category: Shader/Stage                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          🔶─◻ shader   (FragmentShader)     │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  code         TextArea   GLSL Fragment Shader Code                   │  │
│  │  name         String     Name für Debug-Meldungen                    │  │
│  │                                                                       │  │
│  │  DEFAULT CODE                                                         │  │
│  │  ────────────                                                         │  │
│  │  #version 330 core                                                    │  │
│  │  in vec2 vTexCoord;                                                  │  │
│  │  out vec4 fragColor;                                                 │  │
│  │                                                                       │  │
│  │  uniform float uTime;                                                │  │
│  │  uniform sampler2D uTexture;                                         │  │
│  │                                                                       │  │
│  │  void main() {                                                       │  │
│  │      vec4 tex = texture(uTexture, vTexCoord);                        │  │
│  │      fragColor = tex;                                                │  │
│  │  }                                                                   │  │
│  │                                                                       │  │
│  │  ZUSÄTZLICHE AUTO-UNIFORMS                                           │  │
│  │  ─────────────────────────                                           │  │
│  │  uTexture    sampler2D  Input-Textur (wenn verbunden)                │  │
│  │  uSpectrum   sampler1D  Spectrum-Daten als 1D-Textur                 │  │
│  │  uWaveform   sampler1D  Waveform-Daten als 1D-Textur                 │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🔻 GEOMETRY SHADER NODE                                                    │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  GeometryShaderNode      Category: Shader/Stage                       │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          🔻─◻ shader   (GeometryShader)     │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  code         TextArea   GLSL Geometry Shader Code                   │  │
│  │  inputType    Enum       Points/Lines/Triangles                      │  │
│  │  outputType   Enum       Points/LineStrip/TriangleStrip              │  │
│  │  maxVertices  Int        Max. Output-Vertices (1-256)                │  │
│  │                                                                       │  │
│  │  TYPISCHER USE-CASE                                                   │  │
│  │  ──────────────────                                                   │  │
│  │  • Partikel aus Punkten generieren                                   │  │
│  │  • Lines zu Ribbons expandieren                                      │  │
│  │  • Geometrie duplizieren/modifizieren                                │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  ⬛ COMPUTE SHADER NODE                                                     │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ComputeShaderNode       Category: Shader/Compute                     │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🔴◯─ input   (Image, optional)  ⬛─◻ shader   (ComputeShader)       │  │
│  │                                   🔴─◯ output   (Image)               │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  code         TextArea   GLSL Compute Shader Code                    │  │
│  │  workGroupX   Int        Work Group Size X (1-1024)                  │  │
│  │  workGroupY   Int        Work Group Size Y (1-1024)                  │  │
│  │  workGroupZ   Int        Work Group Size Z (1-64)                    │  │
│  │                                                                       │  │
│  │  DEFAULT CODE                                                         │  │
│  │  ────────────                                                         │  │
│  │  #version 430 core                                                    │  │
│  │  layout(local_size_x = 16, local_size_y = 16) in;                    │  │
│  │  layout(rgba32f, binding = 0) uniform image2D uOutput;               │  │
│  │                                                                       │  │
│  │  uniform float uTime;                                                │  │
│  │  uniform vec2 uResolution;                                           │  │
│  │                                                                       │  │
│  │  void main() {                                                       │  │
│  │      ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);                  │  │
│  │      vec2 uv = vec2(pixel) / uResolution;                            │  │
│  │      vec4 color = vec4(uv, 0.5 + 0.5 * sin(uTime), 1.0);             │  │
│  │      imageStore(uOutput, pixel, color);                              │  │
│  │  }                                                                   │  │
│  │                                                                       │  │
│  │  TYPISCHER USE-CASE                                                   │  │
│  │  ──────────────────                                                   │  │
│  │  • Partikel-Simulation (GPU-basiert)                                 │  │
│  │  • Post-Processing Effekte                                           │  │
│  │  • Prozedurales Textur-Generieren                                    │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  💠 SHADER PROGRAM NODE                                                     │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ShaderProgramNode       Category: Shader/Program                     │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  🔷◻─ vertex    (VertexShader)   💠─◻ program  (ShaderProgram)       │  │
│  │  🔶◻─ fragment  (FragmentShader)                                     │  │
│  │  🔻◻─ geometry  (GeometryShader, optional)                           │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  autoCompile   Bool      Bei Änderung neu kompilieren                │  │
│  │  showErrors    Bool      Compile-Errors im Node anzeigen             │  │
│  │                                                                       │  │
│  │  STATUS-ANZEIGE                                                       │  │
│  │  ──────────────                                                       │  │
│  │  ✅ Compiled successfully                                            │  │
│  │  -- ODER --                                                           │  │
│  │  ❌ Error in fragment shader:                                        │  │
│  │     Line 15: undefined variable 'colorr'                             │  │
│  │                                                                       │  │
│  │  HINWEIS                                                              │  │
│  │  ───────                                                              │  │
│  │  Verbindet Shader-Stages zu einem ausführbaren Programm.             │  │
│  │  Vertex + Fragment sind Pflicht, Geometry ist optional.              │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  🎨 SHADER RENDER NODE                                                      │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ShaderRenderNode        Category: Shader/Render                      │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  💠◻─ program   (ShaderProgram)  🔴─◯ image    (Image)               │  │
│  │  🔴◯─ texture0  (Image, optional)                                    │  │
│  │  🔴◯─ texture1  (Image, optional)                                    │  │
│  │  🔴◯─ texture2  (Image, optional)                                    │  │
│  │  🟣◯─ spectrum  (Spectrum, optional)                                 │  │
│  │  🟠◯─ geometry  (Geometry, optional)                                 │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  renderMode    Enum      Fullscreen/Geometry/Points                  │  │
│  │  blendMode     Enum      None/Alpha/Add/Multiply                     │  │
│  │  clearColor    Color     Hintergrundfarbe (wenn Clear)               │  │
│  │  clearBuffer   Bool      Buffer vor Render clearen                   │  │
│  │                                                                       │  │
│  │  DYNAMISCHE UNIFORMS                                                  │  │
│  │  ───────────────────                                                  │  │
│  │  Zusätzliche Uniforms können als Input-Ports exponiert werden:       │  │
│  │                                                                       │  │
│  │  🟩◻─ u_customFloat  → uniform float u_customFloat;                  │  │
│  │  🟪◻─ u_customVec3   → uniform vec3 u_customVec3;                    │  │
│  │  🟪◻─ u_customColor  → uniform vec4 u_customColor;                   │  │
│  │                                                                       │  │
│  │  (Parameter können zu Inputs konvertiert werden via [⚙])             │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📄 SHADER TEMPLATE NODES (Presets)                                         │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  Vorgefertigte Shader für häufige Effekte:                                  │
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  GlowShaderNode          Category: Shader/Preset                      │  │
│  │  ─────────────────────────────────────────────────────────────────────│  │
│  │  Bloom/Glow-Effekt mit konfigurierbarer Schwelle und Intensität      │  │
│  │  Parameters: threshold, intensity, radius, iterations                 │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  BlurShaderNode          Category: Shader/Preset                      │  │
│  │  ─────────────────────────────────────────────────────────────────────│  │
│  │  Gaussian/Kawase Blur mit konfigurierbarem Radius                    │  │
│  │  Parameters: radius, direction (H/V/Both), quality                    │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ChromaticShaderNode     Category: Shader/Preset                      │  │
│  │  ─────────────────────────────────────────────────────────────────────│  │
│  │  Chromatische Aberration (RGB-Versatz)                               │  │
│  │  Parameters: amount, angle, radial                                    │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  DistortShaderNode       Category: Shader/Preset                      │  │
│  │  ─────────────────────────────────────────────────────────────────────│  │
│  │  Verzerrungseffekte (Warp, Ripple, Swirl)                            │  │
│  │  Parameters: type, amount, center, frequency                          │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  KaleidoscopeShaderNode  Category: Shader/Preset                      │  │
│  │  ─────────────────────────────────────────────────────────────────────│  │
│  │  Kaleidoskop-Spiegelung mit N Segmenten                              │  │
│  │  Parameters: segments, rotation, zoom, center                         │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  FeedbackShaderNode      Category: Shader/Preset                      │  │
│  │  ─────────────────────────────────────────────────────────────────────│  │
│  │  Feedback-Loop mit Transformation (wie AVS Dynamic Movement)         │  │
│  │  Parameters: zoom, rotation, translation, decay                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ColorGradeShaderNode    Category: Shader/Preset                      │  │
│  │  ─────────────────────────────────────────────────────────────────────│  │
│  │  Farbkorrektur und LUT-Anwendung                                     │  │
│  │  Parameters: contrast, brightness, saturation, hueShift, lut         │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ═══════════════════════════════════════════════════════════════════════════│
│  📝 GLSL INCLUDE NODE (Code-Bibliothek)                                     │
│  ═══════════════════════════════════════════════════════════════════════════│
│                                                                              │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  GlslIncludeNode         Category: Shader/Utility                     │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │                                                                       │  │
│  │  INPUTS                           OUTPUTS                             │  │
│  │  ────────                         ────────                            │  │
│  │  (keine)                          ⚪─◻ code     (String)             │  │
│  │                                                                       │  │
│  │  PARAMETERS                                                           │  │
│  │  ──────────                                                           │  │
│  │  library      Enum      Noise/Math/Color/Shapes/SDF                  │  │
│  │                                                                       │  │
│  │  VERFÜGBARE BIBLIOTHEKEN                                              │  │
│  │  ───────────────────────                                              │  │
│  │                                                                       │  │
│  │  Noise:    fbm(), perlin(), simplex(), voronoi(), worley()           │  │
│  │  Math:     map(), smoothstep(), bias(), gain(), pulse()              │  │
│  │  Color:    rgb2hsv(), hsv2rgb(), blend(), colormap()                 │  │
│  │  Shapes:   circle(), rect(), polygon(), star(), line()               │  │
│  │  SDF:      sdCircle(), sdBox(), sdHexagon(), opUnion(), opSmooth()   │  │
│  │                                                                       │  │
│  │  VERWENDUNG                                                           │  │
│  │  ──────────                                                           │  │
│  │  Der Output wird als #include am Anfang des Shader-Codes eingefügt.  │  │
│  │  Mehrere GlslIncludeNodes können kombiniert werden.                  │  │
│  │                                                                       │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10.6 Shader-Pipeline Beispiel

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SHADER PIPELINE EXAMPLE                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Audio-reaktiver Custom Shader mit Glow:                                    │
│                                                                              │
│  ┌──────────────────┐                                                       │
│  │ VertexShaderNode │                                                       │
│  │  (passthrough)   │───────┐                                               │
│  │           🔷─○   │       │                                               │
│  └──────────────────┘       │                                               │
│                             │                                               │
│  ┌──────────────────┐       │   ┌────────────────────┐                      │
│  │FragmentShaderNode│       ├──►│  ShaderProgramNode │                      │
│  │                  │       │   │                    │                      │
│  │  // Custom GLSL  │───────┘   │  🔷●─ vertex      │                      │
│  │  void main() {   │           │  🔶●─ fragment    │                      │
│  │    vec3 col = .. │           │                    │                      │
│  │  }               │           │            💠─○   ─┼──┐                   │
│  │           🔶─○   │           └────────────────────┘  │                   │
│  └──────────────────┘                                   │                   │
│                                                         │                   │
│  ┌──────────────────┐                                   │                   │
│  │  AudioLevelNode  │                                   │                   │
│  │           🟩─○ bass ───────────────────────────────┐ │                   │
│  │           🟩─○ mid  ──────────────────────────────┐│ │                   │
│  │           🟩─○ treb ─────────────────────────────┐││ │                   │
│  └──────────────────┘                               │││ │                   │
│                                                     │││ │                   │
│  ┌──────────────────┐                               │││ │                   │
│  │ AudioSourceNode  │                               │││ │                   │
│  │           🟣─○ spectrum ────────────────────────┐│││ │                   │
│  └──────────────────┘                              ││││ │                   │
│                                                    ││││ │                   │
│                     ┌──────────────────────────────┼┼┼┼─┘                   │
│                     │                              ││││                     │
│                     ▼                              ││││                     │
│  ┌──────────────────────────────────────────────┐  ││││                     │
│  │           ShaderRenderNode                   │  ││││                     │
│  │                                              │  ││││                     │
│  │  💠●─ program                                │◄─┘│││                     │
│  │  🟣●─ spectrum                               │◄──┘││                     │
│  │  🟩●─ u_bass (exposed)                      │◄───┘│                     │
│  │  🟩●─ u_mid  (exposed)                      │◄────┘                     │
│  │  🟩●─ u_treb (exposed)                      │◄─────┘                    │
│  │                                              │                           │
│  │  renderMode: Fullscreen                      │                           │
│  │  blendMode: None                             │                           │
│  │                                    🔴─○ image ┼──┐                       │
│  └──────────────────────────────────────────────┘  │                       │
│                                                    │                       │
│                                                    ▼                       │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │                        GlowShaderNode                                 │  │
│  │                                                                       │  │
│  │  🔴●─ input                                                          │  │
│  │                                                                       │  │
│  │  threshold: 0.7                                                      │  │
│  │  intensity: 1.5                                                      │  │
│  │  radius: 10                                                          │  │
│  │                                                           🔴─○ output│  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                              │              │
│                                                              ▼              │
│                                                        [Final Image]        │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 11. Event-Hooks & Scripting

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          SYSTEM VARIABLES                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  TIMING                                                                     │
│  ══════                                                                     │
│  t          float    Zeit seit Start (Sekunden)                             │
│  dt         float    Delta-Time (Sekunden seit letztem Frame)               │
│  frame      int      Frame-Nummer                                           │
│  fps        float    Aktuelle Framerate                                     │
│                                                                              │
│  AUDIO LEVELS                                                               │
│  ════════════                                                               │
│  bass       float    Bass-Level (20-250 Hz), 0..1                           │
│  mid        float    Mitten-Level (250-4000 Hz), 0..1                       │
│  treb       float    Höhen-Level (4000-20000 Hz), 0..1                      │
│  left       float    Linker Kanal Level, 0..1                               │
│  right      float    Rechter Kanal Level, 0..1                              │
│  mono       float    Mono Level (L+R)/2, 0..1                               │
│                                                                              │
│  BEAT                                                                       │
│  ════                                                                       │
│  beat       float    Beat-Intensität (1.0 bei Beat, decays), 0..1          │
│  beatCount  int      Anzahl erkannter Beats seit Start                      │
│  bpm        float    Geschätzte BPM                                         │
│  beatPhase  float    Phase im Beat-Zyklus, 0..1                             │
│                                                                              │
│  VIEWPORT                                                                   │
│  ════════                                                                   │
│  w          int      Viewport-Breite (Pixel)                                │
│  h          int      Viewport-Höhe (Pixel)                                  │
│  aspect     float    Seitenverhältnis (w/h)                                 │
│                                                                              │
│  POINT/PIXEL CONTEXT (nur in perPoint/perPixel)                            │
│  ══════════════════════════════════════════════                            │
│  n          int      Index des Punktes/Pixels                               │
│  i          float    Normalisierter Index, 0..1                             │
│  v          float    Audio-Wert an diesem Punkt, 0..1                       │
│                                                                              │
│  OUTPUT VARIABLES (schreibbar)                                              │
│  ══════════════════════════════                                             │
│  x          float    X-Position, -1..1                                      │
│  y          float    Y-Position, -1..1                                      │
│  z          float    Z-Position (3D), -1..1                                 │
│  r          float    Rot-Komponente, 0..1                                   │
│  g          float    Grün-Komponente, 0..1                                  │
│  b          float    Blau-Komponente, 0..1                                  │
│  a          float    Alpha-Komponente, 0..1                                 │
│  skip       bool     Punkt überspringen (nicht zeichnen)                    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 11.2 Lua API

```lua
-- ═══════════════════════════════════════════════════════════════════════════
-- MATH FUNCTIONS (erweitert)
-- ═══════════════════════════════════════════════════════════════════════════

-- Standard Lua math.* verfügbar
-- Plus:
clamp(value, min, max)      -- Wert begrenzen
lerp(a, b, t)               -- Lineare Interpolation
smoothstep(edge0, edge1, x) -- Smooth Hermite Interpolation
map(value, inMin, inMax, outMin, outMax)  -- Range Mapping
wrap(value, min, max)       -- Wrap-around (für Winkel etc.)
sign(value)                 -- -1, 0, oder 1
fract(value)                -- Nachkommaanteil

-- ═══════════════════════════════════════════════════════════════════════════
-- NOISE FUNCTIONS
-- ═══════════════════════════════════════════════════════════════════════════

noise(x)                    -- 1D Perlin Noise
noise(x, y)                 -- 2D Perlin Noise
noise(x, y, z)              -- 3D Perlin Noise
rand()                      -- Random 0..1
rand(min, max)              -- Random in Range
randSeed(seed)              -- Set random seed

-- ═══════════════════════════════════════════════════════════════════════════
-- COLOR FUNCTIONS
-- ═══════════════════════════════════════════════════════════════════════════

rgb(r, g, b)                -- Set r,g,b (0..1)
rgba(r, g, b, a)            -- Set r,g,b,a (0..1)
hsv(h, s, v)                -- Set color from HSV
hsl(h, s, l)                -- Set color from HSL
blend(r1,g1,b1, r2,g2,b2, t) -- Blend two colors

-- ═══════════════════════════════════════════════════════════════════════════
-- EASING FUNCTIONS
-- ═══════════════════════════════════════════════════════════════════════════

easeIn(t, power)            -- Ease In (power = 2 for quad, 3 for cubic, etc.)
easeOut(t, power)           -- Ease Out
easeInOut(t, power)         -- Ease In-Out
bounce(t)                   -- Bounce easing
elastic(t)                  -- Elastic easing

-- ═══════════════════════════════════════════════════════════════════════════
-- AUDIO ACCESS
-- ═══════════════════════════════════════════════════════════════════════════

getBand(index)              -- Get specific frequency band (0..numBands-1)
getBandRange(low, high)     -- Get average of band range
getWaveform(channel, index) -- Get waveform sample (channel: 0=L, 1=R, 2=mono)
```

### 11.3 Beispiel-Scripts

**Lissajous-Figur (perPoint):**
```lua
-- Lissajous with audio modulation
local freqX = 3 + bass * 2
local freqY = 2 + mid * 2
local phaseShift = t * 0.5

x = math.sin(i * freqX * 2 * math.pi + phaseShift)
y = math.sin(i * freqY * 2 * math.pi)

-- Color based on position
r = (x + 1) / 2
g = (y + 1) / 2
b = 1 - i
a = 1
```

**Beat-Pulse (perFrame):**
```lua
-- Global scale that pulses on beat
if beat > 0.8 then
    pulseScale = 1.5
end
pulseScale = lerp(pulseScale, 1.0, dt * 5)
```

**Spectrum Morph (perBand):**
```lua
-- Morph between bar and circle based on time
local morphT = (math.sin(t) + 1) / 2  -- 0..1 oscillating

-- Bar position
local barX = map(i, 0, 1, -0.9, 0.9)
local barY = v * 0.8

-- Circle position
local angle = i * 2 * math.pi - math.pi / 2
local circleX = math.cos(angle) * (0.3 + v * 0.3)
local circleY = math.sin(angle) * (0.3 + v * 0.3)

-- Interpolate
x = lerp(barX, circleX, morphT)
y = lerp(barY, circleY, morphT)
```

---

## 12. ConfigPanel Integration

### 12.1 UI-Struktur

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         CONFIGPANEL STRUCTURE                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌─ ConfigPanel ───────────────────────────────────────────────────────┐   │
│  │                                                                      │   │
│  │  Active Visual: [Equalizer ▼]                         [Reset All]   │   │
│  │  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━   │   │
│  │                                                                      │   │
│  │  ┌─ Module Groups (Scrollable) ─────────────────────────────────┐   │   │
│  │  │                                                               │   │   │
│  │  │  ▼ Audio Source                                    [↺ Reset] │   │   │
│  │  │  ╔═════════════════════════════════════════════════════════╗ │   │   │
│  │  │  ║  Scale:        [Linear ▼] [Log] [Mel]                   ║ │   │   │
│  │  │  ║  Bands:        [====○=====] 64                          ║ │   │   │
│  │  │  ║  Floor dB:     [===○======] -60                         ║ │   │   │
│  │  │  ║  Ceil dB:      [========○=] 0                           ║ │   │   │
│  │  │  ║                                                         ║ │   │   │
│  │  │  ║  ▼ Smoothing                                            ║ │   │   │
│  │  │  ║  ┌─────────────────────────────────────────────────────┐║ │   │   │
│  │  │  ║  │  Algorithm:  [None ▼] [SMA] [EMA] [WMA] [DEMA]      │║ │   │   │
│  │  │  ║  │  Time (ms):  [===○=====] 50                         │║ │   │   │
│  │  │  ║  │  Preset:     [─ Balanced ─▼]                        │║ │   │   │
│  │  │  ║  └─────────────────────────────────────────────────────┘║ │   │   │
│  │  │  ╚═════════════════════════════════════════════════════════╝ │   │   │
│  │  │                                                               │   │   │
│  │  │  ▶ Color Gradient                                  [↺ Reset] │   │   │
│  │  │  ├─────────────────────────────────────────────────────────┤ │   │   │
│  │  │                                                               │   │   │
│  │  │  ▶ Bars                                            [↺ Reset] │   │   │
│  │  │  ├─────────────────────────────────────────────────────────┤ │   │   │
│  │  │                                                               │   │   │
│  │  │  ▼ Peak Hold                                       [↺ Reset] │   │   │
│  │  │  ╔═════════════════════════════════════════════════════════╗ │   │   │
│  │  │  ║  ☑ Enabled                                              ║ │   │   │
│  │  │  ║  Physics:      [Classic ▼] [Spring]                     ║ │   │   │
│  │  │  ║  Delay (ms):   [====○=====] 120                         ║ │   │   │
│  │  │  ║  Gravity:      [===○======] 5.0                         ║ │   │   │
│  │  │  ║  Falloff:      [===○======] 3.0                         ║ │   │   │
│  │  │  ║                                                         ║ │   │   │
│  │  │  ║  ▶ Appearance                                           ║ │   │   │
│  │  │  ║  ├─────────────────────────────────────────────────────┤║ │   │   │
│  │  │  ║                                                         ║ │   │   │
│  │  │  ║  ▶ Particles                                            ║ │   │   │
│  │  │  ║  ├─────────────────────────────────────────────────────┤║ │   │   │
│  │  │  ╚═════════════════════════════════════════════════════════╝ │   │   │
│  │  │                                                               │   │   │
│  │  └───────────────────────────────────────────────────────────────┘   │   │
│  │                                                                      │   │
│  │  ┌─ Footer ──────────────────────────────────────────────────────┐  │   │
│  │  │  Settings apply to the active visualizer                       │  │   │
│  │  │  [Save as Preset]  [Load Preset]                              │  │   │
│  │  └────────────────────────────────────────────────────────────────┘  │   │
│  │                                                                      │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 12.2 Widget-Typen

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          PARAMETER WIDGETS                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  SLIDER (Float/Int)                                                         │
│  ══════════════════                                                         │
│  Label:     [═══════○═══════]  42.5                     [value input]      │
│              ↑       ↑       ↑                                              │
│            min    thumb    max                                              │
│                                                                              │
│  CHECKBOX (Bool)                                                            │
│  ═══════════════                                                            │
│  ☑ Enable Feature            ☐ Disable Feature                             │
│                                                                              │
│  DROPDOWN (Enum)                                                            │
│  ═══════════════                                                            │
│  Mode:  [Option A ▼]                                                        │
│         ┌──────────┐                                                        │
│         │ Option A │  ← selected                                            │
│         │ Option B │                                                        │
│         │ Option C │                                                        │
│         └──────────┘                                                        │
│                                                                              │
│  BUTTON GROUP (Enum, wenige Optionen)                                       │
│  ════════════════════════════════════                                       │
│  Mode:  [Linear] [Log] [Mel]                                                │
│              ↑                                                               │
│           selected (highlighted)                                            │
│                                                                              │
│  COLOR PICKER (Color4f)                                                     │
│  ══════════════════════                                                     │
│  Color:  [■■■■] ← click to open picker                                     │
│          ┌────────────────────────┐                                         │
│          │   Color Wheel / HSV    │                                         │
│          │   Alpha Slider         │                                         │
│          │   Hex Input            │                                         │
│          └────────────────────────┘                                         │
│                                                                              │
│  TEXT INPUT (String)                                                        │
│  ═══════════════════                                                        │
│  Name:  [___________________________]                                       │
│                                                                              │
│  TEXT AREA (Script)                                                         │
│  ══════════════════                                                         │
│  perPoint:                                                                  │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │ x = math.sin(i * 3.14)                                              │   │
│  │ y = v * 0.8                                                         │   │
│  │ r = i                                                               │   │
│  │ g = 1 - i                                                           │   │
│  │ b = v                                                               │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│  VEC2/VEC3/VEC4 (multi-component)                                          │
│  ════════════════════════════════                                           │
│  Position:  X [===○==] 0.5   Y [====○=] 0.3   Z [==○===] 0.0              │
│                                                                              │
│  KNOB (Float, für Winkel)                                                   │
│  ════════════════════════                                                   │
│  Rotation:    ╭───╮                                                         │
│              ╱  ●  ╲   45°                                                  │
│             │   │   │                                                       │
│              ╲     ╱                                                        │
│               ╰───╯                                                         │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 13. Preset-System

### 13.1 Preset-Struktur

```json
{
  "name": "Neon Fire Bars",
  "description": "Vibrant fire gradient with reactive peaks",
  "author": "User",
  "created": "2025-12-31T12:00:00Z",
  "visualType": "equalizer",
  "version": "1.0",
  "thumbnail": "base64...",
  
  "modules": {
    "audio": {
      "scale": "log",
      "bands": 64,
      "floorDb": -60,
      "ceilDb": 0,
      "smooth": {
        "algorithm": "EMA",
        "timeMs": 50,
        "preset": "balanced"
      }
    },
    "gradient": {
      "preset": "fire",
      "domain": "byPosition"
    },
    "bars": {
      "orientation": "bottomUp",
      "gapPx": 2,
      "roundedCorners": false
    },
    "peak": {
      "enabled": true,
      "physicsMode": "classic",
      "delayMs": 120,
      "gravity": 5,
      "falloff": 3,
      "colorMode": "auto"
    },
    "particle": {
      "enabled": false
    }
  }
}
```

### 13.2 Preset-Browser UI

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          PRESET BROWSER                                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Filter: [All Types ▼]  [Search...________________]                         │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━   │
│                                                                              │
│  ┌─ Built-in ──────────────────────────────────────────────────────────┐   │
│  │                                                                      │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐            │   │
│  │  │ ████████ │  │ ████████ │  │ ████████ │  │ ████████ │            │   │
│  │  │ ████████ │  │ ████████ │  │ ████████ │  │ ████████ │            │   │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘            │   │
│  │   Classic       Neon Fire     Ocean Calm    Rainbow                 │   │
│  │                                                                      │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│  ┌─ User Presets ──────────────────────────────────────────────────────┐   │
│  │                                                                      │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐                          │   │
│  │  │ ████████ │  │ ████████ │  │    +     │  ← Save Current          │   │
│  │  │ ████████ │  │ ████████ │  │          │                          │   │
│  │  └──────────┘  └──────────┘  └──────────┘                          │   │
│  │   My Preset 1   My Preset 2                                         │   │
│  │                                                                      │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│  Selected: "Neon Fire"                                                      │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │  Vibrant fire gradient with reactive peaks                           │  │
│  │  Author: Built-in                                                    │  │
│  │                                                                      │  │
│  │  [Apply]  [Apply & Close]  [Export...]                              │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 14. Serialisierung

### 14.1 BasisVisual JSON

```json
{
  "type": "equalizer",
  "version": "1.0",
  "params": {
    "audio.scale": "log",
    "audio.bands": 64,
    "audio.floorDb": -60,
    "audio.ceilDb": 0,
    "audio.smooth.algorithm": "EMA",
    "audio.smooth.timeMs": 50,
    "gradient.preset": "fire",
    "gradient.domain": "byPosition",
    "bars.orientation": "bottomUp",
    "bars.gapPx": 2,
    "peak.enabled": true,
    "peak.physicsMode": "classic",
    "peak.delayMs": 120
  }
}
```

### 14.2 NodeGraph JSON

```json
{
  "type": "nodeGraph",
  "version": "1.0",
  "name": "Custom Equalizer",
  
  "nodes": [
    {
      "id": "audio_1",
      "type": "audioSource",
      "position": [100, 100],
      "params": {
        "scale": "log",
        "bands": 64
      },
      "exposedParams": []
    },
    {
      "id": "gradient_1",
      "type": "gradient",
      "position": [300, 100],
      "params": {
        "preset": "fire"
      },
      "exposedParams": []
    },
    {
      "id": "bars_1",
      "type": "bars",
      "position": [500, 100],
      "params": {
        "gapPx": 2
      },
      "exposedParams": ["gapPx"]
    },
    {
      "id": "float_1",
      "type": "float",
      "position": [300, 200],
      "params": {
        "value": 5.0
      }
    }
  ],
  
  "connections": [
    {"from": "audio_1.spectrum", "to": "bars_1.spectrum"},
    {"from": "gradient_1.colorMap", "to": "bars_1.colors"},
    {"from": "float_1.out", "to": "bars_1.param_gapPx"}
  ]
}
```

---

## 15. Implementierungs-Roadmap

### 15.1 Phase 1: BasisVisuals (Aktuell)

| Task | Priorität | Status |
|------|-----------|--------|
| IModule Interface definieren | Hoch | 🔲 |
| SmoothingModule implementieren | Hoch | 🔲 |
| AudioSourceModule implementieren | Hoch | 🔲 |
| GradientModule implementieren | Hoch | 🔲 |
| BarsModule implementieren | Hoch | 🔲 |
| IVisual Interface definieren | Hoch | 🔲 |
| EqualizerVisual implementieren | Hoch | 🔲 |
| WaveformVisual implementieren | Mittel | 🔲 |
| VisualRegistry erstellen | Hoch | 🔲 |

### 15.2 Phase 2: ConfigPanel

| Task | Priorität | Status |
|------|-----------|--------|
| ConfigPanel UI-Grundstruktur | Hoch | 🔲 |
| Code-Folding für Module | Hoch | 🔲 |
| Widget-Factory (Slider, Checkbox, etc.) | Hoch | 🔲 |
| Parameter-Binding | Hoch | 🔲 |
| Live-Preview bei Änderung | Mittel | 🔲 |

### 15.3 Phase 3: Preset-System

| Task | Priorität | Status |
|------|-----------|--------|
| Preset-Serialisierung | Hoch | 🔲 |
| Preset-Browser UI | Mittel | 🔲 |
| Built-in Presets | Mittel | 🔲 |
| Import/Export | Niedrig | 🔲 |

### 15.4 Phase 4-6: Node-System & Scripting

| Task | Priorität | Status |
|------|-----------|--------|
| INode Interface | Mittel | 🔲 |
| NodeGraph Implementation | Mittel | 🔲 |
| Node-Editor UI | Mittel | 🔲 |
| Parameter→Input Konvertierung | Mittel | 🔲 |
| Native Value Nodes | Mittel | 🔲 |
| Expression Parser | Niedrig | 🔲 |
| Lua Integration | Niedrig | 🔲 |

---

## 16. Design-Entscheidungen

### 16.1 Module als Member statt Pointer

**Entscheidung:** Module sind direkte Member der Visual-Klasse.

**Begründung:**
- Kein Heap-Allokation Overhead
- Keine Null-Checks nötig
- Einfache Konstruktion

**Konsequenz:** Module können nicht zur Laufzeit ausgetauscht werden.

### 16.2 Parameter-Pfade statt IDs

**Entscheidung:** Hierarchische Pfade (`audio.smooth.timeMs`) statt flache IDs.

**Begründung:**
- Natürliche Gruppierung
- Keine ID-Kollisionen
- Einfache JSON-Serialisierung

### 16.3 BasisVisuals vor Nodes

**Entscheidung:** Erst monolithische Visuals, dann Node-Graph.

**Begründung:**
- Schnellere initiale Implementation
- Validierung der Modul-Konzepte
- Einfachere Debugging

---

## 17. Offene Punkte

### 17.1 Zu klären

| # | Thema | Phase | Status |
|---|-------|-------|--------|
| 1 | GPU-Shader-Module | 5+ | 🔲 |
| 2 | Undo/Redo für Parameter | 3 | 🔲 |
| 3 | Parameter-Animation (Keyframes) | 6 | 🔲 |
| 4 | Multi-Visual Layouts | 4 | 🔲 |
| 5 | Plugin-System für externe Module | 6+ | 🔲 |

### 17.2 Bekannte Limitierungen

- **Phase 1:** Keine Node-Verbindungen, fixe Pipeline
- **Phase 1-3:** Keine Scripting-Unterstützung
- **Alle Phasen:** Kein Hot-Reload von Modulen

---

## 18. Siehe auch

- [LumiPulse_Modules.md](LumiPulse_Modules.md) - Ursprüngliches Modul-Konzept
- [LumiPulse_NodeSystem.md](LumiPulse_NodeSystem.md) - Node-Graph Details
- [equalizer_modulubersicht.md](equalizer_modulubersicht.md) - Equalizer Parameter-Referenz

---

## 19. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.2.0** | **2025-12-31** | **Erweitert: Shader-Typen (VertexShader 🔷, FragmentShader 🔶, GeometryShader 🔻, ComputeShader ⬛, ShaderProgram 💠) im Typen-System, vollständige Shader Node Referenz (VertexShaderNode, FragmentShaderNode, GeometryShaderNode, ComputeShaderNode, ShaderProgramNode, ShaderRenderNode, GlslIncludeNode), 7 Shader-Preset-Nodes (Glow, Blur, Chromatic, Distort, Kaleidoscope, Feedback, ColorGrade), Shader-Pipeline Beispiel, /nodes/shader/ Ordnerstruktur mit presets/** |
| 1.1.0 | 2025-12-31 | Erweitert: Ordnerstruktur (/visualizers/modules/, /visualizers/nodes/, /visualizers/basics/), vollständiges Typen-System mit Port-Farben/Formen (◯◻△), komplette Native Value Node Referenz (20+ Nodes), erweiterte Text/Script Node Referenz (8 Script-Nodes), Namespace-Struktur |
| 1.0.0 | 2025-12-31 | Initial: Vollständige Architektur für Module, BasisVisuals, Node-Graph, Parameter-System, Scripting-Hooks, ConfigPanel-Integration |
