# MilkDrop-Import — Status-Übersicht (SSOT für Fortschritt + Bezeichnungen)

> **Stand:** 2026-07-22 (Session 41) · **Zweck:** EIN Ort für „was ist fertig,
> was ist offen, wie heißt es" — die Detail-Konzepte bleiben in
> [MilkDrop_Import_Konzept.md](MilkDrop_Import_Konzept.md).
> Dieses Dokument wird am Ende jeder Session nachgezogen.

## 1. Bezeichnungs-Legende (verbindlich)

| Schema | Bedeutet | Beispiele |
|---|---|---|
| **M1–M6** | Meilensteine der Import-Roadmap (Konzept §5) | M5 = Blur + Shader-Stufe B; `M6.1` = Teilschritt 1 von M6 |
| **Stufe A/B/C** | Shader-Strategie (Konzept §4) — WIE Shader gerendert werden | A = MD1-Pfad · B = erkannte Default-Familie · C = HLSL→GLSL-Transpiler |
| **C1/C2/C3** | Ausbaustufen der Stufe C | C1 = Transpiler-Kern · C2 = Noise+Texturen · C3 = Loops/Arrays/tex3D |
| **N1/N2** | Node-Integration (aus Entscheid E1; hieß im Chat kurz „B1/B2" — umbenannt wegen Kollision mit Stufe B) | N1 = Milkdrop als Chain-Node · N2 = Panel-Baum + Standalone-Entfernung |
| **E1–E8** | Entscheide Session 40 (Konzept §6b) — keine Phasen | E5 = Crossfade-Verfahren |

Es gibt **kein** „A#"- oder „D#"-Schema. Stufe A ist seit M3/M4 fertig.

## 2. Erledigt ✅

| Was | Session | Verifikation |
|---|---|---|
| **M1** MilkParser-Lib (.milk → Struktur, MD3-Superset) | 39 | Korpus 910/910 |
| **M2** Milk-Skript-Vertrag (q/t-Snapshots, MilkLoudness, symbolCategory) | 39 | Golden-Tests |
| **M3** Render-Kern MD1 (**= Shader-Stufe A**: Warp-Mesh, decay, Wave 0–7, Borders, MD1-Composite) | 39 | Sichttest BESTANDEN (S39) |
| **M4** Custom Waves/Shapes (bis 16), Motion Vectors | 39 | Sichttest BESTANDEN (S39) |
| **M5** Blur-Pyramide + **Shader-Stufe B** (Klassifikation Md1Default/Md1Plus, baked Konstanten, Blur-Layer) + Kalibrier-Raster `render.debugGrid` | 40 | Sichttest BESTANDEN (S40); Korpus-Gates |
| **M6.1** .lvfx-Schwester-Persistenz (MilkdropSerializer, Load-Dispatch, Save nach Host) | 40 | Roundtrip-Tests (Kalibrier-Korpus 26/26); UI-Durchstich-Sichttest offen |
| **Stufe C1** Lib HlslTranspiler + Shader-Laufzeit im Host (Präambel, per-Preset-Programme, Fallback) | 40 | Korpus warp 462/574 · comp 409/598; **GL-Sichttest offen** |
| **Stufe C2** exakter Noise-Port + Custom-Textur-Lader (randNN, texsize_, Platzhalter+Report) | 40 | Build/Suite grün; **GL-Sichttest offen** |
| **E1–E8** Entscheide (Konzept §6b) | 40 | dokumentiert |
| Nebenlieferungen S40: Import-Browser-Persistenz + Settings-Reset · EelScriptEditing.hpp-Extraktion (Basis für N2) · Origin-Icons nutzbar | 40 | Suite 388/388 |
| **C1/C2-Befund GELÖST:** Regression aus dem C2-Umbau (Commit 2ba48a3) — die tryTranspile-AUFRUFE in prepareCustomShaders fehlten → Custom-GLSL blieb leer, stiller MD1-Fallback. Fix + Regressions-Gate im Ladepfad + Diagnose-Trace (`MilkdropTrace.hpp` → `%TEMP%/lumiviz_milkdrop_trace.log`) + Standalone-Testprogramm `MilkdropStandalone` (`--auto`: 8/8 c1 custom, Screenshots + Pixel-Statistik, 02 invertiert / 07 Textur aufrecht verifiziert) | 41 | Suite 391/391; Standalone-Beweislauf Exit 0; **In-App-Sichttest offen** |
| **N1 Chain-Node ✅ (Code):** `MilkdropNodeParams` (PresetState eingebettet + presetDir + meshX/Y + debugGrid + Revision) im EffectParams-Variant, `runMilkdropNode` (per-nodeId-Runtime, MilkdropVisualizer als Engine, Composite-Ziel = beim Kern-Frame-Start gebundenes FBO → Chain-Buffer), ChainSerializer-Key `"milkdrop"` (Preset-JSON via MilkdropSerializer, Klassifikation beim Laden neu abgeleitet) | 41 | Suite 394/394 (Roundtrip, Clamps, loadMilkFile→Node); **GL-Sichttest offen** |
| **N2 Panel + Routing ✅ (Code):** MultiEffect-Panel-Baum mit Anzeige-Kindern (Preset → Code · Waves · Shapes · Shader, Sentinel-Pfade), Property-Editor je Sektion (EelScriptEditing; Shader-Edit reklassifiziert), Palette-Eintrag „Milkdrop"; MainWindow routet .milk + milkdrop-.lvfx auf MultiEffect+Node (`loadMilkFile`/`loadMilkDocument`, Report-Parität über GL-freie Probe-Instanz), Save immer über Chain-Serializer; **Standalone-Registry-Eintrag entfernt (E2)** — Klasse bleibt Node-Engine + Testprogramm-Basis | 41 | Builds VS-Debug/Testing + Ninja-Clang-Release grün; **UI-Sichttest offen** |

## 3. Offen ⬜ (verbindliche Reihenfolge aus §6b)

| # | Was | Gehört zu | Notizen |
|---|---|---|---|
| 0 | **Sichttest-Runde N1/N2 + c1 (In-App):** c1-/m5-Presets über den neuen Node-Pfad (Import-Browser → MultiEffect+Node) sichten; Panel-Baum + Editor-Sektionen prüfen; Standalone-Werkzeug: `MilkdropStandalone --auto` (Kern-Pfad, ohne Chain) | N1/N2, C1/C2 | Kalibrier-READMEs in `asset/calibration/milkdrop/{c1,m5}/`; Trace-Log `%TEMP%/lumiviz_milkdrop_trace.log` läuft mit |
| 3 | **Sprites** (85 Presets) | M-Kür = Pflicht (E7) | Textur-Lader aus C2 vorhanden |
| 4 | **Crossfade** = echtes Doppel-Rendering, Freeze-Frame nur als Performance-Fallback; Ausbaustufe per-Vertex-Warp-UV-Blend | M6 / E5 | zwei Laufzeitzustände (2 Contexts/Feedback/Blur) |
| 5 | **Visual-Playlist** + Hotkeys (schaltet Chains + Milkdrop einheitlich, nutzt Crossfade) | M6 / E6 / P2 | Konzept: ui/Visual_Playlist_Konzept.md |
| 6 | **Stufe C3**: for/while, Arrays, tex3D + Volumen-Noise (noisevol), #if, out-Parameter, Vektor-Vergleiche | C3 | hebt die Korpus-Reste (~110 warp / ~190 comp) |
| 7 | **fShader-Farbwash** (`hue_shader` ist aktuell neutral 1.0) | M-Kür (E7) | kleiner Baustein, MD1 + Präambel |
| 8 | **Decay-Dither** · **.milk-Export** | M-Kür = Pflicht (E7) | ganz ans Ende |
| 9 | **E8-Altpunkte, gebündelt + Kalibrier-Runde:** Wormhole-Bisektion gegen `c8d2bd2` · Sichttests §7/§8 · Set Render Mode→alle Scopes (P1) · Skript-SSOT modul×slot (P3) · Kleinkram S31 | E8 | bewusst ganz am Schluss |

**Außerhalb der MilkDrop-Roadmap notiert:** Standalones→Module (Equalizer/
Oszi/Pulsing), dynamische Modulparameter, Custom-Functions-Modul,
Video-Capture, en-Übersetzungen, App-Umbenennung MyViz→LumiViz.

## 4. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 40): Legende (inkl. Umbenennung B1/B2→N1/N2), Erledigt-/Offen-Tabellen |
| 1.1.0 | 2026-07-22 | Session-Abschluss 40: C1+C2 als Code-✅ eingetragen, Punkt 0 = C1/C2-Befund (Standalone-Entscheid); c1-Kalibrier-Satz + GL-Smoke-Test ergänzt |
| 1.2.0 | 2026-07-22 | Session 41: **C1/C2-Befund gelöst** (tryTranspile-Regression aus 2ba48a3; Trace + Regressions-Gate + MilkdropStandalone-Beweislauf 8/8) · **N1+N2 Code ✅** (Milkdrop = Chain-Node im MultiEffect-Host, Panel-Baum mit Sektions-Kindern, Import-/Save-Routing, Standalone-Registry-Eintrag entfernt) · Offen-Punkt 0 = Sichttest-Runde N1/N2 + c1 |
