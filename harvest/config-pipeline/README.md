# harvest/config-pipeline — Anforderungen & Referenz für die einheitliche Visual-Config

**Quellen:** GreatVisual_most param eq, Visualizer_better_wave, lose Notizen. **Ziel: Phase 4.**

Kernproblem (vom Projektinhaber benannt): Die Config der verschiedenen Visuals soll **eine
gemeinsame Pipeline** mit identischem Verhalten haben — heute verhalten sich Elemente
(z. B. Gradient-Editor) je nach Visual unterschiedlich, die Modularisierung ist zu grob.

## Inhalt

| Datei | Was drinsteckt |
|---|---|
| `equalizer_modulubersicht_parametrierung_ist_stand_ideen.md` | **Die Blaupause.** Vollständige Parameter-Referenz des Equalizers (alle Keys, Ranges, Defaults, Wirkungen, Interaktionen/Stolperfallen) + Modularisierungs-Fahrplan (Module A–E: Bands, Gradient, Bars, Peak-Spawner, Partikel) + nicht umgesetzte Ideen E1–E7 |
| `idee preview$.txt` | Idee: **Preview-Viewer je Parametergruppe** (Raw-Anzeige der Audio-Werte; je Effekt/Gruppe/Untergruppe testbar), **Default-Buttons auf jeder Ebene**, **Presets je Gruppe** |
| `reset to default.txt` | Notizen zum Default-Reset-Verhalten |
| `audosource linear, log usw, filter usw je vis oder gemeinsam.txt` | Grundsatzfrage: AudioSource-Einstellungen (Skala, Filter, …) **pro Visual oder geteilt** — muss die Pipeline als explizites Konzept beantworten |

## Anforderungen (von Patrik, 2026-07-18)

1. **Die Config-UI folgt der Pipeline-Reihenfolge.** Gruppen im ConfigPanel in der Reihenfolge,
   in der die Daten fließen — durchgängig für ALLE Visuals (war bisher nicht konsequent):
   1. AudioSource/Analyse (FFT-Größe, Skala Linear/Log/Mel, EMA-Glättung, dB-Floor/Ceil)
   2. Band-/Daten-Mapping (bands, gain, Orientierung)
   3. Farbe (Gradient/Solid — einheitlicher Gradient-Editor!)
   4. Rendering/Geometrie (Balken, Linien, Z-Order)
   5. Peak/Partikel (Spawner-Physik, Partikel)
   6. Post-Processing
2. **Preset-Typ-Vertrag beachten:** Der PresetManager liefert Zahlen beim Laden IMMER als
   `float` (JSON kennt kein int) — alle `setParam`-Implementierungen müssen float für
   int-Parameter akzeptieren (durch Unit-Test dokumentiert: test_VisualizerPresetManager.cpp).
3. **Gradient-Invariante:** `clearStops()` hält ≥ 2 Stops (ensureMinimumStops) —
   UI/Serialisierung müssen damit rechnen (Test: test_ColorGradientModule.cpp).

## Wiederverwertung

- Aus der Equalizer-Referenz das **generische Parameter-Schema** ableiten (Key, Typ, Range, Default,
  Wirkung, Abhängigkeiten) → gemeinsames Modell für alle Visuals (paramDescs vereinheitlichen).
- Die Ideen (Preview je Gruppe, Default je Ebene, Gruppen-Presets) als Anforderungen in das
  Phase-4-Konzept übernehmen; CommandBus (harvest/core-module) liefert dazu Undo/Redo.
- Gradient-Editor als erstes gemeinsames Modul vereinheitlichen (bekanntester Abweichler).
