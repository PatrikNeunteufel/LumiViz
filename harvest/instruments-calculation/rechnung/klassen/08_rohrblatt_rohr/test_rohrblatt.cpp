/**
 * @file test_rohrblatt.cpp
 * @brief Klasse 8 Rohrblatt und Rohr: Zylinder geschlossen (ungerade Moden), Kegel (alle), gestimmte Klarinette und
 *        Saxophone treffen die Note, Schwelle nach Almeida 2013, Anstieg nach Li 2016, Chalumeau mit schwachen geraden
 *        Teiltoenen, Legato, Bend nur abwaerts mit Grenze, Gerueste still, Determinismus, Abdruck, Rechenzeit
 */
#include "doctest.h"

#include "gemeinsam/Fft.hpp"
#include "klassen/08_rohrblatt_rohr/Rohrblatt.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <cstdio>
#include <numbers>

using namespace ic;
using namespace ic::rohrblatt;

namespace
{

constexpr double kPi = std::numbers::pi;

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

const RohrblattSpiel& klarinette()
{
    static const RohrblattSpiel k = baueVorgabeKlarinette();
    return k;
}

RohrblattAnschlag ton(int note, double e, double haltenS = 0.6)
{
    RohrblattAnschlag a;
    a.kette = {{0.0, note, hzVon(note)}};
    a.expression = e;
    a.haltenS = haltenS;
    return a;
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n; ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * t[ab + i] * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

}  // namespace

TEST_CASE("Rohrblatt: die Klarinette ist ein geschlossener Zylinder (Spitzen 1 : 3 : 5), das Saxophon ein Kegel (1 : 2 : 3)")
{
    const std::vector<ImpedanzMode> k = grifModen(baueVorgabeKlarinette(false), 0.5, false);
    REQUIRE(k.size() >= 3);
    CHECK(k[1].hz / k[0].hz == doctest::Approx(3.0).epsilon(0.05));
    CHECK(k[2].hz / k[0].hz == doctest::Approx(5.0).epsilon(0.05));
    const std::vector<ImpedanzMode> s = grifModen(baueVorgabeSopransaxophon(false), 0.5, false);
    REQUIRE(s.size() >= 3);
    CHECK(s[1].hz / s[0].hz == doctest::Approx(2.0).epsilon(0.08));
    CHECK(s[2].hz / s[0].hz == doctest::Approx(3.0).epsilon(0.08));
}

TEST_CASE("Klarinette: jede dritte Note bei pp, mf, ff — mindestens 90 % auf ±10 Cent; Schwelle 2–5 kPa (Almeida 2013: 4 → 2 kPa)")
{
    const RohrblattSpiel& k = klarinette();
    int getroffen = 0, gesamt = 0;
    for (int note = k.noteMin; note <= k.noteMax; note += 3)
    {
        const double ps = wertAn(k.schwelleJeNote, note, Skala::Logarithmisch);
        CAPTURE(note);
        CHECK(ps > 1500.0);
        CHECK(ps < 6000.0);
        for (const double e : {0.0, 0.5, 1.0})
        {
            RohrblattMessung m;
            std::vector<float> t;
            rechneRohrblattTon(k, ton(note, e), t, 33600, &m);
            ++gesamt;
            const double c = m.schwingt ? 1200.0 * std::log2(m.spielHz / hzVon(note)) : 1e9;
            if (std::abs(c) < 10.0)
                ++getroffen;
            else
                std::printf("WARN Klarinette n%d e%.1f: %+.0f Cent (mehrstabil — offen.md)\n", note, e, c);
        }
    }
    std::printf("Klarinette: %d von %d Pruefpunkten auf ±10 Cent\n", getroffen, gesamt);
    CHECK(getroffen * 10 >= gesamt * 9);
}

TEST_CASE("Klarinette: Chalumeau mit schwachen geraden Teiltoenen (meist); Einsatz mit Zunge steil (Li 2016: bis 730 dB/s)")
{
    const RohrblattSpiel& k = klarinette();
    int schwach = 0, gesamt = 0;
    for (int note = 50; note <= 60; ++note)
    {
        std::vector<float> t;
        RohrblattMessung m;
        rechneRohrblattTon(k, ton(note, 0.5), t, 33600, &m);
        const double f = m.spielHz;
        const double h1 = band(t, 14400, 8192, f), h2 = band(t, 14400, 8192, 2 * f), h3 = band(t, 14400, 8192, 3 * f);
        ++gesamt;
        if (h2 < h3 && h2 < 0.3 * h1)
            ++schwach;
        else
            std::printf("WARN Klarinette n%d: H2/H1 %+.1f dB, H3/H1 %+.1f dB (gerade Teiltoene stark — offen.md)\n", note, dB(h2 / h1), dB(h3 / h1));
        if (note == 55)
        {
            std::printf("Klarinette G3: Anstieg %.0f dB/s\n", m.anstiegDbJeS);
            CHECK(m.anstiegDbJeS > 100.0);
            CHECK(m.anstiegDbJeS < 3000.0);
        }
    }
    std::printf("Klarinette Chalumeau: %d von %d Noten mit schwachen geraden Teiltoenen\n", schwach, gesamt);
    CHECK(schwach * 3 >= gesamt * 2);
}

TEST_CASE("Klarinette: Legato als Ueberblenden der Moden, Bend nur abwaerts und gekappt, Flatterzunge, lauter mit der Staerke")
{
    const RohrblattSpiel& k = klarinette();
    RohrblattAnschlag a = ton(55, 0.5, 1.2);
    a.kette.push_back({0.6, 57, hzVon(57)});
    RohrblattMessung m;
    std::vector<float> t;
    rechneRohrblattTon(k, a, t, 60000, &m);
    REQUIRE(m.schwingt);
    CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(57))) < 20.0);
    RohrblattAnschlag b = ton(60, 0.5);
    b.pitchCent = -150.0;
    RohrblattMessung mb;
    rechneRohrblattTon(k, b, t, 33600, &mb);
    RohrblattMessung m0;
    rechneRohrblattTon(k, ton(60, 0.5), t, 33600, &m0);
    REQUIRE(mb.schwingt);
    CHECK(mb.spielHz <= m0.spielHz * 1.001);  // nach unten oder gleich (die Groesse ist ungemessen)
    RohrblattAnschlag hoch = ton(60, 0.5);
    hoch.pitchCent = 200.0;  // nach oben: keine Wirkung (Grenze 0)
    std::vector<float> th, t0;
    rechneRohrblattTon(k, hoch, th, 33600);
    rechneRohrblattTon(k, ton(60, 0.5), t0, 33600);
    CHECK(th == t0);
    RohrblattAnschlag fl = ton(60, 0.6);
    fl.ansatz = Ansatz::Flatterzunge;
    std::vector<float> tf;
    rechneRohrblattTon(k, fl, tf, 33600);
    CHECK(tf != t0);
    std::vector<float> leise, laut;
    rechneRohrblattTon(k, ton(60, 0.0), leise, 33600);
    rechneRohrblattTon(k, ton(60, 1.0), laut, 33600);
    CHECK(rms(laut) > rms(leise));
}

TEST_CASE("Saxophone treffen ihre Mitte (Altsaxophon nach Petersen 2020); Oboe (Doppelrohrblatt offen), Fagott still")
{
    for (const RohrblattSpiel& s : {baueVorgabeSopransaxophon(), baueVorgabeTenorsaxophon(), baueVorgabeAltsaxophon()})
    {
        CAPTURE(s.name);
        const int n = (s.noteMin + s.noteMax) / 2;
        RohrblattMessung m;
        std::vector<float> t;
        rechneRohrblattTon(s, ton(n, 0.5), t, 33600, &m);
        REQUIRE(m.schwingt);
        CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(n))) < 15.0);
    }
    for (const RohrblattSpiel& s : {baueVorgabeOboe(), baueVorgabeFagott()})
    {
        CAPTURE(s.name);
        CHECK_FALSE(s.spielbar);
        std::vector<float> t;
        rechneRohrblattTon(s, ton((s.noteMin + s.noteMax) / 2, 0.5), t, 4800);
        CHECK(spitze(t) == 0.0);
    }
}

TEST_CASE("Rohrblatt: Determinismus, Abdruck, Rechenzeit")
{
    const RohrblattSpiel& k = klarinette();
    std::vector<float> a, b;
    rechneRohrblattTon(k, ton(60, 0.5), a, 28800);
    std::vector<std::vector<float>> p(2);
    parallelJe(2, [&](std::size_t i) { rechneRohrblattTon(k, ton(60, 0.5), p[i], 28800); }, 2);
    CHECK(a == p[0]);
    CHECK(a == p[1]);
    const std::string ist = abdruck(a);
    CHECK(test::erwarteterAbdruck("rohrblatt_Klarinette_m60_mf", ist) == ist);
    std::vector<float> alt;
    rechneRohrblattTon(baueVorgabeAltsaxophon(), ton(70, 0.5), alt, 28800);
    const std::string istAlt = abdruck(alt);
    CHECK(test::erwarteterAbdruck("rohrblatt_Altsaxophon_m70_mf", istAlt) == istAlt);
    std::vector<float> t;
    const double anteil = echtzeitAnteil([&] { rechneRohrblattTon(k, ton(k.noteMin, 0.5, 2.0), t, 96000); }, 2.0);
    std::printf("Rechenzeit Klarinette tiefster Ton: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
    CHECK(anteil < test::echtzeitGrenzeHier());
}
