# Application Integration Guide

> **Version:** 1.0.0  
> **Datum:** 2025-12-30  
> **Typ:** Tutorial  

---

## Übersicht

Diese Anleitung zeigt, wie die Manager in `Application` und `MainWindow` integriert werden.

---

## 1. Application.hpp erweitern

```cpp
// Application.hpp - Neue Includes und Members

#include "services/ServiceContainer.hpp"
#include <memory>

// Forward declarations
class IEventBus;
class PanelManager;
class MenuManager;
class DialogManager;

class Application : public QObject
{
    Q_OBJECT

public:
    // ... existing ...

    // Service Access
    ServiceContainer& services() { return m_services; }

private:
    bool initServices();
    bool initManagers();

    // Service Container
    ServiceContainer m_services;

    // Managers (owned by Application, not MainWindow)
    std::unique_ptr<PanelManager> m_panelManager;
    std::unique_ptr<MenuManager> m_menuManager;
    std::unique_ptr<DialogManager> m_dialogManager;
};
```

---

## 2. Application.cpp - Service Initialization

```cpp
#include "services/ServiceContainer.hpp"
#include "services/EventBus.hpp"
#include "services/IEventBus.hpp"
#include "UI/Managers/PanelManager.hpp"
#include "UI/Managers/MenuManager.hpp"
#include "UI/Managers/DialogManager.hpp"

bool Application::init()
{
    // 1. Initialize services FIRST
    if (!initServices())
    {
        return false;
    }

    // 2. Create MainWindow
    m_mainWindow = std::make_unique<MainWindow>();

    // 3. Initialize managers (need MainWindow)
    if (!initManagers())
    {
        return false;
    }

    // 4. Show window
    m_mainWindow->show();

    return true;
}

bool Application::initServices()
{
    // Register EventBus
    m_services.registerSingleton<IEventBus, EventBus>();

    // Register other services as needed
    // m_services.registerSingleton<IAudioService, AudioService>();

    return true;
}

bool Application::initManagers()
{
    // Get DockManager from MainWindow
    ads::CDockManager* dockManager = m_mainWindow->dockManager();

    // 1. Create PanelManager
    m_panelManager = std::make_unique<PanelManager>(
        m_services, dockManager, this);

    // 2. Create MenuManager
    m_menuManager = std::make_unique<MenuManager>(m_services, this);
    m_menuManager->setPanelManager(m_panelManager.get());

    // 3. Create DialogManager
    m_dialogManager = std::make_unique<DialogManager>(
        m_services, m_mainWindow.get());
    m_dialogManager->subscribeToEvents();

    // 4. Build menus
    m_menuManager->buildMenuBar(m_mainWindow->menuBar());

    // 5. Create all panels
    m_panelManager->createAllPanels();

    return true;
}
```

---

## 3. MainWindow Anpassungen

MainWindow muss DockManager und MenuBar zugänglich machen:

```cpp
// MainWindow.hpp
class MainWindow : public QMainWindow
{
public:
    // Accessor for DockManager
    ads::CDockManager* dockManager() { return m_dockManager; }

    // MenuBar is already accessible via QMainWindow::menuBar()
};
```

---

## 4. Initialization Order

**Wichtig:** Die Reihenfolge ist entscheidend!

```
1. ServiceContainer
   └── IEventBus registrieren

2. MainWindow erstellen
   └── DockManager erstellen

3. PanelManager
   └── Braucht: ServiceContainer, DockManager

4. MenuManager
   └── Braucht: ServiceContainer, PanelManager

5. DialogManager
   └── Braucht: ServiceContainer, MainWindow (als parent)
   └── subscribeToEvents() aufrufen

6. menuManager->buildMenuBar()

7. panelManager->createAllPanels()

8. mainWindow->show()
```

---

## 5. Shutdown Order

```cpp
Application::~Application()
{
    // Managers in umgekehrter Reihenfolge zerstören
    m_dialogManager.reset();
    m_menuManager.reset();
    m_panelManager.reset();

    // MainWindow
    m_mainWindow.reset();

    // Services (EventBus etc.) werden automatisch aufgeräumt
    m_services.clear();
}
```

---

## 6. Event Flow Beispiel

```
User clicks "Help > About..."
        │
        ▼
MenuManager::onActionTriggered()
        │
        ▼
MenuRegistry callback ausführen
        │
        ▼
eventBus.publish(OpenDialogEvent{"about"})
        │
        ▼
DialogManager empfängt Event
        │
        ▼
DialogManager::show("about")
        │
        ▼
DialogRegistry::create("about", services, parent)
        │
        ▼
AboutDialog wird angezeigt
```

---

## 7. Panel Flow Beispiel

```
User clicks "View > Panels > Playlist"
        │
        ▼
MenuManager::onActionTriggered()
        │
        ▼
m_panelManager->togglePanel("playlist")
        │
        ▼
PanelManager::togglePanel()
        │
        ▼
DockWidget::toggleView()
        │
        ▼
PanelManager::onDockWidgetVisibilityChanged()
        │
        ▼
emit panelVisibilityChanged("playlist", visible)
        │
        ▼
MenuManager::onPanelVisibilityChanged()
        │
        ▼
QAction::setChecked(visible)
```

---

## Checkliste

- [ ] `Application.hpp` um ServiceContainer und Manager erweitern
- [ ] `Application::initServices()` implementieren
- [ ] `Application::initManagers()` implementieren
- [ ] `MainWindow::dockManager()` Accessor hinzufügen
- [ ] Source.cmake für `managers/` Ordner erstellen (lowercase!)
- [ ] Include-Pfade in .cpp Dateien an Verzeichnisnamen anpassen (lowercase!)
