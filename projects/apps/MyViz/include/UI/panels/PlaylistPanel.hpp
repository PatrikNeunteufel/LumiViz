/**
 ****************************************************************************************
 * @file   PlaylistPanel.hpp
 * @brief  Playlist management panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.1.0
 *
 * @details
 * ## PlaylistPanel
 *
 * Provides playlist management:
 * - Track list display with current track highlight (bold)
 * - Add/Remove tracks via file dialog
 * - Search/Filter functionality
 * - Drag & Drop reordering
 * - Double-click to play
 * - Shuffle mode (random next track)
 * - Loop mode (repeat playlist)
 *
 * ## Integration
 *
 * ```
 * ┌─────────────────┐     Events       ┌───────────────┐
 * │    IPlaylist    │ ────────────────►│ PlaylistPanel │
 * │                 │                  │               │
 * ├─────────────────┤                  └───────┬───────┘
 * │   IAudioPlayer  │◄─────────────────────────┘
 * └─────────────────┘   playIndex(n)
 * ```
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <vector>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLineEdit;
class IPlaylist;

/**
 * @class PlaylistPanel
 * @brief Panel for playlist management
 */
class PlaylistPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit PlaylistPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~PlaylistPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

protected:
    void onActivate() override;
    void onDeactivate() override;
    void saveState() override;
    void restoreState() override;

private Q_SLOTS:
    void onAddClicked();
    void onRemoveClicked();
    void onClearClicked();
    void onShuffleClicked();
    void onLoopClicked();
    void onSaveClicked();
    void onLoadClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onSearchChanged(const QString& text);
    void onSelectionChanged();

private:
    void setupUI();
    void setupConnections();
    void subscribeToEvents();
    void unsubscribeFromEvents();
    
    // Sync with playlist service
    void refreshPlaylist();
    void highlightCurrentTrack(int index);
    void updateShuffleButton(bool enabled);
    void updateLoopButton(bool enabled);
    
    // Event handlers
    void onPlaylistChanged();
    void onPlaylistIndexChanged(int newIndex, int oldIndex);
    void onPlaybackModeChanged(bool shuffle, int repeatMode);

    // UI Elements
    QLineEdit* m_pSearchEdit = nullptr;
    QListWidget* m_pListWidget = nullptr;
    QPushButton* m_pAddButton = nullptr;
    QPushButton* m_pRemoveButton = nullptr;
    QPushButton* m_pClearButton = nullptr;
    QPushButton* m_pSaveButton = nullptr;
    QPushButton* m_pLoadButton = nullptr;
    QPushButton* m_pShuffleButton = nullptr;
    QPushButton* m_pLoopButton = nullptr;
    
    // State
    int m_currentPlayingIndex = -1;
    bool m_shuffleEnabled = false;
    bool m_loopEnabled = false;
    std::vector<int> m_subscriptionIds;
};
