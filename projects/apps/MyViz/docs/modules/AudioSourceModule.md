# AudioSourceModule — FFT Processing for Visualization

> **Version:** 1.0.0  
> **Datum:** 2026-01-02  
> **Typ:** CppModuleDoc  
> **Status:** Stabil  
> **Modul:** lumi::modules::AudioSourceModule  
> **Dateien:** AudioSourceModule.hpp  
> **Namespace:** lumi::modules  
> **Abhängigkeiten:** IModule, SmoothingModule  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Interna](#5-interna)
6. [Thread-Sicherheit](#6-thread-sicherheit)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

### 1.1 Zweck

AudioSourceModule ist das **primäre Audio-Eingabemodul** für Visualizer. Es verarbeitet FFT-Daten von BASS und stellt normalisierte Spektrum-Daten bereit.

### 1.2 Verantwortlichkeiten

- FFT-Daten zu Frequenzbändern mappen
- Skalierung (Linear, Log, Mel)
- Normalisierung (Floor, Ceiling, Gain)
- Embedded SmoothingModule für temporale Glättung

### 1.3 Nicht-Verantwortlichkeiten

- Keine Audio-Wiedergabe (→ BassEngine)
- Keine Beat-Detection (→ Geplant)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| IModule | Intern | Basis-Interface |
| SmoothingModule | Intern | Temporale Glättung |
| BASS | Extern | FFT-Daten (via Visualizer) |

---

## 3. API

### 3.1 Konstruktion

```cpp
AudioSourceModule();
~AudioSourceModule() = default;
```

### 3.2 FrequencyScale Enum

| Wert | Name | Beschreibung |
|------|------|--------------|
| 0 | Linear | Lineare Frequenzverteilung |
| 1 | Log | Logarithmische Verteilung |
| 2 | Mel | Mel-Skala (gehöroptimiert) |

### 3.3 Parameter

| ID | Typ | Bereich | Default | Beschreibung |
|----|-----|---------|---------|--------------|
| `preset` | Enum | [Custom], Default, User... | Default | Audio-Preset |
| `scale` | Enum | Linear, Log, Mel | Log | Frequenz-Skalierung |
| `bands` | Int | 8-512 | 64 | Anzahl Ausgabe-Bänder |
| `floorDb` | Float | -120 bis 0 | -60 | Noise Floor in dB |
| `ceilDb` | Float | -60 bis +20 | 0 | Ceiling in dB |
| `clamp01` | Bool | — | true | Auf 0-1 clampen |
| `gain` | Float | 0.1-5.0 | 1.0 | Eingangsverstärkung |

### 3.4 Embedded Smoothing Parameter

| ID | Typ | Bereich | Default | Beschreibung |
|----|-----|---------|---------|--------------|
| `smooth.preset` | Enum | Siehe SmoothingModule | Balanced | Smoothing-Preset |
| `smooth.algorithm` | Enum | None-DEMA | EMA | Algorithmus |
| `smooth.timeMs` | Float | 1-500 | 50 | Zeit (EMA/DEMA) |
| `smooth.windowSize` | Int | 2-60 | 8 | Fenster (SMA/WMA) |

### 3.5 FrequencyBands Struct

```cpp
struct FrequencyBands
{
    float sub;      // 20-60 Hz
    float bass;     // 60-250 Hz
    float lowMid;   // 250-500 Hz
    float mid;      // 500-2000 Hz
    float highMid;  // 2000-4000 Hz
    float treble;   // 4000-20000 Hz
    
    float overall() const;         // Gewichteter Durchschnitt
    float operator[](int i) const; // Index-Zugriff [0-5]
};
```

### 3.6 Öffentliche Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `update(fftData, size, dt)` | `float*`, `int`, `float` | `void` | FFT verarbeiten |
| `reset()` | — | `void` | State zurücksetzen |
| `spectrum()` | — | `const vector<float>&` | Verarbeitetes Spektrum |
| `frequencyBands()` | — | `const FrequencyBands&` | 6-Band Analyse |
| `smoothing()` | — | `SmoothingModule&` | Zugriff auf Smoothing |
| `setGain(gain)` | `float` | `void` | Verstärkung setzen |
| `setBands(count)` | `int` | `void` | Band-Anzahl setzen |
| `savePreset(name)` | `string` | `void` | User-Preset speichern |
| `loadPreset(name)` | `string` | `void` | Preset laden |

### 3.7 Statische Methoden

| Methode | Parameter | Beschreibung |
|---------|-----------|--------------|
| `setUserPresetsDirectory(path)` | `string` | Preset-Verzeichnis setzen |

---

## 4. Verwendung

### 4.1 Einfaches Beispiel

```cpp
AudioSourceModule audio;

// Pro Frame (in Visualizer::render)
audio.update(fftData, fftSize, deltaTime);

// Spektrum für Rendering
const auto& spectrum = audio.spectrum();
for (size_t i = 0; i < spectrum.size(); ++i)
{
    float height = spectrum[i];
    // Render bar...
}
```

### 4.2 Frequenzbänder

```cpp
const auto& bands = audio.frequencyBands();

float bassLevel = bands.bass;      // Für Pulsing
float trebleLevel = bands.treble;  // Für Sparkles
float overall = bands.overall();   // Gesamtlevel
```

### 4.3 Parameter über Pfad

```cpp
// Audio-Parameter
audio.setParam("gain", 1.5f);
audio.setParam("scale", 1);  // Log

// Smoothing über Audio-Modul steuern
audio.setParam("smooth.algorithm", 2);   // EMA
audio.setParam("smooth.timeMs", 100.0f);
```

### 4.4 Preset-System

```cpp
// Directory setzen (einmalig)
AudioSourceModule::setUserPresetsDirectory("/path/to/presets");

// Preset speichern
audio.savePreset("HighGain");

// Preset laden
audio.loadPreset("HighGain");
```

---

## 5. Interna

### 5.1 Processing-Pipeline

```
FFT Input (512-2048 bins)
    │
    ▼
[Frequency Mapping] (Linear/Log/Mel)
    │
    ▼
[Band Reduction] (512 → 64 bands)
    │
    ▼
[Normalization] (Floor/Ceiling/Gain)
    │
    ▼
[Smoothing] (EMA/SMA/WMA/DEMA)
    │
    ▼
Output Spectrum (64 bands, 0-1)
```

### 5.2 Log-Skalierung

```cpp
// Bin zu Frequenz (bei 44.1kHz, 1024 FFT)
float freq = binIndex * sampleRate / fftSize;

// Log-Mapping zu normalisierten Koordinaten
float logFreq = std::log10(freq / 20.0f) / std::log10(20000.0f / 20.0f);
```

### 5.3 dB-Normalisierung

```cpp
// Magnitude zu dB
float dB = 20.0f * std::log10(magnitude + 1e-10f);

// Normalisieren auf 0-1
float normalized = (dB - m_floorDb) / (m_ceilDb - m_floorDb);
normalized = std::clamp(normalized * m_gain, 0.0f, 1.0f);
```

### 5.4 File-Format (.audio)

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
  "smoothTimeMs": 40.0
}
```

---

## 6. Thread-Sicherheit

**Nicht thread-safe.** `update()` vom Render-Thread aufrufen.

---

## 7. Fehlerbehandlung

- `update()` mit nullptr: Keine Verarbeitung, kein Crash
- Ungültige FFT-Size: Clamp auf 512-4096
- Ungültige Gain/Floor/Ceil Werte: Clamp auf gültige Bereiche
- Keine Exceptions

---

## 8. Siehe auch

- [IModule.md](IModule.md) — Basis-Interface
- [SmoothingModule.md](SmoothingModule.md) — Embedded Smoothing
- [Audio_System.md](Audio_System.md) — BASS Integration
- [Preset_System.md](Preset_System.md) — File-Format Details

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: FFT-Processing, FrequencyBands, Embedded Smoothing, File-Presets** |
