# Menu Lazy-Init Refactoring (v2.2.0)

> **Datum:** 2025-12-31

---

## Zusammenfassung

Die Menu-Registrierung wurde vereinfacht: **Automatische Initialisierung** beim ersten `MenuRegistry::instance()` Aufruf mit sauberer Trennung von Framework und App-Code.

## Architektur

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         Trennung: Framework vs. App                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────────────────────┐     ┌──────────────────────────────┐  │
│  │ MenuRegistry.hpp/.cpp        │     │ MenuAutoReg.cpp              │  │
│  │ (Framework - wiederverwendbar)│     │ (App-spezifisch - MyViz)     │  │
│  ├──────────────────────────────┤     ├──────────────────────────────┤  │
│  │ • Singleton                  │     │ • File, View, Settings, Help │  │
│  │ • Container/Group/Item       │     │ • Alle Menu-Items            │  │
│  │ • Query-Methoden             │     │ • Event-Callbacks            │  │
│  │                              │     │                              │  │
│  │ extern initMenuDefaults() ───┼────►│ void initMenuDefaults(reg)   │  │
│  │        (Linker-Referenz)     │     │                              │  │
│  └──────────────────────────────┘     └──────────────────────────────┘  │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

## Vorher (v2.1.0)

```cpp
// MainWindow.cpp - Manueller Aufruf nötig
#include "UI/managers/MenuInit.hpp"

void MainWindow::setupMenuBar()
{
    initMenuRegistrations();  // ← Vergessbar!
    m_pMenuManager->buildMenuBar(menuBar());
}
```

## Nachher (v2.2.0)

```cpp
// MainWindow.cpp - Automatisch!
void MainWindow::setupMenuBar()
{
    m_pMenuManager->buildMenuBar(menuBar());
    // MenuRegistry::instance() triggert automatisch initMenuDefaults()
}
```

---

## Geänderte Dateien

| Datei | Ziel-Pfad | Änderung |
|-------|-----------|----------|
| **MenuRegistry.hpp** | `include/services/` | -initDefaults() Member |
| **MenuRegistry.cpp** | `src/services/` | +extern initMenuDefaults(), lazy-init |
| **MenuAutoReg.cpp** | `src/UI/managers/` | +initMenuDefaults(registry) Implementation |
| **MainWindow.cpp** | `src/UI/` | -initMenuRegistrations(), -MenuInit.hpp |
| **src_managers_Source.cmake** | `src/UI/managers/` | +MenuAutoReg.cpp |
| **include_managers_Source.cmake** | `include/UI/managers/` | -MenuInit.hpp |

## Gelöschte Dateien

- ~~MenuItemsAutoReg.cpp~~ (zusammengeführt mit MenuAutoReg.cpp)
- ~~MenuInit.hpp~~ (nicht mehr nötig)

---

## Linker-Garantie

```cpp
// MenuRegistry.cpp
extern void initMenuDefaults(MenuRegistry& registry);  // ← Externe Referenz

MenuRegistry& MenuRegistry::instance()
{
    static MenuRegistry registry;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initMenuDefaults(registry);  // ← Erzwingt Linkage von MenuAutoReg.cpp
    }
    return registry;
}
```

Der Linker **muss** MenuAutoReg.cpp einbinden, weil MenuRegistry.cpp eine Funktion daraus aufruft.

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
