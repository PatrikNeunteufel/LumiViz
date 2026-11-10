# Changelog Session 185 (2026-11-10)

> **Die Loops mit Kern klingen in voller Lautstärke.** Kein mitgelieferter Loop kommt über −1 dBFS; dazu der Bezug von
> Hoertest entschieden und die Hörlisten auf den Stand nach LP2 b gebracht.

## Geändert

- **Loops der Instrumente mit Kern** (`asset/loop/modelled_instruments/`, ohne `full/`): die Lautstärke kommt aus einer
  gemessenen Tafel (`asset/loop/loop_lautstaerke.py`) — so laut wie möglich, die Spitze bis −1,05 dBFS. Die Loops der
  Nuancen klingen im Median rund 10 dB lauter (−32,2 → −22,4 LUFS).
- **Wächter** `Loop Spitze LV1 a`: kein mitgelieferter Loop über −1 dBFS (zwei Durchläufe, ohne Begrenzer; Ausnahme
  `nocturne_demo`, der mit Begrenzer ausgeliefert wird).
- **Hörlisten** `stab_34g` und `klasse1_34i` neu geschrieben: Kalimba, Wurlitzer und Clavinet in der Fassung nach S183.
- **Test:** 31 bekannte Genres (Fusion seit S182).

## Entschieden

- **HF2 a:** Hoertest kommt wie Sichttest neben die App (`hoertest\Hoertest.exe`), gebaut nach dem ersten Release von
  HoerTest_Helper.
- **HF3 a:** Prüfwerkzeuge (Sichttest, Hoertest) gehören in jeden Arbeitsbau und nie in eine Auslieferung.

## Geprüft

- Voller Lauf nach CMakeCraft v0.14.1: 1855 / 1855 unter Release-clang und Testing-MSVC.
- Alle 14 Hörlisten gültig (`Hoertest --liste-pruefen`).
