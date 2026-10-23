/**
 ****************************************************************************************
 * @file   test_LoopSaiteExpression.cpp
 * @brief  Loop, Stufe 28c-2b: Expression an einem Instrument mit Saiten
 *         (docs/audio/Spielweisen_Konzept.md §3, Entscheide Patrik S140/S141: SF6 a,
 *         SF7 a, SK4 a, SK8 a, SK9 a, SK12 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Dateien aus dem Repo; gemessen an gerechneten Toenen.
 *
 * - Der Name: `:x…` (Spur des Musters) und `:y…` (Kurve am Clip), Tausendstel 0 … 1000,
 *   zerlegt sich wieder; ohne Expression derselbe Name wie vorher.
 * - Die Saite: mit Tonabnehmer e = 0,5 → −12 dB; ohne Tonabnehmer und *Extended KS* Bit
 *   fuer Bit wie ohne (SF6 a). Mit Drive sitzt der Pegel davor (SK12 a): der Ton wird
 *   weniger leise als e².
 * - Jede andere Schicht nimmt Expression immer (SF7 a), auch im Ausklang.
 * - Die Planung: die Toene tragen Spur und Kurve ab ihrem Anschlag bis zum Ende des Clips
 *   oder ihrem eigenen; zwei Clips mit verschiedenen Kurven haben verschiedene Toene; der
 *   Rechenkern legt bei Saiten keinen zweiten Pegel darueber.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Saiten.hpp"
#include "loop/Saitenplan.hpp"

#include "SaitenMessung.hpp"  // kSaitenRate

#include <QSet>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>
#include <vector>

using namespace lumi::loop;
using namespace saitentest;

namespace
{

constexpr int kRate = kSaitenRate;

Saitensatz gitarre()
{
    Saitensatz satz;
    satz.leer = stimmungsvorlagen()[0].leer;  // E2 A2 D3 G3 B3 E4
    satz.mensurMm = 648.0;
    return satz;
}

/// Eine Gitarre mit einer Saite *Waveguide*, mit einem Single coil am Steg oder akustisch.
Instrument welle(bool abnehmer, double drive = 0.0)
{
    Instrument instrument;
    instrument.saiten = gitarre();
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::Wellenleiter;
    s.anstiegMs = 1.0;
    s.klingSekunden = 2.0;
    s.saitenUnterschied = 0.0;
    if (abnehmer)
    {
        s.abnehmerSteg = Abnehmer{AbnehmerArt::SingleCoil, 40.0};
        s.abnehmerWahl = AbnehmerWahl::Steg;
    }
    if (drive > 0.0) s.holeDrive().wert = drive;
    return instrument;
}

/// Eine Gitarre mit einer Saite *Extended KS*.
Instrument ks()
{
    Instrument instrument = welle(false);
    instrument.schichten[0].saitenModell = SaitenModell::ErweiterterKs;
    return instrument;
}

/// Ein Instrument mit Saiten und einer Sinus-Schicht, die nach dem Loslassen lange ausklingt.
Instrument sinus()
{
    Instrument instrument;
    instrument.saiten = gitarre();
    Schicht& s = instrument.schichten[0];
    s.anstiegMs = 1.0;
    s.abfallMs = 0.0;
    s.halten = 1.0;
    s.ausklangMs = 1500.0;
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

/// Der Effektivwert der linken Seite ab `vonS`, `langS` lang.
double effektiv(const Klang& klang, double vonS, double langS)
{
    const auto von = static_cast<std::int64_t>(vonS * kRate);
    const auto bis = std::min(klang.frames(), von + static_cast<std::int64_t>(langS * kRate));
    double summe = 0.0;
    std::int64_t n = 0;
    for (std::int64_t f = von; f < bis; ++f, ++n)
    {
        const double w = klang.daten[static_cast<std::size_t>(2 * f)];
        summe += w * w;
    }
    return n > 0 ? std::sqrt(summe / static_cast<double>(n)) : 0.0;
}

Muster gitarrenMuster(std::vector<Note> noten)
{
    Muster m;
    m.name = QStringLiteral("G");
    m.art = MusterArt::Instrument;
    m.takte = 1;
    m.instrument = QStringLiteral("probe");
    m.noten = std::move(noten);
    return m;
}

}  // namespace

TEST_SUITE("LoopSaiteExpression")
{
    TEST_CASE("Loop Saite Expression 28c-2b: der Name mit :x und :y")
    {
        TonAngabe a = ton(64, 5, 500);
        const QString ohne = tonKlang(a);
        a.ausdruckMuster = {{0, 1000}, {500, 250}};
        a.ausdruckClip = {{0, 800}, {2000, 0}};
        a.endeMs = 1200;
        const QString name = tonKlang(a);
        CHECK(name == QStringLiteral("inst:probe:64:500:s5:x0=1000,500=250:y0=800,2000=0:e1200"));
        TonAngabe gelesen;
        REQUIRE(zerlegeTon(name, gelesen));
        CHECK(gelesen == a);
        // ausserhalb von 0 … 1000 ist es kein Ton
        CHECK_FALSE(zerlegeTon(QStringLiteral("inst:probe:64:500:x0=1001"), gelesen));
        // ohne Expression derselbe Name wie bis Stufe 28c-2a
        CHECK(ohne == QStringLiteral("inst:probe:64:500:s5"));
    }

    TEST_CASE("Loop Saite Expression 28c-2b: an der Saite nur mit Tonabnehmer — gemessen")
    {
        TonAngabe halb = ton(64, 5);
        halb.ausdruckMuster = {{0, 500}};
        // mit Tonabnehmer: der Pegel e² = 0,25, Wert fuer Wert (ohne Drive ist die Reihe linear)
        const Klang voll = rendereTon(welle(true), ton(64, 5), kRate);
        const Klang leise = rendereTon(welle(true), halb, kRate);
        REQUIRE(voll.frames() == leise.frames());
        const double verhaeltnis = effektiv(leise, 0.0, 1.0) / effektiv(voll, 0.0, 1.0);
        MESSAGE("Tonabnehmer, e = 0,5: " << db(verhaeltnis) << " dB");
        CHECK(verhaeltnis == doctest::Approx(0.25).epsilon(1e-4));
        // akustisch und Extended KS: die Saite ueberhoert Expression, Bit fuer Bit
        CHECK(rendereTon(welle(false), halb, kRate).daten == rendereTon(welle(false), ton(64, 5), kRate).daten);
        CHECK(rendereTon(ks(), halb, kRate).daten == rendereTon(ks(), ton(64, 5), kRate).daten);
    }

    TEST_CASE("Loop Saite Expression 28c-2b: mit Drive sitzt der Pegel vor dem Drive (SK12 a)")
    {
        TonAngabe halb = ton(64, 5);
        halb.ausdruckMuster = {{0, 500}};
        const Klang voll = rendereTon(welle(true, 0.75), ton(64, 5), kRate);
        const Klang leise = rendereTon(welle(true, 0.75), halb, kRate);
        const double verhaeltnis = effektiv(leise, 0.0, 1.0) / effektiv(voll, 0.0, 1.0);
        MESSAGE("Tonabnehmer mit Drive 0,75, e = 0,5: " << db(verhaeltnis) << " dB");
        // der Drive drueckt den lauten Ton mehr als den leisen: weniger leise als −12 dB
        CHECK(verhaeltnis > 0.255);
        CHECK(verhaeltnis < 1.0);
    }

    TEST_CASE("Loop Saite Expression 28c-2b: jede andere Schicht immer, auch im Ausklang")
    {
        // eine Sinus-Schicht neben der akustischen Saite: nur sie wird leiser
        Instrument beide = welle(false);
        Schicht s = sinus().schichten[0];
        s.einsatzMs = 0.0;
        beide.schichten.push_back(s);
        Instrument nurSaite = welle(false);
        Instrument nurSinus = sinus();
        TonAngabe halb = ton(64, 5);
        halb.ausdruckMuster = {{0, 500}};
        const Klang mit = rendereTon(beide, halb, kRate);
        const Klang saite = rendereTon(nurSaite, ton(64, 5), kRate);
        const Klang klar = rendereTon(nurSinus, ton(64, 5), kRate);
        double abweichung = 0.0;
        const auto frames = std::min({mit.frames(), saite.frames(), klar.frames()});
        for (std::int64_t f = 0; f < frames; ++f)
        {
            const auto i = static_cast<std::size_t>(2 * f);
            abweichung = std::max(abweichung, std::abs(mit.daten[i] - saite.daten[i] - 0.25 * klar.daten[i]));
        }
        MESSAGE("Saite + 0,25 · Sinus, groesste Abweichung: " << abweichung);
        CHECK(abweichung < 1e-5);

        // im Ausklang: die Kurve am Clip faellt von 1 auf 0 ueber zwei Sekunden, die Note ist
        // nach 0,3 s zu Ende — bei 1,0 s ist e = 0,5 und der Ausklang ein Viertel so laut
        TonAngabe kurz = ton(69, 4, 300);
        TonAngabe fallend = kurz;
        fallend.ausdruckClip = {{0, 1000}, {2000, 0}};
        const Klang ohneKurve = rendereTon(nurSinus, kurz, kRate);
        const Klang mitKurve = rendereTon(nurSinus, fallend, kRate);
        const double imAusklang = effektiv(mitKurve, 0.99, 0.02) / effektiv(ohneKurve, 0.99, 0.02);
        MESSAGE("Ausklang bei 1,0 s: " << imAusklang);
        CHECK(imAusklang == doctest::Approx(0.25).epsilon(0.02));
    }

    TEST_CASE("Loop Saite Expression 28c-2b: die Planung — Spur und Kurve ueber jedem Ton")
    {
        Loop loop;
        // E2 nur auf der tiefsten Saite, E4 auf einer anderen: keiner daempft den anderen im Clip
        Muster m = gitarrenMuster({Note{0, 8, 40}, Note{8, 8, 64}});
        m.zeitspuren.von(Steuerspur::Expression) = {{0.0, 1.0}, {8.0, 0.5}};
        loop.muster = {m};
        LoopClip mitKurve(QStringLiteral("G"), 0, 1);
        mitKurve.expression = {{0.0, 1.0}, {1.0, 0.0}};
        loop.kanaele = {Kanal{QStringLiteral("K"), true, {mitKurve, LoopClip(QStringLiteral("G"), 1, 1)}}};
        loop.richte();
        REQUIRE(loop.bpm == 120.0);
        REQUIRE(loop.schritteJeSchlag == 4);
        const InstrumentVon saiten = [](const QString&) { return gitarre(); };
        const std::vector<ClipToene> toene = toeneImKanal(loop, 0, saiten);
        REQUIRE(toene.size() == 2);
        REQUIRE(toene[0].size() == 1);
        const GeplanterTon& erste = toene[0][0][0];
        REQUIRE(erste.saite != toene[0][0][1].saite);
        REQUIRE(erste.versatzMs == 0.0);
        REQUIRE(erste.endeMs >= 2000);  // die Saite wird im Clip nicht wieder angeschlagen
        REQUIRE(toene[0][0][1].endeMs >= 1000);
        // ein Schritt = 125 ms, der Clip ist 2000 ms lang
        CHECK(erste.ausdruckMuster == std::vector<PegelPunkt>{{0, 1000}, {1000, 500}, {2000, 500}});
        CHECK(erste.ausdruckClip == std::vector<PegelPunkt>{{0, 1000}, {2000, 0}});
        const GeplanterTon& zweite = toene[0][0][1];
        CHECK(zweite.ausdruckMuster == std::vector<PegelPunkt>{{0, 500}, {1000, 500}});
        CHECK(zweite.ausdruckClip == std::vector<PegelPunkt>{{0, 500}, {1000, 0}});
        // der zweite Clip hat keine Kurve: nur die Spur, und andere Toene
        const GeplanterTon& ohneKurve = toene[1][0][0];
        CHECK(ohneKurve.ausdruckClip.empty());
        CHECK(ohneKurve.ausdruckMuster == erste.ausdruckMuster);
        CHECK(tonKlang(ohneKurve) != tonKlang(erste));
        const QStringList namen = loop.klangNamen(saiten);
        CHECK(namen.contains(tonKlang(erste)));
        CHECK(namen.contains(tonKlang(ohneKurve)));
        // der Rechenkern legt bei Saiten keinen zweiten Pegel darueber
        const Ablauf ablauf = plane(loop, kRate, saiten);
        CHECK(ablauf.ausdruecke.empty());
        for (const Stimme& stimme : ablauf.stimmen)
            for (const Schlag& schlag : stimme.schlaege) CHECK(schlag.ausdruck < 0);

        // ohne Spur und Kurve: kein Verlauf, derselbe Name wie vorher
        Loop ohne = loop;
        ohne.muster.front().zeitspuren = Zeitspuren{};
        ohne.kanaele.front().clips.front().expression.clear();
        const std::vector<ClipToene> schlicht = toeneImKanal(ohne, 0, saiten);
        CHECK(schlicht[0][0][0].ausdruckMuster.empty());
        CHECK(schlicht[0][0][0].ausdruckClip.empty());
        CHECK_FALSE(tonKlang(schlicht[0][0][0]).contains(QStringLiteral(":x")));
        // eine Spur, die ueberall 1 ist, aendert den Namen nicht
        Loop eins = ohne;
        eins.muster.front().zeitspuren.von(Steuerspur::Expression) = {{0.0, 1.0}, {4.0, 1.0}};
        CHECK(tonKlang(toeneImKanal(eins, 0, saiten)[0][0][0]) == tonKlang(schlicht[0][0][0]));
    }

    TEST_CASE("Loop Saite Expression 28c-2b: ein Ton mit Ende traegt den Verlauf nur bis dorthin")
    {
        // zweimal dieselbe Tonhoehe: dieselbe Saite, der erste Ton endet beim zweiten Anschlag
        Loop loop;
        Muster m = gitarrenMuster({Note{0, 2, 64}, Note{4, 2, 64}});
        m.zeitspuren.von(Steuerspur::Expression) = {{0.0, 1.0}, {15.0, 0.0}};
        loop.muster = {m};
        loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("G"), 0, 1)}}};
        loop.richte();
        const InstrumentVon saiten = [](const QString&) { return gitarre(); };
        const std::vector<ClipToene> toene = toeneImKanal(loop, 0, saiten);
        const GeplanterTon& erste = toene[0][0][0];
        REQUIRE(erste.endeMs == 500);
        REQUIRE_FALSE(erste.ausdruckMuster.empty());
        CHECK(erste.ausdruckMuster.back().ms == 500);
        // linear von 1 bei Schritt 0 auf 0 bei Schritt 15: bei Schritt 4 4/15 weniger
        CHECK(erste.ausdruckMuster.back().tausendstel == 733);

        // Rechenzeit: ein Muster von 16 Noten mit Spur gegen ohne (meldet nur Zahlen)
        std::vector<Note> sechzehn;
        for (int i = 0; i < 16; ++i) sechzehn.push_back(Note{i, 1, 52 + (i % 4) * 5});
        Loop dicht;
        Muster d = gitarrenMuster(sechzehn);
        dicht.muster = {d};
        dicht.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("G"), 0, 1)}}};
        dicht.richte();
        Loop mitSpur = dicht;
        mitSpur.muster.front().zeitspuren.von(Steuerspur::Expression) = {{0.0, 0.2}, {15.0, 1.0}};
        Instrument instrument = welle(true);
        const auto miss = [&](const Loop& l) {
            const QStringList namen = l.klangNamen(saiten);
            QSet<QString> eigen(namen.begin(), namen.end());
            const auto start = std::chrono::steady_clock::now();
            for (const QString& name : eigen)
            {
                TonAngabe t;
                if (zerlegeTon(name, t)) (void)rendereTon(instrument, t, kRate);
            }
            const auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            return std::pair<qsizetype, double>{eigen.size(), ms};
        };
        const auto [toeneOhne, msOhne] = miss(dicht);
        const auto [toeneMit, msMit] = miss(mitSpur);
        MESSAGE("16 Noten ohne Expression: " << toeneOhne << " Toene in " << msOhne << " ms, mit: " << toeneMit
                                             << " Toene in " << msMit << " ms");
        CHECK(toeneMit >= toeneOhne);
    }
}
