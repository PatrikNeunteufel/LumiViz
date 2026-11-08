# Changelog Session 177 (2026-11-08)

> **Messen, Setzen und Quellen als Bibliotheken.** Die Teilschritte 34e und 34f sind fertig: die Messkette und das Setzen
> stehen in der Bibliothek `InstrumentMeasure`, Lautheit und WAV in `AudioCore`; jeder Wert eines Rechenkerns nennt seine
> Quellen, ein Register und ein Wächter halten fest, was ausgeliefert werden darf (`docs/audio/Instrumente_IC_Konzept.md`
> 0.9.0).

## Neu

- **Bibliothek `InstrumentMeasure`** (`projects/libs/InstrumentMeasure`, STATIC, `lumi::messung`): die Messkette aus den
  Unit-Tests (Teiltöne, Abklingen, Schwebung, Struktur, Zielsumme, Hörschwellen, Probe, Levenberg–Marquardt,
  Nelder–Mead), dazu aus InstrumentsCalculation die Bausteine B1–B8 (Teiltonverfolgung, YIN, Vibrato, Rest,
  Einschwingen, Modalfit nach ESPRIT, Bänder, Gleiten), der Zugang zu lokalen Aufnahmen und das Setzen (`setzen`, mit dem
  LM je Block, Beobachter und Wiederaufnahme). Quellpaket `instrumentmeasure`.
- **Bibliothek `AudioCore`** (`lumi::audio`): Lautheit nach EBU R128 und das Lesen von WAV ohne Qt, aus der App
  herausgelöst (ME1 a) — Composer, Loop und Messung hängen daran. Quellpaket `audiocore`.
- **`.lvinst` Fassung 22:** jede Kurve einer Kern-Schicht nennt in `sources` die Schlüssel ihrer Quellen (WZ1 a). Der
  *Grand Piano* steht in Fassung 22 mit unveränderten Werten.
- **Quellenregister** `asset/loop/instrument_quellen.json` (WZ2 a) und **`instruments/CREDITS.md`** mit den Nennungen der
  Quellen der mitgelieferten Instrumente (Salamander, RT-0425, Stulov & Kartofelev, Rigaud et al.) — geht mit neben die Exe.
- **`InstrumentTuneTool`:** stimmt die gestimmten Werte einer `.lvinst` und setzt ihren Bezug (`--reference`, `--all`,
  `--check`) — für die Klassen, die gestimmte Werte tragen (ab 34j).

## Geändert

- Die Flügel-Suche (Werkzeug der Unit-Tests) setzt über `setzen` (ME3 a); ihre Dateien und die Wiederaufnahme bleiben.
- Beide LM werten den letzten kleinen Schritt aus und nehmen ihn, wenn er den Rest senkt (ME4 a, wie IC).
- `InstrumentCore` 0.2.0: `zerlegeSymmetrisch` (ME2 a), Quellen an Kurve und Angabe, der Bezug in der Tafel
  (`normiereNachTafel`).

## Intern

- Tests **1796 Cases**; voller Lauf 1796 / 1796 unter Release-clang und Testing-MSVC (nach 34e 1789 / 1789).
  `salamander_*.txt` beim Neumessen ohne Diff, `Loop Werte` und Flügel-Bezug unverändert, `klangfassung.txt` nur mit neuen
  Abdrücken (Fassung 22).
- Neue Wächter: `*Instrument Quellen*` (keine Quelle „nicht ausliefern“, jede Pflichtnennung in `CREDITS.md`),
  `*Instrument Stimmen*` (gestimmte Werte und Bezug nicht veraltet); `.gitignore` hält Audio und Messdaten unter
  `modellierung/**/aufnahmen|daten/` heraus.
- Übernahme-Journal 0.4.0 (J19–J30), Modul-Doku `InstrumentMeasure.md`, `AudioCore.md`, `InstrumentCore.md` 0.1.3.
