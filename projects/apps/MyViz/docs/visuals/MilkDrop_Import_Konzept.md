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

### 2.3 UI/Editing — Entscheid Patrik (2026-07-22, **revidiert Session 40**)

**Bearbeitet wird im gedockten Config-Panel wie beim MultiEffect — NICHT als
Overlay im Anzeigefenster** (das Original-MilkDrop-Overlay-Menü war der
erklärte Schwachpunkt).

> **Revision (Patrik, Session 40): KEIN eigenes MilkdropPanel.** Das Editing
> wird ins **MultiEffect-Panel integriert** — AVS und MilkDrop sollen nicht
> getrennt bleiben (dafür stehen die Origin-Icons). Zielbild: Milkdrop als
> **Chain-Node** im MultiEffect-Host (Meganode mit fester interner Pipeline),
> der Panel-Baum zeigt Preset → per_frame/per_pixel · Waves · Shapes ·
> Warp/Comp als Kinder des Nodes. Der gemeinsame Skript-Editor-Baukasten ist
> dafür extrahiert (`UI/panels/EelScriptEditing.hpp`: EelHighlighter,
> symbolCategory-SSOT, ⓘ-Referenz-/⤢-Expand-Dialoge).

- ~~Eigenes `MilkdropPanel`~~ (revidiert, s. o.) — die vorhandenen
  Skript-Editoren (EEL-Highlighting, ⓘ-Referenz, ⤢-Expand,
  Kategorie-Highlighting) werden im MultiEffect-Panel wiederverwendet;
  MilkDrop-Variablenset ist bereits in der `symbolCategory`-Tabelle (M2).
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
- **Stufe B (M5 ✅, Session 40):** Klassifikation statt freier Muster-Suche —
  MilkDrop GENERIERT Shader-Text aus den MD1-Keys (GenWarp/GenCompPShaderText,
  plugin.cpp:8782-8847); `MilkShaderClassifier.hpp` (MilkParser-Lib) erkennt
  diese Familie plus lineare Extras (Blur-Mix `ret += GetBlurN [*k]`,
  `lerp(GetBlurN, GetPixel, k)`, `GetPixel*a + GetBlurN*b`, bare `ret *= k`,
  subtraktiver Warp-Decay `ret -= k`) als affines Modell
  `ret = gain·Basis + Σ Bn·blurN` mit EINGEBACKENEN Echo/Gamma/Hue/Filter-
  Konstanten → **exakt** über den MD1-Composite + additive Blur-Layer gerendert
  (baked = per_frame-Animation der Composite-Werte wirkungslos, wie im
  Original mit Shadern). Rest: Custom → MD1-Fallback + Report mit
  Feature-Summary (Blur/Noise/Texturen/Zufall, Zeilenzahl).
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
| **M5** ✅ | **Blur + Shader-Stufe B** (S40 — **Sichttest BESTANDEN**, alle 8 m5-Presets inkl. Baked-Vertrag 07 + Fallback 08) | Blur-Pyramide 1:1 (6 Texturen/3 Stufen, H+V-Pässe, Range-Kompression + Un-Bias, `b1n..b1ed`-Keys + per_frame-blurN-Vars, `MilkdropBlur.hpp` pur/testbar), `MilkShaderClassifier` (None/Md1Default/Md1Plus/Custom + Feature-Flags), baked Composite (Warp-Decay/Echo/Gamma/Filter aus dem Shader-Text) + additive Blur-Layer, Import-Report je Klasse; Korpus-Statistik + Gate (§6.5) | Suite 367 Cases (Klassifizierer-Fixtures, Blur-Mathe, Korpus-Gate); Kalibrier-Presets `m5/` (8+README); **Sichttest ausstehend** |
| **M6** ◐ | **UX-Abschluss** | ✅ **M6.1 `.lvfx`-Persistenz** (S40): `MilkdropSerializer` (Schwester-Format, header.type="milkdrop", Shader-Text = SSOT → Klassifikation beim Laden neu abgeleitet), Load-Dispatch nach Dokument-Typ + Save nach aktivem Host in MainWindow, `loadPresetDocument`/`savePresetDocument` am Visualizer. **Offen:** `MilkdropPanel` (Config-Panel-Editing, §2.3), Crossfade-Übergang, Playlist-Anbindung, .milk-Export (Kür) | Roundtrip-Tests ✅ (Voll-Fixture + Kalibrier-Korpus 26/26); UI-Sichttest offen |

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
4. **Stufe C** (HLSL-Transpiler) — ~~bleibt vertagt~~ **ENTSCHIEDEN (Patrik,
   Session 40): Stufe C wird KOMPLETT umgesetzt** (C1 Ausdrücke/tex2D/Swizzles
   → C2 Noise-Texturen + Custom-Textur-Lader → C3 Loops/tex3D). Messbasis war
   §6.5; Umsetzungsreihenfolge und Architektur siehe offene Entscheide (E3/E4
   in der Session-40-Vorlage).
5. **Korpus-Statistik M5 (Session 40, Messbasis 910) + Stufe-C-Vorlage:**
   - **600 Presets mit Shader-Code**, 310 ohne (MD1 → bei uns exakt).
     PSVERSION-Verteilung: PS2 318 · PS3 254 · PS4 28.
   - **Klassifikation** (C++-Gate in `test_MilkShaderClassifier.cpp`):
     warp Default/Custom = **20/554** · comp Default/Plus/Custom =
     **20/13/565** · exakte Blur-Konsumenten (Md1Plus) = **13**.
   - **Feature-Nutzung in Shader-Presets:** Blur **88 %** (528!) · Noise 49 %
     (292, prozedural — implementierbar ohne Assets) · echte Custom-Texturen
     28 % (170; Top: worms 47, MilkDrop3_00x, clouds; Assets liegen in
     `asset/Milkdrop3/textures/`) · rand 36 % · Loops nur 8 %, tex3D 9 %.
   - **Befund:** die Default-Familie (Stufe B) deckt exakt ~53 Presets ab;
     die 554/565 echten Custom-Shader sind mit Mustern nicht erreichbar
     (LOC-Median 9–20+). **Blur-Pyramide + Noise-Sampler + Textur-Lader sind
     mit M5 bzw. als kleine Folgebausteine vorhanden/nah — die verbleibende
     Lücke ist die HLSL→GLSL-Ausdrucksübersetzung selbst.** PS2/PS3 sind
     strukturell einfache Ausdrucks-Shader (wenig Kontrollfluss: 8 % Loops).
   - **➜ Empfehlung an Patrik:** Stufe C als eingeschränkter
     HLSL→GLSL-Transpiler lohnt sich (Hebel: bis zu ~550 Presets); sinnvolle
     Schnitte: C1 Ausdrucks-Teilmenge + tex2D/Swizzles/Intrinsics (deckt die
     Masse), C2 Noise-Texturen + Custom-Textur-Lader, C3 Loops/tex3D (Kür).
     Entscheid + Priorisierung bitte freigeben (eigene Session, Muster
     EelTranspiler: Lexer/Parser/CodeGen header-only + Korpus-Gate).

## 6b. Entscheide Session 40 (Patrik, 2026-07-22) + Roadmap-Fortschreibung

Vorlage E1–E8 entschieden:

1. **E1 = B:** Milkdrop wird **Chain-Node** im MultiEffect-Host (Meganode
   rendert die feste Pipeline in den Chain-Buffer); Panel-Baum zeigt
   Preset → per_frame/per_pixel · Waves · Shapes · Warp/Comp als Node-Kinder.
2. **E2:** Standalone-`MilkdropVisualizer` wird **entfernt**, sobald der Node
   gleichwertig ist (Parameter meshX/Y + debugGrid ziehen an den Node um;
   Import-Browser routet auf MultiEffect+Node).
3. **E3:** Reihenfolge **C1 → Node-Integration (N1 Rendering, N2 Panel-Baum)
   → C2 → C3**. *(N1/N2 hieß in der Entscheidungs-Vorlage kurz „B1/B2" —
   umbenannt wegen Kollision mit Shader-Stufe B; Legende:
   [MilkDrop_Import_Status.md](MilkDrop_Import_Status.md).)*
4. **E4:** Stufe C als **eigene header-only Lib `HlslTranspiler`**
   (`projects/libs/`, Muster EelTranspiler: Lexer/Parser/CodeGen +
   Korpus-Gates); MilkParser bleibt bei Text → Struktur.
5. **E5:** Crossfade = **echtes Doppel-Rendering** (beide Presets leben,
   Bild-Mix linear als erste Stufe; per-Vertex-Warp-UV-Blending à la
   m_fBlendProgress als Ausbaustufe). **Freeze-Frame nur als automatischer
   Performance-Fallback.** Patrik: näher am Original ist gewünscht.
6. **E6:** Visual-Playlist **nach** der Node-Integration (schaltet dann
   Chains + Milkdrop-Nodes einheitlich über .lvfx, inkl. E5-Crossfade).
7. **E7:** Kür-Paket ist **PFLICHT** (keine optionale Kür): Sprites nach C2,
   fShader-Farbwash mit C1, Decay-Dither + .milk-Export am Ende.
8. **E8:** geparkte Altpunkte (Wormhole-Bisektion, Sichttests §7/§8,
   Set Render Mode→alle Scopes, Skript-SSOT, Kleinkram S31) **ganz am Ende,
   gebündelt mit einer Kalibrier-Preset-Runde** gelöst.

**Fortgeschriebene Reihenfolge** (Fortschritts-SSOT:
[MilkDrop_Import_Status.md](MilkDrop_Import_Status.md)):
C1 ✅ → C2 ✅ *(in Session 40 vorgezogen, da rein Shader-seitig)* →
**N1/N2 Node-Integration + Standalone-Entfernung** → Sprites → Crossfade (E5)
→ Visual-Playlist → C3 (Loops/Arrays/tex3D) → fShader-Wash → Decay-Dither +
.milk-Export → E8-Altpunkte + Kalibrier-Runde.

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
| 1.5.0 | 2026-07-22 | M5 umgesetzt (Session 40, GL-Sichttest offen): Blur-Pyramide (MilkdropBlur.hpp + GL-Pässe), MilkShaderClassifier (Stufe B: Default-Familie + Blur-Mix exakt, baked Konstanten, Custom→Fallback+Report), Korpus-Statistik §6.5 + Stufe-C-Entscheidungsvorlage; Kalibrier-Satz m5 (8 Presets) |
| 1.5.1 | 2026-07-22 | M5-Sichttest BESTANDEN (Patrik, alle 8 m5-Presets, Runde 1 ohne Stellschrauben); Kalibrier-Raster-Overlay `render.debugGrid` als Host-Parameter (Screen-Overlay nach Composite); Notiz: Milkdrop→Effect-Chain als Post-M6-Backlog bestätigt (Standalones→Module) |
| 1.6.0 | 2026-07-22 | M6.1 umgesetzt (Session 40): .lvfx-Schwester-Persistenz — MilkdropSerializer (JSON, ChainSerializer-Muster, GL-frei), PresetState trägt rohe Shader-Texte (SSOT, Re-Klassifikation beim Laden), MainWindow-Dispatch Laden/Speichern; Roundtrip-Gates (Fixture + Kalibrier-Korpus 26/26) |
| 1.7.0 | 2026-07-22 | Entscheide Patrik (Session 40): §2.3 revidiert — kein eigenes MilkdropPanel, Integration ins MultiEffect-Panel (Zielbild Chain-Node); Stufe C KOMPLETT freigegeben (C1–C3). Skript-Editor-Baukasten extrahiert (EelScriptEditing.hpp), MultiEffectPanel umgestellt |
| 1.8.0 | 2026-07-22 | §6b: Entscheide E1–E8 (Chain-Node, Standalone-Entfernung, C1→Node→C2→C3, Lib HlslTranspiler, Crossfade = Doppel-Rendering mit Freeze-Frame-Fallback, Playlist nach Node, Kür = PFLICHT, Altpunkte am Ende mit Kalibrier-Runde) + fortgeschriebene Roadmap |
| 1.10.0 | 2026-07-22 | **Stufe C2 umgesetzt** (Session 40, GL-Sichttest offen): exakter AddNoiseTex-Port (256/1, 32/1, 256/4, 256/8; RANGE-216-Wrap, X-Hauptzeilen- + Y-Spalten-Cubic; PORT: Catmull-Rom statt dwCubicInterpolate) + Custom-Textur-Lader (QImage jpg/png; Suchpfade `<preset>/textures`, `<preset>/../textures`, `<preset>`; randNN[_mask]-Zufallswahl; texsize_-Uniforms; fehlend → Platzhalter + Report; Upload/Austausch rev-gekoppelt im Render-Thread) |
| 1.9.0 | 2026-07-22 | **Stufe C1 umgesetzt** (Session 40, GL-Sichttest offen): Lib HlslTranspiler (HLSL→GLSL, Typ-Inferenz/Promotions, #define, Funktionen, Casts; Korpus warp 462/574 · comp 409/598, Rest = C3), Host-Integration — include.fx-Gegenstück als GLSL-Präambel, per-Preset-Programme (Warp ersetzt Decay-Blit, Comp ersetzt MD1-Composite), Sampler-Objekte fc/pc/fw/pw, Noise-Platzhalter (exakter Port = C2), q1–q32/rand/roam/Blur-Uniforms, stiller MD1-Fallback bei GL-Fehler; Blur-Pyramide läuft jetzt auch für Custom-Shader-Konsumenten |
