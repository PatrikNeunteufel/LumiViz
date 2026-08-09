# Changelog Session 33 (2026-07-20) — Import-Phase Roadmap 3 + 4

> **Typ:** Produkt-Changelog
> **Bezug:** [Import_Analyse_AVS_MilkDrop.md](../visuals/Import_Analyse_AVS_MilkDrop.md) §9 ·
> [Import_Fundament_Entwurf.md](../visuals/Import_Fundament_Entwurf.md)
> **Tests:** LumiViz.UnitTests 185 Cases grün, 0 Skips (vorher 135)

## Neu

### Roadmap 3 — .avs-Preset-Parser

- **Lib `AvsParser`** (`projects/libs/AvsParser`, header-only, Qt-frei,
  Namespace `lumi::avs`; Doku: `include/AvsParser.md`):
  - Container: Signatur „Nullsoft AVS Preset 0.1/0.2", rekursives TLV,
    verschachtelte Effektlisten (Extended-Data, Listen-EEL-Pseudo-Eintrag),
    APE-ID-Strings + Alias-Tabelle alter benannter APEs, EEL-Strings in neuem
    (längenpräfixiert) und Altformat (256er-Blöcke).
  - Blob-Decoder der Import-Kernmenge (~17 Effekte, 1:1 aus ref/vis_avs):
    Effect List, SuperScope, Movement, Dynamic Movement, Blur, Fadeout,
    Brightness, Fast Brightness, Color Modifier, Colorfade, Clear, OnBeat
    Clear, Buffer Save, Mirror, Invert, Roto Blitter, Blitter Feedback,
    Custom BPM, Set Render Mode.
  - Import-Report: pfad-präfixierte Warnungen, nie hart abbrechen; unbekannte
    Effekte als Roh-Blob konserviert.
  - Verifikation: 17 Test-Cases inkl. Korpus-Lauf — **35/35 Referenz-Presets
    parsen fehlerfrei** (170 Effekte, 5 erwartete Warnungen).

### Roadmap 4 — Feedback-/Skript-Modul-Fundament (Entwurf freigegeben, E1–E5)

- **ScriptContext** (`include/scripting/`): preset-lokal geteilter
  Skript-Zustand — `reg00–99`, `q1–q64` mit MilkDrop-Snapshot-Semantik
  (MD3-Superset), `gmegabuf`; Engines teilen ihn per shared_ptr.
- **ScriptSlotHost**: das EEL-Quartett-Muster (Transpile → compile → run mit
  Fallback + reg/q-Kontext-Sync) als gemeinsamer Baustein — aus dem
  SuperscopeModule extrahiert, Superscope-Verhalten unverändert.
- **ScriptGridModule** (pro Gitterknoten, polar/rect — AVS Movement/DMove,
  MilkDrop per_vertex) und **ScriptLutModule** (256er-RGB-LUT pro Eintrag,
  recompute-Flag — AVS Color Modifier), beide GL-frei.
- **FeedbackBuffer** (`include/visualizers/render/`): Doppel-FBO-Feedback mit
  Blit-Resize (Trails überleben Fenster-Resize) + interner Echo-Pass;
  **OffscreenBufferPool** (8 Slots, AVS-getGlobalBuffer-Semantik) als
  API-Gerüst für den Multieffekt-Host.
- **BeatEstimator** (`modules/processing/`): Port der AVS-„smart beat"-Logik
  (bpm.cpp, BSD-3) — BPM-Lernen mit Konfidenz, Halb-/Doppel-Korrektur,
  Sticky-Einrasten, Vorhersage durch Fade-outs.

### Neue Visualizer-Parameter (alle Default AUS — Verhalten unverändert)

| Visualizer | Parameter | Wirkung |
|---|---|---|
| Superscope | `post.trail.enabled/decay/zoom` | GL-Echo des Vorframes (Blitter-Feedback-Look) |
| Superscope, Pulsing | `map.beat.predict` | Beat aus BPM-Vorhersage statt Onset (läuft durch Fade-outs) |

## Geändert

- LuaScriptEngine: Konstruktor nimmt optional einen geteilten ScriptContext;
  `gmegabuf` wohnt jetzt dort (ohne Kontext: privat = Alt-Verhalten).
- Superscope-Farberkennung im Skript-Modus ist wortgenau + case-insensitiv
  (`RED=1` triggert jetzt; „shredder" nicht mehr).
- Solution.json: Lib AvsParser registriert, als Core-Dependency verdrahtet
  (CMakeCraft wertet Test-Target-`dependencies` derzeit nicht aus — Fix im
  CMakeCraft-Repo angestoßen).

## Bekannte Lücken / Ausblick

- Sichttests ausstehend: Trail (Look/Resize/Undock/Fullscreen/Frametime),
  Predictive Beat, Lua-Geschwindigkeit (S32-Nachzug).
- Roadmap 5: Multieffekt-Host + Übersetzung AvsParser-Baum → LumiViz-Presets;
  Roadmap 6: MilkDrop-Import.
