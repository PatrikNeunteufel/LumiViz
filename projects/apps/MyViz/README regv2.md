# Menu Fix v2 - Direkte Registrierung

## Problem

Die statischen Makros (REGISTER_MENU_CONTAINER, etc.) werden vom Linker entfernt, weil sie keine externen Referenzen erzeugen. Eine leere Init-Funktion hilft nicht.

## Lösung

Die Init-Funktionen führen jetzt die **tatsächliche Registrierung** durch (nicht mehr über statische Makros):

```cpp
// VORHER (funktioniert nicht bei statischen Libraries):
void initMenuAutoReg() 
{
    // Leer - hilft nicht!
}

REGISTER_MENU_CONTAINER(...)  // Wird vom Linker entfernt!

// NACHHER (funktioniert):
void initMenuAutoReg() 
{
    auto& registry = MenuRegistry::instance();
    registry.registerContainer(...);  // Direkte Registrierung
}
```

## Geänderte Dateien

| Datei | Änderung |
|-------|----------|
| **MenuAutoReg.cpp** | Direkte Registrierung in `initMenuAutoReg()` |
| **MenuItemsAutoReg.cpp** | Direkte Registrierung in `initMenuItemsAutoReg()` |
| **MainWindow.cpp** | Aufruf von `initMenuRegistrations()` VOR `buildMenuBar()` |
| **MenuInit.hpp** | Unverändert |

## Verwendung

```cpp
// In MainWindow.cpp:
#include "UI/managers/MenuInit.hpp"

void MainWindow::setupMenuBar()
{
    // WICHTIG: Muss VOR buildMenuBar() aufgerufen werden!
    initMenuRegistrations();
    
    m_pMenuManager = std::make_unique<MenuManager>(*m_pServices, this);
    m_pMenuManager->buildMenuBar(menuBar());
    // ...
}
```

## Hinweis

Deine hochgeladene Version hatte den `initMenuRegistrations()` Aufruf NICHT drin - das war der Hauptfehler. Aber selbst mit dem Aufruf hätten die leeren Init-Funktionen nicht geholfen. Jetzt machen die Init-Funktionen die tatsächliche Arbeit.
