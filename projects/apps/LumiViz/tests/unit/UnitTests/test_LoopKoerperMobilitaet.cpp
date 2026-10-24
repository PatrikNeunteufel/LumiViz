/**
 ****************************************************************************************
 * @file   test_LoopKoerperMobilitaet.cpp
 * @brief  Loop, Stufe 31c-6: die Mobilitaet am Steg aus den Moden — MAESSTRO Gl. 10 und 12,
 *         Terzbaender, Modendichte (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide
 *         Patrik S149: PS1 a, KV1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung.
 *
 * - Ein Einmassenschwinger (eine Mode): Y gegen die geschlossene Loesung, |Y| in der Resonanz
 *   1/(η ω0).
 * - Terzbaender: Mitten nach IEC 61260 (1 kHz, 1,25 kHz …), Grenzen schliessen aneinander an.
 * - Das geometrische Mittel (Gl. 12) einer Mode gegen die Summe in dB; die Modendichte im Band.
 * - Eine grosse duenne Aluminiumplatte (frei aufliegend, η = 2 %): das Mittel von Re(Y) ueber ein
 *   Band vieler Moden gegen die charakteristische Mobilitaet der unendlichen Platte
 *   1/(8 √(ρh D)) (MAESSTRO Gl. 11; Skudrzyk) — der Satz vom Mittelwert, gegen den PS1 a prueft.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/Mobilitaet.hpp"
#include "InstrumentBody/Umsetzungen.hpp"
#include "InstrumentBody/Verformungsenergie.hpp"

#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

namespace kp = lumi::koerper;

TEST_CASE("Loop Koerper 31c-6 Mobilitaet: ein Einmassenschwinger")
{
    kp::ModeImKoerper m;
    m.frequenzHz = 100.0;
    m.verlustfaktor = 0.02;
    m.amStegpunkt = {2.0};  // φ = 2 → modale Masse 1/4 kg
    const std::vector<kp::ModeImKoerper> moden{m};
    const double w0 = 2.0 * std::numbers::pi * 100.0;
    for (const double f : {10.0, 99.0, 100.0, 101.0, 1000.0})
    {
        CAPTURE(f);
        const double w = 2.0 * std::numbers::pi * f;
        const std::complex<double> soll = std::complex<double>(0.0, w) * 4.0 / std::complex<double>(w0 * w0 - w * w, 0.02 * w0 * w);
        const std::complex<double> ist = kp::mobilitaet(moden, 0, f);
        CHECK(std::abs(ist - soll) < 1e-12 * std::abs(soll));
    }
    // in der Resonanz reell und |Y| = φ² / (η ω0)
    const std::complex<double> resonanz = kp::mobilitaet(moden, 0, 100.0);
    CHECK(std::abs(resonanz.imag()) < 1e-12 * std::abs(resonanz));
    CHECK(std::abs(resonanz) == doctest::Approx(4.0 / (0.02 * w0)).epsilon(1e-12));
    CHECK(kp::mobilitaetDb(resonanz) == doctest::Approx(20.0 * std::log10(4.0 / (0.02 * w0))).epsilon(1e-12));
    // ein Stegpunkt, den die Mode nicht kennt, hat keine Mobilitaet
    CHECK(kp::mobilitaet(moden, 1, 100.0) == std::complex<double>(0.0, 0.0));
}

TEST_CASE("Loop Koerper 31c-6 Mobilitaet: Terzbaender, Mittel und Dichte")
{
    const auto baender = kp::terzbaender(100.0, 5000.0);
    REQUIRE(baender.size() == 18);  // 100, 125, 160 … 5000 Hz (gerechnete Mitten 10^(n/10))
    CHECK(baender.front().mitteHz == doctest::Approx(100.0));
    CHECK(baender.back().mitteHz == doctest::Approx(std::pow(10.0, 3.7)));  // „5000 Hz" nominal
    for (std::size_t i = 0; i + 1 < baender.size(); ++i)
    {
        CHECK(baender[i].bisHz == doctest::Approx(baender[i + 1].vonHz));
        CHECK(baender[i].bisHz / baender[i].vonHz == doctest::Approx(std::pow(10.0, 0.1)));
    }
    // das geometrische Mittel ist das Mittel in dB
    kp::ModeImKoerper m;
    m.frequenzHz = 300.0;
    m.verlustfaktor = 0.05;
    m.amStegpunkt = {1.0, 0.5};
    const std::vector<kp::ModeImKoerper> moden{m};
    double summe = 0.0;
    for (double f = 25.0; f <= 5000.0; f += 1.0) summe += kp::mobilitaetDb(kp::mobilitaet(moden, 1, f));
    CHECK(kp::mittlereMobilitaetDb(moden, 1) == doctest::Approx(summe / 4976.0).epsilon(1e-12));
    // halbe Form → 6,02 dB weniger an jedem Punkt, also auch im Mittel
    CHECK(kp::mittlereMobilitaetDb(moden, 0) - kp::mittlereMobilitaetDb(moden, 1) == doctest::Approx(20.0 * std::log10(4.0)).epsilon(1e-9));
    // die Dichte: drei Moden im Band um 1 kHz
    std::vector<kp::ModeImKoerper> drei;
    for (const double f : {900.0, 1000.0, 1100.0, 2000.0})
    {
        kp::ModeImKoerper k;
        k.frequenzHz = f;
        drei.push_back(k);
    }
    const kp::Terzband kilo{1000.0, 1000.0 / std::pow(10.0, 0.05), 1000.0 * std::pow(10.0, 0.05)};
    CHECK(kp::bandDichte(drei, kilo) == doctest::Approx(3.0 / (kilo.bisHz - kilo.vonHz)));
}

TEST_CASE("Loop Koerper 31c-6 Mobilitaet: grosse Platte gegen die unendliche (Satz vom Mittelwert)")
{
    // Aluminium 1,0 × 0,8 m, 1 mm, frei aufliegend, k = 3, η = 2 %
    const double la = 1.0, lb = 0.8, h = 0.001;
    kp::Material alu;
    alu.name = "aluminium";
    alu.art = kp::MaterialArt::Isotrop;
    alu.dichte = 2700.0;
    alu.ex = 70.0e9;
    alu.nuxy = 0.33;
    const double d = alu.ex * h * h * h / (12.0 * (1.0 - alu.nuxy * alu.nuxy));
    const double yc = 1.0 / (8.0 * std::sqrt(alu.dichte * h * d));  // MAESSTRO Gl. 11

    kp::NetzAuftrag netzAuftrag;
    netzAuftrag.umriss = {{0.0, 0.0}, {la, 0.0}, {la, lb}, {0.0, lb}};
    netzAuftrag.kante = 0.03;
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(netzAuftrag);
    REQUIRE(netz.fehler.empty());
    const auto steife = [&](kp::Punkt2) {
        kp::Steife st = kp::gedreht(alu, 0.0);
        st.h = h;
        st.werkstoff = 0;
        return st;
    };
    const kp::LagrangeDreieck element(3);
    const kp::PlattenSystem s = kp::baueSystem(netz, element, steife, kp::Auflager::GelenkigAchsparallel);
    REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
    const kp::Verformungsenergie energie(s, element, steife, 1);
    // drei Punkte im Inneren, fern vom Rand und von den Symmetrielinien
    const std::vector<kp::Stegpunkt> punkte = {{60, {0.37, 0.29}, "a"}, {61, {0.61, 0.47}, "b"}, {62, {0.29, 0.53}, "c"}};
    const kp::Auswertung auswertung(s, element, punkte, {}, {}, {}, &energie, {0.02});

    std::vector<kp::ModeImKoerper> moden;
    kp::EigenAuftrag auftrag;
    auftrag.k = &s.k;
    auftrag.m = &s.m;
    auftrag.bisHz = 1200.0;  // das Band 300–1000 Hz und Moden darueber fuer die Flanke
    auftrag.jeMode = [&](const kp::Mode& mode) { moden.push_back(auswertung.werte(mode)); };
    const kp::EigenErgebnis erg = kp::eigenloeser().front()->loese(auftrag);
    REQUIRE_MESSAGE(erg.fehler.empty(), erg.fehler);
    for (const kp::ModeImKoerper& m : moden) CHECK(m.verlustfaktor == doctest::Approx(0.02).epsilon(1e-9));

    // Mittel von Re(Y) ueber 300–1000 Hz, je Punkt und ueber alle drei
    double summe = 0.0;
    int zahl = 0;
    for (std::size_t j = 0; j < punkte.size(); ++j)
    {
        double re = 0.0;
        int n = 0;
        for (double f = 300.0; f <= 1000.0; f += 1.0, ++n) re += kp::mobilitaet(moden, j, f).real();
        MESSAGE("Punkt ", punkte[j].quelle, ": mittleres Re(Y) ", re / n, " s/kg (unendliche Platte ", yc, ")");
        summe += re;
        zahl += n;
    }
    const double mittel = summe / zahl;
    const kp::Terzband band{650.0, 300.0, 1000.0};
    MESSAGE(moden.size(), " Moden bis 1200 Hz, Dichte im Band ", kp::bandDichte(moden, band), " /Hz (asymptotisch ",
            la * lb / 2.0 * std::sqrt(alu.dichte * h / d), "), mittleres Re(Y) ", mittel, " gegen ", yc, " s/kg");
    CHECK(mittel == doctest::Approx(yc).epsilon(0.15));
}
