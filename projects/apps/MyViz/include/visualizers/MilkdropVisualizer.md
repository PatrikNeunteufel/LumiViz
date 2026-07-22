# MilkdropVisualizer — MilkDrop-Preset-Host (MD1-Kern)

> **Version:** 1.2.0  
> **Datum:** 2026-07-22  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (Import-Phase Roadmap 6, M3–M5) — **GL-Sichttest M5 offen**  
> **Modul:** `MilkdropVisualizer` (global, wie alle `*Visualizer`)  
> **Dateien:** MilkdropVisualizer.hpp, src/visualizers/MilkdropVisualizer.cpp, milkdrop/MilkdropPresetState.hpp, milkdrop/MilkdropBlur.hpp  
> **Abhängigkeiten:** VisualizerBase · MilkParser (Lib, inkl. MilkShaderClassifier) · EelTranspiler via ScriptSlotHost (Dialect::Milkdrop) · ScriptContext (q1–q64) · MilkLoudness · FeedbackBuffer · ScopeRenderer  
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

**M5 (Session 40):**

9. **Blur-Pyramide** (BlurPasses-Port): 6 Texturen halbierender Auflösung
   (2 je Nutzer-Stufe blur1–3), je Stufe langer H- + kurzer V-Pass
   (Gewichte w[8] fix), progressive Range-Kompression aus den per-frame-Vars
   `blurN_min/max` (+ `blur1_edge_darken` nur im 1. V-Pass); Mathe pur/testbar
   in `milkdrop/MilkdropBlur.hpp` (inkl. PORT-Notiz: Referenz-Kollaps
   min==max → Epsilon-Guard statt inf). Quelle = Vorframe (VS0-Semantik),
   läuft nur, wenn der Composite Blur wirklich konsumiert.
10. **Shader-Stufe B** (`MilkShaderClassifier`, MilkParser-Lib): warp/comp
    werden klassifiziert (None/Md1Default/Md1Plus/Custom). Default-Familie +
    lineare Extras (Blur-Mix, Gain, subtraktiver Decay) rendern **exakt** mit
    EINGEBACKENEN Konstanten (baked: per_frame-Animation von gamma/echo/decay
    ist bei Shader-Presets wirkungslos — Original-Verhalten); Blur-Terme als
    additive Layer mit Un-Bias `tex·(max−min)+min`. Custom → MD1-Fallback,
    Import-Report nennt Klasse, PS-Version, Zeilen und Features.

**Noch nicht (M6/Kür):** Sprites, HLSL-Transpiler (Stufe C — Entscheidungs-
vorlage Konzept §6.5), fShader-Farbwash, Noise-/Custom-Texturen,
Preset-Blending/Crossfade.
**Port-Skalen (sichtkalibriert S39):** `kWavePortScale=192`,
`kSpecPortScale=8` in drawCustomWaves.

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
  Entscheid §6.1 · `render.debugGrid` (Bool, Default aus) — Kalibrier-Raster
  8×6 + Mittelkreuz als Screen-Overlay NACH dem Composite (nie im
  Feedback-Loop; Sichttest-Hilfe, S40).

## 4. Threading / GL

Visualizer-Vertrag (Visualizer_Architecture §12): GL nur im Render-Thread,
Kontextwechsel-Erkennung nach PulsingVisualizer-Muster; `loadMilkFile` läuft im
GUI-Thread unter renderMutex und fasst kein GL an. FeedbackBuffer neu:
`currentTexture()`/`swapOnly()` für Hosts mit eigenem Composite-Pass.

## 5. Tests

- `test_MilkdropPreset.cpp`: Original-Defaults, Key-Mapping (inkl. Blur-Keys
  b1n..b1ed), Blur-Mathe (Kernel/Ranges/Größenkette), Korpus-Smoke
  (910 Presets übersetzen; **Transpile-Abdeckung 100 %**: 892/892 per_frame,
  590/590 per_pixel).
- `test_MilkShaderClassifier.cpp`: Klassifizierer-Fixtures (Default-Familie,
  Md1Plus-Extras, Custom-Grenzen, Feature-Flags) + **Korpus-Gate** (910:
  warp 20/554 Default/Custom, comp 20/13/565, 13 exakte Blur-Konsumenten).
- GL-Pfad: M3/M4-Sichttest **bestanden** (Session 39, 3 Kalibrier-Runden);
  **M5-Sichttest ausstehend** — Kalibrier-Satz `asset/calibration/milkdrop/m5/`
  (8 Presets + README).

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 39, M3): MD1-Kern — Warp-Mesh + per_pixel, decay, Waveform 0–7, Borders/DarkenCenter, MD1-Composite, Registry + Import-Anbindung, Mesh-Parameter |
| 1.1.0 | 2026-07-22 | M4 (Session 39): Custom Waves/Shapes (bis 16, eigene SlotHosts, t1–t8-Snapshots, textured-Fan, Border, num_inst), Motion Vectors (reversePropagate), Roh-Waveform-Puffer getrennt von der §0-Glättung |
| 1.2.0 | 2026-07-22 | M5 (Session 40): Blur-Pyramide (MilkdropBlur.hpp + runBlurPasses), Shader-Stufe B (Klassifikation, baked Composite-Konstanten, additive Blur-Layer, subtraktiver Warp-Decay, klassenbasierter Import-Report) |
