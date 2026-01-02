# Parameter — Referenz

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
3. [Parameter nach Modul](#3-parameter-nach-modul)
4. [Schnellreferenz](#4-schnellreferenz)
5. [Verwendung in Code](#5-verwendung-in-code)
6. [Siehe auch](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert alle **Parameter-IDs** des Visualizer Module Systems. Parameter werden über hierarchische Pfade adressiert.

### Parameter-Pfad-Format

```
[modul].[submodul].[parameter]

Beispiele:
  audio.gain              → AudioSourceModule.gain
  audio.smooth.timeMs     → AudioSourceModule → SmoothingModule → timeMs
  shape.color.angle       → PulseShapeModule → ColorGradientModule → angle
```

---

## 2. Konventionen

### 2.1 Typen

| Typ | C++ Typ | UI-Widget |
|-----|---------|-----------|
| `Int` | `int` | QSpinBox + QSlider |
| `Float` | `float` | QDoubleSpinBox + QSlider |
| `Bool` | `bool` | QCheckBox |
| `Enum` | `int` | QComboBox |
| `String` | `std::string` | QLineEdit |
| `Color4f` | `Color4f` | QPushButton + ColorDialog |

### 2.2 Symbole

| Symbol | Bedeutung |
|--------|-----------|
| ✓ | Pflichtparameter |
| — | Optional |
| 🔒 | Nur lesbar |
| 👁️ | Bedingte Sichtbarkeit |

---

## 3. Parameter nach Modul

### 3.1 AudioSourceModule (`audio.*`)

#### `audio.preset`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Enum` |
| **Pflicht** | — |
| **Default** | `1` (Default) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Audio-Preset Auswahl. Index 0 = [Custom], Index 1 = Default, Index 3+ = User-Presets.

**Gültige Werte:**
- `0` — [Custom] (manuell geändert)
- `1` — Default
- `3+` — User-Presets

---

#### `audio.scale`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Enum` |
| **Pflicht** | — |
| **Default** | `1` (Log) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Frequenz-Skalierung für FFT-Mapping.

**Gültige Werte:**
- `0` — Linear
- `1` — Log (logarithmisch)
- `2` — Mel (gehöroptimiert)

---

#### `audio.bands`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Int` |
| **Pflicht** | — |
| **Default** | `64` |
| **Bereich** | 8 - 512 |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Anzahl der Ausgabe-Frequenzbänder.

---

#### `audio.floorDb`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Float` |
| **Pflicht** | — |
| **Default** | `-60.0` |
| **Bereich** | -120.0 - 0.0 |
| **Einheit** | dB |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Noise Floor in Dezibel. Werte unterhalb werden auf 0 gemappt.

---

#### `audio.ceilDb`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Float` |
| **Pflicht** | — |
| **Default** | `0.0` |
| **Bereich** | -60.0 - +20.0 |
| **Einheit** | dB |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Ceiling in Dezibel. Werte oberhalb werden auf 1 gemappt.

---

#### `audio.clamp01`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Bool` |
| **Pflicht** | — |
| **Default** | `true` |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Ausgabewerte auf 0-1 begrenzen.

---

#### `audio.gain`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Float` |
| **Pflicht** | — |
| **Default** | `1.0` |
| **Bereich** | 0.1 - 5.0 |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Eingangsverstärkung. Multipliziert die normalisierten Werte.

---

### 3.2 SmoothingModule (`audio.smooth.*`)

#### `audio.smooth.preset`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Enum` |
| **Pflicht** | — |
| **Default** | `3` (Balanced) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Smoothing-Preset Auswahl.

**Gültige Werte:**
- `0` — [Custom]
- `1` — Instant (None, 0ms)
- `2` — Reactive (EMA, 20ms)
- `3` — Balanced (EMA, 50ms)
- `4` — Smooth (EMA, 100ms)
- `5` — Sluggish (DEMA, 200ms)
- `6+` — User-Presets

---

#### `audio.smooth.algorithm`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Enum` |
| **Pflicht** | — |
| **Default** | `2` (EMA) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Smoothing-Algorithmus.

**Gültige Werte:**
- `0` — None (Pass-through)
- `1` — SMA (Simple Moving Average)
- `2` — EMA (Exponential Moving Average)
- `3` — WMA (Weighted Moving Average)
- `4` — DEMA (Double Exponential MA)

---

#### `audio.smooth.timeMs` 👁️

| Aspekt | Wert |
|--------|------|
| **Typ** | `Float` |
| **Pflicht** | — |
| **Default** | `50.0` |
| **Bereich** | 1.0 - 500.0 |
| **Einheit** | ms |
| **Sichtbar** | Nur bei `algorithm` = 2 (EMA) oder 4 (DEMA) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Zeitkonstante τ für EMA/DEMA. Höhere Werte = mehr Glättung.

---

#### `audio.smooth.windowSize` 👁️

| Aspekt | Wert |
|--------|------|
| **Typ** | `Int` |
| **Pflicht** | — |
| **Default** | `8` |
| **Bereich** | 2 - 60 |
| **Einheit** | Samples |
| **Sichtbar** | Nur bei `algorithm` = 1 (SMA) oder 3 (WMA) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Fenstergröße für SMA/WMA. Anzahl der Samples im Durchschnitt.

---

#### `audio.smooth.primeFirstFrame` 👁️

| Aspekt | Wert |
|--------|------|
| **Typ** | `Bool` |
| **Pflicht** | — |
| **Default** | `true` |
| **Sichtbar** | Nur bei `algorithm` ≠ 0 (None) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Ersten Frame mit Eingangswert initialisieren statt mit 0.

---

### 3.3 ColorGradientModule (`shape.color.*`)

#### `shape.color.preset`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Enum` |
| **Pflicht** | — |
| **Default** | `1` (Fire) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Gradient-Preset Auswahl.

**Gültige Werte:**
- `0` — [Custom]
- `1` — Fire
- `2` — Ocean
- `3` — Neon
- `4` — Forest
- `5` — Sunset
- `6` — Rainbow
- `7` — Monochrome
- `8+` — User-Presets

---

#### `shape.color.mode`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Enum` |
| **Pflicht** | — |
| **Default** | `1` (LinearGradient) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Gradient-Modus.

**Gültige Werte:**
- `0` — Solid (einfarbig)
- `1` — LinearGradient
- `2` — RadialGradient

---

#### `shape.color.solidColor` 👁️

| Aspekt | Wert |
|--------|------|
| **Typ** | `Color4f` |
| **Pflicht** | — |
| **Default** | `{1.0, 1.0, 1.0, 1.0}` (Weiß) |
| **Sichtbar** | Nur bei `mode` = 0 (Solid) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Füllfarbe bei Solid-Modus (RGBA, 0-1).

---

#### `shape.color.angle`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Float` |
| **Pflicht** | — |
| **Default** | `0.0` |
| **Bereich** | 0.0 - 360.0 |
| **Einheit** | Grad |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Winkel für LinearGradient (0° = horizontal).

---

#### `shape.color.outlineWidth`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Float` |
| **Pflicht** | — |
| **Default** | `2.0` |
| **Bereich** | 1.0 - 15.0 |
| **Einheit** | Pixel |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Breite der Shape-Outline.

---

### 3.4 PulseShapeModule (`shape.*`)

#### `shape.shape`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Enum` |
| **Pflicht** | — |
| **Default** | `0` (Circle) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Form-Typ.

**Gültige Werte:**
- `0` — Circle
- `1` — Ring
- `2` — Ngon
- `3` — Star
- `4` — Wave
- `5` — Flash

---

#### `shape.sides` 👁️

| Aspekt | Wert |
|--------|------|
| **Typ** | `Int` |
| **Pflicht** | — |
| **Default** | `6` |
| **Bereich** | 3 - 12 |
| **Sichtbar** | Nur bei `shape` = 2 (Ngon) oder 3 (Star) |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Anzahl der Seiten für Polygon/Stern.

---

#### `shape.innerRadius`

| Aspekt | Wert |
|--------|------|
| **Typ** | `Float` |
| **Pflicht** | — |
| **Default** | `0.5` |
| **Bereich** | 0.0 - 1.0 |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Innenradius für Ring-Shapes (0 = gefüllt, 1 = nur Rand).

---

## 4. Schnellreferenz

### AudioSourceModule

| Parameter | Typ | Default | Bereich |
|-----------|-----|---------|---------|
| `audio.preset` | Enum | 1 | 0-N |
| `audio.scale` | Enum | 1 | 0-2 |
| `audio.bands` | Int | 64 | 8-512 |
| `audio.floorDb` | Float | -60 | -120 - 0 |
| `audio.ceilDb` | Float | 0 | -60 - +20 |
| `audio.clamp01` | Bool | true | — |
| `audio.gain` | Float | 1.0 | 0.1-5.0 |

### SmoothingModule

| Parameter | Typ | Default | Bereich | Visibility |
|-----------|-----|---------|---------|------------|
| `audio.smooth.preset` | Enum | 3 | 0-N | — |
| `audio.smooth.algorithm` | Enum | 2 | 0-4 | — |
| `audio.smooth.timeMs` | Float | 50 | 1-500 | EMA/DEMA |
| `audio.smooth.windowSize` | Int | 8 | 2-60 | SMA/WMA |
| `audio.smooth.primeFirstFrame` | Bool | true | — | ≠None |

### ColorGradientModule

| Parameter | Typ | Default | Bereich |
|-----------|-----|---------|---------|
| `shape.color.preset` | Enum | 1 | 0-N |
| `shape.color.mode` | Enum | 1 | 0-2 |
| `shape.color.solidColor` | Color4f | Weiß | RGBA |
| `shape.color.angle` | Float | 0 | 0-360 |
| `shape.color.outlineWidth` | Float | 2.0 | 1-15 |

### PulseShapeModule

| Parameter | Typ | Default | Bereich |
|-----------|-----|---------|---------|
| `shape.shape` | Enum | 0 | 0-5 |
| `shape.sides` | Int | 6 | 3-12 |
| `shape.innerRadius` | Float | 0.5 | 0-1 |

---

## 5. Verwendung in Code

### Parameter setzen

```cpp
// Einzelne Parameter
viz.setParam("audio.gain", 1.5f);
viz.setParam("audio.smooth.algorithm", 2);

// Enum per Name (Alternative)
viz.setParam("audio.scale", static_cast<int>(FrequencyScale::Log));
```

### Parameter abfragen

```cpp
ParamValue value;
if (viz.getParam("audio.smooth.timeMs", value))
{
    float timeMs = std::get<float>(value);
}
```

### Alle Parameter iterieren

```cpp
for (const auto& desc : viz.paramDescs())
{
    std::cout << desc.id << ": " << desc.displayName << std::endl;
}
```

---

## 6. Siehe auch

- [Enum_Reference.md](Enum_Reference.md) — Alle Enum-Werte
- [IModule.md](../modules/IModule.md) — Parameter-System API
- [ConfigPanel.md](../../include/UI/panels/ConfigPanel.md) — UI-Integration

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: Alle Parameter dokumentiert** |
