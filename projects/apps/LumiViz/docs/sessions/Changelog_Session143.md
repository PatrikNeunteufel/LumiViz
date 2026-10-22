# Changelog Session 143 (2026-10-22)

> **Loops und Clips als WAV-Datei, und die Grundlage für Freeze.** Im Loop-Panel lässt sich der ganze
> Loop oder ein einzelner Clip als WAV exportieren, im Composer ein Loop-Clip. Die Lautstärke des
> Loops wirkt jetzt hinter den Effekten. Für das Einfrieren rechenintensiver Clips stehen Fingerabdruck,
> Klangfassung und der Ordner mit seiner Grenze.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.4 (WX2 a bis WX9 a, HB1 a, FZ1 a bis FZ3 a).

## Neu

- **Export audio...:** im Loop-Panel ein Knopf in der Datei-Zeile (der ganze Loop, wie das Panel ihn
  spielt — mit Fadern, Lautstärke und Limiter; *Seamless loop* oder *With tail*, *Passes* 1–64) und ein
  Eintrag am Clip (der Clip mit Effekten und Nachklang, ohne Fader und Lautstärke des Loops); im
  Composer am Loop-Clip (über Versatz und Länge, ohne Gain, Blenden und Pegelkurve). Ein Fenster mit
  Format (32-bit float, 24-bit, 16-bit mit Dither), Fortschritt, Abbruch, Ergebnis und *Show in folder*.
- **Settings → Panels:** *Loop Sound Memory* (Grenze für gerechnete Töne, Vorgabe 1 GB, Stand, *Clear*)
  und *Frozen Clips* (Grenze für eingefrorene Clips auf der Platte, Vorgabe 2 GB, Stand, *Clear frozen
  clips*).
- **Handbuch:** „Die Töne werden vorbereitet“, „Die Lautstärke des Loops“, „Als WAV exportieren“.

## Geändert

- **Die Lautstärke des Loops wirkt nach den Effekten der Clips**, wie ein Summenregler — sie verstellt
  die Zerrung eines Clips nicht mehr. Hörbar nur in `effects_demo` (*Acid* zerrt stärker).
- Aufnahmen (*Record sample*) schreiben über denselben WAV-Schreiber wie der Export, Byte für Byte wie
  bisher.

## Intern

- `loop/Wellenform` (Clip, ganzer Loop, Composer-Ausschnitt — über den `Klangerzeuger`),
  `audio/WavDatei`, `loop/ExportAuftrag`, `loop/ExportFenster`, `loop/Freeze` (Abdruck eines Clips über
  Dateiinhalte, `kKlangfassung` mit Wächter an `loop_werte.txt`, Ordner `frozen`).
- Neue Tests `test_LoopWellenform`, `test_WavDatei`, `test_LoopExport`, `test_LoopFreeze`, Fälle in
  `test_LoopTonspeicher`; 1485 Fälle. Je Teilschritt unter beiden Compilern grün; **kein voller Lauf**.
- Sichttest-Listen `Sichttest_ToeneVorab.md` und `Sichttest_Export.md` (lokal), Vorlage
  `composer_M_toene.lvcomp`.
