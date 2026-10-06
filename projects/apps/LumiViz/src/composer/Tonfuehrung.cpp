/**
 * @file Tonfuehrung.cpp
 * @brief Composer: die Audio-Kanaele im Mixer (Stufe C5b)
 */

#include "composer/Tonfuehrung.hpp"

#include "audio/AudioPlayer.hpp"  // leseTrackInfo (Naht N14)
#include "audio/IAudioPlayer.hpp"

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
    if (player.isPlaying()) player.pause();
    m_playlist = player.playlist();
    player.setPlaylist(nullptr);
    m_quellen.clear();
    m_laut = -1.0f;
    m_laufend = {};
    m_laufendDauerMs = 0;
    m_ersterTakt = true;
    m_liefZuletzt = false;
    m_letzteStelle = 0;
    m_nachgesetzt = 0;
    return true;
}

void Tonfuehrung::gibFrei()
{
    if (!fuehrt()) return;
    for (const Quelle& q : m_quellen)
        if (q.handle != INVALID_STREAM) m_engine->mixerRemoveSource(q.handle);
    m_quellen.clear();
    m_engine->freeStream(m_mixer);
    m_player->setPlaylist(m_playlist);
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

Tonfuehrung::Ergebnis Tonfuehrung::takt(const Komposition& komposition, std::int64_t stelleMs, bool laeuft)
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

    // --- ist die Uhr gesprungen (Lineal, Stop, Schleife)? Ein Schritt, wie ihn
    //     kein Takt macht: rueckwaerts oder weit voraus — bei stehender oder
    //     eben angelaufener Uhr jede Bewegung.
    const std::int64_t schritt = stelleMs - m_letzteStelle;
    const bool sprang = m_ersterTakt || (laeuft && m_liefZuletzt
                                             ? (schritt < -kToleranzMs || schritt > kSprungMs)
                                             : schritt != 0);
    m_ersterTakt = false;
    m_liefZuletzt = laeuft;
    m_letzteStelle = stelleMs;

    // --- die Quellen des Mixers an die Clips angleichen, die hier spielen oder gleich beginnen
    const std::vector<const Clip*> toene = komposition.toeneBei(stelleMs, kVorlaufMs);
    const auto finde = [this](const Schluessel& s) {
        return std::find_if(m_quellen.begin(), m_quellen.end(),
                            [&s](const Quelle& q) { return q.schluessel == s; });
    };
    for (Quelle& q : m_quellen) q.gesehen = false;
    for (const Clip* clip : toene)
    {
        const Schluessel s{clip->datei, clip->startMs, clip->einsatzMs};
        auto it = finde(s);
        if (it == m_quellen.end())
        {
            Quelle q;
            q.schluessel = s;
            // laesst sie sich nicht oeffnen, bleibt der Eintrag: ein Versuch, dann Stille
            q.handle = engine.mixerAddSource(m_mixer, clip->datei);
            if (q.handle != INVALID_STREAM)
            {
                q.dauerMs = engine.getDurationMs(q.handle);
                engine.setSourcePositionMs(q.handle, static_cast<int>(std::max<std::int64_t>(0, clip->einsatzMs)));
            }
            m_quellen.push_back(q);
            it = std::prev(m_quellen.end());
        }
        it->gesehen = true;
    }
    for (const Quelle& q : m_quellen)
        if (!q.gesehen && q.handle != INVALID_STREAM) engine.mixerRemoveSource(q.handle);
    m_quellen.erase(std::remove_if(m_quellen.begin(), m_quellen.end(), [](const Quelle& q) { return !q.gesehen; }),
                    m_quellen.end());

    // --- je Clip: Pegel, Stelle, laufen — und wer der laufende Song ist
    const Clip* bester = nullptr;
    const Quelle* besteQuelle = nullptr;
    double bestPegel = -1.0;
    std::int64_t besteStelle = 0;
    bool besterZuEnde = false;
    constexpr double kGleich = 1e-6;
    for (const Clip* clip : toene)
    {
        Quelle& q = *finde(Schluessel{clip->datei, clip->startMs, clip->einsatzMs});
        if (q.handle == INVALID_STREAM) continue;
        const std::int64_t imClip = stelleMs - clip->startMs;
        const std::int64_t soll = clip->songStelle(std::max<std::int64_t>(0, imClip));
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
        const double pegelWert = clip->pegelBei(imClip);
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
            if (!setze && laeuft && std::llabs(stelleMs - q.gesetztBeiMs) >= kSchonfristMs)
            {
                // die Quelle ist der Uhr davongelaufen (oder sie ihr)
                const std::int64_t ist = engine.getSourcePositionMs(q.handle);
                if (std::llabs(ist - soll) > kToleranzMs)
                {
                    setze = true;
                    ++m_nachgesetzt;
                    if (m_nachgesetzt <= 5 || m_nachgesetzt % 50 == 0)
                        BasicLogger::logInfo("Composer: audio source realigned to the clock (" +
                                             std::to_string(ist - soll) + " ms off, " +
                                             std::to_string(m_nachgesetzt) + " so far)");
                }
            }
            if (setze)
            {
                engine.setSourcePositionMs(q.handle, static_cast<int>(soll));
                q.gesetztBeiMs = stelleMs;
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
