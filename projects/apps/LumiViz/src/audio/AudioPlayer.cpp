/**
 ****************************************************************************************
 * @file   AudioPlayer.cpp
 * @brief  Audio Player Service Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "pch.h"
#include "audio/AudioPlayer.hpp"
#include "audio/IAudioEngine.hpp"
#include "audio/IPlaylist.hpp"
#include "services/IEventBus.hpp"

#include <BasicLogger.h>

#include <QFileInfo>
#include <QSettings>
#include <algorithm>
#include <random>

// =============================================================================
// Private Implementation
// =============================================================================

struct AudioPlayer::Impl
{
    IAudioEngine& engine;
    IEventBus& eventBus;
    
    // Current state
    PlaybackState state = PlaybackState::Stopped;
    AudioStreamHandle currentStream = INVALID_STREAM;
    TrackInfo currentTrack;
    
    // Volume
    float volume = 1.0f;
    float volumeBeforeMute = 1.0f;
    bool muted = false;

    // Tempo (S91): 1.0 = wie aufgenommen
    float tempo = 1.0f;

    // Composer-Naht N17: ein zweiter Faktor neben der Lautstaerke (Vorgabe 1 =
    // wie ohne ihn). An die Engine geht ueberall `laut()`.
    float pegel = 1.0f;
    [[nodiscard]] float laut() const { return muted ? 0.0f : volume * pegel; }

    // Playlist integration
    IPlaylist* playlist = nullptr;
    int playlistIndex = -1;
    
    // Playback modes
    IAudioPlayer::RepeatMode repeatMode = IAudioPlayer::RepeatMode::None;
    bool shuffle = false;
    
    // Position update throttling
    int lastPositionMs = -1;
    int positionUpdateCounter = 0;
    static constexpr int POSITION_UPDATE_INTERVAL = 5;  // Update every 5th call
    
    explicit Impl(IAudioEngine& eng, IEventBus& bus)
        : engine(eng), eventBus(bus) {}
};

// =============================================================================
// Construction / Destruction
// =============================================================================

AudioPlayer::AudioPlayer(IAudioEngine& engine,
                         IEventBus& eventBus,
                         QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>(engine, eventBus))
{
    // S87 (Befund Patrik): Shuffle, Loop und Repeat waren nach einem Neustart
    // weg — sie gehoeren zum Zustand des Players und bleiben jetzt erhalten.
    // Die Panels lesen den Stand beim Aktivieren vom Player.
    const QSettings settings;
    m_impl->shuffle = settings.value(QStringLiteral("player/shuffle"), false).toBool();
    const int modus = settings.value(QStringLiteral("player/repeatMode"), 0).toInt();
    m_impl->repeatMode = modus == 1   ? RepeatMode::One
                         : modus == 2 ? RepeatMode::All
                                      : RepeatMode::None;
    BasicLogger::logDebug("AudioPlayer created");
}

AudioPlayer::~AudioPlayer()
{
    stop();
    BasicLogger::logDebug("AudioPlayer destroyed");
}

// =============================================================================
// Track Info (Composer-Naht N14, docs/ui/Composer_Konzept.md §12.2)
// =============================================================================

TrackInfo leseTrackInfo(IAudioEngine& engine, AudioStreamHandle stream, const QString& filePath)
{
    TrackInfo track;
    track.filePath = filePath;
    track.durationMs = engine.getDurationMs(stream);

    // Try to get metadata
    QString title, artist, album;
    if (engine.getMetadata(stream, title, artist, album))
    {
        track.title = title;
        track.artist = artist;
        track.album = album;
    }
    else
    {
        // Use filename as title
        QFileInfo info(filePath);
        track.title = info.completeBaseName();
    }
    // Weitere Tags fuer die Text-Vorlagen (S86)
    engine.getMetadataExtra(stream, track.year, track.genre, track.trackNumber);
    // Eingebettete Songtexte fuer den Lyrics-Knoten (S86)
    engine.getLyrics(stream, track.lyricsSynced, track.lyricsPlain);
    return track;
}

// =============================================================================
// Playback Control
// =============================================================================

bool AudioPlayer::load(const QString& filePath)
{
    // Free existing stream
    if (m_impl->currentStream != INVALID_STREAM)
    {
        m_impl->engine.freeStream(m_impl->currentStream);
        m_impl->currentStream = INVALID_STREAM;
    }
    
    setState(PlaybackState::Loading);
    
    // Create new stream
    m_impl->currentStream = m_impl->engine.createStream(filePath);
    
    if (m_impl->currentStream == INVALID_STREAM)
    {
        setState(PlaybackState::Error);
        emit error(m_impl->engine.getLastError());
        return false;
    }
    
    // Update track info (Composer-Naht N14: an einer Stelle, auch fuer den Composer)
    m_impl->currentTrack = leseTrackInfo(m_impl->engine, m_impl->currentStream, filePath);

    // Apply current volume
    m_impl->engine.setVolume(m_impl->currentStream, m_impl->laut());
    // S91: ein neuer Stream laeuft mit der Rate seiner Datei — das Tempo des
    // Players muss ihm eigens gesetzt werden, sonst stimmen beide nicht ueberein
    if (m_impl->tempo != 1.0f) m_impl->engine.setTempo(m_impl->currentStream, m_impl->tempo);

    setState(PlaybackState::Stopped);
    publishTrackChanged();
    
    BasicLogger::logDebug("Loaded track: " + filePath.toStdString());
    return true;
}

void AudioPlayer::play()
{
    if (m_impl->currentStream == INVALID_STREAM)
    {
        // Try to play from playlist
        if (m_impl->playlist && !m_impl->playlist->isEmpty())
        {
            playIndex(m_impl->playlist->currentIndex() >= 0 ? 
                      m_impl->playlist->currentIndex() : 0);
        }
        return;
    }
    
    if (m_impl->engine.play(m_impl->currentStream))
    {
        setState(PlaybackState::Playing);
    }
}

void AudioPlayer::pause()
{
    if (m_impl->currentStream != INVALID_STREAM && 
        m_impl->state == PlaybackState::Playing)
    {
        if (m_impl->engine.pause(m_impl->currentStream))
        {
            setState(PlaybackState::Paused);
        }
    }
}

void AudioPlayer::togglePlayPause()
{
    switch (m_impl->state)
    {
        case PlaybackState::Playing:
            pause();
            break;
        case PlaybackState::Paused:
        case PlaybackState::Stopped:
            play();
            break;
        default:
            break;
    }
}

void AudioPlayer::stop()
{
    if (m_impl->currentStream != INVALID_STREAM)
    {
        m_impl->engine.stop(m_impl->currentStream);
        setState(PlaybackState::Stopped);
        m_impl->lastPositionMs = -1;
    }
}

// =============================================================================
// Playlist Navigation
// =============================================================================

bool AudioPlayer::next()
{
    if (!m_impl->playlist || m_impl->playlist->isEmpty())
    {
        return false;
    }
    
    int newIndex = -1;
    
    if (m_impl->shuffle)
    {
        // Shuffle mode: Select random track (different from current if possible)
        int trackCount = m_impl->playlist->count();
        if (trackCount == 1)
        {
            newIndex = 0;
        }
        else
        {
            // Random index different from current
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(0, trackCount - 1);
            
            do
            {
                newIndex = dist(gen);
            } while (newIndex == m_impl->playlistIndex && trackCount > 1);
        }
    }
    else
    {
        // Normal mode: Next track with optional wrap
        bool wrap = (m_impl->repeatMode == RepeatMode::All);
        newIndex = m_impl->playlist->next(wrap);
    }
    
    if (newIndex >= 0)
    {
        return playIndex(newIndex);
    }
    
    // No next track
    if (m_impl->repeatMode == RepeatMode::One)
    {
        seek(0);
        play();
        return true;
    }
    
    stop();
    return false;
}

bool AudioPlayer::previous()
{
    if (!m_impl->playlist)
    {
        return false;
    }
    
    // If more than 3 seconds into track, restart current
    if (positionMs() > 3000)
    {
        seek(0);
        return true;
    }
    
    bool wrap = (m_impl->repeatMode == RepeatMode::All);
    int newIndex = m_impl->playlist->previous(wrap);
    
    if (newIndex >= 0)
    {
        return playIndex(newIndex);
    }
    
    return false;
}

bool AudioPlayer::playIndex(int index)
{
    if (!m_impl->playlist || index < 0 || index >= m_impl->playlist->count())
    {
        return false;
    }
    
    QString filePath = m_impl->playlist->filePathAt(index);
    if (filePath.isEmpty())
    {
        return false;
    }
    
    if (load(filePath))
    {
        m_impl->playlistIndex = index;
        m_impl->playlist->setCurrentIndex(index);
        play();
        return true;
    }
    
    return false;
}

// =============================================================================
// Seeking
// =============================================================================

void AudioPlayer::seek(int positionMs)
{
    if (m_impl->currentStream != INVALID_STREAM)
    {
        m_impl->engine.setPositionMs(m_impl->currentStream, positionMs);
        publishPositionEvent();
    }
}

void AudioPlayer::seekFraction(float fraction)
{
    if (m_impl->currentStream != INVALID_STREAM)
    {
        fraction = std::clamp(fraction, 0.0f, 1.0f);
        int posMs = static_cast<int>(fraction * durationMs());
        seek(posMs);
    }
}

void AudioPlayer::seekRelative(int deltaMs)
{
    seek(positionMs() + deltaMs);
}

// =============================================================================
// Volume Control
// =============================================================================

float AudioPlayer::volume() const
{
    return m_impl->volume;
}

void AudioPlayer::setVolume(float volume)
{
    m_impl->volume = std::clamp(volume, 0.0f, 1.0f);
    
    if (m_impl->currentStream != INVALID_STREAM && !m_impl->muted)
    {
        m_impl->engine.setVolume(m_impl->currentStream, m_impl->laut());
    }

    // Publish event
    VolumeChangedEvent event;
    event.volume = m_impl->volume;
    event.muted = m_impl->muted;
    m_impl->eventBus.publish(event);
    
    emit volumeChanged(m_impl->volume);
}

bool AudioPlayer::isMuted() const
{
    return m_impl->muted;
}

void AudioPlayer::setMuted(bool muted)
{
    if (m_impl->muted == muted) return;
    
    m_impl->muted = muted;
    
    if (m_impl->currentStream != INVALID_STREAM)
    {
        m_impl->engine.setVolume(m_impl->currentStream, m_impl->laut());
    }

    VolumeChangedEvent event;
    event.volume = m_impl->volume;
    event.muted = muted;
    m_impl->eventBus.publish(event);
}

void AudioPlayer::toggleMute()
{
    setMuted(!m_impl->muted);
}

// =============================================================================
// Tempo
// =============================================================================

float AudioPlayer::tempo() const
{
    return m_impl->tempo;
}

void AudioPlayer::setTempo(float tempo)
{
    tempo = std::clamp(tempo, 0.25f, 2.0f);
    if (m_impl->tempo == tempo) return;

    m_impl->tempo = tempo;
    if (m_impl->currentStream != INVALID_STREAM)
    {
        m_impl->engine.setTempo(m_impl->currentStream, tempo);
    }

    PlaybackTempoEvent event;
    event.tempo = tempo;
    m_impl->eventBus.publish(event);
}

// =============================================================================
// Pegel (Composer-Naht N17)
// =============================================================================

float AudioPlayer::pegel() const
{
    return m_impl->pegel;
}

void AudioPlayer::setPegel(float pegel)
{
    pegel = std::clamp(pegel, 0.0f, 1.0f);
    if (m_impl->pegel == pegel) return;

    m_impl->pegel = pegel;
    // Kein Ereignis: die Lautstaerke des Players — der Regler — bleibt, wie sie ist.
    if (m_impl->currentStream != INVALID_STREAM && !m_impl->muted)
    {
        m_impl->engine.setVolume(m_impl->currentStream, m_impl->laut());
    }
}

// =============================================================================
// State Queries
// =============================================================================

PlaybackState AudioPlayer::state() const
{
    return m_impl->state;
}

bool AudioPlayer::isPlaying() const
{
    return m_impl->state == PlaybackState::Playing;
}

bool AudioPlayer::isPaused() const
{
    return m_impl->state == PlaybackState::Paused;
}

bool AudioPlayer::isStopped() const
{
    return m_impl->state == PlaybackState::Stopped;
}

// =============================================================================
// Position / Duration
// =============================================================================

int AudioPlayer::positionMs() const
{
    if (m_impl->currentStream == INVALID_STREAM) return 0;
    return m_impl->engine.getPositionMs(m_impl->currentStream);
}

int AudioPlayer::durationMs() const
{
    return m_impl->currentTrack.durationMs;
}

float AudioPlayer::positionFraction() const
{
    if (m_impl->currentTrack.durationMs <= 0) return 0.0f;
    return static_cast<float>(positionMs()) / m_impl->currentTrack.durationMs;
}

// =============================================================================
// Track Info
// =============================================================================

TrackInfo AudioPlayer::currentTrack() const
{
    return m_impl->currentTrack;
}

bool AudioPlayer::hasTrack() const
{
    return m_impl->currentStream != INVALID_STREAM;
}

// =============================================================================
// Playlist Integration
// =============================================================================

void AudioPlayer::setPlaylist(IPlaylist* playlist)
{
    m_impl->playlist = playlist;
    m_impl->playlistIndex = playlist ? playlist->currentIndex() : -1;
}

IPlaylist* AudioPlayer::playlist() const
{
    return m_impl->playlist;
}

int AudioPlayer::playlistIndex() const
{
    return m_impl->playlistIndex;
}

// =============================================================================
// Repeat / Shuffle
// =============================================================================

AudioPlayer::RepeatMode AudioPlayer::repeatMode() const
{
    return m_impl->repeatMode;
}

void AudioPlayer::setRepeatMode(RepeatMode mode)
{
    if (m_impl->repeatMode == mode) return;
    
    m_impl->repeatMode = mode;
    QSettings().setValue(QStringLiteral("player/repeatMode"), static_cast<int>(mode));
    publishPlaybackModeChanged();
}

bool AudioPlayer::shuffle() const
{
    return m_impl->shuffle;
}

void AudioPlayer::setShuffle(bool enabled)
{
    if (m_impl->shuffle == enabled) return;
    
    m_impl->shuffle = enabled;
    QSettings().setValue(QStringLiteral("player/shuffle"), enabled);
    publishPlaybackModeChanged();
}

// =============================================================================
// Update
// =============================================================================

void AudioPlayer::update()
{
    if (m_impl->currentStream == INVALID_STREAM) return;
    
    // Check if playback ended
    if (m_impl->state == PlaybackState::Playing &&
        !m_impl->engine.isPlaying(m_impl->currentStream))
    {
        handleTrackEnd();
        return;
    }
    
    // Throttle position updates
    m_impl->positionUpdateCounter++;
    if (m_impl->positionUpdateCounter >= Impl::POSITION_UPDATE_INTERVAL)
    {
        m_impl->positionUpdateCounter = 0;
        
        int currentPos = positionMs();
        if (currentPos != m_impl->lastPositionMs)
        {
            m_impl->lastPositionMs = currentPos;
            publishPositionEvent();
        }
    }
}

// =============================================================================
// Additional Methods
// =============================================================================

IAudioEngine& AudioPlayer::engine() const
{
    return m_impl->engine;
}

AudioStreamHandle AudioPlayer::currentStream() const
{
    return m_impl->currentStream;
}

// =============================================================================
// Private Methods
// =============================================================================

void AudioPlayer::setState(PlaybackState newState)
{
    if (m_impl->state == newState) return;
    
    PlaybackState oldState = m_impl->state;
    m_impl->state = newState;
    
    // Publish event
    PlaybackStateEvent event;
    event.state = newState;
    event.previousState = oldState;
    m_impl->eventBus.publish(event);
    
    emit stateChanged(newState);
}

void AudioPlayer::publishTrackChanged()
{
    TrackChangedEvent event;
    event.track = m_impl->currentTrack;
    event.playlistIndex = m_impl->playlistIndex;
    m_impl->eventBus.publish(event);
    
    emit trackChanged(m_impl->currentTrack);
}

void AudioPlayer::publishPositionEvent()
{
    PlaybackPositionEvent event;
    event.positionMs = positionMs();
    event.durationMs = durationMs();
    event.progress = positionFraction();
    m_impl->eventBus.publish(event);
    
    emit positionChanged(event.positionMs);
}

void AudioPlayer::handleTrackEnd()
{
    BasicLogger::logDebug("Track ended");
    
    switch (m_impl->repeatMode)
    {
        case RepeatMode::One:
            // Repeat current track
            seek(0);
            play();
            break;
            
        case RepeatMode::All:
        case RepeatMode::None:
            // Try to play next (will use shuffle if enabled)
            if (!next())
            {
                setState(PlaybackState::Stopped);
            }
            break;
    }
}

void AudioPlayer::publishPlaybackModeChanged()
{
    PlaybackModeChangedEvent event;
    event.shuffle = m_impl->shuffle;
    event.repeatMode = static_cast<int>(m_impl->repeatMode);  // 0=None, 1=One, 2=All
    m_impl->eventBus.publish(event);
}
