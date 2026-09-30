/**
 ****************************************************************************************
 * @file   ShaderVorlagen.cpp
 * @brief  Umsetzung der Shader-Vorlagen (s. ShaderVorlagen.hpp)
 ****************************************************************************************
 */

#include "visualizers/multieffect/ShaderVorlagen.hpp"

#include "visualizers/multieffect/NodePresetStore.hpp"  // sanitize: dieselbe Namensregel

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include <algorithm>

namespace lumi::multieffect::shadervorlagen {
namespace {

QString& testRoot()
{
    static QString root;
    return root;
}

/**
 * Wurzel der mitgelieferten Vorlagen — Aufwaertssuche ab dem Programmordner wie
 * bei den Knoten-Voreinstellungen; Anker ist `asset/shadertemplates/README.md`.
 */
QString assetRoot()
{
    if (!testRoot().isEmpty()) return QDir(testRoot()).filePath(QStringLiteral("asset"));

    static const QString kRoot = [] {
        QDir dir(QCoreApplication::applicationDirPath());
        for (int i = 0; i < 12; ++i)
        {
            const QString candidate = dir.filePath(QStringLiteral("asset/shadertemplates"));
            if (QFileInfo::exists(candidate + QStringLiteral("/README.md"))) return candidate;
            if (!dir.cdUp()) break;
        }
        return QString();
    }();
    return kRoot;
}

QString userRoot()
{
    if (!testRoot().isEmpty()) return QDir(testRoot()).filePath(QStringLiteral("user"));
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) return QString();
    return QDir(base).filePath(QStringLiteral("shadertemplates"));
}

bool bekannt(const QString& key) { return schluessel().contains(key); }

/// Dateien eines Ordners mit der Endung des Schluessels einsammeln.
void sammle(const QString& dirPath, const QString& key, bool builtin,
            std::vector<Eintrag>& out)
{
    if (dirPath.isEmpty()) return;
    const QDir dir(dirPath);
    const QStringList files = dir.entryList({QStringLiteral("*.") + endung(key)},
                                            QDir::Files, QDir::Name);
    for (const QString& f : files)
    {
        QFile file(dir.filePath(f));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        Eintrag e;
        e.name = QFileInfo(f).completeBaseName();
        e.text = QString::fromUtf8(file.readAll());
        e.pfad = file.fileName();
        e.builtin = builtin;
        out.push_back(std::move(e));
    }
}

}  // namespace

const QStringList& schluessel()
{
    static const QStringList kKeys = {
        QStringLiteral("shadertoy"),     QStringLiteral("meshwarp"),
        QStringLiteral("pixelfilter"),   QStringLiteral("gpuparticles"),
        QStringLiteral("isffilter"),     QStringLiteral("isffilter/vs"),
        QStringLiteral("milkdrop/warp"), QStringLiteral("milkdrop/comp"),
    };
    return kKeys;
}

QString leererRumpf(const QString& key)
{
    if (key == QLatin1String("shadertoy"))
        return QStringLiteral(
            "void mainImage(out vec4 fragColor, in vec2 fragCoord)\n"
            "{\n"
            "    vec2 uv = fragCoord / iResolution.xy;\n"
            "    fragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
            "}\n");
    if (key == QLatin1String("meshwarp"))
        return QStringLiteral(
            "vec2 warp(vec2 uv)\n"
            "{\n"
            "    return uv;\n"
            "}\n");
    if (key == QLatin1String("pixelfilter"))
        return QStringLiteral(
            "vec4 farbe(vec2 uv, vec4 src)\n"
            "{\n"
            "    return src;\n"
            "}\n");
    if (key == QLatin1String("gpuparticles"))
        return QStringLiteral(
            "vec2 kraft(vec2 pos, vec2 vel, float alter)\n"
            "{\n"
            "    return vec2(0.0);\n"
            "}\n");
    // ISF: Quellen und Regler kommen ueber den Import in den Knoten, nicht
    // ueber den Code — ein frischer Knoten hat KEIN inputImage. Der Rumpf
    // benutzt deshalb nur, was der Wrapper immer bereitstellt.
    if (key == QLatin1String("isffilter"))
        return QStringLiteral(
            "void main()\n"
            "{\n"
            "    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
            "}\n");
    if (key == QLatin1String("isffilter/vs"))
        return QStringLiteral(
            "void main()\n"
            "{\n"
            "    isf_vertShaderInit();\n"
            "}\n");
    // MilkDrop: Durchreichen des aktuellen Bilds — die Vorgabe des Originals.
    if (key == QLatin1String("milkdrop/warp") || key == QLatin1String("milkdrop/comp"))
        return QStringLiteral(
            "shader_body\n"
            "{\n"
            "    ret = tex2D(sampler_main, uv).xyz;\n"
            "}\n");
    return {};
}

QString endung(const QString& key)
{
    if (key.startsWith(QLatin1String("milkdrop"))) return QStringLiteral("hlsl");
    if (key == QLatin1String("isffilter")) return QStringLiteral("fs");
    if (key == QLatin1String("isffilter/vs")) return QStringLiteral("vs");
    return QStringLiteral("glsl");
}

QString assetDir(const QString& key)
{
    const QString root = assetRoot();
    if (root.isEmpty() || !bekannt(key)) return QString();
    return QDir(root).filePath(key);
}

QString userDir(const QString& key)
{
    const QString root = userRoot();
    if (root.isEmpty() || !bekannt(key)) return QString();
    return QDir(root).filePath(key);
}

std::vector<Eintrag> liste(const QString& key)
{
    std::vector<Eintrag> out;
    if (!bekannt(key)) return out;

    Eintrag leer;
    leer.name = QStringLiteral("Empty");  // Namen englisch (Vorgabe Patrik S83)
    leer.text = leererRumpf(key);
    out.push_back(std::move(leer));

    std::vector<Eintrag> dateien;
    sammle(assetDir(key), key, true, dateien);
    std::vector<Eintrag> eigene;
    sammle(userDir(key), key, false, eigene);
    // Eigene verdraengen gleichnamige mitgelieferte.
    for (const Eintrag& e : eigene)
        std::erase_if(dateien, [&](const Eintrag& d) { return d.name == e.name; });
    dateien.insert(dateien.end(), eigene.begin(), eigene.end());
    std::sort(dateien.begin(), dateien.end(), [](const Eintrag& a, const Eintrag& b) {
        return QString::localeAwareCompare(a.name, b.name) < 0;
    });
    out.insert(out.end(), dateien.begin(), dateien.end());
    return out;
}

bool speichere(const QString& key, const QString& name, const QString& text, QString* outPfad)
{
    const QString dirPath = userDir(key);
    const QString sauber = nodepresets::sanitize(name);
    if (dirPath.isEmpty() || sauber.isEmpty()) return false;
    if (!QDir().mkpath(dirPath)) return false;
    QFile f(QDir(dirPath).filePath(sauber + QLatin1Char('.') + endung(key)));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return false;
    f.write(text.toUtf8());
    if (outPfad != nullptr) *outPfad = f.fileName();
    return true;
}

void setRootForTesting(const QString& root) { testRoot() = root; }

}  // namespace lumi::multieffect::shadervorlagen
