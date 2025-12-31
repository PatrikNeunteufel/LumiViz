# Menu Lazy-Init Refactoring (v2.2.0)

> **Datum:** 2025-12-31

---

## Zusammenfassung

Die Menu-Registrierung wurde vereinfacht: **Automatische Initialisierung** beim ersten `MenuRegistry::instance()` Aufruf.

## Vorher (v2.1.0)

```cpp
// MainWindow.cpp
#include "UI/managers/MenuInit.hpp"

void MainWindow::setupMenuBar()
{
    initMenuRegistrations();  // ← Manueller Aufruf nötig!
    m_pMenuManager->buildMenuBar(menuBar());
}
```

**Dateien:**
- MenuAutoReg.cpp
- MenuItemsAutoReg.cpp
- MenuInit.hpp

## Nachher (v2.2.0)

```cpp
// MainWindow.cpp
void MainWindow::setupMenuBar()
{
    // Kein init nötig - passiert automatisch!
    m_pMenuManager->buildMenuBar(menuBar());
}
```

**Dateien:**
- ~~MenuAutoReg.cpp~~ (gelöscht)
- ~~MenuItemsAutoReg.cpp~~ (gelöscht)
- ~~MenuInit.hpp~~ (gelöscht)
- MenuRegistry.cpp (enthält jetzt alles)

---

## Geänderte Dateien

| Datei | Aktion | Beschreibung |
|-------|--------|--------------|
| **MenuRegistry.hpp** | Geändert | +initDefaults() private Methode |
| **MenuRegistry.cpp** | Geändert | +initDefaults() mit allen Menüs, lazy-init in instance() |
| **MainWindow.cpp** | Geändert | -initMenuRegistrations() Aufruf, -MenuInit.hpp include |
| **src/managers/Source.cmake** | Geändert | -MenuAutoReg.cpp, -MenuItemsAutoReg.cpp |
| **include/managers/Source.cmake** | Geändert | -MenuInit.hpp |
| MenuAutoReg.cpp | **Gelöscht** | - |
| MenuItemsAutoReg.cpp | **Gelöscht** | - |
| MenuInit.hpp | **Gelöscht** | - |

---

## Architektur

```
┌─────────────────────────────────────────────────────────────────────────┐
│                     Erster Aufruf von MenuRegistry::instance()          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  MenuRegistry::instance()                                                │
│  ┌────────────────────────────────┐                                      │
│  │ static MenuRegistry registry;  │                                      │
│  │ static bool initialized = false│                                      │
│  │                                │                                      │
│  │ if (!initialized) {            │                                      │
│  │     initialized = true;        │                                      │
│  │     registry.initDefaults(); ──┼──► Registriert alle Menüs           │
│  │ }                              │                                      │
│  │                                │                                      │
│  │ return registry;               │                                      │
│  └────────────────────────────────┘                                      │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Vorteile

| Aspekt | Vorher | Nachher |
|--------|--------|---------|
| **Dateien** | 3 zusätzliche Dateien | Alles in MenuRegistry |
| **Aufruf** | Manuell (vergessbar) | Automatisch |
| **Linker** | Init-Workaround nötig | Kein Problem |
| **Wartung** | 3 Stellen synchron halten | 1 zentrale Stelle |

---

## Menü-Struktur (unverändert)

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
