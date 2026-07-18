# Enums — Referenz

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
3. [Enums nach Modul](#3-enums-nach-modul)
4. [Schnellreferenz](#4-schnellreferenz)
5. [Verwendung in Code](#5-verwendung-in-code)
6. [Siehe auch](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert alle **Enum-Typen** des Visualizer Module Systems mit ihren Werten und Beschreibungen.

### Namespace

Alle Enums befinden sich im Namespace `lumi::modules`.

---

## 2. Konventionen

### 2.1 Enum-Werte

- Alle Enums beginnen bei `0`
- Werte sind konsistent nummeriert
- `enum class` wird verwendet (type-safe)

### 2.2 Symbole

| Symbol | Bedeutung |
|--------|-----------|
| ⭐ | Empfohlener/Default-Wert |
| ⚠️ | Mit Einschränkungen |
| 🔒 | Nur intern verwendet |

---

## 3. Enums nach Modul

### 3.1 IModule

#### ParamType

**Header:** `IModule.hpp`  
**Beschreibung:** Datentyp eines Parameters

| Wert | Name | Beschreibung |
|------|------|--------------|
| 0 | `Int` | Integer-Wert |
| 1 | `Float` | Fließkomma-Wert |
| 2 | `Bool` | Boolean-Wert |
| 3 | `Enum` | Auswahl aus Liste |
| 4 | `String` | Textwert |
| 5 | `Vec2` | 2D-Vektor |
| 6 | `Vec3` | 3D-Vektor |
| 7 | `Color4f` | RGBA-Farbe |
| 8 | `Button` | Aktions-Button |

**Beispiel:**

```cpp
ParamBuilder("intensity", ParamType::Float)
    .range(0.0f, 1.0f)
    .build();
```

---

### 3.2 SmoothingModule

#### SmoothingAlgorithm

**Header:** `SmoothingModule.hpp`  
**Beschreibung:** Verfügbare Glättungsalgorithmen

| Wert | Name | Parameter | Beschreibung |
|------|------|-----------|--------------|
| 0 | `None` | — | Kein Smoothing (Pass-through) |
| 1 | `SMA` | windowSize | Simple Moving Average |
| 2 ⭐ | `EMA` | timeMs | Exponential Moving Average |
| 3 | `WMA` | windowSize | Weighted Moving Average |
| 4 | `DEMA` | timeMs | Double Exponential MA |

**Beispiel:**

```cpp
smooth.setAlgorithm(SmoothingAlgorithm::EMA);
```

**Charakteristiken:**

| Algorithmus | Lag | Reaktivität | Glättung |
|-------------|-----|-------------|----------|
| None | Keiner | Maximal | Keine |
| SMA | Hoch | Niedrig | Sehr hoch |
| EMA | Mittel | Mittel | Hoch |
| WMA | Mittel | Mittel | Hoch |
| DEMA | Niedrig | Hoch | Mittel |

---

#### SmoothingPreset

**Header:** `SmoothingModule.hpp`  
**Beschreibung:** Builtin-Presets für Smoothing

| Wert | Name | Algorithm | timeMs | windowSize |
|------|------|-----------|--------|------------|
| 0 🔒 | `Custom` | — | — | — |
| 1 | `Instant` | None | 0 | 2 |
| 2 | `Reactive` | EMA | 20 | 3 |
| 3 ⭐ | `Balanced` | EMA | 50 | 8 |
| 4 | `Smooth` | EMA | 100 | 12 |
| 5 | `Sluggish` | DEMA | 200 | 20 |

**Beispiel:**

```cpp
smooth.applyPreset(SmoothingPreset::Balanced);
```

---

### 3.3 AudioSourceModule

#### FrequencyScale

**Header:** `AudioSourceModule.hpp`  
**Beschreibung:** Frequenz-Skalierung für FFT-Mapping

| Wert | Name | Beschreibung |
|------|------|--------------|
| 0 | `Linear` | Lineare Frequenzverteilung |
| 1 ⭐ | `Log` | Logarithmische Verteilung |
| 2 | `Mel` | Mel-Skala (gehöroptimiert) |

**Beispiel:**

```cpp
audio.setParam("scale", static_cast<int>(FrequencyScale::Log));
```

**Vergleich:**

```
Linear:  |----|----|----|----|  (gleichmäßig)
Log:     |--|---|-----|-------|  (mehr Bass-Auflösung)
Mel:     |--|--|---|------|----| (gehörangepasst)
```

---

#### AudioPreset

**Header:** `AudioSourceModule.hpp`  
**Beschreibung:** Builtin-Presets für Audio-Processing

| Wert | Name | Beschreibung |
|------|------|--------------|
| 0 🔒 | `Custom` | Manuell konfiguriert |
| 1 ⭐ | `Default` | Standard-Einstellungen |

**Hinweis:** User-Presets werden dynamisch hinzugefügt (Index 3+).

---

### 3.4 ColorGradientModule

#### GradientMode

**Header:** `ColorGradientModule.hpp`  
**Beschreibung:** Gradient-Rendering-Modus

| Wert | Name | Beschreibung |
|------|------|--------------|
| 0 | `Solid` | Einfarbig |
| 1 ⭐ | `LinearGradient` | Linearer Farbverlauf |
| 2 | `RadialGradient` | Radialer Farbverlauf |

**Beispiel:**

```cpp
gradient.setParam("mode", static_cast<int>(GradientMode::LinearGradient));
```

---

#### GradientPreset

**Header:** `ColorGradientModule.hpp`  
**Beschreibung:** Builtin-Gradient-Presets

| Wert | Name | Farben |
|------|------|--------|
| 0 🔒 | `Custom` | — |
| 1 ⭐ | `Fire` | Rot → Orange → Gelb |
| 2 | `Ocean` | Dunkelblau → Cyan → Weiß |
| 3 | `Neon` | Cyan → Magenta → Gelb |
| 4 | `Forest` | Dunkelgrün → Hellgrün |
| 5 | `Sunset` | Violett → Orange → Gelb |
| 6 | `Rainbow` | Rot → Orange → Gelb → Grün → Blau → Violett |
| 7 | `Monochrome` | Schwarz → Weiß |

**Beispiel:**

```cpp
gradient.loadPreset("Ocean");
// oder
gradient.setParam("preset", 2);
```

---

### 3.5 PulseShapeModule

#### PulseShape

**Header:** `PulseShapeModule.hpp`  
**Beschreibung:** Verfügbare Form-Typen

| Wert | Name | Beschreibung |
|------|------|--------------|
| 0 ⭐ | `Circle` | Gefüllter Kreis |
| 1 | `Ring` | Ring mit Loch |
| 2 | `Ngon` | N-seitiges Polygon |
| 3 | `Star` | Stern |
| 4 | `Wave` | Wellenform |
| 5 | `Flash` | Vollbild-Flash |

**Beispiel:**

```cpp
shape.setParam("shape", static_cast<int>(PulseShape::Ring));
```

**Zusätzliche Parameter:**

| Shape | Verwendet `sides` | Verwendet `innerRadius` |
|-------|-------------------|-------------------------|
| Circle | ❌ | ❌ |
| Ring | ❌ | ✅ |
| Ngon | ✅ | ❌ |
| Star | ✅ | ✅ |
| Wave | ❌ | ❌ |
| Flash | ❌ | ❌ |

---

## 4. Schnellreferenz

### Alle Enums

| Enum | Modul | Werte | Default |
|------|-------|-------|---------|
| `ParamType` | IModule | 0-8 | — |
| `SmoothingAlgorithm` | SmoothingModule | 0-4 | 2 (EMA) |
| `SmoothingPreset` | SmoothingModule | 0-5 | 3 (Balanced) |
| `FrequencyScale` | AudioSourceModule | 0-2 | 1 (Log) |
| `AudioPreset` | AudioSourceModule | 0-1 | 1 (Default) |
| `GradientMode` | ColorGradientModule | 0-2 | 1 (Linear) |
| `GradientPreset` | ColorGradientModule | 0-7 | 1 (Fire) |
| `PulseShape` | PulseShapeModule | 0-5 | 0 (Circle) |

### SmoothingAlgorithm → Parameter

| Algorithm | Verwendet timeMs | Verwendet windowSize |
|-----------|------------------|----------------------|
| None | ❌ | ❌ |
| SMA | ❌ | ✅ |
| EMA | ✅ | ❌ |
| WMA | ❌ | ✅ |
| DEMA | ✅ | ❌ |

---

## 5. Verwendung in Code

### Enum zu int konvertieren

```cpp
int value = static_cast<int>(SmoothingAlgorithm::EMA);  // 2
```

### int zu Enum konvertieren

```cpp
auto algo = static_cast<SmoothingAlgorithm>(2);  // EMA
```

### Mit setParam verwenden

```cpp
// Direkt als int
viz.setParam("audio.smooth.algorithm", 2);

// Type-safe mit Enum
viz.setParam("audio.smooth.algorithm", 
             static_cast<int>(SmoothingAlgorithm::EMA));
```

### Enum-Options in paramDescs

```cpp
ParamBuilder("algorithm", ParamType::Enum)
    .enumOptions({"None", "SMA", "EMA", "WMA", "DEMA"})
    .defaultValue(2)
    .build();
```

---

## 6. Siehe auch

- [Parameter_Reference.md](Parameter_Reference.md) — Alle Parameter
- [IModule.md](../modules/IModule.md) — ParamType Details
- [SmoothingModule.md](../modules/SmoothingModule.md) — Algorithmus-Details

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: Alle Enums dokumentiert** |
