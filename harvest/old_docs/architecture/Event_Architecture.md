# Event Architecture

> **Version:** 1.0.0  
> **Datum:** 2025-12-31  
> **Status:** Aktuell

---

## Übersicht

MyViz verwendet das Publish/Subscribe Pattern für lose Kopplung zwischen Komponenten.

```
┌────────────────────┐     ┌────────────────────┐     ┌────────────────────┐
│   Publisher        │────►│     EventBus       │────►│    Subscriber      │
│   (Menu Action)    │     │   (IEventBus)      │     │   (DockManager)    │
└────────────────────┘     └────────────────────┘     └────────────────────┘
```

---

## EventBus Interface

```cpp
class IEventBus
{
public:
    virtual ~IEventBus() = default;
    
    // Subscribe to event type T
    template<typename T>
    int subscribe(std::function<void(const T&)> callback);
    
    // Unsubscribe by ID
    virtual void unsubscribe(int subscriptionId) = 0;
    
    // Publish event to all subscribers
    template<typename T>
    void publish(const T& event);
};
```

---

## Event-Definitionen

Alle UI-Events sind in `include/services/events/UIEvents.hpp` definiert:

```cpp
// Visualizer Events
struct CreateVisualizerEvent { std::string title; };
struct ChangeVisualizerEvent { std::string visualizerId; };

// Layout Events
struct ResetLayoutEvent {};
struct SaveDefaultLayoutEvent {};

// Panel Events
struct TogglePanelEvent { std::string panelId; };

// Dialog Events
struct OpenDialogEvent { std::string dialogId; };

// Settings Events
struct FrameModeChangedEvent { FrameMode mode; };
```

---

## Dezentrale Event-Handler

### Architektur

```
┌──────────────────────────────────────────────────────────────────────────┐
│                              EventBus                                     │
├──────────────────────────────────────────────────────────────────────────┤
│  CreateVisualizerEvent ──► DockManager::subscribeToEvents()              │
│  ResetLayoutEvent      ──► DockManager::subscribeToEvents()              │
│  ChangeVisualizerEvent ──► DockManager::subscribeToEvents()              │
│  TogglePanelEvent      ──► DockManager::subscribeToEvents()              │
│  SaveDefaultLayoutEvent──► DockManager::subscribeToEvents()              │
│  FrameModeChangedEvent ──► MainWindow::setupEventHandlers()              │
│  OpenDialogEvent       ──► MainWindow::setupEventHandlers()              │
└──────────────────────────────────────────────────────────────────────────┘
```

### Vorteile

1. **MainWindow bleibt schlank** - Nur App-spezifische Events
2. **DockManager ist self-contained** - Alle Dock-Events intern
3. **Neue Panels/Events** - Keine MainWindow-Änderung nötig
4. **Testbarkeit** - Handler isoliert testbar

---

## Event-Handler Implementierung

### DockManager (Dock-Events)

```cpp
void DockManager::subscribeToEvents()
{
    auto* eventBus = m_impl->pServices->tryResolve<IEventBus>();
    if (!eventBus) return;
    
    // Create Visualizer
    int id1 = eventBus->subscribe<CreateVisualizerEvent>(
        [this](const CreateVisualizerEvent& e) {
            // Check allowMultiple
            const auto* desc = WidgetRegistry::instance().descriptor("visualizer");
            if (desc && !desc->allowMultiple && !m_impl->visualizers.empty()) {
                return;  // Existiert bereits
            }
            createVisualizer(QString::fromStdString(e.title), DockPosition::Center);
        });
    m_impl->subscriptionIds.push_back(id1);
    
    // Reset Layout
    int id2 = eventBus->subscribe<ResetLayoutEvent>(
        [this](const ResetLayoutEvent&) { resetLayout(); });
    m_impl->subscriptionIds.push_back(id2);
    
    // Toggle Panel
    int id3 = eventBus->subscribe<TogglePanelEvent>(
        [this](const TogglePanelEvent& e) {
            m_impl->pPanelManager->togglePanel(QString::fromStdString(e.panelId));
        });
    m_impl->subscriptionIds.push_back(id3);
    
    // ... weitere Events
}

void DockManager::unsubscribeFromEvents()
{
    auto* eventBus = m_impl->pServices->tryResolve<IEventBus>();
    if (!eventBus) return;
    
    for (int id : m_impl->subscriptionIds) {
        eventBus->unsubscribe(id);
    }
    m_impl->subscriptionIds.clear();
}
```

### MainWindow (App-Events)

```cpp
void MainWindow::setupEventHandlers()
{
    auto* pEventBus = m_pServices->tryResolve<IEventBus>();
    if (!pEventBus) return;
    
    // Frame Mode Changed
    m_subscriptionIds.push_back(
        pEventBus->subscribe<FrameModeChangedEvent>(
            [this](const FrameModeChangedEvent& e) {
                emit frameModeChangeRequested(e.mode);
            }));
    
    // Open Dialog
    m_subscriptionIds.push_back(
        pEventBus->subscribe<OpenDialogEvent>(
            [this](const OpenDialogEvent& e) {
                // TODO: DialogManager
                if (e.dialogId == "about") {
                    AboutDialog dlg(this);
                    dlg.exec();
                }
            }));
}
```

---

## Event publizieren

### Aus MenuAutoReg.cpp

```cpp
void initMenuDefaults(MenuRegistry& registry)
{
    // New Visualizer
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.newvisualizer", "menu.view", 50},
            "New Visualizer",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>()) {
                    eventBus->publish(CreateVisualizerEvent{});
                }
            },
            {}, {}, "Ctrl+N"
        });
    
    // Reset Layout
    registry.registerItem(
        MenuItemDesc{
            {"menu.view.resetlayout", "menu.view", 900},
            "Reset Layout",
            [](ServiceContainer& svc) {
                if (auto* eventBus = svc.tryResolve<IEventBus>()) {
                    eventBus->publish(ResetLayoutEvent{});
                }
            }
        });
}
```

### Aus Panel-Code

```cpp
void VisualSelectPanel::onVisualizerSelected(const QString& vizId)
{
    if (auto* eventBus = m_services.tryResolve<IEventBus>()) {
        eventBus->publish(ChangeVisualizerEvent{vizId.toStdString()});
    }
}
```

---

## Neues Event hinzufügen

### 1. Event-Struct definieren

```cpp
// In UIEvents.hpp
struct MyNewEvent
{
    std::string someData;
    int someValue = 0;
};
```

### 2. Publisher implementieren

```cpp
// In MenuAutoReg.cpp oder Panel-Code
eventBus->publish(MyNewEvent{"data", 42});
```

### 3. Subscriber implementieren

```cpp
// Im passenden Manager
eventBus->subscribe<MyNewEvent>(
    [this](const MyNewEvent& e) {
        // Handle event
        doSomething(e.someData, e.someValue);
    });
```

---

## Lifecycle

### Subscription-Cleanup

**Wichtig:** Subscriptions müssen beim Zerstören aufgeräumt werden!

```cpp
class MyComponent
{
    std::vector<int> m_subscriptionIds;
    
    ~MyComponent()
    {
        if (auto* eventBus = m_services.tryResolve<IEventBus>()) {
            for (int id : m_subscriptionIds) {
                eventBus->unsubscribe(id);
            }
        }
    }
};
```

### Thread-Safety

Der aktuelle EventBus ist **nicht thread-safe**. Alle publish/subscribe Aufrufe müssen vom Main-Thread erfolgen.

---

## Best Practices

### 1. Event-Granularität

- Ein Event pro Aktion (nicht "DoEverythingEvent")
- Daten im Event-Struct, nicht via globaler State

### 2. Handler-Verantwortung

- Handler sollte nur eine Sache tun
- Bei komplexer Logik: Handler ruft Manager-Methode auf

### 3. Subscription-IDs aufbewahren

- Immer in `std::vector<int>` speichern
- Im Destruktor aufräumen

### 4. Fehler-Handling

- EventBus kann nullptr sein (Service nicht registriert)
- Immer `tryResolve` mit null-check verwenden

---

## Siehe auch

- [Registry Architecture](Registry_Architecture.md) - Wie Registries funktionieren
- [Panel System](../modules/Panel_System.md) - Panel-Events im Detail
- [Menu System](../modules/Menu_System.md) - Menü-Events im Detail
