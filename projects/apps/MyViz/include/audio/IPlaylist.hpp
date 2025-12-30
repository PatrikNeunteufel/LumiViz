/**
 ****************************************************************************************
 * @file   IPlaylist.hpp
 * @brief  Interface for Playlist Management
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Playlist Service
 *
 * Manages a list of audio tracks with:
 * - Add/Remove/Clear operations
 * - Reordering (drag & drop)
 * - Current track tracking
 * - Save/Load functionality
 *
 * ```
 * ┌────────────────┐     ┌─────────────┐     ┌───────────────┐
 * │ PlaylistPanel  │────►│  IPlaylist  │◄────│  IAudioPlayer │
 * │  (UI)          │     │  (Service)  │     │  (Playback)   │
 * └────────────────┘     └──────┬──────┘     └───────────────┘
 *                               │
 *                               │ Events
 *                               ▼
 *                       ┌───────────────┐
 *                       │   EventBus    │
 *                       └───────────────┘
 * ```
 ****************************************************************************************
 */

#pragma once

#include "AudioEvents.hpp"

#include <QString>
#include <QStringList>
#include <vector>
#include <functional>

// =============================================================================
// IPlaylist Interface
// =============================================================================

/**
 * @class IPlaylist
 * @brief Interface for playlist management
 */
class IPlaylist
{
public:
    virtual ~IPlaylist() = default;

    // =========================================================================
    // Track Management
    // =========================================================================

    /**
     * @brief Add a track to the playlist
     *
     * @param filePath Path to audio file
     * @return Index where track was added, or -1 on error
     */
    virtual int addTrack(const QString& filePath) = 0;

    /**
     * @brief Add multiple tracks
     *
     * @param filePaths List of file paths
     * @return Number of tracks successfully added
     */
    virtual int addTracks(const QStringList& filePaths) = 0;

    /**
     * @brief Insert track at specific position
     *
     * @param index Position to insert at
     * @param filePath Path to audio file
     * @return true if inserted successfully
     */
    virtual bool insertTrack(int index, const QString& filePath) = 0;

    /**
     * @brief Remove track at index
     *
     * @param index Track index to remove
     * @return true if removed successfully
     */
    virtual bool removeTrack(int index) = 0;

    /**
     * @brief Remove multiple tracks
     *
     * @param indices Indices to remove (will be sorted internally)
     * @return Number of tracks removed
     */
    virtual int removeTracks(const std::vector<int>& indices) = 0;

    /**
     * @brief Clear all tracks
     */
    virtual void clear() = 0;

    // =========================================================================
    // Track Access
    // =========================================================================

    /**
     * @brief Get track count
     */
    [[nodiscard]] virtual int count() const = 0;

    /**
     * @brief Check if playlist is empty
     */
    [[nodiscard]] virtual bool isEmpty() const = 0;

    /**
     * @brief Get track info at index
     *
     * @param index Track index
     * @return Track info (empty if invalid index)
     */
    [[nodiscard]] virtual TrackInfo trackAt(int index) const = 0;

    /**
     * @brief Get file path at index
     *
     * @param index Track index
     * @return File path or empty string if invalid
     */
    [[nodiscard]] virtual QString filePathAt(int index) const = 0;

    /**
     * @brief Get all tracks
     */
    [[nodiscard]] virtual std::vector<TrackInfo> tracks() const = 0;

    /**
     * @brief Get all file paths
     */
    [[nodiscard]] virtual QStringList filePaths() const = 0;

    /**
     * @brief Find track index by file path
     *
     * @param filePath Path to find
     * @return Index or -1 if not found
     */
    [[nodiscard]] virtual int indexOf(const QString& filePath) const = 0;

    // =========================================================================
    // Reordering
    // =========================================================================

    /**
     * @brief Move track from one position to another
     *
     * @param fromIndex Source index
     * @param toIndex Destination index
     * @return true if moved successfully
     */
    virtual bool moveTrack(int fromIndex, int toIndex) = 0;

    /**
     * @brief Swap two tracks
     *
     * @param index1 First track index
     * @param index2 Second track index
     * @return true if swapped successfully
     */
    virtual bool swapTracks(int index1, int index2) = 0;

    /**
     * @brief Shuffle playlist order
     */
    virtual void shuffle() = 0;

    /**
     * @brief Sort playlist by field
     *
     * @param field "title", "artist", "album", "duration", "path"
     * @param ascending Sort order
     */
    virtual void sort(const QString& field, bool ascending = true) = 0;

    // =========================================================================
    // Current Track
    // =========================================================================

    /**
     * @brief Get current track index
     * @return Current index or -1 if none
     */
    [[nodiscard]] virtual int currentIndex() const = 0;

    /**
     * @brief Set current track index
     *
     * @param index New current index (-1 to clear)
     */
    virtual void setCurrentIndex(int index) = 0;

    /**
     * @brief Get current track info
     */
    [[nodiscard]] virtual TrackInfo currentTrack() const = 0;

    /**
     * @brief Check if there is a next track
     */
    [[nodiscard]] virtual bool hasNext() const = 0;

    /**
     * @brief Check if there is a previous track
     */
    [[nodiscard]] virtual bool hasPrevious() const = 0;

    /**
     * @brief Advance to next track
     *
     * @param wrap Wrap to first track if at end
     * @return New current index or -1 if no next track
     */
    virtual int next(bool wrap = false) = 0;

    /**
     * @brief Go to previous track
     *
     * @param wrap Wrap to last track if at beginning
     * @return New current index or -1 if no previous track
     */
    virtual int previous(bool wrap = false) = 0;

    // =========================================================================
    // Persistence
    // =========================================================================

    /**
     * @brief Save playlist to file
     *
     * Supports: .m3u, .m3u8, .pls, .json
     *
     * @param filePath Output file path
     * @return true if saved successfully
     */
    virtual bool save(const QString& filePath) const = 0;

    /**
     * @brief Load playlist from file
     *
     * @param filePath Playlist file path
     * @return true if loaded successfully
     */
    virtual bool load(const QString& filePath) = 0;

    // =========================================================================
    // Metadata
    // =========================================================================

    /**
     * @brief Get/Set playlist name
     */
    [[nodiscard]] virtual QString name() const = 0;
    virtual void setName(const QString& name) = 0;

    /**
     * @brief Get total duration of all tracks (ms)
     */
    [[nodiscard]] virtual int totalDurationMs() const = 0;

    /**
     * @brief Refresh metadata for all tracks
     *
     * Useful after loading playlist file paths.
     *
     * @param callback Optional progress callback (index, total)
     */
    virtual void refreshMetadata(
        std::function<void(int, int)> callback = nullptr) = 0;
};
