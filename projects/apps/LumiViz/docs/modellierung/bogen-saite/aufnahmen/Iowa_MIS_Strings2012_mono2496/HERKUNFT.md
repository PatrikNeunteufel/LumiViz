# Herkunft — Iowa MIS 2012, Streicher (Violine, Viola, Cello, Kontrabass), mono „2496“

> **Unter `samples` gelöscht am 2026-10-28** (F-L5-1 a, IC Session 3). Diese Fassung ist an allen 264 Dateien **das
> Mittel (L+R)/2** der Stereofassung [../Iowa_MIS_Strings2012_stereo2496/](../Iowa_MIS_Strings2012_stereo2496/HERKUNFT.md)
> — Viola, Cello, Kontrabass in 24 bit mit Rest −98 bis −115 dB, Violine auf 16 bit abgeschnitten (höchstens 1 LSB).
> Wer mono braucht, bildet das Mittel; wer genau diese Dateien braucht, lädt sie über `LADELISTE.tsv` und prüft sie
> gegen `SHA256SUMS`. Die Angaben unten beschreiben den Stand vor dem Löschen.

- **Quelle:** University of Iowa Electronic Music Studios, Musical Instrument Samples (MIS), Seiten
  `theremin.music.uiowa.edu/MISviolin2012.html`, `MISviola2012.html`, `MIScello2012.html`, `MISdoublebass2012.html`;
  je Instrument die Zips `<Instrument>.arco.mono.2496.zip` und `<Instrument>.pizz.mono.2496.zip` (acht Zips, zusammen
  2 470 126 112 B), geladen am **2026-10-27** (IC Session 3, Klasse 3 F-9 a), außerhalb ausgepackt (ohne `__MACOSX/`),
  Archive danach gelöscht; Größe je Datei gegen das Zip-Verzeichnis geprüft. Die Zips tragen keine Prüfsumme der
  Quelle; ihre SHA-256 stehen in `LADELISTE.tsv`.
- **Warum Zips statt Einzeldateien:** Die Einzeldateien auf den Seiten sind 16 bit/44,1 kHz (an
  `Violin.arco.pp.sulA.C7.mono.aif` nachgelesen); „24-bit, 96 kHz“ bieten die Seiten nur „in zip format“ an.
- **Lizenz:** keine Standardlizenz. Wortlaut auf `MIS.html` (nachgelesen 2026-10-27): „may be downloaded and used for
  any projects, without restrictions“. Vor einer Auslieferung in LumiViz schriftlich bestätigen lassen (wie Klasse 2
  F-10 a).
- **Instrument und Aufnahme** (Kopf der Seiten, nachgelesen 2026-10-27): Violine „Nicolai Tambovsky (1998)“, Spieler
  „Leonid Iogansen“, „12-11-2011“; Viola „Anton Krutz (2001)“, „Manuel Tabora“, „May 11, 2012“; Cello „Charles Quenoil
  (1923)“, „Yoo-Jung Chang“, „May 11, 2012“; Kontrabass „Anton Krutz (2004)“, „Mike White“, „May 11, 2012“; alle
  „Anechoic Chamber“, „5 feet“, „Earthworks QTC40“, „Metric Halo“.
- **Inhalt:** 264 AIFF in vier Ordnern `Violin/` 72, `Viola/` 66, `Cello/` 48, `Bass/` 78; je Datei ein chromatischer
  Lauf auf einer Saite in einer Stärke (pp, mf, ff), arco und pizz (`<Instrument>.<arco|pizz>.<Stärke>.sul<Saite>.<von><bis>.mono.aif`;
  Reihenfolge von Saite und Stärke je Instrument verschieden); zusammen 3 764 056 512 B.
- **Format geprüft — nicht überall wie der Name sagt:** Viola, Cello, Kontrabass alle mono, **24 bit, 96 kHz**. Die
  **Violine** ist trotz „2496“ **16 bit**: 61 Dateien 96 kHz, 11 Dateien **48 kHz** (`Violin.arco.<ff|mf>.sulA.*` — sechs
  Dateien — und `Violin.pizz.ff.sulG.G3B3`, `Violin.pizz.pp.sul<A|D|E>.*` — fünf). Beim Messen die Abtastrate je Datei
  lesen.
- **Prüfsummen:** `SHA256SUMS` (je ausgepackter Datei, Pfad relativ zum Datensatzordner).
- **Zweck:** Bezug Einzeltöne der Streicher ([aufnahmen.md](../../aufnahmen.md), [messplan.md](../../messplan.md)).
- **Übernahme nach LumiViz:** siehe [UEBERNAHME_LUMIVIZ.md](../../../../UEBERNAHME_LUMIVIZ.md).

## Ablage (A-1 a, Patrik 2026-10-26)

Die Dateien liegen **nicht im Repo**, sondern lokal unter `…\cmake\samples\Klasse 3 - Bogen-Saite\Iowa_MIS_Strings2012_mono2496\` (auch für LumiViz erreichbar). Im Repo bleiben diese Datei, `SHA256SUMS` und `LADELISTE.tsv` (Prüfsumme, Pfad, Quelle je Datei) — damit lässt sich jede Datei wiederbeschaffen und prüfen, auch in einer Cloud-Sitzung (Domain der Quelle in der Umgebung freischalten).
