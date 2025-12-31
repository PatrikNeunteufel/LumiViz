# Layout Persistence

> **Datum:** 2025-12-31
> **Feature:** Automatische Layout-Speicherung und -Wiederherstellung

---

## Zusammenfassung

Das Layout (Position und Größe aller Panels) wird automatisch beim Beenden gespeichert und beim Starten wiederhergestellt.

---

## Features

### 1. Automatische Speicherung
- Layout wird in **QSettings** gespeichert (Windows: Registry, Linux: ~/.config)
- Beim Beenden der App wird das aktuelle Layout automatisch gespeichert
- Beim Starten wird das gespeicherte Layout automatisch wiederhergestellt

### 2. Menü-Einträge (View-Menü)
| Menüeintrag | Funktion |
|-------------|----------|
| **Reset Layout** | Setzt auf Default-Layout zurück |
| **Save Layout as Default** | Speichert aktuelles Layout als neues Default |

### 3. Default-Layout
- Beim ersten Start: Das initiale Layout wird als Default gespeichert
- Nach "Save Layout as Default": Aktuelles Layout wird neues Default
- "Reset Layout" stellt immer das Default wieder her

---

## Workflow für Benutzer

### Erstes Einrichten
1. App starten
2. Panels nach Wunsch anordnen (wie im Screenshot)
3. **View → Save Layout as Default** klicken
4. App schließen

### Normaler Betrieb
- Layout wird automatisch gespeichert/wiederhergestellt
- Änderungen bleiben erhalten

### Layout zurücksetzen
- **View → Reset Layout** klicken
- Setzt auf gespeichertes Default zurück

---

## Technische Details

### QSettings Struktur
```
[DockManager]
State=<serialized layout bytes>
Perspectives=<list of perspective names>
```

### Events
```cpp
// Layout zurücksetzen
struct ResetLayoutEvent : public Event
{
    EVENT_TYPE_NAME("ResetLayoutEvent")
};

// Als Default speichern
struct SaveDefaultLayoutEvent : public Event
{
    EVENT_TYPE_NAME("SaveDefaultLayoutEvent")
};
```

### DockManager Methoden
```cpp
// Öffentlich
void resetLayout();           // Stellt Default wieder her
void saveDefaultLayout();     // Speichert aktuelles als Default

// Privat (automatisch)
bool restoreLayoutFromSettings();  // Beim Start
void saveLayoutToSettings();       // Beim Beenden
```

---

## Geänderte Dateien

| Datei | Änderungen |
|-------|------------|
| **DockManager.hpp** | +saveDefaultLayout(), +restoreLayoutFromSettings(), +saveLayoutToSettings() |
| **DockManager.cpp** | +Layout-Persistence-Logik, +SaveDefaultLayoutEvent Handler |
| **UIEvents.hpp** | +SaveDefaultLayoutEvent |
| **MenuAutoReg.cpp** | +"Save Layout as Default" Menüeintrag |

---

## Speicherort (QSettings)

| Plattform | Speicherort |
|-----------|-------------|
| **Windows** | `HKEY_CURRENT_USER\Software\MyViz\MyViz` |
| **Linux** | `~/.config/MyViz/MyViz.conf` |
| **macOS** | `~/Library/Preferences/com.myviz.MyViz.plist` |

---

## Hinweis

Wenn die App abstürzt, wird das Layout **nicht** gespeichert, da der Destruktor nicht aufgerufen wird. Bei normalem Beenden (X-Button, File → Exit) wird das Layout automatisch gespeichert.
