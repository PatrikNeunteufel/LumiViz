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
     * @brief Get/Set volume (0.0 - 1.0)
     */
    [[nodiscard]] virtual float getVolume(AudioStreamHandle stream) const = 0;
    virtual bool setVolume(AudioStreamHandle stream, float volume) = 0;

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
};
