/**
 ****************************************************************************************
 * @file   DateiDrag.hpp
 * @brief  Dateien ziehen: Preset-Endungen und Datei-URLs im Drag (S84)
 *
 * @details
 * Gezogene Dateien reisen als **Datei-URLs** (`text/uri-list`) — dieselbe Form,
 * die der Windows-Explorer und andere Programme liefern. Darum nimmt der
 * Kettenbaum ein Preset aus dem Effect Browser genauso an wie eines aus dem
 * Explorer, und ein kuenftiges Ziel (Timeline) braucht kein eigenes Format.
 *
 * SSOT fuer die Endungen, die als Preset gelten (Kettenbaum). Audio-Endungen
 * kommen von der Engine (`AudioDateien.hpp`).
 ****************************************************************************************
 */

#pragma once

#include <QFileInfo>
#include <QList>
#include <QMimeData>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace lumi::ui
{

/// Drag-Daten fuer lokale Dateien (Aufrufer uebernimmt den Besitz). Leer = nullptr.
[[nodiscard]] inline QMimeData* dateiMime(const QStringList& pfade)
{
    if (pfade.isEmpty()) return nullptr;
    QList<QUrl> urls;
    urls.reserve(pfade.size());
    for (const QString& p : pfade) urls.append(QUrl::fromLocalFile(p));
    auto* data = new QMimeData();
    data->setUrls(urls);
    return data;
}

/// Lokale Dateien eines Drags mit einer der `endungen` (klein, ohne Punkt), in
/// gezogener Reihenfolge. Anderes (Ordner, fremde Endungen, entfernte URLs)
/// wird uebergangen.
[[nodiscard]] inline QStringList dateienMitEndung(const QMimeData* data,
                                                  const QStringList& endungen)
{
    QStringList pfade;
    if (data == nullptr || !data->hasUrls()) return pfade;
    for (const QUrl& url : data->urls())
    {
        if (!url.isLocalFile()) continue;
        const QFileInfo fi(url.toLocalFile());
        if (fi.isFile() && endungen.contains(fi.suffix(), Qt::CaseInsensitive))
            pfade.append(fi.absoluteFilePath());
    }
    return pfade;
}

/// Lokale Preset-Dateien eines Drags, in gezogener Reihenfolge (leer = keine).
[[nodiscard]] inline QStringList presetDateien(const QMimeData* data)
{
    return dateienMitEndung(data, {QStringLiteral("avs"), QStringLiteral("milk"),
                                   QStringLiteral("lvfx"), QStringLiteral("lvfx2")});
}

/// Zusatzformat eines Drags aus der Playlist (S84): die gezogenen Zeilen als
/// „1,4,7". Neben den Datei-URLs, damit Doppel in der Playlist eindeutig
/// bleiben — `indexOf(pfad)` faende immer nur den ersten.
inline constexpr const char* kPlaylistZeilenMime = "application/x-lumiviz-playlist-rows";

/// Playlist-Zeilen eines Drags (leer = kein Playlist-Drag).
[[nodiscard]] inline QList<int> playlistZeilen(const QMimeData* data)
{
    QList<int> zeilen;
    const QString mime = QString::fromUtf8(kPlaylistZeilenMime);
    if (data == nullptr || !data->hasFormat(mime)) return zeilen;
    for (const QByteArray& teil : data->data(mime).split(','))
    {
        bool ok = false;
        const int z = teil.toInt(&ok);
        if (ok && z >= 0) zeilen.append(z);
    }
    return zeilen;
}

}  // namespace lumi::ui
