# Changelog Session 192 (2026-11-12)

> Der Kern der durchschlagenden Zunge folgt jetzt der Anschlagstufe und klingt je Taste so lange aus, wie er es
> tatsächlich tut (34j-7 fertig). Noch kein mitgeliefertes Instrument nutzt ihn (die Dateien kommen mit 34j-8).

## Neu

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
- Eng 101 / 101 unter Release-clang und Testing-MSVC, die Stimmung aller vier unter beiden Bit für Bit gleich. Kein
  voller Lauf (kommt mit 34j-10).
