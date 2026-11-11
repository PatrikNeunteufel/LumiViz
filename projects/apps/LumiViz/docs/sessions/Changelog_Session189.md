# Changelog Session 189 (2026-11-11)

> **Akkordeon, Harmonium, Melodica und Mundharmonika rechnen jetzt in LumiViz — noch ohne Instrument-Datei.** Der Ton
> ist genau der aus InstrumentsCalculation; im Programm erscheinen sie erst mit eigenen Vorgaben (34j-8).

## Neu

- Der Rechenkern `freeReed` (Klasse 10, durchschlagende Zunge) mit seiner Parametertafel: Tasten mit Bass-Seite, Zunge,
  Kanzelle, Wind, Chöre und Bass-Chöre als Listen, Gehäuse, die gestimmten Werte und der Bezug.
- Der Stimmer der Zunge: Frequenz der Zunge und Druckfenster je Taste aus dem Modell.
- Der Beleg aus InstrumentsCalculation (`werte/zunge_ic.txt`) mit dem Programm, das ihn schreibt.

## Bekannt

- Die Schwächen der Vorlage aus Session 188 bleiben bis zur Herleitung je Taste (34j-5) — der Kern rechnet sie
  absichtlich unverändert nach.
- Der Bezug nach LumiViz liegt am Akkordeon an einer tieferen Taste als in InstrumentsCalculation und fällt deshalb
  anders aus (0,36 statt 1,57).

## Geprüft

- Alle 49 Abdrücke und alle vier Stimmungen Bit für Bit gleich InstrumentsCalculation, unter clang und MSVC. Kein voller
  Lauf.
