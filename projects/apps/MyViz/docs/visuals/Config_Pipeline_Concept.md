# Config-Pipeline — Konzept (Phase 4)

> **Version:** 1.0.0
> **Datum:** 2026-07-19
> **Typ:** Concept
> **Status:** Stabil (umgesetzt in Phase 4, Schritte 0–6 — Session 30; Umsetzungs-Details: [Config_Pipeline_Umsetzungsplan.md](Config_Pipeline_Umsetzungsplan.md))
> **Zielgruppe:** App-Entwickler
> **Bezug:** visualizers/ (alle), UI/panels/ConfigPanel, services/EventBus, Preset-System
> **Sprache:** Deutsch

---

## Inhaltsverzeichnis

1. [Einleitung / Vision](#1-einleitung--vision)
2. [Problemstellung](#2-problemstellung)
3. [Lösungsansatz](#3-lösungsansatz)
4. [Architektur](#4-architektur)
5. [Design-Entscheidungen](#5-design-entscheidungen)
6. [Verworfene Alternativen](#6-verworfene-alternativen)
7. [Umsetzungsplan](#7-umsetzungsplan)
8. [Offene Punkte](#8-offene-punkte)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Einleitung / Vision

Die Konfiguration **aller** Visualizer folgt einer gemeinsamen Pipeline mit identischem
Verhalten: Die Config-UI zeigt die Parametergruppen in der Reihenfolge, in der die Daten
fließen — und gleiche Config-Elemente (Gradient-Editor, Preset-Dropdowns, Default-Reset)
verhalten sich überall gleich.

**Die sechs Pipeline-Stufen** (verbindlich, aus `harvest/config-pipeline/README.md`):

1. **AudioSource/Analyse** — FFT-Größe, Skala (Linear/Log/Mel), Glättung, dB-Floor/Ceil
2. **Band-/Daten-Mapping** — bands, gain, Sample-Counts, Trigger, Orientierung
3. **Farbe** — Gradient/Solid, einheitlicher Gradient-Editor
4. **Rendering/Geometrie** — Balken, Linien, Formen, Z-Order, Display
5. **Peak/Partikel** — Spawner-Physik, Partikel
6. **Post-Processing** — Hold/Fade, Phosphor, Mirror, Glow

**Nicht verhandelbar** (Anforderungen vom 2026-07-18):
- Die UI-Reihenfolge folgt der Pipeline — durchgängig für ALLE Visuals.
- Der PresetManager liefert Zahlen beim Laden immer als `float` — jede
  setParam-Implementierung akzeptiert float für int-Parameter.
- Gradient-Invariante: `clearStops()` hält ≥ 2 Stops — UI/Serialisierung rechnen damit.

Begriffsrahmen: Die Stufen entsprechen der Node-Pipeline-Vision aus
`harvest/konzepte/viz_2025_node_reference_manual_visualizer_pipeline.md`
(Audio-Input → Preprocessing → Analyse → Generator → Modifier → Output) — Phase 4
übernimmt das **Stufenmodell und Vokabular**, nicht den Node-Editor (siehe §6.1).

**Langfrist-Ziel (Nordstern):** MyViz soll **AVS- und MilkDrop-Presets importieren**,
gleichartig editieren und mit aktueller Technologie darstellen; **Lua** (liegt als
`externals/lua54` bereit) dient dabei als Skript-Schicht. Referenz-Quellcode liegt in
`../ref/` (Repo-übergreifend): `vis_avs` (Original Winamp AVS), `winamp_orig`,
`MilkDrop3`, `milkdrop2077`, `projectm` (portable MilkDrop-Reimplementierung), `lua`.
Der Import selbst ist eine **eigene, spätere Phase** — aber jede
Phase-4-Entscheidung muss diesen Weg offenhalten (siehe 5.6).

## 2. Problemstellung

Befunde der Code-Analyse (Session 29, Details in
[Visualizer_Architecture.md](Visualizer_Architecture.md) §„Bekannte Altlasten"):

1. **Zwei Parameter-Welten:** Die Modul-Welt (`audio.*`, `audio.smooth.*`,
   `shape.color.*`) und die Equalizer-Welt (`eq.*`, `color.*`, `peakColor.*`, historisch
   `grad.*`/`peak.*`) sind unabhängig gewachsen; Präfix-Konventionen, Gruppen-Zuweisung
   und order-Offsets weichen je Visualizer ab.
2. **Pipeline-Reihenfolge ist nur eine String-Konvention:** `buildUIFromParams` sortiert
   alphabetisch nach Gruppen-Präfixen („1. Audio", „2. Shape", …). Nur der Equalizer hat
   Farbe/Peak als eigene Top-Level-Stufen; bei allen anderen ist Farbe eine Untergruppe,
   Stufe 2 existiert nirgends eigenständig, Stufe 6 ist verstreut oder fehlt.
3. **Gradient-Editor ist der bekannteste Abweichler:** Öffnen/Preview laufen über
   `dynamic_cast`-Kaskaden auf alle fünf Visualizer-Typen plus String-Heuristiken;
   bei Waveform ist nur der Mono-Gradient über den Editor erreichbar.
4. **Duplizierte Logik statt gemeinsamer Module:** Glättung 3× (SmoothingModule,
   Superscope-Hand-EMA, WaveformModule-Skalar), Beat-Detection 3×, Hold/Fade-Frames 4×,
   Stereo-Split/Resample 3×, JSON-Mini-Parser 3×.
5. **Tote Parallelsysteme:** `EqualizerModule` trägt eine komplette ungenutzte
   Audio-Pipeline + Parameter-System; `ModuleConfigWidget` und `ColorSchemeModule`
   (als Farbsystem) sind unreferenziert.
6. **Lifetime-Risiko vor jedem UI-Umbau:** ConfigPanel abonniert den EventBus in
   `onActivate`, der Destruktor unsubscribed nicht → potenzielles use-after-free.
7. **Presets hängen an Param-IDs:** Jede ID-Änderung invalidiert gespeicherte
   User-Presets (`.lvp`) — ohne Migrationspfad gehen Nutzerdaten kaputt.

## 3. Lösungsansatz

**Kernidee: Die Pipeline-Stufe wird ein first-class-Begriff im Parameter-System** —
statt String-Präfix-Konvention. Darauf bauen vier Säulen:

1. **Gemeinsames Parameter-Schema:** `ModuleParamDesc` bekommt eine explizite
   `stage`-Zuordnung (Enum, §4.2) und eine vereinheitlichte Key-Konvention (§4.3).
   Das ConfigPanel rendert generisch: eine Gruppe pro Stufe, in fixer Reihenfolge.
2. **Gemeinsame Module statt Kopien:** Glättung nur über SmoothingModule;
   neue kleine Shared-Module für Beat-Detection, Hold/Fade/Post-Effekte und
   Stereo-Split/Resample (aus den bestehenden Kopien extrahiert, per Tests eingezäunt).
3. **Einheitlicher Gradient-Zugriff über das Interface:** Visualizer exponieren ihre
   Gradienten als benannte Handles (`IVisualizer::gradients()` → Liste von
   (paramPrefix, ColorGradientModule*)). Editor, Preview und Modul-Preset-Save arbeiten
   nur noch über Handles — keine `dynamic_cast`-Kaskaden mehr, Waveform-Left/Right
   werden damit automatisch editierbar.
4. **Fundament vor UI-Umbau:** EventBus-Upgrade (RAII-`SubscriberHandle`, Weak-Abos aus
   `harvest/core-module/eventbus/`) beseitigt die Panel-Lifetime-Falle; CommandBus
   (`harvest/core-module/commandbus/`, Tests liegen bereit) liefert Undo/Redo für
   Parameter-Änderungen und den Gradient-Editor.

Dazu die UX-Anforderungen aus dem Pflichtenheft als Schema-Features:
**Default-Reset je Ebene** (Parameter/Untergruppe/Stufe/Visualizer),
**Presets je Gruppe** (Modul-Presets verallgemeinert — für alle Visualizer, nicht nur
Pulsing) und **Preview je Gruppe** (Raw-Anzeige der Stufen-Ausgangsdaten —
**fester Bestandteil von Phase 4**, Entscheid 2026-07-19; dafür definiert das Schema
die Stufen-Ausgänge als benannte, abonnierbare Tap-Points).

## 4. Architektur

### 4.1 Datenfluss = UI-Reihenfolge

```
Audio (Engine/Player)
      │ FFT / PCM
      ▼
[1] AudioSource/Analyse   audio.*        (AudioSourceModule + SmoothingModule)
      ▼ normalisierte Bänder / Samples
[2] Band-/Daten-Mapping   map.*          (bands/gain/sampleCount/trigger/orientation)
      ▼ gemappte Daten
[3] Farbe                 color.*        (ColorGradientModule, ggf. mehrere Handles)
      ▼ Farbwert je Element
[4] Rendering/Geometrie   render.*       (visualizer-spezifisch)
      ▼ Frame
[5] Peak/Partikel         peak.* / particle.*  (Peak-Spawner, Partikel — optional)
      ▼ Frame
[6] Post-Processing       post.*         (Hold/Fade, Phosphor, Mirror, Glow — optional)
      ▼
   Bildschirm
```

Nicht jeder Visualizer hat jede Stufe (Pulsing hat kein Peak-Modul); fehlende Stufen
werden in der UI schlicht nicht angezeigt — die **Reihenfolge** der vorhandenen bleibt.

### 4.2 Schema-Erweiterung

```cpp
enum class PipelineStage : uint8_t {
    AudioSource = 1,  // Analyse
    Mapping     = 2,
    Color       = 3,
    Render      = 4,
    PeakParticle= 5,
    Post        = 6,
};
// ModuleParamDesc erhält:
PipelineStage stage;      // ersetzt das "1. Audio"-Präfix in group
std::string   subGroup;   // bleibt (fachliche Untergruppe innerhalb der Stufe)
```

`ConfigPanel::buildUIFromParams` sortiert nach `stage` → `subGroup-Order` → `order`.
Die Gruppen-Titel/Icons kommen aus einer zentralen Stage-Tabelle (ein Ort statt
Emoji-Keyword-Heuristik).

### 4.3 Key-Konvention (Soll)

| Stufe | Präfix | Beispiele (Soll) | Heutige Keys (Auszug) |
|---|---|---|---|
| 1 | `audio.` | `audio.scale`, `audio.smooth.timeMs` | unverändert ✓ |
| 2 | `map.` | `map.bands` (nur Equalizer — F2), `map.sampleCount`, `map.trigger.level` | `eq.bands`, `waveform.sampleCount`, `scope.timePerDiv` |
| 3 | `color.` | `color.mode`, `color.<handle>.preset` | `shape.color.*`, `monoColor.*`, `ch1Color.*`, `color.*` |
| 4 | `render.` | `render.orientation`, `render.lineWidth` | `shape.*`, `waveform.*`, `scope.*` (Rest) |
| 5 | `peak.` / `particle.` | `peak.gravity`, `particle.maxPerBand` | `eq.peak*`, `peakColor.*`, `particle.*` |
| 6 | `post.` | `post.hold.fadeTime`, `post.phosphor.decay` | `waveform.holdEnabled`, `scope.phosphor*` |

Farb-Handles: mehrkanalige Visualizer deklarieren benannte Handles
(`color.mono.*`, `color.left.*`, `color.ch1.*` …) — dieselbe Struktur für alle.

### 4.4 Preset-Migration

- `.lvp`-`formatVersion` wird angehoben und beim Laden **ausgewertet** (heute: nur
  geschrieben).
- Eine statische **Alias-Map** `alteID → neueID` je Visualizer übersetzt beim Laden
  alter Presets (Format-Version < neu) die Keys; der float-für-int-Vertrag gilt
  unverändert (Test: `test_VisualizerPresetManager.cpp`).
- Speichern erfolgt immer im neuen Schema.

### 4.5 Modul-Konsolidierung

| Neu/konsolidiert | ersetzt | Quelle |
|---|---|---|
| SmoothingModule (einziger Glätter) | Superscope-Hand-EMA, WaveformModule-`m_smoothing` | vorhanden |
| BeatModule (klein, shared) | 3 Ad-hoc-Implementierungen | Ist-Code + `harvest` E4-Ideen |
| PostFxModule (Hold/Fade/Phosphor/Mirror/Glow) | 4 Frame-Fade-Kopien | Ist-Code; Konzept-Ideen in `harvest/old_docs/concepts/PostProcessModule-Concept.md` (nur als Ideenquelle) |
| AudioUtil (Stereo-Split/Resample) | 3 Kopien | Ist-Code |
| — entfernt: | EqualizerModule-Parameter+Audio-Pipeline (tot), ModuleConfigWidget, ColorSchemeModule-Farbsystem | |

## 5. Design-Entscheidungen

### 5.1 Stage-Enum statt String-Präfixe

**Kontext:** Reihenfolge/Icons/Sichtbarkeit hängen heute an Strings („1. Audio",
Substring „Line Color") — Umbenennungen brechen still.
**Entscheidung:** Explizites `PipelineStage`-Enum in `ModuleParamDesc`; Titel/Icons
zentral je Stage.
**Konsequenzen:** ConfigPanel wird generisch; die „1./2./…"-Präfixe verschwinden aus
den group-Strings; Sonderfall-Heuristiken (`updateVisibility`-Substrings) entfallen.

### 5.2 Gradient-Handles über das Interface statt dynamic_cast

**Kontext:** Editor/Preview/Preset-Save kennen jede Visualizer-Klasse; Waveform-Kanäle
sind teilweise unerreichbar; jeder neue Visualizer erfordert ConfigPanel-Änderungen.
**Entscheidung:** `IVisualizer` exponiert benannte Gradient-Handles; UI arbeitet
ausschließlich damit.
**Konsequenzen:** ConfigPanel verliert die 5 Visualizer-`#include`s; neue Visualizer
bekommen Editor/Preview gratis; Modul-Preset-Save funktioniert überall (heute nur
Pulsing).

### 5.3 AudioSource bleibt pro Visual — geteilt werden Presets

**Kontext:** Grundsatzfrage aus `harvest/config-pipeline/` („AudioSource je Visual oder
gemeinsam?").
**Optionen:**

| Option | Pro | Contra |
|---|---|---|
| Global geteilt | eine Analyse, weniger CPU | nimmt jedem Visual eigene FFT/Skala/Glättung; Ist-Verhalten ändert sich |
| Pro Visual (Ist) | volle Flexibilität, kein Verhaltensbruch | n Analysen, Einstellungen je Visual zu pflegen |
| Pro Visual + geteilte Presets | Flexibilität bleibt, Konsistenz per Preset | Presets müssen überall funktionieren (5.2 liefert das) |

**Entscheidung:** Pro Visual, mit überall verfügbaren Audio-/Smoothing-Presets.
Eine global geteilte Analyse bleibt Zukunftsoption der Node-Vision (§6.1).

### 5.4 Preset-Kompatibilität per Alias-Map, nicht per Dual-Write

**Kontext:** ID-Umbenennung invalidiert User-Presets.
**Entscheidung:** Lesen: Alias-Übersetzung alter Keys; Schreiben: nur neues Schema;
`formatVersion`-Auswertung beim Laden.
**Begründung:** Dual-Write (alte+neue Keys schreiben) hält die Alt-Welt ewig am Leben;
die Alias-Map ist ein einzelner, testbarer Ort.

### 5.5 Reihenfolge: Fundament → Schema → UI → Migration je Visualizer

**Entscheidung:** Erst EventBus-RAII + CommandBus (beseitigt Lifetime-Falle, bringt
Undo/Redo), dann Schema/Stage-Enum, dann generisches ConfigPanel, dann Visualizer
einzeln migrieren — **Equalizer zuerst** (beste Doku/Blaupause, strukturell am
nächsten an der Zielpipeline), danach Pulsing, Waveform, Oscilloscope, Superscope.
**Begründung:** Jeder Schritt bleibt einzeln testbar; die Suite (36 Cases) und die
Preset-Tests zäunen die Verträge ein. Tote Systeme (§4.5) werden **vor** dem Umbau
entfernt, damit niemand gegen tote Pfade refaktoriert.

### 5.6 Leitplanken für den späteren AVS-/MilkDrop-Import

**Kontext:** Das Langfrist-Ziel (§1) ist der Import und die gleichartige Bearbeitung
von AVS-/MilkDrop-Presets. Phase 4 implementiert das nicht, darf es aber nicht verbauen.

**Leitplanken (verbindlich für Phase-4-Entwürfe):**

1. **Stage-Modell deckt die Fremdwelten ab:** AVS ist eine Render-Kette
   (Generatoren → Trans/Movement → Compositing), MilkDrop im Kern
   Feedback/Warp + Waves/Shapes + Composite-Shader. Beides passt auf die Stufen
   2–6; insbesondere ist das geplante **PostFxModule (Stufe 6: Feedback, Trails,
   Warp, Mirror)** die MilkDrop-Essenz und wird darauf ausgelegt, später
   shader-basierte Effekte aufzunehmen.
2. **Wertquelle pro Parameter umschaltbar — UI ↔ Expression:** Jeder Effekt kann
   **parametriert ODER per Expression** laufen (umschaltbar; die Expression ist die
   mächtigere Form, der UI-Wert der Default). Das ist das Binding-Modell aus dem
   viz_2025-Manual (△ UI / ◻ Control / Script je Parameter, Lebenszyklus
   Init/PerFrame/PerBeat/PerPoint). Phase 4 implementiert die Umschaltung noch nicht,
   aber: `ModuleParamDesc::canBeInput` bleibt erhalten, und keine Entscheidung darf
   UI-Wert und Wertquelle untrennbar verschweißen — der Parameterwert ist immer
   „aktueller Wert aus Quelle X", nie „das, was das Widget zuletzt gesetzt hat".
3. **Superscope ist die Keimzelle:** Der SuperscopeVisualizer (AVS-Stil,
   Expression-System) ist der Prototyp für skriptbare Visuals — seine
   Expression-Auswertung wird perspektivisch auf Lua umgestellt; EEL→Lua- bzw.
   HLSL-Übersetzung der Fremd-Presets ist Aufgabe der Import-Phase.
4. **Preset-Lade-Pfad ist erweiterbar:** Die Migrationsmechanik aus §4.4
   (formatVersion-Auswertung + übersetzende Loader) wird so gebaut, dass
   Fremdformat-Importer (`.avs`, `.milk`) als weitere Übersetzer andocken können —
   Import = Übersetzung in das gemeinsame Parameter-Schema + Skripte.
5. **Einzeleffekte bleiben einbettbar (Multieffekt-System):** Die heutigen
   Visualizer sind zugleich die späteren **Bausteine einer AVS-artigen Effektkette**:
   Ein Effekt (z. B. Superscope) läuft heute standalone im Widget und später als
   Element eines Multieffekt-Hosts — dort ebenso parametriert oder per Expression.
   Konsequenz für Phase 4: Effekt-Logik und Parameter-Schema dürfen nicht am
   „ein Visualizer = ein Vollbild-Widget"-Modell festwachsen (kein UI-Zugriff aus
   Effektcode, Zustand pro Instanz statt statisch, Stufen 3–6 als Module statt
   Inline-Code) — genau die Modul-Konsolidierung aus §4.5.

**Konsequenz:** Vor der Import-Phase steht eine Analyse der Referenz-Repos in
`../ref/` (Preset-Formate, EEL-Sprachumfang, projectM-Architektur) — siehe §8.

## 6. Verworfene Alternativen

### 6.1 Vollständiger Node-Editor (viz_2025-Vision)

Der Umbau auf einen frei verdrahtbaren Node-Graphen (Typ-System, Wiring, Skripte)
ist eine eigene Produktgeneration, kein Refactoring — Phase 4 übernimmt nur
Stufenmodell und Vokabular. Die Vision bleibt als Roadmap-Material in
`harvest/konzepte/` bzw. `harvest/old_docs/architecture/LumiPulse_VisualSystem_Architecture.md`.

### 6.2 Weiter String-Präfix-Gruppen („1. Audio")

Funktioniert heute, bricht aber still bei jeder Umbenennung und kodiert die Pipeline
nirgends maschinenlesbar. Verworfen zugunsten des Stage-Enums.

### 6.3 Eine globale AudioSource für alle Visuals

Verworfen für Phase 4 (siehe 5.3) — Verhaltensbruch und Flexibilitätsverlust; als
spätere Option offen.

## 7. Umsetzungsplan

> **Detailplan mit Checklisten und Datei-Ankern:**
> [Config_Pipeline_Umsetzungsplan.md](Config_Pipeline_Umsetzungsplan.md) —
> die Tabelle hier ist die Kurzübersicht.

| Schritt | Inhalt | Absicherung |
|---|---|---|
| 4.0 | Tote Systeme entfernen (EqualizerModule-Params/Audio-Pipeline, ModuleConfigWidget, ColorSchemeModule-Rest) | Build + Suite grün |
| 4.1 | EventBus-Upgrade: RAII-`SubscriberHandle`, Weak-Abos; ConfigPanel-Unsubscribe-Fix | harvest-Tests portieren; Transient-Skip-Test |
| 4.2 | CommandBus einführen (Undo/Redo) | harvest/tests bereit |
| 4.3 | Schema: `PipelineStage`-Enum, Key-Konvention, Gradient-Handles im Interface | neue Unit-Tests Schema/Handles |
| 4.4 | ConfigPanel generisch (Stage-Gruppen, zentrale Stage-Tabelle, Handles statt dynamic_cast); Default-Reset je Ebene; Gruppen-Presets für alle | UI-Smoke + Preset-Tests |
| 4.5 | Preset-Migration: formatVersion-Auswertung + Alias-Map | Roundtrip-Tests alt→neu |
| 4.6 | Visualizer migrieren: Equalizer → Pulsing → Waveform → Oscilloscope → Superscope; dabei Modul-Konsolidierung (§4.5) | je Visualizer: Suite + manueller Sichttest |
| 4.7 | **Preview je Gruppe** (fest, Entscheid 2026-07-19): Mini-Entwurf → Tap-Point-Viewer → Performance-Check | Sichttest + Frametime-Messung |
| 4.8 | Kür: Beat-/PostFx-Ausbau (Ideen E1–E7) | — |

Kleinfixes unterwegs (aus Handover): Transient-resolve()-Dangling (Skip-Test
aktivieren), `Event::consume()` const-fähig, ungenutzte Felder
PulsingVisualizer/SuperscopeModule.

## 8. Offene Punkte

**Entschieden am 2026-07-19** (Q&A mit Patrik): Key-Umbenennung mit Alias-Map (§4.3) ✓ ·
AudioSource pro Visual + geteilte Presets (§5.3) ✓ · Leitplanken §5.6 verbindlich ✓ ·
`AudioAnalyzer` wird in Schritt 0 **entfernt** (sauberer Audio-Verteil-Service kommt
erst mit der Import-/Audio-Phase) ✓ · `DialogManager`-Instanziierung als Kleinfix in
Schritt 1 ✓ · Visualizer-Kategorie-Strings („static Parametring …") werden in Schritt 0
gefixt ✓ · **Preview je Gruppe ist fester Phase-4-Bestandteil** (Schritt 4.7) ✓ ·
en-Doku: wie CMakeCraft — de ist SSOT, en wird später nachgezogen ✓

**Key-Review abgeschlossen (2026-07-19):** Alle 8 Fragen (F1–F8) entschieden — siehe
[Parameter_Key_Migration.md](Parameter_Key_Migration.md). Kernpunkte: kein `map.gain`
(Equalizer-gain → `render.heightScale`, wird in 5.1 neu verdrahtet); `map.bands` nur
beim Equalizer, sonst bleibt `audio.bands`; `waveform.smoothing` entfällt mit
Wert-Konverter für Alt-Presets (Alias-Mechanik braucht Wert-Konverter in Schritt 5);
kanal-strukturierte Render-Keys (`render.mono.offset`).

Noch offen:
- [ ] Preview-Viewer: technischer Mini-Entwurf (Datenabgriff/Tap-Points, Darstellung,
      Performance) — vor Umsetzung von Schritt 4.7
- [ ] en-Übersetzungen der neuen App-Doku nachziehen (de=SSOT, wie CMakeCraft)
- [ ] **Analyse der Referenz-Repos** in `../ref/` (vis_avs, projectm, MilkDrop3,
      milkdrop2077, winamp_orig) für die Import-Phase: Preset-Formate (.avs/.milk),
      EEL-Sprachumfang vs. Lua, projectM-Renderarchitektur — eigene Session nach Phase 4

## 9. Siehe auch

- [Visualizer_Architecture.md](Visualizer_Architecture.md) — Ist-Architektur inkl. Altlasten
- [Parameter_Reference.md](Parameter_Reference.md) — SSOT aller heutigen Parameter
- [../ui/ConfigPanel_Guide.md](../ui/ConfigPanel_Guide.md) — heutiges ConfigPanel
- [../presets/Preset_System.md](../presets/Preset_System.md) · [../presets/FileFormat_Reference.md](../presets/FileFormat_Reference.md)
- `harvest/config-pipeline/README.md` — Pflichtenheft (Repo-Root)
- `harvest/core-module/` — EventBus-/CommandBus-Vorlagen + Tests

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-07-19** | **Status Stabil: Konzept vollständig umgesetzt (Phase 4 Schritte 0–6, Session 30) — alle 5 Visualizer auf Stufen-Schema, Alias-Maps + Wert-Konverter aktiv (formatVersion 2), Shared-Module (Smoothing/Beat/HoldFade/AudioUtil/JsonPresetParser), Stage-Previews im ConfigPanel** |
| 0.3.0 | 2026-07-19 | Kernentscheidungen freigegeben (Q&A): Keys+Alias, AudioSource pro Visual, Leitplanken, Analyzer-Entfernung, Preview fest in Phase 4 (Tap-Points im Schema) |
| 0.2.0 | 2026-07-18 | Langfrist-Ziel AVS/MilkDrop-Import (§1) + Leitplanken 5.6 + ref/-Analyse als offener Punkt |
| 0.1.0 | 2026-07-18 | Initial (Session 29): Stufenmodell, Schema, Entscheidungen 5.1–5.5, Umsetzungsplan |
