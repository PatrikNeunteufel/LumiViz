/**
 ****************************************************************************************
 * @file   PlaylistPanel.hpp
 * @brief  Playlist management panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * ## PlaylistPanel
 *
 * Provides playlist management:
 * - Track list display with current track highlight
 * - Add/Remove tracks via file dialog
 * - Search/Filter functionality
 * - Drag & Drop reordering
 * - Double-click to play
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
    
    // Event handlers
    void onPlaylistChanged();
    void onPlaylistIndexChanged(int newIndex, int oldIndex);

    // UI Elements
    QLineEdit* m_pSearchEdit = nullptr;
    QListWidget* m_pListWidget = nullptr;
    QPushButton* m_pAddButton = nullptr;
    QPushButton* m_pRemoveButton = nullptr;
    QPushButton* m_pClearButton = nullptr;
    
    // State
    int m_currentPlayingIndex = -1;
    std::vector<int> m_subscriptionIds;
};
