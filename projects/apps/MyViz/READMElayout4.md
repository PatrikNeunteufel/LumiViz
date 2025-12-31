# Fix: Layout-Speicherung und Multiple Visualizers

> **Datum:** 2025-12-31
> **Layout-Version:** 4

---

## Behobene Probleme

### 1. Layout-Position wird nicht wiederhergestellt

**Problem:** Qt-ADS `restoreState()` kann nur Widgets positionieren die **bereits existieren**. 

**Ablauf vorher (BUG):**
```
1. DockManager: Panels erstellt
2. DockManager: restoreLayout() ← Visualizer existiert noch nicht!
3. MainWindow: Visualizer erstellt ← Zu spät, Layout bereits restored
```

**Lösung:** Layout-Restore wurde aus DockManager-Konstruktor entfernt. MainWindow ruft jetzt:
```cpp
// 1. Visualizer erstellen
createVisualizer(...)
// 2. DANACH Layout wiederherstellen
m_pDockManager->restoreLayout();
```

### 2. objectName ändert sich

**Problem:** Qt-ADS identifiziert Widgets über `objectName()`. Der Titel wurde als objectName verwendet, aber der ändert sich ("Visualizer" → "Visualizer: Pulsing").

**Lösung:** Fester objectName der sich nicht ändert:
```cpp
QString objectName = (vizNumber == 1)
    ? QStringLiteral("visualizer")
    : QStringLiteral("visualizer_%1").arg(vizNumber);
pDock->setObjectName(objectName);
```

### 3. Layout im Destruktor speichern (zu spät)

**Problem:** Im Destruktor ist `ads::CDockManager` bereits zerstört.

**Lösung:** `QCoreApplication::aboutToQuit` Signal verwenden.

### 4. defaultVisible vor restoreState

**Problem:** Panels wurden basierend auf `defaultVisible` geschlossen BEVOR Layout restored wurde.

**Lösung:** Neue Methode `applyDefaultVisibility()` - wird NUR aufgerufen wenn KEIN Layout existiert.

### 5. Multiple Visualizers trotz `allowMultiple=false`

**Problem:** Check war nur Kommentar.

**Lösung:** Echter Check implementiert.

### 6. Doppelte Panel-Einträge

**Problem:** MenuManager + DockManager fügten beide hinzu.

**Lösung:** Spezielle Behandlung in MenuManager entfernt.

---

## Korrekter Ablauf

### Beim Starten

```
1. DockManager Konstruktor:
   - ads::CDockManager erstellt
   - PanelManager::createAllPanels() - alle Panels erstellt (alle sichtbar)
   - aboutToQuit Signal verbunden
   - restoreLayout() wird NICHT aufgerufen!

2. MainWindow::setupInitialContent():
   - createVisualizer() - Visualizer erstellt
   - restoreLayout() aufgerufen:
     - Wenn Layout gespeichert: restoreState() → Positionen wiederhergestellt
     - Sonst: applyDefaultVisibility() → Settings-Panel schließen
```

### Beim Beenden

```
1. User schließt App
2. aboutToQuit Signal → saveLayoutToSettings()
3. Qt zerstört Objekte
```

---

## Geänderte Dateien

| Datei | Änderungen |
|-------|------------|
| **DockManager.cpp** | restoreLayout() public, fester objectName, Version 4 |
| **DockManager.hpp** | +restoreLayout() Deklaration |
| **MainWindow.cpp** | Visualizer VOR restoreLayout() erstellen |
| **PanelManager.cpp** | +applyDefaultVisibility() |
| **PanelManager.hpp** | +applyDefaultVisibility() Deklaration |
| **MenuManager.cpp** | Spezielle Panels-Behandlung entfernt |

---

## Wichtige Details

### ObjectNames (für Layout-Persistenz)

| Widget | objectName |
|--------|------------|
| Player Panel | `player` |
| Playlist Panel | `playlist` |
| Settings Panel | `config` |
| Visualizers Panel | `visual_select` |
| Visualizer (1) | `visualizer` |
| Visualizer (N) | `visualizer_N` |

### Layout-Version

Version 4 - alte Settings werden automatisch gelöscht.

---

## Test-Schritte

1. **App starten** → Default-Layout
2. **Panels verschieben** (Visualizer links, Panels rechts, Player unten)
3. **App schließen**
4. **App neu starten** → **Layout identisch wie beim Schließen!** ✅

---

## Zu löschende Datei

**`src/UI/managers/MainWindow.cpp`** - überflüssig, wird nicht kompiliert.

