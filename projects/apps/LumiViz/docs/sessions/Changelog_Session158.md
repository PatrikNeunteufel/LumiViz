# Changelog Session 158 (2026-10-28)

> **Chopins Nocturne Op. 9 Nr. 2 liegt ganz bei.** Der Loop *Nocturne Op. 9 No. 2* spielt das Stück auf dem
> gerechneten Flügel, vom Auftakt bis zur Schlussfermate, mit Pedal, Dynamik und den Verzierungen nach einer Regel mit
> Quelle — von Hand aus der Gesamtausgabe 1880 übertragen und gegen die Erstausgabe geprüft. Der Instrument-Creator
> bleibt beim Flügel bedienbar. Ein Hörvergleich mit der Aufnahme zeigt, wo der Flügel noch falsch klingt; das ist
> der nächste Schritt. Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (31d-9, HU1 a, DY2 a).

## Neu

- **Loop *Nocturne Op. 9 No. 2*** (`asset/loop/nocturne_demo.lvloop`): Frédéric Chopin, Auftakt und 34 Takte, 12/8,
  ♪ = 132, 3:38; beide Hände in einem Muster mit den Stimmen *Right hand* und *Left hand*, Sustain nach *Ped.* und ✱
  der Ausgabe, die Kadenz *senza Tempo* auf fünf Takte gestreckt. Erzeugt aus `asset/loop/nocturne_op9_2.py` mit
  `asset/loop/make_fluegel_loops.py`; die Beschreibung (Tafel der Dynamik, Tempo-Stellen, Verzierungen, Abweichungen
  der Erstausgabe) steht in `asset/loop/README.md`, die Ausgaben in `asset/loop/CREDITS.md` (wird mitgeliefert).

## Behoben

- **Edit instrument... beim Flügel:** der Creator rechnete nach jeder Änderung die Pegelmessung im Vordergrund
  (0,65–1,9 s beim *Grand Piano*) und stand so lange. Er misst jetzt auf einem eigenen Thread und merkt sich das
  Ergebnis wie das Fenster *Instruments* (CR1 a); solange gemessen wird, zeigt die Zeile *Measuring loudness...*.

## Intern

- `kPegelRate` steht in `loop/Pegel.hpp` (Creator und Fenster *Instruments* finden denselben gemerkten Wert).
- `make_examples.als_text`: eine Zeile mit leerer Liste (Bend) bleibt einzeilig.
- `werte/loop_werte.txt`: eine Zeile mehr (die Nocturne); bestehende Zeilen unverändert.
- Neu `Loop Pegel CR1`; Messfall `Loop Akkord Fenster S158` (`LUMIVIZ_OEFFNEN_MESSUNG=1`, misst das Öffnen von
  Editor, *Chords* und Creator je Instrument).
- Konzept: DY1 a (Gabeln ausführen), NP1 a (Fader der Nocturne, Spitze −1 dBFS), Hörurteil S158 zu den Hörproben 31d
  mit Messung der Hüllkurve und der Teiltöne, HU1 a (zuerst die Ursache), DY2 a (Tafel mit Quelle); Loop_Konzept CR1 a.
- Tests: 1640 Cases (+2); voller Lauf 1640 / 1640, 0 Skips unter Release-clang und Testing-MSVC. Der volle Lauf dauert
  mit der Nocturne rund 2–3 min länger (Tests über alle mitgelieferten Loops).
