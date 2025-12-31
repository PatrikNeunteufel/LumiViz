# Registry Lazy-Init: Alle Registries

> **Datum:** 2025-12-31

---

## Zusammenfassung

Alle 5 Registries auf einheitliches Lazy-Init Pattern umgestellt:

1. **MenuRegistry** ✅
2. **VisualizerRegistry** ✅
3. **PanelRegistry** ✅
4. **DialogRegistry** ✅
5. **WidgetRegistry** ✅ (neu: mit `allowMultiple` Flag)

---

## Übersicht

| Registry | AutoReg-Datei | Ziel-Pfad |
|----------|---------------|-----------|
| MenuRegistry | MenuAutoReg.cpp | src/UI/managers/ |
| VisualizerRegistry | VisualizerAutoReg.cpp | src/visualizers/ |
| PanelRegistry | PanelAutoReg.cpp | src/UI/panels/ |
| DialogRegistry | DialogAutoReg.cpp | src/UI/dialogs/ |
| WidgetRegistry | WidgetAutoReg.cpp | src/UI/widgets/ |

---

## 1. WidgetRegistry (NEU)

### Neues Feature: `allowMultiple`

```cpp
struct WidgetDescriptor
{
    std::string id;             ///< Unique ID
    std::string name;           ///< Display name
    std::string category;       ///< Category for grouping
    std::string description;    ///< Brief description
    int order = 0;              ///< Sort order
    bool allowMultiple = false; ///< Allow multiple instances (like documents)
};
```

### Architektur

```
┌──────────────────────────────────┐     ┌──────────────────────────────┐
│ WidgetRegistry.hpp/.cpp          │     │ WidgetAutoReg.cpp            │
│ (Framework - wiederverwendbar)   │     │ (MyViz-spezifisch)           │
├──────────────────────────────────┤     ├──────────────────────────────┤
│ • Singleton + Lazy-Init          │     │ void initWidgetDefaults(reg) │
│ • extern initWidgetDefaults() ───┼────►│   - VisualizerWidget (multi) │
│ • +allowMultiple Flag            │     │   - (zukünftige Widgets)     │
└──────────────────────────────────┘     └──────────────────────────────┘
```

### Widget-Registrierungen

| ID | Name | Kategorie | allowMultiple | Beschreibung |
|----|------|-----------|---------------|--------------|
| visualizer | Visualizer | Visualizers | **true** | OpenGL visualization widget |

### Neue Makros

```cpp
// Singleton Widget (allowMultiple = false)
REGISTER_WIDGET("volume", "Volume", VolumeWidget)

// Multi-Instance Widget (allowMultiple = true)
REGISTER_WIDGET_MULTI("visualizer", "Visualizer", "Visualizers", VisualizerWidget)

// Full Control
REGISTER_WIDGET_FULL("id", "Name", "Category", "Description", 100, true, Type)
```

---

## 2. PanelRegistry

### Panel-Registrierungen

| ID | Titel | Order | Default Visible |
|----|-------|-------|-----------------|
| player | Player | 100 | true |
| playlist | Playlist | 200 | true |
| config | Settings | 300 | **false** |
| visual_select | Visualizers | 400 | true |

---

## 3. DialogRegistry

### Dialog-Registrierungen

| ID | Titel | Order | Modal | MenuPath | Shortcut |
|----|-------|-------|-------|----------|----------|
| about | About MyViz | 900 | true | Help | F1 |

---

## Geänderte Dateien (diese Lieferung)

### WidgetRegistry
| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **WidgetRegistry.hpp** | `include/services/` | +allowMultiple, +REGISTER_WIDGET_MULTI |
| **WidgetRegistry.cpp** | `src/services/` | +extern initWidgetDefaults(), lazy-init |
| **WidgetAutoReg.cpp** | `src/UI/widgets/` | **NEU** - registriert VisualizerWidget |
| **src_widgets_Source.cmake** | `src/UI/widgets/` | +WidgetAutoReg.cpp |

### PanelRegistry
| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **PanelRegistry.cpp** | `src/services/` | +extern initPanelDefaults(), lazy-init |
| **PanelAutoReg.cpp** | `src/UI/panels/` | **NEU** |
| **PlayerPanel.cpp** | `src/UI/panels/` | -REGISTER_PANEL Makro |
| **PlaylistPanel.cpp** | `src/UI/panels/` | -REGISTER_PANEL Makro |
| **ConfigPanel.cpp** | `src/UI/panels/` | -REGISTER_PANEL Makro |
| **VisualSelectPanel.cpp** | `src/UI/panels/` | -REGISTER_PANEL Makro |

### DialogRegistry
| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **DialogRegistry.cpp** | `src/services/` | +extern initDialogDefaults(), lazy-init |
| **DialogAutoReg.cpp** | `src/UI/dialogs/` | **NEU** (Feldreihenfolge korrigiert!) |
| **AboutDialog.cpp** | `src/UI/dialogs/` | -REGISTER_DIALOG_SHORTCUT Makro |

### Bugfixes
| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **MainWindow.cpp** | `src/UI/` | -unused lambda capture |

---

## Linker-Garantie

Alle Registries verwenden das gleiche Pattern:

```cpp
// In XxxRegistry.cpp:
extern void initXxxDefaults(XxxRegistry& registry);

XxxRegistry& XxxRegistry::instance()
{
    static XxxRegistry registry;
    static bool initialized = false;
    
    if (!initialized)
    {
        initialized = true;
        initXxxDefaults(registry);  // ← Erzwingt Linkage von XxxAutoReg.cpp
    }
    
    return registry;
}
```

---

## Erwartetes Ergebnis

Nach dem Build sollte:
1. **View → Panels** zeigt alle 4: Player, Playlist, Settings, Visualizers
2. **Help → About** funktioniert (F1)
3. Häkchen im Menü entsprechen dem tatsächlichen Status
4. Tab-Titel zeigt "Visualizer: Pulsing"
5. VisualizerWidget ist in WidgetRegistry registriert (für zukünftige Erweiterungen)
