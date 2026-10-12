/**
 * @file test_ZipArchiv.cpp
 * @brief Ein Zip schreiben und wieder lesen (docs/core-services/
 *        Datenablage_Konzept.md, Stufe A4) — ohne App, in einem Wegwerf-Ordner.
 *
 * `LUMIVIZ_ZIP_PROBE=<datei.zip>` legt das Archiv des ersten Falls ab, damit
 * ein fremdes Werkzeug es entpacken kann (der Beleg, dass es ein Zip ist und
 * nicht nur etwas, das wir selbst wieder lesen).
 */

#include <doctest.h>

#include "core/ZipArchiv.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

namespace
{

using lumi::core::ZipEintrag;

std::vector<ZipEintrag> probe()
{
    QByteArray gross;
    for (int i = 0; i < 20000; ++i) gross += "LumiViz ";  // 160 000 Byte, gut zu packen
    QByteArray binaer;
    for (int i = 0; i < 4096; ++i) binaer += static_cast<char>((i * 37 + 11) & 0xFF);  // auch Nullen
    const QDateTime zeit(QDate(2026, 10, 3), QTime(14, 30, 20));
    return {
        {QStringLiteral("manifest.json"), QByteArray(R"({"format":"lumiviz-data"})"), zeit},
        {QStringLiteral("data/presets/visuals/equalizer/Neon bars.json"), gross, zeit},
        {QStringLiteral("data/karaoke/trails/Glühen äöü.lvtrail"), binaer, zeit},
        {QStringLiteral("data/lyrics/leer.txt"), QByteArray(), zeit},
    };
}

}  // namespace

TEST_CASE("ZipArchiv: schreiben und lesen ergibt dieselben Eintraege — Namen, Bytes, Zeit")
{
    const std::vector<ZipEintrag> hinein = probe();
    QString fehler;
    const QByteArray zip = lumi::core::alsZip(hinein, &fehler);
    REQUIRE_MESSAGE(!zip.isEmpty(), fehler.toStdString());
    CHECK(fehler.isEmpty());
    // gepackt: deutlich kleiner als die 160 000 Byte des grossen Eintrags
    CHECK(zip.size() < 20000);
    CHECK(zip.startsWith("PK"));

    std::vector<ZipEintrag> heraus;
    REQUIRE_MESSAGE(lumi::core::ausZip(zip, heraus, &fehler), fehler.toStdString());
    REQUIRE(heraus.size() == hinein.size());
    for (std::size_t i = 0; i < hinein.size(); ++i)
    {
        CAPTURE(hinein[i].name.toStdString());
        CHECK(heraus[i].name == hinein[i].name);
        CHECK(heraus[i].daten == hinein[i].daten);
        // ein Zip haelt die Zeit auf zwei Sekunden genau
        CHECK(qAbs(heraus[i].geaendert.secsTo(hinein[i].geaendert)) <= 2);
    }

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_ZIP_PROBE"); !ziel.isEmpty())
    {
        REQUIRE(lumi::core::schreibeZip(ziel, hinein, &fehler));
        MESSAGE("Archiv abgelegt: " << ziel.toStdString());
    }
}

TEST_CASE("ZipArchiv: in eine Datei und zurueck, auch mit Umlauten im Pfad")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    REQUIRE(QDir(tmp.path()).mkpath(QStringLiteral("Sicherung äöü")));
    const QString pfad = QDir(tmp.path()).filePath(QStringLiteral("Sicherung äöü/LumiViz ü.lvdata"));

    QString fehler;
    REQUIRE_MESSAGE(lumi::core::schreibeZip(pfad, probe(), &fehler), fehler.toStdString());
    REQUIRE(QFile::exists(pfad));

    std::vector<ZipEintrag> heraus;
    REQUIRE_MESSAGE(lumi::core::leseZip(pfad, heraus, &fehler), fehler.toStdString());
    CHECK(heraus == [] {
        // die Zeit kommt auf zwei Sekunden gerundet zurueck: die Probe hat eine gerade Sekunde
        return probe();
    }());
}

TEST_CASE("ZipArchiv: ein leeres Archiv ist ein Archiv")
{
    QString fehler;
    const QByteArray zip = lumi::core::alsZip({}, &fehler);
    REQUIRE_MESSAGE(!zip.isEmpty(), fehler.toStdString());
    std::vector<ZipEintrag> heraus{{QStringLiteral("vorher"), QByteArray("x"), {}}};
    REQUIRE(lumi::core::ausZip(zip, heraus, &fehler));
    CHECK(heraus.empty());
}

TEST_CASE("ZipArchiv: was kein Zip ist, wird abgelehnt und gemeldet — die Liste bleibt")
{
    std::vector<ZipEintrag> heraus{{QStringLiteral("vorher"), QByteArray("x"), {}}};
    QString fehler;
    CHECK_FALSE(lumi::core::ausZip(QByteArray("das ist kein Archiv"), heraus, &fehler));
    CHECK_FALSE(fehler.isEmpty());
    CHECK(heraus.size() == 1);

    CHECK_FALSE(lumi::core::ausZip(QByteArray(), heraus, &fehler));
    CHECK(heraus.size() == 1);

    // ein abgeschnittenes Archiv: das Verzeichnis am Ende fehlt
    const QByteArray ganz = lumi::core::alsZip(probe());
    REQUIRE(ganz.size() > 100);
    CHECK_FALSE(lumi::core::ausZip(ganz.left(ganz.size() / 2), heraus, &fehler));
    CHECK(heraus.size() == 1);

    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    CHECK_FALSE(lumi::core::leseZip(QDir(tmp.path()).filePath(QStringLiteral("gibt_es_nicht.lvdata")), heraus, &fehler));
    CHECK_FALSE(fehler.isEmpty());
}

TEST_CASE("ZipArchiv: ein beschaedigter Eintrag faellt beim Entpacken auf")
{
    // ein Byte mitten in den gepackten Daten des grossen Eintrags umdrehen:
    // die Pruefsumme des Eintrags stimmt dann nicht mehr
    QByteArray zip = lumi::core::alsZip(probe());
    REQUIRE(zip.size() > 400);
    zip[200] = static_cast<char>(zip[200] ^ 0x5A);
    std::vector<ZipEintrag> heraus;
    QString fehler;
    CHECK_FALSE(lumi::core::ausZip(zip, heraus, &fehler));
    CHECK_FALSE(fehler.isEmpty());
}
