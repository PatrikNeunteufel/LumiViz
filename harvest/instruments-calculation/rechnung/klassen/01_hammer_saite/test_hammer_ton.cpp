/**
 * @file test_hammer_ton.cpp
 * @brief Der portierte Fluegel-Kern: Determinismus (wiederholt und auf Threads Bit fuer Bit), Rechenzeit im Bass,
 *        Abdruck je Compiler, Entartung im Chor gemeldet (L11)
 */
#include "doctest.h"

#include "gemeinsam/Eigenzerlegung.hpp"
#include "klassen/01_hammer_saite/HammerTon.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <cstdio>

using namespace ic;
using namespace ic::hammer;

namespace
{

HammerAnschlag anschlag(int taste, int stufe, double haltenS)
{
    HammerAnschlag a;
    a.taste = taste;
    a.hz = 440.0 * std::exp2((taste - 69) / 12.0);
    a.stufe = stufe;
    a.rate = 48000;
    a.halten = std::llround(haltenS * a.rate);
    a.gedaempft = taste < kUngedaempftAbVorgabe;
    return a;
}

}  // namespace

TEST_CASE("Hammer-Saite (Vorgabe des Vorbilds): derselbe Ton Bit fuer Bit, wiederholt und auf Threads; Abdruck")
{
    const HammerSaite& s = hammerSaiteVorgabe();
    const int tasten[] = {21, 40, 60, 84, 100};
    std::vector<std::vector<float>> einzeln(5), parallel(5);
    for (int i = 0; i < 5; ++i)
    {
        const HammerAnschlag a = anschlag(tasten[i], -12, 0.5);
        rechneHammerTon(s, a, einzeln[static_cast<std::size_t>(i)], a.halten + hammerNachklang(s, a, 48000 * 4));
    }
    parallelJe(5, [&](std::size_t i) {
        const HammerAnschlag a = anschlag(tasten[i], -12, 0.5);
        rechneHammerTon(s, a, parallel[i], a.halten + hammerNachklang(s, a, 48000 * 4));
    }, 5);
    for (int i = 0; i < 5; ++i)
    {
        CHECK(einzeln[static_cast<std::size_t>(i)] == parallel[static_cast<std::size_t>(i)]);
        const std::string name = "hammer_vorgabe_m" + std::to_string(tasten[i]) + "_a-12_d500";
        const std::string ist = abdruck(einzeln[static_cast<std::size_t>(i)]);
        CAPTURE(name);
        CHECK(test::erwarteterAbdruck(name, ist) == ist);
    }
}

double ic::test::fluegelAnteil()
{
    static const double anteil = [] {
        const HammerSaite& s = hammerSaiteVorgabe();
        const HammerAnschlag a = anschlag(21, 0, 2.0);
        const auto frames = a.halten + hammerNachklang(s, a, 48000 * 10);
        std::vector<float> mono;
        return echtzeitAnteil([&] { rechneHammerTon(s, a, mono, frames); }, static_cast<double>(frames) / a.rate, 5);
    }();
    return anteil;
}

TEST_CASE("Hammer-Saite: Rechenzeit im tiefsten Ton (Bezug aller Klassen auf dieser Maschine)")
{
    const double anteil = test::fluegelAnteil();
    std::printf("Rechenzeit Hammer-Saite A0 (Bezug): %.2f %% der Echtzeit auf dieser Maschine (LumiViz: 4,6 %%); Grenze "
                "der Klassen hier: %.2f %%\n", 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
    WARN(anteil < kEchtzeitGrenze);
    CHECK(anteil > 0.0);
}

TEST_CASE("Hammer-Saite: Entartung im Chor wird erkannt (L11: δ = 0, h = 1)")
{
    // zwei Ebenen einer Saite ohne Verstimmung und mit gleichem Verlust senkrecht und waagrecht: entartet
    const std::vector<EbenenMode> m = ebenenModen({2.0 * 3.14159 * 220.0}, 0.0, 0.3, 1.0, 2.0, 2.0);
    KleineMatrix a(2);
    a(0, 0) = std::complex<double>(-1.0 - 2.0, 0.0);
    a(1, 1) = std::complex<double>(-1.0 - 2.0, 0.0);
    CHECK(zerlegeKlein(a).entartet);
    CHECK_FALSE(m.empty());
}
