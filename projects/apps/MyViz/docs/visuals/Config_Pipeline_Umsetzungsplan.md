# MyViz — Implementierungsplan Phase 4 (Config-Pipeline)

> **Version:** 1.0.0
> **Datum:** 2026-07-19
> **Typ:** Implementierungsplan
> **Status:** ✅ ABGESCHLOSSEN (40/40, Session 30 — Suite 91 Cases / 2080 Assertions grün, Sichttests + 6.3-Messung bestanden)
> **Zielgruppe:** Entwickler
> **Bezug:** [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) v0.3.0
> **Phase:** 4
> **Abhängigkeiten:** Phasen 0–3 (abgeschlossen); Konzept freigegeben ✅
> **Geschätzte Dauer:** ~5–7 Arbeitssessions (inkl. Preview-Viewer)
> **Sprache:** Deutsch
> **Methodik:** Test-begleitet (Verträge per doctest einzäunen; harvest-Tests liegen bereit)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Implementierungsschritte](#3-implementierungsschritte)
4. [Dokumentation (letzter Schritt)](#4-dokumentation-letzter-schritt)
5. [Akzeptanzkriterien](#5-akzeptanzkriterien)
6. [Nächste Schritte](#6-nächste-schritte)
7. [Changelog](#7-changelog)

---

## Übersichts-Checkliste

### Schritt 0: Tote Systeme entfernen ✅ (2026-07-19)

- [x] 0.1 EqualizerModule: totes Parameter-System + Audio-Pipeline entfernen
- [x] 0.2 ModuleConfigWidget entfernen
- [x] 0.3 ColorSchemeModule auf GradientDomain-Enum reduzieren (Enum → ColorGradientModule.hpp; dabei tote Config-Events VisualizerColorScheme/Smoothing/PeakHold/Shape + Subscriber-Maschinerie im VisualizerWidget mit entfernt)
- [x] 0.4 AudioAnalyzer entfernen (inkl. IAudioAnalyzer + AudioDataEvent)
- [x] 0.5 Visualizer-Kategorie-Strings fixen (shape/spectrum/waveform)
- [x] 0.6 Ungenutzte Felder entfernen (Pulsing: lowBand/highBand/backgroundFade; Superscope: red/green/blue)
- [x] 0.7 Build + `ctest -R UnitTests` grün (Testing-Preset)

### Schritt 1: EventBus-Upgrade + Lifetime-Fixes ✅ (2026-07-19)

- [x] 1.1 RAII-SubscriberHandle + Weak-Abos aus harvest portieren (Tests zuerst)
- [x] 1.2 ConfigPanel-Unsubscribe-Fix (Destruktor)
- [x] 1.3 Event::consume() const-fähig machen
- [x] 1.4 Transient-resolve()-Dangling fixen, Skip-Test aktivieren
- [x] 1.5 DialogManager instanziieren (Kleinfix — About-Dialog öffnet wieder)

### Schritt 2: CommandBus (Undo/Redo) ✅ (2026-07-19)

- [x] 2.1 CommandBus aus harvest portieren (Tests zuerst)
- [x] 2.2 Als Service registrieren
- [x] 2.3 SetParam-Command als erste Anwendung (Undo/Redo im ConfigPanel)

### Schritt 3: Gemeinsames Schema ✅ (2026-07-19)

- [x] 3.1 PipelineStage-Enum + ModuleParamDesc::stage
- [x] 3.2 Gradient-Handles im IVisualizer-Interface
- [x] 3.3 Key-Migrationstabellen je Visualizer festlegen
- [x] 3.4 Preset-Loader: formatVersion-Auswertung + Alias-Map
- [x] 3.5 Tap-Points: Stufen-Ausgänge als benannte, abonnierbare Daten (Preview-Fundament)
- [x] 3.6 Schema-/Migrations-Tests

### Schritt 4: ConfigPanel generisch ✅ (2026-07-19)

- [x] 4.1 Zentrale Stage-Tabelle + stage-Sortierung mit Legacy-Ziffern-Fallback (Reihenfolge unveraendert bis Migration)
- [x] 4.2 Gradient-Handles statt dynamic_cast; 5 Visualizer-Includes raus; Waveform L/R + Oscilloscope-Kanaele generisch erreichbar
- [x] 4.3 Modul-Preset-Save fuer alle (IVisualizer::audioSourceModule() + Handle-Prefixe)
- [x] 4.4 updateVisibility: nur dependsOn, transitiv (memoisiert); leere (Sub-)Gruppen automatisch aus; String-Sonderfaelle raus
- [x] 4.5 Default-Reset per Kontextmenue je Parameter/SubGruppe/Gruppe (undo-faehig via CommandBus)
- [x] 4.6 Variant-Haertung: kParamValueColorIndex + holdsColor/getColor/makeColorValue als SSOT (holds_alternative unmoeglich: Vec4f=Color4f-Alias); Reststellen -> Schritt 5

### Schritt 5: Visualizer-Migration + Modul-Konsolidierung ✅ (2026-07-19, Sichttests ✅)

- [x] 5.1 Equalizer (Referenz-Migration) — 2026-07-19; Sichttest ✅
- [x] 5.2 Pulsing (+ PulseShapeModule-paramDescs, BeatModule als erster 5.6-Baustein) — 2026-07-19; Sichttest ✅
- [x] 5.3 Waveform (+ SmoothingModule statt m_smoothing; Handles mono/left/right; HoldFadeEffect) — 2026-07-19; Sichttest ✅
- [x] 5.4 Oscilloscope (6 Handles; toter Phosphor-/Trigger-Fade-Code entfernt statt umgezogen — war nie funktional, E7) — 2026-07-19; Sichttest ✅
- [x] 5.5 Superscope (+ SmoothingModule statt Hand-EMA; Audio-Doppelgruppe aufgelöst; BeatModule adaptiv; HoldFadeEffect generisch) — 2026-07-19; Sichttest ✅
- [x] 5.6 Shared-Module: AudioUtil, PostFxModule (HoldFade), BeatModule, JSON-Preset-Helper — 2026-07-19 (Phosphor/Shader-Glow = künftige Features, keine Konsolidierung mehr offen)

### Schritt 6: Preview-Viewer je Parametergruppe ✅ (2026-07-19)

- [x] 6.1 Mini-Entwurf (`Preview_Viewer_Entwurf.md`) — 2026-07-19 von Patrik freigegeben
- [x] 6.2 Preview-Widget im ConfigPanel — 2026-07-19; Sichttest ✅ (inkl. Fixes: Gruppen-Höhensperre, Streifen-Kanal-Sichtbarkeit, Solid-Farbe)
- [x] 6.3 Performance-Check ✅ (2026-07-19, Messung Patrik): Preview aus = Basis 60 fps (N7 ✓); Live-Previews eingeblendet ~52 fps, Scroll-Spitzen ~45; Farbstreifen nach Optimierung (QLinearGradient + Change-Detection) 60 fps

### Schritt 7: Dokumentation (PFLICHT, letzter Schritt) ✅ (2026-07-19)

- [x] 7.1 Parameter_Reference auf neue Keys umstellen (v2.0.0)
- [x] 7.2 Visualizer_Architecture (v1.1.0, Altlasten-Bilanz) + ConfigPanel_Guide (v2.0.0) + FileFormat_Reference (v1.1.0, formatVersion 2)
- [x] 7.3 CppModuleDoc für neue Module (PostFx, Beat, AudioUtil, JsonPresetParser) + EventBus.md-RAII-Nachzug
- [x] 7.4 Konzept-Status auf „Stabil" (v1.0.0), Produkt-Changelog `Changelog_Session30.md`

### Fortschritt

| Schritt | Beschreibung | Aufgaben | Erledigt | Status |
|---------|--------------|----------|----------|--------|
| 0 | Tote Systeme entfernen | 7 | 7 | ✅ |
| 1 | EventBus + Lifetime | 5 | 5 | ✅ |
| 2 | CommandBus | 3 | 3 | ✅ |
| 3 | Schema | 6 | 6 | ✅ |
| 4 | ConfigPanel | 6 | 6 | ✅ |
| 5 | Migration + Module | 6 | 6 | ✅ |
| 6 | Preview-Viewer | 3 | 3 | ✅ |
| 7 | Dokumentation | 4 | 4 | ✅ |
| **Σ** | **Gesamt** | **40** | **40** | **100 %** |

---

## 1. Übersicht

### 1.1 Phasenziel

Alle 5 Visualizer konfigurieren sich über **eine gemeinsame Pipeline** (6 Stufen,
UI folgt dem Datenfluss), gleiche Config-Elemente verhalten sich überall gleich,
bestehende User-Presets bleiben per Migration nutzbar. Grundlagen (Warum/Architektur):
[Config_Pipeline_Concept.md](Config_Pipeline_Concept.md).

### 1.2 Lieferumfang

| Komponente | Beschreibung | Priorität |
|---|---|---|
| PipelineStage-Schema | Enum + `ModuleParamDesc::stage`, Key-Konvention | P1 |
| EventBus-RAII + CommandBus | Lifetime-Sicherheit, Undo/Redo | P1 |
| Generisches ConfigPanel | Stage-Gruppen, Handles, ohne dynamic_cast | P1 |
| Preset-Migration | formatVersion + Alias-Map | P1 |
| 5 migrierte Visualizer | Equalizer zuerst | P1 |
| Shared-Module | AudioUtil, PostFxModule, BeatModule | P2 |
| UX-Extras | Default je Ebene; Gruppen-Presets überall | P2 |
| Preview je Gruppe | Tap-Points + Preview-Widget (fest beauftragt 2026-07-19) | P2 |

### 1.3 Arbeitsweise

> **Verträge zuerst einzäunen:** Vor jedem Umbau-Schritt sichern Tests das
> Soll-Verhalten (bestehende Suite: 36 Cases; harvest/tests liefert CommandBus-/
> EventBus-Vorlagen). Nach jedem Schritt: `ctest -R UnitTests` + VS-Build grün.

---

## 2. Voraussetzungen

| Voraussetzung | Status |
|---|---|
| Phasen 0–3 (Repos, Bootstrap, Doku-Reorg, Test-Fundament 36 Cases) | ✅ |
| Konzept-Freigabe durch Patrik (§4.3 Keys, §5.3 AudioSource, §5.6 Leitplanken) | ✅ 2026-07-19 |
| `harvest/core-module/` (EventBus-/CommandBus-Vorlagen + Guides) | ✅ vorhanden |
| `harvest/tests/` (CommandBus-/BaseTypes-Tests, Catch2→doctest zu portieren) | ✅ vorhanden |
| Session-Beilage Code-Analyse (Datei:Zeile-Anker) | ✅ `.claude/sessions/LumiViz_Session29_beilage_code-analyse.md` (lokal) |

**Merkregeln:** Source-Listen sind explizit (`sources.mode: explicit`) — entfernte/neue
Dateien in der jeweiligen `Source.cmake` pflegen. Preset-JSONs kommentarfrei.

---

## 3. Implementierungsschritte

### Schritt 0: Tote Systeme entfernen

**Ziel:** Niemand refaktoriert gegen tote Pfade; Diff der echten Migration bleibt klein.

**Erwartetes Ergebnis:** Build + Suite grün, App verhält sich unverändert.

#### 0.1 EqualizerModule entkernen

- [x] Totes `paramDescs/getParam/setParam` entfernen (EqualizerModule.cpp, „Audio"-Params :95–144 — kein Aufrufer im Repo)
- [x] Tote Audio-Pipeline entfernen: `processSpectrum()` (EqualizerModule.cpp:544), `mapSpectrum/applyEMA/normalizeDb` + Felder `m_emaAlpha/m_floorDb/m_ceilDb/m_frequencyScale` (EqualizerModule.hpp:305–331)
- [x] Verifizieren: `updateFromProcessed()`-Pfad (EqualizerVisualizer.cpp:1021,1024) bleibt unberührt

#### 0.2 ModuleConfigWidget entfernen

- [x] `src/UI/widgets/ModuleConfigWidget.cpp` (+ Header) löschen, `Source.cmake` pflegen

#### 0.3 ColorSchemeModule reduzieren

- [x] `GradientDomain`-Enum an sinnvollen Ort verschieben (einziger genutzter Teil)
- [x] Rest des Alt-Farbsystems entfernen

#### 0.4 AudioAnalyzer entfernen (Entscheid 2026-07-19)

- [x] `src/audio/AudioAnalyzer.cpp` + Header + `IAudioAnalyzer` entfernen (kein Aufrufer;
      realer Datenweg: `MainWindow::onAudioUpdate`-QTimer), `Source.cmake` pflegen
- [x] Hinweis in [Audio_System.md](../audio/Audio_System.md) §7.3 anpassen
      (Neuentwurf der Verteilstelle kommt mit der Import-/Audio-Phase)

#### 0.5 Visualizer-Kategorie-Strings fixen (Entscheid 2026-07-19)

- [x] `VisualizerAutoReg.cpp` / `initVisualizerDefaults()`: „static Parametring …"-Strings
      durch sinnvolle Kategorien ersetzen (z. B. spectrum/waveform/shape); Wirkung im
      VisualSelectPanel prüfen; [Registries.md](../core-services/Registries.md) nachziehen

#### 0.6 Kleinkram

- [x] Ungenutzte Felder PulsingVisualizer/SuperscopeModule entfernen (Handover-Liste)
- [x] `detectBeat`-Stub Pulsing (PulsingVisualizer.cpp:1252) markieren → Schritt 5.6 BeatModule

#### 0.7 Verifikation

- [x] VS- und CLI-Build grün, `ctest -R UnitTests` SUCCESS

---

### Schritt 1: EventBus-Upgrade + Lifetime-Fixes

**Ziel:** Kein Panel kann mehr mit hängendem Abo sterben — Voraussetzung für jeden UI-Umbau.

#### 1.1 RAII-Abos portieren

- [x] `harvest/core-module/eventbus/` sichten (SubscriberHandle, Weak-Abos)
- [x] Tests zuerst: harvest-EventBus-Tests auf doctest portieren/ergänzen (RED)
- [x] `SubscriberHandle` (RAII) + Weak-Abo-Support in `services/EventBus` (GREEN)
- [x] Bestehende Abonnenten (Panels, DockManager, MainWindow) auf Handles umstellen

#### 1.2 ConfigPanel-Lifetime-Fix

- [x] Unsubscribe im Destruktor sicherstellen (heute: nur `onDeactivate`, `~ConfigPanel() = default`, ConfigPanel.cpp:110; subscribe :293–313) — mit RAII-Handle automatisch
- [x] Regression-Test: Panel zerstören ohne hide → kein dangling Abo

#### 1.3 Event-Verträge

- [x] `Event::consume()` const-fähig machen (const_cast im Test entfernen)
- [x] `VisualizerChangedEvent::visualizerPtr` (void*, :305–308): mindestens typisieren, Lebensdauer-Kontrakt dokumentieren

#### 1.4 ServiceContainer

- [x] Transient-resolve()-Dangling-Bug fixen, Skip-Test aktivieren (dokumentierter Skip der Suite)

#### 1.5 DialogManager instanziieren (Kleinfix, Entscheid 2026-07-19)

- [x] DialogManager im Bootstrap erzeugen/registrieren; `OpenDialogEvent`-Handler in
      MainWindow ersetzt Log-Stub → About-Dialog öffnet wieder
- [x] [Bootstrap_Integration.md](../core-services/Bootstrap_Integration.md) +
      [Event_System.md](../core-services/Event_System.md) nachziehen (Schritt 7)

---

### Schritt 2: CommandBus (Undo/Redo)

**Ziel:** Parameter-Änderungen und Gradient-Edits sind undo-fähig, bevor die UI neu entsteht.

- [x] 2.1 `harvest/core-module/commandbus/` portieren; harvest-Tests (Catch2) auf doctest umziehen (RED→GREEN)
- [x] 2.2 CommandBus im ServiceContainer registrieren (Bootstrap_Integration.md nachziehen → Schritt 6)
- [x] 2.3 `SetParamCommand` (Visualizer, paramId, alt→neu) + Undo/Redo-Aktionen ins ConfigPanel/Menü; Gradient-Editor-Änderungen als Commands

---

### Schritt 3: Gemeinsames Schema

**Ziel:** Pipeline-Stufe und Farb-Zugriff sind first-class — maschinenlesbar statt String-Konvention.

#### 3.1 PipelineStage

- [x] Enum `PipelineStage` (AudioSource=1 … Post=6) in IModule.hpp
- [x] `ModuleParamDesc::stage` + ParamBuilder-Setter; `group`-Präfixe „1.–8." obsolet
- [x] Übergangs-Mapping: solange ein Visualizer unmigriert ist, leitet ConfigPanel stage aus dem alten group-Präfix ab (Kompatibilitätsschicht, fliegt nach Schritt 5)

#### 3.2 Gradient-Handles

- [x] `IVisualizer::gradients()` → Liste (handleId/paramPrefix, ColorGradientModule*)
- [x] Default-Implementierung leer; Visualizer liefern ihre Handles (Waveform: mono/left/right!)

#### 3.3 Key-Migrationstabellen

- [x] Je Visualizer Tabelle „alter Key → neuer Key" nach Konzept §4.3 (`map.` / `color.<handle>.` / `render.` / `peak.` / `particle.` / `post.`)
- [x] Review durch Patrik (Namens-Feinschliff) **vor** Schritt 5

#### 3.4 Preset-Migration

- [x] `formatVersion` beim Laden auswerten (heute nur geschrieben — VisualizerPresetManager)
- [x] Alias-Map-Mechanik: Loader übersetzt alte IDs; Speichern nur neues Schema
- [x] Puffer-Resize-Sonderfälle an neue IDs koppeln (eq.bands→map.bands: EqualizerVisualizer.cpp:1015–1018; sampleCount: WaveformVisualizer.cpp:370–379, OscilloscopeVisualizer.cpp:312–320)

#### 3.5 Tap-Points (Preview-Fundament, Entscheid 2026-07-19)

- [x] Stufen-Ausgänge als benannte, abonnierbare Daten definieren (z. B.
      `tap.audio` = normalisierte Bänder, `tap.map` = gemappte Daten, `tap.color` =
      Farbwerte je Element) — leichtgewichtig, nur bei aktivem Abonnenten befüllt
- [x] In AudioSourceModule + je einem Pilot-Visualizer (Equalizer) verdrahten;
      Rest folgt mit der Migration (Schritt 5)

#### 3.6 Tests

- [x] Schema-Tests (stage gesetzt, Builder), Handle-Tests, Tap-Point-Tests
- [x] Migrations-Roundtrip: altes .lvp (Fixture) → laden → neue Keys korrekt; float-für-int-Vertrag bleibt (bestehender Test)

---

### Schritt 4: ConfigPanel generisch

**Ziel:** ConfigPanel kennt keine konkreten Visualizer mehr; Reihenfolge/Verhalten kommen aus dem Schema.

- [x] 4.1 Zentrale Stage-Tabelle (Titel, Icon, Reihenfolge) ersetzt Emoji-Keyword-Heuristik (ConfigPanel.cpp:58–72) und String-Sortierung (:371–382)
- [x] 4.2 `openGradientEditor` (:1269–1380) + Preview-Delegate (:748–802) auf Handles umstellen; die fünf Visualizer-`#include`s (:18–22) entfernen
- [x] 4.3 `onModulePresetSave` (:1621–1701, Early-Return :1628) + `updateRelatedPresetWidget` (:1008–1067) generisch über Handles/Präfixe — Save-Button funktioniert bei allen Visualizern
- [x] 4.4 `updateVisibility` (:1069–1264): nur noch dependsOn/dependsValues, Sonderfälle („Line Color"-Substring :1139, channelMode-Hardcode) entfernen
- [x] 4.5 Default-Reset je Ebene: Parameter (Kontext/Button), Untergruppe, Stufe, Visualizer (bestehender Default-Eintrag) — als Commands (Undo-fähig)
- [x] 4.6 Variant-Zugriffe härten: `std::holds_alternative<Color4f>` statt `value.index()==7` (:275, :947)

**Erwartetes Ergebnis:** Pulsing (unmigriert, via Kompatibilitätsschicht) und Equalizer
(nach 5.1) rendern beide korrekt in Stage-Reihenfolge.

---

### Schritt 5: Visualizer-Migration + Modul-Konsolidierung

**Ziel:** Jeder Visualizer einzeln auf Schema/Stufen; dabei Duplikate in Shared-Module überführen. Reihenfolge fix; nach jedem Teilschritt Suite + manueller Sichttest.

#### 5.1 Equalizer (Referenz) ✅ (2026-07-19, Sichttest ✅)

- [x] Gruppen „1.–8." → Stufen 1–5 (Post fehlt zulässig); Keys nach Migrationstabelle;
      `render.heightScale` NEU implementiert (E1, wirkt in `updateFromProcessed`);
      `CURRENT_FORMAT_VERSION` = 2; Alias-Mechanik vorab um **Wert-Konverter** erweitert
      (`registerKeyConverter`/`translateLegacyParam`, E3-Formel getestet)
- [x] `eq.bands`↔`audio.bands`-Sync auflösen → ein Key `map.bands` (treibt beide Module;
      Alias: beide Alt-Keys → `map.bands`, `eq.bands` gewinnt per Map-Reihenfolge)
- [x] Alias-Map Equalizer (36 Einträge inkl. Identitäts-Whitelist) + Roundtrip-Tests
      (`test_EqualizerMigration.cpp`, 6 Cases; dabei float-für-int-Vertrag von
      `color.main.domain` gefixt) — Sichttest gegen `harvest/config-pipeline/`-Blaupause
      ✅ (2026-07-19)

#### 5.2 Pulsing ✅ (2026-07-19, Sichttest ✅)

- [x] PulseShapeModule bekommt eigene paramDescs/getParam/setParam (type/sides/innerRadius/
      minSize/maxSize/rotation; UI-Enum-Mapping wandert ins Modul; Modul ist jetzt SSOT der
      Shape-Werte — Visualizer-Duplikat-Member entfernt; float-für-int-Vertrag eingebaut)
- [x] `shape.color.*` → `color.main.*` (beatBrightness-Sonder-Routing beibehalten);
      Shape → `render.*`; Beat-Detection → **BeatModule** (neu,
      `modules/processing/BeatModule.hpp` — erster 5.6-Baustein; tote Stubs
      detectBeat/renderPulse entfernt). Alias-Map (28 Einträge) + Tests
      (`test_PulsingMigration.cpp`, 5 Cases) — Sichttest ✅ (2026-07-19)

#### 5.3 Waveform ✅ (2026-07-19, Sichttest ✅)

- [x] `m_smoothing`-Eigenglättung → SmoothingModule: Param entfällt (E3), Display-Glättung
      läuft über 3 SmoothingModule-Instanzen (Config gespiegelt aus `audio.smooth.*`;
      neues `processArrayPerIndex` mit unabhängigem per-Index-Zustand); Wert-Konverter
      `waveform.smoothing` → `audio.smooth.timeMs` registriert
- [x] 3 Farb-Handles (mono/left/right) auf `color.<handle>.*` — Editor erreicht alle;
      Keys kanal-strukturiert (E8: `render.mono.offset` usw., Übersetzungstabelle
      Sub-ID ↔ Pipeline-Key ist zugleich Quelle der Alias-Map)
- [x] SubGroup „Effects" → `post.*` (`post.mirror.enabled`, `post.hold.*`);
      Hold/Fade-Mechanik → **HoldFadeEffect** (neu, `modules/postfx/PostFxModule.hpp`
      — zweiter 5.6-Baustein); Alias-Map (77 Einträge inkl. `waveform.color.*`-Legacy
      §7.4) + Tests (`test_WaveformMigration.cpp`, 6 Cases) — Sichttest ✅ (2026-07-19)

#### 5.4 Oscilloscope ✅ (2026-07-19, Sichttest ✅)

- [x] 6 Farb-Handles (ch1–ch4, m1, m2) auf `color.<handle>.*` — Editor erreicht alle;
      Übersetzungstabelle Sub-ID ↔ Pipeline-Key (E5: voltsPerDiv/offset → `render.chN/mN.*`,
      Datenwahl → `map.*`; E4: `render.triggerIndicator`)
- [x] Timebase/Trigger → `map.*` (`map.timePerDiv`, `map.trigger.*`), Grid/Display →
      `render.*`, `post.trigger.fadeTime` (Key migriert; Wirkung kommt mit PostFxModule).
      **Befund:** Phosphor-Inline war komplett tot (nie gepusht/gerendert, keine Param-Keys
      — deckt E7) → entfernt statt umgezogen (PhosphorFrame, m_phosphorBuffers,
      updatePhosphorFrames, config-Felder, tote Trigger-State-Member); echtes Phosphor
      folgt als PostFxModule-Feature (5.6). Alias-Map (119 Einträge) + Tests
      (`test_OscilloscopeMigration.cpp`, 4 Cases) — Sichttest ✅ (2026-07-19)

#### 5.5 Superscope ✅ (2026-07-19, Sichttest ✅)

- [x] Hand-EMA → SmoothingModule (`processArrayPerIndex`, 4 Instanzen für Waveform/Spektrum
      L/R; Config gespiegelt aus `audio.smooth.*`); Ad-hoc-Beat → **BeatModule** (neuer
      adaptiver Energie-Modus `updateAdaptive`; tote `m_beatEnergy` entfernt)
- [x] Doppelte „Audio"-Semantik aufgelöst: `scope.audioSource/audioChannel` → `map.*` (Stufe 2)
- [x] Glow/Hold → `post.glow.*`/`post.hold.*`; Hold-Frames über **HoldFadeEffectT**
      (Template — trägt jetzt auch Punktlisten); `render.preset` (E6), `render.mode`;
      Expressions unangetastet. Alias-Map (38 Einträge, strikt getrennt vom
      Oscilloscope-`scope.`-Präfix §7.5 — eigener Test) + Tests
      (`test_SuperscopeMigration.cpp`, 5 Cases) — Sichttest ✅ (2026-07-19)

#### 5.6 Shared-Module ✅ (2026-07-19 — alle existierenden Duplikate konsolidiert)

- [x] AudioUtil (`modules/AudioUtil.hpp`): `splitStereoData` + `resampleNearest` — aktiver
      Nutzer Waveform. **Befund:** Die Kopien in Oscilloscope (split + resample) und
      Superscope (split) waren tot (nie aufgerufen) → entfernt; das Oscilloscope-eigene
      Inline-Display-Resampling (linear + clamp) ist bewusst eigenständig geblieben
- [x] PostFxModule: HoldFadeEffectT (generisch, Sample-/Punkt-Frames) — Waveform +
      Superscope angeschlossen; Oscilloscope-Phosphor war tot und ist entfernt (E7).
      Echtes Phosphor/Shader-Glow = künftige Features (Konzept §5.6 Leitplanke 1),
      keine Duplikat-Konsolidierung mehr offen
- [x] BeatModule: Kanten-Trigger (Pulsing) + adaptiver Energie-Modus (Superscope)
      angeschlossen; Equalizer GradientDomain::Beat hat keinen Detektor (Enum-Fall
      fällt auf Position zurück) — nichts zu konsolidieren, Detektor wäre neues Feature
- [x] JSON-Preset-Helper (`modules/JsonPresetParser.hpp`): ersetzt die 3 Mini-Parser in
      AudioSourceModule, SmoothingModule und ColorGradientModule (Skalare + Arrays via
      Klammerzählung; Semantik 1:1 erhalten; direkte Tests in `test_PipelineSchema.cpp`)

---

### Schritt 6: Preview-Viewer je Parametergruppe

**Ziel:** Je Pipeline-Stufe/Untergruppe ist eine Live-Vorschau der Roh-/Zwischendaten
einblendbar (Pflichtenheft-Idee, fest beauftragt 2026-07-19).

- [x] 6.1 Mini-Entwurf [Preview_Viewer_Entwurf.md](Preview_Viewer_Entwurf.md) ✅
      freigegeben 2026-07-19: Balken/Kurve je Tap (vom Visualizer via
      `TapPoint::display` deklariert), Farbstreifen je Gradient-Handle (Stufe 3),
      Auge-Toggle im Gruppen-Header, 20 Hz, Default aus
- [x] 6.2 Preview-Widget ✅ (2026-07-19, Sichttest offen): `TapPreviewWidget`
      (QPainter, 3 Modi) + `CollapsibleGroupBox::addHeaderWidget`; Taps in allen
      5 Visualizern (Stufe 1 Bänder überall; Stufe 2 Kurve bei Waveform/Oscilloscope/
      Superscope, Balken beim Equalizer); gemeinsamer 20-Hz-Timer läuft NUR bei
      sichtbarem Preview; Zustand je Visualizer+Stufe in QSettings
      (`configpanel/preview/<viz>/<stage>`), Default aus. **Dabei Bestandsbug
      gefixt:** CollapsibleGroupBox ließ nach der Expand-Animation
      `maximumHeight` eingefroren — später wachsender Inhalt (Previews,
      dependsOn-Einblendungen) wurde gequetscht/abgeschnitten; Höhe wird jetzt
      nach dem Aufklappen wieder freigegeben
- [x] 6.3 Performance-Check ✅ (2026-07-19, Messwerte Patrik, FPS-Anzeige):
      Basis ~60 fps · Panel offen, Previews aus = ~60 (**N7 erfüllt** — konstruktiv:
      ohne Abonnent kein Timer/`sample()`) · Live-Previews (Balken/Kurve) an ~52,
      beim Scrollen kurz ~45 · Farbstreifen 60 (Scrollen ~50) nach Optimierung
      (QLinearGradient-Fill, Repaint nur bei Gradient-Änderung, Kurven-Dezimierung).
      Restkosten eingeblendeter Live-Previews + Scroll-Repaints = Main-Thread-Teilung
      Qt↔GL — Entkopplung (Render-Thread) als Post-Phase-4-Kandidat notiert

---

## 4. Dokumentation (letzter Schritt)

**Ziel:** Doku beschreibt den neuen Ist-Stand; Phase 4 gilt erst danach als abgeschlossen.

- [x] 7.1 [Parameter_Reference.md](Parameter_Reference.md) v2.0.0: alle Key-Tabellen auf
      die Stufen-Keys (gegen die paramDescs im Code verifiziert), `render.heightScale`
      ergänzt, `waveform.smoothing` raus (E3-Konverter dokumentiert), §9 Alias-Verweise
- [x] 7.2 [Visualizer_Architecture.md](Visualizer_Architecture.md) v1.1.0 (Altlasten →
      §11 Bilanz; Ist-Stand Stage-Schema/Handles/Taps/Shared-Module),
      [../ui/ConfigPanel_Guide.md](../ui/ConfigPanel_Guide.md) v2.0.0 (Stufen-Gruppen,
      Previews, Einschränkungen aufgelöst), [../presets/FileFormat_Reference.md](../presets/FileFormat_Reference.md)
      v1.1.0 (formatVersion 2 + Migrations-Semantik)
- [x] 7.3 CppModuleDoc: BeatModule.md, PostFxModule.md, AudioUtil.md, JsonPresetParser.md
      (neben den Headern); EventBus.md §9 RAII-Abos (SubscriberHandle)
- [x] 7.4 [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) → **Stabil** (v1.0.0);
      Produkt-Changelog [../sessions/Changelog_Session30.md](../sessions/Changelog_Session30.md);
      INDEX.md nachgezogen

---

## 5. Akzeptanzkriterien

> ⚠ **Nachtrag Session 52:** Die Haken unten stehen sämtlich auf `⬜`, obwohl die
> Schritte 0–7 seit Session 30 als ✅ gelten und die Sichttests 5.1–5.5 von Patrik
> abgenommen sind (§0.2.7 im Changelog). Die Tabelle wurde beim Abschluss schlicht
> nicht nachgezogen — sie ist **kein** Hinweis auf offene Arbeit. Aufräumen
> (nachträglich abhaken oder streichen) steht in
> [Offene_Punkte.md](../Offene_Punkte.md) §5.

### 5.1 Funktionale Anforderungen

| # | Kriterium | Testmethode | Status |
|---|---|---|---|
| A1 | Config-UI aller 5 Visualizer in Stufen-Reihenfolge 1→6 | Sichttest je Visualizer | ⬜ |
| A2 | Gradient-Editor/-Preview funktioniert für **jedes** Farb-Handle (inkl. Waveform L/R, Oscilloscope ch1–m2) | Sichttest | ⬜ |
| A3 | Modul-Preset-Save funktioniert bei allen Visualizern | Sichttest + Test | ⬜ |
| A4 | Alte `.lvp`-Presets laden korrekt (Alias-Map), float-für-int-Vertrag hält | Unit-Tests (Fixtures) | ⬜ |
| A5 | Parameter-Änderungen + Gradient-Edits sind undo-/redo-fähig | Sichttest + CommandBus-Tests | ⬜ |
| A6 | Default-Reset je Ebene verfügbar | Sichttest | ⬜ |
| A7 | Preview je Stufe/Untergruppe einblendbar (Tap-Points) | Sichttest | ⬜ |
| A8 | About-Dialog öffnet (DialogManager instanziiert) | Sichttest | ⬜ |

### 5.2 Nicht-funktionale Anforderungen

| # | Kriterium | Testmethode | Status |
|---|---|---|---|
| N1 | ConfigPanel ohne `#include`/`dynamic_cast` konkreter Visualizer | Code-Review/grep | ⬜ |
| N2 | Keine String-Heuristiken für Reihenfolge/Sichtbarkeit/Preview | Code-Review | ⬜ |
| N3 | Genau EINE Glättungs-, Beat-, Hold/Fade-, Split-Implementierung | Code-Review/grep | ⬜ |
| N4 | Panel-Zerstörung ohne hide → kein dangling EventBus-Abo | Regression-Test | ⬜ |
| N5 | Suite grün (VS + CLI, `ctest -R UnitTests`), 0 Skips (Transient-Fix) | CI/lokal | ⬜ |
| N6 | Leitplanken §5.6 eingehalten (Wertquelle nicht verschweißt, Effekte instanzfähig) | Review gegen Konzept | ⬜ |
| N7 | Ausgeblendetes Preview kostet keine messbare Frametime (Tap nur bei Abonnent) | Messung | ⬜ |

---

## 6. Nächste Schritte

### 6.1 Nach Phase 4

**Import-Phase (AVS/MilkDrop)** — Vorbereitung: Analyse der Referenz-Repos in `../ref/`
(vis_avs, projectm, MilkDrop3, milkdrop2077, winamp_orig): Preset-Formate, EEL-Umfang
vs. Lua, projectM-Renderarchitektur. Danach: Lua-Binding (Superscope zuerst),
Expression-Umschaltung pro Parameter, Multieffekt-Host.

### 6.2 Offene Entscheidungen

- [x] Key-Migrationstabellen-Review durch Patrik (3.3) — ✅ 2026-07-19, F1-F8 entschieden (Details: Parameter_Key_Migration.md); NEU fuer Schritt 5: Alias-Mechanik braucht optionale Wert-Konverter (F3)
- [x] Preview-Mini-Entwurf absegnen (6.1) — ✅ 2026-07-19 freigegeben (Preview_Viewer_Entwurf.md)

Alle übrigen Entscheidungen sind gefallen (2026-07-19, siehe Konzept §8):
Keys+Alias ✓ · AudioSource pro Visual ✓ · Leitplanken ✓ · Analyzer entfernen (0.4) ✓ ·
DialogManager-Kleinfix (1.5) ✓ · Kategorie-Strings fixen (0.5) ✓ · Preview fest (Schritt 6) ✓ ·
en-Doku später wie CMakeCraft ✓

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-07-19** | **PHASE 4 ABGESCHLOSSEN (40/40): 6.3-Messung bestanden (N7 ✓ — Preview aus = Basis 60 fps; Farbstreifen nach QLinearGradient-Optimierung gratis); Render-Thread-Entkopplung als Post-Phase-4-Kandidat notiert** |
| 0.3.0 | 2026-07-19 | Schritt 7 komplett (Session 30): Parameter_Reference v2.0.0, Visualizer_Architecture v1.1.0, ConfigPanel_Guide v2.0.0, FileFormat_Reference v1.1.0, 4 neue CppModuleDocs + EventBus-RAII, Konzept → Stabil v1.0.0, Changelog_Session30; 6.2-Sichttest ✅ inkl. 3 Preview-Fixes; 39/40 (98 %) — offen nur 6.3-Messung |
| 0.2.8 | 2026-07-19 | 6.1 freigegeben + 6.2 umgesetzt (Session 30): TapPreviewWidget (Balken/Kurve/Farbstreifen), TapPoint::display, Taps in allen 5 Visualizern, Auge-Toggle (CollapsibleGroupBox::addHeaderWidget), 20-Hz-Timer nur bei sichtbarem Preview, QSettings-Persistenz; 35/40 (88 %); 6.2-Sichttest + 6.3-Messung offen |
| 0.2.7 | 2026-07-19 | Sichttests 5.1–5.5 bestanden (Patrik, VS-Lauf) — Schritt 5 vollständig abgenommen |
| 0.2.6 | 2026-07-19 | 5.6 abgeschlossen → Schritt 5 komplett (Session 30): AudioUtil (tote Kopien in Oscilloscope/Superscope entfernt), JsonPresetParser ersetzt 3 Mini-Parser; 33/40 (83 %); Sichttests 5.1–5.5 offen |
| 0.2.5 | 2026-07-19 | 5.5 Superscope migriert (Session 30) — alle 5 Visualizer auf Pipeline-Schema: Hand-EMA → SmoothingModule, Beat → BeatModule (adaptiver Modus), Hold → HoldFadeEffectT (generisch), Audio-Doppelgruppe → `map.*` (E6 `render.preset`); Alias-Registrierung je Konstruktion (Magic-Static überlebte clearKeyAliases nicht); 32/40 (80 %); Sichttests 5.1–5.5 offen |
| 0.2.4 | 2026-07-19 | 5.4 Oscilloscope migriert (Session 30): Trigger → `map.trigger.*` (E4: `render.triggerIndicator`), voltsPerDiv/offset → `render.chN/mN.*` (E5), 6 Farb-Handles `color.chN/mN.*`, `post.trigger.fadeTime`; toter Phosphor-Code entfernt (E7-Befund); `PipelineKeys.hpp` (stageForKey/groupForStage als SSOT); 31/40 (78 %); Sichttests 5.1–5.4 offen |
| 0.2.3 | 2026-07-19 | 5.3 Waveform migriert (Session 30): Übersetzungstabelle Sub-ID↔Pipeline-Key (kanal-strukturiert E8), `waveform.smoothing` entfällt (E3-Konverter aktiv, Display-Glättung via SmoothingModule `processArrayPerIndex`), HoldFadeEffect extrahiert (PostFxModule-Baustein), `waveform.color.*`-Legacy-Alias; 30/40 (75 %); Sichttests 5.1–5.3 offen |
| 0.2.2 | 2026-07-19 | 5.2 Pulsing migriert (Session 30): PulseShapeModule-paramDescs (Modul = SSOT der Shape-Werte), BeatModule extrahiert (erster 5.6-Baustein), `shape.*`→`render.*`, `shape.color.*`→`color.main.*`, Alias-Map + Tests; 29/40 (73 %); Sichttests 5.1/5.2 offen |
| 0.2.1 | 2026-07-19 | 5.1 Equalizer migriert (Session 30): Wert-Konverter in Alias-Mechanik (E3), Alias-Map + Roundtrip-Tests, `render.heightScale` (E1), `map.bands` (E2), formatVersion 2; Checkboxen 2/3 nachgezogen; 28/40 (70 %); Sichttest 5.1 offen |
| 0.2.0 | 2026-07-19 | Freigabe eingearbeitet: +0.4 Analyzer-Entfernung, +0.5 Kategorie-Fix, +1.5 DialogManager, +3.5 Tap-Points, Schritt 6 = Preview-Viewer (fest), Doku → Schritt 7 (40 Aufgaben) |
| 0.1.0 | 2026-07-19 | Initial: Schritte 0–6 mit Datei:Zeile-Ankern aus der Session-29-Analyse |
