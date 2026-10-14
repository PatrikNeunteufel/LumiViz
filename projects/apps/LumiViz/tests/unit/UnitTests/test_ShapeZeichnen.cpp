/**
 * @file test_ShapeZeichnen.cpp
 * @brief Klang aus einer Form, Stufe 20a: was Pen und Move point mit einem Pfad
 *        tun (shape/Zeichnen.hpp; docs/audio/Shape_Konzept.md §8, Entscheid
 *        Z1 a, Patrik S122)
 *
 * Ohne Fenster, ohne Qt-Anwendung: frei Hand geglaettet und in Kurven
 * umgerechnet, Eckpunkte, beides gemischt, Ankerpunkte finden und schieben.
 */

#include <doctest.h>

#include "shape/Kontur.hpp"
#include "shape/Zeichnen.hpp"

#include <QLineF>
#include <QPainterPath>
#include <QPointF>

#include <cmath>
#include <vector>

using namespace lumi::shape;

namespace
{

constexpr double kPi = 3.14159265358979323846;

/// Ein Kreis um (100, 100) mit Radius 50 aus `punkte` Punkten, der letzte auf dem ersten.
std::vector<QPointF> kreisZug(int punkte, double zittern = 0.0)
{
    std::vector<QPointF> zug;
    for (int n = 0; n <= punkte; ++n)
    {
        const double w = 2.0 * kPi * n / punkte;
        // ein leichtes Zittern der Hand, das die Glaettung wegnehmen soll
        const double r = 50.0 + ((n % 2 == 0) ? zittern : -zittern);
        zug.emplace_back(100.0 + r * std::cos(w), 100.0 + r * std::sin(w));
    }
    return zug;
}

int kurven(const QPainterPath& pfad)
{
    int zahl = 0;
    for (int i = 0; i < pfad.elementCount(); ++i)
        if (pfad.elementAt(i).type == QPainterPath::CurveToElement) ++zahl;
    return zahl;
}

}  // namespace

TEST_CASE("Shape Zeichnen: frei Hand — geglaettet, wenige Ankerpunkte, Kurven, nah am Zug")
{
    QPainterPath pfad;
    const int dazu = haengeFreihandAn(pfad, kreisZug(200, 0.6), 0.5);
    const std::vector<QPointF> anker = ankerpunkte(pfad);
    INFO("Ankerpunkte: " << anker.size());
    // der Zug endet auf seinem Anfang: der letzte Punkt ist der Schluss und zaehlt nicht als eigener
    CHECK(dazu == static_cast<int>(anker.size()) + 1);
    // aus 201 Punkten werden wenige — und es bleiben genug fuer einen Kreis
    CHECK(anker.size() >= 6);
    CHECK(anker.size() <= 40);
    CHECK(kurven(pfad) == static_cast<int>(anker.size()));
    // jeder Ankerpunkt liegt auf dem Kreis, und die Kurven dazwischen weichen kaum ab
    for (const QPointF& a : anker) CHECK(std::abs(QLineF(a, QPointF(100.0, 100.0)).length() - 50.0) < 1.0);
    pfad.closeSubpath();
    double weitest = 0.0;
    for (const Punkt& p : abtasten(pfad, 512))
        weitest = std::max(weitest, std::abs(std::abs(p - Punkt(100.0, -100.0)) - 50.0));  // abtasten dreht y um
    INFO("weitest vom Kreis: " << weitest);
    CHECK(weitest < 1.5);

    // ein feinerer Zug (kleinere Toleranz) behaelt mehr Punkte
    QPainterPath fein;
    (void)haengeFreihandAn(fein, kreisZug(200), 0.05);
    CHECK(ankerpunkte(fein).size() > anker.size());

    // ein Zug aus einem Punkt beginnt nur; doppelte Punkte (die Maus stand) zaehlen nicht
    QPainterPath einer;
    CHECK(haengeFreihandAn(einer, {QPointF(3.0, 4.0), QPointF(3.0, 4.0)}, 1.0) == 1);
    CHECK(ankerpunkte(einer).size() == 1);
    CHECK(haengeFreihandAn(einer, {}, 1.0) == 0);
}

TEST_CASE("Shape Zeichnen: Eckpunkte — Strecken, schliessen erst ab drei Punkten, beides gemischt")
{
    QPainterPath pfad;
    CHECK(haengeEckeAn(pfad, QPointF(0.0, 0.0)));
    CHECK_FALSE(haengeEckeAn(pfad, QPointF(0.0, 0.0)));  // derselbe Punkt: nichts
    CHECK_FALSE(laesstSichSchliessen(pfad));
    CHECK(haengeEckeAn(pfad, QPointF(100.0, 0.0)));
    CHECK_FALSE(laesstSichSchliessen(pfad));  // zwei Punkte mit einer Strecke umschliessen nichts
    CHECK(haengeEckeAn(pfad, QPointF(50.0, 80.0)));
    CHECK(laesstSichSchliessen(pfad));
    CHECK(kurven(pfad) == 0);
    CHECK(ankerpunkte(pfad).size() == 3);

    // geschlossen: der letzte Punkt liegt auf dem ersten und zaehlt nicht doppelt
    QPainterPath zu = pfad;
    zu.closeSubpath();
    CHECK(ankerpunkte(zu).size() == 3);
    CHECK(pfadAlsText(zu) == QStringLiteral("M 0 0 L 100 0 L 50 80 L 0 0 Z"));

    // gemischt: nach einem Eckpunkt fuehrt eine Strecke zum Anfang des Zugs, dann Kurven
    QPainterPath gemischt;
    (void)haengeEckeAn(gemischt, QPointF(0.0, 0.0));
    std::vector<QPointF> bogen;
    for (int n = 0; n <= 40; ++n) bogen.emplace_back(50.0 + 30.0 * std::cos(kPi * n / 40.0), 30.0 * std::sin(kPi * n / 40.0));
    const int dazu = haengeFreihandAn(gemischt, bogen, 0.3);
    CHECK(dazu >= 3);
    CHECK(gemischt.elementAt(1).type == QPainterPath::LineToElement);
    CHECK(QLineF(QPointF(gemischt.elementAt(1).x, gemischt.elementAt(1).y), bogen.front()).length() < 1e-9);
    CHECK(kurven(gemischt) == dazu - 1);
    CHECK(laesstSichSchliessen(gemischt));

    // zwei Punkte mit einer Kurve dazwischen umschliessen eine Flaeche
    QPainterPath linse;
    linse.moveTo(0.0, 0.0);
    linse.cubicTo(10.0, 20.0, 30.0, 20.0, 40.0, 0.0);
    CHECK(laesstSichSchliessen(linse));
}

TEST_CASE("Shape Zeichnen: Ankerpunkte finden und schieben — die Griffe gehen mit, der Anfang mit dem Ende")
{
    // das mitgelieferte Herz: vier Kurven, geschlossen
    QPainterPath herz;
    REQUIRE(pfadAusText(QStringLiteral("M 100 73.333 C 100 25.926 34.815 20 34.815 70.37 C 34.815 117.778 100 138.519 100 180 "
                                       "C 100 138.519 165.185 117.778 165.185 70.37 C 165.185 20 100 25.926 100 73.333 Z"),
                        herz));
    const std::vector<QPointF> anker = ankerpunkte(herz);
    REQUIRE(anker.size() == 4);
    CHECK(anker[2] == QPointF(100.0, 180.0));

    CHECK(naechsterAnker(herz, QPointF(102.0, 178.0), 5.0) == 2);
    CHECK(naechsterAnker(herz, QPointF(102.0, 170.0), 5.0) == -1);
    CHECK(naechsterAnker(herz, QPointF(100.0, 74.0), 5.0) == 0);

    // die Spitze nach unten ziehen: die beiden Griffe daneben gehen um dasselbe Stueck mit
    QPainterPath gezogen = herz;
    REQUIRE(schiebeAnker(gezogen, 2, QPointF(100.0, 190.0)));
    CHECK(ankerpunkte(gezogen)[2] == QPointF(100.0, 190.0));
    // Kurve 2 endet an der Spitze: ihr zweiter Griff (Element 5) war (100, 138.519)
    CHECK(gezogen.elementAt(5).y == doctest::Approx(148.519));
    // Kurve 3 beginnt an der Spitze: ihr erster Griff (Element 7) war (100, 138.519)
    CHECK(gezogen.elementAt(7).y == doctest::Approx(148.519));
    // die anderen bleiben
    CHECK(gezogen.elementAt(1).y == doctest::Approx(25.926));
    CHECK(ankerpunkte(gezogen)[1] == anker[1]);

    // den Anfang schieben: das Ende geht mit, der Zug bleibt geschlossen
    QPainterPath anfang = herz;
    REQUIRE(schiebeAnker(anfang, 0, QPointF(100.0, 60.0)));
    CHECK(ankerpunkte(anfang).size() == 4);
    CHECK(ankerpunkte(anfang)[0] == QPointF(100.0, 60.0));
    const QPainterPath::Element ende = anfang.elementAt(anfang.elementCount() - 1);
    CHECK(QPointF(ende.x, ende.y) == QPointF(100.0, 60.0));
    CHECK(pfadAlsText(anfang).endsWith(QStringLiteral("Z")));
    // und durch den Text hin und zurueck
    QPainterPath zurueck;
    REQUIRE(pfadAusText(pfadAlsText(anfang), zurueck));
    CHECK(ankerpunkte(zurueck) == ankerpunkte(anfang));

    // einen Punkt, den es nicht gibt, schiebt nichts
    QPainterPath unveraendert = herz;
    CHECK_FALSE(schiebeAnker(unveraendert, 4, QPointF()));
    CHECK_FALSE(schiebeAnker(unveraendert, -1, QPointF()));
    CHECK(unveraendert == herz);

    // ein Dreieck aus Strecken: der dritte Punkt
    QPainterPath dreieck;
    dreieck.moveTo(0.0, 0.0);
    dreieck.lineTo(10.0, 0.0);
    dreieck.lineTo(5.0, 8.0);
    dreieck.closeSubpath();
    REQUIRE(schiebeAnker(dreieck, 2, QPointF(5.0, 12.0)));
    CHECK(ankerpunkte(dreieck) == std::vector<QPointF>{QPointF(0.0, 0.0), QPointF(10.0, 0.0), QPointF(5.0, 12.0)});
}
