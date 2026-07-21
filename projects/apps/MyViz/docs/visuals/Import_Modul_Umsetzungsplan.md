# MyViz — AVS-Modul-Umsetzungsplan (Steuerdokument)

> **Version:** 0.1.0
> **Datum:** 2026-07-21
> **Typ:** Umsetzungsplan
> **Status:** Aktiv — Batches + Entscheide (§6) freigegeben; Bau Phase 1 (A–E) startklar
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Import_Modul_Abdeckung.md](Import_Modul_Abdeckung.md) (Was fehlt + Priorität) ·
> [Import_Multieffekt_Host_Entwurf.md](Import_Multieffekt_Host_Entwurf.md) (Host R5) ·
> [Import_Analyse_AVS_MilkDrop.md](Import_Analyse_AVS_MilkDrop.md) §5
> **Sprache:** Deutsch

---

## 1. Zweck

Konkreter Bauplan für die in [Import_Modul_Abdeckung.md](Import_Modul_Abdeckung.md)
kartierten **Passthrough-Module** (16 Builtins + ~12 relevante APEs). Er definiert
das **wiederverwendbare Rezept** (Abschnitt 2), gruppiert die Arbeit in **Batches**
(Abschnitt 3), listet die **wenige neue Querschnitts-Infrastruktur** (Abschnitt 4)
und die **offenen Entscheide** (Abschnitt 6).

Nicht-Ziel: MilkDrop (Roadmap 6, eigener Plan) und die verworfenen Effekte
(SVP, AVI, FyrewurX, GeissFluid, ParticleSystem, MIDItrace, AVSTrans Automation).

---

## 2. Das Rezept — ein Effekt als vertikaler Schnitt

Jeder neue Effekt ist derselbe 7-Schichten-Schnitt (Muster der 16 S35-Effekte).
Anker = Datei · Funktion:

| # | Schicht | Datei | Was |
|---|---|---|---|
| 1 | **Param-Struct** | `include/visualizers/multieffect/EffectChain.hpp` | `struct XxxParams {…}` mit AVS-treuen Defaults; in `EffectParams`-`std::variant` aufnehmen; Eintrag in `effectTypeName()`-Visitor; optionale Range-Clamps in `detail::compileNode()`. |
| 2 | **Decoder** | `libs/AvsParser/include/AvsParserEffects.hpp` | `decodeXxx(Reader&, EffectNode&)` als 1:1-Transkription der `r_*.cpp`-`load_config` (gleiche Feldreihenfolge/Guards). Dispatch-Zeile in `decodeBuiltin()` (Builtin-Index) **oder** `decodeApe()` (APE-ID-String). Float-Felder als int32-Bits lesen. |
| 3 | **Translator** | `src/visualizers/AvsChainTranslator.cpp` | `case kXxx:` in `mapBuiltin()` **oder** `apeId ==`-Zweig in `mapApe()`: `EffectNode`-Felder → `XxxParams`. Farben per `avsColor()` (COLORREF-Swap), Floats per `memcpy`-Reinterpret. |
| 4 | **Render-Handler** | `src/visualizers/MultiEffectVisualizer.{hpp,cpp}` | `runXxx(node, params)` (GL/CPU-Handler) + Zeile im `renderNode()`-Visitor. Ggf. per-`nodeId` GL-Runtime (in `resetRuntimes`/Cleanup freigeben, **Render-Thread**). |
| 5 | **Serializer** | `include/visualizers/multieffect/ChainSerializer.hpp` (+ .cpp) | `nodeToJson`/`nodeFromJson`: `"type":"xxx"` + Felder. `.lvfx`-Roundtrip. |
| 6 | **Panel-Editor** | `src/UI/panels/MultiEffectPanel.cpp` | Editor-Felder (Trivial-Effekte: nur Typ-Eintrag im Modul-Dropdown; parametrierte: Widgets). |
| 7 | **Tests** | `tests/unit/UnitTests/test_AvsChainTranslator.cpp` (+ `test_AvsParser.cpp`) | Translator-Mapping (inkl. Color-Swap/Float-Bits/APE-Dispatch), Serializer-Roundtrip, `effectTypeName`. Ziel: grün, 0 Skips. |

**Kosten je Effekt:** Schichten 1-3, 5-7 sind mechanisch (~½ Tag). Der Aufwand
steckt fast immer in **Schicht 4** (der eigentliche Shader/CPU-Algorithmus). Die
Aufwand-Spalte in [Import_Modul_Abdeckung.md](Import_Modul_Abdeckung.md) bewertet
genau diese Schicht.

**Regeln (aus dem Handover, gelten weiter):** COLORREF→RRGGBB per `avsColor()`;
Float-Preset-Felder nur als int32-Bits + `memcpy`; APE-Knoten per `apeId` dispatchen
(nicht per id-switch); host-globale Ringpuffer in `resetRuntimes` freigeben; GL-Objekte
gehören dem Render-Thread; kein Logging aus Render-Threads; Qt-Keywords (`slots`,`emit`)
meiden; `/WX`.

---

## 3. Batch-Reihenfolge

Reihenfolge nach Aufwand/Nutzen ([Abdeckung §5](Import_Modul_Abdeckung.md)). Jeder
Batch ist ein in sich grüner Commit; Batches sind unabhängig schneidbar.

### Batch A — Quick-Wins + Preset-Entsperrer (Infra steht)
**Ziel:** die häufigen Screenshot-Zeilen wegräumen und mit wenig Aufwand viele
Presets zum Laufen bringen; nur bestehende Infra.
- **Dynamic Distance Modifier** (Builtin 35, `r_ddm.cpp`) — radiale EEL-Distanz;
  nutzt `ScriptGridModule` wie Movement. Decoder = Code-Quartett + Felder.
- **Dynamic Shift** (Builtin 42, `r_shift.cpp`) — EEL-gesteuerte Translation/Rotation
  (globaler Versatz statt per-Punkt-Grid → `ScriptSlotHost` + Uniform-Offset, §6.2).
- **Moving Particle** (Builtin 8, `r_parts.cpp`) — ein bewegtes Feder-Partikel; im
  Screenshot 9× und in vielen Alt-Presets. CPU-Punkt über `ScopeRenderer`.
- **Comment** (Builtin 21, `r_comment.cpp`) — **Trivial-Fix:** als echtes no-op
  „decoded" behandeln (kein `PassthroughParams`, keine Warnzeile).
- **Set Render Mode vervollständigen** (Builtin 40, `r_linemode.cpp`) — der Unroll
  setzt heute nur die **Linienbreite** folgender Scopes; der **globale Blend-Mode**
  (Bits 0–7) + Adjustable-Alpha (Bits 8–15) wird noch nicht auf die nachfolgenden
  Effekte angewendet. Das nachrüsten (Context-Feld wie `lineWidth`), damit
  linienmodus-abhängige Presets korrekt aussehen. **Kein neuer Effekt** — Ausbau
  des bestehenden Unroll in `AvsChainTranslator.cpp`.

### Batch B — Color Map (APE, höchster APE-Schnitt)
- **Color Map** (`Color Map`) — Helligkeit→Gradient-LUT. Nutzt `ColorGradient` +
  `ScriptLutModule`. Sehr verbreitet. Decoder: Gradient-Stops + Key/Blend/Cycle
  (Feld-Layout aus `AVS-File-Decoder` `components.ts`).

### Batch C — Buffer-/Global-APEs (Infra steht, Screenshot)
- **Misc: Buffer blend** (`Misc: Buffer blend`) — zwei Pool-Buffer mischen
  (`OffscreenBufferPool`).
- **Jheriko: Global** (`Jheriko: Global`) — reg/gmegabuf-Persistenz über `ScriptContext`.
  **Synergie mit Wunschliste #6** (preset-globale Variablen) → gemeinsam bauen.
  Datei-I/O-Optionen zunächst weglassen.

### Batch D — billige Pixel-Trans (Shader-Mathe)
- **Color Clip** (Builtin 12, `r_contrast.cpp`), **Unique Tone** (Builtin 38,
  `r_onetone.cpp`), **Interleave** (Builtin 23, `r_interleave.cpp`).
- **Convolution** (`Holden03: Convolution Filter`) — 7×7-Kernel, 2-Pass.
- **Normalise** (`Trans: Normalise`) — Auto-Levels (Frame-Min/Max per Readback/Reduktion).
- **MultiFilter** (`Jheriko : MULTIFILTER`) — Fest-Pixelfilter. ⚠ Leerzeichen vor `:`.
- **Add Borders** (`Virtual Effect: Addborders`) — farbiger Rahmen.
- **Framerate Limiter** (`VFX FRAMERATE LIMITER`) — bei uns vmtl. **no-op** (eigener Takt).

### Batch E — Scope-Render (teilen `ScopeRenderer`)
- **Oscilliscope Star** (2, `r_oscstar.cpp`), **Ring** (14, `r_oscring.cpp`),
  **Rotating Stars** (13, `r_rotstar.cpp`), **Bass Spin** (7, `r_bspin.cpp`),
  **Simple** (0, `r_simple.cpp`). (Moving Particle vorgezogen → Batch A.)
- Simple ggf. über den vorhandenen Equalizer/Oscilloscope-Renderer abkürzen.

### Batch F — Texer II (großer Brocken, hoher Nutzen)
- **Acko.net: Texer II** (`Acko.net: Texer II`) — Bild + EEL-Quartett + Punktschleife
  + per-Punkt Farbe/Größe/Blend. Braucht **Bild-Lader** (Abschnitt 4) + `ScriptSlotHost`.
  Der häufigste APE überhaupt → eigene Session.

### Batch G — Asset-Subsysteme (danach)
- **Picture** (Builtin 34) + **Picture II** (`Picture II`) + **Texer** (`Texer`) —
  gemeinsamer **Bild-Lader** (Abschnitt 4).
- **Render: Triangle** (`Render: Triangle`) — EEL-Dreiecke (Tri-Rasterizer im `ScopeRenderer`).
- **Text** (Builtin 28, `r_text.cpp`) — **GDI-Textrendering** (Font/Layout); höchster Aufwand.

### Batch H — Fraktal-Module (original, **kein** Import) — nach E–G

Eigenständige, **host-native** Fraktal-Generatoren als Content-Quellen (wie
SuperScope) — kein AVS-Effekt. **Rezept-Abweichung: nur 5 Schichten** (Param ·
Render · Serializer · Panel · Tests) — Decoder + Translator entfallen, weil keine
`.avs`-Herkunft. Alle **dynamisch modifizierbar**: ein EEL-Frame/Beat-Slot
(`ScriptSlotHost`) setzt die Kernparameter pro Frame, plus optionale
Audio-Bindung (bass/mid/treble/beat → Parameter). Farbe über eine gemeinsame
Gradient-LUT (wie Color Map).

1. **Fractal2D** (Escape-Time-Fragment-Shader). Typen: **Mandelbrot, Julia,
   Burning Ship, Tricorn, Multibrot (Potenz p), Newton**. Params: `type`,
   `center(x,y)`, `zoom`, `maxIter`, `juliaC(x,y)`, `power`, `escapeR`,
   Farb-LUT + Cycle, Innen-/Außenfärbung (smooth iteration count). EEL setzt
   center/zoom/juliaC/power → Zoom-Fahrten, Julia-Morph, Audio-Puls.
2. **Fractal3D** (Raymarched Distance-Estimator). Typen: **Mandelbulb (Potenz 8,
   variabel), Mandelbox (Scale/Fold), Menger-Schwamm, Quaternion-Julia**. Params:
   Kamera (Pos/Orbit/FOV), `power`/`scale`/`fold`, `maxSteps`, `maxIter`, Licht
   (Richtung/Ambient/AO), Farbe, Fog. EEL/Audio moduliert power/rotation/orbit/fold.
3. **Drittes Modul** — Auswahl (§6.5):
   - **(a) Domain-Warp fBm** (organischer Noise-Fraktal, fBm + Domain-Warping):
     billig, sehr audio-reaktiv, „Plasma/Nebel"-Look. **Empfehlung** (bester
     Aufwand/Nutzen, ergänzt 2D/3D).
   - **(b) Flame / IFS** (Iterated Function System, Apophysis-Stil): Punkt-
     Akkumulation über Feedback/Compute, spektakulär, sehr reaktiv. Aufwand hoch.
   - **(c) Fractal-Zoomer** (Fractal2D + `FeedbackBuffer` = Endlos-Zoom-Trip),
     nutzt vorhandene Infra.

**Querschnitt:** gemeinsame Fraktal-Farbpalette (ColorGradient-LUT) + optionale
**Audio-Parameter-Bindung** (Modul-übergreifend). **Backlog-Ideen:** Kleinian/
hyperbolische Kachelung, Lyapunov-Fraktal, Buddhabrot, Apollonian-Gasket,
Quaternion-Mandelbrot-Slices.

---

## 4. Neue Querschnitts-Infrastruktur

Die meisten Module brauchen **keine** neue Infra (Schicht 4 = Shader über den
bestehenden Ping-Pong-Surfaces). Ausnahmen, jeweils **einmal** zu bauen und dann
von mehreren Modulen genutzt:

1. **Bild-Lader** (für Texer II, Picture, Picture II, Texer) — Pfadauflösung
   relativ zum Preset, `QImage`→GL-Textur, Caching per `nodeId`/Pfad, Lizenz/Asset-
   Handhabung. Blockiert Batch F/G. **Entscheid nötig** (§6.1).
2. **Frame-Min/Max-Reduktion** (für Normalise) — GL-Reduktion oder CPU-Readback
   des verkleinerten Frames. Klein, nur Normalise.
3. **GDI/QPainter-Text-Renderer** (nur Text) — separat, spät.

Bereits vorhanden und wiederverwendbar: `ScriptGridModule`, `ScriptSlotHost`,
`ScriptContext` (reg/q64/gmegabuf), `ScriptLutModule`, `ColorGradient`,
`OffscreenBufferPool`, `FeedbackBuffer`, `ScopeRenderer`, `BeatEstimator`.

---

## 5. Verifikation je Batch

- **Unit:** Translator-Mapping + Serializer-Roundtrip + `effectTypeName` je Effekt
  (Muster: `test_AvsChainTranslator.cpp`). Parser-Decoder gegen `r_*.cpp`-Feldordnung
  (Muster: `test_AvsParser.cpp`), APE-ID-Strings gegen `AVS-File-Decoder`.
- **Korpus:** Nach jedem Batch die betroffenen `.avs` aus `../ref/vis_avs` re-importieren
  und die Passthrough-Zeilen im Import-Report zählen (müssen sinken). Der neue
  **Import-Browser** macht das jetzt bequem.
- **Sichttest (Patrik):** GL/Shader kompilieren erst zur Laufzeit → je Batch ein
  Sichttest; sichttest-kalibrierte Konstanten im Code markieren (wie S35).
- **Abdeckungsmatrix pflegen:** nach jedem Batch [Import_Modul_Abdeckung.md](Import_Modul_Abdeckung.md)
  Zeilen von „Passthrough" nach „verdrahtet" verschieben.

---

## 6. Entscheide (Patrik, 2026-07-21)

Alle vier festgelegt — verbindlich für den Bau:

1. **Bild-Assets (Batch F/G): einbacken.** Beim Import in dieser Reihenfolge suchen —
   (a) neben der `.avs`, (b) konfigurierbarer Assets-Ordner — und das gefundene Bild
   **base64 ins `.lvfx` einbacken** (self-contained, überlebt Verschieben). Fehlt das
   Bild → Platzhalter + Import-Notiz. Kein globaler Laufzeit-Zustand.
2. **Dynamic Shift: Uniform-Offset.** Globaler affiner Versatz (+ optional Rotation)
   über den `ScriptSlotHost` — AVS-treu (bildglobal), kein Grid-Remap.
3. **Framerate Limiter: no-op + Notiz.** Ignorieren (unsere Frame-Steuerung bleibt
   Herr) und eine Import-Notiz setzen. **Merker:** falls sich zeigt, dass ein Eingriff
   in den Render-Takt für den Look nötig ist, bei Gelegenheit neu erwägen.
4. **Reihenfolge: Phase 1 = A–E, dann Phase 2 = F–G.** Erst alles ohne neue Infra
   (max. Preset-Abdeckung), Texer II + Bild-Lader + Text danach. Entscheid #1 kann
   während Phase 1 vorbereitet werden.

**Offen:**

5. **Fraktal-Modul #3 (Batch H):** Domain-Warp fBm (Empfehlung) · Flame/IFS ·
   Fractal-Zoomer — oder mehrere. Umsetzung erst **nach E–G**.

---

## 7. Changelog

- **0.4.0** (2026-07-21): **Batch H — Fraktal-Module** (original, host-native, 5-Schichten-
  Rezept): Fractal2D (Mandelbrot/Julia/…), Fractal3D (Mandelbulb/Mandelbox/…), drittes
  Modul zur Auswahl (Domain-Warp fBm empfohlen); Audio/EEL-Modulation; §6.5 offen.
- **0.3.0** (2026-07-21): §6 von „offen" auf **Entscheide** (Patrik) — Assets einbacken,
  Dynamic Shift Uniform-Offset, FPS-Limiter no-op+Notiz, Reihenfolge Phase 1 A–E → F–G.
- **0.2.0** (2026-07-21): Moving Particle + Set-Render-Mode-Blend-Vervollständigung
  nach Batch A vorgezogen (Preset-Entsperrer); Moving Particle aus Batch E entfernt.
- **0.1.0** (2026-07-21): Erstfassung — Rezept (7 Schichten, Datei-Anker),
  Batches A-G, Querschnitts-Infra, Verifikation, offene Entscheide.
