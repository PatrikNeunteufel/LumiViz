# Changelog Session 106 (2026-10-10)

> **MilkDrop-Presets mit vielen Shapes rechnen schneller, und lange Bilder
> lassen sich aufschlüsseln.** Anlass war der Befund „MilkDrop-Presets stocken
> manchmal": er ist als Drosselung des Rechners eingegrenzt, nicht als Fehler
> der App.

## Neu

- **Diagnose für lange Bilder im MilkDrop-Renderer:** dauert ein Bild länger
  als 50 ms, schreibt LumiViz eine Zeile `langesBild` in die Trace-Datei
  `<TEMP>/lumiviz_milkdrop_trace.log` — Dauer, `dt` und `fps` wie das Preset
  sie sieht, Bildgröße, die Anteile der dreizehn Stufen, für die Shapes Skript
  gegen Zeichnen samt Instanzen und bedeckter Fläche, für den Render-Thread
  die verbrauchten Prozessorzyklen und den Kern (nur Windows). Die
  Umgebungsvariable `LUMIVIZ_MILKDROP_LANG_MS=<ms>` setzt die Schwelle.

## Geändert

- **Shapes ohne Textur und ohne Rand** gehen gesammelt in einen Zeichenaufruf
  statt in je einen. Eine Instanz mit Textur oder Rand, oder ein Wechsel der
  Mischart, zeichnet zuerst das bis dahin Gesammelte — was was überdeckt,
  bleibt. Die Randfarbe wird einmal je Instanz gelesen statt je Eckpunkt.
  Am Bild ändert sich nichts: 16 Presets, je 241 Bilder, alle 3856
  Bild-Hashes gleich wie vorher.
- **Wirkung an `martin - glassworks 3`** (1350 Instanzen, 1836 × 909, RTX 4090):
  ein Bild 10,0 bis 11,4 ms statt 13,6 ms.

## Befund

- **Das Stocken ist eine Drosselung des Prozessors:** für rund 3 s werden alle
  Stufen des Renderers sieben- bis achtfach langsamer bei gleicher Arbeit, und
  ein unabhängiger Prozess wird in denselben Sekunden 7- bis 12-mal langsamer.
  Wer drosselt, ist nicht belegt. Einzelheiten und Zahlen:
  [Offene_Punkte.md](../Offene_Punkte.md) §3.
- **Zwei solche Presets zugleich** (zwei Kanäle des Composers) laufen mit 33,5
  bis 45,4 Bildern je Sekunde — auch nach dem Umbau keine 60.

## Offen

- Die Übergabe der Variablen ans Skript billiger machen (Stufe 3) ist
  zurückgestellt.
- Kein eigener Test für das Sammeln der Shapes; Debug-MSVC nicht gebaut.

## Tests

- 974 Cases, zwei volle Läufe 974 / 974, 0 Skips (Testing-MSVC).
