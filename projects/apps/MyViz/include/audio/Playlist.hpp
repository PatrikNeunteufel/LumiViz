/**
 ****************************************************************************************
 * @file   Playlist.hpp
 * @brief  Playlist Service Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * Concrete implementation of IPlaylist that:
 * - Manages list of audio tracks
 * - Supports add/remove/reorder operations
 * - Publishes events via EventBus
 * - Supports persistence (M3U, PLS, JSON)
 ****************************************************************************************
 */

#pragma once

#include "audio/IPlaylist.hpp"

#include <QObject>
#include <memory>

// =============================================================================
// Forward Declarations
// =============================================================================

class IEventBus;

// =============================================================================
// Playlist Implementation
// =============================================================================

/**
 * @class Playlist
 * @brief Concrete playlist service
 */
class Playlist : public QObject, public IPlaylist
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Construct Playlist
     *
     * @param eventBus Event bus for publishing events
     * @param parent Qt parent object
     */
    explicit Playlist(IEventBus& eventBus, QObject* parent = nullptr);
    ~Playlist() override;

    // Non-copyable
    Playlist(const Playlist&) = delete;
    Playlist& operator=(const Playlist&) = delete;

    // =========================================================================
    // IPlaylist Implementation
    // =========================================================================

    // Track Management
    int addTrack(const QString& filePath) override;
    int addTracks(const QStringList& filePaths) override;
    bool insertTrack(int index, const QString& filePath) override;
    bool removeTrack(int index) override;
    int removeTracks(const std::vector<int>& indices) override;
    void clear() override;

    // Track Access
    [[nodiscard]] int count() const override;
    [[nodiscard]] bool isEmpty() const override;
    [[nodiscard]] TrackInfo trackAt(int index) const override;
    [[nodiscard]] QString filePathAt(int index) const override;
    [[nodiscard]] std::vector<TrackInfo> tracks() const override;
    [[nodiscard]] QStringList filePaths() const override;
    [[nodiscard]] int indexOf(const QString& filePath) const override;

    // Reordering
    bool moveTrack(int fromIndex, int toIndex) override;
    bool swapTracks(int index1, int index2) override;
    void shuffle() override;
    void sort(const QString& field, bool ascending = true) override;

    // Current Track
    [[nodiscard]] int currentIndex() const override;
    void setCurrentIndex(int index) override;
    [[nodiscard]] TrackInfo currentTrack() const override;
    [[nodiscard]] bool hasNext() const override;
    [[nodiscard]] bool hasPrevious() const override;
    int next(bool wrap = false) override;
    int previous(bool wrap = false) override;

    // Persistence
    bool save(const QString& filePath) const override;
    bool load(const QString& filePath) override;

    // Metadata
    [[nodiscard]] QString name() const override;
    void setName(const QString& name) override;
    [[nodiscard]] int totalDurationMs() const override;
    void refreshMetadata(std::function<void(int, int)> callback = nullptr) override;

Q_SIGNALS:
    // Qt Signals (alternative to EventBus)
    void trackAdded(int index);
    void trackRemoved(int index);
    void tracksCleared();
    void currentIndexChanged(int index);
    void playlistLoaded(const QString& filePath);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    void publishPlaylistChanged(PlaylistChangedEvent::Action action,
                                int affectedIndex = -1);
    void publishIndexChanged(int previousIndex);
    
    // Persistence helpers
    bool saveM3U(const QString& filePath) const;
    bool savePLS(const QString& filePath) const;
    bool saveJSON(const QString& filePath) const;
    bool loadM3U(const QString& filePath);
    bool loadPLS(const QString& filePath);
    bool loadJSON(const QString& filePath);
};
