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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringDecoder>
#include <QStringList>

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

bool songtextPasst(const Songtext& text, const TrackInfo& track)
{
    if (text.laengeMs <= 0 || track.durationMs <= 0) return true;
    const std::int64_t diff = text.laengeMs - static_cast<std::int64_t>(track.durationMs);
    return diff >= -2500 && diff <= 2500;
}

namespace
{
/// Name der Fassung: `<Interpret> - <Titel> [m-ss]`.
QString fassungsName(const QString& name, const TrackInfo& track)
{
    const int sek = (track.durationMs + 500) / 1000;
    return QStringLiteral("%1 [%2-%3]").arg(name).arg(sek / 60).arg(sek % 60, 2, 10, QLatin1Char('0'));
}
}  // namespace

QString songtextBenutzerBasis(const TrackInfo& track, const QString& benutzerOrdner)
{
    const QString name = songtextName(track);
    if (name.isEmpty() || benutzerOrdner.isEmpty()) return QString();
    const QString basis = benutzerOrdner + QLatin1Char('/') + name;
    // Der gemeinsame Name ist von einer ANDEREN Fassung belegt (Dauer in der
    // Datei passt nicht): diese bekommt ihren eigenen Namen.
    for (const char* endung : {".lrc", ".txt"})
    {
        const Songtext s = lese(liesText(basis + QLatin1String(endung)), QString());
        if (!s.leer() && !songtextPasst(s, track))
            return benutzerOrdner + QLatin1Char('/') + fassungsName(name, track);
    }
    return basis;
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
    // Benutzerordner (S87): der gemeinsame Name `<Interpret> - <Titel>` gilt
    // nur, wenn seine Dauerangabe zu diesem Titel passt (oder fehlt); sonst
    // die Datei dieser Fassung `… [m-ss]`. Zwei Fassungen eines Songs
    // ueberschrieben sich sonst gegenseitig (Befund Patrik: „Gettin' Over"
    // auf zwei Alben, Titel nur in feat./Feat. verschieden).
    const auto ausBenutzerordner = [&](const QString& endung) -> Songtext {
        for (const QString& stamm : {name, fassungsName(name, track)})
        {
            Songtext s = leseDatei(benutzerOrdner + QLatin1Char('/') + stamm + endung,
                                   QStringLiteral("user folder: ") + stamm + endung);
            if (!s.leer() && songtextPasst(s, track)) return s;
        }
        return {};
    };
    // 3 — .lrc im Benutzerordner
    Songtext lrcFern;
    if (!fern.isEmpty())
    {
        lrcFern = ausBenutzerordner(QStringLiteral(".lrc"));
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
        txt = ausBenutzerordner(QStringLiteral(".txt"));
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

SongtextZuweisung weiseSongtextZu(const TrackInfo& track, const QString& quellPfad,
                                  const QString& benutzerOrdner)
{
    SongtextZuweisung z;
    const Songtext gelesen = lese(liesText(quellPfad), QString());
    if (gelesen.leer()) return z;  // nichts Lesbares: nichts anfassen
    const QString basis = songtextBenutzerBasis(track, benutzerOrdner);
    if (basis.isEmpty() || track.filePath.isEmpty()) return z;
    const QString endung = gelesen.synchron ? QStringLiteral(".lrc") : QStringLiteral(".txt");
    const QString ziel = basis + endung;

    // unveraendert kopieren (Kopfzeilen und Kodierung bleiben, wie sie sind);
    // liegt die Datei schon an ihrem Platz, ist nichts zu tun
    if (QFileInfo(quellPfad).absoluteFilePath() != QFileInfo(ziel).absoluteFilePath())
    {
        if (!QDir().mkpath(QFileInfo(ziel).absolutePath())) return z;
        if (QFile::exists(ziel) && !QFile::remove(ziel)) return z;
        if (!QFile::copy(quellPfad, ziel)) return z;
    }
    // die fruehere Fassung mit der anderen Endung: weg, wenn LumiViz sie geholt
    // hat (Herkunftsvermerk) — eine eigene Datei bleibt liegen
    const QString andere = basis + (gelesen.synchron ? QStringLiteral(".txt")
                                                     : QStringLiteral(".lrc"));
    if (QFile::exists(andere) && !lese(liesText(andere), QString()).herkunft.empty())
        QFile::remove(andere);

    z.ok = true;
    z.ziel = QFileInfo(ziel).absoluteFilePath();
    z.gilt = findeSongtext(track, benutzerOrdner);
    z.verdeckt = QString::fromStdString(z.gilt.datei) != z.ziel;
    return z;
}

QString songtextSchluessel(const TrackInfo& track)
{
    return QStringList{track.artist.simplified().toLower(), track.title.simplified().toLower(),
                       track.album.simplified().toLower(),
                       QString::number((track.durationMs + 500) / 1000)}
        .join(QLatin1Char('|'));
}

namespace
{
QString versatzDatei(const QString& benutzerOrdner)
{
    return benutzerOrdner + QStringLiteral("/offsets.json");
}

QJsonArray leseVersaetze(const QString& benutzerOrdner)
{
    QFile f(versatzDatei(benutzerOrdner));
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(f.readAll()).array();
}
}  // namespace

int songtextTitelVersatzMs(const TrackInfo& track, const QString& benutzerOrdner)
{
    if (track.title.trimmed().isEmpty()) return 0;
    const QString schluessel = songtextSchluessel(track);
    for (const QJsonValue& v : leseVersaetze(benutzerOrdner))
    {
        const QJsonObject o = v.toObject();
        if (o.value(QStringLiteral("key")).toString() == schluessel)
            return o.value(QStringLiteral("offsetMs")).toInt();
    }
    return 0;
}

bool setzeSongtextTitelVersatz(const TrackInfo& track, int ms, const QString& benutzerOrdner)
{
    if (track.title.trimmed().isEmpty() || benutzerOrdner.isEmpty()) return false;
    const QString schluessel = songtextSchluessel(track);
    QJsonArray neu;
    for (const QJsonValue& v : leseVersaetze(benutzerOrdner))
        if (v.toObject().value(QStringLiteral("key")).toString() != schluessel) neu.append(v);
    if (ms != 0)
        neu.append(QJsonObject{{QStringLiteral("key"), schluessel},
                               {QStringLiteral("title"), songtextName(track)},
                               {QStringLiteral("offsetMs"), ms}});
    if (neu.isEmpty())  // nichts mehr festzuhalten: keine leere Datei liegen lassen
        return !QFile::exists(versatzDatei(benutzerOrdner)) ||
               QFile::remove(versatzDatei(benutzerOrdner));
    if (!QDir().mkpath(benutzerOrdner)) return false;
    QSaveFile f(versatzDatei(benutzerOrdner));
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(neu).toJson(QJsonDocument::Indented));
    return f.commit();
}

}  // namespace lumi::audio
