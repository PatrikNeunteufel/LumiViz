# Changelog Session 118 (2026-10-13)

> **Filter und Effekte sind jetzt eine frei geordnete Reihe — der Signal
> path.** Im Instrument-Creator zeigt jede Schicht ihren Weg als Zeile von
> Kästen: Source, dann Filter, Drive und Envelope in beliebiger Folge, zuletzt
> Level. Im Fenster Clip effects steht dieselbe Zeile für den Clip, mit
> Filter, Distortion, Echo und Reverb. Bausteine lassen sich hinzufügen,
> entfernen und umordnen, höchstens acht je Reihe. Alte Instrumente und Loops
> klingen wie bisher.
> **Im Programm gesehen oder gehört hat davon niemand etwas.** Alles ist mit
> Unit-Tests, Messungen und Bildern aus den Tests geprüft.

## Neu

- **Signal path im Instrument-Creator:** über den Reglern einer Schicht steht
  die Zeile **Source → Bausteine → + Add → Level**. Ein Klick auf einen
  Kasten zeigt darunter seine Regler — immer nur die des gewählten. **+ Add**
  holt einen weiteren **Filter** oder **Drive**; **← Move**, **Move →** oder
  Ziehen mit der Maus ordnen um; **Remove** entfernt. Die **Envelope** gibt
  es genau einmal, sie lässt sich aber verschieben.
- **Die Reihenfolge ist zu hören:** ein Drive hinter der Envelope lässt mit
  dem Ton nach (so klingt eine Kick), zwei Filter hintereinander lassen nur
  ein Band durch (ein Clap).
- **Signal path im Fenster Clip effects:** die Zeile **Clip → Bausteine →
  + Add → Out**. Jeder Effekt geht beliebig oft und in freier Folge. Der
  gewählte Baustein zeigt seinen Haken (aus: er bleibt stehen und wird
  übersprungen) und seine Regler.
- **Filter am Clip:** Low-pass, High-pass oder Band-pass mit **Cutoff in
  Hertz** (40 Hz bis 18 kHz) und Resonance — für den ganzen Clip, gleich
  welche Noten er spielt.
- **Mehrere Clips mit verschiedenen Reihen:** die Zeile zeigt dann einen
  Clip, wählbar unter **Showing**. Gestellt werden die markierten Clips mit
  derselben Reihe; ein Hinweis nennt, welche unberührt bleiben. **Use this
  path for all** gleicht alle an.
- **Benutzerhandbuch:** §16 Loop (Instrument-Creator, Effekte am Clip, die
  gerechneten Drums).

## Geändert

- **Im Creator gibt es die Gruppen Envelope und Filter nicht mehr.** Ihre
  Regler stehen unter ihrem Kasten in der Zeile. **Level**, **Transpose** und
  **Start offset** stehen unter den Kästen Level und Source statt fest oben.
  Der Regler „Envelope" des Filters heißt **Filter envelope**.
- **Im Fenster Clip effects** stehen statt dreier fester Gruppen die Zeile
  und der gewählte Baustein. **Reset** entfernt die Effekte, statt sie
  auszuschalten.
- **Die gerechneten Drums Kick und Clap** sind neu gebaut und liegen näher am
  Kit: der Kick mit Drive hinter der Envelope (er klingt länger nach), der
  Clap mit High-pass und Band-pass hintereinander (weniger Tiefen). Gemessen,
  nicht gehört. Wer sie schon in einem Loop benutzt, hört den Unterschied.
- **Dateiformate:** ein Instrument (`.lvinst`), das dieses LumiViz speichert,
  liest ein älteres nicht mehr. Einen Loop (`.lvloop`) liest es, spielt seine
  Clips aber ohne Effekte. Ältere Dateien liest dieses LumiViz wie bisher.
- **Zwei alte Clips mit verschiedenen Effekten** (einer nur mit Echo, einer
  nur mit Reverb) gelten als verschiedene Reihen und lassen sich nicht mehr
  in einem Zug zusammen stellen.

## Für Entwickler

- Modell: `Schicht::reihe` (`SchichtFilter`, `SchichtDrive`, `SchichtHuelle`)
  und `ClipEffekte::reihe` (`ClipFilter`, `ClipVerzerrung`, `ClipEcho`,
  `ClipHall`); `kBausteineMax` in `loop/Bausteine.hpp`. `.lvinst` Fassung 7
  und `.lvloop` Fassung 5 tragen die Liste `path`.
- Klang: `Instrument.cpp` rechnet die Reihe einer Schicht (`Reihe`),
  `EffektKette` die eines Clips; neu `FilterEffekt`.
- Oberfläche: neu `loop/SignalWeg` (die Zeile, von Creator und Clip effects
  benutzt); `InstrumentCreator` und `ClipEffektFenster` umgebaut.
- **Beleg für alte Dateien:** neu `test_LoopWerte` — je mitgeliefertem
  Instrument drei Töne und je Loop ein Durchlauf gegen
  `tests/unit/UnitTests/werte/loop_werte.txt`. Vor dem Umbau am Klangerzeuger
  festgehalten, danach 199 von 199 Bit für Bit gleich unter beiden
  Compilern. Nach einer gewollten Klangänderung mit
  `LUMIVIZ_LOOP_WERTE_SCHREIBEN=1` neu schreiben.
- Tests: 1152 Cases (10 neu), voller Lauf 1152 / 1152, 0 Skips unter
  Testing-MSVC; unter Release-clang 164 Fälle zu Loop, Instrumenten, Drum,
  Sichttest-Aktionen und Mitgeliefertem. Neu `test_LoopBausteine` (acht
  Fälle), `LoopOhneEngine.hpp`; Bilder mit `LUMIVIZ_CREATOR_PATH_BILD` und
  `LUMIVIZ_CLIP_EFFEKTE_BILD`; `LUMIVIZ_DRUM_KANDIDATEN=<Ordner>` rechnet
  Varianten eines Drum-Instruments.
- Last, Release-clang: eine Schicht mit acht Bausteinen kostet rund das
  Doppelte der alten Reihe; acht Reverb an einem Clip 0,75 % eines Kerns.

## Notiert, nicht gebaut

- **Wunsch Patrik:** im Creator ein Knopf, um ein bestehendes Sample zu
  bearbeiten wie nach einer Aufnahme; mitgelieferte Samples schreibgeschützt
  (`Offene_Punkte.md` §7).

## Offen

- Im Programm ungesehen und ungehört: alles aus dieser Sitzung.
- Patriks Ohr an den Hörproben zu Kick und Clap (alt gegen neu) und zu den
  übrigen Drums — daran hängt die Hi-Hat-Schwingung und ob die gerechneten so
  laut sein sollen wie das Kit.
- Ob Clips mit verschiedenen Effekten doch zusammen stellbar sein sollen.
