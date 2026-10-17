# Changelog Session 131 (2026-10-17 / 18)

> **MilkDrop-Shader rechnen weiter wie im Original.** Presets mit eigenen Shadern lesen das Bild
> über die Ränder hinaus, bekommen `rad`/`ang` wie MilkDrop, bauen Matrizen richtig herum und
> lesen Rauschtexturen aufrecht. *Starfield (Supernova)* ist nicht mehr schwarz,
> *stratospheric turbulences* zeigt dieselbe Szene wie das Original. Der alte Look bleibt je Pass
> wählbar. Steuerdokument: `docs/Offene_Punkte.md` §3. Im Programm ist nichts gesehen.

## Geändert

- **MilkDrop: *Warp shader* und *Comp shader*** (vorher *Warp/Comp shader Y*) — im Standard
  „Original“ laufen die Shader eines Presets jetzt in vier Punkten wie in MilkDrop: `y` nach unten
  (seit S130), `rad`/`ang` wie das Original, `sampler_main` wiederholt das Bild über die Ränder
  (auch bei ausgeschaltetem *Texture wrap*), selbst gebaute Matrizen (`float2x2(…)`) zeilenweise.
  „LumiViz (previous)“ holt den Look bis Session 129 zurück. Handbuch §MilkDrop, Sammelliste der
  „bisher“-Schalter.

## Behoben

- *Starfield (Supernova-Lens-Effect edit)* blieb fast schwarz.
- Drehungen und Scherungen in Shadern, die ihre Matrix selbst bauen, liefen andersherum (im Pack
  rund 12 600 Presets).
- Rauschtexturen wurden in Shadern gespiegelt gelesen.
- Unter „Original“ war `ang` im Warp-Shader seit Session 130 gespiegelt.

## Intern

- HlslTranspiler 1.7.0 (`quadratMatrixZeilenweise`), dazu ein Testfall.
- Sieben neue Sonden unter `asset/calibration/milkdrop/ysonde/` (rad/ang, Wrap, Matrizen,
  Rauschen), jetzt 16; mit `--zufall-lumiviz` fahren.
- MilkDrop-Vergleichslauf (22 Presets, 6 Formate) zweimal: mit den alten Schaltern 748 / 748
  Bilder bitgleich zum Stand von S129; mit dem neuen Standard Magma und Playaround näher an der
  Referenz.
- Voller Testlauf unter Release-clang und Testing-MSVC grün (1289 Fälle).
- `Offene_Punkte.md` 1.252.0: Starfield ✅, `stratospheric turbulences` 🟠 → 🟡, §7 ein Testfall
  hängt an der Reihenfolge (`presetIconDir()`).
