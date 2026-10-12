# Changelog Session 115 (2026-10-12)

> **Samples im Loop.** Ein Instrument kann jetzt aus einer Aufnahme bestehen:
> eine Schicht spielt eine Tondatei je Note höher oder tiefer. Das Programm
> misst ihren Ton, und ein eigenes Fenster nimmt Samples auf, lässt sie
> zuschneiden und in Dauerschleife anhören. Eigene Samples haben einen Ort und
> gehen mit dem Export mit. Dazu ein behobener Befund: Ctrl+Z im Fenster
> Instruments.
> **Im Programm geprüft ist davon nur der behobene Befund** — alles andere
> ist mit Unit-Tests und Bildern aus den Tests geprüft, nicht gesehen und nicht
> gehört. Aufgenommen wurde im Programm noch nie.

## Neu

- **Waveform: Sample** im Instrument-Creator: eine Schicht aus einer Aufnahme.
  **File...** wählt eines der eigenen Samples oder eine Tondatei (wav, flac,
  mp3, ogg, aiff); **Root note** und **Fine tune** sagen, welcher Ton die
  Aufnahme ist; **Sample start** und der **Sustain loop** (Loop from, Loop to)
  bestimmen, was davon spielt. Das obere Bild zeigt die Aufnahme, die Marken
  darin lassen sich ziehen. Eine Sample-Schicht mischt sich mit gerechneten
  Schichten; Envelope, Filter und Drive gelten weiter.
- **Detect** misst den Ton einer Aufnahme und schlägt Root note und Fine tune
  vor („Detected: A3 +12 ct"); **Apply** übernimmt. Ohne klaren Ton steht dort
  „No clear pitch found".
- **Reference tone:** die Klaviatur spielt die Sample-Schicht allein und dazu
  einen reinen Ton derselben Note — zum Stimmen nach Gehör.
- **Record sample** (im Creator **Record...** neben File...): ein Fenster, das
  aufnimmt — vom Mikrofon, einem anderen Eingang oder als Mitschnitt dessen,
  was der Rechner spielt. Die Aufnahme lässt sich mit zwei Griffen
  zuschneiden und mit **Play** und **Repeat** in Dauerschleife anhören,
  während man zieht. **Trim silence at both ends** stellt die Griffe an den
  Ton, **Normalize level** hebt die Lautstärke. **Save and use** legt das
  Sample ab und setzt es in die Schicht.
- **Eigene Samples haben einen Ort:** der Ordner `samples` im Benutzerordner.
  Eine über File... gewählte Datei wird auf Wunsch dorthin kopiert („Copy the
  file to My samples") oder bleibt, wo sie ist. Die Wahl lässt sich merken und
  unter Settings → Panels, **Chosen Sample Files**, ändern.
- **Export und Import** der eigenen Daten kennen den Bereich **Samples**.
- **Die Art Drums** im Fenster Instruments und in Type des Creators.
- **Benutzerhandbuch:** §16 Loop (die Schicht aus einer Aufnahme, Detect,
  Reference tone, Record sample), Settings (Chosen Sample Files, Samples im
  Export).

## Geändert

- **Im Fenster Instruments gelten Ctrl+Z und Ctrl+Y** (Befund aus dem
  Programm: nach Use nahm Ctrl+Z nichts zurück, erst nach einem Klick ins
  Panel Loop). Steht die Schreibmarke in Search, gehören die Tasten dem Text.
- **Das Format `.lvinst` trägt Fassung 4** (die Schwingung Sample). Fassung 1
  bis 3 liest LumiViz weiter; gespeichert wird Fassung 4 — ein älteres LumiViz
  liest ein neu gespeichertes Instrument nicht mehr.
- **Das Format `.lvloop` trägt Fassung 4:** eine Drum-Spur kann ein eigenes
  Sample oder ein Instrument mit Note und Länge nennen. Fassung 2 und 3 liest
  LumiViz weiter. **Im Programm lässt sich das noch nicht einstellen** — der
  Drum-Editor folgt.
- **Im Creator stehen File... und Record...** in einer eigenen Zeile unter dem
  Namen der Datei.
- **Die Überschrift Vibrato** verschwindet im Creator, wenn kein Regler
  darunter steht (bei Plucked string und Sample).

## Für Entwickler

- [Loop_Konzept.md](../audio/Loop_Konzept.md) 0.50.0 — §4h: zwei gebilligte
  Skizzen als Beschreibung (Creator mit Sample-Schicht, Record sample), die
  gebilligte Skizze der Klangliste im Drum-Editor; Entscheide **Z1 a**,
  **S1 a**, **S2 a**, **K1 a**, **K2 a**, **K3 a**; Stufe 11a und 11b gebaut,
  11c Schritt 1; der Beleg zur Aufnahme mit BASS; der Rechenversuch, ob das
  Kit im Creator baubar ist; vorgemerkt 11d bis 11f, drei weitere Bausteine
  und Filter und Effekte als Kette.
- `loop/Sample.hpp` (neu): der Ordner `samples`, `user:<name>`,
  `nimmZuEigenen`, die einmal dekodierten Daten (`sampleDaten`,
  `merkeSampleDaten`), `SampleKopie`. `loop/Grundton.hpp` (neu):
  `messeGrundton`. `loop/Aufnahme.hpp` (neu): `ohneStille`, `schneideAus`,
  `schreibeWav`, `speichereAufnahme`. `loop/AufnahmeFenster.hpp` (neu): das
  Fenster und `AusschnittQuelle`.
- `loop/Instrument.hpp`: `Schwingung::Sample` und die Felder der Schicht
  (`sample`, `grundton`, `feinCent`, `sampleStartMs`, `schleife`, …);
  `zaehleInstrumentFassung`; die neunte Art. **Eine Sample-Schicht spielt die
  Aufnahme in ihrer Lautstärke**, ohne die Halbierung gerechneter Schichten.
- `loop/Loop.hpp`: die `Spur` trägt `instrument`, `hoehe`, `dauerMs` und
  `spielt()`; `istEigenerKlang`.
- **`IAudioEngine` kann aufnehmen:** `getRecordDevices`, `recordStart`,
  `isRecording`, `recordedFrames`, `recordLevel`, `recordStop`; die
  `BassEngine` setzt es um, neben der laufenden Ausgabe.
- `KurvenBild` (im Creator): Griffe zum Ziehen (`setGriffe`, `griffGezogen`),
  die Aufnahme als Fläche (`setGespiegelt`), der Zuschnitt (`setAussenGrau`),
  eine Linie ohne Griff (`setZeiger`).
- **Tests, die aufnehmen, laufen nur mit `LUMIVIZ_AUFNAHME_BELEG=1`**
  (`test_LoopBelegeAufnahme`, der letzte Fall in `test_LoopAufnahme`); ohne
  die Variable melden sie das und prüfen nichts. Die Engine aus Pappe, die
  „aufnimmt", steht in `PappAufnahme.hpp`.
- Bilder aus den Tests: `LUMIVIZ_INSTRUMENT_SAMPLE_BILD`,
  `LUMIVIZ_RECORD_SAMPLE_BILD` (je `=<datei.png>`).
- Gemessen an Patriks Rechner: BASS nimmt in 48000 Hz Stereo float auf, die
  Blöcke kommen alle 20 ms; eine geschriebene WAV liest die BassEngine auf
  16 Bit genau wieder.

## Offen

- Der Blick ins Programm und das Ohr: alles außer Ctrl+Z im Fenster
  Instruments. Aufgenommen, angehört und gespeichert ist bisher nur gegen eine
  Engine aus Pappe.
- Der Drum-Editor kennt die neuen Klänge noch nicht (Stufe 11c, Schritt 2
  und 3).
- Ob das mitgelieferte Kit sich im Creator nachbauen lässt, ist gerechnet,
  nicht gehört: Tom und Snare ja; für Kick, Clap und Hi-Hats fehlt je ein
  Baustein.
- Die Wahl aus „Chosen Sample Files" geht nicht mit Settings in einen Export.
- Während einer Aufnahme zeigt das Bild noch nichts; Player und Loop werden
  beim Aufnehmen nicht angehalten.
