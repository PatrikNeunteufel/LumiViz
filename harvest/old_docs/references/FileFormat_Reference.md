# File Formats — Referenz

> **Version:** 1.0.0  
> **Datum:** 2026-01-02  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Preset-Formate](#3-preset-formate)
4. [Schnellreferenz](#4-schnellreferenz)
5. [Verwendung in Code](#5-verwendung-in-code)
6. [Siehe auch](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert alle **Datei-Formate** des Preset-Systems. Alle Presets werden als JSON gespeichert.

### Speicherorte

```
%APPDATA%/LumiViz/presets/           (Windows)
~/.local/share/LumiViz/presets/      (Linux)
~/Library/Application Support/LumiViz/presets/  (macOS)
├── visualizer/
│   └── {visualizerId}/
│       └── {name}.lvp
├── smoothing/
│   └── {name}.smooth
├── audio/
│   └── {name}.audio
└── gradients/
    └── {name}.grad
```

---

## 2. Konventionen

### 2.1 Dateinamen

| Erlaubt | Verboten |
|---------|----------|
| `MyPreset` | `My/Preset` (Slash) |
| `Preset_1` | `My\Preset` (Backslash) |
| `Cool-Preset` | `My.Preset` (Punkt) |
| `Preset 2` | Leerer Name |

### 2.2 JSON-Format

- UTF-8 Encoding
- 2-Space Indentation (optional)
- Keine Trailing Commas

### 2.3 Symbole

| Symbol | Bedeutung |
|--------|-----------|
| ✓ | Pflichtfeld |
| — | Optional (mit Default) |

---

## 3. Preset-Formate

### 3.1 Visualizer Preset (`.lvp`)

**Dateiendung:** `.lvp` (LumiPulse Visualizer Preset)  
**Speicherort:** `presets/visualizer/{visualizerId}/`

#### Schema

| Feld | Typ | Pflicht | Beschreibung |
|------|-----|---------|--------------|
| `visualizerId` | string | ✓ | ID des Visualizers |
| `version` | string | — | Preset-Format-Version |
| `name` | string | ✓ | Preset-Name |
| `params` | object | ✓ | Parameter-Werte |

#### Vollständiges Beispiel

```json
{
  "visualizerId": "pulsing",
  "version": "1.0",
  "name": "Energetic",
  "params": {
    "audio.scale": 1,
    "audio.bands": 64,
    "audio.floorDb": -50.0,
    "audio.ceilDb": 0.0,
    "audio.clamp01": true,
    "audio.gain": 1.5,
    "audio.smooth.algorithm": 2,
    "audio.smooth.timeMs": 30.0,
    "audio.smooth.windowSize": 8,
    "audio.smooth.primeFirstFrame": true,
    "shape.shape": 1,
    "shape.sides": 6,
    "shape.innerRadius": 0.6,
    "shape.color.mode": 1,
    "shape.color.angle": 45.0,
    "shape.color.outlineWidth": 2.0
  }
}
```

#### params-Objekt

Die Schlüssel im `params`-Objekt entsprechen den Parameter-IDs:

| Typ | JSON-Typ | Beispiel |
|-----|----------|----------|
| Int | number | `"audio.bands": 64` |
| Float | number | `"audio.gain": 1.5` |
| Bool | boolean | `"audio.clamp01": true` |
| Enum | number | `"audio.scale": 1` |
| String | string | `"name": "Test"` |
| Color4f | object | `{"r": 1.0, "g": 0.5, "b": 0.0, "a": 1.0}` |

---

### 3.2 Smoothing Preset (`.smooth`)

**Dateiendung:** `.smooth`  
**Speicherort:** `presets/smoothing/`

#### Schema

| Feld | Typ | Pflicht | Default | Beschreibung |
|------|-----|---------|---------|--------------|
| `name` | string | ✓ | — | Preset-Name |
| `algorithm` | number | ✓ | — | SmoothingAlgorithm (0-4) |
| `timeMs` | number | — | 50.0 | Zeitkonstante für EMA/DEMA |
| `windowSize` | number | — | 8 | Fenstergröße für SMA/WMA |
| `primeFirstFrame` | boolean | — | true | Ersten Frame initialisieren |

#### Beispiel

```json
{
  "name": "UltraSmooth",
  "algorithm": 2,
  "timeMs": 150.0,
  "windowSize": 15,
  "primeFirstFrame": true
}
```

#### Algorithm-Werte

| Wert | Name |
|------|------|
| 0 | None |
| 1 | SMA |
| 2 | EMA |
| 3 | WMA |
| 4 | DEMA |

---

### 3.3 Audio Preset (`.audio`)

**Dateiendung:** `.audio`  
**Speicherort:** `presets/audio/`

#### Schema

| Feld | Typ | Pflicht | Default | Beschreibung |
|------|-----|---------|---------|--------------|
| `name` | string | ✓ | — | Preset-Name |
| `scale` | number | — | 1 | FrequencyScale (0-2) |
| `bands` | number | — | 64 | Anzahl Bänder |
| `floorDb` | number | — | -60.0 | Noise Floor (dB) |
| `ceilDb` | number | — | 0.0 | Ceiling (dB) |
| `clamp01` | boolean | — | true | Clamp auf 0-1 |
| `gain` | number | — | 1.0 | Verstärkung |
| `smoothAlgorithm` | number | — | 2 | Smoothing-Algorithmus |
| `smoothTimeMs` | number | — | 50.0 | Smoothing-Zeit |
| `smoothWindowSize` | number | — | 8 | Smoothing-Fenster |

#### Beispiel

```json
{
  "name": "HighGain",
  "scale": 1,
  "bands": 64,
  "floorDb": -50.0,
  "ceilDb": 0.0,
  "clamp01": true,
  "gain": 2.0,
  "smoothAlgorithm": 2,
  "smoothTimeMs": 40.0,
  "smoothWindowSize": 8
}
```

#### Scale-Werte

| Wert | Name |
|------|------|
| 0 | Linear |
| 1 | Log |
| 2 | Mel |

---

### 3.4 Gradient Preset (`.grad`)

**Dateiendung:** `.grad`  
**Speicherort:** `presets/gradients/`

#### Schema

| Feld | Typ | Pflicht | Default | Beschreibung |
|------|-----|---------|---------|--------------|
| `name` | string | ✓ | — | Preset-Name |
| `mode` | number | — | 1 | GradientMode (0-2) |
| `angle` | number | — | 0.0 | Winkel (Grad) |
| `outlineWidth` | number | — | 2.0 | Outline-Breite |
| `stops` | array | ✓ | — | Color Stops |
| `midpoints` | array | — | [] | Midpoint-Positionen |

#### Stop-Objekt

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `pos` | number | Position (0.0-1.0) |
| `r` | number | Rot (0.0-1.0) |
| `g` | number | Grün (0.0-1.0) |
| `b` | number | Blau (0.0-1.0) |
| `a` | number | Alpha (0.0-1.0) |

#### Midpoint-Objekt

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `index` | number | Stop-Index (0-basiert) |
| `position` | number | Position (0.0-1.0, default 0.5) |

#### Beispiel

```json
{
  "name": "CustomSunset",
  "mode": 1,
  "angle": 90.0,
  "outlineWidth": 3.0,
  "stops": [
    {"pos": 0.0, "r": 0.5, "g": 0.0, "b": 0.5, "a": 1.0},
    {"pos": 0.5, "r": 1.0, "g": 0.5, "b": 0.0, "a": 1.0},
    {"pos": 1.0, "r": 1.0, "g": 1.0, "b": 0.0, "a": 1.0}
  ],
  "midpoints": [
    {"index": 0, "position": 0.3},
    {"index": 1, "position": 0.6}
  ]
}
```

#### Mode-Werte

| Wert | Name |
|------|------|
| 0 | Solid |
| 1 | LinearGradient |
| 2 | RadialGradient |

---

## 4. Schnellreferenz

### Datei-Formate

| Format | Endung | Pflichtfelder |
|--------|--------|---------------|
| Visualizer | `.lvp` | visualizerId, name, params |
| Smoothing | `.smooth` | name, algorithm |
| Audio | `.audio` | name |
| Gradient | `.grad` | name, stops |

### Speicherorte

| Format | Unterordner |
|--------|-------------|
| Visualizer | `visualizer/{vizId}/` |
| Smoothing | `smoothing/` |
| Audio | `audio/` |
| Gradient | `gradients/` |

### Enum-Werte

| Enum | Feld | Werte |
|------|------|-------|
| SmoothingAlgorithm | algorithm | 0-4 |
| FrequencyScale | scale | 0-2 |
| GradientMode | mode | 0-2 |

---

## 5. Verwendung in Code

### Preset speichern

```cpp
// Smoothing
SmoothingModule smooth;
smooth.setAlgorithm(SmoothingAlgorithm::EMA);
smooth.setTimeMs(100.0f);
smooth.savePreset("MySmooth");

// Gradient
ColorGradientModule gradient;
gradient.clearStops();
gradient.addStop(0.0f, {1, 0, 0, 1});
gradient.addStop(1.0f, {0, 0, 1, 1});
gradient.savePreset("RedToBlue");
```

### Preset laden

```cpp
smooth.loadPreset("MySmooth");
gradient.loadPreset("RedToBlue");
```

### Preset löschen

```cpp
bool deleted = smooth.deletePreset("MySmooth");
```

### Directory setzen

```cpp
QString appData = QStandardPaths::writableLocation(
    QStandardPaths::AppDataLocation);

SmoothingModule::setUserPresetsDirectory(
    (appData + "/presets/smoothing").toStdString());
```

---

## 6. Siehe auch

- [Preset_System.md](../modules/Preset_System.md) — API-Details
- [Parameter_Reference.md](Parameter_Reference.md) — Alle Parameter-IDs
- [Enum_Reference.md](Enum_Reference.md) — Enum-Werte

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: .lvp, .smooth, .audio, .grad dokumentiert** |
