# Menü-System — Self-Registration über MenuRegistry und MenuManager

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## 1. Überblick

Die Menüleiste wird nicht im MainWindow verdrahtet, sondern **deklarativ
registriert** und daraus gebaut:

| Komponente | Rolle | Quelle |
|---|---|---|
| `MenuRegistry` | Singleton; speichert Container/Groups/Items | `services/MenuRegistry.hpp/.cpp` |
| `MenuAutoReg.cpp` | App-spezifische Menüstruktur (`initMenuDefaults`) | `src/UI/managers/MenuAutoReg.cpp` |
| `MenuManager` | Baut `QMenuBar`/`QMenu`/`QAction` aus der Registry | `src/UI/managers/MenuManager.cpp` |
| `DockManager` | Befüllt die Submenüs Panels und Perspectives | `src/UI/managers/DockManager.cpp` |

```
MenuAutoReg.cpp ──registriert──► MenuRegistry ──buildMenuBar()──► MenuManager ──► QMenuBar
                                                                       ▲
                             DockManager::populatePanelsMenu()/populatePerspectivesMenu()
```

Wie beim Panel-System gilt Lazy-Init mit Linker-Garantie:
`MenuRegistry::instance()` ruft beim ersten Zugriff `initMenuDefaults()` auf;
die `extern`-Deklaration in `MenuRegistry.cpp` zwingt den Linker,
`MenuAutoReg.cpp` einzubinden.

API-Details: CppModuleDocs [MenuRegistry](../../include/UI/managers/MenuRegistry.md)
und [MenuManager](../../include/UI/managers/MenuManager.md).

---

## 2. Deskriptoren

Alle Deskriptoren teilen sich die Basis `{id, parentId, order}`; Wurzel-Einträge
hängen unter `"toplevel"`.

| Typ | Zweck | Besonderheit |
|---|---|---|
| `MenuContainerDesc` | Menü/Submenü | `title` + **`exclusive`-Flag** — `true` legt für alle Kind-Items eine exklusive `QActionGroup` an (Radio-Verhalten) |
| `MenuGroupDesc` | Separator-Gruppe | kein Titel; MenuManager fügt beim Order-Wechsel einen Separator ein |
| `MenuItemDesc` | Klickbarer Eintrag | `title`, `onClick(ServiceContainer&)`, optional `isChecked`, `isEnabled`, `shortcut` (z. B. `"Ctrl+O"`) |

Konventionen: IDs als `menu.<toplevel>.<submenu>.<item>`; Order in
100er-Schritten; Exit/Reset-Aktionen bei 900.

### Aktionen publizieren Events

Item-Callbacks lösen **Events über den EventBus** aus statt Services direkt zu
manipulieren — die eigentliche Arbeit macht der Subscriber (meist DockManager
oder MainWindow):

```cpp
registry.registerItem(
    MenuItemDesc{
        {"menu.view.newvisualizer", "menu.view", 100},
        "New Visualizer",
        [](ServiceContainer& svc) {
            if (auto* eventBus = svc.tryResolve<IEventBus>())
                eventBus->publish(CreateVisualizerEvent{});
        },
        {}, {}, "Ctrl+N"
    },
    false);
```

---

## 3. Menübaum (verifiziert gegen `MenuAutoReg.cpp`, Stand 2026-07-18)

```
File (100)
├── Open Audio...  (100)  [Ctrl+O]   ← derzeit Stub: nur Debug-Log
├── ─────────────  (800)
└── Exit           (900)  [Alt+F4]

View (200)
├── New Visualizer (100)  [Ctrl+N]   → CreateVisualizerEvent
├── Fullscreen     (150)  [F11]      → ToggleFullscreenEvent
├── Panels         (100)  [Container; befüllt von DockManager]
├── Perspectives   (200)  [Container; befüllt von DockManager]
├── ─────────────  (800)
├── Reset Layout   (900)             → ResetLayoutEvent
└── Save Layout as Default (910)     → SaveDefaultLayoutEvent

Settings (300)
└── Frame Mode (100)      [Container, exclusive = true]
    ├── Limited (60 FPS) (100) [●]   → FrameModeChangedEvent{0}
    ├── Unlimited        (200) [○]   → FrameModeChangedEvent{1}
    └── VSync            (300) [○]   → FrameModeChangedEvent{2}

Help (900)
└── About MyViz... (100)  [F1]       → OpenDialogEvent{"about"}
```

Hinweise:

- **Open Audio...** ist registriert, führt aber noch keine Aktion aus
  (nur Log-Ausgabe) — Dateien kommen derzeit über das PlaylistPanel herein.
- Die `isChecked`-Callbacks der Frame-Mode-Items liefern aktuell **statische
  Werte** (Limited fest `true`); der Haken folgt also nicht dem tatsächlichen
  Modus, die Umschaltung selbst funktioniert über die exklusive ActionGroup.

---

## 4. Exklusive Items

Exklusivität ist eine **Eigenschaft des Containers**, nicht der Items:
`MenuContainerDesc{..., "Frame Mode", true}`. `MenuManager::buildMenuRecursive()`
legt dann eine `QActionGroup` mit `setExclusive(true)` an und hängt alle
Kind-Items hinein; checkable wird ein Item durch einen gesetzten
`isChecked`-Callback.

---

## 5. Panels- und Perspectives-Menü

`MenuAutoReg.cpp` registriert nur die **leeren Container**
`menu.view.panels` / `menu.view.perspectives`. Befüllt werden sie nach dem
`buildMenuBar()` vom MainWindow aus (`MainWindow.cpp`):

```cpp
m_pDockManager->populatePanelsMenu(m_pMenuManager->menu("menu.view.panels"));
m_pDockManager->populatePerspectivesMenu(m_pMenuManager->menu("menu.view.perspectives"));
```

`populatePanelsMenu()` fügt je Dock-Widget die Qt-ADS-`toggleViewAction()` ein:
automatisch checkable und dauerhaft mit der Panel-Sichtbarkeit synchron.
(`MenuManager` besitzt zusätzlich ein eigenes `buildPanelsMenu()` auf
PanelRegistry-Basis; der aktive Pfad ist der über den DockManager.)

---

## 6. Neues Menü-Item hinzufügen

1. In `initMenuDefaults()` (`MenuAutoReg.cpp`) registrieren — Container zuerst,
   falls neu; dann `registerItem` mit ID/Parent/Order.
2. Aktion als Event publizieren; den Subscriber dort implementieren, wo die
   Wirkung hingehört (DockManager, MainWindow, Panel).
3. Für Radio-Gruppen: eigenen Container mit `exclusive = true` anlegen und die
   Varianten als checkable Items (mit `isChecked`) hineinhängen.

---

## 7. Siehe auch

- [Panel_System.md](Panel_System.md) — Panels, DockManager, Events
- CppModuleDocs: [MenuRegistry](../../include/UI/managers/MenuRegistry.md) ·
  [MenuManager](../../include/UI/managers/MenuManager.md) ·
  [DockManager](../../include/UI/managers/DockManager.md)

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 1.0.0 | 2026-07-18 | Neu aus `harvest/old_docs/modules/Menu_System.md` konsolidiert; Menübaum gegen Code verifiziert (Fullscreen/Save-Layout ergänzt, VSync-Einzeleintrag entfernt); Exklusiv-Mechanismus korrigiert (Container-Flag statt Item-actionGroup) |
