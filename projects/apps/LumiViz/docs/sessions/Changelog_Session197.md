# Changelog Session 197 (2026-11-13)

> Laute und Cembalo sind als Körper mit Luftraum dazugekommen. `InstrumentBody` kennt jetzt Öffnungen des Hohlraums,
> die nicht in der Platte liegen. Die Harfe ist gelesen und ihr Holz geeicht; ihr Bauplan hängt an einer offenen
> Frage zum Rand des Bodens (KQ18). Die Stufe 34k steht weiter bei 34k-5 von elf. Im Programm ist davon noch nichts
> zu hören — kein Instrument benutzt die neuen Körper.

## Neu

- **Die Laute** (KQ11 a, KQ12 a): `asset/loop/instrument_bodies/lute.lvinstbodyplan` (Schreiber `make_laute.py`) und
  `lute.lvinstbody` mit 128 Moden bis 5 kHz. Form der Rauwolf-Laute 1598, alle Längen mal 625/677,4; acht
  Stegpunkte, je Chor einer (Tasten 38–67). Die Stegadmittanz mit Stegleiste und Saitenlast trifft vier gemessene
  Spitzen (132 / 277 / 341 / 476 Hz, Woodhouse 2021) in der Lage auf höchstens 4,6 % und in der Höhe auf höchstens
  4,7 dB.
- **Das Cembalo** (KQ13 a–KQ16 a): `harpsichord.lvinstbodyplan` (Schreiber `make_cembalo.py`) und
  `harpsichord.lvinstbody` mit 389 Moden. Der Boden des Dulcken 1755 nach Bader, Plath & Kontopidis 2025: Umriss,
  Stege, Leisten und die gemessene Dicke als Raster. 61 Stegpunkte (Tasten 29–89). Helmholtz bei starrer Wand 37,0 Hz
  wie die Quelle; die gekoppelte Luftmode liegt bei 31,7 Hz, 14 % unter der Messung — bekannte Abweichung.
- **Öffnungen des Hohlraums außerhalb der Platte** (KQ17 a): der Bauplan trägt `cavity.openings` (Ort, Höhe über der
  Platte, Fläche, Lippenlänge, Quelle). Die Rechnung nimmt Löcher der Platte und Öffnungen parallel; eine Öffnung
  strahlt als freie Punktquelle.
- `Auflager::Gestuetzt` in `InstrumentBody` (Rand hebt sich nicht, dreht frei); `InstrumentBodyTool` mit den
  Probe-Schaltern `--free-rim` und `--supported-rim`.
- Tests in `test_LoopKoerperLuftraum`: die mitgelieferte Laute, das mitgelieferte Cembalo, Öffnungen außerhalb der
  Platte.
- Berichte unter `docs/modellierung/gezupfte-saite/`: `2026-11-13_S197_laute.md`, `…_cembalo.md`,
  `…_harfe_gelesen.md`, mit Leseskripten, gelesenen Daten und Kontrollbildern unter `proben/`.

## Geändert

- **`.lvinstbodyplan` Fassung 5** (`cavity.openings`, immer geschrieben; Fassung 1–4 bleibt lesbar). Alle sechs
  mitgelieferten Baupläne stehen in Fassung 5, ihre Körper sind neu gerechnet und gleich bis auf Rechenzeit und
  Abdruck. Die drei Steinway-Körper sind dabei Dateien der Körper-Fassung 3 geworden (`airShare` 0).
- Der Wächter über die mitgelieferten Körper verlangt Laute und Cembalo.
- `InstrumentBody` 0.19.0, Konzept 0.32.0, Offene Punkte 1.353.0.

## Entscheide

KQ11 a (Ziele der Laute in Lage und Höhe), KQ12 a (Form mal 625/677,4), KQ13 a (Körper des Cembalos = Boden des
Dulcken), KQ14 a (Dicke als Raster — im Format vorhanden), KQ15 a (Tiefe 20,7 cm, Luftmode als bekannte Abweichung),
KQ16 a (Randtasten), KQ17 a (Öffnungen außerhalb der Platte). **Offen: KQ18** (Rand des Harfenbodens).

## Tests

doctest zählt 1916 Fälle. Kein voller Lauf: die geänderte Rechnung rufen nur `InstrumentBodyTool` und die
Körper-Tests, das Programm liest nur Körper. Gelaufen unter Release-clang und Testing-MSVC: `*Loop Koerper*`
71 / 71; nach der neuen Bauplan-Fassung `*Loop Koerper*,*Loop Werte*,*Klangfassung*,*Hammer*` 107 / 107, dazu der
Vergleich der sechs Körperdateien Feld für Feld.

## Offen

- Harfe: KQ18, Kapitel 7 von Bell 1988 (fertige Harfe mit Saiten), höhere Luftmoden im Kasten.
- Gitarre: Modalfit am Archiv Mores.
- Cembalo: keine gemessene Mode des Bodens; Dämpfung für alle Frequenzen gleich.
- Laute: gemessene Spitzen bei 150 und 378 Hz ohne Gegenstück.
