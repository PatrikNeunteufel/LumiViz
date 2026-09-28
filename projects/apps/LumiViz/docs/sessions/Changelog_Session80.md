# Changelog Session 80 (2026-09-28/29)

> **MilkDrop-Presets sehen aus wie gemeint — und das ist jetzt gemessen, nicht
> geschätzt.** Das Bild wandert nicht mehr von selbst nach oben links, der
> Zufall in Presets würfelt wie im Original, und Bass/Mitten/Höhen entstehen
> aus derselben Audio-Aufbereitung wie in MilkDrop 2. Der alte Look bleibt
> über Schalter erreichbar — und wer Drift, Wirbel oder Weichzeichnen als
> Gestaltungsmittel will, hat dafür jetzt eigene Regler.

## Neu

- **Gestaltung im Milkdrop-Knoten** (Gruppe „Gestaltung (kein
  Original-Verhalten)“):
  - **Linear-Drift** — Stärke (px/Frame) und Richtung (0° rechts, 90° oben);
    bleibt scharf.
  - **Radial-Drift** — Stärke, Winkel (0° nach außen, 90° Wirbel, 180° Sog)
    und Zentrum.
  - **Weichzeichnen** — mischt jeden Frame mit dem Nachbarpixel.
  - Je Werkzeug ein Schalter; Knöpfe **„Original“** und **„Weicher Sog ↖“**
    (= der Look bis Session 80).
  - Per Skript steuerbar: `drift_lin`, `drift_dir`, `drift_rad`,
    `drift_ang`, `drift_cx`, `drift_cy`, `weich` (Winkel in Bogenmaß).
- **Schalter „Skript-Zufall“** (Standard: Original) — `rand()` im Preset-Code
  würfelt wie MilkDrop 2: Kommazahl, fester Startwert.
- **Schalter „Audio“** (Standard: Original) — `bass`/`mid`/`treb` und die
  Werte der Custom-Waves entstehen aus der Audio-Aufbereitung von MilkDrop 2
  (eigenes FFT, ausgerichtete Wellenform).
- **Mehrfachzuweisung** `r = r2 = red;` in der Skript-Referenz erklärt.

## Behoben

- **Das Bild driftete nach oben links und wurde weich**, bei jedem
  MilkDrop-Preset — ein Portierungsfehler (D3D9-Halbpixel-Ausgleich unter
  OpenGL). Das Bild steht jetzt still wie im Original.
- **`rand()` in MilkDrop-Presets lieferte nur ganze Zahlen** — `rand(1)` war
  immer 0. Presets wie *martin – stratospheric turbulences* zeigen jetzt
  dieselbe Szene wie im Original.
- **Bass/Mitten/Höhen und Spektrum-Waves** kamen aus dem App-Spektrum mit
  geschätzten Faktoren — jetzt wie im Original (gegen den Original-Kern
  geprüft, identische Werte).
- **Eine Custom Wave/Shape/Sprite auf die freie Fläche unter der Effektkette
  gezogen ging verloren** — sie landet jetzt im letzten Milkdrop-Knoten.
- Im Skript-Editor war `b` (Blau) anders eingefärbt als `r`/`g`.

## Geändert

- Die Werkzeug-Einstellungen zum alten Verhalten heißen jetzt „LumiViz
  (bisher)“ — wer den Klang/Look vor Session 80 zurück will, stellt
  „Skript-Zufall“ und „Audio“ darauf und drückt „Weicher Sog ↖“.

## Intern

- Sichttest-Protokoll S79 großteils automatisiert: 21 UI-Tests offscreen
  (Palette, Kettenbaum am echten Panel) und Ketten-Stichproben mit
  Kontaktbögen (`asset/calibration/sichttest/sichttest_auto.py`).
- Messgerät MilkdropRef: gleicher Zufall wie LumiViz (`--zufall-lumiviz`),
  Warp-Mesh wählbar (`--mesh`), Audio-Ausgabe für Sollwert-Tests — und ein
  **Fehler im PCM-Format behoben**, der seit Session 63 alle MilkDrop-
  Referenzzahlen verfälschte.
- Neue Sonden: Drift, Gitterpunkte (Reihenfolge + Zufall je Punkt),
  Comp-Shader-Quellen, Spektrum-Wave.
- Unit-Tests 608 → 648.
