# Beispiel-Presets je Basis-Voreinstellung

Für **jede** mitgelieferte Knoten-Voreinstellung (`asset/nodepresets/`) liegt
hier genau ein Beispiel-Preset mit passendem Render-Material, damit der Effekt
sauber sichtbar ist. Dateiname = `<typkey> - <Vorlagenname>` — Typ und Name der
Voreinstellung stehen im Namen, keine Unterordner (Vorgabe S61).

**Erzeugt** von `asset/calibration/avs/make_example_presets.py` — nach einer
Vorlagen-Änderung dort neu laufen lassen, nicht von Hand editieren.

## Zwei Dateiarten

- **`.avs`** — die Vorlage ließ sich verlustfrei ins AVS-Dateiformat
  zurückabbilden (Umkehrung des kalibrierten Imports). Diese Beispiele rendern
  in **beiden** Welten und sind direkt gegen die Referenz messbar:

  ```
  python asset/calibration/avs/compare_avsref.py "asset/examples/<datei>.avs"
  ```

- **`.lvfx`** — host-eigene Typen (Fraktal-Familie, Bloom, Metaballs,
  Tentacles …), Vorlagen, die freigemachte Host-Konstanten setzen
  (z. B. `rotatingStars.stars`), oder Formeln im LumiViz-EEL-Dialekt
  (`mod()`, `<`/`>` — Original-EEL kennt beides nicht). Nur in LumiViz
  ladbar, kein AvsRef-Vergleich möglich.

## Render-Material

Wie die Modul-Matrix (deterministisch, ohne `rand`): Farbverlaufs-Spirale +
Audio-Wave. Render-Vorlagen stehen allein (Root-Clear je Frame),
Akkumulations-Effekte (Movement, Blitter, Fadeout …) laufen als Trail VOR den
Quellen, alle übrigen Trans-Effekte als Filter HINTER der Static-Szene.
Custom-BPM-Beispiele zeigen den gefilterten Beat als abklingenden Weiß-Blitz.

Erwartbar „dünne" Bilder (kein Fehler): Starfield, Spektral-Vorhang,
Spektrum-Ring, Sternenkranz — wenige helle Pixel auf Schwarz.
`rand()`-basierte Effekte (Grain, Starfield, Zufallsspiegel) sind zwischen
den Renderern nicht bit-stabil — Urteil dort über die Montage, nicht die
Metrik.
