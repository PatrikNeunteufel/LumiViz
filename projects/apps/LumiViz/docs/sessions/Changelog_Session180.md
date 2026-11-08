# Changelog Session 180 (2026-11-08)

> **Woher die Zahlen kommen.** Jedes Instrument aus einem Rechenkern nennt jetzt für jeden seiner Werte die Quellen,
> nicht nur für die Kurven — Trommeln, Stabspiele und Flügel. Damit ist Teilschritt 34h abgeschlossen
> (`docs/audio/Instrumente_IC_Konzept.md` 0.12.0, QM1 a).

## Neu

- **Quellen je Wert:** auch Zahlen, Schalter, Auswahlen und Listen eines Kern-Instruments tragen ihre Quellen
  (`.lvinst` **Fassung 23**, Block `coreSources`). Eine ältere Datei bekommt die Quellen der Vorgabe.
- **Je Instrument genau:** jede Trommel nennt nur die Quellen, die ihre Werte tragen (2 bis 5 der 9 Quellen der Klasse);
  Celesta und Reed electric piano nennen das Hammermodell nach Stulov, das Tine electric piano Sønderbo und Falaize &
  Hélie.

## Geändert

- **`instruments/CREDITS.md`** nennt 19 Quellen, je mit den Instrumenten und Werten, die sie benutzen (neu unter anderem
  Avanzini & Marogna, Bilbao et al., Dahl, Packer, Sathej & Adhikari, Torin et al., Wagner).
- Die Herkunft der Schlägel der Trommeln sagt richtig, dass nur der Stock an der Kontaktzeit gesetzt ist.
- Die 17 mitgelieferten Kern-Instrumente in Fassung 23 — der Klang ist unverändert.
