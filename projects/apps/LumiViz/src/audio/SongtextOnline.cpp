/**
 ****************************************************************************************
 * @file   SongtextOnline.cpp
 * @brief  Songtext aus dem Netz holen (LRCLIB) — nur auf ausdrueckliche Einstellung
 ****************************************************************************************
 */

#include "audio/SongtextOnline.hpp"

#include "audio/SongtextQuelle.hpp"

#include <BasicLogger.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrlQuery>

#include <algorithm>

namespace lumi::audio
{

using lumi::multieffect::lyrics::Songtext;

bool songtextOnlineErlaubt()
{
    return QSettings().value(QStringLiteral("lyrics/onlineSearch"), false).toBool();
}

SongtextAblage songtextAblage()
{
    return QSettings().value(QStringLiteral("lyrics/saveLocation"), 0).toInt() == 1
               ? SongtextAblage::NebenDemSong
               : SongtextAblage::Benutzerordner;
}

QUrl lrclibUrl(const TrackInfo& track)
{
    const QString titel = track.title.trimmed();
    const QString interpret = track.artist.trimmed();
    if (titel.isEmpty() || interpret.isEmpty()) return QUrl();  // beide sind Pflicht
    // QUrlQuery kodiert `&` und `=` selbst; `%` und `+` muessen vorab kodiert
    // werden (ein `+` gaelte beim Server als Leerzeichen).
    const auto wert = [](QString v) {
        v.replace(QLatin1Char('%'), QStringLiteral("%25"));
        v.replace(QLatin1Char('+'), QStringLiteral("%2B"));
        return v;
    };
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("track_name"), wert(titel));
    q.addQueryItem(QStringLiteral("artist_name"), wert(interpret));
    if (!track.album.trimmed().isEmpty())
        q.addQueryItem(QStringLiteral("album_name"), wert(track.album.trimmed()));
    // Dauer in Sekunden, erlaubt 1..3600 — ausserhalb weglassen statt abgewiesen werden
    const int sekunden = (track.durationMs + 500) / 1000;
    if (sekunden >= 1 && sekunden <= 3600)
        q.addQueryItem(QStringLiteral("duration"), QString::number(sekunden));
    QUrl url(QStringLiteral("https://lrclib.net/api/get"));
    url.setQuery(q);
    return url;
}

Songtext songtextAusLrclib(const QByteArray& json)
{
    const QJsonObject o = QJsonDocument::fromJson(json).object();
    if (o.isEmpty() || o.value(QStringLiteral("instrumental")).toBool()) return {};
    // synchron vor unsynchron — dieselbe Regel wie bei den lokalen Quellen
    for (const char* feld : {"syncedLyrics", "plainLyrics"})
    {
        const QString text = o.value(QLatin1String(feld)).toString();
        if (text.trimmed().isEmpty()) continue;
        Songtext s = lumi::multieffect::lyrics::leseLrc(text.toStdString());
        if (s.leer()) continue;
        s.quelle = "LRCLIB";
        return s;
    }
    return {};
}

QString songtextZiel(const TrackInfo& track, bool synchron, SongtextAblage ablage,
                     const QString& benutzerOrdner, bool* ausgewichen)
{
    if (ausgewichen != nullptr) *ausgewichen = false;
    const QString endung = synchron ? QStringLiteral(".lrc") : QStringLiteral(".txt");
    if (ablage == SongtextAblage::NebenDemSong && !track.filePath.isEmpty())
    {
        const QFileInfo song(track.filePath);
        if (QFileInfo(song.absolutePath()).isWritable())
            return song.dir().absoluteFilePath(song.completeBaseName() + endung);
        if (ausgewichen != nullptr) *ausgewichen = true;
    }
    const QString name = songtextName(track);
    if (name.isEmpty() || benutzerOrdner.isEmpty()) return QString();
    return benutzerOrdner + QLatin1Char('/') + name + endung;
}

SongtextOnline::SongtextOnline(QObject* parent) : QObject(parent) {}

void SongtextOnline::suche(const TrackInfo& track)
{
    if (m_laufend != nullptr)  // eine Anfrage zur Zeit; der neue Titel zaehlt
    {
        m_laufend->abort();
        m_laufend = nullptr;
    }
    if (!songtextOnlineErlaubt()) return;
    if (m_pauseBis.isValid() && QDateTime::currentDateTimeUtc() < m_pauseBis) return;
    const QUrl url = lrclibUrl(track);
    if (!url.isValid() || url.isEmpty()) return;

    if (m_net == nullptr) m_net = new QNetworkAccessManager(this);
    QNetworkRequest req(url);
    // verlangt: Programmname, Version und Projektseite (lrclib.net/docs)
    req.setRawHeader("User-Agent",
                     QStringLiteral("LumiViz v%1 (https://github.com/PatrikNeunteufel/LumiViz)")
                         .arg(QCoreApplication::applicationVersion())
                         .toUtf8());
    req.setTransferTimeout(15000);
    QNetworkReply* reply = m_net->get(req);
    m_laufend = reply;
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, track]() { fertig(reply, track); });
}

void SongtextOnline::fertig(QNetworkReply* reply, const TrackInfo& track)
{
    reply->deleteLater();
    if (reply != m_laufend) return;  // ueberholt von einer neueren Suche
    m_laufend = nullptr;

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 429 || status == 503)
    {
        // Retry-After befolgen; ohne Angabe eine Minute Ruhe
        const int sek = std::max(1, reply->rawHeader("Retry-After").toInt());
        m_pauseBis = QDateTime::currentDateTimeUtc().addSecs(
            reply->hasRawHeader("Retry-After") ? sek : 60);
        BasicLogger::logInfo("Lyrics: LRCLIB asks to wait (HTTP " + std::to_string(status) + ")");
        return;
    }
    if (status != 200) return;  // 404 = kein Text; alles andere: still

    Songtext text = songtextAusLrclib(reply->readAll());
    if (text.leer()) return;
    bool ausgewichen = false;
    const QString ziel =
        songtextZiel(track, text.synchron, songtextAblage(), songtextOrdner(), &ausgewichen);
    QString gespeichert;
    if (!ziel.isEmpty() && speichereSongtext(text, ziel)) gespeichert = ziel;
    if (ausgewichen)
        BasicLogger::logInfo("Lyrics: folder of the song is not writable — saved to the user "
                             "folder instead");
    text.quelle = gespeichert.isEmpty()
                      ? "LRCLIB (not saved)"
                      : "LRCLIB, saved: " + QFileInfo(gespeichert).fileName().toStdString();
    Q_EMIT gefunden(track.filePath, text, gespeichert);
}

}  // namespace lumi::audio
