# BeatModule — Gemeinsame Beat-Detection

> **Version:** 1.1.0  
> **Datum:** 2026-07-22  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Visualizers::Modules::BeatModule  
> **Dateien:** BeatModule.hpp (header-only)  
> **Namespace:** lumi::modules  
> **Abhängigkeiten:** keine (C++20)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## 1. Übersicht

Konsolidiert die früheren Ad-hoc-Beat-Detektoren der Visualizer
(Phase 4 Schritt 5.6). Zwei Betriebsarten in einem zustandsbehafteten Modul:

| Modus | Methode | Algorithmus | Nutzer |
|---|---|---|---|
| Kanten-Trigger | `update(level)` | `level × sensitivity` überschreitet `threshold` aufsteigend (Flanke) | PulsingVisualizer (Beat-Reverse-Rotation) |
| Adaptiv | `updateAdaptive(energy)` | mitlaufender Schwellwert (Running Average, Trägheit 0.95); Beat bei `energy > 1.5 × Schwellwert` und über dem Rauschboden 0.1 | SuperscopeVisualizer (Script-Variable `b`) |
| AVS-Onset | `updateAvsOnset(meanAbs)` | Port des vis_avs-Detektors (ref main.cpp:290-329): Peak-Tracker `peak1` (träge, mischt `peak2` ein), Beat bei `Level ≥ peak1×34/32` über dem Floor (Ø-Betrag 16/128), danach Schwellen-Anhebung `(Level+letzter Peak)/2` + Refire-Guard (`beat_cnt`) — diskrete Events statt Mehr-Frame-Bursts | MultiEffectVisualizer (AVS-Import; danach `BeatEstimator::refine`) |

## 2. API

```cpp
lumi::modules::BeatModule beat;
beat.setThreshold(0.4f);      // Kanten-Modus
beat.setSensitivity(1.0f);
bool onBeat = beat.update(audioLevel);        // Flanken-Erkennung
bool isBeat = beat.updateAdaptive(rmsEnergy); // adaptiver Energie-Modus
bool onset  = beat.updateAvsOnset(meanAbs);   // AVS-treuer Onset (Ø|Waveform|, max L/R)
beat.reset();                 // Detektions-Zustand löschen (Config bleibt)
beat.resetToDefaults();       // Config + Zustand
```

Beide `update*`-Methoden sind `[[nodiscard]]`; die adaptiven Konstanten
(`kAdaptiveFollow`/`kAdaptiveRatio`/`kAdaptiveMinEnergy`) sind bewusst
nicht konfigurierbar (Verhalten = bisheriger Superscope-Detektor).

## 3. Hinweise

- Kein Parameter-Schema (`paramDescs`): Beat-Detection hat derzeit keine
  UI-Parameter; die Pulsing-Legacy-API `setBeatSensitivity` delegiert hierher.
- Ein Detektor für den Equalizer (`GradientDomain::Beat` hat heute keinen)
  wäre ein neues Feature auf dieser Basis.

## 4. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 1.1.0 | 2026-07-22 | `updateAvsOnset`: AVS-treuer Onset-Detektor (main.cpp-Port) für den MultiEffect-Host (Import-Treue-Fixplan A3) |
| 1.0.0 | 2026-07-19 | Initial (Phase 4 Schritt 5.2/5.5): Kanten-Trigger + adaptiver Energie-Modus; ersetzt Inline-Detektoren in Pulsing und Superscope |
