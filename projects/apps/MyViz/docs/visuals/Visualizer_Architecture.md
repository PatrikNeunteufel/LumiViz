# Visualizer-Architektur — IVisualizer, Modul-System und Parameter-Mechanik

> **Version:** 1.1.0
> **Datum:** 2026-07-19
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

Konsolidiert aus der Alt-Doku (`harvest/old_docs/`: IModule.md, Visualizer-Architecture-Reference.md,
ColorGradientModule.md, README_MODULES.md) und **gegen den Code verifiziert**
(Stand 2026-07-19, nach Phase 4 „Config-Pipeline vereinheitlichen", Schritte 0–6).
Bei Abweichungen zwischen Alt-Doku und Code gilt der Code.

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Kern-Interfaces: IVisualizer und VisualizerBase](#2-kern-interfaces-ivisualizer-und-visualizerbase)
3. [Modul-System](#3-modul-system)
4. [Parameter-System](#4-parameter-system)
5. [Sichtbarkeits-System (dependsOn)](#5-sichtbarkeits-system-dependson)
6. [Key-Routing (Pipeline-Schema)](#6-key-routing-pipeline-schema)
7. [Die 5 Visualizer im Ist-Stand](#7-die-5-visualizer-im-ist-stand)
8. [Farb-/Gradient-System](#8-farb-gradient-system)
9. [Tap-Points und Stage-Previews](#9-tap-points-und-stage-previews)
10. [Registrierung](#10-registrierung)
11. [Altlasten-Bilanz (nach Phase 4)](#11-altlasten-bilanz-nach-phase-4)
12. [Siehe auch](#12-siehe-auch)

---

## 1. Überblick

Jeder Visualizer in MyViz folgt derselben modularen Architektur:

- **Komposition statt Vererbung:** Visualizer setzen wiederverwendbare Module zusammen
  (AudioSourceModule, ColorGradientModule, je ein visualizer-eigenes Modul, geteilte
  Bausteine wie BeatModule/HoldFadeEffect).
- **Parameter-getriebene Konfiguration:** Alle Einstellungen sind typisierte Parameter,
  erreichbar über `paramDescs()` / `getParam()` / `setParam()`. Das ConfigPanel generiert
  seine UI vollständig aus diesen Deskriptoren.
- **Pipeline-Schema (Phase 4):** Jeder Parameter trägt eine `PipelineStage` (1–6);
  der Key-Präfix kodiert die Stufe (`audio.` / `map.` / `color.<handle>.` /
  `render.` / `peak.`+`particle.` / `post.`). SSOT der Präfix→Stage-Zuordnung:
  `include/visualizers/PipelineKeys.hpp` (`stageForKey`/`groupForStage`).
- **Bedingte Sichtbarkeit:** Parameter nutzen `dependsOn` + `dependsValues` für dynamisches
  Ein-/Ausblenden in der UI.
- **Preset-Portabilität:** Presets speichern Parameter über ihre IDs
  (`formatVersion = 2`); Alt-Presets werden über registrierte Alias-Maps übersetzt
  ([Parameter_Key_Migration.md](Parameter_Key_Migration.md)); Gradients zusätzlich
  als Rohdaten-Fallback (siehe [§8.4](#84-preset-lade-logik)).

Beteiligte Schichten:

```
IVisualizer (Interface)                      include/visualizers/IVisualizer.hpp
  └── VisualizerBase (gemeinsame Basis)      include/visualizers/VisualizerBase.hpp
        ├── PulsingVisualizer
        ├── WaveformVisualizer
        ├── OscilloscopeVisualizer
        ├── SuperscopeVisualizer
        └── EqualizerVisualizer

IModule (Modul-Interface + Parameter-System) include/visualizers/modules/IModule.hpp
  ├── AudioSourceModule (mit eingebettetem SmoothingModule)
  ├── WaveformModule / OscilloscopeModule / SuperscopeModule /
  │   EqualizerModule / PulseShapeModule (je Visualizer)
  ├── ColorGradientModule (kein IModule, aber gleiche Parameter-API — siehe §8)
  └── geteilte Bausteine (kein IModule):
        BeatModule (processing/), HoldFadeEffectT (postfx/PostFxModule.hpp),
        AudioUtil.hpp, JsonPresetParser.hpp
```

---

## 2. Kern-Interfaces: IVisualizer und VisualizerBase

### 2.1 IVisualizer

`include/visualizers/IVisualizer.hpp` — das Interface aller Visualizer:

| Bereich | Methoden |
|---|---|
| Identifikation | `visualizerId()`, `visualizerName()`, `visualizerDescription()` (alle `QString`) |
| OpenGL-Lifecycle | `initialize()`, `render(float deltaTime)`, `resize(const QSize&)`, `cleanup()` |
| Zustand | `isInitialized()` |
| Audio-Daten | `updateSpectrum(const float*, int)`, `updateWaveform(const float*, int)` |
| Parameter | `paramDescs()`, `getParam(id, out)`, `setParam(id, value)`, `resetToDefaults()` |
| Farb-Handles (Phase 4) | `gradients()` → `std::vector<GradientHandle>` (§8.3) |
| Tap-Points (Phase 4) | `tapPoints()` → `std::vector<TapPoint>` (§9) |

`GradientHandle` = {id (z. B. `"main"`, `"ch1"`), displayName, paramPrefix
(z. B. `"color.mono."`), non-owning Zeiger auf das `ColorGradientModule`}.
`TapPoint` = {id, displayName, `PipelineStage`, `sample`-Funktor (Daten-Kopie),
`TapDisplay` (Bars/Curve)}. Beide Defaults sind leer — jeder der 5 Visualizer
überschreibt sie.

Wichtig: Visualizer sind **keine Panels** — sie rendern im zentralen VisualizerWidget;
das VisualSelectPanel wechselt nur den aktiven Visualizer.

### 2.2 VisualizerBase

`include/visualizers/VisualizerBase.hpp` implementiert das Gerüst und deklariert die
Override-Punkte `onInitialize()`, `onRender(float)`, `onResize(const QSize&)`, `onCleanup()`
(die öffentlichen Lifecycle-Methoden sind `final`). Zusätzlich:

- **Thread-sichere Audio-Puffer** (`QMutex`-geschützt): `getSpectrum()`, `getWaveform()`,
  `hasNewAudioData()` liefern Kopien; `updateSpectrum/updateWaveform` dürfen aus dem
  Audio-Thread kommen.
- Viewport-Verwaltung: `viewportSize()`, `width()`, `height()`, `aspectRatio()`.
- Initialisierungs-Tracking (`isInitialized()`).

Das OpenGL-**Context-Tracking** (Schutz gegen Qt-ADS-Undock-Crashes) ist **nicht** in
VisualizerBase implementiert, sondern als Pattern in jedem Visualizer
(`m_lastContext`-Member) — siehe [OpenGL_Context_Handling.md](OpenGL_Context_Handling.md).

---

## 3. Modul-System

### 3.1 IModule

`include/visualizers/modules/IModule.hpp` definiert das Modul-Interface:

| Methode | Beschreibung |
|---|---|
| `moduleId()` / `displayName()` / `category()` / `description()` | Identifikation (`const char*`) |
| `paramDescs()` | Alle Parameter-Deskriptoren (`std::vector<ModuleParamDesc>`) |
| `getParam(id, out)` / `setParam(id, value)` | Laufzeit-Zugriff, `false` bei unbekannter ID |
| `resetToDefaults()` | Auf Standardwerte zurücksetzen |
| `initialize()` / `activate()` / `deactivate()` / `update(deltaTime)` | Lifecycle-Hooks (optional) |

Daneben existieren im Header spezialisierte Interfaces (`IProcessingModule<TIn,TOut>`,
`ISourceModule<TOut>`, `IRenderModule` mit `RenderContext`), die aktuell nicht aktiv
genutzt werden (Vorhalt für das Node-Konzept).

### 3.2 Standard- und Shared-Module

| Modul | Zweck | Key-Präfix / Nutzung |
|---|---|---|
| `AudioSourceModule` | FFT-Mapping (Linear/Log/Mel), dB-Normalisierung, Gain, eingebettetes Smoothing | `audio.` (Stufe 1) |
| `SmoothingModule` | SMA/EMA/WMA/DEMA-Glättung — eingebettet in AudioSourceModule **und** eigenständig als Display-Glätter (`processArrayPerIndex()`: per-Index-EMA, Config von `audio.smooth.*` synchronisiert; Waveform/Oscilloscope) | `audio.smooth.` |
| `ColorGradientModule` | Solid/Linear/Radial/Outline-Farbe, Multi-Stop-Gradients, Presets | `color.<handle>.` (Stufe 3, §8) |
| je Visualizer eigenes Modul | Domänenlogik (WaveformModule, OscilloscopeModule, SuperscopeModule, EqualizerModule, PulseShapeModule) | über Übersetzungstabellen bzw. direkte Deklaration (§6) |
| `BeatModule` (`modules/processing/BeatModule.hpp`) | Geteilte Beat-Erkennung: Kanten-Detektor (Schwellwert auf skaliertem Level) + adaptiver Detektor (Signal-Energie) | Pulsing, Superscope |
| `HoldFadeEffectT<TFrame>` (`modules/postfx/PostFxModule.hpp`) | Geteilter Hold/Fade-Trail (Frames halten, linear ausblenden); Alias `HoldFadeEffect` = `HoldFadeEffectT<std::vector<float>>` | Waveform (3×), Superscope (`SuperscopePoint`-Frames); Schema-Keys `post.hold.*` |
| `AudioUtil.hpp` | `splitStereoData()` (interleaved → L/R), `resampleNearest()` (+ Gain) — ersetzt die früheren toten Funktions-Kopien | Waveform, Superscope (Oscilloscope behält sein lineares Inline-Resampling) |
| `JsonPresetParser.hpp` | Minimale Flat-JSON-Wert-Extraktion — konsolidiert die drei handgerollten Preset-Parser (AudioSource/Smoothing/ColorGradient) | Modul-Preset-Dateien (.audio/.smooth/.grad) |

### 3.3 Kompositions-Muster

Visualizer betten Module als Member ein und aggregieren deren Parameter. Für die
AudioSource ist das ein einfaches Präfixieren mit Stage-Zuweisung:

```cpp
for (const auto& p : m_audioSource.paramDescs())
{
    ModuleParamDesc prefixed = p;
    prefixed.id = "audio." + p.id;
    prefixed.group = "Audio";
    prefixed.stage = PipelineStage::AudioSource;

    // KRITISCH: dependsOn-Referenz mit-übersetzen!
    if (!prefixed.dependsOn.empty())
        prefixed.dependsOn = "audio." + prefixed.dependsOn;

    params.push_back(prefixed);
}
```

Für die visualizer-eigenen Module läuft die Aggregation seit Schritt 5 über
**Übersetzungstabellen** Modul-Sub-ID ↔ Pipeline-Key (§6): das Modul behält seine
internen IDs (`"sampleCount"`, `"monoColor.mode"`), der Visualizer übersetzt sie
auf Pipeline-Keys (`"map.sampleCount"`, `"color.mono.mode"`) und leitet
`stage`/`group` aus dem Key ab (`stageForKey`/`groupForStage`,
`PipelineKeys.hpp`). Modul-Pfade bleiben Punkt-getrennt:

```
"smooth.algorithm"      → SmoothingModule.algorithm
"audio.smooth.timeMs"   → AudioSourceModule → SmoothingModule → timeMs
```

---

## 4. Parameter-System

### 4.1 ParamValue

> **Korrektur gegenüber Alt-Doku:** Beide Alt-Referenzen dokumentierten falsche
> Varianten (mit `std::monostate`/`glm::vec2` bzw. mit `double`/`std::vector<float>`).
> Maßgeblich ist der Code (`IModule.hpp`):

```cpp
/// RGBA color (normalized 0-1)
using Color4f = std::array<float, 4>;
using Vec2f   = std::array<float, 2>;
using Vec3f   = std::array<float, 3>;
using Vec4f   = std::array<float, 4>;

using ParamValue = std::variant<
    bool,           // Index 0
    int,            // Index 1
    float,          // Index 2
    std::string,    // Index 3
    Vec2f,          // Index 4
    Vec3f,          // Index 5
    Vec4f,          // Index 6
    Color4f         // Index 7
>;
```

Achtung: `Vec4f` und `Color4f` sind **derselbe Typ** (`std::array<float,4>`) — für
Farben muss deshalb explizit der Variant-Index 7 gesetzt werden. Seit Phase 4 ist
das zentralisiert: `kParamValueColorIndex` + Helfer `makeColorValue()` /
`isColorValue()` / `getColor()` in `IModule.hpp` — der Index ist nicht mehr an
mehreren Stellen hart verdrahtet.

### 4.2 ParamType und ParamWidget

```cpp
enum class ParamType  { Bool, Int, Float, String, Enum, Vec2, Vec3, Vec4, Color };
enum class ParamWidget{ Default, Slider, Spinbox, Checkbox, Dropdown, ColorPicker,
                        TextInput, TextArea, Knob, Toggle, ButtonGroup };
```

(Alt-Doku-Reihenfolge `Int, Float, Bool, …` und der Typ „Button" sind überholt.)
`Enum` wird als `int` (Options-Index) gespeichert. Das Widget ist nur ein Hint;
`ParamWidget::Default` wählt anhand des Typs (Float/Int → Slider+Spinbox, Bool →
Checkbox, Enum → Dropdown, Color → Farb-Button, String → Zeile).

### 4.3 PipelineStage und ModuleParamDesc

```cpp
enum class PipelineStage : std::uint8_t
{
    None         = 0,  // unmigriert — Legacy-Gruppen-Präfix-Sortierung greift
    AudioSource  = 1,  // Analyse: FFT, Scale, Smoothing, dB-Floor/Ceil
    Mapping      = 2,  // Band-/Daten-Mapping: bands, sampleCount, Trigger
    Color        = 3,  // Gradient-/Solid-Farbe (Gradient-Handles)
    Render       = 4,  // Geometrie/Rendering: Bars, Linien, Shapes, Display
    PeakParticle = 5,  // Peak-Spawner-Physik und Partikel
    Post         = 6   // Post-Processing: Hold/Fade, Mirror, Glow
};
```

Vollständige Feldliste von `ModuleParamDesc` (`IModule.hpp`):

| Feld | Typ | Beschreibung |
|---|---|---|
| `id` | `std::string` | Eindeutig innerhalb des Moduls; Visualizer übersetzen auf Pipeline-Keys |
| `displayName` | `std::string` | UI-Label |
| `group` | `std::string` | Aufklappbare Hauptgruppe (kanonisch je Stage, `groupForStage()`) |
| `subGroup` | `std::string` | Geschachtelte Untergruppe (z. B. `"Line Color Mono"`) |
| `tooltip` | `std::string` | Hilfetext |
| `stage` | `PipelineStage` | **Pipeline-Stufe (Phase 4)** — steuert die Gruppen-Reihenfolge im Panel |
| `type` | `ParamType` | Datentyp (Default `Float`) |
| `defaultValue` | `ParamValue` | Standardwert |
| `minValue` / `maxValue` / `step` | `float` | Wertebereich (Default 0 / 1 / 0.01) |
| `enumOptions` | `std::vector<std::string>` | Optionen für `Enum` |
| `widget` | `ParamWidget` | UI-Widget-Hint |
| `order` | `int` | Sortierung innerhalb der Gruppe |
| `advanced` | `bool` | In „Advanced"-Bereich verstecken |
| `hidden` | `bool` | Komplett unsichtbar — nur für Preset-Serialisierung |
| `canBeInput` | `bool` | Vorhalt: als Node-Input konvertierbar |
| `dependsOn` | `std::string` | ID des steuernden Parameters |
| `dependsValues` | `std::vector<ParamValue>` | Sichtbar, wenn **einer** der Werte matcht (OR-Logik) |
| `unit` | `std::string` | Einheit (z. B. `"ms"`, `"px"`) |
| `format` | `std::string` | printf-Format (z. B. `"%.2f"`) |

### 4.4 ParamBuilder (Fluent API)

```cpp
auto desc = ParamBuilder("timeMs", ParamType::Float)
    .displayName("Time Constant")
    .range(0.0f, 500.0f, 1.0f)
    .defaultValue(50.0f)
    .unit("ms")
    .subGroup("Smoothing")
    .dependsOn("algorithm", std::vector<ParamValue>{2, 4})  // EMA, DEMA
    .order(2)
    .build();
```

Verfügbare Setter: `displayName`, `group`, `subGroup`, `tooltip`, `range(min,max,step)`,
`defaultValue`, `enumOptions`, `widget`, `unit`, `format`, `order`, `stage`,
`advanced`, `canBeInput`, `dependsOn` (Einzelwert oder Werteliste). Für `hidden`
gibt es **keinen** Builder-Setter — solche Parameter werden per direktem
Struct-Befüllen erzeugt.

In der Praxis existieren **beide Stile nebeneinander**: AudioSource/Smoothing/
PulseShape nutzen den ParamBuilder, die übrigen Module und Visualizer befüllen
die Structs überwiegend direkt (kosmetische Rest-Uneinheitlichkeit, kein
Verhaltensunterschied).

### 4.5 Gruppen-Ordnung: Stage-Tabelle statt String-Konvention

> **Behoben in Phase 4:** Die frühere Sortierung über numerische
> Gruppen-Namens-Präfixe (`"1. Audio"` … `"8. Peak Color"`) ist durch echte
> Stage-Zuordnung ersetzt.

Das ConfigPanel rendert die Gruppen **strikt in Stufen-Reihenfolge**: pro Gruppe
gilt die deklarierte `ModuleParamDesc::stage`; Gruppen-Keys sind `stage:N`,
Anzeige-Titel + Icon kommen aus der Stage-Tabelle im Panel (`stageInfo()`:
„1. Audio / Analysis" … „6. Post FX"). Für Parameter mit `stage == None`
(unmigrierte Visualizer — aktuell keine) greift als Fallback weiterhin die
Legacy-Ziffern-Präfix-Sortierung.

---

## 5. Sichtbarkeits-System (dependsOn)

### 5.1 Grundprinzip

```cpp
// Nur sichtbar, wenn render.type == 2 (NGon) oder 3 (Star)
p.id = "render.sides";
p.dependsOn = "render.type";
p.dependsValues = {2, 3};   // OR-Logik
```

### 5.2 Übersetzung der Referenzen

Beim Aggregieren von Modul-Parametern muss `dependsOn` **mit-übersetzt** werden
(Präfixieren bzw. `subIdToKey()`), sonst zeigt die Referenz ins Leere — der
Übersetzungsweg in §6 erledigt das zentral pro Visualizer.

### 5.3 Zweistufiges Muster (Channel-Mode-Pattern)

Für kanalabhängige Farb-Untergruppen (Waveform: `color.mono/left/right`;
Oscilloscope: `color.ch1`–`color.m2`) werden zwei Ebenen kombiniert:

1. **Gruppen-Ebene:** Parameter des Gradient-Moduls, die selbst kein `dependsOn`
   haben (`mode`, hidden-Params), bekommen die Kanal-Bedingung
   (`map.channelMode` bzw. `render.chN.visible`).
2. **Parameter-Ebene:** Parameter mit eigenem `dependsOn` (z. B. `solidColor` →
   `mode`) behalten ihre Abhängigkeit, übersetzt (`color.mono.mode`).

> **Behoben in Phase 4:** Das ConfigPanel blendet ganze Untergruppen über die
> `dependsOn`-Ketten aus (`solidColor` → `mode` → `channelMode`) — die früheren
> String-Heuristiken („SubGruppe beginnt mit ‚Line Color'", `dependsOn`-Ziel
> heißt „channelMode") sind entfernt.

---

## 6. Key-Routing (Pipeline-Schema)

> **Behoben in Phase 4:** Das frühere uneinheitliche Präfix-Routing
> (historisch gewachsene Präfixe `shape.`/`waveform.`/`scope.`/`eq.` mit
> unterschiedlichen Gruppen-/Order-Konventionen) ist durch das einheitliche
> Stufen-Key-Schema ersetzt. Alt-Keys existieren nur noch in den Alias-Maps
> für Preset-Migration ([Parameter_Key_Migration.md](Parameter_Key_Migration.md)).

`getParam`/`setParam` routen einheitlich: `audio.*` → AudioSourceModule; alle
übrigen Keys werden pro Visualizer aufgelöst:

| Visualizer | Mechanismus | Quelle |
|---|---|---|
| Waveform | Übersetzungstabelle `subIdKeyTable()` + Gradient-Präfix-Paare (`monoColor.` ↔ `color.mono.` …); `keyToSubId()`/`subIdToKey()` in beide Richtungen | `src/visualizers/WaveformVisualizer.cpp` |
| Oscilloscope | dito (Kanal-Expansion ch1–ch4/m1–m2 generiert; `ch1Color.` ↔ `color.ch1.` …) | `src/visualizers/OscilloscopeVisualizer.cpp` |
| Superscope | dito (Modul liefert seit 5.5 unpräfixierte Descs: `paramDescs("")`; `color.` ↔ `color.main.`) | `src/visualizers/SuperscopeVisualizer.cpp` |
| Pulsing | direkte Präfixierung: `render.*` → PulseShapeModule, `color.main.*` → ColorGradient (Sonderfall `beatBrightness`) | `src/visualizers/PulsingVisualizer.cpp` |
| Equalizer | direkte Key-Behandlung auf Config-Accessoren (`map.*`, `render.*`, `peak.*`, `particle.*`, `color.main.*`) | `src/visualizers/EqualizerVisualizer.cpp` |

Sub-ID↔Key-Tabellen und Alias-Maps speisen sich aus **denselben Tabellen** —
die Alias-Map entsteht mechanisch als `"<altpräfix>." + subId → neuerKey` plus
`audio.*`-Identitäts-Whitelist.

**Setter-Seiteneffekte** (Puffer-Resize-Kopplungen, Migration §7.2) hängen an den
neuen Keys: `map.sampleCount` (Waveform: Display-Puffer; Oscilloscope: alle
Kanäle), `map.pointCount` (Superscope), `map.bands` (Equalizer: Band-Puffer +
AudioSource-Sync).

---

## 7. Die 5 Visualizer im Ist-Stand

Registrierte IDs: `pulsing`, `equalizer`, `waveform`, `oscilloscope`, `superscope`
(siehe §10). Alle erben von `VisualizerBase`, alle nutzen `AudioSourceModule`,
alle sind auf das Pipeline-Schema migriert und registrieren im Konstruktor ihre
Legacy-Alias-Map. Vollständige Parameterlisten:
[Parameter_Reference.md](Parameter_Reference.md).

### 7.1 PulsingVisualizer (`pulsing`)

- **Eigenes Modul:** `PulseShapeModule` (Geometrie: Circle/Ring/NGon/Star) — hat
  seit Schritt 5.2 **eigene paramDescs** (ParamBuilder); der Visualizer
  präfixiert sie mit `render.`.
- **Gemeinsame Module:** AudioSourceModule, 1× ColorGradientModule (Handle
  `main`), BeatModule (Beat-Erkennung).
- **Stufen:** 1 (`audio.*`) · 3 (`color.main.*` inkl. Eigen-Parameter
  `beatBrightness`) · 4 (`render.*` inkl. Eigen-Parameter `beatReverse`).

### 7.2 WaveformVisualizer (`waveform`)

- **Eigenes Modul:** `WaveformModule` (Kanal-Layout, Linienstil, Fill)
  mit **3 eingebetteten ColorGradientModules** (Handles `mono`/`left`/`right`).
- **Stufen:** 1 · 2 (`map.channelMode`, `map.sampleCount`) · 3 · 4 (`render.*`,
  kanal-strukturiert nach E8) · 6 (`post.mirror.*`, `post.hold.*`).
- Besonderheiten: Channel-Mode-Pattern (§5.3); Hold/Fade über 3×
  `HoldFadeEffect` (§3.2); Display-Glättung über SmoothingModule-Instanzen
  (`processArrayPerIndex`, Config = `audio.smooth.*`) — der frühere separate
  Skalar `waveform.smoothing` ist entfallen (E3, Wert-Konverter in der
  Alias-Map); Puffer-Resize bei `map.sampleCount`.

### 7.3 OscilloscopeVisualizer (`oscilloscope`)

- **Eigenes Modul:** `OscilloscopeModule` (Timebase, Trigger, 4 Signal- + 2
  Math-Kanäle, Grid) mit **6 eingebetteten ColorGradientModules**
  (Handles `ch1`–`ch4`, `m1`, `m2`).
- **Stufen:** 1 · 2 (`map.timePerDiv`/`map.sampleCount`/`map.trigger.*`/
  `map.chN.*`/`map.mN.*`) · 3 · 4 (`render.chN.*`, `render.mN.*`, Grid,
  `render.triggerIndicator` — E4) · 6 (`post.trigger.fadeTime`).
- Besonderheiten: Trigger-System (Edge/Mode/Source/Fade), Math-Operationen;
  Kanal-Sichtbarkeit (`render.chN.visible`) steuert alle Kanal-Parameter;
  Resize **aller** Kanäle bei `map.sampleCount`; Display-Glättung per-Index
  via SmoothingModule (E3). Das frühere Phosphor-System war nie funktional
  und wurde entfernt (E7).

### 7.4 SuperscopeVisualizer (`superscope`)

- **Eigenes Modul:** `SuperscopeModule` (programmierbare Punkt-/Linien-Muster,
  15 Builtin-Presets) mit 1× ColorGradientModule (Handle `main`).
- **Stufen:** 1 · 2 (`map.pointCount`, `map.audioSource`, `map.audioChannel` —
  löst die frühere Doppel-„Audio"-Gruppe auf) · 3 · 4 (`render.preset` (E6),
  `render.mode`, Display) · 6 (`post.glow.*`, `post.hold.*`).
- Besonderheiten: das Modul kann seine Descs selbst präfixieren
  (`paramDescs(prefix)`), wird aber mit leerem Präfix aufgerufen und vom
  Visualizer übersetzt; Hold/Fade über `HoldFadeEffectT<SuperscopePoint-Frames>`;
  die handgestrickte EMA-Glättung ist durch das SmoothingModule ersetzt,
  die Beat-Erkennung durch das BeatModule.

### 7.5 EqualizerVisualizer (`equalizer`)

- **Eigenes Modul:** `EqualizerModule` (Bars, Peak-Physik, Partikel) mit 1×
  ColorGradientModule (Handle `main`). Das frühere parallele (nirgends
  aufgerufene) Modul-Parametersystem samt duplizierter Audio-Pipeline wurde in
  Schritt 0 **entfernt**.
- **Stufen:** 1 (`audio.*` **ohne** `bands`) · 2 (`map.bands` — ersetzt das
  `eq.bands`↔`audio.bands`-Paar, E2; `map.orientation`) · 3 (`color.main.domain`
  + Gradient) · 4 (`render.heightScale` — **neu**, E1; `render.barGap`) ·
  5 (`peak.*`, `particle.*` mit SubGruppen Peak Hold/Thickness/Spring
  Physics/Particles/Peak Color).
- Besonderheiten: `color.main.domain` (Position/Amplitude/Time/Beat) nutzt das
  Enum `GradientDomain`, das seit Schritt 0 in `ColorGradientModule.hpp` lebt
  (ColorSchemeModule entfernt).

---

## 8. Farb-/Gradient-System

### 8.1 ColorGradientModule

`include/visualizers/modules/ColorGradientModule.hpp` — verwaltet Farbe/Farbverläufe
für alle Visualizer. **Kein IModule** (erbt nicht), bietet aber dieselbe Parameter-API
(`paramDescs`/`getParam`/`setParam`) und wird von den Visualizern wie ein Modul
aggregiert (je Instanz ein Gradient-Handle `color.<handle>.`).

- **Modi** (`GradientMode`): `Solid` (0), `Linear` (1), `Radial` (2), `Outline` (3).
  Code-Default ist **Solid**, Startfarbe Magenta.
- **Stops:** 2–8 Color-Stops mit Midpoints (Übergangs-Schwerpunkt pro Segment);
  `sample(t)` interpoliert linear mit Midpoint-Remapping.
- **Builtin-Presets (10):** Fire, Forest, Galaxy, Ice, Lava, Monochrome, Neon, Ocean,
  Rainbow, Sunset (im Preset-Dropdown alphabetisch nach `[Custom]`).
- **User-Presets:** `.grad`-Dateien (JSON) im per `setUserPresetsDirectory()`
  gesetzten Verzeichnis; lazy geladen (Parsing über `JsonPresetParser`).
- **OpenGL-Anbindung:** `getUniformColors()` (8× vec4), `getUniformPositions()` (8×
  float), `getUniformMidpoints()` (7× float) für Shader-Uniforms.

### 8.2 Parameter des Moduls

Siehe [Parameter_Reference.md §2.3](Parameter_Reference.md) — Kernpunkte:
`mode`, `solidColor` (Solid/Outline), `angle` (Linear), `preset` + `editGradient`
(Linear/Radial), `outlineWidth` (Outline) sowie die **hidden**-Parameter
`gradientPresetName` und `gradientData` (Serialisierung, werden in Presets
mitgespeichert).

### 8.3 Gradient-Handles statt dynamic_cast

> **Behoben in Phase 4:** Die frühere `dynamic_cast`-Kaskade des ConfigPanels
> über alle 5 Visualizer-Typen (plus Kanal-Parsing aus der Parameter-ID) ist
> durch das **GradientHandle-Interface** ersetzt.

Jeder Visualizer exponiert seine Gradient-Instanzen über
`IVisualizer::gradients()` (§2.1). Das ConfigPanel nutzt die Handles für:

1. **GradientEditorDialog** (Stop-Editor, via `editGradient`-Button) — das
   Ziel-Modul wird über den `paramPrefix` des Handles gefunden, pro Kanal und
   ohne Casts. Damit sind auch **alle** Waveform-Gradients (Mono/Left/Right)
   und alle 6 Oscilloscope-Kanäle über den Editor erreichbar (die frühere
   Mono-only-Lücke ist geschlossen).
2. **GradientPresetDelegate** (Farbverlaufs-Vorschau im Preset-Dropdown) — das
   Modul kommt vom passenden Handle statt über String-Matching auf die
   Parameter-ID.
3. **Farbstreifen-Previews** je Handle in Stufe 3 (§9).

Neue Visualizer brauchen dafür keine ConfigPanel-Änderungen mehr — sie
deklarieren nur ihre Handles.

### 8.4 Preset-Lade-Logik

Gradients werden in Visualizer-Presets doppelt gesichert:

1. `gradientPresetName` — existiert der Name als Preset, wird er geladen und
   `gradientData` ignoriert.
2. `gradientData` — Fallback (Format `pos,r,g,b,a;pos,r,g,b,a;…`), wenn der Name
   `[Custom]` ist oder nicht gefunden wird.

Da Preset-Parameter in alphabetischer ID-Reihenfolge angewendet werden
(`gradientData` < `gradientPresetName`), prüft der `gradientData`-Handler, ob bereits
ein gültiges Preset geladen wurde.

---

## 9. Tap-Points und Stage-Previews

Neu in Phase 4 Schritt 6: **Live-Previews je Pipeline-Stufe** im ConfigPanel.

- **Deklaration im Visualizer:** `IVisualizer::tapPoints()` liefert Abgriffe
  auf Stufen-Daten — je TapPoint eine Stage, ein `sample`-Funktor (kopiert die
  aktuellen Daten; kostet nichts, solange er nicht aufgerufen wird) und ein
  `TapDisplay` (`Bars` = Band-Amplituden 0..1 als Mini-Equalizer, `Curve` =
  Sample-Daten −1..1 als Polyline). Kein Panel-Raten — der Visualizer
  deklariert die Darstellungsart.
- **Rendering:** `TapPreviewWidget` (`UI/widgets/TapPreviewWidget.{hpp,cpp}`),
  reines QPainter-Widget mit drei Modi: `Bars`, `Curve` und `ColorStrip`
  (Gradient-Handle als horizontaler Farbstreifen — für Stufe 3 wird pro Handle
  automatisch ein Streifen erzeugt; er folgt der Sichtbarkeit des
  `<handle>.mode`-Parameters und verschwindet mit seinem Kanal).
- **Bedienung:** Auge-Toggle im jeweiligen Stufen-Gruppen-Header; der Zustand
  wird per `QSettings` unter `configpanel/preview/<vizId>/<stage:N>` persistiert
  (Default: aus).
- **Kosten:** ein gemeinsamer 20-Hz-Timer (50 ms) pollt die `sample`-Funktoren —
  er läuft **nur**, solange mindestens ein Preview sichtbar ist
  (`updatePreviewTimer()`); `TapPreviewWidget` zeichnet nur bei geänderten Daten
  neu.

---

## 10. Registrierung

Die Registrierung läuft über die `VisualizerRegistry`
(`include/services/VisualizerRegistry.hpp`):

- **Tatsächlicher Mechanismus:** `src/visualizers/VisualizerAutoReg.cpp` definiert
  `initVisualizerDefaults(VisualizerRegistry&)` und registriert dort alle 5 Visualizer
  zentral mit `registerVisualizer(VisualizerDescriptor{id, name, description, category,
  order}, factory, false)`. Die Registry ruft diese Funktion beim ersten
  `instance()`-Zugriff auf; eine `extern`-Referenz in VisualizerRegistry.cpp erzwingt,
  dass der Linker die Datei auch aus statischen Libraries einbindet.
- **Makros:** `REGISTER_VISUALIZER(ID, NAME, DESC, TYPE)` (plus `_CATEGORY`- und
  `_FULL`-Varianten) existieren für Self-Registration per statischem Objekt, werden
  aktuell aber **nicht verwendet** — die Alt-Doku stellt sie fälschlich als den
  aktiven Weg dar.
- **Legacy-Aliases:** Jeder Visualizer registriert seine Alias-Map (alt→neu, §6)
  idempotent im **Konstruktor** beim `VisualizerPresetManager` — ein magic
  static würde `clearKeyAliases()` (Tests) nicht überleben.

Neuen Visualizer hinzufügen: Klasse von `VisualizerBase` ableiten, Header in
VisualizerAutoReg.cpp inkludieren, Registrierung in `initVisualizerDefaults()`
ergänzen (und Quelle in der `Source.cmake` eintragen — Source-Listen sind explizit);
`gradients()`/`tapPoints()` deklarieren, Keys nach dem Stufen-Schema (§1.1 der
Parameter_Reference) vergeben.

---

## 11. Altlasten-Bilanz (nach Phase 4)

Die in v1.0.0 dokumentierten Altlasten (Code-Analyse Session 29) sind durch
Phase 4 (Schritte 0–6, 2026-07-19) weitgehend aufgelöst:

**Behoben:**

1. **Zwei Parameter-Welten / String-Konvention:** `PipelineStage` +
   Stufen-Key-Schema ersetzt die `"1. …"`–`"8. …"`-Gruppen-Konvention; das
   ConfigPanel sortiert nach Stage-Tabelle (§4.5). Präfix-Wildwuchs
   (`shape.`/`waveform.`/doppeltes `scope.`/`eq.`) ist auf die 6 Stufen-Präfixe
   vereinheitlicht; Alt-Keys leben nur noch in den Alias-Maps.
2. **Totes EqualizerModule-Parametersystem:** entfernt (Schritt 0), inkl. der
   duplizierten Audio-Pipeline; der wirkungslose Modul-interne `gain` wurde
   durch den erreichbaren Parameter `render.heightScale` ersetzt (E1).
3. **ColorSchemeModule-Rest:** Modul entfernt; das weiterhin genutzte Enum
   `GradientDomain` lebt jetzt in `ColorGradientModule.hpp`.
4. **Kopierte Post-/Hilfslogik:** Hold/Fade zentral als `HoldFadeEffectT`
   (PostFxModule.hpp) — genutzt von Waveform und Superscope; Beat-Erkennung
   zentral im `BeatModule` (Pulsing, Superscope); tote
   `splitStereoData`/`resampleWaveform`-Kopien durch `AudioUtil.hpp` ersetzt;
   drei handgerollte Preset-Parser durch `JsonPresetParser` konsolidiert.
   Oscilloscope-Phosphor (nie funktional) ist gestrichen (E7).
5. **ConfigPanel-String-Heuristiken:** entfernt — Untergruppen kollabieren über
   `dependsOn`-Ketten (§5.3), Gradient-Editor/Preset-Delegate laufen über
   Gradient-Handles (§8.3), Preview-Darstellung ist per `TapDisplay`
   deklariert (§9). Die `dynamic_cast`-Kaskade ist weg.
6. **Preset-ID-Brüche:** User-Presets im Alt-Schema werden über
   `formatVersion` + Alias-Maps (Key + Wert-Konverter, E3) migriert;
   gespeichert wird nur noch im neuen Schema (v2).
7. **SmoothingModule nur eingebettet / PulseShapeModule ohne paramDescs:**
   SmoothingModule ist jetzt auch eigenständig als per-Index-Display-Glätter im
   Einsatz (`processArrayPerIndex`); PulseShapeModule liefert sein Schema
   selbst (Schritt 5.2). `ModuleConfigWidget` (UI, referenzlos) ist entfernt.
   Der Color-Variant-Index ist über `kParamValueColorIndex`-Helfer
   zentralisiert (§4.1).

**Noch offen (bewusst):**

- **Preset-Dropdown-Quirk:** deklarierte Defaults von `audio.preset`/
  `audio.smooth.preset` sind C++-Enum-Werte, keine Dropdown-Indizes
  (Parameter_Reference §1.4) — von der Key-Migration unabhängig, Bereinigung
  weiter offen.
- **Superscope-Defaults** werden (bis auf `render.preset`) aus den aktuellen
  Member-Werten deklariert (Parameter_Reference §1.4).
- **ParamBuilder vs. direktes Struct-Befüllen** koexistieren weiterhin (§4.4) —
  rein stilistisch.
- **Schritt 7 (Doku-Nachzug)** läuft; die spezialisierten Interfaces
  (`IProcessingModule` etc.) bleiben Vorhalt für das Node-Konzept.

---

## 12. Siehe auch

- [Parameter_Reference.md](Parameter_Reference.md) — SSOT-Referenz aller Parameter
  im Pipeline-Schema (alle 5 Visualizer, aus dem Code erhoben)
- [Parameter_Key_Migration.md](Parameter_Key_Migration.md) — Alt→Neu-Tabellen,
  Review-Entscheide E1–E8, Alias-Map-Regeln
- [Config_Pipeline_Concept.md](Config_Pipeline_Concept.md) /
  [Config_Pipeline_Umsetzungsplan.md](Config_Pipeline_Umsetzungsplan.md) —
  Konzept und Umsetzungsstand Phase 4
- [OpenGL_Context_Handling.md](OpenGL_Context_Handling.md) — Context-Tracking-Pattern
- [../../include/visualizers/Visualizers.md](../../include/visualizers/Visualizers.md) —
  header-nahe Modul-Doku (CppModuleDoc)
- [../ui/ConfigPanel_Guide.md](../ui/ConfigPanel_Guide.md) — UI-Erzeugung aus paramDescs
- [../presets/Preset_System.md](../presets/Preset_System.md) — Preset-Persistenz

---

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| **1.1.0** | **2026-07-19** | **Phase-4-Nachzug (Schritte 0–6): Pipeline-Stage-Schema (§1/§4.3/§4.5), Key-Routing über Sub-ID↔Key-Übersetzungstabellen statt Präfix-Wildwuchs (§6), Gradient-Handles statt dynamic_cast (§8.3), neuer §9 Tap-Points/Stage-Previews, Shared-Module BeatModule/HoldFadeEffectT/AudioUtil/JsonPresetParser (§3.2), Ist-Stand aller 5 Visualizer aktualisiert (§7), Altlasten-Abschnitt zur Bilanz aufgelöst (§11)** |
| 1.0.0 | 2026-07-18 | Initial: konsolidiert aus harvest/old_docs (IModule, Visualizer-Architecture-Reference, ColorGradientModule, README_MODULES), gegen Code verifiziert; ParamValue/ParamType korrigiert; Ist-Stand aller 5 Visualizer; Altlasten-Abschnitt für Phase 4 |
