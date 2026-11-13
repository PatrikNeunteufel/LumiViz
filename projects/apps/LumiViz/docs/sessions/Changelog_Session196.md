# Changelog Session 196 (2026-11-13)

> Die Bibliothek `InstrumentBody` rechnet jetzt Platten mit Löchern und einen Luftraum dahinter. Die Oud ist der erste
> mitgelieferte Körper mit Luftraum. Die Stufe 34k steht weiter bei 34k-5 von elf: Laute, Cembalo, Harfe und Gitarre
> fehlen noch. Im Programm ist davon noch nichts zu hören — kein Instrument benutzt die Oud.

## Neu

- **Löcher in der Platte** (KQ7 a): der Bauplan trägt `plate.holes`, eine Liste geschlossener Innenumrisse. Das Netz
  spart sie aus, ihr Rand ist frei, in Masse und Abstrahlung zählt ihre Fläche nicht. Prüffall: Kreisring gegen
  Kirchhoff auf 0,5 %.
- **Luftraum** (KQ8 a, F-K1 a): der Bauplan trägt `cavity` (Volumen, Mündungskorrektur, Güte, Quelle).
  `InstrumentBody/Luftraum.hpp` koppelt die Moden der Platte und die Luft in den Löchern über den Druck im Hohlraum.
  Prüffälle: Helmholtz-Resonator, Kasten ohne Loch, zwei gekoppelte Schwinger, Vergleich mit einer dichten Rechnung.
- **Die Oud** (KQ10 a): `asset/loop/instrument_bodies/oud.lvinstbodyplan` (Schreiber `make_oud.py`) und
  `oud.lvinstbody` mit 243 Moden bis 5 kHz. Sechs Stegpunkte, je Chor einer (Tasten 36–60). Die gerechneten Moden
  treffen die gemessenen Spitzen bei 101 / 203 / 280 / 362 Hz mit −1 / +2 / −2 / 0 %.
- `InstrumentBodyTool`: der Bericht nennt den Hohlraum, der Schalter `--shapes <datei>` schreibt Knoten, Dreiecke und
  Formen der Moden.
- Tests `test_LoopKoerperLoch` (5 Fälle), `test_LoopKoerperLuftraum` (5 Fälle).

## Geändert

- **`.lvinstbodyplan` Fassung 4** (`plate.holes`, `cavity`, beides immer geschrieben; Fassung 1–3 bleibt lesbar). Die
  drei Steinway-Baupläne stehen in Fassung 4, ihre Körper sind neu gerechnet und gleich bis auf Rechenzeit und Abdruck.
- **`.lvinstbody` Fassung 3**: jede Mode trägt `airShare`, den Anteil ihrer Bewegungsenergie in der Luft der Löcher.
  Fassung 2 bleibt lesbar; die Steinway-Körper bleiben Dateien der Fassung 2.
- Der Wächter über die mitgelieferten Körper rechnet die Dämpfung mit Luftanteil und Güte nach und verlangt die Oud.
- Der Test „Antwort schnell gegen exakt“ nimmt Stegpunkte und Hörorte des jeweiligen Körpers statt fester Tasten.
- `InstrumentBody` 0.17.0, Konzept 0.29.0, Offene Punkte 1.350.0.

## Entschieden

- KQ8 a: zuerst der Luftraum, dann die Oud erneut.
- KQ9 a: Oud mit Balken bis an den Rand — **ersetzt durch KQ10 a**.
- KQ10 a: Oud mit Balken wie in der Quelle gezeichnet, drei Löchern, Luftraum 20,6 l, Decke 1,5 mm, ohne Steg. Ziele
  sind die Spitzen der gemessenen Kurve (Abb. 6 der Quelle), nicht die Tabelle „gemessen gegen gerechnet“.

## Offen

- An der Oud: die gemessene Spitze bei 139 Hz hat kein Gegenstück, die gerechnete Mode bei 169 Hz keine gemessene
  Spitze; die Höhen der Spitzen; kein Steg; die Dämpfung über 400 Hz.
- Nicht geprüft: wie das Programm die Oud in der Körper-Auswahl eines Flügels behandelt (sie hat nur sechs Tasten).
- 34k-5: Laute und Cembalo je mit Luftraum, Bauplan der Harfe, Modalfit der Gitarre. Danach 34k-6.

## Tests

Voller Lauf 1913 / 1913, 0 Skips unter Release-clang (837 s) und Testing-MSVC (886 s).
