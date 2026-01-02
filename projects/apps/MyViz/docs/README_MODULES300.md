# LumiPulse Module Integration v3.0

## Neue Ordnerstruktur

```
include/visualizers/
├── modules/
│   ├── IModule.hpp                 ← NEU: Interface für alle Module
│   │
│   ├── source/                     ← NEU: Ordner
│   │   └── AudioSourceModule.hpp   ← NEU: FFT + Smoothing
│   │
│   ├── processing/                 ← NEU: Ordner
│   │   └── SmoothingModule.hpp     ← NEU: SMA/EMA/WMA/DEMA
│   │
│   ├── ColorSchemeModule.hpp       ← Bestehend
│   └── PulseShapeModule.hpp        ← Bestehend
│
├── PulsingVisualizer.hpp           ← Aktualisiert (v3)
└── ...

src/visualizers/
├── PulsingVisualizer.cpp           ← Aktualisiert (v3)
└── ...
```

## Installation

### 1. Neue Module-Dateien kopieren:

```
include/visualizers/modules/IModule.hpp
include/visualizers/modules/source/AudioSourceModule.hpp
include/visualizers/modules/processing/SmoothingModule.hpp
```

### 2. PulsingVisualizer ersetzen:

Ersetze die bestehenden Dateien mit den v3 Versionen:
- `PulsingVisualizer_v3.hpp` → `PulsingVisualizer.hpp`
- `PulsingVisualizer_v3.cpp` → `PulsingVisualizer.cpp`

### 3. Source.cmake aktualisieren

In `include/visualizers/modules/Source.cmake`:

```cmake
# Add subdirectories for module categories
add_subdirectory(source)
add_subdirectory(processing)
```

Neue Datei `include/visualizers/modules/source/Source.cmake`:
```cmake
set(MODULE_SOURCE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/AudioSourceModule.hpp
)
list(APPEND MYLIB_HEADERS ${MODULE_SOURCE_HEADERS})
```

Neue Datei `include/visualizers/modules/processing/Source.cmake`:
```cmake
set(MODULE_PROCESSING_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/SmoothingModule.hpp
)
list(APPEND MYLIB_HEADERS ${MODULE_PROCESSING_HEADERS})
```

## Neue Features

### Parameter-System

```cpp
PulsingVisualizer viz;

// Alle Parameter abfragen (für ConfigPanel)
auto params = viz.paramDescs();

// Parameter per Pfad setzen
viz.setParam("audio.smooth.timeMs", 100.0f);
viz.setParam("audio.gain", 1.5f);
viz.setParam("color.scheme", 1);  // Neon

// Parameter abfragen
lumi::modules::ParamValue val;
viz.getParam("audio.smooth.algorithm", val);
```

### Parameter-Hierarchie

```
audio.*                    ← AudioSourceModule
├── audio.scale           ← FrequencyScale (Linear/Log/Mel)
├── audio.bands           ← Anzahl Bänder (8-512)
├── audio.floorDb         ← Floor in dB (-120 bis 0)
├── audio.ceilDb          ← Ceiling in dB (-60 bis +20)
├── audio.gain            ← Eingangsverstärkung (0.1-5.0)
└── audio.smooth.*        ← Eingebettetes SmoothingModule
    ├── audio.smooth.algorithm  ← None/SMA/EMA/WMA/DEMA
    ├── audio.smooth.timeMs     ← Glättungszeit (0-500ms)
    └── audio.smooth.preset     ← Instant/Reactive/Balanced/Smooth/Sluggish

shape.*                   ← PulseShapeModule  
├── shape.type           ← Circle/Ring/Flash/N-gon/Star/Wave
├── shape.baseSize       ← Basisgröße (0.1-1.5)
├── shape.sides          ← Anzahl Seiten (3-12)
└── shape.rotationSpeed  ← Rotationsgeschwindigkeit (°/s)

anim.*                    ← Animation
├── anim.decay           ← Linear/Exponential/Hold/Bounce
└── anim.decayTime       ← Abklingzeit (0.05-2.0s)

color.*                   ← ColorSchemeModule
├── color.scheme         ← Classic/Neon/Fire/Ice/Ocean/...
├── color.animSpeed      ← Animationsgeschwindigkeit (Hz)
└── color.beatFlash      ← Beat-Flash aktivieren

bg.*                      ← Hintergrund
└── bg.fade              ← Hintergrund-Fade (0-1)
```

### Modul-Zugriff

```cpp
// Direkter Zugriff auf Module
viz.audioSource().setGain(1.5f);
viz.audioSource().smoothing().setTimeMs(100.0f);

viz.colorSchemeModule().setScheme(lumi::modules::ColorSchemeType::Fire);
viz.pulseShapeModule().setShape(lumi::modules::PulseShape::Ring);
```

## Smoothing-Algorithmen

| Algorithmus | Beschreibung | Lag | Verwendung |
|-------------|--------------|-----|------------|
| None | Kein Smoothing | 0 | Beat-reaktive Effekte |
| SMA | Simple Moving Average | Hoch | Sehr glatte Bewegung |
| EMA | Exponential Moving Average | Mittel | **Empfohlen** - gute Balance |
| WMA | Weighted Moving Average | Mittel | Kompromiss SMA/EMA |
| DEMA | Double EMA | Niedrig | Reaktiv aber glatt |

## Presets

```cpp
// SmoothingModule Presets
viz.audioSource().smoothing().applyPreset(lumi::modules::SmoothingPreset::Balanced);

// Verfügbar:
// - Instant   (0 ms, None)
// - Reactive  (20 ms, EMA)
// - Balanced  (50 ms, EMA)  ← Default
// - Smooth    (100 ms, EMA)
// - Sluggish  (200 ms, DEMA)
```

## JSON Serialisierung (Phase 3)

```json
{
  "type": "pulsing",
  "version": "3.0",
  "params": {
    "audio.scale": "log",
    "audio.bands": 64,
    "audio.gain": 1.2,
    "audio.smooth.algorithm": "EMA",
    "audio.smooth.timeMs": 50,
    "shape.type": "circle",
    "shape.baseSize": 0.6,
    "color.scheme": "neon",
    "color.beatFlash": true
  }
}
```
