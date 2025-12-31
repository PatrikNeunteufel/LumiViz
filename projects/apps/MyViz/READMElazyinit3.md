# Registry Lazy-Init & Dynamic Visualizer Title

> **Datum:** 2025-12-31

---

## Zusammenfassung

1. **VisualizerRegistry** auf Lazy-Init umgestellt (wie MenuRegistry)
2. **Dynamischer Visualizer-Titel** im Tab basierend auf aktivem Visualizer

---

## 1. VisualizerRegistry Lazy-Init

### Architektur

```
┌──────────────────────────────────┐     ┌──────────────────────────────┐
│ VisualizerRegistry.hpp/.cpp      │     │ VisualizerAutoReg.cpp        │
│ (Framework - wiederverwendbar)   │     │ (MyViz-spezifisch)           │
├──────────────────────────────────┤     ├──────────────────────────────┤
│ • Singleton + Lazy-Init          │     │ void initVisualizerDefaults()│
│ • extern initVisualizerDefaults()│────►│   - PulsingVisualizer        │
│                                  │     │   - (zukünftige Visualizer)  │
└──────────────────────────────────┘     └──────────────────────────────┘
```

### Geänderte Dateien

| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **VisualizerRegistry.cpp** | `src/services/` | +extern initVisualizerDefaults(), lazy-init |
| **VisualizerAutoReg.cpp** | `src/visualizers/` | NEU - initVisualizerDefaults() Implementation |
| **PulsingVisualizer.cpp** | `src/visualizers/` | -REGISTER_VISUALIZER_CATEGORY Makro |
| **Application.cpp** | `src/` | -initializeVisualizers() Aufruf |
| **src_visualizers_Source.cmake** | `src/visualizers/` | +VisualizerAutoReg, -VisualizerInit |
| **include_visualizers_Source.cmake** | `include/visualizers/` | -VisualizerInit.hpp |

### Gelöschte Dateien

- ~~VisualizerInit.hpp~~ (nicht mehr benötigt)
- ~~VisualizerInit.cpp~~ (ersetzt durch VisualizerAutoReg.cpp)

---

## 2. Dynamischer Visualizer-Titel

### Titel-Schema

| Situation | Titel |
|-----------|-------|
| Erster Visualizer, leer | "Visualizer" |
| Erster Visualizer, Pulsing aktiv | "Visualizer: Pulsing" |
| Zweiter Visualizer, leer | "Visualizer 2" |
| Zweiter Visualizer, Spectrum aktiv | "Visualizer 2: Spectrum Analyzer" |

### Geänderte Dateien

| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **DockManager.cpp** | `src/UI/managers/` | +Connect zu visualizerChanged, +dynamischer Titel |
| **MainWindow.cpp** | `src/UI/` | +setVisualizer("pulsing") für initialen Visualizer |

### Implementation

```cpp
// DockManager.cpp - createVisualizer()
connect(pVisualizer, &VisualizerWidget::visualizerChanged, this, 
    [pDock, vizNumber](const QString& vizId) {
        auto* viz = qobject_cast<VisualizerWidget*>(pDock->widget());
        if (viz)
        {
            QString vizName = viz->currentVisualizerName();
            QString newTitle;
            if (vizName.isEmpty())
            {
                newTitle = (vizNumber == 1) 
                    ? "Visualizer" 
                    : QString("Visualizer %1").arg(vizNumber);
            }
            else
            {
                newTitle = (vizNumber == 1)
                    ? QString("Visualizer: %1").arg(vizName)
                    : QString("Visualizer %1: %2").arg(vizNumber).arg(vizName);
            }
            pDock->setWindowTitle(newTitle);
        }
    });
```

---

## 3. Verbleibende Registries

| Registry | Status | Aktion nötig? |
|----------|--------|---------------|
| **MenuRegistry** | ✅ Lazy-Init | Bereits umgestellt |
| **VisualizerRegistry** | ✅ Lazy-Init | Jetzt umgestellt |
| **PanelRegistry** | ⚠️ Makros | Prüfen ob Problem |
| **DialogRegistry** | ⚠️ Makros | Prüfen ob Problem |
| **WidgetRegistry** | ✅ Keine Nutzung | Kein Problem |

**Nächster Schritt:** Testen ob PanelRegistry und DialogRegistry funktionieren. Falls Panels im View-Menü fehlen, gleiche Umstellung nötig.

---

## Erwartetes Ergebnis

Nach dem Build sollte:
1. Der Tab-Titel "Visualizer: Pulsing" anzeigen (statt "Spectrum Analyzer")
2. Bei Wechsel des Visualizers sich der Titel automatisch aktualisieren
3. Keine manuellen init*() Aufrufe mehr nötig sein
