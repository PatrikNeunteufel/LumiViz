# Changelog — Session 29 (2026-07-18/19): Config-Pipeline Phase 4, Schritte 0–4

> **Version:** 1.0.0
> **Datum:** 2026-07-19
> **Typ:** Changelog
> **Status:** Abgeschlossen
> **Sprache:** Deutsch

Produkt-Änderungen an MyViz in Session 29 (Phase 4 „Config-Pipeline vereinheitlichen",
Schritte 0–4 von 7). Tests am Ende: **61 Cases / 308 Assertions grün, 0 Skips**;
automatisierter App-Exit-Test 0x0.

## Entfernt (Schritt 0, −3.6k LOC)

- **EqualizerModule entkernt:** ungenutztes paralleles Parameter-System
  (paramDescs/getParam/setParam) und tote Audio-Pipeline
  (processSpectrum/mapSpectrum/applyEMA/normalizeDb) — der Modul-interne `gain`
  war seit dem AudioSourceModule-Umstieg wirkungslos.
- **Gelöscht:** ColorSchemeModule + ColorSchemeTraits (Alt-Farbsystem;
  `GradientDomain` → ColorGradientModule.hpp), ModuleConfigWidget (unreferenziert),
  AudioAnalyzer/IAudioAnalyzer (nie verdrahtet; Datenweg ist
  `MainWindow::onAudioUpdate`).
- **Tote Events ohne Publisher:** VisualizerColorScheme/Smoothing/PeakHold/
  Shape-Events + AudioDataEvent, inkl. Subscriber-Maschinerie im VisualizerWidget.
- Visualizer-Kategorien gefixt (`shape`/`spectrum`/`waveform` statt
  Platzhalter-Strings); 6 ungenutzte Felder (Pulsing, Superscope).

## Neu

- **EventBus-RAII (Schritt 1):** `SubscriberHandle` (auto-unsubscribe,
  Liveness-Token — teardown-sicher), `subscribeScoped/Weak/ScopedWeak` mit
  Auto-Purge; PanelBase-RAII-Abo-Ablage (alle 4 Panels umgestellt →
  use-after-free-Falle beseitigt). `Event::consume()` ist const.
  ServiceContainer: Transient-Dangling gefixt (tryResolve→nullptr, resolve wirft).
  **DialogManager verdrahtet** — About-Dialog (F1) öffnet wieder; Shutdown-Crash
  (AV beim Beenden) behoben.
- **CommandBus (Schritt 2):** Undo/Redo für Parameteränderungen —
  Slider-Drags verschmelzen zu EINEM Undo-Schritt (Merge-Fenster 750 ms);
  neues **Edit-Menü** (Undo Ctrl+Z / Redo Ctrl+Y); Widgets syncen nach Undo/Redo.
- **Gemeinsames Schema (Schritt 3):** `PipelineStage`-Enum (Analyse → Mapping →
  Farbe → Rendering → Peak/Partikel → Post) im Parameter-Descriptor;
  **GradientHandles** (benannter Gradient-Zugriff aller 5 Visualizer, inkl.
  Waveform Left/Right und Oscilloscope ch1–m2); **TapPoints** (pull-basierte
  Stufen-Abgriffe, Equalizer-Pilot); Preset-Loader wertet `formatVersion` aus und
  übersetzt Legacy-Keys über eine Alias-Registry.
- **ConfigPanel generisch (Schritt 4):** Stage-Sortierung mit Legacy-Fallback
  (UI unverändert bis zur Key-Migration); Gradient-Editor/Preview/Preset-Save
  laufen über Handles statt dynamic_cast — **Waveform Left/Right erstmals über
  den Editor erreichbar**, Save-Buttons an allen Gradient-Dropdowns;
  Modul-Preset-Save für alle Visualizer; Sichtbarkeit rein über transitive
  dependsOn-Auswertung; **Default-Reset per Rechtsklick** (Parameter/Gruppe,
  undo-fähig).

## Doku

- App-Doku **nach Domänen neu aufgebaut** (core-services · audio · visuals · ui ·
  presets; Alt-Doku archiviert in `harvest/old_docs/`), gegen den Code korrigiert
  (ParamValue-Typen, `.lvp`-Schema, Parameter-Referenz erstmals für alle 5 Visualizer).
- Phase-4-Steuerdokumente: `Config_Pipeline_Concept.md` v0.3 (freigegeben, inkl.
  AVS/MilkDrop-Leitplanken), `Config_Pipeline_Umsetzungsplan.md` (27/40),
  `Parameter_Key_Migration.md` v0.2 (236 Keys, Review E1–E8 abgeschlossen).

## Bekannte offene Punkte

- Schritt 5 (Visualizer auf neue Keys/Stufen migrieren — erst danach ordnet sich
  die Config-UI sichtbar nach der Pipeline), Schritt 6 (Preview je Gruppe),
  Schritt 7 (Doku-Nachzug inkl. Parameter_Reference auf neue Keys).
- Manuelle Sichttests der neuen UI-Funktionen stehen aus (Undo/Redo, About-Dialog,
  Gradient-Editor L/R, Rechtsklick-Reset).
