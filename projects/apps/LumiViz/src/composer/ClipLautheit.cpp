/**
 * @file ClipLautheit.cpp
 * @brief Composer: Lautheit eines Ton-Clips messen, Songs merken (§9g, G5-3 a)
 */

#include "composer/ClipLautheit.hpp"

#include "audio/IAudioEngine.hpp"
#include "loop/Freeze.hpp"      // eingefrorenVon, frozenFehlt (Stufe 30f)
#include "loop/Instrument.hpp"  // saitenAusInstrumenten (Stufe 23b)
#include "loop/Kit.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Offen.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>
#include <vector>

namespace lumi::composer
{

// =============================================================================
// Messen
// =============================================================================

std::optional<audio::Lautheit> messeSong(IAudioEngine& engine, const QString& datei, std::int64_t einsatzMs,
                                  std::int64_t dauerMs, int rate, const MessFortschritt& fortschritt)
{
    if (rate <= 0) return std::nullopt;
    audio::LautheitsMessung messung(rate);
    const double gesamt = dauerMs > 0 ? static_cast<double>(dauerMs) * rate / 1000.0 : 0.0;
    double bisher = 0.0;
    const bool fertig = engine.decodeStereoChunks(
        datei, rate, std::max<std::int64_t>(0, einsatzMs), std::max<std::int64_t>(0, dauerMs),
        [&](const float* stereo, std::size_t frames) {
            messung.fuettere(stereo, frames);
            bisher += static_cast<double>(frames);
            return !fortschritt || fortschritt(gesamt > 0.0 ? std::min(1.0, bisher / gesamt) : 0.0);
        });
    if (!fertig) return std::nullopt;
    return messung.ergebnis();
}

std::shared_ptr<const loop::Programm> bereiteLoopVor(const QString& datei, IAudioEngine& engine, int rate)
{
    if (rate <= 0) return nullptr;
    // wie die Tonfuehrung: was das Loop-Panel offen hat, auch ungespeichert
    loop::Loop geladen;
    if (!loop::offenerLoop(datei, geladen) && !loop::ladeLoop(datei, geladen)) return nullptr;
    geladen.richte();
    // die Saiten der Instrumente (Stufe 23b): dieselbe Auskunft fuer Namen und Programm
    const loop::SaitenVon saiten = loop::saitenAusInstrumenten();
    // eingefrorene Clips (Stufe 30f) wie in der Tonfuehrung; liess sich eine Wellenform nicht
    // laden, nennt `eingefrorenVon` ihren Clip danach nicht mehr — einmal neu, er klingt live
    loop::Eingefroren eingefroren = loop::eingefrorenVon(geladen, rate, saiten);
    QStringList namen = geladen.klangNamen(saiten, eingefroren);
    loop::Klaenge geladeneKlaenge = loop::ladeKlaenge(namen, engine, rate);
    if (loop::frozenFehlt(namen, geladeneKlaenge))
    {
        eingefroren = loop::eingefrorenVon(geladen, rate, saiten);
        namen = geladen.klangNamen(saiten, eingefroren);
        geladeneKlaenge = loop::ladeKlaenge(namen, engine, rate);
    }
    auto klaenge = std::make_shared<const loop::Klaenge>(std::move(geladeneKlaenge));
    auto programm = loop::baueProgramm(geladen, rate, std::move(klaenge), saiten, eingefroren);
    if (programm == nullptr || programm->ablauf.durchlaufFrames <= 0) return nullptr;
    return programm;
}

std::optional<audio::Lautheit> messeLoop(const std::shared_ptr<const loop::Programm>& programm, int rate,
                                  const MessFortschritt& fortschritt)
{
    if (programm == nullptr || rate <= 0 || programm->ablauf.durchlaufFrames <= 0) return std::nullopt;
    // ein eigener Klangerzeuger; das Programm hat `bereiteLoopVor` eigens
    // gebaut — sein Gedaechtnis der Effekte teilt es mit keiner Wiedergabe
    loop::Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(programm);
    erzeuger.setzeStelle(0);

    const std::int64_t durchlauf = programm->ablauf.durchlaufFrames;
    constexpr int kBlock = 1024;
    std::vector<float> block(2 * kBlock);
    audio::LautheitsMessung messung(rate);
    for (std::int64_t stelle = 0; stelle < 2 * durchlauf; stelle += kBlock)
    {
        const int frames = static_cast<int>(std::min<std::int64_t>(kBlock, 2 * durchlauf - stelle));
        erzeuger.render(block.data(), frames);
        // der erste Durchlauf klingt nur vor (L3b a); gemessen wird ab dem zweiten
        const std::int64_t abHier = std::max<std::int64_t>(0, durchlauf - stelle);
        if (abHier < frames)
            messung.fuettere(block.data() + 2 * abHier, static_cast<std::size_t>(frames - abHier));
        if (fortschritt && !fortschritt(static_cast<double>(stelle + frames) / static_cast<double>(2 * durchlauf)))
            return std::nullopt;
    }
    return messung.ergebnis();
}

// =============================================================================
// Gemerkte Songs
// =============================================================================

namespace
{
QString& dateiFuerTests()
{
    static QString datei;
    return datei;
}

constexpr int kFassung = 1;
}  // namespace

QString SongLautheiten::standardDatei()
{
    if (!dateiFuerTests().isEmpty()) return dateiFuerTests();
    const QString ordner = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (ordner.isEmpty()) return {};
    return QDir(ordner).filePath(QStringLiteral("composer_loudness.json"));
}

void SongLautheiten::setzeDateiFuerTests(const QString& datei)
{
    dateiFuerTests() = datei;
}

SongLautheiten::SongLautheiten() : SongLautheiten(standardDatei()) {}

SongLautheiten::SongLautheiten(QString datei) : m_datei(std::move(datei))
{
    QFile f(m_datei);
    if (m_datei.isEmpty() || !f.open(QIODevice::ReadOnly)) return;
    const QJsonObject wurzel = QJsonDocument::fromJson(f.readAll()).object();
    if (wurzel.value(QStringLiteral("version")).toInt() != kFassung) return;  // eine fremde Fassung: neu messen
    for (const QJsonValue& v : wurzel.value(QStringLiteral("songs")).toArray())
    {
        const QJsonObject o = v.toObject();
        audio::Lautheit l;
        l.lufs = o.value(QStringLiteral("lufs")).toDouble(audio::kLufsStille);
        l.spitzeDb = o.value(QStringLiteral("peak")).toDouble(audio::kLufsStille);
        l.stille = o.value(QStringLiteral("silent")).toBool(true);
        m_werte[o.value(QStringLiteral("key")).toString()] = l;
    }
}

QString SongLautheiten::schluessel(const QString& song, std::int64_t einsatzMs, std::int64_t dauerMs, int rate)
{
    const QFileInfo info(song);
    if (!info.exists()) return {};
    // Groesse und Aenderungszeit: eine ersetzte Datei wird neu gemessen
    return QStringLiteral("%1|%2|%3|%4|%5|%6")
        .arg(QDir::cleanPath(info.absoluteFilePath()))
        .arg(info.size())
        .arg(info.lastModified().toMSecsSinceEpoch())
        .arg(einsatzMs)
        .arg(dauerMs)
        .arg(rate);
}

std::optional<audio::Lautheit> SongLautheiten::hole(const QString& song, std::int64_t einsatzMs, std::int64_t dauerMs,
                                             int rate) const
{
    const QString s = schluessel(song, einsatzMs, dauerMs, rate);
    if (s.isEmpty()) return std::nullopt;
    const auto da = m_werte.find(s);
    if (da == m_werte.end()) return std::nullopt;
    return da->second;
}

void SongLautheiten::merke(const QString& song, std::int64_t einsatzMs, std::int64_t dauerMs, int rate,
                           const audio::Lautheit& wert)
{
    const QString s = schluessel(song, einsatzMs, dauerMs, rate);
    if (s.isEmpty()) return;
    m_werte[s] = wert;
    m_geaendert = true;
}

void SongLautheiten::vergiss(const QString& song)
{
    const QString anfang = QDir::cleanPath(QFileInfo(song).absoluteFilePath()) + QLatin1Char('|');
    for (auto it = m_werte.begin(); it != m_werte.end();)
    {
        if (it->first.startsWith(anfang))
        {
            it = m_werte.erase(it);
            m_geaendert = true;
        }
        else
            ++it;
    }
}

bool SongLautheiten::schreibe()
{
    if (!m_geaendert) return true;
    if (m_datei.isEmpty()) return false;
    QDir().mkpath(QFileInfo(m_datei).absolutePath());
    QJsonArray songs;
    for (const auto& [s, l] : m_werte)
    {
        QJsonObject o;
        o.insert(QStringLiteral("key"), s);
        o.insert(QStringLiteral("lufs"), l.lufs);
        o.insert(QStringLiteral("peak"), l.spitzeDb);
        o.insert(QStringLiteral("silent"), l.stille);
        songs.append(o);
    }
    QJsonObject wurzel;
    wurzel.insert(QStringLiteral("format"), QStringLiteral("LumiViz composer loudness cache"));
    wurzel.insert(QStringLiteral("version"), kFassung);
    wurzel.insert(QStringLiteral("songs"), songs);
    QSaveFile f(m_datei);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(wurzel).toJson(QJsonDocument::Indented));
    if (!f.commit()) return false;
    m_geaendert = false;
    return true;
}

}  // namespace lumi::composer
