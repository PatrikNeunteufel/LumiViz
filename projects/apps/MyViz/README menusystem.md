# Menu System Dokumentation

> **Datum:** 2025-12-31  
> **Version:** 2.1.0

---

## Dateien

### Dokumentation

| Datei | Ziel-Pfad | Beschreibung |
|-------|-----------|--------------|
| **MenuManager.md** | `include/UI/managers/` | MenuManager API und Integration |
| **MenuRegistry.md** | `include/services/` | MenuRegistry API und Registrierung |
| **Registries.md** | `include/services/` | Übersicht aller Registries (aktualisiert) |

### Code (aktualisiert)

| Datei | Ziel-Pfad | Beschreibung |
|-------|-----------|--------------|
| **MenuAutoReg.cpp** | `src/UI/managers/` | Container-Registrierung (direkt) |
| **MenuItemsAutoReg.cpp** | `src/UI/managers/` | Item-Registrierung (direkt) |

---

## Änderungen in v2.1.0

### 1. Linker-Problem behoben

**Problem:** Bei statischen Libraries entfernt der Linker unreferenzierte Translation Units.

**Lösung:** Direkte Registrierung in Init-Funktionen statt statische Makros:

```cpp
// VORHER (funktioniert nicht zuverlässig):
REGISTER_MENU_CONTAINER("menu.file", "File", "toplevel", 100)

// NACHHER (funktioniert):
void initMenuAutoReg()
{
    MenuRegistry::instance().registerContainer(
        {"menu.file", "toplevel", 100}, "File", false},
        false);
}
```

### 2. Neue Dokumentation

- **MenuManager.md** — Vollständige Dokumentation des Menu-Systems
- **MenuRegistry.md** — API-Referenz für die Registry
- **Registries.md** — Aktualisiert mit Linker-Hinweisen

---

## Quick Start

```cpp
// 1. In MainWindow.cpp:
#include "UI/managers/MenuInit.hpp"

void MainWindow::setupMenuBar()
{
    // WICHTIG: VOR buildMenuBar()!
    initMenuRegistrations();
    
    m_pMenuManager = std::make_unique<MenuManager>(*m_pServices, this);
    m_pMenuManager->buildMenuBar(menuBar());
}

// 2. Event-Handler registrieren:
void MainWindow::setupEventHandlers()
{
    auto* pEventBus = m_pServices->tryResolve<IEventBus>();
    
    pEventBus->subscribe<CreateVisualizerEvent>([this](const auto& e) {
        m_pDockManager->createVisualizer("New Visualizer");
    });
}
```

---

## Menü-Struktur

```
File (100)
├── Open Audio... (100)    [Ctrl+O]
├── ────────────── (800)
└── Exit (900)             [Alt+F4]

View (200)
├── New Visualizer (100)   [Ctrl+N]
├── ──────────────
├── Panels (100)           [DockManager]
├── Perspectives (200)     [DockManager]
├── ──────────────
└── Reset Layout (900)

Settings (300)
└── Frame Mode (100)       [Exclusive]
    ├── Limited (100)      [●]
    ├── Unlimited (200)    [○]
    └── VSync (300)        [○]

Help (900)
└── About MyViz... (100)   [F1]
```
