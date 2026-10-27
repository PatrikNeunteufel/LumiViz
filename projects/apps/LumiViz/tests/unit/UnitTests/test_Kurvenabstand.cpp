/**
 ****************************************************************************************
 * @file   test_Kurvenabstand.cpp
 * @brief  Loop, Stufe 31d-7 B, 7.5.3: der Abstand der Kurven an Huellkurven von Hand
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S155: K3 a, K6 a, K6-1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Qt-Anwendung: die Huellkurven sind gerechnet (Abfall in dB, Schwebung zweier Moden
 * als |1 + r·e^(i2πft)|), daraus Kurven nach Schroeder und Autokovarianz wie in der Messung.
 *
 * - Gleiche Huellkurve, gleicher Abstand null — auch, wenn eine Seite weiter reicht (gekuerzt).
 * - Der Anteil Abklingen waechst stetig mit der Abweichung von T60.
 * - Der Anteil Schwebung waechst stetig mit der Abweichung der Schwebungsfrequenz und hat ein Minimum
 *   (SM4 a: keine Schwellen, an denen eine Suche haengen bleibt).
 * - Gewicht nach dem Fall der Kurve; die Taste gewichtet gemittelt.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Kurvenabstand.hpp"

#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

using namespace lumi::messung;

namespace
{

constexpr double kSchritt = 0.01;

/// Eine Huellkurve von Hand: Abfall mit `t60` ab Rahmen 0, dazu eine Schwebung (zweite Mode `r`-mal so laut,
/// `schwebungHz` daneben), `rahmen` Rahmen; Boden weit darunter; Bereich [`von`, `bis`).
Teilton huelle(double t60, double r, double schwebungHz, std::size_t rahmen, std::size_t von, std::size_t bis)
{
    Teilton t;
    t.k = 1;
    for (std::size_t i = 0; i < rahmen; ++i)
    {
        const double zeit = static_cast<double>(i) * kSchritt;
        const std::complex<double> z = 1.0 + r * std::polar(1.0, 2.0 * std::numbers::pi * schwebungHz * zeit);
        t.pegelDb.push_back(-60.0 * zeit / t60 + 20.0 * std::log10(std::abs(z)));
        t.bodenDb.push_back(-250.0);
    }
    t.bereichVon = von;
    t.bereichBis = bis;
    return t;
}

TeiltonKurven kurvenVon(const Teilton& t)
{
    TeiltonKurven k;
    k.erkannt = true;
    k.bereichVon = t.bereichVon;
    k.bereichBis = t.bereichBis;
    k.kurve = schroederKurve(t, kSchritt, t.bereichVon, t.bereichBis);
    k.kovarianz = autokovarianz(t, kSchritt, t.bereichVon, t.bereichBis);
    return k;
}

}  // namespace

TEST_CASE("Loop Kurvenabstand 31d-7 B gleich, gekuerzt, stetig (7.5.3)")
{
    SUBCASE("gleiche Huellkurve: Abstand null; reicht das Modell weiter, wird es gekuerzt")
    {
        const TeiltonKurven s = kurvenVon(huelle(4.0, 0.3, 1.0, 1000, 10, 800));
        const KurvenAbstand gleich = kurvenAbstand(s, s);
        REQUIRE(gleich.gilt);
        CHECK(gleich.abklingenDb == doctest::Approx(0.0).scale(1e-12));
        CHECK(gleich.schwebungDb == doctest::Approx(0.0).scale(1e-12));
        CHECK(gleich.endePunkt == 160);  // 800 Rahmen = 8 s = 160 Punkte
        CHECK(gleich.vonPunkt == 2);           // Rahmen 10 → der erste Rasterpunkt ab 0,1 s
        CHECK(gleich.punkte == 160 - 20 - 2);  // bis 1 s vor dem Ende

        const TeiltonKurven laenger = kurvenVon(huelle(4.0, 0.3, 1.0, 1000, 10, 1000));
        const KurvenAbstand gekuerzt = kurvenAbstand(laenger, s);
        REQUIRE(gekuerzt.gilt);
        MESSAGE("weiter reichendes Modell, gekuerzt: Abklingen " << gekuerzt.abklingenDb << " dB");
        CHECK(gekuerzt.endePunkt == 160);
        CHECK(gekuerzt.abklingenDb < 0.05);  // das Ende liegt auf dem Raster bis zu einem Punkt verschieden
        // spaeterer Anfang des Modells: neu bezogen
        const TeiltonKurven spaeter = kurvenVon(huelle(4.0, 0.3, 1.0, 1000, 60, 800));
        const KurvenAbstand bezogen = kurvenAbstand(spaeter, s);
        REQUIRE(bezogen.gilt);
        CHECK(bezogen.vonPunkt == 12);
        CHECK(bezogen.abklingenDb < 0.05);
    }
    SUBCASE("Anteil Abklingen waechst stetig mit T60, Minimum am Ziel")
    {
        const TeiltonKurven s = kurvenVon(huelle(4.0, 0.0, 1.0, 1000, 10, 800));
        std::vector<double> d;
        for (double t60 = 3.0; t60 <= 5.01; t60 += 0.1) d.push_back(kurvenAbstand(kurvenVon(huelle(t60, 0.0, 1.0, 1000, 10, 800)), s).abklingenDb);
        const auto minimum = static_cast<std::size_t>(std::min_element(d.begin(), d.end()) - d.begin());
        MESSAGE("T60 3 … 5 s: Abklingen " << d.front() << " … " << d[minimum] << " … " << d.back() << " dB");
        CHECK(minimum == 10);  // 4,0 s
        for (std::size_t i = 1; i < d.size(); ++i)
        {
            CAPTURE(i);
            if (i <= minimum)
                CHECK(d[i] < d[i - 1]);
            else
                CHECK(d[i] > d[i - 1]);
        }
    }
    SUBCASE("Anteil Schwebung waechst stetig mit der Schwebungsfrequenz, Minimum am Ziel")
    {
        const TeiltonKurven s = kurvenVon(huelle(10.0, 0.5, 1.0, 1000, 10, 900));
        std::vector<double> d;
        for (double f = 0.8; f <= 1.201; f += 0.02) d.push_back(kurvenAbstand(kurvenVon(huelle(10.0, 0.5, f, 1000, 10, 900)), s).schwebungDb);
        const auto minimum = static_cast<std::size_t>(std::min_element(d.begin(), d.end()) - d.begin());
        double groessterSchritt = 0.0;
        for (std::size_t i = 1; i < d.size(); ++i) groessterSchritt = std::max(groessterSchritt, std::abs(d[i] - d[i - 1]));
        MESSAGE("Schwebung 0,8 … 1,2 Hz: " << d.front() << " … " << d[minimum] << " … " << d.back() << " dB, groesster Schritt "
                                             << groessterSchritt);
        CHECK(minimum == 10);  // 1,0 Hz
        for (std::size_t i = 1; i < d.size(); ++i)
        {
            CAPTURE(i);
            if (i <= minimum)
                CHECK(d[i] < d[i - 1]);
            else
                CHECK(d[i] > d[i - 1]);
        }
        // die Tiefe ebenso: r 0,3 … 0,7
        std::vector<double> t;
        for (double r = 0.3; r <= 0.701; r += 0.05) t.push_back(kurvenAbstand(kurvenVon(huelle(10.0, r, 1.0, 1000, 10, 900)), s).schwebungDb);
        CHECK(std::min_element(t.begin(), t.end()) - t.begin() == 4);  // 0,5
    }
}

TEST_CASE("Loop Kurvenabstand S156 fehlende Kurve des Modells (RV1 a)")
{
    // Salamander: T60 4 s ueber 8 s; das Modell faellt immer schneller, zuletzt fehlt ihm die Kurve
    const TeiltonKurven s = kurvenVon(huelle(4.0, 0.0, 1.0, 1000, 10, 800));
    std::vector<double> d;
    for (const double t60 : {4.0, 2.0, 1.0, 0.5, 0.25, 0.1, 0.05})
        d.push_back(kurvenAbstand(kurvenVon(huelle(t60, 0.0, 1.0, 1000, 10, 800)), s).abklingenDb);
    const KurvenAbstand fehlt = kurvenAbstand(TeiltonKurven{}, s);
    REQUIRE(fehlt.gilt);
    MESSAGE("T60 4 … 0,05 s: " << d[0] << ", " << d[1] << ", " << d[2] << ", " << d[3] << ", " << d[4] << " … " << d.back()
                                << " dB; ohne Kurve " << fehlt.abklingenDb << " dB");
    // waechst, bis das Modell schneller faellt als das Raster (0,05 s): ab T60 0,25 s gleich dem Fall ohne Kurve
    for (std::size_t i = 1; i <= 3; ++i)
    {
        CAPTURE(i);
        CHECK(d[i] > d[i - 1]);
    }
    for (std::size_t i = 4; i < d.size(); ++i)
    {
        CAPTURE(i);
        CHECK(d[i] == doctest::Approx(fehlt.abklingenDb).epsilon(1e-9));
    }
    // nie billiger als ein Modell mit Kurve, und ohne Sprung am Uebergang (T60 0,5 s liegt 0,05 dB darunter)
    for (const double x : d) CHECK(fehlt.abklingenDb >= x - 1e-9);
    CHECK(fehlt.abklingenDb - d[3] < 0.5);
    // dasselbe Gewicht wie mit Kurve
    CHECK(fehlt.gewicht == doctest::Approx(kurvenAbstand(s, s).gewicht).epsilon(0.05));
}

TEST_CASE("Loop Kurvenabstand 31d-7 B Gewicht und Taste (7.5.3)")
{
    // Fall bis 1 s vor dem Ende: T60 20 s ueber 6 s Bereich faellt etwa 15 dB → Gewicht ≈ 0,25; T60 2 s faellt weit
    // ueber 60 dB → 1
    const TeiltonKurven flach = kurvenVon(huelle(20.0, 0.0, 1.0, 800, 10, 700));
    const TeiltonKurven steil = kurvenVon(huelle(2.0, 0.0, 1.0, 800, 10, 700));
    const KurvenAbstand af = kurvenAbstand(flach, flach);
    const KurvenAbstand as = kurvenAbstand(steil, steil);
    MESSAGE("Gewicht flach " << af.gewicht << ", steil " << as.gewicht);
    CHECK(af.gewicht > 0.1);
    CHECK(af.gewicht < 0.4);
    CHECK(as.gewicht == 1.0);

    KurvenAbstand a;
    a.gilt = true;
    a.gewicht = 1.0;
    a.abklingenDb = 3.0;
    a.schwebungDb = 4.0;
    KurvenAbstand b = a;
    b.gewicht = 0.25;
    b.abklingenDb = 0.0;
    b.schwebungDb = 0.0;
    KurvenAbstand c;  // gilt nicht
    c.abklingenDb = 100.0;
    const std::optional<double> d = tastenAbstand({a, b, c});
    REQUIRE(d.has_value());
    CHECK(*d == doctest::Approx(std::sqrt(25.0 / 1.25)));
    CHECK_FALSE(tastenAbstand({c}).has_value());
}
