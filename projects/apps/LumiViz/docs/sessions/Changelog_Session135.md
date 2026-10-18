# Changelog Session 135 (2026-10-18)

> **Spielweisen im Instrument-Editor, Zoom in den Editoren.** Wie ein Akkord angeschlagen wird
> und wie eine Note klingt (Palm mute, Let ring, Tapping, Harmonic), lässt sich jetzt im Loop
> setzen. Beide Editoren des Loops zoomen wie die Zeitachse. Steuerdokumente:
> `docs/audio/Saiten_Konzept.md` (§5q), `docs/audio/Loop_Konzept.md` („Zoom in den Editoren“).

## Neu

- **Zeile Strum** im Instrument-Editor (bei einem Instrument mit Tuning): Spread, Hand, je
  Stelle des Takts ↓ oder ↑, Alternate — für alle Akkorde des Musters.
- **Spur Strum** zwischen der Zeile der Takte und den Noten: an jedem Anschlag ein Pfeil, grau
  aus der Hand, blau mit Nadel von dir gesetzt, ≡ für Together, T für getippte Noten. Ein Klick
  schaltet Follow hand → Down → Up → Together.
- **In der Piano-Roll** stehen die Noten eines Akkords um ihren Versatz verschoben, ein feiner
  Strich zeigt die Reihenfolge des Anschlags; die Artikulationen als Kürzel PM, LR, T, ◇.
- **Rechtsklick auf Noten:** Untermenü Strum (Follow hand, Down, Up, Together, Spread...) und die
  Haken Palm mute, Let ring, Tapping, Harmonic.
- **Instrument wechseln:** Strum und String choice des Musters folgen dem neuen Instrument, wo
  sie noch dessen Vorgabe trugen; eigene Einstellungen bleiben.
- **Zoom** in Instrument- und Drum-Editor: Ctrl+Rad die Zeit, Alt+Rad die Höhe, Ctrl+Alt+Rad
  beides, um die Maus; der Knopf **Fit** zeigt das ganze Muster.

## Behoben

- Ein Klick in die Spur Strum und das Menü Strum kamen nicht im Loop an; ein gesetzter Anschlag
  ging beim Verschieben einer Note nicht mit.

## Intern

- `wechsleVorgaben`, `NotenRaster::uebernimm` meldet jede Änderung am Muster, der Editor übernimmt
  die Anschläge; `Achse::loeseAltVomMenue` für alle drei Stellen mit Alt+Rad.
- Neue Tests: `test_LoopEditorSpielweise`, `test_LoopEditorZoom`; 1360 Fälle. Bereich Loop,
  Instrument, Drum, Achse, Composer-Panel unter beiden Compilern grün; kein voller Lauf (erst
  mit Tabs).
- `Offene_Punkte.md` 1.262.0, `Saiten_Konzept.md` 0.29.0, `Loop_Konzept.md` 0.88.0.
