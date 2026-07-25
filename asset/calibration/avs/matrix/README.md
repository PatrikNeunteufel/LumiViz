# Modul-Matrix — AvsRef-Vergleich über alle Builtins (Session 47)

Je Builtin-Effekt ein Ordner `NN_effektname/` mit mindestens einem
`.avs`-Testpreset (+ eingefrorenem `.lvfx`-Zwilling, `freeze_lvfx_twins.py`).
Erzeugt von `make_matrix_presets.py`, gemessen von `run_matrix.py` — **immer
in zwei Größen** (320×240 + 740×460; kleine Flächen maskieren Größen-Bugs,
Merkregel S46), Beat deterministisch alle 20 Frames (`--beat-period` beidseitig).

## Material

- **MAT_STATIC** (ClearScreen 0x202020 + Farbverlaufs-Spirale mit
  per-Point-Farben + Audio-Wave): für Farb-/Pixel-/Warp-Effekte. Die Basis
  ersetzt jeden Frame → Urteil rein intra-frame. Die Spirale ist bewusst
  **asymmetrisch** (Zentrum oben versetzt) und **farbverlaufend** (red=i,
  green=1−i) — fängt Spiegelungs- und Kanal-Bugs.
- **MAT_TRAIL** (ohne Basis): für Akkumulations-Effekte (Fadeout, Blitter,
  Roto, Water, OnBeat Clear …) — Trails über Frames sichtbar.
- Reine Render-Effekte laufen ohne Material mit Root-Clear je Frame.

## Bewusst ausgelassen

| id | Effekt | Grund |
|---|---|---|
| 10 | SVP Loader | externe .svp-DLL |
| 21 | Comment | rendert nichts |
| 28 | Text | GDI- vs. QPainter-Font-Rasterung — bekannt ◐, Sichttest statt Diff |
| 32 | AVI | externe Datei |
| 33 | Custom BPM | zeit- statt frame-basiert — nicht frame-deterministisch |
| 34 | Picture | externe Datei |

## Erwartete „PRUEFEN" trotz Korrektheit

`rand()` ist zwischen den Engines nicht bit-stabil (bewusster Entscheid,
Instanz-Nonce S45): **16 Scatter, 24 Grain, 27 Starfield, 8 Moving Particle**
liefern strukturell gleiche, pixelweise verschiedene Bilder — dort zählt die
Montage (`out/matrix_compare/<size>/montage/`), nicht die Metrik.

## Lauf

```bash
python asset/calibration/avs/run_matrix.py            # ganze Matrix
python asset/calibration/avs/run_matrix.py 29 43      # nur Bump + DM
```

Report: `out/matrix_compare/report.md` (Wegwerf-Artefakt) — Metriken beider
Größen je Zeile, Urteil = schlechteste Größe (Schwellen wie Kalibrier-Sweep:
dMean ≤ 0,02 · dMaxLuma ≤ 0,10 · MAE ≤ 0,03).

## Befunde des Erstlaufs (S47: 28/41 OK)

Echte Treue-Bugs (Montage-belegt, je eigene Aufgabe):

| Preset | dMean (schlechteste) | Befund |
|---|---|---|
| 00_simple | 0,46 | Original füllt SOLID (Analyzer), wir zeichnen nur eine Linie |
| 04_blitter_feedback | 0,72 | unser Feedback explodiert zum Mosaik-Teppich (Scale-Semantik?) |
| 09_roto_blitter | 0,37 | bei uns Pixel-Rauschen statt weichem Rotations-Zoom (Resampler) |
| 07_bass_spin | 0,14 | gefüllte Dreiecke fehlen fast ganz (nur Mini-Striche) |
| 23_interleave | 0,66 | wirkt bei uns gar nicht (Ref: blaues 4×4-Gitter übers Bild) |
| 39_timescope | 0,24 | bei uns deutlich zu dunkel (Intensitäts-/Band-Mapping) |

Milde Abweichungen (Feinvergleich offen): 17_dot_grid (0,047) ·
20_water (Trail-Abkling) · 31_water_bump (klein schlechter — 40px-Radius!) ·
41_interferences (0,025) · 01_dot_plane (dMaxLuma).
Erwartet & ok: 24_grain, 27_starfield (rand; Metrik nahe 0, Einzelpixel).
