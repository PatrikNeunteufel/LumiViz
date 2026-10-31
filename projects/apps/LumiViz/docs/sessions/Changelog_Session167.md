# Changelog Session 167 (2026-10-30/31)

> **Chords öffnet sofort, und eigene Muster, Clips und Spuren lassen sich in andere Loops mitnehmen.** Das Fenster
> *Chords* hielt beim ersten Öffnen die Oberfläche rund 20 Sekunden an; die mitgelieferten Akkorde liegen jetzt in sechs
> Dateien statt in 6373 und werden nur für die Stimmung des Instruments gelesen. Neu sind drei Dateiarten: eine Basis
> (`.lvpattern`), ein Clip mit Wiederholungen und Effekten (`.lvclip`) und eine ganze Spur (`.lvtrack`) — exportiert und
> eingesetzt über die Menüs der Taktachse, umgerechnet, wenn der andere Loop eine andere Taktart oder ein anderes Raster
> hat. Steuerdokumente: `docs/audio/Saiten_Konzept.md` §5ze, `docs/audio/Loop_Konzept.md` §4o.

## Neu

- **Muster, Clips und Spuren als Dateien** (Stufe 33, Entscheide PE1, PE2 a, PE3 b, LC1 a, PE1-1 a):
  - Rechtsklick auf einen Clip → *Export pattern...* / *Export clip...*; am Kanalnamen *Export track...*; auf einem
    freien Takt *Import pattern...* / *Import clip...*, auf der freien Fläche *Import track...*.
  - Die Dialoge schlagen den Benutzerordner vor (`patterns`, `tracks`), speichern und laden geht überall.
  - Gleicher Name und Inhalt: mitbenutzt; anderer Inhalt: `<Name>_2`. Kein Platz am Takt: ein neuer Kanal.
  - Andere Taktart oder anderes Raster: umgerechnet nach Schlägen — Noten genau, Drum-Schläge auf den nächsten Schritt.
  - Datensicherung (*Patterns*, *Tracks and clips*), Publish (mit den eigenen Instrumenten, Kits, Samples).
- **Datei-Icons** für `.lvpattern`, `.lvclip`, `.lvtrack`, `.lvchords` und `.lvp` (Equalizer-Balken, LP1 a), eingetragen
  im Skript `register_file_icons.ps1`.

## Geändert

- **Chords** (AK1 a): mitgelieferte Akkorde als Sammlungen `<stimmung>.lvchords`; gemessen 25,6 s → 34 ms beim ersten
  Lesen. Namen wie `guitar/c_major_1` bleiben, eigene Akkorde bleiben einzelne `.lvchord`.
- Benutzerhandbuch 1.204.0 (§16, §17, §8).

## Intern

- `loop/Austausch` (herauslösen, Hülle mit kleinem Loop, `rechneUm`, `setzeEin`, `austauschText`); `loopDateiText` in
  `Loop.hpp`; Publish-Arten `Muster`, `Clip`, `Spur`; `setzeMitgelieferteLoopsFuerTests` (KF1 a, wie W1 a).
- Tests: 1685 Cases (+8: `test_LoopAustausch` 4, `test_LoopAustauschMenue` 1, `test_PublishAustausch` 2, AK1 1).
  Voller Lauf unter Release-clang und Testing-MSVC je 1684 / 1685 vor KF1 a (rot nur `Kits 21c` durch eine
  Arbeitsdatei), danach die Kit-Fälle grün.
