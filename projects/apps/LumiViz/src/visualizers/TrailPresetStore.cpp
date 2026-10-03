/**
 ****************************************************************************************
 * @file   TrailPresetStore.cpp
 * @brief  Trail-Presets der Karaoke-Ebene: die Trail-Liste samt Kindern als Datei
 ****************************************************************************************
 */

#include "visualizers/multieffect/TrailPresetStore.hpp"

#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QStandardPaths>

#include <algorithm>
#include <functional>

namespace lumi::multieffect::trailpresets {

namespace
{
/// Test-Wurzel (leer = normales Verhalten). Nur ueber setRootForTesting gesetzt.
QString& testRoot()
{
    static QString root;
    return root;
}

/// Alle *.lvtrail eines Verzeichnisses als Eintraege, nach Namen sortiert.
void collect(const QString& dirPath, bool builtin, std::vector<Entry>& out)
{
    if (dirPath.isEmpty()) return;
    const QDir dir(dirPath);
    if (!dir.exists()) return;
    const QStringList files = dir.entryList(
        {QLatin1Char('*') + QLatin1String(kEndung)}, QDir::Files, QDir::Name | QDir::IgnoreCase);
    for (const QString& file : files)
    {
        Entry e;
        e.name = QFileInfo(file).completeBaseName();
        e.path = dir.filePath(file);
        e.builtin = builtin;
        out.push_back(std::move(e));
    }
}

void ohneIds(ChainNode& n)
{
    n.nodeId = 0;
    for (ChainNode& kind : n.children) ohneIds(kind);
}
}  // namespace

QString assetDir()
{
    if (!testRoot().isEmpty()) return QDir(testRoot()).filePath(QStringLiteral("asset"));

    // Aufwaertssuche ab dem Programmordner wie bei den Knoten-Voreinstellungen;
    // einmal aufgeloest und gemerkt. Anker ist die README des Ordners.
    static const QString kDir = [] {
        QDir dir(QCoreApplication::applicationDirPath());
        for (int i = 0; i < 12; ++i)
        {
            const QString candidate = dir.filePath(QStringLiteral("asset/karaoke/trails"));
            if (QFileInfo::exists(candidate + QStringLiteral("/README.md"))) return candidate;
            if (!dir.cdUp()) break;
        }
        return QString();
    }();
    return kDir;
}

QString userDir()
{
    if (!testRoot().isEmpty()) return QDir(testRoot()).filePath(QStringLiteral("user"));

    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) return QString();
    return QDir(base).filePath(QStringLiteral("karaoke/trails"));
}

std::vector<Entry> list()
{
    std::vector<Entry> eigene;
    collect(userDir(), false, eigene);
    std::vector<Entry> out;
    collect(assetDir(), true, out);
    // ein eigenes mit dem Namen eines mitgelieferten verdraengt es
    out.erase(std::remove_if(out.begin(), out.end(),
                             [&eigene](const Entry& e) {
                                 return std::any_of(eigene.cbegin(), eigene.cend(),
                                                    [&e](const Entry& o) {
                                                        return QString::compare(
                                                                   o.name, e.name,
                                                                   Qt::CaseInsensitive) == 0;
                                                    });
                             }),
              out.end());
    out.insert(out.end(), eigene.begin(), eigene.end());
    return out;
}

Entry finde(const QString& name)
{
    for (const Entry& e : list())
        if (QString::compare(e.name, name, Qt::CaseInsensitive) == 0) return e;
    return Entry{};
}

ChainNode fuerEbene(ChainNode liste)
{
    if (auto* lp = std::get_if<ListParams>(&liste.params))
    {
        lp->blendIn = BlendMode::Ignore;
        lp->blendOut = BlendMode::Ignore;
    }
    liste.displayName = "Karaoke Trail";
    // „Trail on" (das Auge der Liste) gehoert nicht zum Preset: in der Datei
    // steht die Liste immer an, die Ebene behaelt beim Laden ihren Schalter
    liste.enabled = true;
    // genau EIN Lyrics Trail, und der am Ende
    const auto istTrail = [](const ChainNode& n) {
        return std::holds_alternative<LyricsTrailParams>(n.params);
    };
    std::vector<ChainNode> trails;
    for (const ChainNode& kind : liste.children)
        if (istTrail(kind)) trails.push_back(kind);
    liste.children.erase(std::remove_if(liste.children.begin(), liste.children.end(), istTrail),
                         liste.children.end());
    if (trails.empty())
    {
        // die Vorgaben der Ebene (MultiEffectVisualizer::baueKaraokeEbene)
        ChainNode trail;
        LyricsTrailParams tp;
        tp.holdMs = 120;
        tp.yShift = -7;
        tp.blend = 1;
        trail.params = tp;
        trails.push_back(std::move(trail));
    }
    liste.children.push_back(trails.front());
    ohneIds(liste);
    return liste;
}

ChainNode fuerKette(ChainNode liste, const QString& name)
{
    if (auto* lp = std::get_if<ListParams>(&liste.params))
    {
        lp->blendIn = BlendMode::Ignore;
        lp->blendOut = BlendMode::Additive;
    }
    liste.displayName = name.toStdString();
    liste.enabled = true;
    ohneIds(liste);
    return liste;
}

bool gleich(const ChainNode& a, const ChainNode& b)
{
    return nodeToJson(fuerEbene(a)) == nodeToJson(fuerEbene(b));
}

bool load(const QString& path, ChainNode& outListe, QStringList* report)
{
    ChainNode geladen;
    if (!loadChainFromFile(path, geladen, report)) return false;
    if (!std::holds_alternative<ListParams>(geladen.params))
    {
        if (report != nullptr)
            report->append(QStringLiteral("not a trail preset (expected an Effect List)"));
        return false;
    }
    outListe = fuerEbene(std::move(geladen));
    return true;
}

bool save(const QString& name, const ChainNode& liste, QString* outPath)
{
    const QString sauber = nodepresets::sanitize(name);
    const QString ordner = userDir();
    if (sauber.isEmpty() || ordner.isEmpty()) return false;
    if (!std::holds_alternative<ListParams>(liste.params)) return false;
    if (!QDir().mkpath(ordner)) return false;
    const QString pfad = QDir(ordner).filePath(sauber + QLatin1String(kEndung));
    if (!saveChainToFile(fuerEbene(liste), pfad)) return false;
    if (outPath != nullptr) *outPath = pfad;
    return true;
}

bool remove(const Entry& entry)
{
    if (entry.builtin || entry.path.isEmpty()) return false;
    // nur, was wirklich im Benutzerordner liegt
    const QString ordner = QDir::cleanPath(userDir());
    if (ordner.isEmpty() ||
        QDir::cleanPath(QFileInfo(entry.path).absolutePath())
                .compare(ordner, Qt::CaseInsensitive) != 0)
        return false;
    return QFile::remove(entry.path);
}

void setRootForTesting(const QString& root)
{
    testRoot() = root;
}

}  // namespace lumi::multieffect::trailpresets
