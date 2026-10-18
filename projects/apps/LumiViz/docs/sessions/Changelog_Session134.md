# Changelog Session 134 (2026-10-18)

> **Spielweisen der Saiteninstrumente, erster Teil.** Akkorde werden jetzt angeschlagen statt
> gedrückt — die Saiten klingen leicht nacheinander, nach unten und oben im Wechselschlag. Eine
> Saite kann gedämpft (Palm mute), getippt (Tapping) oder als Flageolett (Harmonic) klingen,
> oder weiterklingen (Let ring). Im Instrument-Creator lässt sich das einstellen und anhören;
> setzen in einem Loop lässt es sich ab der nächsten Session (Instrument-Editor). Steuerdokument:
> `docs/audio/Saiten_Konzept.md`.

## Neu

- **Strum:** jeder Akkord eines Saiteninstruments wird angeschlagen, mit 8 ms von Saite zu Saite
  (gemessen an gestrummten Gitarren, Freire 2018); die Hand geht im Takt nach unten und oben.
  Jedes Muster trägt seine Einstellung; ein einzelner Akkord kann abweichen (Down, Up, Together,
  eigener Versatz).
- **Artikulationen je Note:** Palm mute, Let ring, Tapping, Harmonic. Ein Flageolett wird auf
  der Saite gesucht, auf der es natürlich entsteht, sonst gegriffen zwölf Bünde tiefer.
- **Klang:** der Handballen und der Finger am Knoten wirken als Dämpfer auf der Saite; ein
  gedämpfter Ton verliert zuerst die Höhen, ein Flageolett behält nur seine Teiltöne, das fünfte
  klingt 14 Cent tiefer wie auf einer echten Saite.
- **Im Instrument-Creator** (bei einem Instrument mit Tuning):
  - die Zeile **Strum** unter Tuning: Spread, Hand, je Stelle ↓ oder ↑, Alternate — die Vorgabe
    für neue Muster;
  - die Gruppe **Playing**: Up level, Mute (mit der Klingdauer daneben), Tap level, Touch;
  - **Play as** unter der Klaviatur: Normal, Up stroke, Palm mute, Tapping, Harmonic, Strum
    chord.

## Geändert

- **Die mitgelieferten Loops mit Saiten** klingen mit Strum — gewollt (Entscheid S-W10 a);
  Hörproben vorher und nachher lagen zum Vergleich bereit.
- `.lvloop` Fassung 9, `.lvinst` Fassung 11: Strum steht immer in der Datei, damit ein
  gespeicherter Loop gleich klingt, auch wenn sich die Vorgabe des Programms ändert.

## Intern

- Neu: `loop/Spielweise`, `loop/StrumZeile`; `schlageAn`, `flageolettAuf`, `GeplanterTon`,
  Punktdämpfer im Umlauf der Saite (`PunktDaempfer`, nach Pakarinen 2005).
- Der Creator spielt über `vorhoerenVerlangt` mit Noten an (statt `tonVerlangt`), das Loop-Panel
  über `spieleInstrumentNoten`.
- `test_LoopWerte` und die Hörproben planen jetzt wie das Programm mit der Auskunft über die
  Saiten (fehlte seit Stufe 23b); `LUMIVIZ_LOOP_WAV_OHNE_STRUM=1`.
- Neue Tests: `test_LoopSpielweise`, `test_LoopSpielweiseKlang`, `test_LoopCreatorSpielweise`;
  1351 Fälle. Je Teilschritt eng unter beiden Compilern grün; kein voller Lauf (erst mit Tabs).
- `Offene_Punkte.md` 1.261.0, `Saiten_Konzept.md` 0.28.0.
