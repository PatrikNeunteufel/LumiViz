# Changelog Session 89 (2026-10-02)

> **Karaoke bekommt sein Panel — und Presets werden sicher.** Die
> Karaoke-Ebene lässt sich jetzt vollständig einstellen, speichern und per
> Menü oder Taste schalten. Effektketten kennen *Save* und *Save As…*, und
> LumiViz fragt nach, bevor ungespeicherte Arbeit verloren geht. Beide Panels
> tragen eine Preset-Zeile mit Favoriten und den zuletzt benutzten Presets.

## Neu

- **Panel Karaoke** (View → Panels → Karaoke): Schalter *Karaoke on*, Reiter
  *Lyrics* mit allen Feldern der Zeile (Schrift, Farben, Lage, Kontur,
  Mischart, Skripte, Voreinstellungen, Songtext-Tabelle) und Reiter *Lyrics
  Trail* mit *Trail on* und der Liste der Trail-Module. Die Palette des Trails
  zeigt nur Module, die ein Bild umformen oder färben; der Lyrics Trail am
  Ende der Liste ist fest. Jede Änderung gilt sofort und bleibt über den
  Neustart.
- **Karaoke-Presets** (`.lvkar`): der Stand der ganzen Ebene als Datei, an
  einem Ort deiner Wahl. Im Effect Browser mit eigenem Icon (Mikrofon im
  Kreis); ein Doppelklick lädt in die Ebene, die Effektkette bleibt.
- **View → Karaoke** und **Ctrl+K** (*Karaoke On / Off*, umbelegbar) schalten
  die Ebene; Menü, Taste und Panel zeigen denselben Zustand. Das Panel zeigt
  *Preset shows its own lyrics*, wenn das laufende Preset selbst Lyrics zeigt.
- **Save (Ctrl+S) und Save As… (Ctrl+Shift+S)** für die Effektkette. Save
  schreibt ohne Dialog in die eigene `.lvfx`/`.lvfx2`; bei neuer Kette,
  importiertem `.avs`/`.milk`, mitgeliefertem Preset oder wechselnder Endung
  fragt es wie Save As.
- **Rückfrage bei ungespeicherten Änderungen** (Save / Discard / Cancel) vor
  jedem Ersetzen der Kette — New, Import, Load, Doppelklick im Effect Browser,
  Blättern mit Bild ab/Bild auf, Wechsel des Visualizers — und beim Beenden.
  Im Vollbild verlässt LumiViz erst das Vollbild und fragt dann. Als Änderung
  zählt nur, was auch gespeichert würde.
- **Preset-Zeile** oben in den Panels *Effect Chain* und *Karaoke*: das
  laufende Preset (mit `*` bei Änderungen), **Favoriten** (★) und die **zehn
  zuletzt benutzten**; eine Auswahl lädt sofort; dazu Load…, Save, Save As….
- **Host Switcher: Keep group state.** Die Gruppen der Liste merken sich ihren
  Stand, solange sie ruhen, und laufen beim nächsten Betreten dort weiter.
  **Reset buffers** und die Skriptvariable `clearbuffers` lassen sie ganz
  frisch beginnen.
- **Skriptfeld Line** am Lyrics-Knoten und in der Karaoke-Ebene: läuft einmal
  je Zeilenwechsel, liest `line` und `linecount`; Lage, Größe und Farben
  folgen dem Skript von dort.
- **Blend strength (%)** an Lyrics und Lyrics Trail: nimmt die Füllung der
  Buchstaben zurück, die Kontur bleibt.

## Geändert

- **Karaoke-Ebene bei Render Scale:** die Zeile wird in der Auflösung des
  Fensters gezeichnet und bleibt scharf; der Trail folgt der Auflösung des
  Presets.
- **Schalter und Mischarten der Karaoke-Ebene** stehen im Karaoke-Panel statt
  unter Settings → Audio. Dort eingestellte Mischarten werden beim ersten
  Start übernommen.
- **New Effect Chain** fragt nur noch, wenn es ungespeicherte Änderungen gibt.
- **Nach dem Speichern** gilt die gespeicherte Datei als laufendes Preset
  (bisher blieb es die zuletzt geladene).
- **File → Exit** schließt das Fenster wie das X — mit der Rückfrage.
- **Lyrics Trail:** stellt das Feld *Line* jede Zeile anders, weht die alte
  Zeile von dort weg, wo sie stand.

## Entfernt

- Settings → Audio: *Karaoke layer*, *Karaoke line blend*, *Karaoke trail
  blend* (jetzt im Karaoke-Panel).

## Noch nicht im laufenden Programm gesehen

Alles oben ist per Test und an gerenderten Bildern geprüft, aber nicht im
laufenden Programm — besonders die Dialoge (Rückfrage, Dateiauswahl), Menü
und Tasten, das Verlassen des Vollbilds und der Host-Switcher-Puffer mit
echten Presets.
