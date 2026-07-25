# Changelog — Session 47 (2026-07-25)

## Neu

- **Render-Scale-Modul** (natives Ketten-Modul `renderScale`, Vorbild fürs
  Winamp-Pixel-Doubling): die gesamte Chain rendert intern bei Fenster/N
  (1–8) und wird beim Present hochskaliert (nearest = authentisch grob /
  linear = weich); der erste aktive Knoten gewinnt. **AVS-Importe erhalten
  den Knoten automatisch als erstes Kind** — Divisor aus der App-Einstellung
  (Settings→Panels „AVS Import Render Scale", Default 1 = neutral);
  Standalone/Harness importieren immer neutral.
- **Modul-Matrix** (`asset/calibration/avs/matrix/`): 41 Testpresets über
  39 Builtins mit Material-Renderern, README (Konventionen, Ausschlüsse,
  rand()-Erwartungen). Werkzeuge: `avs_preset_lib.py` (geteilte
  Blob-Builder), `make_matrix_presets.py`, `run_matrix.py` (immer 320×240
  UND 740×460, deterministischer Beat, ein Gesamt-Report);
  `freeze_lvfx_twins.py` deckt `matrix/` ab (65 Zwillinge).
- **AvsStandalone `--beat-period N`** (+ Visualizer-Override, Zählung je
  Preset ab 0 wie AvsRef) und `compare_avsref.py --beat-period` —
  History-Preset-Diffs sind damit frame-exakt.
- **Lights-Strang:** Quellcode-Analyse des HelloEnjoy-„Lights"
  (github.com/C4RL05/Lights), Demo-Preset mit Bordmitteln
  (`asset/effectchain/lights_demo.avs|.lvfx`) und Detail-Entwurf
  `docs/visuals/Lights_Module_Entwurf.md` (Bloom · 3D Camera ·
  3D-SuperScope mit Soft-Sprites · Heightfield-Terrain · Glow-Orbs (oval);
  Daten-Grundsatz: keine vorgebackenen Song-Daten — Beat-Prädiktion +
  BASS-Lookahead + Live-Phasenschätzung).

## AVS-Treue-Fixes (alle per AvsRef belegt)

- **Skript-Hosts, Wurzelfix 1:** `setVisData()` vor dem Erst-Compile ging
  verloren (SuperscopeModule + ScriptGridModule) — Init/Beat des ersten
  Frames sahen Null-Audio (`getspec`=0); der Wormhole verpasste so seinen
  Frame-0-Beat (t-Phasen/Farben verschoben). visdata wird jetzt gepuffert
  und beim Erst-Compile nachgefüttert.
- **Skript-Hosts, Wurzelfix 2:** Slot-Reihenfolge auf **Frame VOR Beat**
  korrigiert (r_sscope.cpp:272-273, r_dmove.cpp:297-298) — Beat-Werte
  wirken wie im Original erst im Folgeframe.
- **Bump komplett** (r_bump.cpp zeilengenau): Skip-Regel (vier schwarze
  Tiefen-Nachbarn → Pixel bleibt schwarz — vorher weiße Licht-Flut auf
  großen Flächen) · `buffern`-Tiefenquelle aus dem Global-Buffer (immer
  schreiben bei Fremd-Buffer) · y-Richtung der Lichtposition entspiegelt ·
  1px-Rand schwarz · Beat-Tiefe als linearer Original-Ease
  (a = |depth−depth2|/durFrames, Integer — revidiert den S46-Hart-Halt).
- **Present-Pass:** Quad-Draw statt `glBlitFramebuffer` — skalierende
  Blits in das multisampled App-Fenster (samples=4) sind GL-invalid
  (Bild fror mit Render Scale ein).
- **Wormhole-Bilanz:** baut in allen Größen synchron zum Original
  (Bisektions-Stufen dMean 0,58 → 0,08); das Ausdünnen bei großen Fenstern
  ist Original-Verhalten (pixel-feste Energie-Injektoren) → Antwort ist
  das Render-Scale-Modul.

## Matrix-Erstlauf: sechs neue Befunde (offen, Belege im matrix/README.md)

Simple (solid-Analyzer-Füllung fehlt, dMean 0,46) · Blitter Feedback
(Feedback explodiert, 0,72) · Interleave (wirkt nicht, 0,66) · Roto
Blitter (Pixel-Rauschen statt weichem Zoom, 0,37) · Timescope (zu dunkel,
0,24) · Bass Spin (gefüllte Dreiecke fehlen, 0,14). Milde Reste:
dot_grid · water · water_bump (nur klein) · interferences · dot_plane.

## Verifikation

MyViz.UnitTests **418/418 grün, 0 Skips** (4 neue Gates: Frame-vor-Beat ×2,
visdata-Erst-Compile, Render-Scale-Roundtrip); Builds VS-Debug/VS-Testing
(`/WX`)/Ninja-Clang grün; Zwillinge **65/65** (refreezed: ÷1-Knoten);
Kalibrier-Sweep 19/24 OK (unverändert); Matrix 28/41 OK; Wormhole-Messreihen
320×240 bis 1280×960 inkl. Luma-Kurven + 5-s-Seite-an-Seite-GIF.

## Bekannt / offen

- Sechs Matrix-Befunde (oben) als nächste Fix-Runde; danach P-Preset-Familie
  neu messen (P3/P4/P5 hängen vermutlich daran).
- Kleinrest: Scope-y bei exakt 0,5 rundet 1 Zeile anders (ref 180/lumi 179).
- Lights-Etappen 1–3 nach `Lights_Module_Entwurf.md`.
