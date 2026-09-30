/**
 ****************************************************************************************
 * @file   AudioDateien.hpp
 * @brief  Audiodateien in der Oberflaeche: Endungen, Icons, „abspielen" (S84)
 *
 * @details
 * Gemeinsamer Kern fuer Audio Browser, Playlist, Player-Panel und
 * Visualisierung — eine Wahrheit fuer drei Fragen:
 *
 * - **Was ist abspielbar?** Die Endungen der Engine (`IAudioEngine::
 *   supportedExtensions`, bei BASS inkl. geladener Plugins); ohne Engine eine
 *   kleine Vorgabe.
 * - **Wie sieht es aus?** Ein Icon je Formatfamilie
 *   (`asset/img/logo/icons/audio_*.svg`).
 * - **Was heisst „abspielen"?** Steht die Datei schon in der Playlist, wird zu
 *   diesem Eintrag gesprungen; sonst wird sie angehaengt und gespielt. Ein
 *   Drag aus der Playlist traegt seine Zeilen mit (`kPlaylistZeilenMime`) —
 *   dann gilt genau diese Zeile, auch bei Doppeln.
 ****************************************************************************************
 */

#pragma once

#include "UI/widgets/DateiDrag.hpp"
#include "UI/widgets/PresetTypeIcons.hpp"  // presetIconDir()
#include "audio/IAudioEngine.hpp"
#include "audio/IAudioPlayer.hpp"
#include "audio/IPlaylist.hpp"
#include "services/ServiceContainer.hpp"

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QHash>
#include <QIcon>
#include <QMimeData>
#include <QString>
#include <QStringList>

namespace lumi::ui
{

/// Abspielbare Endungen (klein, ohne Punkt): von der Engine, sonst die Vorgabe.
[[nodiscard]] inline QStringList audioEndungen(ServiceContainer& services)
{
    if (auto* engine = services.tryResolve<IAudioEngine>())
    {
        const QStringList e = engine->supportedExtensions();
        if (!e.isEmpty()) return e;
    }
    return {QStringLiteral("mp3"), QStringLiteral("wav"), QStringLiteral("ogg"),
            QStringLiteral("aiff"), QStringLiteral("flac"), QStringLiteral("m4a")};
}

/// Lokale Audiodateien eines Drags, in gezogener Reihenfolge.
[[nodiscard]] inline QStringList audioDateien(const QMimeData* data, ServiceContainer& services)
{
    return dateienMitEndung(data, audioEndungen(services));
}

/// Formatfamilie einer Endung = Name des Icons ohne „audio_" (leer = allgemein).
[[nodiscard]] inline QString audioFamilie(const QString& suffix)
{
    static const QHash<QString, QString> kFamilie = {
        {QStringLiteral("mp3"), QStringLiteral("mp3")},
        {QStringLiteral("mp2"), QStringLiteral("mp3")},
        {QStringLiteral("mp1"), QStringLiteral("mp3")},
        {QStringLiteral("wav"), QStringLiteral("wav")},
        {QStringLiteral("aiff"), QStringLiteral("wav")},
        {QStringLiteral("aif"), QStringLiteral("wav")},
        {QStringLiteral("flac"), QStringLiteral("flac")},
        {QStringLiteral("ape"), QStringLiteral("flac")},
        {QStringLiteral("wv"), QStringLiteral("flac")},
        {QStringLiteral("tta"), QStringLiteral("flac")},
        {QStringLiteral("ogg"), QStringLiteral("ogg")},
        {QStringLiteral("oga"), QStringLiteral("ogg")},
        {QStringLiteral("opus"), QStringLiteral("ogg")},
        {QStringLiteral("m4a"), QStringLiteral("m4a")},
        {QStringLiteral("aac"), QStringLiteral("m4a")},
        {QStringLiteral("mp4"), QStringLiteral("m4a")},
        {QStringLiteral("alac"), QStringLiteral("m4a")},
    };
    return kFamilie.value(suffix.toLower());
}

/// Icon je Formatfamilie (Null-Icon, wenn das Icon-Verzeichnis fehlt).
[[nodiscard]] inline QIcon audioTypIcon(const QString& suffix)
{
    static QHash<QString, QIcon> cache;
    const QString familie = audioFamilie(suffix);
    auto it = cache.find(familie);
    if (it == cache.end())
    {
        const QString dir = presetIconDir();
        const QString datei = familie.isEmpty() ? QStringLiteral("/audio.svg")
                                                : QStringLiteral("/audio_%1.svg").arg(familie);
        it = cache.insert(familie, dir.isEmpty() ? QIcon() : QIcon(dir + datei));
    }
    return *it;
}

/**
 * @brief Dateien abspielen: vorhandene Eintraege anspringen, fehlende anhaengen
 * @return true = der erste Titel laeuft
 *
 * Gespielt wird die ERSTE Datei; die uebrigen landen nur in der Playlist.
 */
inline bool spieleDateien(ServiceContainer& services, const QStringList& dateien)
{
    auto* playlist = services.tryResolve<IPlaylist>();
    auto* player = services.tryResolve<IAudioPlayer>();
    if (playlist == nullptr || player == nullptr) return false;
    int erster = -1;
    for (const QString& datei : dateien)
    {
        int index = playlist->indexOf(datei);
        if (index < 0) index = playlist->addTrack(datei);
        if (erster < 0) erster = index;
    }
    return erster >= 0 && player->playIndex(erster);
}

/// Traegt der Drag etwas Abspielbares (Playlist-Zeilen oder Audiodateien)?
[[nodiscard]] inline bool hatAbspielbares(const QMimeData* data, ServiceContainer& services)
{
    return !playlistZeilen(data).isEmpty() || !audioDateien(data, services).isEmpty();
}

/// Drop auf Player/Visualisierung: Playlist-Zeile anspringen, sonst Dateien
/// abspielen (s. spieleDateien). true = etwas wurde gestartet.
inline bool spieleDrop(const QMimeData* data, ServiceContainer& services)
{
    const QList<int> zeilen = playlistZeilen(data);
    if (!zeilen.isEmpty())
    {
        auto* player = services.tryResolve<IAudioPlayer>();
        return player != nullptr && player->playIndex(zeilen.first());
    }
    return spieleDateien(services, audioDateien(data, services));
}

/// dragEnter/dragMove eines Abspiel-Ziels (Player, Visualisierung): annehmen,
/// wenn etwas Abspielbares kommt — immer als KOPIE, damit ein gezogener
/// Playlist-Eintrag beim Loslassen nicht verschwindet.
inline void nimmAbspielbaresAn(QDragMoveEvent* event, ServiceContainer& services)
{
    if (hatAbspielbares(event->mimeData(), services) &&
        event->possibleActions().testFlag(Qt::CopyAction))
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

/// dropEvent eines Abspiel-Ziels: abspielen bzw. zum Eintrag springen.
inline void legeAbspielbaresAb(QDropEvent* event, ServiceContainer& services)
{
    if (!spieleDrop(event->mimeData(), services))
    {
        event->ignore();
        return;
    }
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

}  // namespace lumi::ui
