# Parameter-Referenz — Alle Parameter der 5 Visualizer (SSOT für Phase 4)

> **Version:** 2.0.0
> **Datum:** 2026-07-19
> **Typ:** Reference
> **Status:** Aktiv
> **Sprache:** Deutsch

Vollständige Referenz aller Parameter-IDs im **Pipeline-Schema** (Phase 4, Schritt 5),
**aus dem Code erhoben** (Stand 2026-07-19: `paramDescs()`-Implementierungen der
Visualizer und Module). Alle 5 Visualizer sind auf die Stufen-Key-Konvention
(`audio.` / `map.` / `color.<handle>.` / `render.` / `peak.`+`particle.` / `post.`)
migriert; die verbindlichen Alt→Neu-Tabellen stehen in
[Parameter_Key_Migration.md](Parameter_Key_Migration.md) (Kurzübersicht: [§9](#9-alias-tabellen-alt--neu)).

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
9. [Alias-Tabellen (Alt → Neu)](#9-alias-tabellen-alt--neu)
10. [Korrekturen gegenüber der Alt-Doku](#10-korrekturen-gegenüber-der-alt-doku)
11. [Siehe auch](#11-siehe-auch)

---

## 1. Übersicht und Konventionen

### 1.1 Pipeline-Stufen und Key-Format

Jeder Parameter-Key beginnt mit dem Präfix seiner Pipeline-Stufe
(`PipelineStage`-Enum in `include/visualizers/modules/IModule.hpp`; Präfix→Stage-
Zuordnung zentral in `include/visualizers/PipelineKeys.hpp`, `stageForKey()`):

| Stufe | Enum | Präfix | ConfigPanel-Gruppe |
|---|---|---|---|
| 1 | `AudioSource` | `audio.` (inkl. `audio.smooth.`) | „1. Audio / Analysis" |
| 2 | `Mapping` | `map.` | „2. Mapping" |
| 3 | `Color` | `color.<handle>.` | „3. Color" |
| 4 | `Render` | `render.` | „4. Rendering" |
| 5 | `PeakParticle` | `peak.` / `particle.` | „5. Peak / Particles" |
| 6 | `Post` | `post.` | „6. Post FX" |

Beispiele:

```
audio.smooth.timeMs    → Stufe 1 (AudioSourceModule → SmoothingModule)
map.sampleCount        → Stufe 2 (Mapping; Puffer-Resize-Kopplung)
color.mono.preset      → Stufe 3 (Gradient-Handle „mono")
render.heightScale     → Stufe 4 (Render, Equalizer — NEU, E1)
peak.spring.k          → Stufe 5 (Peak-Physik, Equalizer)
post.hold.fadeTime     → Stufe 6 (Hold/Fade-Trail)
```

Das ConfigPanel sortiert die Gruppen strikt nach `ModuleParamDesc::stage`
(Stage-Tabelle, Gruppen-Keys `stage:N`) — die UI folgt dem Datenfluss. Für
Waveform/Oscilloscope/Superscope entstehen die Keys über
Übersetzungstabellen Modul-Sub-ID ↔ Pipeline-Key im jeweiligen
Visualizer-cpp (`subIdKeyTable()`); Pulsing/Equalizer deklarieren direkt.

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
- **SubGruppe**: `subGroup` des Deskriptors; die Stufen-Gruppe ergibt sich aus dem Key (§1.1).
- **Sichtbar wenn** (👁️): `dependsOn` = einer der `dependsValues` (OR-Logik).
  IDs hier bereits als **vollständige Pipeline-Keys** notiert.
- 🔒 = `hidden` (nur Serialisierung) · ⚙ = `advanced`.

### 1.4 Bekannte Deklarations-Quirks

- **Preset-Dropdowns** (audio, smooth, Gradient): Index 0 ist immer `[Custom]`, danach
  die Builtins, danach ggf. `---` + User-Presets. Die *deklarierten* Defaults von
  `audio.preset` (0) und `audio.smooth.preset` (2) sind Enum-Werte der C++-Enums und
  passen dadurch **nicht** auf die Dropdown-Indizes (dort wäre 0 = `[Custom]`).
  Der Laufzeit-Startzustand ist korrekt: „Default" (Index 1) bzw. „Balanced" (Index 3).
- **Superscope**: die meisten Defaults werden aus den *aktuellen Member-Werten*
  deklariert (`defaultValue = m_pointCount` etc.) — nach Parameteränderung meldet
  `paramDescs()` also den geänderten Wert als „Default". Untenstehende Werte sind die
  Member-Initialwerte. Ausnahme seit Schritt 5.5: `render.preset` deklariert fix
  `Spiral` (4).
- `solidColor` (ColorGradient) und `peak.color.fixed` (Equalizer) deklarieren **keinen**
  `defaultValue` (Variant bleibt default-konstruiert = `bool false`); der wirksame
  Startwert kommt aus dem Modul (Magenta) bzw. der Equalizer-Config.

### 1.5 Preset-Format und Legacy-Keys

`CURRENT_FORMAT_VERSION = 2` (`include/visualizers/VisualizerPresetManager.hpp`).
Presets mit `formatVersion < 2` werden beim `applyPreset` durch die pro Visualizer
registrierte **Alias-Map** übersetzt (Key + optional Wert-Konverter, §9);
**gespeichert wird ausschließlich im neuen Schema**. Der float-für-int-Vertrag
des PresetManagers gilt unverändert.

---

## 2. Gemeinsame Module

### 2.1 AudioSourceModule (`audio.*`) — Stufe 1

Eingebunden von **allen 5 Visualizern**. Quelle:
`include/visualizers/modules/source/AudioSourceModule.hpp` (paramDescs).
Der Equalizer filtert `audio.bands` heraus (ersetzt durch `map.bands`, E2, §7).

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `audio.preset` | Enum | [Custom], Default, Bass Heavy, Vocals, Electronic, Ambient, (User…) | Laufzeit: „Default" (s. §1.4) | — (order −1, immer oben) | — |
| `audio.scale` | Enum | Linear / Logarithmic / Mel | 1 (Log) | Mapping | — |
| `audio.bands` | Int | 8–512, Schritt 8 | 64 | Mapping | — |
| `audio.floorDb` | Float | −120–0, Schritt 1 [dB] | −60.0 | Normalization | — |
| `audio.ceilDb` | Float | −60–+20, Schritt 1 [dB] | 0.0 | Normalization | — |
| `audio.clamp01` | Bool | — | true | Normalization | — |
| `audio.gain` | Float | 0.1–5.0, Schritt 0.1 | 1.0 | Gain | — |

`audio.gain` bleibt bei **allen** Visualizern in Stufe 1 (funktionale
AudioSource-Verstärkung vor Floor/Ceil; Entscheid E1 — es gibt kein `map.gain`).

### 2.2 SmoothingModule (`audio.smooth.*`) — Stufe 1

Eingebettet im AudioSourceModule (dort zwischen Mapping und Normalization einsortiert).
Quelle: `include/visualizers/modules/processing/SmoothingModule.hpp` (paramDescs).

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `audio.smooth.preset` | Enum | [Custom], Instant, Reactive, Balanced, Smooth, Sluggish, (User…) | Laufzeit: „Balanced" (s. §1.4) | Smoothing | — |
| `audio.smooth.algorithm` | Enum | None / SMA / EMA / WMA / DEMA | 2 (EMA) | Smoothing | — |
| `audio.smooth.timeMs` | Float | 0–500, Schritt 1 [ms] | 50.0 | Smoothing | `audio.smooth.algorithm` ∈ {2, 4} (EMA, DEMA) |
| `audio.smooth.windowSize` | Int | 2–60, Schritt 1 [samples] | 8 | Smoothing | `audio.smooth.algorithm` ∈ {1, 3} (SMA, WMA) |
| `audio.smooth.primeFirstFrame` ⚙ | Bool | — | true | Smoothing | `audio.smooth.algorithm` ∈ {1, 2, 3, 4} (≠ None) |

Builtin-Presets (Algorithm / timeMs): Instant = None/0 · Reactive = EMA/20 ·
Balanced = EMA/50 · Smooth = EMA/100 · Sluggish = DEMA/200.

Seit E3 steuert `audio.smooth.*` auch die **Display-Glättung** der
Sample-Visualizer: Waveform und Oscilloscope halten pro Anzeigekanal eigene
SmoothingModule-Instanzen, synchronisieren deren Config von der
AudioSource-Instanz und glätten per `processArrayPerIndex()` (per-Index-EMA).
Der frühere separate Skalar `waveform.smoothing` ist **ersatzlos entfallen** (§4/§9).

### 2.3 ColorGradientModule (generisch) — Stufe 3

Quelle: `src/visualizers/modules/ColorGradientModule.cpp` (paramDescs). Wird je
Einbindung unter einem **Gradient-Handle** `color.<handle>.` geführt (Tabelle
unten); die Visualizer exponieren ihre Handles über `IVisualizer::gradients()`.
IDs hier **ohne** Präfix.

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

\* SubGruppe wird von den Einbindungen teils überschrieben (z. B. „Line Color Mono").

**Gradient-Handles** (Instanzen im Ist-Stand):

| Visualizer | Handle(s) → Key-Präfix | Anzahl |
|---|---|---|
| Pulsing | `color.main.` | 1 |
| Waveform | `color.mono.` / `color.left.` / `color.right.` | 3 |
| Oscilloscope | `color.ch1.` … `color.ch4.`, `color.m1.`, `color.m2.` | 6 |
| Superscope | `color.main.` | 1 |
| Equalizer | `color.main.` | 1 |

(Der frühere Waveform-Legacy-Alias `waveform.color.*` existiert nur noch als
Alias-Map-Eintrag → `color.mono.*`, §9.)

---

## 3. PulsingVisualizer (`pulsing`)

Quellen: `src/visualizers/PulsingVisualizer.cpp` (paramDescs) und
`src/visualizers/modules/PulseShapeModule.cpp` (paramDescs — das Shape-Schema
liefert seit Schritt 5.2 das **Modul selbst**; der Visualizer präfixiert nur noch).
Stufen: 1 (§2.1/2.2) · 3 · 4.

### 3.1 Stufe 3 — Color (Handle `main`)

`color.main.*` = ColorGradientModule (§2.3), order 10+. Zusätzlicher Eigen-Parameter
(kein Gradient-Modul-Parameter, wird im Routing gesondert behandelt):

| ID | Typ | Default | Sichtbar wenn |
|---|---|---|---|
| `color.main.beatBrightness` | Bool | true | — |

### 3.2 Stufe 4 — Render

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `render.type` | Enum | Circle / Ring / NGon / Star | 0 (Circle) | — |
| `render.sides` | Int | 3–32, Schritt 1 | 6 | `render.type` ∈ {2, 3} (NGon, Star) |
| `render.innerRadius` | Float | 0–0.95 | 0.5 | `render.type` = 1 (Ring) |
| `render.minSize` | Float | 0.05–1.5 | 0.3 | — |
| `render.maxSize` | Float | 0.1–2.0 | 0.9 | — |
| `render.rotation` | Float | −360–360 [°/s] | 0.0 | — |
| `render.beatReverse` | Bool | — | false | — |

(`render.beatReverse` deklariert der Visualizer selbst, order 6 nach den
Modul-Parametern. Beat-Erkennung läuft über das geteilte `BeatModule`.)

---

## 4. WaveformVisualizer (`waveform`)

Quellen: `src/visualizers/WaveformVisualizer.cpp` (Übersetzungstabelle
`subIdKeyTable()` + paramDescs) und `src/visualizers/modules/WaveformModule.cpp`
(paramDescs — SubGruppen Channel/Layout/Line/Fill/Effects bleiben erhalten).
Stufen: 1 · 2 · 3 · 4 · 6.

**Entfallen (E3):** der frühere Skalar `waveform.smoothing` hat **keinen**
Nachfolge-Key — die Display-Glättung übernimmt das SmoothingModule
(`audio.smooth.*`, per-Index-EMA, §2.2). Alte Presets werden per Wert-Konverter
`timeMs ≈ −16.67/ln(s)` nach `audio.smooth.timeMs` übersetzt (§9).

### 4.1 Stufe 2 — Mapping

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `map.channelMode` | Enum | Mono / Stereo / Both | 0 (Mono) | Channel | — |
| `map.sampleCount` | Int | 64–2048, Schritt 1 (Spinbox) | 512 | Layout | — |

(`map.sampleCount` löst im Visualizer ein Puffer-Resize aus.
`map.channelMode` steuert die Sichtbarkeit vieler Render-/Color-Parameter; in
den Sichtbarkeits-Spalten unten kurz `channelMode`: 0=Mono, 1=Stereo, 2=Both.)

### 4.2 Stufe 3 — Color (Handles `mono`/`left`/`right`)

Je Kanal ein volles ColorGradientModule (§2.3):

| Handle | SubGruppe | Gruppen-Sichtbarkeit |
|---|---|---|
| `color.mono.*` | Line Color Mono | `map.channelMode` ∈ {0, 2} |
| `color.left.*` | Line Color Left | `map.channelMode` ∈ {1, 2} |
| `color.right.*` | Line Color Right | `map.channelMode` ∈ {1, 2} |

Gradient-Parameter **ohne** eigenes `dependsOn` (`mode`, hidden-Params) erhalten die
Kanal-Bedingung; geschachtelte behalten ihre `mode`-Abhängigkeit (präfixiert, z. B.
`color.mono.solidColor` → `color.mono.mode` ∈ {0, 3}).

### 4.3 Stufe 4 — Render

SubGruppe „Layout":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `render.mono.offset` | Float | −1–1 | 0.0 | `channelMode` ∈ {0, 2} |
| `render.mono.amplitude` | Float | 0.1–2.0 | 0.8 | `channelMode` ∈ {0, 2} |
| `render.left.offset` | Float | −1–1 | 0.5 | `channelMode` ∈ {1, 2} |
| `render.left.amplitude` | Float | 0.1–2.0 | 0.4 | `channelMode` ∈ {1, 2} |
| `render.right.offset` | Float | −1–1 | −0.5 | `channelMode` ∈ {1, 2} |
| `render.right.amplitude` | Float | 0.1–2.0 | 0.4 | `channelMode` ∈ {1, 2} |
| `render.displayWidth` | Float | 0.1–1.0 | 1.0 | — |

SubGruppe „Line":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `render.lineStyle` | Enum | Line / Dots / Dashed | 0 (Line) | — |
| `render.mono.lineWidth` | Float | 1–10 | 2.0 | `channelMode` ∈ {0, 2} |
| `render.left.lineWidth` | Float | 1–10 | 2.0 | `channelMode` ∈ {1, 2} |
| `render.right.lineWidth` | Float | 1–10 | 2.0 | `channelMode` ∈ {1, 2} |
| `render.dashLength` | Float | 2–50 [px] | 10.0 | `render.lineStyle` = 2 (Dashed) |
| `render.dashGap` | Float | 1–50 [px] | 5.0 | `render.lineStyle` = 2 (Dashed) |

SubGruppe „Fill" (je Kanal):

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `render.mono.fillEnabled` | Bool | — | false | `channelMode` ∈ {0, 2} |
| `render.mono.fillOpacity` | Float | 0–1 | 0.3 | `render.mono.fillEnabled` = true |
| `render.mono.fillBrightness` | Float | −1–1 | −0.3 | `render.mono.fillEnabled` = true |
| `render.left.fillEnabled` | Bool | — | false | `channelMode` ∈ {1, 2} |
| `render.left.fillOpacity` | Float | 0–1 | 0.3 | `render.left.fillEnabled` = true |
| `render.left.fillBrightness` | Float | −1–1 | −0.3 | `render.left.fillEnabled` = true |
| `render.right.fillEnabled` | Bool | — | false | `channelMode` ∈ {1, 2} |
| `render.right.fillOpacity` | Float | 0–1 | 0.3 | `render.right.fillEnabled` = true |
| `render.right.fillBrightness` | Float | −1–1 | −0.3 | `render.right.fillEnabled` = true |

### 4.4 Stufe 6 — Post FX

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `post.mirror.enabled` | Bool | — | false | — |
| `post.hold.enabled` | Bool | — | false | — |
| `post.hold.fadeTime` | Float | 0.1–5.0 [s] | 1.0 | `post.hold.enabled` = true |
| `post.hold.maxFrames` | Int | 1–120, Schritt 1 (Spinbox) | 60 | `post.hold.enabled` = true |

(Hold/Fade läuft über das geteilte `HoldFadeEffect` aus
`modules/postfx/PostFxModule.hpp` — 3 Instanzen, je Kanal eine.)

---

## 5. OscilloscopeVisualizer (`oscilloscope`)

Quellen: `src/visualizers/OscilloscopeVisualizer.cpp` (Übersetzungstabelle
`subIdKeyTable()` + paramDescs) und `src/visualizers/modules/OscilloscopeModule.cpp`
(paramDescs — SubGruppen Timebase/Trigger/CH1–CH4/M1–M2/Grid/Display bleiben
erhalten). Stufen: 1 · 2 · 3 · 4 · 6. Das frühere Phosphor-System war tot und
wurde entfernt (E7) — ein echter Phosphor-Effekt kommt später via PostFxModule.

### 5.1 Stufe 2 — Mapping

SubGruppe „Timebase":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `map.timePerDiv` | Float | 0.1–100 [ms/Div] | 10.0 | — |
| `map.sampleCount` | Int | 32–8192 | 512 | — |

(`map.sampleCount` löst im Visualizer ein Puffer-Resize **aller** Kanäle aus.)

SubGruppe „Trigger":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `map.trigger.enabled` | Bool | — | true | — |
| `map.trigger.level` | Float | −1–1 | 0.0 | `map.trigger.enabled` = true |
| `map.trigger.tolerance` | Float | 0–2 [Div] | 0.1 | `map.trigger.enabled` = true |
| `map.trigger.position` | Float | 0–1 | 0.5 | `map.trigger.enabled` = true |
| `map.trigger.edge` | Enum | Rising / Falling / Both | 0 (Rising) | `map.trigger.enabled` = true |
| `map.trigger.mode` | Enum | Auto / Normal / Single | 0 (Auto) | `map.trigger.enabled` = true |
| `map.trigger.channel` | Enum | CH1 / CH2 / CH3 / CH4 / M1 / M2 | 0 (CH1) | `map.trigger.enabled` = true |

Signal-Kanäle (Präfix `map.chN.`, N = 1–4; sichtbar wenn `render.chN.visible` = true):

| ID (je Kanal) | Typ | Bereich | Default |
|---|---|---|---|
| `map.chN.source` | Enum | Left / Right / Mono / Mid / Side | Kanalindex − 1 (CH1→Left, CH2→Right, CH3→Mono, CH4→Mid) |
| `map.chN.mode` | Enum | Waveform / Envelope | 0 (Waveform) |
| `map.chN.coupling` | Enum | DC / AC | 0 (DC) |

Math-Kanäle (Präfix `map.mN.`, N = 1–2; sichtbar wenn `render.mN.visible` = true):

| ID (je Kanal) | Typ | Bereich | Default |
|---|---|---|---|
| `map.mN.operation` | Enum | A + B / A − B / A × B / \|A\| / Rectify / −A / \|A − B\| | M1→0 (A+B), M2→1 (A−B) |
| `map.mN.sourceA` | Enum | CH1 / CH2 / CH3 / CH4 | 0 (CH1) |
| `map.mN.sourceB` | Enum | CH1 / CH2 / CH3 / CH4 | 1 (CH2) |

### 5.2 Stufe 3 — Color (Handles `ch1`–`ch4`, `m1`, `m2`)

Je Kanal ein volles ColorGradientModule (§2.3):

| Handle | SubGruppe | Gruppen-Sichtbarkeit |
|---|---|---|
| `color.ch1.*` … `color.ch4.*` | Line Color CH1 … CH4 | `render.chN.visible` = true |
| `color.m1.*`, `color.m2.*` | Line Color M1, M2 | `render.mN.visible` = true |

### 5.3 Stufe 4 — Render

Kanal-Anzeige (Präfix `render.chN.` / `render.mN.`; alle außer `visible`
sichtbar wenn `render.chN.visible` bzw. `render.mN.visible` = true):

| ID (je Kanal) | Typ | Bereich | Default |
|---|---|---|---|
| `render.chN.visible` | Bool | — | true nur für CH1, sonst false |
| `render.chN.voltsPerDiv` | Float | 0.01–2.0 | 0.5 |
| `render.chN.offset` | Float | −4–4 [Div] | 0.0 |
| `render.chN.lineWidth` | Float | 1–5 | 2.0 |
| `render.mN.visible` | Bool | — | false |
| `render.mN.voltsPerDiv` | Float | 0.01–2.0 | 0.5 |
| `render.mN.offset` | Float | −4–4 [Div] | 0.0 |
| `render.mN.lineWidth` | Float | 1–5 | 2.0 |

Trigger-Overlay, Grid und Display:

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `render.triggerIndicator` | Enum | Arrows / Crosshair | 1 (Crosshair) | `map.trigger.enabled` = true |
| `render.gridStyle` | Enum | None / Lines / Dots / Cross | 1 (Lines) | — |
| `render.gridBrightness` | Float | 0–2 | 1.0 | — |
| `render.gridLineWidth` | Float | 0.5–3 [px] | 1.0 | `render.gridStyle` = 1 (Lines) |
| `render.gridDotSize` | Float | 1–5 [px] | 2.0 | `render.gridStyle` = 2 (Dots) |
| `render.gridCrossSize` | Float | 2–10 [px] | 5.0 | `render.gridStyle` = 3 (Cross) |
| `render.interpolation` | Bool | — | true | — |

### 5.4 Stufe 6 — Post FX

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `post.trigger.fadeTime` | Float | 0.1–10 [s] | 2.0 | `map.trigger.mode` ∈ {1, 2} (Normal, Single) |

---

## 6. SuperscopeVisualizer (`superscope`)

Quellen: `src/visualizers/SuperscopeVisualizer.cpp` (Übersetzungstabelle
`subIdKeyTable()` + paramDescs) und `src/visualizers/modules/SuperscopeModule.cpp`
(`paramDescs(prefix)` — wird seit Schritt 5.5 mit **leerem** Präfix aufgerufen;
die Übersetzung auf Pipeline-Keys macht der Visualizer). Stufen: 1 · 2 · 3 · 4 · 6.
Defaults = Member-Initialwerte (Quirk §1.4). Die frühere Doppel-„Audio"-Gruppe
ist aufgelöst: die Datenquellen-Wahl liegt jetzt in Stufe 2 (`map.audioSource`).

### 6.1 Stufe 2 — Mapping

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `map.pointCount` | Int | 8–4096 | 256 | Render | — |
| `map.audioSource` | Enum | Waveform / Spectrum | 0 (Waveform) | Audio | — |
| `map.audioChannel` | Enum | Left / Right / Mono / Mid / Side | 2 (Mono) | Audio | — |

(`map.pointCount` löst das Puffer-Resize des Punkt-Generators aus. Die
SubGruppen-Namen stammen unverändert aus dem Modul.)

### 6.2 Stufe 3 — Color (Handle `main`)

`color.main.*` = ColorGradientModule (§2.3), SubGruppe „Color"
(mode-abhängige Sichtbarkeit, präfixiert).

### 6.3 Stufe 4 — Render

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `render.preset` | Enum | Custom, Horizontal Scope, Vertical Scope, Circle, Spiral, Lissajous, Flower, Star, Starburst, Heart, DNA, Spectrum Bars, Circular Spectrum, Butterfly, Hypocycloid | 4 (Spiral) | Preset | — |
| `render.mode` | Enum | Dots / Lines / Thick Lines | 1 (Lines) | Render | — |
| `render.lineWidth` | Float | 1–20 | 2.0 | Render | `render.mode` ∈ {1, 2} |
| `render.dotSize` | Float | 1–50 | 4.0 | Render | `render.mode` ∈ {0, 2} |
| `render.blendMode` | Enum | Replace / Additive / Alpha | 1 (Additive) | Render | — |
| `render.aspectCorrection` | Bool | — | true | Display | — |
| `render.stretchX` | Float | 0.1–4.0 | 1.0 | Display | — |
| `render.stretchY` | Float | 0.1–4.0 | 1.0 | Display | — |

(`render.preset` ist ein Preset-Dropdown-Key und bleibt in seiner Stufe —
Entscheid E6, Parameter_Key_Migration.md §7.3.)

### 6.4 Stufe 6 — Post FX

| ID | Typ | Bereich | Default | SubGruppe | Sichtbar wenn |
|---|---|---|---|---|---|
| `post.glow.enabled` | Bool | — | true | Glow | — |
| `post.glow.intensity` | Float | 0–2 | 0.5 | Glow | `post.glow.enabled` = true |
| `post.glow.size` | Float | 1–10 | 2.0 | Glow | `post.glow.enabled` = true |
| `post.hold.enabled` | Bool | — | false | Hold/Fade | — |
| `post.hold.fadeTime` | Float | 0.1–10 [s] | 2.0 | Hold/Fade | `post.hold.enabled` = true |
| `post.hold.maxFrames` | Int | 1–60 | 20 | Hold/Fade | `post.hold.enabled` = true |

(Hold/Fade läuft über `HoldFadeEffectT<std::vector<SuperscopePoint>>`; die
frühere handgestrickte EMA-Glättung ist durch das geteilte SmoothingModule
mit `processArrayPerIndex()` ersetzt, Beat-Erkennung durch das `BeatModule`.)

---

## 7. EqualizerVisualizer (`equalizer`)

Quelle: `src/visualizers/EqualizerVisualizer.cpp` (paramDescs — direkte
Deklaration je Stufe). Stufen: 1 · 2 · 3 · 4 · 5. `audio.*` **ohne**
`audio.bands` (ersetzt durch `map.bands`, E2 — der frühere
`eq.bands`↔`audio.bands`-Sync ist entfallen; `setParam("map.bands")` treibt
beide Module).

### 7.1 Stufe 2 — Mapping

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `map.bands` | Int | 8–256, Schritt 1 | 64 | — |
| `map.orientation` | Enum | Bottom Up / Top Down | 0 (Bottom Up) | — |

(`map.bands` löst das Band-Puffer-Resize + AudioSource-Sync aus.)

### 7.2 Stufe 3 — Color (Handle `main`)

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `color.main.domain` | Enum | Position / Amplitude / Time / Beat | 0 (Position) | — |
| `color.main.*` | (§2.3) | ColorGradientModule, order 10+ | — | (mode-abhängig, präfixiert) |

### 7.3 Stufe 4 — Render

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `render.heightScale` | Float | 0–4, Schritt 0.05 | 1.0 | — |
| `render.barGap` | Float | 0–20 [px] | 2.0 | — |

**`render.heightScale` ist NEU** (Entscheid E1, Schritt 5.1): Anzeige-Skalierung
der Balkenhöhen — Ersatz für den in Schritt 0 entfernten, wirkungslosen
EqualizerModule-internen `gain`. Kein Alt-Key, kein Alias.

### 7.4 Stufe 5 — Peak / Particles

SubGruppe „Peak Hold":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `peak.enabled` | Bool | — | true | — |
| `peak.holdDelay` | Float | 0–2000 [ms] | 120.0 | `peak.enabled` = true |
| `peak.gravity` | Float | −15–15 | 9.81 | `peak.enabled` = true |
| `peak.falloff` | Float | 0–20 | 0.5 | `peak.enabled` = true |
| `peak.bounce` | Float | 0–1 | 0.25 | `peak.enabled` = true |
| `peak.respawnOnLeave` | Bool | — | false | `peak.enabled` = true |
| `peak.behind` | Bool | — | false | `peak.enabled` = true |

SubGruppe „Thickness":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `peak.thickness.mode` | Enum | Fixed / Direct (thicker at high) / Inverse (thicker at low) | 0 (Fixed) | `peak.enabled` = true |
| `peak.thickness.base` | Float | 1–20 [px] | 2.0 | `peak.enabled` = true |
| `peak.thickness.scale` | Float | 0–20 [px] | 4.0 | `peak.thickness.mode` ∈ {1, 2} |

SubGruppe „Spring Physics":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `peak.spring.enabled` | Bool | — | false | `peak.enabled` = true |
| `peak.spring.k` | Float | 1–200 | 40.0 | `peak.spring.enabled` = true |
| `peak.spring.damping` | Float | 0–50 | 10.0 | `peak.spring.enabled` = true |
| `peak.spring.useDelay` | Bool | — | true | `peak.spring.enabled` = true |

SubGruppe „Particles":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `particle.spawn` | Bool | — | false | `peak.enabled` = true |
| `particle.minDelta` | Float | 0–1 | 0.0 | `particle.spawn` = true |
| `particle.minInterval` | Float | 0–1000 [ms] | 0.0 | `particle.spawn` = true |
| `particle.maxPerBand` | Int | 1–32 | 8 | `particle.spawn` = true |
| `particle.freezeColor` | Bool | — | false | `particle.spawn` = true |
| `particle.bindToSpawner` | Bool | — | false | `particle.spawn` = true |

SubGruppe „Peak Color":

| ID | Typ | Bereich | Default | Sichtbar wenn |
|---|---|---|---|---|
| `peak.color.auto` | Bool | — | true | `peak.enabled` = true |
| `peak.color.fixed` | Color | RGBA 0–1 | nicht deklariert (s. §1.4) | `peak.color.auto` = false |
| `peak.color.freeze` | Bool | — | false | `peak.enabled` = true |

Das frühere parallele (nirgends aufgerufene) Parameter-System des
`EqualizerModule` wurde in Phase 4 Schritt 0 **entfernt** — es gibt nur noch
die hier dokumentierten Visualizer-Parameter.

---

## 8. Enum-Referenz

Alle Enums beginnen bei 0; `Enum`-Parameter speichern den int-Wert (= Options-Index,
außer bei den Preset-Dropdowns, siehe §1.4).

### 8.1 Parameter-System (`IModule.hpp`)

| Enum | Werte |
|---|---|
| `ParamType` | 0 Bool · 1 Int · 2 Float · 3 String · 4 Enum · 5 Vec2 · 6 Vec3 · 7 Vec4 · 8 Color |
| `ParamWidget` | 0 Default · 1 Slider · 2 Spinbox · 3 Checkbox · 4 Dropdown · 5 ColorPicker · 6 TextInput · 7 TextArea · 8 Knob · 9 Toggle · 10 ButtonGroup |
| `PipelineStage` | 0 None · 1 AudioSource · 2 Mapping · 3 Color · 4 Render · 5 PeakParticle · 6 Post |

### 8.2 AudioSourceModule / SmoothingModule

| Enum | Werte |
|---|---|
| `FrequencyScale` | 0 Linear · 1 Log ⭐ · 2 Mel |
| `AudioPreset` (C++-Enum) | 0 Default ⭐ · 1 BassHeavy · 2 Vocals · 3 Electronic · 4 Ambient · 5 Custom |
| `SmoothingAlgorithm` | 0 None · 1 SMA · 2 EMA ⭐ · 3 WMA · 4 DEMA |
| `SmoothingPreset` (C++-Enum) | 0 Instant · 1 Reactive · 2 Balanced ⭐ · 3 Smooth · 4 Sluggish · 5 Custom |

⚠️ Die **Dropdown-Indizes** weichen von den C++-Enum-Werten ab, weil `[Custom]` im
Dropdown an Index 0 steht (Beispiel: „Balanced" = Enum 2, Dropdown 3).

### 8.3 ColorGradientModule

| Enum | Werte |
|---|---|
| `GradientMode` | 0 Solid ⭐ · 1 Linear · 2 Radial · 3 Outline |
| Gradient-Preset-Dropdown | 0 [Custom] · 1 Fire · 2 Forest · 3 Galaxy · 4 Ice · 5 Lava · 6 Monochrome · 7 Neon · 8 Ocean · 9 Rainbow · 10 Sunset · 11 `---` · 12+ User (alphabetisch) |
| `GradientDomain` (seit Schritt 0 in `ColorGradientModule.hpp`; genutzt von `color.main.domain` des Equalizers) | 0 Position · 1 Amplitude · 2 Time · 3 Beat |

### 8.4 Visualizer-spezifische Enums (aus den enumOptions)

| Parameter | Werte |
|---|---|
| Pulsing `render.type` | 0 Circle · 1 Ring · 2 NGon · 3 Star |
| Waveform `map.channelMode` | 0 Mono · 1 Stereo · 2 Both |
| Waveform `render.lineStyle` | 0 Line · 1 Dots · 2 Dashed |
| Scope `map.trigger.edge` | 0 Rising · 1 Falling · 2 Both |
| Scope `map.trigger.mode` | 0 Auto · 1 Normal · 2 Single |
| Scope `render.triggerIndicator` | 0 Arrows · 1 Crosshair |
| Scope `map.chN.source` (`SignalSource`) | 0 Left · 1 Right · 2 Mono · 3 Mid · 4 Side |
| Scope `map.chN.mode` (`SignalMode`) | 0 Waveform · 1 Envelope |
| Scope `map.chN.coupling` (`CouplingMode`) | 0 DC · 1 AC |
| Scope `map.mN.operation` (`MathOperation`) | 0 A+B · 1 A−B · 2 A×B · 3 \|A\| · 4 Rectify · 5 −A · 6 \|A−B\| |
| Scope `render.gridStyle` (`GridStyle`) | 0 None · 1 Lines · 2 Dots · 3 Cross |
| Superscope `render.preset` (`SuperscopePreset`) | 0 Custom · 1 HorizontalScope · 2 VerticalScope · 3 Circle · 4 Spiral ⭐ · 5 Lissajous · 6 Flower · 7 Star · 8 Starburst · 9 Heart · 10 DNA · 11 SpectrumBars · 12 CircularSpectrum · 13 Butterfly · 14 Hypocycloid |
| Superscope `render.mode` | 0 Dots · 1 Lines ⭐ · 2 ThickLines |
| Superscope `map.audioSource` | 0 Waveform ⭐ · 1 Spectrum |
| Superscope `map.audioChannel` | 0 Left · 1 Right · 2 Mono ⭐ · 3 Mid · 4 Side |
| Superscope `render.blendMode` | 0 Replace · 1 Additive ⭐ · 2 Alpha |
| Equalizer `map.orientation` | 0 Bottom Up · 1 Top Down |
| Equalizer `peak.thickness.mode` | 0 Fixed · 1 Direct · 2 Inverse |

⭐ = Default.

---

## 9. Alias-Tabellen (Alt → Neu)

Die **verbindlichen** Alt→Neu-Migrationstabellen stehen in
[Parameter_Key_Migration.md](Parameter_Key_Migration.md) (Review E1–E8
eingearbeitet) — sie werden hier **nicht dupliziert**:

| Visualizer | Tabelle | Alias-Registrierung im Code |
|---|---|---|
| Pulsing | [Parameter_Key_Migration.md §2](Parameter_Key_Migration.md) | `src/visualizers/PulsingVisualizer.cpp` (`registerLegacyKeyAliases`) |
| Waveform | [§3](Parameter_Key_Migration.md) | `src/visualizers/WaveformVisualizer.cpp` (dito, inkl. E3-Wert-Konverter) |
| Oscilloscope | [§4](Parameter_Key_Migration.md) | `src/visualizers/OscilloscopeVisualizer.cpp` |
| Superscope | [§5](Parameter_Key_Migration.md) | `src/visualizers/SuperscopeVisualizer.cpp` |
| Equalizer | [§6](Parameter_Key_Migration.md) | `src/visualizers/EqualizerVisualizer.cpp` |

Mechanik: Presets mit `formatVersion < 2` werden beim Laden Key-weise durch die
Alias-Map des jeweiligen Visualizers übersetzt (unveränderte Keys stehen als
Identitätseinträge mit drin — die Map ist zugleich die Key-Whitelist des
Loaders); gespeichert wird nur im neuen Schema (`formatVersion = 2`).

**Kurzübersicht der Sonderfälle:**

- **`map.bands` (E2, nur Equalizer):** ersetzt das alte Paar
  `eq.bands` ↔ `audio.bands`; die Equalizer-Alias-Map übersetzt **beide**
  Alt-Keys auf `map.bands`. Bei allen anderen Visualizern bleibt `audio.bands`
  unverändert in Stufe 1.
- **`waveform.smoothing` (E3):** ersatzlos entfallen — kein Nachfolge-Key.
  Der registrierte **Wert-Konverter** übersetzt Alt-Presets nach
  `audio.smooth.timeMs` mit `timeMs ≈ −16.67/ln(s)` (EMA-Beziehung,
  60-FPS-Annahme; s ≤ 0 → 0 ms, s ≥ 1 geclampt).
- **`waveform.color.*` (Legacy):** alter Mono-Alias, wird direkt auf
  `color.mono.*` übersetzt (kein eigener Soll-Key).
- **Doppelter Alt-Präfix `scope.`:** Superscope und Oscilloscope teilten sich
  `scope.` — die Alias-Maps sind **strikt pro Visualizer** registriert, nie
  global. Preset-Dropdown-Keys bleiben in ihrer Stufe (`render.preset` beim
  Superscope, E6).
- **`render.heightScale` (E1, Equalizer):** neuer Parameter ohne Alt-Key —
  kein Alias-Eintrag.

---

## 10. Korrekturen gegenüber der Alt-Doku

Die Alt-Referenzen (Stand 2026-01-02) dokumentierten nur Pulsing und sind in mehreren
Punkten vom Code überholt worden (Keys hier in **Alt-Schreibweise** von vor der
Key-Migration; historische Gegenüberstellung aus v1.0.0):

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
   undokumentiert — §§4–7 sind aus dem Code erhoben.

**Offene Punkte / Unsicherheiten:**

- Deklarierte Preset-Defaults vs. Dropdown-Indizes (§1.4) — Verhalten unverändert,
  Bereinigung weiterhin offen (von der Key-Migration unabhängig,
  Parameter_Key_Migration.md §7.3).
- Superscope-Defaults sind (bis auf `render.preset`) zustandsabhängig deklariert
  (§1.4) — die Tabellen nennen die Member-Initialwerte.
- `solidColor`/`peak.color.fixed` ohne deklarierten Default (§1.4).
- `audio.preset`-Dropdown: User-Presets folgen nach einem `---`-Separator
  (Index hängt von der Anzahl Builtins ab: Audio 7+, Smoothing 7+, Gradient 12+).

---

## 11. Siehe auch

- [Visualizer_Architecture.md](Visualizer_Architecture.md) — Parameter-System-Mechanik,
  Stage-Schema, Routing, Tap-Points/Previews
- [Parameter_Key_Migration.md](Parameter_Key_Migration.md) — verbindliche
  Alt→Neu-Tabellen + Review-Entscheide E1–E8
- [OpenGL_Context_Handling.md](OpenGL_Context_Handling.md) — Context-Tracking-Pattern
- [../presets/FileFormat_Reference.md](../presets/FileFormat_Reference.md) — Dateiformate
  (.lvp/.grad/.audio/.smooth), `formatVersion`
- Quell-Dateien: `src/visualizers/*Visualizer.cpp`, `src/visualizers/modules/*.cpp`,
  `include/visualizers/modules/**`, `include/visualizers/PipelineKeys.hpp`

---

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| **2.0.0** | **2026-07-19** | **Pipeline-Schema (Phase 4 Schritt 5): alle Key-Tabellen auf die Stufen-Keys 1–6 umgestellt (Übersetzungstabellen/Alias-Maps im Code verifiziert); Gradient-Handles statt Präfix-Tabelle; `render.heightScale` (E1) ergänzt; `waveform.smoothing` entfernt (E3, Wert-Konverter dokumentiert); neuer §9 „Alias-Tabellen (Alt → Neu)"; Preset-Format v2 (§1.5); Enum-Referenz auf neue Key-Namen + `PipelineStage`** |
| 1.0.0 | 2026-07-18 | Initial: Zusammenführung Parameter_Reference + Enum_Reference (harvest), erweitert um alle 5 Visualizer aus dem Code; Korrekturen §10 (vormals §9) |
