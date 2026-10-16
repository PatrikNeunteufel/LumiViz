/**
 ****************************************************************************************
 * @file   test_LoopComposerAbfragen.cpp
 * @brief  Loop, S128: der Loop haengt nicht am Composer — die drei Fragen ueber
 *         `loop/ComposerAbfragen`, die Farbreihen in `UI/widgets/Farbreihen`
 *         (Entscheid Patrik N1 b)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Fenster. Der letzte Fall liest die Quellen
 * von `loop/` aus dem Repo und haelt die Richtung fest: kein `#include` aus
 * `composer/`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"  // repoWurzel
#include "UI/widgets/Farbreihen.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Komposition.hpp"
#include "loop/ComposerAbfragen.hpp"

#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QStringList>

using namespace lumi;

TEST_CASE("Loop S128 N1 b: ComposerAbfragen — ohne Anmeldung die Vorgaben, angemeldet die Antworten, leer wieder die Vorgaben")
{
    loop::setzeComposerAbfragen({});
    CHECK_FALSE(loop::composerFuehrtTon());
    CHECK(loop::composerLoopStelleMs(QStringLiteral("a.lvloop")) == -1.0);
    CHECK(loop::composerLoopStellenMs(QStringLiteral("a.lvloop")).empty());
    CHECK_FALSE(loop::mitlaufenSeitenweise());

    bool fuehrt = true;
    QString gefragt;
    loop::setzeComposerAbfragen({[&fuehrt] { return fuehrt; },
                                 [&gefragt](const QString& datei) {
                                     gefragt = datei;
                                     return std::vector<double>{1250.0, 300.0};  // der lauteste zuerst (S130, W8 a)
                                 },
                                 [] { return true; }});
    CHECK(loop::composerFuehrtTon());
    fuehrt = false;
    CHECK_FALSE(loop::composerFuehrtTon());  // gefragt wird jedes Mal, nicht einmal gemerkt
    CHECK(loop::composerLoopStelleMs(QStringLiteral("beat.lvloop")) == 1250.0);
    CHECK(loop::composerLoopStellenMs(QStringLiteral("beat.lvloop")) == std::vector<double>{1250.0, 300.0});
    CHECK(gefragt == QStringLiteral("beat.lvloop"));
    CHECK(loop::mitlaufenSeitenweise());

    // nur eine der drei angemeldet: die anderen geben die Vorgabe
    loop::setzeComposerAbfragen({[] { return true; }, {}, {}});
    CHECK(loop::composerFuehrtTon());
    CHECK(loop::composerLoopStelleMs(QStringLiteral("beat.lvloop")) == -1.0);
    CHECK_FALSE(loop::mitlaufenSeitenweise());

    loop::setzeComposerAbfragen({});
    CHECK_FALSE(loop::composerFuehrtTon());
}

TEST_CASE("Loop S128 N1 b: der Composer meldet an — der Loop bekommt dieselben Antworten wie AppAnbindung")
{
    composer::meldeLoopAbfragenAn();
    // ohne geladene Komposition fuehrt keine Sitzung den Ton
    CHECK(loop::composerFuehrtTon() == composer::fuehrtTon());
    CHECK_FALSE(loop::composerFuehrtTon());
    CHECK(loop::composerLoopStellenMs(QStringLiteral("beat.lvloop")) == composer::loopStellenMs(QStringLiteral("beat.lvloop")));
    CHECK(loop::composerLoopStelleMs(QStringLiteral("beat.lvloop")) == -1.0);
    loop::setzeComposerAbfragen({});  // die anderen Faelle laufen ohne Anmeldung
}

TEST_CASE("Loop S128 N1 b: die Farbreihen — kuehl und warm, laufen um, der Composer nimmt dieselben")
{
    CHECK(ui::kReihenFarben == 6);
    CHECK(composer::kReihenFarben == ui::kReihenFarben);
    CHECK(ui::reihenFarbe(ui::Farbreihe::Kuehl, 0) == 0x5785A5);  // das Blau, das CH1 schon immer hatte
    CHECK(ui::reihenFarbe(ui::Farbreihe::Warm, 0) == 0x967855);   // das Orange der Audio-Clips bis S99
    CHECK(ui::reihenFarbe(ui::Farbreihe::Kuehl, 6) == ui::reihenFarbe(ui::Farbreihe::Kuehl, 0));
    CHECK(ui::reihenFarbe(ui::Farbreihe::Warm, -1) == ui::reihenFarbe(ui::Farbreihe::Warm, 5));
    for (int nr = -3; nr < 9; ++nr)
    {
        CAPTURE(nr);
        CHECK(composer::reihenFarbe(composer::KanalArt::Visual, nr) == ui::reihenFarbe(ui::Farbreihe::Kuehl, nr));
        CHECK(composer::reihenFarbe(composer::KanalArt::Audio, nr) == ui::reihenFarbe(ui::Farbreihe::Warm, nr));
        CHECK(ui::reihenFarbe(ui::Farbreihe::Kuehl, nr) != ui::reihenFarbe(ui::Farbreihe::Warm, nr));
    }
}

TEST_CASE("Loop S128 N1 b: die Richtung — keine Datei unter loop/ bindet etwas aus composer/ ein")
{
    const QDir app(looptest::repoWurzel().filePath(QStringLiteral("projects/apps/LumiViz")));
    const QRegularExpression einbindung(QStringLiteral(R"(^\s*#\s*include\s*[<"]composer/)"),
                                        QRegularExpression::MultilineOption);
    int gelesen = 0;
    QStringList verstoesse;
    for (const QString& ordner : {QStringLiteral("include/loop"), QStringLiteral("src/loop")})
    {
        REQUIRE(app.exists(ordner));
        QDirIterator lauf(app.filePath(ordner), {QStringLiteral("*.hpp"), QStringLiteral("*.cpp")}, QDir::Files,
                          QDirIterator::Subdirectories);
        while (lauf.hasNext())
        {
            QFile datei(lauf.next());
            REQUIRE(datei.open(QIODevice::ReadOnly));
            ++gelesen;
            if (einbindung.match(QString::fromUtf8(datei.readAll())).hasMatch()) verstoesse << app.relativeFilePath(datei.fileName());
        }
    }
    CHECK(gelesen > 40);
    CHECK_MESSAGE(verstoesse.isEmpty(), verstoesse.join(QStringLiteral(", ")).toStdString());
}
