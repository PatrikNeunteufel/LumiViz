# Changelog Session 132 (2026-10-18)

> **Saiteninstrumente, erster großer Teil: das Modell und der Klang.** Ein Instrument kann
> Saiten mit Stimmung, Capo und Mensur tragen. Jede Note bekommt eine Saite, und eine neue Note
> auf derselben Saite dämpft die alte. Dazu kommen zwei neue Saiten-Verfahren, Tonabnehmer für
> E-Instrumente und ein erster gemessener Korpus. Eine Oberfläche dafür gibt es noch nicht, sie
> folgt mit Stufe 23e. Steuerdokument: `docs/audio/Saiten_Konzept.md`.

## Neu

- **Saiten am Instrument:**
  - die Töne der leeren Saiten, Capo, Mensur;
  - sechs Vorlagen: Guitar standard, Drop D, Bass 4, Bass 5, Ukulele, Violin.
- **Saitenwahl:** Eine Note kann ihre Saite tragen. Ohne eigene Saite wählt das Programm eine,
  nach *Per chord* oder *Follow hand* (je Muster). Spielt eine Note auf einer Saite, die noch
  klingt, dämpft sie die alte Note in 5 ms ab.
- **Zwei neue Saiten-Verfahren** neben der alten *Plucked string* (*Classic*, unverändert):
  - *Extended KS*;
  - *Waveguide*.

  Ihre Regler sind *Pluck position*, *Pluck*, *Ring time*, *High damping*, *Stiffness* und
  *String variation*.
- **Tonabnehmer** (Waveguide):
  - Neck und Bridge, jeder mit *Type*: None, Single coil, Humbucker (zwei Spulen, 18 mm), Piezo;
  - dazu *Position* in mm, *Pickup select* und *Cable*.

  Die Resonanz mit dem Kabel stammt aus Messungen (Zollner).
- **Korpus Acoustic guitar (Parlor):** Decke und Luft sind als gekoppeltes Modell mit den
  Messwerten von Hess (2013) gerechnet. Seine Spitzen liegen bei 116 und 219 Hz, wie an der
  Gitarre gemessen.

## Geändert

- Noten reichen von **A0 bis C8**, vorher C1 bis C7. Die Piano-Roll hat jetzt 88 Zeilen.
  Benutzerhandbuch 1.188.0.

## Intern

- Dateien: `.lvinst` Fassung 10 (`strings`, die Felder der neuen Saiten), `.lvloop` Fassung 8 (Saite einer
  Note, `stringChoice`).
- Neue Tests:
  - `test_LoopSaiten` mit 10 Fällen;
  - `test_LoopSaiteKs` mit 10 Fällen;
  - `test_LoopSaiteWg` mit 9 Fällen;
  - `test_LoopKorpus` mit 5 Fällen.

  Viele davon messen am gerechneten Ton: Tonhöhe, Abklingzeit, fehlende Teiltöne, Steifheit,
  Humbucker-Lücke, Piezo-Hochpass, Korpus-Spitzen.
- Mitgelieferte Instrumente und Loops klingen Bit für Bit wie vorher (Loop Werte).
- `Offene_Punkte.md` 1.259.0.
