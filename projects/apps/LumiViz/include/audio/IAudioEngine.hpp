/**
 ****************************************************************************************
 * @file   IAudioEngine.hpp
 * @brief  Interface for Audio Engine (BASS abstraction)
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Audio Engine Architektur
 *
 * ```
 * ┌─────────────────────────────────────────────────────────────┐
 * │                      IAudioEngine                           │
 * │  (Interface für Audio-Backend Abstraktion)                  │
 * ├─────────────────────────────────────────────────────────────┤
 * │  • initialize() / shutdown()                                │
 * │  • getDevices() / setDevice()                               │
 * │  • createStream() / freeStream()                            │
 * │  • getVersion() / isInitialized()                           │
 * └──────────────────────────┬──────────────────────────────────┘
 *                            │
 *                            ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │                      BassEngine                              │
 * │  (Konkrete BASS Library Implementation)                     │
 * └─────────────────────────────────────────────────────────────┘
 * ```
 *
 * Das Interface ermöglicht:
 * - Austauschbare Audio-Backends (BASS, PortAudio, etc.)
 * - Einfaches Mocking für Unit-Tests
 * - Klare Trennung von Abstraktion und Implementation
 ****************************************************************************************
 */

#pragma once

#include <QString>
#include <QStringList>
#include <atomic>
#include <vector>
#include <cstdint>
#include <functional>

// =============================================================================
// Forward Declarations
// =============================================================================

class IEventBus;

// =============================================================================
// Audio Device Info
// =============================================================================

/**
 * @struct AudioDeviceInfo
 * @brief Information about an audio output device
 */
struct AudioDeviceInfo
{
    int id = -1;                ///< Device ID (backend-specific)
    QString name;               ///< Device name
    QString driver;             ///< Driver name
    bool isDefault = false;     ///< Is this the default device?
    bool isEnabled = true;      ///< Is the device enabled/available?
    bool isLoopback = false;    ///< Is this a loopback device?
    int sampleRate = 44100;     ///< Preferred sample rate
    int channels = 2;           ///< Number of channels
};

// =============================================================================
// Audio Stream Handle
// =============================================================================

/**
 * @typedef AudioStreamHandle
 * @brief Opaque handle for audio streams
 */
using AudioStreamHandle = std::uint32_t;

/// Invalid stream handle constant
constexpr AudioStreamHandle INVALID_STREAM = 0;

/// Groesste Lautstaerke, die `IAudioEngine::setVolume` annimmt: 16 = +24 dB,
/// das hoechste Gain eines Ton-Clips im Composer (S126, `kGainMaxDb`).
constexpr float kHoechsteLautstaerke = 16.0f;

// =============================================================================
// Audio Generator
// =============================================================================

/**
 * @class IAudioGenerator
 * @brief Eine Quelle, die ihren Ton selbst rechnet (Loop,
 *        docs/audio/Loop_Konzept.md §6) — als Quelle im Mixer, siehe
 *        `IAudioEngine::mixerAddGenerator`.
 */
class IAudioGenerator
{
public:
    virtual ~IAudioGenerator() = default;

    /**
     * @brief `frames` Abtastwerte nach `out` schreiben: Stereo, float, links
     *        und rechts im Wechsel, in der Abtastrate der Engine.
     *
     * Laeuft im **Ton-Thread** der Engine, in Bloecken wechselnder Groesse:
     * keinen Speicher holen, keine Datei lesen, nicht warten.
     */
    virtual void render(float* out, int frames) = 0;
};

// =============================================================================
// IAudioEngine Interface
// =============================================================================

/**
 * @class IAudioEngine
 * @brief Interface for low-level audio engine operations
 *
 * This interface abstracts the audio backend (BASS library) and provides:
 * - Device enumeration and selection
 * - Stream creation and management
 * - Engine initialization and shutdown
 */
class IAudioEngine
{
public:
    virtual ~IAudioEngine() = default;

    // =========================================================================
    // Initialization
    // =========================================================================

    /**
     * @brief Initialize the audio engine
     *
     * @param deviceId Device to use (-1 for default)
     * @param sampleRate Sample rate (44100, 48000, etc.)
     * @return true if initialization successful
     */
    virtual bool initialize(int deviceId = -1, int sampleRate = 44100) = 0;

    /**
     * @brief Shutdown the audio engine
     *
     * Releases all resources and stops all streams.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Check if engine is initialized
     */
    [[nodiscard]] virtual bool isInitialized() const = 0;

    // =========================================================================
    // Device Management
    // =========================================================================

    /**
     * @brief Get list of available audio devices
     */
    [[nodiscard]] virtual std::vector<AudioDeviceInfo> getDevices() const = 0;

    /**
     * @brief Get current device ID
     */
    [[nodiscard]] virtual int getCurrentDevice() const = 0;

    /**
     * @brief Set output device
     *
     * @param deviceId Device ID from getDevices()
     * @return true if device change successful
     */
    virtual bool setDevice(int deviceId) = 0;

    /**
     * @brief Get current sample rate
     */
    [[nodiscard]] virtual int getSampleRate() const = 0;

    // =========================================================================
    // Stream Management
    // =========================================================================

    /**
     * @brief Create audio stream from file
     *
     * @param filePath Path to audio file
     * @return Stream handle or INVALID_STREAM on error
     */
    [[nodiscard]] virtual AudioStreamHandle createStream(const QString& filePath) = 0;

    /**
     * @brief Create loopback stream for system audio capture
     *
     * @return Stream handle or INVALID_STREAM on error
     */
    [[nodiscard]] virtual AudioStreamHandle createLoopbackStream() = 0;

    /**
     * @brief Free an audio stream
     *
     * @param stream Stream handle
     */
    virtual void freeStream(AudioStreamHandle stream) = 0;

    // =========================================================================
    // Stream Playback Control
    // =========================================================================

    /**
     * @brief Start/resume playback
     */
    virtual bool play(AudioStreamHandle stream) = 0;

    /**
     * @brief Pause playback
     */
    virtual bool pause(AudioStreamHandle stream) = 0;

    /**
     * @brief Stop playback and reset position
     */
    virtual bool stop(AudioStreamHandle stream) = 0;

    /**
     * @brief Check if stream is playing
     */
    [[nodiscard]] virtual bool isPlaying(AudioStreamHandle stream) const = 0;

    /**
     * @brief Check if stream is paused
     */
    [[nodiscard]] virtual bool isPaused(AudioStreamHandle stream) const = 0;

    // =========================================================================
    // Stream Properties
    // =========================================================================

    /**
     * @brief Get stream position in milliseconds
     */
    [[nodiscard]] virtual int getPositionMs(AudioStreamHandle stream) const = 0;

    /**
     * @brief Set stream position in milliseconds
     */
    virtual bool setPositionMs(AudioStreamHandle stream, int positionMs) = 0;

    /**
     * @brief Get stream duration in milliseconds
     */
    [[nodiscard]] virtual int getDurationMs(AudioStreamHandle stream) const = 0;

    /**
     * @brief Dauer einer Datei in ms, ohne sie abzuspielen (S87: Playlist
     *        zeigt die Laenge je Titel). 0 = unbekannt / nicht lesbar.
     *        Schnell statt genau: ohne Vorab-Durchlauf der Datei — bei MP3
     *        mit variabler Bitrate ohne Laengenangabe eine Schaetzung.
     */
    [[nodiscard]] virtual int probeDurationMs(const QString& /*filePath*/) { return 0; }

    /**
     * @brief Huellkurve einer Datei: je `msProWert` Millisekunden der groesste
     *        Betrag ueber alle Kanaele (0..1). Dekodiert die ganze Datei —
     *        darf aus einem eigenen Thread gerufen werden; `abbruch` (optional)
     *        beendet die Arbeit vorzeitig. Leer = nicht lesbar. (S87: Wellenform
     *        im Panel „Lyrics Sync".)
     */
    [[nodiscard]] virtual std::vector<float> probePeaks(const QString& /*filePath*/,
                                                        int /*msProWert*/,
                                                        const std::atomic<bool>* /*abbruch*/ = nullptr)
    {
        return {};
    }

    /**
     * @brief Get/Set volume: 0 = still, 1 = wie die Datei, darueber verstaerkt
     *        (bis `kHoechsteLautstaerke`, S126 — das Gain eines Ton-Clips).
     */
    [[nodiscard]] virtual float getVolume(AudioStreamHandle stream) const = 0;
    virtual bool setVolume(AudioStreamHandle stream, float volume) = 0;

    /**
     * @brief Abspieltempo eines Streams (S91, Lyrics Sync: langsamer klopfen).
     *        `faktor` 1.0 = wie aufgenommen, 0.5 = halb so schnell. Geht ueber
     *        die Abtastrate — der Ton wird mit tiefer. Die Position des Streams
     *        (`getPositionMs`) bleibt Zeit im Titel, nicht Wanduhr.
     *        Default false = die Engine kann es nicht.
     */
    virtual bool setTempo(AudioStreamHandle /*stream*/, float /*faktor*/) { return false; }

    // =========================================================================
    // Mixer (Composer-Naht N12, docs/ui/Composer_Konzept.md §4 und §12.2)
    // =========================================================================
    // Mehrere Dateien zugleich in EINEM Stream: was aus dem Mixer kommt, ist die
    // Summe seiner Quellen mit ihren Pegeln — zu hoeren und zu messen an
    // derselben Stelle. Der Mixer ist ein Stream wie jeder andere (`play`,
    // `pause`, `setVolume`, `getFFTData`, `freeStream` …). Eine Quelle ist kein
    // spielbarer Stream: fuer sie gelten nur die Aufrufe hier, dazu `setVolume`
    // (ihr Pegel im Mix), `getDurationMs` und die Metadaten.
    // Vorgabe ueberall: die Engine kann es nicht.

    /**
     * @brief Einen Mixer anlegen (Stereo, ohne Puffer: ein neuer Pegel oder eine
     *        neue Stelle einer Quelle ist nach wenigen zehn Millisekunden zu
     *        hoeren). Ohne Quellen spielt er Stille und laeuft weiter.
     * @return Stream-Handle oder INVALID_STREAM
     */
    [[nodiscard]] virtual AudioStreamHandle createMixer() { return INVALID_STREAM; }

    /**
     * @brief Eine Datei als Quelle in den Mixer haengen. Sie steht am Anfang und
     *        ist **angehalten** — erst Stelle und Pegel setzen, dann
     *        `setSourcePaused(…, false)`.
     * @return Handle der Quelle oder INVALID_STREAM
     */
    [[nodiscard]] virtual AudioStreamHandle mixerAddSource(AudioStreamHandle /*mixer*/,
                                                           const QString& /*filePath*/)
    {
        return INVALID_STREAM;
    }

    /** @brief Eine Quelle aus ihrem Mixer nehmen und freigeben. */
    virtual void mixerRemoveSource(AudioStreamHandle /*source*/) {}

    /** @brief Stelle einer Quelle in ms — die Stelle, die gerade zu hoeren ist. */
    [[nodiscard]] virtual int getSourcePositionMs(AudioStreamHandle /*source*/) const { return 0; }

    /** @brief Eine Quelle an eine Stelle setzen; ob sie laeuft, bleibt. */
    virtual bool setSourcePositionMs(AudioStreamHandle /*source*/, int /*positionMs*/) { return false; }

    /** @brief Eine Quelle anhalten oder weiterlaufen lassen; der Mixer laeuft davon unberuehrt. */
    virtual bool setSourcePaused(AudioStreamHandle /*source*/, bool /*paused*/) { return false; }

    /** @brief Liefert die Quelle gerade Ton in den Mixer (nicht angehalten, nicht am Ende)? */
    [[nodiscard]] virtual bool isSourcePlaying(AudioStreamHandle /*source*/) const { return false; }

    /**
     * @brief Eine selbst rechnende Quelle in den Mixer haengen (Loop,
     *        docs/audio/Loop_Konzept.md §6). Sie ist **angehalten** — erst
     *        `setSourcePaused(…, false)` laesst den Mixer bei ihr Ton holen.
     *
     * Der Mixer ruft `generator->render` im Ton-Thread. Die Stelle fuehrt der
     * Generator selbst: `setSourcePositionMs` aendert an ihm nichts. Der
     * Generator gehoert dem Aufrufer und muss leben, bis `mixerRemoveSource`
     * zurueck ist.
     * @return Handle der Quelle oder INVALID_STREAM
     */
    [[nodiscard]] virtual AudioStreamHandle mixerAddGenerator(AudioStreamHandle /*mixer*/,
                                                              IAudioGenerator* /*generator*/)
    {
        return INVALID_STREAM;
    }

    /**
     * @brief Den Limiter am Mixer an- oder ausschalten (S126, Composer_Konzept
     *        §9g, L4 a). Er haelt die Summe aller Quellen unter −0,1 dBFS statt
     *        sie hart abzuschneiden; was darunter bleibt, geht Bit fuer Bit
     *        durch. Die Visuals sehen das Signal hinter ihm (Beleg G4).
     * @return false = kein Mixer, oder die Engine kennt keinen Limiter
     */
    virtual bool setMixerLimiter(AudioStreamHandle /*mixer*/, bool /*an*/) { return false; }
    /// Wie viele Frames der Limiter am Mixer bisher begrenzt hat; −1 = keiner dran.
    [[nodiscard]] virtual long long mixerLimiterFrames(AudioStreamHandle /*mixer*/) const { return -1; }

    /**
     * @brief Eine Datei ganz dekodieren: Stereo, float, links und rechts im
     *        Wechsel, umgerechnet auf `sampleRate`; hoechstens `maxSeconds`.
     *        Fuer kurze Klaenge (Samples des Loops), nicht fuer Songs.
     *        Leer = nicht lesbar, oder die Engine kann es nicht.
     */
    [[nodiscard]] virtual std::vector<float> decodeStereo(const QString& /*filePath*/, int /*sampleRate*/,
                                                          double /*maxSeconds*/ = 30.0)
    {
        return {};
    }

    /// Nimmt ein Stueck dekodierten Ton: Stereo, float, links und rechts im
    /// Wechsel; false = abbrechen.
    using DecodeChunk = std::function<bool(const float* stereo, std::size_t frames)>;
    /**
     * @brief Einen Ausschnitt einer Datei stueckweise dekodieren (S127,
     *        Composer_Konzept §9g, G5-3 a): ab `startMs`, hoechstens
     *        `lengthMs` (0 = bis zum Ende), Stereo, umgerechnet auf
     *        `sampleRate`. Fuer Songs — nichts davon liegt ganz im Speicher.
     *        Darf aus einem anderen Thread als dem der Oberflaeche laufen.
     * @return false = nicht lesbar, abgebrochen, oder die Engine kann es nicht
     */
    virtual bool decodeStereoChunks(const QString& /*filePath*/, int /*sampleRate*/, std::int64_t /*startMs*/,
                                    std::int64_t /*lengthMs*/, const DecodeChunk& /*take*/)
    {
        return false;
    }

    // =========================================================================
    // Aufnahme (Loop, Stufe 11b: „Record sample"; docs/audio/Loop_Konzept.md §4h)
    // =========================================================================
    // Eine Aufnahme zur Zeit: Stereo, float, links und rechts im Wechsel. Die
    // Engine sammelt, was der Eingang liefert, bis `recordStop` es abholt.
    // Vorgabe ueberall: die Engine kann es nicht.

    /** @brief Die Eingaenge. `isLoopback` = ein Mitschnitt der Ausgabe. */
    [[nodiscard]] virtual std::vector<AudioDeviceInfo> getRecordDevices() const { return {}; }

    /**
     * @brief Eine Aufnahme beginnen: von `deviceId` (-1 = der Standard-Eingang),
     *        in `sampleRate`, hoechstens `maxSeconds` — danach endet sie von
     *        selbst (`isRecording` wird false, das Aufgenommene bleibt bis
     *        `recordStop`). Eine laufende Aufnahme wird verworfen.
     * @return false = kein Eingang, er laesst sich nicht oeffnen, oder die
     *         Engine kann es nicht
     */
    virtual bool recordStart(int /*deviceId*/, int /*sampleRate*/, double /*maxSeconds*/) { return false; }

    /** @brief Laeuft eine Aufnahme? */
    [[nodiscard]] virtual bool isRecording() const { return false; }

    /** @brief So viele Abtastwerte (je Seite) sind bisher aufgenommen. */
    [[nodiscard]] virtual std::int64_t recordedFrames() const { return 0; }

    /** @brief Der groesste Betrag seit dem letzten Aufruf — fuer eine Pegelanzeige. */
    [[nodiscard]] virtual float recordLevel() { return 0.0f; }

    /**
     * @brief Die Aufnahme beenden und abholen. Der Eingang wird freigegeben.
     * @return das Aufgenommene; leer = es lief keine, oder es kam nichts an
     */
    [[nodiscard]] virtual std::vector<float> recordStop() { return {}; }

    // =========================================================================
    // FFT / Audio Analysis
    // =========================================================================

    /**
     * @brief Get FFT data for visualization
     *
     * @param stream Stream handle
     * @param[out] data Buffer to receive FFT data
     * @param size Number of FFT bins (256, 512, 1024, 2048, 4096, 8192)
     * @return true if data retrieved successfully
     */
    virtual bool getFFTData(AudioStreamHandle stream, float* data, int size) = 0;

    /**
     * @brief Get waveform data
     *
     * @param stream Stream handle
     * @param[out] data Buffer to receive waveform data
     * @param size Number of samples
     * @return true if data retrieved successfully
     */
    virtual bool getWaveformData(AudioStreamHandle stream, float* data, int size) = 0;

    /**
     * @brief Per-channel FFT (BASS_DATA_FFT_INDIVIDUAL): `data` receives
     *        size × channels interleaved bins (bin*channels + channel).
     *        Default returns false (engines without stereo FFT fall back to mono).
     */
    virtual bool getFFTDataStereo(AudioStreamHandle /*stream*/, float* /*data*/,
                                  int /*size*/)
    {
        return false;
    }

    /** @brief Channel count of a stream (1 mono, 2 stereo). Default 1. */
    virtual int getStreamChannels(AudioStreamHandle /*stream*/) { return 1; }

    /**
     * @brief Get channel levels
     *
     * @param stream Stream handle
     * @param[out] left Left channel level (0.0 - 1.0)
     * @param[out] right Right channel level (0.0 - 1.0)
     * @return true if levels retrieved successfully
     */
    virtual bool getChannelLevels(AudioStreamHandle stream, float& left, float& right) = 0;

    // =========================================================================
    // Metadata
    // =========================================================================

    /**
     * @brief Get track metadata
     *
     * @param stream Stream handle
     * @param[out] title Track title
     * @param[out] artist Artist name
     * @param[out] album Album name
     * @return true if metadata available
     */
    virtual bool getMetadata(AudioStreamHandle stream,
                             QString& title,
                             QString& artist,
                             QString& album) = 0;

    /**
     * @brief Further tags of the track (S86, Text node templates)
     *
     * @param[out] year Year (four digits where the tag allows)
     * @param[out] genre Genre name
     * @param[out] track Track number without the total
     * @return true if at least one of them was found
     */
    virtual bool getMetadataExtra(AudioStreamHandle /*stream*/, QString& year, QString& genre,
                                  QString& track)
    {
        year.clear();
        genre.clear();
        track.clear();
        return false;
    }

    /**
     * @brief Embedded lyrics of the track (S86, Lyrics node)
     *
     * @param[out] syncedLrc Synchronised lyrics as LRC text (from ID3 SYLT)
     * @param[out] plain Lyrics text as stored (ID3 USLT, Vorbis/APE/MP4 lyrics)
     * @return true if at least one of them was found
     */
    virtual bool getLyrics(AudioStreamHandle /*stream*/, QString& syncedLrc, QString& plain)
    {
        syncedLrc.clear();
        plain.clear();
        return false;
    }

    // =========================================================================
    // Engine Info
    // =========================================================================

    /**
     * @brief Get engine/library version string
     */
    [[nodiscard]] virtual QString getVersion() const = 0;

    /**
     * @brief Get last error message
     */
    [[nodiscard]] virtual QString getLastError() const = 0;

    /**
     * @brief Get last error code
     */
    [[nodiscard]] virtual int getLastErrorCode() const = 0;

    /**
     * @brief Abspielbare Dateiendungen, klein, ohne Punkt (S84: Audio Browser,
     *        Drops). Leer = unbekannt — die Oberflaeche nimmt dann ihre Vorgabe.
     */
    [[nodiscard]] virtual QStringList supportedExtensions() const { return {}; }
};
