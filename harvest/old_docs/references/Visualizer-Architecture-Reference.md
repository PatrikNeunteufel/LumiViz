# Visualizer Architecture Reference

**Version:** 1.0.0  
**Date:** January 2026  
**Status:** Active

---

## Table of Contents

1. [Overview](#1-overview)
2. [Module System](#2-module-system)
3. [Parameter System](#3-parameter-system)
4. [Dependency System (dependsOn)](#4-dependency-system-dependson)
5. [Color Gradient Module](#5-color-gradient-module)
6. [Preset System](#6-preset-system)
7. [Visualizer Implementation Checklist](#7-visualizer-implementation-checklist)
8. [Code Examples](#8-code-examples)

---

## 1. Overview

### 1.1 Architecture Principles

Every visualizer in LumiViz follows a modular architecture with these core principles:

- **Composition over Inheritance**: Visualizers compose reusable modules (AudioSourceModule, ColorGradientModule, etc.)
- **Parameter-Driven Configuration**: All settings are exposed as typed parameters via `paramDescs()`, `getParam()`, `setParam()`
- **Prefix Namespacing**: Module parameters are prefixed to avoid ID collisions (e.g., `audio.gain`, `shape.color.mode`)
- **Conditional Visibility**: Parameters use `dependsOn` for dynamic UI visibility
- **Preset Portability**: Presets store both references and raw data for cross-system compatibility

### 1.2 Core Interfaces

```cpp
class IVisualizer
{
public:
    virtual QString visualizerId() const = 0;
    virtual QString visualizerName() const = 0;
    
    virtual std::vector<ModuleParamDesc> paramDescs() const = 0;
    virtual bool getParam(const std::string& id, ParamValue& out) const = 0;
    virtual bool setParam(const std::string& id, const ParamValue& value) = 0;
    
    virtual void onInitialize() = 0;
    virtual void onRender(float deltaTime) = 0;
    virtual void onResize(int width, int height) = 0;
    virtual void reset() = 0;
};
```

---

## 2. Module System

### 2.1 Standard Modules

| Module | Purpose | Typical Prefix |
|--------|---------|----------------|
| `AudioSourceModule` | FFT/waveform audio input, normalization, smoothing | `audio.` |
| `ColorGradientModule` | Solid color, linear/radial gradients, presets | `color.` or `shape.color.` |
| `WaveformModule` | Waveform display settings, channel modes | `waveform.` |

### 2.2 Module Integration Pattern

```cpp
class MyVisualizer : public IVisualizer
{
private:
    AudioSourceModule m_audioSource;
    ColorGradientModule m_colorGradient;
    
public:
    std::vector<ModuleParamDesc> paramDescs() const override
    {
        std::vector<ModuleParamDesc> params;
        
        // Integrate AudioSourceModule with "audio." prefix
        for (const auto& p : m_audioSource.paramDescs())
        {
            ModuleParamDesc prefixed = p;
            prefixed.id = "audio." + p.id;
            prefixed.group = "1. Audio";
            
            if (!prefixed.dependsOn.empty())
            {
                prefixed.dependsOn = "audio." + prefixed.dependsOn;
            }
            
            params.push_back(prefixed);
        }
        
        // Add visualizer-specific parameters...
        
        return params;
    }
};
```

### 2.3 Parameter Routing

```cpp
bool MyVisualizer::getParam(const std::string& id, ParamValue& out) const
{
    // Route to AudioSourceModule
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }
    
    // Route to ColorGradientModule
    if (id.rfind("color.", 0) == 0)
    {
        return m_colorGradient.getParam(id.substr(6), out);
    }
    
    // Handle visualizer-specific parameters...
    return false;
}
```

---

## 3. Parameter System

### 3.1 ModuleParamDesc Structure

```cpp
struct ModuleParamDesc
{
    // === Identification ===
    std::string id;             // Unique ID (e.g., "mode", "solidColor")
    std::string displayName;    // UI label (e.g., "Color Mode")
    std::string group;          // Collapsible group (e.g., "2. Shape")
    std::string subGroup;       // Nested group (e.g., "Line Color Mono")
    std::string tooltip;        // Help text
    
    // === Type & Value ===
    ParamType type;             // Bool, Int, Float, Enum, String, Color
    ParamValue defaultValue;
    
    // === Constraints ===
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float step = 0.01f;
    std::vector<std::string> enumOptions;
    
    // === UI Hints ===
    int order = 0;              // Sort order within group
    bool advanced = false;      // Hide in advanced section
    bool hidden = false;        // Completely hidden (for serialization only)
    
    // === Dependencies ===
    std::string dependsOn;                   // Other param ID
    std::vector<ParamValue> dependsValues;   // Values that make this visible
};
```

### 3.2 Parameter Types

| Type | C++ Type | JSON Type | Example |
|------|----------|-----------|---------|
| `Bool` | `bool` | `boolean` | `true` |
| `Int` | `int` | `number` | `42` |
| `Float` | `float` | `number` | `0.5` |
| `Enum` | `int` | `number` | `2` (index) |
| `String` | `std::string` | `string` | `"hello"` |
| `Color` | `Color4f` | `array[4]` | `[1.0, 0.0, 1.0, 1.0]` |

### 3.3 ParamValue Variant

```cpp
using ParamValue = std::variant<
    bool,                    // index 0
    int,                     // index 1
    float,                   // index 2
    double,                  // index 3
    std::string,             // index 4
    std::vector<float>,      // index 5
    std::vector<int>,        // index 6
    Color4f                  // index 7
>;
```

### 3.4 Group Naming Convention

Groups are sorted alphabetically, so use numeric prefixes for ordering:

```cpp
p.group = "1. Audio";      // First section
p.group = "2. Shape";      // Second section
p.group = "3. Effects";    // Third section
```

---

## 4. Dependency System (dependsOn)

### 4.1 Basic Visibility Control

The `dependsOn` system controls parameter visibility based on other parameter values.

```cpp
// This parameter is only visible when shape.type == 2 (NGon) or 3 (Star)
ModuleParamDesc p;
p.id = "shape.sides";
p.dependsOn = "shape.type";
p.dependsValues = {2, 3};  // NGon=2, Star=3
```

### 4.2 Prefix Handling

When integrating modules, **always prefix the dependsOn reference**:

```cpp
for (const auto& p : m_colorGradient.paramDescs())
{
    ModuleParamDesc prefixed = p;
    prefixed.id = "shape.color." + p.id;
    
    // CRITICAL: Also prefix dependsOn!
    if (!prefixed.dependsOn.empty())
    {
        prefixed.dependsOn = "shape.color." + prefixed.dependsOn;
    }
    
    params.push_back(prefixed);
}
```

### 4.3 Two-Tier Dependency (Channel Mode Pattern)

For channel-specific parameters (Mono/Stereo/Both), use two levels:

**Level 1: SubGroup Visibility (channelMode → entire group)**
```cpp
// In WaveformModule - first param of each color group
ModuleParamDesc p;
p.id = "monoColor.mode";           // First param in "Line Color Mono"
p.subGroup = "Line Color Mono";
p.dependsOn = "channelMode";       // Controls entire SubGroup
p.dependsValues = {0, 2};          // Mono=0, Both=2
```

**Level 2: Param Visibility (mode → params within group)**
```cpp
// In ColorGradientModule - params depend on mode
ModuleParamDesc p;
p.id = "solidColor";
p.dependsOn = "mode";              // Within the color group
p.dependsValues = {0, 3};          // Solid=0, Outline=3
```

**After Prefixing by WaveformModule:**
```cpp
// monoColor.mode → dependsOn = "channelMode" → "waveform.channelMode"
// monoColor.solidColor → dependsOn = "mode" → "waveform.monoColor.mode"
```

### 4.4 ConfigPanel Handling

The ConfigPanel handles dependencies with these rules:

1. **SubGroups starting with "Line Color"**: Hidden entirely if channelMode doesn't match
2. **Other SubGroups**: Always visible, individual params hidden/shown
3. **dependsOn ending with "channelMode"**: Treated as channel-level dependency

---

## 5. Color Gradient Module

### 5.1 Gradient Modes

| Mode | Value | Description |
|------|-------|-------------|
| `Solid` | 0 | Single solid color |
| `Linear` | 1 | Linear gradient with angle |
| `Radial` | 2 | Radial gradient from center |
| `Outline` | 3 | Outline stroke with solid color |

### 5.2 Parameters Exposed

| Parameter | Type | Visible When | Description |
|-----------|------|--------------|-------------|
| `mode` | Enum | Always | Color mode selection |
| `solidColor` | Color | mode ∈ {0,3} | Solid/outline color |
| `angle` | Float | mode = 1 | Gradient angle (degrees) |
| `preset` | Enum | mode ∈ {1,2} | Gradient preset selection |
| `editGradient` | String | mode ∈ {1,2} | Button to open editor |
| `outlineWidth` | Float | mode = 3 | Outline stroke width |
| `gradientPresetName` | String | **hidden** | Preset name for serialization |
| `gradientData` | String | **hidden** | Fallback gradient data |

### 5.3 Gradient Data Format

Stops are serialized as semicolon-separated values:

```
pos,r,g,b,a;pos,r,g,b,a;...
```

Example:
```
0.0000,1.0000,0.0000,1.0000,1.0000;0.5000,0.0000,1.0000,1.0000,1.0000;1.0000,1.0000,0.0000,1.0000,1.0000
```

### 5.4 Preset Loading Logic

When loading a preset:

1. **gradientPresetName** is set first
   - If name exists in system presets → load it, ignore gradientData
   - If name is `[Custom]` or not found → store name, wait for gradientData

2. **gradientData** is set second
   - If preset was loaded successfully → ignore this data
   - If preset is `[Custom]` or not found → parse and apply stops

---

## 6. Preset System

### 6.1 Preset File Format (JSON)

```json
{
    "header": {
        "name": "My Preset",
        "visualizerId": "waveform",
        "description": "A cool waveform effect",
        "author": "User",
        "version": 1,
        "formatVersion": 1
    },
    "parameters": {
        "audio.gain": 1.5,
        "audio.smoothing.algorithm": 1,
        "waveform.channelMode": 0,
        "waveform.monoColor.mode": 1,
        "waveform.monoColor.gradientPresetName": "Neon",
        "waveform.monoColor.gradientData": "0.0,1.0,0.0,1.0,1.0;1.0,0.0,1.0,1.0,1.0",
        "waveform.monoColor.angle": 45.0
    }
}
```

### 6.2 Preset Storage Locations

```
%APPDATA%/LumiViz Project/LumiViz/presets/
├── pulsing/
│   ├── Default.json
│   └── Neon Pulse.json
├── waveform/
│   ├── Default.json
│   └── Stereo Rainbow.json
└── gradients/           # User gradient presets (.grad files)
    └── mycolors.grad
```

### 6.3 Parameter Capture

The PresetManager captures all parameters from `paramDescs()`:

```cpp
VisualizerPreset VisualizerPresetManager::capturePreset(IVisualizer* visualizer, ...)
{
    auto params = visualizer->paramDescs();
    for (const auto& desc : params)
    {
        ParamValue value;
        if (visualizer->getParam(desc.id, value))
        {
            preset.parameters[desc.id] = value;
        }
    }
    return preset;
}
```

**Important:** Hidden parameters (`hidden = true`) are still captured and saved!

### 6.4 Parameter Application Order

When applying a preset, parameters are applied in **alphabetical order** by ID. This matters for gradient loading:

- `gradientData` comes before `gradientPresetName` alphabetically
- Solution: The `gradientData` handler checks if `gradientPresetName` was already set

**Best Practice:** Use IDs that ensure correct ordering:
```cpp
p.id = "gradientPresetName";  // Applied first (g comes before g)
p.id = "gradientData";         // Applied second
```

Wait - that's the same! Let me check the actual order...

Actually, in practice: `gradientData` < `gradientPresetName` alphabetically.

**Correct Solution:** In `setParam("gradientData")`, check if current preset name is valid before applying raw data.

---

## 7. Visualizer Implementation Checklist

### 7.1 Required Methods

- [ ] `visualizerId()` - Unique string identifier (lowercase, no spaces)
- [ ] `visualizerName()` - Display name for UI
- [ ] `paramDescs()` - All parameters with proper prefixes and dependencies
- [ ] `getParam()` - Route to modules + handle own params
- [ ] `setParam()` - Route to modules + handle own params
- [ ] `onInitialize()` - OpenGL setup
- [ ] `onRender()` - Main render loop
- [ ] `onResize()` - Handle viewport changes
- [ ] `reset()` - Reset to defaults

### 7.2 Module Integration

For each module:

- [ ] Prefix all parameter IDs: `"prefix." + p.id`
- [ ] Prefix all dependsOn references: `"prefix." + p.dependsOn`
- [ ] Assign correct group: `p.group = "N. GroupName"`
- [ ] Route getParam/setParam with prefix stripping

### 7.3 ColorGradientModule Integration

- [ ] Add module as member: `ColorGradientModule m_colorGradient;`
- [ ] In `paramDescs()`: Prefix with `"color."` or `"shape.color."`
- [ ] In `getParam()`/`setParam()`: Route with prefix check
- [ ] Hidden params `gradientPresetName` and `gradientData` are inherited

### 7.4 Channel Mode Integration (if applicable)

- [ ] Define `channelMode` enum parameter
- [ ] Per-channel SubGroups: "Line Color Mono", "Line Color Left", "Line Color Right"
- [ ] First param in each SubGroup has `dependsOn = "channelMode"`
- [ ] Nested params have `dependsOn = "prefix.mode"`

---

## 8. Code Examples

### 8.1 Complete Visualizer Parameter Setup

```cpp
std::vector<ModuleParamDesc> MyVisualizer::paramDescs() const
{
    using namespace lumi::modules;
    std::vector<ModuleParamDesc> params;
    
    // =========================================================================
    // 1. Audio Parameters
    // =========================================================================
    
    for (const auto& p : m_audioSource.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "1. Audio";
        
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "audio." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }
    
    // =========================================================================
    // 2. Main Effect Parameters
    // =========================================================================
    
    {
        ModuleParamDesc p;
        p.id = "effect.intensity";
        p.displayName = "Intensity";
        p.type = ParamType::Float;
        p.minValue = 0.0f;
        p.maxValue = 2.0f;
        p.defaultValue = 1.0f;
        p.group = "2. Effect";
        p.order = 0;
        params.push_back(p);
    }
    
    // =========================================================================
    // 3. Color Parameters (via ColorGradientModule)
    // =========================================================================
    
    for (const auto& p : m_colorGradient.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "color." + p.id;
        prefixed.group = "3. Color";
        
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "color." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }
    
    return params;
}
```

### 8.2 getParam/setParam Routing

```cpp
bool MyVisualizer::getParam(const std::string& id, ParamValue& out) const
{
    // Audio module
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }
    
    // Color module
    if (id.rfind("color.", 0) == 0)
    {
        return m_colorGradient.getParam(id.substr(6), out);
    }
    
    // Effect parameters
    if (id == "effect.intensity")
    {
        out = m_intensity;
        return true;
    }
    
    return false;
}

bool MyVisualizer::setParam(const std::string& id, const ParamValue& value)
{
    // Audio module
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }
    
    // Color module
    if (id.rfind("color.", 0) == 0)
    {
        return m_colorGradient.setParam(id.substr(6), value);
    }
    
    // Effect parameters
    if (id == "effect.intensity")
    {
        if (auto* v = std::get_if<float>(&value))
        {
            m_intensity = std::clamp(*v, 0.0f, 2.0f);
            return true;
        }
    }
    
    return false;
}
```

### 8.3 Channel Mode with Color SubGroups

```cpp
// In WaveformModule::paramDescs()

// Channel mode selector
{
    ModuleParamDesc p;
    p.id = "channelMode";
    p.displayName = "Channel Mode";
    p.type = ParamType::Enum;
    p.enumOptions = {"Mono", "Stereo", "Both"};
    p.defaultValue = 0;
    p.subGroup = "Channel";
    p.order = 0;
    params.push_back(p);
}

// Mono color (SubGroup controlled by channelMode)
for (const auto& p : m_colorGradients[CHANNEL_MONO].paramDescs())
{
    ModuleParamDesc prefixed = p;
    prefixed.id = "monoColor." + p.id;
    prefixed.subGroup = "Line Color Mono";
    prefixed.order = 200 + p.order;
    
    if (p.dependsOn.empty())
    {
        // First-level params depend on channelMode
        prefixed.dependsOn = "channelMode";
        prefixed.dependsValues = {0, 2};  // Mono, Both
    }
    else
    {
        // Nested params keep their dependency but prefixed
        prefixed.dependsOn = "monoColor." + p.dependsOn;
        prefixed.dependsValues = p.dependsValues;
    }
    
    params.push_back(prefixed);
}

// Left color (for Stereo/Both modes)
for (const auto& p : m_colorGradients[CHANNEL_LEFT].paramDescs())
{
    ModuleParamDesc prefixed = p;
    prefixed.id = "leftColor." + p.id;
    prefixed.subGroup = "Line Color Left";
    prefixed.order = 300 + p.order;
    
    if (p.dependsOn.empty())
    {
        prefixed.dependsOn = "channelMode";
        prefixed.dependsValues = {1, 2};  // Stereo, Both
    }
    else
    {
        prefixed.dependsOn = "leftColor." + p.dependsOn;
        prefixed.dependsValues = p.dependsValues;
    }
    
    params.push_back(prefixed);
}

// Right color (same pattern as Left)
// ...
```

---

## Appendix A: Gradient Preset File Format (.grad)

User-created gradient presets are stored as `.grad` files:

```
# Gradient preset file
name=My Custom Gradient
mode=1
angle=45.0
stops=0.0,1.0,0.0,1.0,1.0;0.5,0.0,1.0,1.0,1.0;1.0,1.0,0.0,1.0,1.0
```

## Appendix B: Common Pitfalls

1. **Forgetting to prefix dependsOn** - Parameters won't show/hide correctly
2. **Wrong order values** - Parameters appear in wrong sequence
3. **Missing hidden flag** - Internal params show in UI
4. **Not handling both gradientPresetName and gradientData** - Presets lose custom gradients
5. **Alphabetical param application** - Can cause loading order issues

---

*End of Document*
