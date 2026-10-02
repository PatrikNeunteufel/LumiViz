/**
 ****************************************************************************************
 * @file   Wiederaufnahme.hpp
 * @brief  „Resume playback": nach dem Neustart dort weiterspielen, wo beendet wurde
 *
 * @details
 * Session 88 (Wunsch Patrik): wird LumiViz WAEHREND des Abspielens beendet, spielt
 * es beim naechsten Start genau dort weiter — wenn die Einstellung an ist
 * (`player/resumePlayback`, Vorgabe aus; Schalter in Settings → Audio und im
 * Player-Panel).
 *
 * - Festgehalten wird beim geordneten Beenden (`aboutToQuit`): Datei, Index in
 *   der Playlist, Abspielposition und ob gerade gespielt wurde. Pausiert oder
 *   gestoppt beendet = beim naechsten Start bleibt es still.
 * - Aufgenommen wird nach dem Wiederherstellen der Sitzungs-Playlist: der
 *   Eintrag wird ueber die Playlist gespielt (Index, sonst Suche nach der
 *   Datei), sonst die Datei direkt geladen; danach der Sprung an die Position.
 * - Nach einem Absturz gibt es keinen Stand — dann gilt der des letzten
 *   geordneten Beendens.
 *
 * Bewusst freie Funktionen ueber `IAudioPlayer` (kein neuer Vertrag im
 * Interface): so laesst sich der Ablauf mit einer Player-Attrappe pruefen.
 ****************************************************************************************
 */

#pragma once

#include "audio/IAudioPlayer.hpp"
#include "audio/IPlaylist.hpp"

#include <QFileInfo>
#include <QSettings>
#include <QString>

namespace lumi::audio
{

/// Was beim Beenden festgehalten wird.
struct WiederaufnahmeStand
{
    QString datei;       ///< absoluter Pfad des Titels, leer = keiner
    int index = -1;      ///< Index in der Playlist beim Beenden (-1 = ohne Playlist)
    int positionMs = 0;  ///< Abspielposition
    bool lief = false;   ///< es wurde gerade gespielt (nicht pausiert, nicht gestoppt)
};

/// Die Einstellung „Resume playback" (QSettings, Vorgabe aus).
[[nodiscard]] inline bool wiederaufnahmeAn()
{
    return QSettings().value(QStringLiteral("player/resumePlayback"), false).toBool();
}

inline void setWiederaufnahme(bool an)
{
    QSettings().setValue(QStringLiteral("player/resumePlayback"), an);
}

/// Stand des Players, wie er beim Beenden festzuhalten ist.
[[nodiscard]] inline WiederaufnahmeStand standZumBeenden(const IAudioPlayer& player)
{
    WiederaufnahmeStand s;
    if (!player.hasTrack()) return s;
    s.datei = player.currentTrack().filePath;
    s.index = player.playlistIndex();
    s.positionMs = player.positionMs();
    s.lief = player.isPlaying();
    return s;
}

/// Stand in die QSettings schreiben (immer — auch „lief nicht", damit ein
/// frueherer Stand nicht stehen bleibt).
inline void speichereWiederaufnahme(const WiederaufnahmeStand& s)
{
    QSettings settings;
    settings.setValue(QStringLiteral("player/resumeFile"), s.datei);
    settings.setValue(QStringLiteral("player/resumeIndex"), s.index);
    settings.setValue(QStringLiteral("player/resumePositionMs"), s.positionMs);
    settings.setValue(QStringLiteral("player/resumeWasPlaying"), s.lief);
}

[[nodiscard]] inline WiederaufnahmeStand ladeWiederaufnahme()
{
    const QSettings settings;
    WiederaufnahmeStand s;
    s.datei = settings.value(QStringLiteral("player/resumeFile")).toString();
    s.index = settings.value(QStringLiteral("player/resumeIndex"), -1).toInt();
    s.positionMs = settings.value(QStringLiteral("player/resumePositionMs"), 0).toInt();
    s.lief = settings.value(QStringLiteral("player/resumeWasPlaying"), false).toBool();
    return s;
}

/**
 * Wiedergabe nach `stand` aufnehmen. Prueft die Einstellung NICHT (das tut der
 * Aufrufer). false = nichts aufzunehmen (lief nicht, Datei fehlt) oder der
 * Titel liess sich nicht laden.
 */
inline bool nimmWiederAuf(IAudioPlayer& player, const WiederaufnahmeStand& stand)
{
    if (!stand.lief || stand.datei.isEmpty() || !QFileInfo::exists(stand.datei)) return false;

    // Ueber die Playlist spielen, wenn der Titel dort steht: am gemerkten
    // Index, sonst wo immer die Datei jetzt liegt (die Liste kann sich
    // geaendert haben) — „Next" laeuft dann wie gewohnt weiter.
    int index = -1;
    if (IPlaylist* liste = player.playlist())
    {
        if (stand.index >= 0 && stand.index < liste->count() &&
            liste->filePathAt(stand.index) == stand.datei)
        {
            index = stand.index;
        }
        else
        {
            for (int i = 0; i < liste->count(); ++i)
            {
                if (liste->filePathAt(i) == stand.datei)
                {
                    index = i;
                    break;
                }
            }
        }
    }
    if (index >= 0)
    {
        if (!player.playIndex(index)) return false;
    }
    else
    {
        if (!player.load(stand.datei)) return false;
        player.play();
    }
    if (stand.positionMs > 0) player.seek(stand.positionMs);
    return true;
}

}  // namespace lumi::audio
