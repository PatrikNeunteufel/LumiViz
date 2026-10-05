# Changelog Session 97 (2026-10-05)

> **Der Weg zu überlagernden Songs ist entschieden, und die Engine kann
> mischen.** Mehrere Songs laufen künftig als Quellen in einem Mixer; die
> Visuals sehen die Summe mit den Pegeln, Karaoke und Tags folgen dem lauteren
> Song, das Panel Player zeigt die Komposition. Gebaut ist der erste Schritt,
> der Mixer in der Audio-Engine. **Im Programm ändert sich noch nichts:** der
> Composer benutzt den Mixer noch nicht, es spielt weiter nur ein Song.

## Neu

- Für den Benutzer nichts Sichtbares. Die Audio-Engine kann mehrere Dateien in
  einem Stream mischen (BASSmix) — die Grundlage für das Überblenden von
  Songs im Composer.

## Geändert

- **BASSmix ist Pflichtpaket** neben BASS und BASSFLAC
  (`externals/bass/SETUP.md`). Wer LumiViz selbst baut, braucht den Ordner
  `externals/bass/bassmix24/`.
- **Build-System CMakeCraft v0.9.2** (vorher v0.9.0): `external_options` eines
  App-Tests wirken jetzt am Test-Target; `CMakeCraftBootstrap.cmake` ist auf
  dem Stand der Vorlage (erkennt Reste eines abgebrochenen Klons).

## Entschieden (Composer, Stufe C5b — noch nicht gebaut)

- **Ein Mixer in der Engine;** die Uhr der Komposition führt, der Ton wird an
  sie gehalten.
- **Die Visuals reagieren auf die Summe mit den Pegeln** — auf das, was zu
  hören ist.
- **Der laufende Song** ist der mit dem höchsten Wert der Pegelkurve an der
  Stelle der Uhr; ihm folgen Karaoke-Ebene, Tags, Songwechsel-Zähler, Tag View
  und Audio Browser.
- **Das Panel Player zeigt die Komposition** (Dateiname, Zeitachse), solange
  der Composer Ton führt; sein Loop-Knopf schaltet dann die Schleife der
  Komposition.
- **Vorgemerkt:** in der Schleife sollen sich Ende und Anfang der Komposition
  überlagern, Bild und Ton gleichermaßen.

## Technik

- **Belege vor dem Entscheid** (`test_ComposerBelegeTon.cpp`): zwei Streams
  der echten `BassEngine` spielen zugleich; Spektrum und Pegel eines Streams
  kennen weder den anderen Stream noch die eigene Lautstärke; der Mixer
  liefert die Summe mit den Pegeln (abgezählt), Hüllkurve und Stelle je
  Quelle; ein Pegelwechsel kommt nach etwa 460 ms an (Puffer 0,5 s), 90 ms
  (0,1 s) oder 30 ms (ohne Puffer).
- **Naht N12** (Konzept §12.2, in der Rückbau-Liste, Commit `e7fe2d5`):
  `IAudioEngine` hat sieben Mixer-Aufrufe mit der Vorgabe „kann es nicht"
  (`createMixer`, `mixerAddSource`, `mixerRemoveSource`,
  `getSourcePositionMs`, `setSourcePositionMs`, `setSourcePaused`,
  `isSourcePlaying`); `BassEngine` setzt sie mit BASSmix um und übergeht
  `bassmix.dll` beim Laden der Decoder-Plugins. Belegt in
  `test_ComposerNahtMixer.cpp` an der echten Engine.
- `Solution.json`: `BASS_MIX` an App-Kern, Runner und Test-Target.
- **930 Cases** im Zweig `composer` (Start der Session: 924).
- Doku: `Composer_Konzept.md` 0.26.2 → 0.28.1, `Offene_Punkte.md` 1.90.0.

## Offen

- **Audio-Kanäle überlagern sich im Programm weiter nicht.** Nächster Schritt:
  der Composer hält den Mixer (je Ton-Clip eine Quelle, Pegel je Takt), dann
  das Spektrum vom Mixer und die Anzeige im Panel Player.
- Nicht gemessen: ob der Mixer ohne Puffer unter Last aussetzt; Songs mit
  verschiedenen Abtastraten im Mixer.
- Das Panel Lyrics Sync springt und bremst direkt am Player — bei führendem
  Composer wirkt das nicht.
- Sichttests: `Sichttests_S96.md` und der Rest von `Sichttests_S95c.md` —
  unverändert offen.
- Weiteres in `Offene_Punkte.md` §7.
