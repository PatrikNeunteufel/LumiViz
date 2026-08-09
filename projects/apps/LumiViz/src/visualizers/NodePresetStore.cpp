/**
 ****************************************************************************************
 * @file   NodePresetStore.cpp
 * @brief  Umsetzung der Knoten-Voreinstellungen (s. NodePresetStore.hpp)
 ****************************************************************************************
 */

#include "visualizers/multieffect/NodePresetStore.hpp"

#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include <algorithm>

namespace lumi::multieffect::nodepresets {
namespace {

constexpr int kFormatVersion = 1;
const char* const kFormatTag = "lumiviz-nodepreset";

/// Test-Wurzel (leer = normales Verhalten). Nur ueber setRootForTesting gesetzt.
QString& testRoot()
{
    static QString root;
    return root;
}

/**
 * Wurzel der mitgelieferten Voreinstellungen — Aufwaertssuche ab dem Programmordner
 * wie bei den Format-Icons (PresetTypeIcons.hpp). Einmal aufgeloest und gemerkt;
 * leer, wenn `asset/nodepresets/README.md` nirgends liegt.
 */
QString assetRoot()
{
    if (!testRoot().isEmpty())
        return QDir(testRoot()).filePath(QStringLiteral("asset"));

    static const QString kRoot = [] {
        QDir dir(QCoreApplication::applicationDirPath());
        for (int i = 0; i < 12; ++i)
        {
            const QString candidate = dir.filePath(QStringLiteral("asset/nodepresets"));
            if (QFileInfo::exists(candidate + QStringLiteral("/README.md")))
                return candidate;
            if (!dir.cdUp()) break;
        }
        return QString();
    }();
    return kRoot;
}

/// Wurzel der Benutzer-Voreinstellungen (wird nicht angelegt).
QString userRoot()
{
    if (!testRoot().isEmpty())
        return QDir(testRoot()).filePath(QStringLiteral("user"));

    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) return QString();
    return QDir(base).filePath(QStringLiteral("nodepresets"));
}

/// Alle *.json eines Verzeichnisses als Eintraege.
void collect(const QString& dirPath, bool builtin, std::vector<Entry>& out)
{
    if (dirPath.isEmpty()) return;
    QDir dir(dirPath);
    if (!dir.exists()) return;
    const QStringList files =
        dir.entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name);
    for (const QString& file : files)
    {
        Entry e;
        e.name = QFileInfo(file).completeBaseName();
        e.path = dir.filePath(file);
        e.builtin = builtin;
        out.push_back(std::move(e));
    }
}

} // namespace

QString sanitize(const QString& name)
{
    QString out;
    out.reserve(name.size());
    for (const QChar c : name)
    {
        if (c.isLetterOrNumber() || c == QLatin1Char(' ') || c == QLatin1Char('_') ||
            c == QLatin1Char('-'))
            out.append(c);
        else
            out.append(QLatin1Char('_'));
    }
    out = out.trimmed();
    // Ein Name, der nur aus Ersatzzeichen besteht, ist kein Name.
    if (std::all_of(out.cbegin(), out.cend(),
                    [](QChar c) { return c == QLatin1Char('_'); }))
        return QString();
    return out;
}

QString userDir(const QString& typeKey)
{
    const QString root = userRoot();
    if (root.isEmpty() || typeKey.isEmpty()) return QString();
    return QDir(root).filePath(typeKey);
}

QString assetDir(const QString& typeKey)
{
    const QString root = assetRoot();
    if (root.isEmpty() || typeKey.isEmpty()) return QString();
    return QDir(root).filePath(typeKey);
}

std::vector<Entry> list(const QString& typeKey)
{
    std::vector<Entry> out;
    collect(assetDir(typeKey), true, out);
    collect(userDir(typeKey), false, out);

    // Benutzer schlaegt Asset: gleichnamige Asset-Eintraege fallen weg.
    std::vector<Entry> merged;
    merged.reserve(out.size());
    for (const Entry& e : out)
    {
        const auto same = [&e](const Entry& o) {
            return QString::compare(o.name, e.name, Qt::CaseInsensitive) == 0;
        };
        if (e.builtin && std::any_of(out.cbegin(), out.cend(), [&](const Entry& o) {
                return !o.builtin && same(o);
            }))
            continue;
        merged.push_back(e);
    }
    std::sort(merged.begin(), merged.end(), [](const Entry& a, const Entry& b) {
        return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
    });
    return merged;
}

bool load(const QString& path, const QString& expectKey, EffectParams& inOut,
          QStringList* report)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return false;

    const QJsonObject node = doc.object().value(QStringLiteral("node")).toObject();
    if (node.isEmpty()) return false;

    // Der Typ MUSS passen: sonst wechselt der Knoten beim Laden seine Art, und
    // ein unbekannter Schluessel wuerde ihn stillschweigend zum Passthrough machen.
    const QString key = node.value(QStringLiteral("type")).toString();
    if (key != expectKey)
    {
        if (report != nullptr)
            report->append(QStringLiteral("preset is for '%1', node is '%2'")
                               .arg(key, expectKey));
        return false;
    }

    // Merge statt Ersatz: die aktuellen Parameter als Grundlage, darueber die
    // Felder der Datei. Ein Teil-Preset (SuperScope-Figur: nur die EEL-Slots)
    // laesst damit Farbe/Breite/Blend des Knotens unangetastet.
    ChainNode base;
    base.params = inOut;
    QJsonObject merged = nodeToJson(base);
    for (auto it = node.constBegin(); it != node.constEnd(); ++it)
        merged[it.key()] = it.value();

    inOut = nodeFromJson(merged, report).params;
    return true;
}

QStringList fieldNames(const EffectParams& params)
{
    ChainNode tmp;
    tmp.params = params;
    QJsonObject node = nodeToJson(tmp);
    for (const char* frame : {"type", "name", "description", "enabled", "children"})
        node.remove(QLatin1String(frame));
    QStringList out = node.keys();
    out.sort(Qt::CaseInsensitive);
    return out;
}

bool save(const QString& name, const EffectParams& params, const QStringList& fields,
          QString* outPath)
{
    const QString clean = sanitize(name);
    const QString typeKey = effectTypeKey(params);
    if (clean.isEmpty() || typeKey.isEmpty()) return false;

    const QString dirPath = userDir(typeKey);
    if (dirPath.isEmpty()) return false;
    QDir dir;
    if (!dir.mkpath(dirPath)) return false;

    // Nur die params: der Knoten-Rahmen (Name, Beschreibung, Kinder, enabled)
    // gehoert nicht in eine Voreinstellung (Konzept §8.2).
    ChainNode tmp;
    tmp.params = params;
    QJsonObject node = nodeToJson(tmp);
    node.remove(QStringLiteral("name"));
    node.remove(QStringLiteral("description"));
    node.remove(QStringLiteral("enabled"));
    node.remove(QStringLiteral("children"));

    // Abgewaehlte Felder fallen raus — `type` bleibt IMMER (der Typwaechter
    // beim Laden haengt daran). Leere Auswahl = alles.
    if (!fields.isEmpty())
    {
        for (const QString& key : node.keys())
        {
            if (key == QStringLiteral("type")) continue;
            if (!fields.contains(key)) node.remove(key);
        }
    }

    QJsonObject root;
    root[QStringLiteral("format")] = QLatin1String(kFormatTag);
    root[QStringLiteral("formatVersion")] = kFormatVersion;
    root[QStringLiteral("node")] = node;

    const QString path = QDir(dirPath).filePath(clean + QStringLiteral(".json"));
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    const qint64 written =
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    if (written <= 0) return false;

    if (outPath != nullptr) *outPath = path;
    return true;
}

bool remove(const Entry& entry)
{
    if (entry.builtin || entry.path.isEmpty()) return false;
    return QFile::remove(entry.path);
}

void setRootForTesting(const QString& root)
{
    testRoot() = root;
}

} // namespace lumi::multieffect::nodepresets
