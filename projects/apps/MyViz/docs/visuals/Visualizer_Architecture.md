# Visualizer-Architektur — IVisualizer, Modul-System und Parameter-Mechanik

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

Konsolidiert aus der Alt-Doku (`harvest/old_docs/`: IModule.md, Visualizer-Architecture-Reference.md,
ColorGradientModule.md, README_MODULES.md) und **gegen den Code verifiziert** (Stand 2026-07-18).
Bei Abweichungen zwischen Alt-Doku und Code gilt der Code — die wichtigsten Korrekturen sind
im Text markiert.

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Kern-Interfaces: IVisualizer und VisualizerBase](#2-kern-interfaces-ivisualizer-und-visualizerbase)
3. [Modul-System](#3-modul-system)
4. [Parameter-System](#4-parameter-system)
5. [Sichtbarkeits-System (dependsOn)](#5-sichtbarkeits-system-dependson)
6. [Präfix-Routing](#6-präfix-routing)
7. [Die 5 Visualizer im Ist-Stand](#7-die-5-visualizer-im-ist-stand)
8. [Farb-/Gradient-System](#8-farb-gradient-system)
9. [Registrierung](#9-registrierung)
10. [Bekannte Altlasten (Stand Phase-4-Vorbereitung)](#10-bekannte-altlasten-stand-phase-4-vorbereitung)
11. [Siehe auch](#11-siehe-auch)

---

## 1. Überblick

Jeder Visualizer in MyViz folgt derselben modularen Architektur:

- **Komposition statt Vererbung:** Visualizer setzen wiederverwendbare Module zusammen
  (AudioSourceModule, ColorGradientModule, je ein visualizer-eigenes Modul).
- **Parameter-getriebene Konfiguration:** Alle Einstellungen sind typisierte Parameter,
  erreichbar über `paramDescs()` / `getParam()` / `setParam()`. Das ConfigPanel generiert
  seine UI vollständig aus diesen Deskriptoren.
- **Präfix-Namespacing:** Modul-Parameter werden mit Präfixen versehen, um ID-Kollisionen
  zu vermeiden (z. B. `audio.gain`, `shape.color.mode`).
- **Bedingte Sichtbarkeit:** Parameter nutzen `dependsOn` + `dependsValues` für dynamisches
  Ein-/Ausblenden in der UI.
- **Preset-Portabilität:** Presets speichern Parameter über ihre IDs; Gradients zusätzlich
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
  └── ColorGradientModule (kein IModule, aber gleiche Parameter-API — siehe §8)
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

### 3.2 Standard-Module

| Modul | Zweck | Übliches Präfix |
|---|---|---|
| `AudioSourceModule` | FFT-Mapping (Linear/Log/Mel), dB-Normalisierung, Gain, eingebettetes Smoothing | `audio.` |
| `SmoothingModule` | SMA/EMA/WMA/DEMA-Glättung — **nur eingebettet** in AudioSourceModule genutzt | `audio.smooth.` |
| `ColorGradientModule` | Solid/Linear/Radial/Outline-Farbe, Multi-Stop-Gradients, Presets | visualizer-spezifisch (§6) |
| je Visualizer eigenes Modul | Domänenlogik (WaveformModule, OscilloscopeModule, SuperscopeModule, EqualizerModule, PulseShapeModule) | visualizer-spezifisch |

### 3.3 Kompositions-Muster

Visualizer betten Module als Member ein und aggregieren deren Parameter mit Präfix:

```cpp
std::vector<ModuleParamDesc> MyVisualizer::paramDescs() const
{
    std::vector<ModuleParamDesc> params;

    for (const auto& p : m_audioSource.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "1. Audio";

        // KRITISCH: dependsOn-Referenz mit-präfixieren!
        if (!prefixed.dependsOn.empty())
            prefixed.dependsOn = "audio." + prefixed.dependsOn;

        params.push_back(prefixed);
    }
    // ... eigene Parameter, weitere Module ...
    return params;
}
```

Module können selbst Module einbetten — Parameter-Pfade sind Punkt-getrennt:

```
"smooth.algorithm"      → SmoothingModule.algorithm
"audio.smooth.timeMs"   → AudioSourceModule → SmoothingModule → timeMs
```

---

## 4. Parameter-System

### 4.1 ParamValue

> **Korrektur gegenüber Alt-Doku:** Beide Alt-Referenzen dokumentierten falsche
> Varianten (mit `std::monostate`/`glm::vec2` bzw. mit `double`/`std::vector<float>`).
> Maßgeblich ist der Code (`IModule.hpp:70–79`):

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

Achtung: `Vec4f` und `Color4f` sind **derselbe Typ** (`std::array<float,4>`) — für Farben
muss deshalb explizit der Variant-Index 7 gesetzt werden (`out.emplace<7>(color)`).
Dieser Index ist an mehreren Stellen hart verdrahtet (u. a. ConfigPanel, Equalizer-
`peakColor.fixed`) — siehe [§10](#10-bekannte-altlasten-stand-phase-4-vorbereitung).

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

### 4.3 ModuleParamDesc

Vollständige Feldliste (`IModule.hpp:135–186`):

| Feld | Typ | Beschreibung |
|---|---|---|
| `id` | `std::string` | Eindeutig innerhalb des Moduls; Visualizer präfixieren |
| `displayName` | `std::string` | UI-Label |
| `group` | `std::string` | Aufklappbare Hauptgruppe (z. B. `"2. Shape"`) |
| `subGroup` | `std::string` | Geschachtelte Untergruppe (z. B. `"Line Color Mono"`) |
| `tooltip` | `std::string` | Hilfetext |
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
`defaultValue`, `enumOptions`, `widget`, `unit`, `format`, `order`, `advanced`,
`canBeInput`, `dependsOn` (Einzelwert oder Werteliste). Für `hidden` gibt es **keinen**
Builder-Setter — solche Parameter werden per direktem Struct-Befüllen erzeugt.

In der Praxis existieren **beide Stile nebeneinander**: AudioSource/Smoothing nutzen den
ParamBuilder, Pulsing/Waveform/Oscilloscope/Superscope/Equalizer befüllen die Structs
überwiegend direkt.

### 4.5 Gruppen-Konvention

Das ConfigPanel sortiert Hauptgruppen **alphabetisch als String** — die Reihenfolge
entsteht ausschließlich über numerische Präfixe:

```cpp
p.group = "1. Audio";
p.group = "2. Shape";
```

Es gibt (noch) keine echte Pipeline-Stage-Zuordnung; die Kette
Audio → Mapping → Farbe → Rendering → Peak/Partikel → Post existiert nur als
String-Konvention (Phase-4-Thema, siehe §10).

---

## 5. Sichtbarkeits-System (dependsOn)

### 5.1 Grundprinzip

```cpp
// Nur sichtbar, wenn shape.type == 2 (NGon) oder 3 (Star)
p.id = "shape.sides";
p.dependsOn = "shape.type";
p.dependsValues = {2, 3};   // OR-Logik
```

### 5.2 Präfixierung

Beim Aggregieren von Modul-Parametern muss `dependsOn` **mit-präfixiert** werden, sonst
zeigt die Referenz ins Leere (häufigster Fehler beim Einbau neuer Module — siehe
Kompositions-Muster in §3.3).

### 5.3 Zweistufiges Muster (Channel-Mode-Pattern)

Für kanalabhängige Farb-Untergruppen (Waveform: Mono/Left/Right; Oscilloscope: CH1–M2)
werden zwei Ebenen kombiniert:

1. **Gruppen-Ebene:** Parameter des Gradient-Moduls, die selbst kein `dependsOn` haben
   (`mode`, hidden-Params), bekommen beim Präfixieren die Kanal-Bedingung
   (`dependsOn = "channelMode"` bzw. `"chN.visible"`).
2. **Parameter-Ebene:** Parameter mit eigenem `dependsOn` (z. B. `solidColor` →
   `mode`) behalten ihre Abhängigkeit, präfixiert (`monoColor.mode`).

Das ConfigPanel behandelt dabei SubGruppen, deren Name mit „Line Color" beginnt, sowie
`dependsOn`-Ziele auf `channelMode` als Kanal-Ebene und blendet ganze Untergruppen aus —
**per String-Heuristik** (siehe §10).

---

## 6. Präfix-Routing

`getParam`/`setParam` routen per Präfix-Strip an die Module. Die Konventionen sind je
Visualizer historisch gewachsen und **uneinheitlich**:

| Visualizer | Präfixe → Ziel |
|---|---|
| Pulsing | `audio.*` → AudioSource · `shape.color.*` → ColorGradient (plus Sonderfall `beatBrightness`) · `shape.*` manuell auf Member |
| Waveform | `audio.*` → AudioSource · `waveform.*` → WaveformModule (darin: `monoColor./leftColor./rightColor.*` → 3× ColorGradient, Legacy-Alias `color.*` → Mono) |
| Oscilloscope | `audio.*` → AudioSource · `scope.*` → OscilloscopeModule (darin: `ch1Color.`…`m2Color.*` → 6× ColorGradient) |
| Superscope | `audio.*` → AudioSource · `scope.*` → SuperscopeModule (Modul erzeugt seine Descs bereits mit Präfix: `paramDescs("scope.")`; darin `scope.color.*` → 1× ColorGradient) |
| Equalizer | `audio.*` → AudioSource (ohne `bands`) · `color.domain` + `color.*` → GradientDomain/ColorGradient · `eq./peak./thickness./spring./particle./peakColor.*` **alles manuell** auf Config-Accessoren |

Weitere Abweichungen im Mechanismus (relevant für Phase 4):

- **Gruppen-Zuweisung:** Pulsing/Equalizer setzen `group` pro Parameter selbst;
  Waveform/Oscilloscope überschreiben pauschal beim Präfixieren; Superscope nur, falls leer.
- **Order-Offsets:** mal `100 + p.order`, mal `10 + p.order`, mal fortlaufend.
- **Sonderfälle an Param-IDs gekoppelt:** Puffer-Resize bei `waveform.sampleCount` /
  `scope.sampleCount`, Band-Sync bei `eq.bands`.

---

## 7. Die 5 Visualizer im Ist-Stand

Registrierte IDs: `pulsing`, `equalizer`, `waveform`, `oscilloscope`, `superscope`
(siehe §9). Alle erben von `VisualizerBase`, alle nutzen `AudioSourceModule`.
Vollständige Parameterlisten: [Parameter_Reference.md](Parameter_Reference.md).

### 7.1 PulsingVisualizer (`pulsing`)

- **Eigenes Modul:** `PulseShapeModule` (Geometrie: Circle/Ring/NGon/Star) — hat selbst
  **keine paramDescs**; der Visualizer deklariert alle Shape-Parameter manuell.
- **Gemeinsame Module:** AudioSourceModule, 1× ColorGradientModule (direkt als Member
  des Visualizers, nicht im Shape-Modul).
- **Gruppen:** „1. Audio", „2. Shape" (Farbe als SubGroup „Color" unter Shape).
- Besonderheiten: Beat-abhängige Rotation/Helligkeit (`shape.beatReverse`,
  `shape.color.beatBrightness`); Legacy-Background-API nur noch im Header.

### 7.2 WaveformVisualizer (`waveform`)

- **Eigenes Modul:** `WaveformModule` (Kanal-Layout, Linienstil, Fill, Effekte)
  mit **3 eingebetteten ColorGradientModules** (Mono/Left/Right).
- **Gruppen:** „1. Audio", „2. Waveform" mit SubGruppen Channel/Layout/Line/Fill/
  Effects/Line Color {Mono,Left,Right}.
- Besonderheiten: Channel-Mode-Pattern (§5.3); Hold/Fade-Trails („Effects");
  Puffer-Resize bei `waveform.sampleCount`; eigene skalare Glättung (`waveform.smoothing`)
  **zusätzlich** zum AudioSource-Smoothing.

### 7.3 OscilloscopeVisualizer (`oscilloscope`)

- **Eigenes Modul:** `OscilloscopeModule` (Timebase, Trigger, 4 Signal- + 2 Math-Kanäle,
  Grid) mit **6 eingebetteten ColorGradientModules** (CH1–CH4, M1, M2).
- **Gruppen:** „1. Audio", „2. Oscilloscope" mit SubGruppen Timebase/Trigger/CH1–CH4/
  M1–M2/Grid/Display/Line Color CH1–M2.
- Besonderheiten: Trigger-System (Edge/Mode/Source/Fade), Math-Operationen (A+B, A−B, …);
  Kanal-Sichtbarkeit steuert alle Kanal-Parameter (`chN.visible`); Resize bei
  `scope.sampleCount`.

### 7.4 SuperscopeVisualizer (`superscope`)

- **Eigenes Modul:** `SuperscopeModule` (programmierbare Punkt-/Linien-Muster,
  15 Builtin-Presets von Horizontal Scope bis Hypocycloid) mit 1× ColorGradientModule.
- **Gruppen:** „1. Audio", „2. Superscope" mit SubGruppen Preset/Render/Audio/Color/
  Glow/Hold/Fade/Display.
- Besonderheiten: Einziger Visualizer, dessen Modul das Präfix selbst einbaut
  (`paramDescs(prefix)`); **doppelte Audio-Semantik** (global `audio.*` +
  SubGroup „Audio" mit `scope.audioSource`/`scope.audioChannel`); Glow und Hold/Fade
  als eigene Effekte; EMA-Glättung von Hand implementiert (dupliziert SmoothingModule).

### 7.5 EqualizerVisualizer (`equalizer`)

- **Eigenes Modul:** `EqualizerModule` (Bars, Peak-Physik, Partikel) mit 1×
  ColorGradientModule. Achtung: das Modul enthält ein voll ausgebautes, aber **nirgends
  aufgerufenes** eigenes Parameter-System und eine tote Audio-Pipeline (§10).
- **Gruppen:** „1. Audio", „2. Equalizer", „3. Color", „4. Peak Hold",
  „5. Peak Thickness", „6. Spring Physics", „7. Particles", „8. Peak Color" —
  strukturell am nächsten an der Ziel-Pipeline von Phase 4.
- Besonderheiten: `audio.bands` wird aus der Audio-Gruppe **herausgefiltert**;
  stattdessen `eq.bands` (8–256), das den AudioSource-Wert synchron hält.
  `color.domain` (Position/Amplitude/Time/Beat) nutzt das Enum `GradientDomain` —
  der letzte lebende Rest des ColorSchemeModules.

---

## 8. Farb-/Gradient-System

### 8.1 ColorGradientModule

`include/visualizers/modules/ColorGradientModule.hpp` — verwaltet Farbe/Farbverläufe
für alle Visualizer. **Kein IModule** (erbt nicht), bietet aber dieselbe Parameter-API
(`paramDescs`/`getParam`/`setParam`) und wird von den Visualizern wie ein Modul
aggregiert.

- **Modi** (`GradientMode`): `Solid` (0), `Linear` (1), `Radial` (2), `Outline` (3).
  (Alt-Doku kannte `Outline` noch nicht bzw. nannte Linear als Default — der
  Code-Default ist **Solid**, Startfarbe Magenta.)
- **Stops:** 2–8 Color-Stops mit Midpoints (Übergangs-Schwerpunkt pro Segment);
  `sample(t)` interpoliert linear mit Midpoint-Remapping.
- **Builtin-Presets (10):** Fire, Forest, Galaxy, Ice, Lava, Monochrome, Neon, Ocean,
  Rainbow, Sunset (im Preset-Dropdown alphabetisch nach `[Custom]`).
- **User-Presets:** `.grad`-Dateien (JSON) im per
  `setUserPresetsDirectory()` gesetzten Verzeichnis; lazy geladen.
- **OpenGL-Anbindung:** `getUniformColors()` (8× vec4), `getUniformPositions()` (8×
  float), `getUniformMidpoints()` (7× float) für Shader-Uniforms.

### 8.2 Parameter des Moduls

Siehe [Parameter_Reference.md §2.3](Parameter_Reference.md) — Kernpunkte:
`mode`, `solidColor` (Solid/Outline), `angle` (Linear), `preset` + `editGradient`
(Linear/Radial), `outlineWidth` (Outline) sowie die **hidden**-Parameter
`gradientPresetName` und `gradientData` (Serialisierung, werden in Presets
mitgespeichert).

### 8.3 Zweigleisigkeit: generische Parameter vs. GradientEditorDialog

Farbe läuft heute über **zwei parallele Wege**:

1. **Generische Parameter** im ConfigPanel (mode/solidColor/angle/preset …) — werden wie
   alle anderen Parameter aus `paramDescs()` gebaut. Preset-Dropdowns erhalten eine
   Farbverlaufs-Vorschau (`GradientPresetDelegate`), aktiviert per String-Match auf
   die Parameter-ID („preset" + „color/Color").
2. **GradientEditorDialog** (Stop-Editor) — wird über den `editGradient`-Button
   geöffnet und arbeitet **direkt auf dem Modul-Zeiger**. Das ConfigPanel ermittelt das
   Ziel-Modul über eine `dynamic_cast`-Kaskade über alle 5 Visualizer-Typen; beim
   Oscilloscope wird der Kanal aus der Parameter-ID geparst.

Bekannte Lücke: Beim **Waveform ist nur der Mono-Gradient** über den Editor erreichbar
(Left/Right nicht). Jeder neue Visualizer erfordert heute ConfigPanel-Änderungen —
Phase 4 soll das über ein Gradient-Handle-Interface abstrahieren (§10).

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

## 9. Registrierung

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

Neuen Visualizer hinzufügen: Klasse von `VisualizerBase` ableiten, Header in
VisualizerAutoReg.cpp inkludieren, Registrierung in `initVisualizerDefaults()`
ergänzen (und Quelle in der `Source.cmake` eintragen — Source-Listen sind explizit).

---

## 10. Bekannte Altlasten (Stand Phase-4-Vorbereitung)

Ehrliche Bestandsaufnahme (Code-Analyse Session 29, 2026-07-18). Phase 4
(„Config-Pipeline vereinheitlichen") soll diese Punkte auflösen; Blaupause:
`harvest/config-pipeline/README.md`.

1. **Zwei Parameter-Welten:** Deskriptoren entstehen teils per ParamBuilder
   (AudioSource/Smoothing), teils per direktem Struct-Befüllen (alle Visualizer);
   Präfix-/Gruppen-/Order-Konventionen sind uneinheitlich (§6). Pipeline-Reihenfolge
   existiert nur als String-Konvention `"1. …"`–`"8. …"`.
2. **Totes EqualizerModule-Parametersystem:** `EqualizerModule` besitzt vollständige
   `paramDescs`/`getParam`/`setParam` **ohne einen einzigen Aufrufer** — der
   EqualizerVisualizer geht direkt auf Config-Accessoren. Zusätzlich dupliziert das
   Modul die Audio-Pipeline (eigene mapSpectrum/EMA/dB-Normalisierung), obwohl der
   Visualizer das AudioSourceModule nutzt.
3. **ColorSchemeModule-Rest:** Als Farbsystem tot; nur das Enum `GradientDomain`
   (Position/Amplitude/Time/Beat) wird noch vom Equalizer verwendet.
4. **Kein PostProcessModule:** Hold/Fade/Phosphor/Mirror/Glow sind je Visualizer einzeln
   implementiert (identische `alpha = 1 - age/fadeTime`-Logik mehrfach kopiert);
   Konzept liegt in `harvest/old_docs/concepts/PostProcessModule-Concept.md`.
5. Weitere Befunde in Kurzform: `SmoothingModule` nur eingebettet nutzbar;
   `ModuleConfigWidget` (UI) ohne Referenzen; `PulseShapeModule` ohne paramDescs;
   Color-Variant-Index 7 hart verdrahtet; ConfigPanel-Heuristiken über Strings
   (Gruppen-Präfixe, „Line Color", „preset+color") brechen still bei Umbenennungen;
   Preset-Snapshots hängen an Param-IDs (ID-Änderungen invalidieren User-Presets —
   Migrationspfad einplanen).

---

## 11. Siehe auch

- [Parameter_Reference.md](Parameter_Reference.md) — SSOT-Referenz aller Parameter
  (alle 5 Visualizer, aus dem Code erhoben)
- [OpenGL_Context_Handling.md](OpenGL_Context_Handling.md) — Context-Tracking-Pattern
- [../../include/visualizers/Visualizers.md](../../include/visualizers/Visualizers.md) —
  header-nahe Modul-Doku (CppModuleDoc)
- [../ui/ConfigPanel_Guide.md](../ui/ConfigPanel_Guide.md) — UI-Erzeugung aus paramDescs
- [../presets/Preset_System.md](../presets/Preset_System.md) — Preset-Persistenz
- `harvest/config-pipeline/README.md` (Repo-Root) — Blaupause/Anforderungen Phase 4

---

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| **1.0.0** | **2026-07-18** | **Initial: konsolidiert aus harvest/old_docs (IModule, Visualizer-Architecture-Reference, ColorGradientModule, README_MODULES), gegen Code verifiziert; ParamValue/ParamType korrigiert; Ist-Stand aller 5 Visualizer; Altlasten-Abschnitt für Phase 4** |
