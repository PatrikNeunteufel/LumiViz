# Changelog Session 145 (2026-10-22)

> **Publish nimmt eingefrorene Clips mit; Freeze im Handbuch; der Flügel ist geplant.** Ein Paket aus
> Publish trägt jetzt die Wellenformen eingefrorener Clips — der Empfänger spielt sie sofort, wo sie bei ihm
> passen. Freeze steht im Benutzerhandbuch. Für den Konzertflügel (Stufe 31) sind Verfahren, Bauplan und
> Datei entschieden; gebaut ist davon noch nichts.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 und §4.4 (FP1 a bis FP3 a, BL1 a, BL2 a, HS1 a,
> HS2 a, HA1 a bis HA4 a).

## Neu

- **Publish mit eingefrorenen Clips:** im Dialog *Publish* unter dem Loop die Gruppe **Frozen clips (n)** mit
  einer Zeile je Clip („ready · 10.1 MB“, „computing… 40 %“); der Haken **Include frozen clips** (Vorgabe an,
  gemerkt). Fehlende Wellenformen rechnen ab dem Öffnen; ein Klick auf **Publish** wartet darauf („Freezing
  clip 1 of 2 … 40 %“) und packt dann von selbst.
- **Import package** zeigt „Frozen clips (2) → frozen clips, … MB“ und legt nur Wellenformen ab, die beim
  Empfänger genauso klingen (dieselben Instrumente und Samples, auch unter neuem Namen, dieselbe Abtastrate);
  die Meldung danach: „Frozen clips: 2 of 3 ready to play — 1 will be computed here“.
- **Benutzerhandbuch 1.202.0:** §16 „Clips einfrieren: Freeze“ (Sanduhr, Freeze clip / Unfreeze clip, die
  Marken, wo die Wellenformen liegen), Publish mit eingefrorenen Clips, §8 *Freeze hint*.

## Geändert

- Ein Paket bleibt in Fassung 2; die Wellenformen stehen unter `frozen/` und im Manifest unter
  `package.frozen` — ein älteres LumiViz übergeht sie.
- **Bauplan Stufe 31 (Hammer-Saite):** modal gerechnet nach Bank, Zambon & Fontana 2010; acht Teilschritte
  31a–31h; die `.lvinst` beschreibt den Flügel mit Kurven über die Tastatur, *Excitation* und *Modal* an der
  Schicht, einen Block *Keyboard* am Instrument; die Anschlagstärke kommt gestuft in den Namen eines Tons.
  Dazu drei Beispiel-Loops nach Chopin, von Hand aus gemeinfreien Ausgaben vor 1900.

## Intern

- `loop/Freeze::legeFrozenDaten`, `paket/Packen` (`frozenTeile`, `frozenPfad`), `paket/Import`
  (`PaketWahl::rate`), `UI/dialogs/PaketDialoge` (`FreezeTon`, `freezeTonVon`); die Meldungen von Publish über
  `fuehreDialogAus`.
- Neue Tests `test_PublishFreeze`, zwei Fälle in `test_PublishDialoge`; 1503 Fälle. 30i-1 und 30i-2 unter
  beiden Compilern grün, der Bereich mit Publish unter Release-clang; **kein voller Lauf**.
