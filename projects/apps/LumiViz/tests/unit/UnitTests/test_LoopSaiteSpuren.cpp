/**
 ****************************************************************************************
 * @file   test_LoopSaiteSpuren.cpp
 * @brief  Loop, Stufe 28c-2a: Vibrato und Pitch an der Saite, der Vibratohebel
 *         (docs/audio/Spielweisen_Konzept.md §3, Entscheide Patrik S140: SF6 a, SF7 a,
 *         SK3 a, SK10 a, SK11 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Dateien aus dem Repo; gemessen an gerechneten Toenen
 * (`spitze` aus SaitenMessung.hpp).
 *
 * - Die Datei: `tremoloDown`, `tremoloUp` immer, Fassung 15, in ihre Grenzen.
 * - Die Zeile Tuning: *Tremolo arm* an setzt 2 nach unten und 1 nach oben, aus beide 0.
 * - Die Saite: Pitch nur mit Hebel, in seinen Grenzen; Vibrato immer (der Finger), ohne
 *   eigene Rate 5,5 Hz; ohne Spuren derselbe Ton Bit fuer Bit.
 * - Was der Saite folgt: mit Hebel in seinen Grenzen, ohne mit der eigenen Pitch range;
 *   eine Schicht mit Own note immer mit der eigenen.
 * - Die Planung: die Toene eines Saiteninstruments tragen die Spuren.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Saiten.hpp"
#include "loop/SaitenZeile.hpp"
#include "loop/Saitenplan.hpp"

#include "SaitenMessung.hpp"  // spitze, cent

#include <QApplication>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace lumi::loop;
using namespace saitentest;

namespace
{

constexpr int kRate = kSaitenRate;

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

Saitensatz gitarre(double runter = 0.0, double hoch = 0.0)
{
    Saitensatz satz;
    satz.leer = stimmungsvorlagen()[0].leer;  // E2 A2 D3 G3 B3 E4
    satz.mensurMm = 648.0;
    satz.hebelRunterHt = runter;
    satz.hebelHochHt = hoch;
    return satz;
}

/// Eine Gitarre mit einer Saite *Extended KS*, wie in test_LoopSpielweiseKlang.
Instrument saite(double runter = 0.0, double hoch = 0.0)
{
    Instrument instrument;
    instrument.saiten = gitarre(runter, hoch);
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::ErweiterterKs;
    s.anstiegMs = 1.0;
    s.ausklangMs = 200.0;
    s.hoehenDaempfung = 0.0;
    s.steifheit = 0.0;
    s.anschlagHaerte = 1.0;
    s.klingSekunden = 3.2;
    s.saitenUnterschied = 0.0;
    return instrument;
}

/// Ein Instrument mit Saiten, aber nur einer Sinus-Schicht — mit oder ohne Follow string.
Instrument folger(bool folgt, double runter = 0.0, double hoch = 0.0)
{
    Instrument instrument;
    instrument.saiten = gitarre(runter, hoch);
    Schicht& s = instrument.schichten[0];
    s.anstiegMs = 1.0;
    s.abfallMs = 0.0;
    s.halten = 1.0;
    s.folgtSaite = folgt;
    return instrument;
}

TonAngabe ton(int hoehe, int saiteNr, int dauerMs = 1500)
{
    TonAngabe t;
    t.instrument = QStringLiteral("probe");
    t.hoehe = hoehe;
    t.dauerMs = dauerMs;
    t.saite = saiteNr;
    return t;
}

/// Die Tonhoehe zwischen 0,2 und 0,7 s in Cent gegen `soll`.
double centBei(const Klang& klang, double soll)
{
    return cent(spitze(klang, soll, 400.0, 0.2, 0.5), soll);
}

}  // namespace

TEST_SUITE("LoopSaiteSpuren")
{
    TEST_CASE("Loop Saite Spuren 28c-2a: der Vibratohebel in der Datei und in der Zeile Tuning")
    {
        Instrument instrument = saite(3.0, 1.5);
        const QJsonObject json = alsJson(instrument);
        CHECK(kInstrumentFassung >= 15);  // den Hebel gibt es seit Fassung 15
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);  // geschrieben wird die juengste
        const QJsonObject saiten = json.value(QStringLiteral("strings")).toObject();
        CHECK(saiten.value(QStringLiteral("tremoloDown")).toDouble() == 3.0);
        CHECK(saiten.value(QStringLiteral("tremoloUp")).toDouble() == 1.5);
        Instrument gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen.saiten == instrument.saiten);
        // ohne Hebel stehen beide da, als 0 (Memory „Datei-Felder immer schreiben")
        CHECK(alsJson(saite()).value(QStringLiteral("strings")).toObject().contains(QStringLiteral("tremoloDown")));
        CHECK_FALSE(saite().saiten.hatHebel());
        // in ihre Grenzen
        Saitensatz zuViel = gitarre(-1.0, 99.0);
        richteSaiten(zuViel);
        CHECK(zuViel.hebelRunterHt == 0.0);
        CHECK(zuViel.hebelHochHt == kHebelMaxHt);

        // die Zeile Tuning
        Anwendung anwendung;
        SaitenZeile zeile;
        zeile.zeige(gitarre());
        auto* haken = zeile.findChild<QCheckBox*>(QStringLiteral("creatorTremoloArm"));
        auto* runter = zeile.findChild<QDoubleSpinBox*>(QStringLiteral("creatorTremoloDown"));
        REQUIRE(haken != nullptr);
        REQUIRE(runter != nullptr);
        CHECK_FALSE(haken->isChecked());
        Saitensatz gemeldet;
        QObject::connect(&zeile, &SaitenZeile::geaendert, [&gemeldet](const Saitensatz& s) { gemeldet = s; });
        haken->setChecked(true);
        CHECK(gemeldet.hebelRunterHt == kHebelRunterVorgabe);
        CHECK(gemeldet.hebelHochHt == kHebelHochVorgabe);
        runter->setValue(4.5);
        CHECK(gemeldet.hebelRunterHt == 4.5);
        haken->setChecked(false);
        CHECK_FALSE(gemeldet.hatHebel());
    }

    TEST_CASE("Loop Saite Spuren 28c-2a: Pitch an der Saite nur mit Hebel, in seinen Grenzen — gemessen")
    {
        const double e4 = 440.0 * std::exp2((64 - 69) / 12.0);
        TonAngabe a = ton(64, 5);  // die hohe E-Saite, leer
        const Klang ohneSpur = rendereTon(saite(2.0, 1.0), a, kRate);
        a.tonhoehe = {{0, 100}};
        // ohne Hebel: die Saite ueberhoert die Spur — Bit fuer Bit wie ohne
        const Klang ohneHebel = rendereTon(saite(), a, kRate);
        CHECK(ohneHebel.daten == rendereTon(saite(), ton(64, 5), kRate).daten);
        CHECK(std::abs(centBei(ohneHebel, e4)) < 3.0);
        // mit Hebel: einen Halbton hoeher (die Grenze nach oben ist 1)
        const double hoeher = centBei(rendereTon(saite(2.0, 1.0), a, kRate), e4);
        MESSAGE("Hebel +1: " << hoeher << " ct");
        CHECK(hoeher == doctest::Approx(100.0).epsilon(0.05));
        // +3 Halbtoene: auf die Grenze nach oben gekappt; −3: auf die nach unten (2)
        a.tonhoehe = {{0, 300}};
        CHECK(centBei(rendereTon(saite(2.0, 1.0), a, kRate), e4) == doctest::Approx(100.0).epsilon(0.05));
        a.tonhoehe = {{0, -300}};
        const double tiefer = centBei(rendereTon(saite(2.0, 1.0), a, kRate), e4);
        MESSAGE("Hebel −3, Grenze 2: " << tiefer << " ct");
        CHECK(tiefer == doctest::Approx(-200.0).epsilon(0.05));
        // ein tiefer Fall: der Ring waechst mit — der Ton bleibt da und rein
        a.tonhoehe = {{0, -2400}};
        const Klang fall = rendereTon(saite(24.0, 0.0), a, kRate);
        const double zweiOktaven = centBei(fall, e4 / 4.0);
        MESSAGE("Hebel −24: " << zweiOktaven << " ct neben E2");
        CHECK(std::abs(zweiOktaven) < 10.0);
        CHECK(ohneSpur.frames() > 0);
    }

    TEST_CASE("Loop Saite Spuren 28c-2a: Vibrato an der Saite — ohne Rate 5,5 Hz, mit der Tiefe der Spur")
    {
        const double e4 = 440.0 * std::exp2((64 - 69) / 12.0);
        TonAngabe a = ton(64, 5, 2000);
        a.vibrato = {{0, 50}};
        const Klang klang = rendereTon(saite(), a, kRate);  // auch ohne Hebel: der Finger
        double hoch = -1e9;
        double tief = 1e9;
        for (double s = 0.2; s < 1.6; s += 0.02)
        {
            const double c = cent(spitze(klang, e4, 120.0, s, 0.03), e4);
            hoch = std::max(hoch, c);
            tief = std::min(tief, c);
        }
        MESSAGE("Vibrato 50 ct an der Saite: " << tief << " bis " << hoch << " ct");
        CHECK(hoch > 30.0);
        CHECK(tief < -30.0);
        // ohne Spur steht der Ton
        const Klang still = rendereTon(saite(), ton(64, 5, 2000), kRate);
        double h2 = -1e9;
        double t2 = 1e9;
        for (double s = 0.2; s < 1.6; s += 0.02)
        {
            const double c = cent(spitze(still, e4, 120.0, s, 0.03), e4);
            h2 = std::max(h2, c);
            t2 = std::min(t2, c);
        }
        CHECK(h2 - t2 < 15.0);
    }

    TEST_CASE("Loop Saite Spuren 28c-2a: was der Saite folgt — mit Hebel in seinen Grenzen, ohne mit der eigenen Pitch range")
    {
        TonAngabe a = ton(69, 4);  // 440 Hz
        a.tonhoehe = {{0, 300}};
        // Follow string mit Hebel (oben 1): folgt der Saite
        CHECK(centBei(rendereTon(folger(true, 2.0, 1.0), a, kRate), 440.0) == doctest::Approx(100.0).epsilon(0.02));
        // Follow string ohne Hebel: rechnet selbst, mit ihrer Pitch range (2) — SF7 a
        CHECK(centBei(rendereTon(folger(true), a, kRate), 440.0) == doctest::Approx(200.0).epsilon(0.02));
        // Own note: immer die eigene Pitch range, auch mit Hebel
        CHECK(centBei(rendereTon(folger(false, 2.0, 1.0), a, kRate), 440.0) == doctest::Approx(200.0).epsilon(0.02));
    }

    TEST_CASE("Loop Saite Spuren 28c-2a: die Planung — die Toene eines Saiteninstruments tragen die Spuren")
    {
        Loop loop;
        Muster m;
        m.name = QStringLiteral("G");
        m.art = MusterArt::Instrument;
        m.takte = 1;
        m.instrument = QStringLiteral("probe");
        m.noten = {Note{0, 8, 64}, Note{8, 8, 67}};
        m.zeitspuren.von(Steuerspur::Pitch) = {{0.0, 0.0}, {8.0, -1.0}};
        m.zeitspuren.von(Steuerspur::Vibrato) = {{0.0, 30.0}};
        loop.muster = {m};
        loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("G"), 0, 1)}}};
        loop.richte();
        const SaitenVon saiten = [](const QString&) { return gitarre(2.0, 1.0); };
        const std::vector<ClipToene> toene = toeneImKanal(loop, 0, saiten);
        REQUIRE(toene.size() == 1);
        const GeplanterTon& erste = toene[0][0][0];
        CHECK(erste.tonhoehe == std::vector<TonPunkt>{{0, 0}, {1000, -100}});
        CHECK(erste.vibrato == std::vector<TonPunkt>{{0, 30}, {1000, 30}});
        const QStringList namen = loop.klangNamen(saiten);
        CHECK(namen.contains(tonKlang(erste)));
        const Ablauf ablauf = plane(loop, kRate, saiten);
        for (const Stimme& stimme : ablauf.stimmen) CHECK(stimme.klang >= 0);
        // ohne Spuren derselbe Name wie bis Stufe 27
        Loop ohne = loop;
        ohne.muster.front().zeitspuren = Zeitspuren{};
        const std::vector<ClipToene> ohneToene = toeneImKanal(ohne, 0, saiten);
        const GeplanterTon& schlicht = ohneToene[0][0][0];
        CHECK(schlicht.tonhoehe.empty());
        CHECK(schlicht.vibrato.empty());
    }
}
