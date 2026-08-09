/**
 ****************************************************************************************
 * @file   IAudioPlayer.hpp
 * @brief  Interface for Audio Player Service
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Audio Player Service
 *
 * High-level audio playback interface that:
 * - Manages playback state (play, pause, stop, seek)
 * - Integrates with Playlist
 * - Publishes events via EventBus
 *
 * ```
 * ┌──────────────┐     ┌───────────────┐     ┌─────────────────┐
 * │ PlayerPanel  │────►│  IAudioPlayer │────►│   IAudioEngine  │
 * │  (UI)        │     │   (Service)   │     │   (BASS)        │
 * └──────────────┘     └───────┬───────┘     └─────────────────┘
 *                              │
 *                              │ Events
 *                              ▼
 *                      ┌───────────────┐
 *                      │   EventBus    │
 *                      └───────────────┘
 * ```
 ****************************************************************************************
 */

#pragma once

#include "AudioEvents.hpp"

#include <QString>

// =============================================================================
// Forward Declarations
// =============================================================================

class IPlaylist;

// =============================================================================
// IAudioPlayer Interface
// =============================================================================

/**
 * @class IAudioPlayer
 * @brief Interface for high-level audio playback control
 *
 * The AudioPlayer service provides:
 * - Simple play/pause/stop/seek API
 * - Playlist integration (next/previous)
 * - Volume and mute control
 * - Event publishing for UI updates
 */
class IAudioPlayer
{
public:
    virtual ~IAudioPlayer() = default;

    // =========================================================================
    // Playback Control
    // =========================================================================

    /**
     * @brief Load and play a file
     *
     * @param filePath Path to audio file
     * @return true if file loaded successfully
     */
    virtual bool load(const QString& filePath) = 0;

    /**
     * @brief Start or resume playback
     */
    virtual void play() = 0;

    /**
     * @brief Pause playback
     */
    virtual void pause() = 0;

    /**
     * @brief Toggle play/pause
     */
    virtual void togglePlayPause() = 0;

    /**
     * @brief Stop playback and reset position
     */
    virtual void stop() = 0;

    // =========================================================================
    // Playlist Navigation
    // =========================================================================

    /**
     * @brief Play next track in playlist
     * @return true if next track exists and was loaded
     */
    virtual bool next() = 0;

    /**
     * @brief Play previous track in playlist
     * @return true if previous track exists and was loaded
     */
    virtual bool previous() = 0;

    /**
     * @brief Play track at specific playlist index
     *
     * @param index Playlist index
     * @return true if track loaded successfully
     */
    virtual bool playIndex(int index) = 0;

    // =========================================================================
    // Seeking
    // =========================================================================

    /**
     * @brief Seek to position in milliseconds
     *
     * @param positionMs Position in milliseconds
     */
    virtual void seek(int positionMs) = 0;

    /**
     * @brief Seek to position as fraction (0.0 - 1.0)
     *
     * @param fraction Position as 0.0 (start) to 1.0 (end)
     */
    virtual void seekFraction(float fraction) = 0;

    /**
     * @brief Seek relative to current position
     *
     * @param deltaMs Milliseconds to seek (negative = backward)
     */
    virtual void seekRelative(int deltaMs) = 0;

    // =========================================================================
    // Volume Control
    // =========================================================================

    /**
     * @brief Get current volume
     * @return Volume level (0.0 - 1.0)
     */
    [[nodiscard]] virtual float volume() const = 0;

    /**
     * @brief Set volume
     *
     * @param volume Volume level (0.0 - 1.0, clamped)
     */
    virtual void setVolume(float volume) = 0;

    /**
     * @brief Check if muted
     */
    [[nodiscard]] virtual bool isMuted() const = 0;

    /**
     * @brief Set mute state
     */
    virtual void setMuted(bool muted) = 0;

    /**
     * @brief Toggle mute
     */
    virtual void toggleMute() = 0;

    // =========================================================================
    // State Queries
    // =========================================================================

    /**
     * @brief Get current playback state
     */
    [[nodiscard]] virtual PlaybackState state() const = 0;

    /**
     * @brief Check if currently playing
     */
    [[nodiscard]] virtual bool isPlaying() const = 0;

    /**
     * @brief Check if paused
     */
    [[nodiscard]] virtual bool isPaused() const = 0;

    /**
     * @brief Check if stopped
     */
    [[nodiscard]] virtual bool isStopped() const = 0;

    // =========================================================================
    // Position / Duration
    // =========================================================================

    /**
     * @brief Get current position in milliseconds
     */
    [[nodiscard]] virtual int positionMs() const = 0;

    /**
     * @brief Get total duration in milliseconds
     */
    [[nodiscard]] virtual int durationMs() const = 0;

    /**
     * @brief Get position as fraction (0.0 - 1.0)
     */
    [[nodiscard]] virtual float positionFraction() const = 0;

    // =========================================================================
    // Track Info
    // =========================================================================

    /**
     * @brief Get current track info
     */
    [[nodiscard]] virtual TrackInfo currentTrack() const = 0;

    /**
     * @brief Check if a track is loaded
     */
    [[nodiscard]] virtual bool hasTrack() const = 0;

    // =========================================================================
    // Playlist Integration
    // =========================================================================

    /**
     * @brief Set the playlist to use
     *
     * @param playlist Playlist instance (can be nullptr)
     */
    virtual void setPlaylist(IPlaylist* playlist) = 0;

    /**
     * @brief Get current playlist
     */
    [[nodiscard]] virtual IPlaylist* playlist() const = 0;

    /**
     * @brief Get current playlist index
     * @return Index or -1 if no playlist/track
     */
    [[nodiscard]] virtual int playlistIndex() const = 0;

    // =========================================================================
    // Repeat / Shuffle
    // =========================================================================

    /**
     * @enum RepeatMode
     * @brief Repeat modes for playback
     */
    enum class RepeatMode
    {
        None,       ///< No repeat
        One,        ///< Repeat current track
        All         ///< Repeat entire playlist
    };

    /**
     * @brief Get/Set repeat mode
     */
    [[nodiscard]] virtual RepeatMode repeatMode() const = 0;
    virtual void setRepeatMode(RepeatMode mode) = 0;

    /**
     * @brief Get/Set shuffle state
     */
    [[nodiscard]] virtual bool shuffle() const = 0;
    virtual void setShuffle(bool enabled) = 0;

    // =========================================================================
    // Update (called from main loop)
    // =========================================================================

    /**
     * @brief Update player state
     *
     * Should be called regularly (e.g., from render loop) to:
     * - Publish position events
     * - Handle track end (auto-advance playlist)
     */
    virtual void update() = 0;
};
