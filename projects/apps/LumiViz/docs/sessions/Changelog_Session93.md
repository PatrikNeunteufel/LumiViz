# Changelog Session 93 (2026-10-03 / 04)

> **Schneller bauen per Rechtsklick, der Start lädt wieder das Richtige, und
> der Composer beginnt.** Der Kettenbaum fügt Milkdrop-Elemente, Presets und
> Module direkt über das Kontextmenü ein. Nach dem Start läuft wieder das
> zuletzt aktive Preset, und die Browser folgen dem Laufenden nur noch auf
> Knopfdruck. Zum Abhaken von Sichttests gibt es ein eigenes Werkzeug. Der
> **Composer** — Visuals über der Zeit anordnen und mischen — hat sein Konzept
> und seine ersten Stufen; zu sehen ist davon in der App noch nichts.

## Neu

- **Effect Chain — Rechtsklick auf einen Milkdrop-Node,** eine seiner
  Sektionen oder ein Element: **Add custom wave**, **Add custom shape**,
  **Add sprite…**. Das Neue ist danach markiert. **Add sprite…** fragt zuerst
  nach der Bilddatei.
- **Milkdrop-Sprite — Feld Image mit Knopf …:** wählt die Bilddatei und trägt
  ihren vollen Pfad ein. Volle Pfade gelten jetzt auch an einem von Hand
  angelegten Milkdrop-Node (ohne Preset-Ordner fand er vorher kein Bild).
- **Effect Chain — Add preset** im Kontextmenü: die **Favorites** und die
  zehn **Recent** der Preset-Zeile. Ein Klick fügt das Preset als
  Presetverweis **in** die Kette ein (die Preset-Zeile lädt es und ersetzt
  die Kette).
- **Effect Chain — Add module** im Kontextmenü: die **Favorites** und die
  **Recently used** der Modul-Palette, ohne das Palettenfenster zu öffnen;
  **All modules…** öffnet es.
- **Preset-Zeile der Effect Chain — + New empty effect chain:** steht immer
  direkt unter dem laufenden Preset, für ein neues Preset in einem Klick.
- **Effect Browser und Audio Browser — Knopf ◎:** springt in den Ordner des
  laufenden Presets bzw. Titels und markiert es. Ein Filter oder Suchtext,
  der es verdeckt, wird dabei gelöst.
- **Werkzeug Sichttest** (`projects/exec/Sichttest`): ein eigenes Fenster
  neben der App, das Punkt für Punkt durch eine Sichttest-Liste führt — Pass,
  Pass mit Befund, Fail, Überspringen, Bemerkung, Screenshots aus der
  Zwischenablage. Liest Markdown-Checklisten und `*.testprotokoll.json`,
  schreibt Testlog, Report und Bilder neben die Liste. Die Liste selbst wird
  nie verändert.

## Geändert

- **Der Start lädt wieder das zuletzt aktive Preset** — gleich, ob es über
  den Effect Browser, die Preset-Zeile, **File → Load** oder **Save As** kam.
  Vorher lud der Start nur, was zuletzt über den Effect Browser geöffnet
  worden war. Lief zuletzt eine neue Kette ohne Namen, ist es das davor
  benannte Preset.
- **Der Effect Browser bleibt in seinem Ordner** — auch über den Neustart,
  und auch wenn das laufende Preset von woanders kommt. Zeigt er dessen
  Ordner, ist es markiert. Früher sprang er beim Start in den Ordner des
  laufenden Presets.
- **Der Audio Browser markiert den laufenden Titel,** wenn er dessen Ordner
  zeigt.
- **Milkdrop-Shape — „Frame (per instance)":** das Frame-Feld sagt jetzt, was
  es schon immer tat: es läuft einmal je Instanz (`instance` zählt von 0 bis
  `num_inst` − 1).
- **Modul-Palette — Recently used** wird auch geführt, wenn das
  Palettenfenster nie offen war.

## Composer (in Arbeit — in der App noch nicht zu sehen)

Der **Composer** ordnet Visuals über der Zeit an, wie das Arrangement einer
DAW: Kanäle untereinander, darin Clips mit Start und Ende, Ein- und
Ausblendkurve, Überlagerung zwischen Kanälen, wählbare Mischart. Ein
Audiokanal kann Songs als Bezug aufnehmen.

- **Konzept:** [Composer_Konzept.md](../ui/Composer_Konzept.md) — alle
  Entscheide gefallen. Der Composer muss sich zurückbauen lassen; er entsteht
  in einem eigenen Zweig und berührt den Renderer nur an benannten
  Nahtstellen.
- **Stand:** Datenmodell, Datei `.lvcomp`, Uhr, Mischer (sieben Mischarten,
  jede mit Pegel) und Wiedergabe sind gebaut und getestet — im Zweig
  `composer`, ohne Panel und ohne Anschluss an die App.
- **Folge:** die Visual-Playlist (Konzept von S51) entfällt als eigenes
  Vorhaben.

## Doku

- Benutzerhandbuch 1.72.0 → **1.78.0** (§11 Milkdrop-Node und Kontextmenü,
  Preset-Zeile, Presets durchblättern, Audio Browser).
- Neu: `docs/ui/Composer_Konzept.md` (0.7.0).
- `Offene_Punkte.md` 1.86.0; `Visual_Playlist_Konzept.md` mit Vermerk.

## Tests

- 836 → **866 Cases** (im Zweig `composer`; auf `master` 851).
- Vergleichslauf nach den Renderer-Nähten des Composers: 90 von 90 Bildern
  bitgleich.

## Sichttest offen

Von dieser Sitzung ist im laufenden Programm noch nichts abgenommen. Am
wichtigsten: der Start mit dem zuletzt aktiven Preset.
