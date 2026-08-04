# MilkdropVisualizer — MilkDrop-Preset-Host (MD1-Kern)

> **Version:** 1.24.0  
> **Datum:** 2026-08-04  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (Import-Phase Roadmap 6, M3–C2) — Standalone-Nachweis C1/C2 grün, In-App-Sichttest c1 offen  
> **Modul:** `MilkdropVisualizer` (global, wie alle `*Visualizer`)  
> **Dateien:** MilkdropVisualizer.hpp, src/visualizers/MilkdropVisualizer.cpp, milkdrop/MilkdropPresetState.hpp, milkdrop/MilkdropBlur.hpp, milkdrop/MilkdropTrace.hpp, milkdrop/MilkdropSamplerName.hpp, milkdrop/MilkdropTextureResolve.hpp  
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
  M5-Sichttest **bestanden** (Session 40) — Kalibrier-Satz
  `asset/calibration/milkdrop/m5/` (8 Presets + README).
- **Regressions-Gate Ladepfad (Session 41):** c1-Presets über `loadMilkFile`
  laden und prüfen, dass `warp/compCustomSource()` gefüllt sind — fängt genau
  den S41-Befund (verlorene tryTranspile-Aufrufe → stiller MD1-Fallback).
- **GL-Smoke** (`test_MilkdropGlSmoke.cpp`): 8 c1-Shader kompilieren + linken
  im Offscreen-3.3-Core-Kontext.
- **MilkdropStandalone** (`projects/exec/MilkdropStandalone/`): isolierter
  End-to-End-Nachweis — echtes GL-Fenster, `--auto` rendert jedes c1-Preset,
  Screenshot + Pixel-Statistik, Exit ≠ 0 wenn ein Preset ohne Custom-Pfad
  bleibt. Diagnose-Trace: `milkdrop/MilkdropTrace.hpp` schreibt Lade-/Render-
  Entscheide nach `<TEMP>/lumiviz_milkdrop_trace.log` (immer aktiv, nur
  Zustandswechsel; `LUMIVIZ_MILKDROP_TRACE=0` schaltet ab).

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 39, M3): MD1-Kern — Warp-Mesh + per_pixel, decay, Waveform 0–7, Borders/DarkenCenter, MD1-Composite, Registry + Import-Anbindung, Mesh-Parameter |
| 1.1.0 | 2026-07-22 | M4 (Session 39): Custom Waves/Shapes (bis 16, eigene SlotHosts, t1–t8-Snapshots, textured-Fan, Border, num_inst), Motion Vectors (reversePropagate), Roh-Waveform-Puffer getrennt von der §0-Glättung |
| 1.2.0 | 2026-07-22 | M5 (Session 40): Blur-Pyramide (MilkdropBlur.hpp + runBlurPasses), Shader-Stufe B (Klassifikation, baked Composite-Konstanten, additive Blur-Layer, subtraktiver Warp-Decay, klassenbasierter Import-Report) |
| 1.3.0 | 2026-07-22 | M6.1 (Session 40): .lvfx-Schwester-Persistenz — MilkdropSerializer.hpp/.cpp (header.type="milkdrop"), loadPresetDocument/savePresetDocument, applyState-Refactor (gemeinsamer Lade-Schwanz), PresetState.warp/compShaderText (SSOT); Kalibrier-Raster render.debugGrid + Sichttest M5 bestanden |
| 1.4.0 | 2026-07-22 | Stufe C1+C2 (Session 40): transpilierte Warp-/Comp-Shader (HlslTranspiler-Lib, GLSL-Präambel als include.fx-Gegenstück, per-Preset-Programme, Sampler-Objekte, q/rand/roam/Blur-Uniforms, stiller MD1-Fallback) + exakter Noise-Port und Custom-Textur-Lader (randNN, texsize_, Platzhalter+Report) |
| 1.5.0 | 2026-07-22 | Session 41: **C1/C2-Befund gelöst** — beim C2-Umbau (2ba48a3) verlorene tryTranspile-Aufrufe in prepareCustomShaders wiederhergestellt (Custom-Quellen blieben leer → stiller MD1-Fallback); Diagnose-Trace MilkdropTrace.hpp (Lade-/Renderpfad, Build-Stempel, Branch-Entscheide), Accessoren warp/compCustomSource()/customGlError(), Regressions-Gate im Ladepfad, Standalone MilkdropStandalone (--auto-Nachweis 8/8 grün) |
| 1.6.0 | 2026-07-22 | N1/N2 (Session 41, Entscheide E1/E2): Klasse wird **Chain-Node-Engine** im MultiEffect-Host — `applyPresetState()` (übersetztes Preset + Textur-Suchbasis übernehmen, GL-frei), Composite-Ziel = das beim onRender-Eintritt gebundene Draw-FBO (`m_targetFbo`, ersetzt die feste Default-FBO-Bindung; Standalone-Verhalten unverändert); Registry-Eintrag "milkdrop" entfernt — Host-Einbettung: `MultiEffectVisualizer::runMilkdropNode` (per-nodeId-Runtime, Audio interleaved durchgereicht, Revision-Vertrag der MilkdropNodeParams) |
| 1.7.0 | 2026-07-22 | **Preset-Sprites** (Session 41, E7-Pflichtkür): MilkDrop2077-`[SPRITEn]`-Sektionen → `SpriteState` (Translator: Keys = Startwerte, Blend-Klemme 0..4, Layer-Sortierung) + `drawUserSprites()` als DrawUserSprites-Port (milkdropfs.cpp:3432-3777) — private EEL-VM je Sprite (`SpriteRuntime`, texmgr-Modell ohne reg/q-Sharing), Referenz-Vertex-Mathe im y-abwärts-Raum mit finaler GL-Negation, 5 Blend-Modi, Colorkey→Alpha beim Laden (`loadSpriteImages`), **burn-in** zeichnet zusätzlich in den Feedback-Buffer, `done` beendet das Sprite; Uploads rev-gekoppelt (`ensureSpriteUploads`); Persistenz `sprites`-Array im MilkdropSerializer; Kalibrier-Satz `s1/` (3 Presets + README). PORT: SpriteSpeed = time-Skalierung (Annahme), 4:3-Burn-Aspekt entfällt |
| 1.8.0 | 2026-07-23 | **fShader-Farbwash** (Session 42, E7): `computeHueCorners()` — 4-Ecken-Regenbogen-Port (milkdropfs.cpp:4033-4062, Zufalls-Phasen `m_hueRandStart` je Preset-Ladung wie m_fRandStart) · MD1-Composite: `uHue0..3` im kTexFragmentShader (bilinear über `vScr`, Amount-Mix gegen Weiß, live = `PresetState.shader`, baked = `compInfo.hueMix`; Reset nach dem Composite, da Programm geteilt) · Custom-Comp: `hue_shader` jetzt im main-Prolog aus `_hue0..3` bilinear berechnet (Roh-Farben wie die comp-Grid-Diffuse; Warp bleibt weiß über Uniform-Defaults) |
| 1.9.0 | 2026-07-23 | **Loudness-Zufuhr-Fix** (Session 43, Befund „Rock The House schwarz trotz Musik"): Band-Loudness las `getSpectrum()` (Mono-Puffer) — der Chain-Node-Pfad füttert aber nur `updateAudioStereo()` (Stereo-Puffer) → Spektrum leer, bass/mid/treb konstant 1.0 (Guard), alle beat-getriebenen Presets tot (im alten Host-Betrieb füllte MainWindow den Mono-Puffer zusätzlich direkt). Jetzt: Kanal-Getter mit L/R-Mix (Mono-Fallback liefern die Getter). Diagnose-Werkzeug: periodische `loudness:`-Trace-Zeile (~1 s erste, dann ~5 s, mit bass-min/max-Dynamik) |
| 1.10.0 | 2026-07-23 | **C3-Kern engine-seitig** (Session 43): Volumen-Noise-Texturen `noisevol_lq/hq` (AddNoiseVol-Port plugin.cpp:2561-2717 — 32³, RANGE 216/256, kubische X/Y/Z-Glättung; GL_TEXTURE_3D + WRAP_R an den Sampler-Objekten, Präfix-Bindings wie 2D, texsize-Uniforms) · GLSL-Präambel: `M_PI`/`M_PI_2`(=2π)/`M_INV_PI_2` + rohe q-Bänke `_qa`–`_qh` (aus fv.qVals gespeist) · sampler3D-Uniform-Deklarationen; preambleDeclares um noisevol erweitert |
| 1.11.0 | 2026-07-23 | **Textur-/Sprite-Suche erweitert** (Befund Patrik S43: Asset-Pack hat `textures/` UND `sprites/`; Presets liegen auch in Unterordnern): Suche läuft jetzt vom Preset-Ordner AUFWÄRTS (bis 4 Ebenen) über `textures/`, `sprites/` und den Ordner selbst — vorher nur `presetDir[/..]/textures` (Custom-Texturen) bzw. eine Ebene (Sprites). Beleg: `lines2.jpg` (liegt in `sprites/`) wird jetzt gefunden, auch aus `presets/<unterordner>/` |
| 1.12.0 | 2026-07-23 | **.lvfx-Bild-Einbettung** (Entscheid Patrik S43): `setEmbeddedImages()` — Loader (Texturen + Sprites) fallen auf eingebettete Original-Dateibytes (Base64) zurück, wenn die Asset-Datei fehlt; Dateien haben Vorrang. Suchlogik nach `milkdrop/MilkdropTextureResolve.hpp` extrahiert (SSOT mit dem ChainSerializer, der beim Speichern genau die referenzierten Bilder einbettet — verwaiste Alt-Einbettungen entfallen; randNN bleibt Ordner-Zufall) |
| 1.24.0 | 2026-08-04 | **Diagnose `LUMIVIZ_MILKDROP_TRACE_VARS` (Session 67, Muster NOSEED/DUMP_WARP):** Komma-Liste von Engine-Variablennamen — nach jedem per_frame werden sie getraced (erste 10 Frames je Ladung, danach jeder 60.). Fand die pixies-Ursache (Kamera-Matrix reg30–38 = NaN ab f0 → EEL-Division-Vertrag, Fix in EelTranspiler 1.3.0 `eel.div`: Nenner 0 ⇒ 0). Kein App-Zustand |
| 1.23.0 | 2026-08-04 | **Sicht-Blende (Session 67, Wunsch Patrik „das Rauschen stört"):** `setSichtBlende(bool)` — an ⇒ nach jeder frischen Rausch-Saat (`seedFeedbackNoise` bei Kaltstart/Resize, `applyFeedbackErbe` bei Löschen/Fading) blendet das COMPOSITE-Ziel `kSichtBlendeSek` = 0,5 s von Schwarz ein (Vollbild-Quad `dst *= rampe²`, Ease-in, nach Sprites/Grid, VOR `swapOnly`) — rein kosmetisch: der Feedback-Loop läuft ungedimmt, Verstärker-Presets zünden unverändert (Beweis: f60-Hash mit/ohne Blende identisch). Default AUS ⇒ Prüfstände/Triage byte-unverändert; die App schaltet per QSettings `milkdrop/sichtBlende` (Default an, Settings-Panel „MilkDrop Start Fade-in", Host reicht je Frame durch). Standalone: `--blende` |
| 1.22.0 | 2026-08-04 | **„RTH erbt Farbe" GELÖST (Session 67, TOP 1):** Der Lazy-Block in `onRender` rief `ensureCustomPrograms()` nur hinter dem Gate `!warpSrc.empty() \|\| !compSrc.empty()` — beim In-Place-Wechsel auf ein Preset OHNE Custom-Quellen (Md1Default/None, z. B. `Rock The House_2024`) wurde der Rev-Wechsel nie verarbeitet und die **Warp-/Comp-PROGRAMME DES VORGÄNGERS renderten das neue Preset weiter** (Trace-Signatur: `warpSrc=nein … Branch warp=CUSTOM`). Bei aa9dc9d0 unsichtbar (jeder Wechsel = frische Instanz, Programme null), in der Triage unsichtbar (ein Preset je Lauf). Fix: `ensureCustomPrograms()` UNCONDITIONAL (Rev-Early-Return ist billig; leere Quellen ⇒ Programme werden resettet, MD1-Branch greift). Beweis: MilkdropStandalone 1.1.0 `--ab`-Wechsellauf (FNV-Hash je Frame) — vorher Spotlight→RTH vs. Beauty→RTH 0/300 gleiche Frames (rot-abklingend vs. weiß-aufhellend), nachher **300/300 bitgleich**; Gegenprobe mit Custom-Shader-Ziel (afterhour mix) war schon vorher bitgleich |
| 1.21.0 | 2026-08-03 | **Puffer-Wechsel „Ausblenden über Zeit" (Session 66, Wunsch Patrik):** `requestFeedbackAusblenden(sekunden)` startet eine Zeit-Ausblendung des Erbes — im Frame dämpft ein Dim-Pass DIREKT nach dem Warp das Echo multiplikativ (Vollbild-Quad, `glBlendFunc(GL_ZERO, GL_SRC_COLOR)`, Faktor `exp(ln(1/256)·dt/T)` ⇒ nach T Sekunden unter 8-bit-Sicht), BEVOR Motion Vectors/Shapes/Waves/Borders frisch zeichnen — der Eigenanteil des neuen Presets bleibt ungedämpft und trägt sich selbst. Mehrfach-Wechsel: die Ausblendung startet neu; frischer Puffer (Kaltstart/Resize) bricht sie ab. Host: `PufferWechsel::Ausblenden` + `pufferAusblendSek` (0.1–60 s, Default 2), Transport via transientem `wechselAusblendSek` (Erbe-Anteil bleibt 1.0). **Nachbefund Patrik (gleiche Session, Bisektion aa9dc9d0→aaceffc + loadDiag-Forschung): Löschen (keep=0) setzt zusätzlich `m_randSeed = kRandSeedInit`** (ein Zug je Ladung: rot_*-Matrizen/hue-Phasen — bewusst VOR `applyPresetState`, damit wie eine frische Instanz gewürfelt wird). Ein `m_loudness.reset()` wurde erprobt und VERWORFEN: die `loadDiag`-Zeilen (5 Frames Audio-Futter je Load) bewiesen, dass laufende Musik auf der leeren Warmup-Rampe zu bass≈17-Spikes explodiert und das Preset unnatürlich in den nächsten Ast tritt. Kern-Erkenntnis: der „je nach Vorgänger anders"-Rest kam vom AUDIO-FUTTER selbst (App-Start = Stille/leise, specSum ~0.6-2 vs. laufende Musik ~4-7) — das ist Eingabe, kein Erbe; kein Puffer-Mechanismus kann das angleichen. `loadDiag` bleibt als Diagnose-Werkzeug |
| 1.20.0 | 2026-08-03 | **Puffer-Wechsel Behalten/Löschen/Fading (Session 66):** Der beim .milk→.milk-Tausch geerbte Feedback-Inhalt ist jetzt steuerbar — `requestFeedbackErbe(keep)` (GUI-Thread unter renderMutex, Host ruft es bei neuem `MilkdropNodeParams.wechselZaehler`) meldet den Erbe-Anteil an, der Render-Thread wendet ihn VOR dem Frame an: keep=1 No-op (Behalten, Original-Semantik), keep=0 = exakte Kaltstart-Saat (`applyFeedbackErbe` schreibt `kaltstartBasis()` — bit-identisch zu `seedFeedbackNoise`, unter NOSEED schwarz), 0<keep<1 = einmaliger CPU-Mix Erbe·keep + Saat·(1−keep) über beide Puffer (Readback via neue FBO-Handle-Getter, FeedbackBuffer 1.1.0). Mehrere Wechsel ohne Frame dazwischen: min(keep) gewinnt; ein frischer Puffer (Kaltstart/Resize) verwirft pending. Aufgelöst wird der Modus (Node-Einstellung `pufferWechsel`/`pufferFading`, App-Default via `setMilkdropPufferWechselDefault`) in `replaceMilkdropPresetInPlace` |
| 1.19.0 | 2026-08-02 | **Regelwerk Legacy/Modern (Strang R, Session 65):** Jeder Milkdrop-Node deklariert seine Betriebsart — `PresetState.regelwerk` ∈ Legacy/Modern/Benutzerdefiniert (Import- und Migrations-Default: **Legacy** = alle vier S64-Emulationen AN, Baseline s64f unverändert) + vier Einzelschalter (`divVertragD3d9`, `unormTrunkierung`, `qGarbageEpsilon`, `uvSanitize`; nur bei Benutzerdefiniert wirksam) + PS-Overrides `psWarp`/`psComp` ∈ Auto/PS2/PS3/MD1erzwingen. Zugriff ausschließlich über `effektiveSchalter()` (Legacy=alle AN, Modern=alle AUS). Gates: HlslTranspiler 1.5.0 `TranspileOptions.d3d9Div` (false ⇒ roher `/`) in `prepareCustomShaders` · MD1-Warp-Programm quantisiert über `uTruncate`-Uniform statt eingebacken · Custom-Warp-Epilog konditional · q-Epsilon nur bei Schalter AN · `fitUv` reicht bei Modern IEEE-Werte roh durch. `MD1erzwingen` setzt die abgeleitete Klassifikation je Stufe auf None (Shader-TEXT bleibt erhalten — die S64-Strip-Bisektion als App-Feature); PS2/PS3 sind reine Angabe-Overrides (GLSL-Emission identisch). Persistenz: MilkdropSerializer 1.1.0 (fehlende Felder ⇒ Legacy+Auto — alle Bestands-.lvfx laden unverändert). Panel: Combo + Schalter (nur bei Benutzerdefiniert aktiv, WYSIWYG-Übernahme beim Wechsel) + zwei PS-Combos, Tooltips je Fixklasse. Prüfstand: `LUMIVIZ_MILKDROP_REGELWERK=modern\|legacy` als Diagnose-Override (Werkzeug-Schalter wie NOSEED, kein App-Zustand) |
| 1.18.0 | 2026-08-02 | **q-Garbage-Epsilon + UV-Fit + Saat wird App-Sache** (Session 64, Forschung D3D9-NaN): (1) OHNE per_frame_init bleibt `q_values_after_init_code` in der Referenz uninitialisierter Heap (state.cpp:440) — Sonde bewies q28 ≈ 8e-7 statt 0; die R-Serie teilt durch diese q's und bleibt so ENDLICH. Nachbau: Presets ohne Init-Code starten mit deterministischem q-Epsilon je Index (1e-6·(1+0,1i), unter der EEL-Vergleichstoleranz 1e-5 — ==0 bleibt wahr, Divisionen werden endlich); Presets MIT Init behalten exakt 0. (2) Riesen-UVs aus solchen Ketten werden IN DOUBLE gewrappt/geklemmt (float-fract auf >2^24 ist exakt 0; D3D9-Fixpunkt behält niederwertige Bits). GRENZE dokumentiert: der konkrete 2077-Look ist Heap-Garbage × Fixpunkt-Überlauf = doppelt undefiniertes Verhalten, nur klassenweise nachstellbar. (3) MilkdropStandalone rendert SAATLOS per Default (`--seed` für App-Verhalten; Entscheid Patrik S64) — R239/R239b jetzt korrekt IST-SO, piercing 01 (App-Befund S63) endlich im Prüfstand reproduzierbar |
| 1.17.0 | 2026-08-02 | **D3D9-UNORM8-Trunkierung + NaN-Vertex-Sanitize** (Session 64, Dunkel-Cluster): (1) D3D9 schneidet beim 8-bit-Rendertarget-Write AB, GL rundet zur nächsten Stufe — multiplikativer Decay stallte bei `v·(1−d) < 0,5/255` (R211: Grauboden exakt 16/255 bei decay 0,97). MD1-Warp-Programm und Custom-Warp-Epilog (`assembleCustomFragment(r, truncateUnorm)`) quantisieren jetzt `floor(ret·255+1e-4)/255`; nur der Feedback-Pfad, der Comp schreibt rückkopplungsfrei auf den Schirm. (2) per_pixel-NaN-Befund (R-Serie: `(x−q26)/q28` mit q=0 — Referenz-q-Sonde beweist q28=0 AUCH dort): beide Engines rechnen NaN-Vertex-UVs, die Referenz rendert sie über den D3D9-Pfad gutartig-lebendig, GL-NaN-Varyings sampeln schwarz und LEEREN den Feedback-Puffer (Beweis: warpin 2181 px → warpout 0, std 0). Näherung: nicht-endliche u/v → 0 (deterministisch); die Look-Treue der NaN-Warp-Klasse bleibt OFFEN (Entscheid nötig). Diagnose-Werkzeug: `LUMIVIZ_MILKDROP_DUMP_WARP=<dir>` dumpt warpin/warpout/postwave + Mesh-UV-Spannen (Frames 0–5) |
| 1.16.0 | 2026-08-02 | **Motion-Vector-/Stereo-Wave-Geisterlinien** (Session 64, Hell-Flach-Cluster): Die Dummy-Trennpunkte zwischen MV-Segmenten (S39) und Stereo-Wave-Linien (Mode 7) nutzten `skip` — seit der S58-Skip-Semantik (Punkt bleibt ANKER) zog jeder Motion Vector ein weißes Geister-Quad aus der (0,0)-Bildmitte (infinity 2: 3071 Quads/Frame, mean 0,999 statt Ref 0,029). Neu `SuperscopePoint::breakStrip` (harter Trenner ohne Anker, ScopeRenderer-Vertrag) an beiden Stellen. Messbeweis: infinity 2 voll 0,999→0,045 (Ref 0,029), MD1-Basis 0,585→0,001 (Ref 0,001) |
| 1.15.0 | 2026-08-02 | **D3D9-Divisionsvertrag in der GLSL-Präambel** (Session 64, Schwarz-Cluster-Rest): `_div`-Overloads (float/vec2–4) — `0/0 → 0` (D3D9-Legacy: 0·rcp(0)=0), `x/0 → ±1e30` statt INF (bleibt multiplizierbar, clampt am Rendertarget). Der HlslTranspiler (1.4.0) emittiert `/` und `/=` als `_div`-Aufruf, Literal-Nenner ausgenommen. Beweis Rainbow Attack NEON: Flash-Term `3*q22/cross` mit q22=cross=0 → IEEE-NaN, `ret += NaN` schwärzte das Vollbild; Referenz beweisbar q-los identisch (probe_qvals schwarz, Voll ≈ noflash 0,161≈0,160). Heilt zusätzlich 02_MW_new01B (mean 0,12 ≈ Ref 0,15) und rainbow spider2 (0,32 vs Ref 0,44-Charakter) |
| 1.14.0 | 2026-08-02 | **Kaltstart-Saat** (Session 63, Ground-Truth-Befund via MilkdropRef): Die frischen Feedback-Texturen wurden schwarz genullt — Verstärker-Presets ohne eigene Energiequelle (Wave-Alpha ~0, Borders/MV aus; z. B. Fractopia, 9× SCHWARZ-Cluster der Triage) leben aber vom ererbten Pufferinhalt; das Original startet mit undefiniertem VRAM und löscht beim Preset-Wechsel nie. Neu `seedFeedbackNoise()`: beide Puffer einmalig mit deterministischem Vollbereichs-Rauschen (xorshift32, fixer Seed → Prüfstände bit-reproduzierbar) statt Schwarz. Messbeweis: Fractopia Kaltstart mean 0,19 (vorher 0,000; Referenz 0,215) |
| 1.13.0 | 2026-07-27 | **Sampler-Namensregel + Rotationsmatrizen** (Session 52, am Quelltext der Referenz gepinnt: `ref/winamp_orig/…/vis_milk2/plugin.cpp:2955`). Neu `milkdrop/MilkdropSamplerName.hpp` als SSOT der Zerlegung (Laden, `preambleDeclares`, Sampler-Objekt beim Binden hatten je eine eigene Regel). Drei Befunde aus einem Blätter-Lauf: `sampler_` wurde UNBEDINGT abgeschnitten — aus `sampler MilkDrop3_001` wurde `3_001` (Textur nicht gefunden, 7 Meldungen im Log), und `sampler tex` (3 Zeichen, 25 Presets des Packs) hätte `std::out_of_range` geworfen · Filter/Wrap-Präfixe nur klein statt case-insensitiv · die umgedrehten Formen `WF_ CF_ WP_ CP_` fehlten ganz. Das Binden filterte zusätzlich auf „beginnt mit sampler_" und ließ präfixlose Texturen ungebunden — jetzt über die Menge der deklarierten Sampler (`m_customSamplerNames`), damit der Platzhalter keine fremde Uniform trifft. Dazu die 24 Matrizen `rot_{s,d,f,vf,uf,rand}1..4` (Präambel-Uniforms `mat3x4`; Basiswinkel/Geschwindigkeit/Verschiebung je Preset nach `state.cpp:RandomizePresetVars` mit `0.9*(k/8)^3.2`, die vier `rot_rand*` jeden Frame neu; Aufbau `((Rx·T)·Rz)·Ry` nach `milkdropfs.cpp:4016-4050`) |
