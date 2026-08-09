# AudioUtil — Gemeinsame Audio-Puffer-Helfer

> **Version:** 1.0.0  
> **Datum:** 2026-07-19  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Visualizers::Modules::AudioUtil  
> **Dateien:** AudioUtil.hpp (header-only, freie Funktionen)  
> **Namespace:** lumi::modules  
> **Abhängigkeiten:** C++20 STL  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## 1. Übersicht

Konsolidiert die pro Visualizer kopierten Stereo-Split-/Resample-Helfer
(Phase 4 Schritt 5.6, N3: genau EINE Split-Implementierung).

```cpp
// L/R-interleaved → zwei Kanal-Puffer (Ziel wird passend resized)
lumi::modules::splitStereoData(interleaved, left, right);

// Nearest-Neighbor-Resampling auf targetSize + Gain (leere Quelle = No-Op)
lumi::modules::resampleNearest(source, target, targetSize, gain);
```

Aktiver Nutzer: WaveformVisualizer (Display-Pfad). Die früheren Kopien in
Oscilloscope und Superscope waren tot und wurden entfernt; das Oscilloscope
behält sein eigenes Inline-Display-Resampling (lineare Interpolation + Clamp
±1 — bewusst andere Semantik für das Div-Raster).

## 2. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 1.0.0 | 2026-07-19 | Initial (Phase 4 Schritt 5.6): splitStereoData + resampleNearest; tote Kopien in Oscilloscope/Superscope entfernt |
