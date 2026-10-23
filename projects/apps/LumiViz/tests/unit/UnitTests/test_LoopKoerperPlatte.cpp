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

#include "InstrumentBody/NetzCdt.hpp"
#include "InstrumentBody/Platte.hpp"
#include "loop/KoerperBauplan.hpp"

#include <Eigen/Dense>

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
        kp::baueSystem(netz, kp::LagrangeDreieck(k), [&](kp::Punkt2 p) { return kp::steifeBei(plan, p); }, kp::Auflager::Eingespannt);
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
