# Panel System

> **Version:** 2.1.0  
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
| `player` | Player | 100 | ✅ | Audio Player Controls |
| `playlist` | Playlist | 200 | ✅ | Playlist Management |
| `config` | Visualizer Config | 300 | ❌ | Visualizer-Konfiguration |
| `settings` | Settings | 350 | ❌ | Audio & Performance Settings |
| `visual_select` | Visualizers | 400 | ✅ | Visualizer Selection |

---

## Panel-Implementierungen

### PlayerPanel

Audio-Wiedergabe-Steuerung mit Loop-Button für Single-Track-Repeat.

```
┌─────────────────────────────────────────────────────────┐
│                    Track Title                           │
│                    Artist Name                           │
│  ○━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━  │
│                   00:45 / 03:30                          │
│                                                          │
│      [⏮]  [▶/⏸]  [⏹]  [⏭]  [🔁]      🔊━━━━━━━○        │
│       ↑      ↑     ↑     ↑     ↑           ↑             │
│      Prev  Play  Stop  Next  Loop      Volume            │
└─────────────────────────────────────────────────────────┘
```

**Loop-Button (🔁)**: Aktiviert `RepeatMode::One` - wiederholt nur aktuellen Track

**Events:**
- Subscribed: `TrackChangedEvent`, `PlaybackStateEvent`, `PlaybackPositionEvent`, `VolumeChangedEvent`, `PlaybackModeChangedEvent`
- Published: Keine (nutzt IAudioPlayer direkt)

---

### PlaylistPanel

Playlist-Verwaltung mit Multi-Selection und Shuffle/Loop-Buttons.

```
┌─────────────────────────────────────────────────────────┐
│  [+] [🗑️] [🔄]  [💾] [📂]              [🔀] [🔁]        │
│   ↑    ↑    ↑     ↑    ↑                ↑    ↑          │
│  Add Remove Clear Save Load         Shuffle Loop        │
├─────────────────────────────────────────────────────────┤
│  ▸ 01. Track One                                        │
│    02. Track Two                                        │
│  ▸ 03. Track Three (selected)                           │
│    04. Track Four                                       │
│  ▸ 05. Track Five (selected)                            │
│  ▸ **06. Current Track** (bold = playing)               │
│    07. Track Seven                                      │
└─────────────────────────────────────────────────────────┘
```

**Multi-Selection:**

| Aktion | Verhalten |
|--------|-----------|
| Klick | Nur dieses Item |
| Ctrl+Klick | Toggle (hinzufügen/entfernen) |
| Shift+Klick | Bereich vom Anker bis Klick |
| Ctrl+Shift+Klick | Bereich zur Auswahl hinzufügen |

**Aktueller Track:** Nur **fette Schrift**, keine Hintergrundfarbe

**Shuffle-Button (🔀)**: Zufällige Wiedergabe

**Loop-Button (🔁)**: `RepeatMode::All` - Playlist-Loop

---

### ConfigPanel (Visualizer Config)

Konfiguration für den **aktiven** Visualizer.

```
┌─────────────────────────────────────────────────────────┐
│  ┌─ Smoothing ─────────────────────────────────────┐    │
│  │  ━━━━━━━━━━━━━━━━━○━━━━━━━━━━━━━━━━━━━  50%    │    │
│  └─────────────────────────────────────────────────┘    │
│                                                          │
│  ┌─ Display Options ───────────────────────────────┐    │
│  │  ☑ Show Peak Hold                               │    │
│  └─────────────────────────────────────────────────┘    │
│                                                          │
│  ┌─ Color Scheme ──────────────────────────────────┐    │
│  │  [▼ Classic                                   ] │    │
│  │     Fire                                        │    │
│  │     Ocean                                       │    │
│  │     Neon                                        │    │
│  │     Monochrome                                  │    │
│  │     Rainbow                                     │    │
│  └─────────────────────────────────────────────────┘    │
│                                                          │
│         Settings apply to the active visualizer          │
└─────────────────────────────────────────────────────────┘
```

---

### SettingsPanel (NEU)

Globale Anwendungseinstellungen (Audio, Performance).

```
┌─────────────────────────────────────────────────────────┐
│  [Audio] [Performance]                                   │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  Device:      [▼ Default Device              ]          │
│                                                          │
│  Buffer Size: [▲▼ 1024] samples                         │
│                                                          │
│  Sample Rate: [▲▼ 44100] Hz                             │
│                                                          │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  [Audio] [Performance]                                   │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  Frame Mode:  [▼ Limited (60 FPS)            ]          │
│               [  Unlimited                    ]          │
│               [  VSync                        ]          │
│                                                          │
│  Target FPS:  [▲▼ 60] FPS                               │
│                                                          │
│  VSync:       ☐ Enable                                  │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

---

### VisualSelectPanel

Visualizer-Auswahl.

```
┌─────────────────────────────────────────────────────────┐
│  Available Visualizers                                   │
├─────────────────────────────────────────────────────────┤
│  ▸ Pulsing (selected)                                   │
│    Spectrum                                             │
│    Waveform                                             │
│    Bars                                                 │
└─────────────────────────────────────────────────────────┘
```

---

## PanelAutoReg.cpp

```cpp
#include "services/PanelRegistry.hpp"
#include "UI/panels/PlayerPanel.hpp"
#include "UI/panels/PlaylistPanel.hpp"
#include "UI/panels/ConfigPanel.hpp"
#include "UI/panels/SettingsPanel.hpp"
#include "UI/panels/VisualSelectPanel.hpp"

void initPanelDefaults(PanelRegistry& registry)
{
    // Player Panel
    registry.registerPanel(
        PanelDescriptor{"player", "Player", 100, true, "View/Panels"},
        [](ServiceContainer& svc) { return std::make_unique<PlayerPanel>(svc); });
    
    // Playlist Panel
    registry.registerPanel(
        PanelDescriptor{"playlist", "Playlist", 200, true, "View/Panels"},
        [](ServiceContainer& svc) { return std::make_unique<PlaylistPanel>(svc); });
    
    // Config Panel (Visualizer Config)
    registry.registerPanel(
        PanelDescriptor{"config", "Visualizer Config", 300, false, "View/Panels"},
        [](ServiceContainer& svc) { return std::make_unique<ConfigPanel>(svc); });
    
    // Settings Panel (Audio + Performance)
    registry.registerPanel(
        PanelDescriptor{"settings", "Settings", 350, false, "View/Panels"},
        [](ServiceContainer& svc) { return std::make_unique<SettingsPanel>(svc); });
    
    // Visual Select Panel
    registry.registerPanel(
        PanelDescriptor{"visual_select", "Visualizers", 400, true, "View/Panels"},
        [](ServiceContainer& svc) { return std::make_unique<VisualSelectPanel>(svc); });
}
```

---

## Neues Panel hinzufügen - Checkliste

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
```

### 2. In PanelAutoReg.cpp registrieren

```cpp
#include "UI/panels/MyPanel.hpp"

// In initPanelDefaults():
registry.registerPanel(
    PanelDescriptor{"mypanel", "My Panel", 500, true, "View/Panels"},
    [](ServiceContainer& svc) { return std::make_unique<MyPanel>(svc); });
```

### 3. In Source.cmake eintragen

```cmake
# include/UI/panels/Source.cmake
set(_local_headers
    # ...
    "${CMAKE_CURRENT_LIST_DIR}/MyPanel.hpp"
)

# src/UI/panels/Source.cmake
set(_local_sources
    # ...
    "${CMAKE_CURRENT_LIST_DIR}/MyPanel.cpp"
)
```

### 4. CMake reconfigure

```bash
cmake --preset windows-ninja-debug-clang
```

---

## PlaybackModeChangedEvent Synchronisation

PlayerPanel und PlaylistPanel synchronisieren sich über `PlaybackModeChangedEvent`:

```cpp
struct PlaybackModeChangedEvent : public Event {
    bool shuffle;      // Shuffle enabled
    int repeatMode;    // 0=None, 1=One, 2=All
};
```

### PlayerPanel (RepeatMode::One)

```cpp
void PlayerPanel::onLoopClicked()
{
    m_loopEnabled = !m_loopEnabled;
    player->setRepeatMode(m_loopEnabled ? RepeatMode::One : RepeatMode::None);
}

// Subscription:
eventBus->subscribe<PlaybackModeChangedEvent>(
    [this](const PlaybackModeChangedEvent& e) {
        m_loopEnabled = (e.repeatMode == 1);  // One
        updateLoopButton(m_loopEnabled);
    });
```

### PlaylistPanel (RepeatMode::All)

```cpp
void PlaylistPanel::onLoopClicked()
{
    m_loopEnabled = !m_loopEnabled;
    player->setRepeatMode(m_loopEnabled ? RepeatMode::All : RepeatMode::None);
}

// Subscription:
eventBus->subscribe<PlaybackModeChangedEvent>(
    [this](const PlaybackModeChangedEvent& e) {
        m_loopEnabled = (e.repeatMode == 2);  // All
        updateLoopButton(m_loopEnabled);
    });
```

**Verhalten:** Wenn PlayerPanel Loop aktiviert → PlaylistPanel Loop deaktiviert (und umgekehrt).

---

## Best Practices

### 1. Event-Cleanup

```cpp
void MyPanel::onDeactivate()
{
    auto* eventBus = m_services.tryResolve<IEventBus>();
    if (eventBus) {
        for (int id : m_subscriptionIds) {
            eventBus->unsubscribe(id);
        }
    }
    m_subscriptionIds.clear();
}
```

### 2. Stabile Panel-ID

- Panel-ID = objectName (für Layout-Persistence)
- **ID niemals ändern nach Release!**

### 3. Multi-Selection für Listen

```cpp
m_pListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

// Remove multiple items (back to front)
QList<int> indices;
for (auto* item : m_pListWidget->selectedItems()) {
    indices.append(m_pListWidget->row(item));
}
std::sort(indices.begin(), indices.end(), std::greater<int>());
for (int idx : indices) {
    playlist->removeTrack(idx);
}
```

---

## Siehe auch

- [Registry Architecture](../architecture/Registry_Architecture.md) - Registry Grundlagen
- [Layout Persistence](../architecture/Layout_Persistence.md) - Layout-Speicherung
- [Audio System](Audio_System.md) - Audio Events
- [Event Architecture](../architecture/Event_Architecture.md) - EventBus Details

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 2.1.0 | 2025-12-31 | SettingsPanel (NEU), ConfigPanel nur für Visualizer, PlayerPanel Loop-Button (RepeatMode::One), PlaylistPanel Multi-Selection (ExtendedSelection), Icons getauscht (🔁↔SP_BrowserReload), Aktueller Track nur bold (kein Background), PlaybackModeChangedEvent Synchronisation |
| 2.0.0 | 2025-12-31 | PlaylistPanel Shuffle/Loop Buttons, M3U Save/Load |
| 1.0.0 | 2025-12-30 | Initial: PanelRegistry, PanelManager, PlayerPanel, PlaylistPanel, ConfigPanel, VisualSelectPanel |
