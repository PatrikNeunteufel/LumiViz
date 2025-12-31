/**
 ****************************************************************************************
 * @file   AudioPlayer.hpp
 * @brief  Audio Player Service Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * Concrete implementation of IAudioPlayer that:
 * - Uses IAudioEngine for low-level playback
 * - Manages playback state machine
 * - Publishes events via EventBus
 * - Integrates with IPlaylist
 ****************************************************************************************
 */

#pragma once

#include "audio/IAudioPlayer.hpp"
#include "audio/IAudioEngine.hpp"  // For AudioStreamHandle

#include <QObject>
#include <memory>

// =============================================================================
// Forward Declarations
// =============================================================================

class IAudioEngine;
class IEventBus;

// =============================================================================
// AudioPlayer Implementation
// =============================================================================

/**
 * @class AudioPlayer
 * @brief Concrete audio player service
 */
class AudioPlayer : public QObject, public IAudioPlayer
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Construct AudioPlayer
     *
     * @param engine Audio engine instance
     * @param eventBus Event bus for publishing events
     * @param parent Qt parent object
     */
    explicit AudioPlayer(IAudioEngine& engine,
                         IEventBus& eventBus,
                         QObject* parent = nullptr);
    ~AudioPlayer() override;

    // Non-copyable
    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    // =========================================================================
    // IAudioPlayer Implementation
    // =========================================================================

    // Playback Control
    bool load(const QString& filePath) override;
    void play() override;
    void pause() override;
    void togglePlayPause() override;
    void stop() override;

    // Playlist Navigation
    bool next() override;
    bool previous() override;
    bool playIndex(int index) override;

    // Seeking
    void seek(int positionMs) override;
    void seekFraction(float fraction) override;
    void seekRelative(int deltaMs) override;

    // Volume Control
    [[nodiscard]] float volume() const override;
    void setVolume(float volume) override;
    [[nodiscard]] bool isMuted() const override;
    void setMuted(bool muted) override;
    void toggleMute() override;

    // State Queries
    [[nodiscard]] PlaybackState state() const override;
    [[nodiscard]] bool isPlaying() const override;
    [[nodiscard]] bool isPaused() const override;
    [[nodiscard]] bool isStopped() const override;

    // Position / Duration
    [[nodiscard]] int positionMs() const override;
    [[nodiscard]] int durationMs() const override;
    [[nodiscard]] float positionFraction() const override;

    // Track Info
    [[nodiscard]] TrackInfo currentTrack() const override;
    [[nodiscard]] bool hasTrack() const override;

    // Playlist Integration
    void setPlaylist(IPlaylist* playlist) override;
    [[nodiscard]] IPlaylist* playlist() const override;
    [[nodiscard]] int playlistIndex() const override;

    // Repeat / Shuffle
    [[nodiscard]] RepeatMode repeatMode() const override;
    void setRepeatMode(RepeatMode mode) override;
    [[nodiscard]] bool shuffle() const override;
    void setShuffle(bool enabled) override;

    // Update
    void update() override;

    // =========================================================================
    // Additional Methods
    // =========================================================================

    /**
     * @brief Get the audio engine
     */
    [[nodiscard]] IAudioEngine& engine() const;

    /**
     * @brief Get current stream handle
     */
    [[nodiscard]] AudioStreamHandle currentStream() const;

Q_SIGNALS:
    // Qt Signals (alternative to EventBus)
    void stateChanged(PlaybackState state);
    void trackChanged(const TrackInfo& track);
    void positionChanged(int positionMs);
    void volumeChanged(float volume);
    void error(const QString& message);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    void setState(PlaybackState newState);
    void publishTrackChanged();
    void publishPositionEvent();
    void publishPlaybackModeChanged();
    void handleTrackEnd();
};
