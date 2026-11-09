/**
 * @file test_hoerschwellen.cpp
 * @brief Die Hoerschwellen nach HW1 a (IC-A2) und die Auswertung der Probe nach HW2 a und HW4 a (IC-A3) — gegen die
 *        Zahlen der Entscheide: Faktor 0,7923 / 1,4309 (Tolonen & Jaervelaeinen 2000, Tab. 6, S. 11), Knick 0,5 dB,
 *        Ewert 2002 S. 41–42 (1,2 dB ab −18 dB, 4,1 dB bei −28 dB), AM-Erkennung m ≈ 0,05 ⇒ 0,87 dB (Jaervelaeinen 2003
 *        S. 29); Vorzeichentest exakt, Holm. Die Vergleichswerte sind unabhaengig in Python nachgerechnet (Journal
 *        2026-10-31).
 */
#include "doctest.h"

#include "messung/Hoerschwellen.hpp"
#include "messung/Probe.hpp"

#include <cmath>
#include <string>
#include <vector>

using namespace ic::messung;

namespace
{

Hoerschwellen tabelle()
{
    const HoerschwellenGelesen g = liesHoerschwellen(std::string(IC_WERTE) + "/hoerschwellen.txt");
    REQUIRE_MESSAGE(g.werte.has_value(), g.fehler);
    return *g.werte;
}

Abweichung abweichung(const std::string& groesse, Skala skala, double modell, double ziel, double toleranz)
{
    Abweichung a;
    a.groesse = groesse;
    a.skala = skala;
    a.modell = modell;
    a.ziel = ziel;
    a.toleranz = toleranz;
    a.residuum = (skala == Skala::Faktor ? std::log(modell / ziel) : modell - ziel) / toleranz;
    a.verhaeltnis = std::abs(a.residuum);
    a.getroffen = a.verhaeltnis <= 1.0;
    return a;
}

}  // namespace

TEST_CASE("Hoerschwellen: die Tabelle nach HW1 a, laengstes passendes Praefix")
{
    const Hoerschwellen h = tabelle();
    CHECK(h.zeilen.size() == 7);
    REQUIRE(h.fuer("vorwaerts_t60_3") != nullptr);
    CHECK(h.fuer("vorwaerts_t60_3")->muster == "vorwaerts_t60");
    CHECK(h.fuer("huelle_20_s")->muster == "huelle_");
    CHECK(h.fuer("schwebung_hz_1")->muster == "schwebung_hz");  // nicht schwebung_db
    CHECK(h.fuer("f0") == nullptr);
    CHECK(std::exp(-h.fuer("t60_spaet_2")->unten) == doctest::Approx(0.7923).epsilon(1e-6));
    CHECK(std::exp(h.fuer("t60_spaet_2")->oben) == doctest::Approx(1.4309).epsilon(1e-6));
}

TEST_CASE("Hoerschwellen: Faktor unsymmetrisch, dB, Ewert")
{
    const Hoerschwellen h = tabelle();
    // Zeiten: kuerzer ab 0,7923, laenger ab 1,4309
    CHECK(hoerbarGleich(h, "vorwaerts_t60_1", 0.80, 1.0));
    CHECK_FALSE(hoerbarGleich(h, "vorwaerts_t60_1", 0.79, 1.0));
    CHECK(hoerbarGleich(h, "vorwaerts_t60_1", 1.43, 1.0));
    CHECK_FALSE(hoerbarGleich(h, "vorwaerts_t60_1", 1.44, 1.0));
    CHECK_FALSE(hoerbarGleich(h, "vorwaerts_t60_1", 0.0, 1.0));  // kein Wert, kein Faktor
    // Knick 0,5 dB
    CHECK(hoerbarGleich(h, "knick_tiefe_db_2", 3.5, 3.0));
    CHECK_FALSE(hoerbarGleich(h, "knick_tiefe_db_2", 3.6, 3.0));
    // ohne Zeile nie gleich
    CHECK_FALSE(hoerbarGleich(h, "f0", 1.0, 1.0));
    // Ewert: D 1,2 dB ab −18 dB, 2,65 dB bei −23 dB, 4,1 dB ab −28 dB
    CHECK(ewertD(std::pow(10.0, -18.0 / 20.0)) == doctest::Approx(1.2));
    CHECK(ewertD(std::pow(10.0, -23.0 / 20.0)) == doctest::Approx(2.65));
    CHECK(ewertD(std::pow(10.0, -30.0 / 20.0)) == doctest::Approx(4.1));
    CHECK(ewertPp(ewertM(6.0)) == doctest::Approx(6.0));
    // 6 dB: Grenze 6,980; 1 dB: 1,443; 0,5 dB: 0,802 (dort gilt die Erkennung 0,87 dB)
    CHECK(ewertGleich(6.0, 6.97));
    CHECK_FALSE(ewertGleich(6.0, 6.99));
    CHECK(ewertGleich(1.0, 1.44));
    CHECK_FALSE(ewertGleich(1.0, 1.45));
    CHECK(ewertGleich(0.5, 0.86));
    CHECK_FALSE(ewertGleich(0.5, 0.88));
    CHECK(hoerbarGleich(h, "schwebung_db_1", 6.97, 6.0));
}

TEST_CASE("Hoerschwellen: hoerbar getroffen (HW2 a) und eindeutig ueber Startpunkte — bei ungleichen Grenzen die engere")
{
    const Hoerschwellen h = tabelle();
    const double tau = kUntergrenzeFaktor;  // ln 1,03
    // 1,3-mal so lang: nicht in τ, aber unter 1,4309
    const Abweichung lang = abweichung("t60_spaet_1", Skala::Faktor, 2.6, 2.0, tau);
    CHECK_FALSE(lang.getroffen);
    CHECK(hoerbarGetroffen(h, lang));
    // eindeutig: 2,0 gegen 2,6 ist in eine Richtung hoerbar (0,769 < 0,7923) — nicht eindeutig; 2,0 gegen 2,4 eindeutig
    const Abweichung a = abweichung("t60_spaet_1", Skala::Faktor, 2.0, 2.0, tau);
    const Abweichung b = abweichung("t60_spaet_1", Skala::Faktor, 2.4, 2.0, tau);
    CHECK_FALSE(eindeutigNachHw2(h, {&a, &lang}));
    CHECK(eindeutigNachHw2(h, {&a, &b}));
    Abweichung fehlt = a;
    fehlt.fehlt = true;
    CHECK_FALSE(eindeutigNachHw2(h, {&a, &fehlt}));
    CHECK_FALSE(hoerbarGetroffen(h, fehlt));
}

TEST_CASE("Probe: Vorzeichentest exakt, Holm, systematisch und Ausreisser (HW4 a)")
{
    CHECK(vorzeichenTestP(0, 10) == doctest::Approx(0.001953125));
    CHECK(vorzeichenTestP(1, 14) == doctest::Approx(0.0018310546875));
    CHECK(vorzeichenTestP(3, 10) == doctest::Approx(0.34375));
    CHECK(vorzeichenTestP(5, 10) == doctest::Approx(1.0));
    CHECK(vorzeichenTestP(0, 0) == 1.0);
    CHECK(holm({0.01, 0.04, 0.03}) == std::vector<bool>{true, false, false});
    CHECK(holm({0.01, 0.02, 0.03}) == std::vector<bool>{true, true, true});
    // zehn Tasten: „x“ immer zu gross (r = 2, an Taste 7 r = 20), „y“ abwechselnd
    std::vector<TastenSumme> tasten;
    for (int t = 0; t < 10; ++t)
    {
        TastenSumme s;
        s.midi = t;
        s.groessen.push_back(abweichung("x", Skala::Dezibel, t == 7 ? 20.0 : 2.0, 0.0, 1.0));
        s.groessen.push_back(abweichung("y", Skala::Dezibel, t % 2 == 0 ? 2.0 : -2.0, 0.0, 1.0));
        tasten.push_back(s);
    }
    const Hoerschwellen leer;
    const std::vector<GroessenBefund> b = systematik(leer, tasten);
    REQUIRE(b.size() == 2);
    CHECK(b[0].groesse == "x");
    CHECK(b[0].systematisch);
    CHECK(b[0].zuGross == 10);
    CHECK(b[0].ausreisser == std::vector<int>{7});
    CHECK(b[1].groesse == "y");
    CHECK_FALSE(b[1].systematisch);
    CHECK(b[1].p == doctest::Approx(1.0));
    const HoerZahlen z = hoerZahlen(leer, tasten);
    CHECK(z.groessen == 20);
    CHECK(z.hoerbarGetroffen == 0);
}
