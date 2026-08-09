# Dateiformate — Referenz der Preset-Formate (.lvp, .smooth, .audio, .grad)

> **Version:** 1.1.0
> **Datum:** 2026-07-19
> **Typ:** Reference
> **Status:** Aktiv
> **Sprache:** Deutsch
> **Änderung 1.1.0:** formatVersion 2 (Pipeline-Key-Schema, Phase 4 Schritt 5) — Loader wertet formatVersion aus und übersetzt Alt-Presets per Alias-Map; Beispiel auf neue Keys

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Visualizer-Preset (.lvp)](#3-visualizer-preset-lvp)
4. [Smoothing-Preset (.smooth)](#4-smoothing-preset-smooth)
5. [Audio-Preset (.audio)](#5-audio-preset-audio)
6. [Gradient-Preset (.grad)](#6-gradient-preset-grad)
7. [Schnellreferenz](#7-schnellreferenz)
8. [Siehe auch](#8-siehe-auch)

---

## 1. Übersicht

Alle Presets sind JSON-Dateien. Speicherorte (Basis: `QStandardPaths::AppDataLocation`,
Organisation „LumiViz Project", App „LumiViz"):

```
%APPDATA%\LumiViz Project\LumiViz\presets\        (Windows; Linux/macOS analog)
├── visuals\{visualizerId}\{Name}.lvp
├── smoothing\{Name}.smooth
├── audio\{Name}.audio
└── gradients\{Name}.grad
```

Hinweis: Der `.lvp`-Unterordner heißt **`visuals`** (Code:
`VisualizerPresetManager::getVisualizerDir()`), nicht `visualizer` wie in älteren
Doku-Ständen.

---

## 2. Konventionen

### 2.1 Keine Kommentare in Preset-JSONs

**Preset-Dateien vertragen KEINE Kommentare** (`//`, `/* */`). Die Loader sind teils
`QJsonDocument` (strikt), teils minimale String-Parser — Kommentare führen zu
Parse-Fehlern bzw. undefinierten Werten. Das gilt unabhängig davon, ob ein Editor
(z. B. Visual Studio) JSON-with-Comments toleriert: Was der Editor akzeptiert, kann
die App/CLI trotzdem nicht laden.

### 2.2 Dateinamen

Der Preset-Name wird 1:1 zum Dateinamen (`<Name><Endung>`). Die UI validiert derzeit
nicht — daher: keine `/`, `\`, `.` oder andere Pfad-Sonderzeichen verwenden.

### 2.3 Encoding und Syntax

- UTF-8, keine Trailing Commas
- `.lvp` wird von Qt „Indented" geschrieben; die Modul-Formate mit 2-Space-Einrückung
- Symbole: ✓ = Pflichtfeld, — = optional (mit Default)

---

## 3. Visualizer-Preset (.lvp)

**Writer/Reader:** `VisualizerPresetManager` (`presetToJson()` / `jsonToPreset()`,
`QJsonDocument`).
**Speicherort:** `presets/visuals/{visualizerId}/`

### 3.1 Schema

Top-Level besteht aus **`header`** und **`parameters`** (das flache
`{visualizerId, name, params}`-Schema älterer Doku ist falsch):

| Feld | Typ | Pflicht | Beschreibung |
|------|-----|---------|--------------|
| `header` | object | ✓ | Metadaten (fehlt es, wird die Datei verworfen) |
| `header.name` | string | ✓ | Preset-Name |
| `header.visualizerId` | string | ✓ | Muss zum Ziel-Visualizer passen (strikte Prüfung beim Laden) |
| `header.description` | string | — | Beschreibung |
| `header.author` | string | — | Autor |
| `header.version` | number (int) | — (Default 1) | Preset-Version |
| `header.formatVersion` | number (int) | — (Default 1) | Key-Schema-Version, aktuell `2` (`CURRENT_FORMAT_VERSION`, Pipeline-Schema seit Phase 4 Schritt 5). Beim Speichern wird immer `2` geschrieben. **Beim Laden ausgewertet:** Presets mit `formatVersion < 2` (auch ohne Feld) laufen in `applyPreset()` durch die Alias-Map des jeweiligen Visualizers — jeder Alt-Key wird auf den Pipeline-Key übersetzt, einzelne Einträge zusätzlich wert-konvertiert (z. B. `waveform.smoothing` → `audio.smooth.timeMs`). Quelle der Tabellen: [Parameter_Key_Migration.md](../visuals/Parameter_Key_Migration.md) |
| `parameters` | object | — | Parameter-ID → Wert (neues Schema: Stufen-Präfixe `audio.` / `map.` / `color.<handle>.` / `render.` / `peak.`/`particle.` / `post.`) |

### 3.2 Beispiel

Es liegen keine Beispiel-Presets im Repo; das Beispiel entspricht dem vom Code
erzeugten Format:

```json
{
    "header": {
        "name": "Neon Pulse",
        "visualizerId": "pulsing",
        "description": "Vibrant neon pulsing effect",
        "author": "LumiViz Team",
        "version": 2,
        "formatVersion": 2
    },
    "parameters": {
        "audio.gain": 1.2,
        "audio.smooth.algorithm": 1,
        "audio.smooth.timeMs": 50.0,
        "render.type": 0,
        "render.minSize": 0.2,
        "color.main.mode": 1,
        "color.main.gradientPresetName": "Neon",
        "color.main.gradientData": "0.0000,1.0000,0.0000,1.0000,1.0000;0.5000,0.0000,1.0000,1.0000,1.0000;1.0000,1.0000,0.0000,1.0000,1.0000",
        "color.main.angle": 0.0
    }
}
```

Alt-Presets (formatVersion 1, Keys wie `shape.type` oder `shape.color.mode`) bleiben
ladbar — die Alias-Map übersetzt sie beim Anwenden; beim nächsten Speichern liegt die
Datei im neuen Schema mit `formatVersion: 2` vor (kein Dual-Write).

### 3.3 Wertetypen in `parameters`

| ParamType | JSON beim Speichern | Beim Laden ankommender Variant-Typ |
|-----------|---------------------|------------------------------------|
| Bool | boolean | `bool` |
| Int | number | **`float`** (!) |
| Float | number | `float` |
| Enum | number (Index) | **`float`** (!) |
| String | string | `std::string` |
| Color | array `[r, g, b, a]` (4 × 0.0–1.0) | `Color4f` |

**float-Vertrag:** JSON unterscheidet Int/Float nicht; der Loader legt *jede* Zahl als
`float` ab. `setParam()`-Implementierungen müssen für Int-/Enum-Parameter daher auch
`float` akzeptieren. Details und Code-Muster:
[Preset_System.md, Abschnitt 6](Preset_System.md#6-der-float-vertrag).

**Arrays:** Nur 4-elementige Zahlen-Arrays werden zurückgelesen (als Color4f). Andere
Array-Werte (`vector<float>`, `vector<int>`) werden zwar serialisiert, beim Laden aber
ignoriert.

**gradientData-String** (Wert des versteckten Parameters `*.gradientData`):

```
position,r,g,b,a;position,r,g,b,a;...
```

Stops per `;` getrennt, Werte per `,`, alle floats 0.0–1.0 (4 Nachkommastellen),
mindestens 2 Stops.

---

## 4. Smoothing-Preset (.smooth)

**Writer/Reader:** `SmoothingModule::savePreset()` / `parsePresetFile()`
(`include/visualizers/modules/processing/SmoothingModule.hpp`; minimaler String-Parser).
**Speicherort:** `presets/smoothing/`

### 4.1 Schema

| Feld | Typ | Pflicht | Default | Beschreibung |
|------|-----|---------|---------|--------------|
| `name` | string | — (Fallback: Dateiname) | — | Preset-Name |
| `algorithm` | number | ✓ | 0 | SmoothingAlgorithm (0–4) |
| `timeMs` | number | — | 0.0 | Zeitkonstante für EMA/DEMA |
| `windowSize` | number | — | 8 | Fenstergröße für SMA/WMA |
| `primeFirstFrame` | boolean | — | false | Ersten Frame initialisieren |

### 4.2 Beispiel

```json
{
  "name": "UltraSmooth",
  "algorithm": 2,
  "timeMs": 150.0,
  "windowSize": 15,
  "primeFirstFrame": true
}
```

### 4.3 Algorithm-Werte

| Wert | Name |
|------|------|
| 0 | None |
| 1 | SMA |
| 2 | EMA |
| 3 | WMA |
| 4 | DEMA |

---

## 5. Audio-Preset (.audio)

**Writer/Reader:** `AudioSourceModule::savePreset()` / `parsePresetFile()`
(`include/visualizers/modules/source/AudioSourceModule.hpp`; minimaler String-Parser).
**Speicherort:** `presets/audio/`

### 5.1 Schema

| Feld | Typ | Pflicht | Default | Beschreibung |
|------|-----|---------|---------|--------------|
| `name` | string | — (Fallback: Dateiname) | — | Preset-Name |
| `scale` | number | — | 0 | FrequencyScale (0–2) |
| `bands` | number | — | 0 | Anzahl Bänder |
| `floorDb` | number | — | 0.0 | Noise Floor (dB) |
| `ceilDb` | number | — | 0.0 | Ceiling (dB) |
| `clamp01` | boolean | — | false | Clamp auf 0–1 |
| `gain` | number | — | 0.0 | Verstärkung |
| `smoothAlgorithm` | number | — | 0 | Eingebettetes Smoothing: Algorithmus (0–4) |
| `smoothTimeMs` | number | — | 0.0 | Eingebettetes Smoothing: Zeitkonstante |

Ein Feld `smoothWindowSize` (Alt-Doku) existiert **nicht** — es wird weder geschrieben
noch gelesen.

### 5.2 Beispiel

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

### 5.3 Scale-Werte

| Wert | Name |
|------|------|
| 0 | Linear |
| 1 | Log |
| 2 | Mel |

---

## 6. Gradient-Preset (.grad)

**Writer/Reader:** `ColorGradientModule::savePreset()` / `parseGradientFile()`
(`src/visualizers/modules/ColorGradientModule.cpp`; minimaler String-Parser, bewusst
Qt-/Fremdbibliotheks-frei).
**Speicherort:** `presets/gradients/`

### 6.1 Schema

| Feld | Typ | Pflicht | Beschreibung |
|------|-----|---------|--------------|
| `name` | string | ✓ | Preset-Name (leer → Datei wird verworfen) |
| `mode` | number | — | GradientMode (0–3) |
| `angle` | number | — | Winkel in Grad |
| `stops` | array | ✓ (≥ 1, praktisch ≥ 2) | Flache Arrays `[pos, r, g, b, a]` |
| `midpoints` | array | — | Flache Liste von Positionen (0.0–1.0), eine je Stop-Paar |

Abweichungen zur Alt-Doku (dort falsch):

- Stops sind **flache 5er-Arrays** `[pos,r,g,b,a]`, keine Objekte `{pos,r,g,b,a}`.
- Midpoints sind eine **flache Zahlenliste** (Position je Stop-Paar), keine
  `{index, position}`-Objekte.
- Es gibt **kein** Feld `outlineWidth` in der Datei.
- `mode` hat 4 Werte (siehe unten), nicht 3.

### 6.2 Beispiel

```json
{
  "name": "CustomSunset",
  "mode": 1,
  "angle": 90.0,
  "stops": [[0, 0.5, 0, 0.5, 1], [0.5, 1, 0.5, 0, 1], [1, 1, 1, 0, 1]],
  "midpoints": [0.3, 0.6]
}
```

### 6.3 Mode-Werte (GradientMode)

| Wert | Name | Bedeutung |
|------|------|-----------|
| 0 | Solid | Einzelfarbe (kein Verlauf) |
| 1 | Linear | Linearer Verlauf mit Winkel |
| 2 | Radial | Verlauf von Mitte nach Rand |
| 3 | Outline | Nur Kontur, Füllung transparent |

---

## 7. Schnellreferenz

| Format | Endung | Unterordner | Pflichtfelder | Parser |
|--------|--------|-------------|---------------|--------|
| Visualizer | `.lvp` | `visuals/{vizId}/` | `header.name`, `header.visualizerId` | QJsonDocument (strikt) |
| Smoothing | `.smooth` | `smoothing/` | `algorithm` (Name: Fallback Dateiname) | String-Parser |
| Audio | `.audio` | `audio/` | — (Name: Fallback Dateiname) | String-Parser |
| Gradient | `.grad` | `gradients/` | `name`, `stops` | String-Parser |

| Enum | Feld | Werte |
|------|------|-------|
| SmoothingAlgorithm | `algorithm`, `smoothAlgorithm` | 0–4 (None, SMA, EMA, WMA, DEMA) |
| FrequencyScale | `scale` | 0–2 (Linear, Log, Mel) |
| GradientMode | `mode` | 0–3 (Solid, Linear, Radial, Outline) |

---

## 8. Siehe auch

- [Preset_System.md](Preset_System.md) — Preset-Hierarchie, API, float-Vertrag, Bedienung
- [../visuals/Parameter_Reference.md](../visuals/Parameter_Reference.md) — Parameter-IDs für den `parameters`-Block
- [../ui/ConfigPanel_Guide.md](../ui/ConfigPanel_Guide.md) — Preset-Bedienung in der UI
