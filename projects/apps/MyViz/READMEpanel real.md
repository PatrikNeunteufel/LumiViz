# Panel Real Implementation

> **Datum:** 2025-12-31
> **Version:** 2.0.0

---

## Zusammenfassung

Alle 4 Panel-Stubs wurden zu echten, funktionalen Panels mit voller Service-Integration umgebaut.

---

## Panel-Übersicht

| Panel | Funktion | Services |
|-------|----------|----------|
| **PlayerPanel** | Audio-Wiedergabe | IAudioPlayer, EventBus |
| **PlaylistPanel** | Playlist-Verwaltung | IPlaylist, IAudioPlayer, EventBus |
| **VisualSelectPanel** | Visualizer-Auswahl | VisualizerRegistry, EventBus |
| **ConfigPanel** | Einstellungen | IAudioEngine, EventBus |

---

## 1. PlayerPanel

### Features
- ▶️ Play/Pause/Stop/Prev/Next Buttons
- 🔊 Volume Slider mit Mute-Button
- 📊 Progress Slider mit Seek-Funktion
- 🎵 Track-Info (Titel, Artist, Zeit)

### Events (Subscribe)
- `TrackChangedEvent` → Aktualisiert Track-Info
- `PlaybackStateEvent` → Aktualisiert Play/Pause-Button
- `PlaybackPositionEvent` → Aktualisiert Progress-Slider
- `VolumeChangedEvent` → Aktualisiert Volume-Slider

### Architektur
```
┌─────────────────┐      Events       ┌──────────────┐
│   IAudioPlayer  │ ─────────────────►│ PlayerPanel  │
│                 │◄───────────────── │              │
└─────────────────┘   User Actions    └──────────────┘
```

---

## 2. PlaylistPanel

### Features
- 📋 Track-Liste mit Highlight für aktiven Track
- ➕ Add-Button (öffnet Datei-Dialog)
- ➖ Remove-Button (entfernt ausgewählten Track)
- 🗑️ Clear-Button (leert Playlist)
- 🔍 Such-/Filter-Funktion
- 🖱️ Doppelklick zum Abspielen

### Events (Subscribe)
- `PlaylistChangedEvent` → Aktualisiert Liste
- `PlaylistIndexChangedEvent` → Highlightet aktiven Track

### Architektur
```
┌─────────────────┐     Events       ┌───────────────┐
│    IPlaylist    │ ────────────────►│ PlaylistPanel │
│                 │                  │               │
├─────────────────┤                  └───────┬───────┘
│   IAudioPlayer  │◄─────────────────────────┘
└─────────────────┘   playIndex(n)
```

---

## 3. VisualSelectPanel

### Features
- 📜 Liste aller Visualizer aus VisualizerRegistry
- 📂 Kategorie-Gruppierung
- 📝 Beschreibung und Details
- ✅ Apply-Button zum Anwenden
- 🖱️ Doppelklick zum sofortigen Anwenden
- 🎵 Audio-Indikator für Audio-reaktive Visualizer

### Events (Publish)
- `ChangeVisualizerEvent` → Ändert aktiven Visualizer

### Architektur
```
┌────────────────────┐
│ VisualizerRegistry │
└─────────┬──────────┘
          │ descriptors()
          ▼
┌─────────────────────┐     apply      ┌──────────────────┐
│  VisualSelectPanel  │ ──────────────►│ VisualizerWidget │
└─────────────────────┘                 └──────────────────┘
```

---

## 4. ConfigPanel

### Tabs

#### Audio Tab
- 🎧 Audio-Device Auswahl (aus IAudioEngine)
- 📊 Buffer Size (256-8192 samples)
- 🔊 Sample Rate (22050-192000 Hz)

#### Visuals Tab
- 📈 Smoothing Slider (0-100%)
- 📊 Peak Hold Checkbox
- 🎨 Color Scheme (Classic, Fire, Ocean, Neon, Mono)

#### Performance Tab
- ⚡ Frame Mode (Limited, Unlimited, VSync)
- 🎯 Target FPS (15-240)
- 🔄 VSync Checkbox

### Events
- `FrameModeChangedEvent` (Subscribe/Publish)

---

## 5. MainWindow Updates

### Neuer Event-Handler
```cpp
// Change Visualizer event
pEventBus->subscribe<ChangeVisualizerEvent>([this](const ChangeVisualizerEvent& e) {
    auto visualizers = m_pDockManager->visualizers();
    if (!visualizers.empty())
    {
        visualizers[0]->setVisualizer(QString::fromStdString(e.visualizerId));
    }
});
```

---

## Geänderte Dateien

### Headers (include/UI/panels/)
| Datei | Änderungen |
|-------|------------|
| **PlayerPanel.hpp** | +Slots, +Event-Handler, +m_subscriptionIds |
| **PlaylistPanel.hpp** | +onItemDoubleClicked(item), +Event-Integration |
| **VisualSelectPanel.hpp** | +Apply-Button, +VisualizerRegistry |
| **ConfigPanel.hpp** | +onActivate/Deactivate, +Event-Integration |

### Sources (src/UI/panels/)
| Datei | Änderungen |
|-------|------------|
| **PlayerPanel.cpp** | Volle Audio-Integration, Event-Subscription |
| **PlaylistPanel.cpp** | IPlaylist-Integration, Track-Highlight |
| **VisualSelectPanel.cpp** | VisualizerRegistry-Integration, Apply |
| **ConfigPanel.cpp** | Service-Integration, Frame-Mode Events |

### MainWindow (src/UI/)
| Datei | Änderungen |
|-------|------------|
| **MainWindow.cpp** | +ChangeVisualizerEvent Handler |

---

## Service-Abhängigkeiten

| Panel | Benötigte Services |
|-------|-------------------|
| PlayerPanel | `IEventBus`, `IAudioPlayer` |
| PlaylistPanel | `IEventBus`, `IPlaylist`, `IAudioPlayer` |
| VisualSelectPanel | `IEventBus`, `VisualizerRegistry` |
| ConfigPanel | `IEventBus`, `IAudioEngine` |

---

## Zukünftige Erweiterungen

1. **PlayerPanel**: Repeat/Shuffle Buttons
2. **PlaylistPanel**: Drag & Drop Reordering mit IPlaylist Sync
3. **VisualSelectPanel**: Per-Visualizer Settings im StackedWidget
4. **ConfigPanel**: Persistente Einstellungen über QSettings
