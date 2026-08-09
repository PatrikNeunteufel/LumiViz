# Audio System

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Status:** Aktuell

---

## Übersicht

Das Audio System bietet vollständige Audio-Wiedergabe und -Analyse für die Visualizer.

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
└──────────────────────────────────┬──────────────────────────────────────┘
                                   │ Events via EventBus
                                   ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  PlayerPanel          PlaylistPanel         VisualizerWidget            │
│  (Controls)           (Tracks)              (Spectrum/Waveform)         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Dateien

### Headers (`include/audio/`)

| Datei | Beschreibung |
|-------|-------------|
| `AudioEvents.hpp` | Event-Definitionen |
| `IAudioEngine.hpp` | Interface für Low-Level Audio (BASS Abstraktion) |
| `IAudioPlayer.hpp` | Interface für High-Level Playback Control |
| `IAudioAnalyzer.hpp` | Interface für Audio-Analyse (FFT, Spectrum, Beat) |
| `IPlaylist.hpp` | Interface für Playlist-Verwaltung |
| `BassEngine.hpp` | BASS Library Implementation |
| `AudioPlayer.hpp` | Concrete AudioPlayer Service |
| `AudioAnalyzer.hpp` | Concrete AudioAnalyzer Service |
| `Playlist.hpp` | Concrete Playlist Service |

### Sources (`src/audio/`)

| Datei | Beschreibung |
|-------|-------------|
| `BassEngine.cpp` | BASS Library Wrapper + Metadata Parsing |
| `AudioPlayer.cpp` | Playback State Machine + Shuffle/Repeat |
| `AudioAnalyzer.cpp` | FFT + Beat Detection |
| `Playlist.cpp` | Track-Verwaltung + M3U Persistence |

---

## Unterstützte Formate

### Basis-Formate (BASS Core)

| Format | Erweiterungen | Hinweis |
|--------|---------------|---------|
| MP3 | `.mp3` | ID3v1/ID3v2 Tags |
| WAV | `.wav` | PCM Audio |
| AIFF | `.aiff`, `.aif` | Apple Format |
| OGG | `.ogg` | Vorbis Comments |

### Plugin-Formate (Auto-Loaded)

| Plugin | Formate | Tags |
|--------|---------|------|
| `bassflac.dll` | `.flac` | Vorbis Comments |
| `basswv.dll` | `.wv` | APE Tags |
| `bassopus.dll` | `.opus` | Vorbis Comments |
| `bass_aac.dll` | `.m4a`, `.aac` | MP4 Tags |

---

## Metadata Parsing

### Tag-Format Priorität

BassEngine versucht Tags in dieser Reihenfolge:

1. **ID3v2** (MP3) - Unicode Support, unbegrenzte Länge
2. **ID3v1** (MP3 Fallback) - Latin-1, max 30 Zeichen
3. **Vorbis Comments** (FLAC, OGG) - UTF-8
4. **APE Tags** (WavPack, einige MP3) - UTF-8
5. **MP4 Tags** (M4A, AAC) - UTF-8

### ID3v2 Encoding Support

| Byte | Encoding | Behandlung |
|------|----------|------------|
| `0x00` | Latin-1 (ISO-8859-1) | `QString::fromLatin1()` |
| `0x01` | UTF-16 mit BOM | BOM-Erkennung, Byte-Swap wenn nötig |
| `0x02` | UTF-16BE | Byte-Swap für Little-Endian |
| `0x03` | UTF-8 | `QString::fromUtf8()` |

### Erkannte Frame-IDs

| ID3v2.3/2.4 | ID3v2.2 | Inhalt |
|-------------|---------|--------|
| `TIT2` | `TT2` | Titel |
| `TPE1` | `TP1` | Künstler |
| `TALB` | `TAL` | Album |

---

## Plugin Auto-Loading

Plugins werden automatisch beim Start aus dem EXE-Verzeichnis geladen:

```cpp
// In BassEngine::initialize()
QString exePath = QCoreApplication::applicationDirPath();
int pluginCount = loadPlugins(exePath);
```

### Plugin-Erkennung

```cpp
// Windows
QStringList filters = {"bass*.dll"};  // bassflac.dll, basswv.dll, etc.

// Linux  
QStringList filters = {"libbass*.so"};
```

### Log-Output

```
Searching for BASS plugins in: C:/path/to/bin/Debug
Found 3 potential plugin files
  Trying to load: bassflac.dll
Loaded BASS plugin: bassflac.dll (formats: 1)
  Trying to load: basswv.dll
Loaded BASS plugin: basswv.dll (formats: 1)
Loaded 2 BASS plugins
```

---

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

// Shuffle/Repeat Mode Changed
struct PlaybackModeChangedEvent : public Event {
    bool shuffle;      // Shuffle enabled
    int repeatMode;    // 0=None, 1=One, 2=All
};
```

### RepeatMode

| Wert | Konstante | Beschreibung | UI |
|------|-----------|--------------|-----|
| 0 | `RepeatMode::None` | Kein Repeat | - |
| 1 | `RepeatMode::One` | Aktueller Track wiederholen | PlayerPanel 🔁 |
| 2 | `RepeatMode::All` | Playlist wiederholen | PlaylistPanel 🔁 |

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

---

## Shuffle & Repeat Logik

### Shuffle Mode

```cpp
void AudioPlayer::playNext()
{
    if (m_impl->shuffle)
    {
        // Random Track (nicht aktueller)
        std::uniform_int_distribution<int> dist(0, trackCount - 1);
        int next = dist(m_impl->rng);
        while (next == currentIdx && trackCount > 1) {
            next = dist(m_impl->rng);
        }
        playTrack(next);
    }
    else
    {
        // Sequential
        int next = currentIdx + 1;
        if (next >= trackCount)
        {
            if (m_impl->repeatMode == RepeatMode::All) {
                next = 0;  // Wrap to start
            } else {
                stop();
                return;
            }
        }
        playTrack(next);
    }
}
```

### RepeatMode::One

```cpp
void AudioPlayer::onTrackEnd()
{
    if (m_impl->repeatMode == RepeatMode::One)
    {
        // Restart same track
        seek(0);
        play();
    }
    else
    {
        playNext();
    }
}
```

---

## M3U Playlist Format

### Speichern

```cpp
void PlaylistPanel::onSaveClicked()
{
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save Playlist"), QString(), 
        tr("M3U Playlist (*.m3u)"));
    
    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "#EXTM3U\n";
    
    for (int i = 0; i < playlist->trackCount(); ++i)
    {
        out << playlist->trackAt(i).filePath << "\n";
    }
}
```

### Laden

```cpp
void PlaylistPanel::onLoadClicked()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("Load Playlist"), QString(),
        tr("Playlists (*.m3u *.m3u8 *.pls)"));
    
    QFile file(path);
    QTextStream in(&file);
    QDir playlistDir = QFileInfo(path).absoluteDir();
    
    playlist->clear();
    
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        
        // Relative Pfade auflösen
        QString filePath = line;
        if (!QFileInfo(line).isAbsolute()) {
            filePath = playlistDir.absoluteFilePath(line);
        }
        
        if (QFileInfo::exists(filePath)) {
            playlist->addTrack(filePath);
        }
    }
}
```

---

## Integration

### ServiceContainer Registration (MainWindow)

```cpp
void MainWindow::setupAudioServices()
{
    auto& eventBus = m_pServices->resolve<IEventBus>();
    
    // 1. BassEngine registrieren und initialisieren
    m_pServices->registerSingleton<IAudioEngine>(
        [](ServiceContainer&) {
            auto engine = std::make_unique<BassEngine>();
            engine->initialize();  // Lädt auch Plugins
            return engine;
        });
    
    // 2. Playlist (unabhängig)
    m_pServices->registerSingleton<IPlaylist>(
        [&eventBus](ServiceContainer&) {
            return std::make_unique<Playlist>(eventBus);
        });
    
    // 3. AudioPlayer (braucht Engine + Playlist)
    auto* pEngine = m_pServices->tryResolve<IAudioEngine>();
    auto* pPlaylist = m_pServices->tryResolve<IPlaylist>();
    
    m_pServices->registerSingleton<IAudioPlayer>(
        [pEngine, pPlaylist, &eventBus](ServiceContainer&) {
            return std::make_unique<AudioPlayer>(*pEngine, *pPlaylist, eventBus);
        });
}
```

### Position Update Timer

```cpp
// In MainWindow::setupAudioServices()
m_pAudioUpdateTimer = new QTimer(this);
m_pAudioUpdateTimer->setInterval(33);  // ~30 Hz
connect(m_pAudioUpdateTimer, &QTimer::timeout, 
        this, &MainWindow::onAudioUpdate);
m_pAudioUpdateTimer->start();

void MainWindow::onAudioUpdate()
{
    auto* player = m_pServices->tryResolve<IAudioPlayer>();
    if (player) {
        player->update();  // Publishes PlaybackPositionEvent
    }
}
```

---

## Frequency Bands

| Band | Frequency Range | Description |
|------|-----------------|-------------|
| Sub | 20-60 Hz | Sub-bass (Kick Drums) |
| Bass | 60-250 Hz | Bass (Bassline) |
| LowMid | 250-500 Hz | Untere Mitten |
| Mid | 500-2000 Hz | Mitten (Vocals) |
| HighMid | 2000-4000 Hz | Obere Mitten |
| High | 4000-20000 Hz | Höhen (Cymbals) |

---

## BASS Library Setup

### CMake Integration

```cmake
# In externals/bass/CMakeLists.txt oder hooks
FetchContent_Declare(
    bass
    URL https://www.un4seen.com/files/bass24-win.zip
)

# Link gegen BASS
target_link_libraries(LumiViz.Core 
    PRIVATE 
        bass
        bassflac   # Optional
)

# Copy DLLs to output
add_custom_command(TARGET LumiViz POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${BASS_DIR}/bass.dll"
        "$<TARGET_FILE_DIR:LumiViz>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${BASSFLAC_DIR}/bassflac.dll"
        "$<TARGET_FILE_DIR:LumiViz>"
)
```

---

## Siehe auch

- [Event Architecture](../architecture/Event_Architecture.md) - EventBus Details
- [Panel System](Panel_System.md) - PlayerPanel, PlaylistPanel
- [Application Integration](../integration/Application_Integration.md) - Service Registration

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 2.0.0 | 2025-12-31 | FLAC/Vorbis Support, Plugin Auto-Loading, ID3v2 UTF-16 Encoding, RepeatMode (None/One/All), M3U Playlist Save/Load, PlaybackModeChangedEvent |
| 1.0.0 | 2025-12-30 | Initial: BassEngine, AudioPlayer, Playlist, Basic Events |
