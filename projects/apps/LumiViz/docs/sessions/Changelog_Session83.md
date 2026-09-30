# Changelog Session 83 (2026-09-30)

> **Presets überall, Vorlagen für jeden Shader, und LumiViz spricht jetzt
> Englisch.** Jede Stelle, an der man etwas einstellt, hat nun eine
> Preset-Zeile — auch einzelne Waves, Shapes, Sprites und die Code- und
> Parameter-Sektion eines Milkdrop-Nodes. Shader-Felder bekommen Vorlagen,
> Skripte lassen sich abschalten, ohne sie zu löschen, und Oberfläche wie
> mitgelieferte Presets sind englisch.

## Neu

- **Preset-Zeile zeigt, woher der Stand kommt:** der gewählte Name bleibt
  stehen; nach einer Änderung steht **„(unsaved) – Name"**, ohne Preset
  **„(none)"**. 🗑 löscht auch im „(unsaved)"-Zustand das Ursprungs-Preset.
- **Presets für Milkdrop-Elemente:** eine einzelne Custom Wave, Custom Shape
  oder ein Sprite hat eine eigene Preset-Zeile — mitgeliefert u. a.
  *Oscilloscope Line*, *Spectrum Circle*, *Pulse With Bass*, *Slow Fade*.
- **Presets für die Milkdrop-Sektionen Code und Parameter:** kleine
  per_frame-/per_pixel-Bausteine (*Slow Rotation*, *Ripple (per pixel)* …)
  und Stellungen wie **Original** oder **Old LumiViz Look (pre-S80)** — der
  weiche Sog nach oben links samt den alten Einstellungen in einem Zug.
- **Shader-Vorlagen im großen Editor:** Zeile **Template:** mit **Empty**
  (nur der Funktionsrumpf) und einfachen Vorlagen je Shader-Art (Shadertoy,
  Mesh Warp, Pixel Filter, GPU Particles, ISF, MilkDrop Warp/Comp); eigene
  Vorlagen mit **Save as template…**. Strg+Z holt den alten Text zurück.
- **Schalter „Script active"** an jedem Knoten mit Formelfeldern: aus = der
  Knoten läuft, als wären seine Formeln leer; der Code bleibt gespeichert.
- **Eigene Hilfeseite (ⓘ) für ISF-Filter.**

## Geändert

- **Alle mitgelieferten Presets, Beispiele und Vorlagen heißen englisch**
  (z. B. *Midnight Blue Veil*, *Gentle Pull*, *Take On Me Comic*) und sind
  englisch kommentiert. Die Zuordnung alt → neu steht in
  `asset/nodepresets/README.md`.
- **Die Oberfläche ist englisch** — auch Milkdrop-Parameter, Video Source,
  ISF, Shadertoy, Host-Gruppen, Host Switcher, Einstellungen und die
  Hilfeseiten. „Testaufnahme" heißt jetzt **Camera Clip**, der Reiter
  „Kamera" **Camera**. (Die Feld-Tooltips folgen in der nächsten Session.)
- Die Vorlagen-Listen von **SuperScope 3D** und **3D Camera** sind jetzt
  normale Presets derselben Namen.

## Behoben

- Die Preset-Auswahl sprang nach dem Laden auf „(unchanged)" zurück — Löschen
  der gewählten Voreinstellung war so nicht möglich.
- Die ISF-Editoren zeigten die Hilfeseite des Pixel Filters.
- Sonderzeichen in den Tabellen der Shader-Hilfeseiten erschienen als
  Zeichensalat.
