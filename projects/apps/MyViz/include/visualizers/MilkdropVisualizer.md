# MilkdropVisualizer — MilkDrop-Preset-Host (MD1-Kern)

> **Version:** 1.0.0  
> **Datum:** 2026-07-22  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (Import-Phase Roadmap 6, M3) — **GL-Sichttest offen**  
> **Modul:** `MilkdropVisualizer` (global, wie alle `*Visualizer`)  
> **Dateien:** MilkdropVisualizer.hpp, src/visualizers/MilkdropVisualizer.cpp, milkdrop/MilkdropPresetState.hpp  
> **Abhängigkeiten:** VisualizerBase · MilkParser (Lib) · EelTranspiler via ScriptSlotHost (Dialect::Milkdrop) · ScriptContext (q1–q64) · MilkLoudness · FeedbackBuffer · ScopeRenderer  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## 1. Übersicht

Eigenständiger Visualizer (id `"milkdrop"`, Kategorie effects) neben MultiEffect —
MilkDrop ist eine **feste Frame-Pipeline**, keine Effektkette
(MilkDrop_Import_Konzept §2.1). M3-Umfang:

1. per_frame-Skript (q-Snapshots nach M2-Vertrag, `monitor` persistent, Clamps
   nur gamma [0,8] + echo_zoom [0.001,1000])
2. **Warp-Pass:** (meshX+1)×(meshY+1)-Gitter, per_pixel-Gleichungen je VERTEX,
   UV-Formel 1:1 aus `ComputeGridAlphaValues` (zoom^(zoomexp^(rad·2−1)),
   Stretch, 4-Term-Warp-Ripple, Rotation, dx/dy, Halb-Texel); zeichnet das
   Vorframe-Bild verzerrt × decay in den Current-Buffer (FeedbackBuffer,
   `swapOnly()` — Präsentation macht der Composite)
3. **Basis-Waveform** Modi 0–7 (DrawWave-Port: Konstanten exakt, SmoothWave-
   Verdopplung, 4×-Offset für thick/dots, Größen-Buckets für Alpha)
4. Borders (ob/ib-Ringe) + Darken Center (Alpha-Verlaufs-Raute)
5. **MD1-Composite** auf den Screen: Video-Echo (Zoom + Orientierungs-Flips),
   Gamma als **additive Mehrfach-Draws**, Filter brighten/darken/solarize/invert
   als Destination-Blend-Pässe

**Y-Konvention (§2.1-Entscheid):** EIN interner Mathe-Raum (Referenz-Formeln
wortgetreu, keine per-Draw-Flips) — der EINZIGE vertikale Flip sitzt im
Composite-Pass. Steht das Bild im Sichttest auf dem Kopf, ist genau diese eine
Stelle die Stellschraube.

**M4 (Session 39, gleiche Session):**

6. **Custom Waves** (bis 16): eigener ScriptSlotHost je Wave am geteilten
   Context; per_frame sieht q vom Frame-Stand + t1–t8 vom Init-Stand (nicht
   persistent, Original-Verhalten); Sample-Aufbau nach Referenz (Quelle
   Waveform ROH bzw. Spektrum, sep-Spreizung, `mix1=sqrt(smoothing·0.98)`
   vor-/rückwärts-IIR, mult), per_point mit Default `x=0.5+value1`,
   SmoothWave-Pass, dots/thick.
7. **Custom Shapes** (bis 16, `num_inst`-Instanzen): Triangle-Fan mit
   Center→Edge-Farbverlauf, Winkelbasis +45°, Aspect auf dem cos-Term;
   `textured` sampelt das Vorframe-Bild (PORT: previous statt current);
   Border-Kontur + thickOutline.
8. **Motion Vectors:** Gitter nach Referenzformel, Herkunft je Punkt per
   bilinearem `reversePropagate` über die Warp-Mesh-UVs, Mindestlängen.

**Noch nicht (M5+/Kür):** Sprites, Blur-Pyramide, HLSL-Muster-Module,
fShader-Farbwash, Preset-Blending/Crossfade.
**Port-Skalen als Sichttest-Kalibrierpunkte:** `kWavePortScale=128` (Waveform
±1 statt ±128), `kSpecPortScale=32` (Spektrum-Magnituden) in drawCustomWaves.

## 2. Datenfluss

```
.milk → MilkParser::parseFile → milkdrop::translate() → PresetState
      → ScriptSlotHost (Init=per_frame_init, Frame=per_frame, Point=per_pixel)
Frame:  restoreInitSnapshot → pushFrameInputs → run(Frame) → pullFrameOutputs
      → captureFrameSnapshot → Warp-Mesh (run(Point) je Vertex) → Wave/Borders
      → Composite → swapOnly
```

- `MilkdropPresetState` (header-only, `lumi::milkdrop`): alle MD1-Scalars mit
  den **Original-Defaults aus CState::Default** — fehlende Keys verhalten sich
  exakt wie im Original. GL-frei testbar (`test_MilkdropPreset.cpp`).
- Audio: `MilkLoudness` (bass/mid/treb ~1.0-Baseline + `*_att`) aus
  Oktav-Dritteln 200–11025 Hz; Waveform → 576-Puffer resampled + Original-
  IIR-Glättung (`wave_smoothing`/`wave_scale`).
- **PORT-Abweichungen (markiert im Code):** `progress` zykelt über 60 s (keine
  Preset-Playlist), Waveform-Skala ±1 statt ±128, Band-Bins nehmen 0–22050 Hz
  linear an, fShader-Wash fehlt (M4).

## 3. UI-Anbindung

- Registriert in `VisualizerAutoReg.cpp` → erscheint automatisch im
  VisualSelectPanel (Kategorie effects, Order 110).
- Import-Browser-Doppelklick `.milk` → `ImportMilkPresetEvent` → MainWindow
  aktiviert den Host (`setVisualizer("milkdrop")`) und ruft `loadMilkFile()`
  **unter `renderMutex()`** (AVS-Muster); Report-Notizen als Dialog.
- Parameter (ConfigPanel generisch + Preset-Support gratis):
  `render.meshX` (8–96, Default 32) · `render.meshY` (6–72, Default 24) —
  Entscheid §6.1.

## 4. Threading / GL

Visualizer-Vertrag (Visualizer_Architecture §12): GL nur im Render-Thread,
Kontextwechsel-Erkennung nach PulsingVisualizer-Muster; `loadMilkFile` läuft im
GUI-Thread unter renderMutex und fasst kein GL an. FeedbackBuffer neu:
`currentTexture()`/`swapOnly()` für Hosts mit eigenem Composite-Pass.

## 5. Tests

- `test_MilkdropPreset.cpp`: Original-Defaults, Key-Mapping, Korpus-Smoke
  (910 Presets übersetzen; **Transpile-Abdeckung 100 %**: 892/892 per_frame,
  590/590 per_pixel — nach Session-39-Fixes int(), Argument-Sequenzen,
  Kommentar-Stripping).
- GL-Pfad: **Sichttest ausstehend** (erster Pixel-Test der Import-Phase M3) —
  Kandidaten: MD1-Presets aus dem winamp-Pack (303 ohne Shader).

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 39, M3): MD1-Kern — Warp-Mesh + per_pixel, decay, Waveform 0–7, Borders/DarkenCenter, MD1-Composite, Registry + Import-Anbindung, Mesh-Parameter |
| 1.1.0 | 2026-07-22 | M4 (Session 39): Custom Waves/Shapes (bis 16, eigene SlotHosts, t1–t8-Snapshots, textured-Fan, Border, num_inst), Motion Vectors (reversePropagate), Roh-Waveform-Puffer getrennt von der §0-Glättung |
