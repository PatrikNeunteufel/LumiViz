# Layout-Persistenz — Qt-ADS-Docking-Layout über QSettings sichern und wiederherstellen

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Speicherorte und QSettings-Struktur](#2-speicherorte-und-qsettings-struktur)
3. [Kritische Reihenfolge: Widgets vor restoreState](#3-kritische-reihenfolge-widgets-vor-restorestate)
4. [Stabile objectNames](#4-stabile-objectnames)
5. [Save-Timing: aboutToQuit statt Destruktor](#5-save-timing-abouttoquit-statt-destruktor)
6. [Default-Sichtbarkeit](#6-default-sichtbarkeit)
7. [Layout-Versionierung](#7-layout-versionierung)
8. [Ablauf](#8-ablauf)
9. [Menü-Integration](#9-menü-integration)
10. [Troubleshooting](#10-troubleshooting)
11. [Siehe auch](#11-siehe-auch)

---

## 1. Übersicht

Die Panel-Positionen (Qt Advanced Docking System) werden beim Beenden in `QSettings`
gespeichert und beim Start wiederhergestellt. Implementierung:
`src/UI/managers/DockManager.cpp` (Save/Restore), `src/UI/managers/PanelManager.cpp`
(Panel-Erzeugung/Sichtbarkeit), `src/UI/MainWindow.cpp` (Aufruf-Reihenfolge).

```
┌────────────────────┐     ┌────────────────────┐     ┌────────────────────┐
│   App schließen    │────►│    QSettings       │────►│   App starten      │
│ saveLayoutTo…()    │     │  (Registry/File)   │     │  restoreLayout()   │
└────────────────────┘     └────────────────────┘     └────────────────────┘
```

---

## 2. Speicherorte und QSettings-Struktur

Organisation „LumiViz Project", Applikation „LumiViz" (gesetzt in `Application::run()`):

| Plattform | Speicherort |
|-----------|-------------|
| Windows | `HKEY_CURRENT_USER\Software\LumiViz Project\LumiViz` |
| Linux | `~/.config/LumiViz Project/LumiViz.conf` |
| macOS | `~/Library/Preferences/com.myviz-project.LumiViz.plist` |

```
[DockManager]
Version=4                                  ← LAYOUT_VERSION
State=<serialisierte Layout-Bytes>         ← ads::CDockManager::saveState()
Perspectives=<Liste der Perspektiven-Namen>
```

Die Schlüsselnamen sind Konstanten in `DockManager.cpp`
(`SETTINGS_GROUP`, `SETTINGS_STATE`, `SETTINGS_PERSPECTIVES`, `SETTINGS_VERSION`).

---

## 3. Kritische Reihenfolge: Widgets vor restoreState

Qt-ADS `restoreState()` kann nur Widgets positionieren, die **bereits existieren**.

```
FALSCH:
1. restoreState()        ← Visualizer existiert noch nicht!
2. createVisualizer()    ← zu spät, Layout bereits restored

RICHTIG:
1. createAllPanels()     ← alle Panels existieren (DockManager-Konstruktor)
2. createVisualizer()    ← Visualizer existiert
3. restoreLayout()       ← jetzt können alle Widgets positioniert werden
```

Umsetzung in `MainWindow::setupInitialContent()` (Visualizer erzeugen, danach
`m_pDockManager->restoreLayout()`); der DockManager-Konstruktor ruft `restoreLayout()`
bewusst **nicht** selbst auf.

---

## 4. Stabile objectNames

Qt-ADS identifiziert Dock-Widgets beim Restore über `objectName()`. Titel dürfen sich
ändern („Visualizer" → „Visualizer: Pulsing"), der objectName nicht.

- Panels: `PanelBase` setzt `setObjectName(id)`; `PanelManager` überträgt die Panel-ID
  auf das Dock-Widget. Die IDs stammen aus `PanelAutoReg.cpp`.
- Visualizer-Docks: `DockManager::createVisualizer()` vergibt feste Namen
  (`visualizer`, `visualizer_2`, …), der Fenstertitel bleibt dynamisch.

### ObjectName-Tabelle (Stand: Code `src/UI/panels/PanelAutoReg.cpp`, 2026-07-18)

| Widget | objectName / Panel-ID | Titel (änderbar) | defaultVisible |
|--------|-----------------------|------------------|----------------|
| Player-Panel | `player` | "Player" | ja |
| Playlist-Panel | `playlist` | "Playlist" | ja |
| Config-Panel | `config` | "Visualizer Config" | nein |
| Settings-Panel | `settings` | "Settings" | nein |
| Visualizer-Auswahl | `visual_select` | "Visualizers" | ja |
| Visualizer 1 | `visualizer` | "Visualizer: Pulsing" (dynamisch) | — |
| Visualizer N | `visualizer_N` | "Visualizer N: …" (dynamisch) | — |

Änderungen gegenüber der Alt-Doku: Das Panel `config` heißt heute
**„Visualizer Config"** (nicht mehr „Settings"); neu dazugekommen ist das
**SettingsPanel** (`settings`, Titel „Settings") für globale App-Einstellungen.

---

## 5. Save-Timing: aboutToQuit statt Destruktor

Im DockManager-Destruktor ist der `ads::CDockManager` bereits zerstört (beide sind
Children des MainWindow). Deshalb wird im Konstruktor das `aboutToQuit`-Signal
verbunden:

```cpp
// DockManager-Konstruktor:
connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
    saveLayoutToSettings();   // VOR der Zerstörung
});
// Im Destruktor wird NICHT mehr gespeichert.
```

`saveLayoutToSettings()` schreibt Version, State und Perspektiven und ruft
`settings.sync()` auf.

---

## 6. Default-Sichtbarkeit

`defaultVisible` aus den Panel-Deskriptoren darf erst greifen, wenn **kein**
gespeichertes Layout existiert — sonst würden Panels geschlossen, bevor der Restore
sie positioniert:

```cpp
// PanelManager
void createAllPanels();          // NUR erstellen, keine Sichtbarkeit setzen
void applyDefaultVisibility();   // NUR aufrufen, wenn kein Layout existiert

// DockManager
bool restoreLayout()
{
    if (restoreLayoutFromSettings())
        return true;                                   // Layout restored
    m_impl->pPanelManager->applyDefaultVisibility();   // Erststart-Defaults
    return false;
}
```

---

## 7. Layout-Versionierung

Ändert sich die Panel-Struktur (neue Panels, umbenannte IDs), sind alte gespeicherte
Layouts inkompatibel. Schutz: Versions-Check beim Restore.

```cpp
// DockManager.cpp
constexpr int LAYOUT_VERSION = 4;  // bei Struktur-Änderung erhöhen!
```

`restoreLayoutFromSettings()` vergleicht die gespeicherte Version mit
`LAYOUT_VERSION`; bei Abweichung werden die alten Settings der Gruppe **gelöscht** und
`false` zurückgegeben (→ Default-Sichtbarkeit greift). Beim Speichern wird die
aktuelle Version mitgeschrieben.

**Version erhöhen, wenn:** Panels hinzukommen, Panel-IDs sich ändern oder die
Dock-Struktur umgebaut wird. (Das SettingsPanel kam mit Version 4 hinzu.)

---

## 8. Ablauf

### Beim Starten

```
1. DockManager-Konstruktor
   • ads::CDockManager erstellen
   • PanelManager::createAllPanels() — alle Panels erzeugen
   • aboutToQuit-Signal verbinden
   • restoreLayout() wird NICHT aufgerufen
2. MainWindow::setupInitialContent()
   • createVisualizer() — Visualizer erzeugen
   • restoreLayout():
     ├─ Layout gespeichert + Version passt → restoreState()
     └─ sonst → applyDefaultVisibility()
```

Ein erfolgreich restauriertes Layout wird zusätzlich als `defaultState` übernommen
(Basis für „Reset Layout").

### Beim Beenden

```
1. User schließt die App
2. aboutToQuit → saveLayoutToSettings()
   (Version + State + Perspectives, dann sync())
3. Qt zerstört die Objekte
```

---

## 9. Menü-Integration

Zwei Menüpunkte (registriert in `MenuAutoReg.cpp`) arbeiten über den EventBus:

| Menüpunkt | Event | Wirkung im DockManager |
|-----------|-------|------------------------|
| Reset Layout | `ResetLayoutEvent` | `resetLayout()` — stellt den gemerkten `defaultState` wieder her |
| Save Layout as Default | `SaveDefaultLayoutEvent` | `saveDefaultLayout()` — aktuellen `saveState()` als `defaultState` übernehmen |

---

## 10. Troubleshooting

### Layout wird nicht gespeichert

1. Ist das `aboutToQuit`-Signal verbunden?
2. Ist `ads::CDockManager` zum Save-Zeitpunkt noch gültig?
3. Wird `settings.sync()` aufgerufen?

### Layout wird nicht wiederhergestellt

1. Existieren alle Widgets **vor** `restoreState()`?
2. Stimmen die `objectName`s (siehe [Tabelle](#4-stabile-objectnames))?
3. Stimmt die Layout-Version? (Log: `DockManager: Ignoring old layout …`)

### Panels an falscher Position

1. Wurde `LAYOUT_VERSION` nach einer Struktur-Änderung erhöht?
2. Notfalls alte Settings manuell löschen (Registry-Key bzw. Config-Datei, Gruppe
   `DockManager`).

---

## 11. Siehe auch

- [Preset_System.md](Preset_System.md) — Persistenz der Visualizer-Konfigurationen
- [../ui/ConfigPanel_Guide.md](../ui/ConfigPanel_Guide.md) — Config-Panel (`config`, „Visualizer Config")
