# SmoothingModule — Temporal Smoothing Algorithms

> **Version:** 1.0.0  
> **Datum:** 2026-01-02  
> **Typ:** CppModuleDoc  
> **Status:** Stabil  
> **Modul:** lumi::modules::SmoothingModule  
> **Dateien:** SmoothingModule.hpp  
> **Namespace:** lumi::modules  
> **Abhängigkeiten:** IModule, std::vector, std::deque  
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

SmoothingModule bietet **temporale Glättungsalgorithmen** für Audio-Visualisierung. Es reduziert Flicker und erzeugt flüssige Animationen.

### 1.2 Verantwortlichkeiten

- Implementierung von None, SMA, EMA, WMA, DEMA Algorithmen
- File-basierte User-Presets (.smooth Dateien)
- Automatisches [Custom]-Preset bei manuellen Änderungen

### 1.3 Nicht-Verantwortlichkeiten

- Keine FFT-Verarbeitung (→ AudioSourceModule)
- Keine UI-Widgets (→ ConfigPanel)

### 1.4 Algorithmus-Vergleich

```
Input:    ╱╲    ╱╲╱╲      ╱╲
         ╱  ╲  ╱    ╲    ╱  ╲

None:     ╱╲    ╱╲╱╲      ╱╲     (identisch)

EMA:     ╱╲    ╱─╲       ╱╲     (abgerundet)

SMA:    ╱──╲  ╱───╲     ╱──╲    (verzögert + glatt)

DEMA:    ╱╲   ╱╲╱╲      ╱╲       (weniger Lag)
```

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| IModule | Intern | Basis-Interface |
| std::deque | Standard | Ringbuffer für SMA/WMA |
| std::filesystem | Standard | Preset-Dateien |
| nlohmann/json | Extern | JSON Parsing |

---

## 3. API

### 3.1 Konstruktion

```cpp
SmoothingModule();
~SmoothingModule() = default;
```

### 3.2 SmoothingAlgorithm Enum

| Wert | Name | Parameter | Beschreibung |
|------|------|-----------|--------------|
| 0 | None | — | Pass-through |
| 1 | SMA | windowSize | Simple Moving Average |
| 2 | EMA | timeMs | Exponential Moving Average |
| 3 | WMA | windowSize | Weighted Moving Average |
| 4 | DEMA | timeMs | Double Exponential MA |

### 3.3 Parameter

| ID | Typ | Bereich | Default | Sichtbar bei |
|----|-----|---------|---------|--------------|
| `preset` | Enum | [Custom], Builtin, User... | Balanced | Immer |
| `algorithm` | Enum | None-DEMA | EMA | Immer |
| `timeMs` | Float | 1-500 ms | 50 | EMA, DEMA |
| `windowSize` | Int | 2-60 | 8 | SMA, WMA |
| `primeFirstFrame` | Bool | — | true | Alle außer None |

### 3.4 Builtin Presets

| Name | Algorithm | timeMs | windowSize |
|------|-----------|--------|------------|
| Instant | None | 0 | 2 |
| Reactive | EMA | 20 | 3 |
| **Balanced** | EMA | 50 | 8 |
| Smooth | EMA | 100 | 12 |
| Sluggish | DEMA | 200 | 20 |

### 3.5 Öffentliche Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `process(input, deltaTime)` | `float`, `float` | `float` | Wert glätten |
| `processArray(in, out, size, dt)` | Arrays | `void` | Array glätten |
| `reset()` | — | `void` | Buffer leeren |
| `setAlgorithm(algo)` | `SmoothingAlgorithm` | `void` | Algorithmus setzen |
| `setTimeMs(ms)` | `float` | `void` | Zeitkonstante (EMA/DEMA) |
| `setWindowSize(size)` | `int` | `void` | Fenstergröße (SMA/WMA) |
| `savePreset(name)` | `string` | `void` | User-Preset speichern |
| `loadPreset(name)` | `string` | `void` | Preset laden |
| `deletePreset(name)` | `string` | `bool` | User-Preset löschen |
| `presetNames()` | — | `vector<string>` | Alle Preset-Namen |

### 3.6 Statische Methoden

| Methode | Parameter | Beschreibung |
|---------|-----------|--------------|
| `setUserPresetsDirectory(path)` | `string` | Preset-Verzeichnis setzen |

---

## 4. Verwendung

### 4.1 Einfaches Beispiel

```cpp
SmoothingModule smooth;
smooth.setAlgorithm(SmoothingAlgorithm::EMA);
smooth.setTimeMs(50.0f);

// Pro Frame
float smoothed = smooth.process(rawValue, deltaTime);
```

### 4.2 Array-Verarbeitung

```cpp
std::vector<float> input(64), output(64);
smooth.processArray(input.data(), output.data(), 64, deltaTime);
```

### 4.3 Presets speichern/laden

```cpp
// Directory setzen (einmalig beim Start)
SmoothingModule::setUserPresetsDirectory("/path/to/presets");

// Preset speichern
smooth.savePreset("MySmooth");

// Preset laden
smooth.loadPreset("MySmooth");
```

---

## 5. Interna

### 5.1 EMA-Berechnung

```cpp
// Zeitkonstante τ = timeMs / 1000
float tau = m_timeMs / 1000.0f;
float alpha = 1.0f - std::exp(-deltaTime / tau);
m_lastOutput = alpha * input + (1.0f - alpha) * m_lastOutput;
```

### 5.2 SMA-Berechnung

```cpp
m_buffer.push_back(input);
if (m_buffer.size() > m_windowSize)
{
    m_buffer.pop_front();
}

float sum = 0.0f;
for (float val : m_buffer)
{
    sum += val;
}
return sum / m_buffer.size();
```

### 5.3 File-Format (.smooth)

```json
{
  "name": "MyPreset",
  "algorithm": 2,
  "timeMs": 75.0,
  "windowSize": 10,
  "primeFirstFrame": true
}
```

---

## 6. Thread-Sicherheit

**Nicht thread-safe.** Eine Instanz pro Thread verwenden.

---

## 7. Fehlerbehandlung

- `loadPreset()` bei ungültigem Namen: Keine Änderung, Log-Warnung
- `deletePreset()` gibt `false` bei Builtin-Presets
- `setTimeMs()` mit Wert außerhalb 1-500: Clamp
- `setWindowSize()` mit Wert außerhalb 2-60: Clamp
- Keine Exceptions

---

## 8. Siehe auch

- [IModule.md](IModule.md) — Basis-Interface
- [AudioSourceModule.md](AudioSourceModule.md) — Verwendet SmoothingModule
- [Preset_System.md](Preset_System.md) — File-Format Details

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Separate timeMs/windowSize mit Visibility, File-basierte Presets** |
