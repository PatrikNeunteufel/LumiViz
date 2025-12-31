/**
 ****************************************************************************************
 * @file   PlayerPanel.hpp
 * @brief  Audio player controls panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * ## PlayerPanel
 *
 * Provides audio playback controls:
 * - Play/Pause/Stop/Prev/Next buttons
 * - Volume slider with mute
 * - Progress slider with seek
 * - Track info display (title, artist, time)
 *
 * ## Integration
 *
 * ```
 * ┌─────────────────┐      Events       ┌──────────────┐
 * │   AudioPlayer   │ ─────────────────►│ PlayerPanel  │
 * │                 │◄───────────────── │              │
 * └─────────────────┘   User Actions    └──────────────┘
 * ```
 *
 * ## Events
 *
 * Subscribes to:
 * - TrackChangedEvent
 * - PlaybackStateEvent
 * - PlaybackPositionEvent
 * - VolumeChangedEvent
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

// Forward declarations
class QLabel;
class QPushButton;
class QSlider;
class IAudioPlayer;

/**
 * @class PlayerPanel
 * @brief Panel for audio playback controls
 */
class PlayerPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit PlayerPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~PlayerPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

protected:
    void onActivate() override;
    void onDeactivate() override;

private Q_SLOTS:
    // Button handlers
    void onPlayClicked();
    void onStopClicked();
    void onPrevClicked();
    void onNextClicked();
    void onMuteClicked();
    void onLoopClicked();
    
    // Slider handlers
    void onVolumeChanged(int value);
    void onProgressPressed();
    void onProgressReleased();
    void onProgressMoved(int value);

private:
    void setupUI();
    void setupConnections();
    void subscribeToEvents();
    void unsubscribeFromEvents();
    
    // Event handlers
    void onTrackChanged(const QString& title, const QString& artist, int durationMs);
    void onPlaybackStateChanged(bool isPlaying, bool isPaused);
    void onPlaybackPositionChanged(int positionMs, int durationMs);
    void onVolumeChangedEvent(float volume, bool muted);
    
    // Helper
    static QString formatTime(int ms);
    void updatePlayButton(bool isPlaying);
    void updateMuteButton(bool muted);
    void updateLoopButton(bool enabled);

    // UI Elements
    QLabel* m_pTrackLabel = nullptr;
    QLabel* m_pArtistLabel = nullptr;
    QLabel* m_pTimeLabel = nullptr;
    QPushButton* m_pPlayButton = nullptr;
    QPushButton* m_pStopButton = nullptr;
    QPushButton* m_pPrevButton = nullptr;
    QPushButton* m_pNextButton = nullptr;
    QPushButton* m_pMuteButton = nullptr;
    QPushButton* m_pLoopButton = nullptr;
    QSlider* m_pVolumeSlider = nullptr;
    QSlider* m_pProgressSlider = nullptr;
    
    // State
    bool m_isSeeking = false;           ///< True while user is dragging progress
    bool m_loopEnabled = false;         ///< True when single-track loop is active
    int m_currentDurationMs = 0;        ///< Current track duration
    std::vector<int> m_subscriptionIds; ///< Event subscription IDs for cleanup
};
