# LumiPulse Module System — Kurzreferenz

> **Version:** 4.0.0  
> **Datum:** 2026-01-02  
> **Typ:** Reference  
> **Status:** Aktuell  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Ordnerstruktur](#2-ordnerstruktur)
3. [Parameter-Hierarchie](#3-parameter-hierarchie)
4. [Preset-System](#4-preset-system)
5. [Smoothing-Algorithmen](#5-smoothing-algorithmen)
6. [Verwendung](#6-verwendung)
7. [Siehe auch](#7-siehe-auch)

---

## 1. Übersicht

Das Visualizer Module System bietet modulare, wiederverwendbare Komponenten mit einheitlichem Parameter-Interface und dateibasiertem Preset-System.

---

## 2. Ordnerstruktur

```
include/visualizers/
├── modules/
│   ├── IModule.hpp                 ← Interface + ParamBuilder
│   ├── source/
│   │   └── AudioSourceModule.hpp   ← FFT + Embedded Smoothing
│   ├── processing/
│   │   └── SmoothingModule.hpp     ← SMA/EMA/WMA/DEMA
│   ├── ColorGradientModule.hpp     ← Farb-Gradients
│   └── PulseShapeModule.hpp        ← Form-Geometrie
├── PulsingVisualizer.hpp
├── IVisualizer.hpp
└── VisualizerBase.hpp
```

---

## 3. Parameter-Hierarchie

```
audio.*                         ← AudioSourceModule
├── audio.preset               ← [Custom], Default, User-Presets
├── audio.scale                ← Linear/Log/Mel
├── audio.bands                ← 8-512
├── audio.floorDb              ← -120 bis 0 dB
├── audio.ceilDb               ← -60 bis +20 dB
├── audio.clamp01              ← Bool
├── audio.gain                 ← 0.1-5.0
└── audio.smooth.*             ← Embedded SmoothingModule
    ├── audio.smooth.preset    ← [Custom], Builtin, User
    ├── audio.smooth.algorithm ← None/SMA/EMA/WMA/DEMA
    ├── audio.smooth.timeMs    ← 1-500 ms (EMA/DEMA)
    └── audio.smooth.windowSize← 2-60 (SMA/WMA)

shape.*                         ← PulseShapeModule
├── shape.shape                ← Circle/Ring/Ngon/Star/Wave/Flash
├── shape.sides                ← 3-12
├── shape.innerRadius          ← 0-1
└── shape.color.*              ← ColorGradientModule
    ├── shape.color.preset     ← [Custom], Fire, Ocean, User
    ├── shape.color.mode       ← Solid/Linear/Radial
    ├── shape.color.angle      ← 0-360°
    └── shape.color.outlineWidth ← 1-15
```

---

## 4. Preset-System

### Dropdown-Struktur

```
[Custom]     ← Index 0: Manuell geändert
Default      ← Index 1: Hardcoded Defaults
---          ← Separator (disabled)
UserPreset1  ← Index 3+: User-Presets
```

### Datei-Formate

| Modul | Endung | Speicherort |
|-------|--------|-------------|
| Visualizer | `.lvp` | `presets/visualizer/{vizId}/` |
| Smoothing | `.smooth` | `presets/smoothing/` |
| Audio | `.audio` | `presets/audio/` |
| Gradient | `.grad` | `presets/gradients/` |

---

## 5. Smoothing-Algorithmen

| Algorithmus | Parameter | Beschreibung |
|-------------|-----------|--------------|
| None | — | Pass-through |
| SMA | windowSize | Simple Moving Average |
| **EMA** | timeMs | Exponential Moving Average (empfohlen) |
| WMA | windowSize | Weighted Moving Average |
| DEMA | timeMs | Double EMA (reduzierter Lag) |

**Visibility:** `timeMs` erscheint nur bei EMA/DEMA, `windowSize` nur bei SMA/WMA.

---

## 6. Verwendung

```cpp
PulsingVisualizer viz;

// Parameter setzen
viz.setParam("audio.gain", 1.5f);
viz.setParam("audio.smooth.algorithm", 2);  // EMA
viz.setParam("shape.color.preset", 3);      // Ocean

// Modul-Zugriff
viz.audioSource()->setGain(1.5f);
viz.audioSource()->smoothing().setTimeMs(100.0f);

// Preset speichern
viz.audioSource()->smoothing().savePreset("MySmooth");

// Auf Defaults zurücksetzen
viz.resetToDefaults();
```

---

## 7. Siehe auch

- [IModule.md](modules/IModule.md) — Interface-Dokumentation
- [SmoothingModule.md](modules/SmoothingModule.md) — Smoothing-Algorithmen
- [AudioSourceModule.md](modules/AudioSourceModule.md) — FFT-Verarbeitung
- [ColorGradientModule.md](modules/ColorGradientModule.md) — Gradient-System
- [Preset_System.md](modules/Preset_System.md) — File-basierte Presets
