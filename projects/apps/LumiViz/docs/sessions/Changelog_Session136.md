# Changelog Session 136 (2026-10-19)

> **Vorgaben je Saiteninstrument, Hammer-on, Pull-off, Slide und Bend.** Jedes mitgelieferte
> Saiteninstrument schlägt mit dem Abstand seiner Bauart an; vier Beispiel-Loops zeigen die
> Spielweisen. Verbundene Noten auf einer Saite klingen jetzt als ein Ton — eingeben lassen sie
> sich erst mit den Tabs. Steuerdokument: `docs/audio/Saiten_Konzept.md` (§3, §5r–§5t, §6c).

## Neu

- **Spread je Instrument** aus dem Saitenabstand seiner Bauart (Nylon 8,0 ms, Steelstring 7,5,
  E-Gitarren 7,0, E-Bass 12,5, Kontrabass 17,5, Ukulele 9,5, Violine 7,0); die Bässe dämpfen bei
  Palm mute leichter (Mute 0,37). Die mitgelieferten Loops schlagen mit dem Strum ihrer
  Instrumente an.
- **Tap level** gilt bei 220 Hz: tiefe getippte Töne klingen kräftiger, hohe leiser.
- **Vier Beispiel-Loops:** `strum_demo`, `palm_mute_demo`, `harmonics_demo`, `tapping_demo`.
- **Hammer-on, Pull-off, Slide, Bend** im Klang: eine Kette verbundener Noten ist ein Ton der
  Saite; ein Slide gleitet von Bund zu Bund (bundlos glatt) und kommt auf dem Schritt der
  Zielnote an; ein Bend biegt die Tonhöhe nach einer Kurve. In `.lvloop`-Dateien lassen sie sich
  schon setzen (Fassung 10); die Bedienung kommt mit den Tabs.
- **Creator:** Haken **Fretless** in der Zeile Tuning; in *Playing* **Slide time** und **Legato
  level**.
- **Handbuch 1.190.0:** Spielweisen in Creator und Instrument-Editor, Zoom in beiden Editoren.

## Geändert

- Die Saiten *Extended KS* und *Waveguide* lesen ihre Verzögerung genauer (Lagrange 5. Ordnung):
  hohe Töne etwas heller und gleichmäßiger.
- `.lvloop` Fassung 10, `.lvinst` Fassung 12.

## Intern

- Neue und erweiterte Tests in `test_LoopSpielweise`, `test_LoopSpielweiseKlang`,
  `test_LoopCreatorSpielweise`; 1367 Fälle; Loop und Instrument unter beiden Compilern grün; kein
  voller Lauf (erst mit Tabs).
- Sichttest-Steuerung: Übergabe aus SichtTest_Helper (`Sichttest_Steuerung.md` 1.2.0).
- `Saiten_Konzept.md` 0.31.0, `Offene_Punkte.md` 1.263.0.
