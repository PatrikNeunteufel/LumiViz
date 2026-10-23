/**
 ****************************************************************************************
 * @file   test_LoopKoerperAuswertung.cpp
 * @brief  Loop, Stufe 31c-5b: was der Koerper je Mode traegt — Wert der Form an den
 *         Stegpunkten, Daempfung, Abstrahlgewicht nach Rayleigh je Hoerort
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S146–S148: KB1 a, BM6 a,
 *         PL31c-5 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung; nur der Steinway liest seinen Bauplan.
 *
 * - Daempfung: α(f) = a f² + b f + c nach RT-0425 Gl. 1, η = α/(2πf) im gemessenen Bereich.
 * - Abtastung: an einem Knoten genau sein Wert; Polynome bis zur Ordnung des Elements werden
 *   an beliebigen Stellen genau wiedergegeben; ein Punkt ausserhalb wird geklemmt.
 * - Rayleigh am Kolben: eine Scheibe, die sich ganz bewegt, auf der Achse gegen die geschlossene
 *   Loesung — auch im Nahfeld.
 * - Die erste Mode der frei aufliegenden Pruefplatte: Wert an zwei Punkten gegen Navier
 *   (sin · sin), Abstrahlung im Fernfeld gegen ρ/(2π) e^{−ikR}/R ∫φ dS.
 * - Der Steinway D: jeder Stegpunkt liegt im Netz; meldet die Zeit der Vorbereitung.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/Umsetzungen.hpp"
#include "loop/KoerperBauplan.hpp"

#include <QtGlobal>

#include <chrono>
#include <cmath>
#include <complex>
#include <numbers>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

kp::Material fichte()
{
    kp::Material m;
    m.name = "spruce";
    m.dichte = 380.0;
    m.ex = 11.0e9;
    m.ey = 0.65e9;
    m.gxy = 0.66e9;
    m.gxz = 1.2e9;
    m.gyz = 0.042e9;
    m.nuxy = 0.26;
    return m;
}

kp::PlattenSystem platte(const std::vector<kp::Punkt2>& umriss, double kante, int k, double h, kp::Auflager auflager)
{
    kp::NetzAuftrag auftrag;
    auftrag.umriss = umriss;
    auftrag.kante = kante;
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(auftrag);
    REQUIRE(netz.fehler.empty());
    const kp::Material mat = fichte();
    kp::PlattenSystem s = kp::baueSystem(netz, kp::LagrangeDreieck(k),
                                         [&](kp::Punkt2) {
                                             kp::Steife st = kp::gedreht(mat, 0.0);
                                             st.h = h;
                                             return st;
                                         },
                                         auflager);
    REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
    return s;
}

/// Eine Form, die an jedem w den Wert f(Knoten) traegt, sonst 0.
template <typename F>
std::vector<double> formAus(const kp::PlattenSystem& s, F f)
{
    std::vector<double> form(static_cast<std::size_t>(s.k.n), 0.0);
    for (std::size_t q = 0; q < s.knoten.size(); ++q)
        if (const int g = s.frei[3 * q]; g >= 0) form[static_cast<std::size_t>(g)] = f(s.knoten[q]);
    return form;
}

}  // namespace

TEST_CASE("Loop Koerper 31c-5b Daempfung")
{
    const kp::Daempfung d;  // RT-0425 Gl. 1: a = 2·10⁻⁵, b = 0,07, c = 0
    CHECK(d.alpha(100.0) == doctest::Approx(0.2 + 7.0));
    CHECK(d.verlustfaktor(100.0) == doctest::Approx(0.072 / (2.0 * std::numbers::pi)));
    CHECK(d.verlustfaktor(1000.0) == doctest::Approx(0.09 / (2.0 * std::numbers::pi)));
    // im gemessenen Bereich 1–3 % (Ege, Boutillon & Rebillat 2013) von 50 Hz bis 5 kHz
    for (const double f : {50.0, 200.0, 1000.0, 3000.0, 5000.0})
    {
        CAPTURE(f);
        CHECK(d.verlustfaktor(f) > 0.01);
        CHECK(d.verlustfaktor(f) < 0.03);
    }
    CHECK(d.verlustfaktor(0.0) == 0.0);
}

TEST_CASE("Loop Koerper 31c-5b Abtastung")
{
    const std::vector<kp::Punkt2> rechteck = {{0.0, 0.0}, {0.3, 0.0}, {0.3, 0.2}, {0.0, 0.2}};
    for (const int k : {1, 2, 3})
    {
        CAPTURE(k);
        const kp::LagrangeDreieck element(k);
        const kp::PlattenSystem s = platte(rechteck, 0.05, k, 0.008, kp::Auflager::Frei);
        REQUIRE(s.elemente.size() > 0);
        // ein Polynom vom Grad k wird genau wiedergegeben
        const auto f = [k](kp::Punkt2 p) { return 1.0 + 2.0 * p.x - p.y + (k >= 2 ? 3.0 * p.x * p.y : 0.0) + (k >= 3 ? p.y * p.y * p.x : 0.0); };
        const std::vector<double> form = formAus(s, f);
        for (const kp::Punkt2 p : {kp::Punkt2{0.123, 0.071}, kp::Punkt2{0.29, 0.19}, kp::Punkt2{0.01, 0.15}})
        {
            const kp::Abtastung a = kp::abtastung(s, element, p);
            CHECK(a.imNetz);
            CHECK(kp::wertBei(a, form) == doctest::Approx(f(p)).epsilon(1e-10));
        }
        // an einem Knoten genau sein Wert
        const std::size_t q = s.knoten.size() / 2;
        const kp::Abtastung amKnoten = kp::abtastung(s, element, s.knoten[q]);
        CHECK(kp::wertBei(amKnoten, form) == doctest::Approx(form[static_cast<std::size_t>(s.frei[3 * q])]).epsilon(1e-10));
        // ausserhalb: geklemmt, mit Abstand
        const kp::Abtastung draussen = kp::abtastung(s, element, {-0.05, 0.1});
        CHECK_FALSE(draussen.imNetz);
        CHECK(draussen.abstand == doctest::Approx(0.05).epsilon(1e-9));
        CHECK(kp::wertBei(draussen, form) == doctest::Approx(f({0.0, 0.1})).epsilon(1e-10));
    }
}

TEST_CASE("Loop Koerper 31c-5b Rayleigh Kolben")
{
    // eine Scheibe, die sich ganz mit w = 1 bewegt: auf der Achse in der Hoehe z ist
    // ∫ e^{−ikR}/R dS = 2π (e^{−ik√(z²+a²)} − e^{−ikz}) / (−ik), genau (auch im Nahfeld)
    std::vector<kp::Punkt2> kreis;
    const int ecken = 256;
    const double radius = 0.1;
    for (int i = 0; i < ecken; ++i)
    {
        const double w = 2.0 * std::numbers::pi * i / ecken;
        kreis.push_back({radius * std::cos(w), radius * std::sin(w)});
    }
    const double a = std::sqrt(kp::flaeche(kreis) / std::numbers::pi);  // flaechengleicher Radius
    const int k = 2;
    const kp::PlattenSystem s = platte(kreis, 0.015, k, 0.008, kp::Auflager::Frei);
    const kp::Luft luft;
    std::vector<kp::Hoerort> orte;
    for (const double z : {0.05, 0.2, 1.0}) orte.push_back({"z" + std::to_string(z), {0.0, 0.0}, z, "Achse"});
    const kp::Auswertung auswertung(s, kp::LagrangeDreieck(k), {}, orte, luft);
    kp::Mode mode;
    mode.form = formAus(s, [](kp::Punkt2) { return 1.0; });
    for (const double f : {200.0, 2000.0, 8000.0})
    {
        mode.frequenzHz = f;
        const kp::ModeImKoerper m = auswertung.werte(mode);
        REQUIRE(m.abstrahlung.size() == orte.size());
        const double wellenzahl = 2.0 * std::numbers::pi * f / luft.schall;
        const std::complex<double> i(0.0, 1.0);
        for (std::size_t h = 0; h < orte.size(); ++h)
        {
            const double z = orte[h].hoehe;
            const std::complex<double> integral =
                2.0 * std::numbers::pi * (std::exp(-i * wellenzahl * std::sqrt(z * z + a * a)) - std::exp(-i * wellenzahl * z)) / (-i * wellenzahl);
            const std::complex<double> soll = luft.dichte / (2.0 * std::numbers::pi) * integral;
            CAPTURE(f);
            CAPTURE(z);
            CHECK(std::abs(m.abstrahlung[h] - soll) < 2e-3 * std::abs(soll));
        }
    }
}

TEST_CASE("Loop Koerper 31c-5b erste Mode der Pruefplatte")
{
    // frei aufliegend 0,3 × 0,2 m, 8 mm Fichte, k = 3: die erste Mode ist sin(πx/a) sin(πy/b),
    // bezueglich M normiert mit der Amplitude √(4/(ρhab)) (die Drehtraegheit aendert das um Promille)
    const double la = 0.3, lb = 0.2, h = 0.008, rho = 380.0;
    const kp::PlattenSystem s = platte({{0.0, 0.0}, {la, 0.0}, {la, lb}, {0.0, lb}}, 0.04, 3, h, kp::Auflager::GelenkigAchsparallel);
    const std::vector<kp::Stegpunkt> punkte = {{60, {la / 2, lb / 2}, "Mitte"}, {61, {la / 4, lb / 4}, "Viertel"}};
    const kp::Hoerort fern{"far", {la / 2, lb / 2}, 20.0, "Fernfeld"};
    const kp::Auswertung auswertung(s, kp::LagrangeDreieck(3), punkte, {fern});

    for (const auto& loeser : kp::eigenloeser())
    {
        CAPTURE(loeser->name());
        kp::EigenAuftrag auftrag;
        auftrag.k = &s.k;
        auftrag.m = &s.m;
        auftrag.bisHz = 400.0;  // Navier: f11 rund 300 Hz, f21 rund 500 Hz
        std::vector<kp::ModeImKoerper> moden;
        auftrag.jeMode = [&](const kp::Mode& mode) { moden.push_back(auswertung.werte(mode)); };
        const kp::EigenErgebnis erg = loeser->loese(auftrag);
        REQUIRE_MESSAGE(erg.fehler.empty(), erg.fehler);
        REQUIRE(moden.size() == 1);
        const kp::ModeImKoerper& m = moden.front();
        REQUIRE(m.amStegpunkt.size() == 2);
        CHECK(auswertung.stegpunkte()[0].imNetz);

        const double amplitude = std::sqrt(4.0 / (rho * h * la * lb));
        CHECK(std::abs(m.amStegpunkt[0]) == doctest::Approx(amplitude).epsilon(0.01));
        CHECK(m.amStegpunkt[1] / m.amStegpunkt[0] == doctest::Approx(0.5).epsilon(1e-3));  // sin²(π/4)
        CHECK(m.verlustfaktor == doctest::Approx(kp::Daempfung{}.verlustfaktor(m.frequenzHz)));

        // Fernfeld: G ≈ ρ/(2π) e^{−ikR}/R ∫φ dS, ∫ sin·sin = 4ab/π² (Vorzeichen wie die Mitte)
        const kp::Luft luft;
        const double integral = std::copysign(amplitude * 4.0 * la * lb / (std::numbers::pi * std::numbers::pi), m.amStegpunkt[0]);
        const double wellenzahl = 2.0 * std::numbers::pi * m.frequenzHz / luft.schall;
        const std::complex<double> soll =
            luft.dichte / (2.0 * std::numbers::pi) * std::exp(std::complex<double>(0.0, -wellenzahl * fern.hoehe)) / fern.hoehe * integral;
        REQUIRE(m.abstrahlung.size() == 1);
        CHECK(std::abs(m.abstrahlung[0] - soll) < 0.01 * std::abs(soll));
        MESSAGE(loeser->name(), ": f1 ", m.frequenzHz, " Hz, phi(Mitte) ", m.amStegpunkt[0], " (Navier ", amplitude, "), |G| ",
                std::abs(m.abstrahlung[0]), " (", std::abs(soll), ")");
    }
}

TEST_CASE("Loop Koerper 31c-5b Steinway D")
{
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(QStringLiteral("asset/loop/instrument_bodies/steinway_d") + QLatin1String(kBauplanEndung), plan),
                    "die Test-Exe aus dem Projekt-Root starten");
    REQUIRE(plan.hoerorte.size() == 2);
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(kp::netzAuftrag(plan, 1000.0));
    REQUIRE(netz.fehler.empty());
    const kp::LagrangeDreieck element(2);
    const kp::PlattenSystem s =
        kp::baueSystem(netz, element, [&](kp::Punkt2 p) { return kp::steifeBei(plan, p); }, kp::Auflager::Eingespannt);
    REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
    const auto beginn = std::chrono::steady_clock::now();
    const kp::Auswertung auswertung(s, element, plan.stegpunkte, plan.hoerorte);
    const double sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
    int imNetz = 0;
    double groessterAbstand = 0.0;
    for (const kp::Abtastung& a : auswertung.stegpunkte())
    {
        imNetz += a.imNetz ? 1 : 0;
        groessterAbstand = std::max(groessterAbstand, a.abstand);
    }
    MESSAGE("Steinway D (Netz bis 1 kHz, k = 2): ", imNetz, " von ", plan.stegpunkte.size(), " Stegpunkten im Netz, groesster Abstand ",
            groessterAbstand * 1e3, " mm; Vorbereitung ", sekunden, " s");
    CHECK(imNetz == static_cast<int>(plan.stegpunkte.size()));
}
