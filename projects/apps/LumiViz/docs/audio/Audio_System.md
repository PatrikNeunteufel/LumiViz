# Audio-System — Wiedergabe, Analyse und Signalkette zur Visualisierung

> **Version:** 1.3.0
> **Datum:** 2026-10-11
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## 1. Übersicht

Das Audio-System liefert Audio-Wiedergabe und Echtzeit-Analysedaten für die Visualizer.
Es besteht aus vier Diensten (Interface + Implementierung) über der proprietären
BASS-Library:

| Dienst | Interface | Implementierung | Aufgabe |
|---|---|---|---|
| Engine | [`IAudioEngine.hpp`](../../include/audio/IAudioEngine.hpp) | [`BassEngine.hpp`](../../include/audio/BassEngine.hpp) | BASS-Abstraktion: Devices, Streams, FFT/Waveform, Metadaten, Plugins |
| Player | [`IAudioPlayer.hpp`](../../include/audio/IAudioPlayer.hpp) | [`AudioPlayer.hpp`](../../include/audio/AudioPlayer.hpp) | Play/Pause/Stop/Seek, Volume/Mute, Playlist-Navigation, Shuffle/Repeat |
| ~~Analyzer~~ | — | — | **entfernt** (Phase 4 Schritt 0) — war nie verdrahtet, siehe 7.3 |
| Playlist | [`IPlaylist.hpp`](../../include/audio/IPlaylist.hpp) | [`Playlist.hpp`](../../include/audio/Playlist.hpp) | Track-Verwaltung, Sortierung, Persistenz (M3U/PLS/JSON) |

```
┌──────────────────────────────────────────────────────────────────┐
│                         Audio-System                             │
│                                                                  │
│  IAudioEngine ◄── IAudioPlayer ──► IPlaylist                     │
│  (BassEngine)     (AudioPlayer)    (Playlist)                    │
│       │                │               │                         │
│       │ FFT/Waveform   │ Events        │ Events                  │
└───────┼────────────────┼───────────────┼─────────────────────────┘
        │                ▼               ▼
        │           EventBus ──► PlayerPanel / PlaylistPanel
        │
        ▼  (Pull in MainWindow::onAudioUpdate, ~30 Hz)
  VisualizerWidget ──► Visualizer ──► AudioSourceModule ──► Rendering
                                      (inkl. SmoothingModule)
```

Kernprinzip: **UI-Panels** hängen über Events am EventBus; die **Visualizer** bekommen
Roh-FFT/Waveform per direktem Push (Pull aus der Engine im Update-Timer) und
bereiten die Daten selbst mit ihrem eingebetteten `AudioSourceModule` auf (Abschnitt 7).

Die Engine ist stream-basiert: `createStream(filePath)` liefert ein opakes
`AudioStreamHandle`, alle Playback-/Analyse-Aufrufe (`play`, `getPositionMs`,
`getFFTData`, …) nehmen dieses Handle. Der `AudioPlayer` kapselt das und bietet die
handle-freie High-Level-API (`load`, `play`, `next`, `seekFraction`, …).

Dateien: Header in [`include/audio/`](../../include/audio/), Implementierungen in
`src/audio/` (`BassEngine.cpp`, `AudioPlayer.cpp`, `Playlist.cpp`).

---

## 2. Service-Registrierung

Registrierung erfolgt in `MainWindow::setupAudioServices()`
(`src/UI/MainWindow.cpp`) — Reihenfolge und Muster sind bewusst gewählt:
Die Factories rufen **kein** `resolve()` im ServiceContainer auf, sondern bekommen
ihre Abhängigkeiten als gecapturte Referenzen/Pointer; nach jeder Registrierung wird
die Instanz per `tryResolve()` sofort erzeugt („Force-Init"):

1. `IAudioEngine` → `BassEngine`, `initialize()` in der Factory (lädt auch Plugins)
2. `IPlaylist` → `Playlist(eventBus)`
3. `IAudioPlayer` → `AudioPlayer(engine, eventBus)` + `setPlaylist(playlist)`

Ein Analyzer-Service existiert nicht mehr (entfernt, siehe 7.3).

**Update-Takt:** Ein `QTimer` in `MainWindow` (Intervall 33 ms, ~30 Hz) ruft
`onAudioUpdate()` auf. Das ruft `IAudioPlayer::update()` (publiziert
`PlaybackPositionEvent`, behandelt Track-Ende) und zieht anschließend FFT-/
Waveform-Daten für die Visualizer (Abschnitt 7.1).

---

## 3. Playback-Events

Alle Events sind in [`AudioEvents.hpp`](../../include/audio/AudioEvents.hpp) definiert
(Basisklasse `Event`, Publikation über den EventBus):

| Event | Publisher | Inhalt / Zweck |
|---|---|---|
| `TrackChangedEvent` | AudioPlayer | `TrackInfo` + Playlist-Index — neuer Track geladen |
| `PlaybackStateEvent` | AudioPlayer | `state`/`previousState` (`Stopped/Playing/Paused/Loading/Error`) |
| `PlaybackPositionEvent` | AudioPlayer (`update()`) | Position/Dauer in ms + `progress` 0–1 (Progress-Bar) |
| `VolumeChangedEvent` | AudioPlayer | Volume 0–1 + Mute-Flag |
| `PlaybackModeChangedEvent` | AudioPlayer | `shuffle` (bool) + `repeatMode` (int: 0=None, 1=One, 2=All) |
| `PlaylistChangedEvent` | Playlist | Action (`Added/Removed/Cleared/Reordered/Loaded`), Track-Anzahl, Index |
| `PlaylistIndexChangedEvent` | Playlist | aktueller/vorheriger Index |
| `AudioEngineErrorEvent` | Engine/Player | Fehlertyp (InitFailed, FileNotFound, FormatNotSupported, …) |
| `AudioDeviceChangedEvent` | Engine | Gerätewechsel |
| `BeatEvent` | — | definiert, derzeit ohne Publisher (`AudioDataEvent` wurde mit dem Analyzer entfernt, 7.3) |

`TrackInfo` (ebenfalls `AudioEvents.hpp`) trägt die Metadaten: `filePath`, `title`,
`artist`, `album`, `durationMs`, `sampleRate`, `channels`, `bitrate`.

---

## 4. Metadaten & Formate

### 4.1 Unterstützte Formate

Basis-Formate über den BASS-Kern: **MP3, WAV, AIFF, OGG**. Weitere Formate über
Plugins, die `BassEngine::initialize()` automatisch aus dem EXE-Verzeichnis lädt
(Filter `bass*.dll` bzw. `libbass*.so`):

| Plugin | Formate |
|---|---|
| `bassflac` | `.flac` |
| `basswv` | `.wv` (WavPack) |
| `bassopus` | `.opus` |
| `bass_aac` | `.m4a`, `.aac` |

### 4.2 Tag-Parsing (BassEngine)

`BassEngine::getMetadata()` probiert Tag-Quellen in dieser Reihenfolge:

1. **ID3v2** (MP3) — eigener Frame-Parser (`TIT2/TPE1/TALB`, ID3v2.2-Kurzformen
   `TT2/TP1/TAL`), alle Encodings: Latin-1, UTF-16 (BOM/BE), UTF-8
2. **Vorbis Comments** (OGG/FLAC) — UTF-8
3. **APE-Tags** (WavPack, teils MP3/FLAC) — UTF-8
4. **MP4-Tags** (M4A/AAC)

Fallback bei fehlenden Tags: Titel aus dem Dateinamen.

### 4.3 BASS-Beschaffung

BASS ist proprietär (un4seen-Lizenz); die Binaries sind **bewusst untracked**.
Beschaffung und Ablage: [`externals/bass/SETUP.md`](../../../../../externals/bass/SETUP.md).
Das DLL-Deployment zum Build-Output läuft deklarativ über den `"externals"`-Eintrag
des Targets in `Solution.json` (kein manuelles CMake-Copy).

---

## 5. Playlist: Persistenz, Navigation

`Playlist` verwaltet die Tracks und publiziert Änderungs-Events; `AudioPlayer` nutzt
sie für `next()/previous()/playIndex()`.

- **Persistenz** direkt im Service (`Playlist::save/load`), Format nach
  Dateiendung: **M3U/M3U8** (Default, `#EXTM3U`), **PLS**, **JSON**.
  Nach dem Laden: `PlaylistChangedEvent(Loaded)`; `refreshMetadata()` liest
  Metadaten für geladene Pfade nach (optionaler Progress-Callback).
- **Navigation:** `next(wrap)/previous(wrap)`, `hasNext()/hasPrevious()`,
  `currentIndex`; außerdem Umsortieren (`moveTrack`, `swapTracks`, `sort` nach
  title/artist/album/duration/path) und `shuffle()` (mischt die Listen-Reihenfolge
  einmalig — nicht zu verwechseln mit dem Shuffle-*Modus* des Players).
- **Session-Playlist (automatisch):** Beim Beenden speichert MainWindow die
  aktuelle Playlist (falls nicht leer) nach `AppDataLocation/session.m3u8` und
  merkt sich den aktuellen Index in QSettings (`playlist/sessionIndex`); beim
  nächsten Start wird beides wiederhergestellt (ohne Wiedergabestart). Eine
  leere Playlist beim Beenden löscht die Session-Datei. Gleiches
  `aboutToQuit`-Muster wie die Layout-Persistenz
  ([Layout_Persistence.md](../presets/Layout_Persistence.md)).

### 5.1 Shuffle & Repeat (AudioPlayer)

`RepeatMode` ist im Interface verschachtelt (`IAudioPlayer::RepeatMode`):
`None`, `One` (aktueller Track), `All` (Playlist wiederholen).

- `next()` im **Shuffle-Modus**: zufälliger Index ≠ aktueller Index
  (`std::mt19937` + `uniform_int_distribution`); sonst sequenziell mit
  Wrap-around genau dann, wenn `RepeatMode::All`.
- **Track-Ende** (`handleTrackEnd()`, aus `update()`): bei `RepeatMode::One`
  Seek auf 0 und weiterspielen, sonst `next()` (inkl. Shuffle); ohne nächsten
  Track → `stop()`.
- `previous()`: läuft der Track länger als **3 Sekunden**, wird nur an den
  Track-Anfang gespult; sonst vorheriger Track.
- Moduswechsel publizieren `PlaybackModeChangedEvent`.

---

## 6. Engine-Details (BassEngine)

- `initialize(deviceId = -1, sampleRate = 44100)`; Geräte-Enumeration über
  `getDevices()` (`AudioDeviceInfo`), Gerätewechsel per `setDevice()`.
- Streams: `createStream(filePath)`, `createLoopbackStream()` (System-Audio-Capture),
  `freeStream()`; Steuerung/Abfragen pro Handle (`play/pause/stop`,
  `getPositionMs/setPositionMs`, `getDurationMs`, `getVolume/setVolume`).
- Analyse-Rohdaten: `getFFTData(stream, data, size)` mit size ∈ 256…8192 Bins,
  `getWaveformData()`, `getChannelLevels()`.
- Fehlerdiagnose: `getLastError()/getLastErrorCode()`, Version über `getVersion()`.

---

## 6a. Mixer, Quellen — und wer den Ton hat

Der Player spielt **einen** Stream. Composer und Loop spielen mehrere Quellen
zugleich; dafür hat die Engine einen **Mixer** (BASSmix, `bassmix.dll`). Die
Aufrufe stehen in `IAudioEngine` mit leeren Vorgaben — eine Engine ohne Mixer
liefert `INVALID_STREAM`, und wer ihn braucht, lässt dann alles, wie es war.

| Aufruf | Zweck |
|---|---|
| `createMixer()` | Ein Mixer als Stream: er wird gespielt, angehalten und befragt (FFT, Wellenform, Pegel) wie jeder andere |
| `mixerAddSource(mixer, datei)` | Eine Datei als Quelle; `mixerRemoveSource` nimmt sie wieder heraus |
| `getSourcePositionMs` / `setSourcePositionMs` | Die Stelle einer Quelle — die, die gerade zu hören ist |
| `setSourcePaused(quelle, an)` | Eine Quelle hält, der Mixer läuft weiter |
| `mixerAddGenerator(mixer, generator)` | Eine **selbst rechnende** Quelle: `IAudioGenerator::render(out, frames)` wird im Ton-Thread gerufen. Sie kommt angehalten in den Mixer; ihre Stelle führt der Generator selbst (`setSourcePositionMs` ändert an ihr nichts). Der Generator gehört dem Aufrufer und muss länger leben als die Quelle |
| `decodeStereo(datei, rate, maxSekunden)` | Eine Datei ganz dekodiert, Stereo, auf die Abtastrate umgerechnet — die Klänge eines Loops |
| `probeDurationMs(datei)`, `probePeaks(datei, …)` | Länge und Hüllkurve einer Datei, ohne sie zu spielen (Clips und Wellenform im Composer) |

**Drei, die spielen können — einer hat den Ton** (Rangfolge: Composer,
Loop-Panel, Player):

| Wer | Klasse | Was er spielt |
|---|---|---|
| Player | `AudioPlayer` | Einen Stream aus der Playlist |
| Composer | `composer/Tonfuehrung` | Einen Mixer mit den Clips der Audio-Kanäle: Songs als Datei-Quellen, Loops (`.lvloop`) als Generatoren. Die Uhr der Komposition führt; eine Quelle, die ihr mehr als 80 ms davonläuft, wird nachgesetzt |
| Loop-Panel | `loop/Spieler` | Einen Mixer mit einem Generator (`loop/Klangerzeuger`): der geladene Loop im Kreis, oder ein Muster allein |

Wer den Ton übernimmt, hält den Player an und gibt ihn beim Freigeben zurück;
Lautstärke, Mute und Pegel des Players gelten für den jeweiligen Mixer. Hat
der Composer den Ton, spielt das Loop-Panel nicht. Der Composer hängt dabei
die Playlist am Player aus, das Loop-Panel nicht.

Details: [Composer_Konzept.md](../ui/Composer_Konzept.md) §4 (Uhr, Tonführung)
und [Loop_Konzept.md](Loop_Konzept.md) (Rechenkern, Klangerzeuger, Messungen).

---

## 7. Signalkette zur Visualisierung

### 7.1 Datenweg (Ist-Stand)

**Welcher Stream gelesen wird:** `MainWindow::onAudioUpdate` holt Spektrum und
Wellenform von dem, der den Ton hat — vom Mixer des Composers
(`composer::tonMixer`), sonst vom Mixer des Loops (`loop::tonMixer`), sonst
vom Stream des Players. Die Visuals reagieren so immer auf das, was zu hören
ist; alles ab dem Pull ist für alle drei gleich.

```
BassEngine (BASS-FFT, 1024 Bins + Waveform 1024 Samples)
    │   Pull im QTimer ~30 Hz (MainWindow::onAudioUpdate, Main-Thread)
    ▼
VisualizerWidget::updateSpectrum(512 Bins) / updateWaveform(1024)
    │   (untere FFT-Hälfte = nutzbare Frequenz-Bins)
    ▼
Audio-Snapshot-Puffer (Double-Buffer, Mutex)   ← Thread-Grenze!
    │   Render-Thread übernimmt den jüngsten Stand am Frame-Anfang
    │   und ruft updateSpectrum/updateWaveform des Visualizers auf
    ▼
Visualizer (Equalizer/Oscilloscope/Pulsing/…), je Instanz eigenes
    ▼
AudioSourceModule            ── Band-Mapping (Linear/Log/Mel),
    │                           dB-Normierung (floorDb→0, ceilDb→1, Gain),
    │                           Reduktion auf N Ausgabe-Bänder,
    │                           6-Band-Analyse (Sub…Treble)
    ▼
SmoothingModule (eingebettet) ── None / SMA / EMA / WMA / DEMA
    ▼
normalisiertes Spektrum (0–1) → Rendering (im Render-Thread)
```

Seit der Render-Thread-Entkopplung (Session 31) endet der Main-Thread-Anteil
am Snapshot-Puffer: alles ab dem Visualizer läuft im Render-Thread des
jeweiligen Fensters ([Render-Thread-Entwurf](../visuals/Render_Thread_Entwurf.md),
Threading-Vertrag: [Visualizer_Architecture.md §12](../visuals/Visualizer_Architecture.md)).

Jeder Visualizer besitzt ein eigenes `AudioSourceModule` (`m_audioSource`) und ruft
pro Frame `m_audioSource.update(spectrum, count, deltaTime)` auf. Die Parameter des
Moduls werden in den Visualizer-Parametern unter dem Präfix `audio.*` durchgereicht,
das eingebettete Smoothing darunter als `audio.smooth.*` — so landet die komplette
Kette automatisch im ConfigPanel. Beide Module unterstützen datei-basierte
User-Presets (`.audio` / `.smooth`).

### 7.2 Modul-Referenz

Details (Parameter, Algorithmen, Presets, Fehlerverhalten) stehen header-nah:

- [`AudioSourceModule.hpp`](../../include/visualizers/modules/source/AudioSourceModule.hpp)
  — FrequencyScale (Linear/Log/Mel), AudioPresets (Default/BassHeavy/Vocals/
  Electronic/Ambient/Custom), `FrequencyBands` (sub/bass/lowMid/mid/highMid/treble)
- [`SmoothingModule.hpp`](../../include/visualizers/modules/processing/SmoothingModule.hpp)
  — Algorithmen None/SMA/EMA/WMA/DEMA, Builtin-Presets (Instant…Sluggish),
  `timeMs` (EMA/DEMA) vs. `windowSize` (SMA/WMA)

### 7.3 Status AudioAnalyzer

`AudioAnalyzer`/`IAudioAnalyzer` (samt `AudioDataEvent`) wurden **entfernt**
(Phase 4, Schritt 0 — 2026-07-19): Die Implementierung war vollständig, aber nie
als Service registriert und ohne einen einzigen Aufrufer; der produktive Datenweg
ist der direkte Push aus `MainWindow::onAudioUpdate()` (7.1). Eine saubere
Audio-Verteilstelle (Service statt MainWindow-QTimer) wird mit der
Import-/Audio-Phase neu entworfen; die alte Implementierung liegt in der
Git-Historie (bis Commit „Phase 4 Schritt 0").

---

## 8. Siehe auch

- [`include/audio/AudioSystem.md`](../../include/audio/AudioSystem.md) — header-nahe CppModuleDoc des Audio-Moduls
- [`include/visualizers/Visualizers.md`](../../include/visualizers/Visualizers.md) — Visualizer-Übersicht
- `docs/architecture/` — Architektur-Doku (EventBus, ServiceContainer)
- [`externals/bass/SETUP.md`](../../../../../externals/bass/SETUP.md) — BASS-Beschaffung

---

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 1.3.0 | 2026-10-11 | Session 109. **§6a NEU: Mixer, Quellen — und wer den Ton hat** (die Mixer-Aufrufe der Engine samt selbst rechnender Quelle und `decodeStereo`; Player, Tonführung des Composers, Spieler des Loop-Panels; Rangfolge). §7.1: welcher Stream für die Visuals gelesen wird. Der Mixer selbst ist seit S97 da und war hier nicht beschrieben |
| 1.2.0 | 2026-07-19 | Signalkette §7.1: Audio-Snapshot-Puffer als Thread-Grenze (Render-Thread-Entkopplung, Session 31) |
| 1.1.0 | 2026-07-19 | Session-Playlist dokumentiert (Auto-Save bei Exit, Auto-Restore bei Start — MainWindow, Session 31) |
| 1.0.0 | 2026-07-18 | Konsolidiert aus harvest/old_docs (Audio_System, AudioSourceModule, SmoothingModule), gegen Code abgeglichen (Ist-Stand: Direkt-Push statt Analyzer-Events, 30-Hz-Timer, PLS/JSON-Persistenz) |
