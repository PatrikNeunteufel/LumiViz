# AudioSystem — BASS Audio Engine Integration

> **Version:** 1.0.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::Audio  
> **Dateien:** IAudioEngine.hpp, IAudioPlayer.hpp, IAudioAnalyzer.hpp, IPlaylist.hpp, BassEngine.hpp/cpp, AudioPlayer.hpp/cpp, AudioAnalyzer.hpp/cpp, Playlist.hpp/cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** BASS Library, EventBus, ServiceContainer  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [Interfaces](#3-interfaces)
4. [Events](#4-events)
5. [Integration](#5-integration)
6. [BASS Library Setup](#6-bass-library-setup)
7. [Audio-Analyse](#7-audio-analyse)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

### 1.1 Zweck

Das Audio System bietet vollständige Audio-Wiedergabe und -Analyse für die Visualizer:

- **BassEngine**: Low-Level BASS Library Wrapper
- **AudioPlayer**: High-Level Playback Control (Play/Pause/Stop)
- **AudioAnalyzer**: FFT, Spectrum, Beat Detection
- **Playlist**: Track-Verwaltung mit Persistence

### 1.2 Architektur-Diagramm

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

---

## 2. Architektur

### 2.1 Dateien

#### Headers (`include/audio/`)

| Datei | Beschreibung |
|-------|-------------|
| `AudioEvents.hpp` | Event-Definitionen |
| `IAudioEngine.hpp` | Interface für Low-Level Audio |
| `IAudioPlayer.hpp` | Interface für High-Level Playback |
| `IAudioAnalyzer.hpp` | Interface für Audio-Analyse |
| `IPlaylist.hpp` | Interface für Playlist-Verwaltung |
| `BassEngine.hpp` | BASS Library Implementation |
| `AudioPlayer.hpp` | Concrete AudioPlayer Service |
| `AudioAnalyzer.hpp` | Concrete AudioAnalyzer Service |
| `Playlist.hpp` | Concrete Playlist Service |

#### Sources (`src/audio/`)

| Datei | Beschreibung |
|-------|-------------|
| `BassEngine.cpp` | BASS Library Wrapper |
| `AudioPlayer.cpp` | Playback State Machine |
| `AudioAnalyzer.cpp` | FFT + Beat Detection |
| `Playlist.cpp` | Track-Verwaltung + Persistence |

---

## 3. Interfaces

### 3.1 IAudioEngine

```cpp
class IAudioEngine
{
public:
    virtual ~IAudioEngine() = default;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    virtual bool loadFile(const std::string& path) = 0;
    virtual void unload() = 0;
    
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    
    virtual void setPosition(int positionMs) = 0;
    virtual int position() const = 0;
    virtual int duration() const = 0;
    
    virtual void setVolume(float volume) = 0;
    virtual float volume() const = 0;
    
    virtual const float* getFFTData(int& size) = 0;
    virtual const float* getWaveData(int& size) = 0;
};
```

### 3.2 IAudioPlayer

```cpp
class IAudioPlayer
{
public:
    virtual ~IAudioPlayer() = default;
    
    virtual bool loadTrack(const TrackInfo& track) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void togglePlayPause() = 0;
    
    virtual PlaybackState state() const = 0;
    virtual float progress() const = 0;
    
    virtual void update() = 0;  // Muss regelmäßig aufgerufen werden
};
```

### 3.3 IAudioAnalyzer

```cpp
class IAudioAnalyzer
{
public:
    virtual ~IAudioAnalyzer() = default;
    
    virtual void update() = 0;  // FFT berechnen, Events publizieren
    
    virtual const std::vector<float>& spectrum() const = 0;
    virtual const std::vector<float>& waveform() const = 0;
    virtual const std::vector<float>& bands() const = 0;
    
    virtual float levelLeft() const = 0;
    virtual float levelRight() const = 0;
    virtual bool beatDetected() const = 0;
    virtual float bpm() const = 0;
};
```

---

## 4. Events

### 4.1 Playback Events

```cpp
// Track wurde geladen/gewechselt
struct TrackChangedEvent {
    TrackInfo track;
    int playlistIndex;
};

// Playback State (Playing, Paused, Stopped)
struct PlaybackStateEvent {
    PlaybackState state;
    PlaybackState previousState;
};

// Position Update (für Progress Bar)
struct PlaybackPositionEvent {
    int positionMs;
    int durationMs;
    float progress;  // 0.0 - 1.0
};
```

### 4.2 Audio Analysis Events

```cpp
// Audio-Daten für Visualizer (60 Hz)
struct AudioDataEvent {
    std::vector<float> spectrum;   // FFT Spectrum (512 Bins)
    std::vector<float> waveform;   // Raw Waveform
    float levelLeft;               // Level 0.0-1.0
    float levelRight;
    bool beatDetected;             // Beat this frame?
};

// Beat erkannt
struct BeatEvent {
    float intensity;   // 0.0 - 1.0
    float bpm;         // Geschätzte BPM
};
```

---

## 5. Integration

### 5.1 ServiceContainer Registration

```cpp
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

### 5.2 Panel Connection

```cpp
void PlayerPanel::onActivate()
{
    auto* eventBus = m_services.tryResolve<IEventBus>();
    if (!eventBus) return;
    
    m_subscriptionIds.push_back(
        eventBus->subscribe<PlaybackStateEvent>(
            [this](const PlaybackStateEvent& e) {
                updatePlayButton(e.state == PlaybackState::Playing);
            }));
    
    m_subscriptionIds.push_back(
        eventBus->subscribe<PlaybackPositionEvent>(
            [this](const PlaybackPositionEvent& e) {
                m_pProgressSlider->setValue(static_cast<int>(e.progress * 100));
            }));
}
```

### 5.3 Visualizer Connection

```cpp
void VisualizerWidget::connectToAudio()
{
    auto* eventBus = m_services.tryResolve<IEventBus>();
    if (!eventBus) return;
    
    m_audioSubId = eventBus->subscribe<AudioDataEvent>(
        [this](const AudioDataEvent& e) {
            m_spectrum = e.spectrum;
            m_waveform = e.waveform;
            m_beatPulse = e.beatDetected ? 1.0f : m_beatPulse * 0.9f;
            update();  // Trigger repaint
        });
}
```

---

## 6. BASS Library Setup

### 6.1 CMake Integration

```cmake
# In externals/bass/CMakeLists.txt
FetchContent_Declare(
    bass
    URL https://www.un4seen.com/files/bass24-win.zip
)

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

### 6.2 Plugin Loading

```cpp
// In BassEngine::initialize()
loadPlugins("plugins/bass");  // Lädt bassflac.dll, basswv.dll, etc.
```

---

## 7. Audio-Analyse

### 7.1 Frequency Bands

| Band | Frequency Range | Beschreibung |
|------|-----------------|--------------|
| Sub | 20-60 Hz | Sub-bass (Kick Drums) |
| Bass | 60-250 Hz | Bass (Bassline) |
| LowMid | 250-500 Hz | Untere Mitten |
| Mid | 500-2000 Hz | Mitten (Vocals) |
| HighMid | 2000-4000 Hz | Obere Mitten |
| High | 4000-20000 Hz | Höhen (Cymbals) |

### 7.2 Beat Detection

Einfacher Energy-basierter Algorithmus:

1. Berechne Energie im Bass-Bereich (20-250 Hz)
2. Vergleiche mit Rolling Average der letzten ~1 Sekunde
3. Beat erkannt wenn: `aktuelle_energie > threshold * durchschnitt`
4. BPM wird aus Beat-Intervallen geschätzt

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-31** | **Initial: BASS Integration, Events, Interfaces** |
