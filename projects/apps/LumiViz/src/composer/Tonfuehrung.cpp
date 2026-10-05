/**
 * @file Tonfuehrung.cpp
 * @brief Composer: der Audiokanal fuehrt den Player (Stufe C5)
 */

#include "composer/Tonfuehrung.hpp"

#include "audio/IAudioPlayer.hpp"

#include <algorithm>
#include <cstdlib>

namespace lumi::composer
{

void Tonfuehrung::uebernimm(IAudioPlayer& player)
{
    if (m_player == &player) return;
    gibFrei();
    m_player = &player;
    // die Playlist ruht: ohne sie spielt am Titelende nichts von selbst weiter
    m_playlist = player.playlist();
    player.setPlaylist(nullptr);
    m_schluessel = {};
    m_ersterTakt = true;
    m_songAus = false;
    m_liefZuletzt = false;
}

void Tonfuehrung::gibFrei()
{
    if (m_player == nullptr) return;
    m_player->setPlaylist(m_playlist);
    if (m_pegel) m_pegel(1.0f);
    m_player = nullptr;
    m_playlist = nullptr;
}

Tonfuehrung::Ergebnis Tonfuehrung::takt(const Komposition& komposition, std::int64_t stelleMs, bool laeuft)
{
    Ergebnis ergebnis;
    if (m_player == nullptr) return ergebnis;
    IAudioPlayer& player = *m_player;

    const Clip* clip = komposition.tonBei(stelleMs);
    const Schluessel jetzt =
        clip != nullptr ? Schluessel{clip->datei, clip->startMs, clip->einsatzMs} : Schluessel{};
    // Einstieg: ein anderer Clip als im letzten Takt — auch aus der Luecke heraus
    const bool einstieg = m_ersterTakt || !(jetzt == m_schluessel);
    const std::int64_t uhrSchritt = stelleMs - m_letzteStelle;
    m_ersterTakt = false;
    m_schluessel = jetzt;
    m_letzteStelle = stelleMs;

    // --- Luecke: der Player schweigt, die Wanduhr fuehrt
    if (clip == nullptr)
    {
        if (player.isPlaying()) player.pause();
        m_songAus = false;
        m_liefZuletzt = false;
        m_letztePos = player.positionMs();
        return ergebnis;
    }

    const std::int64_t imClip = stelleMs - clip->startMs;
    const std::int64_t soll = clip->songStelle(imClip);  // was an dieser Stelle zu hoeren ist
    if (m_pegel) m_pegel(static_cast<float>(clip->pegelBei(imClip)));

    // den Player an die Stelle setzen, die die Uhr nennt, und laufen oder halten lassen
    const auto setze = [&] {
        player.seek(static_cast<int>(soll));
        if (laeuft && !player.isPlaying()) player.play();
        if (!laeuft && player.isPlaying()) player.pause();
        m_songAus = false;
        m_liefZuletzt = laeuft;
        m_letztePos = soll;
    };

    if (einstieg)
    {
        if (!player.hasTrack() || player.currentTrack().filePath != clip->datei)
        {
            if (!player.load(clip->datei))
            {
                // nicht zu laden: Stille bis zum Ende des Clips — auch wenn
                // der Player den vorigen Song noch haette
                if (player.isPlaying()) player.pause();
                m_songAus = true;
                m_liefZuletzt = false;
                m_letztePos = 0;
                return ergebnis;
            }
        }
        setze();
        return ergebnis;
    }

    const std::int64_t pos = player.positionMs();
    const std::int64_t tonSchritt = pos - m_letztePos;
    m_letztePos = pos;
    // Ein Schritt, wie ihn kein Takt macht: rueckwaerts oder weit voraus —
    // bei stehender Uhr jede Bewegung.
    const auto sprang = [laeuft](std::int64_t schritt) {
        return laeuft ? (schritt < -kToleranzMs || schritt > kSprungMs) : std::llabs(schritt) > kToleranzMs;
    };

    // --- die Uhr ist gesprungen (Lineal, Stop, Schleife): sie fuehrt dieses eine Mal
    if (sprang(uhrSchritt))
    {
        setze();
        return ergebnis;
    }
    // --- der Song ist aus, sein Clip noch nicht: die Wanduhr fuehrt
    if (m_songAus) return ergebnis;

    if (laeuft && !player.isPlaying())
    {
        if (m_liefZuletzt)
        {
            // er spielte eben noch: der Song ist vor dem Ende seines Clips zu Ende
            m_songAus = true;
            m_liefZuletzt = false;
            return ergebnis;
        }
        player.play();  // die Uhr laeuft wieder
    }
    else if (!laeuft && player.isPlaying())
        player.pause();
    m_liefZuletzt = laeuft;

    // --- der Ton fuehrt: die Uhr gehoert dorthin, wo zu hoeren ist
    const std::int64_t tonStelle = std::max<std::int64_t>(0, clip->startMs + (pos - clip->einsatzMs));
    const bool daneben = std::llabs(stelleMs - tonStelle) > kToleranzMs;
    if (daneben && (laeuft || sprang(tonSchritt)))
    {
        ergebnis.uhrStellen = true;
        ergebnis.stelleMs = tonStelle;
        m_letzteStelle = tonStelle;
    }
    return ergebnis;
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
