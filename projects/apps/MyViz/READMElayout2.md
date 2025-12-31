# Fix: Layout-Speicherung und Multiple Visualizers

> **Datum:** 2025-12-31

---

## Behobene Probleme

### 1. Layout wird nicht gespeichert/wiederhergestellt

**Bug in `restoreLayoutFromSettings()`:**
```cpp
// VORHER - BUG: settings.beginGroup() doppelt aufgerufen
if (savedVersion != LAYOUT_VERSION)
{
    settings.endGroup();
    settings.beginGroup(SETTINGS_GROUP);  // ← BUG: Doppelter beginGroup!
    settings.remove("");
    settings.endGroup();
    return false;
}

// NACHHER - KORRIGIERT
if (savedVersion != LAYOUT_VERSION)
{
    settings.remove("");  // Innerhalb der bereits offenen Gruppe
    settings.endGroup();
    return false;
}
```

### 2. Multiple Visualizers trotz `allowMultiple=false`

**Problem:** Der allowMultiple Check war nur ein Kommentar!

```cpp
// VORHER - Nur ein Kommentar, kein echter Check
int id1 = eventBus->subscribe<CreateVisualizerEvent>(
    [this](const CreateVisualizerEvent& e) {
        // Check if multiple instances are allowed via WidgetRegistry
        // For now, just create  ← NUR KOMMENTAR!
        createVisualizer(title, DockPosition::Center);
    });

// NACHHER - Echter Check implementiert
int id1 = eventBus->subscribe<CreateVisualizerEvent>(
    [this](const CreateVisualizerEvent& e) {
        const auto* desc = WidgetRegistry::instance().descriptor("visualizer");
        if (desc != nullptr && !desc->allowMultiple)
        {
            if (!m_impl->visualizers.empty())
            {
                BasicLogger::logInfo("Multiple visualizers not allowed - focusing existing");
                // Focus existing visualizer
                return;
            }
        }
        createVisualizer(title, DockPosition::Center);
    });
```

### 3. Visualizer-Tracking bei Schließen

**Problem:** Wenn ein Visualizer geschlossen wird, wurde er nicht aus der Tracking-Liste entfernt.

```cpp
// NACHHER - Cleanup bei dockWidgetRemoved
connect(m_impl->pAdsDockManager, &ads::CDockManager::dockWidgetRemoved,
        this, [this](ads::CDockWidget* pDock) {
            // Remove visualizer from tracking list
            QWidget* widget = pDock->widget();
            auto* visualizer = qobject_cast<VisualizerWidget*>(widget);
            if (visualizer != nullptr)
            {
                auto& v = m_impl->visualizers;
                v.erase(std::remove(v.begin(), v.end(), visualizer), v.end());
            }
            emit dockWidgetClosed(pDock->objectName());
        });
```

### 4. Doppelte Panel-Einträge im Menü

**Problem:** Panels wurden zweimal zum Menü hinzugefügt:
- MenuManager::buildPanelsMenu() 
- DockManager::populatePanelsMenu()

**Lösung:** Die spezielle "Panels"-Behandlung in MenuManager entfernt.

---

## Zusätzliche Änderungen

### Layout-Version erhöht auf 3
Alte inkompatible Settings werden automatisch gelöscht.

### Includes hinzugefügt
- `#include "services/WidgetRegistry.hpp"`
- `#include <algorithm>`

---

## Zu löschende Datei

**WICHTIG:** Die Datei `/src/UI/managers/MainWindow.cpp` sollte gelöscht werden!

Sie wird nicht kompiliert (nicht in Source.cmake), aber ihre Existenz kann zu Verwirrung führen. Die echte MainWindow.cpp ist in `/src/UI/MainWindow.cpp`.

---

## Test-Schritte

1. App starten → Panels werden einmal angezeigt
2. Layout anpassen
3. **View → Save Layout as Default**
4. App schließen
5. App neu starten → Layout ist wiederhergestellt ✅
6. **View → New Visualizer** (Ctrl+N) → Kein neuer Visualizer (weil allowMultiple=false) ✅
