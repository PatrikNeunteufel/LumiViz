/**
 * @file Tonfuehrung.cpp
 * @brief Composer: die Audio-Kanaele im Mixer (Stufe C5b)
 */

#include "composer/Tonfuehrung.hpp"

#include "audio/AudioPlayer.hpp"  // leseTrackInfo (Naht N14)
#include "audio/IAudioPlayer.hpp"

#include <QFileInfo>

#include <BasicLogger.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace lumi::composer
{

bool Tonfuehrung::uebernimm(IAudioEngine& engine, IAudioPlayer& player)
{
    if (fuehrt()) return true;
    const AudioStreamHandle mixer = engine.createMixer();
    if (mixer == INVALID_STREAM) return false;  // die Engine hat keinen Mixer
    m_engine = &engine;
    m_player = &player;
    m_mixer = mixer;
    // der Player schweigt, und die Playlist ruht: ohne sie spielt am Titelende
    // nichts von selbst weiter
    m_playerSpielte = player.isPlaying();
    if (m_playerSpielte) player.pause();
    m_playlist = player.playlist();
    player.setPlaylist(nullptr);
    m_quellen.clear();
    m_laut = -1.0f;
    m_laufend = {};
    m_laufendDauerMs = 0;
    m_liefZuletzt = false;
    m_nachgesetzt = 0;
    return true;
}

void Tonfuehrung::gibFrei(bool playerWeiter)
{
    if (!fuehrt()) return;
    for (const Quelle& q : m_quellen)
        if (q.handle != INVALID_STREAM) m_engine->mixerRemoveSource(q.handle);
    m_quellen.clear();
    m_engine->freeStream(m_mixer);
    m_player->setPlaylist(m_playlist);
    if (playerWeiter && m_playerSpielte) m_player->play();
    m_playerSpielte = false;
    m_engine = nullptr;
    m_player = nullptr;
    m_playlist = nullptr;
    m_mixer = INVALID_STREAM;
}

int Tonfuehrung::quellen() const
{
    return static_cast<int>(std::count_if(m_quellen.begin(), m_quellen.end(),
                                          [](const Quelle& q) { return q.handle != INVALID_STREAM; }));
}

Tonfuehrung::Ergebnis Tonfuehrung::takt(const Komposition& komposition, std::int64_t stelleMs, bool laeuft,
                                        Schleifenlage schleife)
{
    Ergebnis ergebnis;
    if (!fuehrt()) return ergebnis;
    IAudioEngine& engine = *m_engine;

    // --- der Player schweigt; seine Lautstaerke und sein Mute gelten fuer den ganzen Mixer
    if (m_player->isPlaying())
    {
        m_player->pause();
        ergebnis.playerAngehalten = true;
    }
    const float laut = m_player->isMuted() ? 0.0f : m_player->volume();
    if (laut != m_laut)
    {
        engine.setVolume(m_mixer, laut);
        m_laut = laut;
    }

    const bool liefZuletzt = m_liefZuletzt;
    m_liefZuletzt = laeuft;

    // --- die Stimmen dieses Takts: jeder Ton-Clip, der spielt oder gleich
    //     beginnt, an seiner Stelle der Zeitachse. Ausserhalb der Schleife ist
    //     das die Stelle der Uhr; im Fenster der Ueberlappung (S99) kommen die
    //     Clips der hinteren Stelle dazu, und vor dem Umschlag die ersten der
    //     naechsten Runde (im Vorlauf, an einer Stelle vor null).
    struct Stimme
    {
        const Clip* clip;
        std::int64_t stelleMs;  ///< ihre Stelle auf der Zeitachse
        std::int64_t sollMs;    ///< die Stelle im Song, die dort zu hoeren ist
    };
    std::vector<Stimme> stimmen;
    const auto stimme = [](const Clip* clip, std::int64_t stelle) {
        return Stimme{clip, stelle, clip->songStelle(std::max<std::int64_t>(0, stelle - clip->startMs))};
    };
    const auto hat = [&stimmen](const Clip* clip) {
        return std::find_if(stimmen.begin(), stimmen.end(),
                            [clip](const Stimme& s) { return s.clip == clip; });
    };
    for (const Clip* clip : komposition.toeneBei(stelleMs, kVorlaufMs)) stimmen.push_back(stimme(clip, stelleMs));
    if (schleife.zweiteMs >= 0)
    {
        for (const Clip* clip : komposition.toeneBei(schleife.zweiteMs))
        {
            const auto vorn = hat(clip);
            if (vorn == stimmen.end() || komposition.songsDoppelt)
                stimmen.push_back(stimme(clip, schleife.zweiteMs));
            else
                *vorn = stimme(clip, schleife.zweiteMs);  // ohne zweite Quelle spielt er sein Ende aus
        }
    }
    if (schleife.umschlagInMs >= 0 && schleife.umschlagInMs <= kVorlaufMs)
    {
        for (const Clip* clip : komposition.toeneBei(-schleife.umschlagInMs, kVorlaufMs))
            if (hat(clip) == stimmen.end() || komposition.songsDoppelt)
                stimmen.push_back(stimme(clip, -schleife.umschlagInMs));
    }

    // --- die Quellen des Mixers den Stimmen zuordnen. Eine Quelle bleibt bei
    //     der Stimme ihres Clips, deren Stelle im Song ihr am naechsten liegt:
    //     so spielt am Umschlag die laufende Quelle das Ende weiter, und der
    //     Anfang bekommt eine frische — nichts wird dafuer neu gesetzt.
    const auto schluesselVon = [](const Clip* clip) {
        return Schluessel{clip->datei, clip->startMs, clip->einsatzMs};
    };
    struct Paar
    {
        std::int64_t abstand;
        std::size_t quelle;
        std::size_t stimme;
    };
    std::vector<Paar> paare;
    for (std::size_t s = 0; s < stimmen.size(); ++s)
        for (std::size_t q = 0; q < m_quellen.size(); ++q)
            if (m_quellen[q].schluessel == schluesselVon(stimmen[s].clip))
                paare.push_back({std::llabs(m_quellen[q].sollMs - stimmen[s].sollMs), q, s});
    std::stable_sort(paare.begin(), paare.end(), [](const Paar& a, const Paar& b) { return a.abstand < b.abstand; });
    constexpr std::size_t kKeine = static_cast<std::size_t>(-1);
    std::vector<std::size_t> quelleVon(stimmen.size(), kKeine);
    for (Quelle& q : m_quellen) q.vergeben = false;
    for (const Paar& p : paare)
    {
        if (m_quellen[p.quelle].vergeben || quelleVon[p.stimme] != kKeine) continue;
        m_quellen[p.quelle].vergeben = true;
        quelleVon[p.stimme] = p.quelle;
    }
    std::vector<Quelle> quellen;  // danach: quellen[i] gehoert zu stimmen[i]
    quellen.reserve(stimmen.size());
    for (std::size_t s = 0; s < stimmen.size(); ++s)
    {
        if (quelleVon[s] != kKeine)
        {
            quellen.push_back(m_quellen[quelleVon[s]]);
            continue;
        }
        const Clip* clip = stimmen[s].clip;
        Quelle q;
        q.schluessel = schluesselVon(clip);
        q.sollMs = stimmen[s].sollMs;
        // laesst sie sich nicht oeffnen, bleibt der Eintrag: ein Versuch, dann Stille
        q.handle = engine.mixerAddSource(m_mixer, clip->datei);
        if (q.handle != INVALID_STREAM)
        {
            q.dauerMs = engine.getDurationMs(q.handle);
            engine.setSourcePositionMs(q.handle, static_cast<int>(std::max<std::int64_t>(0, clip->einsatzMs)));
        }
        quellen.push_back(q);
    }
    for (const Quelle& q : m_quellen)
        if (!q.vergeben && q.handle != INVALID_STREAM) engine.mixerRemoveSource(q.handle);
    m_quellen = std::move(quellen);

    // --- je Stimme: Pegel, Stelle, laufen — und wer der laufende Song ist
    const Clip* bester = nullptr;
    const Quelle* besteQuelle = nullptr;
    double bestPegel = -1.0;
    std::int64_t besteStelle = 0;
    bool besterZuEnde = false;
    constexpr double kGleich = 1e-6;
    for (std::size_t s = 0; s < stimmen.size(); ++s)
    {
        const Clip* clip = stimmen[s].clip;
        Quelle& q = m_quellen[s];
        const std::int64_t stimmStelle = stimmen[s].stelleMs;
        const std::int64_t soll = stimmen[s].sollMs;
        // Ist diese Stimme gesprungen (Lineal, Stop, ein Sprung im Song)? Ein
        // Schritt im Song, wie ihn kein Takt macht: rueckwaerts oder weit
        // voraus — bei stehender oder eben angelaufener Uhr jede Bewegung.
        // Der Umschlag der Schleife ist keiner: die Stimme des Endes laeuft
        // stetig weiter.
        const std::int64_t schritt = soll - q.sollMs;
        const bool sprang = laeuft && liefZuletzt ? (schritt < -kToleranzMs || schritt > kSprungMs)
                                                  : schritt != 0;
        q.sollMs = soll;
        if (q.handle == INVALID_STREAM) continue;
        const std::int64_t imClip = stimmStelle - clip->startMs;
        // im Vorlauf, oder der Song ist vor dem Ende seines Clips aus: die Quelle haelt
        const bool zuEnde = q.dauerMs > 0 && soll >= q.dauerMs;
        if (imClip < 0 || zuEnde)
        {
            if (q.imClip)
            {
                engine.setSourcePaused(q.handle, true);
                q.imClip = false;
            }
            if (imClip < 0) continue;
        }
        // die Pegelkurve des Clips mal die Blende der ganzen Komposition an der Stelle der Stimme
        const double pegelWert = clip->pegelBei(imClip) * komposition.blendeBei(stimmStelle);
        if (!zuEnde)
        {
            const auto pegel = static_cast<float>(pegelWert);
            if (pegel != q.pegel)
            {
                engine.setVolume(q.handle, pegel);
                q.pegel = pegel;
            }
            const bool einstieg = !q.imClip;
            bool setze = einstieg || sprang;
            if (!setze && laeuft && std::llabs(stimmStelle - q.gesetztBeiMs) >= kSchonfristMs)
            {
                const std::int64_t daneben = engine.getSourcePositionMs(q.handle) - soll;
                if (!q.hatGrund)
                {
                    // Was die Engine als „zu hoeren" meldet, liegt um die Latenz
                    // der Ausgabe hinter der Uhr (gemessen S98: stetig etwa 45 ms
                    // am Geraet des Rechners; ueber Funk ein Vielfaches). Das ist
                    // der Grundabstand dieser Quelle, kein Davonlaufen.
                    q.grundMs = daneben;
                    q.hatGrund = true;
                }
                else if (std::llabs(daneben - q.grundMs) > kToleranzMs)
                {
                    // die Quelle ist der Uhr davongelaufen (oder sie ihr)
                    setze = true;
                    ++m_nachgesetzt;
                    if (m_nachgesetzt <= 200 || m_nachgesetzt % 50 == 0)
                        BasicLogger::logInfo("Composer: audio source realigned to the clock (" +
                                             std::to_string(daneben - q.grundMs) + " ms off its usual " +
                                             std::to_string(q.grundMs) + " ms, " +
                                             QFileInfo(clip->datei).fileName().toStdString() + ", " +
                                             std::to_string(m_nachgesetzt) + " so far)");
                }
            }
            if (setze)
            {
                engine.setSourcePositionMs(q.handle, static_cast<int>(soll));
                q.gesetztBeiMs = stimmStelle;
                q.hatGrund = false;  // nach dem Setzen neu messen
            }
            if (einstieg)
            {
                engine.setSourcePaused(q.handle, false);
                q.imClip = true;
            }
        }
        // der laufende Song: der hoechste Wert der Pegelkurve; bei Gleichstand
        // der bisherige, sonst der oberste Kanal (der erste in der Liste)
        const bool gleich = bester != nullptr && std::abs(pegelWert - bestPegel) <= kGleich;
        if (bester == nullptr || (!gleich && pegelWert > bestPegel) || (gleich && q.schluessel == m_laufend))
        {
            bester = clip;
            besteQuelle = &q;
            bestPegel = pegelWert;
            besteStelle = soll;
            besterZuEnde = zuEnde;
        }
    }

    // --- haelt die Uhr, haelt der Mixer
    const bool mixerSpielt = engine.isPlaying(m_mixer);
    if (laeuft && !mixerSpielt)
        engine.play(m_mixer);
    else if (!laeuft && mixerSpielt)
        engine.pause(m_mixer);

    if (bester != nullptr)
    {
        ergebnis.hatSong = true;
        ergebnis.songStelleMs = besteStelle;
        ergebnis.songLaeuft = laeuft && !besterZuEnde;
        if (!(besteQuelle->schluessel == m_laufend))
        {
            m_laufend = besteQuelle->schluessel;
            m_laufendDauerMs = besteQuelle->dauerMs;
            ergebnis.songGewechselt = true;
            ergebnis.song = leseTrackInfo(engine, besteQuelle->handle, bester->datei);
        }
        ergebnis.songDauerMs = m_laufendDauerMs;
    }
    return ergebnis;
}

std::int64_t Tonfuehrung::stelleImLaufenden(const Komposition& komposition, double anteil) const
{
    if (m_laufend.startMs < 0 || m_laufendDauerMs <= 0) return -1;
    for (const Kanal& kanal : komposition.kanaele)
    {
        if (kanal.art != KanalArt::Audio || !kanal.an) continue;
        for (const Clip& c : kanal.clips)
        {
            if (!(Schluessel{c.datei, c.startMs, c.einsatzMs} == m_laufend)) continue;
            const auto imSong = static_cast<std::int64_t>(std::clamp(anteil, 0.0, 1.0) * m_laufendDauerMs);
            // was vor dem Einsatz oder hinter dem Ende des Clips laege, gibt es in der Komposition nicht
            return std::clamp<std::int64_t>(c.startMs + (imSong - c.einsatzMs), c.startMs, c.endeMs - 1);
        }
    }
    return -1;
}

std::int64_t Tonfuehrung::songAnfang(const Komposition& komposition, std::int64_t stelleMs, bool vor)
{
    // die Songs aller eingeschalteten Audio-Kanaele, nach ihrem Anfang
    std::vector<Clip> clips;
    for (const Kanal& kanal : komposition.kanaele)
        if (kanal.art == KanalArt::Audio && kanal.an)
            clips.insert(clips.end(), kanal.clips.begin(), kanal.clips.end());
    std::stable_sort(clips.begin(), clips.end(),
                     [](const Clip& a, const Clip& b) { return a.startMs < b.startMs; });
    if (vor)
    {
        for (const Clip& c : clips)
            if (c.startMs > stelleMs) return c.startMs;
        return -1;
    }
    std::int64_t ziel = -1;
    for (const Clip& c : clips)
    {
        if (c.startMs > stelleMs) break;
        // mitten im Song: an seinen Anfang; in seinen ersten Sekunden: zum Song davor
        const bool drin = stelleMs < c.endeMs;
        if (drin && stelleMs - c.startMs <= kZurueckMs)
            return ziel >= 0 ? ziel : c.startMs;
        ziel = c.startMs;
    }
    return ziel;
}

}  // namespace lumi::composer
