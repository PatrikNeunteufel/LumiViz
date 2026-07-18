# Changelog — Session 27b (Nachtrag, 2026-01-04 bis 2026-01-05)

> **Version:** 1.0.0
> **Datum:** 2026-07-18 (nachgetragen)
> **Typ:** Changelog
> **Status:** Abgeschlossen

> **Nachtrag:** Diese Änderungen wurden im Januar 2026 committet, aber nie im Changelog
> dokumentiert (Lücke zwischen Session 27 und der Repo-Reorganisation im Juli 2026).
> Quellen: Commits `d2de8c4`…`1a1c8c1`.

## Neue Features

### 🎚️ Equalizer-Visualizer (`4ab675c "equalizer added"`)

Neuer Visualizer **Equalizer** (`EqualizerVisualizer`, ~2.000 Zeilen inkl. Modul):
FFT-Band-Darstellung mit umfangreicher Parametrierung — Bands/Gain/Orientierung,
AudioSource-Settings (Skala Linear/Log/Mel, EMA-Glättung, dB-Floor/Ceil), Gradient-Farbgebung
(ByPosition/ByAmplitude, Custom-Stops), Peak-Hold mit Spawner-Physik (Delay/Gravity/Falloff,
optional Spring-Follow) und Peak-Partikeln. Vollständige Parameter-Referenz:
`harvest/config-pipeline/equalizer_modulubersicht_parametrierung_ist_stand_ideen.md`.

### 🧬 Superscope-Verbesserung (`d2de8c4 "better dna in superscope"`)

DNA-Preset im Superscope überarbeitet.

## Fixes (`e7af4c2`)

- **Preset-Bugs** behoben (mehrere)
- **Audio-Order für den Equalizer** korrigiert
- **Gradient-Editor-Dialog** für den Equalizer repariert

## Infrastruktur (`1a1c8c1`)

- Case-Sensitivity-Fixes für Linux-Builds (Dateinamen/Includes)

---

*Danach folgte die Repo-Reorganisation (Juli 2026): NewViz → LumiViz, Build-System-Extraktion
nach CMakeCraft — dokumentiert in der CMakeCraft-/LumiViz-Historie und den lokalen
Session-Reports (`.claude/sessions/`).*
