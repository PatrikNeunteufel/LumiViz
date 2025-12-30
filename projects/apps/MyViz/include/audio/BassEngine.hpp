/**
 ****************************************************************************************
 * @file   BassEngine.hpp
 * @brief  BASS Library Audio Engine Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## BASS Audio Library Integration
 *
 * This class wraps the BASS audio library (un4seen.com) for:
 * - Audio file playback (MP3, FLAC, WAV, OGG, etc.)
 * - System audio loopback capture
 * - FFT spectrum analysis
 * - Real-time audio data access
 *
 * ### BASS Library
 *
 * BASS is a widely-used audio library that supports:
 * - Multiple audio formats via plugins
 * - Low-latency playback
 * - Built-in FFT analysis
 * - WASAPI loopback for system audio capture
 *
 * @see https://www.un4seen.com/bass.html
 ****************************************************************************************
 */

#pragma once

#include "audio/IAudioEngine.hpp"

#include <QString>
#include <memory>

// =============================================================================
// BassEngine Implementation
// =============================================================================

/**
 * @class BassEngine
 * @brief BASS library audio engine implementation
 */
class BassEngine : public IAudioEngine
{
public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    BassEngine();
    ~BassEngine() override;

    // Non-copyable
    BassEngine(const BassEngine&) = delete;
    BassEngine& operator=(const BassEngine&) = delete;
    BassEngine(BassEngine&&) = delete;
    BassEngine& operator=(BassEngine&&) = delete;

    // =========================================================================
    // IAudioEngine Implementation
    // =========================================================================

    // Initialization
    bool initialize(int deviceId = -1, int sampleRate = 44100) override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const override;

    // Device Management
    [[nodiscard]] std::vector<AudioDeviceInfo> getDevices() const override;
    [[nodiscard]] int getCurrentDevice() const override;
    bool setDevice(int deviceId) override;
    [[nodiscard]] int getSampleRate() const override;

    // Stream Management
    [[nodiscard]] AudioStreamHandle createStream(const QString& filePath) override;
    [[nodiscard]] AudioStreamHandle createLoopbackStream() override;
    void freeStream(AudioStreamHandle stream) override;

    // Playback Control
    bool play(AudioStreamHandle stream) override;
    bool pause(AudioStreamHandle stream) override;
    bool stop(AudioStreamHandle stream) override;
    [[nodiscard]] bool isPlaying(AudioStreamHandle stream) const override;
    [[nodiscard]] bool isPaused(AudioStreamHandle stream) const override;

    // Stream Properties
    [[nodiscard]] int getPositionMs(AudioStreamHandle stream) const override;
    bool setPositionMs(AudioStreamHandle stream, int positionMs) override;
    [[nodiscard]] int getDurationMs(AudioStreamHandle stream) const override;
    [[nodiscard]] float getVolume(AudioStreamHandle stream) const override;
    bool setVolume(AudioStreamHandle stream, float volume) override;

    // FFT / Audio Analysis
    bool getFFTData(AudioStreamHandle stream, float* data, int size) override;
    bool getWaveformData(AudioStreamHandle stream, float* data, int size) override;
    bool getChannelLevels(AudioStreamHandle stream, float& left, float& right) override;

    // Metadata
    bool getMetadata(AudioStreamHandle stream,
                     QString& title,
                     QString& artist,
                     QString& album) override;

    // Engine Info
    [[nodiscard]] QString getVersion() const override;
    [[nodiscard]] QString getLastError() const override;
    [[nodiscard]] int getLastErrorCode() const override;

    // =========================================================================
    // BASS-Specific Methods
    // =========================================================================

    /**
     * @brief Load BASS plugins from directory
     *
     * @param pluginDir Directory containing BASS plugins
     * @return Number of plugins loaded
     */
    int loadPlugins(const QString& pluginDir);

    /**
     * @brief Get list of supported file extensions
     */
    [[nodiscard]] QStringList supportedExtensions() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
