/**
 ****************************************************************************************
 * @file   AudioEvents.hpp
 * @brief  Event definitions for the Audio System
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Audio Events
 *
 * Diese Events werden vom Audio-System über den EventBus publiziert:
 *
 * ```
 * AudioPlayer ──► TrackChangedEvent     ──► PlaylistPanel, PlayerPanel
 *            ──► PlaybackStateEvent    ──► PlayerPanel
 *            ──► PlaybackPositionEvent ──► PlayerPanel (Progress)
 *            ──► VolumeChangedEvent    ──► PlayerPanel (Volume Slider)
 *
 * AudioAnalyzer ──► AudioDataEvent     ──► VisualizerWidget (Spectrum/Waveform)
 * ```
 ****************************************************************************************
 */

#pragma once

#include "services/events/Event.hpp"

#include <QString>
#include <vector>
#include <cstdint>

// =============================================================================
// Playback State
// =============================================================================

/**
 * @enum PlaybackState
 * @brief Audio playback state
 */
enum class PlaybackState
{
    Stopped,    ///< No track loaded or playback stopped
    Playing,    ///< Currently playing
    Paused,     ///< Paused at current position
    Loading,    ///< Loading track (buffering)
    Error       ///< Error occurred
};

// =============================================================================
// Track Information
// =============================================================================

/**
 * @struct TrackInfo
 * @brief Metadata for an audio track
 */
struct TrackInfo
{
    QString filePath;       ///< Full file path
    QString title;          ///< Track title (from metadata or filename)
    QString artist;         ///< Artist name
    QString album;          ///< Album name
    int durationMs = 0;     ///< Duration in milliseconds
    int sampleRate = 0;     ///< Sample rate (e.g., 44100)
    int channels = 0;       ///< Number of channels (1=mono, 2=stereo)
    int bitrate = 0;        ///< Bitrate in kbps
};

// =============================================================================
// Playback Events
// =============================================================================

/**
 * @struct TrackChangedEvent
 * @brief Emitted when a new track is loaded
 */
struct TrackChangedEvent : public Event
{
    EVENT_TYPE_NAME("TrackChangedEvent")
    
    TrackInfo track;        ///< Track metadata
    int playlistIndex = -1; ///< Index in playlist (-1 if not in playlist)
};

/**
 * @struct PlaybackStateEvent
 * @brief Emitted when playback state changes
 */
struct PlaybackStateEvent : public Event
{
    EVENT_TYPE_NAME("PlaybackStateEvent")
    
    PlaybackState state = PlaybackState::Stopped;
    PlaybackState previousState = PlaybackState::Stopped;
};

/**
 * @struct PlaybackPositionEvent
 * @brief Emitted periodically during playback (for progress bar)
 *
 * Typically emitted 10-30 times per second.
 */
struct PlaybackPositionEvent : public Event
{
    EVENT_TYPE_NAME("PlaybackPositionEvent")
    
    int positionMs = 0;     ///< Current position in milliseconds
    int durationMs = 0;     ///< Total duration in milliseconds
    float progress = 0.0f;  ///< Progress as 0.0 - 1.0
};

/**
 * @struct VolumeChangedEvent
 * @brief Emitted when volume changes
 */
struct VolumeChangedEvent : public Event
{
    EVENT_TYPE_NAME("VolumeChangedEvent")
    
    float volume = 1.0f;    ///< Volume level (0.0 - 1.0)
    bool muted = false;     ///< Whether audio is muted
};

// =============================================================================
// Playlist Events
// =============================================================================

/**
 * @struct PlaylistChangedEvent
 * @brief Emitted when playlist content changes
 */
struct PlaylistChangedEvent : public Event
{
    EVENT_TYPE_NAME("PlaylistChangedEvent")
    
    enum class Action { Added, Removed, Cleared, Reordered, Loaded };
    
    Action action = Action::Added;
    int trackCount = 0;     ///< Total tracks in playlist
    int affectedIndex = -1; ///< Index of affected track (-1 for bulk operations)
};

/**
 * @struct PlaylistIndexChangedEvent
 * @brief Emitted when current playlist index changes
 */
struct PlaylistIndexChangedEvent : public Event
{
    EVENT_TYPE_NAME("PlaylistIndexChangedEvent")
    
    int currentIndex = -1;  ///< New current index
    int previousIndex = -1; ///< Previous index
};

// =============================================================================
// Audio Analysis Events
// =============================================================================

/**
 * @struct AudioDataEvent
 * @brief Emitted with audio analysis data (spectrum + waveform)
 *
 * This event is published at the frame rate (60 Hz typically) and contains
 * real-time audio analysis data for visualization.
 *
 * ## Usage in Visualizer
 *
 * ```cpp
 * eventBus.subscribe<AudioDataEvent>([this](const AudioDataEvent& e) {
 *     m_visualizer->updateSpectrum(e.spectrum.data(), e.spectrum.size());
 *     m_visualizer->updateWaveform(e.waveform.data(), e.waveform.size());
 * });
 * ```
 */
struct AudioDataEvent : public Event
{
    EVENT_TYPE_NAME("AudioDataEvent")
    
    /// Spectrum data (FFT magnitudes, typically 512-2048 bands)
    /// Values are normalized 0.0 - 1.0
    std::vector<float> spectrum;
    
    /// Waveform data (raw audio samples, typically 512-2048 samples)
    /// Values are -1.0 to 1.0
    std::vector<float> waveform;
    
    /// Left channel level (0.0 - 1.0)
    float levelLeft = 0.0f;
    
    /// Right channel level (0.0 - 1.0)
    float levelRight = 0.0f;
    
    /// Beat detected this frame
    bool beatDetected = false;
    
    /// Timestamp in milliseconds
    std::uint64_t timestampMs = 0;
};

/**
 * @struct BeatEvent
 * @brief Emitted when a beat is detected
 */
struct BeatEvent : public Event
{
    EVENT_TYPE_NAME("BeatEvent")
    
    float intensity = 0.0f;     ///< Beat intensity (0.0 - 1.0)
    float bpm = 0.0f;           ///< Estimated BPM (if available)
    std::uint64_t timestampMs = 0;
};

// =============================================================================
// Audio Engine Events
// =============================================================================

/**
 * @struct AudioEngineErrorEvent
 * @brief Emitted when an audio error occurs
 */
struct AudioEngineErrorEvent : public Event
{
    EVENT_TYPE_NAME("AudioEngineErrorEvent")
    
    enum class ErrorType
    {
        InitFailed,         ///< Engine initialization failed
        DeviceNotFound,     ///< Audio device not found
        FileNotFound,       ///< Audio file not found
        FormatNotSupported, ///< Unsupported audio format
        DecodingError,      ///< Error decoding audio
        StreamError,        ///< Streaming error
        Unknown             ///< Unknown error
    };
    
    ErrorType type = ErrorType::Unknown;
    QString message;        ///< Human-readable error message
    int errorCode = 0;      ///< Backend-specific error code
};

/**
 * @struct AudioDeviceChangedEvent
 * @brief Emitted when audio output device changes
 */
struct AudioDeviceChangedEvent : public Event
{
    EVENT_TYPE_NAME("AudioDeviceChangedEvent")
    
    QString deviceName;     ///< New device name
    int deviceId = -1;      ///< Backend-specific device ID
};

/**
 * @struct PlaybackModeChangedEvent
 * @brief Emitted when shuffle or loop mode changes
 */
struct PlaybackModeChangedEvent : public Event
{
    EVENT_TYPE_NAME("PlaybackModeChangedEvent")
    
    bool shuffle = false;   ///< Shuffle mode enabled
    bool loop = false;      ///< Loop playlist mode enabled (RepeatMode::All)
};
