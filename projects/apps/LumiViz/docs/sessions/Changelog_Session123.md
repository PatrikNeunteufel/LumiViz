# Changelog Session 123 (2026-10-14)

> **Vorlagen.** Zwanzig mitgelieferte Formen statt sechs, achtunddreißig
> mitgelieferte Samples aus einer freien Sammlung (Stimmen, Beatbox,
> Drum-Maschinen, Schlagwerk, Instrumente mit Ton), neunzig Instrumente statt
> fünfundfünfzig und neun neue Loops, die das alles spielen. Dazu ein Konzept
> für VST3. **Geprüft mit Unit-Tests unter beiden Compilern und mit
> Messungen.** Gehört oder im Programm gesehen ist nichts; Hörproben liegen
> bereit.

## Neu

- **Mitgelieferte Samples** — im Drum-Editor und im Menü **File...** einer
  Sample-Schicht die neue Gruppe **Samples** zwischen Kit und My samples. Sie
  heißen im Loop `lib:<name>`, sind schreibgeschützt wie das Kit und stammen
  aus der OLPC-Sammlung auf archive.org (CC BY 3.0); die Nennung der Urheber
  steht in `asset\loop\samples\CREDITS.md` neben der Exe.
- **Vierzehn Formen** mehr: Moon, Drop, Bolt, Note, Flower, Gear, Cross,
  House, Infinity, Cat; mit mehreren Konturen Smiley, Yin and yang, Rings,
  Atom.
- **Elf Instrumente aus Formen**: Heart Pad, Rings Choir (Stereo XY),
  Infinity Drone, Gear Bass, Moon Bass, Star Lead, Bolt Pluck, Flower Flute,
  Smiley Keys und Atom Bell (je Kontur eine Schicht), Cat (ein Miau).
- **Dreizehn Instrumente aus Samples**: Female Choir, Hum Pad, Vocal Lead,
  Vocal Chop, Harp, Koto, Dulcimer, Thumb Piano, Shakuhachi, Alto Sax, Alto
  Flute; aus zwei Aufnahmen Angel Harp (Harfe und Chor) und Wind Voices
  (Flöte und Summen).
- **Elf Drums**: gerechnet 808 Kick, Low Tom, High Tom, Shaker, Ride, Crash;
  aus Samples geschichtet Punch Kick, Fat Snare, Clap Stack, Hat Shake,
  Beatbox Kick.
- **Neun Loops** unter `genres`: Vocal House, Angel Steps, Beatbox Groove,
  Desert Caravan, Zen Garden, Flute Tropics, Sax After Dark, Shape Synthwave,
  Geometry.

- **Kits (Stufe 21a, nur das Modell — im Programm noch nicht zu sehen):** die
  Datei `.lvkit`, Rollen je Drum-Spur (kick, snare, clap, rim, hat, open hat,
  ride, crash, tom, perc, fx, nummeriert), der Kitwechsel, der die Schritte
  behält, und zehn mitgelieferte Kits mit angeglichener Lautstärke: Classic,
  808, 909 House, Electro, Hand drums, Beatbox, Lo-fi, Reggae, Reggaeton,
  Strange. Eigene Kits gehen mit Export und Import mit (Bereich **Kits**).

## Geändert

- `asset/Asset.cmake`: `loop/samples/*.wav` und `CREDITS.md` gehen mit neben
  die Exe — die eine Ausnahme von „nur eigene Werke"; `THIRD_PARTY_NOTICES.md`
  führt die Samples.
- Die Unit-Tests rechnen Sample-Schichten ohne Engine aus der WAV-Datei
  (`LoopOhneEngine.hpp`); ein Loop lässt sich als Hörprobe ablegen
  (`LUMIVIZ_LOOP_WAV`, `LUMIVIZ_LOOP_WAV_ZIEL`).

## Konzept

- `docs/audio/VST3_Konzept.md` 0.1.0 — fremde Instrumente und Effekte im Loop:
  wo sie andocken (Effekt am Clip; Instrument vorab je Note oder live),
  Laden, Fenster, Zustand in den Dateien, Absturzschutz, Bauen; das VST3 SDK
  steht seit VST 3.8 unter MIT (Pressemitteilung Steinberg vom
  29. Oktober 2025). Nichts entschieden.
- `docs/audio/Loop_Konzept.md` 0.71.0 §4l — die Entscheide zu Samples und
  Kits und die Skizze der Kits als Beschreibung.

## Offen

- Die Kits im Programm (Stufen 21b bis 21e): der Knopf Kit im Drum-Editor, das
  Fenster Kits, die akustischen Kits, das Icon.
- Hören: `.claude\handover\hoerproben_s123` (lokal).
- Kein Männerchor — die Männerstimmen der Sammlung halten keinen Ton.
