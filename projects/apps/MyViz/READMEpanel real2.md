# Panel Real Implementation

> **Datum:** 2025-12-31
> **Version:** 2.0.0

---

## Zusammenfassung

1. Alle 4 Panel-Stubs zu echten Panels mit Service-Integration umgebaut
2. **DockManager erstellt automatisch alle registrierten Panels**
3. **Dezentrale Event-Handler** - MainWindow muss nicht mehr für neue Panels/Events geändert werden

---

## Architektur: Dezentrale Events

```
┌──────────────────────────────────────────────────────────────────────────┐
│                              EventBus                                     │
├──────────────────────────────────────────────────────────────────────────┤
│  CreateVisualizerEvent ──► DockManager                                   │
│  ResetLayoutEvent      ──► DockManager                                   │
│  ChangeVisualizerEvent ──► DockManager                                   │
│  TogglePanelEvent      ──► DockManager (via PanelManager)                │
│  FrameModeChangedEvent ──► MainWindow (emit frameModeChangeRequested)    │
│  OpenDialogEvent       ──► MainWindow (TODO: DialogManager)              │
└──────────────────────────────────────────────────────────────────────────┘
```

**Vorteile:**
- MainWindow muss nicht für neue Panels/Dialogs geändert werden
- Events werden von der Komponente gehandelt, die sie betrifft
- Neue Panels: nur PanelAutoReg.cpp ändern

---

## DockManager: Automatische Panel-Erstellung

```cpp
// Im DockManager Konstruktor:
m_impl->pPanelManager = new PanelManager(services, m_impl->pAdsDockManager, this);
m_impl->pPanelManager->createAllPanels();  // ← Alle registrierten Panels!
subscribeToEvents();
```

**Flow:**
1. DockManager erstellt PanelManager
2. PanelManager liest aus PanelRegistry
3. Alle registrierten Panels werden automatisch erstellt
4. Neue Panels: nur in PanelAutoReg.cpp registrieren

---

## Panel-Übersicht

| Panel | Funktion | Services |
|-------|----------|----------|
| **PlayerPanel** | Audio-Wiedergabe | IAudioPlayer, EventBus |
| **PlaylistPanel** | Playlist-Verwaltung | IPlaylist, IAudioPlayer, EventBus |
| **VisualSelectPanel** | Visualizer-Auswahl | VisualizerRegistry, EventBus |
| **ConfigPanel** | Einstellungen | IAudioEngine, EventBus |

---

## Geänderte Dateien

### DockManager (WICHTIGSTE ÄNDERUNG)
| Datei | Änderungen |
|-------|------------|
| **DockManager.hpp** | +subscribeToEvents(), +unsubscribeFromEvents() |
| **DockManager.cpp** | +PanelManager, +createAllPanels(), +Event-Subscriptions |

### MainWindow (MINIMIERT)
| Datei | Änderungen |
|-------|------------|
| **MainWindow.cpp** | -CreateVisualizerEvent, -ResetLayoutEvent, -ChangeVisualizerEvent, -WidgetRegistry |

### Panels
| Datei | Änderungen |
|-------|------------|
| **PlayerPanel.hpp/cpp** | Volle Audio-Integration |
| **PlaylistPanel.hpp/cpp** | IPlaylist-Integration |
| **VisualSelectPanel.hpp/cpp** | VisualizerRegistry |
| **ConfigPanel.hpp/cpp** | IAudioEngine |

---

## Events im DockManager

```cpp
void DockManager::subscribeToEvents()
{
    // CreateVisualizerEvent - Neuen Visualizer erstellen
    eventBus->subscribe<CreateVisualizerEvent>([this](auto& e) {
        createVisualizer(e.title, DockPosition::Center);
    });
    
    // ResetLayoutEvent - Layout zurücksetzen
    eventBus->subscribe<ResetLayoutEvent>([this](auto& /*e*/) {
        resetLayout();
    });
    
    // ChangeVisualizerEvent - Visualizer wechseln
    eventBus->subscribe<ChangeVisualizerEvent>([this](auto& e) {
        m_impl->visualizers[0]->setVisualizer(e.visualizerId);
    });
    
    // TogglePanelEvent - Panel ein/ausblenden
    eventBus->subscribe<TogglePanelEvent>([this](auto& e) {
        m_impl->pPanelManager->togglePanel(e.panelId);
    });
}
```

---

## Neue Panels hinzufügen (Zukunft)

1. Panel-Klasse erstellen (von PanelBase ableiten)
2. In **PanelAutoReg.cpp** registrieren:
   ```cpp
   registry.registerPanel(
       PanelDescriptor{"new_panel", "New Panel", 500, true},
       [](ServiceContainer& svc) {
           return std::make_unique<NewPanel>(svc);
       },
       false);
   ```
3. **FERTIG** - Keine Änderungen an MainWindow oder DockManager nötig!

---

## Service-Abhängigkeiten

| Panel | Benötigte Services |
|-------|-------------------|
| PlayerPanel | `IEventBus`, `IAudioPlayer` |
| PlaylistPanel | `IEventBus`, `IPlaylist`, `IAudioPlayer` |
| VisualSelectPanel | `IEventBus`, `VisualizerRegistry` |
| ConfigPanel | `IEventBus`, `IAudioEngine` |
