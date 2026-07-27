# MyViz — Offene Punkte (Arbeitsliste)

> **Version:** 1.0.0
> **Datum:** 2026-07-27 (Session 52)
> **Typ:** Status/Arbeitsliste
> **Status:** Aktiv — **SSOT für „was ist noch offen"**
> **Sprache:** Deutsch
> **Ersetzt:** `Offene_Implementierungen.md` + `Offene_Sichttests.md` (beide standen
> auf Session 37/38 und waren in weiten Teilen überholt — Git hat sie)

Dieses Dokument beantwortet **eine** Frage: was ist noch zu tun. Die Detail-Konzepte
bleiben, wo sie sind; hier steht je Punkt nur so viel, dass man ihn aufgreifen kann,
plus der Verweis auf die Quelle.

**Pflege:** am Ende jeder Session nachziehen — zusammen mit dem Handover
(`.claude/handover/HANDOVER.md`, lokal). Wo beide sich widersprechen, gilt das
Handover; dann dieses Dokument nachziehen.

Legende: 🔴 blockiert anderes · 🟠 Befund mit Messwert · 🟡 Entscheid nötig ·
⬜ Sichttest/Urteil offen · ⚪ Backlog (bewusst nichts tun) · 🔧 Kleinkram

---

## 1. AVS-Kalibrierung — offene Befunde mit Messwert

Metrik ist der Abstand zur Referenz (`AvsRef`), 0 = gleich. Methodik:
[AVS_Kalibrier_Methodik.md](visuals/AVS_Kalibrier_Methodik.md) — **Urteil über
gezeichnete Pixelmenge + Schwerpunkt**, nicht über dMean allein (die Metrik lügt bei
dünnen Inhalten).

| Preset / Sonde | Wert | Diagnosestand | |
|---|---|---|---|
| **15 Alien Alloy** | 0,683 | Zeichner + Transport pixelgleich verifiziert, es fehlt **Menge**: Stufe `l06` Referenz 75126 px (Energie 40434), wir 4685 (1893) bei praktisch gleicher Spitzenhelligkeit. Per Messung ausgeschlossen: Audio-Werte, rand-Init, Warp/Gitter/Abtastung, Colorfade (auch asymmetrisch). Nächster Verdacht: erreicht der Set-Render-Mode-Zustand (`lineBlend 2` = `BLEND_MAX`) den Texer über den dazwischenliegenden Custom-BPM-Knoten? Plus Sprite-Geometrie am Rand (`y=0.98`) | 🟠 |
| **01 Picture II** | 0,516 | verarbeitet die drei Bilder falsch — unanalysiert | 🟠 |
| **Inhaler** | 0,416 | war 0,246. Der `dt`-Fix (S51) hat die Zufalls-Startphase dort erst wirksam gemacht (`dt=rand(100)/40` im Init). **Kein Regress** — gehört zum rand-Ausrichtungs-Faden | 🟠 |
| **Deep Red Sea** | 0,858 | derselbe rand-Faden | 🟠 |
| **Alternate Reality** | 0,272 | derselbe rand-Faden | 🟠 |
| **30 Bright Light District** | 0,252 | Bruch bei Stufe 2 lokalisiert (Dynamic Shift schiebt ein 4-Pixel-Saatkorn im persistenten Puffer) — **nicht bewiesen** | 🟠 |
| **P3_HpR20 Rotor** | 0,461 | Rotor-Rest seit S48 (0,37 → 0,12 ◐, dann Rest) | 🟠 |
| **Tie Tunnel DM** | 0,154 | Altbestand seit S49 | 🟠 |
| **Sonde `convolution_kante`** | ~560 px | ≈ eine Zeile plus eine Spalte. `scale` geprüft (acht Kennlinienpunkte exakt), Kern seit S50 richtig orientiert. **Einzige offene Modul-Sonde** (40/41) | 🟠 |
| **Modul-Matrix-Reste** | 37/41 | `dot_grid` · `water` · `water_bump` · `interferences` | 🟠 |
| Color-Map-Kennlinie | ±1 | Altbestand | 🔧 |
| Colorfade-Zufalls-Beatmodus | — | Altbestand | 🔧 |
| nicht-statisches Grain | — | zieht inhaltsabhängig → kippt die rand-Ausrichtung des ganzen Presets (S49-Merkregel) | 🔧 |

**Vor „Regression!" den Vorstand MESSEN** (stash + Rebuild), nie gegen notierte Zahlen
einer anderen Messreihe — zwei von drei Auffälligkeiten in S51 waren auf HEAD identisch.

## 2. Urteile, die nur Seite-an-Seite fallen können

Prüfplan, Presets, Audio und Kriterien stehen in
[AVS_Sichttest_Protokoll.md §7](visuals/AVS_Sichttest_Protokoll.md). Angelegt in S45,
**keine Zeile ausgefüllt**. Test-Audio: `…\cmake\TestAudio\` (WAV = Master, MP3 für
Winamp-Komfort).

| # | Frage | Ohne Antwort passiert | |
|---|---|---|---|
| P1 | **S7** — konvergiert echtes AVS bei XOR-in/50-50-out-Listen auch zu Uniform-Grau? | unklar, ob überhaupt ein Bug vorliegt | ⬜ |
| P2 | **S13** — zeichnet AVS im nicht-quadratischen Fenster eine Ellipse, wo wir einen Kreis zeichnen? | Scope-Mapping bleibt aspektquadratisch | ⬜ |
| P3 | **S9-Rest** — sättigt das Original bei ZeroG/Novae/Rotor auch zu Weiß/Gelb? | ColorMap-auf-Weiß / FastBright-Ketten unbisektiert | ⬜ |
| P4 | **S12** — Spektrum-Amplitudenskala (`kSpecGain`) | Sichtkalibrierung offen | ⬜ |
| P5 | **S1** — fuzzify/blocky-Optik (Körnung, Blockraster) | S1-Rest nicht geschlossen | ⬜ |
| P6 | **Ego** — Subtract-Helligkeitsbalance | Schwarz ist gefixt (S14), Balance ungeprüft | ⬜ |
| P7 | Kür: Blend-Modi + L/R-Kanalvertrag | — | ⬜ |

**Nicht nachgemessen seit S45** (ein Teil dürfte durch S48–S52 erledigt sein):

- **HISTORY-pRELOADED, 12 Befunde** ([Protokoll §5](visuals/AVS_Sichttest_Protokoll.md)):
  HpR05 fast nur schwarz · HpR10 Kontrast · HpR11/HpR12 zu dunkel · HpR14 wird in <1 s
  weiß · HpRX2 „da fehlt was" · HpRX3/HpRX4/HpRX6 nur schwarz · HpRX5 wird zu hell ·
  HpRX7 Matrix-Text-Dichte.
- **JC-big stuff, 3 Weiß-Sättigungs-Verdachtsfälle** (§2.1): don't make a mess ·
  don't try to aphect ME · how much 4 the cool glowin thingi — Bisektion ausstehend.

## 3. MilkDrop

| Was | Stand | |
|---|---|---|
| **`rot_*`-Matrizen optisch** | seit S52 im Code (24 Matrizen + Matrix-Indizierung), 9 Presets im Pack betroffen. Werte sind zufällig und unser PRNG ist ein anderer als der von MilkDrop — prüfbar ist nur: laden sie durch, bewegt sich Plausibles | ⬜ |
| **Texturen `worms`, `rose`, `grad3`** | 27 von 35 Zeilen im Fehler-Log vom 2026-07-27. Existieren **nirgends** im Projekt, auch nicht im Original-Winamp-Pack (dort 21 Texturen, alle vorhanden seit `onefish.jpg` in S52). Entweder beschaffen oder als „nicht im Pack" abhaken — solange übertönen sie im Log alles andere | 🟡 |
| **In-App-Sichttest-Runde** | c1- + m5-Presets über den Node-Pfad, Panel-Baum + Editor-Sektionen, Session-A-Features (Wave/Shape/Sprite anlegen/entfernen/klonen, Sprite-Editor, fShader-Wash). Offen seit S42 — [Status Punkt 0](visuals/MilkDrop_Import_Status.md) | ⬜ |
| **Decay-Dither** + **`.milk`-Export** | Punkt 8, ganz ans Ende | 🟡 |
| **Playlist-Anbindung** | hängt an E6 (§6) | 🟡 |
| Host-Gruppen-Feinschliff | exakter paarweiser 2er-Mix statt sequentiellem Adjustable | 🔧 |

## 4. Sichttests, die nie stattgefunden haben

| Was | Umfang | |
|---|---|---|
| **Batch H — 9 Fraktal-Module** | Fractal 2D (9 Typen) · Fractal 3D (Raymarch-DE) · Domain Warp · Fractal Zoomer · Lyapunov · Kleinian · Strange Attractors · Flame/IFS · Reaction-Diffusion. Gebaut in S37, Unit-Tests grün, **GL-Sichttest komplett offen**. Kalibrierpunkte je Modul: Färbung/Banding, Kamera-Defaults, feed/kill, Reseed bei Divergenz, Punktzahl vs. Helligkeit. Querschnitt: Gradient-Paletten je Modul, Blend über die Kette, Audio-EEL-Reaktion | ⬜ |
| **FeedbackBuffer / `post.trail.*`** | Roadmap 4.3 — Sichttest (Trail-Look, Resize, Undock/Vollbild) + **Frametime-Vergleich** ausstehend. Einziges Modul-Doku mit offenem Status ([FeedbackBuffer.md](../include/visualizers/render/FeedbackBuffer.md)) | ⬜ |
| **BeatEstimator** | Roadmap 4.4 — Beat-Stabilität am laufenden Bild | ⬜ |
| **Multi-Drag zwischen Listen** | Block-Reparenting, Index-Mathematik nur compile-verifiziert; bei Bugs auf „Reorder in gleicher Ebene" beschränken | ⬜ |
| **Stereo `getspec`/`getosc`** | ch=1 (L) vs. ch=2 (R) getrennt? Nur compile-verifiziert. ⚠ **Hörtest**. Stellschrauben: `BASS_DATA_FFT_INDIVIDUAL`-Layout (`bin*chans+ch`) + Waveform-Interleaving | ⬜ |

## 5. Offene Entscheide

| Quelle | Entscheid | |
|---|---|---|
| [Hotkey_Konzept §9](ui/Hotkey_Konzept.md) | §9.2 Blättert Stufe 1 in Unterordner hinein? · §9.3 Am Verzeichnisende halten oder umlaufen? · §9.4 Verhalten bei Mehrfachauswahl · §9.5 Zeigen Menü-Einträge die Tasten? *(§9.1 in S52 entschieden: `Bild ab` = vorwärts)* | 🟡 |
| [Visual_Playlist §6](ui/Visual_Playlist_Konzept.md) | Pfad-Referenzen vs. eingebettete `.lvfx` · Auslöser-Default (Songwechsel vs. Timer) · Beat-Quantisierung des Timer-Wechsels · Import-Browser-Erweiterung jetzt oder mit der Playlist *(Hotkey-Frage ist nach Hotkey_Konzept ausgelagert und dort beantwortet)* | 🟡 |
| [Lights_Module_Entwurf](visuals/Lights_Module_Entwurf.md) | Entscheid 3: BASS-Lookahead als eigener Service (`AudioLookahead`) — Umfang/Session ungeplant. Bis dahin reichen Beat-Prädiktion + `gettime()` | 🟡 |
| [Parameter_Reference §10](visuals/Parameter_Reference.md) | Deklarierte Preset-Defaults vs. Dropdown-Indizes bereinigen · `solidColor`/`peak.color.fixed` ohne deklarierten Default | 🔧 |
| MilkDrop-Texturen | siehe §3 | 🟡 |
| [Config_Pipeline_Umsetzungsplan](visuals/Config_Pipeline_Umsetzungsplan.md) | **Formale Abnahme**: A1–A8 und N1–N7 stehen sämtlich auf `⬜`, obwohl die Schritte 0–7 als ✅ gelten und die Sichttests 5.1–5.5 abgenommen sind. Entweder nachträglich abhaken oder die Tabelle als erledigt streichen | 🔧 |

## 6. Konzept-Phasen, noch nicht begonnen

- 🟡 **Vereinheitlichung V2–V5** ([Konzept v1.2.0](visuals/Vereinheitlichung_Konzept.md)):
  V2 Audio-SSOT (MilkLoudness überall, `visdata`-Baustein, `getspec`/`getosc` in
  Milk-Slots, `ScriptInputFeeder`) · V3 Konstanten + Funktions-Abgleich gegen EEL2 ·
  V4 Standalones → Module · V5 Gradient-Parameter-Typ + LUT-Baustein.
  **V1** (Basis-Key-Registry + Import-Umbenennung) ist mit `ScriptBaseKeys.hpp` und
  der D2-Regel in S51 vorgezogen worden. Ausführung ausdrücklich **nach** der
  Kalibrier-Runde.
- 🟡 **P3 Skript-SSOT modul×slot** ([Skript_Variablen_Konzept](visuals/Skript_Variablen_Konzept.md) §3/§8):
  1. Symboltabelle Modul×Slot×Name → Kategorie/Typ/Range/Text · 2. Referenz daraus
  generieren statt Hand-HTML · 3. Kategorie-Highlighter modul-bewusst ·
  4. Fehler-Markierung je Slot statt global konservativ.
- 🟡 **P2 Visual-Playlist** — hängt an den Entscheiden §5.
- 🔧 **P1 Set Render Mode auf alle Scope-Effekte** — durch S45/S3 weitgehend erledigt
  (`drawScopeShape`, `drawDots` zeichnen über den SRM-Zustand); als Punkt nie
  formal geschlossen. Prüfen und schließen.
- ⚪ **Hotkeys Stufe 2/3** — Stufe 2 (Transport) ist in S52 verdrahtet; Stufe 3
  (Composer-Spuren) ist Fernziel.

## 7. Backlog (bewusst nichts tun)

⚪ Preset-Warmup/Pre-Roll (bei Laden/Resize N Frames vorrechnen — gegen
Schwarz-Start/Flackern) · Custom-Functions-Modul · Video-Capture-Modul ·
dynamische Modulparameter (alle Params per init/frame/beat/point) ·
Stereo `bass`/`mid`/`treb` · Variable-Set Variante B (benannte Globals) ·
Assets-Ordner-Fallback für Bild-Lader · **MilkdropRef** (zurückgestellt —
Reaktivierungs-Kriterium: ein MilkDrop-Treue-Bug, der nach mehr als einer Session
Diagnose keine klare Ursache hat. Für *Semantik*-Fragen reicht der Quelltext:
`cmake/ref/winamp_orig/Src/Plugins/Visualization/vis_milk2/`).

## 8. Werkzeug- und Doku-Schulden

- 🔧 **`bisect_avs.py` Pfad-Modus** rekonstruiert nicht verlustfrei (dieselbe
  Konstruktion: Referenz einmal 240, einmal 4 Pixel) — bis dahin nur die
  Top-Level-Leiter nutzen.
- 🔴 **Wächter-Lücken in der Modul-Matrix**: „Effect-List mit Extended-Config + EEL"
  und „Scope liest `reg` aus einem anderen Knoten" — beide Konstruktionen haben in
  S50 je einen **Totalausfall** verursacht und hatten keinen Wächter. Ebenso eine
  APE-Zeile (Convolution, Texer II, Video Delay, Multiplier, Picture II,
  Channel Shift, AddBorders, Multifilter). Die Bauer stehen seit S50 in
  `avs_preset_lib.py`.
- 🔴 **D2-Kollisionsregel hat keinen Wächter**: Matrix und Sonden enthalten keine
  kollidierenden Namen und können eine Regression strukturell nicht sehen. Nur ein
  Sweep über echte Presets bewacht sie.
- 🔧 **Produkt-Changelog Session 45 fehlt** in [sessions/](sessions/) — die Reihe
  läuft 43, 44, **46**, 47 … Der Session-Report existiert lokal
  (`.claude/sessions/LumiViz_Session45_…`), nur der Changelog wurde nie
  geschrieben. Nachziehen oder die Lücke bewusst vermerken.
- 🔧 Kür: en-Übersetzungen (de = SSOT) · `CMakeUserPresets.json` → `.example` ·
  App-Umbenennung **MyViz → LumiViz** · Pulsing-Defaults-Mismatch ·
  `File → Open Audio…`-Stub · Undock-Dauertest · Waveform-Glättungs-Default.

## 9. Bewusste Grenzen — kein Handlungsbedarf

| Bereich | Grenze |
|---|---|
| AVS-Builtins | SVP Loader (10) = externe UVS/SVP-DLL, nicht decodierbar |
| AVS-APEs | 5 verworfen: GeissFluid · ParticleSystem · MIDI Trace · AVI Player · AVSTrans Automation (closed-source bzw. Meta) · Framerate Limiter = no-op (der Host taktet) |
| HLSL-Transpiler | `#elif` und Nicht-Literal-`#if` → sauberer Fehler, MD1-Fallback wie im Original |
| MilkDrop-Referenz | GPU-Rendering ist nicht bit-deterministisch — Vergleich über Statistik/Montagen, nie Pixelgleichheit |

## 10. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-27 | Erstfassung (Session 52) — zusammengeführt aus `Offene_Implementierungen.md` (Stand S37) und `Offene_Sichttests.md` (Stand S37/38), beide überholt und entfernt, plus den aktuellen Befunden aus Handover, `MilkDrop_Import_Status.md` und `AVS_Sichttest_Protokoll.md` |
