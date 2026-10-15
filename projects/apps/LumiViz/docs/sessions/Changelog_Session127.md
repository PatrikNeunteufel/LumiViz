# Changelog Session 127 (2026-10-15)

> **Lautstärke im Composer fertig, Loop läuft mit.** Ton-Clips lassen sich jetzt
> messen und gleich laut stellen, ein Limiter fängt Spitzen ab. Der Loop-Editor
> zeigt, wo der Composer gerade spielt; ein Loop im Panel ist der laufende Song;
> Kanäle und Spuren lassen sich ordnen. Konzepte: `docs/ui/Composer_Konzept.md`
> §9g, `docs/audio/Loop_Konzept.md`. **Geprüft mit Unit-Tests unter beiden
> Compilern.** Im Programm gesehen ist nichts.

## Neu

- **Limiter:** Haken in der Zeile von *Loop overlap* (Standard an). Er senkt
  Spitzen über 0 dB weich ab, statt den Ausgang hart abzuschneiden. Handbuch §15.
- **Match loudness:** ein Dialog, der jeden Ton-Clip nach EBU R128 misst und je
  Clip ein Gain vorschlägt — Ziel ein Clip oder ein fester Wert in LUFS. Man
  hört jede Änderung sofort, **Listen A/B** vergleicht mit vorher, **▶** springt
  in einen Clip; **OK** ist ein Undo-Schritt. Knopf unten rechts im Panel
  Composer, oder Rechtsklick auf einen Ton-Clip → *Match loudness to this
  clip...*. Gemessene Songs merkt LumiViz. Handbuch §15.
- **Die Marke läuft mit:** spielt der Composer den Loop, der im Panel Loop
  offen ist, laufen die Marke im Panel und in Drum- und Instrument-Editor mit;
  ebenso in den Editoren, wenn das Panel den ganzen Loop spielt. Haken **Scroll
  with playhead** im Panel Loop. Handbuch §16.
- **Ein Loop allein ist der laufende Song:** beim Play im Panel Loop zeigt das
  Panel Player „Loop - <Titel>“, Text und Karaoke lesen seine Tags. Handbuch §16.
- **Kanäle und Spuren ordnen:** im Panel Loop einen Kanal, im Drum-Editor eine
  Spur am Namen ziehen oder Rechtsklick *Move … up/down*. Handbuch §16.

## Intern

- Stückweises Dekodieren eines Ausschnitts (`decodeStereoChunks`), Messung je
  Clip, Cache für Songs; die Regel des Mitrollens als eine Funktion für alle
  rollenden Ansichten.
- Gemessen: 32 Loop-Clips mit Verzerrer, Echo und Hall brauchen gut 5 % eines
  Prozessorkerns; ein Song von vier Minuten ist in rund ¼ Sekunde gemessen.
- Voller Testlauf unter Release-clang grün; Debug-MSVC baut.
