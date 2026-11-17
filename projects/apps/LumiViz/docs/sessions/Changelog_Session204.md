# Changelog Session 204 (2026-11-17)

> Der Umlauf-Filter der gezupften Saite ist entworfen und entschieden (UL1 a–UL3 a): je Taste eine vorab gerechnete
> Tafel mit einem Verlustfilter 8. Ordnung und einer Kette von Biquad-Allpässen für die Steifheit. Am Ton mit Körper
> liegt der Entwurf höchstens 0,36 LU und 1,7 Cent neben dem genauen Umlauf. Kein Programmcode — der Bau folgt in 34k-7.

## Neu

- **Entwurf über den ganzen Umfang** (295 Tasten der fünf Instrumente, starrer Steg):
  `docs/modellierung/gezupfte-saite/proben/2026-11-17_S204_umlauf_umfang.py` (Verlust in drei Fassungen, gleiche
  Allpässe gegen Biquads) und `…_S204_dispersion_biquads.py` (Biquad-Kette je Taste angepasst; Ausgaben für die Zählung
  nach UL1 a und für „jeder Teilton bis 20 kHz“).
- **Drei Prüfungen zu UL3 a:** `…_S204_umlauf_mit_koerper.py` (im Prüfstand mit Körper), `…_S204_kurzer_umlauf.py`
  (hohe Tasten mit Teiltönen über 10 kHz), `…_S204_verlauf_buende.py` (Beiwerte längs der Bünde, gebundene Anpassung
  je Saite). Je Skript die Ausgabe als `.txt` daneben.

## Geändert

- `docs/audio/Instrumente_IC_Konzept.md` 0.54.0: UL1 a (es zählen Teiltöne mit Abklingrate ≤ 100 1/s), UL2 a (je
  Teilton ±3 Cent, Abklingzeit im Faktor nach HW1 a), UL3 a (Verfahren), Absatz „UL3 a geprüft“.
- `docs/Offene_Punkte.md` 1.396.0: Zeile 34k Punkt (23).

## Befunde

- Die Hochfrequenz-Vorgabe aus Bank & Välimäki 2003 (die letzten fünf Sollpunkte in einen Ein-Pol überblenden) war die
  Ursache der großen Fehler an den obersten Teiltönen; ohne sie trifft Ordnung 8 jeden Teilton bis 21,6 kHz auf wenige
  Prozent.
- Die Quelle gewichtet den Fehler der Abklingzeit in Sekunden; die Hörschwelle ist ein Faktor. Daraus das Gewicht des
  relativen Fehlers 1 / (g ln g)².
- Gleiche Allpässe 1. Ordnung halten 10 Cent nur an 103 von 295 Tasten, die angepasste Biquad-Kette an 290 (3 Cent: 279).
- Bei kurzem Umlauf entscheidet der ganzzahlige Verzug über den Fehler (0 bis 60 Cent); er und das Thiran-Glied gehören
  in die Anpassung.
- Längs der Bünde springen einzeln angepasste Ketten; eine gebundene Anpassung je Saite (Polynom je Biquad) hält an den
  Saiten 6–2 höchstens 4,5 Cent. Der Verlustfilter lässt sich zwischen Nachbarbünden mitteln.

## Offen

- Gebundene Anpassung mit Stützstellen zwischen den Bünden, für Saite 1, Laute und Oud; die Fassung für kurze Umläufe
  über alle Tasten; Cembalo 71 / 76; Slide schneller als ein Halbton in 50 ms.
- Für die Lage einzelner Teiltöne ist keine Hörschwelle an einer Quelle belegt (Järveläinen, Välimäki & Karjalainen
  2001 nicht erreicht).
