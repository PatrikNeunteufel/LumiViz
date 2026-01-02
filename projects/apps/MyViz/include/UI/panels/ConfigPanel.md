# ConfigPanel — Visualizer Configuration UI

> **Version:** 2.0.0  
> **Datum:** 2026-01-02  
> **Typ:** CppModuleDoc  
> **Status:** Stabil  
> **Modul:** ConfigPanel  
> **Dateien:** ConfigPanel.hpp, ConfigPanel.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6, PanelBase, IVisualizer  
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

ConfigPanel ist das zentrale **UI-Element zur Konfiguration von Visualizern**. Es generiert automatisch Widgets basierend auf den `paramDescs()` des aktiven Visualizers.

### 1.2 Verantwortlichkeiten

- Automatische Widget-Generierung aus Parameter-Definitionen
- Visibility-System für bedingte Parameter
- SubGroup-Gruppierung in QGroupBox
- Preset-Dropdown mit [Custom], Default, User-Presets
- Save-Button neben Modul-Preset-Dropdowns

### 1.3 Nicht-Verantwortlichkeiten

- Keine Parameter-Logik (→ IModule)
- Keine Preset-Persistenz (→ VisualizerPresetManager)
- Keine Visualizer-Erstellung (→ VisualizerWidget)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6::Widgets | Extern | UI-Komponenten |
| PanelBase | Intern | Basis-Klasse für Dock-Panels |
| IVisualizer | Intern | Parameter-Zugriff |
| VisualizerPresetManager | Intern | Preset-Verwaltung |

---

## 3. API

### 3.1 Konstruktion

```cpp
explicit ConfigPanel(QWidget* parent = nullptr);
~ConfigPanel() override;
```

### 3.2 Widget-Typen

| ParamType | Widget | Besonderheiten |
|-----------|--------|----------------|
| `Int` | QSpinBox + QSlider | Synchronisiert |
| `Float` | QDoubleSpinBox + QSlider | 100 Steps |
| `Bool` | QCheckBox | — |
| `Enum` | QComboBox | Separator `---` disabled |
| `String` | QLineEdit | — |
| `Color4f` | QPushButton + QColorDialog | Zeigt aktuelle Farbe |
| `Button` | QPushButton | Für Aktionen |

### 3.3 Öffentliche Methoden

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `setVisualizer(viz)` | `IVisualizer*` | `void` | Visualizer setzen, Widgets generieren |
| `syncFromVisualizer()` | — | `void` | Widgets mit Visualizer synchronisieren |

### 3.4 Signale

| Signal | Parameter | Beschreibung |
|--------|-----------|--------------|
| `paramChanged(id, value)` | `QString`, `QVariant` | Parameter wurde geändert |

---

## 4. Verwendung

### 4.1 Panel einrichten

```cpp
// In MainWindow oder DockManager
auto* configPanel = new ConfigPanel();
configPanel->setVisualizer(m_visualizer);

// Bei Visualizer-Wechsel
void onVisualizerChanged(IVisualizer* newViz)
{
    configPanel->setVisualizer(newViz);
}
```

### 4.2 Preset-Dropdown Struktur

```
┌─────────────────────────────────┐
│ [Custom]              ▼ │ Save │ Delete │
├─────────────────────────────────┤
│ [Custom]         ← Index 0: Manuell geändert
│ Default          ← Index 1: Hardcoded Defaults
│ ───────────────  ← Separator (disabled)
│ UserPreset1      ← Index 3+: User-Presets
│ UserPreset2
└─────────────────────────────────┘
```

### 4.3 [Custom] Auto-Switch

Bei jeder Parameter-Änderung:

1. Visualizer-Preset springt auf `[Custom]` (Index 0)
2. Zugehöriges Modul-Preset wird aktualisiert

```cpp
// User ändert audio.smooth.timeMs
// → audio.smooth.preset wird [Custom]
// → Visualizer-Preset wird [Custom]
```

---

## 5. Interna

### 5.1 Widget-Registrierung

```cpp
struct ParamWidget
{
    ModuleParamDesc desc;
    QWidget* control;      // SpinBox, ComboBox, etc.
    QWidget* container;    // Row mit Label + Control
};

std::vector<ParamWidget> m_paramWidgets;
std::map<std::string, QWidget*> m_controlMap;
```

### 5.2 Visibility-Update

```cpp
void ConfigPanel::updateVisibility()
{
    for (auto& widget : m_paramWidgets)
    {
        if (widget.desc.dependsOn.empty()) continue;
        
        ParamValue depValue;
        m_visualizer->getParam(widget.desc.dependsOn, depValue);
        
        bool visible = false;
        for (const auto& reqValue : widget.desc.dependsValues)
        {
            if (depValue == reqValue)
            {
                visible = true;
                break;
            }
        }
        
        widget.container->setVisible(visible);
    }
}
```

### 5.3 SubGroup-Handling

```cpp
void ConfigPanel::buildWidgets()
{
    std::map<std::string, QGroupBox*> subGroups;
    
    for (const auto& desc : params)
    {
        if (!desc.subGroup.empty())
        {
            if (subGroups.find(desc.subGroup) == subGroups.end())
            {
                auto* group = new QGroupBox(desc.subGroup.c_str());
                subGroups[desc.subGroup] = group;
            }
            // Widget zu SubGroup hinzufügen
        }
    }
}
```

### 5.4 Module Preset Save-Button

```cpp
bool isModulePreset = desc.id.find("preset") != std::string::npos &&
                      (desc.id.find("smooth") != std::string::npos ||
                       desc.id.find("audio") != std::string::npos ||
                       desc.id.find("color") != std::string::npos);

if (isModulePreset)
{
    auto* saveBtn = new QPushButton("Save");
    saveBtn->setFixedWidth(50);
    connect(saveBtn, &QPushButton::clicked, [=]() {
        onModulePresetSave(desc.id);
    });
    layout->addWidget(saveBtn);
}
```

### 5.5 Parameter-zu-Preset Mapping

```cpp
void ConfigPanel::updateRelatedPresetWidget(const std::string& paramId)
{
    std::string presetId;
    
    // Finde zugehöriges Preset
    size_t smoothPos = paramId.find("smooth.");
    if (smoothPos != std::string::npos)
    {
        presetId = paramId.substr(0, smoothPos) + "smooth.preset";
    }
    else if (paramId.rfind("audio.", 0) == 0)
    {
        presetId = "audio.preset";
    }
    else if (paramId.rfind("shape.color.", 0) == 0)
    {
        presetId = "shape.color.preset";
    }
    
    // Widget aktualisieren
    auto it = m_controlMap.find(presetId);
    if (it != m_controlMap.end())
    {
        auto* combo = qobject_cast<QComboBox*>(it->second);
        ParamValue presetValue;
        if (m_visualizer->getParam(presetId, presetValue))
        {
            QSignalBlocker blocker(combo);
            combo->setCurrentIndex(std::get<int>(presetValue));
        }
    }
}
```

---

## 6. Thread-Sicherheit

**Nicht thread-safe.** Alle Operationen vom UI-Thread.

---

## 7. Fehlerbehandlung

- `setVisualizer(nullptr)`: Leert Panel, keine Widgets
- Ungültige Parameter-Typen: Widget wird übersprungen
- Preset-Speicherfehler: QMessageBox Warnung
- Keine Exceptions

---

## 8. Siehe auch

- [IModule.md](IModule.md) — Parameter-System
- [Preset_System.md](Preset_System.md) — Preset-Verwaltung
- [Panel_System.md](Panel_System.md) — PanelBase Dokumentation

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.0.0** | **2026-01-02** | **[Custom]/Default Struktur, Module Save-Buttons, Auto-Switch** |
| 1.0.0 | 2025-12-31 | Initial: Widget-Generation, SubGroups, Visibility |
