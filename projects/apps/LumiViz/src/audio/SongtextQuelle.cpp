/**
 ****************************************************************************************
 * @file   SongtextQuelle.cpp
 * @brief  Den Songtext zu einem Titel finden — lokale Quellen in fester Reihenfolge
 ****************************************************************************************
 */

#include "audio/SongtextQuelle.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringDecoder>

namespace lumi::audio
{

namespace
{
using lumi::multieffect::lyrics::Songtext;

/// Textdatei lesen: UTF-8, sonst Latin-1 (alte .lrc-Sammlungen).
QString liesText(const QString& pfad)
{
    QFile f(pfad);
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) return QString();
    const QByteArray roh = f.readAll();
    QStringDecoder utf8(QStringDecoder::Utf8);
    const QString text = utf8.decode(roh);
    return utf8.hasError() ? QString::fromLatin1(roh) : text;
}

Songtext lese(const QString& text, const QString& quelle)
{
    if (text.trimmed().isEmpty()) return {};
    Songtext s = lumi::multieffect::lyrics::leseLrc(text.toStdString());
    if (!s.leer()) s.quelle = quelle.toStdString();
    return s;
}

/// Datei lesen und ihren Pfad am Ergebnis festhalten („Ordner oeffnen").
Songtext leseDatei(const QString& pfad, const QString& quelle)
{
    Songtext s = lese(liesText(pfad), quelle);
    if (!s.leer()) s.datei = QFileInfo(pfad).absoluteFilePath().toStdString();
    return s;
}
}  // namespace

QString songtextOrdner()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
           QStringLiteral("/lyrics");
}

QString songtextName(const TrackInfo& track)
{
    if (track.title.trimmed().isEmpty()) return QString();
    QString name = track.artist.trimmed().isEmpty()
                       ? track.title.trimmed()
                       : track.artist.trimmed() + QStringLiteral(" - ") + track.title.trimmed();
    static const QString kVerboten = QStringLiteral("<>:\"/\\|?*");
    for (QChar& c : name)
    {
        if (c.unicode() < 0x20 || kVerboten.contains(c)) c = QLatin1Char('_');
    }
    while (name.endsWith(QLatin1Char('.')) || name.endsWith(QLatin1Char(' '))) name.chop(1);
    return name;
}

Songtext findeSongtext(const TrackInfo& track, const QString& benutzerOrdner)
{
    if (track.filePath.isEmpty()) return {};
    const QFileInfo song(track.filePath);
    const QString neben = song.dir().absoluteFilePath(song.completeBaseName());
    const QString name = songtextName(track);
    const QString fern =
        name.isEmpty() || benutzerOrdner.isEmpty() ? QString() : benutzerOrdner + QLatin1Char('/') + name;

    // Jede Quelle wird hoechstens einmal gelesen; ein unsynchroner Fund wartet,
    // bis feststeht, dass keine synchrone Quelle dahinter kommt.
    Songtext unsynchron;
    const auto pruefe = [&](Songtext s) -> bool {
        if (s.leer()) return false;
        if (s.synchron)
        {
            unsynchron = std::move(s);  // als Ergebnis zurueckgeben
            return true;
        }
        if (unsynchron.leer()) unsynchron = std::move(s);
        return false;
    };

    // 1 — eingebettet synchron (SYLT; oder eingebetteter Text mit Zeitmarken)
    if (pruefe(lese(track.lyricsSynced, QStringLiteral("embedded (synced)"))))
        return unsynchron;
    Songtext eingebettet = lese(track.lyricsPlain, QStringLiteral("embedded"));
    if (eingebettet.synchron)
    {
        eingebettet.quelle = "embedded (synced)";
        return eingebettet;
    }
    // 2 — .lrc neben dem Song
    Songtext lrcNeben =
        leseDatei(neben + QStringLiteral(".lrc"), song.completeBaseName() + QStringLiteral(".lrc"));
    if (lrcNeben.synchron) return lrcNeben;
    // 3 — .lrc im Benutzerordner
    Songtext lrcFern;
    if (!fern.isEmpty())
    {
        lrcFern = leseDatei(fern + QStringLiteral(".lrc"),
                            QStringLiteral("user folder: ") + name + QStringLiteral(".lrc"));
        if (lrcFern.synchron) return lrcFern;
    }
    // 4 — eingebettet unsynchron
    if (!eingebettet.leer()) return eingebettet;
    // 5 — .txt neben dem Song, sonst die .lrc dort ohne Zeitmarken
    Songtext txt =
        leseDatei(neben + QStringLiteral(".txt"), song.completeBaseName() + QStringLiteral(".txt"));
    if (!txt.leer()) return txt;
    if (!lrcNeben.leer()) return lrcNeben;
    // 6 — .txt im Benutzerordner, sonst die .lrc dort ohne Zeitmarken
    if (!fern.isEmpty())
    {
        txt = leseDatei(fern + QStringLiteral(".txt"),
                        QStringLiteral("user folder: ") + name + QStringLiteral(".txt"));
        if (!txt.leer()) return txt;
        if (!lrcFern.leer()) return lrcFern;
    }
    return {};
}

bool speichereSongtext(const Songtext& text, const QString& pfad)
{
    if (pfad.isEmpty() || text.leer()) return false;
    if (!QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
    QSaveFile f(pfad);
    if (!f.open(QIODevice::WriteOnly)) return false;
    const std::string lrc = lumi::multieffect::lyrics::schreibeLrc(text);
    f.write(lrc.data(), static_cast<qint64>(lrc.size()));
    return f.commit();
}

}  // namespace lumi::audio
