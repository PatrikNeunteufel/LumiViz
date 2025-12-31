# Panels — Dock-Panel Architektur

> **Version:** 1.0.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::UI::Panels  
> **Dateien:** IPanel.hpp, PanelBase.hpp, PanelBase.cpp, *Panel.hpp/cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6, Qt-ADS, ServiceContainer, EventBus  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [PanelBase API](#3-panelbase-api)
4. [Panel erstellen](#4-panel-erstellen)
5. [Panel-Lifecycle](#5-panel-lifecycle)
6. [Existierende Panels](#6-existierende-panels)
7. [Best Practices](#7-best-practices)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

### 1.1 Zweck

**Panels** sind dockbare UI-Komponenten, die mit Qt-ADS (Advanced Docking System) verwaltet werden. Sie können angedockt, schwebend, tabuliert oder versteckt werden.

### 1.2 Panel-Typen

| Panel | Beschreibung | Default |
|-------|--------------|---------|
| **PlayerPanel** | Play/Pause, Volume, Progress | Sichtbar |
| **PlaylistPanel** | Track-Liste, Drag&Drop | Sichtbar |
| **VisualSelectPanel** | Visualizer-Auswahl | Sichtbar |
| **ConfigPanel** | Einstellungen | Versteckt |

### 1.3 Docking-Features

```
┌─────────────────────────────────────────────────────────────────────────┐
│ MyViz                                                     [_][□][X]    │
├─────────────────────────────────────────────────────────────────────────┤
│ File  Edit  View  Help                                                  │
├─────────┬───────────────────────────────────────────────┬───────────────┤
│ Player  │                                               │ Visualizers   │
│ ┌─────┐ │              Visualization                    │ ┌───────────┐ │
│ │ ▶   │ │                  Area                         │ │ ○ Pulsing │ │
│ │ ■   │ │                                               │ │ ○ Spectrum│ │
│ │ 🔊  │ │                                               │ │ ○ Waveform│ │
│ └─────┘ │                                               │ └───────────┘ │
├─────────┴───────────────────────────────────────────────┴───────────────┤
│ Playlist                                                                │
│ ┌─────────────────────────────────────────────────────────────────────┐ │
│ │ 1. Track One                                                        │ │
│ │ 2. Track Two                                                        │ │
│ └─────────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Architektur

### 2.1 Klassendiagramm

```
                    ┌─────────────────────┐
                    │       IPanel        │  (Interface)
                    ├─────────────────────┤
                    │ + panelId()         │
                    │ + panelTitle()      │
                    │ + preferredArea()   │
                    │ + saveState()       │
                    │ + restoreState()    │
                    └──────────┬──────────┘
                               │
                               ▼
         ┌─────────────────────────────────────────┐
         │              PanelBase                   │
         │  (QWidget + IPanel Implementation)       │
         ├─────────────────────────────────────────┤
         │ # m_services : ServiceContainer&         │
         │ # m_panelId : QString                    │
         │ # m_isActive : bool                      │
         ├─────────────────────────────────────────┤
         │ + setActive(bool)                        │
         │ # services() : ServiceContainer&         │
         │ # eventBus() : IEventBus*                │
         │ # onActivate() [virtual]                 │
         │ # onDeactivate() [virtual]               │
         └──────────┬──────────────────────────────┘
                    │
    ┌───────────────┼───────────────┬───────────────┐
    ▼               ▼               ▼               ▼
┌─────────┐   ┌─────────┐   ┌─────────────┐   ┌──────────┐
│ Player  │   │ Playlist│   │ VisualSelect│   │ Config   │
│ Panel   │   │ Panel   │   │    Panel    │   │ Panel    │
└─────────┘   └─────────┘   └─────────────┘   └──────────┘
```

### 2.2 Integration mit Qt-ADS

```
┌──────────────────────────────────────────────────────────────────┐
│                        DockManager                                │
│  (Verwaltet alle CDockWidgets)                                   │
├──────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌──────────────────┐    ┌──────────────────┐                    │
│  │  CDockWidget     │    │  CDockWidget     │                    │
│  │  "Player"        │    │  "Playlist"      │                    │
│  ├──────────────────┤    ├──────────────────┤                    │
│  │  PlayerPanel     │    │  PlaylistPanel   │                    │
│  │  (PanelBase)     │    │  (PanelBase)     │                    │
│  └──────────────────┘    └──────────────────┘                    │
│                                                                   │
└──────────────────────────────────────────────────────────────────┘
```

---

## 3. PanelBase API

### 3.1 Konstruktor

```cpp
PanelBase(ServiceContainer& services,
          const QString& id,
          const QString& title,
          QWidget* parent = nullptr);
```

### 3.2 IPanel-Methoden

```cpp
// Identifikation
[[nodiscard]] QString panelId() const;    // z.B. "player"
[[nodiscard]] QString panelTitle() const; // z.B. "Player Controls"

// Dock-Position (override in Subklasse)
[[nodiscard]] virtual int preferredArea() const; // ads::DockWidgetArea

// State-Persistenz
virtual void saveState();
virtual void restoreState();
```

### 3.3 Activation

```cpp
// Prüfen ob Panel aktiv ist
[[nodiscard]] bool isActive() const;

// Setzen (von DockManager aufgerufen)
void setActive(bool active);

// Signals
void activated();
void deactivated();
void closeRequested();
```

### 3.4 Protected Helpers

```cpp
// ServiceContainer-Zugriff
ServiceContainer& services();

// EventBus-Zugriff (Convenience)
IEventBus* eventBus() const;

// Settings-Prefix
QString settingsPrefix() const;  // "panels/{id}/"

// Lifecycle-Hooks (override in Subklasse)
virtual void onActivate() {}
virtual void onDeactivate() {}
```

---

## 4. Panel erstellen

### 4.1 Header-Datei

```cpp
// PlayerPanel.hpp
#pragma once

#include "PanelBase.hpp"

class QLabel;
class QPushButton;
class QSlider;

class PlayerPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit PlayerPanel(ServiceContainer& services, 
                         QWidget* parent = nullptr);
    ~PlayerPanel() override = default;

    // Preferred dock area (optional)
    [[nodiscard]] int preferredArea() const override;

protected:
    void onActivate() override;
    void onDeactivate() override;
    void saveState() override;
    void restoreState() override;

private:
    void setupUI();
    void setupConnections();

    // UI Elements
    QPushButton* m_pPlayButton = nullptr;
    QPushButton* m_pStopButton = nullptr;
    QSlider* m_pVolumeSlider = nullptr;
    QLabel* m_pTrackLabel = nullptr;
};
```

### 4.2 Implementation

```cpp
// PlayerPanel.cpp
#include "PlayerPanel.hpp"
#include "services/PanelRegistry.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "audio/AudioEvents.hpp"

#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

#include <DockAreaWidget.h>  // Qt-ADS

PlayerPanel::PlayerPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "player", tr("Player"), parent)
{
    setupUI();
    setupConnections();
}

int PlayerPanel::preferredArea() const
{
    return ads::LeftDockWidgetArea;
}

void PlayerPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    
    m_pTrackLabel = new QLabel(tr("No track loaded"));
    m_pPlayButton = new QPushButton(tr("Play"));
    m_pStopButton = new QPushButton(tr("Stop"));
    m_pVolumeSlider = new QSlider(Qt::Horizontal);
    m_pVolumeSlider->setRange(0, 100);
    m_pVolumeSlider->setValue(80);
    
    layout->addWidget(m_pTrackLabel);
    layout->addWidget(m_pPlayButton);
    layout->addWidget(m_pStopButton);
    layout->addWidget(m_pVolumeSlider);
    layout->addStretch();
}

void PlayerPanel::setupConnections()
{
    connect(m_pPlayButton, &QPushButton::clicked, this, [this]() {
        auto& player = services().resolve<IAudioPlayer>();
        player.togglePlayPause();
    });
    
    connect(m_pStopButton, &QPushButton::clicked, this, [this]() {
        auto& player = services().resolve<IAudioPlayer>();
        player.stop();
    });
}

void PlayerPanel::onActivate()
{
    // Subscribe to events
    if (auto* bus = eventBus()) {
        m_trackSubId = bus->subscribe<TrackChangedEvent>(
            [this](const TrackChangedEvent& e) {
                m_pTrackLabel->setText(e.track.title);
            });
    }
}

void PlayerPanel::onDeactivate()
{
    // Unsubscribe
    if (auto* bus = eventBus()) {
        bus->unsubscribe(m_trackSubId);
    }
}

void PlayerPanel::saveState()
{
    PanelBase::saveState();
    // Eigene State-Daten speichern
    QSettings settings;
    settings.setValue(settingsPrefix() + "volume", m_pVolumeSlider->value());
}

void PlayerPanel::restoreState()
{
    PanelBase::restoreState();
    // Eigene State-Daten laden
    QSettings settings;
    m_pVolumeSlider->setValue(
        settings.value(settingsPrefix() + "volume", 80).toInt());
}

// Self-Registration am Ende der Datei:
REGISTER_PANEL("player", "Player", true, PlayerPanel)
```

---

## 5. Panel-Lifecycle

### 5.1 Lifecycle-Diagramm

```
┌────────────────────────────────────────────────────────────────────┐
│                        Panel Lifecycle                              │
├────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. REGISTRATION (Static Initialization)                           │
│     REGISTER_PANEL() ───► PanelRegistry                            │
│                                                                     │
│  2. CREATION (Application::init)                                   │
│     PanelRegistry::create() ───► Konstruktor                       │
│                                                                     │
│  3. DOCKING (DockManager)                                          │
│     CDockWidget::setWidget(panel)                                  │
│                                                                     │
│  4. ACTIVATION (User shows panel)                                  │
│     showEvent() ───► setActive(true) ───► onActivate()             │
│                ───► EventBus subscriptions                         │
│                                                                     │
│  5. USAGE                                                           │
│     User interacts, Events werden empfangen                        │
│                                                                     │
│  6. DEACTIVATION (User hides panel)                                │
│     hideEvent() ───► setActive(false) ───► onDeactivate()          │
│                 ───► EventBus unsubscriptions                      │
│                                                                     │
│  7. STATE SAVE (Application shutdown)                              │
│     saveState() ───► QSettings                                     │
│                                                                     │
│  8. DESTRUCTION                                                     │
│     Destruktor                                                      │
│                                                                     │
└────────────────────────────────────────────────────────────────────┘
```

### 5.2 Activation-Pattern

```cpp
// WICHTIG: EventBus Subscriptions nur in onActivate/onDeactivate!

void MyPanel::onActivate()
{
    // ✅ Richtig: Subscribe wenn sichtbar
    m_subId = eventBus()->subscribe<MyEvent>([this](auto& e) {
        // Handle event
    });
}

void MyPanel::onDeactivate()
{
    // ✅ Richtig: Unsubscribe wenn versteckt
    eventBus()->unsubscribe(m_subId);
}

// ❌ Falsch: Im Konstruktor subscriben
MyPanel::MyPanel() {
    eventBus()->subscribe<MyEvent>(...);  // Läuft auch wenn Panel versteckt!
}
```

---

## 6. Existierende Panels

### 6.1 PlayerPanel

- **ID:** `player`
- **Funktion:** Play/Pause, Stop, Volume, Progress
- **Events:** TrackChanged, PlaybackState, PlaybackPosition

### 6.2 PlaylistPanel

- **ID:** `playlist`
- **Funktion:** Track-Liste, Add/Remove, Drag&Drop
- **Events:** PlaylistChanged, PlaylistIndexChanged

### 6.3 VisualSelectPanel

- **ID:** `visual-select`
- **Funktion:** Visualizer-Auswahl aus Registry
- **Events:** VisualizerChanged

### 6.4 ConfigPanel

- **ID:** `config`
- **Funktion:** App-Einstellungen, GPU-Auswahl
- **Default:** Versteckt

---

## 7. Best Practices

### 7.1 Lazy Initialization

```cpp
// ❌ Schlecht: Alles im Konstruktor
MyPanel::MyPanel() {
    setupComplexUI();        // Langsam
    loadData();              // I/O
    connectToServices();     // Vielleicht nicht nötig
}

// ✅ Gut: Lazy in onActivate
MyPanel::MyPanel() {
    setupBasicUI();  // Nur Basis-Layout
}

void MyPanel::onActivate() {
    if (!m_initialized) {
        loadData();
        connectToServices();
        m_initialized = true;
    }
}
```

### 7.2 Event-Cleanup

```cpp
class MyPanel : public PanelBase {
    std::vector<IEventBus::SubscriberId> m_subscriptions;
    
    void onActivate() override {
        m_subscriptions.push_back(
            eventBus()->subscribe<Event1>(...));
        m_subscriptions.push_back(
            eventBus()->subscribe<Event2>(...));
    }
    
    void onDeactivate() override {
        for (auto id : m_subscriptions) {
            eventBus()->unsubscribe(id);
        }
        m_subscriptions.clear();
    }
};
```

### 7.3 Responsive UI

```cpp
// ❌ Schlecht: Blocking im Main-Thread
void MyPanel::loadPlaylist() {
    auto tracks = loadFromDisk();  // Blockiert UI!
    updateList(tracks);
}

// ✅ Gut: Async mit Signals
void MyPanel::loadPlaylist() {
    QtConcurrent::run([this]() {
        auto tracks = loadFromDisk();
        QMetaObject::invokeMethod(this, [this, tracks]() {
            updateList(tracks);
        }, Qt::QueuedConnection);
    });
}
```

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-31** | **Initial: PanelBase, IPanel, Lifecycle-Hooks** |
