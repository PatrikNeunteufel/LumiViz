/**
 ****************************************************************************************
 * @file   test_LoopZeitspur.cpp
 * @brief  Loop, Stufe 28a: die Spuren ueber der Zeit in Modell und Datei
 *         (docs/audio/Spielweisen_Konzept.md §3, Entscheide Patrik S139/S140: SF5 d,
 *         SK1 a bis SK8 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Engine, ohne Dateien aus dem Repo — nur das Modell.
 *
 * - Richten: Werte in ihre Grenzen, nach der Stelle sortiert, je Stelle ein Punkt,
 *   im Muster nichts hinter dem Ende; Schalter mit Namen, je Name einer, 0 oder 1.
 * - Lesen: Stufen halten bis zum naechsten Punkt, Kurven linear, leer = Vorgabe.
 * - Die Datei: `lanes` am Instrument-Muster und `expression` am Clip stehen immer da
 *   (auch leer), Fassung 12; eine Datei ohne sie liest sich als leer; ein Schalter,
 *   den das Programm nicht kennt, bleibt stehen.
 * - Bearbeiten: *Repeat when lengthening* wiederholt die Spuren, kuerzer schneidet
 *   sie ab, ein neues Raster haelt die Stelle im Schlag; am Clip bleiben Punkte vor
 *   dem Anfang und hinter dem Ende, die linke Kante laesst die Kurve an ihrer Stelle.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/Zeitspur.hpp"

#include <QJsonArray>
#include <QJsonObject>

#include <vector>

using namespace lumi::loop;

namespace
{

/// Ein Instrument-Muster „P" von `takte` Takten (4/4, 16 Schritte je Takt) mit einer
/// Note, in einem Clip ab Takt `start` mit `mal` Wiederholungen.
Loop instrumentLoop(int takte = 1, int start = 0, int mal = 1)
{
    Loop loop;
    Muster m;
    m.name = QStringLiteral("P");
    m.art = MusterArt::Instrument;
    m.takte = takte;
    m.instrument = QStringLiteral("piano");
    m.noten = {Note{0, 4, 60}};
    loop.muster = {m};
    loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("P"), start, mal)}}};
    loop.richte();
    return loop;
}

Muster& muster(Loop& loop)
{
    return loop.muster.front();
}

LoopClip& clip(Loop& loop)
{
    return loop.kanaele.front().clips.front();
}

}  // namespace

TEST_SUITE("LoopZeitspur")
{
    TEST_CASE("Loop Zeitspur 28a: richten — Grenzen, Reihenfolge, je Stelle ein Punkt, Ende des Musters, Schalter")
    {
        Zeitspuren s;
        s.von(Steuerspur::Sustain) = {{8.0, 1.0}, {2.5, 0.5}, {20.0, 1.0}, {2.5, 2.0}, {-1.0, 1.0}};
        s.von(Steuerspur::Expression) = {{0.0, -0.3}, {4.0, 0.7}};
        s.von(Steuerspur::Vibrato) = {{0.0, 250.0}};
        s.von(Steuerspur::Pitch) = {{0.0, -30.0}, {1.0, 30.0}, {2.0, 0.5}};
        s.schalter = {SchalterSpur{QStringLiteral("unaCorda"), {{4.0, 0.7}, {0.0, 0.2}}},
                      SchalterSpur{QStringLiteral("  "), {{0.0, 1.0}}},
                      SchalterSpur{QStringLiteral("leslie"), {{1.0, 1.0}}},
                      SchalterSpur{QStringLiteral("unaCorda"), {{9.0, 1.0}}}};
        richteZeitspuren(s, 16.0);

        // Stufen 0 … 1, sortiert; an derselben Stelle gilt der spaetere; vor 0 auf 0, ab dem Ende weg
        const std::vector<SpurPunkt> sustain{{0.0, 1.0}, {2.5, 1.0}, {8.0, 1.0}};
        CHECK(s.von(Steuerspur::Sustain) == sustain);
        CHECK(s.von(Steuerspur::Expression) == std::vector<SpurPunkt>{{0.0, 0.0}, {4.0, 0.7}});
        CHECK(s.von(Steuerspur::Vibrato) == std::vector<SpurPunkt>{{0.0, kVibratoCentMax}});
        CHECK(s.von(Steuerspur::Pitch) ==
              std::vector<SpurPunkt>{{0.0, -kPitchHalbtoeneMax}, {1.0, kPitchHalbtoeneMax}, {2.0, 0.5}});
        CHECK(s.von(Steuerspur::Sostenuto).empty());

        // Schalter: ohne Namen weg, je Name der erste, nach dem Namen sortiert, an oder aus
        REQUIRE(s.schalter.size() == 2);
        CHECK(s.schalter[0].art == QStringLiteral("leslie"));
        CHECK(s.schalter[1].art == QStringLiteral("unaCorda"));
        CHECK(s.schalter[1].punkte == std::vector<SpurPunkt>{{0.0, 0.0}, {4.0, 1.0}});

        // hoechstens kZeitspurPunkteMax
        std::vector<SpurPunkt> viele;
        for (int i = 0; i < kZeitspurPunkteMax + 10; ++i) viele.push_back({i * 0.01, 0.5});
        richtePunkte(viele, 0.0, 1.0, 1000.0);
        CHECK(viele.size() == static_cast<std::size_t>(kZeitspurPunkteMax));

        CHECK(Zeitspuren{}.leer());
        CHECK_FALSE(s.leer());
        CHECK(spurMin(Steuerspur::Pitch) == -kPitchHalbtoeneMax);
        CHECK(istStufenspur(Steuerspur::Sostenuto));
        CHECK_FALSE(istStufenspur(Steuerspur::Expression));
    }

    TEST_CASE("Loop Zeitspur 28a: lesen — Stufen halten, Kurven linear, leer ist die Vorgabe")
    {
        const std::vector<SpurPunkt> pedal{{2.0, 1.0}, {6.0, 0.5}, {10.0, 0.0}};
        CHECK(stufeBei(pedal, 0.0) == 0.0);  // vor dem ersten: oben
        CHECK(stufeBei(pedal, 2.0) == 1.0);
        CHECK(stufeBei(pedal, 5.99) == 1.0);
        CHECK(stufeBei(pedal, 6.0) == 0.5);  // das halbe Pedal
        CHECK(stufeBei(pedal, 12.0) == 0.0);
        CHECK(stufeBei({}, 3.0) == 0.0);

        const std::vector<SpurPunkt> kurve{{4.0, 0.2}, {8.0, 0.6}};
        CHECK(kurveBei(kurve, 0.0, 1.0) == doctest::Approx(0.2));  // vor dem ersten: der erste
        CHECK(kurveBei(kurve, 6.0, 1.0) == doctest::Approx(0.4));
        CHECK(kurveBei(kurve, 20.0, 1.0) == doctest::Approx(0.6));
        CHECK(kurveBei({}, 3.0, 1.0) == 1.0);  // leer: die Vorgabe
    }

    TEST_CASE("Loop Zeitspur 28a: die Datei — lanes und expression immer, Fassung 12, alte Dateien, fremde Schalter")
    {
        // leer: die Felder stehen trotzdem da (Memory „Datei-Felder immer schreiben")
        const QJsonObject leer = alsJson(instrumentLoop());
        CHECK(leer.value(QStringLiteral("version")).toInt() == 12);
        const QJsonObject lanes =
            leer.value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("lanes")).toObject();
        for (const char* name : {"sustain", "sostenuto", "expression", "vibrato", "pitch"})
        {
            CAPTURE(name);
            REQUIRE(lanes.value(QLatin1String(name)).isArray());
            CHECK(lanes.value(QLatin1String(name)).toArray().isEmpty());
        }
        CHECK(lanes.value(QStringLiteral("switches")).isObject());
        const QJsonObject c = leer.value(QStringLiteral("channels")).toArray().at(0).toObject()
                                  .value(QStringLiteral("clips")).toArray().at(0).toObject();
        REQUIRE(c.value(QStringLiteral("expression")).isArray());

        // hin und zurueck, mit Bruchteilen, einem fremden Schalter und Punkten hinter dem Clip
        Loop voll = instrumentLoop(2, 0, 2);
        Zeitspuren& z = muster(voll).zeitspuren;
        z.von(Steuerspur::Sustain) = {{0.25, 1.0}, {15.5, 0.0}, {16.25, 0.5}};
        z.von(Steuerspur::Sostenuto) = {{4.0, 1.0}};
        z.von(Steuerspur::Expression) = {{0.0, 0.3}, {31.0, 1.0}};
        z.von(Steuerspur::Vibrato) = {{8.0, 0.0}, {12.0, 35.0}};
        z.von(Steuerspur::Pitch) = {{28.0, 0.0}, {31.75, -12.5}};
        z.schalter = {SchalterSpur{QStringLiteral("someFutureSwitch"), {{2.0, 1.0}}}};
        clip(voll).expression = {{-1.0, 0.1}, {0.0, 0.2}, {3.5, 1.0}, {9.0, 0.4}};  // der Clip ist 4 Takte lang
        voll.richte();
        Loop gelesen;
        REQUIRE(ausJson(alsJson(voll), gelesen));
        CHECK(gelesen == voll);
        CHECK(clip(gelesen).expression.size() == 4);  // vor dem Anfang und hinter dem Ende bleiben
        REQUIRE(muster(gelesen).zeitspuren.schalter.size() == 1);
        CHECK(muster(gelesen).zeitspuren.schalter.front().art == QStringLiteral("someFutureSwitch"));

        // eine Datei der Fassung 11 (ohne die Felder) liest sich als leer
        QJsonObject alt = alsJson(voll);
        alt[QStringLiteral("version")] = 11;
        QJsonArray muster11 = alt.value(QStringLiteral("patterns")).toArray();
        QJsonObject m11 = muster11.at(0).toObject();
        m11.remove(QStringLiteral("lanes"));
        muster11[0] = m11;
        alt[QStringLiteral("patterns")] = muster11;
        QJsonArray kanaele11 = alt.value(QStringLiteral("channels")).toArray();
        QJsonObject k11 = kanaele11.at(0).toObject();
        QJsonArray clips11 = k11.value(QStringLiteral("clips")).toArray();
        QJsonObject c11 = clips11.at(0).toObject();
        c11.remove(QStringLiteral("expression"));
        clips11[0] = c11;
        k11[QStringLiteral("clips")] = clips11;
        kanaele11[0] = k11;
        alt[QStringLiteral("channels")] = kanaele11;
        Loop alter;
        REQUIRE(ausJson(alt, alter));
        CHECK(muster(alter).zeitspuren.leer());
        CHECK(clip(alter).expression.empty());

        // ein Drumkit-Muster traegt keine Spuren — auch wenn jemand sie setzt
        Loop drums;
        Muster d;
        d.name = QStringLiteral("D");
        d.takte = 1;
        d.spuren = {Spur(QStringLiteral("Kick"), QStringLiteral("kit:kick"))};
        d.zeitspuren.von(Steuerspur::Sustain) = {{0.0, 1.0}};
        drums.muster = {d};
        drums.richte();
        CHECK(drums.muster.front().zeitspuren.leer());
        CHECK_FALSE(alsJson(drums).value(QStringLiteral("patterns")).toArray().at(0).toObject().contains(QStringLiteral("lanes")));
    }

    TEST_CASE("Loop Zeitspur 28a: bearbeiten — Repeat when lengthening, kuerzer, Raster, linke Kante des Clips")
    {
        // aus einem Takt zwei: die Spuren wiederholen sich mit den Noten
        Loop loop = instrumentLoop(1);
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{0.5, 1.0}, {12.0, 0.0}};
        muster(loop).zeitspuren.schalter = {SchalterSpur{QStringLiteral("unaCorda"), {{4.0, 1.0}}}};
        loop.richte();
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("P"), 2, true));
        CHECK(muster(loop).zeitspuren.von(Steuerspur::Sustain) ==
              std::vector<SpurPunkt>{{0.5, 1.0}, {12.0, 0.0}, {16.5, 1.0}, {28.0, 0.0}});
        CHECK(muster(loop).zeitspuren.schalter.front().punkte == std::vector<SpurPunkt>{{4.0, 1.0}, {20.0, 1.0}});
        CHECK(muster(loop).noten.size() == 2);

        // ohne Wiederholen bleiben die neuen Takte leer; kuerzer schneidet ab
        Loop ohne = instrumentLoop(1);
        muster(ohne).zeitspuren.von(Steuerspur::Expression) = {{2.0, 0.5}};
        ohne.richte();
        REQUIRE(setzeMusterTakte(ohne, QStringLiteral("P"), 2, false));
        CHECK(muster(ohne).zeitspuren.von(Steuerspur::Expression) == std::vector<SpurPunkt>{{2.0, 0.5}});
        REQUIRE(setzeMusterTakte(loop, QStringLiteral("P"), 1, false));
        CHECK(muster(loop).zeitspuren.von(Steuerspur::Sustain) == std::vector<SpurPunkt>{{0.5, 1.0}, {12.0, 0.0}});

        // ein neues Raster: 4 Schritte je Schlag -> 3; die Stelle im Schlag bleibt, auch zwischen Schritten
        Loop raster = instrumentLoop(1);
        muster(raster).zeitspuren.von(Steuerspur::Pitch) = {{2.0, 1.0}, {5.0, 2.0}, {14.0, 3.0}};
        raster.richte();
        REQUIRE(setzeRaster(raster, 4, 3));
        CHECK(muster(raster).zeitspuren.von(Steuerspur::Pitch) ==
              std::vector<SpurPunkt>{{1.5, 1.0}, {3.75, 2.0}, {10.5, 3.0}});
        // 4/4 -> 3/4: ein Punkt im vierten Schlag geht
        REQUIRE(setzeRaster(raster, 3, 3));
        CHECK(muster(raster).zeitspuren.von(Steuerspur::Pitch) == std::vector<SpurPunkt>{{1.5, 1.0}, {3.75, 2.0}});

        // die linke Kante eines Clips: die Kurve bleibt an ihrer Stelle im Arrangement
        Loop kante = instrumentLoop(1, 4, 2);  // Takt 4 bis 6
        clip(kante).expression = {{0.0, 0.2}, {2.0, 1.0}};
        kante.richte();
        REQUIRE(setzeAnfang(kante, 0, 0, 2) == 0);  // jetzt Takt 2 bis 6
        CHECK(clip(kante).startTakt == 2);
        CHECK(clip(kante).expression == std::vector<SpurPunkt>{{2.0, 0.2}, {4.0, 1.0}});
        REQUIRE(setzeAnfang(kante, 0, 0, 5) == 0);  // jetzt Takt 5 bis 6
        CHECK(clip(kante).startTakt == 5);
        CHECK(clip(kante).expression == std::vector<SpurPunkt>{{-1.0, 0.2}, {1.0, 1.0}});  // davor bleibt
        kante.richte();
        CHECK(clip(kante).expression.size() == 2);
    }
}
