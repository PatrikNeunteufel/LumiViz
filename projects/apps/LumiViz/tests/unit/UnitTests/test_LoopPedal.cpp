/**
 ****************************************************************************************
 * @file   test_LoopPedal.cpp
 * @brief  Loop, Stufe 28b: Sustain und Sostenuto halten Noten in der Planung
 *         (docs/audio/Spielweisen_Konzept.md §3, Entscheide Patrik S140: SK5 a, SK6 a,
 *         SF6 a, SF7 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Engine, ohne Dateien aus dem Repo: die Toene rechnet
 * ein Instrument, das der Test selbst baut.
 *
 * - Die Regel: Sustain haelt bis zum Loslassen, an der Naht nur mit einem gedrueckten
 *   Punkt am Anfang, am Ende des Clips nie weiter; die Schwelle 0,5 (SK6 a); Sostenuto
 *   faengt nur, was beim Treten klingt; derselbe Ton nimmt den gehaltenen ab.
 * - Die Namen: ohne Pedal wie bis Stufe 27, mit Pedal je Vorkommen; `plane` findet
 *   jeden Klang, den `klangNamen` nennt.
 * - Der Klang: hinter dem Ende der Note klingt der gehaltene Ton weiter, der andere
 *   ist still — gemessen an gerechneten Toenen.
 * - Mit Saiten: die Saite haelt nicht (SF6 a); die Schichten mit Own note halten als
 *   eigener Ton (SF7 a).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Pedalplan.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Saitenplan.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Ein Takt (16 Schritte) des Instruments „probe" mit diesen Noten, als Clip mit `mal`
/// Wiederholungen; 120 BPM — ein Schritt sind 125 ms.
Loop pedalLoop(std::vector<Note> noten, int mal = 1)
{
    Loop loop;
    loop.laut = 1.0;
    Muster m;
    m.name = QStringLiteral("P");
    m.art = MusterArt::Instrument;
    m.takte = 1;
    m.instrument = QStringLiteral("probe");
    m.noten = std::move(noten);
    loop.muster = {m};
    loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("P"), 0, mal)}}};
    loop.richte();
    return loop;
}

Muster& muster(Loop& loop)
{
    return loop.muster.front();
}

double bis(const Loop& loop, int runde, std::size_t note)
{
    return gehaltenBis(loop, loop.muster.front(), loop.kanaele.front().clips.front().wiederholungen, runde, note);
}

/// Ein Sinus ohne Saiten, mit kurzem Ausklang.
Instrument probe()
{
    Instrument instrument;
    instrument.name = QStringLiteral("probe");
    instrument.schichten[0].ausklangMs = 50.0;
    return instrument;
}

/// Der Durchlauf, gerechnet: die Klaenge rechnet `probe()` aus ihren Namen.
std::vector<float> rechneLoop(const Loop& loop)
{
    const Ablauf ablauf = plane(loop, kRate);
    Klaenge klaenge;
    for (const QString& name : ablauf.klangNamen)
    {
        TonAngabe ton;
        REQUIRE(zerlegeTon(name, ton));
        klaenge.push_back(std::make_shared<const Klang>(rendereTon(probe(), ton, kRate)));
    }
    std::vector<float> aus(static_cast<std::size_t>(ablauf.durchlaufFrames) * 2);
    rechne(ablauf, klaenge, 0, static_cast<int>(ablauf.durchlaufFrames), aus.data());
    return aus;
}

/// Der Effektivwert links zwischen zwei Stellen in ms.
double effektiv(const std::vector<float>& aus, double vonMs, double bisMs)
{
    const auto von = static_cast<std::size_t>(vonMs * kRate / 1000.0);
    const auto bis = std::min(aus.size() / 2, static_cast<std::size_t>(bisMs * kRate / 1000.0));
    double summe = 0.0;
    for (std::size_t f = von; f < bis; ++f) summe += static_cast<double>(aus[2 * f]) * aus[2 * f];
    return bis > von ? std::sqrt(summe / static_cast<double>(bis - von)) : 0.0;
}

}  // namespace

TEST_SUITE("LoopPedal")
{
    TEST_CASE("Loop Pedal 28b: Sustain haelt bis zum Loslassen — Naht, Clip-Ende, Schwelle, derselbe Ton")
    {
        // ohne Pedal: das Ende der Note
        Loop loop = pedalLoop({Note{0, 4, 60}, Note{14, 2, 64}}, 2);
        CHECK_FALSE(hatPedal(muster(loop)));
        CHECK(bis(loop, 0, 0) == 4.0);

        // gedrueckt bei 2, los bei 10: die Note (Ende 4) haelt bis 10
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{2.0, 1.0}, {10.0, 0.0}};
        CHECK(hatPedal(muster(loop)));
        CHECK(bis(loop, 0, 0) == 10.0);
        CHECK(bis(loop, 1, 0) == 26.0);  // in der zweiten Wiederholung genauso

        // nie losgelassen, aber kein Punkt am Anfang: an der Naht ist es oben (SK5 a)
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{2.0, 1.0}};
        CHECK(bis(loop, 0, 1) == 16.0);
        // ein gedrueckter Punkt am Anfang: die Note haelt ueber die Naht, bis derselbe Ton neu anschlaegt
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{0.0, 1.0}};
        CHECK(bis(loop, 0, 1) == 30.0);  // die 64 schlaegt bei 16 + 14 wieder an
        CHECK(bis(loop, 0, 0) == 16.0);  // die 60 schlaegt bei 16 wieder an
        // am letzten Ende des Clips wird es losgelassen
        CHECK(bis(loop, 1, 1) == 32.0);
        CHECK(bis(loop, 1, 0) == 32.0);

        // die Schwelle an einer Synth-Schicht (SK6 a): 0,4 haelt nicht, 0,5 haelt
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{2.0, 0.4}, {10.0, 0.0}};
        CHECK(bis(loop, 0, 0) == 4.0);
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{2.0, 0.5}, {10.0, 0.0}};
        CHECK(bis(loop, 0, 0) == 10.0);

        // losgelassen genau am Ende der Note: nicht gehalten
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{2.0, 1.0}, {4.0, 0.0}};
        CHECK(bis(loop, 0, 0) == 4.0);
    }

    TEST_CASE("Loop Pedal 28b: Sostenuto faengt nur, was beim Treten klingt")
    {
        // A klingt beim Treten (2), D schlaegt im selben Moment an, C erst danach
        Loop loop = pedalLoop({Note{0, 4, 60}, Note{2, 2, 72}, Note{3, 2, 67}});
        muster(loop).zeitspuren.von(Steuerspur::Sostenuto) = {{2.0, 1.0}, {12.0, 0.0}};
        loop.richte();
        // nach `richte` nach Schritt, dann Tonhoehe sortiert: 60 (0), 72 (2), 67 (3)
        REQUIRE(muster(loop).noten[1].hoehe == 72);
        CHECK(bis(loop, 0, 0) == 12.0);
        CHECK(bis(loop, 0, 1) == 12.0);
        CHECK(bis(loop, 0, 2) == 5.0);
        // Sustain und Sostenuto zusammen: das laengere Halten gilt
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{4.5, 1.0}, {14.0, 0.0}};
        CHECK(bis(loop, 0, 2) == 14.0);
        CHECK(bis(loop, 0, 0) == 12.0);  // endet bei 4, Sustain kommt erst bei 4,5
    }

    TEST_CASE("Loop Pedal 28b: Namen und Planung — ohne Pedal wie bisher, mit Pedal je Vorkommen")
    {
        Loop ohne = pedalLoop({Note{0, 4, 60}}, 2);
        CHECK(ohne.klangNamen() == QStringList{tonKlang(QStringLiteral("probe"), 60, ohne.notenMs(ohne.muster.front().noten.front()))});
        CHECK(gehaltenMs(ohne, ohne.muster.front(), 2, 0, 0) == 500);

        Loop mit = ohne;
        muster(mit).zeitspuren.von(Steuerspur::Sustain) = {{0.0, 1.0}, {8.0, 0.0}};
        mit.richte();
        // 0 bis 8 Schritte = 1000 ms; in beiden Wiederholungen dasselbe, also ein Name
        CHECK(gehaltenMs(mit, mit.muster.front(), 2, 0, 0) == 1000);
        CHECK(mit.klangNamen() == QStringList{tonKlang(QStringLiteral("probe"), 60, 1000)});

        // die Planung findet jeden Klang, den die Namen nennen — eine Note ab Schritt 2, das Pedal
        // immer gedrueckt: in der ersten Wiederholung bis zum neuen Anschlag (16 Schritte), in der
        // zweiten bis zum Ende des Clips (14) — zwei Dauern
        Loop zwei = pedalLoop({Note{2, 4, 60}}, 2);
        muster(zwei).zeitspuren.von(Steuerspur::Sustain) = {{0.0, 1.0}};
        zwei.richte();
        CHECK(gehaltenMs(zwei, zwei.muster.front(), 2, 0, 0) == 2000);
        CHECK(gehaltenMs(zwei, zwei.muster.front(), 2, 1, 0) == 1750);
        const Ablauf ablauf = plane(zwei, kRate);
        CHECK(ablauf.klangNamen.size() == 2);
        for (const Stimme& stimme : ablauf.stimmen)
        {
            CHECK(stimme.klang >= 0);
            CHECK(stimme.klang < ablauf.klangNamen.size());
        }
    }

    TEST_CASE("Loop Pedal 28b: gemessen — hinter dem Ende der Note klingt nur der gehaltene Ton")
    {
        // eine Note von 4 Schritten (500 ms), das Pedal bis Schritt 12 (1500 ms)
        Loop ohne = pedalLoop({Note{0, 4, 69}});
        Loop mit = ohne;
        muster(mit).zeitspuren.von(Steuerspur::Sustain) = {{0.0, 1.0}, {12.0, 0.0}};
        mit.richte();
        const std::vector<float> trocken = rechneLoop(ohne);
        const std::vector<float> gehalten = rechneLoop(mit);
        // vor dem Ende der Note gleich (derselbe Ton, nur laenger gehalten)
        const double vorn = effektiv(trocken, 100.0, 400.0);
        REQUIRE(vorn > 0.0);
        CHECK(effektiv(gehalten, 100.0, 400.0) == doctest::Approx(vorn).epsilon(0.01));
        // zwischen dem Ende des Ausklangs (500 + 50 ms) und dem Loslassen
        const double still = effektiv(trocken, 700.0, 1400.0);
        const double klingt = effektiv(gehalten, 700.0, 1400.0);
        MESSAGE("Effektivwert: vorn " << vorn << ", 700 bis 1400 ms ohne Pedal " << still << ", mit " << klingt);
        CHECK(still < 0.01 * vorn);
        CHECK(klingt > 0.5 * vorn);
        // nach dem Loslassen (1500 ms) und dem Ausklang still
        CHECK(effektiv(gehalten, 1700.0, 1990.0) < 0.01 * vorn);
    }

    TEST_CASE("Loop Pedal 28b: mit Saiten haelt die Saite nicht, die Schichten mit Own note als eigener Ton")
    {
        const InstrumentVon saiten = [](const QString&) {
            Saitensatz satz;
            satz.leer = stimmungsvorlagen()[0].leer;
            satz.mensurMm = 648.0;
            return satz;
        };
        Loop loop = pedalLoop({Note{0, 4, 64}, Note{8, 2, 69}});
        muster(loop).zeitspuren.von(Steuerspur::Sustain) = {{2.0, 1.0}, {7.0, 0.0}};
        loop.richte();
        const std::vector<ClipToene> toene = toeneImKanal(loop, 0, saiten);
        REQUIRE(toene.size() == 1);
        REQUIRE(toene[0].size() == 1);
        const GeplanterTon& gehalten = toene[0][0][0];
        CHECK(gehalten.teil == TonTeil::Folgend);  // die Saite und was ihr folgt
        CHECK(gehalten.dauerMs == loop.notenMs(muster(loop).noten[0]));
        REQUIRE(gehalten.zusatz.size() == 1);
        CHECK(gehalten.zusatz[0].teil == TonTeil::Eigen);
        CHECK(gehalten.zusatz[0].dauerMs == 875);  // 7 Schritte zu 125 ms
        CHECK(gehalten.zusatz[0].endeMs == 0);     // keine Saite daempft sie
        const GeplanterTon& frei = toene[0][0][1];  // endet bei 10, das Pedal ist oben
        CHECK(frei.teil == TonTeil::Alle);
        CHECK(frei.zusatz.empty());

        // die Namen und die Planung passen zusammen
        const QStringList namen = loop.klangNamen(saiten);
        CHECK(namen.contains(tonKlang(gehalten.zusatz[0])));
        const Ablauf ablauf = plane(loop, kRate, saiten);
        for (const Stimme& stimme : ablauf.stimmen) CHECK(stimme.klang >= 0);
    }
}
