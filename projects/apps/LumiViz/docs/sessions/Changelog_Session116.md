# Changelog Session 116 (2026-10-12)

> **Drums aus Instrumenten, und der Creator bekommt die Bausteine dazu.** Eine
> Drum-Spur spielt jetzt, was man will: einen Klang des Kits, ein eigenes
> Sample, eine Tondatei oder einen Ton eines Instruments — und **Edit
> sound...** macht aus jedem Klang ein Instrument. Der Instrument-Creator hat
> Rauschen, einen fallenden oder steigenden Ton, Hochpass, Bandpass und
> Resonanz; seine Gruppen lassen sich einklappen, und ein Ton kann beim
> Einstellen immer wieder spielen. Dazu eine Liste von Suchordnern für die
> Bilder und Texturen importierter Presets.
> **Im Programm gesehen hat Patrik davon das Einklappen** (und dabei einen
> Fehler gefunden, der behoben ist). Alles andere ist mit Unit-Tests und
> Bildern aus den Tests geprüft, nicht gesehen und nicht gehört.

## Neu

- **Der Klang einer Drum-Spur:** die Auswahl **Sound** im Drum-Editor nennt
  unter **Kit** die Klänge des Kits, unter **My samples** die eigenen Samples,
  und darunter **Instrument...** (das Fenster Instruments), **Sample file...**
  (fragt wie der Creator, ob die Datei kopiert wird) und **Record sample...**.
- **Der Knopf „…"** an einer Spur, die ein Instrument spielt: **Note**,
  **Length**, **Play**, **Browse...**, **Edit instrument...**.
- **Edit sound...** (Rechtsklick auf eine Spur): aus einem Klang des Kits,
  einem Sample oder einer Tondatei wird ein Instrument der Art Drums mit einer
  Sample-Schicht. Der Loop ändert sich erst, wenn der Creator speichert.
- **Waveform: Noise** im Creator — Rauschen, geformt mit Hüllkurve, Filter und
  Drive.
- **Pitch sweep** und **Sweep time:** jede Note beginnt neben ihrer Tonhöhe
  und kommt auf sie — darüber fällt sie („↓ 22 st", der Bauch einer Kick),
  darunter steigt sie („↑ 12 st"). Auch für eine Sample-Schicht.
- **Der Filter mit Type und Resonance:** Low-pass, High-pass, Band-pass;
  Resonance betont die Grenze und stellt beim Band-pass die Breite.
- **Die Gruppen des Creators lassen sich ein- und ausklappen** (Oscillator,
  Envelope, Filter, Vibrato); das Programm merkt die Stellung.
- **Repeat note** mit **every … beats:** der angeklickte Ton spielt immer
  wieder und darf bis zum nächsten klingen; die Taste ist dabei hervorgehoben.
- **Image and Texture Search Folders** (Settings → Panels): eine Liste von
  Ordnern, in denen die Bilder von AVS-Presets (Picture, Texer, Texer II, AVI)
  und die Texturen und Sprites von MilkDrop-Presets gesucht werden, wenn sie
  nicht beim Preset liegen. Für MilkDrop gab es das bisher nicht.
- **Benutzerhandbuch:** §8 (die Suchordner), §16 Loop (der Klang einer Spur,
  Edit sound..., Noise, Pitch sweep, der Filter, die Überschriften, Repeat
  note).

## Geändert

- **„Favorites" überall:** im Fenster Instruments heißt der Knopf „★" und die
  Überschrift „★ Favorites", wie in der Effect Palette; dort und in der
  Preset-Zeile steht „Favorites" statt „Favourites".
- **Das Format `.lvinst` trägt Fassung 6** (Fassung 5: Noise und Pitch sweep;
  Fassung 6: Art und Resonanz des Filters). Fassung 1 bis 5 liest LumiViz
  weiter; ein älteres LumiViz liest ein neu gespeichertes Instrument nicht
  mehr.
- **Der Creator behält seine Größe.** Die Regler der Schicht stehen in einer
  Liste mit Rollbalken (Befund aus dem Programm: mit jeder aufgeklappten
  Gruppe wuchs das Fenster, bis über den Bildschirm). Das Mausrad über einem
  Regler dort rollt die Liste und verstellt ihn nicht mehr.
- **Wer in Sound einen Klang wählt, nimmt der Spur ihr Instrument.**
- **Statt eines „Image Search Folder" eine Liste;** ein bisher gesetzter
  Ordner steht als erster Eintrag darin.
- **Save im Creator** schlägt bei einem neuen Instrument seinen Titel ohne
  Zahl dahinter vor.

## Für Entwickler

- [Loop_Konzept.md](../audio/Loop_Konzept.md) 0.55.0 — §4h: 11c Schritt 2 und
  3, 11d, 11e, die Gruppen, der behobene Befund, Repeat note; die gebilligten
  Skizzen zu 11d und 11e als Beschreibung; Entscheide **N1** (beide
  Richtungen), **N2 a**, **H1 b**.
- `loop/DrumEditor`: `setzeSpurInstrument`, `setzeNote`, `setzeDauer`,
  `nimmSampleDatei`, `spieleSpur`, `oeffnePalette`, `oeffneCreator`,
  `oeffneAufnahme`, `oeffneDetail`, `bearbeiteKlang`, `instrumentAusKlang`,
  `klangZeilen`; eigene Fenster Instruments, Creator und Record sample.
- `loop/SampleFrage.hpp` (neu): die Rückfrage aus S2 a, einmal für Creator
  und Drum-Editor.
- `loop/LoopPanel`: `spieleSpur`, `spieleAllein`, `spieleInstrumentTon` mit
  `klingtMs`.
- `loop/Instrument`: `Schwingung::Rauschen`, `tonhoeheStartHt`,
  `tonhoeheZeitMs`, `FilterArt`, `filterResonanz`; im Rechenkern `Tonweg`,
  `Filter` (ein Tiefpass ohne Resonanz bleibt der alte, Wert für Wert),
  `rechneRauschen`.
- `loop/InstrumentCreator`: `setzeGruppeZu`, `gruppeZu`,
  `setzeGruppenMerken`; `setzeWiederholen`, `setzeWiederholSchlaege`,
  `setzeSchlagMs`, `tonLangVerlangt`; das Feld `creatorLayerScroll`.
- `loop/InstrumentPalette`: `setzeZiel` mit dem Namen der Spur, `zeigeArt`;
  `liesNeu` liest Favorites und Recently used neu.
- `core/Suchordner.hpp` (neu, header-only): `liste`, `setzeListe`,
  `findeDatei`; `MilkdropTextureResolve.hpp` und der AVS-Import benutzen es.
- **In der Test-Exe ist das Merken der Gruppen abgeschaltet** (eine Zeile in
  `test_LoopInstrumentEditor.cpp`, sie gilt für alle Dateien).
- Bilder und Hörproben aus den Tests: `LUMIVIZ_DRUM_KLANG_BILD`,
  `LUMIVIZ_INSTRUMENT_NOISE_BILD` (je `=<datei.png>`), `LUMIVIZ_11D_WAV`,
  `LUMIVIZ_11E_WAV` (je `=<Ordner>`).
- Wer die Ausgabe eines Testlaufs mit `Select-Object -First` kürzt, bricht den
  Lauf ab (Rückgabe −1) — `-Last` nehmen.

## Offen

- Der Blick ins Programm und das Ohr: alles außer dem Einklappen. Die
  Hörproben zu 11d und 11e sind gerechnet, nicht gehört.
- **Untersucht, nicht als Fehler im Code bestätigt:** AVS sahen zum Teil
  anders aus, MilkDrop zeigte zeitweise nichts (später wieder gut). Kein
  Render-Code ist seit S106 geändert; den AVS-Presets unter `asset\avs` fehlen
  Texer-Bilder, die nur in der Sammlung an anderem Ort liegen (dafür die
  Suchordner); „AVS Import Render Scale" steht auf window / 4. Was bei
  MilkDrop nichts zeigte, ist nicht nachgestellt.
- Zwischen Resonance 0 und 1 % wechselt der Low-pass die Rechnung (ein kleiner
  Sprung in der Helligkeit); mit viel Resonance wird der Ton leiser.
- Im Creator heißen zwei Felder „Type".
- Unter Drums steht im Fenster Instruments noch nichts: mitgelieferte
  Drum-Instrumente bringt Stufe 11f, eigene stehen nur unter My files.
- Beim Anspielen aus dem Creator sind die Effekte am Clip nicht zu hören.
- Decay steht in Schritten von 10 ms — ein sehr kurzer Klick lässt sich am
  Regler nicht genau stellen.
- Zwei Filter hintereinander (der Clap des Kits) gehören zur vorgemerkten
  Kette für Filter und Effekte.
