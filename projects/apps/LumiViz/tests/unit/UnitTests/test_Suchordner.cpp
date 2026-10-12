/**
 * @file test_Suchordner.cpp
 * @brief Die Liste der Suchordner fuer Bilder und Texturen importierter Presets
 *        (core/Suchordner.hpp, Wunsch Patrik S116) — die Liste in den
 *        Einstellungen, die Suche einer Datei, und was MilkDrop daraus macht
 *
 * Ohne Fenster und ohne App: eigene Einstellungen (`LumiVizTest` /
 * `Suchordner`), Wegwerf-Ordner.
 */

#include <doctest.h>

#include "core/Suchordner.hpp"
#include "visualizers/milkdrop/MilkdropTextureResolve.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QTemporaryDir>

namespace
{

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("Suchordner"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

QString datei(const QTemporaryDir& tmp, const QString& rel)
{
    const QString pfad = QDir(tmp.path()).filePath(rel);
    REQUIRE(QDir().mkpath(QFileInfo(pfad).absolutePath()));
    QFile f(pfad);
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write("x");
    return QDir::cleanPath(pfad);
}

}  // namespace

TEST_SUITE("Suchordner")
{
    TEST_CASE("Suchordner: die Liste in den Einstellungen — der alte einzelne Ordner gilt, bis sie geschrieben wird")
    {
        namespace so = lumi::core::suchordner;
        const EigeneEinstellungen einstellungen;

        CHECK(so::liste().isEmpty());
        // der Ordner von frueher (`import/imageSearchDir`) ist der erste Eintrag
        QSettings().setValue(QLatin1String(so::kSchluesselAlt), QStringLiteral("D:\\Bilder\\avs"));
        CHECK(so::liste() == QStringList{QStringLiteral("D:/Bilder/avs")});

        // setzen: ohne Leeres, ohne Doppelte (auch anders geschrieben), in der Reihenfolge
        so::setzeListe({QStringLiteral("D:/Bilder/avs"), QString(), QStringLiteral("  "), QStringLiteral("E:/Milk/"),
                        QStringLiteral("d:\\bilder\\AVS"), QStringLiteral("E:/Milk/textures")});
        CHECK(so::liste() == QStringList{QStringLiteral("D:/Bilder/avs"), QStringLiteral("E:/Milk"),
                                         QStringLiteral("E:/Milk/textures")});
        CHECK_FALSE(QSettings().contains(QLatin1String(so::kSchluesselAlt)));  // der alte Schluessel ist weg

        // eine leere Liste bleibt leer — der alte Ordner kommt nicht zurueck
        QSettings().setValue(QLatin1String(so::kSchluesselAlt), QStringLiteral("D:/alt"));
        so::setzeListe({});
        CHECK(so::liste().isEmpty());
    }

    TEST_CASE("Suchordner: eine Datei finden — beim genannten Namen oder nur beim Dateinamen, der erste Ordner gewinnt")
    {
        namespace so = lumi::core::suchordner;
        const EigeneEinstellungen einstellungen;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const QString eins = QDir(tmp.path()).filePath(QStringLiteral("eins"));
        const QString zwei = QDir(tmp.path()).filePath(QStringLiteral("zwei"));
        const QString inZwei = datei(tmp, QStringLiteral("zwei/texer_circle.bmp"));
        const QString tief = datei(tmp, QStringLiteral("zwei/pack/bild.jpg"));
        REQUIRE(QDir().mkpath(eins));

        CHECK(so::findeDatei(QStringLiteral("texer_circle.bmp")).isEmpty());  // die Liste ist leer
        so::setzeListe({eins, QStringLiteral("X:/gibt/es/nicht"), zwei});
        CHECK(so::findeDatei(QStringLiteral("texer_circle.bmp")) == inZwei);
        CHECK(so::findeDatei(QStringLiteral("pack/bild.jpg")) == tief);              // mit Unterordner, wie genannt
        CHECK(so::findeDatei(QStringLiteral("C:\\Winamp\\avs\\texer_circle.bmp")) == inZwei);  // ein fremder Pfad: nur der Name
        CHECK(so::findeDatei(QStringLiteral("bild.jpg")).isEmpty());                 // nicht in Unterordnern gesucht
        CHECK(so::findeDatei(QStringLiteral("fehlt.bmp")).isEmpty());
        CHECK(so::findeDatei(QString()).isEmpty());
        CHECK(so::findeDatei(QStringLiteral("pack")).isEmpty());                     // ein Ordner ist keine Datei
        // dieselbe Datei in einem frueheren Ordner gewinnt
        const QString inEins = datei(tmp, QStringLiteral("eins/texer_circle.bmp"));
        CHECK(so::findeDatei(QStringLiteral("texer_circle.bmp")) == inEins);
    }

    TEST_CASE("Suchordner: MilkDrop sucht Texturen und Sprites erst beim Preset, dann in der Liste")
    {
        namespace so = lumi::core::suchordner;
        const EigeneEinstellungen einstellungen;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const QString presets = QDir(tmp.path()).filePath(QStringLiteral("auswahl/presets"));
        REQUIRE(QDir().mkpath(presets));
        const QString sammlung = QDir(tmp.path()).filePath(QStringLiteral("sammlung"));
        const QString lines = datei(tmp, QStringLiteral("sammlung/textures/lines2.jpg"));
        const QString stern = datei(tmp, QStringLiteral("sammlung/sprites/stern.png"));
        const QString lose = datei(tmp, QStringLiteral("sammlung/wolke.png"));

        // ohne Liste: beim Preset liegt nichts
        CHECK(lumi::milkdrop::resolveTextureFile(presets, QStringLiteral("lines2")).isEmpty());
        CHECK(lumi::milkdrop::resolveSpriteFile(presets, QStringLiteral("stern.png")).isEmpty());
        CHECK(lumi::milkdrop::textureSearchDirs(QString()).isEmpty());

        so::setzeListe({sammlung});
        CHECK(lumi::milkdrop::resolveTextureFile(presets, QStringLiteral("lines2")) == lines);
        CHECK(lumi::milkdrop::resolveTextureFile(presets, QStringLiteral("wolke")) == lose);
        CHECK(lumi::milkdrop::resolveSpriteFile(presets, QStringLiteral("stern.png")) == stern);
        CHECK(lumi::milkdrop::resolveSpriteFile(presets, QStringLiteral("textures\\lines2.jpg")) == lines);
        // auch ein Knoten ohne Preset-Ordner findet es
        CHECK(lumi::milkdrop::resolveTextureFile(QString(), QStringLiteral("lines2")) == lines);
        // die Ordner der Liste stehen hinter denen des Presets
        const QStringList dirs = lumi::milkdrop::textureSearchDirs(presets);
        CHECK(dirs.first() == QDir(presets).absolutePath() + QStringLiteral("/textures"));
        CHECK(dirs.mid(dirs.size() - 3) == QStringList{sammlung + QStringLiteral("/textures"),
                                                       sammlung + QStringLiteral("/sprites"), sammlung});

        // was beim Preset liegt, gilt zuerst
        const QString beimPreset = datei(tmp, QStringLiteral("auswahl/textures/lines2.png"));
        CHECK(lumi::milkdrop::resolveTextureFile(presets, QStringLiteral("lines2")) == beimPreset);
    }
}
