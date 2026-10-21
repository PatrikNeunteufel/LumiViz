# Changelog Session 141 (2026-10-21)

> **Die Spuren über der Zeit sind bedienbar.** Pedale, Expression, Vibrato und Pitch lassen sich
> im Instrument-Editor zeichnen, die Expression eines Clips als Kurve im Loop-Panel, und an einer
> Gitarre mit Tonabnehmer wirkt Expression wie der Lautstärkeknopf. Dazu zwei kleine Fehler behoben
> und das Konzept für das Klavier begonnen: zuerst ein Fundament, das teure Töne vorab rechnet und
> Clips einfrieren kann.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` (SK12 a, SK13 a, §4 Hammer-Saite mit H1 a bis
> H3 a und F1 a bis F6 a).

## Neu

- **Expression an der Saite:** an einer Saite mit Tonabnehmer direkt dahinter, vor dem Drive —
  ein leiserer Ton zerrt weniger. Andere Schichten nehmen sie immer.
- **Spuren im Instrument-Editor:** die Zeile **Lanes** (Haken je Spur, dazu **Tab**), die Spuren über
  dem Raster mit Kürzeln; Punkte setzen (auf den Schritt, mit Alt frei), ziehen, mit Doppelklick
  löschen, **Clear lane**. Es erscheinen nur die Spuren, die das Instrument trägt.
- **Expression am Clip:** eine Kurve über den ganzen Clip im Loop-Panel, am markierten Clip zu
  bearbeiten (gestrichelte Linie „Expression 1“), **Clear expression** im Menü.
- **Beispiel `lanes_demo`** („Slow Swell“): jede Spur hörbar in einem Kanal.
- Handbuch 1.197.0: Spuren, Kurve am Clip, **Pitch range** und **Tremolo arm** im Creator.

## Geändert

- Der Haken **Tab** steht jetzt in der Zeile **Lanes**.
- In den Tab-Linien verdeckt die Höhe eines Bends keine Zahl mehr; wo kein Platz ist, nennt sie der
  Tooltip der Note.
- Im Karaoke-Panel geht der Stern „geändert“ nach dem Speichern sofort weg (vorher bis zu 500 ms
  später).

## Intern

- `loop/Stueckweise.hpp`, `PegelPunkt`, Töne mit `:x`/`:y`; `traegtSpur`, `pitchBereichHt`,
  `InstrumentEditorSpuren.cpp`; Kurve in der `Taktachse`; `expressionDb`; `make_spuren_loops.py`.
- Neue Tests `test_LoopSaiteExpression`, `test_LoopEditorSpuren`, `test_LoopClipExpression`,
  `test_LoopSpurenBeispiel`, je ein Fall mehr in `test_LoopEditorTab` und `test_PaletteUi`; 1450
  Fälle. Bereiche unter beiden Compilern grün; **kein voller Lauf**.
- Konzept: Familie Hammer-Saite (Zielbild), **Töne vorab** (Speicher je Tonname, Hintergrund,
  Vorladen) und **Freeze** (Clips als Wellenform, per Hash) entschieden; Bauplan: Stufe 29 Töne
  vorab, 30 Freeze, danach das Klavier.
- `Spielweisen_Konzept.md` 0.27.0, `Offene_Punkte.md` 1.268.0.
