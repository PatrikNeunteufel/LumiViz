# Changelog Session 192 (2026-11-12)

> Der Kern der durchschlagenden Zunge folgt jetzt der Anschlagstufe und klingt je Taste so lange aus, wie er es
> tatsächlich tut (34j-7 fertig). Vier neue mitgelieferte Instrumente spielen auf ihm: Accordion, Harmonium, Melodica,
> Harmonica (34j-8), mit einem Beispiel-Loop und je einem Loop ihrer Nuancen (34j-9). Damit ist die Stufe 34j
> abgeschlossen (34j-10).

## Neu

- Vier Instrumente mit durchschlagender Zunge im Fenster *Instruments*: Accordion und Harmonium unter *Keys*, Melodica
  und Harmonica unter *Wind* — gerechnet, je Taste gestimmt, auf den gemeinsamen Pegel gebracht, ungemessen; Genre
  *Folk*.
- Der Loop *Free reeds demo* (Folk): Akkordeon mit Melodie und Bass-Seite, das Harmonium schwillt über die Expression am
  Clip an, danach Melodica und Mundharmonika. Je Instrument ein Loop der Nuancen (Lage, Stärke, Länge, Expression;
  Spielweisen *Vibrato* und am Akkordeon und an der Melodica *Pallet half closed*).
- Die Hörliste `zunge_34j` für HoerTest_Helper (53 Klänge, 51 Punkte); sie ersetzt die Liste `10_zunge` aus
  InstrumentsCalculation.
- Die Anschlagstufe wirkt: je Taste gemessen, bei welcher Stärke der Ton 2, 4 … 20 dB leiser klingt; eine Note bei
  −x dB klingt am Ton nachgemessen x LU leiser, höchstens bis zur Dynamik der Taste (Akkordeon und Melodica auf
  1,8 LU, Harmonium und Mundharmonika auf 0,3 LU). Mit Expression gilt Stärke mal Expression.
- Der Nachklang je Taste: der längste Ausklang über Haltedauern von 0,1 bis 4 s und drei Stärken, bis 60 dB unter der
  lautesten Stelle der Note — kein Ton wird abgeschnitten (Akkordeon bis 4,6 s im Bass, Mundharmonika bis 0,34 s).

## Entschieden

- ZV13 a: der Nachklang ist die Hülle über Haltedauer und Stärke (präzisiert ZV11 a, das die Haltedauer offen ließ).
- ZV14 a: der Kern bleibt; die Klangfarbe wird an der gespielten Tonhöhe gemessen.

## Behoben

- Der Befund „mit mehr Wind wird der Ton dunkler“ (S191) war größtenteils ein Fehler der Messung: sie las die Teiltöne
  an der Sollfrequenz, die Tonhöhe sinkt aber mit dem Druck. Richtig gemessen wird der Ton mit dem Wind heller
  (Harmonium und Mundharmonika an jeder Taste, Melodica an 6 von 8, Akkordeon an 12 von 17).

## Bekannt

- Akkordeon E3–A3 und Melodica F3–A3 werden mit dem Wind leicht dunkler (das Ventil drosselt dort); die Zunge des
  Harmoniums schwingt mit mehr Druck weiter aus, als gemessen. Beides wird in der Hörprobe 34j-9 geprüft.
- Das Stimmen aller vier Instrumente dauert jetzt 41 s statt 13 s (18 Ausklänge je Taste).

## Geprüft

- Messungen `docs/modellierung/zunge/proben/2026-11-12_S192_messung_34j7.md` und `…_messung_zv12.md`, Befund
  `docs/modellierung/zunge/2026-11-12_S192_zv12_klangfarbe_ueber_druck.md`, Quellenbericht unter `recherche/`.
- Eng 101 / 101 unter Release-clang und Testing-MSVC, die Stimmung aller vier unter beiden Bit für Bit gleich; nach
  34j-8 der Bereich der Instrumente, Pegel, Werte und Quellen 292 / 292; nach 34j-9 Loops, Zunge, Creator, Werte, Klangfassung, Instrumente, Pegel, Quellen und Hörlisten 161 / 161 unter beiden. Kein voller Lauf (kommt mit
  34j-10).
- Die Rechenzeit der Zungen nur gemessen, nicht geprüft (Kühlung): 1,3 % der Echtzeit am Akkordeon, 0,6 % an den übrigen.
- Nach 34j-10 der volle Lauf: 1888 / 1888, 0 Skips unter Release-clang (13 min) und Testing-MSVC (13 min); die
  Quellpakete `instrumentcore` 0.6.0, `instrumentmodels` 0.7.0, `instrumentmeasure` 0.4.0 mit gültigen Summen.
