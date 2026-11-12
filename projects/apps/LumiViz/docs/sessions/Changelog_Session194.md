# Changelog Session 194 (2026-11-12)

> Die gezupften Saiten aus InstrumentsCalculation stehen jetzt als Kern `pluckedString` in der Bibliothek — Bit für Bit
> wie die Vorlage, noch ohne Datei und ohne Instrument im Programm. Die Stufe 34k ist damit bei 34k-3 von elf.

## Neu

- **Kern `pluckedString`** (Klasse 2) unter `projects/libs/InstrumentModels/zupf/`: Cembalo, Harfe, Gitarre, Laute und
  Oud als Vorlagen (`harpsichord`, `concert_harp`, `classical_guitar`, `lute`, `oud`), mit Tafel „Plucked string“ (Saite,
  Zupf, Stegmoden als Liste, Chöre, mitschwingende Saiten, gestimmter Faktor auf die Laufzeiten, Bezug). Die Stärke folgt
  der Anschlagstufe in dB wie bei Hammer, Stab und Trommel.
- **Beleg aus der Vorlage:** `docs/modellierung/anbindung-ic/abdruck_zupf.cpp` mit Baubefehl, `werte/zupf_ic.txt`
  (65 Abdrücke, Bezüge, gestimmte Faktoren). Alle 65 Abdrücke rechnet der neue Kern gleich, unter beiden Compilern.
- **Stimmer** in `InstrumentMeasure/ZupfStimmen`: stimmt alle fünf in zusammen einer Sekunde, gleich der Vorlage.
- **Zweite Tonhöhenmessung** `InstrumentMeasure/Grundton`: der Grundton aus den Teiltönen — auch wenn er im Ton fehlt
  oder schwach ist, an gerechneten Tönen auf weniger als ein Tausendstel Cent.

## Gemessen

- An den nach der Vorlage gestimmten Tönen liegt die zweite Messung bis 5,7 Cent neben der Note (Gitarre), keine Taste
  über 10 Cent — ein Befund für die Herleitung 34k-6.

## Entschieden

- PD1 a: die Pedale der Harfe kommen mit 34k-7 in die Tafel, wenn sie über das Mitschwingen je Note wirken.
