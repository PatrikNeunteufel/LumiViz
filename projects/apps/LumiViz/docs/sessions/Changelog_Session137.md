# Changelog Session 137 (2026-10-19)

> **Schichten folgen der Saite, Tab-Linien und Akkorde.** In einem Hammer-on, Slide oder Bend
> kann jede Schicht eines Saiteninstruments der Saite folgen. Der Instrument-Editor zeigt unter der
> Spur Strum Tab-Linien, in die man Bünde tippt, und setzt Legato, Slide und Bend über das Menü.
> Dazu das Modell und 6373 mitgelieferte Akkorde für das kommende Fenster *Chords*.
> Steuerdokument: `docs/audio/Saiten_Konzept.md` (§1 S-T1 a bis S-T7 a, §3 S-L13 a bis S-L15 b,
> §5u–§5x).

## Neu

- **In a legato** je Schicht im Instrument-Creator (Kasten Source): *Own note* (Vorgabe) oder
  *Follow string* — die Schicht geht Hammer-on, Pull-off, Slide und Bend als ein Ton mit. Gilt für
  Sinus, Dreieck, Säge, Rechteck, Form, Sample und die Saite *Classic*; beim Rauschen folgt der
  Filter. In der Liste *Layers* steht „~“ hinter jeder Schicht, die folgt.
- **Tab-Linien** im Instrument-Editor zwischen Spur Strum und Noten: je Saite eine Linie, an jeder
  Note der Bund. Eine Linie anklicken und eine Zahl tippen setzt eine Note (zwei Ziffern schnell
  hintereinander: zweistellig); eine Zahl auf eine andere Linie ziehen legt den Ton auf diese
  Saite. Haken **Tab** am Ende der Zeile Strum.
- **Legato (hammer-on / pull-off)** und **Slide** als Haken im Menü der Noten; Untermenü **Bend**
  mit *Bend*, *Bend and release*, *Pre-bend*, *Pre-bend and release*, der Höhe und **Edit
  bend...** (Kurve mit ziehbaren Punkten). Gezeigt als Bogen h/p, Slide-Pfeil und Bend-Kurve in
  der Piano-Roll, als Bogen, „/“ und Pfeil in den Tab-Linien.
- **Akkorde** als Datei `.lvchord` (Griffbild oder Voicing) mit eigenem Icon; 6373 mitgelieferte:
  Gitarre und Ukulele aus chords-db (MIT), für Drop D, Bass und Violine gerechnet, dazu Voicings.
  Bereich *Chords* in Export/Import data und Publish. Das Fenster zum Wählen kommt in der nächsten
  Stufe.

## Geändert

- Ein gebogener Ton klingt im Halten so lange wie ein gegriffener derselben Höhe (vorher kürzer);
  ein Bend um einen Ganzton ist jetzt rund 0,6 dB leiser als der ungebogene Ton statt 1,1 dB.
- Der Bau von LumiViz gleicht die mitgelieferten Dateien schneller ab (mit den Akkorden 1,9 s
  statt 35 s).

## Intern

- Neue Tests `test_LoopAkkord`, `test_LoopEditorTab`, `test_LoopEditorVerbunden`; Fälle zu 25c-4
  in `test_LoopSpielweise`, `test_LoopSpielweiseKlang`, `test_LoopCreatorSpielweise`; 1383 Fälle;
  Editor, Instrument, Spielweise, Zoom, Akkord unter beiden Compilern grün; kein voller Lauf (erst
  am Ende von Stufe 26).
- `asset/loop/chords/make_chords.py`, `CREDITS.md`, `THIRD_PARTY_NOTICES.md`; `AssetAbgleich.cmake`
  ohne quadratische Suche.
- `Saiten_Konzept.md` 0.32.0, `Offene_Punkte.md` 1.264.0.
