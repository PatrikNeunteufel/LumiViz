# Audio System für MyViz

## Übersicht

Das Audio System bietet vollständige Audio-Wiedergabe und -Analyse für die Visualizer:

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           Audio System                                   │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌───────────────┐    ┌───────────────┐    ┌───────────────────────┐   │
│  │  IAudioEngine │    │ IAudioPlayer  │    │   IAudioAnalyzer      │   │
│  │  (Interface)  │    │  (Interface)  │    │     (Interface)       │   │
│  └───────┬───────┘    └───────┬───────┘    └───────────┬───────────┘   │
│          │                    │                        │                │
│  ┌───────┴───────┐    ┌───────┴───────┐    ┌───────────┴───────────┐   │
│  │  BassEngine   │────│  AudioPlayer  │────│    AudioAnalyzer      │   │
│  │ (BASS Library)│    │ (Play/Pause)  │    │   (FFT/Spectrum)      │   │
│  └───────────────┘    └───────────────┘    └───────────────────────┘   │
│                                                                          │
│  ┌───────────────────────────────────────────────────────────────────┐  │
│  │                         Playlist                                    │  │
│  │                    (Track-Verwaltung)                               │  │
│  └───────────────────────────────────────────────────────────────────┘  │
│                                                                          │
└──────────────────────────────┬──────────────────────────────────────────┘
                               │ Events via EventBus
                               ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  PlayerPanel          PlaylistPanel         VisualizerWidget            │
│  (Controls)           (Tracks)              (Spectrum/Waveform)         │
└─────────────────────────────────────────────────────────────────────────┘
```

## Dateien

### Headers (`include/audio/`)

| Datei | Beschreibung |
|-------|-------------|
| `AudioEvents.hpp` | Event-Definitionen (TrackChanged, PlaybackState, AudioData, etc.) |
| `IAudioEngine.hpp` | Interface für Low-Level Audio (BASS Abstraktion) |
| `IAudioPlayer.hpp` | Interface für High-Level Playback Control |
| `IAudioAnalyzer.hpp` | Interface für Audio-Analyse (FFT, Spectrum, Beat) |
| `IPlaylist.hpp` | Interface für Playlist-Verwaltung |
| `BassEngine.hpp` | BASS Library Implementation |
| `AudioPlayer.hpp` | Concrete AudioPlayer Service |
| `AudioAnalyzer.hpp` | Concrete AudioAnalyzer Service |
| `Playlist.hpp` | Concrete Playlist Service |

### Sources (`src/audio/`)

| Datei | Zeilen | Beschreibung |
|-------|--------|-------------|
| `BassEngine.cpp` | ~550 | BASS Library Wrapper |
| `AudioPlayer.cpp` | ~400 | Playback State Machine |
| `AudioAnalyzer.cpp` | ~450 | FFT + Beat Detection |
| `Playlist.cpp` | ~450 | Track-Verwaltung + Persistence |

## Events

### Playback Events

```cpp
// Track wurde geladen/gewechselt
struct TrackChangedEvent : public Event {
    TrackInfo track;
    int playlistIndex;
};

// Playback State (Playing, Paused, Stopped)
struct PlaybackStateEvent : public Event {
    PlaybackState state;
    PlaybackState previousState;
};

// Position Update (für Progress Bar)
struct PlaybackPositionEvent : public Event {
    int positionMs;
    int durationMs;
    float progress;  // 0.0 - 1.0
};
```

### Audio Analysis Events

```cpp
// Audio-Daten für Visualizer (60 Hz)
struct AudioDataEvent : public Event {
    std::vector<float> spectrum;   // FFT Spectrum (512 Bins)
    std::vector<float> waveform;   // Raw Waveform
    float levelLeft;               // Level 0.0-1.0
    float levelRight;
    bool beatDetected;             // Beat this frame?
};

// Beat erkannt
struct BeatEvent : public Event {
    float intensity;   // 0.0 - 1.0
    float bpm;         // Geschätzte BPM
};
```

## Integration

### 1. ServiceContainer Registration

```cpp
// In Application.cpp oder AudioInit.cpp
void registerAudioServices(ServiceContainer& services)
{
    // Engine (BASS)
    services.registerSingleton<IAudioEngine, BassEngine>();
    
    // Player
    services.registerSingleton<IAudioPlayer>([&services]() {
        auto& engine = services.resolve<IAudioEngine>();
        auto& eventBus = services.resolve<IEventBus>();
        return std::make_unique<AudioPlayer>(engine, eventBus);
    });
    
    // Playlist
    services.registerSingleton<IPlaylist>([&services]() {
        auto& eventBus = services.resolve<IEventBus>();
        return std::make_unique<Playlist>(eventBus);
    });
    
    // Analyzer
    services.registerSingleton<IAudioAnalyzer>([&services]() {
        auto& engine = services.resolve<IAudioEngine>();
        auto& player = services.resolve<IAudioPlayer>();
        auto& eventBus = services.resolve<IEventBus>();
        return std::make_unique<AudioAnalyzer>(engine, player, eventBus);
    });
}
```

### 2. Panel Connection (PlayerPanel)

```cpp
void PlayerPanel::onActivate()
{
    auto& eventBus = m_services.resolve<IEventBus>();
    
    // Subscribe to playback events
    m_stateSubId = eventBus.subscribe<PlaybackStateEvent>(
        [this](const PlaybackStateEvent& e) {
            updatePlayButton(e.state == PlaybackState::Playing);
        });
    
    m_positionSubId = eventBus.subscribe<PlaybackPositionEvent>(
        [this](const PlaybackPositionEvent& e) {
            m_pProgressSlider->setValue(static_cast<int>(e.progress * 100));
            m_pTimeLabel->setText(formatTime(e.positionMs, e.durationMs));
        });
}

void PlayerPanel::onPlayClicked()
{
    auto& player = m_services.resolve<IAudioPlayer>();
    player.togglePlayPause();
}
```

### 3. Visualizer Connection

```cpp
void VisualizerWidget::connectToAudio()
{
    auto& eventBus = m_services.resolve<IEventBus>();
    
    m_audioSubId = eventBus.subscribe<AudioDataEvent>(
        [this](const AudioDataEvent& e) {
            // Update visualizer with audio data
            m_spectrum = e.spectrum;
            m_waveform = e.waveform;
            m_beatPulse = e.beatDetected ? 1.0f : m_beatPulse * 0.9f;
            update();  // Trigger repaint
        });
}
```

### 4. Main Loop Update

```cpp
// In Application oder MainWindow render loop
void Application::update()
{
    // Update audio services
    auto& player = m_services.resolve<IAudioPlayer>();
    auto& analyzer = m_services.resolve<IAudioAnalyzer>();
    
    player.update();     // Check track end, publish position
    analyzer.update();   // FFT, beat detection, publish AudioDataEvent
    
    // Dispatch queued events
    auto& eventBus = m_services.resolve<IEventBus>();
    eventBus.dispatchQueued();
}
```

## BASS Library Setup

### CMake Integration

```cmake
# In externals/bass/CMakeLists.txt oder hooks
FetchContent_Declare(
    bass
    URL https://www.un4seen.com/files/bass24-win.zip
    # Oder lokaler Pfad
)

# Link gegen BASS
target_link_libraries(MyViz.Core 
    PRIVATE 
        bass
        basswasapi  # Für Loopback
)

# Copy DLL to output
add_custom_command(TARGET MyViz POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${BASS_DIR}/bass.dll"
        "$<TARGET_FILE_DIR:MyViz>"
)
```

### Plugin Loading

```cpp
// In BassEngine::initialize()
loadPlugins("plugins/bass");  // Lädt bassflac.dll, basswv.dll, etc.
```

## Frequency Bands

```
Band    | Frequency Range | Description
--------|-----------------|------------------------
Sub     | 20-60 Hz        | Sub-bass (Kick Drums)
Bass    | 60-250 Hz       | Bass (Bassline)
LowMid  | 250-500 Hz      | Untere Mitten
Mid     | 500-2000 Hz     | Mitten (Vocals)
HighMid | 2000-4000 Hz    | Obere Mitten
High    | 4000-20000 Hz   | Höhen (Cymbals)
```

## Beat Detection

Einfacher Energy-basierter Algorithmus:

1. Berechne Energie im Bass-Bereich (20-250 Hz)
2. Vergleiche mit Rolling Average der letzten ~1 Sekunde
3. Beat erkannt wenn: `aktuelle_energie > threshold * durchschnitt`
4. BPM wird aus Beat-Intervallen geschätzt

## Nächste Schritte

1. **BASS in CMake einbinden** (FetchContent oder lokale Kopie)
2. **AudioInit.cpp** erstellen für Service-Registrierung
3. **PlayerPanel/PlaylistPanel** mit Events verbinden
4. **VisualizerWidget** für AudioDataEvent erweitern
5. **Loopback-Capture** für System-Audio (BASSWASAPI)
