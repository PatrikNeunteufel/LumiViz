/**
 ****************************************************************************************
 * @file   PlayerPanel.cpp
 * @brief  PlayerPanel implementation with full audio integration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 ****************************************************************************************
 */

#include "UI/panels/PlayerPanel.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "audio/IAudioPlayer.hpp"
#include "audio/AudioEvents.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QStyle>

#include <BasicLogger.h>

// =============================================================================
// Construction
// =============================================================================

PlayerPanel::PlayerPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("player"), tr("Player"), parent)
{
    setupUI();
    setupConnections();
    
    // Initial state
    updatePlayButton(false);
    updateMuteButton(false);
}

// =============================================================================
// IPanel Implementation
// =============================================================================

int PlayerPanel::preferredArea() const
{
    return Qt::BottomDockWidgetArea;
}

// =============================================================================
// Lifecycle
// =============================================================================

void PlayerPanel::onActivate()
{
    subscribeToEvents();
    
    // Sync with current audio state
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        updatePlayButton(player->isPlaying());
        onVolumeChangedEvent(player->volume(), player->isMuted());
        
        if (player->hasTrack())
        {
            auto track = player->currentTrack();
            onTrackChanged(track.title, track.artist, track.durationMs);
            onPlaybackPositionChanged(player->positionMs(), track.durationMs);
        }
    }
}

void PlayerPanel::onDeactivate()
{
    unsubscribeFromEvents();
}

// =============================================================================
// Event Subscription
// =============================================================================

void PlayerPanel::subscribeToEvents()
{
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        BasicLogger::logWarning("PlayerPanel: EventBus not available");
        return;
    }
    
    // Track changed
    int id1 = eventBus->subscribe<TrackChangedEvent>(
        [this](const TrackChangedEvent& e) {
            onTrackChanged(e.track.title, e.track.artist, e.track.durationMs);
        });
    m_subscriptionIds.push_back(id1);
    
    // Playback state changed
    int id2 = eventBus->subscribe<PlaybackStateEvent>(
        [this](const PlaybackStateEvent& e) {
            bool isPlaying = (e.state == PlaybackState::Playing);
            bool isPaused = (e.state == PlaybackState::Paused);
            onPlaybackStateChanged(isPlaying, isPaused);
        });
    m_subscriptionIds.push_back(id2);
    
    // Position changed
    int id3 = eventBus->subscribe<PlaybackPositionEvent>(
        [this](const PlaybackPositionEvent& e) {
            onPlaybackPositionChanged(e.positionMs, e.durationMs);
        });
    m_subscriptionIds.push_back(id3);
    
    // Volume changed
    int id4 = eventBus->subscribe<VolumeChangedEvent>(
        [this](const VolumeChangedEvent& e) {
            onVolumeChangedEvent(e.volume, e.muted);
        });
    m_subscriptionIds.push_back(id4);
    
    // Playback mode changed (shuffle/repeat)
    int id5 = eventBus->subscribe<PlaybackModeChangedEvent>(
        [this](const PlaybackModeChangedEvent& e) {
            // repeatMode: 0=None, 1=One (single track), 2=All (playlist)
            bool loopOne = (e.repeatMode == 1);
            m_loopEnabled = loopOne;
            updateLoopButton(loopOne);
        });
    m_subscriptionIds.push_back(id5);
    
    BasicLogger::logDebug("PlayerPanel: Subscribed to audio events");
}

void PlayerPanel::unsubscribeFromEvents()
{
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        return;
    }
    
    for (int id : m_subscriptionIds)
    {
        eventBus->unsubscribe(id);
    }
    m_subscriptionIds.clear();
    
    BasicLogger::logDebug("PlayerPanel: Unsubscribed from audio events");
}

// =============================================================================
// Button Handlers
// =============================================================================

void PlayerPanel::onPlayClicked()
{
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        player->togglePlayPause();
    }
}

void PlayerPanel::onStopClicked()
{
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        player->stop();
    }
}

void PlayerPanel::onPrevClicked()
{
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        player->previous();
    }
}

void PlayerPanel::onNextClicked()
{
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        player->next();
    }
}

void PlayerPanel::onMuteClicked()
{
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        player->toggleMute();
    }
}

// =============================================================================
// Slider Handlers
// =============================================================================

void PlayerPanel::onVolumeChanged(int value)
{
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        float volume = static_cast<float>(value) / 100.0f;
        player->setVolume(volume);
    }
}

void PlayerPanel::onProgressPressed()
{
    m_isSeeking = true;
}

void PlayerPanel::onProgressReleased()
{
    m_isSeeking = false;
    
    // Seek to final position
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        float fraction = static_cast<float>(m_pProgressSlider->value()) / 1000.0f;
        player->seekFraction(fraction);
    }
}

void PlayerPanel::onProgressMoved(int value)
{
    if (m_isSeeking && m_currentDurationMs > 0)
    {
        // Update time label while seeking (preview)
        int posMs = static_cast<int>((static_cast<float>(value) / 1000.0f) * 
                                      static_cast<float>(m_currentDurationMs));
        m_pTimeLabel->setText(formatTime(posMs) + " / " + formatTime(m_currentDurationMs));
    }
}

// =============================================================================
// Event Handlers
// =============================================================================

void PlayerPanel::onTrackChanged(const QString& title, const QString& artist, int durationMs)
{
    m_currentDurationMs = durationMs;
    
    // Update track info
    QString displayTitle = title.isEmpty() ? tr("Unknown Title") : title;
    m_pTrackLabel->setText(displayTitle);
    
    QString displayArtist = artist.isEmpty() ? tr("Unknown Artist") : artist;
    m_pArtistLabel->setText(displayArtist);
    
    // Reset progress
    m_pProgressSlider->setValue(0);
    m_pTimeLabel->setText(formatTime(0) + " / " + formatTime(durationMs));
    
    BasicLogger::logInfo("PlayerPanel: Track changed - " + displayTitle.toStdString());
}

void PlayerPanel::onPlaybackStateChanged(bool isPlaying, bool isPaused)
{
    Q_UNUSED(isPaused);
    updatePlayButton(isPlaying);
}

void PlayerPanel::onPlaybackPositionChanged(int positionMs, int durationMs)
{
    // Don't update while user is seeking
    if (m_isSeeking)
    {
        return;
    }
    
    m_currentDurationMs = durationMs;
    
    // Update progress slider (0-1000 range for precision)
    if (durationMs > 0)
    {
        int sliderValue = static_cast<int>((static_cast<float>(positionMs) / 
                                            static_cast<float>(durationMs)) * 1000.0f);
        m_pProgressSlider->setValue(sliderValue);
    }
    
    // Update time label
    m_pTimeLabel->setText(formatTime(positionMs) + " / " + formatTime(durationMs));
}

void PlayerPanel::onVolumeChangedEvent(float volume, bool muted)
{
    // Update volume slider without triggering signal
    m_pVolumeSlider->blockSignals(true);
    m_pVolumeSlider->setValue(static_cast<int>(volume * 100.0f));
    m_pVolumeSlider->blockSignals(false);
    
    updateMuteButton(muted);
}

// =============================================================================
// Helpers
// =============================================================================

QString PlayerPanel::formatTime(int ms)
{
    int totalSeconds = ms / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

void PlayerPanel::updatePlayButton(bool isPlaying)
{
    if (isPlaying)
    {
        m_pPlayButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        m_pPlayButton->setToolTip(tr("Pause"));
    }
    else
    {
        m_pPlayButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_pPlayButton->setToolTip(tr("Play"));
    }
}

void PlayerPanel::updateMuteButton(bool muted)
{
    if (muted)
    {
        m_pMuteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        m_pMuteButton->setToolTip(tr("Unmute"));
    }
    else
    {
        m_pMuteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        m_pMuteButton->setToolTip(tr("Mute"));
    }
}

void PlayerPanel::onLoopClicked()
{
    auto* player = services().tryResolve<IAudioPlayer>();
    if (player == nullptr)
    {
        return;
    }
    
    m_loopEnabled = !m_loopEnabled;
    player->setRepeatMode(m_loopEnabled ? IAudioPlayer::RepeatMode::One 
                                        : IAudioPlayer::RepeatMode::None);
    updateLoopButton(m_loopEnabled);
    
    BasicLogger::logDebug("PlayerPanel: Single-track loop " + 
                          std::string(m_loopEnabled ? "enabled" : "disabled"));
}

void PlayerPanel::updateLoopButton(bool enabled)
{
    m_pLoopButton->setChecked(enabled);
    
    if (enabled)
    {
        m_pLoopButton->setStyleSheet(
            "QPushButton { background-color: #4a6fa5; border-radius: 4px; }");
        m_pLoopButton->setToolTip(tr("Repeat current track (ON)"));
    }
    else
    {
        m_pLoopButton->setStyleSheet(QString());
        m_pLoopButton->setToolTip(tr("Repeat current track"));
    }
}

// =============================================================================
// UI Setup
// =============================================================================

void PlayerPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(4);

    // Track info section
    m_pTrackLabel = new QLabel(tr("No track loaded"), this);
    m_pTrackLabel->setAlignment(Qt::AlignCenter);
    m_pTrackLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    mainLayout->addWidget(m_pTrackLabel);
    
    m_pArtistLabel = new QLabel(QString(), this);
    m_pArtistLabel->setAlignment(Qt::AlignCenter);
    m_pArtistLabel->setStyleSheet("color: gray; font-size: 10px;");
    mainLayout->addWidget(m_pArtistLabel);

    // Progress section
    m_pProgressSlider = new QSlider(Qt::Horizontal, this);
    m_pProgressSlider->setRange(0, 1000);  // 0.1% precision
    m_pProgressSlider->setValue(0);
    mainLayout->addWidget(m_pProgressSlider);

    // Time label
    m_pTimeLabel = new QLabel(QStringLiteral("00:00 / 00:00"), this);
    m_pTimeLabel->setAlignment(Qt::AlignCenter);
    m_pTimeLabel->setStyleSheet("font-size: 10px;");
    mainLayout->addWidget(m_pTimeLabel);

    // Control buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_pPrevButton = new QPushButton(this);
    m_pPrevButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    m_pPrevButton->setToolTip(tr("Previous"));
    m_pPrevButton->setFixedSize(32, 32);
    buttonLayout->addWidget(m_pPrevButton);

    m_pPlayButton = new QPushButton(this);
    m_pPlayButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_pPlayButton->setToolTip(tr("Play"));
    m_pPlayButton->setFixedSize(40, 40);
    buttonLayout->addWidget(m_pPlayButton);

    m_pStopButton = new QPushButton(this);
    m_pStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_pStopButton->setToolTip(tr("Stop"));
    m_pStopButton->setFixedSize(32, 32);
    buttonLayout->addWidget(m_pStopButton);

    m_pNextButton = new QPushButton(this);
    m_pNextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    m_pNextButton->setToolTip(tr("Next"));
    m_pNextButton->setFixedSize(32, 32);
    buttonLayout->addWidget(m_pNextButton);

    // Loop button (single track repeat)
    m_pLoopButton = new QPushButton(this);
    m_pLoopButton->setText(QStringLiteral("🔂"));  // Unicode single repeat symbol
    m_pLoopButton->setCheckable(true);
    m_pLoopButton->setToolTip(tr("Repeat current track"));
    m_pLoopButton->setFixedSize(32, 32);
    buttonLayout->addWidget(m_pLoopButton);

    buttonLayout->addSpacing(20);

    // Volume section
    m_pMuteButton = new QPushButton(this);
    m_pMuteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    m_pMuteButton->setToolTip(tr("Mute"));
    m_pMuteButton->setFixedSize(24, 24);
    m_pMuteButton->setFlat(true);
    buttonLayout->addWidget(m_pMuteButton);

    m_pVolumeSlider = new QSlider(Qt::Horizontal, this);
    m_pVolumeSlider->setRange(0, 100);
    m_pVolumeSlider->setValue(80);
    m_pVolumeSlider->setMaximumWidth(100);
    m_pVolumeSlider->setToolTip(tr("Volume"));
    buttonLayout->addWidget(m_pVolumeSlider);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void PlayerPanel::setupConnections()
{
    // Buttons
    connect(m_pPlayButton, &QPushButton::clicked, this, &PlayerPanel::onPlayClicked);
    connect(m_pStopButton, &QPushButton::clicked, this, &PlayerPanel::onStopClicked);
    connect(m_pPrevButton, &QPushButton::clicked, this, &PlayerPanel::onPrevClicked);
    connect(m_pNextButton, &QPushButton::clicked, this, &PlayerPanel::onNextClicked);
    connect(m_pMuteButton, &QPushButton::clicked, this, &PlayerPanel::onMuteClicked);
    connect(m_pLoopButton, &QPushButton::clicked, this, &PlayerPanel::onLoopClicked);
    
    // Volume slider
    connect(m_pVolumeSlider, &QSlider::valueChanged, this, &PlayerPanel::onVolumeChanged);
    
    // Progress slider - track drag start/end for seeking
    connect(m_pProgressSlider, &QSlider::sliderPressed, this, &PlayerPanel::onProgressPressed);
    connect(m_pProgressSlider, &QSlider::sliderReleased, this, &PlayerPanel::onProgressReleased);
    connect(m_pProgressSlider, &QSlider::sliderMoved, this, &PlayerPanel::onProgressMoved);
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================
// NOTE: Registration is now handled centrally in PanelAutoReg.cpp
// to avoid linker issues with static libraries (dead code elimination).
// The REGISTER_PANEL macro is no longer used here.
