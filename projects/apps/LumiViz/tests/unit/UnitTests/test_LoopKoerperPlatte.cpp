/**
 ****************************************************************************************
 * @file   test_LoopKoerperPlatte.cpp
 * @brief  Loop, Stufe 31c-3: die Platte des Instrumentkoerpers — Reissner-Mindlin,
 *         orthotrop, Lagrange-Dreiecke der Ordnung k (docs/audio/Spielweisen_Konzept.md §4.1,
 *         Entscheide Patrik S147: BM1 a, EL1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Die Frequenzen der Pruefplatten rechnet hier
 * der dichte Loeser von Eigen — nur im Test, als Werkzeug; der Eigenloeser der Bibliothek
 * kommt mit 31c-4.
 *
 * - Ansatzfunktionen: Zerlegung der Eins, Kronecker an den Knoten, k = 1 … 4.
 * - Integration auf dem Dreieck: Monome genau bis zum Grad 2n − 2.
 * - Werkstoff gedreht: 0°, 90°, isotrop unveraendert.
 * - Navier: frei aufliegende orthotrope Rechteckplatte nach Mindlin, die ersten sechs Moden;
 *   Konvergenz mit k; eine duenne Pruefplatte (Locking, EL1 a: Kipppunkt bei rund 1 %).
 * - Gedreht: dieselbe Platte und dieselbe Faser um 30° gedreht gibt dieselben Frequenzen.
 * - Der Steinway D (grob, bis 1 kHz): K und M lassen sich bauen, die Masse stimmt mit dem
 *   Bauplan. `LUMIVIZ_PLATTE_MESSUNG=1` baut ihn bis 5 kHz mit k = 3 und meldet Groesse und Zeit.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/NetzCdt.hpp"
#include "InstrumentBody/Platte.hpp"
#include "InstrumentBody/Verformungsenergie.hpp"
#include "loop/Koerper.hpp"
#include "loop/KoerperBauplan.hpp"

#include <Eigen/Dense>

#include <QJsonArray>
#include <QJsonObject>
#include <QtGlobal>

#include <algorithm>
#include <chrono>
#include <cmath>
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

kp::Netz rechteckNetz(double a, double b, double kante)
{
    kp::NetzAuftrag auftrag;
    auftrag.umriss = {{0.0, 0.0}, {a, 0.0}, {a, b}, {0.0, b}};
    auftrag.kante = kante;
    return kp::NetzgeberCdt().vernetze(auftrag);
}

/// Die kleinsten Eigenfrequenzen in Hz (dicht; nur fuer kleine Systeme).
std::vector<double> frequenzen(const kp::PlattenSystem& s, int wieviele)
{
    const int n = s.k.n;
    Eigen::MatrixXd k = Eigen::MatrixXd::Zero(n, n), m = Eigen::MatrixXd::Zero(n, n);
    for (int i = 0; i < n; ++i)
    {
        for (std::int64_t e = s.k.start[i]; e < s.k.start[i + 1]; ++e)
        {
            const int j = s.k.spalten[static_cast<std::size_t>(e)];
            k(i, j) = k(j, i) = s.k.werte[static_cast<std::size_t>(e)];
            m(i, j) = m(j, i) = s.m.werte[static_cast<std::size_t>(e)];
        }
    }
    Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::MatrixXd> loeser(k, m, Eigen::EigenvaluesOnly);
    std::vector<double> f;
    for (int i = 0; i < std::min(wieviele, n); ++i) f.push_back(std::sqrt(std::max(0.0, loeser.eigenvalues()(i))) / (2.0 * std::numbers::pi));
    return f;
}

/// Navier, Mindlin, frei aufliegend, Faser laengs x: die kleinste Frequenz je (m, n), sortiert.
std::vector<double> navier(const kp::Material& mat, double a, double b, double h, int wieviele)
{
    const kp::Steife s = kp::gedreht(mat, 0.0);
    const double h3 = h * h * h / 12.0;
    const double d11 = h3 * s.q[0], d12 = h3 * s.q[1], d22 = h3 * s.q[3], d66 = h3 * s.q[5];
    const double sx = kp::kSchubKorrektur * h * s.g[0], sy = kp::kSchubKorrektur * h * s.g[2];
    std::vector<double> f;
    for (int mm = 1; mm <= 10; ++mm)
    {
        for (int nn = 1; nn <= 10; ++nn)
        {
            const double al = mm * std::numbers::pi / a, be = nn * std::numbers::pi / b;
            Eigen::Matrix3d k, m = Eigen::Matrix3d::Zero();
            k << sx * al * al + sy * be * be, -sx * al, -sy * be,  //
                -sx * al, d11 * al * al + d66 * be * be + sx, (d12 + d66) * al * be,  //
                -sy * be, (d12 + d66) * al * be, d66 * al * al + d22 * be * be + sy;
            m(0, 0) = s.rho * h;
            m(1, 1) = m(2, 2) = s.rho * h3;
            Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::Matrix3d> l(k, m, Eigen::EigenvaluesOnly);
            f.push_back(std::sqrt(l.eigenvalues()(0)) / (2.0 * std::numbers::pi));
        }
    }
    std::sort(f.begin(), f.end());
    f.resize(static_cast<std::size_t>(wieviele));
    return f;
}

double groessterFehler(const std::vector<double>& fe, const std::vector<double>& soll)
{
    double e = 0.0;
    for (std::size_t i = 0; i < soll.size(); ++i) e = std::max(e, std::abs(fe[i] / soll[i] - 1.0));
    return e;
}

kp::PlattenSystem gleichePlatte(const kp::Netz& netz, int k, double h, double faserGrad, kp::Auflager auflager)
{
    const kp::Material mat = fichte();
    return kp::baueSystem(netz, kp::LagrangeDreieck(k),
                          [&](kp::Punkt2) {
                              kp::Steife s = kp::gedreht(mat, faserGrad);
                              s.h = h;
                              return s;
                          },
                          auflager);
}

}  // namespace

TEST_CASE("Loop Koerper 31c-3 Ansatz")
{
    for (int k = 1; k <= 4; ++k)
    {
        CAPTURE(k);
        const kp::LagrangeDreieck e(k);
        const auto lage = e.lage();
        REQUIRE(static_cast<int>(lage.size()) == e.knoten());
        CHECK(e.knoten() == (k + 1) * (k + 2) / 2);
        std::vector<double> n, dxi, deta;
        // Kronecker an den Knoten
        for (std::size_t j = 0; j < lage.size(); ++j)
        {
            e.basis(lage[j][0], lage[j][1], n, dxi, deta);
            for (std::size_t i = 0; i < n.size(); ++i) CHECK(n[i] == doctest::Approx(i == j ? 1.0 : 0.0).epsilon(1e-12));
        }
        // Zerlegung der Eins an einem Punkt im Inneren
        e.basis(0.21, 0.37, n, dxi, deta);
        double s = 0.0, sx = 0.0, sy = 0.0;
        for (std::size_t i = 0; i < n.size(); ++i)
        {
            s += n[i];
            sx += dxi[i];
            sy += deta[i];
        }
        CHECK(s == doctest::Approx(1.0).epsilon(1e-12));
        CHECK(std::abs(sx) < 1e-10);
        CHECK(std::abs(sy) < 1e-10);
    }
}

TEST_CASE("Loop Koerper 31c-3 Integration")
{
    // ∫_Dreieck x^a y^b = a! b! / (a + b + 2)!
    const auto fak = [](int z) {
        double f = 1.0;
        for (int i = 2; i <= z; ++i) f *= i;
        return f;
    };
    for (int n = 1; n <= 6; ++n)
    {
        const auto punkte = kp::dreiecksPunkte(n);
        for (int a = 0; a <= 2 * n - 2; ++a)
        {
            for (int b = 0; a + b <= 2 * n - 2; ++b)
            {
                double s = 0.0;
                for (const auto& [x, y, w] : punkte) s += w * std::pow(x, a) * std::pow(y, b);
                CHECK_MESSAGE(s == doctest::Approx(fak(a) * fak(b) / fak(a + b + 2)).epsilon(1e-12), "n ", n, " x^", a, " y^", b);
            }
        }
    }
}

TEST_CASE("Loop Koerper 31c-3 Werkstoff")
{
    const kp::Material m = fichte();
    const double nu21 = m.nuxy * m.ey / m.ex, nenner = 1.0 - m.nuxy * nu21;
    const kp::Steife s0 = kp::gedreht(m, 0.0);
    CHECK(s0.q[0] == doctest::Approx(m.ex / nenner));
    CHECK(s0.q[3] == doctest::Approx(m.ey / nenner));
    CHECK(s0.q[1] == doctest::Approx(m.nuxy * m.ey / nenner));
    CHECK(s0.q[5] == doctest::Approx(m.gxy));
    CHECK(std::abs(s0.q[2]) < 1e-3);
    CHECK(s0.g[0] == doctest::Approx(m.gxz));
    CHECK(s0.g[2] == doctest::Approx(m.gyz));
    // um 90° gedreht: die Achsen tauschen
    const kp::Steife s90 = kp::gedreht(m, 90.0);
    CHECK(s90.q[0] == doctest::Approx(s0.q[3]));
    CHECK(s90.q[3] == doctest::Approx(s0.q[0]));
    CHECK(s90.q[1] == doctest::Approx(s0.q[1]));
    CHECK(s90.g[0] == doctest::Approx(m.gyz));
    CHECK(s90.g[2] == doctest::Approx(m.gxz));
    CHECK(std::abs(s90.q[2]) < 1e-3 * s0.q[0]);
    // isotrop: jede Drehung gleich
    kp::Material glas;
    glas.art = kp::MaterialArt::Isotrop;
    glas.dichte = 2500.0;
    glas.ex = 70.0e9;
    glas.nuxy = 0.22;
    const kp::Steife g0 = kp::gedreht(glas, 0.0), g37 = kp::gedreht(glas, 37.0);
    for (int i = 0; i < 6; ++i) CHECK(g37.q[i] == doctest::Approx(g0.q[i]).epsilon(1e-9).scale(g0.q[0]));
    CHECK(g0.q[5] == doctest::Approx(70.0e9 / (2.0 * 1.22)));
}

/// Eine frei aufliegende Pruefplatte a × b, Dicke h, die ersten `moden` Moden; das Netz nach der
/// Regel von MAESSTRO Gl. 1 (Kante = ein Viertel der Biegewelle quer zur Faser bei der hoechsten
/// dieser Moden). Zurueck je Ordnung k = 1 … 3 der groesste Fehler gegen Navier.
std::vector<double> pruefplatte(double a, double b, double h, int moden)
{
    const auto soll = navier(fichte(), a, b, h, moden);
    const double kante = kp::kuerzesteBiegewelle(fichte(), h, soll.back()) / 4.0;
    const kp::Netz netz = rechteckNetz(a, b, kante);
    REQUIRE(netz.fehler.empty());
    std::vector<double> fehler;
    for (int k = 1; k <= 3; ++k)
    {
        const kp::PlattenSystem s = gleichePlatte(netz, k, h, 0.0, kp::Auflager::GelenkigAchsparallel);
        REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
        CHECK(s.masse == doctest::Approx(380.0 * h * a * b).epsilon(1e-9));
        const auto f = frequenzen(s, moden);
        fehler.push_back(groessterFehler(f, soll));
        MESSAGE(a, " x ", b, " m, h = ", h * 1e3, " mm, Kante ", kante * 1e3, " mm (Dicke/Kante ", h / kante, "), k = ", k, ": ",
                s.k.n, " Groessen, f1 ", f[0], " Hz (Navier ", soll[0], "), f", moden, " ", f.back(), " Hz (", soll.back(),
                "), groesster Fehler ", fehler.back() * 100.0, " %");
    }
    return fehler;
}

TEST_CASE("Loop Koerper 31c-3 Navier")
{
    // 8 mm Fichte, die ersten vier Moden: der Fehler faellt mit der Ordnung, k = 3 trifft auf 0,5 %
    const auto fehler = pruefplatte(0.3, 0.2, 0.008, 4);
    CHECK(fehler[1] < fehler[0]);
    CHECK(fehler[2] < fehler[1]);
    CHECK(fehler[2] < 0.005);
}

TEST_CASE("Loop Koerper 31c-3 Locking")
{
    // EL1 a: an einer duennen Pruefplatte darf k = 3 nicht mehr als rund 1 % zu hoch liegen
    for (const double h : {0.002, 0.001})
    {
        CAPTURE(h);
        const auto fehler = pruefplatte(0.2, 0.15, h, 3);
        CHECK(fehler[2] < 0.01);
    }
}

TEST_CASE("Loop Koerper 31c-3 Drehung")
{
    // dieselbe eingespannte Platte, Netz und Faser um 30° gedreht: dieselben Frequenzen
    const kp::Netz netz = rechteckNetz(0.5, 0.3, 0.1);
    REQUIRE(netz.fehler.empty());
    kp::Netz gedreht = netz;
    const double w = 30.0 * std::numbers::pi / 180.0;
    for (auto& p : gedreht.knoten) p = {p.x * std::cos(w) - p.y * std::sin(w), p.x * std::sin(w) + p.y * std::cos(w)};
    const auto f0 = frequenzen(gleichePlatte(netz, 2, 0.008, 0.0, kp::Auflager::Eingespannt), 8);
    const auto f30 = frequenzen(gleichePlatte(gedreht, 2, 0.008, 30.0, kp::Auflager::Eingespannt), 8);
    for (std::size_t i = 0; i < f0.size(); ++i) CHECK(f30[i] == doctest::Approx(f0[i]).epsilon(1e-8));
    // und mit anders gedrehter Faser nicht
    const auto f60 = frequenzen(gleichePlatte(netz, 2, 0.008, 60.0, kp::Auflager::Eingespannt), 8);
    CHECK(std::abs(f60[0] / f0[0] - 1.0) > 0.01);
}

namespace
{

/// Lévy, Kirchhoff, orthotrop (Faser laengs x): Rechteck a × b, alle Raender in w gehalten, an x = 0 und x = a
/// eine Drehfeder kd (N) gegen die Neigung quer zum Rand, die Raender y = 0 und y = b gelenkig. Die tiefste
/// Mode (symmetrisch in x, eine Halbwelle in y): W = A cosh αx + C cos βx um die Mitte, am Rand W = 0 und
/// D11 W'' = −kd W'. Gesucht ist β zwischen gelenkig (π / a) und 2π / a.
double levyMitDrehfeder(const kp::Material& mat, double a, double b, double h, double kd)
{
    const kp::Steife s = kp::gedreht(mat, 0.0);
    const double h3 = h * h * h / 12.0;
    const double d11 = h3 * s.q[0], d12 = h3 * s.q[1], d22 = h3 * s.q[3], d66 = h3 * s.q[5];
    const double hh = d12 + 2.0 * d66, ny = std::numbers::pi / b;
    const auto f = [&](double beta) {
        const double alpha = std::sqrt(2.0 * hh * ny * ny / d11 + beta * beta);
        const double c = std::cos(beta * a / 2.0), si = std::sin(beta * a / 2.0);
        return d11 * c * (alpha * alpha + beta * beta) + kd * (c * alpha * std::tanh(alpha * a / 2.0) + beta * si);
    };
    double unten = std::numbers::pi / a, oben = 2.0 * std::numbers::pi / a;
    if (kd <= 0.0) oben = unten;
    for (int i = 0; i < 200 && kd > 0.0; ++i)
    {
        const double mitte = 0.5 * (unten + oben);
        (f(mitte) > 0.0 ? unten : oben) = mitte;
    }
    const double beta = 0.5 * (unten + oben);
    const double omega2 = (d11 * std::pow(beta, 4) + 2.0 * hh * ny * ny * beta * beta + d22 * std::pow(ny, 4)) / (s.rho * h);
    return std::sqrt(omega2) / (2.0 * std::numbers::pi);
}

/// Lévy: Platte a × b, an x = 0 und an beiden y-Raendern gelenkig, die Kante x = a frei (KQ20 a). Erste Mode
/// (eine Halbwelle in y): W = A sinh αx + B sin βx; an x = a kein Moment und keine Ersatzquerkraft.
double levyEineKanteFrei(const kp::Material& mat, double a, double b, double h)
{
    const kp::Steife s = kp::gedreht(mat, 0.0);
    const double h3 = h * h * h / 12.0;
    const double d11 = h3 * s.q[0], d12 = h3 * s.q[1], d22 = h3 * s.q[3], d66 = h3 * s.q[5];
    const double hh = d12 + 2.0 * d66, k2 = std::pow(std::numbers::pi / b, 2), m = s.rho * h;
    const auto f = [&](double omega2) {
        const double wurzel = std::sqrt(hh * hh * k2 * k2 + d11 * (m * omega2 - d22 * k2 * k2));
        const double alpha = std::sqrt((hh * k2 + wurzel) / d11), beta = std::sqrt((wurzel - hh * k2) / d11);
        const double ms = (d11 * alpha * alpha - d12 * k2) * std::sinh(alpha * a);
        const double vs = alpha * (d11 * alpha * alpha - (d12 + 4.0 * d66) * k2) * std::cosh(alpha * a);
        const double mn = -(d11 * beta * beta + d12 * k2) * std::sin(beta * a);
        const double vn = -beta * (d11 * beta * beta + (d12 + 4.0 * d66) * k2) * std::cos(beta * a);
        return ms * vn - mn * vs;
    };
    const double start = d22 * k2 * k2 / m;  // der Streifen laengs y: darueber liegt die Mode
    double unten = start * (1.0 + 1e-9), oben = unten;
    const double vorzeichen = f(unten);
    for (int i = 0; i < 100000; ++i)
    {
        oben = unten * 1.0005;
        if (f(oben) * vorzeichen < 0.0) break;
        unten = oben;
    }
    for (int i = 0; i < 100; ++i)
    {
        const double mitte = 0.5 * (unten + oben);
        (f(mitte) * vorzeichen > 0.0 ? unten : oben) = mitte;
    }
    return std::sqrt(0.5 * (unten + oben)) / (2.0 * std::numbers::pi);
}

}  // namespace

TEST_CASE("Loop Koerper KQ18 Drehfeder am Rand")
{
    // Gauss auf der Strecke: Gewichte 1, Monome genau bis zum Grad 2n − 1
    for (const int n : {2, 5})
    {
        const auto punkte = kp::gaussAufStrecke(n);
        REQUIRE(punkte.size() == static_cast<std::size_t>(n));
        for (int grad = 0; grad <= 2 * n - 1; ++grad)
        {
            double summe = 0.0;
            for (const auto& p : punkte) summe += p[1] * std::pow(p[0], grad);
            CHECK(summe == doctest::Approx(1.0 / (grad + 1.0)).epsilon(1e-12));
        }
    }

    const double a = 0.3, b = 0.4, h = 0.003;
    const kp::Material mat = fichte();
    const kp::Netz netz = rechteckNetz(a, b, 0.04);
    REQUIRE(netz.fehler.empty());
    const double d11 = h * h * h / 12.0 * kp::gedreht(mat, 0.0).q[0];
    const auto platte = [&](const kp::Netz& n, double faserGrad, kp::Auflager auflager, const std::function<double(kp::Punkt2)>& feder) {
        return kp::baueSystem(n, kp::LagrangeDreieck(3),
                              [&, faserGrad](kp::Punkt2) {
                                  kp::Steife s = kp::gedreht(mat, faserGrad);
                                  s.h = h;
                                  return s;
                              },
                              auflager, feder);
    };
    const auto anXRaendern = [&](double kd) {
        return [=](kp::Punkt2 p) { return (p.x < 1e-9 || p.x > a - 1e-9) ? kd : 0.0; };
    };

    // ohne Feder und mit Feder 0: der gestuetzte Rand, unveraendert
    const double gestuetzt = frequenzen(platte(netz, 0.0, kp::Auflager::Gestuetzt, {}), 1)[0];
    CHECK(frequenzen(platte(netz, 0.0, kp::Auflager::Gestuetzt, anXRaendern(0.0)), 1)[0] == gestuetzt);
    CHECK(gestuetzt == doctest::Approx(levyMitDrehfeder(mat, a, b, h, 0.0)).epsilon(0.01));
    // am eingespannten Rand wirkt sie nicht (die Drehungen sind dort gehalten)
    const double eingespannt = frequenzen(platte(netz, 0.0, kp::Auflager::Eingespannt, {}), 1)[0];
    CHECK(frequenzen(platte(netz, 0.0, kp::Auflager::Eingespannt, [](kp::Punkt2) { return 1.0e4; }), 1)[0] == eingespannt);

    // gegen Lévy: zwei Federn zwischen gelenkig und eingespannt, und die Frequenz steigt mit der Feder
    double vorher = gestuetzt;
    for (const double faktor : {1.0, 10.0, 100.0})
    {
        CAPTURE(faktor);
        const double kd = faktor * d11 / a;
        const double fe = frequenzen(platte(netz, 0.0, kp::Auflager::Gestuetzt, anXRaendern(kd)), 1)[0];
        const double soll = levyMitDrehfeder(mat, a, b, h, kd);
        CHECK(fe == doctest::Approx(soll).epsilon(0.01));
        CHECK(fe > vorher * 1.02);
        vorher = fe;
    }
    // eine sehr harte Feder am ganzen Rand naehert sich dem eingespannten Rand
    const double hart = frequenzen(platte(netz, 0.0, kp::Auflager::Gestuetzt, [&](kp::Punkt2) { return 1.0e6 * d11 / a; }), 1)[0];
    CHECK(hart == doctest::Approx(eingespannt).epsilon(0.01));
    CHECK(hart <= eingespannt * (1.0 + 1e-9));

    // die Feder haengt an der Randnormalen: Netz und Faser um 30° gedreht geben dieselben Frequenzen
    kp::Netz gedreht = netz;
    const double w = 30.0 * std::numbers::pi / 180.0;
    for (auto& p : gedreht.knoten) p = {p.x * std::cos(w) - p.y * std::sin(w), p.x * std::sin(w) + p.y * std::cos(w)};
    const auto gleich = [&](kp::Punkt2) { return 3.0 * d11 / a; };
    const auto f0 = frequenzen(platte(netz, 0.0, kp::Auflager::Gestuetzt, gleich), 6);
    const auto f30 = frequenzen(platte(gedreht, 30.0, kp::Auflager::Gestuetzt, gleich), 6);
    for (std::size_t i = 0; i < f0.size(); ++i) CHECK(f30[i] == doctest::Approx(f0[i]).epsilon(1e-8));
}

TEST_CASE("Loop Koerper KQ18 Rand im Bauplan")
{
    const double a = 0.3, b = 0.4, h = 0.003;
    kp::Bauplan plan;
    plan.titel = "Test";
    plan.materialien = {fichte()};
    plan.platte.material = "spruce";
    plan.platte.umriss = {{0.0, 0.0}, {a, 0.0}, {a, b}, {0.0, b}};
    plan.platte.dicke = h;
    const kp::Netz netz = rechteckNetz(a, b, 0.04);
    REQUIRE(netz.fehler.empty());
    const kp::LagrangeDreieck element(3);
    const auto steife = [&](kp::Punkt2 p) { return kp::steifeBei(plan, p); };

    // die Vorgabe: eingespannt, wie bis Fassung 5
    CHECK_FALSE(plan.rand.elastisch());
    const double eingespannt = frequenzen(kp::baueSystem(netz, element, steife, kp::Auflager::Eingespannt), 1)[0];
    CHECK(frequenzen(kp::baueSystem(netz, element, plan), 1)[0] == eingespannt);
    // elastisch ohne Feder: gestuetzt; mit Feder: derselbe Wert am ganzen Rand
    plan.rand.halt = kp::Rand::Halt::Elastisch;
    const double gestuetzt = frequenzen(kp::baueSystem(netz, element, steife, kp::Auflager::Gestuetzt), 1)[0];
    CHECK(frequenzen(kp::baueSystem(netz, element, plan), 1)[0] == gestuetzt);
    plan.rand.drehfeder = 40.0;
    plan.rand.quelle = "Test";
    const double mitFeder = frequenzen(kp::baueSystem(netz, element, plan), 1)[0];
    CHECK(mitFeder == frequenzen(kp::baueSystem(netz, element, steife, kp::Auflager::Gestuetzt, [](kp::Punkt2) { return 40.0; }), 1)[0]);
    CHECK(mitFeder > gestuetzt * 1.05);
    CHECK(mitFeder < eingespannt * 0.95);

    // die Datei: `rim` immer geschrieben (Fassung 6), Hin- und Rueckweg; Fassung 5 ohne den Block: eingespannt
    const QJsonObject json = alsJson(plan);
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kBauplanFassung);
    const QJsonObject r = json.value(QStringLiteral("rim")).toObject();
    for (const char* feld : {"support", "rotationalStiffness", "source"}) CHECK_MESSAGE(r.contains(QLatin1String(feld)), feld);
    CHECK(r.value(QStringLiteral("support")).toString() == QStringLiteral("elastic"));
    CHECK(r.value(QStringLiteral("rotationalStiffness")).toDouble() == 40.0);
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == plan);
    CHECK(alsJson(kp::Bauplan{}).value(QStringLiteral("rim")).toObject().value(QStringLiteral("support")).toString() ==
          QStringLiteral("clamped"));
    QJsonObject fuenf = json;
    fuenf.remove(QStringLiteral("rim"));
    fuenf[QStringLiteral("version")] = 5;
    kp::Bauplan alt;
    REQUIRE(ausJson(fuenf, alt));
    CHECK(alt.rand == kp::Rand{});
    // ein unbekannter Halt wird nicht geraten
    QJsonObject fremd = json;
    QJsonObject rFremd = r;
    rFremd[QStringLiteral("support")] = QStringLiteral("hinged");
    fremd[QStringLiteral("rim")] = rFremd;
    CHECK_FALSE(ausJson(fremd, alt));
    // der Abdruck haengt am Rand
    kp::Bauplan haerter = plan;
    haerter.rand.drehfeder = 41.0;
    CHECK(bauplanAbdruck(haerter) != bauplanAbdruck(plan));

    // die Pruefung
    const auto befundMit = [](const kp::Bauplan& p, const char* stueck) {
        for (const std::string& bf : kp::pruefe(p))
            if (bf.find(stueck) != std::string::npos) return true;
        return false;
    };
    CHECK_FALSE(befundMit(plan, "rim:"));
    kp::Bauplan falsch = plan;
    falsch.rand.drehfeder = -1.0;
    CHECK(befundMit(falsch, "rim: rotational stiffness must not"));
    falsch = plan;
    falsch.rand.halt = kp::Rand::Halt::Eingespannt;
    CHECK(befundMit(falsch, "rim: a rotational stiffness needs"));
}

TEST_CASE("Loop Koerper KQ20 Randabschnitte")
{
    const double a = 0.3, b = 0.4, h = 0.003;
    const kp::Material mat = fichte();
    const kp::Netz netz = rechteckNetz(a, b, 0.04);
    REQUIRE(netz.fehler.empty());
    const double d11 = h * h * h / 12.0 * kp::gedreht(mat, 0.0).q[0];
    const auto platte = [&](kp::Auflager auflager, const std::function<std::optional<kp::RandFedern>(kp::Punkt2)>& abschnitt) {
        return kp::baueSystem(netz, kp::LagrangeDreieck(3),
                              [&](kp::Punkt2) {
                                  kp::Steife s = kp::gedreht(mat, 0.0);
                                  s.h = h;
                                  return s;
                              },
                              auflager, {}, abschnitt);
    };
    const auto kanteRechts = [&](double weg, double dreh) {
        return [=](kp::Punkt2 mitte) -> std::optional<kp::RandFedern> {
            if (mitte.x > a - 1e-9) return kp::RandFedern{weg, dreh};
            return std::nullopt;
        };
    };
    const double gestuetzt = frequenzen(platte(kp::Auflager::Gestuetzt, {}), 1)[0];
    const double eingespannt = frequenzen(platte(kp::Auflager::Eingespannt, {}), 1)[0];

    // ein Abschnitt, der keine Kante trifft: nichts aendert sich
    CHECK(frequenzen(platte(kp::Auflager::Gestuetzt, [](kp::Punkt2) -> std::optional<kp::RandFedern> { return std::nullopt; }), 1)[0] ==
          gestuetzt);
    // die Kante x = a frei: gegen Lévy auf 1 %
    const double frei = frequenzen(platte(kp::Auflager::Gestuetzt, kanteRechts(0.0, 0.0)), 1)[0];
    CHECK(frei == doctest::Approx(levyEineKanteFrei(mat, a, b, h)).epsilon(0.01));
    CHECK(frei < gestuetzt * 0.8);
    // die Frequenz steigt mit der Wegfeder; eine sehr harte naehert sich dem gestuetzten Rand von unten
    double vorher = frei;
    for (const double weg : {1.0e3, 1.0e4, 1.0e5})
    {
        CAPTURE(weg);
        const double f = frequenzen(platte(kp::Auflager::Gestuetzt, kanteRechts(weg, 0.0)), 1)[0];
        CHECK(f > vorher * 1.01);
        CHECK(f < gestuetzt);
        vorher = f;
    }
    const double hart = frequenzen(platte(kp::Auflager::Gestuetzt, kanteRechts(1.0e12, 0.0)), 1)[0];
    CHECK(hart == doctest::Approx(gestuetzt).epsilon(1e-3));
    CHECK(hart <= gestuetzt * (1.0 + 1e-9));
    // harte Weg- und Drehfeder am Abschnitt eines eingespannten Rands: der eingespannte Rand
    const double hartBeides = frequenzen(platte(kp::Auflager::Eingespannt, kanteRechts(1.0e12, 1.0e6 * d11 / a)), 1)[0];
    CHECK(hartBeides == doctest::Approx(eingespannt).epsilon(0.01));
    CHECK(hartBeides <= eingespannt * (1.0 + 1e-9));
    // die Drehfeder des Abschnitts wirkt: mit harter Wegfeder wie die Drehfeder an dieser Kante (KQ18 a)
    const double kd = 10.0 * d11 / a;
    const double mitDreh = frequenzen(platte(kp::Auflager::Gestuetzt, kanteRechts(1.0e12, kd)), 1)[0];
    const double bezug = frequenzen(kp::baueSystem(netz, kp::LagrangeDreieck(3),
                                                   [&](kp::Punkt2) {
                                                       kp::Steife s = kp::gedreht(mat, 0.0);
                                                       s.h = h;
                                                       return s;
                                                   },
                                                   kp::Auflager::Gestuetzt, [&](kp::Punkt2 p) { return p.x > a - 1e-9 ? kd : 0.0; }),
                                    1)[0];
    CHECK(mitDreh == doctest::Approx(bezug).epsilon(1e-3));
    CHECK(mitDreh > gestuetzt * 1.01);

    // im Bauplan: die Kante 1 des Umrisses ((a,0) → (a,b)) als Abschnitt
    kp::Bauplan plan;
    plan.titel = "Test";
    plan.materialien = {mat};
    plan.platte.material = "spruce";
    plan.platte.umriss = {{0.0, 0.0}, {a, 0.0}, {a, b}, {0.0, b}};
    plan.platte.dicke = h;
    plan.rand.halt = kp::Rand::Halt::Elastisch;
    const kp::LagrangeDreieck element(3);
    CHECK(kp::naechsteUmrissKante(plan.platte.umriss, {a, 0.1}) == 1);
    CHECK(kp::naechsteUmrissKante(plan.platte.umriss, {0.1, b}) == 2);
    CHECK(kp::naechsteUmrissKante({}, {0.0, 0.0}) == -1);
    CHECK(frequenzen(kp::baueSystem(netz, element, plan), 1)[0] == gestuetzt);
    plan.rand.abschnitte = {{"right edge", 1, 1, 0.0, 0.0, "Test"}};
    CHECK(frequenzen(kp::baueSystem(netz, element, plan), 1)[0] == frei);
    plan.rand.abschnitte[0].wegfeder = 1.0e4;
    plan.rand.abschnitte[0].drehfeder = 2.0;
    CHECK(frequenzen(kp::baueSystem(netz, element, plan), 1)[0] ==
          frequenzen(platte(kp::Auflager::Gestuetzt, kanteRechts(1.0e4, 2.0)), 1)[0]);
    CHECK(kp::pruefe(plan).empty());

    // die Datei: `rim.sections` immer geschrieben (Fassung 7), Hin- und Rueckweg; Fassung 6 ohne die Liste
    const QJsonObject json = alsJson(plan);
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kBauplanFassung);
    CHECK(kp::kBauplanFassung >= 7);
    const QJsonObject r = json.value(QStringLiteral("rim")).toObject();
    REQUIRE(r.value(QStringLiteral("sections")).toArray().size() == 1);
    const QJsonObject ab = r.value(QStringLiteral("sections")).toArray().at(0).toObject();
    for (const char* feld : {"name", "edges", "translationalStiffness", "rotationalStiffness", "source"})
        CHECK_MESSAGE(ab.contains(QLatin1String(feld)), feld);
    CHECK(ab.value(QStringLiteral("translationalStiffness")).toDouble() == 1.0e4);
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == plan);
    CHECK(alsJson(kp::Bauplan{}).value(QStringLiteral("rim")).toObject().contains(QStringLiteral("sections")));
    QJsonObject sechs = json;
    QJsonObject rSechs = r;
    rSechs.remove(QStringLiteral("sections"));
    sechs[QStringLiteral("rim")] = rSechs;
    sechs[QStringLiteral("version")] = 6;
    kp::Bauplan alt;
    REQUIRE(ausJson(sechs, alt));
    CHECK(alt.rand.abschnitte.empty());
    CHECK(alt.rand.elastisch());
    // ein Abschnitt ohne zwei Kanten wird nicht geraten
    QJsonObject kaputt = json;
    QJsonObject rKaputt = r;
    QJsonObject abKaputt = ab;
    QJsonArray eine;
    eine.append(1);
    abKaputt[QStringLiteral("edges")] = eine;
    QJsonArray liste;
    liste.append(abKaputt);
    rKaputt[QStringLiteral("sections")] = liste;
    kaputt[QStringLiteral("rim")] = rKaputt;
    CHECK_FALSE(ausJson(kaputt, alt));
    // der Abdruck haengt an den Abschnitten
    kp::Bauplan weicher = plan;
    weicher.rand.abschnitte[0].wegfeder = 0.9e4;
    CHECK(bauplanAbdruck(weicher) != bauplanAbdruck(plan));

    // die Pruefung
    const auto befundMit = [](const kp::Bauplan& p, const char* stueck) {
        for (const std::string& bf : kp::pruefe(p))
            if (bf.find(stueck) != std::string::npos) return true;
        return false;
    };
    kp::Bauplan falsch = plan;
    falsch.rand.abschnitte[0].wegfeder = -1.0;
    CHECK(befundMit(falsch, "stiffness must not be negative"));
    falsch = plan;
    falsch.rand.abschnitte[0].kanteBis = 4;
    CHECK(befundMit(falsch, "edges outside the outline"));
    falsch = plan;
    falsch.rand.abschnitte.push_back({"again", 1, 2, 0.0, 0.0, ""});
    CHECK(befundMit(falsch, "overlaps another section"));
    falsch = plan;
    falsch.rand.abschnitte[0].kanteVon = 0;
    falsch.rand.abschnitte[0].kanteBis = 3;
    CHECK(befundMit(falsch, "must leave a part of the rim held"));
}

TEST_CASE("Loop Koerper 31c-3 Steinway D")
{
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(QStringLiteral("asset/loop/instrument_bodies/steinway_d") + QLatin1String(kBauplanEndung), plan),
                    "die Test-Exe aus dem Projekt-Root starten");
    const bool messung = qEnvironmentVariableIntValue("LUMIVIZ_PLATTE_MESSUNG") == 1;
    const double fmax = messung ? 5000.0 : 1000.0;
    const int k = messung ? 3 : 2;
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(kp::netzAuftrag(plan, fmax));
    REQUIRE(netz.fehler.empty());
    const auto start = std::chrono::steady_clock::now();
    const kp::PlattenSystem s =
        kp::baueSystem(netz, kp::LagrangeDreieck(k), plan);
    const double sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
    MESSAGE("Steinway D bis ", fmax, " Hz, k = ", k, ": ", netz.dreiecke.size(), " Dreiecke, ", s.knoten.size(), " Knoten, ", s.k.n,
            " freie Groessen, ", s.k.eintraege(), " Eintraege je Matrix (oberes Dreieck), Masse ", s.masse, " kg, ", sekunden, " s");
    CHECK(s.k.n > 0);
    // die Masse der Platte nach den Zonen gegen die des Bauplans (Platte, Rippen, Steg je fuer sich):
    // unter dem Steg rechnet die Zone Buche statt Fichte fuer die Plattendicke — wenige Prozent
    CHECK(s.masse == doctest::Approx(kp::masse(plan)).epsilon(0.03));
    // Diagonalen positiv
    int nichtPositiv = 0;
    for (int i = 0; i < s.k.n; ++i)
        if (s.k.wert(i, i) <= 0.0 || s.m.wert(i, i) <= 0.0) ++nichtPositiv;
    CHECK(nichtPositiv == 0);
}

TEST_CASE("Loop Koerper KQ31-1 Leisten uebereinander")
{
    const double a = 0.3, b = 0.4, h = 0.003;
    kp::Material buche = fichte();
    buche.name = "beech";
    buche.dichte = 700.0;
    buche.ex = 14.0e9;
    kp::Bauplan plan;
    plan.titel = "Test";
    plan.materialien = {fichte(), buche};
    plan.platte.material = "spruce";
    plan.platte.umriss = {{0.0, 0.0}, {a, 0.0}, {a, b}, {0.0, b}};
    plan.platte.dicke = h;
    // Deckleiste (Steg, 5 mm hoch, 20 mm breit) und Zugleiste (Rippe, 20 mm hoch, 40 mm breit) auf derselben Linie
    const std::vector<kp::Punkt2> linie = {{0.15, 0.05}, {0.15, 0.35}};
    plan.stege = {{"cover bar", "beech", linie, {0.005, 0.005}, {0.02, 0.02}}};
    plan.rippen = {{"straining bar", "beech", linie, {0.02, 0.02}, {0.04, 0.04}}};
    REQUIRE(kp::pruefe(plan).empty());
    const kp::Punkt2 mitte{0.15, 0.2}, daneben{0.165, 0.2}, frei{0.05, 0.2};

    // die Vorgabe: nur die erste zaehlt (der Steg geht vor) — wie bis Fassung 7
    CHECK(plan.leistenUebereinander == kp::LeistenUebereinander::NurDieErste);
    CHECK(kp::steifeBei(plan, mitte).h == doctest::Approx(h + 0.005).epsilon(1e-12));
    CHECK(kp::steifeBei(plan, daneben).h == doctest::Approx(h + 0.02).epsilon(1e-12));
    CHECK(kp::steifeBei(plan, frei).h == h);
    CHECK(kp::steifeBei(plan, mitte).werkstoff == 1);

    // addieren: die Hoehen zaehlen zusammen, Werkstoff und Richtung der ersten; daneben und frei wie vorher
    kp::Bauplan summe = plan;
    summe.leistenUebereinander = kp::LeistenUebereinander::Addieren;
    CHECK(kp::steifeBei(summe, mitte).h == doctest::Approx(h + 0.025).epsilon(1e-12));
    CHECK(kp::steifeBei(summe, mitte).werkstoff == 1);
    CHECK(kp::steifeBei(summe, mitte).q == kp::steifeBei(plan, mitte).q);
    CHECK(kp::steifeBei(summe, daneben).h == kp::steifeBei(plan, daneben).h);
    CHECK(kp::steifeBei(summe, frei).h == h);
    // ohne Ueberdeckung aendert das Feld nichts
    kp::Bauplan einzeln = plan;
    einzeln.rippen.clear();
    kp::Bauplan einzelnSumme = einzeln;
    einzelnSumme.leistenUebereinander = kp::LeistenUebereinander::Addieren;
    for (const kp::Punkt2 p : {mitte, daneben, frei}) CHECK(kp::steifeBei(einzelnSumme, p).h == kp::steifeBei(einzeln, p).h);

    // die Rechnung: mit „addieren“ dieselbe Platte wie ein Bauplan, dessen Deckleiste die Summe traegt
    // (so die Probe der S205); schwerer und steifer als mit „nur die erste“
    const kp::Netz netz = rechteckNetz(a, b, 0.04);  // grob: die dichte Eigenwertaufgabe bleibt klein
    REQUIRE(netz.fehler.empty());
    const kp::LagrangeDreieck element(3);
    kp::Bauplan vonHand = plan;
    vonHand.stege[0].hoehe = {0.025, 0.025};
    const kp::PlattenSystem sErste = kp::baueSystem(netz, element, plan);
    const kp::PlattenSystem sSumme = kp::baueSystem(netz, element, summe);
    const kp::PlattenSystem sHand = kp::baueSystem(netz, element, vonHand);
    CHECK(sSumme.masse == doctest::Approx(sHand.masse).epsilon(1e-12));
    CHECK(sSumme.masse > sErste.masse);
    const auto fErste = frequenzen(sErste, 3), fSumme = frequenzen(sSumme, 3), fHand = frequenzen(sHand, 3);
    for (std::size_t i = 0; i < fSumme.size(); ++i) CHECK(fSumme[i] == doctest::Approx(fHand[i]).epsilon(1e-9));
    CHECK(std::abs(fSumme[0] - fErste[0]) > 0.01 * fErste[0]);

    // die Meldung: was liegt uebereinander
    const auto paare = kp::ueberlagerungen(plan);
    REQUIRE(paare.size() == 1);
    CHECK(paare[0].oben == "cover bar");
    CHECK(paare[0].unten == "straining bar");
    CHECK(paare[0].gleicherWerkstoff);
    CHECK(paare[0].flaeche == doctest::Approx(0.3 * 0.02).epsilon(0.02));
    CHECK(paare[0].volumen == doctest::Approx(0.3 * 0.02 * 0.02).epsilon(0.02));
    CHECK(kp::ueberlagerungen(einzeln).empty());
    kp::Bauplan gemischt = plan;
    gemischt.rippen[0].material = "spruce";
    CHECK_FALSE(kp::ueberlagerungen(gemischt).at(0).gleicherWerkstoff);
    // eine dritte Leiste unter den beiden zaehlt gegen die erste, nicht gegen die zweite
    kp::Bauplan drei = plan;
    drei.rippen.push_back({"third", "beech", linie, {0.01, 0.01}, {0.01, 0.01}});
    const auto paareDrei = kp::ueberlagerungen(drei);
    REQUIRE(paareDrei.size() == 2);
    CHECK(paareDrei[1].oben == "cover bar");
    CHECK(paareDrei[1].unten == "third");
    drei.leistenUebereinander = kp::LeistenUebereinander::Addieren;
    CHECK(kp::steifeBei(drei, mitte).h == doctest::Approx(h + 0.035).epsilon(1e-12));

    // die Datei: `barOverlap` immer geschrieben (Fassung 8), Hin- und Rueckweg; Fassung 7 ohne das Feld
    CHECK(kp::kBauplanFassung >= 8);
    CHECK(alsJson(plan).value(QStringLiteral("barOverlap")).toString() == QStringLiteral("first"));
    const QJsonObject json = alsJson(summe);
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kBauplanFassung);
    CHECK(json.value(QStringLiteral("barOverlap")).toString() == QStringLiteral("add"));
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == summe);
    QJsonObject sieben = json;
    sieben.remove(QStringLiteral("barOverlap"));
    sieben[QStringLiteral("version")] = 7;
    kp::Bauplan alt;
    REQUIRE(ausJson(sieben, alt));
    CHECK(alt.leistenUebereinander == kp::LeistenUebereinander::NurDieErste);
    // eine unbekannte Art wird nicht geraten
    QJsonObject fremd = json;
    fremd[QStringLiteral("barOverlap")] = QStringLiteral("max");
    CHECK_FALSE(ausJson(fremd, alt));
    // der Abdruck haengt am Feld
    CHECK(bauplanAbdruck(summe) != bauplanAbdruck(plan));
}

namespace
{

/// K und M dicht, und alle Eigenpaare (Formen bezueglich M normiert, aufsteigend).
struct DichteModen
{
    Eigen::VectorXd lambda;
    Eigen::MatrixXd formen;
};

DichteModen dichteModen(const kp::PlattenSystem& s)
{
    const int n = s.k.n;
    Eigen::MatrixXd k = Eigen::MatrixXd::Zero(n, n), m = Eigen::MatrixXd::Zero(n, n);
    for (int i = 0; i < n; ++i)
    {
        for (std::int64_t e = s.k.start[i]; e < s.k.start[i + 1]; ++e)
        {
            const int j = s.k.spalten[static_cast<std::size_t>(e)];
            k(i, j) = k(j, i) = s.k.werte[static_cast<std::size_t>(e)];
            m(i, j) = m(j, i) = s.m.werte[static_cast<std::size_t>(e)];
        }
    }
    Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::MatrixXd> loeser(k, m);
    return {loeser.eigenvalues(), loeser.eigenvectors()};
}

/// Navier, Mindlin, frei aufliegend, Faser laengs x: je (m, n) bis `bis` alle drei Aeste als (λ, φ(p)²) — φ der Wert
/// der bezueglich der Masse normierten Form am Punkt `p`.
std::vector<std::array<double, 2>> navierAmPunkt(const kp::Material& mat, double a, double b, double h, kp::Punkt2 p, int bis)
{
    const kp::Steife s = kp::gedreht(mat, 0.0);
    const double h3 = h * h * h / 12.0;
    const double d11 = h3 * s.q[0], d12 = h3 * s.q[1], d22 = h3 * s.q[3], d66 = h3 * s.q[5];
    const double sx = kp::kSchubKorrektur * h * s.g[0], sy = kp::kSchubKorrektur * h * s.g[2];
    std::vector<std::array<double, 2>> aus;
    for (int mm = 1; mm <= bis; ++mm)
    {
        for (int nn = 1; nn <= bis; ++nn)
        {
            const double al = mm * std::numbers::pi / a, be = nn * std::numbers::pi / b;
            Eigen::Matrix3d k, m = Eigen::Matrix3d::Zero();
            k << sx * al * al + sy * be * be, -sx * al, -sy * be,  //
                -sx * al, d11 * al * al + d66 * be * be + sx, (d12 + d66) * al * be,  //
                -sy * be, (d12 + d66) * al * be, d66 * al * al + d22 * be * be + sy;
            m(0, 0) = s.rho * h;
            m(1, 1) = m(2, 2) = s.rho * h3;
            Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::Matrix3d> l(k, m);
            const double ort = std::sin(al * p.x) * std::sin(be * p.y);
            // vᵀ M v = 1 je Flaeche; ueber die Platte integriert sin² · sin² = a b / 4
            for (int ast = 0; ast < 3; ++ast)
                aus.push_back({l.eigenvalues()(ast), l.eigenvectors()(0, ast) * l.eigenvectors()(0, ast) * ort * ort * 4.0 / (a * b)});
        }
    }
    std::sort(aus.begin(), aus.end());
    return aus;
}

/// Die Eigenwerte λ mit einer Feder `steife` am Punkt: die Nullstellen von 1 + k Σ φ_r² / (λ_r − λ), je eine
/// zwischen zwei Polen; die ersten `wieviele`.
std::vector<double> mitFederNachReihe(const std::vector<std::array<double, 2>>& reihe, double steife, int wieviele)
{
    const auto g = [&](double lambda) {
        double summe = 0.0;
        for (const auto& [l, phi2] : reihe) summe += phi2 / (l - lambda);
        return 1.0 + steife * summe;
    };
    std::vector<double> aus;
    for (int r = 0; r < wieviele; ++r)
    {
        double unten = reihe[static_cast<std::size_t>(r)][0], oben = reihe[static_cast<std::size_t>(r) + 1][0];
        const double spanne = oben - unten;
        unten += 1e-12 * spanne;
        oben -= 1e-12 * spanne;
        for (int schritt = 0; schritt < 200; ++schritt)
        {
            const double mitte = 0.5 * (unten + oben);
            (g(mitte) < 0.0 ? unten : oben) = mitte;  // g steigt von −∞ nach +∞ zwischen zwei Polen
        }
        aus.push_back(0.5 * (unten + oben));
    }
    return aus;
}

}  // namespace

TEST_CASE("Loop Koerper KQ46 Punktfedern")
{
    const double a = 0.5, b = 0.4, h = 0.003;
    const kp::Material mat = fichte();
    const kp::Punkt2 ort{0.37 * a, 0.41 * b};  // kein Knoten einer Mode, kein Knoten des Netzes
    const auto steife = [&](kp::Punkt2) {
        kp::Steife s = kp::gedreht(mat, 0.0);
        s.h = h;
        s.werkstoff = 0;
        return s;
    };
    const kp::LagrangeDreieck element(3);
    const auto hz = [](double lambda) { return std::sqrt(std::max(0.0, lambda)) / (2.0 * std::numbers::pi); };

    // --- gegen die Reihe nach Navier: frei aufliegende Platte, eine Feder am Punkt ---------------------
    const auto reihe = navierAmPunkt(mat, a, b, h, ort, 40);
    const double f1 = hz(reihe[0][0]);
    // eine Feder, die die erste Mode deutlich hebt: k φ₁² ≈ 0,5 λ₁
    const double feder = 0.5 * reihe[0][0] / reihe[0][1];
    const auto soll = mitFederNachReihe(reihe, feder, 2);
    CHECK(hz(soll[0]) > 1.1 * f1);
    // die Reihe haengt kaum an ihrer Laenge (die Punktnachgiebigkeit der schubweichen Platte waechst nur langsam)
    const auto sollKurz = mitFederNachReihe(navierAmPunkt(mat, a, b, h, ort, 20), feder, 2);
    CHECK(sollKurz[0] == doctest::Approx(soll[0]).epsilon(0.005));

    const kp::Netz netz = rechteckNetz(a, b, 0.07);  // grob: die dichten Eigenwertaufgaben bleiben klein
    REQUIRE(netz.fehler.empty());
    const kp::PlattenSystem ohne = kp::baueSystem(netz, element, steife, kp::Auflager::GelenkigAchsparallel);
    REQUIRE(ohne.fehler.empty());
    kp::PlattenSystem mit = ohne;
    kp::setzePunktfedern(mit, element, {{ort, feder, 0.0, "test"}});
    const DichteModen mo = dichteModen(ohne), mm = dichteModen(mit);
    MESSAGE("KQ46: f1 ohne Feder " << hz(mo.lambda(0)) << " Hz (Navier " << f1 << "), mit Feder " << hz(mm.lambda(0)) << " / "
                                   << hz(mm.lambda(1)) << " Hz, Reihe " << hz(soll[0]) << " / " << hz(soll[1]) << " Hz");
    CHECK(hz(mo.lambda(0)) == doctest::Approx(f1).epsilon(0.01));
    CHECK(hz(mm.lambda(0)) == doctest::Approx(hz(soll[0])).epsilon(0.01));
    CHECK(hz(mm.lambda(1)) == doctest::Approx(hz(soll[1])).epsilon(0.01));
    // M bleibt, K waechst nur um die Feder: die Spur von K steigt um k Σ N²
    const kp::Abtastung ab = kp::abtastung(ohne, element, ort);
    REQUIRE(ab.imNetz);
    double n2 = 0.0;
    for (const double gw : ab.gewichte) n2 += gw * gw;
    double spurOhne = 0.0, spurMit = 0.0;
    for (int i = 0; i < ohne.k.n; ++i)
    {
        spurOhne += ohne.k.wert(i, i);
        spurMit += mit.k.wert(i, i);
    }
    CHECK(spurMit - spurOhne == doctest::Approx(feder * n2).epsilon(1e-6));
    CHECK(mit.m.werte == ohne.m.werte);

    // --- gegen die Rechnung im Modenraum (wie `harfe_saiten.py`): K = diag(λ) + k φ φᵀ ---------------
    // mit allen Moden des Netzes dasselbe System; mit den ersten 60 nahe daran
    for (const int zahl : {static_cast<int>(mo.lambda.size()), 60})
    {
        Eigen::VectorXd phi(zahl);
        for (int r = 0; r < zahl; ++r)
        {
            const Eigen::VectorXd spalte = mo.formen.col(r);
            phi(r) = kp::wertBei(ab, std::vector<double>(spalte.data(), spalte.data() + spalte.size()));
        }
        Eigen::MatrixXd km = feder * phi * phi.transpose();
        for (int r = 0; r < zahl; ++r) km(r, r) += mo.lambda(r);
        const Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> l(km, Eigen::EigenvaluesOnly);
        CAPTURE(zahl);
        MESSAGE("KQ46 Modenraum mit " << zahl << " Moden: " << hz(l.eigenvalues()(0)) << " / " << hz(l.eigenvalues()(1)) << " Hz");
        const double grenze = zahl == static_cast<int>(mo.lambda.size()) ? 1e-6 : 0.005;
        CHECK(hz(l.eigenvalues()(0)) == doctest::Approx(hz(mm.lambda(0))).epsilon(grenze));
        CHECK(hz(l.eigenvalues()(1)) == doctest::Approx(hz(mm.lambda(1))).epsilon(grenze));
    }

    // --- am feineren Netz: die Feder wird nur wenig weicher ----------------------------------------
    {
        const kp::Netz fein = rechteckNetz(a, b, 0.045);
        REQUIRE(fein.fehler.empty());
        kp::PlattenSystem sf = kp::baueSystem(fein, element, steife, kp::Auflager::GelenkigAchsparallel);
        kp::setzePunktfedern(sf, element, {{ort, feder, 0.0, "test"}});
        const double feinHz = frequenzen(sf, 1)[0];
        MESSAGE("KQ46 Netz 7 cm (" << ohne.k.n << " Groessen) " << hz(mm.lambda(0)) << " Hz, 4,5 cm (" << sf.k.n << ") " << feinHz << " Hz");
        CHECK(feinHz == doctest::Approx(hz(mm.lambda(0))).epsilon(0.005));
    }

    // --- die Daempfung (KQ49 a): die Energie in der Feder traegt deren Verlustfaktor ----------------
    {
        const kp::Verformungsenergie energie(mit, element, steife, 1);
        kp::Daempfung gesetz;
        gesetz.c = 3.0;
        kp::Mode mode;
        mode.frequenzHz = hz(mm.lambda(0));
        const Eigen::VectorXd spalte = mm.formen.col(0);
        mode.form.assign(spalte.data(), spalte.data() + spalte.size());
        const double w = kp::wertBei(ab, mode.form);
        const double anteil = feder * w * w / mm.lambda(0);  // U_F / U bei einer bezueglich M normierten Form
        CHECK(anteil > 0.1);
        CHECK(anteil < 0.6);
        const double etaGesetz = gesetz.verlustfaktor(mode.frequenzHz);

        kp::Auswertung aus(mit, element, {}, {}, {}, gesetz, &energie, {0.0});
        // ohne Federn in der Auswertung: wie bisher das Gesetz
        CHECK(aus.werte(mode).verlustfaktor == etaGesetz);
        CHECK(aus.werte(mode).federAnteil == 0.0);
        aus.setzePunktfedern(mit, element, {{ort, feder, 0.0, "test"}});
        const kp::ModeImKoerper ohneVerlust = aus.werte(mode);
        CHECK(ohneVerlust.federAnteil == doctest::Approx(anteil).epsilon(1e-6));
        CHECK(ohneVerlust.verlustfaktor == doctest::Approx((1.0 - anteil) * etaGesetz).epsilon(1e-6));
        REQUIRE(ohneVerlust.werkstoffAnteil.size() == 1);
        CHECK(ohneVerlust.werkstoffAnteil[0] == doctest::Approx(1.0).epsilon(1e-12));  // auf die Werkstoffe normiert
        aus.setzePunktfedern(mit, element, {{ort, feder, 0.5, "test"}});
        CHECK(aus.werte(mode).verlustfaktor == doctest::Approx((1.0 - anteil) * etaGesetz + anteil * 0.5).epsilon(1e-6));
        // eine Feder mit dem Verlust des Gesetzes aendert nichts
        aus.setzePunktfedern(mit, element, {{ort, feder, etaGesetz, "test"}});
        CHECK(aus.werte(mode).verlustfaktor == doctest::Approx(etaGesetz).epsilon(1e-9));
        // ohne die Verformungsenergie: derselbe Anteil aus ½ (2πf)²
        kp::Auswertung schlicht(mit, element, {}, {}, {}, gesetz);
        schlicht.setzePunktfedern(mit, element, {{ort, feder, 0.0, "test"}});
        CHECK(schlicht.werte(mode).federAnteil == doctest::Approx(anteil).epsilon(1e-6));
    }

    // --- der Bauplan: `baueSystem` setzt seine Federn ------------------------------------------------
    kp::Bauplan plan;
    plan.titel = "Test";
    plan.materialien = {mat};
    plan.platte.material = "spruce";
    plan.platte.umriss = {{0.0, 0.0}, {a, 0.0}, {a, b}, {0.0, b}};
    plan.platte.dicke = h;
    REQUIRE(kp::pruefe(plan).empty());
    const kp::PlattenSystem planOhne = kp::baueSystem(netz, element, plan);
    kp::Bauplan gefedert = plan;
    gefedert.punktfedern = {{ort, feder, 0.0, "test"}, {{0.1, 0.1}, 0.25 * feder, 0.01, "test"}, {{0.2, 0.3}, 0.0, 0.0, "off"}};
    REQUIRE(kp::pruefe(gefedert).empty());
    const kp::PlattenSystem planMit = kp::baueSystem(netz, element, gefedert);
    REQUIRE(planMit.fehler.empty());
    // ohne Federn Bit fuer Bit die Platte wie bisher; `baueSystemOhnePunktfedern` laesst sie weg
    CHECK(kp::baueSystemOhnePunktfedern(netz, element, gefedert).k.werte == planOhne.k.werte);
    kp::PlattenSystem vonHand = planOhne;
    kp::setzePunktfedern(vonHand, element, gefedert.punktfedern);
    CHECK(planMit.k.werte == vonHand.k.werte);
    CHECK(planMit.m.werte == planOhne.m.werte);
    CHECK(planMit.masse == planOhne.masse);  // eine Feder wiegt nichts
    CHECK(frequenzen(planMit, 1)[0] > 1.05 * frequenzen(planOhne, 1)[0]);

    // was `pruefe` abweist
    {
        kp::Bauplan p = gefedert;
        p.punktfedern[0].steife = -1.0;
        CHECK_FALSE(kp::pruefe(p).empty());
        p = gefedert;
        p.punktfedern[0].verlustfaktor = -0.1;
        CHECK_FALSE(kp::pruefe(p).empty());
        p = gefedert;
        p.punktfedern[0].ort = {a + 0.1, 0.1};
        CHECK_FALSE(kp::pruefe(p).empty());
        p = gefedert;
        p.platte.loecher = {{{0.16, 0.14}, {0.21, 0.14}, {0.21, 0.19}, {0.16, 0.19}}};  // um die erste Feder
        CHECK_FALSE(kp::pruefe(p).empty());
    }

    // die Datei: `pointSprings` immer geschrieben (Fassung 9), Hin- und Rueckweg; Fassung 8 ohne das Feld
    CHECK(kp::kBauplanFassung >= 9);
    REQUIRE(alsJson(plan).contains(QStringLiteral("pointSprings")));
    CHECK(alsJson(plan).value(QStringLiteral("pointSprings")).toArray().isEmpty());
    const QJsonObject json = alsJson(gefedert);
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kBauplanFassung);
    const QJsonArray liste = json.value(QStringLiteral("pointSprings")).toArray();
    REQUIRE(liste.size() == 3);
    for (const char* feld : {"x", "y", "stiffness", "lossFactor", "source"}) CHECK(liste.at(1).toObject().contains(QLatin1String(feld)));
    CHECK(liste.at(1).toObject().value(QStringLiteral("lossFactor")).toDouble() == 0.01);
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == gefedert);
    QJsonObject acht = json;
    acht.remove(QStringLiteral("pointSprings"));
    acht[QStringLiteral("version")] = 8;
    kp::Bauplan alt;
    REQUIRE(ausJson(acht, alt));
    CHECK(alt.punktfedern.empty());
    CHECK(alt == plan);
    // der Abdruck haengt an den Federn
    CHECK(bauplanAbdruck(gefedert) != bauplanAbdruck(plan));
}
