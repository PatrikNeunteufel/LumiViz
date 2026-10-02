# Changelog Session 87 (2026-10-01)

> **Songtexte, die ankommen — und sich richten lassen.** Der Online-Abruf
> gibt nicht mehr beim ersten „beschäftigt" auf, ein falscher Text lässt sich
> gegen eine andere Fassung tauschen, ein unsynchroner von Hand synchron
> klopfen. Dazu ein neuer Knoten, der die alte Zeile verwehen lässt.

## Neu

- **Lyrics-Angaben als Tabelle** im Editor: Status, Quelle, synchron oder
  nicht, Zeilenzahl, Herkunft, Datei. Solange ein Abruf läuft, steht dort
  *…downloading…*.
- **Alternatives…:** andere Fassungen eines Songtexts bei lrclib.net suchen —
  mit eigenen Suchbegriffen. Die Liste zeigt Album, Dauer, Zeilenzahl; bis zu
  drei Fassungen stehen nebeneinander, mit Zeitmarken, und ein Balken zeigt,
  wo der Song gerade ist. Oben stehen Titel, Länge und Spielzeit; wechselt
  der Song, bleibt das Fenster beim bisherigen Titel, nennt den neuen und
  bietet **Switch to the playing track** an.
- **Wrong lyrics:** verwirft den gezeigten Text und sperrt den Eintrag. Die
  Sperrliste steht unter Settings → Audio → *Blocked lyrics*.
- **Panel „Lyrics Sync"** (View → Panels): Zeiten je Zeile mitklopfen (Tap,
  Enter oder T), um ±100 ms nachstellen, per Doppelklick vorhören, als `.lrc`
  speichern. Jede Änderung ist sofort im Bild zu sehen. Oben laufen die
  Spielzeit und eine Wellenform um die Abspielposition mit, in der die Zeiten
  der Zeilen als Striche stehen; **▶** an jeder Zeile springt genau dorthin.
- **Knoten „Lyrics Trail":** zeichnet die eben beendete Zeile noch einmal,
  damit die Effekte danach sie verblassen oder wegziehen. Ein Beispiel steht
  im Mix `20261001.mix.lvfx2`.
- **Voreinstellungen:** sechs für den **Host Switcher** (Random Runtime,
  Random Runtime per Group, Random Seconds, Every 16 Beats, Bass Drop, One per
  Song) und zehn neue für **Lyrics**, darunter bewegte Karaoke-Varianten
  (Beat Colours, Song Colours, Bounce, Flip).
- **Playlist:** die Dauer je Titel steht rechts in der Zeile.

## Geändert

- **Songtexte aus dem Netz:** antwortet der Dienst „beschäftigt", fragt
  LumiViz für den laufenden Titel von selbst wieder (nach 2, 5, 10, 20 und
  40 Sekunden). Kennt der Dienst den vollen Titel nicht, folgt eine Suche
  ohne Zusätze wie „(Video Version)" oder „feat. …"; übernommen wird daraus
  nur ein Text mit passender Dauer.
- **Zwei Fassungen desselben Songs** teilen sich im Benutzerordner keine
  Datei mehr: geholte Texte tragen die Dauer ihres Titels.
- **Wiederholte Zeilen** blenden kurz vor jeder Wiederholung zur
  Vorschau-Farbe — man sieht, wann die Zeile neu einsetzt.
- **Playlist:** die Zeile folgt der Breite der Liste, der Name wird gekürzt;
  kein waagrechtes Rollen mehr. **Shuffle, Loop und Repeat** bleiben über den
  Neustart erhalten.
- **Lyrics-Voreinstellungen** setzen jetzt auch die Skriptfelder (auch leer) —
  ein Wechsel lässt kein Skript der vorigen stehen.

## Behoben

- Ein online geholter Songtext erschien erst beim nächsten Abspielen
  (Ursache: der Dienst war beschäftigt, und es wurde nicht wieder gefragt).
- Ein Presetverweis, der an die Stelle eines entfernten gezogen wurde, zeigte
  weiter das alte Preset — bis zum Neuladen.
- Eine Knoten-Voreinstellung mit Song-Skript brachte dieses Skript nicht mit.
- Der Suchfilter der Playlist war nach Aus- und Einblenden weg, der Begriff
  stand aber noch im Feld.

## Bekannt

- Lyrics Trail arbeitet nur mit synchronem Text und folgt nicht den
  skriptgesteuerten Werten des Lyrics-Knotens (Karaoke Bounce, Flip).
- Lyrics Sync kennt kein Rückgängig und keine Wort-Zeiten.
- Titel ohne Text und unbekannte Titel werden bei jedem Abspielen neu
  angefragt.
- Die Dauer in der Playlist ist bei MP3 mit variabler Bitrate ohne
  Längenangabe eine Schätzung.
- Zu „Gettin' Over" (David Guetta) führt der Dienst offenbar keinen wirklich
  passenden Text.
