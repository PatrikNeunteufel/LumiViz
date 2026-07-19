# Changelog — Session 30 (2026-07-19): Config-Pipeline Phase 4, Schritte 5–7

> **Version:** 1.0.0
> **Datum:** 2026-07-19
> **Typ:** Changelog
> **Status:** Abgeschlossen
> **Sprache:** Deutsch

Produkt-Änderungen an MyViz in Session 30 (Phase 4 „Config-Pipeline vereinheitlichen",
Schritte 5–7 — **Phase 4 damit ABGESCHLOSSEN, 40/40**).
Tests am Ende: **91 Cases / 2080 Assertions grün, 0 Skips**. Sichttests 5.1–5.5 und
6.2 sowie die 6.3-Frametime-Messung durch Patrik bestanden (Preview aus = Basis
60 fps → N7 erfüllt; Live-Previews eingeblendet ~52 fps; Farbstreifen nach
Optimierung — QLinearGradient-Fill, Repaint nur bei Gradient-Änderung,
Kurven-Dezimierung — kostenlos).

## Neu

- **Key-Migration aller 5 Visualizer (Schritt 5.1–5.5):** Equalizer, Pulsing,
  Waveform, Oscilloscope und Superscope sprechen jetzt das Pipeline-Schema
  (`audio.` → `map.` → `color.<handle>.` → `render.` → `peak./particle.` → `post.`,
  Stufen explizit im Parameter-Descriptor — das ConfigPanel zeigt alle Visualizer
  in Datenfluss-Reihenfolge). Verbindliche Tabellen: `Parameter_Key_Migration.md`
  (Entscheide E1–E8).
  - Equalizer: **`render.heightScale` NEU** (Anzeige-Skalierung der Balkenhöhen,
    E1 — wirkt konsistent auf Balken, Peaks, Partikel, Taps); `map.bands` ersetzt
    das `eq.bands`↔`audio.bands`-Paar (E2).
  - Waveform: `waveform.smoothing` **entfällt** (E3) — die Stufe-1-Glättung
    (`audio.smooth.*`) übernimmt auch die Display-Glättung.
  - Oscilloscope: Trigger → `map.trigger.*`, `render.triggerIndicator` (E4),
    Div-Raster → `render.chN/mN.*` (E5).
  - Superscope: Doppel-„Audio"-Gruppe aufgelöst (`map.audioSource/audioChannel`),
    Glow/Hold → `post.*`, `render.preset` (E6); Expressions unangetastet.
- **Preset-Migration (formatVersion 2):** `.lvp`-Presets mit formatVersion < 2
  werden beim Laden pro Visualizer über Alias-Maps übersetzt (Key → Key, einzelne
  Einträge **wert-konvertiert**: `waveform.smoothing` s → `audio.smooth.timeMs`,
  timeMs ≈ −16.67/ln(s)); gespeichert wird nur im neuen Schema. Alte User-Presets
  bleiben ladbar (inkl. `waveform.color.*`-Legacy; `scope.`-Präfix strikt pro
  Visualizer).
- **Shared-Module (Schritt 5.6):**
  - `BeatModule` (Kanten-Trigger + adaptiver Energie-Detektor) ersetzt die
    Ad-hoc-Beat-Detektoren in Pulsing und Superscope.
  - `PostFxModule`/`HoldFadeEffectT` (generische Hold/Fade-Trails für Sample- und
    Punkt-Frames) ersetzt die kopierte Frame-Fade-Logik in Waveform und Superscope.
  - `SmoothingModule::processArrayPerIndex` (per-Sample-EMA) ersetzt die
    Hand-EMAs — es gibt nur noch EINE Glättungs-Implementierung; Waveform- und
    Superscope-Display-Glättung folgen der `audio.smooth.*`-Konfiguration.
  - `AudioUtil` (splitStereoData/resampleNearest) und `JsonPresetParser`
    (ersetzt 3 Mini-Parser in AudioSource/Smoothing/ColorGradient).
  - `PipelineKeys.hpp`: Key-Präfix → Stufe/Gruppe als gemeinsame Konvention.
- **Stage-Previews im ConfigPanel (Schritt 6, Entwurf freigegeben):** Auge-Toggle
  je Stufen-Gruppe — Live-Balken (Bänder), Live-Kurve (Samples) bzw. ein
  Farbstreifen pro Gradient-Handle (folgt Kanal-Sichtbarkeit und Gradient-Edits).
  Default aus, Zustand je Visualizer+Stufe persistiert; 20-Hz-Poll läuft nur bei
  sichtbarem Preview (ausgeblendet = 0 Kosten).

## Gefixt

- **`color.domain` (jetzt `color.main.domain`) verletzte den float-für-int-Vertrag** —
  die Gradient-Domain ging beim Laden JSON-basierter Presets immer verloren.
- **CollapsibleGroupBox quetschte nachwachsenden Inhalt:** Nach dem Aufklappen
  blieb die animierte Maximalhöhe stehen — später eingeblendete Parameter
  (channelMode-Wechsel) oder Previews wurden gestaucht/abgeschnitten.
- Preview-Farbstreifen: Solid-Modus zeigte Weiß statt der Farbfläche; Streifen
  ignorierten die Kanal-Sichtbarkeit.

## Entfernt (tote Systeme)

- Oscilloscope-Phosphor komplett (nie gepusht/gerendert, keine Param-Keys —
  deckt Entscheid E7) inkl. toter Trigger-State-Member; echtes Phosphor kommt
  künftig als PostFx-Feature.
- Tote `splitStereoData`/`resampleWaveform`-Kopien (Oscilloscope, Superscope),
  `detectBeat`-Stub und ungenutzte Beat-Member (Pulsing/Superscope),
  Duplikat-Shape-Member des PulsingVisualizers (PulseShapeModule ist jetzt SSOT
  der Shape-Werte inkl. eigener paramDescs).

## Doku

- `Parameter_Reference.md` auf das neue Key-Schema (SSOT), `Parameter_Key_Migration.md`
  als Alias-Quelle; `Config_Pipeline_Concept.md` → Status **Stabil** (1.0.0);
  `ConfigPanel_Guide.md` 2.0.0 (Stufen-Gruppen, Previews, Einschränkungen aufgelöst);
  `FileFormat_Reference.md` (formatVersion 2); `Visualizer_Architecture.md`
  (Altlasten aufgelöst); NEU `Preview_Viewer_Entwurf.md` (freigegeben).

## Offen (nach Phase 4)

- Merge `phase1-buildsystem-bezug` → `master` + Push (Voraussetzung:
  CMakeCraft-Tags v0.7.0/v0.7.1 pushen).
- **Render-Thread-Entkopplung** (neues Arbeitspaket): GL-Rendering aus dem
  Main-Thread (QOpenGLWindow + createWindowContainer + Render-Thread) — behebt
  die Scroll-/Preview-FPS-Einbrüche strukturell; Vorsicht Undocking-Kontext-
  Handling. Optionaler Quickwin: Preview-Tick 20 → ~10 Hz (Entwurfsänderung,
  Freigabe ausstehend).
- Import-Phase (AVS/MilkDrop): ref/-Repos analysieren, Lua-Schicht
  (Superscope zuerst) — Leitplanken Konzept §5.6.
