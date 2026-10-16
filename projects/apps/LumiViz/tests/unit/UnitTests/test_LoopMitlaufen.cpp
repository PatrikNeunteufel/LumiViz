/**
 ****************************************************************************************
 * @file   test_LoopMitlaufen.cpp
 * @brief  Loop, S127: die Marke laeuft mit — Stelle im Arrangement → Stelle im
 *         Muster, und die Regel des Mitrollens (Entscheide Patrik W1 a, W3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton. Was Panel und Composer daraus machen, pruefen
 * die Faelle „S127 W…" in `test_LoopSpieler.cpp`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/widgets/Achse.hpp"
#include "loop/Loop.hpp"

using namespace lumi::loop;

namespace
{

/// Zwei Muster: A (2 Takte), B (1 Takt); 4 Schlaege zu 4 Schritten, 120 BPM.
/// K1: A ab Takt 0 zweimal (Takt 0–3), B ab Takt 6; K2: B ab Takt 1;
/// K3 (aus): A ab Takt 4.
Loop arrangement()
{
    Loop loop;
    Muster a;
    a.name = QStringLiteral("A");
    a.takte = 2;
    Muster b;
    b.name = QStringLiteral("B");
    b.takte = 1;
    loop.muster = {a, b};
    loop.kanaele = {{QStringLiteral("K1"), true, {{QStringLiteral("A"), 0, 2}, {QStringLiteral("B"), 6, 1}}},
                    {QStringLiteral("K2"), true, {{QStringLiteral("B"), 1, 1}}},
                    {QStringLiteral("K3"), false, {{QStringLiteral("A"), 4, 1}}}};
    return loop;
}

}  // namespace

TEST_SUITE("LoopMitlaufen")
{
    TEST_CASE("S127 W1 Loop: die Stelle des Arrangements als Stelle im Muster")
    {
        const Loop loop = arrangement();
        REQUIRE(loop.schritteJeTakt() == 16);
        // A spielt zweimal: Takt 0 bis 3; die Stelle im Muster laeuft zweimal von vorn
        CHECK(loop.stelleImMuster(0.0, QStringLiteral("A")) == doctest::Approx(0.0));
        CHECK(loop.stelleImMuster(20.5, QStringLiteral("A")) == doctest::Approx(20.5));
        CHECK(loop.stelleImMuster(32.0 + 5.25, QStringLiteral("A")) == doctest::Approx(5.25));  // zweite Wiederholung
        // B liegt in K2 ab Takt 1 und in K1 ab Takt 6
        CHECK(loop.stelleImMuster(16.0 + 7.0, QStringLiteral("B")) == doctest::Approx(7.0));
        CHECK(loop.stelleImMuster(96.0 + 2.0, QStringLiteral("B")) == doctest::Approx(2.0));
        CHECK(loop.stelleImMuster(40.0, QStringLiteral("B")) == -1.0);  // Takt 2: B spielt dort nicht
        // A in einem ausgeschalteten Kanal: gezeigt, wenn sonst keiner dort liegt
        CHECK(loop.stelleImMuster(64.0 + 3.0, QStringLiteral("A")) == doctest::Approx(3.0));
        // hinter dem Ende, vor dem Anfang, ein Muster, das es nicht gibt
        CHECK(loop.stelleImMuster(200.0, QStringLiteral("A")) == -1.0);
        CHECK(loop.stelleImMuster(-1.0, QStringLiteral("A")) == -1.0);
        CHECK(loop.stelleImMuster(0.0, QStringLiteral("C")) == -1.0);

        // ein eingeschalteter Kanal geht einem ausgeschalteten vor
        Loop beide = loop;
        beide.kanaele[2].clips[0].startTakt = 0;  // A auch im ausgeschalteten K3 ab Takt 0, einmal
        beide.kanaele[0].clips[0].startTakt = 1;  // A in K1 jetzt ab Takt 1
        beide.richte();
        CHECK(beide.stelleImMuster(16.0 + 4.0, QStringLiteral("A")) == doctest::Approx(4.0));  // K1, nicht K3 (dort 20)

        // S130, W8-2 a: dasselbe Muster zugleich in zwei eingeschalteten Kanaelen — beide Stellen,
        // der zuletzt begonnene zuerst (wie im Sichttest: Drums Grund ab Takt 0, Percussion Grund ab Takt 1)
        Loop doppelt = loop;
        doppelt.kanaele.push_back({QStringLiteral("K4"), true, {{QStringLiteral("A"), 1, 1}}});  // A in K4 ab Takt 1
        doppelt.richte();
        const std::vector<double> zwei = doppelt.stellenImMuster(16.0 + 5.0, QStringLiteral("A"));
        REQUIRE(zwei.size() == 2);
        CHECK(zwei[0] == doctest::Approx(5.0));         // K4, eben begonnen: Takt 0 im Muster
        CHECK(zwei[1] == doctest::Approx(16.0 + 5.0));  // K1, seit Takt 0: Takt 1 im Muster
        CHECK(doppelt.stelleImMuster(16.0 + 5.0, QStringLiteral("A")) == doctest::Approx(5.0));
        // vor dem Einsatz in K4 nur K1
        CHECK(doppelt.stellenImMuster(5.0, QStringLiteral("A")) == std::vector<double>{5.0});
        // gleicher Anfang: der obere Kanal vorn — K1 spielt A zweimal, K4 einmal: in Takt 2
        // steht K1 wieder am Anfang des Musters, K4 ist vorbei
        Loop gleich = loop;
        gleich.kanaele.push_back({QStringLiteral("K4"), true, {{QStringLiteral("A"), 0, 1}}});
        gleich.richte();
        CHECK(gleich.stellenImMuster(3.0, QStringLiteral("A")) == std::vector<double>{3.0, 3.0});
        CHECK(gleich.stellenImMuster(32.0 + 1.0, QStringLiteral("A")) == std::vector<double>{1.0});
        // ein ausgeschalteter Kanal kommt nur, wenn kein eingeschalteter dort liegt — und dann nur einer
        CHECK(beide.stellenImMuster(16.0 + 4.0, QStringLiteral("A")) == std::vector<double>{4.0});
        CHECK(loop.stellenImMuster(64.0 + 3.0, QStringLiteral("A")) == std::vector<double>{3.0});
        CHECK(loop.stellenImMuster(40.0, QStringLiteral("B")).empty());

        // Millisekunden als Schritte: ein Durchlauf ist 7 Takte, bei 120 BPM 14 s
        CHECK(loop.durchlaufSchritte() == 112);
        CHECK(loop.durchlaufMs() == doctest::Approx(14000.0));
        CHECK(loop.schritteVonMs(7000.0) == doctest::Approx(56.0));
        CHECK(loop.schritteVonMs(125.0) == doctest::Approx(1.0));
        Loop still;
        still.bpm = 0.0;
        CHECK(still.schritteVonMs(500.0) == 0.0);
    }

    TEST_CASE("S127 W3 Achse: die Regel des Mitrollens fuer jede rollende Ansicht")
    {
        using lumi::ui::Achse;
        // stetig: die Marke in der Mitte (geklemmt wird vom Aufrufer)
        CHECK(Achse::rollFuerMarke(Achse::Mitlaufen::Stetig, 500, 200, 0) == 400);
        CHECK(Achse::rollFuerMarke(Achse::Mitlaufen::Stetig, 50, 200, 0) == -50);
        // seitenweise: nur wenn die Marke den Ausschnitt verlaesst, dann steht sie links
        CHECK(Achse::rollFuerMarke(Achse::Mitlaufen::Seitenweise, 150, 200, 0) == 0);
        CHECK(Achse::rollFuerMarke(Achse::Mitlaufen::Seitenweise, 200, 200, 0) == 200);
        CHECK(Achse::rollFuerMarke(Achse::Mitlaufen::Seitenweise, 50, 200, 100) == 50);
        // aus: bleiben
        CHECK(Achse::rollFuerMarke(Achse::Mitlaufen::Aus, 900, 200, 30) == 30);
    }
}
