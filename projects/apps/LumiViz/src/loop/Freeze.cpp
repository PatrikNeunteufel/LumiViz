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

#include "audio/WavDatei.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

#include <algorithm>
#include <atomic>
#include <map>
#include <mutex>
#include <set>

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
    clip.frozen.clear();  // die Marke klingt nicht (30f) — sonst aenderte das Einfrieren den Abdruck
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

// --- die Dateien (Stufe 30e) ----------------------------------------------------------

namespace
{

constexpr char kFrozenGrenze[] = "loop/frozenMB";

QString& frozenOrdnerImTest()
{
    static QString ordner;
    return ordner;
}

/// Heisst die Datei wie ein Abdruck (40 Hex-Zeichen, `.wav`)?
bool istFrozenName(const QString& name)
{
    static const QRegularExpression muster(QStringLiteral("^[0-9a-f]{40}\\.wav$"));
    return muster.match(name).hasMatch();
}

/// Die eingefrorenen Wellenformen im Ordner, die am laengsten nicht gespielte zuerst.
QFileInfoList frozenDateien()
{
    const QString ordner = frozenOrdner();
    if (ordner.isEmpty()) return {};
    QFileInfoList alle = QDir(ordner).entryInfoList({QStringLiteral("*.wav")}, QDir::Files, QDir::Time | QDir::Reversed);
    alle.erase(std::remove_if(alle.begin(), alle.end(), [](const QFileInfo& i) { return !istFrozenName(i.fileName()); }),
               alle.end());
    return alle;
}

}  // namespace

QString frozenOrdner()
{
    if (!frozenOrdnerImTest().isEmpty()) return frozenOrdnerImTest();
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return basis.isEmpty() ? QString() : QDir(basis).filePath(QStringLiteral("frozen"));
}

void setzeFrozenOrdnerFuerTests(const QString& ordner)
{
    frozenOrdnerImTest() = ordner;
}

QString frozenDatei(const QByteArray& abdruck)
{
    const QString ordner = frozenOrdner();
    const QString name = QString::fromLatin1(abdruck) + QStringLiteral(".wav");
    if (ordner.isEmpty() || !istFrozenName(name)) return {};
    return QDir(ordner).filePath(name);
}

namespace
{

/// Abdruecke, deren Datei sich nicht lesen liess (30f) — bis sie neu abgelegt werden.
std::mutex& unlesbarSperre()
{
    static std::mutex sperre;
    return sperre;
}

std::set<QByteArray>& unlesbar()
{
    static std::set<QByteArray> abdruecke;
    return abdruecke;
}

std::atomic<std::uint64_t>& frozenZaehler()
{
    static std::atomic<std::uint64_t> zaehler{0};
    return zaehler;
}

/// Liegt die Wellenform zum Abdruck vor — und liess sie sich bisher lesen?
bool liegtVor(const QByteArray& abdruck)
{
    {
        const std::lock_guard<std::mutex> halt(unlesbarSperre());
        if (unlesbar().contains(abdruck)) return false;
    }
    const QString datei = frozenDatei(abdruck);
    return !datei.isEmpty() && QFileInfo(datei).isFile();
}

}  // namespace

bool legeFrozen(const QByteArray& abdruck, const std::vector<float>& stereo, int rate)
{
    const QString datei = frozenDatei(abdruck);
    if (datei.isEmpty() || !QDir().mkpath(QFileInfo(datei).absolutePath())) return false;
    if (!audio::schreibeWav(datei, stereo, rate, audio::WavTiefe::Gleitkomma32).geschrieben) return false;
    {
        const std::lock_guard<std::mutex> halt(unlesbarSperre());
        unlesbar().erase(abdruck);
    }
    halteFrozenGrenze();
    ++frozenZaehler();
    return true;
}

std::uint64_t frozenFassung()
{
    return frozenZaehler().load();
}

std::optional<std::vector<float>> holeFrozen(const QByteArray& abdruck, int rate)
{
    const QString datei = frozenDatei(abdruck);
    if (datei.isEmpty() || !QFileInfo(datei).isFile()) return std::nullopt;
    std::optional<audio::WavInhalt> inhalt = audio::liesWav(datei);
    if (!inhalt || inhalt->rate != rate) return std::nullopt;
    // gerade gespielt: die Aenderungszeit ist der letzte Gebrauch
    QFile zeit(datei);
    if (zeit.open(QIODevice::ReadWrite)) (void)zeit.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileModificationTime);
    return std::move(inhalt->stereo);
}

FrozenStand frozenStand()
{
    FrozenStand stand;
    for (const QFileInfo& info : frozenDateien())
    {
        ++stand.dateien;
        stand.bytes += info.size();
    }
    return stand;
}

int leereFrozen()
{
    int geloescht = 0;
    for (const QFileInfo& info : frozenDateien())
        if (QFile::remove(info.absoluteFilePath())) ++geloescht;
    ++frozenZaehler();
    return geloescht;
}

void halteFrozenGrenze()
{
    halteFrozenGrenze(static_cast<qint64>(gemerkteFrozenGrenzeMb()) << 20);
}

void halteFrozenGrenze(qint64 grenze)
{
    const QFileInfoList alle = frozenDateien();
    qint64 summe = 0;
    for (const QFileInfo& info : alle) summe += info.size();
    for (const QFileInfo& info : alle)
    {
        if (summe <= grenze) break;
        if (QFile::remove(info.absoluteFilePath()))
        {
            summe -= info.size();
            ++frozenZaehler();
        }
    }
}

int gemerkteFrozenGrenzeMb()
{
    const int mb = QSettings().value(QLatin1String(kFrozenGrenze), kFrozenVorgabeMb).toInt();
    return std::clamp(mb, kFrozenMinMb, kFrozenMaxMb);
}

void merkeFrozenGrenzeMb(int mb)
{
    mb = std::clamp(mb, kFrozenMinMb, kFrozenMaxMb);
    QSettings einstellungen;
    if (mb == kFrozenVorgabeMb)
        einstellungen.remove(QLatin1String(kFrozenGrenze));
    else
        einstellungen.setValue(QLatin1String(kFrozenGrenze), mb);
    halteFrozenGrenze();
}

// --- abspielen (Stufe 30f) -------------------------------------------------------------

Eingefroren eingefrorenVon(const Loop& loop, int rate, const SaitenVon& saiten)
{
    Eingefroren eingefroren;
    for (std::size_t k = 0; k < loop.kanaele.size(); ++k)
        for (const LoopClip& clip : loop.kanaele[k].clips)
        {
            if (clip.frozen.isEmpty()) continue;
            const ClipOrt ort{static_cast<int>(k), clip.startTakt};
            // der jetzige Abdruck, nicht der gespeicherte (FV2 a)
            const QByteArray abdruck = clipAbdruck(loop, ort, rate, saiten);
            if (!liegtVor(abdruck)) continue;
            const QString datei = frozenDatei(abdruck);
            // gebraucht: die Aenderungszeit ist der letzte Gebrauch (auch wenn der Klang
            // dann aus dem Tonspeicher kommt)
            QFile zeit(datei);
            if (zeit.open(QIODevice::ReadWrite)) (void)zeit.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileModificationTime);
            eingefroren[{ort.kanal, ort.startTakt}] = QString::fromLatin1(abdruck);
        }
    return eingefroren;
}

void vergissFrozen(const QByteArray& abdruck)
{
    {
        const std::lock_guard<std::mutex> halt(unlesbarSperre());
        unlesbar().insert(abdruck);
    }
    if (const QString datei = frozenDatei(abdruck); !datei.isEmpty()) (void)QFile::remove(datei);
    ++frozenZaehler();
}

// --- bedienen (Stufe 30g) --------------------------------------------------------------

namespace
{

LoopClip* clipAn(Loop& loop, ClipOrt ort)
{
    const int nr = findeClip(loop, ort);
    return nr < 0 ? nullptr : &loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(nr)];
}

}  // namespace

FrozenZustand frozenZustand(const Loop& loop, ClipOrt ort, int rate, const SaitenVon& saiten)
{
    const int nr = findeClip(loop, ort);
    if (nr < 0) return FrozenZustand::Aus;
    const LoopClip& clip = loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(nr)];
    if (clip.frozen.isEmpty()) return FrozenZustand::Aus;
    const QByteArray jetzt = clipAbdruck(loop, ort, rate, saiten);
    if (liegtVor(jetzt)) return FrozenZustand::Frozen;
    return QString::fromLatin1(jetzt) == clip.frozen ? FrozenZustand::Fehlt : FrozenZustand::Veraendert;
}

std::vector<ClipOrt> setzeFrozenMarken(Loop& loop, const std::vector<ClipOrt>& orte, int rate, const SaitenVon& saiten)
{
    std::vector<ClipOrt> zuRechnen;
    for (const ClipOrt& ort : orte)
    {
        LoopClip* clip = clipAn(loop, ort);
        if (clip == nullptr) continue;
        // der Abdruck sieht die Marke nicht: es ist gleich, ob sie vorher schon stand
        const QByteArray jetzt = clipAbdruck(loop, ort, rate, saiten);
        if (jetzt.isEmpty()) continue;
        clip->frozen = QString::fromLatin1(jetzt);
        if (!liegtVor(jetzt)) zuRechnen.push_back(ort);
    }
    return zuRechnen;
}

std::vector<ClipOrt> frischeFrozenMarken(Loop& loop, int rate, const SaitenVon& saiten)
{
    std::vector<ClipOrt> markiert;
    for (std::size_t k = 0; k < loop.kanaele.size(); ++k)
        for (const LoopClip& clip : loop.kanaele[k].clips)
            if (!clip.frozen.isEmpty()) markiert.push_back(ClipOrt{static_cast<int>(k), clip.startTakt});
    return setzeFrozenMarken(loop, markiert, rate, saiten);
}

void loeseFrozenMarken(Loop& loop, const std::vector<ClipOrt>& orte)
{
    for (const ClipOrt& ort : orte)
        if (LoopClip* clip = clipAn(loop, ort)) clip->frozen.clear();
}

std::vector<ClipOrt> fehlendeFrozen(const Loop& loop, int rate, const SaitenVon& saiten)
{
    std::vector<ClipOrt> fehlend;
    for (std::size_t k = 0; k < loop.kanaele.size(); ++k)
        for (const LoopClip& clip : loop.kanaele[k].clips)
        {
            const ClipOrt ort{static_cast<int>(k), clip.startTakt};
            if (!clip.frozen.isEmpty() && frozenZustand(loop, ort, rate, saiten) == FrozenZustand::Fehlt)
                fehlend.push_back(ort);
        }
    return fehlend;
}

bool frozenFehlt(const QStringList& namen, const Klaenge& klaenge)
{
    for (qsizetype i = 0; i < namen.size(); ++i)
        if (!abdruckVonKlang(namen[i]).isEmpty() &&
            (static_cast<std::size_t>(i) >= klaenge.size() || klaenge[static_cast<std::size_t>(i)] == nullptr))
            return true;
    return false;
}

}  // namespace lumi::loop
