# Herkunft — Iowa MIS 2012, Streicher (Violine, Viola, Cello, Kontrabass), stereo „2496“

- **Quelle:** University of Iowa Electronic Music Studios, Musical Instrument Samples (MIS), Seiten
  `theremin.music.uiowa.edu/MISviolin2012.html`, `MISviola2012.html`, `MIScello2012.html`, `MISdoublebass2012.html`;
  je Instrument die Zips `<Instrument>.arco.stereo.2496.zip` und `<Instrument>.pizz.stereo.2496.zip` (acht Zips,
  zusammen 5 474 224 821 B), geladen am **2026-10-28** (IC Session 3, F-L5-1 a), außerhalb ausgepackt (ohne
  `__MACOSX/`), Archive danach gelöscht; Größe je Datei gegen das Zip-Verzeichnis geprüft. Die Zips tragen keine
  Prüfsumme der Quelle; ihre SHA-256 stehen in `LADELISTE.tsv`.
- **Lizenz, Instrument und Aufnahme:** wie die mono-Fassung, siehe
  [../Iowa_MIS_Strings2012_mono2496/HERKUNFT.md](../Iowa_MIS_Strings2012_mono2496/HERKUNFT.md).
- **Inhalt:** 264 AIFF in vier Ordnern `Violin/` 72, `Viola/` 66, `Cello/` 48, `Bass/` 78 — dieselben Läufe wie mono,
  Datei für Datei; zusammen 8 086 826 202 B. Namen: Violine und Viola `….stereo.aif`, Cello und Kontrabass nur `….aif`.
- **Format geprüft:** alle 2 Kanäle, **24 bit**; 96 kHz, außer denselben **11 Violin-Dateien mit 48 kHz** wie in mono
  (`Violin.arco.<ff|mf>.sulA.*`, `Violin.pizz.ff.sulG.G3B3`, `Violin.pizz.pp.sul<A|D|E>.*`).
- **Zwei Kanäle, zwei Mikrofonorte** (F-L5, an vier Dateien gemessen): Terzbänder L−R im Median 0,9–1,9 dB, bis
  9,8 dB; Korrelation 0,41 (Viola) bis 0,99 (Kontrabass). **Ausnahme:** 17 Pizzicato-Dateien der Violine sind
  **doppelt mono** (L = R bitgleich, Inhalt nur 16 bit): `Violin.pizz.ff.sulD.{C5B5,C6D6,D4B4}`,
  `Violin.pizz.ff.sulE.{C6B6,C7E7,E5B5}`, `Violin.pizz.ff.sulG.C5F5`, `Violin.pizz.mf.sulA.{C5B5,C6F6}`,
  `Violin.pizz.mf.sulD.C6D6`, `Violin.pizz.mf.sulE.{C6B6,C7E7,E5B5}`, `Violin.pizz.pp.sulA.C6A6`,
  `Violin.pizz.pp.sulD.C6Eb6`, `Violin.pizz.pp.sulE.C7F7`, `Violin.pizz.pp.sulG.C5Gb5` — dort trägt die Fassung keine
  Kanalstreuung (L1).
- **Verhältnis zu mono** (2026-10-28, alle 264 Paare): mono ist **das Mittel (L+R)/2** dieser Dateien — Viola, Cello,
  Kontrabass in 24 bit mit Rest −98 bis −115 dB, ohne Versatz, Pegel 1,000; Violine dasselbe Mittel **auf 16 bit
  abgeschnitten** (Abweichung höchstens 1 LSB). Die mono-Dateien sind deshalb unter `samples` gelöscht (F-L5-1 a);
  ihre Begleitdateien bleiben als Verweis. Wer mono braucht, bildet das Mittel.
- **Prüfsummen:** `SHA256SUMS` (je ausgepackter Datei, Pfad relativ zum Datensatzordner).
- **Zweck:** Bezug Einzeltöne der Streicher mit zwei Kanälen als Streuung des Bezugs (L1; [aufnahmen.md](../../aufnahmen.md),
  [messplan.md](../../messplan.md)).
- **Übernahme nach LumiViz:** siehe [UEBERNAHME_LUMIVIZ.md](../../../../UEBERNAHME_LUMIVIZ.md).

## Ablage (A-1 a, Patrik 2026-10-26)

Die Dateien liegen **nicht im Repo**, sondern lokal unter `…\cmake\samples\Klasse 3 - Bogen-Saite\Iowa_MIS_Strings2012_stereo2496\` (auch für LumiViz erreichbar). Im Repo bleiben diese Datei, `SHA256SUMS` und `LADELISTE.tsv` (Prüfsumme, Pfad, Quelle je Datei) — damit lässt sich jede Datei wiederbeschaffen und prüfen, auch in einer Cloud-Sitzung (Domain der Quelle in der Umgebung freischalten).
