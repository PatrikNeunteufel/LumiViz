# Preset System — File-basierte Konfigurationsverwaltung

> **Version:** 1.0.0  
> **Datum:** 2026-01-02  
> **Typ:** CppModuleDoc  
> **Status:** Stabil  
> **Modul:** lumi::VisualizerPresetManager  
> **Dateien:** VisualizerPresetManager.hpp, VisualizerPresetManager.cpp  
> **Namespace:** lumi  
> **Abhängigkeiten:** Qt6, nlohmann/json  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Interna](#5-interna)
6. [Thread-Sicherheit](#6-thread-sicherheit)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Das Preset-System ermöglicht das **Speichern und Laden von Konfigurationen** auf verschiedenen Ebenen: komplette Visualizer-Presets sowie einzelne Modul-Presets.

### 1.2 Verantwortlichkeiten

- Visualizer-Presets (.lvp) verwalten
- Modul-Preset-Verzeichnisse initialisieren
- Dropdown-Struktur mit [Custom], Default, Separator, User-Presets

### 1.3 Nicht-Verantwortlichkeiten

- Keine UI-Widgets (→ ConfigPanel)
- Keine Modul-interne Preset-Logik (→ jeweiliges Modul)

### 1.4 Preset-Hierarchie

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Visualizer Preset (.lvp)                          │
│                    Speichert ALLE Parameter                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────────────┐ │
│  │ Audio Preset   │  │ Smoothing      │  │ Gradient Preset        │ │
│  │ (.audio)       │  │ Preset         │  │ (.grad)                │ │
│  │                │  │ (.smooth)      │  │                        │ │
│  └────────────────┘  └────────────────┘  └────────────────────────┘ │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6 | Extern | QStandardPaths, QDir |
| nlohmann/json | Extern | JSON Serialisierung |
| IVisualizer | Intern | Parameter-Zugriff |
| std::filesystem | Standard | Datei-Operationen |

---

## 3. API

### 3.1 Preset-Typen

| Typ | Dateiendung | Speicherort | Beschreibung |
|-----|-------------|-------------|--------------|
| Visualizer | `.lvp` | `presets/visualizer/{vizId}/` | Komplette Visualizer-Config |
| Smoothing | `.smooth` | `presets/smoothing/` | SmoothingModule Einstellungen |
| Audio | `.audio` | `presets/audio/` | AudioSourceModule Einstellungen |
| Gradient | `.grad` | `presets/gradients/` | ColorGradientModule Einstellungen |

### 3.2 Dropdown-Struktur

```
Index 0: [Custom]      ← Manuell geändert
Index 1: Default       ← Hardcoded Defaults
Index 2: ---           ← Separator (disabled)
Index 3+: User-Presets ← Alphabetisch sortiert
```

### 3.3 VisualizerPresetManager

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `setPresetsDirectory(path)` | `QString` | `void` | Basisverzeichnis setzen |
| `availablePresets(vizId)` | `QString` | `QStringList` | Verfügbare Presets |
| `savePreset(preset)` | `VisualizerPreset` | `bool` | Preset speichern |
| `loadPreset(vizId, name)` | `QString`, `QString` | `optional<Preset>` | Preset laden |
| `deletePreset(vizId, name)` | `QString`, `QString` | `bool` | Preset löschen |
| `applyPreset(viz, preset)` | `IVisualizer*`, `Preset` | `void` | Preset anwenden |
| `capturePreset(viz, name)` | `IVisualizer*`, `QString` | `VisualizerPreset` | Aktuellen State erfassen |

### 3.4 VisualizerPreset Struct

```cpp
struct VisualizerPreset
{
    QString visualizerId;
    QString name;
    QString version;
    std::map<std::string, ParamValue> params;
};
```

---

## 4. Verwendung

### 4.1 Initialisierung (Application)

```cpp
void Application::init()
{
    QString appData = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    
    // Visualizer Presets
    m_presetManager->setPresetsDirectory(appData + "/presets/visualizer");
    
    // Module Presets
    ColorGradientModule::setUserPresetsDirectory(
        (appData + "/presets/gradients").toStdString());
    SmoothingModule::setUserPresetsDirectory(
        (appData + "/presets/smoothing").toStdString());
    AudioSourceModule::setUserPresetsDirectory(
        (appData + "/presets/audio").toStdString());
}
```

### 4.2 Preset speichern

```cpp
// In ConfigPanel
void ConfigPanel::onSavePresetClicked()
{
    QString name = QInputDialog::getText(this, "Save Preset", "Name:");
    
    if (name.isEmpty()) return;
    
    // Validierung
    if (name.contains('/') || name.contains('\\') || name.contains('.'))
    {
        QMessageBox::warning(this, "Invalid Name",
            "Name cannot contain /, \\, or .");
        return;
    }
    
    auto preset = m_presetManager->capturePreset(m_visualizer, name);
    m_presetManager->savePreset(preset);
    refreshPresetList();
}
```

### 4.3 Preset laden

```cpp
void ConfigPanel::onPresetSelected(int index)
{
    if (index == 0) return;  // [Custom] - keine Aktion
    
    if (index == 1)  // Default
    {
        m_visualizer->resetToDefaults();
        syncFromVisualizer();
        return;
    }
    
    if (index == 2) return;  // Separator
    
    // User-Preset (Index 3+)
    QString name = m_presetCombo->currentText();
    auto preset = m_presetManager->loadPreset(m_vizId, name);
    
    if (preset)
    {
        m_presetManager->applyPreset(m_visualizer, *preset);
        syncFromVisualizer();
    }
}
```

### 4.4 [Custom] Auto-Switch

```cpp
void ConfigPanel::onParamChanged(const std::string& paramId,
                                  const ParamValue& value)
{
    m_visualizer->setParam(paramId, value);
    
    // Visualizer-Preset auf [Custom]
    if (m_presetCombo->currentIndex() != 0)
    {
        QSignalBlocker blocker(m_presetCombo);
        m_presetCombo->setCurrentIndex(0);
    }
    
    // Zugehöriges Modul-Preset aktualisieren
    updateRelatedPresetWidget(paramId);
}
```

---

## 5. Interna

### 5.1 Speicherort

```
%APPDATA%/LumiViz/presets/           (Windows)
~/.local/share/LumiViz/presets/      (Linux)
~/Library/Application Support/LumiViz/presets/  (macOS)
├── visualizer/
│   └── pulsing/
│       ├── Energetic.lvp
│       └── Chill.lvp
├── audio/
│   └── HighGain.audio
├── smoothing/
│   └── UltraSmooth.smooth
└── gradients/
    └── Sunset.grad
```

### 5.2 Visualizer Preset Format (.lvp)

```json
{
  "visualizerId": "pulsing",
  "version": "1.0",
  "name": "Energetic",
  "params": {
    "audio.scale": 1,
    "audio.bands": 64,
    "audio.gain": 1.5,
    "audio.smooth.algorithm": 2,
    "audio.smooth.timeMs": 30.0,
    "shape.shape": 1,
    "shape.innerRadius": 0.6,
    "shape.color.mode": 1,
    "shape.color.angle": 45.0
  }
}
```

### 5.3 Parameter-zu-Preset Mapping

| Parameter-Prefix | Preset-Widget |
|------------------|---------------|
| `audio.smooth.*` | `audio.smooth.preset` |
| `audio.*` | `audio.preset` |
| `shape.color.*` | `shape.color.preset` |
| Sonstige | Visualizer-Preset |

---

## 6. Thread-Sicherheit

**Nicht thread-safe.** Preset-Operationen vom UI-Thread aufrufen.

---

## 7. Fehlerbehandlung

- Ungültiger Preset-Name: Warnung, Operation abgebrochen
- Datei nicht lesbar: `loadPreset()` gibt `std::nullopt` zurück
- Datei nicht schreibbar: `savePreset()` gibt `false` zurück
- Builtin-Preset löschen: `deletePreset()` gibt `false` zurück
- Keine Exceptions

---

## 8. Siehe auch

- [IModule.md](IModule.md) — Parameter-System
- [SmoothingModule.md](SmoothingModule.md) — .smooth Format
- [AudioSourceModule.md](AudioSourceModule.md) — .audio Format
- [ColorGradientModule.md](ColorGradientModule.md) — .grad Format

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: File-basierte Presets, [Custom] Auto-Switch, Dropdown-Struktur** |
