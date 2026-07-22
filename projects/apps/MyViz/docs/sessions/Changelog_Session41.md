# Changelog — Session 41 (2026-07-22)

MilkDrop-Import: C1/C2-Befund gelöst (Regression), Milkdrop wird Chain-Node
(N1+N2, Entscheide E1/E2), Preset-Sprites (E7-Pflichtkür). Tests am Ende:
**397 Cases grün, 0 Skips, 10451 Assertions**; Builds VS-Debug/-Testing
(`/WX`) + Ninja-Clang-Release grün.

## Behoben

- **C1/C2-Shader wirkten nicht (Sichttest-Befund Session 40):** Ursache war
  eine Regression aus dem C2-Umbau — die Transpile-Aufrufe im Ladepfad gingen
  verloren, alle Presets liefen still im MD1-Fallback. Custom-Shader rendern
  jetzt; ein Regressions-Test im Ladepfad verhindert die Wiederholung.

## Neu

- **Milkdrop ist jetzt ein Chain-Node im Multi-Effect-Host** (statt eigener
  Visualizer): `.milk`-Import und Milkdrop-`.lvfx` landen als Node in der
  Effektkette; der Baum zeigt Preset → Code · Waves · Shapes · Shader als
  Kinder, alle Skript-/Shader-Slots sind im Panel editierbar (Shader-Edits
  reklassifizieren live). Speichern läuft einheitlich über das Ketten-Format
  (Preset eingebettet); alte Milkdrop-Dokumente bleiben ladbar. Der separate
  „Milkdrop"-Eintrag in der Visualizer-Liste entfällt.
- **Preset-Sprites (MilkDrop2077-Format):** eingebettete `[SPRITEn]`-Sektionen
  werden gerendert — Bild + per-Frame-EEL (Position/Größe/Rotation/Farbe/
  Alpha), 5 Blend-Modi inkl. Colorkey, `burn` brennt das Sprite in den
  Feedback-Loop (Warp zieht Spuren), `done` beendet es. Kalibrier-Satz
  `asset/calibration/milkdrop/s1/` (3 Presets + README) — Sichttest bestanden.
- **Diagnose-Trace:** `%TEMP%/lumiviz_milkdrop_trace.log` protokolliert den
  kompletten Milkdrop-Lade-/Renderpfad (inkl. Build-Zeitstempel des Binaries
  und Render-Branch-Entscheiden) — immer aktiv, nur Zustandswechsel;
  `LUMIVIZ_MILKDROP_TRACE=0` schaltet ab.
- **MilkdropStandalone** (neues Konsolen-Tool): isolierter Kern-Test mit
  eigenem GL-Fenster; `--auto` rendert einen Preset-Ordner durch und legt
  Screenshots + Pixel-Statistik ab (Kalibrier-Beweisläufe c1 8/8, s1 3/3).

## Verifikation

Neue Test-Gates: Ladepfad-Regression (c1 → Custom-GLSL gefüllt),
MilkdropNode-Roundtrip/Clamps/Import, Sprite-Translator/-Serializer/-Korpus.
Sichttests: s1 in-app **bestanden**; c1/m5 in-app über den Node-Pfad stehen aus.

## Offen

Sichttest-Runde c1+m5 über den Node-Pfad, dann Crossfade (E5, echtes
Doppel-Rendering) → Visual-Playlist → C3. Fortschritts-SSOT:
`visuals/MilkDrop_Import_Status.md` (v1.3.0).
