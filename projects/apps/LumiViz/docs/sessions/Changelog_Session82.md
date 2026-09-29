# Changelog Session 82 (2026-09-29)

> **Ganze Presets werden zu Bausteinen, und LumiViz wechselt selbst zwischen
> ihnen.** Ein fertiges `.avs`, `.milk` oder `.lvfx` lässt sich als Gruppe
> einsetzen — als Kopie oder als Verweis auf die Datei. Der neue Host
> Switcher schaltet zwischen den Gruppen nach Zeit, beim Songwechsel oder per
> Skript, und jedes Skript-Modul kann jetzt auf einen neuen Titel reagieren.

## Neu

- **„Preset as Group…"** in der Effekt-Palette: ein Preset als Host-Gruppe
  einsetzen (Kopie, frei bearbeitbar); „Aus Quelle neu laden" liest die Datei
  erneut. Gruppen laden jetzt alle drei Formate.
- **„Preset Reference…"**: dasselbe als **Verweis** — die Kette speichert nur
  den Pfad, das Preset wird bei jedem Laden frisch gelesen. Pfad eintippen
  oder öffnen; ein Schalter wandelt Kopie und Verweis ineinander um.
- **Host Switcher**: Liste von Gruppen (umsortierbar, Index für Skripte),
  Startgruppe, Wechsel nach Sekunden, beim Songwechsel, in Listenreihenfolge
  oder zufällig, und per Skript (`switch = n`, `nextgroup = 1`). Er schaltet
  nur zur Laufzeit — die gespeicherte Kette bleibt, wie sie eingestellt ist.
- **Song-Feld** in jedem Knoten mit Frame-Feld: läuft einmal bei jedem
  Songwechsel, vor dem Frame-Feld (z. B. neue Farbe würfeln, Zähler auf 0).
- **Eindeutige Gruppennamen**: eine zweite „Host Group" heißt automatisch
  „Host Group 2"; Umbenennen zieht alle Switcher-Listen mit.
- **Messwerkzeug:** `--kerne ccd2` legt Referenz und Prüfling auf den kühleren
  Prozessor-Teil (nur Kalibrierung).

## Geändert

- „Import Browser" heißt jetzt **Effect Browser** — er lädt auch die eigenen
  `.lvfx`.
- Beim Laden eines Presets in eine Gruppe bleiben die Einstellungen seiner
  Wurzel erhalten (etwa AVS „clear every frame").
- Kalibrier-Tabelle der mitgelieferten MilkDrop-Presets neu auf der RTX 4090
  in fünf Formaten; *The Beauty and the Math* stimmt in allen überein.

## Behoben

- Eingeklappte Gruppen klappten beim Wechsel zu einer anderen Gruppe (und bei
  jeder anderen Strukturänderung) wieder auf.
