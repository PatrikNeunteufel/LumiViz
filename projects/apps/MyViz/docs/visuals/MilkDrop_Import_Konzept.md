# MilkDrop-Import — Hauptkonzept, Plan und Roadmap (Import-Phase Roadmap 6)

> **Stand:** 2026-07-22 (Session 38, Entwurf) · **Status:** freigegeben zur Umsetzung
> (Editor-Entscheid Patrik s. §2.3) · **Basis:**
> [Import_Analyse_AVS_MilkDrop.md](Import_Analyse_AVS_MilkDrop.md) §6 (MilkDrop im
> Detail), §7 (EEL→Lua), §9 (Roadmap-Punkt 6), §10 (Entscheide E1–E5) ·
> **Referenzen:** `../ref/MilkDrop3/code` (BSD — nutzbar), `../ref/projectm`
> (LGPL — NUR Konzepte/Ideen, nie Code), `../ref/milkdrop2077` (Lizenz vor
> jedem Code-Blick prüfen; primär als Preset-/Verhaltens-Referenz),
> `../ref/winamp_orig` (Verhaltens-Referenz)

## 1. Zielbild

**Übersetzen statt Emulieren** (wie AVS-Phase, Analyse §4): `.milk`-Presets werden
in LumiViz-eigene Strukturen übersetzt und von einem host-nativen Renderer
ausgeführt — kein DirectX-Emulator, kein projectM-Einbau. Die in den Sessions
32–38 gebauten Fundamente werden wiederverwendet:

| Baustein | Wiederverwendung für MilkDrop |
|---|---|
| `LuaScriptEngine` (Sandbox, Prelude, getspec/getosc/gettime) | per_frame/per_pixel/Wave/Shape-Code |
| `EelTranspiler` (Milk-Dialekt-Kern existiert: Infix, `?:`, Kompound, `buf[]`/`gmem[]`, Cap 2^20) | Gleichungs-Übersetzung |
| `ScriptContext` (reg/gmegabuf preset-lokal, q-Sync) | Variablen-Datenfluss q1–q64, t1–t8, reg/gmegabuf |
| AVS-treuer Audio-Pfad (Log-Spektrum, Beat, bass/mid/treb) | Loudness-Inputs (+ `_att`-Varianten, §3.4) |
| Warp-Mesh-Infrastruktur (ScriptGrid/applyGridWarp, Vertex-Attribute) | per_pixel-Mesh (32×24, „per_pixel" = per VERTEX) |
| FeedbackBuffer/SurfacePair (Ping-Pong) | Double-Buffer previous/current |
| Import-Browser (filtert `.milk` bereits) + Import-Report | Einstieg + Diagnose |
| `Origin::MilkDrop` + Icon (Session 38) | UI-Kennzeichnung |

**Nicht-Ziele (bewusst):** kein LGPL-Code aus projectM; keine generische
HLSL→GLSL-Übersetzung in der ersten Ausbaustufe (§4, M5); kein
Preset-Parameter-Morphing zwischen Presets (Crossfade stattdessen, §3.6).

## 2. Architektur

### 2.1 Eigener Host: `MilkdropVisualizer`

MilkDrop ist eine **feste Pipeline** (Frame-Modell, Analyse §6.1), keine
Effekt-Kette → eigener Visualizer neben MultiEffect (kein MultiEffect-Meganode).
Er folgt dem Visualizer-Vertrag (Threading §12, GL-Runtimes nur im
Render-Thread, `renderMutex()`-Regeln) und nutzt intern Module nach dem
5-Schichten-Rezept.

Frame-Ablauf (1:1 nach Analyse §6.1, Y-Konvention EINMAL festlegen: GL-Norm,
Flip nur beim Präsentieren):

1. per_frame-Code (q-Vars einsammeln, Clamps)
2. Motion Vectors ins Vorframe-Bild
3. **Warp-Pass:** 32×24-Mesh (per_pixel-Gleichungen je Vertex) zeichnet das
   Vorframe-Bild verzerrt in den aktuellen Buffer; decay multiplikativ
4. Blur-Pyramide (3 Stufen; erst M5 — bis dahin `blurN`-Reads = Passthrough)
5. Custom Shapes (4/16×) → Custom Waves (4/16×) → Basis-Waveform
6. Darken Center, Borders
7. **Composite-Pass** (MD1-Fallback: VideoEcho + Gamma/Filter; HLSL s. §4)

### 2.2 Neue Lib: `projects/libs/MilkParser` (header-only, Qt-frei)

Analog AvsParser (Vertragsgrenze: Text → Struktur, kein Transpile, kein Qt):

- Zeilenbasiertes `key=value`; Code-Blöcke `per_frame_1..N` (Abbruch bei
  erster Lücke, Konkatenation), Backtick-Zeilen (`warp_N`/`comp_N`).
- **MD3-Superset parsen:** q1–q64, bis 16 Waves/Shapes, `MILKDROP_PRESET_VERSION`,
  `PSVERSION[_WARP/_COMP]` — was der Renderer (noch) nicht kann, landet
  strukturiert im Import-Report statt zu scheitern (Fehlerphilosophie §4.3
  der Analyse: nie hart abbrechen).
- Parser-Verhalten von projectMs `PresetFileParser.hpp` **abgelesen, neu
  geschrieben** (~120 Zeilen, Analyse §6.3) — kein LGPL-Code.
- Eigene Test-Suite + Korpus-Lauf (Referenz-Presets aus `../ref`-Beständen +
  `asset/milkdrop/`-Ordner, sobald vorhanden).

### 2.3 UI/Editing — Entscheid Patrik (2026-07-22)

**Bearbeitet wird im gedockten Config-Panel wie beim MultiEffect — NICHT als
Overlay im Anzeigefenster** (das Original-MilkDrop-Overlay-Menü war der
erklärte Schwachpunkt). Konkret:

- Eigenes `MilkdropPanel` nach dem Muster des MultiEffect-Panels: links
  Struktur (Preset → per_frame/per_pixel · Waves 1..N · Shapes 1..N ·
  Warp/Comp), rechts Property-Editoren + die vorhandenen Skript-Editoren
  (EEL-Highlighting, ⓘ-Referenz, ⤢-Expand, Kategorie-Highlighting) —
  dieselben Widgets, MilkDrop-Variablenset als eigene `symbolCategory`-Tabelle.
- Import-Browser-Doppelklick auf `.milk` → Host-Auto-Switch auf
  `MilkdropVisualizer` + Panel zeigt das Preset (wie AVS→MultiEffect heute).
- Persistenz als `.lvfx`-Schwester (JSON via ChainSerializer-Muster), Export
  zurück nach `.milk` als Kür (M6).

### 2.4 Milk-Skript-Vertrag (M2 — umgesetzt 2026-07-22, Session 39)

Verbindliche Semantik für alle Milk-Slots; Referenz = MilkDrop3-Quelle (BSD),
Goldens in `test_MilkScriptContract.cpp`:

- **Variablen-Sets** (aus `state.cpp:267-512` abgelesen): per_frame (Warp-Params
  i/o, Audio/Zeit-Inputs, wave_*/ob_*/ib_*/mv_*/echo_*/blur*-Regler, `monitor`
  persistent) · per_pixel (Warp-Params + `x y rad ang`, read-only Frame-Infos) ·
  Wave per_frame/per_point (`t1-t8`, `r g b a`, `samples`, `sample value1
  value2`, `x y`) · Shape per_frame (`t1-t8`, Geometrie/Farben inkl. `r2 g2 b2
  a2`, `border_*`, `instance num_inst`). Alle Namen sind in der
  `symbolCategory`-Tabelle (MultiEffectPanel) kategorisiert — die künftige
  MilkdropPanel-Referenz zieht daraus.
- **q1–q64:** ScriptContext-Snapshots — nach per_frame_init `captureInitSnapshot`,
  je Frame-Beginn `restoreInitSnapshot` (q startet JEDEN Frame vom Init-Stand),
  nach per_frame `captureFrameSnapshot`; Waves/Shapes/per_pixel lesen den
  Frame-Stand, ihre Schreibzugriffe leaken nicht (milkdropfs.cpp:491-493, 673).
- **t1–t8:** wave-/shape-LOKAL (kein Context): nach dem Wave-Init einfrieren,
  je Frame vor per_frame zurücksetzen (`t_values_after_init_code`,
  milkdropfs.cpp:2278-2285/2415) — Host-Mechanik = engine get/set, M3.
- **Audio:** `bass/mid/treb` = Band relativ zum Langzeit-Mittel (~1.0-Baseline!),
  `*_att` = attenuierte Variante — Modul `MilkLoudness`
  (plugin.cpp:8749-8779-Port, fps-korrigiert, Stille-Guard → 1.0).
- **Clamps nach per_frame:** nur `gamma` [0,8] und `echo_zoom` [0.001,1000]
  (milkdropfs.cpp:677-678) — mehr klemmt das Original nicht.
- **Funktions-Deltas:** Korpus-Messung (910 Presets): einziges Delta war
  `int()` = **floor-Alias** (nseel-eval.c:284) → Transpiler-Mapping ergänzt;
  `$pi/$e/$phi` konnte der Lexer schon; `assign/exec3` nutzt kein Preset;
  `getspec` (1 Fundstelle) stellt die Engine ohnehin.

## 3. Optimierte Ideen (projectM als Ideengeber — Konzepte, kein Code)

1. **per_vertex-Fast-Path** (Analyse §6.3): Importer erkennt statisch, ob
   per_pixel-Code `gmegabuf`/`reg` nutzt (Mehrheit: nein) → Vertex-Läufe sind
   unabhängig → ein Lua-Lauf je Vertex ohne Sync-Zwang, cachebar wenn der
   Code frame-invariant ist (Movement-Cache-Muster aus Session 38).
2. **Loudness per EMA** (`bass/mid/treb` + `*_att` als geglättete Varianten) —
   Port-Idee aus projectM `Audio/Loudness.cpp`, implementiert auf unserem
   schon AVS-treuen Spektrum (Log-Kennlinie bleibt getspec-exklusiv; die
   Milk-Bänder rechnen auf den Roh-Bändern wie das Original).
3. **WaveformAligner-Idee** (Mip-basiertes Angleichen aufeinanderfolgender
   Wellenformen) — stabilisiert die Basis-Waveform; als eigenes Modul, das
   später auch Oscilloscope/Waveform-Visualizer aufwertet.
4. **Standard-Post-Module statt HLSL-String-Patching:** die häufigsten
   warp-/comp-Shader-Muster (Echo, Gamma, Hue-Shift, Chroma, Invert,
   Kaleidoskop-Varianten) als parametrisierte GLSL-Bausteine; der Importer
   mappt erkannte Muster darauf (Report vermerkt „übersetzt als Modul X").
5. **Bild-Crossfade** für Preset-Übergänge (statt Parameter-Blending) — passt
   direkt zur geplanten Visual-Playlist (P2).
6. **Kein Triple-Y-Flip:** eine Y-Konvention im Host, Flip ausschließlich beim
   Blit auf den Screen (projectM-Falle, Analyse §6.1).

## 4. Shader-Strategie (der harte Teil, bewusst gestuft)

- **Stufe A (M3/M4):** `PSVERSION == 0` bzw. fehlende Shader → MD1-Pfad
  (fixe Warp-Formel aus zoom/rot/warp/dx/dy/sx/sy + VideoEcho/Filter-Comp).
  Deckt MD1-Presets und viele MD2-Presets mit Default-Shadern ab.
- **Stufe B (M5):** Muster-Erkennung → Standard-Post-Module (§3.4); Rest:
  Preset lädt mit Report-Hinweis „Shader ersetzt durch Fallback" (Bild bleibt
  plausibel: Warp + Decay + Waves tragen die meiste Optik).
- **Stufe C (Kür, eigene Entscheidung später):** eingeschränkter
  HLSL→GLSL-Transpiler für die verbleibende Teilmenge (ps_2_0-artige
  Ausdrücke, `tex2D`, Swizzles). Erst angehen, wenn Korpus-Statistik zeigt,
  dass Stufe B zu wenig abdeckt — Messung wie bei sourcemapped (Session 38).

## 5. Roadmap (jeder Schritt einzeln testbar, Entwurf→Freigabe→Umsetzung)

| # | Schritt | Inhalt | Verifikation |
|---|---|---|---|
| **M1** ✅ | **MilkParser-Lib** (S39) | .milk → Struktur (MD3-Superset, Backtick, Lückenabbruch, Report) — `projects/libs/MilkParser` | 21 Cases; Korpus **910/910** parsen ok (352 Milkdrop3 + 558 winamp; 4 Dateien mit Report-Warnungen = echte Preset-Defekte) |
| **M2** ✅ | **Skript-Vertrag** (S39) | §2.4: Variablen-Sets, q1–q64-Snapshots (ScriptContext vorhanden), t1–t8-Muster, Clamps, `MilkLoudness` (*_att), Funktions-Delta `int()`→floor, `symbolCategory`-Namen | `test_MilkScriptContract.cpp` (11 Cases, end-to-end EEL→Lua→Engine) |
| **M3** ◐ | **Render-Kern (MD1)** (S39, umgesetzt — **GL-Sichttest offen**) | `MilkdropVisualizer` + `MilkdropPresetState` (Original-Defaults): Double-Buffer (FeedbackBuffer + swapOnly), Warp-Mesh + per_pixel je Vertex, decay, Waveform 0–7, Borders/Darken-Center, MD1-Comp (Echo/Gamma/Filter); Registry + Import-Browser-Anbindung; Mesh-Parameter 32×24/Cap 96×72. Dabei Dialekt-Löcher geschlossen (Argument-`;`-Sequenzen, Kommentar-Stripping) → **Transpile 100 %** (892/892 pf, 590/590 pp) | Translator-Tests ✅ (Korpus 910); **GL-Sichttest MD1-Presets AUSSTEHEND** (Y-Flip-Stellschraube: Composite) |
| **M4** ◐ | **Waves/Shapes/MV** (S39, umgesetzt — **GL-Sichttest offen**) | Custom Waves + Shapes bis 16 (eigene SlotHosts am geteilten Context; t1–t8-Snapshot je Wave/Shape, q vom Frame-Stand; Wave-Glättung/Skalierung + per_point, Shape-Fan mit Center/Edge-Farbverlauf, textured = Vorframe-Sampling, Border + thick, num_inst-Instanzen), Motion Vectors (ReversePropagate über das Warp-Mesh); VideoEcho/Filter waren schon in M3. Offen: Sprites, fShader-Wash (Kür) | Translator-Tests ✅; Sichttest zusammen mit M3; Port-Skalen kWave=128/kSpec=32 = Kalibrierpunkte |
| **M5** | **Blur + Shader-Stufe B** | Blur-Pyramide (3 Stufen, `blurN`-Sampling), Muster→Standard-Post-Module, Korpus-Statistik zur Shader-Abdeckung | Sichttest + Report-Auswertung |
| **M6** | **UX-Abschluss** | `MilkdropPanel` (Config-Panel-Editing, §2.3), Crossfade-Übergang, Playlist-Anbindung, `.lvfx`-Persistenz (+ .milk-Export als Kür) | UI-Sichttest; Roundtrip-Tests |

Empfohlene Session-Schnitte: M1+M2 zusammen (reine Lib-/Engine-Arbeit, wie
Sessions 32/33), M3 einzeln (erster Pixel-Sichttest!), M4–M6 nach Befund.

## 6. Entscheidungen (Patrik, 2026-07-22, Session 39)

Messbasis: 910 Presets gescannt (winamp_orig MD2-Pack 558 · asset/Milkdrop3 352).
Verteilung: MD1 (ohne Versions-Header) 310 · MD2 (v201) 514 · MD3 (v300) 86;
Shader PS2=318 · PS3=255 · PS4=27. **`mesh_x/mesh_y` steht in KEINEM Preset** —
die Mesh-Auflösung ist im Original ein App-Setting, kein Preset-Feld.

1. **Mesh-Auflösung ✅:** Visualizer-Parameter, **Default 32×24**
   (Original-Default, originaltreue Sichttest-Basis), **Cap 96×72** (konsistent
   mit DM-Cap, Session 38). Kein Preset-Feld — Frage „respektieren" war
   gegenstandslos.
2. **MD3-Erweiterungen ✅:** parsen sofort (q1–q64, bis 16 Waves/Shapes —
   Datenmodell als Superset/Arrays), rendern ab M4.
3. **Korpus ✅:** beide Packs — `asset/Milkdrop3/presets/` (primär, inkl. 86 MD3)
   + `../ref/winamp_orig/.../Milkdrop2/presets/` (sekundär, inkl. 303 MD1).
   Ziel M1: 910/910 parsen ok. Korpus-Test skippt sauber bei fehlendem Ordner
   (AVS-Korpus-Muster). `asset/Milkdrop3/` ist **bewusst untracked**
   (Community-Rechte; .gitignore-Eintrag mit Herkunfts-Notiz). Der Ordner
   bringt zusätzlich `textures/`, `sprites/`, `shapes/`, `waves/` mit —
   Texturen ab M5 relevant.
4. **Stufe C** (HLSL-Transpiler) — bleibt vertagt: Entscheidung erst nach
   M5-Messung. Vorgeschmack aus dem Scan: PS2 dominiert (318/600 Shader-Presets)
   → spricht für Muster-Module (Stufe B).

## 7. Siehe auch

- [Import_Analyse_AVS_MilkDrop.md](Import_Analyse_AVS_MilkDrop.md) — §6, §7, §9, §10
- [Import_Treue_Fixplan.md](Import_Treue_Fixplan.md) — AVS-Treue-Befunde (Audio-Pfad!)
- [Import_Modul_Umsetzungsplan.md](Import_Modul_Umsetzungsplan.md) — 5-Schichten-Rezept
- [Visualizer_Architecture.md](Visualizer_Architecture.md) — §12 Threading-Vertrag

## 8. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 38): Hauptkonzept + Plan + Roadmap M1–M6; Editor-Entscheid Config-Panel statt Overlay |
| 1.1.0 | 2026-07-22 | §6-Entscheide gefallen (Session 39): Mesh als Parameter 32×24/Cap 96×72; MD3-Superset parsen sofort; Korpus = beide Packs (910); asset/Milkdrop3 untracked. Messbasis 910 Presets dokumentiert |
| 1.2.0 | 2026-07-22 | M1+M2 umgesetzt (Session 39): MilkParser-Lib (Korpus 910/910) + §2.4 Milk-Skript-Vertrag (int()→floor, MilkLoudness, q/t-Snapshot-Kontrakt, symbolCategory-Namen); Roadmap-Status nachgezogen |
| 1.3.0 | 2026-07-22 | M3 umgesetzt (Session 39, GL-Sichttest offen): MilkdropVisualizer (MD1-Kern komplett), PresetState-Translator, Registry-/Import-Anbindung; Dialekt-Löcher geschlossen (Argument-Sequenzen, Kommentar-Stripping) → Transpile-Abdeckung 100 % auf 910 Presets |
| 1.4.0 | 2026-07-22 | M4 umgesetzt (Session 39, GL-Sichttest offen): Custom Waves/Shapes (bis 16, t1–t8-Snapshots, textured, Instanzen), Motion Vectors (ReversePropagate); WaveState/ShapeState im Translator; Sprites + fShader-Wash bewusst offen |
