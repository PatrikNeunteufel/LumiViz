# Layout Persistence

> **Version:** 1.0.0  
> **Datum:** 2025-12-31  
> **Layout-Version:** 4  
> **Status:** Aktuell

---

## Übersicht

Qt-ADS Layout-Persistence speichert und stellt Panel-Positionen über App-Neustarts wieder her.

```
┌────────────────────┐     ┌────────────────────┐     ┌────────────────────┐
│   App schließen    │────►│    QSettings       │────►│   App starten      │
│   saveLayout()     │     │   (Registry/File)  │     │   restoreLayout()  │
└────────────────────┘     └────────────────────┘     └────────────────────┘
```

---

## Speicherorte

| Platform | Speicherort |
|----------|-------------|
| Windows | `HKEY_CURRENT_USER\Software\MyViz Project\MyViz` |
| Linux | `~/.config/MyViz Project/MyViz.conf` |
| macOS | `~/Library/Preferences/com.myviz-project.MyViz.plist` |

### QSettings-Struktur

```
[DockManager]
Version=4
State=<serialized layout bytes>
Perspectives=<list of perspective names>
```

---

## Kritische Reihenfolge

### Problem: Widget-Existenz

Qt-ADS `restoreState()` kann nur Widgets positionieren die **bereits existieren**.

```
FALSCH:
1. restoreState()        ← Visualizer existiert noch nicht!
2. createVisualizer()    ← Zu spät, Layout bereits restored

RICHTIG:
1. createAllPanels()     ← Panels existieren
2. createVisualizer()    ← Visualizer existiert
3. restoreState()        ← Alle Widgets können positioniert werden
```

### Implementierung in MainWindow

```cpp
void MainWindow::setupInitialContent()
{
    // 1. Visualizer erstellen BEVOR Layout restored wird
    auto* pVisualizer = m_pDockManager->createVisualizer(
        QString(), DockPosition::Center);
    
    // 2. JETZT Layout wiederherstellen
    m_pDockManager->restoreLayout();
}
```

---

## ObjectName-Stabilität

### Problem: Dynamische Titel

Qt-ADS identifiziert Widgets über `objectName()`. Der Titel kann sich ändern ("Visualizer" → "Visualizer: Pulsing"), aber der objectName muss stabil bleiben.

### Lösung: Feste ObjectNames

```cpp
// In DockManager::createVisualizer()
QString objectName = (vizNumber == 1)
    ? QStringLiteral("visualizer")
    : QStringLiteral("visualizer_%1").arg(vizNumber);
pDock->setObjectName(objectName);  // ← Stabil!
pDock->setWindowTitle(dynamicTitle);  // ← Kann sich ändern
```

### ObjectName-Tabelle

| Widget | objectName | Titel (kann sich ändern) |
|--------|------------|--------------------------|
| Player Panel | `player` | "Player" |
| Playlist Panel | `playlist` | "Playlist" |
| Settings Panel | `config` | "Settings" |
| Visualizers Panel | `visual_select` | "Visualizers" |
| Visualizer 1 | `visualizer` | "Visualizer: Pulsing" |
| Visualizer N | `visualizer_N` | "Visualizer N: Bars" |

---

## Save-Timing

### Problem: Destruktor zu spät

Im Destruktor ist `ads::CDockManager` bereits zerstört (beide sind children von MainWindow).

### Lösung: aboutToQuit Signal

```cpp
// In DockManager-Konstruktor:
connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
    saveLayoutToSettings();  // ← VOR Zerstörung
});

// In DockManager-Destruktor:
// NICHT mehr speichern - zu spät!
```

---

## Default-Visibility

### Problem: defaultVisible vor restoreState

Panels wurden basierend auf `defaultVisible` geschlossen BEVOR Layout restored wurde.

### Lösung: Separate Methode

```cpp
// PanelManager
void createAllPanels()
{
    // NUR erstellen, KEINE Sichtbarkeit setzen!
    for (const auto& desc : descriptors) {
        createPanel(desc.id);
    }
}

void applyDefaultVisibility()
{
    // NUR aufrufen wenn KEIN Layout existiert!
    for (const auto& desc : descriptors) {
        if (!desc.defaultVisible) {
            m_dockWidgets[desc.id]->closeDockWidget();
        }
    }
}

// DockManager
bool restoreLayout()
{
    if (restoreLayoutFromSettings()) {
        return true;  // Layout restored, defaultVisible ignorieren
    }
    
    // Kein Layout - jetzt defaultVisible anwenden
    m_impl->pPanelManager->applyDefaultVisibility();
    return false;
}
```

---

## Versionierung

### Problem: Inkompatible Layouts

Wenn sich die Panel-Struktur ändert (neue Panels, umbenannte IDs), können alte Layouts Probleme verursachen.

### Lösung: Version-Check

```cpp
namespace {
    constexpr int LAYOUT_VERSION = 4;  // Bei Struktur-Änderung erhöhen!
}

bool DockManager::restoreLayoutFromSettings()
{
    QSettings settings;
    settings.beginGroup("DockManager");
    
    int savedVersion = settings.value("Version", 0).toInt();
    if (savedVersion != LAYOUT_VERSION) {
        // Alte Settings löschen
        settings.remove("");
        settings.endGroup();
        return false;
    }
    
    // ... restore state
}

void DockManager::saveLayoutToSettings()
{
    QSettings settings;
    settings.beginGroup("DockManager");
    settings.setValue("Version", LAYOUT_VERSION);
    settings.setValue("State", m_impl->pAdsDockManager->saveState());
    settings.endGroup();
    settings.sync();
}
```

### Wann Version erhöhen?

- Neue Panels hinzugefügt
- Panel-IDs geändert
- Panel-Struktur geändert

---

## Ablauf-Diagramm

### Beim Starten

```
┌─────────────────────────────────────────────────────────────────────┐
│ 1. DockManager Konstruktor                                          │
├─────────────────────────────────────────────────────────────────────┤
│    • ads::CDockManager erstellen                                    │
│    • PanelManager::createAllPanels() - alle Panels erstellen        │
│    • aboutToQuit Signal verbinden                                   │
│    • restoreLayout() wird NICHT aufgerufen!                         │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│ 2. MainWindow::setupInitialContent()                                │
├─────────────────────────────────────────────────────────────────────┤
│    • createVisualizer() - Visualizer erstellen                      │
│    • restoreLayout() aufrufen:                                      │
│      ├─ Wenn Layout gespeichert: restoreState()                     │
│      └─ Sonst: applyDefaultVisibility()                             │
└─────────────────────────────────────────────────────────────────────┘
```

### Beim Beenden

```
┌─────────────────────────────────────────────────────────────────────┐
│ 1. User schließt App                                                │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│ 2. aboutToQuit Signal                                               │
├─────────────────────────────────────────────────────────────────────┤
│    • saveLayoutToSettings()                                         │
│      ├─ Version speichern                                           │
│      ├─ State speichern                                             │
│      └─ sync()                                                      │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│ 3. Qt zerstört Objekte                                              │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Menü-Integration

### Reset Layout

```cpp
// In MenuAutoReg.cpp
registry.registerItem(
    MenuItemDesc{
        {"menu.view.resetlayout", "menu.view", 900},
        "Reset Layout",
        [](ServiceContainer& svc) {
            if (auto* eventBus = svc.tryResolve<IEventBus>()) {
                eventBus->publish(ResetLayoutEvent{});
            }
        }
    });

// In DockManager
eventBus->subscribe<ResetLayoutEvent>([this](const ResetLayoutEvent&) {
    resetLayout();  // Stellt defaultState wieder her
});
```

### Save Layout as Default

```cpp
// In MenuAutoReg.cpp
registry.registerItem(
    MenuItemDesc{
        {"menu.view.savedefault", "menu.view", 910},
        "Save Layout as Default",
        [](ServiceContainer& svc) {
            if (auto* eventBus = svc.tryResolve<IEventBus>()) {
                eventBus->publish(SaveDefaultLayoutEvent{});
            }
        }
    });

// In DockManager
eventBus->subscribe<SaveDefaultLayoutEvent>([this](const SaveDefaultLayoutEvent&) {
    m_impl->defaultState = m_impl->pAdsDockManager->saveState();
});
```

---

## Troubleshooting

### Layout wird nicht gespeichert

1. Prüfen: Wird `aboutToQuit` Signal verbunden?
2. Prüfen: Ist `ads::CDockManager` noch gültig?
3. Prüfen: Wird `settings.sync()` aufgerufen?

### Layout wird nicht wiederhergestellt

1. Prüfen: Existieren alle Widgets VOR `restoreState()`?
2. Prüfen: Stimmen die `objectName`s?
3. Prüfen: Stimmt die Layout-Version?

### Panels an falscher Position

1. Prüfen: Wurde Layout-Version erhöht nach Struktur-Änderung?
2. Prüfen: Alte Settings manuell löschen (Registry/Config-Datei)

---

## Siehe auch

- [Panel System](../modules/Panel_System.md) - Panel-Details
- [Event Architecture](Event_Architecture.md) - Reset/Save Events
