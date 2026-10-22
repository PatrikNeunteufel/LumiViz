/**
 * @file Freeze.cpp
 * @brief Loop: Freeze — der Fingerabdruck eines Clips (Stufe 30d; Dateien ueber ihren Inhalt seit FZ2 a)
 */

#include "loop/Freeze.hpp"

#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Sample.hpp"
#include "loop/Wellenform.hpp"
#include "shape/Kontur.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

#include <map>
#include <mutex>

namespace lumi::loop
{

namespace
{

/// Namen, die nicht klingen, werden fest — so teilen gleiche Clips einen Abdruck.
constexpr char kClipName[] = "clip";
constexpr char kPlatzName[] = "~tail";

/// Der Inhalts-Hash einer Datei, gemerkt zu Ort, Groesse und Aenderungszeit (FZ2 a): gelesen
/// wird nur, wenn sich davon etwas geaendert hat. Von jedem Thread aus benutzbar.
struct DateiStand
{
    qint64 groesse = -1;
    qint64 zeit = -1;
    QByteArray hash;
};

std::mutex& inhaltSperre()
{
    static std::mutex sperre;
    return sperre;
}

std::map<QString, DateiStand>& inhalte()
{
    static std::map<QString, DateiStand> gemerkt;
    return gemerkt;
}

/// Leer = keine Datei.
QByteArray inhaltVon(const QString& datei)
{
    const QFileInfo info(datei);
    if (datei.isEmpty() || !info.isFile()) return {};
    const QString pfad = QDir::cleanPath(info.absoluteFilePath());
    const qint64 groesse = info.size();
    const qint64 zeit = info.lastModified().toMSecsSinceEpoch();
    {
        const std::lock_guard<std::mutex> halt(inhaltSperre());
        const auto ort = inhalte().find(pfad);
        if (ort != inhalte().end() && ort->second.groesse == groesse && ort->second.zeit == zeit) return ort->second.hash;
    }
    QFile lesen(pfad);
    if (!lesen.open(QIODevice::ReadOnly)) return {};
    QCryptographicHash hash(QCryptographicHash::Sha1);
    if (!hash.addData(&lesen)) return {};
    const QByteArray ergebnis = hash.result().toHex();
    const std::lock_guard<std::mutex> halt(inhaltSperre());
    inhalte()[pfad] = DateiStand{groesse, zeit, ergebnis};
    return ergebnis;
}

/// Ein Verweis auf eine Datei, ersetzt durch ihren Inhalt; fehlt sie, bleibt die Nennung.
QString verweis(const QString& nennung, const QString& datei)
{
    const QByteArray hash = inhaltVon(datei);
    return hash.isEmpty() ? QStringLiteral("missing:") + nennung : QStringLiteral("content:") + QString::fromLatin1(hash);
}

/// Je Instrument einmal: es selbst mit seinen Samples und Zeichnungen, aus dem Inhalt.
class Instrumente
{
public:
    QString inhalt(const QString& nennung)
    {
        const auto [ort, neu] = m_gemerkt.try_emplace(nennung);
        if (!neu) return ort->second;
        Instrument instrument;
        if (!holeInstrument(nennung, instrument))
        {
            ort->second = QStringLiteral("missing:") + nennung;
            return ort->second;
        }
        for (Schicht& schicht : instrument.schichten)
        {
            if (schicht.schwingung == Schwingung::Sample) schicht.sample = verweis(schicht.sample, sampleDatei(schicht.sample));
            if (schicht.schwingung == Schwingung::Form)
            {
                shape::Zeichnung zeichnung;
                schicht.form = shape::holeZeichnung(schicht.form, zeichnung)
                                   ? QStringLiteral("shape:") +
                                         QString::fromLatin1(QCryptographicHash::hash(
                                                                 QJsonDocument(shape::alsJson(zeichnung)).toJson(QJsonDocument::Compact),
                                                                 QCryptographicHash::Sha1)
                                                                 .toHex())
                                   : QStringLiteral("missing:") + schicht.form;
            }
        }
        ort->second = QStringLiteral("instrument:") +
                      QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(alsJson(instrument)).toJson(QJsonDocument::Compact),
                                                                   QCryptographicHash::Sha1)
                                              .toHex());
        return ort->second;
    }

private:
    std::map<QString, QString> m_gemerkt;
};

}  // namespace

std::optional<Loop> abdruckFassung(const Loop& loop, ClipOrt ort)
{
    const int nr = findeClip(loop, ort);
    if (nr < 0) return std::nullopt;
    // die Stelle im Loop klingt nicht: der Clip allein auf Takt 0
    Loop kopie = loop;
    LoopClip clip = loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(nr)];
    clip.startTakt = 0;
    kopie.kanaele[static_cast<std::size_t>(ort.kanal)].clips = {clip};
    std::optional<Loop> fassung = clipFassung(kopie, ClipOrt{ort.kanal, 0});
    if (!fassung || fassung->muster.size() != 2 || fassung->kanaele.size() != 2) return std::nullopt;

    // was nicht klingt, faellt weg oder wird fest
    fassung->tags = {};
    fassung->limiter = true;
    Muster& muster = fassung->muster[0];
    muster.name = QLatin1String(kClipName);
    muster.kit.clear();
    Kanal& eigener = fassung->kanaele[0];
    eigener.name.clear();
    eigener.clips.front().muster = muster.name;
    fassung->muster[1].name = QLatin1String(kPlatzName);
    Kanal& platz = fassung->kanaele[1];
    platz.name.clear();
    platz.clips.front().muster = fassung->muster[1].name;
    return fassung;
}

QByteArray clipAbdruck(const Loop& loop, ClipOrt ort, int rate, const SaitenVon& saiten)
{
    const std::optional<Loop> fassung = abdruckFassung(loop, ort);
    if (!fassung || rate <= 0) return {};
    const QStringList namen = fassung->klangNamen(saiten);  // mit den Nennungen: zum Nachschlagen

    // jeder Verweis auf eine Datei wird ihr Inhalt (FZ2 a)
    Instrumente instrumente;
    Loop inhalt = *fassung;
    for (Muster& muster : inhalt.muster)
    {
        if (!muster.instrument.isEmpty()) muster.instrument = instrumente.inhalt(muster.instrument);
        for (Spur& spur : muster.spuren)
        {
            if (!spur.instrument.isEmpty())
                spur.instrument = instrumente.inhalt(spur.instrument);
            else if (!spur.klang.isEmpty())
                spur.klang = verweis(spur.klang, klangDatei(spur.klang));
        }
    }

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(QByteArrayView("lumiviz-freeze|"));
    hash.addData(QByteArray::number(kKlangfassung) + '|' + QByteArray::number(rate) + '|');
    hash.addData(QJsonDocument(alsJson(inhalt)).toJson(QJsonDocument::Compact));
    for (const QString& name : namen)
    {
        hash.addData(QByteArrayView("|"));
        if (istTon(name))
        {
            TonAngabe ton;
            if (!zerlegeTon(name, ton))
            {
                hash.addData(name.toUtf8());
                continue;
            }
            ton.instrument = instrumente.inhalt(ton.instrument);
            hash.addData(tonKlang(ton).toUtf8());
        }
        else
            hash.addData(verweis(name, klangDatei(name)).toUtf8());
    }
    return hash.result().toHex();
}

}  // namespace lumi::loop
