# Changelog Session 154 (2026-10-26)

> **Der Hammer trifft die Saiten eines Chors jetzt ungleich.** Bei einem echten Flügel liegt der Filz nie ganz gleich
> an allen drei Saiten an; dadurch löschen sie sich im Nachklang nicht fast vollständig aus, wie es der gerechnete Ton
> bisher tat. Das Modell kann das jetzt rechnen. Eingestellt ist es noch nicht: das Einstellen an der Aufnahme hat sich
> als die eigentliche Arbeit herausgestellt und wird in der nächsten Sitzung neu aufgesetzt. Bis dahin klingt jedes
> Instrument wie vorher.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (TB1 a, SZ1 a, SZ2 a, US1 a bis US7 a, US6-1 a, RB1-1 a,
> SZ3 a, SM1 a bis SM4 a).

## Neu

- **Ungleicher Stoß je Saite** an der Hammer-Saite: zwei neue Kurven in der `.lvinst` (Fassung 19), *Strike unevenness
  soft* und *loud* — leise und laut verschieden, dazwischen nach der Anschlagstärke. Vorerst überall 0; ältere Dateien
  bekommen die Vorgabe. In einer Probe an C4 sinkt der tiefste Einbruch damit von 31 auf 7,5 dB.

## Intern

- Die Messung gegen *Salamander* misst die **Tiefe der Einbrüche** je Periode der Schwebung (die alte Größe blieb bei
  rund 17 dB stehen) und endet 90 dB unter der Spitze (am Modell zählte sie sonst Reste der Rechnung als Einbrüche).
- Messwerte von *Salamander* neu: Tiefe der Einbrüche (gesamt, leise, laut), Abklingen von Teilton 2 und 3.
- Die Moden je Teilton sind eine eigene Funktion (`hammerTeiltonModen`) — der Ton rechnet Bit für Bit wie vorher.
- Werkzeuge zur Untersuchung (Bilder der Hüllkurven, Tiefe im Chor, Raster aus Stoß und Winkel, Diskant, Blindanteil
  des Bodens, Moden gegen Ton) und ein neues Setzen in zwei Stufen; das Setzen ist noch nicht brauchbar, nichts davon
  ist in die Vorgabe eingetragen.
- Klangfassung 3 (eingefrorene Clips mit Hammer-Saite werden neu gerechnet).
- Release-clang: die Fälle zu Hammer-Saite, Werten, Klangfassung, Instrument, Salamander und Teiltonmessung grün;
  **Testing-MSVC und der volle Lauf stehen seit S153 aus.**
