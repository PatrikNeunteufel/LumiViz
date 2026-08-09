# Preset System Reference

**Version:** 1.1.0  
**Date:** January 2026

---

## 1. Preset File Structure

### 1.1 JSON Schema

```json
{
    "header": {
        "name": "string (required)",
        "visualizerId": "string (required)",
        "description": "string (optional)",
        "author": "string (optional)",
        "version": "integer (default: 1)",
        "formatVersion": "integer (current: 1)"
    },
    "parameters": {
        "param.id": "value",
        ...
    }
}
```

### 1.2 Example Preset

```json
{
    "header": {
        "name": "Neon Pulse",
        "visualizerId": "pulsing",
        "description": "Vibrant neon pulsing effect",
        "author": "LumiViz Team",
        "version": 1,
        "formatVersion": 1
    },
    "parameters": {
        "audio.gain": 1.2,
        "audio.smoothing.algorithm": 1,
        "audio.smoothing.timeConstant": 50.0,
        "shape.type": 0,
        "shape.minSize": 0.2,
        "shape.maxSize": 0.9,
        "shape.color.mode": 1,
        "shape.color.gradientPresetName": "Neon",
        "shape.color.gradientData": "0.0000,1.0000,0.0000,1.0000,1.0000;0.5000,0.0000,1.0000,1.0000,1.0000;1.0000,1.0000,0.0000,1.0000,1.0000",
        "shape.color.angle": 0.0
    }
}
```

---

## 2. Parameter Value Types

| ParamType | JSON Type | Example Value |
|-----------|-----------|---------------|
| Bool | boolean | `true`, `false` |
| Int | number | `42` |
| Float | number | `0.75` |
| Enum | number | `2` (index into enumOptions) |
| String | string | `"Neon"` |
| Color | array[4] | `[1.0, 0.0, 1.0, 1.0]` (RGBA) |

### 2.1 Type Conversion

**Wichtig:** JSON unterscheidet nicht zwischen Int und Float - alle Zahlen sind `number`.

Beim Laden von Presets werden alle numerischen Werte als `float` interpretiert. 
Die `setParam()`-Implementierungen müssen daher **beide Typen** akzeptieren:

```cpp
// Korrekte Implementierung für Int-Parameter:
if (id == "sampleCount") { 
    if (auto* v = std::get_if<int>(&value)) { setSampleCount(*v); return true; }
    if (auto* v = std::get_if<float>(&value)) { setSampleCount(static_cast<int>(*v)); return true; }
}

// Korrekte Implementierung für Float-Parameter:
if (id == "timePerDiv") { 
    if (auto* v = std::get_if<float>(&value)) { setTimePerDiv(*v); return true; }
    if (auto* v = std::get_if<int>(&value)) { setTimePerDiv(static_cast<float>(*v)); return true; }
}

// Korrekte Implementierung für Enum-Parameter:
if (id == "triggerMode") { 
    if (auto* v = std::get_if<int>(&value)) { m_triggerMode = static_cast<TriggerMode>(*v); return true; }
    if (auto* v = std::get_if<float>(&value)) { m_triggerMode = static_cast<TriggerMode>(static_cast<int>(*v)); return true; }
}
```

**Grund:** Wenn ein Wert wie `74.0` in JSON gespeichert wird, kann er beim Laden als 
`float` (74.0f) interpretiert werden. Ohne den Fallback würde `std::get_if<int>()` 
`nullptr` zurückgeben und der Parameter würde ignoriert.

---

## 3. Gradient Serialization

### 3.1 Two-Parameter System

Gradients are stored with redundancy for cross-system compatibility:

| Parameter | Purpose |
|-----------|---------|
| `gradientPresetName` | Name of the gradient preset (e.g., "Neon", "[Custom]") |
| `gradientData` | Raw stop data as fallback |

### 3.2 gradientPresetName Values

- **Built-in preset**: `"Rainbow"`, `"Fire"`, `"Ocean"`, `"Neon"`, etc.
- **User preset**: Name of user-created `.grad` file
- **Custom**: `"[Custom]"` indicates manually edited gradient

### 3.3 gradientData Format

```
position,red,green,blue,alpha;position,red,green,blue,alpha;...
```

- Values are comma-separated within each stop
- Stops are semicolon-separated
- All values are floats (0.0 - 1.0)
- Minimum 2 stops required

**Example:**
```
0.0000,1.0000,0.0000,1.0000,1.0000;0.5000,0.0000,1.0000,1.0000,1.0000;1.0000,1.0000,0.0000,1.0000,1.0000
```

Decoded:
| Position | Color |
|----------|-------|
| 0.0 | Magenta (1,0,1,1) |
| 0.5 | Cyan (0,1,1,1) |
| 1.0 | Yellow (1,1,0,1) |

### 3.4 Loading Priority

When a preset is loaded:

1. `gradientPresetName` is applied
   - If preset exists on system → Load it, gradientData ignored
   - If `[Custom]` or not found → Store name, wait for data
   
2. `gradientData` is applied
   - If preset was loaded → Skip (data already set)
   - If preset missing/custom → Parse and apply stops

---

## 4. File Locations

### 4.1 Windows

```
%APPDATA%\LumiViz Project\LumiViz\presets\
├── pulsing\
│   ├── Default.json
│   └── Neon Pulse.json
├── waveform\
│   ├── Default.json
│   └── Stereo.json
└── gradients\
    └── custom.grad
```

### 4.2 Linux

```
~/.config/LumiViz Project/LumiViz/presets/
├── pulsing/
├── waveform/
└── gradients/
```

### 4.3 macOS

```
~/Library/Application Support/LumiViz Project/LumiViz/presets/
├── pulsing/
├── waveform/
└── gradients/
```

---

## 5. Preset API

### 5.1 Saving a Preset

```cpp
VisualizerPresetManager manager;

// Capture current state
VisualizerPreset preset = manager.capturePreset(
    visualizer,           // IVisualizer*
    "My Preset",          // name
    "Cool effect"         // description (optional)
);

// Save to file
manager.savePreset(preset);
```

### 5.2 Loading a Preset

```cpp
// Load from file
auto preset = manager.loadPreset("waveform", "My Preset");

if (preset)
{
    // Apply to visualizer
    manager.applyPreset(visualizer, *preset);
}
```

### 5.3 Listing Available Presets

```cpp
QStringList presets = manager.listPresets("pulsing");
// Returns: ["Default", "Neon Pulse", ...]
```

---

## 6. Hidden Parameters

Hidden parameters (`hidden = true`) are:

- **NOT shown** in the ConfigPanel UI
- **ARE captured** when saving presets
- **ARE applied** when loading presets

### 6.1 Standard Hidden Parameters

| Module | Parameter | Purpose |
|--------|-----------|---------|
| ColorGradientModule | `gradientPresetName` | Gradient preset reference |
| ColorGradientModule | `gradientData` | Raw gradient stop data |

### 6.2 Declaring Hidden Parameters

```cpp
ModuleParamDesc p;
p.id = "internalData";
p.type = ParamType::String;
p.hidden = true;  // Not in UI, but in presets
p.order = 999;    // At end of param list
```

---

## 7. Preset Compatibility

### 7.1 Cross-Version Compatibility

- `formatVersion` tracks preset format changes
- Older presets with missing params use defaults
- Unknown params are ignored (forward compatibility)

### 7.2 Cross-System Compatibility

- Gradient presets may not exist on all systems
- `gradientData` provides fallback
- User-created gradient presets referenced by name only

### 7.3 Visualizer ID Validation

```cpp
bool VisualizerPresetManager::applyPreset(IVisualizer* visualizer, 
                                          const VisualizerPreset& preset)
{
    // Strict validation - IDs must match
    if (preset.visualizerId != visualizer->visualizerId())
    {
        return false;  // Reject mismatched preset
    }
    // ...
}
```

---

## 8. Best Practices

### 8.1 Preset Naming

- Use descriptive names: "Neon Pulse", "Calm Ocean"
- Avoid special characters: stick to alphanumeric, spaces, hyphens
- Keep names concise (under 30 characters)

### 8.2 Default Preset

- Every visualizer should have a "Default" preset
- Default preset captures initial state after `reset()`
- Shipped with application, not user-modifiable

### 8.3 Parameter IDs

- Use consistent prefixes: `audio.`, `shape.`, `color.`
- Keep IDs stable across versions (breaking change if renamed)
- Document any ID changes in release notes

### 8.4 Gradient Data

- Always save both `gradientPresetName` and `gradientData`
- `gradientData` should match current stops even for presets
- This ensures sharing presets works across systems

---

## 9. Troubleshooting

### 9.1 Preset Won't Load

- Check `visualizerId` matches current visualizer
- Verify JSON syntax is valid
- Check file permissions

### 9.2 Gradient Not Restored

- Ensure both `gradientPresetName` AND `gradientData` are saved
- Check `gradientData` format (semicolon separators, 5 values per stop)
- Verify at least 2 stops in `gradientData`

### 9.3 Parameters Missing After Load

- Check parameter ID hasn't changed
- Verify parameter is in `paramDescs()` output
- Check `hidden` flag is set correctly for internal params

### 9.4 Numeric Parameter Not Applied

**Symptom:** Parameter wie `timePerDiv`, `gain`, oder Enum-Werte werden beim 
Preset-Laden ignoriert, obwohl sie in der JSON-Datei vorhanden sind.

**Ursache:** `setParam()` prüft nur auf einen Typ (`float` oder `int`), aber 
JSON-Werte wie `74.0` werden möglicherweise als anderer Typ geladen.

**Diagnose:** Im Log erscheint:
```
PresetManager: Failed to set param 'scope.timePerDiv'
```

**Lösung:** `setParam()` muss beide numerischen Typen akzeptieren:

```cpp
// FALSCH - funktioniert nicht für alle JSON-Werte:
if (auto* v = std::get_if<float>(&value)) { setTimePerDiv(*v); return true; }

// RICHTIG - akzeptiert beide Typen:
if (auto* v = std::get_if<float>(&value)) { setTimePerDiv(*v); return true; }
if (auto* v = std::get_if<int>(&value)) { setTimePerDiv(static_cast<float>(*v)); return true; }
```

**Betroffene Module:** OscilloscopeModule, ColorGradientModule, WaveformModule, 
und alle anderen Module mit Float/Int/Enum-Parametern.

---

*End of Document*
