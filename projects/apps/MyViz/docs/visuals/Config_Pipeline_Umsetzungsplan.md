# MyViz — Implementierungsplan Phase 4 (Config-Pipeline)

> **Version:** 0.2.0
> **Datum:** 2026-07-19
> **Typ:** Implementierungsplan
> **Status:** In Umsetzung (Schritte 0–2 ✅ 2026-07-19; Schritt 3 anstehend)
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

### Schritt 2: CommandBus (Undo/Redo) ⬜

- [ ] 2.1 CommandBus aus harvest portieren (Tests zuerst)
- [ ] 2.2 Als Service registrieren
- [ ] 2.3 SetParam-Command als erste Anwendung (Undo/Redo im ConfigPanel)

### Schritt 3: Gemeinsames Schema ⬜

- [ ] 3.1 PipelineStage-Enum + ModuleParamDesc::stage
- [ ] 3.2 Gradient-Handles im IVisualizer-Interface
- [ ] 3.3 Key-Migrationstabellen je Visualizer festlegen
- [ ] 3.4 Preset-Loader: formatVersion-Auswertung + Alias-Map
- [ ] 3.5 Tap-Points: Stufen-Ausgänge als benannte, abonnierbare Daten (Preview-Fundament)
- [ ] 3.6 Schema-/Migrations-Tests

### Schritt 4: ConfigPanel generisch ⬜

- [ ] 4.1 Zentrale Stage-Tabelle (Titel/Icons), Sortierung nach stage
- [ ] 4.2 dynamic_cast-Kaskaden durch Gradient-Handles ersetzen
- [ ] 4.3 Modul-Preset-Save für ALLE Visualizer
- [ ] 4.4 updateVisibility ohne String-Sonderfälle
- [ ] 4.5 Default-Reset je Ebene (Parameter/Untergruppe/Stufe/Visualizer)
- [ ] 4.6 Variant-Zugriffe härten (holds_alternative statt index()==7)

### Schritt 5: Visualizer-Migration + Modul-Konsolidierung ⬜

- [ ] 5.1 Equalizer (Referenz-Migration)
- [ ] 5.2 Pulsing (+ PulseShapeModule-paramDescs)
- [ ] 5.3 Waveform (+ SmoothingModule statt m_smoothing; Handles mono/left/right)
- [ ] 5.4 Oscilloscope (+ PostFx statt Phosphor-Inline; 6 Handles)
- [ ] 5.5 Superscope (+ SmoothingModule statt Hand-EMA; Audio-Doppelgruppe auflösen)
- [ ] 5.6 Shared-Module: AudioUtil, PostFxModule, BeatModule, JSON-Preset-Helper

### Schritt 6: Preview-Viewer je Parametergruppe ⬜

- [ ] 6.1 Mini-Entwurf: Tap-Point-Abgriff, Darstellung je Stufe, Abschaltbarkeit
- [ ] 6.2 Preview-Widget im ConfigPanel (je Stufe/Untergruppe einblendbar)
- [ ] 6.3 Performance-Check (Frametime mit/ohne Preview) + Review gegen Pflichtenheft

### Schritt 7: Dokumentation (PFLICHT, letzter Schritt) ⬜

- [ ] 7.1 Parameter_Reference auf neue Keys umstellen
- [ ] 7.2 Visualizer_Architecture + ConfigPanel_Guide + FileFormat_Reference nachziehen
- [ ] 7.3 CppModuleDoc für neue Module (PostFx, Beat, AudioUtil)
- [ ] 7.4 Konzept-Status auf „Stabil", Produkt-Changelog schreiben

### Fortschritt

| Schritt | Beschreibung | Aufgaben | Erledigt | Status |
|---------|--------------|----------|----------|--------|
| 0 | Tote Systeme entfernen | 7 | 7 | ✅ |
| 1 | EventBus + Lifetime | 5 | 5 | ✅ |
| 2 | CommandBus | 3 | 3 | ✅ |
| 3 | Schema | 6 | 0 | ⬜ |
| 4 | ConfigPanel | 6 | 0 | ⬜ |
| 5 | Migration + Module | 6 | 0 | ⬜ |
| 6 | Preview-Viewer | 3 | 0 | ⬜ |
| 7 | Dokumentation | 4 | 0 | ⬜ |
| **Σ** | **Gesamt** | **40** | **15** | **38 %** |

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

- [ ] Enum `PipelineStage` (AudioSource=1 … Post=6) in IModule.hpp
- [ ] `ModuleParamDesc::stage` + ParamBuilder-Setter; `group`-Präfixe „1.–8." obsolet
- [ ] Übergangs-Mapping: solange ein Visualizer unmigriert ist, leitet ConfigPanel stage aus dem alten group-Präfix ab (Kompatibilitätsschicht, fliegt nach Schritt 5)

#### 3.2 Gradient-Handles

- [ ] `IVisualizer::gradients()` → Liste (handleId/paramPrefix, ColorGradientModule*)
- [ ] Default-Implementierung leer; Visualizer liefern ihre Handles (Waveform: mono/left/right!)

#### 3.3 Key-Migrationstabellen

- [ ] Je Visualizer Tabelle „alter Key → neuer Key" nach Konzept §4.3 (`map.` / `color.<handle>.` / `render.` / `peak.` / `particle.` / `post.`)
- [ ] Review durch Patrik (Namens-Feinschliff) **vor** Schritt 5

#### 3.4 Preset-Migration

- [ ] `formatVersion` beim Laden auswerten (heute nur geschrieben — VisualizerPresetManager)
- [ ] Alias-Map-Mechanik: Loader übersetzt alte IDs; Speichern nur neues Schema
- [ ] Puffer-Resize-Sonderfälle an neue IDs koppeln (eq.bands→map.bands: EqualizerVisualizer.cpp:1015–1018; sampleCount: WaveformVisualizer.cpp:370–379, OscilloscopeVisualizer.cpp:312–320)

#### 3.5 Tap-Points (Preview-Fundament, Entscheid 2026-07-19)

- [ ] Stufen-Ausgänge als benannte, abonnierbare Daten definieren (z. B.
      `tap.audio` = normalisierte Bänder, `tap.map` = gemappte Daten, `tap.color` =
      Farbwerte je Element) — leichtgewichtig, nur bei aktivem Abonnenten befüllt
- [ ] In AudioSourceModule + je einem Pilot-Visualizer (Equalizer) verdrahten;
      Rest folgt mit der Migration (Schritt 5)

#### 3.6 Tests

- [ ] Schema-Tests (stage gesetzt, Builder), Handle-Tests, Tap-Point-Tests
- [ ] Migrations-Roundtrip: altes .lvp (Fixture) → laden → neue Keys korrekt; float-für-int-Vertrag bleibt (bestehender Test)

---

### Schritt 4: ConfigPanel generisch

**Ziel:** ConfigPanel kennt keine konkreten Visualizer mehr; Reihenfolge/Verhalten kommen aus dem Schema.

- [ ] 4.1 Zentrale Stage-Tabelle (Titel, Icon, Reihenfolge) ersetzt Emoji-Keyword-Heuristik (ConfigPanel.cpp:58–72) und String-Sortierung (:371–382)
- [ ] 4.2 `openGradientEditor` (:1269–1380) + Preview-Delegate (:748–802) auf Handles umstellen; die fünf Visualizer-`#include`s (:18–22) entfernen
- [ ] 4.3 `onModulePresetSave` (:1621–1701, Early-Return :1628) + `updateRelatedPresetWidget` (:1008–1067) generisch über Handles/Präfixe — Save-Button funktioniert bei allen Visualizern
- [ ] 4.4 `updateVisibility` (:1069–1264): nur noch dependsOn/dependsValues, Sonderfälle („Line Color"-Substring :1139, channelMode-Hardcode) entfernen
- [ ] 4.5 Default-Reset je Ebene: Parameter (Kontext/Button), Untergruppe, Stufe, Visualizer (bestehender Default-Eintrag) — als Commands (Undo-fähig)
- [ ] 4.6 Variant-Zugriffe härten: `std::holds_alternative<Color4f>` statt `value.index()==7` (:275, :947)

**Erwartetes Ergebnis:** Pulsing (unmigriert, via Kompatibilitätsschicht) und Equalizer
(nach 5.1) rendern beide korrekt in Stage-Reihenfolge.

---

### Schritt 5: Visualizer-Migration + Modul-Konsolidierung

**Ziel:** Jeder Visualizer einzeln auf Schema/Stufen; dabei Duplikate in Shared-Module überführen. Reihenfolge fix; nach jedem Teilschritt Suite + manueller Sichttest.

#### 5.1 Equalizer (Referenz)

- [ ] Gruppen „1.–8." → Stufen 1–5 (Post fehlt zulässig); Keys nach Migrationstabelle
- [ ] `eq.bands`↔`audio.bands`-Sync (EqualizerVisualizer.cpp:203–205) auflösen → ein Key `map.bands`
- [ ] Alias-Map Equalizer; Sichttest gegen `harvest/config-pipeline/`-Blaupause (Defaults/Ranges)

#### 5.2 Pulsing

- [ ] PulseShapeModule bekommt eigene paramDescs (heute: manuelle Deklaration im Visualizer)
- [ ] `shape.color.*` → `color.main.*`; Shape → `render.*`; Beat-Params → BeatModule-Anschluss (5.6)

#### 5.3 Waveform

- [ ] `m_smoothing`-Eigenglättung (WaveformModule.hpp:146, Anwendung WaveformVisualizer.cpp:590) → SmoothingModule
- [ ] 3 Farb-Handles (mono/left/right) — Editor erreicht jetzt alle (heute nur Mono, ConfigPanel.cpp:1288)
- [ ] SubGroup „Effects" (Mirror/Hold/Fade) → `post.*` (PostFxModule, 5.6)

#### 5.4 Oscilloscope

- [ ] 6 Farb-Handles (ch1–ch4, m1, m2) statt paramId-Parsing (:1296–1303)
- [ ] Phosphor/Trigger-Fade → `post.*`; Timebase/Trigger → `map.*`

#### 5.5 Superscope

- [ ] Hand-EMA (SuperscopeVisualizer.cpp:211–215, 283–287) → SmoothingModule
- [ ] Doppelte „Audio"-Semantik auflösen: `scope.audioSource/audioChannel` → Stufe 2 (`map.*`)
- [ ] Glow/Hold-Fade → `post.*`; Expressions unangetastet (Lua-Umstellung = Import-Phase)

#### 5.6 Shared-Module (begleitend, jeweils beim ersten Nutzer extrahieren)

- [ ] AudioUtil: `splitStereoData`/`resampleWaveform` (3 Kopien: Waveform/Oscilloscope/Superscope)
- [ ] PostFxModule: Hold/Fade/Phosphor/Mirror/Glow (4 Frame-Fade-Kopien; shader-erweiterbar — Konzept §5.6 Leitplanke 1)
- [ ] BeatModule: 3 Ad-hoc-Detektoren (Pulsing :1051, Superscope :255–257, Equalizer GradientDomain::Beat)
- [ ] Ein JSON-Preset-Helper statt 3 Mini-Parser (AudioSourceModule.hpp:1238–1304, SmoothingModule.hpp:1040–1101, ColorGradientModule)

---

### Schritt 6: Preview-Viewer je Parametergruppe

**Ziel:** Je Pipeline-Stufe/Untergruppe ist eine Live-Vorschau der Roh-/Zwischendaten
einblendbar (Pflichtenheft-Idee, fest beauftragt 2026-07-19).

- [ ] 6.1 Mini-Entwurf (½ Seite, vor Implementierung absegnen): welche Tap-Points (3.5)
      wie darstellen (Balken/Kurve/Farbstreifen je Stufe), Ein-/Ausblenden je Gruppe,
      Update-Rate, Abschaltbarkeit (Default aus)
- [ ] 6.2 Preview-Widget im ConfigPanel: einblendbar je Stufe/Untergruppe, gespeist
      aus Tap-Points; Default-Zustand pro Panel persistieren
- [ ] 6.3 Performance-Check: Frametime mit/ohne aktives Preview messen; kein messbarer
      Einfluss bei ausgeblendetem Preview (Tap nur bei Abonnent aktiv)

---

## 4. Dokumentation (letzter Schritt)

**Ziel:** Doku beschreibt den neuen Ist-Stand; Phase 4 gilt erst danach als abgeschlossen.

- [ ] 7.1 [Parameter_Reference.md](Parameter_Reference.md) auf neue Keys/Stufen umstellen (SSOT), Alias-Tabellen anhängen
- [ ] 7.2 [Visualizer_Architecture.md](Visualizer_Architecture.md) (Altlasten-Abschnitt auflösen), [../ui/ConfigPanel_Guide.md](../ui/ConfigPanel_Guide.md) (Einschränkungen raus, Preview dokumentieren), [../presets/FileFormat_Reference.md](../presets/FileFormat_Reference.md) (neue formatVersion)
- [ ] 7.3 CppModuleDoc neben den Headern für neue Module (PostFx, Beat, AudioUtil; SubscriberHandle in EventBus.md)
- [ ] 7.4 [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) → Status „Stabil"; Produkt-Changelog `../sessions/` schreiben

---

## 5. Akzeptanzkriterien

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

- [ ] Key-Migrationstabellen-Review durch Patrik (3.3) — blockiert Schritt 5
- [ ] Preview-Mini-Entwurf absegnen (6.1) — blockiert 6.2/6.3

Alle übrigen Entscheidungen sind gefallen (2026-07-19, siehe Konzept §8):
Keys+Alias ✓ · AudioSource pro Visual ✓ · Leitplanken ✓ · Analyzer entfernen (0.4) ✓ ·
DialogManager-Kleinfix (1.5) ✓ · Kategorie-Strings fixen (0.5) ✓ · Preview fest (Schritt 6) ✓ ·
en-Doku später wie CMakeCraft ✓

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.2.0** | **2026-07-19** | **Freigabe eingearbeitet: +0.4 Analyzer-Entfernung, +0.5 Kategorie-Fix, +1.5 DialogManager, +3.5 Tap-Points, Schritt 6 = Preview-Viewer (fest), Doku → Schritt 7 (40 Aufgaben)** |
| 0.1.0 | 2026-07-19 | Initial: Schritte 0–6 mit Datei:Zeile-Ankern aus der Session-29-Analyse |
