/**
 ****************************************************************************************
 * @file   test_ComposerPegelform.cpp
 * @brief  Composer, Stufe C4 Teil 2: Form einer Blende oder eines Mittelstuecks,
 *         Platz der Teile im Clip (docs/ui/Composer_Konzept.md §9)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Reines Modell, kein Fenster. Die Form in Anteilen (0…1), ihre zwei
 * Spiegelungen — Ausblendung an der y-Achse, gekipptes Stueck an der x-Achse —,
 * das Richten, die Datei; dann `verteile`: Teile behalten ihre Weite, bis sie
 * sich ueberdecken, und werden dann gleichmaessig gestaucht.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "composer/Pegelform.hpp"

#include <QJsonObject>

using namespace lumi::composer;

TEST_SUITE("ComposerPegelform")
{
    TEST_CASE("Eine einfache Blende laeuft wie die Kurvenform, die sie traegt")
    {
        const Pegelform linear = einfacheBlende();
        CHECK(linear.wert(0.0) == doctest::Approx(0.0));
        CHECK(linear.wert(0.25) == doctest::Approx(0.25));
        CHECK(linear.wert(1.0) == doctest::Approx(1.0));

        const Pegelform weich = einfacheBlende(Kurvenform::SKurve);
        CHECK(weich.wert(0.25) == doctest::Approx(kurvenWert(Kurvenform::SKurve, 0.25)));

        const BezierGriffe griffe = {0.1, 0.7, 0.3, 1.0};
        const Pegelform bezier = einfacheBlende(Kurvenform::Bezier, griffe);
        CHECK(bezier.wert(0.4) == doctest::Approx(bezierWert(griffe, 0.4)));
    }

    TEST_CASE("Zwischenpunkte teilen die Blende in Abschnitte")
    {
        Pegelform f = einfacheBlende();
        f.punkte.insert(f.punkte.begin() + 1, FormPunkt{0.5, 0.8});
        CHECK(f.wert(0.25) == doctest::Approx(0.4));
        CHECK(f.wert(0.5) == doctest::Approx(0.8));
        CHECK(f.wert(0.75) == doctest::Approx(0.9));
    }

    TEST_CASE("Als Ausblendung laeuft dieselbe Form rueckwaerts")
    {
        Pegelform f = einfacheBlende();
        f.punkte.insert(f.punkte.begin() + 1, FormPunkt{0.5, 0.8});
        // Einblendung: nach einem Viertel 0.4 — Ausblendung: ein Viertel vor Schluss 0.4
        CHECK(blendenPegel(f, 0.25, false) == doctest::Approx(0.4));
        CHECK(blendenPegel(f, 0.75, true) == doctest::Approx(0.4));
        CHECK(blendenPegel(f, 0.0, true) == doctest::Approx(1.0));
        CHECK(blendenPegel(f, 1.0, true) == doctest::Approx(0.0));
    }

    TEST_CASE("Ein gekipptes Stueck tauscht oben und unten")
    {
        const Pegelform d = delle();
        CHECK(stueckPegel(d, 0.0, false) == doctest::Approx(1.0));
        CHECK(stueckPegel(d, 0.5, false) == doctest::Approx(0.0));
        CHECK(stueckPegel(d, 1.0, false) == doctest::Approx(1.0));
        // der Buckel
        CHECK(stueckPegel(d, 0.0, true) == doctest::Approx(0.0));
        CHECK(stueckPegel(d, 0.5, true) == doctest::Approx(1.0));
        CHECK(stueckPegel(d, 0.3, true) == doctest::Approx(1.0 - stueckPegel(d, 0.3, false)));
    }

    TEST_CASE("Richten sortiert, klemmt und setzt die Enden")
    {
        Pegelform blende;
        blende.punkte = {FormPunkt{0.7, 1.4}, FormPunkt{0.2, 0.5}, FormPunkt{0.0, 0.3}};
        blende.richte();
        REQUIRE(blende.punkte.size() == 4);
        CHECK(blende.punkte[0].x == doctest::Approx(0.0));
        CHECK(blende.punkte[0].y == doctest::Approx(0.0));  // eine Blende beginnt bei null
        CHECK(blende.punkte[1].x == doctest::Approx(0.2));
        CHECK(blende.punkte[2].y == doctest::Approx(1.0));  // geklemmt
        CHECK(blende.punkte[3].x == doctest::Approx(1.0));
        CHECK(blende.punkte[3].y == doctest::Approx(1.0));

        Pegelform leer;
        leer.richte();
        REQUIRE(leer.punkte.size() == 2);
        CHECK(leer.wert(0.5) == doctest::Approx(0.5));

        Pegelform stueck;
        stueck.art = FormArt::Stueck;
        stueck.punkte = {FormPunkt{0.5, 0.2}};
        stueck.richte();
        REQUIRE(stueck.punkte.size() == 3);
        CHECK(stueck.punkte.front().y == doctest::Approx(1.0));  // die Enden auf vollem Pegel
        CHECK(stueck.punkte.back().y == doctest::Approx(1.0));
        CHECK(stueck.wert(0.5) == doctest::Approx(0.2));
    }

    TEST_CASE("Die Form uebersteht die Datei")
    {
        Pegelform f = delle();
        f.punkte[0].form = Kurvenform::Bezier;
        f.punkte[0].bezier = {0.2, 0.1, 0.9, 0.6};
        Pegelform gelesen;
        REQUIRE(formAusJson(formAlsJson(f), gelesen));
        CHECK(gelesen.art == FormArt::Stueck);
        CHECK(gleich(f, gelesen));
        CHECK(gelesen.punkte[0].bezier[3] == doctest::Approx(0.6));

        Pegelform unberuehrt = einfacheBlende();
        CHECK_FALSE(formAusJson(QJsonObject{}, unberuehrt));
        CHECK(gleich(unberuehrt, einfacheBlende()));
    }

    TEST_CASE("Teile, die sich nicht ueberdecken, behalten ihre Weite")
    {
        const Verteilung v = verteile(10000, 1000, 2000, {{0.5, 1000}});
        CHECK(v.ein.vonMs == 0);
        CHECK(v.ein.bisMs == 1000);
        CHECK(v.aus.vonMs == 8000);
        CHECK(v.aus.bisMs == 10000);
        REQUIRE(v.stuecke.size() == 1);
        CHECK(v.stuecke[0].vonMs == 4500);
        CHECK(v.stuecke[0].bisMs == 5500);
        CHECK_FALSE(v.ein.gestaucht);
        CHECK_FALSE(v.aus.gestaucht);
        CHECK_FALSE(v.stuecke[0].gestaucht);
    }

    TEST_CASE("Blosses Beruehren staucht nicht")
    {
        const Verteilung v = verteile(6000, 3000, 3000);
        CHECK(v.ein.bisMs == 3000);
        CHECK(v.aus.vonMs == 3000);
        CHECK_FALSE(v.ein.gestaucht);
        CHECK_FALSE(v.aus.gestaucht);
    }

    TEST_CASE("Ueberdecken sich die Blenden, teilen sie den Clip nach ihren Weiten")
    {
        const Verteilung gleichLang = verteile(6000, 5000, 5000);
        CHECK(gleichLang.ein.bisMs == 3000);
        CHECK(gleichLang.aus.vonMs == 3000);
        CHECK(gleichLang.aus.bisMs == 6000);
        CHECK(gleichLang.ein.gestaucht);
        CHECK(gleichLang.aus.gestaucht);

        // 2 : 1 — die laengere Blende behaelt den groesseren Teil
        const Verteilung ungleich = verteile(6000, 8000, 4000);
        CHECK(ungleich.ein.bisMs == 4000);
        CHECK(ungleich.aus.vonMs == 4000);

        // wieder laenger gezogen: die eingestellten Weiten sind zurueck
        const Verteilung lang = verteile(20000, 5000, 5000);
        CHECK(lang.ein.bisMs == 5000);
        CHECK(lang.aus.vonMs == 15000);
        CHECK_FALSE(lang.ein.gestaucht);
    }

    TEST_CASE("Eine Blende allein, laenger als der Clip, fuellt ihn")
    {
        const Verteilung v = verteile(4000, 8000, 0);
        CHECK(v.ein.vonMs == 0);
        CHECK(v.ein.bisMs == 4000);
        CHECK(v.ein.gestaucht);
        CHECK(v.aus.weiteMs() == 0);
    }

    TEST_CASE("Ein Stueck, das die Einblendung ueberdeckt, wird mit ihr gestaucht")
    {
        // Einblendung 0–5 s, Stueck 2–6 s: zusammen 0–6 s, im Verhaeltnis 5 : 4
        const Verteilung v = verteile(10000, 5000, 1000, {{0.4, 4000}});
        CHECK(v.ein.vonMs == 0);
        CHECK(v.ein.bisMs == 3333);
        CHECK(v.stuecke[0].vonMs == 3333);
        CHECK(v.stuecke[0].bisMs == 6000);
        CHECK(v.ein.gestaucht);
        CHECK(v.stuecke[0].gestaucht);
        // die Ausblendung beruehrt niemanden und bleibt
        CHECK(v.aus.vonMs == 9000);
        CHECK_FALSE(v.aus.gestaucht);
    }

    TEST_CASE("Ein Stueck an der Kante rueckt hinein; die Ausblendung bleibt zuhinterst")
    {
        const Verteilung allein = verteile(10000, 0, 0, {{1.0, 2000}});
        CHECK(allein.stuecke[0].vonMs == 8000);
        CHECK(allein.stuecke[0].bisMs == 10000);
        CHECK_FALSE(allein.stuecke[0].gestaucht);

        const Verteilung mitAus = verteile(10000, 0, 1000, {{1.0, 2000}});
        CHECK(mitAus.stuecke[0].vonMs == 8000);
        CHECK(mitAus.stuecke[0].bisMs == 9333);
        CHECK(mitAus.aus.vonMs == 9333);
        CHECK(mitAus.aus.bisMs == 10000);
    }

    TEST_CASE("Stuecke kommen in der Reihenfolge der Eingabe zurueck, liegen aber nach ihrer Lage")
    {
        // das zweite liegt vor dem ersten; beide ueberdecken sich
        const Verteilung v = verteile(10000, 0, 0, {{0.6, 4000}, {0.4, 4000}});
        REQUIRE(v.stuecke.size() == 2);
        CHECK(v.stuecke[1].vonMs == 2000);
        CHECK(v.stuecke[1].bisMs == 5000);
        CHECK(v.stuecke[0].vonMs == 5000);
        CHECK(v.stuecke[0].bisMs == 8000);
    }

    TEST_CASE("Der Positionsanteil haelt ein Stueck beim Strecken an seiner Stelle")
    {
        const Verteilung kurz = verteile(10000, 0, 0, {{0.25, 1000}});
        const Verteilung lang = verteile(20000, 0, 0, {{0.25, 1000}});
        CHECK(kurz.stuecke[0].vonMs == 2000);
        CHECK(lang.stuecke[0].vonMs == 4500);
        CHECK(lang.stuecke[0].weiteMs() == 1000);  // die Weite bleibt
    }
}
