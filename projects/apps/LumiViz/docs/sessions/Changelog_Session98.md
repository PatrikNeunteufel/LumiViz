# Changelog Session 98 (2026-10-06)

> **Im Composer überlagern sich die Songs.** Alle eingeschalteten Audio-Kanäle
> spielen zugleich, jeder Song mit seiner Pegelkurve — das Überblenden von
> einem Song in den nächsten ist im Programm gehört. Das Bild reagiert auf die
> Summe, Songtext und Tags folgen dem lauteren Song, das Panel Player zeigt
> ihn als „Composer - <Titel>". Dazu Kopieren und Einfügen von Clips mit einer
> Einfügemarke und ein Schatten beim Hineinziehen. **Im Programm bestätigt**
> ist das Überlagern samt Bildreaktion und Transport; alles Übrige ist gebaut
> und per Test belegt, aber noch nicht gesehen.

## Neu

- **Audio-Kanäle überlagern sich** (Composer, Stufe C5b): Songs in
  verschiedenen Audio-Kanälen spielen zugleich; blendet einer aus, während
  der andere einblendet, ist das eine Überblendung.
- **Das Bild reagiert auf die Summe** der gerade spielenden Songs, jeder mit
  seinem Pegel.
- **Der laufende Song:** Songtext (Karaoke-Ebene, Lyrics), Tags in
  Text-Vorlagen, Tag View, Audio Browser und „bei Songwechsel" im Host
  Switcher folgen dem Song, dessen Pegelkurve an der Stelle am höchsten steht
  — in einer Überblendung wechselt das am Kreuzungspunkt.
- **Panel Player bei laufender Komposition:** Titel „Composer - <Song>",
  Balken und Zeit des laufenden Songs, ein Griff in den Balken springt in der
  Komposition mit; der Play-Knopf zeigt, ob die Komposition läuft; der
  Loop-Knopf ist grau.
- **Clips kopieren, ausschneiden, einfügen** (Ctrl+C / Ctrl+X / Ctrl+V und
  Rechtsklick), Bild und Ton, mit Pegelkurve, Mischart und Einsatz.
- **Einfügemarke:** ein Klick auf eine freie Stelle einer Kanalzeile setzt
  sie; dorthin legt Einfügen den Anfang des Clips. Sie ist nicht die
  Abspielmarke und steht still, wenn die Komposition läuft.
- **Schatten beim Hineinziehen:** ein Preset oder Song erscheint beim Ziehen
  blass dort, wo er landen wird; über einer Zeile der anderen Art erscheint
  nichts.
- **Zeit unter dem Zeiger** im Lineal des Composers.
- **Settings → Panels**, drei neue Einstellungen: *Composer off: Player
  resumes…*, *Composer on: Composition continues…* (beide Vorgabe an) und
  *New clip length* (Vorgabe 20 s).

## Geändert

- **Die Uhr führt die Komposition**, nicht mehr der Ton: die Songs werden an
  der Uhr gehalten (vorher richtete sich die Uhr nach dem einen spielenden
  Song).
- **Nach „Composer off"** spielt der Player von selbst weiter, wenn er vor dem
  Einschalten spielte; **nach „Composer on"** läuft die Komposition weiter,
  wenn sie vor dem Ausschalten lief. Beides lässt sich abschalten. (Vorher:
  der Song des Composers spielte im Player weiter; Composer on wartete immer
  auf Play.)
- **Tag View** zeigt beim Einblenden den laufenden Song der Komposition
  (Befund im Hörtest: es zeigte „no track loaded").

## Entschieden

- **Das Panel Player zeigt den laufenden Song als „Composer - <Song>"** —
  der Entscheid aus Session 97 („zeigt die Komposition") ist zurückgenommen;
  gebaut war davon nichts.
- **Der Play-Knopf folgt der Komposition**, auch in einer Lücke ohne Song.
- **Eine eigene Einfügemarke** statt der Abspielmarke als Ziel des Einfügens.

## Technik

- **`Tonfuehrung` neu** gegen `IAudioEngine` und `IAudioPlayer`: ein Mixer,
  je Ton-Clip eine Quelle (zwei Sekunden vor ihrem Clip geöffnet), gesetzt
  bei Einstieg, Sprung der Uhr und Abweichung; Pegel je Takt; Lautstärke und
  Mute des Players am Mixer. `Komposition::toeneBei` ersetzt `tonBei`.
- **Nachsetzen, gemessen und behoben:** an der echten `BassEngine` liegt die
  gemeldete Stelle stetig 40 bis 50 ms hinter der Uhr (Latenz der Ausgabe),
  das Öffnen einer Quelle dauert 3 bis 4 ms. Die Ausreißer im Hörtest kamen
  von einer veralteten Stelle bei langen Bildern des Render-Threads
  (`Bildquelle::standStelleJetztMs` rechnet jetzt hoch) und davon, dass die
  Latenz als Abweichung zählte (jetzt: Grundabstand je Quelle).
- **Nähte** (Konzept §12.2, Rückbau-Liste): N14 `leseTrackInfo`
  (`6f87331`), N13 `onAudioUpdate` liest den Mixer (`3b8161c`), N5 Anzeige —
  vier Stellen in `PlayerPanel.cpp` (`a22e797`), N15 Tag View und N9 mit zwei
  Haken (`4b3e652`), N9 mit *New clip length* (`45ead4e`). **Naht N7a
  (`setPegel` am Player) ist zurückgenommen** (`4b3e652`).
- Die Sitzung meldet den laufenden Song über den Bus (`TrackChangedEvent`,
  `PlaybackPositionEvent`, `PlaybackStateEvent`); „läuft" am Bus heißt: die
  Komposition läuft.
- `Bearbeiten::fuegeKopieEin`; `Zeitachse::fallSchatten` rechnet den Schatten
  mit demselben Code wie das Fallenlassen.
- **934 Cases** im Zweig `composer` (Start der Session: 930).
- Doku: `Composer_Konzept.md` 0.28.1 → 0.34.0, `Benutzerhandbuch.md` 1.91.1 →
  1.94.0, `Offene_Punkte.md` 1.91.0.

## Offen

- **Im Programm noch nicht gesehen:** das Panel Player bei laufender
  Komposition, die Korrekturen nach dem Hörtest, das Weiterspielen,
  Einfügemarke, Kopieren / Einfügen, Schatten, Tooltip, Vorgabelänge.
- **Wellenform beim Kürzen eines Songs:** beobachtet wurde, dass sie
  zusammengeschoben statt abgeschnitten aussieht — nicht nachgestellt.
- Nach „Composer off" ohne geladenen Titel im Player bleibt im Panel Player
  der letzte „Composer - <Song>" stehen.
- In der Schleife sollen sich Ende und Anfang der Komposition überlagern
  (vorgemerkt seit Session 97).
- Das Panel Lyrics Sync springt und bremst direkt am Player — bei führendem
  Composer wirkt das nicht.
- Weiteres in `Offene_Punkte.md` §7.
