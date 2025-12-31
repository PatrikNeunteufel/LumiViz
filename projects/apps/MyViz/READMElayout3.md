# Fix: Layout-Speicherung und Multiple Visualizers

> **Datum:** 2025-12-31

---

## Behobene Probleme

### 1. Layout wird beim Beenden nicht gespeichert

**Problem:** Layout wurde im Destruktor gespeichert, aber zu dem Zeitpunkt war `ads::CDockManager` bereits zerstört (beide sind children von MainWindow).

**Lösung:** Layout wird jetzt über `QCoreApplication::aboutToQuit` Signal gespeichert - **VOR** der Zerstörung.

```cpp
// Im Konstruktor:
connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
    BasicLogger::logDebug("aboutToQuit - saving layout");
    saveLayoutToSettings();
});

// Im Destruktor:
// NICHT mehr speichern - zu spät!
```

### 2. Panels werden beim Restore alle geschlossen

**Problem:** `createAllPanels()` hat die Sichtbarkeit basierend auf `defaultVisible` gesetzt BEVOR `restoreState()` aufgerufen wurde. Qt-ADS konnte die geschlossenen Panels nicht wieder öffnen.

**Lösung:** 
- `createAllPanels()` setzt KEINE Sichtbarkeit mehr
- Neue Methode `applyDefaultVisibility()` - wird NUR aufgerufen wenn KEIN Layout restored wurde

```cpp
// VORHER (BUG):
createAllPanels();     // Schließt Panels basierend auf defaultVisible
restoreState();        // Kann geschlossene Panels nicht öffnen!

// NACHHER (FIX):
createAllPanels();     // Erstellt nur, keine Sichtbarkeitsänderung
if (!restoreLayoutFromSettings()) {
    applyDefaultVisibility();  // Nur bei erstem Start
}
```

### 3. Multiple Visualizers trotz `allowMultiple=false`

**Problem:** Der allowMultiple Check war nur ein Kommentar - nicht implementiert!

**Lösung:** Echter Check mit WidgetRegistry implementiert.

### 4. Doppelte Panel-Einträge im Menü

**Problem:** Panels wurden von MenuManager UND DockManager hinzugefügt.

**Lösung:** Spezielle "Panels"-Behandlung in MenuManager entfernt.

### 5. QSettings Bug

**Problem:** `settings.beginGroup()` wurde doppelt aufgerufen.

**Lösung:** Korrigiert.

---

## Geänderte Dateien

| Datei | Änderungen |
|-------|------------|
| **DockManager.cpp** | aboutToQuit Signal, allowMultiple Check, visualizer cleanup |
| **PanelManager.cpp** | Neue `applyDefaultVisibility()` Methode |
| **PanelManager.hpp** | `applyDefaultVisibility()` Deklaration |
| **MenuManager.cpp** | Spezielle Panels-Behandlung entfernt |

---

## Ablauf beim Starten

```
1. DockManager erstellt
2. ads::CDockManager erstellt
3. PanelManager::createAllPanels() - erstellt alle Panels (alle sichtbar)
4. restoreLayoutFromSettings():
   - Wenn Version != 3: alte Settings löschen, return false
   - Wenn Settings vorhanden: restoreState() → return true
   - Sonst: return false
5. Wenn kein Layout restored:
   - applyDefaultVisibility() → schließt Panels mit defaultVisible=false
   - defaultState speichern
6. aboutToQuit Signal verbunden
```

## Ablauf beim Beenden

```
1. User schließt App (X-Button oder File→Exit)
2. QCoreApplication::aboutToQuit Signal wird ausgelöst
3. DockManager::saveLayoutToSettings() wird aufgerufen
   - Version 3 gespeichert
   - Layout state gespeichert
4. Danach: Qt zerstört alle Objekte
```

---

## Test-Schritte

1. **Erster Start:** Alle Panels sichtbar (außer Settings)
2. **Panels anordnen** wie gewünscht
3. **App schließen**
4. **App neu starten** → Layout ist wiederhergestellt ✅
5. **View → New Visualizer** → Kein neuer (allowMultiple=false) ✅
6. **Panels im Menü** → Nur einmal gelistet ✅

---

## Zu löschende Datei

**`/src/UI/managers/MainWindow.cpp`** - überflüssig, wird nicht kompiliert.

