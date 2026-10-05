/**
 ****************************************************************************************
 * @file   test_ComposerFormPresets.cpp
 * @brief  Composer, Stufe C4 Teil 2: Kurvenformen als Presets — Blenden und Stuecke
 *         (docs/ui/Composer_Konzept.md §9)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Kein Fenster. Speichern, Listen, Laden und Loeschen in einer Test-Wurzel; die
 * zwei Arten liegen getrennt und lassen sich nicht vertauschen; ein eigenes
 * Preset verdraengt ein mitgeliefertes gleichen Namens, mitgelieferte bleiben
 * geschuetzt. Dazu: jedes mitgelieferte Preset des Repos laedt.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "composer/FormPresets.hpp"

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTemporaryDir>

using namespace lumi::composer;

namespace
{

/// Setzt die Test-Wurzel und nimmt sie am Ende wieder weg.
struct Wurzel
{
    explicit Wurzel(const QString& pfad) { formpresets::setzeWurzelFuerTests(pfad); }
    ~Wurzel() { formpresets::setzeWurzelFuerTests({}); }
    Wurzel(const Wurzel&) = delete;
    Wurzel& operator=(const Wurzel&) = delete;
};

QStringList namen(FormArt art)
{
    QStringList aus;
    for (const formpresets::Eintrag& e : formpresets::liste(art))
        aus << (e.mitgeliefert ? QStringLiteral("[%1]").arg(e.name) : e.name);
    return aus;
}

}  // namespace

TEST_SUITE("ComposerFormPresets")
{
    TEST_CASE("Speichern, listen, laden, loeschen — je Art im eigenen Ordner")
    {
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const Wurzel wurzel(dir.path());
        CHECK(formpresets::liste(FormArt::Blende).empty());

        Pegelform blende = einfacheBlende(Kurvenform::Bezier, {0.2, 0.9, 0.5, 1.0});
        blende.punkte.insert(blende.punkte.begin() + 1, FormPunkt{0.6, 0.4});
        QString pfad;
        REQUIRE(formpresets::speichere(QStringLiteral("Mein Anlauf"), blende, &pfad));
        CHECK(pfad.endsWith(QStringLiteral("user/composer/fades/Mein Anlauf.lvfade")));
        REQUIRE(formpresets::speichere(QStringLiteral("Tal"), delle()));

        // getrennte Listen
        CHECK(namen(FormArt::Blende) == QStringList{QStringLiteral("Mein Anlauf")});
        CHECK(namen(FormArt::Stueck) == QStringList{QStringLiteral("Tal")});

        Pegelform gelesen;
        REQUIRE(formpresets::lade(pfad, FormArt::Blende, gelesen));
        CHECK(gleich(gelesen, blende));
        // die andere Art wird abgelehnt, die Form bleibt, wie sie war
        Pegelform unberuehrt = delle();
        CHECK_FALSE(formpresets::lade(pfad, FormArt::Stueck, unberuehrt));
        CHECK(gleich(unberuehrt, delle()));

        // der Name wird bereinigt; ein leerer ist keiner
        REQUIRE(formpresets::speichere(QStringLiteral("  a/b:c  "), einfacheBlende(), &pfad));
        CHECK(QFileInfo(pfad).completeBaseName() == QStringLiteral("a_b_c"));
        CHECK_FALSE(formpresets::speichere(QStringLiteral("   "), einfacheBlende()));

        // finden ohne Gross / Klein, loeschen
        const formpresets::Eintrag tal = formpresets::finde(FormArt::Stueck, QStringLiteral("TAL"));
        REQUIRE_FALSE(tal.pfad.isEmpty());
        CHECK(formpresets::finde(FormArt::Blende, QStringLiteral("Tal")).pfad.isEmpty());
        CHECK(formpresets::loesche(FormArt::Stueck, tal));
        CHECK(formpresets::liste(FormArt::Stueck).empty());
    }

    TEST_CASE("Ein eigenes Preset verdraengt das mitgelieferte; mitgelieferte sind geschuetzt")
    {
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const Wurzel wurzel(dir.path());

        // zwei mitgelieferte: von Hand in den Asset-Ordner der Test-Wurzel
        const QString asset = formpresets::assetOrdner(FormArt::Blende);
        REQUIRE(QDir().mkpath(asset));
        for (const char* name : {"Linear", "Weich"})
        {
            QFile f(QDir(asset).filePath(QString::fromLatin1(name) + QStringLiteral(".lvfade")));
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write(R"({"format":"lumiviz-curve","version":1,"kind":"fade",)"
                    R"("points":[{"x":0,"y":0,"curve":0},{"x":1,"y":1,"curve":0}]})");
        }
        CHECK(namen(FormArt::Blende) == QStringList{QStringLiteral("[Linear]"), QStringLiteral("[Weich]")});

        REQUIRE(formpresets::speichere(QStringLiteral("weich"), einfacheBlende(Kurvenform::SKurve)));
        CHECK(namen(FormArt::Blende) == QStringList{QStringLiteral("[Linear]"), QStringLiteral("weich")});
        Pegelform gelesen;
        REQUIRE(formpresets::lade(formpresets::finde(FormArt::Blende, QStringLiteral("Weich")).pfad,
                                  FormArt::Blende, gelesen));
        CHECK(gelesen.punkte.front().form == Kurvenform::SKurve);  // das eigene

        // ein mitgeliefertes loescht die App nicht
        const formpresets::Eintrag linear = formpresets::finde(FormArt::Blende, QStringLiteral("Linear"));
        REQUIRE(linear.mitgeliefert);
        CHECK_FALSE(formpresets::loesche(FormArt::Blende, linear));
        CHECK(QFile::exists(linear.pfad));
        // und auch nichts ausserhalb des Benutzerordners, selbst wenn der Eintrag es behauptet
        formpresets::Eintrag falsch = linear;
        falsch.mitgeliefert = false;
        CHECK_FALSE(formpresets::loesche(FormArt::Blende, falsch));

        // unlesbar oder fremd: abgelehnt
        QFile kaputt(QDir(asset).filePath(QStringLiteral("Kaputt.lvfade")));
        REQUIRE(kaputt.open(QIODevice::WriteOnly));
        kaputt.write("{ kein json");
        kaputt.close();
        CHECK_FALSE(formpresets::lade(kaputt.fileName(), FormArt::Blende, gelesen));
        QFile fremd(QDir(asset).filePath(QStringLiteral("Fremd.lvfade")));
        REQUIRE(fremd.open(QIODevice::WriteOnly));
        fremd.write(R"({"format":"etwas-anderes","points":[]})");
        fremd.close();
        CHECK_FALSE(formpresets::lade(fremd.fileName(), FormArt::Blende, gelesen));
    }

    TEST_CASE("Jedes mitgelieferte Kurven-Preset laedt und ist von seiner Art")
    {
        for (const FormArt art : {FormArt::Blende, FormArt::Stueck})
        {
            const QString ordner = formpresets::assetOrdner(art);
            REQUIRE_MESSAGE(!ordner.isEmpty(), "asset/composer nicht gefunden");
            int mitgeliefert = 0;
            for (const formpresets::Eintrag& e : formpresets::liste(art))
            {
                if (!e.mitgeliefert) continue;
                ++mitgeliefert;
                CAPTURE(e.name.toStdString());
                Pegelform form;
                REQUIRE(formpresets::lade(e.pfad, art, form));
                CHECK(form.art == art);
                CHECK(form.punkte.size() >= 2);
                if (art == FormArt::Blende)
                {
                    CHECK(form.wert(0.0) == doctest::Approx(0.0));
                    CHECK(form.wert(1.0) == doctest::Approx(1.0));
                }
                else
                {
                    // ein Stueck beginnt und endet auf dem Grundpegel — kein Sprung an den Raendern
                    CHECK(form.wert(0.0) == doctest::Approx(1.0));
                    CHECK(form.wert(1.0) == doctest::Approx(1.0));
                }
            }
            CHECK(mitgeliefert == (art == FormArt::Blende ? 7 : 3));
        }
        // die sieben Formen aus C4 Teil 1 sind als Blenden da
        Pegelform bezier;
        REQUIRE(formpresets::lade(formpresets::finde(FormArt::Blende, QStringLiteral("Bezier")).pfad,
                                  FormArt::Blende, bezier));
        CHECK(gleich(bezier, einfacheBlende(Kurvenform::Bezier)));
        Pegelform weich;
        REQUIRE(formpresets::lade(formpresets::finde(FormArt::Stueck, QStringLiteral("Dip Soft")).pfad,
                                  FormArt::Stueck, weich));
        CHECK(gleich(weich, delle()));
    }
}
