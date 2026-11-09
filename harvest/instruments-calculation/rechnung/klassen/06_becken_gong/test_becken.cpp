/**
 * @file test_becken.cpp
 * @brief Klasse 6 Becken und Gong: freie Kreisplatte gegen Leissa, Energie des SAV-Schemas erhalten, Aufbluehen nach
 *        oben nur bei ff (Legge & Fletcher), Kontakt (DAFx-15), Choke, Hi-Hat-Oeffnung, Schlagstellen, Determinismus,
 *        Abdruck, Rechenzeit vorab
 */
#include "doctest.h"

#include "klassen/06_becken_gong/Becken.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <cstdio>

using namespace ic;
using namespace ic::becken;

namespace
{

const BeckenSpiel& crash()
{
    static const BeckenSpiel c = baueVorgabeCrash();
    return c;
}

double energieAb(const std::vector<float>& t, std::size_t ab, std::size_t bis)
{
    double e = 0.0;
    for (std::size_t i = ab; i < std::min(bis, t.size()); ++i) e += double(t[i]) * t[i];
    return e;
}

}  // namespace

TEST_CASE("Becken: die freie Kreisplatte gegen Leissa (ν = 0,33: λ² = 5,253 / 9,084 / 12,23 / 20,52 — Lehrbuch, offen)")
{
    const std::vector<PlattenMode> m = freiePlatte(0.33, 6.0);
    REQUIRE(m.size() >= 4);
    CHECK(m[0].m == 2);
    CHECK(m[0].lambda * m[0].lambda == doctest::Approx(5.253).epsilon(0.005));
    CHECK(m[1].m == 0);
    CHECK(m[1].lambda * m[1].lambda == doctest::Approx(9.084).epsilon(0.005));
    CHECK(m[2].lambda * m[2].lambda == doctest::Approx(12.23).epsilon(0.005));
    CHECK(m[3].lambda * m[3].lambda == doctest::Approx(20.52).epsilon(0.005));
}

TEST_CASE("Becken: das SAV-Schema erhaelt die Energie ohne Daempfung (auf 10⁻⁹) auch bei ff")
{
    BeckenAnschlag a;
    a.staerke = 1.0;
    BeckenMessung m;
    std::vector<float> t;
    rechneBeckenTon(crash(), a, t, 9600, &m, true);
    std::printf("Crash ff ohne Daempfung: Energiefehler %.2e\n", m.energieMaxFehler);
    CHECK(m.energieAnfang > 0.0);
    CHECK(m.energieMaxFehler < 1e-9);
}

TEST_CASE("Becken: bei ff wandert die Energie nach oben (Legge & Fletcher 1989, nur Bericht), bei pp bleibt das Spektrum")
{
    BeckenMessung pp, ff;
    std::vector<float> t;
    BeckenAnschlag a;
    a.staerke = 0.0;
    rechneBeckenTon(crash(), a, t, 19200, &pp);
    a.staerke = 1.0;
    rechneBeckenTon(crash(), a, t, 19200, &ff);
    REQUIRE(pp.bandEnergie.size() >= 4);
    REQUIRE(ff.bandEnergie.size() >= 4);
    std::printf("Crash: Anteil > 1 kHz pp %.2f → %.2f, ff %.2f → %.2f (je 50 ms)\n", pp.bandEnergie.front(), pp.bandEnergie[3], ff.bandEnergie.front(),
                ff.bandEnergie[3]);
    CHECK(ff.bandEnergie[3] > 3.0 * pp.bandEnergie[3]);
    CHECK(ff.bandEnergie[2] > ff.bandEnergie[0]);
    CHECK(pp.bandEnergie[3] <= pp.bandEnergie[0] * 1.05);
    std::printf("Crash ff: Kontakt %.2f ms, Kraft %.0f N (DAFx-15: 0,8–1 ms, 100–300 N — Modellwerte)\n", 1e3 * ff.kontaktS, ff.kraftSpitzeN);
    CHECK(ff.kontaktS > 0.0003);
    CHECK(ff.kontaktS < 0.003);
    CHECK(ff.kraftSpitzeN < 1500.0);
}

TEST_CASE("Becken: Choke daempft, das Hi-Hat geschlossen kuerzer als offen, die Schlagstellen unterscheiden sich")
{
    std::vector<float> frei, choke;
    BeckenAnschlag a;
    a.staerke = 0.6;
    rechneBeckenTon(crash(), a, frei, 14400);
    a.chokeAbS = 0.1;
    rechneBeckenTon(crash(), a, choke, 14400);
    CHECK(energieAb(choke, 9600, 14400) < 0.1 * energieAb(frei, 9600, 14400));
    const BeckenSpiel hh = baueVorgabeHihat();
    std::vector<float> offen, zu;
    BeckenAnschlag h;
    h.oeffnung = 1.0;
    rechneBeckenTon(hh, h, offen, 9600);
    h.oeffnung = 0.0;
    rechneBeckenTon(hh, h, zu, 9600);
    CHECK(energieAb(zu, 4800, 9600) < 0.2 * energieAb(offen, 4800, 9600));
    std::vector<float> flaeche, kuppe, rand;
    BeckenAnschlag s;
    rechneBeckenTon(crash(), s, flaeche, 4800);
    s.stelle = Stelle::Kuppe;
    rechneBeckenTon(crash(), s, kuppe, 4800);
    s.stelle = Stelle::Rand;
    rechneBeckenTon(crash(), s, rand, 4800);
    CHECK(kuppe != flaeche);
    CHECK(rand != flaeche);
}

TEST_CASE("Becken: alle Instrumente klingen, lauter mit der Staerke")
{
    for (const BeckenSpiel& b : {baueVorgabeTamtam(), baueVorgabeGong(), baueVorgabeChina(), baueVorgabeSplash(), baueVorgabeRide()})
    {
        CAPTURE(b.name);
        std::vector<float> pp, ff;
        BeckenAnschlag a;
        a.staerke = 0.0;
        rechneBeckenTon(b, a, pp, 4800);
        a.staerke = 1.0;
        rechneBeckenTon(b, a, ff, 4800);
        CHECK(rms(ff) > rms(pp));
        CHECK(rms(pp) > 0.0);
    }
}

TEST_CASE("Becken: Determinismus, Abdruck, Rechenzeit vorab (F-1 a: vorab je Staerke — Grenze 10 × Echtzeit)")
{
    const BeckenSpiel& c = crash();
    BeckenAnschlag a;
    a.staerke = 0.8;
    std::vector<float> x;
    rechneBeckenTon(c, a, x, 9600);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneBeckenTon(c, a, q[i], 9600); }, 2);
    CHECK(x == q[0]);
    CHECK(x == q[1]);
    const std::string ist = abdruck(x);
    CHECK(test::erwarteterAbdruck("becken_Crash_e80", ist) == ist);
    std::vector<float> t;
    const double anteil = echtzeitAnteil([&] { rechneBeckenTon(c, a, t, 24000); }, 0.5);
    std::printf("Rechenzeit Crash: %.1f × Echtzeit (vorab; Grenze 10 ×)\n", anteil);
    CHECK(anteil < 10.0);
}
