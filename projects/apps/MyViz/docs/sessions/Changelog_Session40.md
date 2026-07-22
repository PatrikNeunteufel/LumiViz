# Changelog — Session 40 (2026-07-22)

MilkDrop-Import: M5 + M6.1 + Shader-Stufen C1/C2, Entscheide E1–E8,
Import-Browser-Persistenz. Tests am Ende: **390 Cases grün, 0 Skips,
10326 Assertions**; Builds VS-Debug/-Testing (`/WX`) + Ninja-Clang-Release grün.

## Neu

- **M5 — Blur-Pyramide + Shader-Stufe B (Sichttest BESTANDEN):** Original-treue
  Blur-Kette (blur1–3, Range-Kompression `b1n..b1ed`, per_frame-animierbar);
  Shader-Klassifikation (MilkShaderClassifier): von MilkDrop generierte
  MD1-Default-Shader und lineare Blur-/Gain-Varianten rendern **exakt** mit
  eingebackenen Konstanten; Import-Report meldet die Klasse. Kalibrier-Satz
  `asset/calibration/milkdrop/m5/` (8 Presets).
- **M6.1 — Persistenz:** Milkdrop-Presets speichern/laden als `.lvfx`
  (Schwester-Format; Laden erkennt den Typ automatisch, `File → Save Effect
  Chain…` speichert je nach aktivem Host). Roundtrip-Tests 26/26.
- **Stufe C1 — HLSL→GLSL-Transpiler:** neue Lib **HlslTranspiler** übersetzt
  echte Preset-Shader (Ausdrücke, Swizzles, Intrinsics, `#define`, Funktionen,
  Casts, if/ternary) nach GLSL; der Host kompiliert pro Preset Warp-/Comp-
  Programme (Sampler-Varianten fc/pc/fw/pw, q1–q32/rand/roam/Blur-Uniforms);
  bei Fehlern stiller MD1-Fallback. Korpus: warp 462/574, comp 409/598
  (Rest = Stufe C3: Loops/Arrays/tex3D).
- **Stufe C2 — Noise + Texturen:** exakter AddNoiseTex-Port (lq/lq_lite/mq/hq
  inkl. kubischer Glättung) und Custom-Textur-Lader (`<preset>/textures`,
  `randNN`, `texsize_`-Uniforms, Platzhalter + Report bei fehlenden Dateien).
- **Kalibrier-Satz `c1/`** (8 Presets mit exaktem Sollverhalten + Testbild);
  Import-Dialog erscheint nur noch bei **echten** Warnungen (Bestätigungen
  laufen als ℹ-Zeilen still mit).
- **Import-Browser:** merkt sich Ordner + Filter über App-Neustarts;
  Zurücksetzen unter Settings → Panels. (Root-Fix aktiviert auch die
  Playlist-Panel-Persistenz.)
- **Kalibrier-Raster** `render.debugGrid` (Milkdrop-Host): 8×6-Referenzraster
  als Screen-Overlay, geht nicht in den Feedback-Loop ein.
- **Doku:** neues Fortschritts-SSOT `visuals/MilkDrop_Import_Status.md`
  (inkl. Bezeichnungs-Legende M/Stufen/C#/N#/E#); Konzept v1.10.0 mit den
  Entscheiden E1–E8 (u. a.: Milkdrop wird Chain-Node, Standalone entfällt
  danach; Crossfade als echtes Doppel-Rendering; Kür ist Pflicht).

## Bekannter offener Befund

Stufe C1/C2 zeigen im Sichttest noch keine Wirkung (alle c1-Presets im
MD1-Look), obwohl Transpile, glslangValidator und der neue **GL-Smoke-Test**
(Offscreen-3.3-Kontext, echter Qt-Pfad) alle 8 Shader fehlerfrei bauen —
Verdacht veraltetes Lauf-Binary; wird nächste Session per Standalone-
Testprogramm isoliert. Diagnose-Hilfen: GL-Fehler-Dump nach
`%TEMP%\lumiviz_glsl_error.txt` + Warnung beim erneuten Laden.
