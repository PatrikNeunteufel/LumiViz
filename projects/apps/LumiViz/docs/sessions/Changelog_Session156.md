# Changelog Session 156 (2026-10-27)

> **Das Einstellen der Hammer-Saite an den Abklingkurven trägt nicht; ein neuer, letzter Anlauf ist geplant.** Die
> Probe vor dem Einstellen lief zweimal: die Suche fand keine glatte, eindeutige Lösung. Ein Fehler im Vergleich ist
> behoben, das Ergebnis blieb. Für den nächsten Anlauf misst LumiViz das Modell an der Form der Aufnahme und sucht mit
> einem Verfahren für glatte Ausgleichsrechnungen; scheitert auch er, wird nach dem Gehör eingestellt. Die Proben rechnen
> schneller, Bit für Bit gleich. Eingestellt ist noch nichts, jedes Instrument klingt wie vorher.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (GP1 a, RV1 a, PB1 a, ZF1 a bis ZF9 a, ZZ1 a).

## Neu

- **`docs/modellierung/hammer-saite/proben/`** — die Berichte aller Proben und Gegenproben je Lauf, mit Index
  (`README.md`): erster Lauf (S155, abgebrochen), Zeit je Phase, beide Proben S156, Abstand je Kurve an den Sprüngen.

## Intern

- **Hammer-Saite:** die Längskraft liest die Formen der Moden flach statt je Mode verstreut — 10–13 % schneller, Bit
  für Bit gleich (Bezug, `Loop Werte`, Klangfassung unverändert); Rechenzeit Taste 40 3,73–3,82 % der Echtzeit.
- **Probe:** rechnet auf einem Pool mit 11 Threads (Punkte der Schnitte zugleich, Suchen nebeneinander), 54 statt rund
  120 min; Fall „Pool wie nacheinander“. Werkzeuge `LUMIVIZ_75_ZEIT` (Zeit je Phase, Durchsatz je Zahl der Threads) und
  `LUMIVIZ_75_SPRUNG` (Abstand je Kurve an zwei Nachbarpunkten).
- **Befund Rechner:** der Prozessor steht unter jeder Last an der Wärmegrenze (Tctl 94–99,6 °C); mehr als 11 Threads
  machen die Rechnung langsamer.
- **Kurvenabstand:** eine fehlende Kurve des Modells zählt als sofort bis −100 dB gefallen statt als „klingt gar nicht
  ab“ (RV1 a); Fall „fehlende Kurve des Modells (RV1 a)“.
- Tests: 1632 Cases; voller Lauf 1631 / 1631, 0 Skips unter Release-clang und Testing-MSVC vor RV1 a; nach RV1 a eng
  65 / 65 unter beiden.
