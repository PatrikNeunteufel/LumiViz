# AVS-/MilkDrop-Import — Analyse der Referenz-Repos

> **Version:** 1.4.0
> **Datum:** 2026-07-20
> **Typ:** Analyse
> **Status:** Aktiv (Grundlage der Import-Phase)
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) §5.6 (Leitplanken) · Superscope (Keimzelle) · `externals/lua54`
> **Sprache:** Deutsch

---

## Inhaltsverzeichnis

1. [Auftrag und Quellenlage](#1-auftrag-und-quellenlage)
2. [Kernbefunde je Quelle](#2-kernbefunde-je-quelle)
3. [Lizenzmatrix](#3-lizenzmatrix)
4. [Zielbild: Übersetzen statt Emulieren](#4-zielbild-übersetzen-statt-emulieren)
5. [AVS im Detail](#5-avs-im-detail)
6. [MilkDrop im Detail](#6-milkdrop-im-detail)
7. [EEL → Lua](#7-eel--lua)
8. [Mapping auf die LumiViz-Architektur — Lücken und neue Bausteine](#8-mapping-auf-die-lumiviz-architektur--lücken-und-neue-bausteine)
9. [Empfohlene Roadmap der Import-Phase](#9-empfohlene-roadmap-der-import-phase)
10. [Entscheidungen](#10-entscheidungen-patrik-2026-07-20)
11. [Siehe auch](#11-siehe-auch)
12. [Changelog](#12-changelog)

---

## 1. Auftrag und Quellenlage

**Auftrag (Patrik, 2026-07-20):** Kein 1:1-Import. AVS-/MilkDrop-Presets werden in die
LumiViz-Modularisierung übersetzt (Stage-Modell, Parameter-Schema, später Multieffekt-Host),
mit moderner GL-Technik gerendert, und Expressions laufen in **Lua statt EEL** — EEL wird
beim Import einmalig transpiliert, zur Laufzeit existiert kein EEL.

Analysierte Repos (Referenz-Klone, im Arbeitsbaum neben LumiViz unter `../ref/`
erwartet — sie sind **nicht** Teil dieses Repositorys; Datei-Referenzen in dieser
Doku sind relativ zum jeweiligen Repo-Root):

| Repo | Inhalt | Rolle für den Import |
|---|---|---|
| `vis_avs` | Original Winamp AVS 2.81 (Nullsoft, Open-Source-Release 2005), Code unter `avs/` | **Primärquelle AVS**: Render-Modell, Effekt-Inventar, .avs-Format, EEL-Integration |
| `projectm` | Portable MilkDrop-Reimplementierung (GL 3.3 Core) | **Primärquelle MilkDrop-Semantik**: .milk-Format, Frame-Ablauf, moderne Umsetzung |
| `MilkDrop3` | Fork milkdrop2077/MilkDrop3 — veröffentlichter Code ist nur der MD-2.25c-Alpha-Unterbau | Sekundär: BSD-lizenzierter .milk-Parser als Referenz; 3.x-Features nur als README |
| `milkdrop2077` | Lazarus/Pascal-Preset-Mixer desselben Autors, kein Renderer | Praktisch irrelevant; belegt zeilenbasierte .milk-Struktur |
| `winamp_orig` | Winamp-Quelltext, enthält modernes ns-eel2 | Referenz für EEL-Dialektvergleich |
| `lua` | Lua 5.4 (in LumiViz bereits als `externals/lua54` eingebunden) | Ziel-Skriptsprache |

## 2. Kernbefunde je Quelle

**vis_avs:** AVS ist eine imperative CPU-Pixel-Pipeline (32-bit-Softwarebuffer, Ping-Pong
`framebuffer`/`fbout`) über **beliebig verschachtelbare Effektlisten** — eine Liste ist
selbst ein Effekt, hält einen eigenen **frame-persistenten Buffer** (Trails/Feedback), 
blendet mit 14 Blend-Modi ein und aus und kann OnBeat- sowie EEL-gesteuert aktiviert
werden. 46 Builtin-Effekte + 5 eincompilierte APEs; ~16 Kern-Effekte decken die Masse
realer Presets ab. Beat-Erkennung ist zweistufig (Onset + BPM-Prädiktor `bpm.cpp`) und im
Effektfluss **mutierbar** (Custom BPM). Das .avs-Binärformat ist ein einfaches rekursives
TLV-Format. → Details §5.

**projectM:** Bestätigt unseren Ansatz doppelt: (a) Presets werden auch dort in ein
eigenes, moderneres Ausführungsmodell **übersetzt statt emuliert** (HLSL-Transpiler,
eigene Eval-Engine), (b) Preset-Übergänge laufen als **Bild-Crossfade** oberhalb der
Pipeline, nicht als Parameter-Blending. MilkDrop-Frame = Doppelpuffer-Feedback: Warp-Mesh
verzerrt das Vorframe-Bild, darüber Waves/Shapes, am Ende Composite-Shader. Per-Vertex-
Gleichungen laufen dort CPU-seitig single-threaded (32×24-Gitter ≈ 825 Interpreter-Läufe
pro Frame) — nur wegen `gmegabuf`/`reg`-Aliasing. → Details §6.

**MilkDrop3 / milkdrop2077:** Die echten 3.x-Quellen sind **nicht öffentlich** (der
veröffentlichte Code ist MilkDrop 2.25c auf BeatDrop-Basis, BSD-3 — darin liegt aber mit
`code/vis_milk2/state.cpp` ein vollständiger .milk-Parser mit allen Defaults als
Referenz). MD3-Erweiterungen innerhalb von .milk sind reine **Superset-Toleranzen**
(q1–q64, >4 Waves/Shapes, größere Wertebereiche). Das `.milk2`-Format ist unspezifiziert
und verzichtbar. milkdrop2077 ist nur ein Preset-Textmixer (Lizenz unklar, keinen Code
übernehmen).

**EEL:** Drei relevante Dialekte — AVS nutzt `avs/ns-eel` (v1, **nicht** die alte
evallib), MilkDrop seine ns-eel2-Kopie, Winamp das moderne ns-eel2. Gemeinsame Semantik:
alles double, ε=1e-5-Truthiness/Gleichheit, `%`/`&`/`|` integer-basiert,
`sqrt(x)=sqrt(|x|)`, `if`/`?:`/`&&`/`||` lazy, `band`/`bor` eager, implizite Variablen=0,
case-insensitiv, globale `reg00–99`. Die Sprache ist winzig (~15 Produktionen); ein
Transpiler nach Lua 5.4 ist **mittlere Komplexität (grob 2–4 k LOC inkl. Tests)** und
Lua-Performance reicht ohne JIT locker (Superscope 1000 Punkte × 60 fps ≈ <5 % eines
Kerns). → Details §7.

## 3. Lizenzmatrix

| Quelle | Lizenz | Konsequenz für LumiViz |
|---|---|---|
| vis_avs (inkl. ns-eel) | **BSD 3-Clause** (Nullsoft 2005) | Portierung, Teilübernahme (z. B. `bpm.cpp`-Logik, Preset-Feldtabellen) und Neuimplementierung **uneingeschränkt zulässig**; Copyright-Hinweis mitführen |
| projectM `src/` | **LGPL 2.1** | **Nur als Referenz lesen, keinen Code kopieren** (Konzepte nachbauen ist frei); dynamisches Linken wäre zulässig, ist aber nicht unser Weg |
| projectm-eval (vendored, Submodule) | MIT | Direkt nutzbar — für uns nur als Verhaltensreferenz interessant |
| hlslparser (projectM-vendored) | MIT | Nutzbar, falls je generische HLSL-Übersetzung nötig wird |
| MilkDrop3 `code/` | **BSD 3-Clause** (BeatDrop/Nullsoft-Header) | Portierung erlaubt — gilt nur für den veröffentlichten Alpha-Stand (= MD 2.25c) |
| milkdrop2077 | Widersprüchlich (GPL-3 vs. CC BY-NC-SA im Header) | Keinen Code übernehmen (ohnehin wertlos) |
| Community-APEs (Texer II etc.) | Nicht im Repo, Lizenz je Autor | Bei Bedarf Neuimplementierung nach Verhaltensbeschreibung |
| Fremd-Presets (.avs/.milk) | Urheberrecht der Preset-Autoren | Import beim Nutzer ist ok; keine Preset-Sammlungen ins Repo committen |

## 4. Zielbild: Übersetzen statt Emulieren

1. **Import = einmalige Übersetzung** in das LumiViz-Parameter-Schema + Lua-Skripte.
   Kein EEL-Interpreter, kein D3D, keine CPU-Pixel-Pipeline im Produkt.
2. **Import-Ziele:**
   - `.avs` (Nullsoft AVS Preset 0.1/0.2, rekursives TLV) → Multieffekt-Preset
   - `.milk` mit `MILKDROP_PRESET_VERSION=200/201` (MilkDrop 2, PS 2/3) → Visualizer-Preset
     mit Feedback-Post-Stage; Parser als **tolerantes Superset** (q1–q64, beliebig viele
     `wavecode_N`/`shapecode_N`, keine MD2-Wertebereichs-Clamps, unbekannte Keys warnend
     überlesen, `PSVERSION_WARP`/`PSVERSION_COMP` getrennt)
   - **Nicht unterstützt:** `.milk2` (unspezifiziert, closed-source, marginal verbreitet)
3. **Fehlertoleranz als Grundhaltung:** Preset-Bestände sind schmutzig. Nicht
   übersetzbare Teile deaktivieren + im Import-Report ausweisen; niemals hart abbrechen
   (AVS kapselt sogar jeden Effekt-Render in SEH — Presets dürfen kaputt sein).
4. **Shader-Strategie MilkDrop:** Die häufigsten warp/comp-Muster (decay, echo, blur-mix,
   hue) als **parametrisierte Standard-Post-Module** erkennen statt beliebiges HLSL zu
   übersetzen; generische HLSL→GLSL-Übersetzung ist ein separates, teures Spät-Feature
   (projectM macht es per String-Patching + hlslparser — erklärtermaßen „best effort").
5. **Preset-Übergänge** (AVS-Transitions, projectM-Blending) sind Player-Features
   oberhalb der Pipeline — Bild-Crossfade zweier Instanzen, nicht Parameter-Blending.

## 5. AVS im Detail

### 5.1 Render-Modell

- 32-bit-Pixel `0x00RRGGBB`, Buffer = Fenstergröße, jeder Effekt bekommt
  `framebuffer` (in-place) + `fbout`; Rückgabe-Bit 0 toggelt den Ping-Pong-Swap
  (`avs/vis_avs/r_list.cpp:497`).
- **Effektliste = Effekt** (`C_RenderListClass`): beliebig verschachtelbar. Nicht-Root-
  Listen halten einen **eigenen frame-persistenten Buffer** (`thisfb` → Trails/Feedback
  innerhalb der Liste), blenden den Eltern-Buffer hinein (Input-Blend), rendern die
  Kinder, blenden zurück (Output-Blend).
- **14 Blend-Modi** (In wie Out): Ignore, Replace, 50/50, Maximum, Additive,
  Subtractive 1-2/2-1, Every other line, Every other pixel, XOR, Adjustable (0–255),
  Multiply, Buffer (Alpha aus globalem Buffer, invertierbar), Minimum
  (`r_list.cpp:995-1011`).
- **OnBeat-Aktivierung** je Liste (aktiv für N Frames nach Beat) + **EEL-Slot-Paar**
  (Init/Frame) der Liste steuert `enabled`, `clear`, `beat`, `alphain`, `alphaout`.
- **8 globale Named Buffers** (`getGlobalBuffer`, `rlib.cpp:430-451`): genutzt von
  Buffer Save (`r_stack.cpp`), Blend-Modus „Buffer" und APEs.
- **Beat-System:** Onset-Detektor (`main.cpp:288-330`) + BPM-Prädiktor mit Konfidenz,
  Halb-/Doppel-Beat-Korrektur und Einrasten (`bpm.cpp`) — **lohnende Portierungsvorlage
  (BSD)**. Beat ist im Effektfluss mutierbar: Render-Rückgabeflags `0x10000000` (Beat
  setzen) / `0x20000000` (Beat löschen), genutzt von Custom BPM (`r_bpm.cpp`).
- Audio: `visdata[2][2][576]` — Spektrum + Waveform, je 576 Bytes/Kanal, liefert der Host.

### 5.2 Effekt-Inventar (46 Builtins, ID = Registrierungsreihenfolge = Preset-ID)

| ID | Name (Menü) | Funktion | EEL-Slots |
|---:|---|---|---|
| 0 | Render / Simple | Spektrum-/Oszilloskop-Scope (Linien/Solid/Punkte) | — |
| 1 | Render / Dot Plane | 3D-rotierende Punkteebene | — |
| 2 | Render / Oscilliscope Star | Sternförmiges Oszilloskop | — |
| 3 | Trans / Fadeout | Farben Richtung Zielfarbe abklingen | — |
| 4 | Trans / Blitter Feedback | Zoom-Feedback-Blit, OnBeat-Zoom | — |
| 5 | Render / OnBeat Clear | Screen alle N Beats löschen | — |
| 6 | Trans / Blur | Box-Blur (3 Stärken) | — |
| 7 | Render / Bass Spin | Bassgetriebene rotierende Fächer | — |
| 8 | Render / Moving Particle | Springender Partikelball | — |
| 9 | Trans / Roto Blitter | Rotations-/Zoom-Blit | — |
| 10 | Render / SVP Loader | Externe SVP/UVS-DLLs | — |
| 11 | Trans / Colorfade | Kanalabhängige Farbverschiebung | — |
| 12 | Trans / Color Clip | Schwellwert-Clipping auf Farbe | — |
| 13 | Render / Rotating Stars | Rotierende Sternpolygone | — |
| 14 | Render / Ring | Ring-Oszilloskop | — |
| 15 | Trans / Movement | Koordinaten-Remap (polar/rect), 23 Formeln + Expression | Point (einmalig tabelliert) |
| 16 | Trans / Scatter | Zufällige Pixelverschiebung | — |
| 17 | Render / Dot Grid | Bewegtes Punktraster | — |
| 18 | Misc / Buffer Save | Framebuffer ↔ globaler Buffer 1–8, mit Blend | — |
| 19 | Render / Dot Fountain | 3D-Punktefontäne | — |
| 20 | Trans / Water | Wasserwellen | — |
| 21 | Misc / Comment | Nur Text | — |
| 22 | Trans / Brightness | RGB-Skalierung, Ausschlussfarbe | — |
| 23 | Trans / Interleave | H/V-Zeilenraster | — |
| 24 | Trans / Grain | Korn-Rauschen | — |
| 25 | Render / Clear screen | Löschen (jedes/alle N Frames/OnBeat) | — |
| 26 | Trans / Mirror | Spiegelung H/V, OnBeat-Random | — |
| 27 | Render / Starfield | 3D-Sternenfeld | — |
| 28 | Render / Text | Textrendering | — |
| 29 | Trans / Bump | Bump-Mapping-Licht | Init/Frame/Beat — `x,y,isbeat,islbeat,bi` |
| 30 | Trans / Mosaic | Pixelierung | — |
| 31 | Trans / Water Bump | Höhenfeld-Wassertropfen | — |
| 32 | Render / AVI | AVI-Video | — |
| 33 | Misc / Custom BPM | Beat-Signal ändern | — |
| 34 | Render / Picture | BMP-Bild | — |
| 35 | Trans / Dynamic Distance Modifier | Radiales Distanz-Remap, scriptbar | Point/Frame/Beat/Init — `d,b` |
| 36 | Render / SuperScope | Frei scriptbarer Scope | Point/Frame/Beat/Init — `n,b,x,y,i,v,w,h,red,green,blue,linesize,skip,drawmode` |
| 37 | Trans / Invert | XOR 0xFFFFFF | — |
| 38 | Trans / Unique tone | Auf einen Farbton mappen | — |
| 39 | Render / Timescope | Scrollendes Spektrogramm | — |
| 40 | Misc / Set render mode | Globaler Linien-Blendmodus/-breite für Folge-Effekte | — |
| 41 | Trans / Interferences | N rotierte Kopien geblendet | — |
| 42 | Trans / Dynamic Shift | Subpixel-Frame-Verschiebung, scriptbar | Init/Frame/Beat — `x,y,w,h,b,alpha` |
| 43 | Trans / Dynamic Movement | Gitterbasiertes Koordinaten-Remap, scriptbar | Point/Frame/Beat/Init — `x,y,d,r,b,w,h,alpha` |
| 44 | Trans / Fast Brightness | ×2 / ×0.5 | — |
| 45 | Trans / Color Modifier | Per-Kanal-Farbkurve als 256er-LUT, scriptbar | Level/Frame/Beat/Init — `red,green,blue,beat` |

Builtin-„APEs" (per ID-String serialisiert): Channel Shift, Color Reduction, Multiplier,
Video Delay, Multi Delay. Effect List hat die ID `0xFFFFFFFE`; unbekannte Effekte werden
als Passthrough konserviert (`r_unkn.cpp`). Ältere APE-Namen werden per Aliastabelle auf
Builtins gemappt (`rlib.cpp:159-177`).

**Priorisierung für den Import:**

- **Kernmenge (~16, deckt das Gros realer Presets):** Effect List, SuperScope, Movement,
  Dynamic Movement, Blur, Fadeout, Brightness/Fast Brightness, Color Modifier/Colorfade,
  Clear/OnBeat Clear, Buffer Save, Mirror, Invert, Roto/Blitter Feedback, Custom BPM,
  Set render mode (als Übersetzungsregel — den globalen Zustand beim Import „ausrollen").
- **Mittel, lohnend:** DDM, Dynamic Shift, Bump, Water(-Bump), Interferences, Grain,
  Mosaic, Scatter, Delays, Dot-Renderer, Starfield, Timescope.
- **Exot/verzichtbar:** AVI, Picture, SVP Loader, Text, Laser-Zweig, SMP-Mechanik (GPU
  ersetzt sie), `getkbmouse`/`setmousepos`.

### 5.3 Preset-Format (.avs)

```
"Nullsoft AVS Preset 0.2\x1a"        ← Signatur (0.1 wird ebenfalls akzeptiert)
[1 Byte]  Root-Mode (Bit 0 = Clear every frame)
0..n Effekt-Einträge:
  [int32]  Effekt-ID   (<16384: Builtin-Index · >=16384: es folgen 32 Bytes APE-ID-String
                        · 0xFFFFFFFE: verschachtelte Effektliste)
  [int32]  Länge L des Config-Blobs
  [L Bytes] Config-Blob (effekt-spezifisch, meist Folge von int32-Feldern)
```

Verschachtelte Listen: Config-Blob = Mode-Byte (Bit 7 → voller 32-bit-Mode + Extended-Data
mit Blend-Werten/OnBeat-Feldern), danach Kind-Effekte im selben Format; der EEL-Code der
Liste steckt in einem Pseudo-Eintrag mit ID-String `"AVS 2.8+ Effect List Config"`
(`r_list.cpp:50,106-109`). EEL-Strings: neues Format längenpräfixiert (Versions-Byte
`0x01`), Altformat feste 256/768/1024-Byte-Blöcke. Slot-Reihenfolge im File:
`[0]=Point, [1]=Frame, [2]=Beat, [3]=Init`.

**Aufwand:** Container-Parser trivial (rekursives TLV); die Arbeit sind die ~50
effekt-spezifischen Blob-Layouts (je `load_config` ablesen) — Kern-Effekte in Tagen,
Vollabdeckung Fleißarbeit. **Testkorpus:** 35 Presets unter `avs/vis_avs/presets/`.

## 6. MilkDrop im Detail

### 6.1 Frame-Modell (Referenz: projectM `MilkdropPreset.cpp:78-170`)

Doppelpuffer (previous/current, Swap am Frame-Ende):

1. **per_frame**-Code ausführen (q-Vars einsammeln, Werte clampen)
2. Motion Vectors ins Vorframe-Bild
3. **Warp-Pass:** Per-Vertex-Mesh (Default 32×24) zeichnet das Vorframe-Bild verzerrt in
   den aktuellen Buffer (Warp-Shader; Verschiebung im Vertex-Shader)
4. Blur-Pyramide (3 Stufen) aktualisieren
5. Custom Shapes (4×) → Custom Waves (4×) → Basis-Waveform
6. Darken Center, Borders
7. **Composite-Pass** (comp-Shader; MD-1.x-Fallback: VideoEcho + Filter)

Y-Flip-Falle DirectX↔GL: projectM flippt dreimal pro Frame — bei Eigenbau von Anfang an
eine Konvention festlegen.

### 6.2 .milk-Struktur

Zeilenbasiertes `key=value`; Code-Blöcke als nummerierte Keys (`per_frame_1..N` bis zur
ersten Lücke, zu einer Zeile konkateniert); Shader-Zeilen mit Backtick-Präfix
(`warp_N`/`comp_N`). Abschnitte: Basiswerte (zoom, rot, warp, decay, wave_*, ob_*/ib_*,
mv_*, Blur-Grenzen …), `per_frame_init`, `per_frame`, `per_pixel` (historischer Name —
läuft pro **Mesh-Vertex**), 4× Custom Waves (`wavecode_N_*` + init/per_frame/per_point),
4× Custom Shapes (`shapecode_N_*` + init/per_frame), Warp-/Comp-HLSL, Versionen
(`MILKDROP_PRESET_VERSION`, `PSVERSION[_WARP/_COMP]`).

**Variablen-Datenfluss:** per_frame schreibt `q1–q32` → per_pixel/Waves/Shapes/Shader
lesen sie (Snapshot nach Init); `t1–t8` lokal je Wave/Shape; `reg00–99` und `gmegabuf`
global über alle Kontexte. MD3 erweitert auf q1–q64 und 16 Waves/Shapes → Parser-Superset.

### 6.3 Was projectM heute anders machen würde (und wir übernehmen)

- **per_vertex auf schnellem Pfad:** Der Importer erkennt statisch, ob ein Preset
  `gmegabuf`/`reg` im per_pixel-Code nutzt (Mehrheit nicht) — dann sind die Vertex-Läufe
  unabhängig und parallelisierbar/vorkompilierbar; projectMs CPU-Single-Thread-Schleife
  existiert nur wegen dieses Aliasing.
- **Standard-Post-Module statt String-Patching** für die häufigen Shader-Muster (§4.4).
- **Bild-Crossfade** statt Parameter-Blending für Übergänge.
- Nützliche Portierungsvorlagen (Konzept, nicht LGPL-Code): Warp-Mesh-Ansatz
  (`PerPixelMesh.cpp`), Loudness/Beat (`Audio/Loudness.cpp` — bass/mid/treb + _att via
  EMA), WaveformAligner (Mip-basiertes Angleichen — würde Oscilloscope/Waveform sofort
  aufwerten), Frame-Ablauf als Spezifikation. Parser-Verhalten (Backtick, Lückenabbruch)
  aus `PresetFileParser.hpp` ablesen, neu schreiben (~120 Zeilen).

## 7. EEL → Lua

### 7.1 Dialekte

AVS nutzt `avs/ns-eel` (v1; Vergleiche/Logik **nur als Funktionen**: `equal/above/below/
band/bor/bnot/if`, kein `^`, Zuweisung nur top-level bzw. `assign()`, Hex `3Bh`,
`loop`-Cap 4096, signifikante Namenslänge 8!). MilkDrop nutzt seine ns-eel2-Kopie
(Infix `== != <= >= && || ?: ^`, Zuweisung als Expression, `buf[i]`/`gmem[i]`, Hex `$x3B`,
Caps 2^20). **Ein gemeinsamer Parser mit Dialekt-Flag `{avs, milkdrop}`** reicht; der
MilkDrop-Text-Preprocessor muss nicht nachgebaut werden.

### 7.2 Dialektunabhängige Kernsemantik (Transpiler-Vertrag)

- Ein Typ: double. Implizite Variablen = 0.0. Case-insensitiv. `reg00–99` app-global.
- Truthiness/Gleichheit mit ε=1e-5 (`|x|>1e-5` bzw. `|a-b|<1e-5`).
- `%`, `&`, `|` runden auf Integer (round-to-nearest); `x%0` → 0 (bzw. Divisor-Clamp).
- `sqrt(x)` = `sqrt(|x|)` — kein NaN. Division durch 0 → ±Inf/NaN, nie Fehler.
- `if(c,a,b)`, `?:`, `&&`, `||` sind **lazy**; `band`/`bor` sind **eager** (0/1).
- `rand(x)`: Integer 0…x−1 (Empfehlung: Integer-Semantik für beide Dialekte).
- Laufzeitfehler gibt es nicht — jede Operation liefert irgendeinen double.

### 7.3 Lua-5.4-Mapping (Entscheidungen)

| Problem | Lösung |
|---|---|
| Zuweisung als Expression | **Hoisting** bei der Codegen (`x=(y=5)*2` → `y=5; x=y*2`); in lazy Kontexten Statement-Form mit Temporärvariable — nie über Zweiggrenzen hoisten |
| Int/Float-Split von Lua 5.4 | Konsequent Float emittieren (`5` → `5.0`); Int-Operationen (`%`, Bit-Ops) in Prelude kapseln (`eel.mod`, `eel.band64` — Luas Floor-`%` weicht bei Negativen ab!) |
| megabuf/gmegabuf (0-basiert) | Lua-Tables `mb`/`gmb` mit `eel.mbread/mbwrite` (Default 0, Index-Clamp auf Original-Kapazität); `buf[i]=v`/`assign(megabuf(i),v)` als Lvalue-Spezialfall |
| Lazy `if`/`?:`/`&&`/`||` | Nebenwirkungsfrei → `(eel.truthy(c) and (a) or (b))` (EEL-Werte sind Zahlen, in Lua immer truthy — kein and/or-Problem); sonst Lua-`if`-Statement |
| Implizite Globals | `load(code, name, "t", env)` je Skript-Quartett; `env`-Metatable `__index → 0.0`; Identifier lowercasen; **Local-Promotion** (benutzte Variablen am Chunk-Anfang in Locals, am Ende zurück — wichtig für per_point) |
| ε-Logik, sqrt, rand | Prelude: `eel.truthy/eq/bnot/sqrt/rand` (eigener PRNG, deterministisch seedbar; kein `math.random`) |
| `^` / `pow` | Lua `^` direkt (beide C-`pow`, inkl. `0^0=1`) |
| Host-Funktionen | `getosc/getspec/gettime` als C-Callbacks aus MyViz; `getkbmouse/setmousepos` als No-op-Stubs |

### 7.4 Transpiler-Architektur und Aufwand

Dreistufig zur **Import-Zeit**: Lexer (~200–300 LOC) → rekursiv absteigender Parser mit
Pratt-Präzedenzen (~400–600 LOC, Grammatik ≈ 15 Produktionen) → Codegen AST→Lua-Quelltext
(~500–800 LOC), plus Funktionskatalog-Mapping (~45 Einträge) und Lua-Prelude (~150 LOC).
**Gesamt: 2–4 k LOC inkl. Golden-Tests.** Der heikle Teil ist Semantik-Treue der
Randfälle → Golden-Test-Suite (Ausdruck→Erwartungswert) direkt aus §7.2 ableiten.
Fehlerbehandlung: Syntaxfehler → Skriptteil leer (AVS-Verhalten) + Import-Report;
EEL2-Vollsprachfeatures (Strings, `function`, stack_*) → Skript deaktivieren + Warnung.

**Performance:** Superscope 1000 Punkte × 60 fps ≈ 0,3–3 M VM-Ops/s — **<5 % eines
Kerns** in Lua 5.4 ohne JIT (MilkDrop-per_vertex gleiche Größenordnung). Bedingungen:
Chunk einmal `load`en, pro Frame nur `lua_call`, Local-Promotion, Zahlen direkt ins
env-Table, `collectgarbage("generational")`. LuaJIT/Batch nur als Option für Extrem-Presets.

### 7.5 Sandbox

`load(…, "t", env)` (kein Bytecode); Environment-Whitelist (kein io/os/debug/package/
load/string.dump/Metatable-Zugriff/Koroutinen); `math`-Teilmenge; `loop`/`while`-Caps hart
einkompiliert (4096 AVS / 2^20 MilkDrop) — Ausführungszeit strukturell beschränkt, kein
`debug.sethook`-Zähler nötig; megabuf-Clamps begrenzen den Speicher. Ein `lua_State` pro
Render-Thread; `gmegabuf`/`regs` als von MyViz verwaltete Strukturen.

## 8. Mapping auf die LumiViz-Architektur — Lücken und neue Bausteine

### 8.1 Was direkt passt

| Fremd-Konzept | LumiViz-Ziel |
|---|---|
| bass/mid/treb + _att, FFT, Waveform | AudioSource-Stage (1); Loudness-Modell als Vorlage |
| per_frame / Frame-/Beat-/Init-Slots | Lua-Frame-Skript = Wertquelle „Expression" der Parameter (geplantes Binding-Modell §5.6.2) |
| AVS-Farboperationen (Brightness, Invert, LUT …) | Post-Stage (6) bzw. Farbmodule — je ein trivialer Fragment-Shader; Color Modifier = 1D-LUT-Textur, von Lua bei Bedarf neu berechnet |
| Waves/Shapes/Scopes | Render-Stage (4); per_point ≙ Superscope-Punkt-Skript |
| Wellenfarben | Color-Stage (3) |
| decay/Fadeout | Post (Hold/Fade existiert) |
| Blur-Pyramide | Post/Glow (ggf. zu 3 Stufen ausbauen) |

### 8.2 Lücken — vor bzw. in der Import-Phase zu bauen

1. **Doppelpuffer-Feedback als Pipeline-Fähigkeit** (MilkDrop-Essenz, AVS-Trails):
   zwei persistente FBOs mit Swap pro Frame; „PreviousFrame" als **benannter
   Textur-Input** im Stage-Schema, den mehrere Stages deklarieren können (Feedback-Warp,
   textured Shapes, Echo).
2. **Skriptbare Module / Feld-Expressions:** Parameter-Expressions (skalar) decken
   Frame/Beat/Init ab, aber **nicht** die Point-Slots. Es braucht einen Modultyp
   „skriptbares Modul": Lua-Funktion läuft pro Punkt (SuperScope), pro Gitterknoten
   (Dynamic Movement / MilkDrop-per_vertex: `f(x,y,rad,ang) → {zoom,rot,…}` auf
   konfigurierbarem Gitter → Displacement) oder pro LUT-Eintrag (Color Modifier).
3. **Multieffekt-Host** (AVS-Effektketten): Container-Knoten mit verschachtelten FBOs,
   **persistentem Listen-Buffer je Knoten**, 14 Blend-Modi als Shader, OnBeat-Aktivierung,
   Lua-Steuerung von enabled/clear/alpha. Ohne diesen Baustein ist kein nennenswertes
   AVS-Preset importierbar. (Das ist der konkrete, begrenzte „Node-System-Unterbau" —
   kein frei verdrahtbarer Node-Editor, vgl. Concept §6.1.)
4. **Geteilter Skript-Kontext:** deklarierter Datenfluss zwischen den Lua-Skripten eines
   Presets (Frame-Skript schreibt q1–q64, Stage-/Punkt-Skripte lesen; t1–t8 lokal;
   Snapshot-Semantik nach Init); `reg00–99`/`gmegabuf` **preset-lokal** + kleines
   app-globales Atomic-Register-Set als LumiViz-Erweiterung (Entscheid §10.3).
5. **BeatService:** chain-scoped, von Modulen überschreibbar (Custom BPM), mit
   BPM-Prädiktion — AVS `bpm.cpp` (BSD) als Portierungsvorlage.
6. **Globaler Buffer-Pool:** 8 benannte FBOs je Preset (Buffer Save, Blend-Modus
   „Buffer").
7. **Frame-History-Ringpuffer** (Video/Multi Delay) — klar umrissen, mittlere Prio.
8. **Preset-Übergänge:** Bild-Crossfade zweier Instanzen am Render-Thread/Compositor —
   oberhalb der Stage-Pipeline, späteres Feature.
9. **Standard-Inputs:** Noise-/Random-Texturen (MilkDrop `texsize_noise_*` etc.) als vom
   Host bereitgestellte Texturen.

### 8.3 Architektur-Einordnung

AVS-Presets zielen auf den **Multieffekt-Host** (Baustein 3); „flache" Presets und
MilkDrop-Presets passen auf das **erweiterte Stage-Modell** eines einzelnen Visualizers
(Bausteine 1–2). Beides teilt sich Transpiler, Skript-Kontext, BeatService und
Buffer-Infrastruktur — deshalb zuerst die gemeinsamen Fundamente (§9).

## 9. Empfohlene Roadmap der Import-Phase

Reihenfolge so gewählt, dass jeder Schritt einzeln testbar ist und das jeweils Gelernte
den nächsten Entwurf informiert (bewährtes Muster: Entwurf → Freigabe → Umsetzung):

1. ✅ **Lua-Fundament im Superscope (Keimzelle)** *(Session 32, 2026-07-20)*:
   lua54 eingebunden (dynamisch + DLL-Deploy), `lumi::scripting::LuaScriptEngine`
   (Sandbox + eel-Prelude + app-globales Atomic-Register-Set, Doku:
   `include/scripting/LuaScriptEngine.md`), Superscope-Lua-Modus
   (`render.script.lua`, 4 Slots, Fallback auf Preset-Mathematik). 16 neue
   Test-Cases (Sandbox, Prelude-Golden-Tests, Superscope, Performance) — Suite
   107/107. **Gemessen: 1000 Punkte ≈ 0,3–0,6 ms/Frame (~3 % des
   60-fps-Budgets)** — Prognose §7.4 bestätigt, LuaJIT unnötig (§10.5).
2. ✅ **EEL→Lua-Transpiler** *(Session 32, 2026-07-20)*: Lib
   `projects/libs/EelTranspiler` (header-only, Qt-frei; Doku:
   `include/EelTranspiler.md`) — Lexer/Pratt-Parser/Codegen mit Hoisting,
   Lazy-Konstrukten, eel-Prelude-Mapping, Dialekt-Flag (AVS-Cap 4096 /
   MilkDrop 2^20), 8-Zeichen-Aliasing-Warnung (§10.2). AVS-Dialekt komplett,
   MilkDrop-Kern (Infix, `?:`, Kompound-Zuweisung, `buf[]`/`gmem[]`).
   25 Golden-Cases end-to-end (EEL → Lua → Sandbox-Ausführung) — Suite 132/132.
3. ✅ **.avs-Parser** *(Session 33, 2026-07-20)*: Lib `projects/libs/AvsParser`
   (header-only, Qt-frei; Doku: `include/AvsParser.md`) — Container-TLV
   (Signatur 0.1/0.2, verschachtelte Listen inkl. Extended-Data + Listen-EEL,
   APE-ID-Strings + Alias-Tabelle, Altformat-Strings), Blob-Decoder der
   Kernmenge (~17 Effekte + Effect List), Import-Report-Gerüst
   (pfad-präfixierte Warnungen, nie hart abbrechen). 17 Test-Cases inkl.
   Korpus-Lauf: **35/35 Referenz-Presets parsen ok** (170 Effekte, 5 Warnungen
   = Community-APE FunkyFX). Suite 152/152.
4. **Feedback-/Skript-Modul-Fundament** (§8.2 Bausteine 1–2 + 4–6) als Entwurf mit
   Freigabe — hier fließt alles Gelernte aus 1–3 ein.
5. **Multieffekt-Host** (Baustein 3) + Übersetzung der AVS-Kernmenge (§5.2).
6. **MilkDrop-Import:** .milk-Parser (Superset, §4.2), per_frame/per_vertex→Lua,
   Warp-Mesh + Standard-Post-Module; generische HLSL-Übersetzung bewusst hinten.

## 10. Entscheidungen (Patrik, 2026-07-20)

1. **`rand`-Semantik: Integer für beide Dialekte** (`rand(x)` → Integer 0…x−1, sauberer
   PRNG statt C-`rand()`); intern Dialekt-Flag, um bei konkret abweichenden Presets
   gezielt umschalten zu können.
2. **AVS-8-Zeichen-Variablen-Aliasing wird NICHT emuliert.** Der Transpiler **warnt** im
   Import-Report, wenn ein AVS-Skript zwei Bezeichner mit identischen ersten 8 Zeichen
   enthält (mögliches unbeabsichtigtes Aliasing → manuelle Sichtung).
3. **`reg00–99`/`gmegabuf` sind preset-lokal** (alle Skripte eines Presets teilen sie;
   keine Kopplung zwischen Visualizern, kein Locking zwischen Render-Threads).
   **Zusätzlich als LumiViz-Erweiterung:** ein kleines **app-globales Register-Set**
   (32 Slots) als `std::atomic<double>`-Array im Host — Lua-Zugriff über gebundene
   C-Funktionen (`app.gget(i)`/`app.gset(i, v)`), lock-frei, jeder `lua_State` bleibt
   threadgebunden. Jeder Slot einzeln atomar, keine Transaktions-Garantie über mehrere
   Slots. Der Importer benutzt das Set nie — Angebot für handgeschriebene/modernisierte
   Skripte (z. B. Visualizer-übergreifende Signale).
4. **Transpiler als eigene Lib** `projects/libs/EelTranspiler` (BasicLogger-Muster):
   Qt-frei, eigenes Test-Target mit Golden-Test-Suite, Vertragsgrenze String→String.
5. **Lua 5.4 festgeschrieben** (kein LuaJIT-Rücksichts-Design). Notausgang für
   Extrem-Presets ist Batch-Ausführung in C++, kein Runtime-Wechsel.

## 11. Siehe auch

- [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) — §5.6 Leitplanken (verbindlich), §6.1 Node-Editor-Abgrenzung
- [Visualizer_Architecture.md](Visualizer_Architecture.md) — Modul-System, §12 Threading-Vertrag
- `harvest/konzepte/viz_2025_node_reference_manual_visualizer_pipeline.md` — Binding-Modell-Vorbild
- Referenz-Repos: `../../../../../../ref/` (vis_avs, projectm, MilkDrop3, milkdrop2077, winamp_orig, lua)

## 12. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.4.0 | 2026-07-20 | §9.3 umgesetzt: AvsParser-Lib (Container-TLV, Kernmengen-Decoder, Import-Report; Korpus 35/35) — Session 33 |
| 1.3.0 | 2026-07-20 | §9.2 umgesetzt: EelTranspiler-Lib (Lexer/Parser/Codegen, 25 Golden-Cases) — Session 32 |
| 1.2.0 | 2026-07-20 | §9.1 umgesetzt: Lua-Fundament im Superscope (LuaScriptEngine, eel-Prelude, Messwerte) — Session 32 |
| 1.1.0 | 2026-07-20 | §10: alle fünf offenen Fragen entschieden (rand=Integer, kein 8-Zeichen-Aliasing + Warnung, reg/gmegabuf preset-lokal + globales Atomic-Register-Set, Lib EelTranspiler, Lua 5.4 fix) |
| 1.0.0 | 2026-07-20 | Erstfassung — Analyse der ref/-Repos (Session 32): AVS/projectM/MilkDrop3-Befunde, Lizenzmatrix, EEL→Lua-Mapping, Architektur-Lücken, Roadmap |
