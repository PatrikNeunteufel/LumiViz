/**
 ****************************************************************************************
 * @file   PappTon.hpp
 * @brief  Composer-Tests, Stufe C5: ein Player und eine Audio-Engine aus Pappe
 *
 * @details
 * Kein Ton, keine Datei: beide kennen nur die Laenge ihrer „Dateien", schreiben
 * mit, was man ihnen sagt, und laufen nur weiter, wenn der Test es will.
 *
 * - `PappPlayer` steht fuer den Player der App (`IAudioPlayer`) — gegen ihn
 *   laufen die Fuehrung und die Sitzung.
 * - `PappEngine` steht fuer BASS (`IAudioEngine`) — mit ihr laeuft der ECHTE
 *   `AudioPlayer`, um die Naehte in ihm zu belegen.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen (nur die Composer-Tests benutzen sie).
 ****************************************************************************************
 */

#pragma once

#include "audio/IAudioEngine.hpp"
#include "audio/IAudioPlayer.hpp"

#include <QMap>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <atomic>
#include <vector>

namespace papp
{

/// Ein Player aus Pappe: kennt die Laenge seiner „Dateien", schreibt mit, was
/// er tun soll, und laeuft nur, wenn der Test `vor` ruft.
class PappPlayer : public IAudioPlayer
{
public:
    QMap<QString, int> laengen;  ///< was sich laden laesst, und wie lang es ist
    QStringList protokoll;       ///< die Aufrufe, in ihrer Reihenfolge

    /// Die Zeit weiterdrehen: ein spielender Song rueckt vor und haelt an seinem Ende.
    void vor(int ms)
    {
        if (m_state != PlaybackState::Playing) return;
        m_pos += ms;
        if (m_pos >= m_track.durationMs)
        {
            m_pos = m_track.durationMs;
            m_state = PlaybackState::Stopped;  // wie der echte Player ohne Playlist
        }
    }
    [[nodiscard]] int zahl(const QString& anfang) const
    {
        return static_cast<int>(std::count_if(protokoll.begin(), protokoll.end(),
                                              [&](const QString& z) { return z.startsWith(anfang); }));
    }

    bool load(const QString& filePath) override
    {
        protokoll << QStringLiteral("load ") + filePath;
        if (!laengen.contains(filePath)) return false;
        m_track = TrackInfo{};
        m_track.filePath = filePath;
        m_track.durationMs = laengen.value(filePath);
        m_hat = true;
        m_pos = 0;
        m_state = PlaybackState::Stopped;
        return true;
    }
    void play() override
    {
        protokoll << QStringLiteral("play");
        if (m_hat) m_state = PlaybackState::Playing;
    }
    void pause() override
    {
        protokoll << QStringLiteral("pause");
        if (m_state == PlaybackState::Playing) m_state = PlaybackState::Paused;
    }
    void togglePlayPause() override { protokoll << QStringLiteral("toggle"); }
    void stop() override
    {
        protokoll << QStringLiteral("stop");
        m_state = PlaybackState::Stopped;
        m_pos = 0;
    }
    bool next() override
    {
        protokoll << QStringLiteral("next");
        return false;
    }
    bool previous() override
    {
        protokoll << QStringLiteral("previous");
        return false;
    }
    bool playIndex(int) override { return false; }
    void seek(int positionMs) override
    {
        protokoll << QStringLiteral("seek %1").arg(positionMs);
        m_pos = std::clamp(positionMs, 0, m_track.durationMs);
    }
    void seekFraction(float) override {}
    void seekRelative(int) override {}
    float laut = 1.0f;   ///< was `volume()` nennt — der Test stellt es
    bool stumm = false;  ///< was `isMuted()` nennt
    [[nodiscard]] float volume() const override { return laut; }
    void setVolume(float) override { protokoll << QStringLiteral("volume"); }
    [[nodiscard]] bool isMuted() const override { return stumm; }
    void setMuted(bool) override {}
    void toggleMute() override { protokoll << QStringLiteral("mute"); }
    [[nodiscard]] PlaybackState state() const override { return m_state; }
    [[nodiscard]] bool isPlaying() const override { return m_state == PlaybackState::Playing; }
    [[nodiscard]] bool isPaused() const override { return m_state == PlaybackState::Paused; }
    [[nodiscard]] bool isStopped() const override { return m_state == PlaybackState::Stopped; }
    [[nodiscard]] int positionMs() const override { return m_pos; }
    [[nodiscard]] int durationMs() const override { return m_track.durationMs; }
    [[nodiscard]] float positionFraction() const override { return 0.0f; }
    [[nodiscard]] TrackInfo currentTrack() const override { return m_track; }
    [[nodiscard]] bool hasTrack() const override { return m_hat; }
    void setPlaylist(IPlaylist* playlist) override { m_playlist = playlist; }
    [[nodiscard]] IPlaylist* playlist() const override { return m_playlist; }
    [[nodiscard]] int playlistIndex() const override { return -1; }
    [[nodiscard]] RepeatMode repeatMode() const override { return RepeatMode::None; }
    void setRepeatMode(RepeatMode) override {}
    [[nodiscard]] bool shuffle() const override { return false; }
    void setShuffle(bool) override {}
    void update() override {}

private:
    TrackInfo m_track;
    bool m_hat = false;
    int m_pos = 0;
    PlaybackState m_state = PlaybackState::Stopped;
    IPlaylist* m_playlist = nullptr;
};

/// Eine Audio-Engine aus Pappe: jeder Stream ist ein Eintrag mit Stelle,
/// Lautstaerke und „spielt". `beende` laesst einen Song zu Ende gehen.
class PappEngine : public IAudioEngine
{
public:
    QMap<QString, int> laengen;  ///< was sich oeffnen laesst, und wie lang es ist
    QStringList geoeffnet;       ///< die Dateien, in der Reihenfolge von `createStream`

    struct Stream
    {
        QString datei;
        int dauer = 0;
        int pos = 0;
        float lautstaerke = 1.0f;
        bool spielt = false;
        bool haelt = false;
        bool frei = false;
    };
    [[nodiscard]] const Stream& stream(AudioStreamHandle h) const { return m_streams[h - 1]; }
    /// Der zuletzt geoeffnete Stream.
    [[nodiscard]] AudioStreamHandle letzter() const { return static_cast<AudioStreamHandle>(m_streams.size()); }
    /// Der Song ist aus: er spielt nicht mehr und steht an seinem Ende.
    void beende(AudioStreamHandle h)
    {
        m_streams[h - 1].spielt = false;
        m_streams[h - 1].pos = m_streams[h - 1].dauer;
    }

    bool initialize(int, int) override { return true; }
    void shutdown() override {}
    [[nodiscard]] bool isInitialized() const override { return true; }
    [[nodiscard]] std::vector<AudioDeviceInfo> getDevices() const override { return {}; }
    [[nodiscard]] int getCurrentDevice() const override { return -1; }
    bool setDevice(int) override { return true; }
    [[nodiscard]] int getSampleRate() const override { return 44100; }
    [[nodiscard]] AudioStreamHandle createStream(const QString& filePath) override
    {
        if (!laengen.contains(filePath)) return INVALID_STREAM;
        geoeffnet << filePath;
        Stream s;
        s.datei = filePath;
        s.dauer = laengen.value(filePath);
        m_streams.push_back(s);
        return letzter();
    }
    [[nodiscard]] AudioStreamHandle createLoopbackStream() override { return INVALID_STREAM; }
    void freeStream(AudioStreamHandle h) override
    {
        m_streams[h - 1].frei = true;
        m_streams[h - 1].spielt = false;
    }
    bool play(AudioStreamHandle h) override
    {
        m_streams[h - 1].spielt = true;
        m_streams[h - 1].haelt = false;
        return true;
    }
    bool pause(AudioStreamHandle h) override
    {
        m_streams[h - 1].spielt = false;
        m_streams[h - 1].haelt = true;
        return true;
    }
    bool stop(AudioStreamHandle h) override
    {
        m_streams[h - 1].spielt = false;
        m_streams[h - 1].haelt = false;
        m_streams[h - 1].pos = 0;
        return true;
    }
    [[nodiscard]] bool isPlaying(AudioStreamHandle h) const override { return m_streams[h - 1].spielt; }
    [[nodiscard]] bool isPaused(AudioStreamHandle h) const override { return m_streams[h - 1].haelt; }
    [[nodiscard]] int getPositionMs(AudioStreamHandle h) const override { return m_streams[h - 1].pos; }
    bool setPositionMs(AudioStreamHandle h, int positionMs) override
    {
        m_streams[h - 1].pos = positionMs;
        return true;
    }
    [[nodiscard]] int getDurationMs(AudioStreamHandle h) const override { return m_streams[h - 1].dauer; }
    [[nodiscard]] int probeDurationMs(const QString& filePath) override { return laengen.value(filePath, 0); }
    /// Eine gleichmaessige Huellkurve, halbe Hoehe — genug, um eine Wellenform zu sehen.
    [[nodiscard]] std::vector<float> probePeaks(const QString& filePath, int msProWert,
                                                const std::atomic<bool>* = nullptr) override
    {
        const int dauer = laengen.value(filePath, 0);
        return std::vector<float>(static_cast<std::size_t>(msProWert > 0 ? dauer / msProWert : 0), 0.5f);
    }
    [[nodiscard]] float getVolume(AudioStreamHandle h) const override { return m_streams[h - 1].lautstaerke; }
    bool setVolume(AudioStreamHandle h, float volume) override
    {
        m_streams[h - 1].lautstaerke = volume;
        return true;
    }
    bool getFFTData(AudioStreamHandle, float*, int) override { return false; }
    bool getWaveformData(AudioStreamHandle, float*, int) override { return false; }
    bool getChannelLevels(AudioStreamHandle, float&, float&) override { return false; }
    bool getMetadata(AudioStreamHandle, QString&, QString&, QString&) override { return false; }
    [[nodiscard]] QString getVersion() const override { return QStringLiteral("Pappe"); }
    [[nodiscard]] QString getLastError() const override { return QStringLiteral("not found"); }
    [[nodiscard]] int getLastErrorCode() const override { return 0; }

protected:
    std::vector<Stream> m_streams;
};

/// Die Engine aus Pappe mit einem Mixer (Naht N12, Stufe C5b): Quellen sind
/// Streams, die `haelt` tragen, solange sie angehalten sind. Die Zeit dreht
/// der Test mit `vor` weiter — nur wenn der Mixer spielt, und nur fuer
/// Quellen, die laufen. `PappEngine` selbst bleibt ohne Mixer.
class PappMischEngine : public PappEngine
{
public:
    QStringList protokoll;  ///< was an Quellen und Mixer geschah, in seiner Reihenfolge
    [[nodiscard]] int zahl(const QString& anfang) const
    {
        return static_cast<int>(std::count_if(protokoll.begin(), protokoll.end(),
                                              [&](const QString& z) { return z.startsWith(anfang); }));
    }
    AudioStreamHandle mixer = INVALID_STREAM;  ///< der zuletzt angelegte Mixer
    /// Die Quelle im Mixer, die diese Datei spielt; INVALID_STREAM = keine.
    [[nodiscard]] AudioStreamHandle quelle(const QString& datei) const
    {
        for (const AudioStreamHandle h : m_quellen)
            if (m_streams[h - 1].datei == datei) return h;
        return INVALID_STREAM;
    }
    [[nodiscard]] int quellenZahl() const { return static_cast<int>(m_quellen.size()); }
    /// Alle Quellen im Mixer, die diese Datei spielen — im Fenster einer
    /// Schleife mit Ueberlappung koennen es zwei sein (S99).
    [[nodiscard]] std::vector<AudioStreamHandle> quellenVon(const QString& datei) const
    {
        std::vector<AudioStreamHandle> alle;
        for (const AudioStreamHandle h : m_quellen)
            if (m_streams[h - 1].datei == datei) alle.push_back(h);
        return alle;
    }
    /// Die Zeit weiterdrehen: laufende Quellen ruecken vor und halten an ihrem Ende.
    void vor(int ms)
    {
        if (mixer == INVALID_STREAM || !m_streams[mixer - 1].spielt) return;
        for (const AudioStreamHandle h : m_quellen)
        {
            Stream& s = m_streams[h - 1];
            if (!s.haelt) s.pos = std::min(s.dauer, s.pos + ms);
        }
    }
    /// Eine Quelle laeuft der Uhr davon (oder bleibt zurueck).
    void verschiebe(AudioStreamHandle h, int ms) { m_streams[h - 1].pos += ms; }

    [[nodiscard]] AudioStreamHandle createMixer() override
    {
        Stream s;
        s.datei = QStringLiteral("<mixer>");
        m_streams.push_back(s);
        mixer = letzter();
        protokoll << QStringLiteral("mixer");
        return mixer;
    }
    [[nodiscard]] AudioStreamHandle mixerAddSource(AudioStreamHandle, const QString& filePath) override
    {
        protokoll << QStringLiteral("add ") + filePath;
        const AudioStreamHandle h = createStream(filePath);
        if (h == INVALID_STREAM) return h;
        m_streams[h - 1].haelt = true;  // kommt angehalten
        m_quellen.push_back(h);
        return h;
    }
    void mixerRemoveSource(AudioStreamHandle source) override
    {
        protokoll << QStringLiteral("remove ") + m_streams[source - 1].datei;
        m_quellen.erase(std::remove(m_quellen.begin(), m_quellen.end(), source), m_quellen.end());
        m_streams[source - 1].frei = true;
    }
    [[nodiscard]] int getSourcePositionMs(AudioStreamHandle source) const override
    {
        return m_streams[source - 1].pos;
    }
    bool setSourcePositionMs(AudioStreamHandle source, int positionMs) override
    {
        protokoll << QStringLiteral("seek %1 %2").arg(m_streams[source - 1].datei).arg(positionMs);
        m_streams[source - 1].pos = positionMs;
        return true;
    }
    bool setSourcePaused(AudioStreamHandle source, bool paused) override
    {
        protokoll << (paused ? QStringLiteral("hold ") : QStringLiteral("run ")) + m_streams[source - 1].datei;
        m_streams[source - 1].haelt = paused;
        return true;
    }
    [[nodiscard]] bool isSourcePlaying(AudioStreamHandle source) const override
    {
        const Stream& s = m_streams[source - 1];
        return !s.haelt && s.pos < s.dauer;
    }
    void freeStream(AudioStreamHandle h) override
    {
        if (h == mixer) protokoll << QStringLiteral("free mixer");
        PappEngine::freeStream(h);
    }

private:
    std::vector<AudioStreamHandle> m_quellen;
};

}  // namespace papp
