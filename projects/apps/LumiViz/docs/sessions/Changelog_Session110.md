# Changelog Session 110 (2026-10-11)

> **Der Loop spielt Instrumente.** Neben Drumkit-Clips gibt es Instrument-Clips
> mit Noten über einer Klaviatur, vierzehn gerechnete Instrumente — sieben
> davon für House — und einen Instrument-Creator, mit dem eigene Instrumente
> entstehen. Den Loop aus Session 109 hat Patrik im Programm gesehen: „sieht
> gut aus". **Was in dieser Session dazukam, hat im Programm noch niemand
> gesehen oder gehört** — geprüft ist es mit Unit-Tests, Bildern aus dem Test
> und gerechneten Hörproben.

## Neu

- **Instrument-Clips:** im Panel Loop legt der Rechtsklick neben *New drumkit
  clip* auch **New instrument clip** an. Ein Muster spielt ein Instrument und
  beliebig viele Töne zugleich.
- **Der Instrument-Editor** (Doppelklick auf einen Instrument-Clip oder *Edit
  base*): links eine Klaviatur über sechs Oktaven (C1 bis C7), rechts das
  Raster. Ein Klick setzt eine Note, weitergezogen wird sie länger; die rechte
  Kante ändert die Länge, Ziehen verschiebt in Zeit und Tonhöhe, Rechtsklick
  oder Entf löscht. Unten die Lautstärke je Note. **Instrument**, **Volume**,
  **Pan**, **M**, **Bars**, **Play pattern**, **Peak** wie im Drum-Editor. Ein
  Klick auf eine Taste spielt den Ton an.
- **Vierzehn Instrumente,** gerechnet, nicht aufgenommen: Piano, Strings,
  Violin, Guitar, Bass, Organ, Lead — und für House: House Piano, House Organ,
  Chord Stab, Warm Pad, Sub Bass, Acid Bass, Pluck.
- **Instrumente sind Dateien** (`.lvinst`), wie die Klänge des Kits: die
  mitgelieferten unter `asset\loop\instruments`, eigene an beliebigem Ort.
- **Der Instrument-Creator** (*Edit instrument...* im Instrument-Editor):
  Regler für Waveform, Voices, Detune, Obertöne, Damping, Hüllkurve, Filter,
  Vibrato und Level; zwei Bilder (eine Schwingung, die Hüllkurve einer Note);
  eine Klaviatur zum Anspielen — auch ungespeichert. **Save** und **Save
  as...** schreiben die Datei, das Muster spielt sie danach. **Load
  instrument...** nimmt eine vorhandene Datei.
- **Zwei Beispiele:** `asset\loop\instrument_demo.lvloop` und
  `asset\loop\house_demo.lvloop`.
- **Sichttest-Steuerung:** drei Aktionen für das Panel Loop — `loop_laden`,
  `loop_wiedergabe`, `loop_editor`.
- **Benutzerhandbuch §16:** Instrument-Clips, der Instrument-Editor, eigene
  Instrumente.

## Geändert

- **Das Panel Loop fragt beim Beenden von LumiViz nach Ungespeichertem.** Die
  Rückfrage bietet jetzt überall **Save / Discard / Cancel** wie im Composer —
  auch vor New und Load, wo es bisher nur Discard und Cancel gab.
- *Edit base* öffnet je nach Clip den Drum-Editor oder den Instrument-Editor;
  beide Fenster können zugleich offen sein.

## Behoben

- **Der Haken Karaoke im Panel Composer** war nach dem Start leer, obwohl die
  Karaoke-Ebene an war — bis man sie schaltete. Er stimmt jetzt von Anfang an.

## Für Entwickler

- [Loop_Konzept.md](../audio/Loop_Konzept.md) 0.19.0 — neu §4f
  (Instrument-Clip, Instrument-Editor, das Format `.lvinst`,
  Instrument-Creator), Entscheide I1 c, I2 b, I3 a; Stufen 9a, 9b, 10a, 10b.
- [Composer_Konzept.md](../ui/Composer_Konzept.md) 0.82.0 — neu §9f: die
  gemeinsame Basis `lumi::ui::Achse` und der Loop als Clip im Audio-Kanal.
- [Sichttest_Steuerung.md](../Sichttest_Steuerung.md) 1.1.1 — die drei
  Aktionen des Loops.
- Neu: `loop/Instrument`, `loop/InstrumentEditor`, `loop/InstrumentCreator`,
  `loop/SichttestAktionen`. Eine Note wird vorab in einen Klang gerechnet und
  vom vorhandenen Rechenkern gespielt wie ein Schlag.
- Hörproben und Bilder aus den Tests: `LUMIVIZ_INSTRUMENT_WAV=<Ordner>`,
  `LUMIVIZ_INSTRUMENT_EDITOR_BILD=<datei.png>`,
  `LUMIVIZ_INSTRUMENT_CREATOR_BILD=<datei.png>`.

## Offen

- Der Blick ins Programm auf alles aus dieser Session; die Instrumente sind
  nach Rechnung eingestellt, nicht nach Gehör.
- Ein Instrument aus mehreren überlagerten Schichten (Wunsch Patrik) — die
  Skizze folgt in der nächsten Session.
- Der Acid Bass hat keine Resonanz: der Filter kennt keine.
- Ein laufendes Muster klingt erst nach Save mit den neuen Einstellungen.
- `asset\loop` (Kit und Instrumente) liegt im Release-Paket nicht neben der
  Exe — dort schwiegen alle Loops.
- Instrumente aus Samples; ein eigenes Icon für `.lvloop`.
- Debug-MSVC nicht gebaut; der volle Testlauf lief nicht unter clang.

## Tests

- 1040 Cases (15 neu), voller Lauf 1040 / 1040, 0 Skips (Testing-MSVC); die
  Fälle zu Loop, Editor und Creator auch unter Release-clang grün. Ein neuer
  Fall wartet rund eine Sekunde auf das Ende eines angespielten Tons.
