# Changelog Session 37 (2026-07-21) — Batch H · Skript-/Audio-System · Stereo

> **Typ:** Produkt-Changelog
> **Bezug:** [Import_Modul_Umsetzungsplan.md](../visuals/Import_Modul_Umsetzungsplan.md) ·
> [Skript_Variablen_Konzept.md](../visuals/Skript_Variablen_Konzept.md) ·
> [Offene_Sichttests.md](../Offene_Sichttests.md) ·
> [Offene_Implementierungen.md](../Offene_Implementierungen.md)
> **Tests:** MyViz.UnitTests 298 Cases grün, 0 Skips, 4072 Assertions (vorher 289)

## Neu

### Batch H — 9 host-native Fraktal-Module

Eigenständige Content-Quellen (kein AVS-Import), im MultiEffect-Panel unter
„— Fractals —": **Fractal 2D** (Mandelbrot/Julia/Burning Ship/Tricorn/Multibrot/
Newton/Phoenix/Magnet/Nova), **Fractal 3D** (Raymarch: Mandelbulb/Mandelbox/Menger/
Quaternion-Julia/KIFS), **Domain Warp** (fBm-Plasma), **Fractal Zoomer** (Endlos-Zoom
+ Feedback), **Lyapunov**, **Kleinian** (stilisierte Kachelung), **Strange Attractors**
(Lorenz/Clifford/DeJong/Aizawa), **Flame/IFS**, **Reaction-Diffusion** (Gray-Scott).
Alle mit Gradient-Palette, optionalen EEL-Slots (init/frame/beat) mit Audio-Reaktion
und Blend über die Kette. **GL-Sichttest offen.**

### Audio-Analyse im Skript (getspec/getosc/gettime)

Die AVS-Funktionen `getspec(band,width,ch)` / `getosc(...)` / `gettime(sc)` sind jetzt
real in der Skript-Engine verfügbar (originalgetreu) und werden an **alle** scripted
Effekte gespeist — inkl. `bass/mid/treb/vol/beat/time`. Damit importieren AVS-Presets,
die auf Spektrum-/Waveform-Zugriff bauen, **korrekt** statt fehlerhaft.

### Echtes L/R-Stereo

Getrennte Kanäle für `getspec/getosc` (ch=1 links, ch=2 rechts) über eine additive
Stereo-Pipeline (Mono bleibt Fallback). **Hörtest offen** (Kanal-Layout-Verifikation).

### Skript-Editor

Jedes Code-Feld hat einen **ⓘ-Button** (Variablen-/Funktions-Referenz je Modul) und
einen **⤢-Button** (großer, größenveränderbarer Editor mit Referenz daneben).
**Syntax-Highlighting** färbt Variablen nach Kategorie (read-only/input/output/…),
markiert **Fehler rot** (Schreiben auf read-only/Konstante, doppelte Global-Init).

### Effektliste — Mehrfach-Selektion

Shift (Bereich) / Ctrl (einzeln) auf gleicher Ebene; Gruppen verschieben (↑/↓ + Drag)
und entfernen (Button + Entf/Backspace).

## Geändert

- **Set Render Mode** ist ein echter Ketten-Knoten (setzt Linienbreite/Blend live für
  folgende Scopes) statt Import-Zeit-Auflösung — keine „passthrough"-Notizen mehr.
- **MultiEffect-Palette** nach 6 Kategorien gegliedert; Effekte tragen einen
  Herkunfts-Marker `· AVS` / `· LumiViz` (Vorbereitung für Icons).

## Bekannte Einschränkungen

- **GL-Sichttest (Batch H, Set Render Mode) + Audio-Hörtest (Stereo) stehen aus** —
  Details/Kalibrier-Punkte in `docs/Offene_Sichttests.md`.
- Set Render Mode wirkt aktuell nur auf SuperScope (Ausdehnung auf alle Scopes geplant).
- Kategorie-Highlighting ist modul-unabhängig (Verfeinerung geplant).
