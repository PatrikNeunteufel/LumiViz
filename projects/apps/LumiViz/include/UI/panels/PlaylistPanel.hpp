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

#include <QHash>
#include <QList>
#include <QString>

#include <vector>

class QTimer;
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

    /**
     * @brief Reihenfolge nach dem Verschieben von `zeilen` vor die Zeile `ziel`
     *        (S84; `ziel` = anzahl heisst ans Ende)
     * @return alte Indizes in neuer Reihenfolge; die gezogenen bleiben
     *         untereinander in ihrer Reihenfolge. Frei von Widgets, pruefbar.
     */
    [[nodiscard]] static std::vector<int> neueReihenfolge(int anzahl, const QList<int>& zeilen,
                                                          int ziel);

    /// Gezogene Zeilen vor `ziel` stellen — im DIENST (moveTrack); die Liste
    /// folgt ueber das PlaylistChangedEvent (S84). Vorher sortierte Qt nur die
    /// Anzeige um, und ein Doppelklick spielte danach den falschen Titel.
    static void verschiebeZeilen(IPlaylist& playlist, const QList<int>& zeilen, int ziel);

    /// Rolle der Listeneintraege, unter der die Dauer in ms liegt (S87);
    /// 0 / ungesetzt = noch nicht gelesen oder nicht lesbar.
    static constexpr int kDauerRolle = 0x0101;  // Qt::UserRole + 1

    /// `3:07` bzw. `1:02:03` — die Anzeige der Dauer rechts in der Zeile.
    [[nodiscard]] static QString dauerText(int ms);

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
    
    // S87: Dauer je Titel — nach und nach gelesen, damit eine lange Liste
    // die Oberflaeche nicht anhaelt (Pfad -> ms, 0 = nicht lesbar)
    void fuelleDauern();
    QTimer* m_dauerUhr = nullptr;
    QHash<QString, int> m_dauern;

    // State
    int m_currentPlayingIndex = -1;
    bool m_shuffleEnabled = false;
    bool m_loopEnabled = false;
};
