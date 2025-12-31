# Registry Lazy-Init: PanelRegistry & DialogRegistry

> **Datum:** 2025-12-31

---

## Zusammenfassung

Ergänzung zu den vorherigen Lazy-Init Änderungen:

1. **PanelRegistry** auf Lazy-Init umgestellt
2. **DialogRegistry** auf Lazy-Init umgestellt

---

## 1. PanelRegistry Lazy-Init

### Problem

Im Screenshot fehlte "Player" Panel im Menü, obwohl alle 4 Panels im Build waren.
Das liegt am Linker-Problem: Ohne externe Referenzen werden .obj Dateien
aus static libraries nicht eingebunden.

### Lösung

```
┌──────────────────────────────────┐     ┌──────────────────────────────┐
│ PanelRegistry.hpp/.cpp           │     │ PanelAutoReg.cpp             │
│ (Framework - wiederverwendbar)   │     │ (MyViz-spezifisch)           │
├──────────────────────────────────┤     ├──────────────────────────────┤
│ • Singleton + Lazy-Init          │     │ void initPanelDefaults(reg)  │
│ • extern initPanelDefaults() ────┼────►│   - PlayerPanel              │
│                                  │     │   - PlaylistPanel            │
│                                  │     │   - ConfigPanel              │
│                                  │     │   - VisualSelectPanel        │
└──────────────────────────────────┘     └──────────────────────────────┘
```

### Geänderte Dateien

| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **PanelRegistry.cpp** | `src/services/` | +extern initPanelDefaults(), lazy-init |
| **PanelAutoReg.cpp** | `src/UI/panels/` | **NEU** - registriert alle Panels |
| **PlayerPanel.cpp** | `src/UI/panels/` | -REGISTER_PANEL Makro |
| **PlaylistPanel.cpp** | `src/UI/panels/` | -REGISTER_PANEL Makro |
| **ConfigPanel.cpp** | `src/UI/panels/` | -REGISTER_PANEL Makro |
| **VisualSelectPanel.cpp** | `src/UI/panels/` | -REGISTER_PANEL Makro |
| **src_panels_Source.cmake** | `src/UI/panels/` | +PanelAutoReg.cpp |

### Panel-Registrierungen

| ID | Titel | Order | Default Visible |
|----|-------|-------|-----------------|
| player | Player | 100 | true |
| playlist | Playlist | 200 | true |
| config | Settings | 300 | **false** |
| visual_select | Visualizers | 400 | true |

---

## 2. DialogRegistry Lazy-Init

### Architektur

```
┌──────────────────────────────────┐     ┌──────────────────────────────┐
│ DialogRegistry.hpp/.cpp          │     │ DialogAutoReg.cpp            │
│ (Framework - wiederverwendbar)   │     │ (MyViz-spezifisch)           │
├──────────────────────────────────┤     ├──────────────────────────────┤
│ • Singleton + Lazy-Init          │     │ void initDialogDefaults(reg) │
│ • extern initDialogDefaults() ───┼────►│   - AboutDialog              │
└──────────────────────────────────┘     └──────────────────────────────┘
```

### Geänderte Dateien

| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **DialogRegistry.cpp** | `src/services/` | +extern initDialogDefaults(), lazy-init |
| **DialogAutoReg.cpp** | `src/UI/dialogs/` | **NEU** - registriert alle Dialoge |
| **AboutDialog.cpp** | `src/UI/dialogs/` | -REGISTER_DIALOG_SHORTCUT Makro |
| **src_dialogs_Source.cmake** | `src/UI/dialogs/` | +DialogAutoReg.cpp |

---

## Gesamtübersicht aller Registries

| Registry | Status | AutoReg-Datei |
|----------|--------|---------------|
| **MenuRegistry** | ✅ Lazy-Init | MenuAutoReg.cpp |
| **VisualizerRegistry** | ✅ Lazy-Init | VisualizerAutoReg.cpp |
| **PanelRegistry** | ✅ Lazy-Init | PanelAutoReg.cpp |
| **DialogRegistry** | ✅ Lazy-Init | DialogAutoReg.cpp |
| **WidgetRegistry** | ✅ Keine Nutzung | (nicht benötigt) |

---

## Erwartetes Ergebnis

Nach dem Build sollte:
1. **View → Panels** zeigt alle 4 Panels: Player, Playlist, Settings, Visualizers
2. **Help → About** funktioniert (F1)
3. Häkchen im Menü entsprechen dem tatsächlichen Sichtbarkeitsstatus
4. "Player" Panel ist jetzt vorhanden (vorher fehlend)
