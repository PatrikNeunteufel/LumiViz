# Application Integration Guide

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Status:** Aktuell

---

## Übersicht

Diese Anleitung zeigt, wie alle Komponenten in `Application` und `MainWindow` integriert werden.

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           Application                                    │
├─────────────────────────────────────────────────────────────────────────┤
│  1. QApplication         │  2. ServiceContainer     │  3. MainWindow    │
│     (Qt Event Loop)      │     (DI Container)       │     (UI Host)     │
└─────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                           MainWindow                                     │
├─────────────────────────────────────────────────────────────────────────┤
│  DockManager    │    MenuManager    │    DialogManager    │  StatusBar  │
│  (Qt-ADS)       │    (Menü-Aufbau)  │    (Dialog-Handler) │  (FPS etc.) │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Initialization Order

**Kritisch:** Die Reihenfolge muss eingehalten werden!

```
┌─────────────────────────────────────────────────────────────────────────┐
│ 1. Application erstellt                                                  │
├─────────────────────────────────────────────────────────────────────────┤
│    • QApplication erstellen                                              │
│    • ServiceContainer erstellen                                          │
│    • IEventBus registrieren                                              │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ 2. MainWindow erstellt                                                   │
├─────────────────────────────────────────────────────────────────────────┤
│    • DockManager erstellt                                                │
│    •   → PanelManager erstellt                                           │
│    •   → createAllPanels() (alle Panels erstellt)                        │
│    • MenuManager erstellt + buildMenuBar()                               │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ 3. Initial Content                                                       │
├─────────────────────────────────────────────────────────────────────────┤
│    • createVisualizer() - initialen Visualizer erstellen                │
│    • restoreLayout() - Layout wiederherstellen                          │
│    •   oder applyDefaultVisibility() wenn kein Layout                   │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ 4. MainWindow anzeigen                                                   │
├─────────────────────────────────────────────────────────────────────────┤
│    • show()                                                              │
│    • Event Loop starten                                                  │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## MainWindow Aufbau

```cpp
// MainWindow.hpp
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    // DockManager Access
    DockManager* dockManager() { return m_pDockManager.get(); }
    
    // Visualizer Access
    std::vector<VisualizerWidget*> visualizers();

private:
    void setupUI();
    void setupDockManager();
    void setupMenuBar();
    void setupStatusBar();
    void setupInitialContent();
    void setupEventHandlers();

    std::unique_ptr<ServiceContainer> m_pServices;
    std::unique_ptr<DockManager> m_pDockManager;
    std::unique_ptr<MenuManager> m_pMenuManager;
    
    // Event Subscriptions
    std::vector<int> m_subscriptionIds;
};
```

### MainWindow::MainWindow()

```cpp
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // 1. ServiceContainer erstellen
    m_pServices = std::make_unique<ServiceContainer>();
    
    // 2. EventBus registrieren
    m_pServices->registerSingleton<IEventBus, EventBus>();
    
    // 3. UI aufbauen
    setupUI();
    setupDockManager();    // Erstellt DockManager + PanelManager
    setupMenuBar();        // Erstellt MenuManager + buildMenuBar()
    setupStatusBar();
    setupInitialContent(); // Visualizer + Layout Restore
    setupEventHandlers();
}
```

### setupDockManager()

```cpp
void MainWindow::setupDockManager()
{
    // DockManager erstellt:
    // - ads::CDockManager
    // - PanelManager (+ createAllPanels)
    // - Event Subscriptions
    m_pDockManager = std::make_unique<DockManager>(*m_pServices, this);
}
```

### setupMenuBar()

```cpp
void MainWindow::setupMenuBar()
{
    m_pMenuManager = std::make_unique<MenuManager>(*m_pServices, this);
    m_pMenuManager->buildMenuBar(menuBar());
    
    // Panels-Menü von DockManager befüllen
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

### setupInitialContent()

```cpp
void MainWindow::setupInitialContent()
{
    // 1. Visualizer erstellen BEVOR Layout restored wird
    auto* pVisualizer = m_pDockManager->createVisualizer(
        QString(), DockPosition::Center);
    
    if (pVisualizer)
    {
        pVisualizer->setVisualizer("pulsing");
    }
    
    // 2. Layout wiederherstellen
    m_pDockManager->restoreLayout();
}
```

---

## Service Registration

### In Application oder MainWindow

```cpp
void registerServices(ServiceContainer& services)
{
    // Core Services
    services.registerSingleton<IEventBus, EventBus>();
    
    // Audio Services
    services.registerSingleton<IAudioEngine, BassEngine>();
    services.registerSingleton<IAudioPlayer>([&services]() {
        return std::make_unique<AudioPlayer>(
            services.resolve<IAudioEngine>(),
            services.resolve<IEventBus>());
    });
    
    // ... weitere Services
}
```

### Registries (Automatisch)

Registries werden beim ersten `instance()` Aufruf initialisiert:

```cpp
// Automatisch bei erster Verwendung:
auto& menuReg = MenuRegistry::instance();      // → initMenuDefaults()
auto& panelReg = PanelRegistry::instance();    // → initPanelDefaults()
auto& dialogReg = DialogRegistry::instance();  // → initDialogDefaults()
auto& widgetReg = WidgetRegistry::instance();  // → initWidgetDefaults()
auto& vizReg = VisualizerRegistry::instance(); // → initVisualizerDefaults()
```

---

## Event Flow

### Menü → DockManager

```
User klickt "View → New Visualizer"
        │
        ▼
MenuManager::onActionTriggered()
        │
        ▼
MenuRegistry callback: eventBus.publish(CreateVisualizerEvent{})
        │
        ▼
DockManager::subscribeToEvents() empfängt Event
        │
        ▼
DockManager::createVisualizer()
```

### Panel → Visualizer

```
User wählt Visualizer in VisualSelectPanel
        │
        ▼
VisualSelectPanel::onVisualizerSelected("bars")
        │
        ▼
eventBus.publish(ChangeVisualizerEvent{"bars"})
        │
        ▼
DockManager empfängt Event
        │
        ▼
m_impl->visualizers[0]->setVisualizer("bars")
```

---

## Shutdown Order

```cpp
MainWindow::~MainWindow()
{
    // Event Subscriptions aufräumen
    if (auto* eventBus = m_pServices->tryResolve<IEventBus>())
    {
        for (int id : m_subscriptionIds)
        {
            eventBus->unsubscribe(id);
        }
    }
    
    // Manager in umgekehrter Reihenfolge zerstören
    m_pMenuManager.reset();
    m_pDockManager.reset();  // Speichert Layout via aboutToQuit
    
    // Services aufräumen
    m_pServices.reset();
}
```

---

## Wichtige Hinweise

### 1. Layout-Restore nach Widget-Erstellung

```cpp
// FALSCH:
restoreLayout();      // ← Visualizer existiert noch nicht!
createVisualizer();

// RICHTIG:
createVisualizer();   // ← Erst erstellen
restoreLayout();      // ← Dann Layout wiederherstellen
```

### 2. aboutToQuit für Layout-Speicherung

```cpp
// Im DockManager-Konstruktor:
connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
    saveLayoutToSettings();  // VOR Destruktor!
});
```

### 3. Event-Cleanup in Destruktoren

```cpp
// IMMER Subscriptions aufräumen:
~MyComponent()
{
    if (auto* eventBus = m_services.tryResolve<IEventBus>())
    {
        for (int id : m_subscriptionIds)
        {
            eventBus->unsubscribe(id);
        }
    }
}
```

---

## Checkliste

- [ ] ServiceContainer erstellt
- [ ] IEventBus registriert
- [ ] DockManager erstellt (inkl. PanelManager)
- [ ] MenuManager erstellt + buildMenuBar()
- [ ] Panels-Menü von DockManager befüllt
- [ ] Initial-Visualizer erstellt
- [ ] restoreLayout() nach Widget-Erstellung
- [ ] aboutToQuit Signal für Layout-Speicherung
- [ ] Event-Cleanup in Destruktoren

---

## Siehe auch

- [Registry Architecture](../architecture/Registry_Architecture.md)
- [Event Architecture](../architecture/Event_Architecture.md)
- [Layout Persistence](../architecture/Layout_Persistence.md)
- [Panel System](../modules/Panel_System.md)
- [Menu System](../modules/Menu_System.md)
