# Changelog Session 88 (2026-10-02)

> **Songtext über jedem Preset.** Die neue Karaoke-Ebene legt den Text über
> alles, was läuft — auch über importierte AVS- und MilkDrop-Presets —, und
> mischt ihn auf Wunsch mit dem Bild dahinter. Dazu die letzten Lyrics-Reste,
> ein eigenes Icon für Preset-Stapel und „Resume playback".

## Neu

- **Karaoke-Ebene** (Settings → Audio → *Karaoke layer*): der Songtext des
  laufenden Titels über jedem Preset, ohne Lyrics-Knoten in der Kette. Die
  Ebene liegt über dem fertigen Bild — die Effekte des Presets ziehen sie nicht
  mit. Hat das Preset selbst einen eingeschalteten Lyrics-Knoten, gewinnt das
  Preset, und der Text steht nie doppelt da. Schrift und Lage sind vorerst
  fest; ein Karaoke-Panel folgt.
- **Mischarten** für die Karaoke-Ebene, getrennt für Zeile und Trail, je mit
  Stärke: Normal, Additive, 50/50, Subtract, Invert (weißer Text kehrt die
  Farbe darunter um), XOR, Multiply, Maximum, Minimum. Die Kontur bleibt
  normal, damit der Text lesbar bleibt.
- **Dieselben Mischarten am Lyrics-Knoten und am Lyrics Trail** (Feld
  *Blend*). Replace, Additive und 50/50 rechnen wie bisher.
- **Resume playback** (Taste im Player-Panel, Settings → Audio): wird LumiViz
  beendet, während ein Titel läuft, spielt der nächste Start ihn an derselben
  Stelle weiter.
- **Versatz je Titel:** *Offset for this track* im Lyrics-Editor und *Track
  offset* im Panel Lyrics Sync — je Titel gemerkt, ohne die Songtext-Datei
  anzufassen; gilt auch für eingebetteten Text.
- **Lyrics file…:** eine vorhandene `.lrc`/`.txt` dem laufenden Titel
  zuweisen. Sie wird in den Benutzerordner kopiert; gewinnt danach eine Quelle
  mit Vorrang (etwa eingebetteter Text), sagt LumiViz das.
- **Lyrics Sync: Undo / Redo** (Ctrl+Z / Ctrl+Y in der Liste) für jeden
  Schritt an den Zeiten.
- **Tag View** zeigt den Songtext des laufenden Titels (Quelle, synchron oder
  nicht, Zeilenzahl, Text).
- **Eigenes Icon für `.lvfx2`** (Preset-Stapel) im Effect Browser.

## Geändert

- **Songtexte aus dem Netz:** hat der Dienst zu einem Titel nichts, merkt sich
  LumiViz das und fragt 7 Tage nicht wieder (bei als instrumental
  eingetragenen Titeln 30 Tage). Zurücksetzen: Settings → Audio → *Lyrics not
  found* → *Ask again*; eine Freigabe in der Sperrliste setzt ebenfalls zurück.
- **Lyrics Trail** folgt den skriptgesteuerten Werten des Lyrics-Knotens
  (Karaoke Bounce, Flip) und arbeitet jetzt auch mit unsynchronem Text (Wechsel
  alle N Beats).
- **Lyrics-Editor:** die Knöpfe stehen in zwei Reihen (Alternatives… / Wrong
  lyrics, darunter Lyrics file… / Open folder), der Versatz je Titel in einer
  eigenen Zeile.

## Behoben

- Ein im Panel gelöschter Knoten ließ seinen Zustand bis zum nächsten Laden
  liegen — ein gelöschter Video-/Kamera-Knoten hielt seine Quelle offen. Jetzt
  wird nach jedem Edit aufgeräumt, die Kamera geht aus.

## Bekannt

- Die Karaoke-Ebene erreicht alles, was als Kette läuft, nicht die
  eigenständigen Visualizer (Equalizer, Oscilloscope, Waveform, Pulsing).
- Nach einem Preset-Wechsel beginnt der Trail der Karaoke-Ebene neu.
- XOR ist auf dem Fenster der App noch nicht geprüft.
- Bei Presets mit Render Scale ist der Text der Ebene so grob wie das Preset
  (Text in Fensterauflösung ist geplant).
- Multiply und Minimum lassen bei weißem Text nur die Kontur übrig.
- Am Lyrics-Knoten gibt es keinen Stärke-Regler für die Mischart.
- Wort-Zeiten des Dienstes werden nicht gelesen (anderes Format, zurückgestellt).
- Resume playback hält den Stand nur beim geordneten Beenden fest, nicht bei
  einem Absturz.
