# Module System — Cheatsheet

> **Version:** 1.0.0  
> **Letzte Aktualisierung:** 2026-01-02  
> **Für:** MyViz v1.0  
> **Sprache:** Deutsch  

---

## Schnellstart

```cpp
// Parameter setzen
viz.setParam("audio.gain", 1.5f);
viz.setParam("audio.smooth.algorithm", 2);  // EMA

// Preset laden
smooth.loadPreset("Balanced");
gradient.loadPreset("Fire");
```

---

## Parameter-Pfade

| Modul | Prefix | Beispiel |
|-------|--------|----------|
| AudioSource | `audio.` | `audio.gain`, `audio.bands` |
| Smoothing | `audio.smooth.` | `audio.smooth.timeMs` |
| ColorGradient | `shape.color.` | `shape.color.angle` |
| PulseShape | `shape.` | `shape.shape`, `shape.sides` |

---

## IModule Interface

| Methode | Rückgabe | Beschreibung |
|---------|----------|--------------|
| `paramDescs()` | `vector<Desc>` | Alle Parameter |
| `getParam(id, out)` | `bool` | Parameter lesen |
| `setParam(id, val)` | `bool` | Parameter setzen |
| `resetToDefaults()` | `void` | Auf Default |

---

## AudioSourceModule

### Methoden

| Methode | Beschreibung |
|---------|--------------|
| `update(fft, size, dt)` | FFT verarbeiten |
| `spectrum()` | Verarbeitetes Spektrum |
| `frequencyBands()` | 6-Band Analyse |
| `smoothing()` | Embedded SmoothingModule |

### Parameter

| ID | Default | Bereich |
|----|---------|---------|
| `audio.scale` | 1 (Log) | 0-2 |
| `audio.bands` | 64 | 8-512 |
| `audio.floorDb` | -60 | -120 - 0 |
| `audio.gain` | 1.0 | 0.1-5.0 |

---

## SmoothingModule

### Algorithmen

| Wert | Name | Parameter |
|------|------|-----------|
| 0 | None | — |
| 1 | SMA | windowSize |
| 2 | **EMA** | timeMs |
| 3 | WMA | windowSize |
| 4 | DEMA | timeMs |

### Presets

| Name | Algorithm | timeMs |
|------|-----------|--------|
| Instant | None | 0 |
| Reactive | EMA | 20 |
| **Balanced** | EMA | 50 |
| Smooth | EMA | 100 |
| Sluggish | DEMA | 200 |

### Parameter

| ID | Default | Sichtbar |
|----|---------|----------|
| `smooth.algorithm` | 2 | Immer |
| `smooth.timeMs` | 50 | EMA/DEMA |
| `smooth.windowSize` | 8 | SMA/WMA |

---

## ColorGradientModule

### Modi

| Wert | Name |
|------|------|
| 0 | Solid |
| 1 | LinearGradient |
| 2 | RadialGradient |

### Methoden

| Methode | Beschreibung |
|---------|--------------|
| `sample(t)` | Farbe bei 0-1 |
| `addStop(pos, color)` | Stop hinzufügen |
| `loadPreset(name)` | Preset laden |
| `savePreset(name)` | Preset speichern |

### Builtin Presets

Fire · Ocean · Neon · Forest · Sunset · Rainbow · Monochrome

---

## ParamBuilder

```cpp
ParamBuilder("timeMs", ParamType::Float)
    .displayName("Time Constant")
    .range(1.0f, 500.0f)
    .defaultValue(50.0f)
    .tooltip("Smoothing time in ms")
    .subGroup("Smoothing")
    .dependsOn("algorithm", {2, 4})
    .build();
```

| Methode | Beschreibung |
|---------|--------------|
| `range(min, max)` | Min/Max |
| `defaultValue(val)` | Default |
| `enumOptions({...})` | Dropdown-Optionen |
| `subGroup(name)` | UI-Gruppe |
| `dependsOn(id, {vals})` | Visibility |

---

## Preset-System

### Presets speichern/laden

```cpp
// Smoothing
smooth.savePreset("MySmooth");
smooth.loadPreset("MySmooth");
smooth.deletePreset("MySmooth");

// Gradient
gradient.savePreset("MyGrad");
gradient.loadPreset("MyGrad");
```

### Verzeichnisse setzen

```cpp
SmoothingModule::setUserPresetsDirectory(path);
ColorGradientModule::setUserPresetsDirectory(path);
AudioSourceModule::setUserPresetsDirectory(path);
```

### Datei-Formate

| Modul | Endung |
|-------|--------|
| Visualizer | `.lvp` |
| Smoothing | `.smooth` |
| Audio | `.audio` |
| Gradient | `.grad` |

---

## Dropdown-Struktur

```
[Custom]    ← Index 0 (manuell geändert)
Default     ← Index 1
---         ← Separator (disabled)
UserPreset  ← Index 3+
```

---

## Tipps

- 💡 `timeMs` → EMA/DEMA, `windowSize` → SMA/WMA
- 💡 `dependsOn` für bedingte Sichtbarkeit
- 📌 Preset-Änderung → automatisch `[Custom]`
- 📌 `resetToDefaults()` für kompletten Reset

---

## Siehe auch

- [Parameter_Reference.md](Parameter_Reference.md) — Alle Parameter
- [Enum_Reference.md](Enum_Reference.md) — Alle Enums
- [FileFormat_Reference.md](FileFormat_Reference.md) — JSON-Formate
