# Changelog Session 138 (2026-10-19)

> **Das Fenster Chords.** Im Instrument-Editor öffnet **Chords...** ein Fenster mit Akkorden für
> das Muster: bei Saiteninstrumenten Griffbilder der Stimmung, sonst Voicings auf einer
> Klaviatur — anhören, verschieben, an die Einfügemarke setzen oder ins Raster ziehen, eigene
> zeichnen. Dazu Bends bis zwei Ganztöne, *Play as* mit Hammer-on, Slide und Bend und zwei neue
> Beispiele. Stufe 26 ist bis auf den vollen Testlauf fertig.
> Steuerdokument: `docs/audio/Saiten_Konzept.md` (§1 S-T8 a bis S-T14 a, §3 S-L15 b, §5y–§5zd).

## Neu

- **Fenster Chords** (Knopf **Chords...** in der Zeile Instrument): links Ordner (★ *Favorites*,
  *Recently used*, die zwölf Grundtöne mit ihren Arten, *Your chords*), rechts eine Galerie
  kleiner Griffbilder bzw. Klaviaturen je Lage, unten das gewählte groß mit Tönen. Suche über
  alle Akkorde, Auswahl *Type*. Ein Klick spielt an, ein Doppelklick oder **Use** setzt an die
  Einfügemarke, ein Bild auf Tab-Linien oder Piano-Roll ziehen setzt es dort.
- **Anhören** in vier Spielarten (↓ Down, ↑ Up, ↓↑ Down-up, = Together); die gewählte bleibt und
  wird über einen Neustart gemerkt. **Move ±1 fret** verschiebt ein Griffbild auf dem Hals.
- **computed** an gerechneten Griffbildern, **unusual** an Lagen aus chords-db, die Töne außerhalb
  ihrer Art greifen (sie bleiben, wie die Quelle sie liefert).
- **Eigene Akkorde:** *New...*, *Edit...* (eine Vorlage wird eine eigene Kopie), *Duplicate*,
  *Delete*; der Dialog **Edit chord** mit anklickbarem Griffbrett (Punkt, ○/×, Finger, Barré
  durch Ziehen), *First fret*, Titel, Grundton, Art, Beschreibung, dem Vorschlag *sounds like*
  mit **Use name** und eigenem Undo.
- **Voicings** für Instrumente ohne Saiten: Klaviaturen in der Galerie (root, 1st/2nd/3rd inv.,
  open, drop 2), *Lowest note* mit −8va/+8va, ein Knopf *Play*; gezogen landet der tiefste Ton
  auf der Zeile unter der Maus; eigene Voicings mit Tasten im Dialog.
- **Play as** im Creator: *Hammer-on*, *Slide*, *Bend* — sie kommen auf dem angeklickten Ton an.
- **Beispiele** `tabs_demo.lvloop` (Blues: Griffbilder, Lead mit Hammer-on, Pull-off, Slide,
  Bend) und `voicings_demo.lvloop` (ii-V-I-vi aus Voicings).

## Geändert

- Ein **Bend** reicht bis **zwei Ganztöne** (vorher anderthalb): Menü *Height 2*, *Edit bend...*
  bis 2.
- **Fit** im Instrument-Editor zeigt das ganze Muster (bei manchen Fensterbreiten ragte es um
  einige Pixel über den Rand).
- Guitar, Overdrive Guitar, Finger Bass, Tine Piano und Round Bass tragen die Genres der neuen
  Beispiele (Blues, Jazz).
- Benutzerhandbuch 1.191.0: Tab-Linien, Legato/Slide/Bend, Fenster Chords, Edit chord, Voicings,
  im Creator *In a legato*, *Slide time*, *Legato level*; `.lvchord`.

## Intern

- Neue Tests `test_LoopGriffbild`, `test_LoopAkkordFenster`, `test_LoopAkkordDialog`,
  `test_LoopAkkordVoicing`; Fälle in `test_LoopSpielweise`, `test_LoopSpielweiseKlang`,
  `test_LoopEditorVerbunden`, `test_LoopCreatorSpielweise`, `test_LoopSpieler`; 1399 Fälle.
  Zuletzt `*Loop*,*Instrument*,*Datensicherung*,*Publish*` unter beiden Compilern 331 / 331;
  **kein voller Lauf** (Patrik: noch nicht).
- Die Arten der Akkorde stehen in `asset/loop/chords/types.json` — das Programm und
  `make_chords.py` lesen beide daraus (alle 6373 Vorlagen byte-gleich neu erzeugt).
- `asset/loop/make_tab_loops.py`; `make_spielweisen_loops.saiten_muster` schreibt den Bend mit.
- `Saiten_Konzept.md` 0.39.0, `Offene_Punkte.md` 1.265.0.
