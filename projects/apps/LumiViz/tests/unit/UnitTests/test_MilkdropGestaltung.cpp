/**
 ****************************************************************************************
 * @file   test_MilkdropGestaltung.cpp
 * @brief  Gestaltungs-Regler des Milkdrop-Knotens (S80): Linear-/Radial-Drift und
 *         Weichzeichnen je Frame — Rechenregel und Speicherung
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * Am Bild belegt (Sonde `asset/calibration/milkdrop/drift/`): „Weicher Sog ↖"
 * reproduziert die Bilder vor dem S80-Fix bitgleich. Hier wird die Regel
 * dahinter festgehalten.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/milkdrop/MilkdropGestaltung.hpp"
#include "visualizers/milkdrop/MilkdropPresetState.hpp"
#include "visualizers/milkdrop/MilkdropSerializer.hpp"

#include <cmath>

using lumi::milkdrop::GestaltungsLauf;
using lumi::milkdrop::naechsterVersatz;
using lumi::milkdrop::polar;
using lumi::milkdrop::radialVersatz;

namespace
{
constexpr double kPi = 3.14159265358979323846;
}

TEST_CASE("Gestaltung: aus = kein Versatz, in keinem Frame")
{
    GestaltungsLauf lauf;
    for (int i = 0; i < 20; ++i)
    {
        const auto v = naechsterVersatz(lauf, 0.0, 0.0, 0.0);
        CHECK(v.u == 0.0);
        CHECK(v.v == 0.0);
    }
}

TEST_CASE("Gestaltung: Weicher Sog (0,7071 px bei 135° + Weich 1) = je Frame +0,5 px")
{
    // Vor S80 addierte der Mesh u += 0,5/w, v += 0,5/h — Frame fuer Frame. Die
    // Panel-Staerke hat vier Nachkommastellen; polar() muss trotzdem exakt
    // −0,5/+0,5 liefern, sonst kippt das Konto und der Look ist nicht bitgleich.
    const auto lin = polar(0.7071, 135.0 * kPi / 180.0);
    CHECK(lin.u == -0.5);
    CHECK(lin.v == 0.5);
    GestaltungsLauf lauf;
    for (int i = 0; i < 50; ++i)
    {
        const auto v = naechsterVersatz(lauf, lin.u, lin.v, 1.0);
        CHECK(v.u == doctest::Approx(0.5));
        CHECK(v.v == doctest::Approx(0.5));
    }
}

TEST_CASE("Gestaltung: Linear-Drift allein ist scharf — nur ganze Pixel, Summe stimmt")
{
    GestaltungsLauf lauf;
    double summeU = 0.0;
    double summeV = 0.0;
    for (int i = 0; i < 40; ++i)
    {
        const auto v = naechsterVersatz(lauf, 0.3, -1.25, 0.0);
        CHECK(v.u == std::floor(v.u));  // ganzzahlig: kein GL_LINEAR-Mischen
        CHECK(v.v == std::floor(v.v));
        summeU += v.u;
        summeV += v.v;
    }
    // Bild nach rechts = Abgriff u −; Bild nach unten (−1,25) = Abgriff v −
    CHECK(summeU == doctest::Approx(-0.3 * 40).epsilon(0.05));
    CHECK(summeV == doctest::Approx(-1.25 * 40).epsilon(0.05));
}

TEST_CASE("Gestaltung: Richtung — 0° rechts, 90° oben")
{
    const auto rechts = polar(1.0, 0.0);
    CHECK(rechts.u == doctest::Approx(1.0));
    CHECK(rechts.v == doctest::Approx(0.0));
    const auto oben = polar(1.0, kPi / 2.0);
    CHECK(oben.u == doctest::Approx(0.0));
    CHECK(oben.v == doctest::Approx(1.0));
}

TEST_CASE("Gestaltung: Weichzeichnen allein verschiebt im Mittel nichts")
{
    GestaltungsLauf lauf;
    double summe = 0.0;
    for (int i = 0; i < 10; ++i)
    {
        const auto v = naechsterVersatz(lauf, 0.0, 0.0, 0.6);
        CHECK(std::abs(v.u) == doctest::Approx(0.3));
        summe += v.u;
    }
    CHECK(summe == doctest::Approx(0.0));
}

TEST_CASE("Gestaltung: Radial-Drift — aussen, Wirbel, innen; Zentrum ruht")
{
    // Punkt rechts vom Zentrum (x rechts, y oben). Abgriff: Bild nach rechts = u −.
    const auto aussen = radialVersatz(110.0, 50.0, 50.0, 50.0, 2.0, 0.0);
    CHECK(aussen.u == doctest::Approx(-2.0));  // Bild wandert nach rechts (weg)
    CHECK(aussen.v == doctest::Approx(0.0));
    const auto wirbel = radialVersatz(110.0, 50.0, 50.0, 50.0, 2.0, kPi / 2.0);
    CHECK(wirbel.u == doctest::Approx(0.0));
    CHECK(wirbel.v == doctest::Approx(2.0));   // ↺: rechts vom Zentrum geht es nach oben
    const auto innen = radialVersatz(110.0, 50.0, 50.0, 50.0, 2.0, kPi);
    CHECK(innen.u == doctest::Approx(2.0));    // Sog: zurueck zum Zentrum
    // Gleiche Geschwindigkeit unabhaengig vom Abstand (anders als zoom)
    const auto nah = radialVersatz(52.0, 50.0, 50.0, 50.0, 2.0, 0.0);
    CHECK(nah.u == doctest::Approx(-2.0));
    const auto mitte = radialVersatz(50.0, 50.0, 50.0, 50.0, 2.0, 0.0);
    CHECK(mitte.u == 0.0);
    CHECK(mitte.v == 0.0);
}

TEST_CASE("Gestaltung: Speichern/Laden, fehlende Felder = aus, Staerken geklemmt")
{
    lumi::milkdrop::PresetState a;
    a.gestaltLinAn = true;
    a.gestaltLinStaerke = 0.7071;
    a.gestaltLinRichtung = 135.0;
    a.gestaltRadAn = true;
    a.gestaltRadStaerke = 1.5;
    a.gestaltRadWinkel = 90.0;
    a.gestaltRadCx = 0.25;
    a.gestaltRadCy = 0.75;
    a.gestaltWeichAn = true;
    a.gestaltWeich = 0.75;
    const auto b = lumi::milkdrop::presetFromJson(lumi::milkdrop::presetToJson(a), nullptr);
    CHECK(b.gestaltLinAn);
    CHECK(b.gestaltLinStaerke == doctest::Approx(0.7071));
    CHECK(b.gestaltLinRichtung == doctest::Approx(135.0));
    CHECK(b.gestaltRadAn);
    CHECK(b.gestaltRadStaerke == doctest::Approx(1.5));
    CHECK(b.gestaltRadWinkel == doctest::Approx(90.0));
    CHECK(b.gestaltRadCx == doctest::Approx(0.25));
    CHECK(b.gestaltRadCy == doctest::Approx(0.75));
    CHECK(b.gestaltWeichAn);
    CHECK(b.gestaltWeich == doctest::Approx(0.75));

    // Altbestand ohne die Felder: original-treu (alles aus)
    const auto alt =
        lumi::milkdrop::presetFromJson(lumi::milkdrop::presetToJson({}), nullptr);
    CHECK_FALSE(alt.gestaltLinAn);
    CHECK_FALSE(alt.gestaltRadAn);
    CHECK_FALSE(alt.gestaltWeichAn);

    lumi::milkdrop::PresetState zuviel;
    zuviel.gestaltLinStaerke = 9.0;
    CHECK(lumi::milkdrop::presetFromJson(lumi::milkdrop::presetToJson(zuviel), nullptr)
              .gestaltLinStaerke == doctest::Approx(4.0));
}
