# MilkDrop-Import — Status-Übersicht (SSOT für Fortschritt + Bezeichnungen)

> **Stand:** 2026-07-23 (Session 42) · **Zweck:** EIN Ort für „was ist fertig,
> was ist offen, wie heißt es" — die Detail-Konzepte bleiben in
> [MilkDrop_Import_Konzept.md](MilkDrop_Import_Konzept.md).
> Dieses Dokument wird am Ende jeder Session nachgezogen.

## 1. Bezeichnungs-Legende (verbindlich)

| Schema | Bedeutet | Beispiele |
|---|---|---|
| **M1–M6** | Meilensteine der Import-Roadmap (Konzept §5) | M5 = Blur + Shader-Stufe B; `M6.1` = Teilschritt 1 von M6 |
| **Stufe A/B/C** | Shader-Strategie (Konzept §4) — WIE Shader gerendert werden | A = MD1-Pfad · B = erkannte Default-Familie · C = HLSL→GLSL-Transpiler |
| **C1/C2/C3** | Ausbaustufen der Stufe C | C1 = Transpiler-Kern · C2 = Noise+Texturen · C3 = Loops/Arrays/tex3D |
| **N1/N2/N3** | Node-Integration (aus Entscheid E1; hieß im Chat kurz „B1/B2" — umbenannt wegen Kollision mit Stufe B) | N1 = Milkdrop als Chain-Node · N2 = Panel-Baum + Standalone-Entfernung · N3 = Panel-Vollausbau (Add/Remove Waves/Shapes, numerische Parameter, Sprites) |
| **HG1–HG3** | Host-Gruppen + Crossfade — Umsetzungsschnitte aus [HostGruppen_Crossfade_Entwurf.md](HostGruppen_Crossfade_Entwurf.md) (Session 42, zur Freigabe) | HG1 = Node-Typ/.lvfx2 · HG2 = Crossfade-Mix · HG3 = Fallback/progress |
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
| **HG1 ✅ (Code): Host-Gruppen-Fundament** — Node-Typ `HostGroupParams` im EffectParams-Variant (children wie Liste, `isContainer()`); `renderHostGroup`: persistenter Gruppen-Buffer OHNE per-Frame-Clear (Feedback) + **Runtime-Trennung** (eigener OffscreenBufferPool + eigener ScriptContext je Gruppe via `activePool()`/`activeContext()`-Scope-Switch); Tiefenregel dreifach (Compile-Pass degradiert verschachtelte Gruppen zur Liste + Panel-Add-Guard + Drag&Drop-Guard); Serializer-Key `"hostgroup"`, **Save wählt `.lvfx2`** sobald eine Gruppe in der Kette ist (Load/Import-Browser können beide); Panel: Palette „Host Group", Editor (Blend Out, Crossfade-Dauer mit **Sync über alle Gruppen**, individuelle Ein-/Ausgangskurven [linear], `.lvfx-in-Gruppe`-Import mit frischen nodeIds) | 42 | Builds grün; neue Gates: hostgroup-Roundtrip + Tiefenregel-Degradierung; **In-App-Sichttest offen** (Punkt 0). Merkposten: verschobene Nodes behalten ihren Erzeugungs-ScriptContext bis zur Runtime-Neuerzeugung; Buffer-Blend als Gruppen-blendOut ohne Slot-Feld |
| **N3.2 ✅ (Code): numerische Editoren** — neue Baum-Sektion „Parameter (Basiswerte)" mit der kompletten Preset-Fläche (General/Composite inkl. fShader, Basis-Waveform, Motion/Warp, Borders, Motion Vectors, Blur-Pyramide; Member-Pointer-Setter über milkOf/Revision) + numerische Init-Parameter in den Wave-/Shape-**Einzel-Ansichten** (Samples/sep/Flags/Skalierung/Farben bzw. Seiten/Instanzen/Textur/Geometrie/3 Farbsätze); Hinweise: Startwerte-vs-per_frame + Baked-Vertrag bei Comp-Shadern | 42 | Builds + Suite grün; **In-App-Sichttest offen** (Punkt 0) |
| **Session A ✅ (Code): N3.1 + N3.3 + fShader-Wash** — Panel: Waves/Shapes/Sprites als Element-Items im Baum, Add über Palette-Einträge „Custom Wave/Custom Shape/Sprite" (+ „+", Cap 16, kleinster freier index), Remove/Clone über Element-Selektion, Sprite-Editor mit allen Startwerten + per-Frame-EEL (Revision-Vertrag durchgängig); Renderer: fShader-Farbwash-Port (milkdropfs.cpp:4033-4062/4311-4389 — 4-Ecken-Regenbogen mit Zufalls-Phasen, MD1 = Amount-Mix gegen Weiß in allen Composite-Passes, baked = ci.hueMix, Custom-Comp = Roh-Ecken bilinear als `hue_shader`, Warp bleibt weiß); Kleinfixes: mutate()-Labelguard für Sentinel-Items, Report-Texte („Platzhalter bis C2" korrigiert, hue-Hinweis jetzt ℹ), /bigobj für MultiEffectPanel.cpp (C1128) | 42 | Builds grün; **In-App-Sichttest offen** (Punkt 0) |
| **Sprites ✅ (Code + Standalone-Sichtnachweis):** MilkDrop2077-`[SPRITEn]`-Sektionen → `SpriteState` (Translator, Layer-Sortierung, Blend-Klemme 0..4), Port von `DrawUserSprites` (milkdropfs.cpp:3432-3777): private EEL-VM je Sprite (texmgr-Modell), Referenz-Vertex-Mathe (y-abwärts, finale GL-Negation), 5 Blend-Modi, Colorkey→Alpha beim Laden, **burn-in in den Feedback-Buffer**, done-Kill; Persistenz im MilkdropSerializer; Kalibrier-Satz `s1/` (3 Presets + README). PORT-Annahmen: SpriteSpeed = time-Skalierung, SpriteLayer = Sortier-Schlüssel (2077 ohne Quelle) | 41 | Suite 397/397; Standalone-Screenshots: 01 aufrecht/zentriert, 02 Rotation+Colorkey+Layer, 03 Burn-Spuren; **In-App-Sichttest BESTANDEN** (S41, Node-Pfad; s1-Presets dabei um `mv_a=0` ergänzt) |

## 3. Offen ⬜ (verbindliche Reihenfolge — Entscheid Patrik 2026-07-23, Session 42)

| # | Was | Gehört zu | Notizen |
|---|---|---|---|
| 0 | **Sichttest-Runde N1/N2 + c1 + m5 + Session A (In-App):** c1-/m5-Presets über den neuen Node-Pfad (Import-Browser → MultiEffect+Node) sichten; Panel-Baum + Editor-Sektionen prüfen — **jetzt inkl. Session-A-Features:** Wave/Shape/Sprite anlegen/entfernen/klonen, Sprite-Editor, fShader-Wash (Preset mit `fShader=1` sichten). Standalone-Werkzeug: `MilkdropStandalone --auto [ordner]` (Kern-Pfad, ohne Chain). *(s1/Sprites: BESTANDEN in S41)* | N1/N2, C1/C2, N3.1/N3.3/E7 | Kalibrier-READMEs in `asset/calibration/milkdrop/{c1,m5}/`; Trace-Log `%TEMP%/lumiviz_milkdrop_trace.log` läuft mit |
| 4 | **HG2 Crossfade-Mix:** Zustandsmodell, Bild-Mix mit **individuellen Ein-/Ausgangskurven je Gruppe** (out-Kurve(A) × in-Kurve(B); Wechsel-Settings synchron mit UI-Hinweis) | E5 / HG2 | echtes Doppel-Rendering (E5), beide audio-live; Fundament (HG1) steht |
| 5 | **HG3:** Freeze-Frame-Performance-Fallback + `progress`-Anbindung (löst den 60-s-Stub ab) | E5 / HG3 | |
| 6 | **Visual-Playlist** + Hotkeys (schaltet Host-Gruppen einheitlich, nutzt HG2-Crossfade) | M6 / E6 / P2 | Konzept: ui/Visual_Playlist_Konzept.md |
| 7 | **Stufe C3**: for/while, Arrays, tex3D + Volumen-Noise (noisevol), #if, out-Parameter, Vektor-Vergleiche | C3 | hebt die Korpus-Reste (~110 warp / ~190 comp) |
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
| 1.3.0 | 2026-07-22 | Session 41: **Sprites ✅** (E7-Pflichtkür, Punkt 3) — MilkDrop2077-Sektionen → SpriteState + DrawUserSprites-Port (private EEL-VM, 5 Blend-Modi, Colorkey, burn-in, done-Kill), Serializer-Persistenz, Kalibrier-Satz s1 (3 Presets), Standalone-Sichtnachweis; Suite 397/397. Nächster Punkt: Crossfade (E5) |
| 1.3.1 | 2026-07-22 | Session-Abschluss 41: **s1-Sichttest in-app BESTANDEN** (Node-Pfad; Orientierung/y-Konvention/Rotation/Layer/Colorkey/Burn korrekt); s1-Presets um `mv_a=0` ergänzt (Motion-Vector-Default 1.0 störte den Sichttest — Preset-Autorenschaft, kein Port-Fehler); Offen-Punkt 0 = Sichttest c1+m5 |
| 1.4.0 | 2026-07-22 | Session 42: **N3 Panel-Vollausbau** als Offen-Punkt 3b aufgenommen (Befund: Panel editiert nur Skripte/Shader/enabled — keine Add/Remove-Aktionen, keine numerischen Parameter, keine Sprites); **E5 auf Host-Gruppen generalisiert** (Entscheide Patrik: neuer Node-Typ, `.lvfx2` sobald Host-Gruppe enthalten, Crossfade-Settings je Host-Modul mit Sync+Hinweis, Playlist global, Tiefenregel 1 Ebene) — Entwurf HostGruppen_Crossfade_Entwurf.md 1.0.0 zur Freigabe; Legende um N3 + HG1–HG3 ergänzt |
| 1.5.0 | 2026-07-23 | Session 42: **Reihenfolge-Entscheide Patrik** — Offen-Tabelle neu geordnet: N3.1+N3.3+fShader-Kür (Session A) → N3.2 (klassische Editoren JETZT, nicht aufs dynamische Parametermodell warten) → HG1→HG2→HG3 → E6-Playlist → C3 → Dither/Export → E8. fShader-Wash aus „nach C3" vorgezogen. Entwurf 1.2.0: Host-Gruppen-Anzahl unbegrenzt (mehrere gleichzeitig aktiv), Ein-/Ausgangskurven je Gruppe individuell |
| 1.6.0 | 2026-07-23 | Session 42: **Session A umgesetzt (Code ✅)** — N3.1 Add/Remove/Clone für Waves/Shapes (Palette-Einträge + Element-Items im Baum), N3.3 Sprite-Sektion mit Voll-Editor, fShader-Farbwash-Port (MD1 + baked + Custom-Comp; Warp weiß). HG-Entwurf **freigegeben** (v1.2.0). Offen-Punkt 0 um Session-A-Sichttest erweitert. Nebenbei: Qt-ADS-Pin 4.4.1→5.0.0 (Dock-Undock/Redock-Bugs, s. DockManager.md 2.1.0) |
| 1.7.0 | 2026-07-23 | Session 42: **N3.2 umgesetzt (Code ✅)** — Baum-Sektion „Parameter (Basiswerte)" (komplette numerische Preset-Fläche in 6 Gruppen) + Wave-/Shape-Init-Parameter in den Einzel-Ansichten; Startwerte-/Baked-Hinweise. **N3 damit komplett** — nächster Punkt: HG1 |
| 1.8.0 | 2026-07-23 | Session 42: **HG1 umgesetzt (Code ✅)** — Host-Gruppen-Node-Typ mit Runtime-Trennung (eigener Buffer-Pool + ScriptContext je Gruppe, persistenter Feedback-Buffer), Tiefenregel (Compile-Degradierung + Panel-/DnD-Guards), `.lvfx2`-Persistenz + Import-Browser, Panel-Editor mit Settings-Sync und `.lvfx`-Import in Gruppe; 2 neue Gates (Roundtrip, Tiefenregel). Nächster Punkt: HG2 Crossfade-Mix |
