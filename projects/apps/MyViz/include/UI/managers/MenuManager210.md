# MenuManager — Self-Registration Menu System

> **Version:** 2.1.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::UI::MenuManager  
> **Dateien:** MenuManager.hpp, MenuManager.cpp, MenuInit.hpp, MenuAutoReg.cpp, MenuItemsAutoReg.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6::Widgets, MenuRegistry, ServiceContainer, EventBus  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Menu-Struktur](#5-menu-struktur)
6. [Dezentrale Registrierung](#6-dezentrale-registrierung)
7. [Troubleshooting](#7-troubleshooting)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Das Menu-System ermöglicht:
- **Self-Registration:** Menu-Items werden dort definiert, wo sie gebraucht werden
- **Event-Driven:** Actions triggern Events statt direkter Callbacks
- **QActionGroup:** Exklusive Checkboxen (Radio-Button-Stil)
- **DockManager-Integration:** Panels und Perspectives Menüs

### 1.2 Komponenten

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         Menu System                                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐    │
│  │ MenuAutoReg.cpp │     │MenuItemsAutoReg │     │   MenuInit.hpp  │    │
│  │ (Container)     │     │     .cpp        │     │ (Deklarationen) │    │
│  └────────┬────────┘     └────────┬────────┘     └─────────────────┘    │
│           │                       │                                      │
│           └───────────┬───────────┘                                      │
│                       │ initMenuRegistrations()                          │
│                       ▼                                                  │
│              ┌─────────────────┐                                         │
│              │  MenuRegistry   │ ◄── Singleton, speichert alle Menüs     │
│              │  (Singleton)    │                                         │
│              └────────┬────────┘                                         │
│                       │                                                  │
│                       ▼                                                  │
│              ┌─────────────────┐                                         │
│              │  MenuManager    │ ◄── Baut QMenuBar aus Registry          │
│              │                 │                                         │
│              └────────┬────────┘                                         │
│                       │                                                  │
│           ┌───────────┼───────────┐                                      │
│           ▼           ▼           ▼                                      │
│       QMenuBar     QMenu      QAction                                    │
│                                   │                                      │
│                                   │ triggered()                          │
│                                   ▼                                      │
│                              EventBus                                    │
│                                   │                                      │
│                                   ▼                                      │
│                             MainWindow                                   │
│                          (Event Handler)                                 │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Architektur

### 2.1 Datenfluss

```
                    Startup
                       │
                       ▼
          ┌────────────────────────┐
          │ initMenuRegistrations()│ ◄── MainWindow::setupMenuBar()
          └────────────────────────┘
                       │
          ┌────────────┴────────────┐
          ▼                         ▼
┌──────────────────┐     ┌──────────────────┐
│ initMenuAutoReg()│     │initMenuItemsAutoReg│
│   (Container)    │     │     (Items)        │
└────────┬─────────┘     └─────────┬──────────┘
         │                         │
         └────────────┬────────────┘
                      ▼
              ┌─────────────┐
              │MenuRegistry │
              │  - containers│
              │  - groups    │
              │  - items     │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │ MenuManager │
              │buildMenuBar()│
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │  QMenuBar   │
              │  ├─ File    │
              │  ├─ View    │
              │  ├─ Settings│
              │  └─ Help    │
              └─────────────┘
```

### 2.2 Event-Architektur

Menu-Actions triggern Events statt direkter Callbacks:

```
┌──────────────┐    click     ┌──────────────┐
│  QAction     │ ───────────► │ MenuRegistry │
│ "New Viz"    │              │  onClick()   │
└──────────────┘              └──────┬───────┘
                                     │
                                     ▼
                              ┌──────────────┐
                              │   EventBus   │
                              │  publish()   │
                              └──────┬───────┘
                                     │
                              CreateVisualizerEvent
                                     │
                                     ▼
                              ┌──────────────┐
                              │  MainWindow  │
                              │ (subscriber) │
                              └──────────────┘
```

---

## 3. API

### 3.1 MenuInit.hpp

```cpp
// Registriert alle Container
void initMenuAutoReg();

// Registriert alle Items
void initMenuItemsAutoReg();

// Convenience: Ruft beide auf
inline void initMenuRegistrations();
```

### 3.2 MenuManager

| Methode | Beschreibung |
|---------|--------------|
| `buildMenuBar(QMenuBar*)` | Baut Menü aus Registry |
| `menu(id)` | Gibt QMenu* für ID zurück |
| `action(id)` | Gibt QAction* für ID zurück |
| `updateCheckStates()` | Aktualisiert Checkboxen |

### 3.3 MenuRegistry (Singleton)

| Methode | Beschreibung |
|---------|--------------|
| `registerContainer(desc)` | Registriert Submenu |
| `registerGroup(desc)` | Registriert Separator-Gruppe |
| `registerItem(desc)` | Registriert Menu-Item |
| `childrenOf(parentId)` | Kinder eines Containers |

### 3.4 Deskriptor-Typen

```cpp
struct MenuContainerDesc {
    MenuBaseDesc base;  // id, parentId, order
    std::string title;
    bool exclusive;     // QActionGroup für Kinder
};

struct MenuItemDesc {
    MenuBaseDesc base;
    std::string title;
    Callback onClick;
    IsChecked isChecked;  // Optional
    IsEnabled isEnabled;  // Optional
    std::string shortcut;
};
```

---

## 4. Verwendung

### 4.1 Setup in MainWindow

```cpp
#include "UI/managers/MenuInit.hpp"
#include "UI/managers/MenuManager.hpp"

void MainWindow::setupMenuBar()
{
    // WICHTIG: Muss VOR buildMenuBar() aufgerufen werden!
    initMenuRegistrations();
    
    // MenuManager erstellen
    m_pMenuManager = std::make_unique<MenuManager>(*m_pServices, this);
    
    // Menü aus Registry bauen
    m_pMenuManager->buildMenuBar(menuBar());
    
    // DockManager-Integration
    if (m_pDockManager)
    {
        QMenu* pPanelsMenu = m_pMenuManager->menu("menu.view.panels");
        if (pPanelsMenu)
        {
            m_pDockManager->populatePanelsMenu(pPanelsMenu);
        }
    }
}
```

### 4.2 Event-Handler registrieren

```cpp
void MainWindow::setupEventHandlers()
{
    auto* pEventBus = m_pServices->tryResolve<IEventBus>();
    
    pEventBus->subscribe<CreateVisualizerEvent>([this](const auto& e) {
        m_pDockManager->createVisualizer("New Visualizer");
    });
    
    pEventBus->subscribe<FrameModeChangedEvent>([this](const auto& e) {
        emit frameModeChangeRequested(e.mode);
    });
}
```

---

## 5. Menu-Struktur

### 5.1 Aktuelle Struktur

```
File (100)
├── Open Audio... (100)    [Ctrl+O]
├── ────────────── (800)   [Separator]
└── Exit (900)             [Alt+F4]

View (200)
├── New Visualizer (100)   [Ctrl+N]
├── ────────────── (50)    [Separator]
├── Panels (100)           [→ von DockManager]
├── Perspectives (200)     [→ von DockManager]
├── ────────────── (800)   [Separator]
└── Reset Layout (900)

Settings (300)
└── Frame Mode (100)       [Exclusive Container]
    ├── Limited (100)      [●] Default
    ├── Unlimited (200)    [○]
    └── VSync (300)        [○]

Help (900)
└── About MyViz... (100)   [F1]
```

### 5.2 ID-Konvention

```
menu.<toplevel>.<container>.<item>

Beispiele:
  menu.file                    → File Menü
  menu.file.exit               → Exit Item
  menu.settings.framemode      → Frame Mode Submenu
  menu.settings.framemode.vsync→ VSync Item
```

---

## 6. Dezentrale Registrierung

### 6.1 Konzept

Items können in ihren jeweiligen Komponenten registriert werden:

```cpp
// In AboutDialog.cpp
void initAboutDialogMenu()
{
    MenuRegistry::instance().registerItem(
        MenuItemDesc{
            {"menu.help.about", "menu.help", 100},
            "About MyViz...",
            [](ServiceContainer& svc) {
                svc.resolve<DialogManager>().open("about");
            },
            {}, {}, "F1"
        },
        false);
}

// Dann in MenuInit.hpp ergänzen:
void initAboutDialogMenu();

inline void initMenuRegistrations()
{
    initMenuAutoReg();
    initMenuItemsAutoReg();
    initAboutDialogMenu();  // NEU
}
```

### 6.2 Geplante Dezentralisierung

| Item | Aktuelle Datei | Ziel-Datei |
|------|----------------|------------|
| Open Audio... | MenuItemsAutoReg.cpp | AudioPlayer.cpp |
| New Visualizer | MenuItemsAutoReg.cpp | VisualizerWidget.cpp |
| Reset Layout | MenuItemsAutoReg.cpp | DockManager.cpp |
| Frame Mode | MenuItemsAutoReg.cpp | Application.cpp |
| About MyViz | MenuItemsAutoReg.cpp | AboutDialog.cpp |

---

## 7. Troubleshooting

### 7.1 Menü erscheint nicht

**Problem:** Leere Menüleiste nach Build.

**Ursache:** `initMenuRegistrations()` wird nicht aufgerufen.

**Lösung:**
```cpp
void MainWindow::setupMenuBar()
{
    initMenuRegistrations();  // ← MUSS HIER STEHEN!
    m_pMenuManager->buildMenuBar(menuBar());
}
```

### 7.2 Linker entfernt Registrierungen

**Problem:** Bei statischen Libraries werden unreferenzierte TUs entfernt.

**Ursache:** Statische Makros (REGISTER_MENU_ITEM) erzeugen keine externen Symbole.

**Lösung (v2.1):** Direkte Registrierung in Init-Funktionen:
```cpp
void initMenuAutoReg()
{
    // Direkte Registrierung, NICHT statische Makros
    MenuRegistry::instance().registerContainer(...);
}
```

### 7.3 Checkboxen werden nicht aktualisiert

**Problem:** Frame-Mode-Auswahl wird nicht angezeigt.

**Lösung:** `updateCheckStates()` nach Änderung aufrufen oder `isChecked`-Callback korrekt implementieren.

### 7.4 Exclusive Items funktionieren nicht

**Problem:** Mehrere Items gleichzeitig ausgewählt.

**Lösung:** Container muss `exclusive = true` haben:
```cpp
registry.registerContainer(
    MenuContainerDesc{
        {"menu.settings.framemode", "menu.settings", 100},
        "Frame Mode",
        true  // ← EXCLUSIVE!
    },
    false);
```

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.1.0** | **2025-12-31** | **Fix: Direkte Registrierung in Init-Funktionen (Linker-Problem)** |
| 2.0.0 | 2025-12-31 | Refactoring: Self-Registration, Event-Driven, QActionGroup |
| 1.0.0 | 2025-12-28 | Initial: MenuRegistry, MenuManager, Basis-Makros |
