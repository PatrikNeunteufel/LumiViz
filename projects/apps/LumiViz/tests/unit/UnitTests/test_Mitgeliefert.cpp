/**
 * @file test_Mitgeliefert.cpp
 * @brief Wo ein mitgelieferter Bereich liegt: neben der Exe oder im
 *        Projektordner (docs/core-services/Datenablage_Konzept.md, Stufe A1;
 *        Entscheid D9 a mit Schalter) — die Suche selbst, in einem
 *        Wegwerf-Ordner, ohne Einstellungen
 */

#include <doctest.h>

#include "core/Mitgeliefert.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

namespace
{

using lumi::core::mitgeliefert::Herkunft;
using lumi::core::mitgeliefert::suche;

void lege(const QString& pfad)
{
    REQUIRE(QDir().mkpath(QFileInfo(pfad).absolutePath()));
    QFile datei(pfad);
    REQUIRE(datei.open(QIODevice::WriteOnly));
    datei.write("x");
}

/// Ein Baum wie im Repo: `<wurzel>/asset/<bereich>/README.md` und die Exe
/// vier Ebenen tiefer unter `out`.
struct Baum
{
    QTemporaryDir tmp;
    QString wurzel;
    QString exe;

    Baum()
    {
        REQUIRE(tmp.isValid());
        wurzel = QDir(tmp.path()).filePath(QStringLiteral("repo"));
        exe = QDir(wurzel).filePath(QStringLiteral("out/build/x/bin"));
        REQUIRE(QDir().mkpath(exe));
    }
    [[nodiscard]] QString imProjekt(const QString& bereich) const
    {
        return QDir(wurzel).filePath(QStringLiteral("asset/") + bereich);
    }
    [[nodiscard]] QString nebenExe(const QString& bereich) const
    {
        return QDir(exe).filePath(QStringLiteral("asset/") + bereich);
    }
};

const QString kAnker = QStringLiteral("README.md");

}  // namespace

TEST_CASE("Mitgeliefert: nirgends etwas — kein Fund")
{
    const Baum baum;
    const auto fund = suche(baum.exe, QStringLiteral("loop"), kAnker, false);
    CHECK(fund.ordner.isEmpty());
    CHECK(fund.herkunft == Herkunft::Keine);
    CHECK(suche(baum.exe, QString(), kAnker, false).ordner.isEmpty());
}

TEST_CASE("Mitgeliefert: nur im Projekt — aufwaerts gefunden, der Anker entscheidet")
{
    const Baum baum;
    REQUIRE(QDir().mkpath(baum.imProjekt(QStringLiteral("loop"))));
    // ohne Anker ist es kein Fund: irgendein Ordner `asset/loop` ueber der Exe genuegt nicht
    CHECK(suche(baum.exe, QStringLiteral("loop"), kAnker, false).ordner.isEmpty());

    lege(baum.imProjekt(QStringLiteral("loop")) + QStringLiteral("/README.md"));
    for (const bool projektZuerst : {false, true})
    {
        const auto fund = suche(baum.exe, QStringLiteral("loop"), kAnker, projektZuerst);
        CHECK(QDir(fund.ordner) == QDir(baum.imProjekt(QStringLiteral("loop"))));
        CHECK(fund.herkunft == Herkunft::Projekt);
    }
}

TEST_CASE("Mitgeliefert: nur neben der Exe — die Kopie gilt ohne Anker (weitergegebene Exe)")
{
    const Baum baum;
    REQUIRE(QDir().mkpath(baum.nebenExe(QStringLiteral("karaoke/trails"))));
    for (const bool projektZuerst : {false, true})
    {
        const auto fund = suche(baum.exe, QStringLiteral("karaoke/trails"), kAnker, projektZuerst);
        CHECK(QDir(fund.ordner) == QDir(baum.nebenExe(QStringLiteral("karaoke/trails"))));
        CHECK(fund.herkunft == Herkunft::NebenExe);
    }
}

TEST_CASE("Mitgeliefert: beides da — die Kopie zuerst, mit Schalter das Projekt (D9 a)")
{
    const Baum baum;
    REQUIRE(QDir().mkpath(baum.nebenExe(QStringLiteral("nodepresets"))));
    lege(baum.imProjekt(QStringLiteral("nodepresets")) + QStringLiteral("/README.md"));

    const auto vorgabe = suche(baum.exe, QStringLiteral("nodepresets"), kAnker, false);
    CHECK(vorgabe.herkunft == Herkunft::NebenExe);
    CHECK(QDir(vorgabe.ordner) == QDir(baum.nebenExe(QStringLiteral("nodepresets"))));

    const auto geschaltet = suche(baum.exe, QStringLiteral("nodepresets"), kAnker, true);
    CHECK(geschaltet.herkunft == Herkunft::Projekt);
    CHECK(QDir(geschaltet.ordner) == QDir(baum.imProjekt(QStringLiteral("nodepresets"))));
}

TEST_CASE("Mitgeliefert: eine Kopie, deren Anker mitkopiert ist, zaehlt nicht als Projekt")
{
    // die Format-Icons: ihr Anker `lumiviz.ico` liegt auch neben der Exe
    const Baum baum;
    const QString bereich = QStringLiteral("img/logo/icons");
    const QString anker = QStringLiteral("lumiviz.ico");
    lege(baum.nebenExe(bereich) + QLatin1Char('/') + anker);

    const auto fund = suche(baum.exe, bereich, anker, true);
    CHECK(fund.herkunft == Herkunft::NebenExe);
}

TEST_CASE("Mitgeliefert: zwoelf Ordner aufwaerts, den Programmordner mitgezaehlt")
{
    const Baum baum;
    lege(baum.imProjekt(QStringLiteral("loop")) + QStringLiteral("/README.md"));
    // zwoelf Ordner unter der Wurzel: sie waere der dreizehnte
    QString tief = baum.wurzel;
    for (int i = 0; i < lumi::core::mitgeliefert::kEbenenAufwaerts; ++i) tief += QStringLiteral("/e");
    REQUIRE(QDir().mkpath(tief));
    CHECK(suche(tief, QStringLiteral("loop"), kAnker, false).ordner.isEmpty());

    // elf darunter: die Wurzel ist der zwoelfte
    QString grenze = baum.wurzel;
    for (int i = 0; i < lumi::core::mitgeliefert::kEbenenAufwaerts - 1; ++i) grenze += QStringLiteral("/e");
    REQUIRE(QDir().mkpath(grenze));
    CHECK(suche(grenze, QStringLiteral("loop"), kAnker, false).herkunft == Herkunft::Projekt);

    // der Programmordner selbst zaehlt mit: beginnt die Suche im Projekt-Root
    // (ein Test ohne App im Arbeitsordner), ist `asset/loop` dort der Fund —
    // er gilt dann als der Ordner neben der Exe
    const auto imRoot = suche(baum.wurzel, QStringLiteral("loop"), kAnker, true);
    CHECK(QDir(imRoot.ordner) == QDir(baum.imProjekt(QStringLiteral("loop"))));
    CHECK(imRoot.herkunft == Herkunft::NebenExe);
}
