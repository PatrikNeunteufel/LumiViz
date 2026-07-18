# Preset-System — Speichern, Laden und Verwalten von Visualizer-Konfigurationen

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## Inhaltsverzeichnis

1. [Überblick und Hierarchie](#1-überblick-und-hierarchie)
2. [Speicherorte](#2-speicherorte)
3. [Dropdown-Verhalten](#3-dropdown-verhalten)
4. [Bedienung](#4-bedienung)
5. [API: VisualizerPresetManager](#5-api-visualizerpresetmanager)
6. [Der float-Vertrag](#6-der-float-vertrag)
7. [Gradient-Serialisierung](#7-gradient-serialisierung)
8. [Versteckte Parameter](#8-versteckte-parameter)
9. [Troubleshooting](#9-troubleshooting)
10. [Siehe auch](#10-siehe-auch)

---

## 1. Überblick und Hierarchie

Das Preset-System speichert Konfigurationen auf zwei Ebenen:

```
┌─────────────────────────────────────────────────────────┐
│              Visualizer-Preset (.lvp)                   │
│              speichert ALLE Parameter                   │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │
│  │   Audio     │  │  Smoothing  │  │    Gradient     │  │
│  │  (.audio)   │  │  (.smooth)  │  │    (.grad)      │  │
│  └─────────────┘  └─────────────┘  └─────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

| Typ | Endung | Verwaltet von | Inhalt |
|-----|--------|---------------|--------|
| Visualizer | `.lvp` | `VisualizerPresetManager` | Komplette Visualizer-Konfiguration (alle Parameter aus `paramDescs()`) |
| Audio | `.audio` | `AudioSourceModule` | FFT-Settings + eingebettetes Smoothing |
| Smoothing | `.smooth` | `SmoothingModule` | Glättungsparameter |
| Gradient | `.grad` | `ColorGradientModule` | Farbverlauf (Mode, Winkel, Stops, Midpoints) |

Ein Visualizer-Preset schneidet quer durch alle Module: Beim Erfassen (`capturePreset()`)
werden **alle** Parameter des Visualizers per `paramDescs()`/`getParam()` eingesammelt —
inklusive versteckter Parameter. Die Modul-Presets sind davon unabhängige, kleinere
Einheiten, die einzelne Modul-Zustände benennbar machen.

Alle Dateiformate im Detail: [FileFormat_Reference.md](FileFormat_Reference.md).

---

## 2. Speicherorte

Die Verzeichnisse werden in `Application::run()` **nach** dem Erzeugen der `QApplication`
und **vor** dem `MainWindow` gesetzt (Organisation: „MyViz Project", App: „MyViz"):

```cpp
QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

lumi::modules::ColorGradientModule::setUserPresetsDirectory((appData + "/presets/gradients").toStdString());
lumi::modules::SmoothingModule::setUserPresetsDirectory((appData + "/presets/smoothing").toStdString());
lumi::modules::AudioSourceModule::setUserPresetsDirectory((appData + "/presets/audio").toStdString());
```

Der `VisualizerPresetManager` nutzt per Default ebenfalls `AppDataLocation` + `/presets`
und legt `.lvp`-Dateien unter `presets/visuals/<visualizerId>/` ab
(**nicht** `visualizer/` — das stand so nur in der Alt-Doku).

Resultierende Struktur (Windows; Linux/macOS analog unter der jeweiligen
`AppDataLocation`, z. B. `~/.local/share/MyViz Project/MyViz/`):

```
%APPDATA%\MyViz Project\MyViz\presets\
├── visuals\
│   └── {visualizerId}\          z. B. pulsing\, waveform\
│       └── {Name}.lvp
├── audio\
│   └── {Name}.audio
├── smoothing\
│   └── {Name}.smooth
└── gradients\
    └── {Name}.grad
```

---

## 3. Dropdown-Verhalten

### 3.1 Visualizer-Preset-Dropdown (ConfigPanel)

```
Index 0:  [Custom]      ← Zustand „manuell geändert" (keine Aktion bei Auswahl)
Index 1:  Default       ← ruft resetToDefaults() auf (KEINE Datei!)
Index 2:  ---           ← Separator, deaktiviert (nur wenn User-Presets existieren)
Index 3+: User-Presets  ← .lvp-Dateien, alphabetisch
```

Wichtig: **„Default" ist kein gespeichertes Preset**, sondern der hartkodierte
Ausgangszustand des Visualizers (`resetToDefaults()`). Es gibt keine mitgelieferten
`.lvp`-Dateien — die Alt-Doku behauptete das fälschlich.

### 3.2 Modul-Preset-Dropdowns (.audio / .smooth / .grad)

Die Modul-Dropdowns speisen sich aus `presetNames()` des jeweiligen Moduls:

```
[Custom]           ← aktueller (veränderter) Zustand
<Builtin-Presets>  ← hartkodiert im Modul (z. B. Gradients: Fire, Galaxy, Monochrome, …)
---                ← Separator (nur wenn User-Presets existieren)
<User-Presets>     ← von Platte geladene Dateien (lazy beim ersten Zugriff)
```

### 3.3 [Custom]-Auto-Switch

Sobald ein Parameter manuell geändert wird, springt das betroffene Dropdown auf
`[Custom]` (Index 0). Die Module setzen dabei intern `m_currentPreset = "[Custom]"`;
das ConfigPanel synchronisiert das Dropdown mit `QSignalBlocker`, um Signal-Loops zu
vermeiden. Ein Visualizer-Parameterwechsel schaltet auch das Visualizer-Preset-Dropdown
auf `[Custom]`.

---

## 4. Bedienung

### 4.1 Preset speichern

1. Parameter im ConfigPanel wie gewünscht einstellen.
2. **Save** neben dem Preset-Dropdown klicken.
3. Namen eingeben und bestätigen. Existiert der Name bereits, fragt die App vor dem
   Überschreiben nach.

**Hinweis zu Namen:** Der Name wird 1:1 zum Dateinamen (`<Name>.lvp`). Die UI validiert
aktuell **nicht** auf Sonderzeichen — Zeichen wie `/`, `\` oder `.` daher vermeiden
(die Alt-Doku zeigte eine Validierung, die im Code nicht existiert).

Modul-Presets (Audio/Smoothing/Gradient) haben eigene **Save**-Buttons neben ihrem
jeweiligen Dropdown.

### 4.2 Preset laden

Preset im Dropdown auswählen — die Parameter werden sofort angewendet und die UI
synchronisiert. Schlägt das Laden fehl, springt das Dropdown zurück auf **Default**.

### 4.3 Preset löschen

**Delete**-Button; funktioniert nur für User-Presets (Index ≥ 3), mit
Bestätigungsdialog. Die Löschung ist permanent (kein Papierkorb). Builtin-Modul-Presets
(z. B. Gradient „Fire") können nicht gelöscht werden.

### 4.4 Presets teilen

Preset-Dateien sind einzelne JSON-Dateien und können direkt kopiert werden:
Datei aus dem [Speicherort](#2-speicherorte) in den passenden Unterordner des
Zielsystems legen, App neu starten (bzw. Liste aktualisieren). Für Gradients sorgt die
[Zwei-Parameter-Serialisierung](#7-gradient-serialisierung) dafür, dass `.lvp`-Presets
auch ohne das referenzierte `.grad`-File funktionieren.

---

## 5. API: VisualizerPresetManager

Dateien: `include/visualizers/VisualizerPresetManager.hpp`,
`src/visualizers/VisualizerPresetManager.cpp` (Namespace `lumi`).

### 5.1 Datenstruktur

```cpp
struct VisualizerPreset
{
    QString name;              // Pflicht
    QString visualizerId;      // Pflicht — muss zum Ziel-Visualizer passen
    QString description;       // optional
    QString author;            // optional
    int     version = 1;       // Preset-Format-Version

    std::map<std::string, modules::ParamValue> parameters;

    bool isValid() const;      // name und visualizerId nicht leer
};
```

### 5.2 Methoden

| Methode | Rückgabe | Beschreibung |
|---------|----------|--------------|
| `setPresetsDirectory(path)` | `void` | Basisverzeichnis (Default: `AppDataLocation` + `/presets`) |
| `availablePresets(vizId)` | `QStringList` | `.lvp`-Namen im Visualizer-Ordner, alphabetisch |
| `presetExists(vizId, name)` | `bool` | Datei-Existenzprüfung |
| `loadPreset(vizId, name)` | `optional<VisualizerPreset>` | Laden; `nullopt` bei IO-/Parse-Fehler oder ID-Mismatch |
| `savePreset(preset)` | `bool` | Speichern (legt Verzeichnis bei Bedarf an) |
| `deletePreset(vizId, name)` | `bool` | Datei löschen |
| `renamePreset(vizId, alt, neu)` | `bool` | Laden → umbenennen → speichern → alte Datei löschen |
| `capturePreset(viz, name, desc)` | `VisualizerPreset` | Snapshot über `paramDescs()` + `getParam()` |
| `applyPreset(viz, preset)` | `bool` | Anwenden via `setParam()`; `true` nur wenn alle Parameter gesetzt wurden |
| `presetExtension()` | `QString` | statisch, `".lvp"` |

Fehlerbehandlung: keine Exceptions — Fehler werden geloggt (`BasicLogger`) und über
`false`/`nullopt` gemeldet. Nicht thread-safe; vom UI-Thread aufrufen.

### 5.3 Anwendungs-Reihenfolge in applyPreset()

`applyPreset()` prüft zuerst strikt `preset.visualizerId == visualizer->visualizerId()`
und wendet dann die Parameter **in zwei Durchgängen** an:

1. Alle Parameter, deren ID `.preset` enthält (z. B. `shape.color.preset`) — diese
   laden Preset-Defaults in die Module.
2. Alle übrigen Parameter — diese überschreiben die Preset-Defaults mit den konkret
   gespeicherten Werten.

Unbekannte Parameter (z. B. aus neueren Versionen) schlagen in `setParam()` fehl,
werden geloggt und gezählt; fehlende Parameter behalten ihre Defaults
(Vorwärts-/Rückwärts-Kompatibilität).

### 5.4 Typische Verwendung (aus ConfigPanel)

```cpp
// Speichern
auto preset = m_presetManager->capturePreset(m_visualizer, name);
m_presetManager->savePreset(preset);

// Laden
auto preset = m_presetManager->loadPreset(vizId, presetName);
if (preset)
{
    m_presetManager->applyPreset(m_visualizer, *preset);
    syncFromVisualizer();  // UI nachziehen
}
```

---

## 6. Der float-Vertrag

**Kernregel:** JSON unterscheidet nicht zwischen Int und Float. Der Loader
(`jsonToPreset()`) legt deshalb **alle numerischen Werte als `float`** in
`preset.parameters` ab — auch Werte, die als Int/Enum gespeichert wurden:

```cpp
// VisualizerPresetManager::jsonToPreset()
else if (jsonValue.isDouble())
{
    // JSON doesn't distinguish int/float - always store as float
    // The setParam implementations should handle both types
    preset.parameters[...] = static_cast<float>(jsonValue.toDouble());
}
```

Daraus folgt die **verbindliche Anforderung an alle `setParam()`-Implementierungen**
(Module und Visualizer): Int- und Enum-Parameter müssen zusätzlich `float`
akzeptieren, sonst werden sie beim Preset-Laden stillschweigend ignoriert
(Log: `PresetManager: Failed to set param '…'`).

```cpp
// Muster für Int-/Enum-Parameter:
if (auto* v = std::get_if<int>(&value))   { setSampleCount(*v); return true; }
if (auto* v = std::get_if<float>(&value)) { setSampleCount(static_cast<int>(*v)); return true; }

// Muster für Float-Parameter (Gegenrichtung ebenfalls abdecken):
if (auto* v = std::get_if<float>(&value)) { setTimePerDiv(*v); return true; }
if (auto* v = std::get_if<int>(&value))   { setTimePerDiv(static_cast<float>(*v)); return true; }
```

Der Vertrag ist per Unit-Test abgesichert:
`tests/unit/UnitTests/test_VisualizerPresetManager.cpp` speichert `int`-Parameter und
prüft, dass sie **als float** zurückkommen (Roundtrip-Test). Er ist zugleich eine
dokumentierte Anforderung an die Config-Pipeline (Phase 4).

---

## 7. Gradient-Serialisierung

Farbverläufe werden in `.lvp`-Presets **redundant** über zwei versteckte
String-Parameter des `ColorGradientModule` gespeichert:

| Parameter | Zweck |
|-----------|-------|
| `gradientPresetName` | Name des Gradient-Presets (Builtin/User) oder `[Custom]` |
| `gradientData` | Roh-Stops als Fallback: `pos,r,g,b,a;pos,r,g,b,a;…` (4 Nachkommastellen) |

**Lade-Priorität** (beide Parameter werden beim Anwenden gesetzt):

1. `gradientPresetName`: Existiert das Preset auf dem System → laden, fertig.
   Ist es `[Custom]` oder unbekannt → nur Name merken.
2. `gradientData`: Wird **nur** angewendet, wenn Schritt 1 kein Preset laden konnte
   (Custom oder fehlendes Preset). Parsen erfordert ≥ 2 gültige Stops; leerer String
   ist gültig (Stops bleiben unverändert).

Damit funktionieren geteilte `.lvp`-Dateien auch auf Systemen, auf denen das
referenzierte User-Gradient-Preset nicht existiert.

---

## 8. Versteckte Parameter

Parameter mit `hidden = true` werden

- **nicht** im ConfigPanel angezeigt,
- **wohl aber** beim Speichern erfasst und beim Laden angewendet.

Aktuell deklariert das `ColorGradientModule` zwei versteckte Parameter
(`gradientPresetName`, order 998 und `gradientData`, order 999, beide `ParamType::String`).

```cpp
ModuleParamDesc p;
p.id     = "internalData";
p.type   = ParamType::String;
p.hidden = true;   // nicht in der UI, aber im Preset
p.order  = 999;    // ans Ende der Parameterliste
```

---

## 9. Troubleshooting

| Symptom | Ursache | Lösung |
|---------|---------|--------|
| Preset lädt nicht (`nullopt`) | `visualizerId` passt nicht, JSON defekt, Datei nicht lesbar | ID im `header` prüfen; JSON validieren (keine Kommentare!); Rechte prüfen |
| Numerischer Parameter wird ignoriert (`Failed to set param`) | `setParam()` akzeptiert nur einen numerischen Typ | [float-Vertrag](#6-der-float-vertrag) umsetzen |
| Preset fehlt im Dropdown | falscher Ordner/Endung, Liste nicht aktualisiert | Speicherort und Endung prüfen; App neu starten |
| Speichern schlägt fehl | Verzeichnis nicht anlegbar, keine Schreibrechte, Name leer | Log prüfen (`PresetManager: Cannot write to …`) |
| Gradient nicht wiederhergestellt | `gradientData` fehlt/defekt (< 2 Stops, falsches Format) | Beide Gradient-Parameter im `.lvp` prüfen |
| Delete wirkungslos | Builtin-Preset oder `[Custom]`/`Default`/Separator gewählt | Nur User-Presets (Index ≥ 3) sind löschbar |
| Werte nach Laden „falsch" | Preset aus anderer Version, Parameter-IDs geändert | Preset neu erfassen; ID-Änderungen sind Breaking Changes |

---

## 10. Siehe auch

- [FileFormat_Reference.md](FileFormat_Reference.md) — alle Dateiformate (.lvp, .smooth, .audio, .grad)
- [Layout_Persistence.md](Layout_Persistence.md) — Persistenz des UI-Layouts (QSettings)
- [../visuals/Parameter_Reference.md](../visuals/Parameter_Reference.md) — Parameter-IDs und -Typen
- [../ui/ConfigPanel_Guide.md](../ui/ConfigPanel_Guide.md) — Bedienung des ConfigPanels
