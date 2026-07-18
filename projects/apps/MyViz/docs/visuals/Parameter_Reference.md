# Parameter-Referenz — Alle Parameter der 5 Visualizer (SSOT für Phase 4)

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Reference
> **Status:** Aktiv
> **Sprache:** Deutsch

Vollständige Referenz aller Parameter-IDs, **aus dem Code erhoben** (Stand 2026-07-18:
`paramDescs()`-Implementierungen der Visualizer und Module). Ersetzt und erweitert die
Alt-Dokumente `harvest/old_docs/references/Parameter_Reference.md` und
`Enum_Reference.md` (die nur Pulsing kannten). Abweichungen zur Alt-Doku: [§9](#9-korrekturen-gegenüber-der-alt-doku).

---

## Inhaltsverzeichnis

1. [Übersicht und Konventionen](#1-übersicht-und-konventionen)
2. [Gemeinsame Module](#2-gemeinsame-module)
3. [PulsingVisualizer](#3-pulsingvisualizer-pulsing)
4. [WaveformVisualizer](#4-waveformvisualizer-waveform)
5. [OscilloscopeVisualizer](#5-oscilloscopevisualizer-oscilloscope)
6. [SuperscopeVisualizer](#6-superscopevisualizer-superscope)
7. [EqualizerVisualizer](#7-equalizervisualizer-equalizer)
8. [Enum-Referenz](#8-enum-referenz)
9. [Korrekturen gegenüber der Alt-Doku](#9-korrekturen-gegenüber-der-alt-doku)
10. [Siehe auch](#10-siehe-auch)

---

## 1. Übersicht und Konventionen

### 1.1 Parameter-Pfad-Format

```
[präfix].[unterpräfix].[parameter]

audio.gain                  → AudioSourceModule.gain
audio.smooth.timeMs         → AudioSourceModule → SmoothingModule → timeMs
shape.color.mode            → PulsingVisualizer → ColorGradientModule → mode
waveform.monoColor.preset   → WaveformModule → ColorGradient (Mono) → preset
```

### 1.2 Typen

| Typ | C++-Typ | UI-Widget (Default) |
|---|---|---|
| `Bool` | `bool` | QCheckBox |
| `Int` | `int` | QSpinBox (+ Slider) |
| `Float` | `float` | QDoubleSpinBox + QSlider |
| `String` | `std::string` | QLineEdit (bzw. Button bei `editGradient`) |
| `Enum` | `int` (Options-Index) | QComboBox |
| `Color` | `Color4f` = `std::array<float,4>` | Farb-Button + Dialog |

### 1.3 Spalten der Tabellen

- **Bereich**: `minValue`–`maxValue` (Schrittweite nur, wo explizit gesetzt; sonst 0.01).
- **Default**: der in `paramDescs()` deklarierte `defaultValue`.
- **SubGruppe**: `subGroup` des Deskriptors; die Hauptgruppe (`group`) steht je Abschnitt.
- **Sichtbar wenn** (👁️): `dependsOn` = einer der `dependsValues` (OR-Logik).
  IDs hier bereits **mit Visualizer-Präfix** notiert.
- 🔒 = `hidden` (nur Serialisierung) · ⚙ = `advanced`.

### 1.4 Bekannte Deklarations-Quirks

- **Preset-Dropdowns** (audio, smooth, Gradient): Index 0 ist immer `[Custom]`, danach
  die Builtins, danach ggf. `---` + User-Presets. Die *deklarierten* Defaults von
  `audio.preset` (0) und `audio.smooth.preset` (2) sind Enum-Werte der C++-Enums und
  passen dadurch **nicht** auf die Dropdown-Indizes (dort wäre 0 = `[Custom]`).
  Der Laufzeit-Startzustand ist korrekt: „Default" (Index 1) bzw. „Balanced" (Index 3).
- **Superscope**: die Defaults werden aus den *aktuellen Member-Werten* deklariert
  (`defaultValue = m_pointCount` etc.) — nach Parameteränderung meldet `paramDescs()`
  also den geänderten Wert als „Default". Untenstehende Werte sind die
  Member-Initialwerte.
- `solidColor` (ColorGradient) und `peakColor.fixed` (Equalizer) deklarieren **keinen**
  `defaultValue` (Variant bleibt default-konstruiert = `bool false`); der wirksame
  Startwert kommt aus dem Modul (Magenta) bzw. der Equalizer-Config.

---

## 2. Gemeinsame Module

### 2.1 AudioSourceModule (`audio.*`)

Eingebunden von **allen 5 Visualizern**, Gruppe „1. Audio". Quelle:
`include/visualizers/modules/source/AudioSourceModule.hpp` (paramDescs :518).
Der Equalizer filtert `audio.bands` heraus (ersetzt durch `eq.bands`, §7).

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `audio.preset` | Enum | [Custom], Default, Bass Heavy, Vocals, Electronic, Ambient, (User…) | Laufzeit: „Default" (s. §1.4) | — (order −1, immer oben) | — |
| `audio.scale` | Enum | Linear / Logarithmic / Mel | 1 (Log) | Mapping | — |
| `audio.bands` | Int | 8–512, Schritt 8 | 64 | Mapping | — |
| `audio.floorDb` | Float | −120–0, Schritt 1 [dB] | −60.0 | Normalization | — |
| `audio.ceilDb` | Float | −60–+20, Schritt 1 [dB] | 0.0 | Normalization | — |
| `audio.clamp01` | Bool | — | true | Normalization | — |
| `audio.gain` | Float | 0.1–5.0, Schritt 0.1 | 1.0 | Gain | — |

### 2.2 SmoothingModule (`audio.smooth.*`)

Eingebettet im AudioSourceModule (dort zwischen Mapping und Normalization einsortiert).
Quelle: `include/visualizers/modules/processing/SmoothingModule.hpp` (paramDescs :388).

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `audio.smooth.preset` | Enum | [Custom], Instant, Reactive, Balanced, Smooth, Sluggish, (User…) | Laufzeit: „Balanced" (s. §1.4) | Smoothing | — |
| `audio.smooth.algorithm` | Enum | None / SMA / EMA / WMA / DEMA | 2 (EMA) | Smoothing | — |
| `audio.smooth.timeMs` | Float | 0–500, Schritt 1 [ms] | 50.0 | Smoothing | `audio.smooth.algorithm` ∈ {2, 4} (EMA, DEMA) |
| `audio.smooth.windowSize` | Int | 2–60, Schritt 1 [samples] | 8 | Smoothing | `audio.smooth.algorithm` ∈ {1, 3} (SMA, WMA) |
| `audio.smooth.primeFirstFrame` ⚙ | Bool | — | true | Smoothing | `audio.smooth.algorithm` ∈ {1, 2, 3, 4} (≠ None) |

Builtin-Presets (Algorithm / timeMs): Instant = None/0 · Reactive = EMA/20 ·
Balanced = EMA/50 · Smooth = EMA/100 · Sluggish = DEMA/200.

### 2.3 ColorGradientModule (generisch)

Quelle: `src/visualizers/modules/ColorGradientModule.cpp` (paramDescs :44). Wird unter
je eigenem Präfix eingebunden (Tabelle unten). IDs hier **ohne** Präfix.

| ID | Typ | Bereich | Default | SubGruppe* | Sichtbar wenn |
|---|---|---|---|---|---|
| `mode` | Enum | Solid / Linear Gradient / Radial Gradient / Outline | 0 (Solid) | Color | — |
| `solidColor` | Color | RGBA 0–1 | nicht deklariert; Laufzeit Magenta {1,0,1,1} | Color | `mode` ∈ {0, 3} (Solid, Outline) |
| `angle` | Float | 0–360 [°] | 0.0 | Color | `mode` = 1 (Linear) |
| `preset` | Enum | [Custom], Fire, Forest, Galaxy, Ice, Lava, Monochrome, Neon, Ocean, Rainbow, Sunset, (User…) | 0 ([Custom]) | Color | `mode` ∈ {1, 2} (Linear, Radial) |
| `editGradient` | String (Button) | — | — | Color | `mode` ∈ {1, 2} |
| `outlineWidth` | Float | 1–15 [px] | 3.0 | Color | `mode` = 3 (Outline) |
| `gradientPresetName` 🔒 | String | — | — | — | — (Serialisierung) |
| `gradientData` 🔒 | String | `pos,r,g,b,a;…` | — | — | — (Serialisierung) |

\* SubGruppe „Color" wird von den meisten Einbindungen überschrieben (z. B.
„Line Color Mono").

**Einbindungen** (Instanzen im Ist-Stand):

| Visualizer | Präfix(e) | Anzahl |
|---|---|---|
| Pulsing | `shape.color.` | 1 |
| Waveform | `waveform.monoColor.` / `waveform.leftColor.` / `waveform.rightColor.` (Legacy-Alias `waveform.color.` → Mono) | 3 |
| Oscilloscope | `scope.ch1Color.` … `scope.ch4Color.`, `scope.m1Color.`, `scope.m2Color.` | 6 |
| Superscope | `scope.color.` | 1 |
| Equalizer | `color.` | 1 |

---

## 3. PulsingVisualizer (`pulsing`)

Quelle: `src/visualizers/PulsingVisualizer.cpp` (paramDescs :372–533).
Gruppen: „1. Audio" (= §2.1/2.2) · „2. Shape".

### 3.1 Gruppe „2. Shape"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `shape.type` | Enum | Circle / Ring / NGon / Star | 0 (Circle) | — |
| `shape.sides` | Int | 3–32 | 6 | `shape.type` ∈ {2, 3} (NGon, Star) |
| `shape.innerRadius` | Float | 0–0.95 | 0.5 | `shape.type` = 1 (Ring) |
| `shape.minSize` | Float | 0.05–1.5 | 0.3 | — |
| `shape.maxSize` | Float | 0.1–2.0 | 0.9 | — |
| `shape.rotation` | Float | −360–360 [°/s] | 0.0 | — |
| `shape.beatReverse` | Bool | — | false | — |

### 3.2 Farbe (SubGruppe „Color" unter „2. Shape")

`shape.color.*` = ColorGradientModule (§2.3), order 10+. Zusätzlicher Eigen-Parameter
(kein Gradient-Modul-Parameter, wird im Routing gesondert behandelt):

| ID | Typ | Default | Sichtbar wenn |
|---|---|---|---|
| `shape.color.beatBrightness` | Bool | true | — |

Hinweis: `PulseShapeModule` liefert selbst keine paramDescs — die Shape-Parameter
deklariert der Visualizer manuell und routet sie auf Modul-Setter/Member.

---

## 4. WaveformVisualizer (`waveform`)

Quellen: `src/visualizers/WaveformVisualizer.cpp` (paramDescs :306–342, Präfix
`waveform.`, order 100+) und `src/visualizers/modules/WaveformModule.cpp`
(paramDescs :178). Gruppen: „1. Audio" · „2. Waveform".

### 4.1 SubGruppe „Channel"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `waveform.channelMode` | Enum | Mono / Stereo / Both | 0 (Mono) | — |

### 4.2 SubGruppe „Layout"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `waveform.monoOffset` | Float | −1–1 | 0.0 | `channelMode` ∈ {0, 2} |
| `waveform.monoAmplitude` | Float | 0.1–2.0 | 0.8 | `channelMode` ∈ {0, 2} |
| `waveform.leftOffset` | Float | −1–1 | 0.5 | `channelMode` ∈ {1, 2} |
| `waveform.leftAmplitude` | Float | 0.1–2.0 | 0.4 | `channelMode` ∈ {1, 2} |
| `waveform.rightOffset` | Float | −1–1 | −0.5 | `channelMode` ∈ {1, 2} |
| `waveform.rightAmplitude` | Float | 0.1–2.0 | 0.4 | `channelMode` ∈ {1, 2} |
| `waveform.displayWidth` | Float | 0.1–1.0 | 1.0 | — |
| `waveform.sampleCount` | Int | 64–2048, Schritt 1 (Spinbox) | 512 | — |
| `waveform.smoothing` | Float | 0–0.95 | 0.3 | — |

(`channelMode` in den Sichtbarkeits-Spalten = `waveform.channelMode`; 0=Mono, 1=Stereo,
2=Both. `sampleCount` löst im Visualizer ein Puffer-Resize aus.)

### 4.3 SubGruppe „Line"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `waveform.lineStyle` | Enum | Line / Dots / Dashed | 0 (Line) | — |
| `waveform.monoLineWidth` | Float | 1–10 | 2.0 | `channelMode` ∈ {0, 2} |
| `waveform.leftLineWidth` | Float | 1–10 | 2.0 | `channelMode` ∈ {1, 2} |
| `waveform.rightLineWidth` | Float | 1–10 | 2.0 | `channelMode` ∈ {1, 2} |
| `waveform.dashLength` | Float | 2–50 [px] | 10.0 | `waveform.lineStyle` = 2 (Dashed) |
| `waveform.dashGap` | Float | 1–50 [px] | 5.0 | `waveform.lineStyle` = 2 (Dashed) |

### 4.4 SubGruppe „Fill" (je Kanal)

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `waveform.monoFillEnabled` | Bool | — | false | `channelMode` ∈ {0, 2} |
| `waveform.monoFillOpacity` | Float | 0–1 | 0.3 | `waveform.monoFillEnabled` = true |
| `waveform.monoFillBrightness` | Float | −1–1 | −0.3 | `waveform.monoFillEnabled` = true |
| `waveform.leftFillEnabled` | Bool | — | false | `channelMode` ∈ {1, 2} |
| `waveform.leftFillOpacity` | Float | 0–1 | 0.3 | `waveform.leftFillEnabled` = true |
| `waveform.leftFillBrightness` | Float | −1–1 | −0.3 | `waveform.leftFillEnabled` = true |
| `waveform.rightFillEnabled` | Bool | — | false | `channelMode` ∈ {1, 2} |
| `waveform.rightFillOpacity` | Float | 0–1 | 0.3 | `waveform.rightFillEnabled` = true |
| `waveform.rightFillBrightness` | Float | −1–1 | −0.3 | `waveform.rightFillEnabled` = true |

### 4.5 SubGruppe „Effects" (global)

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `waveform.mirrorEnabled` | Bool | — | false | — |
| `waveform.holdEnabled` | Bool | — | false | — |
| `waveform.fadeTime` | Float | 0.1–5.0 [s] | 1.0 | `waveform.holdEnabled` = true |
| `waveform.maxHoldFrames` | Int | 1–120, Schritt 1 (Spinbox) | 60 | `waveform.holdEnabled` = true |

### 4.6 Farb-SubGruppen (Channel-Mode-Pattern)

Je Kanal ein volles ColorGradientModule (§2.3):

| Präfix | SubGruppe | Order | Gruppen-Sichtbarkeit |
|---|---|---|---|
| `waveform.monoColor.*` | Line Color Mono | 200+ | `channelMode` ∈ {0, 2} |
| `waveform.leftColor.*` | Line Color Left | 300+ | `channelMode` ∈ {1, 2} |
| `waveform.rightColor.*` | Line Color Right | 400+ | `channelMode` ∈ {1, 2} |

Gradient-Parameter **ohne** eigenes `dependsOn` (`mode`, hidden-Params) erhalten die
Kanal-Bedingung; geschachtelte behalten ihre `mode`-Abhängigkeit (präfixiert, z. B.
`waveform.monoColor.solidColor` → `waveform.monoColor.mode` ∈ {0, 3}).

---

## 5. OscilloscopeVisualizer (`oscilloscope`)

Quellen: `src/visualizers/OscilloscopeVisualizer.cpp` (paramDescs :248–284, Präfix
`scope.`, order 100+) und `src/visualizers/modules/OscilloscopeModule.cpp`
(paramDescs :96). Gruppen: „1. Audio" · „2. Oscilloscope".

### 5.1 SubGruppe „Timebase"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `scope.timePerDiv` | Float | 0.1–100 [ms/Div] | 10.0 | — |
| `scope.sampleCount` | Int | 32–8192 | 512 | — |

(`sampleCount` löst im Visualizer ein Puffer-Resize aller Kanäle aus.)

### 5.2 SubGruppe „Trigger"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `scope.triggerEnabled` | Bool | — | true | — |
| `scope.triggerLevel` | Float | −1–1 | 0.0 | `scope.triggerEnabled` = true |
| `scope.triggerTolerance` | Float | 0–2 [Div] | 0.1 | `scope.triggerEnabled` = true |
| `scope.triggerPosition` | Float | 0–1 | 0.5 | `scope.triggerEnabled` = true |
| `scope.triggerEdge` | Enum | Rising / Falling / Both | 0 (Rising) | `scope.triggerEnabled` = true |
| `scope.triggerMode` | Enum | Auto / Normal / Single | 0 (Auto) | `scope.triggerEnabled` = true |
| `scope.triggerIndicator` | Enum | Arrows / Crosshair | 1 (Crosshair) | `scope.triggerEnabled` = true |
| `scope.triggerChannel` | Enum | CH1 / CH2 / CH3 / CH4 / M1 / M2 | 0 (CH1) | `scope.triggerEnabled` = true |
| `scope.triggerFadeTime` | Float | 0.1–10 [s] | 2.0 | `scope.triggerMode` ∈ {1, 2} (Normal, Single) |

### 5.3 Signal-Kanäle CH1–CH4 (SubGruppen „CH1"…„CH4")

Präfix `scope.chN.` (N = 1–4), order 100 + (N−1)·20. Alle Parameter außer `visible`
sind sichtbar wenn `scope.chN.visible` = true.

| ID (je Kanal) | Typ | Bereich | Default |
|---|---|---|---|
| `scope.chN.visible` | Bool | — | true nur für CH1, sonst false |
| `scope.chN.source` | Enum | Left / Right / Mono / Mid / Side | Kanalindex − 1 (CH1→Left, CH2→Right, CH3→Mono, CH4→Mid) |
| `scope.chN.mode` | Enum | Waveform / Envelope | 0 (Waveform) |
| `scope.chN.coupling` | Enum | DC / AC | 0 (DC) |
| `scope.chN.voltsPerDiv` | Float | 0.01–2.0 | 0.5 |
| `scope.chN.offset` | Float | −4–4 [Div] | 0.0 |
| `scope.chN.lineWidth` | Float | 1–5 | 2.0 |

### 5.4 Math-Kanäle M1–M2 (SubGruppen „M1"/„M2")

Präfix `scope.mN.` (N = 1–2), order 200 + (N−1)·20. Alle außer `visible` sichtbar wenn
`scope.mN.visible` = true.

| ID (je Kanal) | Typ | Bereich | Default |
|---|---|---|---|
| `scope.mN.visible` | Bool | — | false |
| `scope.mN.operation` | Enum | A + B / A − B / A × B / \|A\| / Rectify / −A / \|A − B\| | M1→0 (A+B), M2→1 (A−B) |
| `scope.mN.sourceA` | Enum | CH1 / CH2 / CH3 / CH4 | 0 (CH1) |
| `scope.mN.sourceB` | Enum | CH1 / CH2 / CH3 / CH4 | 1 (CH2) |
| `scope.mN.voltsPerDiv` | Float | 0.01–2.0 | 0.5 |
| `scope.mN.offset` | Float | −4–4 [Div] | 0.0 |
| `scope.mN.lineWidth` | Float | 1–5 | 2.0 |

### 5.5 SubGruppen „Grid" und „Display"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `scope.gridStyle` | Enum | None / Lines / Dots / Cross | 1 (Lines) | — |
| `scope.gridBrightness` | Float | 0–2 | 1.0 | — |
| `scope.gridLineWidth` | Float | 0.5–3 [px] | 1.0 | `scope.gridStyle` = 1 (Lines) |
| `scope.gridDotSize` | Float | 1–5 [px] | 2.0 | `scope.gridStyle` = 2 (Dots) |
| `scope.gridCrossSize` | Float | 2–10 [px] | 5.0 | `scope.gridStyle` = 3 (Cross) |
| `scope.interpolation` | Bool | — | true | — |

### 5.6 Farb-SubGruppen

Je Kanal ein volles ColorGradientModule (§2.3), order 500 + c·20:

| Präfix | SubGruppe | Gruppen-Sichtbarkeit |
|---|---|---|
| `scope.ch1Color.*` … `scope.ch4Color.*` | Line Color CH1 … CH4 | `scope.chN.visible` = true |
| `scope.m1Color.*`, `scope.m2Color.*` | Line Color M1, M2 | `scope.mN.visible` = true |

---

## 6. SuperscopeVisualizer (`superscope`)

Quellen: `src/visualizers/SuperscopeVisualizer.cpp` (paramDescs :121–159) und
`src/visualizers/modules/SuperscopeModule.cpp` (`paramDescs("scope.")` :774 — das Modul
präfixiert selbst). Gruppen: „1. Audio" · „2. Superscope". Defaults = Member-Initialwerte
(siehe Quirk §1.4).

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `scope.preset` | Enum | Custom, Horizontal Scope, Vertical Scope, Circle, Spiral, Lissajous, Flower, Star, Starburst, Heart, DNA, Spectrum Bars, Circular Spectrum, Butterfly, Hypocycloid | 4 (Spiral) | Preset | — |
| `scope.pointCount` | Int | 8–4096 | 256 | Render | — |
| `scope.renderMode` | Enum | Dots / Lines / Thick Lines | 1 (Lines) | Render | — |
| `scope.lineWidth` | Float | 1–20 | 2.0 | Render | `scope.renderMode` ∈ {1, 2} |
| `scope.dotSize` | Float | 1–50 | 4.0 | Render | `scope.renderMode` ∈ {0, 2} |
| `scope.blendMode` | Enum | Replace / Additive / Alpha | 1 (Additive) | Render | — |
| `scope.audioSource` | Enum | Waveform / Spectrum | 0 (Waveform) | Audio | — |
| `scope.audioChannel` | Enum | Left / Right / Mono / Mid / Side | 2 (Mono) | Audio | — |
| `scope.color.*` | (§2.3) | ColorGradientModule | — | Color | (mode-abhängig, präfixiert) |
| `scope.glowEnabled` | Bool | — | true | Glow | — |
| `scope.glowIntensity` | Float | 0–2 | 0.5 | Glow | `scope.glowEnabled` = true |
| `scope.glowSize` | Float | 1–10 | 2.0 | Glow | `scope.glowEnabled` = true |
| `scope.holdEnabled` | Bool | — | false | Hold/Fade | — |
| `scope.fadeTime` | Float | 0.1–10 [s] | 2.0 | Hold/Fade | `scope.holdEnabled` = true |
| `scope.maxHoldFrames` | Int | 1–60 | 20 | Hold/Fade | `scope.holdEnabled` = true |
| `scope.aspectCorrection` | Bool | — | true | Display | — |
| `scope.stretchX` | Float | 0.1–4.0 | 1.0 | Display | — |
| `scope.stretchY` | Float | 0.1–4.0 | 1.0 | Display | — |

Hinweis: „Audio" existiert doppelt — global als Gruppe „1. Audio" (`audio.*`) und als
SubGruppe „Audio" im Superscope (`scope.audioSource`/`scope.audioChannel` wählen die
Datenquelle der `v`-Variable).

---

## 7. EqualizerVisualizer (`equalizer`)

Quelle: `src/visualizers/EqualizerVisualizer.cpp` (paramDescs :194–693).
Gruppen „1."–„8."; `audio.*` **ohne** `audio.bands` (wird von `eq.bands` mitgeführt:
`setParam("eq.bands")` ruft auch `m_audioSource.setBands()`).

### 7.1 Gruppe „2. Equalizer"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `eq.bands` | Int | 8–256, Schritt 1 | 64 | — |
| `eq.barGap` | Float | 0–20 [px] | 2.0 | — |
| `eq.orientation` | Enum | Bottom Up / Top Down | 0 (Bottom Up) | — |

### 7.2 Gruppe „3. Color"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `color.domain` | Enum | Position / Amplitude / Time / Beat | 0 (Position) | — |
| `color.*` | (§2.3) | ColorGradientModule, order 10+ | — | (mode-abhängig, präfixiert) |

### 7.3 Gruppe „4. Peak Hold"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `peak.enabled` | Bool | — | true | — |
| `peak.holdDelay` | Float | 0–2000 [ms] | 120.0 | `peak.enabled` = true |
| `peak.gravity` | Float | −15–15 | 9.81 | `peak.enabled` = true |
| `peak.falloff` | Float | 0–20 | 0.5 | `peak.enabled` = true |
| `peak.bounce` | Float | 0–1 | 0.25 | `peak.enabled` = true |
| `peak.respawnOnLeave` | Bool | — | false | `peak.enabled` = true |
| `peak.behind` | Bool | — | false | `peak.enabled` = true |

### 7.4 Gruppe „5. Peak Thickness"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `thickness.mode` | Enum | Fixed / Direct (thicker at high) / Inverse (thicker at low) | 0 (Fixed) | `peak.enabled` = true |
| `thickness.base` | Float | 1–20 [px] | 2.0 | `peak.enabled` = true |
| `thickness.scale` | Float | 0–20 [px] | 4.0 | `thickness.mode` ∈ {1, 2} |

### 7.5 Gruppe „6. Spring Physics"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `spring.enabled` | Bool | — | false | `peak.enabled` = true |
| `spring.k` | Float | 1–200 | 40.0 | `spring.enabled` = true |
| `spring.damping` | Float | 0–50 | 10.0 | `spring.enabled` = true |
| `spring.useDelay` | Bool | — | true | `spring.enabled` = true |

### 7.6 Gruppe „7. Particles"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `particle.spawn` | Bool | — | false | `peak.enabled` = true |
| `particle.minDelta` | Float | 0–1 | 0.0 | `particle.spawn` = true |
| `particle.minInterval` | Float | 0–1000 [ms] | 0.0 | `particle.spawn` = true |
| `particle.maxPerBand` | Int | 1–32 | 8 | `particle.spawn` = true |
| `particle.freezeColor` | Bool | — | false | `particle.spawn` = true |
| `particle.bindToSpawner` | Bool | — | false | `particle.spawn` = true |

### 7.7 Gruppe „8. Peak Color"

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `peakColor.auto` | Bool | — | true | `peak.enabled` = true |
| `peakColor.fixed` | Color | RGBA 0–1 | nicht deklariert (s. §1.4) | `peakColor.auto` = false |
| `peakColor.freeze` | Bool | — | false | `peak.enabled` = true |

Hinweis: `EqualizerModule` besitzt zusätzlich ein eigenes, **nirgends aufgerufenes**
Parameter-System (paralleles totes System) — diese Referenz dokumentiert nur die
tatsächlich wirksamen Visualizer-Parameter. Siehe
[Visualizer_Architecture.md §10](Visualizer_Architecture.md).

---

## 8. Enum-Referenz

Alle Enums beginnen bei 0; `Enum`-Parameter speichern den int-Wert (= Options-Index,
außer bei den Preset-Dropdowns, siehe §1.4).

### 8.1 Parameter-System (`IModule.hpp`)

| Enum | Werte |
|---|---|
| `ParamType` | 0 Bool · 1 Int · 2 Float · 3 String · 4 Enum · 5 Vec2 · 6 Vec3 · 7 Vec4 · 8 Color |
| `ParamWidget` | 0 Default · 1 Slider · 2 Spinbox · 3 Checkbox · 4 Dropdown · 5 ColorPicker · 6 TextInput · 7 TextArea · 8 Knob · 9 Toggle · 10 ButtonGroup |

### 8.2 AudioSourceModule / SmoothingModule

| Enum | Werte |
|---|---|
| `FrequencyScale` | 0 Linear · 1 Log ⭐ · 2 Mel |
| `AudioPreset` (C++-Enum) | 0 Default ⭐ · 1 BassHeavy · 2 Vocals · 3 Electronic · 4 Ambient · 5 Custom |
| `SmoothingAlgorithm` | 0 None · 1 SMA · 2 EMA ⭐ · 3 WMA · 4 DEMA |
| `SmoothingPreset` (C++-Enum) | 0 Instant · 1 Reactive · 2 Balanced ⭐ · 3 Smooth · 4 Sluggish · 5 Custom |

⚠️ Die **Dropdown-Indizes** weichen von den C++-Enum-Werten ab, weil `[Custom]` im
Dropdown an Index 0 steht (Beispiel: „Balanced" = Enum 2, Dropdown 3).

### 8.3 ColorGradientModule / ColorSchemeModule

| Enum | Werte |
|---|---|
| `GradientMode` | 0 Solid ⭐ · 1 Linear · 2 Radial · 3 Outline |
| Gradient-Preset-Dropdown | 0 [Custom] · 1 Fire · 2 Forest · 3 Galaxy · 4 Ice · 5 Lava · 6 Monochrome · 7 Neon · 8 Ocean · 9 Rainbow · 10 Sunset · 11 `---` · 12+ User (alphabetisch) |
| `GradientDomain` (ColorSchemeModule; genutzt nur von `color.domain` des Equalizers) | 0 Position · 1 Amplitude · 2 Time · 3 Beat |

### 8.4 Visualizer-spezifische Enums (aus den enumOptions)

| Parameter | Werte |
|---|---|
| Pulsing `shape.type` | 0 Circle · 1 Ring · 2 NGon · 3 Star |
| Waveform `channelMode` | 0 Mono · 1 Stereo · 2 Both |
| Waveform `lineStyle` | 0 Line · 1 Dots · 2 Dashed |
| Scope `triggerEdge` | 0 Rising · 1 Falling · 2 Both |
| Scope `triggerMode` | 0 Auto · 1 Normal · 2 Single |
| Scope `triggerIndicator` | 0 Arrows · 1 Crosshair |
| Scope `chN.source` (`SignalSource`) | 0 Left · 1 Right · 2 Mono · 3 Mid · 4 Side |
| Scope `chN.mode` (`SignalMode`) | 0 Waveform · 1 Envelope |
| Scope `chN.coupling` (`CouplingMode`) | 0 DC · 1 AC |
| Scope `mN.operation` (`MathOperation`) | 0 A+B · 1 A−B · 2 A×B · 3 \|A\| · 4 Rectify · 5 −A · 6 \|A−B\| |
| Scope `gridStyle` (`GridStyle`) | 0 None · 1 Lines · 2 Dots · 3 Cross |
| Superscope `preset` (`SuperscopePreset`) | 0 Custom · 1 HorizontalScope · 2 VerticalScope · 3 Circle · 4 Spiral ⭐ · 5 Lissajous · 6 Flower · 7 Star · 8 Starburst · 9 Heart · 10 DNA · 11 SpectrumBars · 12 CircularSpectrum · 13 Butterfly · 14 Hypocycloid |
| Superscope `renderMode` | 0 Dots · 1 Lines ⭐ · 2 ThickLines |
| Superscope `audioSource` | 0 Waveform ⭐ · 1 Spectrum |
| Superscope `audioChannel` | 0 Left · 1 Right · 2 Mono ⭐ · 3 Mid · 4 Side |
| Superscope `blendMode` | 0 Replace · 1 Additive ⭐ · 2 Alpha |
| Equalizer `eq.orientation` | 0 Bottom Up · 1 Top Down |
| Equalizer `thickness.mode` | 0 Fixed · 1 Direct · 2 Inverse |

⭐ = Default.

---

## 9. Korrekturen gegenüber der Alt-Doku

Die Alt-Referenzen (Stand 2026-01-02) dokumentierten nur Pulsing und sind in mehreren
Punkten vom Code überholt worden:

1. **ParamValue/ParamType:** siehe [Visualizer_Architecture.md §4](Visualizer_Architecture.md)
   — beide Alt-Varianten waren falsch; `ParamType`-Reihenfolge ist `Bool, Int, Float,
   String, Enum, Vec2, Vec3, Vec4, Color` (kein „Button"-Typ).
2. **Audio-Presets:** heute 5 Builtins (Default, Bass Heavy, Vocals, Electronic,
   Ambient) statt nur „Default".
3. **`audio.smooth.timeMs`:** Bereich 0–500 ms (Alt: 1–500).
4. **GradientMode:** Default ist **Solid** (Alt: Linear); Modus **Outline** (3) ist
   hinzugekommen; `outlineWidth`-Default ist 3.0 (Alt: 2.0); Startfarbe Magenta
   (Alt: Weiß/„Fire").
5. **Gradient-Builtins:** 10 statt 7 (neu: Galaxy, Ice, Lava); Dropdown-Reihenfolge ist
   alphabetisch, nicht Fire-first; Preset-Default ist `[Custom]` (0).
6. **PulseShape:** UI kennt nur Circle/Ring/NGon/Star (Alt-Doku listete zusätzlich
   Wave/Flash); `shape.sides` geht bis 32 (Alt: 12); `shape.innerRadius` bis 0.95
   (Alt: 1.0). Neu gegenüber Alt: `shape.minSize`, `shape.maxSize`, `shape.rotation`,
   `shape.beatReverse`, `shape.color.beatBrightness`.
7. **Waveform/Oscilloscope/Superscope/Equalizer:** in der Alt-Referenz komplett
   undokumentiert — §§4–7 sind neu aus dem Code erhoben.

**Offene Punkte / Unsicherheiten:**

- Deklarierte Preset-Defaults vs. Dropdown-Indizes (§1.4) — Verhalten dokumentiert,
  Bereinigung ist Phase-4-Kandidat.
- Superscope-Defaults sind zustandsabhängig deklariert (§1.4) — die Tabelle nennt die
  Member-Initialwerte.
- `solidColor`/`peakColor.fixed` ohne deklarierten Default (§1.4).
- `audio.preset`-Dropdown: User-Presets folgen nach einem `---`-Separator; die
  Alt-Doku-Angabe „Index 3+" gilt nicht mehr allgemein (Index hängt von der Anzahl
  Builtins ab: Audio 7+, Smoothing 7+, Gradient 12+).

---

## 10. Siehe auch

- [Visualizer_Architecture.md](Visualizer_Architecture.md) — Parameter-System-Mechanik,
  Routing, Altlasten
- [OpenGL_Context_Handling.md](OpenGL_Context_Handling.md) — Context-Tracking-Pattern
- [../presets/FileFormat_Reference.md](../presets/FileFormat_Reference.md) — Dateiformate
  (.lvp/.grad/.audio/.smooth)
- Quell-Dateien: `src/visualizers/*Visualizer.cpp`, `src/visualizers/modules/*.cpp`,
  `include/visualizers/modules/**` (Zeilenangaben je Abschnitt)

---

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| **1.0.0** | **2026-07-18** | **Initial: Zusammenführung Parameter_Reference + Enum_Reference (harvest), erweitert um alle 5 Visualizer aus dem Code; Korrekturen §9** |
