# Changelog — Session 46 (2026-07-24/25)

## Neu

- **AvsRef** (`tools/AvsRef/`): Referenz-Renderer um den originalen
  vis_avs-Kern (BSD-3, lokales Werkzeug) — headless auf Speicher-Framebuffer,
  byte-identisches synthetisches Audio, deterministischer Original-Beat;
  BMP + Luma-Stats; `--frames/--size/--out/--save-every/--beat-period/
  --silence`. Eigenständiges 32-bit-CMake-Projekt (`-A Win32`), Referenzbaum
  bleibt unangetastet (gepatchte Kopien + Begründung: `tools/AvsRef/README.md`).
- **Diff-Harness** `asset/calibration/avs/compare_avsref.py`: AvsRef vs.
  AvsStandalone je Preset — Metriken (dMean/dMaxLuma/MAE), Montagen,
  `report.md`.
- **Preset-Bisektion** `asset/calibration/avs/bisect_avs.py`: kumulative
  Teil-Presets (Top-Level/Listen/Index-Pfad) für die Täter-Suche.
- **Kalibrier-Gruppe `s15_y_richtung/`** (2 Presets + README + Zwillinge):
  Gate auf die AVS-y-Konvention am Skript-Rand.
- **AvsStandalone `--save-every M`**: Frame-Sequenzen für Bewegungs-/GIF-
  Vergleiche (gleiche Frame-Zählung wie AvsRef).

## AVS-Treue-Fixes (alle per AvsRef belegt)

- **y-Konvention:** Skripte sehen den AVS-Raum (y+ = unten, r im
  Screen-Drehsinn); Übersetzung ausschließlich am Modulrand
  (SuperscopeModule-Punktausgang, ScriptGridModule-Sicht + Rückweg
  rect/polar). Vorher war die gesamte Skript-Welt vertikal gespiegelt.
- **S13:** Import-Scopes skalieren je Achse (Kreis-Skript = 4:3-Ellipse wie
  r_sscope) — keine Aspekt-Korrektur im Import-Pfad.
- **BLEND4-Resampler:** Warp-Shader repliziert die Integer-Trunkierung des
  Originals (Feedback-Trails dunkeln ab und sterben aus statt zu sättigen).
- **Linien:** SuperScope-Import 1-px-Bresenham-nah (GL_LINE_STRIP, dotSize 1);
  dicke SRM-Linien verbreitern ACHSENPARALLEL wie linedraw.cpp
  („wall-thick bars"). s2-Zoom-Kalibrierung: dMean 0,50 → 0,009.
- **Farben:** AVS-Preset-Farben sind Framebuffer-RGB (kein COLORREF) —
  globaler R/B-Swap im Translator entfernt (Wormhole gelb statt grün).
- **Multiplier XI/XS:** Modus 0 = jeder Nicht-Null-Pixel → Weiß, Modus 7 =
  nur exakt Weiß bleibt (Anemone-Familie renderte schwarz).
- **EelTranspiler:** Nicht-ASCII-Bytes (z. B. `;©;`-Signaturen) werden wie
  im Original ignoriert statt den Slot still zu deaktivieren.
- **Bump (Teil):** depth2 hält beim Beat HART für durFrames (kein Ease),
  Skript-`bi` skaliert die Tiefe, `isbeat/islbeat` = −1/+1.

## Verifikation

LumiViz.UnitTests **414/414 grün, 0 Skips** (16 Gates auf belegte Konventionen
umgeschrieben, 2 neue); Builds VS-Debug/VS-Testing(`/WX`)/Ninja-Clang grün;
`.lvfx`-Zwillinge 24/24 (nach beabsichtigter Übersetzungs-Änderung
refreezed); Kalibrier-Sweep 8/22 → 19/24 OK.

## Bekannt / offen

- **Wormhole bei großen Flächen** baut weiter nicht auf (klein ok) — heiße
  Spur: Bump-`buffern` evtl. nicht decodiert + Listen-Out bei leerem Buffer;
  Werkzeuge und Teilketten liegen bereit.
- Merkregel ab jetzt: **Referenz-Vergleiche immer in zwei Größen**
  (320×240 + 740×460) — Pixel-feste AVS-Größen maskieren Bugs auf kleinen
  Flächen.
- Geplant: Modul-Matrix — je Effekt ein .avs+.lvfx-Testpaar mit
  Parametervarianten, automatisiert über den Harness.
