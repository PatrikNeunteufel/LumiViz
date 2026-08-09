# Import-Treue — Befund & Fixplan (Session 38)

> **Status (Nachtrag S52):** **Archiv.** Alle Fixplan-Schritte 1–6 sind umgesetzt
> (§3b). Die Treue-Arbeit läuft seit Session 44 über die Kalibrier-Runde gegen
> `AvsRef` — Protokoll: [AVS_Sichttest_Protokoll.md](AVS_Sichttest_Protokoll.md),
> Methodik: [AVS_Kalibrier_Methodik.md](AVS_Kalibrier_Methodik.md), offene Punkte:
> [Offene_Punkte.md](../Offene_Punkte.md). Der `kSpecGain`-Wert unten (12) ist
> überholt — seit S38/S44 gilt **8**, hergeleitet aus dem Winamp-Spektrum-Vertrag.
>
> **Stand:** 2026-07-22 · **Quelle:** Preset-Diagnose EyeCandy2 (`asset/avs/greatwho2006/
> 4resample/EyeCandy2`) + Statistik über alle 612 Presets in `asset/avs/**` +
> Code-Vergleich gegen die Referenz `../ref/vis_avs` (BSD-3, Nullsoft 2005).
> **Anlass:** Presets rendern teils schwarz/blass/bewegungsarm, obwohl alle
> Effekte „importierbar" sind (Abdeckung laut `Import_Modul_Abdeckung.md` ✅).

## 0. Methodik

- Alle `.avs` mit einem Python-Dumper (Spiegel von `AvsParser.hpp`/`r_list.cpp`)
  strukturell zerlegt: Effektbaum, Listen-Modi/Blends, EEL-Slots.
- Ergebnis: **612 Dateien, 0 Parse-Fehler** — der Parser ist nicht das Problem.
- Drei Vergleichsstränge gegen `ref/vis_avs/avs/vis_avs/`: Listen-Blends
  (r_list.cpp), Pixel-Effekte (r_*.cpp), Skript-Semantik (r_sscope/r_dmove/evallib).
- Kernmuster der Presets: Bewegung kommt fast ausschließlich aus
  `beat: ti=getspec(0.5…0.8,…)` + `frame: t=t-ti` — ohne korrekte
  getspec-Werte und Beat-Ereignisse steht das Bild.

## 1. Befunde

### Cluster A — Audio-Antrieb (betrifft praktisch jedes Preset)

| # | Schwere | Befund | LumiViz | Referenz |
|---|---------|--------|---------|----------|
| A1 | — | **ENTFÄLLT (verifiziert korrekt):** `BASS_DATA_FFT_INDIVIDUAL` multipliziert laut BASS-Doku (bass.chm, BASS_ChannelGetData) die Datenmenge ×Kanäle → FFT1024 stereo = 512 Bins/Kanal interleaved; `binsPerCh=512` in MainWindow stimmt. Damit ist die offene „FFT-Layout-Annahme" aus Session 37 POSITIV verifiziert (Layout korrekt) | `MainWindow.cpp:716ff` | BASS-Doku FFT_INDIVIDUAL |
| A2 | HOCH | **Log-Kennlinie fehlt:** Original mappt jedes Spektrum-Byte durch `g_logtab` = `log(x·60/255+1)/log(60)·255` (aus 10 % werden 47 %); LumiViz speist lineare BASS-Magnituden → getspec um Größenordnungen zu klein | `MultiEffectVisualizer.cpp buildVisData` | `main.cpp:242-249` |
| A3 | HOCH | **Beat-Detektor:** Original = Betragssumme Waveform ≥ Peak-Tracker×34/32, Floor 576·16, Schwellen-Anhebung nach Beat, Refire-Guard, dann `refineBeat()`; LumiViz = RMS×1.5 über trägem Mittel, ohne Flanke (Mehr-Frame-Bursts), `BeatEstimator::refine()`-Rückgabe wird VERWORFEN | `BeatModule.hpp:72-77`, `MultiEffectVisualizer.cpp:1904-1906` | `main.cpp:290-329` |

Sichtfolgen: „kaum/zu langsame Bewegung", „dauert bis es losgeht",
„Richtungswechsel-Zähler kippen nie" (Stargate `rc`/`di`).

### Cluster B — Skript-Semantik (SuperScope / Dynamic Movement)

| # | Schwere | Befund | LumiViz | Referenz |
|---|---------|--------|---------|----------|
| B1 | HOCH | **SuperScope `n`** wird jeden Frame VOR beat/frame auf den fixen `pointCount` (Default 256) zurückgeschrieben; Init-`n=800` gilt nie, Beat-`n` flackert | `SuperscopeModule.cpp:486` | `r_sscope.cpp:210,281` |
| B2 | HOCH | **DM `alpha`** wird berechnet, aber verworfen (Warp-Shader schreibt `vec4(rgb,1.0)`); Original: BLEND_ADJ des verschobenen Pixels mit dem Original | `ScriptGridModule.cpp:193-196`, `MultiEffectVisualizer.cpp:2464-2467` | `r_dmove.cpp:358-362,526-535` |
| B3 | HOCH | **DM-Flags `blend`/`nomove`/`subpixel`** fehlen im Param-Struct — DM ersetzt immer | `EffectChain.hpp:390-400` | `r_dmove.cpp:91,526-543` |
| B4 | MITTEL | `drawmode`/`linesize` skriptgesteuert ignoriert; **Farb-Vorbelegung pro Punkt** (Gradient) statt frame-konstant aus gecycleter Farbtabelle | `SuperscopeModule.cpp:644-666` | `r_sscope.cpp:265-270,298,325` |

Bestätigt KORREKT: Var-Persistenz (Slots/Punkte/Frames), Init-einmal, EEL-Inventar
(`bnot/if/above/below/equal/rand/atan2/pow/sqr/…`), `sqrt(neg)=√|x|`, `rand(x)`=int,
DM-Polar (d Ecke→1, r+π/2), wrap, noframe-Variante, Div/0-Parität (beide inf).

### Cluster C — Pixel-Effekte & Listen

| # | Schwere | Befund | LumiViz | Referenz |
|---|---------|--------|---------|----------|
| C1 | HOCH | **Bump multiplikativ statt additiv** (`orig·bright/255` statt `orig+coul`) → dämpft Bild Richtung Schwarz | Shader `MultiEffectVisualizer.cpp:465-468` | `r_bump.cpp:324-331` |
| C2 | HOCH | **Clear Screen:** `blend`/`blendavg` verworfen → immer harter Schwarz-Clear; zerstört Trails/Feedback | `AvsChainTranslator.cpp:422-426` | `r_clear.cpp:121-124` |
| C3 | HOCH | **Buffer Save: Blend 1↔2 vertauscht** (Original 1=50/50, 2=additiv); zudem `dir` 2/3 (frameweise save/restore-alternierend) nicht abgebildet, Blends 3–11 fehlen | `AvsChainTranslator.cpp:833-836` | `r_stack.cpp:124-231` |
| C4 | MITTEL | **Movement builtin: 32×24-Gitter** statt Pro-Pixel-Tabelle — hochfrequente Formeln (`cos(r·32)`) unterabgetastet; `sourcemapped` fehlt | `MultiEffectVisualizer.cpp:2400` | `r_trans.cpp:296-647` |
| C5 | MITTEL | **Mirror:** 4 Richtungs-Bits → 2 Bools kollabiert (V2/H2 falsche Hälfte); Smooth-Transition (`BLEND_ADAPT` über `slower`) fehlt | `AvsChainTranslator.cpp:407-410`, `EffectChain.hpp:328-334` | `r_mirror.cpp:65-68,152-257` |
| C6 | HOCH | **Effect List `beat_render`:** „enabled aus + on-beat an" (Original `fake_enabled`) rendert in LumiViz NIE (statischer enabled-Check vor OnBeat-Logik); enabled-Liste mit beatRender wird fälschlich gegated | `MultiEffectVisualizer.cpp:1970,2106-2110` | `r_list.h:162`, `r_list.cpp:361-362` |
| C7 | KLEIN | Starfield immer additiv (Blend-Flags ignoriert) · Set Render Mode setzt bei `!enabled` lineBlend=1 statt „unverändert" · Listen-EEL `alphain/alphaout` 0..255 statt 0.0..1.0 | diverse | `r_stars.cpp:245`, `r_linemode.cpp:96-104`, `r_list.cpp:404-418` |

Bestätigt KORREKT: komplette Listen-Blend-Tabelle 0–13 (inkl. Adjustable,
Buffer-Blend+Invert), `mode`-Decode (`^1`), clear-Semantik, Root-Fast-Path,
Fadeout (16/255), Fast Brightness, Blur-Kernel, Water-Formel, Unique Tone,
Simple-Bitfeld, Movement-Blob-Decode + 24er-Builtin-Tabelle.

### Cluster D — Abdeckung (Statistik über 612 Presets)

Top-Nutzung: SuperScope 2605 · Moving Particle 903 · DM 525 · Blur 524 ·
Bump 377 · Buffer Save 366 · Movement 336. Alle vorkommenden APEs implementiert
**außer `FunkyFX FyrewurX v1`**: bisher ✖ (closed-source), aber in **68 Presets
(11 %)** enthalten (Spitze: greatwho2006_2 26/108). **Entscheid Patrik 2026-07-22:
nachbauen** (Verhalten rekonstruieren, kein Code — audio-reaktive
Feuerwerks-Partikel). Rest-Lücken sind Einzelfälle (Gnosisoft Lyrics 1×,
VisFrac 1×, Movin' Lyrics — bleiben ✖).

## 2. Symptom-Zuordnung EyeCandy2

| Preset | Symptom (Patrik) | Ursachen |
|--------|------------------|----------|
| 01_fractal Dreams | kaum Textur/Bewegung | B1 (n=800→256), A1–A3 (ti), B2 (alpha), C3 (Buffer Save) |
| 02_color extasy | dito, zu hell | B2 (alpha-Layering fehlt), B1, A1–A3 |
| 02_flowers | fast schwarz, Aufflackern | C1 (Bump), A3 (Beat-Bursts → dmx/dmy-Flips) |
| 04_rings | kaum Movement | A1–A3 |
| 05_wormhole | Movement gut, Textur fehlt | C1 (Bump), C5 (Mirror), B2 (alpha), C3 |
| 06_Stargate | Rotation kehrt nie | A3 (rc/di-Zähler), A1/A2 (getspec·5 bleibt < Schwelle) |
| 07_movin wall | keine Textur, kaum Movement | B1 (n=50), A1–A3 |
| 08_noname | fast schwarz, rote Flecken | C2 (Clear Screen), C1 (Bump), B2 |
| 09_rotating_things | zu langsam, pink, Streifen | A1–A3 (Tempo), B1 (n-Flackern), B4 (Farbbasis); Streifen = Movement-Code `x=0` (gewollt) |
| 10_the ring | nur türkis | B2 (alpha in beiden DMs), A1–A3; Türkis = Scope 1 + Movement `x=0`-Schmier (gewollt) |

## 3. Fixplan (Reihenfolge = Umsetzung)

1. **A2** — Log-Kennlinie in `buildVisData` (exakte g_logtab-Formel, mit
   Gain-Vorstufe als Kalibrierpunkt). Größter Hebel. (A1 entfiel nach
   BASS-Doku-Verifikation.)
2. **A3** — AVS-Beat-Detektor (main.cpp-Algorithmus) auf die Waveform portieren;
   `m_frameBeat = m_beatEstimator.refine(onset, …)` (Rückgabe nutzen).
3. **C3** Buffer-Save-Blend-Swap + dir 2/3 · **C1** Bump additiv ·
   **C2** ClearParams um blend/blendavg erweitern + Render.
4. **B1** `n` nur beim (Re-)Compile seeden · **B2** alpha in den Warp-Pfad ·
   **B3** blend/nomove(/subpixel) abbilden.
5. **C6** `fake_enabled`-Semantik · **C5** Mirror-Bits + smooth ·
   **C4** Movement-Auflösung · **C7/B4** Kleinkram.
6. **D** FyrewurX-Nachbau (host-natives Modul, 5-Schichten-Rezept).

Jeder Schritt: Suite grün (`ctest -R UnitTests`), /WX-Build; Sichttest-Punkte
sammeln in [`docs/Offene_Punkte.md`](../Offene_Punkte.md) *(bis S52:
`Offene_Sichttests.md`)*.

## 3b. Umsetzungsstand (Session 38, 2026-07-22)

**Alle Fixplan-Schritte 1–6 umgesetzt**, Suite 305/305 grün, VS-Testing +
VS-Debug bauen:

- **A2** ✅ g_logtab-Kennlinie in `buildVisData` (`kSpecGain = 12` =
  Kalibrierpunkt). **A1** entfiel (BASS-Doku bestätigt Layout).
- **A3** ✅ `BeatModule::updateAvsOnset` (main.cpp-Port, 3 neue Tests) +
  `m_frameBeat = m_beatEstimator.refine(...)`.
- **C3** ✅ Buffer Save: dir 0–3 (alternierend, `bufDirCh`), volle
  r_stack-Blend-Tabelle 0–11 (1=50/50! 2=additiv!), Blend auch beim SPEICHERN
  (Scratch-SurfacePair); Param `save` → `dir` (Serializer liest legacy `save`).
- **C1** ✅ Bump-Shader additiv (`min(orig+bright, 254/255)`).
- **C2** ✅ Clear Screen: Modus 0–3 (Replace/Additiv/50-50/Line-Blend).
- **B1** ✅ SuperScope-`n` persistiert (Seed nur bei Compile/UI-Änderung);
  Klemme min 1 (n=2-Linien); Import-Default n=100; Weiß-Default-Farbtabelle
  wenn Preset keine Farben hat (r_sscope-ctor).
- **B2/B3** ✅ DM-`alpha` in den Warp-Pfad (Vertex-Attribut + Shader-Mix),
  alpha-Seed 0.5 einmalig (persistent, nicht pro Punkt); Flags `blend`/`nomove`.
- **C6** ✅ Listen-`fake_enabled` (disabled+beatRender rendert im Beat-Fenster;
  enabled-Listen nicht mehr übergated).
- **C5** ✅ Mirror: 4 Richtungs-Bits 1:1 + Smooth-Rampe (16 Stufen je `slower`
  Frames); Ein-Pass-Approximation (Abweichung nur bei gleichzeitig
  gegenläufigen Richtungen, im Shader kommentiert).
- **C4** ✅ Movement: 96×72-Gitter, Feld statisch gecacht (nur bei
  Compile/Resize neu skriptet); `blend`-Flag (50/50 via alpha-Default).
- **C7** ✅ Starfield-Blend (Replace/Additiv/50-50) · Set Render Mode lässt
  Blend bei `!enabled` unverändert · Listen-EEL `alphain/alphaout` in 0..1.
- **B4** ✅ `drawmode`/`linesize` skriptbar (Frame-Level-Readback; pro-Punkt-
  Umschalten bewusst nicht — ein Primitive-Batch je Frame).
- **D** ✅ FyrewurX-Nachbau (`FyrewurXParams`, Beat-Bursts + Gravitation,
  additiv; Parser-Decode `enabled`+`config`; Palette „Scopes & Sources").

**Bewusst offen (zur Absegnung):** ~~DM `subpixel` · DM `buffern` · Movement
`sourcemapped` · SuperScope pro-Punkt-`drawmode`~~ — **alle vier am 2026-07-22
freigegeben und in Runde 2 umgesetzt (§3c).**

## 3c. Runde 2 (Session 38, nach Patriks Sichttest)

**Sichttest-Feedback:** Mehrheit besser; Regression 05_wormhole (Tunnelform/
Drehbewegung schlechter); Community Picks teils zu hell (Vollflächen orange/
blassgelb), teils schwarz, teils „fehlt was".

**Diagnose (Dump-Analyse):**
- Wormhole-Tunnel-DM (Jheriko) hat `blend=1` + **`buffern=1`**: seit
  DM-alpha aktiv ist, wurde mit dem FALSCHEN Quellbild geblendet (buffern
  fehlte). Gleiches Muster in „alien intercourse 4" (Scopes rendern mit out=0
  NUR in einen Buffer — ohne buffern bleibt das Bild schwarz), „Alienated",
  „el-vis golden".
- **AVS wertet `xres+1` DM-Gitterpunkte aus** (min 2): Presets mit xres=0
  bekamen 16 statt 2 → falsche Warp-Geometrie (Alienated, Ex Deux, Data flow).
- „Helium" schwarz: Inhalt = Texer II mit fehlendem BMP → Original zeichnet
  seinen Default-Punkt-Sprite, LumiViz zeichnete nichts.
- Vollflächen-Sättigung: LumiViz-Line-Blend-Default war ADDITIV; AVS'
  `g_line_blend_mode` startet als REPLACE.
- `&`-Operator und `$pi` (Geometric Sustinance / Data flow) sind im
  Transpiler vorhanden — kein Befund.
- **sourcemapped-Messung: 4 von 336 Movement-Instanzen (4 Presets, 0,65 %).**

**Umgesetzt (alle, Suite 308/308 grün):**
1. **DM `buffern`** — Warp-Quelle aus Global-Buffer (Pool-Textur an
   `uSrcTex`); fehlender Buffer = Passthrough (r_dmove.cpp:290); `nomove`
   blendet Buffer alpha-gesteuert ein.
2. **DM/Movement `subpixel`** — GL_NEAREST-Sampling bei aus (nach Draw auf
   LINEAR zurückgestellt).
3. **DM `xres+1`-Semantik** im Translator (0 → 2, 31 → 32).
4. **Movement `sourcemapped`** — Scatter-Approximation: inverses Warp-Mesh
   (Position=Ziel, Texcoord=Quelle) unter GL_MAX-Blending auf Schwarz bzw.
   Bildkopie (blend); Beat-Toggle-Bit als Runtime-Zustand. GPU-Näherung:
   gestreckte Dreiecke füllen, wo das Original Lücken ließe (Sichttest).
5. **SuperScope pro-Punkt-`drawmode`** — Point-Code-Readback je Punkt,
   Host splittet in Lines-/Dots-Runs (Lines-Run behält den Anschlusspunkt).
6. **Line-Blend-Default → REPLACE** (Host-RenderMode + Import-Scopes);
   Set Render Mode schaltet weiterhin um.
7. **Texer/Texer II Default-Sprite** (weicher weißer Punkt) bei fehlendem/
   kaputtem Bild — Helium & Co. zeigen wieder Inhalt.

## 4. Verifikations-Notizen

- getspec-Erwartungswerte des Engine-Tests bleiben gültig (getvis-Port selbst
  ist AVS-treu; A2 betrifft die ZUFÜHRUNG, nicht die Funktion).
- A2-Kennlinie als reine Funktion testbar; der Gain vor der Kennlinie ist
  Sichttest-Kalibrierpunkt.
- Beat-Detektor (A3): Port als eigenes Modul testbar (synthetische
  Energie-Folgen → erwartete Beat-Frames, Refire-Guard, Schwellen-Anhebung).
