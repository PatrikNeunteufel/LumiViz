# BeatEstimator — vorhersagender BPM-Schätzer (AVS smart beat)

> **Version:** 1.0.0
> **Datum:** 2026-07-20
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Import-Phase Roadmap 4.4, Entscheid E3)
> **Modul:** lumi::modules::BeatEstimator
> **Dateien:** processing/BeatEstimator.hpp (header-only)
> **Namespace:** lumi::modules
> **Abhängigkeiten:** keine
> **Lizenzhinweis:** Port von `ref/vis_avs/avs/vis_avs/bpm.cpp` — BSD-3, Copyright 2005 Nullsoft, Inc. (Hinweis im Header mitgeführt)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Sitzt HINTER einem Onset-Detektor ([BeatModule](BeatModule.hpp)): je Frame
`refine(onsetBeat, nowMs)` füttern, zurück kommt der verfeinerte/vorhergesagte
Beat. Lernt BPM aus einer 8er-Beat-Historie (Konfidenz aus Real-Anteil +
Intervall-Drift), diskriminiert Halb-/Zwischen-Beats, korrigiert
Halb-/Doppeltempo (60–170 BPM), rastet bei anhaltend hoher Konfidenz ein
(Sticky) und **hält den Beat durch Fade-outs** (Vorhersage-Ticks).
Chain-scoped Objekt — kein Service, kein Locking (Render-Thread-Besitz);
der Custom-BPM-Override-Punkt kommt mit dem Multieffekt-Host (Roadmap 5).

## 2. Verwendung

```cpp
const bool onset = m_beat.updateAdaptive(energy);            // Onset wie bisher
const bool beat  = m_estimator.refine(onset, BeatEstimator::steadyNowMs());
```

Pulsing + Superscope: Param **`map.beat.predict`** (Bool, Default aus) wählt
zwischen Onset (Ist-Verhalten) und Vorhersage; der Estimator wird auch bei
„aus" gefüttert (warm beim Umschalten). `notifyTrackChanged()` beim
Titelwechsel aufrufen (AVS-Songwechsel-Semantik — harte Tempo-Sprünge im
laufenden Betrieb adaptiert der Algorithmus bewusst nur träge).

## 3. Bewusste Abweichungen vom Original

1. Zeitbasis: monotone Millisekunden vom Aufrufer (testbar, kein GetTickCount);
   Dialog-/Winamp-/Slider-Kopplung und die ungenutzte Zweit-Historie entfallen.
2. Drei Original-Defekte begradigt (im Header markiert): unintialisiertes
   Resync-BPM (Shadowing), stehen gebliebener Intervall-Akkumulator in der
   zweiten Mittelungsschicht, sqrt aus negativer Varianz (Klemmung auf 0).
3. `cfg_smartbeat` ist immer „an" — die Wahl trifft der Konsument über den
   Param; `sticky`/`onlySticky` sind Config.

## 4. Tests

`test_BeatEstimator.cpp` — synthetische Onset-Folgen (10-ms-Ticks): Lernen auf
120 BPM (±2 durch den Resync-Nudge des Originals), Vorhersage durch Stille,
Halbierung bei 240 BPM, Sticky-Einfrieren, Neu-Lernen nach Track-Wechsel,
onlySticky-Durchreichung, Reset.

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-20 | Erstfassung — bpm.cpp-Port + map.beat.predict in Pulsing/Superscope (Session 33) |
