# Panel System

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Status:** Aktuell

---

## Übersicht

Das Panel-System verwendet Qt-ADS für dockbare Panels mit Self-Registration.

```
┌──────────────────────────────────────────────────────────────────────────┐
│                           Panel System                                    │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│   ┌─────────────────────┐         ┌─────────────────────┐                │
│   │   PanelRegistry     │◄────────│   PanelAutoReg.cpp  │                │
│   │  (Panel-Deskriptor) │         │   (Registrierungen) │                │
│   └─────────┬───────────┘         └─────────────────────┘                │
│             │                                                             │
│             ▼                                                             │
│   ┌─────────────────────┐         ┌─────────────────────┐                │
│   │   PanelManager      │────────►│  ads::CDockManager  │                │
│   │ (Panel-Erstellung)  │         │   (Qt-ADS)          │                │
│   └─────────┬───────────┘         └─────────────────────┘                │
│             │                                                             │
│             ▼                                                             │
│   ┌─────────────────────┐                                                │
│   │    DockManager      │                                                │
│   │   (Event-Handler)   │                                                │
│   └─────────────────────┘                                                │
│                                                                           │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## Registrierte Panels

| ID | Titel | Order | Default Visible | Beschreibung |
|----|-------|-------|-----------------|--------------|
| player | Player | 100 | ✅ | Audio Player Controls |
| playlist | Playlist | 200 | ✅ | Playlist Management |
| config | Settings | 300 | ❌ | Visualizer Configuration |
| visual_select | Visualizers | 400 | ✅ | Visualizer Selection |

---

## PanelBase

Basisklasse für alle Panels:

```cpp
class PanelBase : public QWidget
{
    Q_OBJECT

public:
    explicit PanelBase(ServiceContainer& services, QWidget* parent = nullptr);
    virtual ~PanelBase();
    
    // Lifecycle
    virtual void onActivate() {}    // Panel wird sichtbar
    virtual void onDeactivate() {}  // Panel wird versteckt
    
    // Area Preference
    virtual int preferredArea() const { return Qt::LeftDockWidgetArea; }

protected:
    ServiceContainer& m_services;
};
```

---

## Panel-Implementierungen

### PlayerPanel

```cpp
class PlayerPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit PlayerPanel(ServiceContainer& services, QWidget* parent = nullptr);
    
    int preferredArea() const override { return Qt::BottomDockWidgetArea; }
    void onActivate() override;
    void onDeactivate() override;

private:
    void setupUI();
    void connectEvents();
    
    // UI Elements
    QPushButton* m_pPlayButton = nullptr;
    QPushButton* m_pStopButton = nullptr;
    QSlider* m_pVolumeSlider = nullptr;
    QSlider* m_pProgressSlider = nullptr;
    QLabel* m_pTrackLabel = nullptr;
    QLabel* m_pTimeLabel = nullptr;
    
    // Event Subscriptions
    std::vector<int> m_subscriptionIds;
};
```

### PlaylistPanel

```cpp
class PlaylistPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit PlaylistPanel(ServiceContainer& services, QWidget* parent = nullptr);
    
    int preferredArea() const override { return Qt::RightDockWidgetArea; }

private:
    void setupUI();
    void connectEvents();
    
    QListWidget* m_pTrackList = nullptr;
    QPushButton* m_pAddButton = nullptr;
    QPushButton* m_pRemoveButton = nullptr;
    QPushButton* m_pClearButton = nullptr;
    
    std::vector<int> m_subscriptionIds;
};
```

### ConfigPanel

```cpp
class ConfigPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ConfigPanel(ServiceContainer& services, QWidget* parent = nullptr);
    
    int preferredArea() const override { return Qt::RightDockWidgetArea; }

private:
    void setupUI();
    
    QComboBox* m_pVisualizerCombo = nullptr;
    QSlider* m_pSensitivitySlider = nullptr;
    QCheckBox* m_pSmoothingCheck = nullptr;
};
```

### VisualSelectPanel

```cpp
class VisualSelectPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit VisualSelectPanel(ServiceContainer& services, QWidget* parent = nullptr);
    
    int preferredArea() const override { return Qt::LeftDockWidgetArea; }

private:
    void setupUI();
    void populateVisualizers();
    void onVisualizerSelected(const QString& vizId);
    
    QListWidget* m_pVisualizerList = nullptr;
    
    std::vector<int> m_subscriptionIds;
};
```

---

## PanelAutoReg.cpp

```cpp
#include "services/PanelRegistry.hpp"
#include "UI/panels/PlayerPanel.hpp"
#include "UI/panels/PlaylistPanel.hpp"
#include "UI/panels/ConfigPanel.hpp"
#include "UI/panels/VisualSelectPanel.hpp"

void initPanelDefaults(PanelRegistry& registry)
{
    // Player Panel
    registry.registerPanel(
        PanelDescriptor{
            "player",           // id
            "Player",           // title
            100,                // order
            true,               // defaultVisible
            "View/Panels"       // menuPath
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<PlayerPanel>(svc);
        });
    
    // Playlist Panel
    registry.registerPanel(
        PanelDescriptor{
            "playlist",
            "Playlist",
            200,
            true,
            "View/Panels"
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<PlaylistPanel>(svc);
        });
    
    // Config Panel (Settings) - hidden by default
    registry.registerPanel(
        PanelDescriptor{
            "config",
            "Settings",
            300,
            false,              // defaultVisible = false
            "View/Panels"
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<ConfigPanel>(svc);
        });
    
    // Visual Select Panel
    registry.registerPanel(
        PanelDescriptor{
            "visual_select",
            "Visualizers",
            400,
            true,
            "View/Panels"
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<VisualSelectPanel>(svc);
        });
}
```

---

## Event-Integration

### EventBus Subscriptions

```cpp
void PlayerPanel::onActivate()
{
    auto* eventBus = m_services.tryResolve<IEventBus>();
    if (!eventBus) return;
    
    // Playback State
    m_subscriptionIds.push_back(
        eventBus->subscribe<PlaybackStateEvent>(
            [this](const PlaybackStateEvent& e) {
                bool playing = (e.state == PlaybackState::Playing);
                m_pPlayButton->setIcon(playing ? pauseIcon() : playIcon());
            }));
    
    // Track Changed
    m_subscriptionIds.push_back(
        eventBus->subscribe<TrackChangedEvent>(
            [this](const TrackChangedEvent& e) {
                m_pTrackLabel->setText(QString::fromStdString(e.track.title));
            }));
    
    // Position Update
    m_subscriptionIds.push_back(
        eventBus->subscribe<PlaybackPositionEvent>(
            [this](const PlaybackPositionEvent& e) {
                m_pProgressSlider->setValue(static_cast<int>(e.progress * 100));
            }));
}

void PlayerPanel::onDeactivate()
{
    auto* eventBus = m_services.tryResolve<IEventBus>();
    if (!eventBus) return;
    
    for (int id : m_subscriptionIds) {
        eventBus->unsubscribe(id);
    }
    m_subscriptionIds.clear();
}
```

### Event Publishing

```cpp
void VisualSelectPanel::onVisualizerSelected(const QString& vizId)
{
    auto* eventBus = m_services.tryResolve<IEventBus>();
    if (eventBus) {
        eventBus->publish(ChangeVisualizerEvent{vizId.toStdString()});
    }
}
```

---

## Dezentrale Event-Architektur

DockManager handhabt alle Panel-Events:

```
┌──────────────────────────────────────────────────────────────────────────┐
│                              EventBus                                     │
├──────────────────────────────────────────────────────────────────────────┤
│  CreateVisualizerEvent ──► DockManager                                   │
│  ResetLayoutEvent      ──► DockManager                                   │
│  ChangeVisualizerEvent ──► DockManager                                   │
│  TogglePanelEvent      ──► DockManager (via PanelManager)                │
│  SaveDefaultLayoutEvent──► DockManager                                   │
└──────────────────────────────────────────────────────────────────────────┘
```

### Vorteil

Neue Panels erfordern KEINE Änderung an MainWindow:
1. Panel-Klasse erstellen
2. In PanelAutoReg.cpp registrieren
3. **Fertig**

---

## PanelManager

Verwaltet Panel-Instanzen:

```cpp
class PanelManager : public QObject
{
public:
    PanelManager(ServiceContainer& services, ads::CDockManager* dockManager, QObject* parent);
    
    void createAllPanels();
    void applyDefaultVisibility();
    
    PanelBase* panel(const QString& panelId) const;
    ads::CDockWidget* dockWidget(const QString& panelId) const;
    
    bool isPanelVisible(const QString& panelId) const;
    void showPanel(const QString& panelId);
    void hidePanel(const QString& panelId);
    void togglePanel(const QString& panelId);

private:
    ServiceContainer& m_services;
    ads::CDockManager* m_dockManager;
    QHash<QString, PanelBase*> m_panels;
    QHash<QString, ads::CDockWidget*> m_dockWidgets;
};
```

### Wichtig: createAllPanels vs. applyDefaultVisibility

```cpp
void PanelManager::createAllPanels()
{
    // NUR erstellen, KEINE Sichtbarkeit setzen!
    // (wegen Layout-Restore)
    for (const auto& desc : descriptors) {
        createPanel(desc.id);
    }
}

void PanelManager::applyDefaultVisibility()
{
    // NUR aufrufen wenn KEIN Layout restored wurde!
    for (const auto& desc : descriptors) {
        if (!desc.defaultVisible) {
            m_dockWidgets[desc.id]->closeDockWidget();
        }
    }
}
```

---

## Neues Panel hinzufügen

### 1. Panel-Klasse erstellen

```cpp
// include/UI/panels/MyPanel.hpp
class MyPanel : public PanelBase
{
    Q_OBJECT
public:
    explicit MyPanel(ServiceContainer& services, QWidget* parent = nullptr);
    int preferredArea() const override { return Qt::LeftDockWidgetArea; }
    void onActivate() override;
    void onDeactivate() override;
private:
    void setupUI();
    std::vector<int> m_subscriptionIds;
};

// src/UI/panels/MyPanel.cpp
MyPanel::MyPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, parent)
{
    setupUI();
}

void MyPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("My Panel Content"));
}
```

### 2. In PanelAutoReg.cpp registrieren

```cpp
#include "UI/panels/MyPanel.hpp"

void initPanelDefaults(PanelRegistry& registry)
{
    // ... bestehende Panels ...
    
    // My Panel
    registry.registerPanel(
        PanelDescriptor{
            "mypanel",
            "My Panel",
            500,
            true,
            "View/Panels"
        },
        [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {
            return std::make_unique<MyPanel>(svc);
        });
}
```

### 3. In Source.cmake eintragen

```cmake
# src/UI/panels/Source.cmake
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/PanelAutoReg.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PlayerPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PlaylistPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ConfigPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/VisualSelectPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MyPanel.cpp"  # ← NEU
)
```

---

## Best Practices

### 1. Event-Cleanup

```cpp
void MyPanel::onDeactivate()
{
    // IMMER Subscriptions aufräumen!
    auto* eventBus = m_services.tryResolve<IEventBus>();
    if (eventBus) {
        for (int id : m_subscriptionIds) {
            eventBus->unsubscribe(id);
        }
    }
    m_subscriptionIds.clear();
}
```

### 2. Stabile objectName

- Panel-ID = objectName (für Layout-Persistence)
- ID niemals ändern nach Release!

### 3. preferredArea

- Sinnvolle Default-Position wählen
- User kann per Drag&Drop ändern

---

## Siehe auch

- [Registry Architecture](../architecture/Registry_Architecture.md) - Registry Grundlagen
- [Layout Persistence](../architecture/Layout_Persistence.md) - Layout-Speicherung
- [Event Architecture](../architecture/Event_Architecture.md) - EventBus Details
