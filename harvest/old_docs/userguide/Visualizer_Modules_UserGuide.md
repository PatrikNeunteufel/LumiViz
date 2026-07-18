# Visualizer Module System — Benutzerhandbuch

> **Version:** 1.0.0  
> **Datum:** 2026-01-02  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler, UI-Designer  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Wie arbeite ich mit Audio-Parametern?](#4-wie-arbeite-ich-mit-audio-parametern)
5. [Wie konfiguriere ich Smoothing?](#5-wie-konfiguriere-ich-smoothing)
6. [Wie erstelle ich eigene Farbverläufe?](#6-wie-erstelle-ich-eigene-farbverläufe)
7. [Wie speichere und lade ich Presets?](#7-wie-speichere-und-lade-ich-presets)
8. [Wie passe ich die Visualizer-Form an?](#8-wie-passe-ich-die-visualizer-form-an)
9. [Stolpersteine und Lösungen](#9-stolpersteine-und-lösungen)
10. [Troubleshooting](#10-troubleshooting)
11. [Siehe auch](#11-siehe-auch)
12. [Changelog](#12-changelog)

---

## 1. Überblick

Das Visualizer Module System bietet eine modulare Architektur für Audio-Visualisierungen. Jedes Modul kapselt eine spezifische Funktionalität und kann unabhängig konfiguriert werden.

### Features

- **AudioSourceModule:** FFT-Verarbeitung mit Log/Mel-Skalierung
- **SmoothingModule:** Temporale Glättung (EMA, SMA, WMA, DEMA)
- **ColorGradientModule:** Farbverläufe mit Presets
- **PulseShapeModule:** Verschiedene Formen (Circle, Ring, Star, etc.)
- **File-basierte Presets:** Speichere und teile deine Konfigurationen

### Modulare Architektur

```
PulsingVisualizer
├── AudioSourceModule
│   └── SmoothingModule (embedded)
├── PulseShapeModule
│   └── ColorGradientModule (embedded)
└── Parameter-System (IModule)
```

---

## 2. Voraussetzungen

### Technisch

- [ ] Qt6 6.5+ installiert
- [ ] OpenGL 3.3+ fähige Grafikkarte
- [ ] BASS Audio Library eingebunden
- [ ] C++17 Compiler

### Wissen

- [ ] Grundkenntnisse Qt/C++
- [ ] Verständnis von Audio-Spektren (FFT)
- [ ] Grundlagen der Farbtheorie (optional)

---

## 3. Schnellstart

### 3.1 Visualizer erstellen

```cpp
#include "visualizers/PulsingVisualizer.hpp"

// Visualizer erstellen
PulsingVisualizer viz;

// Standard-Konfiguration verwenden
viz.resetToDefaults();
```

### 3.2 Parameter anpassen

```cpp
// Gain erhöhen für leise Audio-Quellen
viz.setParam("audio.gain", 2.0f);

// Smoothing für flüssigere Animation
viz.setParam("audio.smooth.algorithm", 2);  // EMA
viz.setParam("audio.smooth.timeMs", 80.0f);

// Farbschema ändern
viz.setParam("shape.color.preset", 2);  // Ocean
```

### 3.3 Im Render-Loop verwenden

```cpp
void MyWidget::paintGL()
{
    // FFT-Daten vom Audio-System holen
    float* fftData = m_audioEngine->getFFTData();
    int fftSize = m_audioEngine->getFFTSize();
    
    // Visualizer updaten und rendern
    viz.update(fftData, fftSize, m_deltaTime);
    viz.render();
}
```

---

## 4. Wie arbeite ich mit Audio-Parametern?

### 4.1 Frequenz-Skalierung wählen

Die Skalierung bestimmt, wie FFT-Bins auf sichtbare Bänder gemappt werden:

| Skalierung | Wann verwenden? |
|------------|-----------------|
| **Linear** | Technische Analyse, alle Frequenzen gleichgewichtet |
| **Log** | Standard für Musik, mehr Bass-Auflösung |
| **Mel** | Sprachanalyse, gehöroptimiert |

```cpp
// Log-Skalierung (empfohlen für Musik)
viz.setParam("audio.scale", 1);
```

### 4.2 Dynamikbereich anpassen

Passe Floor und Ceiling an deine Audio-Quelle an:

```cpp
// Für leise Quellen (z.B. Klassik)
viz.setParam("audio.floorDb", -80.0f);
viz.setParam("audio.ceilDb", -10.0f);
viz.setParam("audio.gain", 1.5f);

// Für laute Quellen (z.B. EDM)
viz.setParam("audio.floorDb", -40.0f);
viz.setParam("audio.ceilDb", 0.0f);
viz.setParam("audio.gain", 0.8f);
```

### 4.3 Frequenzbänder nutzen

Greife auf vordefinierte Frequenzbänder zu:

```cpp
const auto& bands = viz.audioSource()->frequencyBands();

// Für Beat-Detection
if (bands.bass > 0.7f)
{
    triggerBeatEffect();
}

// Für Sparkle-Effekte
float trebleIntensity = bands.treble;
```

| Band | Frequenzbereich | Typische Verwendung |
|------|-----------------|---------------------|
| `sub` | 20-60 Hz | Sub-Bass, kaum hörbar |
| `bass` | 60-250 Hz | Kick, Bass, Beat-Detection |
| `lowMid` | 250-500 Hz | Wärme, Fülle |
| `mid` | 500-2000 Hz | Vocals, Instrumente |
| `highMid` | 2000-4000 Hz | Präsenz, Klarheit |
| `treble` | 4000-20000 Hz | Höhen, Hi-Hats |

---

## 5. Wie konfiguriere ich Smoothing?

### 5.1 Den richtigen Algorithmus wählen

| Algorithmus | Eigenschaften | Empfohlen für |
|-------------|---------------|---------------|
| **None** | Kein Smoothing, raw | Debugging, schnelle Reaktion |
| **SMA** | Gleichmäßig, verzögert | Langsame, gleichmäßige Animationen |
| **EMA** | Schnell, natürlich | ⭐ Standard für Musik |
| **WMA** | Gewichtet, mittel | Alternative zu SMA |
| **DEMA** | Schnell, wenig Lag | Reaktive Visualisierungen |

```cpp
// EMA mit 50ms Zeitkonstante (Standard)
viz.setParam("audio.smooth.algorithm", 2);
viz.setParam("audio.smooth.timeMs", 50.0f);
```

### 5.2 Parameter verstehen

**timeMs (für EMA/DEMA):**
- Niedrig (20-40ms): Reaktiv, folgt schnellen Änderungen
- Mittel (50-100ms): Ausgewogen, flüssig
- Hoch (100-200ms): Träge, sehr glatt

**windowSize (für SMA/WMA):**
- Klein (3-5): Wenig Glättung
- Mittel (8-12): Moderate Glättung
- Groß (15-30): Starke Glättung, spürbare Verzögerung

### 5.3 Builtin-Presets verwenden

```cpp
// Schnellauswahl über Preset
viz.setParam("audio.smooth.preset", 3);  // Balanced
```

| Preset | Algorithm | timeMs/windowSize | Verwendung |
|--------|-----------|-------------------|------------|
| Instant | None | — | Raw-Daten |
| Reactive | EMA | 20ms | Schnelle Reaktion |
| **Balanced** | EMA | 50ms | ⭐ Allround |
| Smooth | EMA | 100ms | Ruhige Animationen |
| Sluggish | DEMA | 200ms | Sehr glatt |

---

## 6. Wie erstelle ich eigene Farbverläufe?

### 6.1 Builtin-Presets laden

```cpp
// Preset laden
viz.setParam("shape.color.preset", 1);  // Fire
```

| Index | Name | Beschreibung |
|-------|------|--------------|
| 1 | Fire | Rot → Orange → Gelb |
| 2 | Ocean | Dunkelblau → Cyan → Weiß |
| 3 | Neon | Cyan → Magenta → Gelb |
| 4 | Forest | Dunkelgrün → Hellgrün |
| 5 | Sunset | Violett → Orange → Gelb |
| 6 | Rainbow | Vollständiges Spektrum |
| 7 | Monochrome | Schwarz → Weiß |

### 6.2 Eigenen Gradient erstellen

```cpp
auto& gradient = viz.colorGradient();

// Alle Stops entfernen
gradient.clearStops();

// Neue Stops hinzufügen (Position 0-1, RGBA 0-1)
gradient.addStop(0.0f, {0.1f, 0.0f, 0.3f, 1.0f});  // Dunkelviolett
gradient.addStop(0.5f, {1.0f, 0.0f, 0.5f, 1.0f});  // Pink
gradient.addStop(1.0f, {1.0f, 0.8f, 0.0f, 1.0f});  // Gold

// Optional: Midpoint anpassen
gradient.setMidpoint(0, 0.3f);  // Pink erscheint früher
```

### 6.3 Gradient-Modus wählen

```cpp
// Linear (Standard)
viz.setParam("shape.color.mode", 1);
viz.setParam("shape.color.angle", 45.0f);  // 45° Winkel

// Radial (vom Zentrum nach außen)
viz.setParam("shape.color.mode", 2);

// Solid (einfarbig)
viz.setParam("shape.color.mode", 0);
viz.setParam("shape.color.solidColor", Color4f{1, 0.5f, 0, 1});
```

---

## 7. Wie speichere und lade ich Presets?

### 7.1 Preset-Verzeichnisse einrichten

Einmalig beim Programmstart:

```cpp
#include <QStandardPaths>

void Application::initPresets()
{
    QString appData = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    
    // Module-Presets
    SmoothingModule::setUserPresetsDirectory(
        (appData + "/presets/smoothing").toStdString());
    ColorGradientModule::setUserPresetsDirectory(
        (appData + "/presets/gradients").toStdString());
    AudioSourceModule::setUserPresetsDirectory(
        (appData + "/presets/audio").toStdString());
    
    // Visualizer-Presets
    m_presetManager->setPresetsDirectory(
        appData + "/presets/visualizer");
}
```

### 7.2 Modul-Preset speichern

```cpp
// Smoothing-Preset speichern
viz.audioSource()->smoothing().savePreset("MySmooth");

// Gradient-Preset speichern
viz.colorGradient().savePreset("MyGradient");

// Audio-Preset speichern
viz.audioSource()->savePreset("MyAudio");
```

### 7.3 Modul-Preset laden

```cpp
// Per Name
viz.audioSource()->smoothing().loadPreset("MySmooth");

// Per Index im Dropdown
viz.setParam("audio.smooth.preset", 6);  // Index 6 = erstes User-Preset
```

### 7.4 Komplettes Visualizer-Preset

```cpp
// Aktuellen Zustand erfassen
auto preset = m_presetManager->capturePreset(&viz, "Energetic");

// Speichern
m_presetManager->savePreset(preset);

// Später laden
auto loaded = m_presetManager->loadPreset("pulsing", "Energetic");
if (loaded)
{
    m_presetManager->applyPreset(&viz, *loaded);
}
```

### 7.5 Preset-Dateien

Die Presets werden als JSON gespeichert:

```
%APPDATA%/MyViz/presets/
├── visualizer/pulsing/Energetic.lvp
├── smoothing/MySmooth.smooth
├── audio/MyAudio.audio
└── gradients/MyGradient.grad
```

---

## 8. Wie passe ich die Visualizer-Form an?

### 8.1 Form wählen

```cpp
viz.setParam("shape.shape", 1);  // Ring
```

| Index | Form | Beschreibung |
|-------|------|--------------|
| 0 | Circle | Gefüllter Kreis |
| 1 | Ring | Ring mit Loch |
| 2 | Ngon | N-seitiges Polygon |
| 3 | Star | Stern |
| 4 | Wave | Wellenform |
| 5 | Flash | Vollbild-Flash |

### 8.2 Form-spezifische Parameter

**Ring:**

```cpp
viz.setParam("shape.shape", 1);
viz.setParam("shape.innerRadius", 0.7f);  // 70% Loch
```

**Polygon/Stern:**

```cpp
viz.setParam("shape.shape", 2);  // Ngon
viz.setParam("shape.sides", 6);  // Hexagon

viz.setParam("shape.shape", 3);  // Star
viz.setParam("shape.sides", 5);  // 5-zackiger Stern
viz.setParam("shape.innerRadius", 0.4f);  // Spitzigkeit
```

### 8.3 Outline anpassen

```cpp
viz.setParam("shape.color.outlineWidth", 3.0f);  // Dickere Outline
```

---

## 9. Stolpersteine und Lösungen

### 9.1 Visualisierung reagiert nicht auf Audio

**Problem:** Die Form pulsiert nicht zur Musik.

**Ursachen:**
1. Audio-Gain zu niedrig
2. Floor/Ceiling falsch eingestellt
3. FFT-Daten werden nicht übergeben

**Lösung:**

```cpp
// 1. Gain erhöhen
viz.setParam("audio.gain", 2.0f);

// 2. Dynamikbereich prüfen
viz.setParam("audio.floorDb", -60.0f);
viz.setParam("audio.ceilDb", 0.0f);

// 3. FFT-Update prüfen
void paintGL()
{
    // Wird update() aufgerufen?
    viz.update(fftData, fftSize, deltaTime);  // ← Nicht vergessen!
    viz.render();
}
```

---

### 9.2 Animation flackert stark

**Problem:** Die Visualisierung springt und flackert.

**Ursache:** Kein oder zu wenig Smoothing.

**Lösung:**

```cpp
// EMA mit längerer Zeitkonstante
viz.setParam("audio.smooth.algorithm", 2);
viz.setParam("audio.smooth.timeMs", 100.0f);

// Oder Preset verwenden
viz.setParam("audio.smooth.preset", 4);  // Smooth
```

---

### 9.3 [Custom] erscheint nicht im Dropdown

**Problem:** Nach manueller Änderung zeigt das Dropdown nicht [Custom].

**Ursache:** UI-Update fehlt.

**Lösung im ConfigPanel:**

```cpp
void ConfigPanel::onParamChanged(const std::string& paramId,
                                  const ParamValue& value)
{
    m_visualizer->setParam(paramId, value);
    
    // Preset auf [Custom] setzen
    QSignalBlocker blocker(m_presetCombo);
    m_presetCombo->setCurrentIndex(0);  // ← [Custom]
}
```

---

### 9.4 timeMs/windowSize werden nicht angezeigt

**Problem:** Parameter fehlen im UI.

**Ursache:** Visibility-System funktioniert korrekt — diese Parameter sind nur bei bestimmten Algorithmen sichtbar.

**Erklärung:**

| Algorithm | timeMs | windowSize |
|-----------|--------|------------|
| None | ❌ | ❌ |
| SMA | ❌ | ✅ |
| EMA | ✅ | ❌ |
| WMA | ❌ | ✅ |
| DEMA | ✅ | ❌ |

---

### 9.5 Preset-Speichern schlägt fehl

**Problem:** `savePreset()` speichert nicht.

**Ursachen:**
1. Verzeichnis nicht gesetzt
2. Ungültiger Name

**Lösung:**

```cpp
// 1. Verzeichnis setzen (einmalig)
SmoothingModule::setUserPresetsDirectory("/valid/path");

// 2. Gültigen Namen verwenden
// ❌ "My/Preset"   (enthält /)
// ❌ "My.Preset"   (enthält .)
// ✅ "MyPreset"
// ✅ "My_Preset"
// ✅ "My Preset"
```

---

## 10. Troubleshooting

### Checkliste bei Problemen

- [ ] Wird `update()` jeden Frame aufgerufen?
- [ ] Sind FFT-Daten gültig (nicht nullptr)?
- [ ] Ist deltaTime korrekt (in Sekunden)?
- [ ] Wurde `resetToDefaults()` nach Erstellung aufgerufen?
- [ ] Sind Preset-Verzeichnisse gesetzt?

### Häufige Fehler

| Symptom | Wahrscheinliche Ursache | Lösung |
|---------|-------------------------|--------|
| Keine Visualisierung | OpenGL-Context fehlt | `makeCurrent()` prüfen |
| Keine Audio-Reaktion | FFT-Daten null | Audio-Engine prüfen |
| Extreme Werte | clamp01 deaktiviert | `audio.clamp01 = true` |
| Preset lädt nicht | Falscher Pfad | Verzeichnis prüfen |
| UI reagiert nicht | Signale blockiert | QSignalBlocker prüfen |

### Debug-Ausgabe aktivieren

```cpp
// Frequenzbänder ausgeben
const auto& bands = viz.audioSource()->frequencyBands();
qDebug() << "Bass:" << bands.bass 
         << "Mid:" << bands.mid 
         << "Treble:" << bands.treble;

// Aktuellen Smoothing-Status
ParamValue algo;
viz.getParam("audio.smooth.algorithm", algo);
qDebug() << "Algorithm:" << std::get<int>(algo);
```

---

## 11. Siehe auch

### Referenzen

- [Parameter_Reference.md](reference/Parameter_Reference.md) — Alle Parameter-IDs
- [Enum_Reference.md](reference/Enum_Reference.md) — Alle Enum-Werte
- [FileFormat_Reference.md](reference/FileFormat_Reference.md) — Preset-Formate
- [API_Cheatsheet.md](reference/API_Cheatsheet.md) — Schnellreferenz

### Modul-Dokumentation

- [IModule.md](modules/IModule.md) — Parameter-System
- [AudioSourceModule.md](modules/AudioSourceModule.md) — FFT-Verarbeitung
- [SmoothingModule.md](modules/SmoothingModule.md) — Glättungsalgorithmen
- [ColorGradientModule.md](modules/ColorGradientModule.md) — Farbverläufe

---

## 12. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: Vollständiges Benutzerhandbuch** |
