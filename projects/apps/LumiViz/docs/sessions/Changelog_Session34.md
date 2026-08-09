# Changelog Session 34 (2026-07-20) — Import-Phase Roadmap 5 (Multieffekt-Host)

> **Typ:** Produkt-Changelog
> **Bezug:** [Import_Multieffekt_Host_Entwurf.md](../visuals/Import_Multieffekt_Host_Entwurf.md)
> (freigegeben, E1–E7) · [Import_Analyse_AVS_MilkDrop.md](../visuals/Import_Analyse_AVS_MilkDrop.md) §5/§8.2
> **Tests:** LumiViz.UnitTests 222 Cases grün, 0 Skips (vorher 185)

## Neu

### Multieffekt-Host — AVS-Effektketten rendern, importieren, editieren

- **Visualizer „Multi Effect"** (`MultiEffectVisualizer`): rendert einen
  Effektketten-Baum nach dem AVS-Render-Modell — persistente Ping-Pong-
  Arbeitsfläche, verschachtelte Listen mit eigenem frame-persistentem Buffer
  (`thisfb`), 14 Blend-Modi (In/Out), OnBeat-Aktivierung, EEL-Listen-Slots
  (`enabled/clear/beat/alphain/alphaout`), geteilter Skript-Kontext.
- **17 AVS-Effekte:** Clear, Fadeout, Invert, Brightness, Fast Brightness, Blur,
  Mirror, OnBeat Clear, Colorfade, Color Modifier, Movement, Dynamic Movement,
  Blitter Feedback, Roto Blitter, Buffer Save, Custom BPM, SuperScope.
  Nicht abgedeckte Effekte werden als **Passthrough konserviert** (nie Absturz).
- **AVS-Import:** `File → Import AVS Preset…` (Ctrl+I) parst ein `.avs`-Preset,
  übersetzt es in die Kette und rendert es; ein Report listet konservierte/nicht
  abgedeckte Effekte. **Korpus: 35/35 Referenz-Presets übersetzen fehlerfrei.**
- **Host-Presets:** `File → Load/Save Effect Chain…` (`.lvfx`, verschachteltes
  JSON, vorwärtskompatibel).
- **Ketten-Editor-Panel** (`View → Panels → Effect Chain`): Baumansicht der Kette
  (hinzufügen/entfernen/umordnen/aktivieren), Parameter-Editor je Effekt inkl.
  EEL-Skriptfeldern, editierbare **Name**- und **Description**-Spalten.

### Neue Bausteine (wiederverwendbar)

- **ScopeRenderer** — Punkt-/Linien-Zeichenkern (Dots/Thin/Thick), aus dem
  Superscope extrahiert.
- **AvsChainTranslator** — AVS-Baum → Host-Kette (COLORREF→RGB, Set-Render-Mode
  ausgerollt, Passthrough-Report).
- **ChainSerializer** — `.lvfx`-Persistenz.

## Geändert

- **Superscope Trail Decay:** Maximum von 1.0 auf **0.98** gedeckelt (Clamp +
  Slider) — verhindert den Weiß-Ausbruch bei „Additive" + Decay ganz rechts.
- Solution.json: Libs/Targets für den Host verdrahtet; neue Quellen registriert.

## Bekannte Lücken / Ausblick

- **Nächster Schritt:** restliche Effekt-Abdeckung (AVS „Mittel"-Stufe: DDM,
  Bump, Water, Interferences, Grain, Mosaic, Scatter, Delays, Starfield,
  Timescope … + die 23 Movement-Builtin-Formeln) und die **exotischen
  Blend-Modi** (XOR, Every-other-line/-pixel, Subtractive, Buffer) — derzeit
  Fallback „Replace".
- Feinschliff: Skriptfelder committen pro Tastendruck (Debounce folgt);
  Blitter/Roto-Zoom-Mapping approximativ; SuperScope nutzt noch einen privaten
  Skript-Kontext; `SuperscopeVisualizer` selbst noch nicht auf `ScopeRenderer`
  umgezogen.
- Roadmap 6: MilkDrop-Import.
