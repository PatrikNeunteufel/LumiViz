/**
 ****************************************************************************************
 * @file   test_LoopKorpus.cpp
 * @brief  Saiteninstrumente: der Korpus — das gekoppelte Modell aus Decke, Boden und Luft
 *         nach Christensen/Vistisen; Parlor mit den Werten aus Hess (2013), Tab. 1,
 *         Classical guitar mit Wrights Satz TOP26 (1996, S. 138–140)
 *         (docs/audio/Saiten_Konzept.md; Entscheide Patrik S132: S-K4 a, S-K7 a, S-K8 a;
 *         S133: S-K10 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Der Beleg: das Modell hat seine beiden
 * gekoppelten Spitzen dort, wo Hess sie an der Gitarre gemessen hat (116 und 219 Hz,
 * S. 3) — eingegeben sind nur die entkoppelten Werte (Decke rund 211 Hz, Luft 130 Hz);
 * die Classical guitar hat ihre sieben Spitzen dort, wo Wright sie nennt (Abb. 7.1).
 * `LUMIVIZ_KORPUS_WAV=<Ordner>` legt Hoerproben ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/Instrument.hpp"
#include "loop/Saiten.hpp"

#include "SaitenMessung.hpp"  // betrag, spitze, db, teilton

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>
#include <utility>
#include <vector>

using namespace lumi::loop;
using namespace saitentest;

namespace
{

constexpr int kRate = kSaitenRate;

/// Der Betrag der Impulsantwort bei `hz` (ueber die ganze Antwort, ohne Fenster).
double antwortBei(const std::vector<double>& antwort, double hz)
{
    double re = 0.0;
    double im = 0.0;
    for (std::size_t n = 0; n < antwort.size(); ++n)
    {
        const double phase = 2.0 * kPi * hz * static_cast<double>(n) / kRate;
        re += antwort[n] * std::cos(phase);
        im -= antwort[n] * std::sin(phase);
    }
    return std::sqrt(re * re + im * im);
}

/// Eine Saite *Extended KS* (oder Waveguide), mit oder ohne Korpus.
template <class F>
Instrument saiteMit(F stelle)
{
    Instrument instrument;
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::ErweiterterKs;
    s.anstiegMs = 1.0;
    s.ausklangMs = 200.0;
    stelle(s);
    return instrument;
}

}  // namespace

TEST_CASE("Loop Saite Korpus: Parlor — die gekoppelten Spitzen liegen, wo Hess sie gemessen hat")
{
    CHECK(korpusImpulsantwort(KorpusArt::Keiner, kRate, 100).empty());
    const std::vector<double> antwort = korpusImpulsantwort(KorpusArt::Parlor, kRate, 2 * kRate);
    REQUIRE(antwort.size() == static_cast<std::size_t>(2 * kRate));
    // die beiden groessten Spitzen zwischen 60 und 400 Hz, in Schritten von 0,5 Hz
    std::vector<double> hz;
    std::vector<double> wert;
    for (double f = 60.0; f <= 400.0; f += 0.5)
    {
        hz.push_back(f);
        wert.push_back(antwortBei(antwort, f));
    }
    std::vector<std::pair<double, double>> spitzen;  // (Betrag, Frequenz)
    for (std::size_t i = 1; i + 1 < wert.size(); ++i)
        if (wert[i] > wert[i - 1] && wert[i] >= wert[i + 1]) spitzen.emplace_back(wert[i], hz[i]);
    REQUIRE(spitzen.size() >= 2);
    std::sort(spitzen.begin(), spitzen.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
    double tief = std::min(spitzen[0].second, spitzen[1].second);
    double hoch = std::max(spitzen[0].second, spitzen[1].second);
    MESSAGE("Parlor: gekoppelte Spitzen bei " << tief << " Hz und " << hoch << " Hz (Hess gemessen: 116 und 219 Hz)");
    CHECK(tief == doctest::Approx(116.0).epsilon(0.05));
    CHECK(hoch == doctest::Approx(219.0).epsilon(0.05));
    // und die zweite Spitze ist die staerkere (Hess Abb. 1a: rund 86 gegen 79 dB/N)
    const double unterschied = db(antwortBei(antwort, hoch) / antwortBei(antwort, tief));
    MESSAGE("Parlor: zweite gegen erste Spitze " << unterschied << " dB (Hess Abb. 1a abgelesen: rund +7 dB)");
    CHECK(unterschied > 0.0);
}

TEST_CASE("Loop Saite Korpus: Classical guitar — die gekoppelten Spitzen liegen, wo Wright sie fuer TOP26 nennt")
{
    // eine halbe Sekunde reicht: die langsamste Mode (Luft, Q 30 bei 133 Hz) ist nach 72 ms auf 1/e
    const std::vector<double> antwort = korpusImpulsantwort(KorpusArt::Klassik, kRate, kRate / 2);
    REQUIRE(antwort.size() == static_cast<std::size_t>(kRate / 2));
    // der Betrag bei hz nach Goertzel (eine Multiplikation je Abtastwert)
    const auto betragBei = [&](double hz) {
        const double k = 2.0 * std::cos(2.0 * kPi * hz / kRate);
        double s1 = 0.0;
        double s2 = 0.0;
        for (const double x : antwort)
        {
            const double s = x + k * s1 - s2;
            s2 = s1;
            s1 = s;
        }
        return std::sqrt(std::max(0.0, s1 * s1 + s2 * s2 - k * s1 * s2));
    };
    // die groesste Spitze im Fenster ±5 % um das Ziel, in Schritten von 0,25 Hz
    const auto spitzeUm = [&](double ziel) {
        std::vector<double> hz;
        std::vector<double> wert;
        for (double f = 0.95 * ziel; f <= 1.05 * ziel; f += 0.25)
        {
            hz.push_back(f);
            wert.push_back(betragBei(f));
        }
        std::pair<double, double> beste{0.0, 0.0};  // (Betrag, Frequenz)
        for (std::size_t i = 1; i + 1 < wert.size(); ++i)
            if (wert[i] > wert[i - 1] && wert[i] >= wert[i + 1] && wert[i] > beste.first) beste = {wert[i], hz[i]};
        return beste;
    };
    // Wright S. 139, Abb. 7.1: „T(1,1)1 112 Hz, T(1,1)2 195 Hz, T(1,1)3 215 Hz, T(1,2) 435 Hz,
    // T(3,1) 505 Hz, T(1,3) 674 Hz, T(5,1) 880 Hz“ — eingegeben sind nur die entkoppelten Werte
    constexpr std::array<double, 7> ziele = {112.0, 195.0, 215.0, 435.0, 505.0, 674.0, 880.0};
    std::array<std::pair<double, double>, 7> gefunden{};
    for (std::size_t i = 0; i < ziele.size(); ++i)
    {
        gefunden[i] = spitzeUm(ziele[i]);
        CAPTURE(ziele[i]);
        REQUIRE(gefunden[i].first > 0.0);
        CHECK(gefunden[i].second == doctest::Approx(ziele[i]).epsilon(0.05));
    }
    std::ostringstream zeile;
    for (std::size_t i = 0; i < ziele.size(); ++i)
        zeile << " " << gefunden[i].second << " Hz (" << db(gefunden[i].first / gefunden[1].first) << " dB)";
    MESSAGE("Classical: Spitzen" << zeile.str() << " — Wright Abb. 7.1: 112 195 215 435 505 674 880 Hz");
    // die zweite des Tripletts ist die staerkste (Abb. 7.1: rund 85 dB, alle anderen darunter)
    for (std::size_t i = 0; i < ziele.size(); ++i)
        if (i != 1) CHECK(gefunden[i].first < gefunden[1].first);
}

TEST_CASE("Loop Saite Korpus: die Datei — Body und Body amount")
{
    Instrument mit = saiteMit([](Schicht& s) {
        s.korpus = KorpusArt::Parlor;
        s.korpusAnteil = 0.8;
    });
    const QJsonObject json = alsJson(mit);
    const QJsonObject schicht = json.value(QStringLiteral("layers")).toArray().at(0).toObject();
    CHECK(schicht.value(QStringLiteral("body")).toString() == QStringLiteral("parlor"));
    CHECK(schicht.value(QStringLiteral("bodyAmount")).toDouble() == 0.8);
    Instrument gelesen;
    REQUIRE(ausJson(json, gelesen));
    CHECK(gelesen == mit);
    // Classical guitar
    Instrument klassik = saiteMit([](Schicht& s) { s.korpus = KorpusArt::Klassik; });
    const QJsonObject jsonKlassik = alsJson(klassik);
    CHECK(jsonKlassik.value(QStringLiteral("layers")).toArray().at(0).toObject().value(QStringLiteral("body")).toString() ==
          QStringLiteral("classical"));
    Instrument gelesenKlassik;
    REQUIRE(ausJson(jsonKlassik, gelesenKlassik));
    CHECK(gelesenKlassik == klassik);
    // ein Korpus, den das Programm nicht kennt: keiner
    QJsonObject fremd = schicht;
    fremd[QStringLiteral("body")] = QStringLiteral("dreadnought");
    QJsonObject wurzel = json;
    wurzel[QStringLiteral("layers")] = QJsonArray{fremd};
    REQUIRE(ausJson(wurzel, gelesen));
    CHECK(gelesen.schichten[0].korpus == KorpusArt::Keiner);
}

TEST_CASE("Loop Saite Korpus: im Ton — er hebt, was um seine Spitzen liegt; Body amount 0 und ein Tonabnehmer lassen ihn aus")
{
    // A2 (110 Hz) liegt nah an der ersten Spitze: mit Korpus steigt der Grundton gegen den fuenften Teilton
    const double f0 = tonHz(45);
    const auto verhaeltnis = [&](const Instrument& instrument) {
        const Klang ton = rendereTon(instrument, 45, 1200.0, kRate);
        return db(teilton(ton, f0, 1) / teilton(ton, f0, 5));
    };
    const double ohne = verhaeltnis(saiteMit([](Schicht&) {}));
    const double mit = verhaeltnis(saiteMit([](Schicht& s) {
        s.korpus = KorpusArt::Parlor;
        s.korpusAnteil = 1.0;
    }));
    MESSAGE("Grundton gegen Teilton 5: ohne Korpus " << ohne << " dB, mit Parlor " << mit << " dB");
    CHECK(mit > ohne + 3.0);

    // Body amount 0: die Saite allein, Bit fuer Bit
    const Klang allein = rendereTon(saiteMit([](Schicht&) {}), 45, 600.0, kRate);
    CHECK(rendereTon(saiteMit([](Schicht& s) {
                         s.korpus = KorpusArt::Parlor;
                         s.korpusAnteil = 0.0;
                     }),
                     45, 600.0, kRate)
              .daten == allein.daten);
    // ein magnetischer Tonabnehmer nimmt die Saite selbst ab: der Korpus wirkt nicht
    const auto waveguide = [](KorpusArt korpus) {
        return saiteMit([&](Schicht& s) {
            s.saitenModell = SaitenModell::Wellenleiter;
            s.abnehmerSteg = Abnehmer{AbnehmerArt::SingleCoil, 41.0};
            s.korpus = korpus;
            s.korpusAnteil = 1.0;
        });
    };
    CHECK(rendereTon(waveguide(KorpusArt::Parlor), 45, 600.0, kRate).daten ==
          rendereTon(waveguide(KorpusArt::Keiner), 45, 600.0, kRate).daten);
}

TEST_CASE("Loop Saite Korpus: jeder Ton im Umfang bleibt endlich und im Rahmen")
{
    for (const int hoehe : {kHoeheMin, 40, 64, 88, kHoeheMax})
        for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
            for (const KorpusArt korpus : {KorpusArt::Parlor, KorpusArt::Klassik})
        {
            CAPTURE(hoehe);
            CAPTURE(static_cast<int>(modell));
            CAPTURE(static_cast<int>(korpus));
            const Instrument instrument = saiteMit([&](Schicht& s) {
                s.saitenModell = modell;
                s.korpus = korpus;
                s.korpusAnteil = 1.0;
            });
            const Klang ton = rendereTon(instrument, hoehe, 500.0, 48000);
            bool endlich = true;
            float groesster = 0.0f;
            for (const float x : ton.daten)
            {
                if (!std::isfinite(x)) endlich = false;
                groesster = std::max(groesster, std::abs(x));
            }
            CHECK(endlich);
            CHECK(groesster <= 0.9f);
            CHECK(groesster > 0.0005f);
            CHECK(std::abs(ton.daten[ton.daten.size() - 2]) < 0.002f);
        }
}

TEST_CASE("Loop Saite Korpus: Hoerproben (LUMIVIZ_KORPUS_WAV)")
{
    const QString ordner = qEnvironmentVariable("LUMIVIZ_KORPUS_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_KORPUS_WAV=<Ordner> legt Hoerproben ab");
        return;
    }
    REQUIRE(QDir().mkpath(ordner));
    const auto probe = [&](const char* name, const Instrument& instrument) {
        std::vector<float> folge(static_cast<std::size_t>(2 * kRate * 5), 0.0f);
        int nr = 0;
        for (const int hoehe : {40, 45, 52, 57, 64})
        {
            const Klang ton = rendereTon(instrument, hoehe, 2000.0, kRate);
            const auto ab = static_cast<std::size_t>(2 * nr * kRate / 2);
            for (std::size_t i = 0; i < ton.daten.size() && ab + i < folge.size(); ++i) folge[ab + i] += ton.daten[i] * 0.5f;
            ++nr;
        }
        REQUIRE(schreibeWav(QDir(ordner).filePath(QString::fromLatin1(name) + QStringLiteral(".wav")), folge, kRate));
    };
    for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
    {
        const bool wg = modell == SaitenModell::Wellenleiter;
        probe(wg ? "wg_ohne_korpus" : "ks_ohne_korpus", saiteMit([&](Schicht& s) { s.saitenModell = modell; }));
        probe(wg ? "wg_parlor" : "ks_parlor", saiteMit([&](Schicht& s) {
                  s.saitenModell = modell;
                  s.korpus = KorpusArt::Parlor;
              }));
        probe(wg ? "wg_classical" : "ks_classical", saiteMit([&](Schicht& s) {
                  s.saitenModell = modell;
                  s.korpus = KorpusArt::Klassik;
              }));
    }
    MESSAGE("6 Hoerproben in " << ordner.toStdString());
}
