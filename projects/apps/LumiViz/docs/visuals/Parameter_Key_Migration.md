# Parameter-Key-Migration — Alt → Neu (Phase 4, Schritt 3.3)

> **Version:** 0.2.0
> **Datum:** 2026-07-19
> **Typ:** Reference
> **Status:** Aktiv (Review abgeschlossen 2026-07-19)
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) v1.0.0 (§4.2 PipelineStage, §4.3 Key-Konvention, §4.4 Alias-Map) · [Parameter_Reference.md](Parameter_Reference.md) v2.0.0 (SSOT, neues Schema — dieses Dokument ist die Alt→Neu-Übersetzung)
> **Sprache:** Deutsch

Vollständige Migrationstabellen aller Parameter-Keys der 5 Visualizer vom Ist-Schema
(Parameter_Reference.md) auf die Soll-Key-Konvention der Config-Pipeline
(Config_Pipeline_Concept.md §4.3). Diese Tabellen sind die **Quelle der Alias-Maps**
für Schritt 5 (Preset-Migration) und die Arbeitsgrundlage der Visualizer-Migration
(Schritt 6). Die im Entwurf (v0.1.0) offenen Review-Fragen sind entschieden —
Entscheide in [§8](#8-review-entscheide-2026-07-19).

---

## Inhaltsverzeichnis

1. [Konventionen und Regeln](#1-konventionen-und-regeln)
2. [PulsingVisualizer](#2-pulsingvisualizer)
3. [WaveformVisualizer](#3-waveformvisualizer)
4. [OscilloscopeVisualizer](#4-oscilloscopevisualizer)
5. [SuperscopeVisualizer](#5-superscopevisualizer)
6. [EqualizerVisualizer](#6-equalizervisualizer)
7. [Sonderfälle](#7-sonderfälle)
8. [Review-Entscheide (2026-07-19)](#8-review-entscheide-2026-07-19)
9. [Alias-Map-Hinweis](#9-alias-map-hinweis)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

## 1. Konventionen und Regeln

### 1.1 Stufen (PipelineStage, Konzept §4.2)

| Stufe | Enum | Präfix |
|---|---|---|
| 1 | AudioSource | `audio.` (inkl. `audio.smooth.`) |
| 2 | Mapping | `map.` |
| 3 | Color | `color.<handle>.` |
| 4 | Render | `render.` |
| 5 | PeakParticle | `peak.` / `particle.` |
| 6 | Post | `post.` |

### 1.2 Unverändert: `audio.*` und `audio.smooth.*`

Alle Keys aus Parameter_Reference §2.1/§2.2 bleiben **unverändert** (Stufe 1) und
erscheinen unten je Visualizer nur als **Sammelzeile**:
`audio.preset`, `audio.scale`, `audio.bands`, `audio.floorDb`, `audio.ceilDb`,
`audio.clamp01`, `audio.gain`, `audio.smooth.preset`, `audio.smooth.algorithm`,
`audio.smooth.timeMs`, `audio.smooth.windowSize`, `audio.smooth.primeFirstFrame`.
Es gibt **kein** `map.gain` (Entscheid E1); `audio.gain` bleibt bei ALLEN
Visualizern unverändert in Stufe 1 — auch beim Equalizer (funktionale
AudioSource-Verstärkung vor Floor/Ceil). Eine **Equalizer-spezifische Ausnahme**
(Entscheid E2, Details §6/§7.1): dort wird `audio.bands` durch `map.bands`
ersetzt. `render.heightScale` (E1) ist ein **neuer** Parameter ohne Alt-Key (§6).

### 1.3 Gradient-Sub-Keys (ein Muster je Farb-Handle)

Jede ColorGradientModule-Einbindung (Parameter_Reference §2.3) trägt dieselben
**8 Sub-Keys**; sie migrieren als Block mit dem Handle-Präfix mit:

```
mode · solidColor · angle · preset · editGradient · outlineWidth
gradientPresetName 🔒 · gradientData 🔒
```

Eine Tabellenzeile `alt.*` → `color.<handle>.*` steht für genau diese 8 Keys.
`color.<handle>.preset` ist ein Preset-Dropdown-Key und **bleibt in Stufe 3**
(Regel §7.3).

### 1.4 Zählweise

„Jeder Key genau einmal": Muster-Zeilen (`*` = 8 Gradient-Sub-Keys; `chN`/`mN` =
Kanal-Expansion wie in Parameter_Reference §5.3/§5.4) decken ihre Expansion
vollständig ab. Die Summenzeile am Ende jedes Abschnitts nennt die expandierte
Key-Anzahl (ohne die unveränderten `audio.*`-Keys).

---

## 2. PulsingVisualizer

| Alter Key | Neuer Key | Stufe (1–6) | Bemerkung |
|---|---|---|---|
| `audio.*` / `audio.smooth.*` | unverändert | 1 | Sammelzeile (§1.2) |
| `shape.type` | `render.type` | 4 | Grundform (Circle/Ring/NGon/Star) |
| `shape.sides` | `render.sides` | 4 | — |
| `shape.innerRadius` | `render.innerRadius` | 4 | — |
| `shape.minSize` | `render.minSize` | 4 | — |
| `shape.maxSize` | `render.maxSize` | 4 | — |
| `shape.rotation` | `render.rotation` | 4 | — |
| `shape.beatReverse` | `render.beatReverse` | 4 | Beat-getriebene Rotationsumkehr — bleibt Render (wirkt auf Geometrie) |
| `shape.color.*` | `color.main.*` | 3 | Gradient-Block (8 Sub-Keys, §1.3), Handle `main` |
| `shape.color.beatBrightness` | `color.main.beatBrightness` | 3 | Kein Gradient-Modul-Parameter (Eigen-Parameter mit Sonder-Routing, Parameter_Reference §3.2) — Routing bei Migration beibehalten |

**Summe Pulsing: 16 migrierte Keys** (8 direkt + 8 Gradient).

---

## 3. WaveformVisualizer

| Alter Key | Neuer Key | Stufe (1–6) | Bemerkung |
|---|---|---|---|
| `audio.*` / `audio.smooth.*` | unverändert | 1 | Sammelzeile (§1.2) |
| `waveform.channelMode` | `map.channelMode` | 2 | Kanalwahl (Mono/Stereo/Both) → Mapping; steuert Sichtbarkeit vieler Render-/Color-Gruppen — dependsOn-IDs bei Migration mitziehen |
| `waveform.sampleCount` | `map.sampleCount` | 2 | **Puffer-Resize-Kopplung** (§7.2) |
| `waveform.smoothing` | — (entfällt, Wert-Konverter → `audio.smooth.timeMs`) | 1 | **Entscheid E3 (Hybrid):** kein Nachfolge-Key — das SmoothingModule übernimmt (Konzept §4.5/5.3). Beim Laden alter Presets wird der Skalar s näherungsweise konvertiert: `timeMs ≈ −16.67 / ln(s)` (EMA-Beziehung, 60-FPS-Annahme; s = 0.9 ≈ 158 ms). Erfordert **Wert-Konverter** in der Alias-Mechanik (→ §9) |
| `waveform.monoOffset` | `render.mono.offset` | 4 | Layout; kanal-strukturiert (Entscheid E8, konsistent zu `color.mono.*`) |
| `waveform.monoAmplitude` | `render.mono.amplitude` | 4 | — |
| `waveform.leftOffset` | `render.left.offset` | 4 | — |
| `waveform.leftAmplitude` | `render.left.amplitude` | 4 | — |
| `waveform.rightOffset` | `render.right.offset` | 4 | — |
| `waveform.rightAmplitude` | `render.right.amplitude` | 4 | — |
| `waveform.displayWidth` | `render.displayWidth` | 4 | kanal-unabhängig → flach |
| `waveform.lineStyle` | `render.lineStyle` | 4 | — |
| `waveform.monoLineWidth` | `render.mono.lineWidth` | 4 | — |
| `waveform.leftLineWidth` | `render.left.lineWidth` | 4 | — |
| `waveform.rightLineWidth` | `render.right.lineWidth` | 4 | — |
| `waveform.dashLength` | `render.dashLength` | 4 | — |
| `waveform.dashGap` | `render.dashGap` | 4 | — |
| `waveform.monoFillEnabled` | `render.mono.fillEnabled` | 4 | — |
| `waveform.monoFillOpacity` | `render.mono.fillOpacity` | 4 | — |
| `waveform.monoFillBrightness` | `render.mono.fillBrightness` | 4 | — |
| `waveform.leftFillEnabled` | `render.left.fillEnabled` | 4 | — |
| `waveform.leftFillOpacity` | `render.left.fillOpacity` | 4 | — |
| `waveform.leftFillBrightness` | `render.left.fillBrightness` | 4 | — |
| `waveform.rightFillEnabled` | `render.right.fillEnabled` | 4 | — |
| `waveform.rightFillOpacity` | `render.right.fillOpacity` | 4 | — |
| `waveform.rightFillBrightness` | `render.right.fillBrightness` | 4 | — |
| `waveform.mirrorEnabled` | `post.mirror.enabled` | 6 | Post-Effekt Mirror |
| `waveform.holdEnabled` | `post.hold.enabled` | 6 | Hold/Fade → künftiges PostFxModule (Konzept §4.5) |
| `waveform.fadeTime` | `post.hold.fadeTime` | 6 | Konzept-§4.3-Beispiel |
| `waveform.maxHoldFrames` | `post.hold.maxFrames` | 6 | Umbenennung `maxHoldFrames` → `maxFrames` (Redundanz zum Präfix `hold.`) |
| `waveform.monoColor.*` | `color.mono.*` | 3 | Gradient-Block (8 Sub-Keys), Handle `mono` |
| `waveform.leftColor.*` | `color.left.*` | 3 | Gradient-Block (8 Sub-Keys), Handle `left` |
| `waveform.rightColor.*` | `color.right.*` | 3 | Gradient-Block (8 Sub-Keys), Handle `right` |
| `waveform.color.*` (Legacy-Alias) | `color.mono.*` | 3 | Nur Alias-Map-Eintrag — Ist-Code mappt `waveform.color.` bereits auf Mono (Parameter_Reference §2.3); kein eigener Soll-Key |

**Summe Waveform: 52 migrierte Keys** (28 direkt + 3 × 8 Gradient) **+ 1 wert-konvertierter Alt-Key** (`waveform.smoothing` → `audio.smooth.timeMs`, E3; Legacy-Alias `waveform.color.*` zählt nicht, da nur Lese-Übersetzung).

---

## 4. OscilloscopeVisualizer

| Alter Key | Neuer Key | Stufe (1–6) | Bemerkung |
|---|---|---|---|
| `audio.*` / `audio.smooth.*` | unverändert | 1 | Sammelzeile (§1.2) |
| `scope.timePerDiv` | `map.timePerDiv` | 2 | Zeitbasis |
| `scope.sampleCount` | `map.sampleCount` | 2 | **Puffer-Resize-Kopplung** aller Kanäle (§7.2) |
| `scope.triggerEnabled` | `map.trigger.enabled` | 2 | Trigger → Mapping |
| `scope.triggerLevel` | `map.trigger.level` | 2 | Konzept-§4.3-Beispiel |
| `scope.triggerTolerance` | `map.trigger.tolerance` | 2 | — |
| `scope.triggerPosition` | `map.trigger.position` | 2 | — |
| `scope.triggerEdge` | `map.trigger.edge` | 2 | — |
| `scope.triggerMode` | `map.trigger.mode` | 2 | — |
| `scope.triggerIndicator` | `render.triggerIndicator` | 4 | **Entscheid E4:** Arrows/Crosshair ist ein Anzeige-Overlay → Render (einzige Trigger-Ausnahme; Rest unter `map.trigger.*`) |
| `scope.triggerFadeTime` | `post.trigger.fadeTime` | 6 | Trigger-Fade ist Frame-Nachleuchten → Post. `scope.phosphor*` (Konzept §4.3) existiert nicht im Code — Alias-Map hat dafür nichts zu übersetzen (Entscheid E7) |
| `scope.chN.visible` (N=1–4) | `render.chN.visible` | 4 | Kanal-Sichtbarkeit (Anzeige) |
| `scope.chN.source` (N=1–4) | `map.chN.source` | 2 | Kanalwahl (Left/Right/Mono/Mid/Side) → Mapping |
| `scope.chN.mode` (N=1–4) | `map.chN.mode` | 2 | Waveform/Envelope = Datenaufbereitung |
| `scope.chN.coupling` (N=1–4) | `map.chN.coupling` | 2 | DC/AC = Signalaufbereitung |
| `scope.chN.voltsPerDiv` (N=1–4) | `render.chN.voltsPerDiv` | 4 | Div-Raster-Abbildung → Render (Entscheid E5) |
| `scope.chN.offset` (N=1–4) | `render.chN.offset` | 4 | Vertikale Position im Div-Raster |
| `scope.chN.lineWidth` (N=1–4) | `render.chN.lineWidth` | 4 | — |
| `scope.mN.visible` (N=1–2) | `render.mN.visible` | 4 | Math-Kanal-Sichtbarkeit |
| `scope.mN.operation` (N=1–2) | `map.mN.operation` | 2 | Datenableitung (A+B, A−B, …) → Mapping |
| `scope.mN.sourceA` (N=1–2) | `map.mN.sourceA` | 2 | — |
| `scope.mN.sourceB` (N=1–2) | `map.mN.sourceB` | 2 | — |
| `scope.mN.voltsPerDiv` (N=1–2) | `render.mN.voltsPerDiv` | 4 | wie `chN.voltsPerDiv` (Entscheid E5) |
| `scope.mN.offset` (N=1–2) | `render.mN.offset` | 4 | — |
| `scope.mN.lineWidth` (N=1–2) | `render.mN.lineWidth` | 4 | — |
| `scope.gridStyle` | `render.gridStyle` | 4 | — |
| `scope.gridBrightness` | `render.gridBrightness` | 4 | — |
| `scope.gridLineWidth` | `render.gridLineWidth` | 4 | — |
| `scope.gridDotSize` | `render.gridDotSize` | 4 | — |
| `scope.gridCrossSize` | `render.gridCrossSize` | 4 | — |
| `scope.interpolation` | `render.interpolation` | 4 | — |
| `scope.ch1Color.*` | `color.ch1.*` | 3 | Gradient-Block (8 Sub-Keys), Handle `ch1` |
| `scope.ch2Color.*` | `color.ch2.*` | 3 | Gradient-Block, Handle `ch2` |
| `scope.ch3Color.*` | `color.ch3.*` | 3 | Gradient-Block, Handle `ch3` |
| `scope.ch4Color.*` | `color.ch4.*` | 3 | Gradient-Block, Handle `ch4` |
| `scope.m1Color.*` | `color.m1.*` | 3 | Gradient-Block, Handle `m1` |
| `scope.m2Color.*` | `color.m2.*` | 3 | Gradient-Block, Handle `m2` |
| `scope.triggerChannel` | `map.trigger.channel` | 2 | Kanalwahl des Triggers (CH1…M2) |

**Summe Oscilloscope: 107 migrierte Keys** (59 direkt, expandiert: 2 Timebase + 9 Trigger (davon `triggerFadeTime` → Stufe 6, `triggerIndicator` → Stufe 4) + 4×7 CH + 2×7 Math + 6 Grid/Display; plus 6 × 8 Gradient).

---

## 5. SuperscopeVisualizer

| Alter Key | Neuer Key | Stufe (1–6) | Bemerkung |
|---|---|---|---|
| `audio.*` / `audio.smooth.*` | unverändert | 1 | Sammelzeile (§1.2) |
| `scope.preset` | `render.preset` | 4 | Shape-/Skript-Preset (Spiral, Lissajous, …) — Preset-Dropdown-Key, bleibt in seiner Stufe (§7.3); Stufe 4 bestätigt (Entscheid E6) |
| `scope.pointCount` | `map.pointCount` | 2 | Sample-Count-Analogon → Mapping („Sample-Counts → map"); **Puffer-Resize-Kopplung** (§7.2) |
| `scope.renderMode` | `render.mode` | 4 | Dots/Lines/Thick Lines; `renderMode` → `mode` (Redundanz zum Präfix) |
| `scope.lineWidth` | `render.lineWidth` | 4 | Konzept-§4.3-Beispiel |
| `scope.dotSize` | `render.dotSize` | 4 | — |
| `scope.blendMode` | `render.blendMode` | 4 | — |
| `scope.audioSource` | `map.audioSource` | 2 | Waveform/Spectrum — Datenquelle der `v`-Variable (Vorgabe: Kanalwahl → map). Löst die Doppel-„Audio"-Gruppe auf (Parameter_Reference §6, Hinweis) |
| `scope.audioChannel` | `map.audioChannel` | 2 | Left/Right/Mono/Mid/Side |
| `scope.color.*` | `color.main.*` | 3 | Gradient-Block (8 Sub-Keys), Handle `main` |
| `scope.glowEnabled` | `post.glow.enabled` | 6 | Glow → Post |
| `scope.glowIntensity` | `post.glow.intensity` | 6 | — |
| `scope.glowSize` | `post.glow.size` | 6 | — |
| `scope.holdEnabled` | `post.hold.enabled` | 6 | Hold/Fade → Post (gleiches Schema wie Waveform → künftiges PostFxModule) |
| `scope.fadeTime` | `post.hold.fadeTime` | 6 | — |
| `scope.maxHoldFrames` | `post.hold.maxFrames` | 6 | Umbenennung analog Waveform |
| `scope.aspectCorrection` | `render.aspectCorrection` | 4 | Display-Geometrie |
| `scope.stretchX` | `render.stretchX` | 4 | — |
| `scope.stretchY` | `render.stretchY` | 4 | — |

**Summe Superscope: 25 migrierte Keys** (17 direkt + 8 Gradient).

Achtung Alias-Map: Superscope und Oscilloscope teilen den **alten** Präfix `scope.` —
die Alias-Maps sind **je Visualizer** getrennt (Konzept §4.4), sonst kollidieren
z. B. `scope.lineWidth` (Superscope) und `scope.chN.lineWidth` (Oscilloscope) nicht,
aber `scope.sampleCount`/`scope.pointCount`-Verwechslungen wären möglich.

---

## 6. EqualizerVisualizer

| Alter Key | Neuer Key | Stufe (1–6) | Bemerkung |
|---|---|---|---|
| `audio.*` / `audio.smooth.*` | unverändert | 1 | Sammelzeile (§1.2) — inkl. `audio.gain` (funktionale AudioSource-Verstärkung, bleibt!); **ohne** `audio.bands` (eigene Zeile, Entscheid E2) |
| `eq.bands` (+ Alt-Sync `audio.bands`) | `map.bands` | 2 | **Entscheid E2 / Sonderfall §7.1:** ein Key ersetzt das `eq.bands`↔`audio.bands`-Paar, der Sync-Mechanismus entfällt; **Puffer-Resize-Kopplung** (§7.2) |
| — (kein Alt-Key) | `render.heightScale` | 4 | **Entscheid E1, NEUER Parameter (Schritt 5.1):** Anzeige-Skalierung der Balkenhöhen — Ersatz für den in Schritt 0 entfernten, wirkungslosen EqualizerModule-internen `gain` (hatte keinen erreichbaren Parameter-Key); kein Alias nötig |
| `eq.barGap` | `render.barGap` | 4 | Balken-Geometrie |
| `eq.orientation` | `map.orientation` | 2 | Bottom Up / Top Down — per Vorgabe „orientation → map"; wirkt de facto auf die Zeichenrichtung |
| `color.domain` | `color.main.domain` | 3 | Gradient-Domain (Position/Amplitude/Time/Beat) — Eigen-Parameter neben dem Gradient-Block |
| `color.*` | `color.main.*` | 3 | Gradient-Block (8 Sub-Keys), Handle `main` |
| `peak.enabled` | `peak.enabled` | 5 | unverändert (Ist-Key trägt bereits das Soll-Präfix) |
| `peak.holdDelay` | `peak.holdDelay` | 5 | unverändert |
| `peak.gravity` | `peak.gravity` | 5 | unverändert (Konzept-§4.3-Beispiel) |
| `peak.falloff` | `peak.falloff` | 5 | unverändert |
| `peak.bounce` | `peak.bounce` | 5 | unverändert |
| `peak.respawnOnLeave` | `peak.respawnOnLeave` | 5 | unverändert |
| `peak.behind` | `peak.behind` | 5 | unverändert |
| `thickness.mode` | `peak.thickness.mode` | 5 | Peak-Thickness unter `peak.` gezogen |
| `thickness.base` | `peak.thickness.base` | 5 | — |
| `thickness.scale` | `peak.thickness.scale` | 5 | — |
| `spring.enabled` | `peak.spring.enabled` | 5 | Spring-Physik gehört zum Peak-System |
| `spring.k` | `peak.spring.k` | 5 | — |
| `spring.damping` | `peak.spring.damping` | 5 | — |
| `spring.useDelay` | `peak.spring.useDelay` | 5 | — |
| `particle.spawn` | `particle.spawn` | 5 | unverändert |
| `particle.minDelta` | `particle.minDelta` | 5 | unverändert |
| `particle.minInterval` | `particle.minInterval` | 5 | unverändert |
| `particle.maxPerBand` | `particle.maxPerBand` | 5 | unverändert (Konzept-§4.3-Beispiel) |
| `particle.freezeColor` | `particle.freezeColor` | 5 | unverändert |
| `particle.bindToSpawner` | `particle.bindToSpawner` | 5 | unverändert |
| `peakColor.auto` | `peak.color.auto` | 5 | Peak-Farbe bleibt Stufe 5 (kein Gradient-Handle — an das Peak-System gebunden, kein ColorGradientModule) |
| `peakColor.fixed` | `peak.color.fixed` | 5 | ohne deklarierten Default (Parameter_Reference §1.4) — Verhalten bei Migration beibehalten |
| `peakColor.freeze` | `peak.color.freeze` | 5 | — |

**Summe Equalizer: 35 migrierte Keys** (27 direkt + 8 Gradient; `render.heightScale`
ist NEU ohne Alt-Key und zählt nicht als Migration); davon 13 unverändert (`peak.*` 7,
`particle.*` 6), aber in der Alias-Map als Identität mitgeführt (→ §9).

---

## 7. Sonderfälle

### 7.1 `eq.bands` ↔ `audio.bands` → ein Key `map.bands`

Der Equalizer filtert heute `audio.bands` aus der UI und führt es über
`setParam("eq.bands")` mit (`m_audioSource.setBands()`, Parameter_Reference §7).
**Entscheid E2 (kontextabhängig):** Im Soll-Schema gibt es beim Equalizer **einen**
Key `map.bands`, der Sync-Mechanismus entfällt. Die Alias-Map des Equalizers
übersetzt **beide** Alt-Keys (`eq.bands` UND `audio.bands`) auf `map.bands` —
bei widersprüchlichen Werten in einem Alt-Preset gewinnt `eq.bands` (der wirksame
UI-Key). Bei den übrigen Visualizern bleibt `audio.bands` unverändert in Stufe 1.

### 7.2 Puffer-Resize-Kopplungen

Diese Keys lösen beim Setzen ein Puffer-Resize aus — die Migration muss das
Routing (Setter-Seiteneffekt) erhalten, und die UI-Reihenfolge beim Preset-Laden
darf keine Zwischenzustände mit falscher Puffergröße rendern:

| Neuer Key | Visualizer | Wirkung |
|---|---|---|
| `map.bands` | Equalizer | Band-Puffer + AudioSource-Bands |
| `map.sampleCount` | Waveform | Sample-Puffer-Resize |
| `map.sampleCount` | Oscilloscope | Puffer-Resize **aller** Kanäle (CH1–CH4, M1–M2) |
| `map.pointCount` | Superscope | Punkt-Puffer des Generators |

### 7.3 Preset-Dropdown-Keys bleiben in ihrer Stufe

| Key (neu) | Stufe | Anmerkung |
|---|---|---|
| `audio.preset` | 1 | unverändert |
| `audio.smooth.preset` | 1 | unverändert |
| `color.<handle>.preset` | 3 | je Gradient-Handle |
| `render.preset` (Superscope) | 4 | Shape-/Skript-Preset (Entscheid E6) |

Der Dropdown-Index-Quirk (deklarierte Defaults ≠ Dropdown-Indizes,
Parameter_Reference §1.4) ist von der Key-Migration unabhängig, sollte aber im
selben Schritt bereinigt werden (Phase-4-Kandidat laut Parameter_Reference §9).

### 7.4 Legacy-Alias `waveform.color.*`

Der Ist-Code akzeptiert `waveform.color.*` als Alias für den Mono-Gradienten.
In der Alias-Map wird er direkt auf `color.mono.*` übersetzt; der Zwischenschritt
über `waveform.monoColor.*` entfällt.

### 7.5 Alter Präfix `scope.` ist doppelt vergeben

Superscope und Oscilloscope nutzen beide `scope.` — die Alias-Maps sind strikt
**pro Visualizer** (Konzept §4.4: „je Visualizer"), nie global.

---

## 8. Review-Entscheide (2026-07-19)

Alle 8 Review-Fragen des Entwurfs (v0.1.0) sind durch Patrik entschieden
(2026-07-19); die Tabellen oben sind auf diesem Stand:

- **E1 — Gains:** `audio.gain` bleibt bei ALLEN Visualizern unverändert in
  Stufe 1 (funktionale AudioSource-Verstärkung vor Floor/Ceil); es gibt **kein**
  `map.gain`. Der wirkungslose EqualizerModule-interne `gain` (tote Anwendung in
  Schritt 0 entfernt, kein erreichbarer Parameter-Key) wird in Schritt 5.1 durch
  den **neuen** Parameter **`render.heightScale`** (Stufe 4, Anzeige-Skalierung
  der Balkenhöhen) ersetzt — ohne Alias.
- **E2 — `audio.bands`:** Kontextabhängig. Equalizer: `eq.bands` → `map.bands`,
  der `audio.bands`-Sync entfällt. Alle anderen Visualizer: `audio.bands` bleibt
  unverändert in Stufe 1.
- **E3 — `waveform.smoothing`:** Hybrid. Entfällt im neuen Schema ersatzlos
  (SmoothingModule übernimmt, Konzept 5.3); beim Laden alter Presets wird der
  Skalar s näherungsweise nach `audio.smooth.timeMs` konvertiert:
  `timeMs ≈ −16.67 / ln(s)` (EMA-Beziehung, 60-FPS-Annahme; s = 0.9 ≈ 158 ms).
  Die Alias-Mechanik braucht dafür **Wert-Konverter** (→ §9).
- **E4 — `scope.triggerIndicator`:** → `render.triggerIndicator` (Stufe 4) —
  Anzeige-Overlay, nicht Mapping.
- **E5 — Oscilloscope `voltsPerDiv`/`offset`:** → `render.chN.*` / `render.mN.*`
  (Stufe 4, Div-Raster-Abbildung) — wie tabelliert bestätigt.
- **E6 — Superscope `scope.preset`:** → `render.preset` (Stufe 4) bestätigt;
  bleibt Preset-Dropdown-Key.
- **E7 — `scope.phosphor*`:** Bestätigt: existiert nicht im Code — die Alias-Map
  hat nichts zu übersetzen; erst mit dem künftigen PostFxModule relevant.
- **E8 — Naming-Stil:** Kanal-strukturiert — `render.mono.offset`,
  `render.ch1.voltsPerDiv` usw., konsistent zu den Farb-Handles
  (`color.mono.*`); kanal-unabhängige Keys bleiben flach (`render.lineStyle`).

---

## 9. Alias-Map-Hinweis

Diese Tabellen sind die **Quelle der statischen Alias-Maps** `alterKey → neuerKey`
je Visualizer für Schritt 5 (Preset-Migration, Konzept §4.4):

- Der `.lvp`-Loader wertet `formatVersion` aus; bei **formatVersion < 2** wird
  jeder Key vor dem `setParam`-Routing durch die Alias-Map des jeweiligen
  Visualizers übersetzt (alt → neu). Unbekannte Keys: loggen und überspringen.
- **Gespeichert wird ausschließlich im neuen Schema** (kein Dual-Write,
  Konzept §5.4); `formatVersion` wird beim Speichern angehoben.
- Muster-Zeilen expandieren mechanisch: `waveform.monoColor.*` → `color.mono.*`
  erzeugt 8 Einträge (Gradient-Sub-Keys §1.3); `scope.chN.source` → `map.chN.source`
  erzeugt 4 Einträge (N = 1–4).
- Auch **unveränderte** Keys (Equalizer `peak.*`, `particle.*`; überall `audio.*`)
  stehen als Identitätseinträge bzw. Whitelist in der Map — so ist die Map
  zugleich die vollständige Key-Prüfliste des Loaders (unbekannt ≠ alt).
- **Wert-Konverter (Erweiterung für Schritt 5, Entscheid E3):** Die
  Alias-Mechanik aus Konzept §4.4 ist bisher rein Key→Key — sie muss um
  **optionale Wert-Konverter** je Alias-Eintrag erweitert werden
  (`alterKey → (neuerKey, optional<Konverter>)`). Erster Anwendungsfall:
  `waveform.smoothing` (Skalar s) → `audio.smooth.timeMs` mit
  `timeMs ≈ −16.67 / ln(s)` (EMA-Beziehung, 60-FPS-Annahme; s = 0.9 ≈ 158 ms;
  Randfälle s ≤ 0 → 0 ms „keine Glättung", s ≥ 1 clampen).
- Sonderfälle der Map: `eq.bands` **und** `audio.bands` → `map.bands` (nur
  Equalizer, §7.1); `waveform.color.*` → `color.mono.*` (Legacy, §7.4); `waveform.smoothing` →
  Wert-Konverter (E3, s. o.); Alias-Maps strikt pro Visualizer wegen des
  doppelten `scope.`-Präfixes (§7.5).
- Der float-für-int-Vertrag des PresetManagers gilt unverändert
  (Test: `test_VisualizerPresetManager.cpp`).
- Zieltests (Schritt 5): Roundtrip alt→neu je Visualizer — ein Alt-Preset mit
  allen Alt-Keys laden, gegen die Soll-Keys prüfen, neu speichern, wieder laden.

---

## 10. Siehe auch

- [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) — Stufenmodell (§4.2),
  Key-Konvention (§4.3), Migrationsmechanik (§4.4)
- [Config_Pipeline_Umsetzungsplan.md](Config_Pipeline_Umsetzungsplan.md) — Schritt 3.3
  (dieses Dokument), Schritt 5 (Alias-Map), Schritt 6 (Visualizer-Migration)
- [Parameter_Reference.md](Parameter_Reference.md) — Ist-SSOT aller Keys
  (Typen, Bereiche, Defaults, Sichtbarkeiten)
- [../presets/FileFormat_Reference.md](../presets/FileFormat_Reference.md) —
  `.lvp`-Format und `formatVersion`

---

## 11. Changelog

| Version | Datum | Änderungen |
|---|---|---|
| **0.2.0** | **2026-07-19** | **Review-Entscheide E1–E8 eingearbeitet (§8): render.heightScale statt map.gain (Equalizer), map.bands nur Equalizer, waveform.smoothing entfällt + Wert-Konverter-Anforderung, render.triggerIndicator, kanal-strukturierte Render-Keys; Status Aktiv** |
| 0.1.0 | 2026-07-19 | Initial (Schritt 3.3): vollständige Migrationstabellen für alle 5 Visualizer, 8 Review-Fragen, Sonderfälle, Alias-Map-Regeln |
