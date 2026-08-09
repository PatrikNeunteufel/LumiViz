# PostFxModule — Gemeinsame Post-Processing-Bausteine (HoldFade)

> **Version:** 1.0.0  
> **Datum:** 2026-07-19  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Visualizers::Modules::PostFx  
> **Dateien:** PostFxModule.hpp (header-only)  
> **Namespace:** lumi::modules  
> **Abhängigkeiten:** C++20 STL  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## 1. Übersicht

Konsolidiert die pro Visualizer kopierte Hold/Fade-Frame-Mechanik
(Phase 4 Schritt 5.6, Konzept §5.6 Leitplanke 1: Wertquelle und Effekt bleiben
entkoppelt). Kern ist das Template **`HoldFadeEffectT<TFrameData>`** — ein
Trail vergangener Frames mit linearem Ausblenden:

- `HoldFadeEffect` (= `HoldFadeEffectT<std::vector<float>>`): Sample-Puffer,
  Nutzer WaveformVisualizer (`post.hold.*`, je Kanal eine Instanz).
- `HoldFadeEffectT<std::vector<SuperscopePoint>>`: Punktlisten,
  Nutzer SuperscopeVisualizer.

Die **Konfiguration** (enabled/fadeTime/maxFrames) bleibt im Parameter-Schema
des jeweiligen Visualizers (`post.hold.*`) — das Modul besitzt nur die
Frame-Mechanik.

## 2. API

```cpp
lumi::modules::HoldFadeEffect trail;
trail.push(samples, maxFrames);        // Frame einreihen (älteste fliegen raus)
trail.update(deltaTime, fadeTimeSec);  // altern; alpha = 1 − age/fadeTime; Verblasste entfernen
for (const auto& frame : trail.frames())   // älteste zuerst
    render(frame.data, frame.alpha);
trail.clear();
```

`HeldFrameT<T>` trägt `data` (Payload), `age` (Sekunden) und `alpha` (1 → frisch,
0 → verblasst).

## 3. Hinweise

- Oscilloscope-Phosphor war toter Code (nie befüllt/gerendert, E7) und wurde
  entfernt — ein echtes Phosphor/Glow als Shader-Effekt ist ein künftiges
  PostFx-Feature auf dieser Basis.

## 4. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 1.0.0 | 2026-07-19 | Initial (Phase 4 Schritt 5.3/5.5): HoldFadeEffectT generisch (Sample-/Punkt-Frames); ersetzt Frame-Fade-Kopien in Waveform und Superscope |
