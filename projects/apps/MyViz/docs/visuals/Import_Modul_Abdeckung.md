# MyViz — AVS-Modul-Abdeckung (Steuerdokument)

> **Version:** 0.3.0
> **Datum:** 2026-07-21
> **Typ:** Abdeckungsmatrix + Priorisierung
> **Status:** Aktiv (Arbeitsgrundlage Import-Phase, Roadmap 5 Nachzug)
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Import_Modul_Umsetzungsplan.md](Import_Modul_Umsetzungsplan.md) (Rezept + Batches) ·
> [Import_Analyse_AVS_MilkDrop.md](Import_Analyse_AVS_MilkDrop.md) §5 ·
> `libs/AvsParser/include/AvsParserEffects.hpp` (Decoder) ·
> `src/visualizers/AvsChainTranslator.cpp` (Mapping) ·
> `include/visualizers/multieffect/EffectChain.hpp` (Module)
> **Sprache:** Deutsch

---

## 1. Zweck

**Live-Statusmatrix** aller AVS-Effekte (Builtins + Community-APEs): was der Import
schon voll rendert, was noch **Passthrough** ist (Roh-Blob konserviert, aber nicht
dargestellt), und wie testbar der jeweilige Stand ist. Grundlage: statische Sichtung
von Decoder + Translator + Render-Host, Passthrough-Meldungen echter Importe.

**Drei Ebenen** = „voll verdrahtet": Decoder (`AvsParserEffects.hpp`) → Translator
(`AvsChainTranslator.cpp`) → Modul/Render (`MultiEffectVisualizer` + Serializer +
Panel + Tests). Das Bau-Rezept dazu: [Import_Modul_Umsetzungsplan.md](Import_Modul_Umsetzungsplan.md).

## 2. Legende

**Status:** ✅ voll verdrahtet · ◐ teilweise/Sonderfall · ⬜ Passthrough (offen) ·
✖ verworfen (nicht sinnvoll portierbar).

**Test:**
- `U` = deterministische Unit-Tests vorhanden (Translator-Mapping + Serializer-Roundtrip).
- `U*` = gleich testbar, Tests beim Bau zu schreiben.
- `S` = zusätzlich GL-**Sichttest** nötig (Shader/Renderer kompilieren erst zur Laufzeit) — für den ganzen S35-Block noch **offen**.
- `A` = schwer testbar (Bild-/Video-/GDI-/externe-DLL-Bindung).
- `—` = kein Render (no-op/Meta), nichts sichtzutesten.

**Ist-Stand:** 33 Builtins ✅ + Set Render Mode ◐ · 6 APEs ✅ (inkl. Color Map).
Unit-Suite grün (262 Cases); GL-Sichttest von Batch A/B + §5.2-Block steht aus.

---

## 3. Builtins — vollständige Matrix

`id` = Registrierungsreihenfolge in `rlib.cpp` = Preset-ID. Alle offenen haben
BSD-Quelle (`ref/vis_avs/.../r_*.cpp`).

| id | Effekt | Kat. | Status | Modul / ref-Quelle | Test |
|---|---|---|---|---|---|
| 0 | Simple | Render | ⬜ | `r_simple.cpp` | U*·S |
| 1 | Dot Plane | Render | ✅ | `DotPlaneParams` | U·S offen |
| 2 | Oscilliscope Star | Render | ⬜ | `r_oscstar.cpp` | U*·S |
| 3 | Fadeout | Trans | ✅ | `FadeoutParams` | U·S offen |
| 4 | Blitter Feedback | Trans | ✅ | `BlitterFeedbackParams` | U·S offen |
| 5 | OnBeat Clear | Render | ✅ | `OnBeatClearParams` | U·S offen |
| 6 | Blur | Trans | ✅ | `BlurParams` | U·S offen |
| 7 | Bass Spin | Render | ⬜ | `r_bspin.cpp` | U*·S |
| 8 | **Moving Particle** | Render | ✅ | `MovingParticleParams` (Feder-Partikel als Dot) | U·S offen |
| 9 | Roto Blitter | Trans | ✅ | `RotoBlitterParams` | U·S offen |
| 10 | SVP Loader | Render | ✖ | externe UVS/SVP-DLL | — |
| 11 | Colorfade | Trans | ✅ | `ColorfadeParams` | U·S offen |
| 12 | Color Clip | Trans | ⬜ | `r_contrast.cpp` — Batch D | U*·S |
| 13 | Rotating Stars | Render | ⬜ | `r_rotstar.cpp` — Batch E | U*·S |
| 14 | Ring | Render | ⬜ | `r_oscring.cpp` — Batch E | U*·S |
| 15 | Movement | Trans | ✅ | `MovementParams` (+23 Builtin-Formeln) | U·S offen |
| 16 | Scatter | Trans | ✅ | `ScatterParams` | U·S offen |
| 17 | Dot Grid | Render | ✅ | `DotGridParams` | U·S offen |
| 18 | Buffer Save | Misc | ✅ | `BufferSaveParams` | U·S offen |
| 19 | Dot Fountain | Render | ✅ | `DotFountainParams` | U·S offen |
| 20 | Water | Trans | ✅ | `WaterParams` | U·S offen |
| 21 | Comment | Misc | ✅ | stiller no-op (keine Passthrough-Warnung) | — |
| 22 | Brightness | Trans | ✅ | `BrightnessParams` | U·S offen |
| 23 | Interleave | Trans | ⬜ | `r_interleave.cpp` — Batch D | U*·S |
| 24 | Grain | Trans | ✅ | `GrainParams` | U·S offen |
| 25 | Clear Screen | Render | ✅ | `ClearParams` | U·S offen |
| 26 | Mirror | Trans | ✅ | `MirrorParams` | U·S offen |
| 27 | Starfield | Render | ✅ | `StarfieldParams` | U·S offen |
| 28 | Text | Render | ⬜ | `r_text.cpp` (GDI) — Batch G | A·S |
| 29 | Bump | Trans | ✅ | `BumpParams` | U·S offen |
| 30 | Mosaic | Trans | ✅ | `MosaicParams` | U·S offen |
| 31 | Water Bump | Trans | ✅ | `WaterBumpParams` | U·S offen |
| 32 | AVI | Render | ✖ | Video-Wiedergabe | — |
| 33 | Custom BPM | Misc | ✅ | `CustomBpmParams` | U·S offen |
| 34 | Picture | Render | ⬜ | `r_picture.cpp` (Bild) — Batch G | A·S |
| 35 | **Dynamic Distance Modifier** | Trans | ✅ | `DynamicDistanceModifierParams` (radiale d-LUT) | U·S offen |
| 36 | SuperScope | Render | ✅ | `SuperScopeParams` (+Figuren/Farbe) | U·S offen |
| 37 | Invert | Trans | ✅ | `InvertParams` | U·S offen |
| 38 | Unique Tone | Trans | ⬜ | `r_onetone.cpp` — Batch D | U*·S |
| 39 | Timescope | Render | ✅ | `TimescopeParams` | U·S offen |
| 40 | **Set Render Mode** | Misc | ◐ | „unrolled": setzt **Linienbreite + Blend-Mode** folgender Scopes (Bits 0–7 → SuperScope `lineBlend`). Kein eigener Effekt | U (Unroll) |
| 41 | Interferences | Trans | ✅ | `InterferencesParams` | U·S offen |
| 42 | **Dynamic Shift** | Trans | ✅ | `DynamicShiftParams` (EEL Uniform-Offset) | U·S offen |
| 43 | Dynamic Movement | Trans | ✅ | `DynamicMovementParams` | U·S offen |
| 44 | Fast Brightness | Trans | ✅ | `FastBrightnessParams` | U·S offen |
| 45 | Color Modifier | Trans | ✅ | `ColorModifierParams` | U·S offen |

**Builtin-Bilanz:** ✅ 33 · ◐ 1 (Set Render Mode) · ⬜ 10 · ✖ 2.

---

## 4. Community-APEs — vollständige Matrix

APEs werden am **32-Byte-ID-String** erkannt (Exakt-Match, Marker `0xFFFFFFFE`).
Referenz für Strings + Feld-Layouts: `grandchild/AVS-File-Decoder`
(`src/lib/components.ts`); Verhalten: `grandchild/vis_avs`.

| ID-String | Effekt | Kat. | Status | Modul / Infra | Test | Verbr. |
|---|---|---|---|---|---|---|
| `Channel Shift` | Channel Shift | Trans | ✅ | `ChannelShiftParams` (uType-Shader) | U·S offen | sehr hoch |
| `Color Reduction` | Color Reduction | Trans | ✅ | `ColorReductionParams` | U·S offen | hoch |
| `Multiplier` | Multiplier | Trans | ✅ | `MultiplierParams` | U·S offen | hoch |
| `Holden04: Video Delay` | Video Delay | Trans | ✅ | `VideoDelayParams` (per-Node-Ring) | U·S offen | hoch |
| `Holden05: Multi Delay` | Multi Delay | Trans | ✅ | `MultiDelayParams` (6 host-globale Ringe) | U·S offen | mittel–hoch |
| `Color Map` | Color Map | Trans | ✅ | `ColorMapParams` (256-LUT-Textur + Blend-Shader) | U·S offen | sehr hoch |
| `Acko.net: Texer II` | Texer II | Render | ⬜ | Bild+EEL+Punktschleife — Batch F | A·S | sehr hoch |
| `Holden03: Convolution Filter` | Convolution | Trans | ⬜ | 7×7-Kernel-Shader — Batch D | U*·S | hoch |
| `Misc: Buffer blend` | Buffer Blend | Misc | ⬜ | `OffscreenBufferPool` — Batch C (Screenshot) | U*·S | mittel–hoch |
| `Jheriko: Global` | Global Variables | Misc | ⬜ | `ScriptContext` reg/gmegabuf — Batch C (Screenshot) | U*·S | mittel |
| `Trans: Normalise` | Normalise | Trans | ⬜ | Min/Max-Reduktion — Batch D | U*·S | mittel |
| `Jheriko : MULTIFILTER` | MultiFilter | Misc | ⬜ | Fest-Pixelfilter — Batch D ⚠ Space vor `:` | U*·S | mittel |
| `Virtual Effect: Addborders` | Add Borders | Misc | ⬜ | Rahmen-Shader — Batch D | U*·S | niedrig–mittel |
| `VFX FRAMERATE LIMITER` | Framerate Limiter | Misc | ⬜ | vmtl. no-op — Batch D | — | niedrig–mittel |
| `Render: Triangle` | Triangle | Render | ⬜ | EEL+Tri-Raster — Batch G | U*·S | mittel |
| `Picture II` | Picture II | Misc | ⬜ | Bild-Lader — Batch G | A·S | mittel |
| `Texer` | Texer (I) | Render | ⬜ | Bild-Lader — Batch G | A·S | niedrig |
| `Misc: AVSTrans Automation` | Trans Automation | Misc | ✖ | Meta/eigenwillig | — | niedrig |
| `FunkyFX FyrewurX v1` | FyrewurX | Misc | ✖ | closed-source | — | niedrig–mittel |
| `GeissFluid` | Fluid | Misc | ✖ | closed-source (Fluid-Sim) | — | niedrig |
| `ParticleSystem` | Particle System | Render | ✖ | Partikel-Engine | — | niedrig |
| `Nullsoft Pixelcorps: MIDItrace ` | MIDI Trace | Misc | ✖ | MIDI-Input ⚠ Trailing-Space | — | sehr niedrig |
| `VFX AVI PLAYER` | AVI Player | Misc | ✖ | Video-Decode | — | sehr niedrig |

**APE-Bilanz:** ✅ 6 · ⬜ 11 · ✖ 6.

**Parser-Fallstricke (Exakt-Match!):** `Jheriko : MULTIFILTER` hat ein **Leerzeichen
vor dem Doppelpunkt**; `Nullsoft Pixelcorps: MIDItrace ` endet mit **Leerzeichen**.

---

## 5. Gesamt-Priorisierung

Details + Rezept: [Import_Modul_Umsetzungsplan.md](Import_Modul_Umsetzungsplan.md).

1. **Batch A — EEL-Trans + Quick-Wins (Infra steht, entsperrt viele Presets):**
   Dynamic Distance Modifier · Dynamic Shift · **Moving Particle** · Comment-Fix.
   Dazu **Set Render Mode vervollständigen** (globalen Blend-Mode-Teil anwenden, nicht
   nur Linienbreite). Räumt die meisten Screenshot-Zeilen.
2. **Batch B — Color Map** (bester APE-Schnitt, nur Gradient-LUT).
3. **Batch C — Buffer blend + Jheriko: Global** (Infra steht; Global synerg. mit Wunschliste #6).
4. **Batch D — billige Pixel-Trans:** Color Clip · Unique Tone · Interleave ·
   Convolution · Normalise · MultiFilter · Add Borders · Framerate Limiter.
5. **Batch E — Scope-Render:** Oscilliscope Star · Ring · Rotating Stars · Bass Spin · Simple.
6. **Batch F — Texer II** (großer Brocken, häufigster APE — Bild-Lader nötig).
7. **Batch G — Asset-Subsysteme:** Picture · Picture II · Texer · Triangle · Text.

---

## 6. Referenzen

- `grandchild/AVS-File-Decoder` — ID-Strings + Feld-Layouts. <https://github.com/grandchild/AVS-File-Decoder>
- `grandchild/vis_avs` — moderner Port, Verhaltens-Referenz. <https://github.com/grandchild/vis_avs/>
- Nullsoft `vis_avs` (BSD, 2005) — die `r_*.cpp` im `ref/`-Korpus.
- APE-Häufigkeiten: <https://visbot.github.io/AVS-Forums/html/t-323045.html>

---

## 7. Changelog

- **0.3.0** (2026-07-21): Umbau zu **vollständiger Statusmatrix** (alle 46 Builtins +
  23 APEs mit Status-Haken + Test-Spalte + Legende); Moving Particle nach Batch A;
  Set Render Mode als ◐ mit offenem Blend-Teil vermerkt.
- **0.2.0** (2026-07-21): §4 Community-APEs (Web-Recherche), §5 Priorisierung, §6 Referenzen.
- **0.1.0** (2026-07-21): Erstfassung — Passthrough-Builtin-Matrix.
