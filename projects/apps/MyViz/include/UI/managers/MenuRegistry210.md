# MenuRegistry — Menu Self-Registration Registry

> **Version:** 2.1.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::Services::MenuRegistry  
> **Dateien:** MenuRegistry.hpp, MenuRegistry.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** ServiceContainer  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Deskriptor-Typen](#2-deskriptor-typen)
3. [API](#3-api)
4. [Registrierung](#4-registrierung)
5. [Abfragen](#5-abfragen)
6. [Beispiele](#6-beispiele)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

### 1.1 Zweck

MenuRegistry ist ein Singleton, das alle Menu-Definitionen speichert:

- **Container:** Submenüs (File, View, Settings, Help)
- **Groups:** Separatoren/Trennlinien
- **Items:** Klickbare Einträge mit Callbacks

### 1.2 Hierarchie

```
toplevel
├── menu.file (Container)
│   ├── menu.file.group.open (Group) ← Separator
│   ├── menu.file.open (Item)
│   ├── menu.file.group.exit (Group) ← Separator
│   └── menu.file.exit (Item)
├── menu.view (Container)
│   ├── menu.view.panels (Container)
│   └── menu.view.perspectives (Container)
├── menu.settings (Container)
│   └── menu.settings.framemode (Container, exclusive=true)
│       ├── menu.settings.framemode.limited (Item, checkable)
│       ├── menu.settings.framemode.unlimited (Item, checkable)
│       └── menu.settings.framemode.vsync (Item, checkable)
└── menu.help (Container)
    └── menu.help.about (Item)
```

---

## 2. Deskriptor-Typen

### 2.1 MenuBaseDesc

Basis für alle Deskriptoren:

```cpp
struct MenuBaseDesc
{
    std::string id;        // Eindeutige ID, z.B. "menu.file.exit"
    std::string parentId;  // Parent-ID, "toplevel" für Hauptmenü
    int order;             // Sortierreihenfolge (aufsteigend)
};
```

### 2.2 MenuContainerDesc

Für Submenüs:

```cpp
struct MenuContainerDesc : MenuBaseDesc
{
    std::string title;   // Anzeige-Titel, z.B. "File"
    bool exclusive;      // true = QActionGroup (Radio-Buttons)
};
```

### 2.3 MenuGroupDesc

Für Separatoren:

```cpp
struct MenuGroupDesc : MenuBaseDesc
{
    // Nur Basis-Felder
    // Erzeugt Separator VOR dem nächsten Item mit höherem order
};
```

### 2.4 MenuItemDesc

Für klickbare Einträge:

```cpp
struct MenuItemDesc : MenuBaseDesc
{
    std::string title;    // Anzeige-Titel, z.B. "Exit"
    
    // Callback bei Klick (erforderlich)
    using Callback = std::function<void(ServiceContainer&)>;
    Callback onClick;
    
    // Optional: Checkbox-Status
    using IsChecked = std::function<bool(ServiceContainer&)>;
    IsChecked isChecked;
    
    // Optional: Aktiviert/Deaktiviert
    using IsEnabled = std::function<bool(ServiceContainer&)>;
    IsEnabled isEnabled;
    
    // Optional: Tastenkürzel, z.B. "Ctrl+O"
    std::string shortcut;
};
```

---

## 3. API

### 3.1 Singleton-Zugriff

```cpp
MenuRegistry& registry = MenuRegistry::instance();
```

### 3.2 Registrierungs-Methoden

| Methode | Beschreibung |
|---------|--------------|
| `registerContainer(desc, replace)` | Submenu registrieren |
| `registerGroup(desc, replace)` | Separator registrieren |
| `registerItem(desc, replace)` | Item registrieren |

**Parameter:**
- `desc`: Deskriptor-Objekt
- `replace`: `true` = überschreiben, `false` = nur wenn neu

### 3.3 Abfrage-Methoden

| Methode | Beschreibung |
|---------|--------------|
| `container(id)` | Container-Deskriptor oder nullptr |
| `group(id)` | Group-Deskriptor oder nullptr |
| `item(id)` | Item-Deskriptor oder nullptr |
| `childrenOf(parentId)` | Sortierte Liste aller Kinder |
| `toplevelContainers()` | Hauptmenü-Container |

### 3.4 MenuNode

Rückgabetyp von `childrenOf()`:

```cpp
struct MenuNode
{
    std::string id;
    MenuNodeType type;  // Container, Group, Item
    int order;
};
```

---

## 4. Registrierung

### 4.1 Direkte Registrierung (Empfohlen)

```cpp
void initMenuAutoReg()
{
    auto& registry = MenuRegistry::instance();
    
    // Container
    registry.registerContainer(
        MenuContainerDesc{
            {"menu.file", "toplevel", 100},
            "File",
            false  // nicht exclusive
        },
        false);
    
    // Item
    registry.registerItem(
        MenuItemDesc{
            {"menu.file.exit", "menu.file", 900},
            "Exit",
            [](ServiceContainer&) { QApplication::quit(); },
            {},      // isChecked (leer = nicht checkable)
            {},      // isEnabled (leer = immer enabled)
            "Alt+F4"
        },
        false);
}
```

### 4.2 Makros (Nur für nicht-statische Libraries)

⚠️ **Warnung:** Funktioniert NICHT zuverlässig bei statischen Libraries!

```cpp
// Container
REGISTER_MENU_CONTAINER("menu.file", "File", "toplevel", 100)

// Exclusive Container (QActionGroup)
REGISTER_MENU_CONTAINER_EXCLUSIVE("menu.settings.framemode", "Frame Mode", 
                                   "menu.settings", 100)

// Separator
REGISTER_MENU_GROUP("menu.file.group.exit", "menu.file", 800)

// Item ohne Shortcut
REGISTER_MENU_ITEM("menu.view.reset", "Reset Layout", "menu.view", 900,
    [](ServiceContainer& svc) { /* ... */ })

// Item mit Shortcut
REGISTER_MENU_ITEM_SHORTCUT("menu.file.exit", "Exit", "menu.file", 900, "Alt+F4",
    [](ServiceContainer& svc) { QApplication::quit(); })

// Checkable Item
REGISTER_MENU_ITEM_CHECKED("menu.settings.framemode.vsync", "VSync", 
    "menu.settings.framemode", 300,
    [](ServiceContainer&) { return currentMode == VSync; },  // isChecked
    [](ServiceContainer&) { setMode(VSync); })               // onClick
```

---

## 5. Abfragen

### 5.1 Einzelne Elemente

```cpp
auto& registry = MenuRegistry::instance();

// Container abfragen
const auto* fileMenu = registry.container("menu.file");
if (fileMenu)
{
    qDebug() << fileMenu->title;  // "File"
}

// Item abfragen
const auto* exitItem = registry.item("menu.file.exit");
if (exitItem)
{
    qDebug() << exitItem->shortcut;  // "Alt+F4"
}
```

### 5.2 Kinder eines Containers

```cpp
auto children = registry.childrenOf("menu.file");
for (const auto& node : children)
{
    switch (node.type)
    {
    case MenuNodeType::Container:
        // Submenu
        break;
    case MenuNodeType::Group:
        // Separator
        break;
    case MenuNodeType::Item:
        // Klickbarer Eintrag
        break;
    }
}
```

### 5.3 Toplevel-Menüs

```cpp
auto topLevel = registry.toplevelContainers();
// ["menu.file", "menu.view", "menu.settings", "menu.help"]
```

---

## 6. Beispiele

### 6.1 Checkable Item mit Event

```cpp
registry.registerItem(
    MenuItemDesc{
        {"menu.settings.framemode.vsync", "menu.settings.framemode", 300},
        "VSync",
        // onClick
        [](ServiceContainer& svc) {
            if (auto* bus = svc.tryResolve<IEventBus>())
            {
                bus->publish(FrameModeChangedEvent{2});
            }
        },
        // isChecked
        [](ServiceContainer& svc) {
            // Hier: Status abfragen (z.B. aus Settings)
            return svc.resolve<Settings>().frameMode() == FrameMode::VSync;
        },
        {},  // isEnabled (immer aktiv)
        {}   // kein Shortcut
    },
    false);
```

### 6.2 Conditional Item (manchmal deaktiviert)

```cpp
registry.registerItem(
    MenuItemDesc{
        {"menu.edit.undo", "menu.edit", 100},
        "Undo",
        [](ServiceContainer& svc) {
            svc.resolve<UndoManager>().undo();
        },
        {},  // nicht checkable
        // isEnabled
        [](ServiceContainer& svc) {
            return svc.resolve<UndoManager>().canUndo();
        },
        "Ctrl+Z"
    },
    false);
```

### 6.3 Dynamisches Submenu

```cpp
// Container
registry.registerContainer(
    MenuContainerDesc{
        {"menu.file.recent", "menu.file", 200},
        "Recent Files",
        false
    },
    false);

// Items dynamisch hinzufügen
for (size_t i = 0; i < recentFiles.size(); ++i)
{
    registry.registerItem(
        MenuItemDesc{
            {"menu.file.recent." + std::to_string(i), "menu.file.recent", static_cast<int>(i)},
            recentFiles[i].filename(),
            [path = recentFiles[i]](ServiceContainer& svc) {
                svc.resolve<FileManager>().open(path);
            },
            {}, {}, {}
        },
        true);  // replace=true für Updates
}
```

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.1.0** | **2025-12-31** | **+exclusive Flag, +REGISTER_MENU_CONTAINER_EXCLUSIVE** |
| 2.0.0 | 2025-12-31 | Refactoring für Self-Registration Pattern |
| 1.0.0 | 2025-12-28 | Initial: Container, Group, Item, Makros |
